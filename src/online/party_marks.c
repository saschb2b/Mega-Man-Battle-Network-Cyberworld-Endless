/* party_marks.h */
#include "party_marks.h"

#include <stdio.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_team_view.h"
#include "bn6.h"
#include "crew_look.h"
#include "dive_words.h"
#include "gfx.h"
#include "hp_plate.h"
#include "online_transport.h"
#include "online_visit.h"
#include "pet_look.h"
#include "pixfont.h"
#include "platform.h"

#define BUDGET PARTY_MARKS_BUDGET
#define BELOW  1    /* a plate's top under his feet' line */
#define PAD    1    /* its plate's edge around the letters */
#define NAME_H (PIXFONT_CAP + 2 * PAD + 1)   /* a name's plate (+1: the shade's pixel) */
#define HP_GAP 1    /* a partner's HP under his name, this far */
#define CHIP_LINE 148   /* BN6's chip's name in a battle, its top on the picture (seen in the captures) */
#define BODY 47         /* a MegaMan on the field, from his feet to over his head (seen in the captures) */
#define BODY_HALF 17    /* ... and from his feet' middle to either side, his buster held out (seen in the captures) */
#define HALF 120        /* the player's side of the field, the picture's left half: a plate stays off the enemies' */
#define HUD_W 48        /* BN6's HP box and the emotion window under it, at the picture's top left */
#define HUD_H 34
#define GAUGE_H 15      /* BN6's Custom gauge, across the picture's top */

/* the partners' names in the crew's colour (crew_look.h, as on a layer) */
#define OWN_COLOUR  LOOK_ORANGE   /* the player's own: the PET cursor's orange */

_Static_assert(ONLINE_SEATS == PARTY_MOST, "a party's seats are START's");

static const SDL_Color shade = { 0, 24, 41, 255 };

/* each seat's mark as drawn (the player's own without its arrow) */
static char tags[PARTY_MOST][ONLINE_NAME_BYTES + 1];

void party_marks_cut(const char *name, const char *suffix, int budget, char *out, size_t size) {
	char take[ONLINE_NAME_BYTES + 1];
	size_t n = strlen(name);
	if (n > ONLINE_NAME_BYTES) n = ONLINE_NAME_BYTES;
	for (;; --n) {
		snprintf(take, sizeof take, "%.*s", (int)n, name);
		while (n && take[n - 1] == ' ') take[--n] = 0;
		snprintf(out, size, "%s%s", take, suffix);
		if (!n || pixfont_width(out, 1) <= budget) return;
	}
}

static const char *named(const OnlineSnapshot *s, uint32_t id) {
	for (size_t i = 0; id && s->connected && i < s->peer_count; ++i)
		if (s->peers[i].id == id) return s->peers[i].name;
	return "";
}

void party_marks_seats(const uint32_t seats[PARTY_MOST], unsigned mine) {
	static OnlineSnapshot s;   /* (large: kept off the stack) */
	const char *names[PARTY_MOST];
	int arrow = pixfont_width(PIXFONT_PLAY, 1) + 1;
	if (seats) online_transport_snapshot(&s);
	for (unsigned k = 0; k < PARTY_MOST; ++k) {
		names[k] = !seats ? "" : k == mine ? online_visit_name() : named(&s, seats[k]);
		if (!names[k][0]) {
			snprintf(tags[k], sizeof tags[k], "P%u", k + 1);
			continue;
		}
		party_marks_cut(names[k], "", k == mine ? BUDGET - arrow : BUDGET, tags[k], sizeof tags[k]);
	}
	/* (two names alike as cut: each its seat's digit after it) */
	for (unsigned a = 0; a < PARTY_MOST; ++a)
		for (unsigned b = a + 1; b < PARTY_MOST; ++b) {
			if (strcmp(tags[a], tags[b]) || !names[a][0] || !names[b][0]) continue;
			for (unsigned k = a; k <= b; k += b - a) {
				char digit[2] = { (char)('1' + k), 0 };
				party_marks_cut(names[k], digit, k == mine ? BUDGET - arrow : BUDGET, tags[k], sizeof tags[k]);
			}
		}
}

/* the field as seat `mine` sees it: the HP box his (0: BN6's own, the lead's) and each MegaMan standing, where */
typedef struct {
	unsigned hp_box, count, seat[PARTY_MOST], hp[PARTY_MOST];
	uint32_t object[PARTY_MOST];
	int x[PARTY_MOST], y[PARTY_MOST];
	bool open, own_box;
} Seen;

static bool seen(struct mCore *c, unsigned mine, unsigned n, Seen *v) {
	if (!c || n < 2 || mine >= PARTY_MOST) return false;
	memset(v, 0, sizeof *v);
	v->open = (c->rawRead8(c, BN6_BATTLE_PHASE, -1) & 0xFF) == BN6_PHASE_CUSTOM;
	int x, y;
	unsigned hp;
	/* (a partner's HP his own, over BN6's box, which shows the lead's: 0 once he is down, never the lead's, issue #780) */
	if (mine) {
		uint32_t o = arena_team_view_megaman(c, 0, mine);
		v->own_box = true;
		v->hp_box = o >= BN6_EWRAM && o < BN6_EWRAM_END ? (unsigned)(uint16_t)c->rawRead16(c, o + BN6_T1_HP, -1) : 0;
	}
	for (unsigned k = 0; k < n && k < PARTY_MOST && !v->open; ++k) {
		uint32_t o = arena_team_view_megaman(c, 0, k);
		if (!arena_team_view_feet(c, o, 0, &x, &y, &hp)) continue;
		v->seat[v->count] = k;
		v->object[v->count] = o;
		v->hp[v->count] = hp;
		v->x[v->count] = x;
		v->y[v->count++] = y;
	}
	return true;
}

/* where each MegaMan's plate stands on the picture (issue #886): the first of four spots beside him that covers no
 * MegaMan's body, no plate placed before it, nor BN6's HP box, emotion window or gauge, nor the enemies' half, nor
 * BN6's chip name at the picture's foot. Under his feet first, where BN6 writes an enemy's HP; then in front of him
 * (the next column, the plate's foot on his feet' line), behind him, over his head. Where none is clear, the one
 * that covers least. The player's own is placed first, then the partners by seat */
typedef struct { int left, top, w, h; } Box;
typedef struct { Box pair; int name_w, arrow, digits; } Placed;

static bool meets(Box a, Box b) {
	return a.left < b.left + b.w && b.left < a.left + a.w && a.top < b.top + b.h && b.top < a.top + a.h;
}

static int covered(Box a, Box b) {
	if (!meets(a, b)) return 0;
	int l = a.left > b.left ? a.left : b.left, t = a.top > b.top ? a.top : b.top;
	int r = a.left + a.w < b.left + b.w ? a.left + a.w : b.left + b.w, d = a.top + a.h < b.top + b.h ? a.top + a.h : b.top + b.h;
	return (r - l) * (d - t);
}

/* a MegaMan's body on the picture, his feet at (x, y) */
static Box body(int x, int y) { return (Box){ x - BODY_HALF, y - BODY, 2 * BODY_HALF, BODY + 1 }; }

static unsigned digits(unsigned hp) {
	unsigned n = 1;
	for (hp = hp > 9999 ? 9999 : hp; hp >= 10; hp /= 10) ++n;
	return n;
}

/* the spot `k` (0 under, 1 in front, 2 behind, 3 over) of a plate w x h for a MegaMan at (x, y); false where it leaves
 * the side's half of the picture or meets BN6's own pieces */
static bool spot(int k, int x, int y, int w, int h, Box *b) {
	static const Box hud[] = { { 0, 0, HUD_W, HUD_H }, { 0, 0, 240, GAUGE_H } };
	*b = (Box){ x - w / 2, y + BELOW, w, h };
	if (k == 1) *b = (Box){ x + BODY_HALF + 1, y - h, w, h };
	if (k == 2) *b = (Box){ x - BODY_HALF - 1 - w, y - h, w, h };
	if (k == 3) b->top = y - BODY - h;
	if (b->left < 0 || b->left + b->w > HALF || b->top + b->h > CHIP_LINE) return false;
	for (size_t i = 0; i < sizeof hud / sizeof hud[0]; ++i)
		if (meets(*b, hud[i])) return false;
	return true;
}

static void layout(const Seen *v, unsigned mine, int ox, int oy, Placed out[PARTY_MOST]) {
	Box bodies[PARTY_MOST], done[PARTY_MOST];
	unsigned order[PARTY_MOST], n = 0, placed = 0;
	for (unsigned i = 0; i < v->count; ++i) {
		bodies[i] = body(v->x[i] - ox, v->y[i] - oy);
		if (v->seat[i] == mine) order[n++] = i;
	}
	for (unsigned i = 0; i < v->count; ++i)
		if (v->seat[i] != mine) order[n++] = i;
	for (unsigned o = 0; o < n; ++o) {
		unsigned i = order[o];
		bool own = v->seat[i] == mine;
		Placed *p = &out[i];
		p->arrow = own ? pixfont_width(PIXFONT_PLAY, 1) + 1 : 0;
		p->name_w = pixfont_width(tags[v->seat[i]], 1) + p->arrow + 2 * PAD + 1;   /* (+1: the shade's pixel) */
		p->digits = own ? 0 : (int)digits(v->hp[i]) * BN6_HP_DIGIT_STEP;
		int w = p->name_w > p->digits ? p->name_w : p->digits, h = NAME_H + (own ? 0 : HP_GAP + BN6_HP_DIGIT_H);
		int x = v->x[i] - ox, y = v->y[i] - oy, least = -1;
		for (int k = 0; k < 4; ++k) {
			Box b;
			if (!spot(k, x, y, w, h, &b)) continue;
			int cost = 0;
			for (unsigned j = 0; j < v->count; ++j) cost += covered(b, bodies[j]);
			for (unsigned j = 0; j < placed; ++j) cost += covered(b, done[j]);
			if (least < 0 || cost < least) { least = cost; p->pair = b; }
			if (!cost) break;
		}
		if (least < 0) p->pair = (Box){ x - w / 2, y + BELOW, w, h };
		done[placed++] = p->pair;
	}
}

/* a mark: the player's own behind the PET's arrow, in its orange; on a plate, so that it reads over any panel or sprite;
 * a partner's HP under his name, as BN6 writes an enemy's under him and in its digits (his own is in the HP box; issue
 * #717) */
void party_marks_draw(struct mCore *c, unsigned mine, unsigned n) {
	static HpPlate hp[PARTY_MOST];   /* (each partner's number as last drawn: the drawing's own) */
	Seen v;
	Placed m[PARTY_MOST];
	if (!seen(c, mine, n, &v)) return;
	if (v.own_box) arena_team_view_hp(v.hp_box, v.open);
	layout(&v, mine, P.core_x, P.core_y, m);
	for (unsigned i = 0; i < v.count; ++i) {
		bool own = v.seat[i] == mine;
		const char *tag = tags[v.seat[i]];
		int cx = P.core_x + m[i].pair.left + m[i].pair.w / 2, left = cx - m[i].name_w / 2, top = P.core_y + m[i].pair.top;
		fill_rect(left, top, m[i].name_w, NAME_H, rgba(0, 24, 41, 190));
		if (own) pixfont_draw(left + PAD, top + PAD, PIXFONT_PLAY, OWN_COLOUR, shade, PIXFONT_LEFT, 1);
		pixfont_draw(left + PAD + m[i].arrow, top + PAD, tag, own ? OWN_COLOUR : CREW_COLOUR, shade, PIXFONT_LEFT, 1);
		if (!own) hp_plate_draw(&hp[v.seat[i]], c, v.object[i], cx, top + NAME_H + HP_GAP - BN6_HP_DIGIT_TOP);
	}
}

void party_marks_down(bool spent) {
	look_band(P.core_x, P.core_y + 160 - LOOK_BAND_H - 1, 240, dive_down_crew_words(spent));
}

void party_marks_describe(FILE *f, struct mCore *c, unsigned mine, unsigned n) {
	Seen v;
	Placed m[PARTY_MOST];
	if (!seen(c, mine, n, &v)) return;
	fprintf(f, "hpbox %s\n", v.own_box ? "own" : "bn6");
	if (v.own_box) fprintf(f, "hpbox_hp %u\n", v.hp_box);
	/* (where his mark stands on the 240x160 picture: its name's plate's left, top and width; its whole plate, the HP under
	 * the name, and his body; whether any two names meet, and whether any plate meets a MegaMan's body: issue #886) */
	layout(&v, mine, P.core_x, P.core_y, m);
	bool apart = true, clear = true;
	for (unsigned i = 0; i < v.count; ++i) {
		unsigned k = v.seat[i];
		Box name = { m[i].pair.left + (m[i].pair.w - m[i].name_w) / 2, m[i].pair.top, m[i].name_w, NAME_H };
		Box b = body(v.x[i] - P.core_x, v.y[i] - P.core_y), q = m[i].pair;
		fprintf(f, "mark%u %s at %d %d width %d hp %u tag %s\n", k, k == mine ? "own" : "crew", name.left, name.top, name.w, v.hp[i], tags[k]);
		fprintf(f, "plate%u %d %d %d %d seat %d %d %d %d\n", k, q.left, q.top, q.w, q.h, b.left, b.top, b.w, b.h);
		for (unsigned j = 0; j < v.count; ++j) {
			Box o = m[j].pair;
			Box on = { o.left + (o.w - m[j].name_w) / 2, o.top, m[j].name_w, NAME_H };
			if (j < i) apart &= !meets(name, on);
			clear &= !meets(q, body(v.x[j] - P.core_x, v.y[j] - P.core_y)) && (j == i || !meets(q, o));
		}
	}
	fprintf(f, "marks %u %s\n", v.count, apart ? "apart" : "MEET");
	fprintf(f, "plates %u %s\n", v.count, clear ? "clear" : "COVER");
}


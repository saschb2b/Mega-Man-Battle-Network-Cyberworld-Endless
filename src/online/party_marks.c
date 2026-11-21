/* party_marks.h */
#include "party_marks.h"

#include <stdio.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_team_view.h"
#include "bn6.h"
#include "crew_look.h"
#include "gfx.h"
#include "online_transport.h"
#include "online_visit.h"
#include "pet_look.h"
#include "pixfont.h"
#include "platform.h"

#define BUDGET PARTY_MARKS_BUDGET
#define BELOW  1    /* a mark's top under his feet' line */
#define PAD    1    /* its plate's edge around the letters */
#define CHIP_LINE 148   /* BN6's chip's name in a battle, its top on the picture (seen in the captures) */

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

/* a mark's plate centred under a MegaMan's feet (x, y): its left, top, width and height on the screen */
typedef struct { int left, top, w, h, arrow; } Plate;

static Plate plate(int x, int y, const char *tag, bool own) {
	Plate p;
	p.arrow = own ? pixfont_width(PIXFONT_PLAY, 1) + 1 : 0;
	p.w = pixfont_width(tag, 1) + p.arrow + 2 * PAD + 1;   /* (+1: the shade's pixel) */
	p.h = PIXFONT_CAP + 2 * PAD + 1;
	p.left = x - p.w / 2;
	p.top = y + BELOW;
	return p;
}

/* a mark: the player's own behind the PET's arrow, in its orange; on a plate, so that it reads over any panel or sprite */
static void mark(int x, int y, const char *tag, bool own) {
	Plate p = plate(x, y, tag, own);
	fill_rect(p.left, p.top, p.w, p.h, rgba(0, 24, 41, 190));
	if (own) pixfont_draw(p.left + PAD, p.top + PAD, PIXFONT_PLAY, OWN_COLOUR, shade, PIXFONT_LEFT, 1);
	pixfont_draw(p.left + PAD + p.arrow, p.top + PAD, tag, own ? OWN_COLOUR : CREW_COLOUR, shade, PIXFONT_LEFT, 1);
}

/* the field as seat `mine` sees it: the HP box his (0: BN6's own, the lead's) and each MegaMan standing, where */
typedef struct { unsigned hp_box, count, seat[PARTY_MOST], hp[PARTY_MOST]; int x[PARTY_MOST], y[PARTY_MOST]; bool open; } Seen;

static bool seen(struct mCore *c, unsigned mine, unsigned n, Seen *v) {
	if (!c || n < 2 || mine >= PARTY_MOST) return false;
	memset(v, 0, sizeof *v);
	v->open = (c->rawRead8(c, BN6_BATTLE_PHASE, -1) & 0xFF) == BN6_PHASE_CUSTOM;
	int x, y;
	unsigned hp;
	/* (a partner's HP his own, over BN6's box, which shows the lead's) */
	if (mine && arena_team_view_feet(c, arena_team_view_megaman(c, 0, mine), 0, &x, &y, &hp)) v->hp_box = hp;
	for (unsigned k = 0; k < n && k < PARTY_MOST && !v->open; ++k)
		if (arena_team_view_feet(c, arena_team_view_megaman(c, 0, k), 0, &x, &y, &hp)) {
			v->seat[v->count] = k;
			v->hp[v->count] = hp;
			v->x[v->count] = x;
			v->y[v->count++] = y;
		}
	return true;
}

void party_marks_draw(struct mCore *c, unsigned mine, unsigned n) {
	Seen v;
	if (!seen(c, mine, n, &v)) return;
	if (v.hp_box) arena_team_view_hp(v.hp_box, v.open);
	for (unsigned i = 0; i < v.count; ++i) {
		bool own = v.seat[i] == mine;
		mark(v.x[i], v.y[i], tags[v.seat[i]], own);
		/* (a partner's HP under his name, as BN6 writes an enemy's under him: his own is in the HP box; issue #717) */
		if (own) continue;
		char n[8];
		Plate p = plate(v.x[i], v.y[i], tags[v.seat[i]], false);
		snprintf(n, sizeof n, "%u", v.hp[i]);
		/* (on the bottom row, above his name: under it is BN6's chip's name at the picture's foot) */
		int hp_y = p.top + p.h + 1;
		if (hp_y + PIXFONT_H > P.core_y + CHIP_LINE) hp_y = p.top - PIXFONT_CAP - 2;
		pixfont_draw(v.x[i], hp_y, n, LOOK_TEXT, shade, PIXFONT_CENTER, 1);
	}
}

void party_marks_describe(FILE *f, struct mCore *c, unsigned mine, unsigned n) {
	Seen v;
	if (!seen(c, mine, n, &v)) return;
	fprintf(f, "hpbox %s\n", v.hp_box ? "own" : "bn6");
	if (v.hp_box) fprintf(f, "hpbox_hp %u\n", v.hp_box);
	/* (where his mark stands on the 240x160 picture: its plate's left, top and width; and whether any two meet) */
	Plate m[PARTY_MOST];
	bool apart = true;
	for (unsigned i = 0; i < v.count; ++i) {
		unsigned k = v.seat[i];
		m[i] = plate(v.x[i] - P.core_x, v.y[i] - P.core_y, tags[k], k == mine);
		fprintf(f, "mark%u %s at %d %d width %d hp %u tag %s\n", k, k == mine ? "own" : "crew", m[i].left, m[i].top, m[i].w, v.hp[i], tags[k]);
		for (unsigned j = 0; j < i; ++j)
			apart &= m[i].left + m[i].w <= m[j].left || m[j].left + m[j].w <= m[i].left || m[i].top + m[i].h <= m[j].top ||
				m[j].top + m[j].h <= m[i].top;
	}
	fprintf(f, "marks %u %s\n", v.count, apart ? "apart" : "MEET");
}

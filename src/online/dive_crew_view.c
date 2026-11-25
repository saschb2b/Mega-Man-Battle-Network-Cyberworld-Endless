/* dive_crew_view.h */
#include "dive_crew_view.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "crew_look.h"
#include "director.h"
#include "emu.h"
#include "dive_crew.h"
#include "dive_party.h"
#include "dive_wire.h"
#include "gfx.h"
#include "online_transport.h"
#include "party_marks.h"
#include "party_play.h"
#include "pet_look.h"
#include "pixfont.h"
#include "platform.h"
#include "talk.h"

/* the plates' column, under BN6's HP box on the map (its 44 x 16 at the
 * picture's top left, arena_team_view.c's measure), a line each */
#define LEFT    2
#define TOP     18
#define LINE    12
#define PAD     2
#define H       (PIXFONT_CAP + 2 + 2)   /* letters, a row, the bar */
#define GAP     3
#define HP_GAP  6     /* ... and after the layer, before the HP: "L1 100/100", not "L1100/100" (issue #781) */
#define WHERE_MS 250        /* where MegaMan is, said at most so often */

static const SDL_Color shade = { 0, 24, 41, 255 };

static struct {
	bool linked;
	uint32_t me;
	int hp, max;          /* MegaMan's as last sent (-1 none since the link came) */
	uint32_t sent_at;
	int where;            /* where MegaMan was last said to be (-1 not yet), ONLINE_WHERE_* */
	uint32_t where_at;
} V = { false, 0, -1, -1, 0, -1, 0 };

/* MegaMan's own entry in the crew (his layer the server's word), NULL none */
static const CrewMember *own(void);

/* where MegaMan is (issue #756): home between acts (drawn on no layer for the others), a battle (his own, or a party's
 * field), or the layer */
static int where_now(void) {
	if (director_in_town() || director_in_hp()) return ONLINE_WHERE_HOME;
	if (director_in_battle() || party_play_held()) return ONLINE_WHERE_BATTLE;
	/* (v29, issue #758: on the layer, not free: a chat, the PET, a shop) */
	if (!director_on_map() || emu_read8(BN6_CHATBOX) || talk_busy()) return ONLINE_WHERE_BUSY;
	return ONLINE_WHERE_LAYER;
}

/* where it changed, said: a quarter of a second apart at most, its latest */
static void where_frame(uint32_t now) {
	int w = where_now();
	if (w == V.where || now - V.where_at < WHERE_MS) return;
	uint8_t m[ONLINE_DIVE_WHERE_BYTES];
	size_t n = dive_wire_where(m, (unsigned)w);
	if (!n || !online_transport_send(m, n)) return;
	fprintf(stderr, "dive: MegaMan %s\n", w == ONLINE_WHERE_HOME ? "home" : w == ONLINE_WHERE_BATTLE ? "in a battle" : w == ONLINE_WHERE_BUSY ? "busy" : "on the layer");
	V.where = w;
	V.where_at = now;
}

void dive_crew_view_frame(bool linked, uint32_t me, uint32_t now) {
	V.linked = linked;
	V.me = me;
	if (!linked) { V.hp = V.max = V.where = -1; return; }
	where_frame(now);
	int hp, max;
	/* (on a party's field his box is the field's: the run's core stands still under it) */
	if (!party_play_hp(&hp, &max)) director_megaman_hp(&hp, &max);
	/* (sent where it changed, or where the crew's word of it differs: one sent before the server had him in a room is
	 * dropped there, and this sends it again) */
	const CrewMember *mine = own();
	bool told = hp == V.hp && max == V.max && (!mine || (mine->hp == hp && mine->max_hp == max));
	if (told || now - V.sent_at < ONLINE_CREW_VITALS_MS) return;
	uint8_t m[ONLINE_DIVE_VITALS_BYTES];
	size_t n = hp >= 0 && max > 0 ? dive_wire_vitals(m, (unsigned)hp, (unsigned)max) : 0;
	if (!n || !online_transport_send(m, n)) return;
	V.hp = hp;
	V.max = max;
	V.sent_at = now;
}

static const CrewMember *own(void) {
	const CrewMember *m;
	for (int k = 0; (m = dive_crew_member(k)) != NULL; ++k)
		if (m->session == V.me) return m;
	return NULL;
}

/* a crewmate's row: his name, HP and layer as the crew's word has them */
static void crew_row(CrewRow *r, const CrewMember *m, const CrewMember *me) {
	r->session = m->session;
	r->crewmate = true;
	r->now = m->hp;
	r->most = m->max_hp;
	snprintf(r->full, sizeof r->full, "%s", m->name);
	party_marks_cut(m->name, "", PARTY_MARKS_BUDGET, r->name, sizeof r->name);
	if (m->max_hp) snprintf(r->hp, sizeof r->hp, "%u/%u", m->hp, m->max_hp);
	else snprintf(r->hp, sizeof r->hp, "--");
	r->layer[0] = 0;
	/* (where he is: home between acts, a battle, busy, as a mark on his plate, the owner's call of 22 November 2026, issue
	 * #758; another layer; nothing where he stands on MegaMan's) */
	r->mark = m->where != ONLINE_WHERE_LAYER;
	if (m->where == ONLINE_WHERE_HOME) snprintf(r->layer, sizeof r->layer, "HOME");
	else if (m->where == ONLINE_WHERE_BATTLE) snprintf(r->layer, sizeof r->layer, "BATTLE");
	else if (m->where == ONLINE_WHERE_BUSY) snprintf(r->layer, sizeof r->layer, "BUSY");
	else if (m->depth && (!me || me->depth != m->depth)) snprintf(r->layer, sizeof r->layer, "L%u", m->depth);
}

/* the party forming at a guardian's door, leader first (issue #753): a
 * crewmate as the crew's rows have him, another diver his name alone */
static int party_rows(CrewRow out[ONLINE_PARTY_MOST], const QueueEntry *e, int count, const CrewMember *me) {
	int n = 0;
	for (int k = 0; k < count && n < ONLINE_PARTY_MOST; ++k) {
		if (e[k].session == V.me) continue;
		const CrewMember *m = NULL;
		for (int j = 0; (m = dive_crew_member(j)) != NULL && m->session != e[k].session; ++j) {}
		CrewRow *r = &out[n++];
		if (m) { crew_row(r, m, me); continue; }
		memset(r, 0, sizeof *r);
		r->session = e[k].session;
		snprintf(r->full, sizeof r->full, "%s", e[k].name);
		party_marks_cut(e[k].name, "", PARTY_MARKS_BUDGET, r->name, sizeof r->name);
	}
	return n;
}

int dive_crew_view_rows(CrewRow out[ONLINE_PARTY_MOST]) {
	const CrewMember *m, *me = own();
	const QueueEntry *e;
	int n = 0, party = dive_party_members(&e);
	if (!V.linked) return 0;
	if (party) return party_rows(out, e, party, me);
	if (dive_crew_count() < 2) return 0;
	for (int k = 0; (m = dive_crew_member(k)) != NULL; ++k)
		if (m->session != V.me) crew_row(&out[n++], m, me);
	return n;
}

void dive_crew_view_bar(int x, int y, int w, const CrewRow *r) {
	int bar = r->most ? w * r->now / r->most : 0;
	fill_rect(x, y, w, 1, LOOK_NAVY);
	if (bar) fill_rect(x, y, bar, 1, r->now * 4 <= r->most ? LOOK_ORANGE : LOOK_GREEN_HI);
}

static SDL_Color mark_fill(const char *mark) {
	return mark[1] == 'A' ? LOOK_ORANGE : mark[1] == 'U' ? LOOK_GOLD : LOOK_NAVY;   /* BATTLE, BUSY, HOME */
}

/* the rows now and their columns' widths: the rows' count, one width for every plate */
static int measure(CrewRow w[ONLINE_PARTY_MOST], int *name_w, int *layer_w, int *width) {
	int n = dive_crew_view_rows(w), hp_w = 0;
	*name_w = *layer_w = 0;
	for (int i = 0; i < n; ++i) {
		/* (a mark in the tag's own small capitals, a margin each side) */
		int a = pixfont_width(w[i].name, 1), c = pixfont_width(w[i].hp, 1);
		int b = !w[i].layer[0] ? 0 : (w[i].mark ? look_caps_width(w[i].layer) + 4 : pixfont_width(w[i].layer, 1)) + HP_GAP;
		if (a > *name_w) *name_w = a;
		if (b > *layer_w) *layer_w = b;
		if (c > hp_w) hp_w = c;
	}
	/* (one width for every plate: the names, layers and HP in columns) */
	*width = PAD + *name_w + GAP + *layer_w + hp_w + PAD + 1;
	return n;
}

bool dive_crew_view_box(int *x, int *y, int *w, int *h) {
	CrewRow r[ONLINE_PARTY_MOST];
	int name_w, layer_w, n = measure(r, &name_w, &layer_w, w);
	*x = LEFT;
	*y = TOP;
	*h = n ? (n - 1) * LINE + H : 0;
	return n > 0;
}

void dive_crew_view_draw(void) {
	CrewRow w[ONLINE_PARTY_MOST];
	int name_w, layer_w, width, n = measure(w, &name_w, &layer_w, &width);
	for (int i = 0; i < n; ++i) {
		int x = P.core_x + LEFT, y = P.core_y + TOP + i * LINE;
		fill_rect(x, y, width, H, rgba(0, 24, 41, 190));
		pixfont_draw(x + PAD, y + 1, w[i].name, w[i].crewmate ? CREW_COLOUR : LOOK_TEXT, shade, PIXFONT_LEFT, 1);
		/* (where he is: a mark as the information box tags a key item, BN6's PET piece, its colour the state's; another
		 * layer in plain letters) */
		if (w[i].mark) look_tag(x + PAD + name_w + GAP - 1, y, layer_w - HP_GAP, H - 2, w[i].layer, mark_fill(w[i].layer));
		else if (w[i].layer[0]) pixfont_draw(x + PAD + name_w + GAP, y + 1, w[i].layer, LOOK_EDGE_HI, shade, PIXFONT_LEFT, 1);
		pixfont_draw(x + width - PAD - 1, y + 1, w[i].hp, LOOK_TEXT, shade, PIXFONT_RIGHT, 1);
		/* (his HP as a bar under it, the plate's width his max; orange from a quarter down) */
		if (w[i].crewmate) dive_crew_view_bar(x + 1, y + H - 2, width - 2, &w[i]);
	}
}

void dive_crew_view_describe(FILE *f) {
	CrewRow w[ONLINE_PARTY_MOST];
	int n = dive_crew_view_rows(w);
	const QueueEntry *e;
	fprintf(f, "crewview %d%s\n", n, dive_party_members(&e) ? " party" : "");
	for (int i = 0; i < n; ++i) {
		const char *at = !w[i].layer[0] ? "here" : !w[i].mark ? w[i].layer + 1 : w[i].layer[1] == 'A' ? "battle" : w[i].layer[1] == 'U' ? "busy" : "home";
		fprintf(f, "mate%d %s hp %s layer %s%s\n", i, w[i].full, w[i].hp[0] ? w[i].hp : "none", at, w[i].crewmate ? "" : " stranger");
	}
}

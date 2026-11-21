/* dive_crew_view.h */
#include "dive_crew_view.h"

#include <stdio.h>
#include <string.h>

#include "crew_look.h"
#include "director.h"
#include "dive_crew.h"
#include "dive_wire.h"
#include "gfx.h"
#include "online_transport.h"
#include "party_marks.h"
#include "pet_look.h"
#include "pixfont.h"
#include "platform.h"

/* the plates' column, under BN6's HP box on the map (its 44 x 16 at the
 * picture's top left, arena_team_view.c's measure), a line each */
#define LEFT    2
#define TOP     18
#define LINE    12
#define PAD     2
#define H       (PIXFONT_CAP + 2 + 2)   /* letters, a row, the bar */
#define GAP     3

static const SDL_Color shade = { 0, 24, 41, 255 };

static struct {
	bool linked;
	uint32_t me;
	int hp, max;          /* MegaMan's as last sent (-1 none since the link came) */
	uint32_t sent_at;
} V = { false, 0, -1, -1, 0 };

/* MegaMan's own entry in the crew (his layer the server's word), NULL none */
static const CrewMember *own(void);

void dive_crew_view_frame(bool linked, uint32_t me, uint32_t now) {
	V.linked = linked;
	V.me = me;
	if (!linked) { V.hp = V.max = -1; return; }
	int hp, max;
	director_megaman_hp(&hp, &max);
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

/* the crewmates shown, in the order they came: none alone or with the link down */
static int mates(const CrewMember *out[ONLINE_PARTY_MOST]) {
	const CrewMember *m;
	int n = 0;
	if (!V.linked || dive_crew_count() < 2) return 0;
	for (int k = 0; (m = dive_crew_member(k)) != NULL; ++k)
		if (m->session != V.me) out[n++] = m;
	return n;
}

/* a plate's words: the name cut, HP as BN6 writes it over max HP ("--" not yet told), and the layer where not MegaMan's */
typedef struct { char name[ONLINE_NAME_BYTES + 1], hp[12], layer[6]; } Words;

static Words words(const CrewMember *m, const CrewMember *me) {
	Words w;
	party_marks_cut(m->name, "", PARTY_MARKS_BUDGET, w.name, sizeof w.name);
	if (m->max_hp) snprintf(w.hp, sizeof w.hp, "%u/%u", m->hp, m->max_hp);
	else snprintf(w.hp, sizeof w.hp, "--");
	w.layer[0] = 0;
	if (m->depth && (!me || me->depth != m->depth)) snprintf(w.layer, sizeof w.layer, "L%u", m->depth);
	return w;
}

void dive_crew_view_draw(void) {
	const CrewMember *m[ONLINE_PARTY_MOST], *me = own();
	int n = mates(m);
	Words w[ONLINE_PARTY_MOST];
	int name_w = 0, layer_w = 0, hp_w = 0;
	for (int i = 0; i < n; ++i) {
		w[i] = words(m[i], me);
		int a = pixfont_width(w[i].name, 1), b = w[i].layer[0] ? pixfont_width(w[i].layer, 1) + GAP : 0, c = pixfont_width(w[i].hp, 1);
		if (a > name_w) name_w = a;
		if (b > layer_w) layer_w = b;
		if (c > hp_w) hp_w = c;
	}
	/* (one width for every plate: the names, layers and HP in columns) */
	int width = PAD + name_w + GAP + layer_w + hp_w + PAD + 1;
	for (int i = 0; i < n; ++i) {
		int x = P.core_x + LEFT, y = P.core_y + TOP + i * LINE;
		fill_rect(x, y, width, H, rgba(0, 24, 41, 190));
		pixfont_draw(x + PAD, y + 1, w[i].name, CREW_COLOUR, shade, PIXFONT_LEFT, 1);
		if (w[i].layer[0]) pixfont_draw(x + PAD + name_w + GAP, y + 1, w[i].layer, LOOK_EDGE_HI, shade, PIXFONT_LEFT, 1);
		pixfont_draw(x + width - PAD - 1, y + 1, w[i].hp, LOOK_TEXT, shade, PIXFONT_RIGHT, 1);
		/* (his HP as a bar under it, the plate's width his max; orange from a quarter down) */
		int full = width - 2, bar = m[i]->max_hp ? full * m[i]->hp / m[i]->max_hp : 0;
		fill_rect(x + 1, y + H - 2, full, 1, LOOK_NAVY);
		if (bar) fill_rect(x + 1, y + H - 2, bar, 1, m[i]->hp * 4 <= m[i]->max_hp ? LOOK_ORANGE : LOOK_GREEN_HI);
	}
}

void dive_crew_view_describe(FILE *f) {
	const CrewMember *m[ONLINE_PARTY_MOST], *me = own();
	int n = mates(m);
	fprintf(f, "crewview %d\n", n);
	for (int i = 0; i < n; ++i) {
		Words w = words(m[i], me);
		fprintf(f, "mate%d %s hp %s layer %s\n", i, m[i]->name, w.hp, w.layer[0] ? w.layer + 1 : "here");
	}
}

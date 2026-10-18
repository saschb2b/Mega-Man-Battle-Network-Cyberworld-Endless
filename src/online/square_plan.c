#include "square_plan.h"

#include <string.h>

/* The areas a Square wears (rom.c's net areas: Central, Sky and Green
 * Area, BN6's busy daylight nets), one picked by the seed; not Seaside,
 * whose layers stand in another map's slot (rom.c's host) */
static const int areas[] = { 0, 2, 3 };

/* The plaza (13 panels a side) and its middle, where the landmark stands */
#define PLAZA0 11
#define PLAZA1 23
#define MID 17

static void floor_box(SquarePlan *p, int x0, int y0, int x1, int y1) {
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) p->cell[y][x] = C_PATH;
}

static void raise_box(SquarePlan *p, int x0, int y0, int x1, int y1) {
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) p->level[y][x] = 1;
}

/* A prop on (x, y): in a walled hole where that is floor, else in the void
 * past a rim (docs/LEVEL_DESIGN.md, Props: edge or hole). */
static void prop(SquarePlan *p, int look, int x, int y) {
	if (p->nprops >= SQUARE_PROPS) return;
	p->props[p->nprops++] = (NetProp){ .kind = PROP_SPRITE, .x = x, .y = y, .look = look };
	if (p->cell[y][x]) p->cell[y][x] = C_SOLID;
}

static void place(SquarePlan *p, int k, int x, int y) { p->at[k][0] = x; p->at[k][1] = y; }

/* A raised room (the stage, the ring): its floor, and its stair two panels
 * wide on its +y side at (sx, sy), climbing towards -y as Sky's maps'
 * stairs do, its foot on the floor below (net_height.c's rules: nothing
 * else within a panel of where the room is drawn, a panel up the
 * screen). */
static void dais(SquarePlan *p, int x0, int y0, int x1, int y1, int sx) {
	floor_box(p, x0, y0, x1, y1);
	raise_box(p, x0, y0, x1, y1);
	floor_box(p, sx, y1 + 1, sx + 1, y1 + 2);
	p->stairs[p->nstairs++] = (Stair){ .x = sx, .y = y1 + 1, .dir = STAIR_UP_NY };
}

/* The floor: the plaza; the log-in plaza (+x) off a walkway; the stage
 * (-y) up its stair from the plaza's edge; Trade Central (-x) off a
 * walkway; the Arena (+y): a walk down its spectators' floor, which turns
 * round before its ring, the ring up its stair from that floor. */
static void floors(SquarePlan *p) {
	floor_box(p, PLAZA0, PLAZA0, PLAZA1, PLAZA1);
	floor_box(p, 24, 17, 26, 18); floor_box(p, 27, 14, 32, 21);
	dais(p, 15, 4, 19, 7, 16);
	floor_box(p, 16, 10, 17, 10);
	floor_box(p, 9, 17, 10, 18); floor_box(p, 3, 15, 8, 20);
	floor_box(p, 19, 24, 21, 31); floor_box(p, 12, 32, 21, 33);
	dais(p, 12, 26, 16, 29, 13);
	/* the pad home at the log-in plaza's far end */
	for (int y = 17; y <= 18; ++y)
		for (int x = 31; x <= 32; ++x) p->pad[y][x] = 1;
}

/* The landmark ringed by four props in holes round it, the area's own
 * first: Green's giant cybertree among cybertrees; elsewhere the statue or
 * the monument by the seed, braziers or cybertrees round it. */
static void centrepiece(SquarePlan *p, uint32_t pick) {
	int ring = LOOK_BRAZIER;
	if (p->area == 3) { p->landmark = LOOK_GIANT_TREE; ring = LOOK_TREE; }
	else {
		p->landmark = pick & 1 ? LOOK_MONUMENT : LOOK_STATUE;
		if (pick & 2) ring = LOOK_TREE;
	}
	prop(p, p->landmark, MID, MID);
	static const int round[4][2] = { { -2, -2 }, { 2, -2 }, { -2, 2 }, { 2, 2 } };
	for (int k = 0; k < 4; ++k) prop(p, ring, MID + round[k][0], MID + round[k][1]);
}

/* Cybertrees past the plaza's back rim towards the stage (-y, the top of
 * the screen), two panels or more from the stair's mouth and mirrored
 * about it; by the seed a mirrored pair more past the -x rim either side
 * of Trade Central's walkway, or a row of three a panel apart along the
 * Arena's walk; and an avenue of two along the walkway to the log-in
 * plaza. */
static void trees(SquarePlan *p, uint32_t pick) {
	static const int pair[2] = { 12, 14 };
	for (int k = 0; k < 2; ++k) {
		prop(p, LOOK_TREE, pair[k], PLAZA0 - 1); prop(p, LOOK_TREE, 33 - pair[k], PLAZA0 - 1);
		if (!(pick & 1)) { prop(p, LOOK_TREE, PLAZA0 - 1, pair[k]); prop(p, LOOK_TREE, PLAZA0 - 1, 35 - pair[k]); }
	}
	if (pick & 1)
		for (int y = 25; y <= 29; y += 2) prop(p, LOOK_TREE, 22, y);
	prop(p, LOOK_TREE, 25, 16); prop(p, LOOK_TREE, 25, 19);
}

/* The places, and the walkers' routes: one round the landmark, one from
 * Trade Central across the plaza and down to the Arena and back. */
static void places(SquarePlan *p) {
	place(p, SQ_EXIT, 31, 17);
	place(p, SQ_ARRIVE, 29, 17);
	place(p, SQ_SIGN, 31, 14);
	place(p, SQ_BOARD, 28, 14);
	place(p, SQ_KEEPER, 23, 15);
	place(p, SQ_TRADE, 5, 17);
	place(p, SQ_ARENA, 20, 28);
	place(p, SQ_CENTER, MID, MID);
	place(p, SQ_STAGE, 17, 5);
	place(p, SQ_RING, 14, 27);
	place(p, SQ_GREETER, 28, 20);
	place(p, SQ_REFEREE, 17, 32);
	static const int routes[SQUARE_WALKERS][SQUARE_ROUTE][2] = {
		{ { 13, 13 }, { 21, 13 }, { 21, 21 }, { 13, 21 } },
		{ { 6, 18 }, { 20, 18 }, { 20, 30 }, { 20, 18 } },
	};
	memcpy(p->route, routes, sizeof routes);
}

void square_plan(uint32_t seed, SquarePlan *out) {
	memset(out, 0, sizeof *out);
	out->area = areas[seed % (sizeof areas / sizeof *areas)];
	floors(out);
	places(out);
	uint32_t pick = seed / (sizeof areas / sizeof *areas);
	centrepiece(out, pick);
	trees(out, pick >> 2);
	/* the Board and the welcome sign past the log-in plaza's back rim;
	 * a brazier either side of the stage, past its back rim */
	prop(out, LOOK_BBS, 28, 13);
	prop(out, LOOK_SIGN, 31, 13);
	prop(out, LOOK_BRAZIER, 15, 3); prop(out, LOOK_BRAZIER, 19, 3);
	out->level[3][15] = out->level[3][19] = 1;
}

static uint32_t mix(uint32_t h, const uint8_t *b, int n) {
	for (int i = 0; i < n; ++i) h = (h ^ b[i]) * 16777619u;
	return h;
}

static uint32_t mix_ints(uint32_t h, const int *v, int n) {
	for (int i = 0; i < n; ++i) h = (h ^ (uint8_t)v[i]) * 16777619u;
	return h;
}

uint32_t square_plan_hash(const SquarePlan *p) {
	uint32_t h = 2166136261u;
	h = mix(h, &p->cell[0][0], SQUARE_W * SQUARE_H);
	h = mix(h, &p->pad[0][0], SQUARE_W * SQUARE_H);
	h = mix(h, &p->level[0][0], SQUARE_W * SQUARE_H);
	for (int i = 0; i < p->nstairs; ++i) h = mix_ints(h, (const int[3]){ p->stairs[i].x, p->stairs[i].y, p->stairs[i].dir }, 3);
	for (int i = 0; i < p->nprops; ++i)
		h = mix_ints(h, (const int[4]){ p->props[i].kind, p->props[i].x, p->props[i].y, p->props[i].look }, 4);
	h = mix_ints(h, &p->route[0][0][0], SQUARE_WALKERS * SQUARE_ROUTE * 2);
	return (h ^ (uint8_t)p->area) * 16777619u;
}

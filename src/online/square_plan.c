#include "square_plan.h"

#include <string.h>

/* The areas a Square wears (rom.c's net areas: Central, Sky and Green
 * Area, BN6's busy daylight nets), one picked by the seed; not Seaside,
 * whose layers stand in another map's slot (rom.c's host) */
static const int areas[] = { 0, 2, 3 };

/* The welcome plaza's middle, where the landmark stands: every place
 * below is given from it, a panel a step (grid x down the screen to the
 * right, grid y down to the left) */
#define CX 17
#define CY 20
#define H SQUARE_PLAZA_HALF

static void floor_box(SquarePlan *p, int x0, int y0, int x1, int y1) {
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) p->cell[CY + y][CX + x] = C_PATH;
}

static void raise_box(SquarePlan *p, int x0, int y0, int x1, int y1) {
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) p->level[CY + y][CX + x] = 1;
}

/* A prop on (x, y): in a walled hole where that is floor, else in the void
 * past a rim (docs/LEVEL_DESIGN.md, Props: edge or hole); its index. */
static int prop(SquarePlan *p, int look, int x, int y) {
	if (p->nprops >= SQUARE_PROPS) return -1;
	p->props[p->nprops] = (NetProp){ .kind = PROP_SPRITE, .x = CX + x, .y = CY + y, .look = look };
	if (p->cell[CY + y][CX + x]) p->cell[CY + y][CX + x] = C_SOLID;
	return p->nprops++;
}

/* ... at the height of the raised floor before it */
static void prop_high(SquarePlan *p, int look, int x, int y) {
	prop(p, look, x, y);
	p->level[CY + y][CX + x] = 1;
}

static void place(SquarePlan *p, int k, int x, int y) { p->at[k][0] = CX + x; p->at[k][1] = CY + y; }

/* A raised room (the stage, the ring): its floor, and its stair two panels
 * wide on its +y side at (sx, sy), climbing towards -y as Sky's maps'
 * stairs do, its foot on the floor below (net_height.c's rules: nothing
 * else within a panel of where the room is drawn, a panel up the
 * screen). */
static void dais(SquarePlan *p, int x0, int y0, int x1, int y1, int sx) {
	floor_box(p, x0, y0, x1, y1);
	raise_box(p, x0, y0, x1, y1);
	floor_box(p, sx, y1 + 1, sx + 1, y1 + 2);
	p->stairs[p->nstairs++] = (Stair){ .x = CX + sx, .y = CY + y1 + 1, .dir = STAIR_UP_NY };
}

/* Warp k: its gem in the plaza (`hub`: x, y) and in its part (`there`),
 * each on the middle of a 3 x 3 pad, and the way MegaMan faces landing on
 * either (each one's third). BN6 lands him on the gem and steps him off it the way he faces,
 * 20 units a world axis: one of the screen's diagonals (0 up, 2 right, 4
 * down, 6 left) takes him 28 units out, past its trigger's 26; along a
 * grid axis he stopped inside it and was beamed straight back. */
static void warp(SquarePlan *p, int k, const int hub[3], const int there[3]) {
	const int *at[2] = { hub, there };
	for (int e = 0; e < 2; ++e) {
		p->gem[k][e][0] = CX + at[e][0];
		p->gem[k][e][1] = CY + at[e][1];
		p->gem_face[k][e] = at[e][2];
		for (int y = -1; y <= 1; ++y)
			for (int x = -1; x <= 1; ++x) p->pad[CY + at[e][1] + y][CX + at[e][0] + x] = 1;
	}
}

static void part(SquarePlan *p, int k, int x0, int y0, int x1, int y1) {
	p->part[k][0] = CX + x0; p->part[k][1] = CY + y0;
	p->part[k][2] = CX + x1; p->part[k][3] = CY + y1;
}

/* The floor. The plaza, 9 panels a side, a 3 x 3 pad off the middle of
 * three of its sides for each warp's gem, and the pad home off the fourth
 * (+x, the screen's front right) beside the arrival. Two void panels on,
 * each warp's part, seen across the void from its pad: Trade Central (-x)
 * a floor round its desk; the Arena (-y) its stands' floor, the ring up
 * its stair from it; the stage (+y) up its stair from its audience's
 * floor. MegaMan lands on each part's gem facing what it is for, and on
 * the plaza's facing the landmark. */
static void floors(SquarePlan *p) {
	floor_box(p, -H, -H, H, H);
	floor_box(p, -7, -1, -5, 1);
	floor_box(p, -1, -7, 1, -5);
	floor_box(p, -1, 5, 1, 7);
	floor_box(p, 5, 0, 6, 1);
	for (int y = 0; y <= 1; ++y)
		for (int x = 5; x <= 6; ++x) p->pad[CY + y][CX + x] = 1;
	part(p, SQ_PART_PLAZA, -7, -7, 6, 7);
	/* Trade Central */
	floor_box(p, -15, -2, -10, 2);
	part(p, SQ_PART_TRADE, -15, -2, -10, 2);
	warp(p, SQ_PART_TRADE - 1, (const int[3]){ -6, 0, 2 }, (const int[3]){ -11, 0, 0 });
	/* the Arena: its ring raised over the stands' floor */
	floor_box(p, -3, -12, 3, -10);
	dais(p, -2, -18, 2, -15, 0);
	part(p, SQ_PART_ARENA, -3, -18, 3, -10);
	warp(p, SQ_PART_ARENA - 1, (const int[3]){ 0, -6, 4 }, (const int[3]){ -2, -11, 2 });
	/* the stage, raised over its audience's floor */
	dais(p, -2, 10, 2, 12, 0);
	floor_box(p, -3, 15, 3, 17);
	part(p, SQ_PART_STAGE, -3, 10, 3, 17);
	warp(p, SQ_PART_STAGE - 1, (const int[3]){ 0, 6, 0 }, (const int[3]){ -2, 16, 2 });
}

/* The landmark ringed by four props in holes round it, the area's own
 * first: Green's giant cybertree among cybertrees; elsewhere the statue or
 * the monument by the seed, braziers or cybertrees round it. The
 * landmark's base spreads over the panels round its own (the statue's
 * pedestal, the giant cybertree's roots), so all nine are walled off:
 * MegaMan walks round it, never onto it. */
static void centrepiece(SquarePlan *p, uint32_t pick) {
	int ring = LOOK_BRAZIER;
	if (p->area == 3) { p->landmark = LOOK_GIANT_TREE; ring = LOOK_TREE; }
	else {
		p->landmark = pick & 1 ? LOOK_MONUMENT : LOOK_STATUE;
		if (pick & 2) ring = LOOK_TREE;
	}
	for (int y = -1; y <= 1; ++y)
		for (int x = -1; x <= 1; ++x) p->cell[CY + y][CX + x] = C_SOLID;
	prop(p, p->landmark, 0, 0);
	static const int round[4][2] = { { -2, -2 }, { 2, -2 }, { -2, 2 }, { 2, 2 } };
	for (int k = 0; k < 4; ++k) prop(p, ring, round[k][0], round[k][1]);
}

/* What each part wears, and what invites to it from the plaza: each
 * warp's pad flanked past the plaza's rim by its part's own (the stage's
 * past the front rim, where they hide no floor of the plaza's). The
 * Arena's fire: a brazier either side of its warp, and at either back
 * corner of its ring, past its back rim at its height, seen from the
 * plaza across the void. The stage's grove: cybertrees either side of its
 * warp and behind the stage. Trade Central's board of offers beside its
 * warp, its desk two cybertrees behind it. The welcome sign by the pad
 * home, the Board past the plaza's back rim by the keeper; by the seed,
 * cybertrees past the plaza's top corner, or its left and right ones. */
static void dress(SquarePlan *p, uint32_t pick) {
	p->sign[SQ_PART_PLAZA] = prop(p, LOOK_SIGN, 5, -1);
	prop(p, LOOK_BBS, 3, -5);
	p->sign[SQ_PART_TRADE] = prop(p, LOOK_BBS, -5, -2);
	p->sign[SQ_PART_ARENA] = prop(p, LOOK_BRAZIER, -2, -5);
	prop(p, LOOK_BRAZIER, 2, -5);
	prop_high(p, LOOK_BRAZIER, -2, -19); prop_high(p, LOOK_BRAZIER, 2, -19);
	p->sign[SQ_PART_STAGE] = prop(p, LOOK_TREE, -2, 5);
	prop(p, LOOK_TREE, 2, 5);
	prop_high(p, LOOK_TREE, -2, 9); prop_high(p, LOOK_TREE, 2, 9);
	prop(p, LOOK_TREE, -16, -2); prop(p, LOOK_TREE, -16, 2);
	if (pick & 1) { prop(p, LOOK_TREE, -5, -4); prop(p, LOOK_TREE, -4, -5); }
	else { prop(p, LOOK_TREE, -5, 4); prop(p, LOOK_TREE, 4, -5); }
}

/* The places, and the walkers' routes: the GreenNavi round the landmark,
 * standing at the plaza's corners; the GirlNavi on the stage's floor before
 * it, pacing to and fro. */
static void places(SquarePlan *p) {
	place(p, SQ_ARRIVE, 3, 0);
	place(p, SQ_EXIT, 5, 0);
	place(p, SQ_SIGN, 4, -1);
	place(p, SQ_BOARD, 3, -4);
	place(p, SQ_KEEPER, 4, -2);
	place(p, SQ_GREETER, 4, 2);
	place(p, SQ_CENTER, 0, 0);
	place(p, SQ_TRADE, -14, 0);
	place(p, SQ_ARENA, 1, -11);
	place(p, SQ_RING, 0, -17);
	place(p, SQ_REFEREE, 2, -12);
	place(p, SQ_STAGE, 0, 11);
	static const int routes[SQUARE_WALKERS][SQUARE_ROUTE][3] = {
		{ { -3, -3, 4 }, { 3, -3, 4 }, { 3, 3, 4 }, { -3, 3, 4 } },
		{ { 0, 15, 6 }, { 3, 15, 0 }, { 3, 17, 6 }, { 0, 17, 0 } },
	};
	for (int w = 0; w < SQUARE_WALKERS; ++w)
		for (int k = 0; k < 4; ++k) {
			p->route[w][k][0] = CX + routes[w][k][0];
			p->route[w][k][1] = CY + routes[w][k][1];
			p->route[w][k][2] = routes[w][k][2];
		}
	p->route_len[0] = p->route_len[1] = 4;
	/* (the parade: three panels out from the landmark, inside the plaza's rim) */
	static const int parade[4][2] = { { -3, -3 }, { 3, -3 }, { 3, 3 }, { -3, 3 } };
	for (int k = 0; k < 4; ++k) { p->parade[k][0] = CX + parade[k][0]; p->parade[k][1] = CY + parade[k][1]; }
}

void square_plan(uint32_t seed, SquarePlan *out) {
	memset(out, 0, sizeof *out);
	out->area = areas[seed % (sizeof areas / sizeof *areas)];
	floors(out);
	places(out);
	uint32_t pick = seed / (sizeof areas / sizeof *areas);
	centrepiece(out, pick);
	dress(out, pick >> 2);
}

int square_plan_part(const SquarePlan *p, int x, int y) {
	for (int k = 0; k < SQ_PARTS; ++k)
		if (x >= p->part[k][0] && y >= p->part[k][1] && x <= p->part[k][2] && y <= p->part[k][3]) return k;
	return -1;
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
	h = mix_ints(h, &p->gem[0][0][0], SQUARE_WARPS * 4);
	h = mix_ints(h, &p->gem_face[0][0], SQUARE_WARPS * 2);
	h = mix_ints(h, &p->part[0][0], SQ_PARTS * 4);
	h = mix_ints(h, p->sign, SQ_PARTS);
	h = mix_ints(h, &p->route[0][0][0], SQUARE_WALKERS * SQUARE_ROUTE * 3);
	h = mix_ints(h, p->route_len, SQUARE_WALKERS);
	h = mix_ints(h, &p->parade[0][0], 8);
	return (h ^ (uint8_t)p->area) * 16777619u;
}

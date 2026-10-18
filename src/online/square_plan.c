#include "square_plan.h"

#include <string.h>

/* The areas a Square wears (rom.c's net areas: Central, Seaside, Sky and
 * Green Area, BN6's busy daylight nets), one picked by the seed */
static const int areas[] = { 0, 1, 2, 3 };

static void floor_box(SquarePlan *p, int x0, int y0, int x1, int y1) {
	for (int y = y0; y <= y1; ++y)
		for (int x = x0; x <= x1; ++x) p->cell[y][x] = C_PATH;
}

static void prop(SquarePlan *p, int look, int x, int y) {
	if (p->nprops >= SQUARE_PROPS) return;
	p->props[p->nprops++] = (NetProp){ .kind = PROP_SPRITE, .x = x, .y = y, .look = look };
	p->cell[y][x] = C_SOLID;
}

void square_plan(uint32_t seed, SquarePlan *out) {
	memset(out, 0, sizeof *out);
	out->area = areas[seed % (sizeof areas / sizeof *areas)];
	/* the plaza, 10 panels a side, in the middle */
	floor_box(out, 9, 9, 18, 18);
	/* the wings, each a room off a two-panel walkway from the plaza's
	 * side: +x the arrival and the pad home, -y Trade Central, +y the
	 * Arena, -x the Board */
	floor_box(out, 19, 13, 21, 14); floor_box(out, 22, 11, 26, 16);
	floor_box(out, 13, 7, 14, 8);   floor_box(out, 11, 2, 16, 6);
	floor_box(out, 13, 19, 14, 21); floor_box(out, 11, 22, 16, 26);
	floor_box(out, 7, 13, 8, 14);   floor_box(out, 2, 11, 6, 16);
	/* the pad home at the arrival room's far end, MegaMan set down before
	 * it facing the plaza */
	out->at[SQ_EXIT][0] = 25; out->at[SQ_EXIT][1] = 13;
	out->pad[13][25] = out->pad[13][26] = out->pad[14][25] = out->pad[14][26] = 1;
	out->at[SQ_ARRIVE][0] = 23; out->at[SQ_ARRIVE][1] = 13;
	/* the welcome sign where the arrival's walkway meets the plaza, the
	 * channel keeper beside it */
	out->at[SQ_SIGN][0] = 17; out->at[SQ_SIGN][1] = 11;
	prop(out, LOOK_SIGN, 17, 11);
	out->at[SQ_KEEPER][0] = 17; out->at[SQ_KEEPER][1] = 16;
	out->at[SQ_TRADE][0] = 13; out->at[SQ_TRADE][1] = 4;
	out->at[SQ_ARENA][0] = 13; out->at[SQ_ARENA][1] = 24;
	out->at[SQ_BOARD][0] = 4; out->at[SQ_BOARD][1] = 13;
	prop(out, LOOK_BBS, 2, 13);
}

uint32_t square_plan_hash(const SquarePlan *p) {
	uint32_t h = 2166136261u;
	const uint8_t *parts[] = { &p->cell[0][0], &p->pad[0][0] };
	for (int k = 0; k < 2; ++k)
		for (int i = 0; i < SQUARE_W * SQUARE_H; ++i) h = (h ^ parts[k][i]) * 16777619u;
	for (int i = 0; i < p->nprops; ++i) {
		int v[4] = { p->props[i].kind, p->props[i].x, p->props[i].y, p->props[i].look };
		for (int k = 0; k < 4; ++k) h = (h ^ (uint8_t)v[k]) * 16777619u;
	}
	return (h ^ (uint8_t)p->area) * 16777619u;
}

/* The Net Square's plan (square_plan.c, docs/ONLINE.md, The Square): the
 * same on every client from the server's seed alone, no ROM and no run in
 * it, and built into BN6's own map formats by netmap_build from one of
 * BN6's areas' tiles. A hub and its spokes: the welcome plaza, where every
 * Navi arrives, the area's landmark in its middle, the greeter, the Board,
 * the channel keeper and the pad home by the arrival; on its rim three of
 * BN6's gem pads, each one's sign beside it, warping within the map to a
 * part across the void built for one use: Trade Central (-x), the Arena's
 * ring and its stands (-y) and the stage (+y), each with its gem back by
 * where MegaMan lands. The stage and the ring are raised on stairs where
 * the area's maps have stairs (Sky), flat daises elsewhere. */
#ifndef CW_SQUARE_PLAN_H
#define CW_SQUARE_PLAN_H

#include <stdint.h>

#include "net.h"

/* The Square generator's version: a change to what a seed makes bumps it
 * (protocol.json's square_make, held by the server's tests), and
 * SQUARE_HASH beside it is square_plan_hash of seed 120's plan, which the
 * unit tests hold the generator to. */
#define SQUARE_MAKE 3
#define SQUARE_HASH 0xAB972E7Bu

#define SQUARE_W 26
#define SQUARE_H 39
/* The welcome plaza's panels out from its middle (SQ_CENTER), each way: 9
 * a side */
#define SQUARE_PLAZA_HALF 4
#define SQUARE_PROPS 24
#define SQUARE_STAIRS 2
/* The ambient Navis who walk the Square, and their routes' waypoints at
 * most. */
#define SQUARE_WALKERS 2
#define SQUARE_ROUTE 8

/* Its places (panel coordinates): where MegaMan arrives, the pad home, the
 * welcome sign's and the Board's fronts, the channel keeper, Trade
 * Central's desk, the Arena's spectators' floor, the landmark, the
 * stage's and the ring's middles, the greeter and the referee. */
enum { SQ_ARRIVE, SQ_EXIT, SQ_SIGN, SQ_KEEPER, SQ_TRADE, SQ_ARENA, SQ_BOARD, SQ_CENTER, SQ_STAGE, SQ_RING,
	SQ_GREETER, SQ_REFEREE, SQ_PLACES };

/* Its parts: the welcome plaza, and the three its warps lead to (warp k
 * to part k + 1). */
enum { SQ_PART_PLAZA, SQ_PART_TRADE, SQ_PART_ARENA, SQ_PART_STAGE, SQ_PARTS };
#define SQUARE_WARPS (SQ_PARTS - 1)

typedef struct {
	uint8_t cell[SQUARE_H][SQUARE_W];   /* C_PATH floor, C_SOLID a prop's hole, C_VOID none */
	uint8_t pad[SQUARE_H][SQUARE_W];    /* 1 on the pads' panels: the pad home's, the gems' */
	uint8_t level[SQUARE_H][SQUARE_W];  /* 1 on the stage's and the ring's floor (and props past their rims) */
	Stair stairs[SQUARE_STAIRS];        /* their stairs, climbing towards grid -y (STAIR_UP_NY) */
	int nstairs;
	NetProp props[SQUARE_PROPS];
	int nprops;
	int area;                           /* the net area whose tiles it wears (rom.h's net areas) */
	int landmark;                       /* the centrepiece's LOOK_* */
	int at[SQ_PLACES][2];
	/* each warp's two gems, [0] in the plaza and [1] in its part (each a
	 * panel with its pad's floor all round it), and the way MegaMan faces
	 * landing on either (BN6's 0-7, up first): towards the landmark, or
	 * the part's own (the desk, the ring, the stage) */
	int gem[SQUARE_WARPS][2][2];
	int gem_face[SQUARE_WARPS][2];
	int part[SQ_PARTS][4];              /* each part's floor's bounds: x0, y0, x1, y1 */
	/* each part's prop by its warp in the plaza (its pair's the same
	 * look), which says what awaits there; the plaza's: the welcome sign */
	int sign[SQ_PARTS];
	/* each walker's waypoints (x, y and the seconds he stands there),
	 * walked round in order, each leg along a grid axis */
	int route[SQUARE_WALKERS][SQUARE_ROUTE][3];
	int route_len[SQUARE_WALKERS];
	int parade[4][2];   /* the walkers' parade round the landmark: its corners, walked in order */
} SquarePlan;

/* The plan for server seed `seed`. */
void square_plan(uint32_t seed, SquarePlan *out);
/* Its cells, pads, levels, stairs, props, area, warps and routes as a
 * hash (FNV-1a). */
uint32_t square_plan_hash(const SquarePlan *p);
/* The part grid panel (x, y) is in (SQ_PART_*), -1 none. */
int square_plan_part(const SquarePlan *p, int x, int y);

#endif

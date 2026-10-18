/* The Net Square's plan (square_plan.c, docs/ONLINE.md, The Square): the
 * same on every client from the server's seed alone, no ROM and no run in
 * it, and built into BN6's own map formats by netmap_build from one of
 * BN6's areas' tiles. A plaza with the area's landmark in its middle, ringed
 * and framed by props; off it a stage (-y), Trade Central (-x), the Arena's
 * ring and its spectators' floor (+y), and the log-in plaza with the pad
 * home and the Board (+x). The stage and the ring are raised on stairs
 * where the area's maps have stairs (Sky), flat daises elsewhere. */
#ifndef CW_SQUARE_PLAN_H
#define CW_SQUARE_PLAN_H

#include <stdint.h>

#include "net.h"

/* The Square generator's version: a change to what a seed makes bumps it
 * (protocol.json's square_make, held by the server's tests), and
 * SQUARE_HASH beside it is square_plan_hash of seed 120's plan, which the
 * unit tests hold the generator to. */
#define SQUARE_MAKE 2
#define SQUARE_HASH 0xFF0E6F94u

#define SQUARE_W 36
#define SQUARE_H 36
#define SQUARE_PROPS 24
#define SQUARE_STAIRS 2
/* The ambient Navis who walk the Square, and their routes' waypoints. */
#define SQUARE_WALKERS 2
#define SQUARE_ROUTE 4

/* Its places (panel coordinates): where MegaMan arrives, the pad home, the
 * welcome sign's and the Board's fronts, the channel keeper, Trade
 * Central's desk, the Arena's spectators' floor, the landmark, the
 * stage's and the ring's middles, the greeter and the referee. */
enum { SQ_ARRIVE, SQ_EXIT, SQ_SIGN, SQ_KEEPER, SQ_TRADE, SQ_ARENA, SQ_BOARD, SQ_CENTER, SQ_STAGE, SQ_RING,
	SQ_GREETER, SQ_REFEREE, SQ_PLACES };

typedef struct {
	uint8_t cell[SQUARE_H][SQUARE_W];   /* C_PATH floor, C_SOLID a prop's hole, C_VOID none */
	uint8_t pad[SQUARE_H][SQUARE_W];    /* 1 on the pad home's panels */
	uint8_t level[SQUARE_H][SQUARE_W];  /* 1 on the stage's and the ring's floor (and props past their rims) */
	Stair stairs[SQUARE_STAIRS];        /* their stairs, climbing towards grid -y (STAIR_UP_NY) */
	int nstairs;
	NetProp props[SQUARE_PROPS];
	int nprops;
	int area;                           /* the net area whose tiles it wears (rom.h's net areas) */
	int landmark;                       /* the centrepiece's LOOK_* */
	int at[SQ_PLACES][2];
	int route[SQUARE_WALKERS][SQUARE_ROUTE][2];   /* each walker's waypoints, walked round in order */
} SquarePlan;

/* The plan for server seed `seed`. */
void square_plan(uint32_t seed, SquarePlan *out);
/* Its cells, pads, levels, stairs, props, area and routes as a hash
 * (FNV-1a). */
uint32_t square_plan_hash(const SquarePlan *p);

#endif

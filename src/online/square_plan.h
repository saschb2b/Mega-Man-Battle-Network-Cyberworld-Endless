/* The Net Square's plan (square_plan.c, docs/ONLINE.md): the same on
 * every client from the server's seed alone, no ROM and no run in it, and
 * built into BN6's own map formats by netmap_build from one of BN6's
 * areas' tiles. A plaza with four wings: the arrival and its pad home
 * (+x), Trade Central (-y), the Arena (+y), the Board (-x). */
#ifndef CW_SQUARE_PLAN_H
#define CW_SQUARE_PLAN_H

#include <stdint.h>

#include "net.h"

/* The Square generator's version: a change to what a seed makes bumps it
 * (protocol.json's square_make, held by the server's tests), and
 * SQUARE_HASH beside it is square_plan_hash of seed 120's plan, which the
 * unit tests hold the generator to. */
#define SQUARE_MAKE 1
#define SQUARE_HASH 0u

#define SQUARE_W 28
#define SQUARE_H 28
#define SQUARE_PROPS 4

/* Its places (panel coordinates): where MegaMan arrives, the pad home,
 * the welcome sign, the channel keeper, and the wings' middles. */
enum { SQ_ARRIVE, SQ_EXIT, SQ_SIGN, SQ_KEEPER, SQ_TRADE, SQ_ARENA, SQ_BOARD, SQ_PLACES };

typedef struct {
	uint8_t cell[SQUARE_H][SQUARE_W];   /* C_PATH floor, C_SOLID a prop's hole, C_VOID none */
	uint8_t pad[SQUARE_H][SQUARE_W];    /* 1 on the pad home's panels */
	NetProp props[SQUARE_PROPS];
	int nprops;
	int area;                           /* the net area whose tiles it wears (rom.h's net areas) */
	int at[SQ_PLACES][2];
} SquarePlan;

/* The plan for server seed `seed`. */
void square_plan(uint32_t seed, SquarePlan *out);
/* Its cells, pads, props and area as a hash (FNV-1a). */
uint32_t square_plan_hash(const SquarePlan *p);

#endif

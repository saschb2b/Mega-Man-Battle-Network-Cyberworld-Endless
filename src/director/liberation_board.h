/* A liberation's board (docs/LIBERATION.md, Building it: stage 1): the
 * dark layer's own floor taken at 32 world units a panel, one layer cell a
 * panel, its kinds placed by BN5's rules as tools/liberation.py
 * prototypes them, from the layer's seed alone (no ROM, no game RAM: a
 * checkpoint rebuilds it as it rebuilds the layer). */
#ifndef CW_LIBERATION_BOARD_H
#define CW_LIBERATION_BOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "net.h"

/* A panel's kind (BN5's, src/emu/bn5_liberation.h's BN5_PANEL_*: the
 * same few, numbered here), LIB_VOID off the board */
enum { LIB_VOID, LIB_CLEAN, LIB_DARK, LIB_ITEM, LIB_BONUS, LIB_HOLE, LIB_RING, LIB_BOSS };
/* What an item panel holds (BN5's item kinds): Order Points, or a heart */
enum { LIB_GOT_OP, LIB_GOT_HEART };
/* The units BN5 sets on a board: the Darkloid, and the guardians of its
 * DarkHoles (BN5's BigBrute, TinHawk and Bladia: their rules, not their
 * looks, docs/LIBERATION.md) */
enum { UNIT_DARKLOID, UNIT_BRUTE, UNIT_HAWK, UNIT_BLADIA };
#define LIB_UNITS 8
#define LIB_HOLES 5
/* Order Points: BN5's, 3 at the start and 8 at most (BN5_LIB_OP_ADD) */
#define LIB_OP_START 3
#define LIB_OP_MAX   8
/* The panels a board may fence (liberation_fence.c: BN6's map events
 * 0x1640 + n gate their walls, n below 0x80; 0x7D-0x7F are Rush's) */
#define LIB_FENCES   120

typedef struct {
	uint8_t kind;      /* UNIT_* */
	uint8_t x, y;      /* its panel */
	int8_t hole;       /* the DarkHole it guards, -1 a roamer or the Darkloid */
	bool gone;         /* deleted */
} LibUnit;

typedef struct {
	int act;                        /* 0-based, the layer's */
	uint8_t kind[MAP_H][MAP_W];     /* LIB_* */
	uint8_t item[MAP_H][MAP_W];     /* an item panel's: LIB_GOT_* | amount << 4 (Order Points) */
	int start_x, start_y;           /* MegaMan's first panel (the arrival) */
	int boss_x, boss_y;             /* the Darkloid's panel, the middle of his 3x3 */
	int nholes;
	uint8_t hole_x[LIB_HOLES], hole_y[LIB_HOLES];
	int nunits;
	LibUnit unit[LIB_UNITS];        /* the Darkloid first */
	int panels, dark;               /* the board's panels, and those dark as it begins (items and rings among them) */
	int par;                        /* phases: BN5's yardstick (lib_par) */
	int tries;                      /* the seeds it took (a board rejected tries the next) */
} LibBoard;

/* The board for the layer just generated (net.h's `layer`, a dark one) in
 * act `act` (0-based), for a team of `team` Navis (1: MegaMan alone),
 * from `seed`: its kinds, DarkHoles, units and par; false where no seed of
 * a few gives one that keeps BN5's guarantees (the layer is then a plain
 * one). */
bool lib_board_make(LibBoard *b, uint32_t seed, int act, int team);
/* Whether panel (x, y) is on the board and walked (clean: BN5's units
 * move over clean panels alone). */
bool lib_walkable(const LibBoard *b, int x, int y);
/* Whether panel (x, y) can be liberated as the board stands: dark, an
 * item's, a DarkHole; the Darkloid's ring once every DarkHole is gone. */
bool lib_liberable(const LibBoard *b, int x, int y);
/* The dark panels around (x, y) (its eight), which make its battle harder. */
int lib_dark_around(const LibBoard *b, int x, int y);
/* The DarkHoles still standing. */
int lib_holes_left(const LibBoard *b);
/* The par: phases a lone MegaMan liberates the board in, by BN5's
 * yardstick (docs/LIBERATION.md, The par; stage 1's estimate). */
int lib_par(const LibBoard *b);

#endif

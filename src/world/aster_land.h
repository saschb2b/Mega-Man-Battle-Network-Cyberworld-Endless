/* AsterLand, Central Town's chip store (aster_land.c, docs/HOME.md piece
 * 10): BN6's own map as a place of home. */
#ifndef CW_ASTER_LAND_H
#define CW_ASTER_LAND_H

#include <stdbool.h>

#define ASTER_GROUP 0x01   /* Central Town's (bn6f constants/enums/GameAreas.inc) */
#define ASTER_LAND  0x04

/* AsterLand installed (indoors.c): its counter, Request BBS, checks and
 * Chip Trader as BN6 has them, its people (place_lines.c), BN6's Number
 * Trader off, its door out to where the planned town has it. False where
 * its door cannot be read. */
bool aster_land_install(void);
/* Home's shops in the game's shop data: the SubChip seller's stock for the
 * act ahead, and the Order Service as a run has it (one order a visit,
 * HOME_ORDER_FLAG, cleared as a visit begins). Again after a CONTINUE's
 * state, over its lists in RAM: BN6's screen shows only the entries that
 * match the ROM copy's, and a save whose act ahead wanted other keys came
 * back with an empty seller (session 71). */
void aster_land_shops(void);
/* Whether map (group, number) is AsterLand. */
bool aster_land_map(int group, int number);
/* Its door out: the middle of its cells (world units); false none. */
bool aster_land_door(int *x, int *y);
/* Where Lan comes out of it in the town, before its door (the town's world
 * units); false before it is installed. */
bool aster_land_front(int *x, int *y);
/* Whether world (x, y) is on the Number Trader's check (0xF9: what A
 * reads there, place_lines.c, for the one BN6 has). */
bool aster_land_number_trader(int x, int y);
/* ... and on the request board's (0xF6: its post, the run's request). */
bool aster_land_board(int x, int y);
/* Each frame in AsterLand: its door the only one (indoors_shut). */
void aster_land_frame(void);

#endif

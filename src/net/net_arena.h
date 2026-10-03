/* The guardian's arena, carved onto a boss layer after its layout. */
#ifndef NET_ARENA_H
#define NET_ARENA_H

#include "net.h"

typedef struct {
	int room;             /* the arena's room */
	int ante;             /* the room its bridge leaves from */
	int dir;              /* DIR_*: from the antechamber into the arena */
	int exit_x, exit_y;   /* the exit pad's cell, on the arena's far side */
} ArenaInfo;

/* Attaches an n x n arena by one bridge, as far from room 0 as there is
 * room for; its room index, or -1 (nothing carved) when none fits. */
int arena_attach(int n, ArenaInfo *out);

/* Whether panel (x, y) is on the layer's way into its arena: in the
 * antechamber's box, or on the bridge from it (the arena's own panels
 * not); false on a layer without an arena. */
bool arena_approach(int x, int y);

#endif

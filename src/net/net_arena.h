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

#endif

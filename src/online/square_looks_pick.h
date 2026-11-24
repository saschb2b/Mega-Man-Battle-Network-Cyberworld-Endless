/* Which of the Net Square's peers are drawn in their own looks (issue #854,
 * docs/online/square/looks.md): BN6 gives each frame's sprites their OBJ
 * palette slots by content, and past the free ones leaves a sprite in a
 * stale slot's colours, so the engine shows as many looks as the part of
 * the Square the player stands in has room for, the nearest peers first,
 * and draws the rest in BN6's own colours. Pure: no ROM, no core. */
#ifndef CW_SQUARE_LOOKS_PICK_H
#define CW_SQUARE_LOOKS_PICK_H

#include <stdbool.h>

#include "looks.h"

/* One peer as the pick sees him: his look, how far he stands from MegaMan
 * (any measure that orders) and whether he was drawn in it last time (who
 * keeps his look against a nearer one by `hold`, so a pick does not flip
 * back and forth as two peers walk at one distance). */
typedef struct {
	Look look;
	int distance;
	bool held;
} LookPeer;

/* `shown[i]` set where peer i is drawn in his own look, else in BN6's own
 * colours, so that MegaMan's own look, the peers' looks drawn and BN6's
 * own (where a peer falls back to it) are at most `cap` + 1 palettes:
 * `cap` the looks the part has room for besides MegaMan's own. A peer in
 * MegaMan's own look, or in one drawn already, costs nothing. */
void square_looks_pick(const LookPeer *peers, int n, const Look *own, int cap, int hold, bool *shown);

/* Whether two looks draw alike (a channel BN6's own in both, or the same
 * palette and colour). */
bool square_looks_same(const Look *a, const Look *b);

#endif

/* The shared dive's divers in their own looks (issue #878;
 * docs/online/shared-dive.md, Each in his own look): the divers' puppets
 * on a layer (dive_people.h) draw a copy of BN6's field MegaMan with a
 * palette each (look_sprite.h), given the colours of the look the diver's
 * game sent (protocol v30's LOOK, in every SNAPSHOT on a dive as in the
 * Square). How many looks a layer has room for is measured from BN6's own
 * OBJ palette slots as the layer draws them (its Mr.Progs, Mystery Data,
 * traces): the crew first, then the nearest; the rest in BN6's own colours,
 * as the Square does it (docs/online/square/looks.md). */
#ifndef CW_DIVE_LOOKS_H
#define CW_DIVE_LOOKS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "looks.h"

/* One puppet of the layer's this frame: the diver it shows (0 none), his
 * look, how far he stands from MegaMan (world units) and whether he is in
 * MegaMan's crew. */
typedef struct {
	uint32_t id;
	uint32_t script;
	Look look;
	int distance;
	bool crew;
} DiveLookPuppet;

/* As the layer is built (layer_objs_more, inside its claims): the copy for
 * `puppets` puppets made in the layer's room; the list-6 number they take. */
int dive_looks_install(int puppets);
/* Each frame on the layer the copy was made on (inside the caller's
 * claims): the puppets' palettes and colours. */
void dive_looks_frame(const DiveLookPuppet *puppets, int n);
/* Forgotten: the dive's people off, or another layer begun. */
void dive_looks_reset(void);

/* For the state in words: whether puppet `k` is drawn in his own look (1),
 * in BN6's own colours (0) or not at all (-1); the looks the layer has
 * room for besides MegaMan's own (-1 none measured yet); the puppets BN6
 * drew in a stale slot's colours this frame. */
int dive_looks_shown(int k);
/* Dev (divelooks=N): N looks at most besides MegaMan's own on each layer, whatever BN6 leaves. */
void dive_looks_dev_room(int most);
int dive_looks_room(void);
int dive_looks_stale(void);

#endif

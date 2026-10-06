/* A layer's signature room (docs/LEVEL_DESIGN.md, Identity; issue #98):
 * the one room a layer is remembered by, as each of BN6's net maps has one
 * landmark. Each area draws its layers' signatures from a small pool after
 * its own maps; a layout sets the signature where its shape has a heart (a
 * chain's middle, a hub's centre, the field itself), on the way across. */
#ifndef NET_SIGNATURE_H
#define NET_SIGNATURE_H

#include <stdint.h>

enum {
	SIG_NONE,
	SIG_CRATER,   /* a field round a pit: Central Area 3's */
	SIG_PLAZA,    /* a giant plaza: Central Area 1's field, Sky's pods grown, Green Area 1's holed grass */
	SIG_RING,     /* a ring road round a sunken middle with a pad in it: Sky Area 3's */
	SIG_FIELD,    /* one great field with ragged edges: Seaside's */
	SIG_LAGOON,   /* ... with a pool of the void in it: Seaside Area 2's field round its water */
	SIG_GROVE,    /* a field whose back rim holds the giant cybertree: Green Area 2's */
	SIG_SLAB,     /* a great slab, a line of holes through it: the Graveyard's, its monument at the back */
	SIG_COURT,    /* a plateau that holds the statue between its braziers: the Undernet's */
	SIG_CROSS,    /* a great plus: the Underground's round its raised block */
	SIG_COUNT
};

extern const char *const sig_names[SIG_COUNT];

/* The signatures of an act's three layers (index 0-2): the area's pool in an
 * order drawn from `act_seed` by their weights, so an act repeats none while
 * its pool has others. */
int sig_in_act(int biome, uint32_t act_seed, int index);
/* A side layer's (an Undernet detour, the Secret Area, a trip back): one of
 * the pool by its weight, from the layer's seed. */
int sig_for_seed(int biome, uint32_t seed);
/* How many signatures area `biome` draws from. */
int sig_pool_size(int biome);

/* The signature's box at a layer's `size` (0-2), panels along grid x and y. */
void sig_box(int sig, int biome, int size, int *w, int *h);
/* Carves signature `sig` into the w x h box from (x, y), its shape fitted
 * to the box, where the layout has found it room; its room (layer.sig_room
 * too), -1 for none. */
int sig_carve(int sig, int biome, int x, int y, int w, int h);

#endif

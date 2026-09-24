/* The layouts a layer is built in, after the shape language of each of
 * BN6's net areas (docs/LEVEL_DESIGN.md). */
#ifndef NET_LAYOUTS_H
#define NET_LAYOUTS_H

#include <stdint.h>

enum {
	LAYOUT_ROUTE,     /* platforms in a winding chain, pads on spurs */
	LAYOUT_FIELD,     /* one big field framed by comb boardwalks */
	LAYOUT_LADDER,    /* parallel planks joined by rungs, grass blocks */
	LAYOUT_HUB,       /* a centre platform, mirrored spokes to pods */
	LAYOUT_SLABS,     /* big slabs with holes, long bridges between */
	LAYOUT_WEB,       /* scattered plateaus, crossing bridges, stubs */
	LAYOUT_CROSSES,   /* plus-shaped platforms on a lattice */
	LAYOUT_CATWALKS,  /* a maze of 1-wide turns between plazas */
	LAYOUT_COUNT
};

/* One of the area's layouts, from the generator's random numbers, or
 * `layout_forced` when that is set (tools and tests; -1 otherwise). */
int layout_pick(int biome);
extern int layout_forced;
/* Layer `index` of an act (three layers in one area): the area's layouts in
 * an order drawn from `act_seed` by their weights, so an act does not
 * repeat one while the area has others. */
int layout_in_act(int biome, uint32_t act_seed, int index);
/* Builds `layout` into the (cleared) layer; room 0 is where MegaMan
 * arrives. `size` 0-2 grows with depth. */
void layout_build(int layout, int biome, int size);

extern const char *const layout_names[LAYOUT_COUNT];

#endif

/* Walls and triggers in the game's coordinate-data format. */
#ifndef CW_COORDS_H
#define CW_COORDS_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"   /* CoordCell */

/* A warp pad centred on world (x, y) that takes warp `index` (1-15) of the
 * map's warp list. */
typedef struct { int x, y, index; } CoordPad;

/* Cells the layer adds to each section beyond walls and pads, in world
 * units: raised floor heights, and the stairs' ramps, walls and layer
 * priorities. */
typedef struct {
	const CoordCell *cells[4];
	int n[4];
} CoordExtra;

/* Writes the layer's walls (from netmap_floor_cell, each level at its
 * height), pads and `extra`, and points the coordinate-data pointer at ROM
 * offset `slot` at them. */
bool coords_write(uint32_t slot, const CoordPad *pads, int npads, const CoordExtra *extra);

#endif

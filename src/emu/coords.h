/* Walls and triggers in the game's coordinate-data format. */
#ifndef CW_COORDS_H
#define CW_COORDS_H

#include <stdbool.h>
#include <stdint.h>

/* A warp pad centred on world (x, y) that takes warp `index` (1-15) of the
 * map's warp list. */
typedef struct { int x, y, index; } CoordPad;

/* Writes the layer's walls (from netmap_floor_cell) and pads, and points
 * the coordinate-data pointer at ROM offset `slot` at them. */
bool coords_write(uint32_t slot, const CoordPad *pads, int npads);

#endif

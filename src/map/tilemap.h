/* A whole tile map picked (tilemap.c): each tile by tiles_pick, then again
 * where it meets its neighbours badly. */
#ifndef CW_TILEMAP_H
#define CW_TILEMAP_H

#include <stdint.h>

#include "tiles.h"

/* Both layers of the g->tw x g->th map into `map` (layer 0, then layer 1).
 * `left` (a byte per tile) gets where tiles still meet badly: bit 0 with the
 * one to the right, bit 1 with the one below. */
void tilemap_pick(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g,
	TileFloor floor, const void *ctx, uint16_t *map, uint8_t *left);

#endif

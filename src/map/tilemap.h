/* A whole tile map picked (tilemap.c): each tile by tiles_pick, then again
 * where it meets its neighbours badly. */
#ifndef CW_TILEMAP_H
#define CW_TILEMAP_H

#include <stdint.h>

#include "tiles.h"

/* Both layers of the g->tw x g->th map into `map` (layer 0, then layer 1).
 * `left` (a byte per tile) gets where tiles still meet badly: bit 0 with the
 * one to the right, bit 1 with the one below; and in bits 2-3 how far its
 * pair was seen from its own neighbourhood (tiles_pick_off), in bits 4-6 why
 * (tiles_pick_why). With g->apart, each piece
 * (TILE_APART panels joined side by side) and the rest of the floor are
 * picked apart, each as if the others were not there, and where two draw a
 * tile, the one whose top covers the other's side faces (tiles_in_front)
 * goes on the second layer, which the game shows in front. */
void tilemap_pick(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g,
	TileFloor floor, const void *ctx, uint16_t *map, uint8_t *left);

#endif

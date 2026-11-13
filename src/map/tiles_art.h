/* A map's floor's own art, pixel by pixel, and the floor panel each pixel
 * of it belongs to (tiles_art.c), for learning a map's panels
 * (tiles_learn.c, panels.c). */
#ifndef CW_TILES_ART_H
#define CW_TILES_ART_H

#include <stdint.h>

#include "tiles_parts.h"

/* The colours of a map's pads (TILES_PADS_ALONE): what their tops show
 * well inside them, and what the map draws off the floor where a pad's side
 * face must be drawn. A byte per colour; NULL without memory. (The map's
 * pixels are kept: tiles_keep_begin.) */
uint8_t *tl_pad_colours(const Src *src, const TileGrid *g);
/* The map's floor's own art (Src.art): a byte per map pixel, 0 where it is
 * none, else the floor panel it belongs to. With `seeds` (tl_pad_colours),
 * the art in those colours alone. NULL without memory (free it). */
uint8_t *tl_own_art(const Src *src, const TileGrid *g, const uint8_t *seeds);
/* The pixels of tile (tx, ty) that are the art of a floor in or round its
 * panel. */
uint64_t tl_art_of(const Src *src, const TileGrid *g, int tx, int ty);
#endif

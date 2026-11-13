/* What the layers take from a map (lent.c, docs/devtools/tile-test.md, What
 * the layers take from a map): the tile pictures an area's generated layers
 * draw, held against each of its original maps. */
#ifndef CW_LENT_H
#define CW_LENT_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

/* Where a tile no layer drew lies in its map, by the panel its middle is
 * in as the area's tiles are learned: on the floor a layer is made of; on
 * floor of another kind, which lends no tiles (a road, a pad of another
 * look, another surface); off the floor beside the one or the other, a
 * panel away at most (rims, side faces, walls, what stands at an edge); or
 * further off (scenery). */
enum { LENT_OWN, LENT_OTHER, LENT_BY_OWN, LENT_BY_OTHER, LENT_AWAY, LENT_PLACES };

/* A map's tiles that draw something (each layer's counted), those whose
 * picture a layer drew, and the same over its different pictures; of the
 * different pictures no layer drew, how many lie where (`left`). */
typedef struct { int drawn, used, kinds, kinds_used, left[LENT_PLACES]; } Lent;

/* Forgets the layers seen. */
void lent_begin(void);
/* A generated layer's tiles (two layers, as area_src_render takes them),
 * drawn with map (group, number)'s of game `rom`: their pictures are seen,
 * and their mirror images. False without memory. */
bool lent_layer(int rom, int group, int number, const uint16_t *tiles, int tw, int th);
/* Map a's picture, each tile no layer seen drew tinted red, and its counts;
 * NULL without memory (free it). `panels` is the map as its area's tiles
 * are learned (tiles_src_panels; NULL: nothing told by place). */
uint32_t *lent_map(const AreaSrc *a, const uint8_t *panels, Lent *n);

#endif

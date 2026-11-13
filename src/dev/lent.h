/* What the layers take from a map (lent.c, docs/devtools/tile-test.md, What
 * the layers take from a map): the tile pictures an area's generated layers
 * draw, held against each of its original maps. */
#ifndef CW_LENT_H
#define CW_LENT_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

/* A map's tiles that draw something (each layer's counted), those whose
 * picture a layer drew, and the same over its different pictures. */
typedef struct { int drawn, used, kinds, kinds_used; } Lent;

/* Forgets the layers seen. */
void lent_begin(void);
/* A generated layer's tiles (two layers, as area_src_render takes them),
 * drawn with map (group, number)'s of game `rom`: their pictures are seen,
 * and their mirror images. False without memory. */
bool lent_layer(int rom, int group, int number, const uint16_t *tiles, int tw, int th);
/* Map a's picture, each tile no layer seen drew tinted red, and its counts;
 * NULL without memory (free it). */
uint32_t *lent_map(const AreaSrc *a, Lent *n);

#endif

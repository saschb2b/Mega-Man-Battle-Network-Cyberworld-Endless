/* Scenery cut from an area's original maps and set beside a generated
 * layer's floor (decor.c). */
#ifndef CW_DECOR_H
#define CW_DECOR_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

#define DECOR_MAX 48

/* One piece: w x h tiles of the two layers' entries (0: nothing there). */
typedef struct {
	uint8_t w, h;
	uint16_t *e0, *e1;
} DecorPiece;

typedef struct {
	DecorPiece piece[DECOR_MAX];
	int n;
} DecorBook;

/* Adds the scenery of map `a` to `out`, each look once. */
void decor_learn(const AreaSrc *a, bool bg_in_map, DecorBook *out);
void decor_free(DecorBook *b);
/* Sets some pieces into the layers of a tw x th tile map (layer 1 after
 * layer 0's cells), on empty tiles near its floor, chosen by `seed`; how
 * many it set. */
int decor_place(const DecorBook *b, uint16_t *map, int tw, int th, uint32_t seed);

#endif

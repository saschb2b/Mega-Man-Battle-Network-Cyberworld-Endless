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

/* Adds the scenery of map `a` to `out`, each look once; `reach`: the
 * pixels under a floor's lower edges that are its own, its faces and what
 * hangs there (0: 44). */
void decor_learn(const AreaSrc *a, bool bg_in_map, int reach, DecorBook *out);
/* Adds what map `a`, whose background is its own tiles (NetAreaDef.bg_in_map:
 * decor_learn reads its first layer alone), draws over its floor on its
 * second layer: whole groups of its tiles nine tenths over ground floor
 * and on no raised floor (Mr. Weather Comp's low clouds; its walkable
 * ones lie on floor at height 64), each look once. */
void decor_learn_floating(const AreaSrc *a, DecorBook *out);
void decor_free(DecorBook *b);
/* Sets some pieces into the layers of a tw x th tile map (layer 1 after
 * layer 0's cells), on empty tiles near its floor, chosen by `seed`; how
 * many it set. */
int decor_place(const DecorBook *b, uint16_t *map, int tw, int th, uint32_t seed);

#endif

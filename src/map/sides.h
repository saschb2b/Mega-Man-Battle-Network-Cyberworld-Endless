/* Side pieces: what an original map hangs on its floor's sides on its second
 * tile layer (CopyBot Comp's claws round its walkways' bends), cut whole
 * with the neighbourhood of the panel it grips and set on a generated
 * floor's panel of the same neighbourhood (sides.c;
 * docs/world/level-design/side-pieces.md). */
#ifndef CW_SIDES_H
#define CW_SIDES_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"
#include "stairs.h"   /* StairTile */

#define SIDES_MAX 24

/* One piece: its second layer's entries, screen pixels from the corner of
 * the panel it grips (the lowest world X and Y), and that panel's
 * neighbourhood in its map: bit k set where the panel at (k % 3 - 1,
 * k / 3 - 1) from it, in panels along world X and Y, is ground floor. */
typedef struct {
	uint16_t floor;
	int ntiles;
	StairTile *tiles;
} SidePiece;

typedef struct {
	SidePiece piece[SIDES_MAX];
	int n;
} SideBook;

/* Adds map `a`'s side pieces to `out`, each look once; a piece's tiles are
 * its book's owner's to free. */
void sides_learn(const AreaSrc *a, SideBook *out);

#endif

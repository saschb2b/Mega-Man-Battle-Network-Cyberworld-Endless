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
#define SIDE_UNDER 16   /* panels a piece draws over, kept */

/* One piece: its second layer's entries, screen pixels from the corner of
 * the panel it grips (the lowest world X and Y), and that panel's
 * neighbourhood in its map: bit k set where the panel at (k % 3 - 1,
 * k / 3 - 1) from it, in panels along world X and Y, is ground floor. */
typedef struct {
	uint16_t floor;
	int ntiles;
	StairTile *tiles;
	CoordCell *prio;   /* its map's layer priorities (section 2) on that panel and the eight round it, world units
	                    * from the panel's corner: where MegaMan is drawn behind its art */
	int nprio;
	/* the panels it draws over, (along world X, along world Y) from its own
	 * and whether ground floor (1) or none (0): a layer sets it where they
	 * are the same, a flame beside a floor never over another floor */
	int8_t under[SIDE_UNDER][3];
	int nunder;
} SidePiece;

typedef struct {
	SidePiece piece[SIDES_MAX];
	int n;
} SideBook;

/* Adds map `a`'s side pieces to `out`, each look once; a piece's tiles and
 * priorities are its book's owner's to free. */
void sides_learn(const AreaSrc *a, SideBook *out);

#endif

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
	bool hang;         /* what stands under a raised floor's lower edge (hangs_learn): its tiles the first layer's
	                    * (StairTile.e0), its panel one of a pad's on a layer */
	bool behind;       /* ... or what stands behind a floor's upper edge, its panel any free one of a layer's floor; the
	                    * first layer's tiles where `hang`, else the second's (sides_learn_behind) */
	bool over;         /* a side piece kept with the first layer's tiles under it (StairTile.e0, 0 where it held
	                    * none), set only where a layer's are the same: the Nest's crystals over its rims' tabs and
	                    * spikes (sides_learn_under) */
	uint8_t hole[2];   /* what stands in a walled hole in the floor (hole_learn): the hole's panels along world X and Y
	                    * from its own, the lowest; 0 none */
} SidePiece;

typedef struct {
	SidePiece piece[SIDES_MAX];
	int n;
} SideBook;

/* Adds map `a`'s side pieces to `out`, each look once; a piece's tiles and
 * priorities are its book's owner's to free. */
void sides_learn(const AreaSrc *a, SideBook *out);

/* ... and what it sets there under a floor's lower edge, touching none of
 * its top, kept with the panel straight above it: each with the first
 * layer's tiles under it (SidePiece.over), the Nest's crystals in front of
 * its rims and behind them. */
void sides_learn_under(const AreaSrc *a, SideBook *out);

/* Adds what view `r` of a map (area_src_raise_in: its raised floor brought
 * down to the ground) sets under that floor's lower edges, past its side
 * face: Aquarium Comp's pillars under its glass pads, the bubbles under them too. */
void hangs_learn(const AreaSrc *r, SideBook *out);

/* Adds what map `a` sets on its first layer behind its floor's upper edges,
 * off the floor: the flames behind the catwalks of BN4's Undernet, where
 * the ones in front lie on the second layer (sides_learn). */
void behind_learn(const AreaSrc *a, SideBook *out);

/* ... and what it sets on its second layer behind its floor's upper edges,
 * touching none of its top (net_area.sides' SIDES_BEHIND_FRONT): the
 * Undernet's fire bowls by its exits and in a row along a walkway, which is
 * cut into its copies, a bowl each; each with the first layer's tiles under
 * it (SidePiece.over) and the panels it stands behind (SidePiece.under). */
void sides_learn_behind(const AreaSrc *a, SideBook *out);

/* The first piece map `a` sets in a walled hole in its floor, whole on its
 * second layer, into `out` where it has none (BN5's End Area's pagoda): a
 * landmark, its tiles, the panels it draws over and the map's layer
 * priorities round it; its tiles and priorities its owner's to free. */
void hole_learn(const AreaSrc *a, SidePiece *out);

#endif

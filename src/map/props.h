/* Props cut whole out of an area's original maps: a counter's tiles, its
 * walls and where its navi stands behind it (docs/LEVEL_DESIGN.md, Props).
 * A layer places one where the generator set a PROP_COUNTER. */
#ifndef CW_PROPS_H
#define CW_PROPS_H

#include <stdbool.h>

#include "area_src.h"
#include "net.h"      /* FACES_X, FACES_Y */
#include "stairs.h"   /* StairTile */

typedef struct {
	bool ok;
	int len;              /* panels along its run */
	/* from its anchor, the corner of its panels with the lowest world X
	 * and Y at z 0: the second layer's entries (screen pixels from the
	 * anchor's point), its walls (section 0), where its navi stands and
	 * the navi's talk centre from there (towards the counter's front) */
	StairTile *tiles;
	int ntiles;
	CoordCell *walls;
	int nwalls;
	int navi_x, navi_y, talk_x, talk_y;
	/* the source's tile lattice (world units mod 32 of tile 0's centre),
	 * against which the layer's is matched */
	int fx, fy;
} PropStamp;

/* Cuts the counter whose wall ring holds world (x, y) out of `a`, facing
 * `faces` (FACES_X: its front towards world +Y; FACES_Y: towards -X). */
bool props_learn_counter(const AreaSrc *a, int x, int y, int faces, PropStamp *out);
/* The walkable ornament whose second-layer tiles hold tile `seed` in `a`
 * (a pad's centrepiece: the red gem 0x379, the link ring 0x372, the cube
 * on its base 0x375), anchored on the corner of the panel it lies on. */
bool props_learn_ornament(const AreaSrc *a, int seed, PropStamp *out);
/* ... the same for art that stands in the void past the floor (Green's
 * potted bushes between its planks: 0x292, in flower 0x361). */
bool props_learn_void_art(const AreaSrc *a, int seed, PropStamp *out);
/* An emblem drawn in the floor itself (the first layer) of `a`, found by
 * its colour `bgr` (BGR555), which the floor shows nowhere else: the tiles
 * that draw it, joined 8 ways, where they lie on one panel with floor all
 * round (the Graveyard's cyan crosses). Its tiles are layer-one entries
 * (StairTile.e0), anchored as an ornament's. */
bool props_learn_floor_emblem(const AreaSrc *a, uint16_t bgr, PropStamp *out);
void props_free(PropStamp *p);
/* Gives the mirror image `m` of `a` (area_src_mirror, tiles only) a's walls,
 * mirrored with it: world (X, Y) to (-Y, -X). */
void props_mirror_walls(const AreaSrc *a, AreaSrc *m);

#endif

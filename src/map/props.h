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
	CoordCell *prio;      /* its layer priorities (section 2): behind it and beside its ends,
	                       * MegaMan is drawn behind its art */
	int nprio;
	int navi_x, navi_y, talk_x, talk_y;
	/* the source's tile lattice (world units mod 32 of tile 0's centre),
	 * against which the layer's is matched */
	int fx, fy;
	int low;              /* a pad's: the floor beside its lower sides, which its faces show cut off (0: whole) */
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
/* An emblem drawn in the floor of `a`, found by its colour `bgr` (BGR555),
 * which the floor shows nowhere else: the tiles that draw it, joined 8
 * ways, where they lie on one panel with floor all round (the Graveyard's
 * cyan crosses, the Undernet's magenta ones). Its tiles are the first
 * layer's entries (StairTile.e0) where the first layer draws it, else the
 * second's (e1), anchored as an ornament's. */
bool props_learn_floor_emblem(const AreaSrc *a, uint16_t bgr, PropStamp *out);
/* A whole pad of `a`: a 3 x 3 island of floor (bridges at most beside it)
 * whose middle panel has a hue in `hues` (buckets as `styles`): the first
 * layer's tiles over its panels, the faces under them and its rim,
 * anchored on its corner of lowest world X and Y (Central's and Seaside's
 * framed pads, their recess holding a centrepiece). The islands are laid
 * over each other, the one with the fewest bridges beside its lower sides
 * first, each tile taken where one of them draws its island alone (e1 1: a
 * bridge beside an island comes along in its tiles there); called again on
 * another map of the area, it fills in `out`. */
bool props_learn_pad(const AreaSrc *a, uint16_t hues, PropStamp *out);
/* An arrow panel of `a` (issue #43), carrying MegaMan towards BN6's way d
 * (0 +X, 1 +Y, 2 -X, 3 -Y): one in a lane a panel wide, closed by the
 * lane's end cells (section 3, 0x4C + d); the tiles of both layers whose
 * middle lies on its diamond, anchored on its corner of lowest X and Y.
 * Called again on another map of the area, it keeps one it has. */
bool props_learn_arrow(const AreaSrc *a, int d, PropStamp *out);
void props_free(PropStamp *p);
/* Gives the mirror image `m` of `a` (area_src_mirror, tiles only) a's walls
 * and layer priorities, mirrored with it: world (X, Y) to (-Y, -X). */
void props_mirror_walls(const AreaSrc *a, AreaSrc *m);

#endif

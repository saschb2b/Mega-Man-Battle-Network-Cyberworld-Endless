/* Stairs cut whole out of an area's original maps: the ramp's floor heights,
 * the walls and layer priorities around it, and the tiles that draw it. A
 * layer places one where a corridor climbs to a raised room
 * (docs/world/level-design/heights.md). */
#ifndef CW_STAIRS_H
#define CW_STAIRS_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"
#include "net.h"   /* STAIR_UP_NX, STAIR_UP_NY, STAIR_WIDE, STAIR_NARROW */

/* Which way a stair climbs, in layer grid terms (grid x is world +Y, grid y
 * world -X): towards grid -x is the game's ramp type 0x14, towards grid -y
 * type 0x13. The originals have no other: every ramp of theirs climbs up
 * the screen. */
#define STAIR_DIRS 2

typedef struct { int16_t px, py; uint16_t e0, e1; uint8_t opaque, over; } StairTile;   /* (opaque: of its 64 pixels, as the map draws them;
                                                                               * over: a side piece's tile its map draws over floor, its foot) */

typedef struct {
	bool ok;
	int rise;              /* world z the stair climbs */
	int len, wide;         /* panels along the climb and across it */
	/* its box of panels: cells are relative to the corner with the lowest
	 * panel indices (world X, Y), z relative to the stair's foot */
	CoordCell *ramp, *walls, *prio;
	int nramp, nwalls, nprio;
	StairTile *tiles;      /* screen pixels from that corner's point at z 0 */
	int ntiles;
	int group, number;     /* the map it was cut from */
} StairTemplate;

/* An area's stairs, by the way each climbs and its width. */
typedef StairTemplate StairKit[STAIR_DIRS][STAIR_WIDTHS];

/* Cuts out of `a` the stairs `out` lacks: the first straight ramp of each
 * way and width, one or two panels wide and two to four long, whose rise is
 * a whole panel's height or two and is the rise of those learned already (a
 * layer has one), and whose tiles line up with the grid of the area's own
 * map (ex, ey its panels' edges; a generated map is an even number of tiles
 * a side). With `report`, each ramp found and why one is left (debug). */
void stairs_learn(const AreaSrc *a, int ex, int ey, StairKit out, bool report);
void stairs_free(StairKit t);

#endif

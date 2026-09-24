/* Stairs cut whole out of an area's original map: the ramp's floor heights,
 * the walls and layer priorities around it, and the tiles that draw it. A
 * layer places one where a corridor climbs to a raised room. */
#ifndef CW_STAIRS_H
#define CW_STAIRS_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"
#include "net.h"   /* STAIR_UP_NX, STAIR_UP_NY */

/* Which way a stair climbs, in layer grid terms (grid x is world +Y, grid y
 * world -X): towards grid -x is the game's ramp type 0x14, towards grid -y
 * type 0x13. */
#define STAIR_DIRS 2

typedef struct { int16_t px, py; uint16_t e0, e1; } StairTile;

typedef struct {
	bool ok;
	int rise;              /* world z the stair climbs */
	/* 2 x 2 panels: cells are relative to the corner with the lowest panel
	 * indices (world X, Y), z relative to the stair's foot */
	CoordCell *ramp, *walls, *prio;
	int nramp, nwalls, nprio;
	StairTile *tiles;      /* screen pixels from that corner's point at z 0 */
	int ntiles;
} StairTemplate;

/* Cuts the first 2 x 2-panel stair of each direction out of `a`. */
void stairs_learn(const AreaSrc *a, StairTemplate out[STAIR_DIRS]);
void stairs_free(StairTemplate t[STAIR_DIRS]);

#endif

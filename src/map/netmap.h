/* Generated net layers in the game's own map formats (docs/EMULATION.md). */
#ifndef CW_NETMAP_H
#define CW_NETMAP_H

#include <stdbool.h>
#include <stdint.h>

#include "coords.h"
#include "net.h"

/* A layer: panels on a gw x gh grid (1 = floor). Grid x runs down-right on
 * screen, y down-left, as in the layer generator. */
typedef struct {
	int gw, gh;
	const uint8_t *cell;
	const uint8_t *level;   /* 1: raised by `rise` (NULL: all flat) */
	int rise;
	const Stair *stairs;
	int nstairs;
	int ax, ay, aw, ah;     /* the guardian's arena, drawn in the walkway floor (aw 0: none) */
} NetLayout;

/* Builds the layer from biome `area`'s original map (RomLayout.net_area),
 * writes its tile map and walls into the free ROM space and points that map
 * at them. The map is (group, number) of the area, entered with emu_warp. */
bool netmap_build(int area, const NetLayout *lay);
/* The stairs area `area` can draw (bit per STAIR_UP_*) and their rise. */
unsigned netmap_stair_dirs(int area, int *rise);

/* World position of the centre of grid panel (x, y) in the last layer. */
void netmap_world(int x, int y, int *wx, int *wy);
/* The grid panel under world (wx, wy), or false outside the grid. */
bool netmap_panel(int wx, int wy, int *x, int *y);
/* Whether wall cell (cx, cy) (8x8 world units) lies on the last layer's
 * floor at `level` (0 ground, 1 raised); stairs belong to both. */
bool netmap_floor_cell(int cx, int cy, int level);
/* Whether wall cell (cx, cy) lies on a stair. */
bool netmap_stair_cell(int cx, int cy);
/* The raised floor's world z, 0 when the layer is flat. */
int netmap_rise(void);
/* The layer's warp pads (the walls are written again with them). */
bool netmap_set_pads(const CoordPad *pads, int n);

#endif

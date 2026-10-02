/* Generated net layers in the game's own map formats (docs/EMULATION.md). */
#ifndef CW_NETMAP_H
#define CW_NETMAP_H

#include <stdbool.h>
#include <stdint.h>

#include "coords.h"
#include "legal.h"
#include "net.h"

/* A layer: panels on a gw x gh grid (1 = floor). Grid x runs down-right on
 * screen, y down-left, as in the layer generator. */
typedef struct {
	int gw, gh;
	uint8_t *cell;          /* made drawable where `locked` allows (legal.c) */
	const uint8_t *locked;  /* 1 where the floor must stay (NULL: all of it) */
	const uint8_t *level;   /* 1: raised by `rise` (NULL: all flat) */
	int rise;
	const Stair *stairs;
	int nstairs;
	int ax, ay, aw, ah;     /* the guardian's arena, drawn in the walkway floor (aw 0: none) */
	uint32_t seed;          /* where the scenery goes */
	const uint8_t *pad;     /* 1 on the cells of pads (NULL: none) */
	const NetProp *props;   /* the props set on its floor (net.h) */
	int nprops;
} NetLayout;

/* Builds the layer from biome `area`'s original map (RomLayout.net_area),
 * writes its tile map and walls into the free ROM space and points that map
 * at them. The map is (group, number) of the area, entered with emu_warp. */
bool netmap_build(int area, const NetLayout *lay);
/* netmap_build for the current layer (net.h), its scenery placed by `seed`. */
bool netmap_build_layer(int area, uint32_t seed);
/* The pieces of scenery the last layer got. */
extern int netmap_scenery;
/* The floor cells the last layer's floor changed to be drawable, and the
 * panels still not drawn exactly. */
extern LegalStats netmap_legal;
/* The last tile map written: tw x th entries of layer 0, then layer 1. */
const uint16_t *netmap_last_tiles(int *tw, int *th);
/* ... and where its tiles meet as no original map shows (per tile: bit 0
 * with the one to the right, bit 1 with the one below), in bits 2-3 how far
 * its pair was seen from its own neighbourhood (tiles_pick_off), in bits 4-6
 * why (tiles_pick_why). */
const uint8_t *netmap_last_seams(void);
/* ... and the tiles set whole over what the classes picked (a bit each). */
enum { NETMAP_PASTED_PAD = 1, NETMAP_PASTED_EMBLEM = 2, NETMAP_PASTED_STAIR = 4 };
const uint8_t *netmap_last_pasted(void);
/* (dev) The last layer's floor as text, a row per grid y: '.' void, 'a'
 * platform floor, 'b' walkway floor, 's' walkway floor across a platform
 * (TILES_CROSSING), 'p' a pad; upper case (a void panel '*') where a tile
 * centred on the panel was seen with other floors where it shows them. */
void netmap_last_cells(char out[MAP_H][MAP_W + 1]);
/* What area `area`'s layers can draw: its stairs and counters (net.h). */
void netmap_kit(int area, LayerKit *kit);
/* Where the navi of the last layer's prop `i` stands (world units), and its
 * talk centre from there; false for a prop not drawn. */
bool netmap_prop_navi(int i, int *wx, int *wy, int *tx, int *ty);

/* World position of the centre of grid panel (x, y) in the last layer. */
void netmap_world(int x, int y, int *wx, int *wy);
/* The grid panel under world (wx, wy), or false outside the grid. */
bool netmap_panel(int wx, int wy, int *x, int *y);
/* The same point on the grid, not rounded to a panel (a panel's centre at
 * whole numbers). */
void netmap_grid(int wx, int wy, double *x, double *y);
/* Whether wall cell (cx, cy) (8x8 world units) lies on the last layer's
 * floor at `level` (0 ground, 1 raised); stairs belong to both. */
bool netmap_floor_cell(int cx, int cy, int level);
/* Whether wall cell (cx, cy) lies on a stair. */
bool netmap_stair_cell(int cx, int cy);
/* Rush's gaps (issue #14): for the walls a walkway, whose panels count as
 * floor unless `shut`; the gap a wall cell lies in, 1 the layer's first, 0
 * none. */
void netmap_gaps_shut(bool shut);
int netmap_gap_at(int cx, int cy);
bool netmap_gap_any(void);
/* A Link Navi obstacle's place in the world (issue #42): BN6's direction
 * from the room into its walkway (0 +X, 1 +Y, 2 -X, 3 -Y), the edge along
 * it where the walkway leaves the room, and the walkway's lower edge
 * across. */
void netmap_block_edges(const NetBlock *b, int *dir, int *edge, int *side);
/* The raised floor's world z, 0 when the layer is flat. */
int netmap_rise(void);
/* The layer's warp pads (the walls are written again with them). */
bool netmap_set_pads(const CoordPad *pads, int n);

#endif

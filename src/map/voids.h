/* The floor's holes (voids.c): the safety net under a generated layer's
 * tiles. Its picks, the panels laid whole, the pieces laid over each other
 * and what is pasted over them each hold to their own rules, and where
 * those meet as no rule foresaw, a tile on the floor can draw nothing over
 * part of its top: the background shows through a walkable panel (BN6's
 * RoboDog Comp, a walkway's lower half between a platform and a room; BN4's
 * ACDC Area, navy squares cut into its cyan roads). Once the map is whole,
 * each tile's top is held to the pixels its entries draw, and a hole is
 * filled from behind with a tile of the area's own maps from that place in
 * a panel, or the tile is laid again with one of their pairs
 * (docs/world/level-design/floor-holes.md). */
#ifndef CW_VOIDS_H
#define CW_VOIDS_H

#include <stdbool.h>
#include <stdint.h>

#include "tiles.h"

/* The tiles a map entry can name (its low 10 bits). */
#define VOID_TILES 1024

/* A hole: more pixels of a tile's top than this left undrawn, where the
 * area's own maps draw them at a tile of the same neighbourhood (where one
 * of theirs leaves them undrawn too, they are its art). The floor's
 * outline is not its top: a pixel is top where the floor lies 4 pixels to
 * either side of it and 2 above and below. */
#define VOID_SLACK 2

/* A map's floor as the net reads it: its grid and floor as the pick had
 * them, the books it picked from (g->roads a road's), and its tiles'
 * drawn pixels by tile number (bit y * 8 + x, unflipped; NULL: none known,
 * nothing is mended). */
typedef struct {
	const TileBook *books;
	int nbooks;
	const TileGrid *g;
	TileFloor floor;
	const void *ctx;
	const uint64_t *opaque;
} VoidFloor;

/* Mends the holes in tile map `map` (g->tw x g->th entries of layer 0,
 * then layer 1): each tile whose top shows a hole takes a tile of the
 * books from its place in a panel behind its own, or one of their pairs
 * in its stead, the nearest neighbourhood that covers the hole and draws
 * nothing where the floor must not be. How many tiles were mended; *left
 * those that could not be. */
int voids_mend(const VoidFloor *f, uint16_t *map, int *left);

/* The pixels each tile of game `rom`'s map (group, number) draws,
 * unflipped (area_src_opaque), kept from one call to the next while the
 * map is the same (a layer's tile set is its area's): NULL where its tile
 * set cannot be read. */
const uint64_t *voids_opaque(int rom, int group, int number);

/* (dev) Off where false: the atlas draws a layer without the net, the
 * holes as its tiles left them. And, kept where voids_measure is set (the
 * atlas), what the last map's pass found, per tile: VOID_FOUND, and
 * VOID_MENDED where it was mended; NULL before any, or unmeasured. */
extern bool voids_on, voids_measure;
enum { VOID_FOUND = 1, VOID_MENDED = 2 };
const uint8_t *voids_last(int *tw, int *th);

#endif

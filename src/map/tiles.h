/* Floor tiles learned from an original map, by class: where a tile's centre
 * falls inside a panel and which of the 3x3 panels around it are floor
 * (see tiles.c). */
#ifndef CW_TILES_H
#define CW_TILES_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

#define TILE_PLAIN 4   /* plain floor looks kept per phase */

/* One pair of layer entries seen for a class (phase << 9 | neighbours), how
 * often, which of its 64 pixels are drawn (bit y * 8 + x) and their
 * colours (BGR555). */
typedef struct {
	uint32_t key;
	uint16_t e0, e1;
	uint32_t count;
	uint64_t mask;
	uint16_t px[64];
} TileCand;

/* Sorted by key and then most common first. */
typedef struct {
	TileCand *cand;
	int n;
	int dv, face;                       /* how the floor is drawn, see TileGrid */
	int plain[64][TILE_PLAIN], nplain[64];   /* per phase: the floor's usual looks inside a room */
} TileBook;

/* A tile map's size, where its panel edges fall (world units mod 32), how
 * many pixels below them the floor is drawn and how tall the side faces
 * under its bottom edges are (from TileBook). */
typedef struct { int tw, th, ex, ey, dv, face; } TileGrid;

/* Whether world panel (A, B) is floor. */
typedef bool (*TileFloor)(int A, int B, const void *ctx);

void tiles_learn(const AreaSrc *a, uint16_t styles, bool bg_in_map, TileBook *out);
void tiles_free(TileBook *b);

/* The class of tile (tx, ty): its phase and the panel it lies in. */
void tile_class(const TileGrid *g, int tx, int ty, int *phase, int *A, int *B);

/* The best pair for tile (tx, ty) of a map whose floor is `floor`: of those
 * whose pixels cover the floor there, do not reach beyond it and look like
 * plain floor well inside it, the one seen with the nearest neighbours;
 * NULL off the floor. */
const TileCand *tiles_pick(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty,
	TileFloor floor, const void *ctx);

#endif

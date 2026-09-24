/* Floor tiles learned from an original map, by class: where a tile's centre
 * falls inside a panel and which of the 3x3 panels around it are floor
 * (see tiles.c). */
#ifndef CW_TILES_H
#define CW_TILES_H

#include <stdbool.h>
#include <stdint.h>

#include "area_src.h"

#define TILE_PLAIN 4   /* plain floor looks kept per phase */

/* Floor materials: the area's platform floor and its walkway floor. */
enum { TILE_VOID, TILE_A, TILE_B };
/* With a floor: this panel is a pad (a small platform on a spur), drawn in
 * the pads' own look where the original has one. */
#define TILE_PAD 4
#define TILE_MATERIAL(m) ((m) & 3)

/* One pair of layer entries seen for a class (phase, then the platform and
 * walkway panels around), how
 * often, which of its 64 pixels are drawn (bit y * 8 + x) and their
 * colours (BGR555). */
typedef struct {
	uint32_t key;
	uint16_t e0, e1;
	uint32_t count;
	uint8_t pad;          /* seen on a pad of the original */
	uint64_t mask;
	uint16_t px[64];
} TileCand;

/* Sorted by key and then most common first. */
typedef struct {
	TileCand *cand;
	int n;
	int dv, face, hang;                 /* how the floor is drawn, see TileGrid */
	int joins;                          /* pairs where the two floors meet */
	int plain[2][64][TILE_PLAIN], nplain[2][64];   /* per material and phase: the floor's usual looks */
} TileBook;

/* A tile map's size, where its panel edges fall (world units mod 32), how
 * many pixels below them the floor is drawn, how tall the side faces under
 * its bottom edges are and how far below those edges things may hang, such
 * as the pedestals under a homepage's panels (from TileBook). */
typedef struct {
	int tw, th, ex, ey, dv, face, hang;
	bool rimmed;   /* floor edges are rims, not plain floor (areas told by shape) */
} TileGrid;

/* World panel (A, B)'s floor: TILE_VOID, TILE_A or TILE_B, maybe | TILE_PAD. */
typedef int (*TileFloor)(int A, int B, const void *ctx);

/* Learns the tiles of the map's panels whose middle has a hue in `styles`
 * (platforms) or `walk_styles` (walkways; 0 for none), or with
 * TILES_BY_SHAPE, platform floor what lies in 2 x 2 blocks of floor with
 * floor all around, and walkway the rest (walkways and platforms' rims),
 * as a generated layer tells them. */
#define TILES_BY_SHAPE 0x8000
void tiles_learn(const AreaSrc *a, uint16_t styles, uint16_t walk_styles, bool bg_in_map, TileBook *out);
void tiles_free(TileBook *b);

/* The class of tile (tx, ty): its phase and the panel it lies in. */
void tile_class(const TileGrid *g, int tx, int ty, int *phase, int *A, int *B);

/* The layer entries for tile (tx, ty) of a map whose floor is `floor`: of
 * the pairs whose pixels cover the floor there, do not reach beyond it and
 * look like plain floor well inside it, the one seen with the nearest
 * neighbours. Where the two floors meet and the original never joins them,
 * the walkway's tile over the platform's. False off the floor. */
bool tiles_pick(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty,
	TileFloor floor, const void *ctx, uint16_t *e0, uint16_t *e1);

#endif

/* What a tile is checked by (tiles.c), for learning and picking tiles (tiles_learn.c, tiles_pick.c). */
#ifndef CW_TILES_PARTS_H
#define CW_TILES_PARTS_H

#include <stdbool.h>
#include <stdint.h>

#include "tiles.h"

#define SLACK 2    /* stray pixels a tile may have */

int tl_floordiv(int a, int b);
int tl_style_at(const AreaSrc *a, int cx, int cy);

#define SPAN 128   /* panels cached per axis, centred on the world origin */

#define OTHER 3    /* a source panel of neither material */

typedef struct {
	const AreaSrc *a;
	uint32_t styles;
	uint16_t walk_styles, skip_styles, joint_hues;
	bool bg_in_map;
	int8_t *state;   /* SPAN x SPAN panel states, -1 until measured */
	int8_t *drawn;   /* ... whether each is floor of this view at all */
	bool inner_walls;   /* rings of walls inside the floor ring no holes (TILES_INNER_WALLS) */
	const uint8_t *pads;   /* its pads (find_pads), once found: SKIP_OFF_PADS spares them */
} Src;

int tl_src_panel(const Src *s, int A, int B);
int tl_src_floor(int A, int B, const void *ctx);

#define PAD_LOOK 8      /* how far a tile of the wrong look (pad or not) is */

uint8_t *tl_find_pads(const Src *s);
unsigned tl_nearest(int phase, bool corner);
void tl_occupancy(TileFloor floor, const void *ctx, int A, int B, unsigned *a, unsigned *b);
/* (bits set: libgcc's call, on a machine without the instruction, was a
 * twentieth of making a layer) */
static inline int tl_popcount64(uint64_t x) {
	x -= x >> 1 & 0x5555555555555555ull;
	x = (x & 0x3333333333333333ull) + (x >> 2 & 0x3333333333333333ull);
	x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0Full;
	return (int)(x * 0x0101010101010101ull >> 56);
}

/* A class: phase, then the platform and walkway panels around. */
#define KEY(phase, a, b) ((uint32_t)(phase) << 18 | (uint32_t)(a) << 9 | (uint32_t)(b))

#define KEY_PHASE(k) ((int)((k) >> 18))

#define KEY_A(k) (((k) >> 9) & 0x1FFu)

#define KEY_B(k) ((k) & 0x1FFu)

/* unplain's answers through a map's pick as well: a pair and the tile's
 * deep pixels decide them, and a map's tiles share them (the pick's passes
 * ask again besides) */
#define MEMO_SIZE (1 << 16)

#define MEMO_PROBES 8

typedef struct { const void *pair; uint64_t deep; int m, value; } PlainMemo;
extern PlainMemo *tl_plain_memo;

#define FACE_TALL 32        /* taller faces are a raised floor's, down to the ground */

#define OUT 3               /* pixels beyond the floor's edge that may be drawn */
#define WIN (8 + 2 * OUT)   /* the columns a tile's pixels look at */

typedef struct {
	int up[8][WIN];   /* per pixel and the OUT columns beside the tile, how far up the nearest floor is (NO_FLOOR: none as far as looked) */
	uint64_t inner, below, deep;
} Look;

int tl_reach_of(int face, int hang);
void tl_expect_look(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, int reach, Look *l);
void tl_expect_for(const Look *l, int face, int hang, int map_face, uint64_t *must, uint64_t *never);
void tl_expect(const TileGrid *g, int tx, int ty, TileFloor floor, const void *ctx, int cm, uint64_t *must, uint64_t *never, uint64_t *deep);
int tl_misses(uint64_t mask, uint64_t must, uint64_t never);
bool tl_flat_tile(const AreaSrc *a, int tx, int ty);
uint64_t tl_drawn(const AreaSrc *a, bool first, int tx, int ty, uint16_t px[64]);
void tl_calibrate(const AreaSrc *a, const Src *src, TileGrid *g);
int tl_cmp_cand(const void *a, const void *b);
int tl_cmp_u32(const void *a, const void *b);
int tl_cmp_count(const void *a, const void *b);
void tl_find_plain(TileBook *b);
bool tl_pad_look(const Src *s, const uint8_t *pads, int A, int B);
bool tl_skipped(const Src *s, int A, int B);

int tl_first_class(const TileBook *b, uint32_t key);
int tl_first_of(const TileBook *b, uint32_t key);

#endif

/* The safety net under a layer's tiles (src/map/voids.c), without a ROM: a
 * floor whose tiles leave a hole in its top is mended from the books, a
 * hole nothing covers is counted, and a whole floor is left alone. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "area_src.h"
#include "tiles_parts.h"
#include "voids.h"

/* (the tile set's pixels come from the ROM in the game: here, the test's) */
bool area_src_opaque(int rom, int group, int number, uint64_t out[1024]) { return false; }

#define TW 16
#define TH 16
#define UPPER 0x00000000FFFFFFFFull   /* a tile's rows 0-3 */
#define LOWER 0xFFFFFFFF00000000ull   /* ... 4-7 */

static int all_floor(int A, int B, const void *ctx) { return TILE_A; }

static int phase_of(const TileGrid *g, int tx, int ty) {
	int phase, A, B;
	tile_class(g, tx, ty, &phase, &A, &B);
	return phase;
}

int main(void) {
	static uint64_t opaque[VOID_TILES];
	opaque[1] = ~0ull;    /* a whole tile */
	opaque[2] = UPPER;    /* its upper half */
	opaque[3] = LOWER;    /* its lower half */
	TileGrid g = { .tw = TW, .th = TH };
	voids_measure = true;   /* (the marks the atlas reads, below) */
	static uint16_t map[2 * TW * TH];
	size_t cells = TW * TH;
	for (size_t i = 0; i < cells; ++i) map[i] = 1;

	/* a whole floor: no hole */
	VoidFloor floor = { NULL, 0, &g, all_floor, NULL, opaque };
	int left = -1, mended = voids_mend(&floor, map, &left);
	assert(mended == 0 && left == 0);

	/* a tile drawing its upper half alone, and one its upper half flipped
	 * upside down (its lower half), each of the same phase; one tile of
	 * another phase drawing nothing */
	int hx = 8, hy = 8, fx = -1, fy = -1, ox = -1, oy = -1, ph = phase_of(&g, hx, hy);
	for (int ty = 2; ty < TH - 2 && (fx < 0 || ox < 0); ++ty)
		for (int tx = 2; tx < TW - 2; ++tx) {
			if (tx == hx && ty == hy) continue;
			if (fx < 0 && phase_of(&g, tx, ty) == ph) { fx = tx; fy = ty; }
			else if (ox < 0 && phase_of(&g, tx, ty) != ph) { ox = tx; oy = ty; }
		}
	assert(fx >= 0 && ox >= 0);
	size_t h = (size_t)hy * TW + hx, f = (size_t)fy * TW + fx, o = (size_t)oy * TW + ox;
	map[h] = 2;
	map[f] = 2 | 0x800;
	map[o] = 0;
	/* the books (by key): at that phase, a whole tile of another
	 * neighbourhood, and the tile's own (a platform's middle) drawing the
	 * lower half */
	TileCand cand[2] = {
		{ .key = KEY(ph, 0x010, 0), .e0 = 1, .count = 9, .mask = ~0ull },
		{ .key = KEY(ph, 0x1FF, 0), .e0 = 3, .count = 4, .mask = LOWER },
	};
	TileBook book = { .cand = cand, .n = 2 };
	floor.books = &book;
	floor.nbooks = 1;
	mended = voids_mend(&floor, map, &left);
	assert(mended == 1 && left == 1);
	/* behind the tile's own, the tile of its neighbourhood: drawn whole */
	assert(map[h] == 3 && map[cells + h] == 2);
	/* the flipped one: its hole is its upper half, which the maps' own tile
	 * of its neighbourhood leaves undrawn too: their art, no hole */
	assert(map[f] == (2 | 0x800) && map[cells + f] == 0);
	assert(map[o] == 0 && map[cells + o] == 0);
	int tw, th;
	const uint8_t *marks = voids_last(&tw, &th);
	assert(marks && tw == TW && th == TH);
	assert(marks[h] == (VOID_FOUND | VOID_MENDED) && marks[o] == VOID_FOUND && !marks[f] && !marks[0]);

	/* off (the atlas's before): found, left as they are */
	voids_on = false;
	map[h] = 2;
	map[cells + h] = 0;
	mended = voids_mend(&floor, map, &left);
	assert(mended == 0 && left == 2 && map[h] == 2);
	puts("voids: holes in a floor's top found, mended from the books behind, the maps' own art left, the rest counted");
	return 0;
}

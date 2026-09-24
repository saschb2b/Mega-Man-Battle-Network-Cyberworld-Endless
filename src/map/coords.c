/* A layer's coordinate data (bn6f decompressCoordEventData): a 16-byte
 * header of four section offsets, then LZ77 data. Sections 0 (walls) and 3
 * (triggers) hold a count, (key, offset) entries sorted by key and 4-byte
 * shapes (lowest z, value, height, type); key = (y / 8 + 127) * 254 +
 * x / 8 + 127. Walls: types 1 NE, 2 SW, 3 SE, 4 NW edges and 5 E, 6 S, 7 N,
 * 8 W outer corners. Triggers: a warp pad is 3x3 cells, types 9-C at the
 * corners and 0x11 elsewhere, the value its warp index (see docs/ROM_DATA.md). */
#include "coords.h"

#include <stdio.h>
#include <stdlib.h>

#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "lz.h"
#include "netmap.h"

#define COORD_AT (EMU_FREE + 0x60000) /* generated coordinate data */

/* One cell's shape: key, then lowest z, value, height, type. */
typedef struct { uint16_t key; uint8_t shape[4]; } Cell;

static int cmp_cell(const void *a, const void *b) {
	const Cell *x = a, *y = b;
	return x->key != y->key ? (x->key < y->key ? -1 : 1) : x->shape[3] - y->shape[3];
}

static uint16_t key_of(int cx, int cy) { return (uint16_t)((cy + 127) * 254 + (cx + 127)); }

static Cell cell(int cx, int cy, int value, int type) {
	Cell c = { key_of(cx, cy), { 0, (uint8_t)value, 8, (uint8_t)type } };   /* z 0, height 8 */
	return c;
}

/* The wall cells around the floor. */
static int walls(Cell *w, int cap) {
	int n = 0;
	for (int cy = -126; cy < 126; ++cy)
		for (int cx = -126; cx < 126; ++cx) {
			if (netmap_floor_cell(cx, cy)) continue;
			int types[4], nt = 0;
			if (netmap_floor_cell(cx - 1, cy)) types[nt++] = 1;
			if (netmap_floor_cell(cx + 1, cy)) types[nt++] = 2;
			if (netmap_floor_cell(cx, cy - 1)) types[nt++] = 3;
			if (netmap_floor_cell(cx, cy + 1)) types[nt++] = 4;
			if (!nt) {
				if (netmap_floor_cell(cx - 1, cy + 1)) types[nt++] = 7;
				else if (netmap_floor_cell(cx - 1, cy - 1)) types[nt++] = 5;
				else if (netmap_floor_cell(cx + 1, cy - 1)) types[nt++] = 6;
				else if (netmap_floor_cell(cx + 1, cy + 1)) types[nt++] = 8;
			}
			for (int k = 0; k < nt && n < cap; ++k) w[n++] = cell(cx, cy, 0, types[k]);
		}
	return n;
}

/* A warp pad's 3x3 trigger cells. */
static int pad(Cell *t, const CoordPad *p) {
	static const int corner[3][3] = { { 0x09, 0x11, 0x0A }, { 0x11, 0x11, 0x11 }, { 0x0B, 0x11, 0x0C } };
	int cx = p->x >> 3, cy = p->y >> 3, n = 0;   /* arithmetic shift: floor */
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) t[n++] = cell(cx + dx, cy + dy, p->index, corner[dy + 1][dx + 1]);
	return n;
}

/* count, (key, offset) entries sorted by key, then the shapes; offsets
 * count from just past the count */
static size_t section(uint8_t *d, Cell *c, int n) {
	qsort(c, (size_t)n, sizeof *c, cmp_cell);
	put32(d, (uint32_t)n);
	for (int i = 0; i < n; ++i) {
		put16(d + 4 + i * 4, c[i].key);
		put16(d + 6 + i * 4, (uint32_t)(n * 4 + i * 4));
		for (int k = 0; k < 4; ++k) d[4 + n * 4 + i * 4 + k] = c[i].shape[k];
	}
	return 4 + (size_t)n * 8;
}

static void debug_print(const Cell *w, int n) {
	for (int cy = -20; cy < 36; ++cy) {
		char line[57];
		for (int cx = -20; cx < 36; ++cx) {
			char c = netmap_floor_cell(cx, cy) ? '.' : ' ';
			for (int i = 0; i < n; ++i) if (w[i].key == key_of(cx, cy)) c = (char)('0' + w[i].shape[3]);
			line[cx + 20] = c;
		}
		line[56] = 0;
		fprintf(stderr, "%4d %s\n", cy * 8, line);
	}
}

bool coords_write(uint32_t slot, const CoordPad *pads, int npads) {
	enum { WALLS_MAX = 8192, TRIGGERS_MAX = 9 * 16 };
	Cell *w = malloc(WALLS_MAX * sizeof *w), t[TRIGGERS_MAX];
	int nw = walls(w, WALLS_MAX), nt = 0;
	for (int i = 0; i < npads && nt + 9 <= TRIGGERS_MAX; ++i) nt += pad(t + nt, &pads[i]);
	if (emu_debug_on()) debug_print(w, nw);
	/* sections 1 (height changes) and 2 (layer priorities) stay empty */
	size_t cap = 16 + (size_t)(nw + nt) * 8;
	uint8_t *d = calloc(cap, 1);
	size_t s0 = section(d, w, nw);
	size_t s3 = s0 + 8;
	size_t total = s3 + section(d + s3, t, nt);
	uint8_t *out = malloc(16 + total + total / 8 + 16);
	put32(out, 0);
	put32(out + 4, (uint32_t)s0);
	put32(out + 8, (uint32_t)(s0 + 4));
	put32(out + 12, (uint32_t)s3);
	size_t lz = lz_literal(d, total, out + 16);
	emu_write(COORD_AT, out, 16 + lz);
	emu_write32(0x08000000u + slot, COORD_AT);
	free(out);
	free(d);
	free(w);
	return true;
}

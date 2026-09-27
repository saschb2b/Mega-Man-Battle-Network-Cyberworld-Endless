/* A layer's coordinate data (bn6f decompressCoordEventData): a 16-byte
 * header of four section offsets, then LZ77 data. Sections 0 (walls) and 3
 * (triggers) hold a count, (key, offset) entries sorted by key and 4-byte
 * shapes (lowest z, value, height, type); key = (y / 8 + 127) * 254 +
 * x / 8 + 127. Walls: types 1 NE, 2 SW, 3 SE, 4 NW edges and 5 E, 6 S, 7 N,
 * 8 W outer corners. Triggers: an original warp pad is 3x3 cells, types 9-C
 * at the corners and 0x11 elsewhere, the value its warp index (see
 * docs/ROM_DATA.md); a layer's covers the pad's square whole. */
#include "coords.h"

#include <stdio.h>
#include <stdlib.h>

#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "lz.h"
#include "netmap.h"

#define COORD_AT (EMU_FREE + 0x60000) /* generated coordinate data */
#define TOWN_COORD_AT (EMU_FREE + 0x130000) /* the town's */

/* One cell's shape: key, then lowest z, value, height, type. */
typedef struct { uint16_t key; uint8_t shape[4]; } Cell;

static int cmp_cell(const void *a, const void *b) {
	const Cell *x = a, *y = b;
	return x->key != y->key ? (x->key < y->key ? -1 : 1) : x->shape[3] - y->shape[3];
}

static uint16_t key_of(int cx, int cy) { return (uint16_t)((cy + 127) * 254 + (cx + 127)); }

static Cell cell(int cx, int cy, int z, int value, int height, int type) {
	Cell c = { key_of(cx, cy), { (uint8_t)z, (uint8_t)value, (uint8_t)height, (uint8_t)type } };
	return c;
}

static Cell from(const CoordCell *c) {
	Cell o = { key_of(c->x >> 3, c->y >> 3), { (uint8_t)c->z, c->value, c->height, c->type } };
	return o;
}

/* The floor the walls ring: the layer's by level (netmap), or the town's. */
static bool (*town_floor)(int cx, int cy);
static bool floor_at(int cx, int cy, int level) { return town_floor ? town_floor(cx, cy) : netmap_floor_cell(cx, cy, level); }

/* The wall cells around the floor of `level`, 8 high from z. Walls beside
 * a stair rise with it from the ground (the original's are 10 higher than
 * the climb), and the ground has none where a stair's top meets the raised
 * floor. */
static int walls(Cell *w, int cap, int level, int z, int rise) {
	/* edges (types 1-4), else the first outer corner (7, 5, 6, 8) */
	static const int dir[8][3] = {
		{ -1, 0, 1 }, { 1, 0, 2 }, { 0, -1, 3 }, { 0, 1, 4 },
		{ -1, 1, 7 }, { -1, -1, 5 }, { 1, -1, 6 }, { 1, 1, 8 },
	};
	int n = 0;
	for (int cy = -126; cy < 126; ++cy)
		for (int cx = -126; cx < 126; ++cx) {
			if (floor_at(cx, cy, level)) continue;
			if (!level && rise && floor_at(cx, cy, 1)) continue;
			bool edge = false;
			for (int k = 0; k < 8 && n < cap; ++k) {
				if (k >= 4 && edge) break;
				int nx = cx + dir[k][0], ny = cy + dir[k][1];
				if (!floor_at(nx, ny, level)) continue;
				if (k < 4) edge = true;
				bool by_stair = rise && netmap_stair_cell(nx, ny);
				if (by_stair && level) { if (k >= 4) break; continue; }   /* the ground's wall covers it */
				w[n++] = by_stair ? cell(cx, cy, 0, 0, rise + 10, dir[k][2]) : cell(cx, cy, z, 0, 8, dir[k][2]);
				if (k >= 4) break;
			}
		}
	return n;
}

/* A warp pad's trigger cells: every cell over the square 12 units round
 * its centre, whole (3x3 for a centre in a cell's middle, 4x4 on a cell's
 * corner). The originals cut their corner cells (types 9-C); in the
 * isometric view those corners are the ring's top, bottom and sides on
 * screen, so MegaMan stood on the pad's rim, the sprite over its middle,
 * and stayed. */
#define PAD_CELLS 16
static int pad(Cell *t, const CoordPad *p) {
	int x0 = (p->x - 12) >> 3, x1 = (p->x + 11) >> 3, y0 = (p->y - 12) >> 3, y1 = (p->y + 11) >> 3, n = 0;   /* (arithmetic shift: floor) */
	for (int cy = y0; cy <= y1 && n < PAD_CELLS; ++cy)
		for (int cx = x0; cx <= x1 && n < PAD_CELLS; ++cx) t[n++] = cell(cx, cy, 0, p->index, 8, 0x11);
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
			char c = netmap_floor_cell(cx, cy, 0) ? '.' : netmap_floor_cell(cx, cy, 1) ? ':' : ' ';
			for (int i = 0; i < n; ++i) if (w[i].key == key_of(cx, cy)) c = (char)('0' + w[i].shape[3]);
			line[cx + 20] = c;
		}
		line[56] = 0;
		fprintf(stderr, "%4d %s\n", cy * 8, line);
	}
}

static bool write_at(uint32_t at, uint32_t slot, const CoordPad *pads, int npads, const CoordExtra *extra);

bool coords_write(uint32_t slot, const CoordPad *pads, int npads, const CoordExtra *extra) {
	town_floor = NULL;
	return write_at(COORD_AT, slot, pads, npads, extra);
}

bool coords_write_town(uint32_t slot, bool (*floor)(int cx, int cy), const CoordExtra *extra) {
	town_floor = floor;
	bool ok = write_at(TOWN_COORD_AT, slot, NULL, 0, extra);
	town_floor = NULL;
	return ok;
}

static bool write_at(uint32_t at_bus, uint32_t slot, const CoordPad *pads, int npads, const CoordExtra *extra) {
	enum { WALLS_MAX = 16384, TRIGGERS_MAX = PAD_CELLS * 16 };
	static Cell sec[4][WALLS_MAX];
	int n[4] = { 0 };
	int rise = town_floor ? 0 : netmap_rise();
	n[0] = walls(sec[0], WALLS_MAX, 0, 0, rise);
	if (rise) n[0] += walls(sec[0] + n[0], WALLS_MAX - n[0], 1, rise, rise);
	for (int i = 0; i < npads && n[3] + PAD_CELLS <= TRIGGERS_MAX; ++i) n[3] += pad(sec[3] + n[3], &pads[i]);
	/* raised floor heights, stairs' ramps, walls and layer priorities */
	for (int s = 0; extra && s < 4; ++s)
		for (int i = 0; i < extra->n[s] && n[s] < WALLS_MAX; ++i) sec[s][n[s]++] = from(&extra->cells[s][i]);
	if (emu_debug_on() && !town_floor) debug_print(sec[0], n[0]);
	size_t cap = 32 + (size_t)(n[0] + n[1] + n[2] + n[3]) * 8;
	uint8_t *d = calloc(cap, 1);
	size_t at[4], len = 0;
	for (int s = 0; s < 4; ++s) {
		at[s] = len;
		len += section(d + len, sec[s], n[s]);
	}
	uint8_t *out = malloc(16 + len + len / 8 + 16);
	for (int s = 0; s < 4; ++s) put32(out + s * 4, (uint32_t)at[s]);
	size_t lz = lz_literal(d, len, out + 16);
	emu_write(at_bus, out, 16 + lz);
	emu_write32(0x08000000u + slot, at_bus);
	free(out);
	free(d);
	return true;
}

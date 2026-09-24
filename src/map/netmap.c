/* Generated layers as game maps.
 *
 * Tiles: each tile takes a pair of layer entries the area's original map
 * uses at the same place in a panel with the same or the nearest floor
 * around it, checked pixel by pixel (tiles.c).
 *
 * Walls: the game's collision is a list of wall cells (8x8 world units) keyed
 * by row * 254 + column (both offset by 127), each pointing to a 4-byte shape
 * (lowest z, flag, height, type). Panel edges run through cell centres; a
 * cell is floor when its centre is inside floor panels, and the cells around
 * the floor get the wall types the original maps use: 1 NE, 2 SW, 3 SE, 4 NW
 * edges and 5 E, 6 S, 7 N, 8 W outer corners. */
#include "netmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "bytes.h"
#include "coords.h"
#include "debug.h"
#include "emu.h"
#include "lz.h"
#include "stairs.h"
#include "tiles.h"
#include "rom.h"

#define TILEMAP_AT  (EMU_FREE + 0x10000) /* generated tile map (LZ77) */
#define MAX_TILE_BYTES 0x14000           /* the game's tile map buffer */

typedef struct {
	bool tried, ok;
	int ex, ey, tw, th;
	TileBook book[2];         /* the map, then its mirror image */
	uint32_t desc, coord_slot;
	StairTemplate stairs[STAIR_DIRS];
} Learned;

static Learned learned[8];

/* the current layer's placement */
static struct { int gx0, gy0, ex, ey; } place;

/* ---- helpers ---- */

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

static bool learn(int area, Learned *L) {
	const __typeof__(R.layout->net_area[0]) *na = &R.layout->net_area[area];
	AreaSrc a, m;
	if (!area_src_load(na->group, na->number, &a)) return false;
	L->ex = a.ex; L->ey = a.ey; L->tw = a.tw; L->th = a.th;
	L->desc = a.desc; L->coord_slot = a.coord_slot;
	tiles_learn(&a, na->styles, na->bg_in_map, &L->book[0]);
	stairs_learn(&a, L->stairs);
	if (emu_debug_on()) {
		fprintf(stderr, "tiles area %d floor %d px down, faces %d px\n", area, L->book[0].dv, L->book[0].face);
		for (int d = 0; d < STAIR_DIRS; ++d)
			fprintf(stderr, "stairs area %d dir %d ok %d rise %d ramp %d walls %d prio %d tiles %d\n", area, d, L->stairs[d].ok,
				L->stairs[d].rise, L->stairs[d].nramp, L->stairs[d].nwalls, L->stairs[d].nprio, L->stairs[d].ntiles);
	}
	/* the map mirrored, for edges the original only has on its other side */
	area_src_mirror(&a, &m);
	tiles_learn(&m, na->styles, na->bg_in_map, &L->book[1]);
	area_src_free(&m);
	area_src_free(&a);
	return true;
}

/* ---- layout <-> world ---- */

/* grid x (down-right on screen) is world +Y, grid y (down-left) world -X */
static void grid_to_panel(int x, int y, int *A, int *B) { *A = -(y - place.gy0); *B = x - place.gx0; }

void netmap_world(int x, int y, int *wx, int *wy) {
	int A, B;
	grid_to_panel(x, y, &A, &B);
	*wx = place.ex + 16 + 32 * A;
	*wy = place.ey + 16 + 32 * B;
}

bool netmap_panel(int wx, int wy, int *x, int *y) {
	int A = floordiv(wx - place.ex, 32), B = floordiv(wy - place.ey, 32);
	*x = B + place.gx0;
	*y = -A + place.gy0;
	return true;
}

static const NetLayout *cur;
static uint32_t coord_slot;   /* the layer map's coordinate-data pointer */

enum { K_VOID, K_FLOOR, K_RAISED, K_STAIR };

/* What the layer has at grid cell (x, y). */
static int kind(int x, int y) {
	if (x < 0 || y < 0 || x >= cur->gw || y >= cur->gh || !cur->cell[y * cur->gw + x]) return K_VOID;
	for (int i = 0; i < cur->nstairs; ++i)
		if (x >= cur->stairs[i].x && x < cur->stairs[i].x + 2 && y >= cur->stairs[i].y && y < cur->stairs[i].y + 2) return K_STAIR;
	return cur->level && cur->level[y * cur->gw + x] ? K_RAISED : K_FLOOR;
}

static int kind_at(int A, int B) { return kind(B + place.gx0, -A + place.gy0); }

/* Floor as the screen shows it: ground panels where they are, raised ones
 * where a flat panel `rise` world units up the screen would be (stairs are
 * drawn from their own pieces). */
static bool floor_at(int A, int B) {
	int k = cur->rise / 32;
	return kind_at(A, B) == K_FLOOR || (k && kind_at(A - k, B + k) == K_RAISED);
}

static bool floor_cb(int A, int B, const void *ctx) { (void)ctx; return floor_at(A, B); }

int netmap_rise(void) { return cur ? cur->rise : 0; }

/* ---- output ---- */

/* A stair block's corner with the lowest panel indices, in world units. */
static void stair_origin(const Stair *st, int *X0, int *Y0) {
	int A0 = -(st->y + 1 - place.gy0), B0 = st->x - place.gx0;
	*X0 = place.ex + 32 * A0;
	*Y0 = place.ey + 32 * B0;
}

/* The stairs' own tiles over whatever the classes put there. */
static void paste_stairs(const Learned *L, uint16_t *map, int tw, int th) {
	size_t cells = (size_t)tw * th;
	for (int i = 0; i < cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		stair_origin(&cur->stairs[i], &X0, &Y0);
		int px0 = X0 + Y0 + tw * 4, py0 = (Y0 - X0) / 2 + th * 4;
		for (int k = 0; k < t->ntiles; ++k) {
			int px = px0 + t->tiles[k].px, py = py0 + t->tiles[k].py;
			if (px < 0 || py < 0 || (px & 7) || (py & 7) || px / 8 >= tw || py / 8 >= th) continue;
			size_t at = (size_t)(py / 8) * tw + px / 8;
			map[at] = t->tiles[k].e0;
			map[cells + at] = t->tiles[k].e1;
		}
	}
}

static bool write_tilemap(const Learned *L) {
	/* extent of the floor around the world origin */
	int umax = 0, vmax = 0;
	for (int y = 0; y < cur->gh; ++y)
		for (int x = 0; x < cur->gw; ++x) {
			if (!cur->cell[y * cur->gw + x]) continue;
			int X, Y;
			netmap_world(x, y, &X, &Y);
			int u = abs(X + Y), v = abs((Y - X) / 2);
			if (u > umax) umax = u;
			if (v > vmax) vmax = v;
		}
	int tw = 2 * ((umax + 64 + 7) / 8), th = 2 * ((vmax + 48 + cur->rise + 7) / 8);
	if ((tw & 1) != (L->tw & 1)) ++tw;   /* same tile phase as the source */
	if ((th & 1) != (L->th & 1)) ++th;
	if (tw > 255 || th > 255 || (size_t)tw * th * 4 > MAX_TILE_BYTES) return false;
	size_t cells = (size_t)tw * th;
	uint16_t *map = calloc(cells * 2, 2);
	TileGrid grid = { tw, th, place.ex, place.ey, L->book[0].dv, L->book[0].face };
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			const TileCand *b = tiles_pick(L->book, 2, &grid, tx, ty, floor_cb, NULL);
			if (!b) continue;
			map[(size_t)ty * tw + tx] = b->e0;
			map[cells + (size_t)ty * tw + tx] = b->e1;
		}
	size_t raw = cells * 4;
	uint8_t *out = malloc(16 + raw + raw / 8 + 16);
	paste_stairs(L, map, tw, th);
	size_t lz = lz_literal((const uint8_t *)map, raw, out + 12);
	out[0] = (uint8_t)tw; out[1] = (uint8_t)th; out[2] = out[3] = 0;
	put32(out + 4, 12);
	put32(out + 8, (uint32_t)(12 + cells * 2)); /* the second layer, in the decompressed buffer */
	emu_write(TILEMAP_AT, out, 12 + lz);
	if (emu_debug_on()) {
		FILE *f = emu_debug_file("gen_tilemap.bin");
		if (f) { fwrite(out, 1, 12, f); fwrite(map, 2, cells * 2, f); fclose(f); }
	}
	emu_write32(0x08000000u + L->desc + 8, TILEMAP_AT);
	free(out);
	free(map);
	return true;
}

/* the layer's floor at `level` in world panels: its own level and stairs */
static bool floor_level(int A, int B, int level) {
	int k = kind_at(A, B);
	return k == K_STAIR || k == (level ? K_RAISED : K_FLOOR);
}

bool netmap_floor_cell(int cx, int cy, int level) {
	/* the cell centre, relative to the panel edges */
	int X = cx * 8 + 4 - place.ex, Y = cy * 8 + 4 - place.ey;
	int A = floordiv(X, 32), B = floordiv(Y, 32);
	bool onx = (X & 31) == 0, ony = (Y & 31) == 0;
	if (!floor_level(A, B, level)) return false;
	if (onx && !floor_level(A - 1, B, level)) return false;
	if (ony && !floor_level(A, B - 1, level)) return false;
	if (onx && ony && !floor_level(A - 1, B - 1, level)) return false;
	return true;
}

bool netmap_stair_cell(int cx, int cy) {
	int A = floordiv(cx * 8 + 4 - place.ex, 32), B = floordiv(cy * 8 + 4 - place.ey, 32);
	return kind_at(A, B) == K_STAIR;
}

/* Coordinate cells beyond the walls: raised floor heights and the stairs'
 * own ramps, walls and layer priorities, placed at their blocks. */
static CoordExtra extra;
static CoordCell extra_cells[4][4096];

static void add_extra(int s, CoordCell c) {
	if (extra.n[s] < 4096) extra_cells[s][extra.n[s]++] = c;
}

static void build_extra(const Learned *L) {
	memset(&extra, 0, sizeof extra);
	for (int s = 0; s < 4; ++s) extra.cells[s] = extra_cells[s];
	for (int y = 0; y < cur->gh; ++y)
		for (int x = 0; x < cur->gw; ++x) {
			if (kind(x, y) != K_RAISED) continue;
			int A, B;
			grid_to_panel(x, y, &A, &B);
			int X0 = place.ex + 32 * A, Y0 = place.ey + 32 * B;
			/* the 8 x 8 cells whose centres lie on the panel */
			for (int cy = floordiv(Y0 - 4 + 7, 8); cy * 8 + 4 < Y0 + 32; ++cy)
				for (int cx = floordiv(X0 - 4 + 7, 8); cx * 8 + 4 < X0 + 32; ++cx) {
					CoordCell c = { (int16_t)(cx * 8), (int16_t)(cy * 8), (int8_t)cur->rise, 0, 0, 0x11 };
					add_extra(1, c);
				}
		}
	for (int i = 0; i < cur->nstairs; ++i) {
		const StairTemplate *t = &L->stairs[cur->stairs[i].dir];
		if (!t->ok) continue;
		int X0, Y0;
		stair_origin(&cur->stairs[i], &X0, &Y0);
		const CoordCell *src[3] = { t->walls, t->ramp, t->prio };
		int n[3] = { t->nwalls, t->nramp, t->nprio };
		for (int s = 0; s < 3; ++s)
			for (int k = 0; k < n[s]; ++k) {
				CoordCell c = src[s][k];
				c.x = (int16_t)(c.x + X0);
				c.y = (int16_t)(c.y + Y0);
				add_extra(s, c);
			}
	}
}

bool netmap_build(int area, const NetLayout *lay) {
	if (area < 0 || area >= 8) return false;
	Learned *L = &learned[area];
	if (!L->tried) { L->tried = true; L->ok = learn(area, L); }
	if (!L->ok) return false;
	cur = lay;
	/* centre the floor on the world origin, across (x - y) and up and down
	 * (x + y) the screen */
	int u0 = 1 << 30, u1 = -(1 << 30), v0 = 1 << 30, v1 = -(1 << 30);
	for (int y = 0; y < lay->gh; ++y)
		for (int x = 0; x < lay->gw; ++x)
			if (lay->cell[y * lay->gw + x]) {
				if (x - y < u0) u0 = x - y;
				if (x - y > u1) u1 = x - y;
				if (x + y < v0) v0 = x + y;
				if (x + y > v1) v1 = x + y;
			}
	if (u1 < u0) return false;
	int cu = (u0 + u1) / 2, cv = (v0 + v1) / 2;
	place.gx0 = (cu + cv) / 2;
	place.gy0 = (cv - cu) / 2;
	place.ex = L->ex;
	place.ey = L->ey;
	coord_slot = L->coord_slot;
	build_extra(L);
	return write_tilemap(L) && coords_write(coord_slot, NULL, 0, &extra);
}

bool netmap_set_pads(const CoordPad *pads, int n) { return coords_write(coord_slot, pads, n, &extra); }

unsigned netmap_stair_dirs(int area, int *rise) {
	if (area < 0 || area >= 8) return 0;
	Learned *L = &learned[area];
	if (!L->tried) { L->tried = true; L->ok = learn(area, L); }
	unsigned dirs = 0;
	*rise = 0;
	for (int d = 0; d < STAIR_DIRS; ++d)
		if (L->ok && L->stairs[d].ok) { dirs |= 1u << d; *rise = L->stairs[d].rise; }
	return dirs;
}

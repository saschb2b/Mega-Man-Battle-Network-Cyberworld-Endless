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
#include "decor.h"
#include "emu.h"
#include "legal.h"
#include "lz.h"
#include "stairs.h"
#include "tilemap.h"
#include "tiles.h"
#include "rom.h"

#define TILEMAP_AT  (EMU_FREE + 0x10000) /* generated tile map (LZ77) */
#define MAX_TILE_BYTES 0x14000           /* the game's tile map buffer */

#define MAX_BOOKS (4 * (1 + NET_MORE_MAPS))   /* maps, their mirrors, two heights each */

typedef struct {
	bool tried, ok;
	int ex, ey, tw, th;
	TileBook book[MAX_BOOKS]; /* each source map, then its mirror image */
	int nbooks;
	TileSeams seams;          /* the tiles the maps set side by side */
	DecorBook decor;          /* the scenery of the area's maps */
	uint32_t desc, coord_slot;
	StairTemplate stairs[STAIR_DIRS];
} Learned;

static Learned learned[NET_AREAS];

int netmap_scenery;
LegalStats netmap_legal;

/* the last tile map written, both layers (for the dev tools) */
static struct { uint16_t *map; uint8_t *seams; int tw, th; } last;   /* seams: bit 0 right, bit 1 below */

/* the current layer's placement */
static struct { int gx0, gy0, ex, ey; } place;

/* ---- helpers ---- */

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* Where map a's tiles fall in its panels (world units mod 32, of tile 0's
 * centre). */
static void lattice(const AreaSrc *a, int *fx, int *fy) {
	int u = 4 - a->tw * 4, v = 4 - a->th * 4;
	*fx = ((u - 2 * v) / 2 - a->ex) & 31;
	*fy = ((u + 2 * v) / 2 - a->ey) & 31;
}

/* Whether map b's tiles lie where map a's do in their panels: then a tile
 * of b drawn in a layer made on a's grid shows as it did in b. Tiles step
 * by (4, 4) and (-8, 8) world units. */
static bool aligned(const AreaSrc *a, const AreaSrc *b) {
	int ax, ay, bx, by;
	lattice(a, &ax, &ay);
	lattice(b, &bx, &by);
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 4; ++j)
			if (((ax + 4 * i - 8 * j - bx) & 31) == 0 && ((ay + 4 * i + 8 * j - by) & 31) == 0) return true;
	return false;
}

/* Learns the tiles of map `src` and its mirror image, where their tiles
 * line up with the layer's grid (`grid_of`). */
static void learn_view(const AreaSrc *src, const AreaSrc *grid_of, int area, Learned *L) {
	const __typeof__(R.layout->net_area[0]) *na = &R.layout->net_area[area];
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, src)) {
		tiles_learn(src, na->styles, na->walk_styles, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, src, na->bg_in_map);
	}
	AreaSrc m;
	area_src_mirror(src, &m);
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, &m)) {
		tiles_learn(&m, na->styles, na->walk_styles, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, &m, na->bg_in_map);
	}
	area_src_free(&m);
}

#define LEVEL_MIN_CELLS 144   /* nine panels of floor at a height to learn it */

/* ... at each of its floor heights: raised floors are drawn higher, so each
 * is learned in a view that brings it down to the ground. */
static void learn_map(const AreaSrc *src, const AreaSrc *grid_of, int area, Learned *L) {
	int count[256] = { 0 };
	for (int i = 0; src->hz && i < src->hw * src->hh; ++i) count[src->hz[i]]++;
	learn_view(src, grid_of, area, L);
	for (int z = 8; z < HEIGHT_UNEVEN; z += 8) {
		if (count[z] < LEVEL_MIN_CELLS) continue;
		AreaSrc r;
		area_src_raise(src, z, &r);
		learn_view(&r, grid_of, area, L);
		area_src_free(&r);
	}
}

static bool learn(int area, Learned *L) {
	const __typeof__(R.layout->net_area[0]) *na = &R.layout->net_area[area];
	AreaSrc a;
	if (!area_src_load(na->group, na->number, &a)) return false;
	L->ex = a.ex; L->ey = a.ey; L->tw = a.tw; L->th = a.th;
	L->desc = a.desc; L->coord_slot = a.coord_slot;
	L->nbooks = 0;
	learn_map(&a, &a, area, L);
	decor_learn(&a, na->bg_in_map, &L->decor);
	stairs_learn(&a, L->stairs);
	/* the area's other maps in the same tiles and colours, for the places
	 * this one never shows */
	for (int k = 0; k < NET_MORE_MAPS && na->more[k][0]; ++k) {
		AreaSrc b;
		if (!area_src_load(na->more[k][0], na->more[k][1], &b)) continue;
		learn_map(&b, &a, area, L);
		decor_learn(&b, na->bg_in_map, &L->decor);
		area_src_free(&b);
	}
	if (emu_debug_on()) {
		fprintf(stderr, "tiles area %d floor %d px down, faces %d px, hanging %d px, %d books, %d pieces of scenery\n", area, L->book[0].dv, L->book[0].face, L->book[0].hang, L->nbooks, L->decor.n);
		for (int d = 0; d < STAIR_DIRS; ++d)
			fprintf(stderr, "stairs area %d dir %d ok %d rise %d ramp %d walls %d prio %d tiles %d\n", area, d, L->stairs[d].ok,
				L->stairs[d].rise, L->stairs[d].nramp, L->stairs[d].nwalls, L->stairs[d].nprio, L->stairs[d].ntiles);
	}
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
static bool one_floor;        /* the area has no walkway floor: all is platform */
static bool by_shape;         /* its floors are told by shape: an arena is platform */
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


/* A walkway: a floor cell in no 2x2 block of floor, as the original areas
 * draw their 1-wide paths in their second floor. */
static bool walkway(int x, int y) {
	for (int dy = -1; dy <= 0; ++dy)
		for (int dx = -1; dx <= 0; ++dx)
			if (kind(x + dx, y + dy) && kind(x + dx + 1, y + dy) && kind(x + dx, y + dy + 1) && kind(x + dx + 1, y + dy + 1)) return false;
	return true;
}

/* The floor's material as the screen shows it: ground panels where they
 * are, raised ones where a flat panel `rise` world units up the screen
 * would be (stairs are drawn from their own pieces). */
/* A floor cell beside the void (8 ways): a platform's rim. */
static bool edge(int x, int y) {
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
			if (kind(x + dx, y + dy) == K_VOID) return true;
	return false;
}

static int floor_cb(int A, int B, const void *ctx) {
	(void)ctx;
	int k = cur->rise / 32, x = B + place.gx0, y = -A + place.gy0;
	if (kind(x, y) != K_FLOOR && k) {
		x += k; y += k;   /* a raised panel drawn here */
		if (kind(x, y) != K_RAISED) return TILE_VOID;
	} else if (kind(x, y) != K_FLOOR) return TILE_VOID;
	int pad = cur->pad && cur->pad[y * cur->gw + x] ? TILE_PAD : 0;
	if (one_floor) return TILE_A | pad;
	/* by shape: walkways and platforms' rims one floor, their middles the other */
	if (by_shape) return (walkway(x, y) || edge(x, y) ? TILE_B : TILE_A) | pad;
	if (!by_shape && x >= cur->ax && x < cur->ax + cur->aw && y >= cur->ay && y < cur->ay + cur->ah) return TILE_B | pad;
	return (walkway(x, y) ? TILE_B : TILE_A) | pad;
}

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
	TileGrid grid = { tw, th, place.ex, place.ey, L->book[0].dv, L->book[0].face, L->book[0].hang, by_shape };
	free(last.seams);
	last.seams = calloc(cells, 1);
	tilemap_pick(L->book, L->nbooks, &L->seams, &grid, floor_cb, NULL, map, last.seams);
	for (size_t i = 0; i < cells; ++i) tiles_stats.seams += (last.seams[i] & 1) + (last.seams[i] >> 1);
	size_t raw = cells * 4;
	uint8_t *out = malloc(16 + raw + raw / 8 + 16);
	paste_stairs(L, map, tw, th);
	netmap_scenery = decor_place(&L->decor, map, tw, th, cur->seed);
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
	free(last.map);
	last.map = map;
	last.tw = tw;
	last.th = th;
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

/* Centres the floor on the world origin, across (x - y) and up and down
 * (x + y) the screen; false without floor. */
static bool centre(const NetLayout *lay) {
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
	return true;
}

/* ---- a floor the tiles can draw ---- */

#define LEGAL_BUDGET 400   /* cells changed at most */

/* Whether the panel of grid cell (x, y) has a neighbourhood the original
 * maps show. */
static bool legal_clean(int x, int y, void *ctx) {
	const Learned *L = ctx;
	int A, B;
	grid_to_panel(x, y, &A, &B);
	unsigned oa = 0, ob = 0;
	for (int k = 0; k < 9; ++k) {
		int m = TILE_MATERIAL(floor_cb(A + k % 3 - 1, B + k / 3 - 1, NULL));
		if (m == TILE_A) oa |= 1u << k;
		if (m == TILE_B) ob |= 1u << k;
	}
	return tiles_shape_seen(L->book, L->nbooks, TILE_SHAPE(oa, ob, floor_cb(A, B, NULL) & TILE_PAD));
}

static void legalize(Learned *L, const NetLayout *lay) {
	LegalGrid g = { lay->gw, lay->gh, lay->cell, lay->locked, legal_clean, L };
	netmap_legal = legal_fix(&g, LEGAL_BUDGET);
}

bool netmap_build(int area, const NetLayout *lay) {
	if (area < 0 || area >= NET_AREAS) return false;
	Learned *L = &learned[area];
	if (!L->tried) { L->tried = true; L->ok = learn(area, L); }
	if (!L->ok) return false;
	cur = lay;
	one_floor = !R.layout->net_area[area].walk_styles;
	by_shape = R.layout->net_area[area].styles & TILES_BY_SHAPE;
	place.ex = L->ex;
	place.ey = L->ey;
	if (!centre(lay)) return false;
	netmap_legal = (LegalStats){ 0, 0 };
	if (lay->locked) {
		legalize(L, lay);
		centre(lay);
	}
	coord_slot = L->coord_slot;
	build_extra(L);
	return write_tilemap(L) && coords_write(coord_slot, NULL, 0, &extra);
}

bool netmap_set_pads(const CoordPad *pads, int n) { return coords_write(coord_slot, pads, n, &extra); }

unsigned netmap_stair_dirs(int area, int *rise) {
	if (area < 0 || area >= NET_AREAS) return 0;
	Learned *L = &learned[area];
	if (!L->tried) { L->tried = true; L->ok = learn(area, L); }
	unsigned dirs = 0;
	*rise = 0;
	for (int d = 0; d < STAIR_DIRS; ++d)
		if (L->ok && L->stairs[d].ok) { dirs |= 1u << d; *rise = L->stairs[d].rise; }
	return dirs;
}

/* Locks the w x h cells from (x, y) and `margin` around them. */
static void lock(uint8_t locked[MAP_H][MAP_W], int x, int y, int w, int h, int margin) {
	for (int j = y - margin; j < y + h + margin; ++j)
		for (int i = x - margin; i < x + w + margin; ++i)
			if (i >= 0 && j >= 0 && i < MAP_W && j < MAP_H) locked[j][i] = 1;
}

bool netmap_build_layer(int area, uint32_t seed) {
	/* the pads, in their own look (and where the area's platforms are pads,
	 * the small platforms) */
	static uint8_t pads[MAP_H][MAP_W];
	memset(pads, 0, sizeof pads);
	for (int r = 0; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		int small = R.layout->net_area[area].pad_rooms;
		if (m->kind != ROOM_PAD && !(m->kind == ROOM_PLATFORM && m->w * m->h <= small)) continue;
		for (int y = m->y; y < m->y + m->h; ++y)
			for (int x = m->x; x < m->x + m->w; ++x) pads[y][x] = 1;
	}
	/* what the floor must keep: the cells of objects, rooms' anchors, pads
	 * and the guardian's arena, and the stairs and raised floors with the
	 * cells beside them */
	static uint8_t locked[MAP_H][MAP_W];
	memset(locked, 0, sizeof locked);
	for (int i = 0; i < layer.nobj; ++i) lock(locked, (int)layer.obj[i].x, (int)layer.obj[i].y, 1, 1, 0);
	for (int r = 0; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		lock(locked, m->ax, m->ay, 1, 1, 0);
		if (m->kind == ROOM_PAD || r == layer.arena) lock(locked, m->x, m->y, m->w, m->h, 0);
	}
	for (int i = 0; i < layer.nstairs; ++i) lock(locked, layer.stair[i].x, layer.stair[i].y, 2, 2, 2);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (layer.level[y][x]) lock(locked, x, y, 1, 1, 2);
	static NetLayout lay;
	lay = (NetLayout){ MAP_W, MAP_H, &layer.cell[0][0], &locked[0][0], &layer.level[0][0], layer.rise, layer.stair, layer.nstairs, 0, 0, 0, 0, seed, &pads[0][0] };
	if (layer.arena >= 0) {
		const Room *a = &layer.rooms[layer.arena];
		lay.ax = a->x; lay.ay = a->y; lay.aw = a->w; lay.ah = a->h;
	}
	return netmap_build(area, &lay);
}

const uint16_t *netmap_last_tiles(int *tw, int *th) {
	*tw = last.tw;
	*th = last.th;
	return last.map;
}

const uint8_t *netmap_last_seams(void) { return last.seams; }

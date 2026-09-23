/* Generated layers as game maps.
 *
 * Tiles: every tile of the original area map is classified by where its
 * centre falls inside a panel (in 4-unit steps) and which of the 3x3 panels
 * around it are floor; the most common pair of layer entries per class is
 * kept (only around panels of the area's chosen style). A generated layer
 * then takes, tile by tile, the pair for its own class, with fewer
 * neighbours when the original has no example.
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
#include "emu.h"
#include "rom.h"

#define TILEMAP_AT  (EMU_FREE + 0x10000) /* generated tile map (LZ77) */
#define COORD_AT    (EMU_FREE + 0x60000) /* generated coordinate data */
#define MAX_TILE_BYTES 0x14000           /* the game's tile map buffer */

typedef struct { uint32_t key; uint16_t e0, e1; } Sample;
typedef struct { uint32_t key; uint16_t e0, e1; } Best;

typedef struct {
	bool tried, ok;
	int ex, ey, tw, th;
	Best *best[3];       /* per fallback level, sorted by key */
	int nbest[3];
	uint32_t desc, coord_slot;
} Learned;

static Learned learned[8];

/* the current layer's placement */
static struct { int gx0, gy0, ex, ey; } place;

/* ---- helpers ---- */

static int floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

static int style_at(const AreaSrc *a, int cx, int cy) {
	long r = 0, g = 0, b = 0, n = 0;
	int W = a->tw * 8, H = a->th * 8;
	for (int y = cy - 4; y < cy + 4; ++y)
		for (int x = cx - 8; x < cx + 8; ++x) {
			if (x < 0 || y < 0 || x >= W || y >= H) continue;
			uint32_t c = a->px[(size_t)y * W + x];
			r += (c >> 16) & 255; g += (c >> 8) & 255; b += c & 255; ++n;
		}
	if (!n) return 12;
	float fr = (float)r / n, fg = (float)g / n, fb = (float)b / n;
	float mx = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb);
	float mn = fr < fg ? (fr < fb ? fr : fb) : (fg < fb ? fg : fb);
	if (mx <= 0 || (mx - mn) / mx <= 0.25f) return 12;
	float d = mx - mn, hue = mx == fr ? (fg - fb) / d : mx == fg ? 2 + (fb - fr) / d : 4 + (fr - fg) / d;
	hue /= 6;
	if (hue < 0) hue += 1;
	int k = (int)(hue * 12);
	return k > 11 ? 11 : k;
}

/* Panel state in the source map: 0 empty, 1 floor of another style, 2 good floor */
static int src_panel(const AreaSrc *a, int A, int B, uint16_t styles) {
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	int px = area_px(a->tw, X, Y), py = area_py(a->th, X, Y);
	int W = a->tw * 8, H = a->th * 8;
	if (px < 0 || py < 0 || px >= W || py >= H || !(a->px[(size_t)py * W + px] >> 24)) return 0;
	return (styles >> style_at(a, px, py) & 1) ? 2 : 1;
}

/* Class of a tile at (px, py) of a map tw x th tiles with edges (ex, ey):
 * phase (6 bits) and the panel it lies in. */
static void tile_class(int tw, int th, int ex, int ey, int tx, int ty, int *phase, int *A, int *B) {
	int u = tx * 8 + 4 - tw * 4, v = ty * 8 + 4 - th * 4;
	int X = (u - 2 * v) / 2, Y = (u + 2 * v) / 2;
	*A = floordiv(X - ex, 32);
	*B = floordiv(Y - ey, 32);
	int fx = X - ex - 32 * *A, fy = Y - ey - 32 * *B;
	*phase = (fx / 4) * 8 + fy / 4;
}

static const uint16_t level_mask[3] = { 0x1FF, 0x0BA, 0x010 }; /* all 9, centre + axes, centre */

static uint32_t make_key(int phase, unsigned occ, int level) { return (uint32_t)phase << 9 | (occ & level_mask[level]); }

static int cmp_sample(const void *a, const void *b) {
	const Sample *x = a, *y = b;
	if (x->key != y->key) return x->key < y->key ? -1 : 1;
	if (x->e0 != y->e0) return x->e0 < y->e0 ? -1 : 1;
	return (x->e1 > y->e1) - (x->e1 < y->e1);
}

static bool learn(int area, Learned *L) {
	const __typeof__(R.layout->net_area[0]) *na = &R.layout->net_area[area];
	AreaSrc a;
	if (!area_src_load(na->group, na->number, &a)) return false;
	L->ex = a.ex; L->ey = a.ey; L->tw = a.tw; L->th = a.th;
	L->desc = a.desc; L->coord_slot = a.coord_slot;
	size_t cells = (size_t)a.tw * a.th;
	Sample *s = malloc(cells * sizeof *s);
	for (int level = 0; level < 3; ++level) {
		size_t n = 0;
		for (int ty = 0; ty < a.th; ++ty)
			for (int tx = 0; tx < a.tw; ++tx) {
				int phase, A, B;
				tile_class(a.tw, a.th, a.ex, a.ey, tx, ty, &phase, &A, &B);
				unsigned occ = 0;
				bool mixed = false;
				for (int k = 0; k < 9; ++k) {
					int st = src_panel(&a, A + k % 3 - 1, B + k / 3 - 1, na->styles);
					if (st == 1) mixed = true;
					if (st) occ |= 1u << k;
				}
				if (mixed || !occ) continue;
				size_t i = (size_t)ty * a.tw + tx;
				s[n++] = (Sample){ make_key(phase, occ, level), a.tile[0][i], a.layers > 1 ? a.tile[1][i] : 0 };
			}
		qsort(s, n, sizeof *s, cmp_sample);
		/* keep the most common pair per key */
		L->best[level] = malloc((n + 1) * sizeof(Best));
		int nb = 0;
		for (size_t i = 0; i < n;) {
			size_t j = i, best_at = i, best_run = 0;
			while (j < n && s[j].key == s[i].key) {
				size_t k = j;
				while (k < n && s[k].key == s[j].key && s[k].e0 == s[j].e0 && s[k].e1 == s[j].e1) ++k;
				if (k - j > best_run) { best_run = k - j; best_at = j; }
				j = k;
			}
			L->best[level][nb++] = (Best){ s[best_at].key, s[best_at].e0, s[best_at].e1 };
			i = j;
		}
		L->nbest[level] = nb;
	}
	free(s);
	area_src_free(&a);
	return true;
}

static const Best *lookup(const Learned *L, int level, uint32_t key) {
	int lo = 0, hi = L->nbest[level] - 1;
	while (lo <= hi) {
		int mid = (lo + hi) / 2;
		uint32_t k = L->best[level][mid].key;
		if (k == key) return &L->best[level][mid];
		if (k < key) lo = mid + 1; else hi = mid - 1;
	}
	return NULL;
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
static bool floor_at(int A, int B) {
	int x = B + place.gx0, y = -A + place.gy0;
	return x >= 0 && y >= 0 && x < cur->gw && y < cur->gh && cur->cell[y * cur->gw + x];
}

/* ---- output ---- */

static size_t lz_literal(const uint8_t *src, size_t n, uint8_t *out) {
	/* GBA LZ77 (type 0x10) made of literal blocks only */
	size_t o = 0;
	out[o++] = 0x10; out[o++] = (uint8_t)n; out[o++] = (uint8_t)(n >> 8); out[o++] = (uint8_t)(n >> 16);
	for (size_t i = 0; i < n; i += 8) {
		out[o++] = 0;
		for (size_t k = 0; k < 8; ++k) out[o++] = i + k < n ? src[i + k] : 0;
	}
	while (o & 3) out[o++] = 0;
	return o;
}

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

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
	int tw = 2 * ((umax + 64 + 7) / 8), th = 2 * ((vmax + 48 + 7) / 8);
	if ((tw & 1) != (L->tw & 1)) ++tw;   /* same tile phase as the source */
	if ((th & 1) != (L->th & 1)) ++th;
	if (tw > 255 || th > 255 || (size_t)tw * th * 4 > MAX_TILE_BYTES) return false;
	size_t cells = (size_t)tw * th;
	uint16_t *map = calloc(cells * 2, 2);
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			int phase, A, B;
			tile_class(tw, th, place.ex, place.ey, tx, ty, &phase, &A, &B);
			unsigned occ = 0;
			for (int k = 0; k < 9; ++k)
				if (floor_at(A + k % 3 - 1, B + k / 3 - 1)) occ |= 1u << k;
			if (!occ) continue;
			const Best *b = NULL;
			for (int level = 0; level < 3 && !b; ++level) b = lookup(L, level, make_key(phase, occ, level));
			if (!b) continue;
			map[(size_t)ty * tw + tx] = b->e0;
			map[cells + (size_t)ty * tw + tx] = b->e1;
		}
	size_t raw = cells * 4;
	uint8_t *out = malloc(16 + raw + raw / 8 + 16);
	size_t lz = lz_literal((const uint8_t *)map, raw, out + 12);
	out[0] = (uint8_t)tw; out[1] = (uint8_t)th; out[2] = out[3] = 0;
	put32(out + 4, 12);
	put32(out + 8, (uint32_t)(12 + cells * 2)); /* the second layer, in the decompressed buffer */
	emu_write(TILEMAP_AT, out, 12 + lz);
	if (getenv("CYBERWORLD_EMU_DEBUG")) {
		FILE *f = fopen("/src/.build/gen_tilemap.bin", "wb");
		if (f) { fwrite(out, 1, 12, f); fwrite(map, 2, cells * 2, f); fclose(f); }
	}
	emu_write32(0x08000000u + L->desc + 8, TILEMAP_AT);
	free(out);
	free(map);
	return true;
}

typedef struct { uint16_t key, type; } Wall;
static int cmp_wall(const void *a, const void *b) {
	const Wall *x = a, *y = b;
	return x->key != y->key ? (x->key < y->key ? -1 : 1) : (x->type - y->type);
}

static bool floor_cell(int cx, int cy) {
	/* the cell centre, relative to the panel edges */
	int X = cx * 8 + 4 - place.ex, Y = cy * 8 + 4 - place.ey;
	int A = floordiv(X, 32), B = floordiv(Y, 32);
	bool onx = (X & 31) == 0, ony = (Y & 31) == 0;
	if (!floor_at(A, B)) return false;
	if (onx && !floor_at(A - 1, B)) return false;
	if (ony && !floor_at(A, B - 1)) return false;
	if (onx && ony && !floor_at(A - 1, B - 1)) return false;
	return true;
}

static bool write_walls(const Learned *L) {
	int cap = 8192, n = 0;
	Wall *w = malloc((size_t)cap * sizeof *w);
	for (int cy = -126; cy < 126; ++cy)
		for (int cx = -126; cx < 126; ++cx) {
			if (floor_cell(cx, cy)) continue;
			bool mx = floor_cell(cx - 1, cy), px = floor_cell(cx + 1, cy), my = floor_cell(cx, cy - 1), py = floor_cell(cx, cy + 1);
			int types[4], nt = 0;
			if (mx) types[nt++] = 1;
			if (px) types[nt++] = 2;
			if (my) types[nt++] = 3;
			if (py) types[nt++] = 4;
			if (!nt) {
				if (floor_cell(cx - 1, cy + 1)) types[nt++] = 7;
				else if (floor_cell(cx - 1, cy - 1)) types[nt++] = 5;
				else if (floor_cell(cx + 1, cy - 1)) types[nt++] = 6;
				else if (floor_cell(cx + 1, cy + 1)) types[nt++] = 8;
			}
			for (int k = 0; k < nt && n < cap; ++k) w[n++] = (Wall){ (uint16_t)((cy + 127) * 254 + (cx + 127)), (uint16_t)types[k] };
		}
	qsort(w, (size_t)n, sizeof *w, cmp_wall);
	if (getenv("CYBERWORLD_EMU_DEBUG")) {
		for (int cy = -20; cy < 36; ++cy) {
			char line[80];
			for (int cx = -20; cx < 36; ++cx) {
				char c = floor_cell(cx, cy) ? '.' : ' ';
				for (int i = 0; i < n; ++i) if (w[i].key == (cy + 127) * 254 + (cx + 127)) c = (char)('0' + w[i].type);
				line[cx + 20] = c;
			}
			line[56] = 0;
			fprintf(stderr, "%4d %s\n", cy * 8, line);
		}
	}
	/* section 0: count, (key, offset) entries, then one shape per type */
	size_t s0 = 4 + (size_t)n * 4 + 8 * 4;
	size_t total = s0 + 12;
	uint8_t *d = calloc(total, 1);
	put32(d, (uint32_t)n);
	uint16_t shape_off = (uint16_t)(n * 4); /* relative to d + 4 */
	for (int i = 0; i < n; ++i) {
		uint16_t off = (uint16_t)(shape_off + (w[i].type - 1) * 4);
		d[4 + i * 4] = (uint8_t)w[i].key; d[5 + i * 4] = (uint8_t)(w[i].key >> 8);
		d[6 + i * 4] = (uint8_t)off; d[7 + i * 4] = (uint8_t)(off >> 8);
	}
	for (int t = 1; t <= 8; ++t) {
		uint8_t *sh = d + 4 + shape_off + (t - 1) * 4;
		sh[0] = 0; sh[1] = 0; sh[2] = 8; sh[3] = (uint8_t)t;   /* z 0, no flag, height 8 */
	}
	/* sections 1-3 (height changes, layer priorities, triggers) stay empty */
	uint8_t *out = malloc(16 + total + total / 8 + 16);
	put32(out, 0);
	put32(out + 4, (uint32_t)s0);
	put32(out + 8, (uint32_t)(s0 + 4));
	put32(out + 12, (uint32_t)(s0 + 8));
	size_t lz = lz_literal(d, total, out + 16);
	emu_write(COORD_AT, out, 16 + lz);
	emu_write32(0x08000000u + L->coord_slot, COORD_AT);
	free(out);
	free(d);
	free(w);
	return true;
}

bool netmap_build(int area, const NetLayout *lay) {
	if (area < 0 || area >= 8) return false;
	Learned *L = &learned[area];
	if (!L->tried) { L->tried = true; L->ok = learn(area, L); }
	if (!L->ok) return false;
	cur = lay;
	/* centre the floor on the world origin */
	int x0 = lay->gw, y0 = lay->gh, x1 = -1, y1 = -1;
	for (int y = 0; y < lay->gh; ++y)
		for (int x = 0; x < lay->gw; ++x)
			if (lay->cell[y * lay->gw + x]) {
				if (x < x0) x0 = x;
				if (x > x1) x1 = x;
				if (y < y0) y0 = y;
				if (y > y1) y1 = y;
			}
	if (x1 < 0) return false;
	place.gx0 = (x0 + x1) / 2;
	place.gy0 = (y0 + y1) / 2;
	place.ex = L->ex;
	place.ey = L->ey;
	if (!write_tilemap(L) || !write_walls(L)) return false;
	return true;
}

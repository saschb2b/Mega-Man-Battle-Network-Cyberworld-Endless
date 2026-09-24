/* Decoding an original internet map: MapBGDescriptor (tile sets, palette,
 * LZ77 tile map with two layers) from the table at 0x0329C4, and the
 * coordinate data (LZ77 wall list) from the table at 0x03354C, whose walls
 * give where the panel edges fall in world units. */
#include "area_src.h"

#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "rom.h"

#define MAP_TABLE   0x0329C4u /* MapBGDescriptor lists, internet groups from 0x80 */
#define COORD_TABLE 0x03354Cu /* coordinate data lists, internet groups from 0x80 */

static bool decode_tiles(AreaSrc *a) {
	uint32_t list = rom_u32(MAP_TABLE + (uint32_t)(a->group - 0x80) * 4);
	if (!rom_is_ptr(list)) return false;
	a->desc = rom_off(list) + (uint32_t)a->number * 12;
	uint32_t ts = rom_u32(a->desc), pal = rom_u32(a->desc + 4), tm = rom_u32(a->desc + 8);
	if (!rom_is_ptr(ts) || !rom_is_ptr(pal) || !rom_is_ptr(tm)) return false;
	ts = rom_off(ts); pal = rom_off(pal) + 4; tm = rom_off(tm);
	a->tw = R.data[tm]; a->th = R.data[tm + 1];
	size_t n = 0;
	uint8_t *m = lz77_decompress(R.data + tm + 12, ROM_SIZE - (tm + 12), &n);
	if (!m || a->tw <= 0 || a->th <= 0) { free(m); return false; }
	size_t cells = (size_t)a->tw * a->th;
	a->layers = (int)(n / (cells * 2));
	if (a->layers > 2) a->layers = 2;
	for (int l = 0; l < a->layers; ++l) {
		a->tile[l] = malloc(cells * 2);
		for (size_t i = 0; i < cells; ++i) a->tile[l][i] = (uint16_t)(m[(l * cells + i) * 2] | m[(l * cells + i) * 2 + 1] << 8);
	}
	free(m);
	/* draw it, back layer first */
	uint8_t *vram = calloc(0x10000, 1);
	for (int k = 0; k < 2; ++k) {
		uint32_t wc = rom_u32(ts + (uint32_t)k * 12), off = rom_u32(ts + (uint32_t)k * 12 + 4), vo = rom_u32(ts + (uint32_t)k * 12 + 8);
		if (!wc) continue;
		size_t tn = 0;
		uint8_t *t = lz77_decompress(R.data + ts + off, ROM_SIZE - (ts + off), &tn);
		if (!t) continue;
		size_t want = (size_t)wc * 4 < tn ? (size_t)wc * 4 : tn;
		if (vo < 0x10000) memcpy(vram + vo, t, want < 0x10000 - vo ? want : 0x10000 - vo);
		free(t);
	}
	uint32_t colors[256];
	for (int i = 0; i < 256; ++i) colors[i] = bgr555(rom_u16(pal + (uint32_t)i * 2));
	a->px = calloc(cells * 64, 4);
	a->front = calloc(cells * 64, 1);
	int W = a->tw * 8;
	for (int l = a->layers - 1; l >= 0; --l)
		for (int ty = 0; ty < a->th; ++ty)
			for (int tx = 0; tx < a->tw; ++tx) {
				uint16_t e = a->tile[l][ty * a->tw + tx];
				if (!(e & 0x3FF)) continue;
				const uint8_t *tile = vram + (e & 0x3FF) * 32;
				for (int y = 0; y < 8; ++y)
					for (int x = 0; x < 8; ++x) {
						uint8_t v = tile[y * 4 + x / 2];
						int ci = (x & 1) ? v >> 4 : v & 15;
						if (!ci) continue;
						int X = (e & 0x400) ? 7 - x : x, Y = (e & 0x800) ? 7 - y : y;
						a->px[(size_t)(ty * 8 + Y) * W + tx * 8 + X] = colors[(e >> 12) * 16 + ci];
						if (l == 0) a->front[(size_t)(ty * 8 + Y) * W + tx * 8 + X] = 1;
					}
			}
	free(vram);
	return true;
}

/* The coordinate data's four sections, each a count, (key, offset)
 * entries and 4-byte shapes (see coords.c). */
static void decode_coords(AreaSrc *a) {
	uint32_t list = rom_u32(COORD_TABLE + (uint32_t)(a->group - 0x80) * 4);
	if (!rom_is_ptr(list)) return;
	a->coord_slot = rom_off(list) + (uint32_t)a->number * 4;
	uint32_t c = rom_u32(a->coord_slot);
	if (!rom_is_ptr(c)) return;
	c = rom_off(c);
	size_t n = 0;
	uint8_t *d = lz77_decompress(R.data + c + 16, ROM_SIZE - (c + 16), &n);
	if (!d) return;
	for (int s = 0; s < 4; ++s) {
		uint32_t at = rom_u32(c + (uint32_t)s * 4);
		if (at + 4 > n) continue;
		uint32_t count = (uint32_t)(d[at] | d[at + 1] << 8 | d[at + 2] << 16 | d[at + 3] << 24);
		a->sec[s] = calloc(count + 1, sizeof(CoordCell));
		for (uint32_t i = 0; i < count && at + 8 + i * 4 <= n; ++i) {
			uint32_t e = at + 4 + i * 4;
			int key = d[e] | d[e + 1] << 8, off = d[e + 2] | d[e + 3] << 8;
			if ((size_t)(at + 4 + off + 4) > n) continue;
			const uint8_t *sh = d + at + 4 + off;
			CoordCell *cc = &a->sec[s][a->nsec[s]++];
			cc->x = (int16_t)((key % 254 - 127) * 8);
			cc->y = (int16_t)((key / 254 - 127) * 8);
			cc->z = (int8_t)sh[0];
			cc->value = sh[1];
			cc->height = sh[2];
			cc->type = sh[3];
		}
	}
	free(d);
}

/* The most common edge position (mod 32) of the NE (type 1) and NW (type 4)
 * walls: a wall cell's centre lies on the panel edge. */
static void decode_edges(AreaSrc *a) {
	int hx[4] = { 0 }, hy[4] = { 0 };
	for (int i = 0; i < a->nsec[0]; ++i) {
		const CoordCell *c = &a->sec[0][i];
		if (c->type == 1) hx[((c->x + 4) & 31) / 8]++;
		if (c->type == 4) hy[((c->y + 4) & 31) / 8]++;
	}
	int bx = 0, by = 0;
	for (int k = 1; k < 4; ++k) { if (hx[k] > hx[bx]) bx = k; if (hy[k] > hy[by]) by = k; }
	a->ex = bx * 8 + 4;
	a->ey = by * 8 + 4;
}

static int cell_of(int w) { return w >= 0 ? w / 8 : -((-w + 7) / 8); }

/* Section 1 as a grid of heights: type 0x11 raises a cell, ramps (0x13,
 * 0x14) are uneven. */
static void decode_heights(AreaSrc *a) {
	if (!a->nsec[1]) return;
	int x0 = 1 << 20, y0 = 1 << 20, x1 = -(1 << 20), y1 = -(1 << 20);
	for (int i = 0; i < a->nsec[1]; ++i) {
		int cx = cell_of(a->sec[1][i].x), cy = cell_of(a->sec[1][i].y);
		if (cx < x0) x0 = cx;
		if (cx > x1) x1 = cx;
		if (cy < y0) y0 = cy;
		if (cy > y1) y1 = cy;
	}
	a->hx0 = x0; a->hy0 = y0; a->hw = x1 - x0 + 1; a->hh = y1 - y0 + 1;
	a->hz = calloc((size_t)a->hw * a->hh, 1);
	for (int i = 0; i < a->nsec[1]; ++i) {
		const CoordCell *c = &a->sec[1][i];
		uint8_t z = c->type == 0x11 ? (uint8_t)(c->z < 0 ? 0 : c->z) : c->type == 0x13 || c->type == 0x14 ? HEIGHT_UNEVEN : 0;
		a->hz[(size_t)(cell_of(c->y) - y0) * a->hw + cell_of(c->x) - x0] = z;
	}
}

/* Rings of walls around each cell: from outside the walls' box, stepping
 * onto a wall from open cells counts one ring (a 0-1 breadth-first walk). */
static void decode_rings(AreaSrc *a) {
	if (!a->nsec[0]) return;
	int x0 = 1 << 20, y0 = 1 << 20, x1 = -(1 << 20), y1 = -(1 << 20);
	for (int i = 0; i < a->nsec[0]; ++i) {
		int cx = cell_of(a->sec[0][i].x), cy = cell_of(a->sec[0][i].y);
		if (cx < x0) x0 = cx;
		if (cx > x1) x1 = cx;
		if (cy < y0) y0 = cy;
		if (cy > y1) y1 = cy;
	}
	a->rx0 = x0 - 1; a->ry0 = y0 - 1; a->rw = x1 - x0 + 3; a->rh = y1 - y0 + 3;
	size_t n = (size_t)a->rw * a->rh;
	uint8_t *wall = calloc(n, 1);
	for (int i = 0; i < a->nsec[0]; ++i)
		wall[(size_t)(cell_of(a->sec[0][i].y) - a->ry0) * a->rw + cell_of(a->sec[0][i].x) - a->rx0] = 1;
	a->rings = malloc(n);
	memset(a->rings, 255, n);
	/* ring by ring: flood what is reachable without stepping onto a wall
	 * from open cells, and start the next ring where that happens */
	int *cur = malloc(sizeof(int) * n), *next = malloc(sizeof(int) * n), *stack = malloc(sizeof(int) * n);
	int ncur = 1, ring = 0;
	cur[0] = 0;
	a->rings[0] = 0;
	while (ncur && ring < 254) {
		int nnext = 0, top = 0;
		for (int i = 0; i < ncur; ++i) stack[top++] = cur[i];
		while (top) {
			int c = stack[--top], x = c % a->rw, y = c / a->rw;
			static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
			for (int k = 0; k < 4; ++k) {
				int nx = x + d[k][0], ny = y + d[k][1];
				if (nx < 0 || ny < 0 || nx >= a->rw || ny >= a->rh) continue;
				int nc = ny * a->rw + nx;
				if (a->rings[nc] != 255) continue;
				if (wall[nc] && !wall[c]) {
					a->rings[nc] = (uint8_t)(ring + 1);
					next[nnext++] = nc;
				} else {
					a->rings[nc] = (uint8_t)ring;
					stack[top++] = nc;
				}
			}
		}
		int *t = cur; cur = next; next = t;
		ncur = nnext;
		++ring;
	}
	free(cur);
	free(next);
	free(stack);
	/* walls themselves belong to the floor they ring */
	for (size_t i = 0; i < n; ++i) if (wall[i] && a->rings[i] != 255 && !(a->rings[i] & 1)) a->rings[i]++;
	free(wall);
}

int area_src_walled_floor(const AreaSrc *a, int X, int Y) {
	int cx = cell_of(X) - a->rx0, cy = cell_of(Y) - a->ry0;
	if (!a->rings) return -1;
	if (cx < 0 || cy < 0 || cx >= a->rw || cy >= a->rh) return 0;
	return a->rings[(size_t)cy * a->rw + cx] & 1;
}

int area_src_height(const AreaSrc *a, int X, int Y) {
	int cx = cell_of(X) - a->hx0, cy = cell_of(Y) - a->hy0;
	if (!a->hz || cx < 0 || cy < 0 || cx >= a->hw || cy >= a->hh) return 0;
	return a->hz[(size_t)cy * a->hw + cx];
}

bool area_src_load(int group, int number, AreaSrc *a) {
	memset(a, 0, sizeof *a);
	a->group = group;
	a->number = number;
	if (!decode_tiles(a)) { area_src_free(a); return false; }
	decode_coords(a);
	decode_edges(a);
	decode_heights(a);
	decode_rings(a);
	return true;
}

void area_src_free(AreaSrc *a) {
	for (int l = 0; l < 2; ++l) free(a->tile[l]);
	free(a->px);
	free(a->front);
	for (int k = 0; k < 4; ++k) free(a->sec[k]);
	free(a->hz);
	free(a->rings);
	memset(a, 0, sizeof *a);
}

void area_src_mirror(const AreaSrc *a, AreaSrc *m) {
	*m = *a;
	int W = a->tw * 8, H = a->th * 8;
	size_t cells = (size_t)a->tw * a->th;
	for (int l = 0; l < 2; ++l) {
		m->tile[l] = NULL;
		if (!a->tile[l]) continue;
		m->tile[l] = malloc(cells * 2);
		for (int ty = 0; ty < a->th; ++ty)
			for (int tx = 0; tx < a->tw; ++tx)
				m->tile[l][ty * a->tw + (a->tw - 1 - tx)] = a->tile[l][ty * a->tw + tx] ^ 0x400;
	}
	m->px = malloc((size_t)W * H * 4);
	m->front = malloc((size_t)W * H);
	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x) {
			m->px[(size_t)y * W + (W - 1 - x)] = a->px[(size_t)y * W + x];
			m->front[(size_t)y * W + (W - 1 - x)] = a->front[(size_t)y * W + x];
		}
	for (int k = 0; k < 4; ++k) { m->sec[k] = NULL; m->nsec[k] = 0; }   /* the mirror is for tiles only */
	m->ex = (32 - a->ey) & 31;
	m->ey = (32 - a->ex) & 31;
	/* heights: cell (x, y) becomes (-y - 1, -x - 1) */
	m->hz = NULL;
	if (a->hz) {
		m->hw = a->hh; m->hh = a->hw;
		m->hx0 = -(a->hy0 + a->hh); m->hy0 = -(a->hx0 + a->hw);
		m->hz = malloc((size_t)m->hw * m->hh);
		for (int y = 0; y < a->hh; ++y)
			for (int x = 0; x < a->hw; ++x)
				m->hz[(size_t)(a->hw - 1 - x) * m->hw + (a->hh - 1 - y)] = a->hz[(size_t)y * a->hw + x];
	}
	m->rings = NULL;
	if (a->rings) {
		m->rw = a->rh; m->rh = a->rw;
		m->rx0 = -(a->ry0 + a->rh); m->ry0 = -(a->rx0 + a->rw);
		m->rings = malloc((size_t)m->rw * m->rh);
		for (int y = 0; y < a->rh; ++y)
			for (int x = 0; x < a->rw; ++x)
				m->rings[(size_t)(a->rw - 1 - x) * m->rw + (a->rh - 1 - y)] = a->rings[(size_t)y * a->rw + x];
	}
}

void area_src_raise(const AreaSrc *a, int z, AreaSrc *r) {
	*r = *a;
	int W = a->tw * 8, H = a->th * 8, rows = z / 8;
	size_t cells = (size_t)a->tw * a->th;
	for (int l = 0; l < 2; ++l) {
		r->tile[l] = NULL;
		if (!a->tile[l]) continue;
		r->tile[l] = calloc(cells, 2);
		for (int ty = rows; ty < a->th; ++ty)
			memcpy(r->tile[l] + (size_t)ty * a->tw, a->tile[l] + (size_t)(ty - rows) * a->tw, (size_t)a->tw * 2);
	}
	r->px = calloc((size_t)W * H, 4);
	r->front = calloc((size_t)W * H, 1);
	for (int y = z; y < H; ++y) {
		memcpy(r->px + (size_t)y * W, a->px + (size_t)(y - z) * W, (size_t)W * 4);
		memcpy(r->front + (size_t)y * W, a->front + (size_t)(y - z) * W, (size_t)W);
	}
	for (int k = 0; k < 4; ++k) { r->sec[k] = NULL; r->nsec[k] = 0; }
	r->hz = NULL;
	if (a->hz) {
		r->hz = malloc((size_t)a->hw * a->hh);
		memcpy(r->hz, a->hz, (size_t)a->hw * a->hh);
	}
	r->rings = NULL;
	if (a->rings) {
		r->rings = malloc((size_t)a->rw * a->rh);
		memcpy(r->rings, a->rings, (size_t)a->rw * a->rh);
	}
	r->level = z;
}

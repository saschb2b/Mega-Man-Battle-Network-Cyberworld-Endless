/* Decoding an original map: MapBGDescriptor (tile sets, palette, LZ77 tile
 * map with two layers) from the tables at 0x0329A8 (real world, groups
 * 0x00-0x06) and 0x0329C4 (internet, from 0x80), and the coordinate data
 * (LZ77 wall list) from 0x033530 and 0x03354C, whose walls give where the
 * panel edges fall in world units. */
#include "area_src.h"

#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "rom.h"

#define MAP_TABLE_RW   0x0329A8u /* MapBGDescriptor lists, real-world groups 0x00-0x06 */
#define MAP_TABLE      0x0329C4u /* ... internet groups from 0x80 */
#define COORD_TABLE_RW 0x033530u /* coordinate data lists, real-world groups */
#define COORD_TABLE    0x03354Cu /* ... internet groups from 0x80 */
#define RW_GROUPS      7

/* The ROM the map being read comes from: BN6's, or an extra game's
 * (docs/MULTIROM.md), whose maps are in the same formats. */
static struct { const uint8_t *data; uint32_t map_rw, map_net, coord_rw, coord_net; int rw_groups, net_groups; } src;

static void use_rom(int xrom) {
	if (xrom >= 0 && xrom < XROM_COUNT && XR[xrom].data) {
		const XRomLayout *x = XR[xrom].layout;
		src.data = XR[xrom].data;
		src.map_rw = x->map_table_rw; src.map_net = x->map_table;
		src.coord_rw = x->coord_table_rw; src.coord_net = x->coord_table;
		src.rw_groups = x->rw_groups; src.net_groups = x->net_groups;
		return;
	}
	src.data = R.data;
	src.map_rw = MAP_TABLE_RW; src.map_net = MAP_TABLE;
	src.coord_rw = COORD_TABLE_RW; src.coord_net = COORD_TABLE;
	src.rw_groups = RW_GROUPS; src.net_groups = 0x80;   /* (BN6's own maps: no bound kept) */
}

static uint32_t su32(uint32_t off) {
	const uint8_t *d = src.data + off;
	return (uint32_t)d[0] | (uint32_t)d[1] << 8 | (uint32_t)d[2] << 16 | (uint32_t)d[3] << 24;
}
static uint16_t su16(uint32_t off) { return (uint16_t)(src.data[off] | src.data[off + 1] << 8); }

/* The group's entry in a real-world or internet table; 0 for a group the
 * ROM's tables do not hold. */
static uint32_t group_slot(uint32_t rw_table, uint32_t net_table, int group) {
	if (group < 0x80) return group < src.rw_groups ? rw_table + (uint32_t)group * 4 : 0;
	return group - 0x80 < src.net_groups ? net_table + (uint32_t)(group - 0x80) * 4 : 0;
}


/* A map's tile graphics (as the game loads them to VRAM) and colours. */
static uint8_t *map_gfx(uint32_t ts, uint32_t pal, uint32_t colors[256]) {
	uint8_t *vram = calloc(0x10000, 1);
	if (!vram) return NULL;
	for (int k = 0; k < 2; ++k) {
		uint32_t wc = su32(ts + (uint32_t)k * 12), off = su32(ts + (uint32_t)k * 12 + 4), vo = su32(ts + (uint32_t)k * 12 + 8);
		if (!wc) continue;
		size_t tn = 0;
		uint8_t *t = lz77_decompress(src.data + ts + off, ROM_SIZE - (ts + off), &tn);
		if (!t) continue;
		size_t want = (size_t)wc * 4 < tn ? (size_t)wc * 4 : tn;
		if (vo < 0x10000) memcpy(vram + vo, t, want < 0x10000 - vo ? want : 0x10000 - vo);
		free(t);
	}
	for (int i = 0; i < 256; ++i) colors[i] = bgr555(su16(pal + (uint32_t)i * 2));
	return vram;
}

/* Draws tile maps into px (tw * 8 x th * 8), layer `top` over the others,
 * marking its pixels when given. The real world's maps use 256-colour
 * tiles (64 bytes, a colour index per pixel), the internet's 16-colour
 * ones (32 bytes, a bank per map entry). */
static void draw_layers(const uint8_t *vram, const uint32_t colors[256], uint16_t *const *tile, int layers, int top, int tw, int th, uint32_t *px, uint8_t *front, uint8_t *idx, bool bpp8) {
	int W = tw * 8;
	for (int k = 0; k < layers; ++k) {
		int l = k == layers - 1 ? top : k < top ? k : k + 1;   /* the others first, then `top` */
		for (int ty = 0; ty < th; ++ty)
			for (int tx = 0; tx < tw; ++tx) {
				uint16_t e = tile[l][ty * tw + tx];
				if (!(e & 0x3FF)) continue;
				const uint8_t *t = vram + (e & 0x3FF) * (bpp8 ? 64 : 32);
				if ((e & 0x3FF) * (bpp8 ? 64 : 32) + (bpp8 ? 64 : 32) > 0x10000) continue;
				for (int y = 0; y < 8; ++y)
					for (int x = 0; x < 8; ++x) {
						int ci;
						if (bpp8) ci = t[y * 8 + x];
						else { uint8_t v = t[y * 4 + x / 2]; ci = (x & 1) ? v >> 4 : v & 15; }
						if (!ci) continue;
						int X = (e & 0x400) ? 7 - x : x, Y = (e & 0x800) ? 7 - y : y;
						px[(size_t)(ty * 8 + Y) * W + tx * 8 + X] = colors[bpp8 ? ci : (e >> 12) * 16 + ci];
						if (idx) idx[(size_t)(ty * 8 + Y) * W + tx * 8 + X] = (uint8_t)(bpp8 ? ci : (e >> 12) * 16 + ci);
						if (l == top && front) front[(size_t)(ty * 8 + Y) * W + tx * 8 + X] = 1;
					}
			}
	}
}

static bool map_desc(int group, int number, uint32_t *desc, uint32_t *ts, uint32_t *pal, uint32_t *tm) {
	uint32_t list = su32(group_slot(src.map_rw, src.map_net, group));
	if (!rom_is_ptr(list)) return false;
	*desc = rom_off(list) + (uint32_t)number * 12;
	*ts = su32(*desc); *pal = su32(*desc + 4); *tm = su32(*desc + 8);
	if (!rom_is_ptr(*ts) || !rom_is_ptr(*pal) || !rom_is_ptr(*tm)) return false;
	*ts = rom_off(*ts); *pal = rom_off(*pal) + 4; *tm = rom_off(*tm);
	return true;
}

bool area_src_slots(int group, int number, uint32_t *desc, uint32_t *coord_slot) {
	use_rom(-1);   /* (the slots a layer takes over are BN6's) */
	uint32_t ts, pal, tm, list = su32(group_slot(src.coord_rw, src.coord_net, group));
	if (!map_desc(group, number, desc, &ts, &pal, &tm) || !rom_is_ptr(list)) return false;
	*coord_slot = rom_off(list) + (uint32_t)number * 4;
	return true;
}

static bool decode_tiles(AreaSrc *a) {
	uint32_t ts, pal, tm;
	if (!map_desc(a->group, a->number, &a->desc, &ts, &pal, &tm)) return false;
	a->tw = src.data[tm]; a->th = src.data[tm + 1];
	size_t n = 0;
	uint8_t *m = lz77_decompress(src.data + tm + 12, ROM_SIZE - (tm + 12), &n);
	if (!m || a->tw <= 0 || a->th <= 0) { free(m); return false; }
	size_t cells = (size_t)a->tw * a->th;
	a->layers = (int)(n / (cells * 2));
	if (a->layers > 2) a->layers = 2;
	for (int l = 0; l < a->layers; ++l) {
		/* (a whole map's tiles, its pixels twice: a New 3DS ran short of
		 * memory in the Undernet, so none is written unchecked) */
		if (!(a->tile[l] = malloc(cells * 2))) { free(m); return false; }
		for (size_t i = 0; i < cells; ++i) a->tile[l][i] = (uint16_t)(m[(l * cells + i) * 2] | m[(l * cells + i) * 2 + 1] << 8);
	}
	free(m);
	/* draw it as the game shows it, the second layer (BG2, priority 2) in
	 * front of the first (BG1, priority 3): the real world's ground under
	 * what stands on it, the internet's floors under what the originals set
	 * in front of them (bridges, stairs, spikes, the floors that overlap
	 * others on screen); and the first layer alone */
	uint32_t colors[256];
	a->px = calloc(cells * 64, 4);
	a->px0 = calloc(cells * 64, 4);
	a->front = calloc(cells * 64, 1);
	a->idx = calloc(cells * 64, 1);
	if (!a->px || !a->px0 || !a->front || !a->idx) return false;
	uint8_t *vram = map_gfx(ts, pal, colors);
	if (!vram) return false;
	bool rw = a->group < RW_GROUPS;
	draw_layers(vram, colors, a->tile, a->layers, a->layers - 1, a->tw, a->th, a->px, a->front, a->idx, rw);
	draw_layers(vram, colors, a->tile, 1, 0, a->tw, a->th, a->px0, NULL, NULL, rw);
	free(vram);
	return true;
}

uint32_t *area_src_render(int group, int number, const uint16_t *tiles, int tw, int th) {
	use_rom(-1);
	uint32_t desc, ts, pal, tm, colors[256];
	if (!map_desc(group, number, &desc, &ts, &pal, &tm)) return NULL;
	uint8_t *vram = map_gfx(ts, pal, colors);
	uint32_t *px = calloc((size_t)tw * th * 64, 4);
	if (!vram || !px) { free(vram); free(px); return NULL; }
	/* (draw_layers only reads them) */
	union { const uint16_t *c; uint16_t *v; } t = { tiles };
	uint16_t *layers[2] = { t.v, t.v + (size_t)tw * th };
	/* as the game shows them, in the real world and the internet alike: the
	 * second layer (BG2, priority 2) over the first (BG1, priority 3) */
	draw_layers(vram, colors, layers, 2, 1, tw, th, px, NULL, NULL, group < RW_GROUPS);
	free(vram);
	return px;
}

/* The coordinate data's four sections, each a count, (key, offset)
 * entries and 4-byte shapes (see coords.c). */
static void decode_coords(AreaSrc *a) {
	uint32_t list = su32(group_slot(src.coord_rw, src.coord_net, a->group));
	if (!rom_is_ptr(list)) return;
	a->coord_slot = rom_off(list) + (uint32_t)a->number * 4;
	uint32_t c = su32(a->coord_slot);
	if (!rom_is_ptr(c)) return;
	c = rom_off(c);
	size_t n = 0;
	uint8_t *d = lz77_decompress(src.data + c + 16, ROM_SIZE - (c + 16), &n);
	if (!d) return;
	for (int s = 0; s < 4; ++s) {
		uint32_t at = su32(c + (uint32_t)s * 4);
		if (at + 4 > n) continue;
		uint32_t count = (uint32_t)(d[at] | d[at + 1] << 8 | d[at + 2] << 16 | d[at + 3] << 24);
		/* (no more than the data holds, and checked: another game's
		 * sections are read as BN6's) */
		if (count > (n - at) / 4) count = (uint32_t)((n - at) / 4);
		if (!(a->sec[s] = calloc(count + 1, sizeof(CoordCell)))) continue;
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
	for (int i = 0; a->sec[0] && i < a->nsec[0]; ++i) {
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
	if (!a->nsec[1] || !a->sec[1]) return;
	int x0 = 1 << 20, y0 = 1 << 20, x1 = -(1 << 20), y1 = -(1 << 20);
	for (int i = 0; i < a->nsec[1]; ++i) {
		int cx = cell_of(a->sec[1][i].x), cy = cell_of(a->sec[1][i].y);
		if (cx < x0) x0 = cx;
		if (cx > x1) x1 = cx;
		if (cy < y0) y0 = cy;
		if (cy > y1) y1 = cy;
	}
	a->hx0 = x0; a->hy0 = y0; a->hw = x1 - x0 + 1; a->hh = y1 - y0 + 1;
	if (a->hw <= 0 || a->hh <= 0) { a->hw = a->hh = 0; return; }
	if (!(a->hz = calloc((size_t)a->hw * a->hh, 1))) { a->hw = a->hh = 0; return; }
	for (int i = 0; i < a->nsec[1]; ++i) {
		const CoordCell *c = &a->sec[1][i];
		uint8_t z = c->type == 0x11 ? (uint8_t)(c->z < 0 ? 0 : c->z) : c->type == 0x13 || c->type == 0x14 ? HEIGHT_UNEVEN : 0;
		a->hz[(size_t)(cell_of(c->y) - y0) * a->hw + cell_of(c->x) - x0] = z;
	}
}

/* Rings of walls around each cell: from outside the walls' box, stepping
 * onto a wall from open cells counts one ring (a 0-1 breadth-first walk). */
/* The walls' cells' bounds, a cell of margin round them (false: none). */
static bool ring_bounds(AreaSrc *a) {
	int x0 = 1 << 20, y0 = 1 << 20, x1 = -(1 << 20), y1 = -(1 << 20);
	for (int i = 0; i < a->nsec[0]; ++i) {
		int cx = cell_of(a->sec[0][i].x), cy = cell_of(a->sec[0][i].y);
		if (cx < x0) x0 = cx;
		if (cx > x1) x1 = cx;
		if (cy < y0) y0 = cy;
		if (cy > y1) y1 = cy;
	}
	a->rx0 = x0 - 1; a->ry0 = y0 - 1; a->rw = x1 - x0 + 3; a->rh = y1 - y0 + 3;
	if (a->rw <= 0 || a->rh <= 0) { a->rw = a->rh = 0; return false; }
	return true;
}

/* Ring by ring: flood what is reachable without stepping onto a wall from
 * open cells, and start the next ring where that happens. */
static void flood_rings(AreaSrc *a, const uint8_t *wall, int *cur, int *next, int *stack) {
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
}

static void decode_rings(AreaSrc *a) {
	if (!a->nsec[0] || !a->sec[0] || !ring_bounds(a)) return;
	size_t n = (size_t)a->rw * a->rh;
	uint8_t *wall = calloc(n, 1);
	a->rings = malloc(n);
	int *cur = malloc(sizeof(int) * n), *next = malloc(sizeof(int) * n), *stack = malloc(sizeof(int) * n);
	if (!wall || !a->rings || !cur || !next || !stack) {
		free(wall); free(a->rings); free(cur); free(next); free(stack);
		a->rings = NULL;
		a->rw = a->rh = 0;
		return;
	}
	for (int i = 0; i < a->nsec[0]; ++i)
		wall[(size_t)(cell_of(a->sec[0][i].y) - a->ry0) * a->rw + cell_of(a->sec[0][i].x) - a->rx0] = 1;
	memset(a->rings, 255, n);
	flood_rings(a, wall, cur, next, stack);
	free(cur);
	free(next);
	free(stack);
	/* walls themselves belong to the floor they ring */
	for (size_t i = 0; i < n; ++i) if (wall[i] && a->rings[i] != 255 && !(a->rings[i] & 1)) a->rings[i]++;
	free(wall);
}

int area_src_rings(const AreaSrc *a, int X, int Y) {
	int cx = cell_of(X) - a->rx0, cy = cell_of(Y) - a->ry0;
	if (!a->rings) return -1;
	if (cx < 0 || cy < 0 || cx >= a->rw || cy >= a->rh) return 0;
	return a->rings[(size_t)cy * a->rw + cx];
}

int area_src_walled_floor(const AreaSrc *a, int X, int Y) {
	int r = area_src_rings(a, X, Y);
	return r < 0 ? -1 : r & 1;
}

int area_src_height(const AreaSrc *a, int X, int Y) {
	int cx = cell_of(X) - a->hx0, cy = cell_of(Y) - a->hy0;
	if (!a->hz || cx < 0 || cy < 0 || cx >= a->hw || cy >= a->hh) return 0;
	return a->hz[(size_t)cy * a->hw + cx];
}

static bool load(int group, int number, AreaSrc *a) {
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

bool area_src_load(int group, int number, AreaSrc *a) {
	use_rom(-1);
	return load(group, number, a);
}

bool area_src_load_x(int xrom, int group, int number, AreaSrc *a) {
	if (xrom < 0 || xrom >= XROM_COUNT || !XR[xrom].data) return false;
	use_rom(xrom);
	bool ok = load(group, number, a);
	a->rom = (int8_t)(xrom + 1);
	return ok;
}

void area_src_free(AreaSrc *a) {
	for (int l = 0; l < 2; ++l) free(a->tile[l]);
	free(a->px);
	free(a->px0);
	free(a->front);
	free(a->idx);
	for (int k = 0; k < 4; ++k) free(a->sec[k]);
	free(a->hz);
	free(a->rings);
	memset(a, 0, sizeof *a);
}

/* (each copy checked: a whole map's pixels twice, and a New 3DS ran short
 * of memory in the Undernet) */
static bool copy_parts(const AreaSrc *a, AreaSrc *c, bool zeroed) {
	int W = a->tw * 8, H = a->th * 8;
	size_t cells = (size_t)a->tw * a->th, n = (size_t)W * H;
	for (int k = 0; k < 4; ++k) { c->sec[k] = NULL; c->nsec[k] = 0; }
	c->tile[0] = c->tile[1] = NULL;
	c->hz = c->rings = NULL;
	c->px = zeroed ? calloc(n, 4) : malloc(n * 4);
	c->px0 = zeroed ? calloc(n, 4) : malloc(n * 4);
	c->front = zeroed ? calloc(n, 1) : malloc(n);
	c->idx = !a->idx ? NULL : zeroed ? calloc(n, 1) : malloc(n);
	bool ok = c->px && c->px0 && c->front && (!a->idx || c->idx);
	for (int l = 0; l < 2 && ok; ++l)
		if (a->tile[l]) ok = (c->tile[l] = calloc(cells, 2)) != NULL;
	if (a->hz && ok) ok = (c->hz = malloc((size_t)a->hw * a->hh)) != NULL;
	if (a->rings && ok) ok = (c->rings = malloc((size_t)a->rw * a->rh)) != NULL;
	if (!ok) area_src_free(c);
	return ok;
}

bool area_src_mirror(const AreaSrc *a, AreaSrc *m) {
	*m = *a;
	if (!copy_parts(a, m, false)) return false;   /* (the mirror is for tiles only: no sections) */
	int W = a->tw * 8, H = a->th * 8;
	for (int l = 0; l < 2; ++l) {
		if (!a->tile[l]) continue;
		for (int ty = 0; ty < a->th; ++ty)
			for (int tx = 0; tx < a->tw; ++tx)
				m->tile[l][ty * a->tw + (a->tw - 1 - tx)] = a->tile[l][ty * a->tw + tx] ^ 0x400;
	}
	for (int y = 0; y < H; ++y)
		for (int x = 0; x < W; ++x) {
			m->px[(size_t)y * W + (W - 1 - x)] = a->px[(size_t)y * W + x];
			m->px0[(size_t)y * W + (W - 1 - x)] = a->px0[(size_t)y * W + x];
			m->front[(size_t)y * W + (W - 1 - x)] = a->front[(size_t)y * W + x];
			if (m->idx) m->idx[(size_t)y * W + (W - 1 - x)] = a->idx[(size_t)y * W + x];
		}
	m->ex = (32 - a->ey) & 31;
	m->ey = (32 - a->ex) & 31;
	/* heights: cell (x, y) becomes (-y - 1, -x - 1) */
	if (a->hz) {
		m->hw = a->hh; m->hh = a->hw;
		m->hx0 = -(a->hy0 + a->hh); m->hy0 = -(a->hx0 + a->hw);
		for (int y = 0; y < a->hh; ++y)
			for (int x = 0; x < a->hw; ++x)
				m->hz[(size_t)(a->hw - 1 - x) * m->hw + (a->hh - 1 - y)] = a->hz[(size_t)y * a->hw + x];
	}
	if (a->rings) {
		m->rw = a->rh; m->rh = a->rw;
		m->rx0 = -(a->ry0 + a->rh); m->ry0 = -(a->rx0 + a->rw);
		for (int y = 0; y < a->rh; ++y)
			for (int x = 0; x < a->rw; ++x)
				m->rings[(size_t)(a->rw - 1 - x) * m->rw + (a->rh - 1 - y)] = a->rings[(size_t)y * a->rw + x];
	}
	return true;
}

bool area_src_raise(const AreaSrc *a, int z, AreaSrc *r) {
	*r = *a;
	if (!copy_parts(a, r, true)) return false;
	int W = a->tw * 8, H = a->th * 8, rows = z / 8;
	for (int l = 0; l < 2; ++l) {
		if (!a->tile[l]) continue;
		for (int ty = rows; ty < a->th; ++ty)
			memcpy(r->tile[l] + (size_t)ty * a->tw, a->tile[l] + (size_t)(ty - rows) * a->tw, (size_t)a->tw * 2);
	}
	for (int y = z; y < H; ++y) {
		memcpy(r->px + (size_t)y * W, a->px + (size_t)(y - z) * W, (size_t)W * 4);
		memcpy(r->px0 + (size_t)y * W, a->px0 + (size_t)(y - z) * W, (size_t)W * 4);
		memcpy(r->front + (size_t)y * W, a->front + (size_t)(y - z) * W, (size_t)W);
		if (r->idx) memcpy(r->idx + (size_t)y * W, a->idx + (size_t)(y - z) * W, (size_t)W);
	}
	if (a->hz) memcpy(r->hz, a->hz, (size_t)a->hw * a->hh);
	if (a->rings) memcpy(r->rings, a->rings, (size_t)a->rw * a->rh);
	r->level = z;
	return true;
}

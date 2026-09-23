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

/* The most common edge position (mod 32) of the NE (type 1) and NW (type 4)
 * walls: a wall cell's centre lies on the panel edge. */
static void decode_edges(AreaSrc *a) {
	a->ex = 4; a->ey = 4;
	uint32_t list = rom_u32(COORD_TABLE + (uint32_t)(a->group - 0x80) * 4);
	if (!rom_is_ptr(list)) return;
	a->coord_slot = rom_off(list) + (uint32_t)a->number * 4;
	uint32_t c = rom_u32(a->coord_slot);
	if (!rom_is_ptr(c)) return;
	c = rom_off(c);
	size_t n = 0;
	uint8_t *d = lz77_decompress(R.data + c + 16, ROM_SIZE - (c + 16), &n);
	if (!d || n < 4) { free(d); return; }
	uint32_t count = (uint32_t)(d[0] | d[1] << 8 | d[2] << 16 | d[3] << 24);
	int hx[4] = { 0 }, hy[4] = { 0 };
	for (uint32_t i = 0; i < count && 4 + i * 4 + 4 <= n; ++i) {
		int key = d[4 + i * 4] | d[5 + i * 4] << 8, off = d[6 + i * 4] | d[7 + i * 4] << 8;
		if ((size_t)(4 + off + 4) > n) continue;
		int type = d[4 + off + 3];
		int x = key % 254 - 127, y = key / 254 - 127;
		if (type == 1) hx[((x * 8 + 4) & 31) / 8]++;
		if (type == 4) hy[((y * 8 + 4) & 31) / 8]++;
	}
	free(d);
	int bx = 0, by = 0;
	for (int k = 1; k < 4; ++k) { if (hx[k] > hx[bx]) bx = k; if (hy[k] > hy[by]) by = k; }
	a->ex = bx * 8 + 4;
	a->ey = by * 8 + 4;
}

bool area_src_load(int group, int number, AreaSrc *a) {
	memset(a, 0, sizeof *a);
	a->group = group;
	a->number = number;
	if (!decode_tiles(a)) { area_src_free(a); return false; }
	decode_edges(a);
	return true;
}

void area_src_free(AreaSrc *a) {
	for (int l = 0; l < 2; ++l) free(a->tile[l]);
	free(a->px);
	free(a->front);
	memset(a, 0, sizeof *a);
}

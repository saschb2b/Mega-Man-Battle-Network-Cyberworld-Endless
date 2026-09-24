/* Battle backgrounds from the ROM (docs/ROM_DATA.md): the game's own table
 * of BGAnimData records (tiles, map, palette) and of the tile and palette
 * animation scripts that bring each to life. */
#include "backdrop.h"

#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "rom.h"

#define PAL_RAM 0x03001960u   /* the game's BG palette buffer */
#define VRAM 0x06000000u

/* Scroll per background, as the game's scroll callbacks (listed per id
 * before the BGAnimData table) move the BG offsets, in 1/16 pixel a frame:
 * diagonal (-8, -4), up (0, -4), down (0, 4), fast left (-8, 0) and slow
 * right (1, 0); the blank, the Undernet's and the Underground's stand still. */
static const int8_t scroll[BACKDROP_COUNT][2] = {
	{ -8, -4 }, { -8, -4 }, { 0, 0 }, { -8, -4 }, { -8, -4 }, { -8, -4 }, { 0, -4 }, { -8, -4 },
	{ -8, -4 }, { -8, -4 }, { -8, 0 }, { -8, -4 }, { -8, 0 }, { -8, -4 }, { -8, -4 }, { 0, 0 },
	{ -8, -4 }, { 0, 0 }, { 0, 4 }, { -8, -4 }, { 1, 0 }, { -8, -4 },
};

static bool load_anims(Backdrop *b, int id) {
	b->nanim = 0;
	uint32_t list = rom_u32(R.layout->battle_bg_anims + (uint32_t)id * 4);
	if (!rom_is_ptr(list)) return true;
	for (uint32_t a = rom_off(list); b->nanim < 4 && a + 4 <= ROM_SIZE && rom_u32(a) != 0xFFFFFFFFu; a += 4) {
		if (!rom_is_ptr(rom_u32(a))) return false;
		uint32_t d = rom_off(rom_u32(a));
		BackdropAnim *an = &b->anim[b->nanim];
		memset(an, 0, sizeof *an);
		an->p0 = rom_u32(d);
		an->p1 = rom_u32(d + 4);
		an->cmd = R.data[d + 8];
		an->count = R.data[d + 10];
		an->key = -1;
		if (an->cmd != 0 && an->cmd != 4) continue;
		/* (parameter, delay) pairs; 0 ends, 1 loops, 2 jumps (taken as a loop) */
		for (uint32_t q = d + 12; an->nsteps < 16 && q + 8 <= ROM_SIZE; q += 8) {
			uint32_t v = rom_u32(q);
			if (v <= 2) { an->loop = v != 0; break; }
			if (!rom_is_ptr(v)) return false;
			an->step[an->nsteps] = rom_off(v);
			an->delay[an->nsteps] = (int)rom_u32(q + 4);
			an->total += an->delay[an->nsteps++];
		}
		if (an->nsteps && an->total > 0) b->nanim++;
	}
	return true;
}

bool backdrop_load(Backdrop *b, int id) {
	b->id = -1;
	if (id < 0 || id >= BACKDROP_COUNT) return false;
	uint32_t rec = rom_u32(R.layout->battle_bgs + (uint32_t)id * 4);
	if (!rom_is_ptr(rec)) return false;
	rec = rom_off(rec);
	uint32_t gfx = rom_u32(rec), dest = rom_u32(rec + 4), map = rom_u32(rec + 8);
	uint32_t pal = rom_u32(rec + 16), pal_dest = rom_u32(rec + 20), pal_size = rom_u32(rec + 24);
	if (!rom_is_ptr(gfx) || !rom_is_ptr(map) || dest < VRAM || dest >= VRAM + sizeof b->vram) return false;
	memset(b->vram, 0, sizeof b->vram);
	memset(b->pal, 0, sizeof b->pal);

	/* tiles: a word count and the offset of their LZ77 data */
	gfx = rom_off(gfx);
	size_t n = 0;
	uint8_t *t = lz77_decompress(R.data + gfx + rom_u32(gfx + 4), ROM_SIZE - gfx - rom_u32(gfx + 4), &n);
	if (!t) return false;
	dest -= VRAM;
	memcpy(b->vram + dest, t, n < sizeof b->vram - dest ? n : sizeof b->vram - dest);
	free(t);

	/* map: width and height bytes, then LZ77 at +12 */
	map = rom_off(map);
	if (R.data[map] != 32 || R.data[map + 1] != 32) return false;
	uint8_t *m = lz77_decompress(R.data + map + 12, ROM_SIZE - map - 12, &n);
	if (!m) return false;
	if (n < sizeof b->map) { free(m); return false; }
	for (int i = 0; i < 32 * 32; ++i) b->map[i] = (uint16_t)(m[i * 2] | m[i * 2 + 1] << 8);
	free(m);

	/* palette: a size word, then colours (some take theirs from an animation) */
	if (rom_is_ptr(pal) && pal_dest >= PAL_RAM && pal_dest - PAL_RAM + pal_size <= sizeof b->pal)
		memcpy(b->pal + (pal_dest - PAL_RAM), R.data + rom_off(pal) + 4, pal_size);
	if (!load_anims(b, id)) return false;
	b->dx = scroll[id][0];
	b->dy = scroll[id][1];
	b->id = id;
	return true;
}

static int anim_step(const BackdropAnim *an, int frame) {
	int t = an->loop ? frame % an->total : (frame < an->total ? frame : an->total - 1);
	for (int i = 0; i < an->nsteps; ++i) {
		if (t < an->delay[i]) return i;
		t -= an->delay[i];
	}
	return an->nsteps - 1;
}

static void anim_apply(Backdrop *b, BackdropAnim *an, int st) {
	uint32_t src = an->step[st];
	if (an->cmd == 0) {
		/* palette copy: p0 destination, p1 size */
		if (an->p0 >= PAL_RAM && an->p0 - PAL_RAM + an->p1 <= sizeof b->pal && src + an->p1 <= ROM_SIZE)
			memcpy(b->pal + (an->p0 - PAL_RAM), R.data + src, an->p1);
		return;
	}
	/* tile copy: `count` references (tile of p0, flips in bits 10-11) to VRAM p1 on */
	if (!rom_is_ptr(an->p0) || an->p1 < VRAM) return;
	uint32_t base = rom_off(an->p0), dest = an->p1 - VRAM;
	for (int k = 0; k < an->count; ++k) {
		uint16_t e = rom_u16(src + (uint32_t)k * 2);
		uint32_t from = base + (uint32_t)(e & 0x3FF) * 32, to = dest + (uint32_t)k * 32;
		if (from + 32 > ROM_SIZE || to + 32 > sizeof b->vram) return;
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x) {
				int sx = (e & 0x400) ? 7 - x : x, sy = (e & 0x800) ? 7 - y : y;
				uint8_t v = R.data[from + sy * 4 + sx / 2];
				v = (sx & 1) ? v >> 4 : v & 15;
				uint8_t *d = &b->vram[to + y * 4 + x / 2];
				*d = (x & 1) ? (uint8_t)((*d & 0x0F) | v << 4) : (uint8_t)((*d & 0xF0) | v);
			}
	}
}

static int wrap(int v) { return v & 255; }

void backdrop_draw(Backdrop *b, int frame, int ox, int oy, const uint8_t *dim, int mosaic, uint32_t *px) {
	if (b->id < 0) {
		memset(px, 0, 240 * 160 * sizeof *px);
		return;
	}
	for (int i = 0; i < b->nanim; ++i) {
		int st = anim_step(&b->anim[i], frame);
		if (st != b->anim[i].key) {
			anim_apply(b, &b->anim[i], st);   /* each step rewrites the same colours or tiles */
			b->anim[i].key = st;
		}
	}
	/* the BG offsets: counters stepped every frame, shown in whole pixels */
	int hofs = (int)((int64_t)frame * b->dx >> 4) + ox, vofs = (int)((int64_t)frame * b->dy >> 4) + oy;
	if (mosaic < 1) mosaic = 1;
	uint32_t col[16 * 16];
	int dim_at = -1;
	for (int y = 0; y < 160; ++y) {
		int d = dim ? dim[y] : 0;
		if (d != dim_at) {
			for (int i = 0; i < 256; ++i) {
				uint16_t c = (uint16_t)(b->pal[i * 2] | b->pal[i * 2 + 1] << 8);
				int r = (c & 31) - d, g = ((c >> 5) & 31) - d, bl = ((c >> 10) & 31) - d;
				col[i] = bgr555((uint16_t)((r < 0 ? 0 : r) | (g < 0 ? 0 : g) << 5 | (bl < 0 ? 0 : bl) << 10));
			}
			dim_at = d;
		}
		int by = wrap(y - y % mosaic + vofs);
		for (int x = 0; x < 240; ++x) {
			int bx = wrap(x - x % mosaic + hofs);
			uint16_t e = b->map[(by >> 3) * 32 + (bx >> 3)];
			int tx = bx & 7, ty = by & 7;
			if (e & 0x400) tx = 7 - tx;
			if (e & 0x800) ty = 7 - ty;
			uint8_t v = b->vram[(e & 0x3FF) * 32 + ty * 4 + tx / 2];
			v = (tx & 1) ? v >> 4 : v & 15;
			px[y * 240 + x] = col[(e >> 12) * 16 + v];
		}
	}
}

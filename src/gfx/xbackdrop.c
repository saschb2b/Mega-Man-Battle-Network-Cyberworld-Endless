/* Another game's battle backgrounds: BN5 keeps them as BN6 does (bn6f
 * sub_8080DA0, docs/ROM_DATA.md), a BGAnimData record (tiles, their VRAM,
 * map, its offset, palette, its RAM, its size), a list of GFXAnim scripts
 * and an entry of scroll callbacks per number. Tiles and map are LZ77,
 * decoded and encoded again as literal blocks; a palette is a size word,
 * then colours; a tile animation's frames are lists of tile numbers (flips
 * in bits 10-11) in a sheet of tiles, a palette animation's frames
 * colours. Callbacks are code: an entry is BN6's own whose callbacks do
 * the same. */
#include "xbackdrop.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"
#include "debug.h"
#include "emu.h"
#include "lz.h"
#include "rom.h"

#define XBG_AT   (EMU_FREE + 0x2A0000)   /* (docs/EMULATION.md) */
#define XBG_END  (EMU_FREE + 0x2E0000)
#define BUS      0x08000000u
#define BN6_BGS  22                      /* BN6's own, 0x00-0x15 */
#define XBGS_MAX 8                       /* room after them in the tables' copies */
#define SCRIPTS_MAX 4
#define FRAMES_MAX  64
#define SHAPE_MAX   64
/* The staging buffer a tile animation names (bn6f off_8001AB8, the same
 * address in Gregar): BN6's backgrounds stage theirs at 0x0200DF40, buffer
 * 0, 50 tiles at most; buffer 3, 0x0200E640, holds 72. Another game's
 * numbers name other buffers there. */
#define STAGE_SMALL 0
#define STAGE_BIG   3
#define STAGE_SMALL_TILES 50
#define STAGE_BIG_TILES   72

static struct {
	bool ready;
	uint32_t next, tables[3];   /* the copies: BGAnimData pointers, animation lists, scroll entries */
	int n;
	struct { int rom, bg, id; } bg[XBGS_MAX];
} X;

static const uint8_t *xr;   /* the ROM a background comes from */

/* `len` bytes into the free space, at a word; 0 where it is full. */
static uint32_t put(const void *src, uint32_t len) {
	if (!len || X.next + len > XBG_END) return 0;
	uint32_t at = X.next;
	emu_write(at, src, len);
	X.next = (at + len + 3) & ~3u;
	return at;
}

static bool xptr(uint32_t v, uint32_t len) { return v >= BUS && v - BUS <= ROM_SIZE - len; }

/* The LZ77 data at `lz` (a bus address in the other ROM) after a header
 * of `nhead` bytes: decoded, encoded again (where it ends is only known by
 * decoding it); the header's address, 0 where it cannot be. */
static uint32_t copy_lz(uint32_t lz, const uint8_t *head, uint32_t nhead) {
	if (!xptr(lz, 4)) return 0;
	size_t n = 0;
	uint8_t *raw = lz77_decompress(xr + (lz - BUS), ROM_SIZE - (lz - BUS), &n), *out = raw ? malloc(nhead + 4 + n + n / 8 + 8) : NULL;
	uint32_t at = 0;
	if (out) {
		memcpy(out, head, nhead);
		at = put(out, nhead + (uint32_t)lz_literal(raw, n, out + nhead));
	}
	free(out);
	free(raw);
	return at;
}

/* A script's frames: how many come before its end, which it returns (0
 * ends, 1 loops, 2 jumps; 3 where it cannot be read), and how many tiles
 * of its sheet a tile copy's lists reach. */
static uint32_t script_frames(uint32_t s, uint8_t cmd, uint8_t count, uint32_t *nf, uint32_t *tiles) {
	const uint8_t *h = xr + (s - BUS);
	*nf = *tiles = 0;
	for (; *nf < FRAMES_MAX && xptr(s + 12 + 8 * *nf, 8); ++*nf) {
		uint32_t v = get32(h + 12 + 8 * *nf);
		if (v <= 2) return v;
		if (cmd == 0) continue;
		if (!xptr(v, 2u * count)) return 3;
		for (int k = 0; k < count; ++k) {
			uint32_t t = (get16(xr + (v - BUS) + 2 * k) & 0x3FFu) + 1;
			if (t > *tiles) *tiles = t;
		}
	}
	return 3;
}

/* A script's frames (`len` bytes each) copied, each once (most scripts
 * repeat theirs), their pointers in `out` moved to the copies. */
static bool copy_frames(uint8_t *out, const uint8_t *h, uint32_t nf, uint32_t len) {
	static uint32_t seen[FRAMES_MAX][2];
	int nseen = 0;
	for (uint32_t i = 0; i < nf; ++i) {
		uint32_t from = get32(h + 12 + 8 * i), to = 0;
		for (int k = 0; k < nseen && !to; ++k)
			if (seen[k][0] == from) to = seen[k][1];
		if (!to) {
			to = xptr(from, len) ? put(xr + (from - BUS), len) : 0;
			if (!to) return false;
			seen[nseen][0] = from;
			seen[nseen++][1] = to;
		}
		put32(out + 12 + 8 * i, to);
	}
	return true;
}

/* An animation script (header; (parameter, delay) pairs ending 0, 1 or 2
 * and an address) copied with what its frames point at: a tile copy's
 * lists and the tiles they number, staged where BN6 has room for them, a
 * palette copy's colours, its palette RAM moved by `pal_move`; 0 for
 * another command or what cannot be read. */
static uint32_t copy_script(uint32_t s, uint32_t pal_move) {
	if (!xptr(s, 12)) return 0;
	const uint8_t *h = xr + (s - BUS);
	uint8_t cmd = h[8], count = h[10];
	if ((cmd != 0 && cmd != 4 && cmd != 0x18) || (cmd != 0 && (!count || count > STAGE_BIG_TILES))) return 0;
	uint32_t nf, tiles, term = script_frames(s, cmd, count, &nf, &tiles), unit = cmd == 0x18 ? 64 : 32;
	if (!nf || term > 2) return 0;
	uint32_t len = 12 + 8 * nf + (term == 2 ? 8 : 4);
	static uint8_t out[12 + 8 * FRAMES_MAX + 8];
	memcpy(out, h, len);
	if (cmd == 0) put32(out, get32(h) + pal_move);
	else {
		uint32_t sheet = xptr(get32(h), tiles * unit) ? put(xr + (get32(h) - BUS), tiles * unit) : 0;
		if (!sheet) return 0;
		put32(out, sheet);
		out[11] = count > STAGE_SMALL_TILES ? STAGE_BIG : STAGE_SMALL;
	}
	if (!copy_frames(out, h, nf, cmd == 0 ? get32(h + 4) : 2u * count)) return 0;
	/* (a jump stays inside its script, which lands at the free space's end) */
	if (term == 2) {
		uint32_t jump = get32(h + 16 + 8 * nf);
		if (jump < s + 12 || jump >= s + 12 + 8 * nf) return 0;
		put32(out + 16 + 8 * nf, X.next + (jump - s));
	}
	return put(out, len);
}

/* A list of animation scripts copied, ending 0xFFFFFFFF as the game's
 * do; the scripts that cannot be left out. */
static uint32_t copy_anims(uint32_t list, uint32_t pal_move) {
	uint8_t out[4 * (SCRIPTS_MAX + 1)];
	int n = 0;
	for (uint32_t a = list; xptr(a, 4) && get32(xr + (a - BUS)) != 0xFFFFFFFFu && n < SCRIPTS_MAX; a += 4) {
		uint32_t s = copy_script(get32(xr + (a - BUS)), pal_move);
		if (s) put32(out + 4 * n++, s);
	}
	put32(out + 4 * n++, 0xFFFFFFFFu);
	return put(out, 4u * (uint32_t)n);
}

/* A background's BGAnimData record copied with its tiles, map and
 * palette, the map's offset and the palette's RAM as BN6's own records
 * (`own`) name them; 0 where it cannot be. */
static uint32_t copy_record(uint32_t rec, const uint8_t *own) {
	if (!xptr(rec, 28)) return 0;
	uint8_t out[28];
	memcpy(out, xr + (rec - BUS), sizeof out);
	memcpy(out + 12, own + 12, 4);
	memcpy(out + 20, own + 20, 4);
	uint32_t gfx = get32(out), map = get32(out + 8), pal = get32(out + 16), size = get32(out + 24);
	if (gfx) {
		/* (tiles: a word count and the offset of their LZ77; the map: its
		 * width and height, LZ77 at +12) */
		if (!xptr(gfx, 8) || !xptr(map, 12)) return 0;
		uint8_t head[8];
		put32(head, get32(xr + (gfx - BUS)));
		put32(head + 4, sizeof head);
		uint32_t g = copy_lz(gfx + get32(xr + (gfx - BUS) + 4), head, sizeof head), m = g ? copy_lz(map + 12, xr + (map - BUS), 12) : 0;
		if (!m) return 0;
		put32(out, g);
		put32(out + 8, m);
	}
	if (pal) {
		uint32_t p = size <= 0x200 && xptr(pal, 4 + size) ? put(xr + (pal - BUS), 4 + size) : 0;
		if (!p) return 0;
		put32(out + 16, p);
	}
	return put(out, sizeof out);
}

/* Thumb code from `addr` in `rom` to its first return, its BL targets and
 * PC-relative loads' offsets left out: what two games' copies of a routine
 * share. Its length in halfwords, 0 where it is not code in the ROM. */
static int code_shape(const uint8_t *rom, uint32_t addr, uint16_t *out) {
	if (!(addr & 1) || !xptr(addr & ~1u, 2)) return 0;
	uint32_t o = (addr & ~1u) - BUS;
	for (int n = 0; n < SHAPE_MAX && o + 2 * (uint32_t)n + 2 <= ROM_SIZE; ++n) {
		uint16_t hw = get16(rom + o + 2 * n);
		if ((hw & 0xF800) == 0x4800) hw &= 0xFF00;            /* ldr rN, [pc, #...] */
		else if ((hw & 0xF000) == 0xF000) hw &= 0xF800;       /* a BL's halves */
		out[n] = hw;
		if (hw == 0x4770 || hw == 0x46F7 || (hw & 0xFF00) == 0xBD00) return n + 1;   /* bx lr, mov pc lr, pop {..pc} */
	}
	return 0;
}

static bool same_code(const uint8_t *a_rom, uint32_t a, const uint8_t *b_rom, uint32_t b) {
	uint16_t sa[SHAPE_MAX], sb[SHAPE_MAX];
	int na = code_shape(a_rom, a, sa), nb = code_shape(b_rom, b, sb);
	return na && na == nb && !memcmp(sa, sb, (size_t)na * 2);
}

/* BN6's scroll entry whose map and BG callbacks do what the other game's
 * entry `e` does (its hblank callback is IWRAM code, BN6's own nothing for
 * every background); BN6's still one, background 2's, where none does. */
static int scroll_like(const uint8_t *e) {
	for (int i = 0; i < BN6_BGS; ++i) {
		const uint8_t *own = R.data + R.layout->battle_bg_scroll + 16 * (uint32_t)i;
		if (same_code(xr, get32(e), R.data, get32(own)) && same_code(xr, get32(e + 4), R.data, get32(own + 4)) && get32(e + 12) == get32(own + 12)) return i;
	}
	return 2;
}

/* Every background the other games' areas bring, then BN6's tables with
 * room for them, then the game's loader pointed at those. */
static void copy_all(void) {
	static uint8_t bgs[4 * (BN6_BGS + XBGS_MAX)], anims[4 * (BN6_BGS + XBGS_MAX)], scroll[16 * (BN6_BGS + XBGS_MAX)];
	memcpy(bgs, R.data + R.layout->battle_bgs, 4 * BN6_BGS);
	memcpy(anims, R.data + R.layout->battle_bg_anims, 4 * BN6_BGS);
	memcpy(scroll, R.data + R.layout->battle_bg_scroll, 16 * BN6_BGS);
	const uint8_t *own = R.data + (get32(bgs) - BUS);   /* (BN6's first record: the map offset and palette RAM its records name) */
	for (int k = 0; k < XAREAS_MAX && X.n < XBGS_MAX; ++k) {
		const NetAreaDef *a = net_area_def(NET_AREAS + k);
		int xi = a ? a->xrom - 1 : -1;
		if (xi < 0 || xi >= XROM_COUNT || !a->xbg || !XR[xi].data || !XR[xi].layout->battle_bgs) continue;
		bool have = false;
		for (int i = 0; i < X.n; ++i) have |= X.bg[i].rom == xi && X.bg[i].bg == a->xbg;
		if (have) continue;
		xr = XR[xi].data;
		const XRomLayout *l = XR[xi].layout;
		uint32_t rec = get32(xr + l->battle_bgs + 4u * a->xbg), list = get32(xr + l->battle_bg_anims + 4u * a->xbg);
		uint32_t to = copy_record(rec, own), an = to ? copy_anims(list, get32(own + 20) - get32(xr + (rec - BUS) + 20)) : 0;
		if (!an) continue;
		int id = BN6_BGS + X.n;
		put32(bgs + 4 * id, to);
		put32(anims + 4 * id, an);
		memcpy(scroll + 16 * id, R.data + R.layout->battle_bg_scroll + 16 * (uint32_t)scroll_like(xr + l->battle_bg_scroll + 16u * a->xbg), 16);
		X.bg[X.n++] = (__typeof__(X.bg[0])){ xi, a->xbg, id };
		if (emu_debug_on()) fprintf(stderr, "xbackdrop: background %d of %s as %#x, %u bytes so far\n", a->xbg, l->name, id, X.next - XBG_AT);
	}
	if (!X.n) return;
	uint32_t n = BN6_BGS + (uint32_t)X.n;
	X.tables[0] = put(bgs, 4 * n);
	X.tables[1] = put(anims, 4 * n);
	X.tables[2] = put(scroll, 16 * n);
	if (!X.tables[0] || !X.tables[1] || !X.tables[2]) { X.n = 0; return; }
	for (int t = 0; t < 3; ++t) emu_write32(BUS + R.layout->battle_bg_refs[t], X.tables[t]);
}

int xbackdrop_install(int xrom, int bg, int fallback) {
	if (!R.data || !R.layout || !R.layout->battle_bg_scroll) return fallback;
	/* (the core's ROM copy made anew: the backgrounds went with the old) */
	if (X.n && emu_read32(BUS + R.layout->battle_bg_refs[0]) != X.tables[0]) memset(&X, 0, sizeof X);
	if (!X.ready) {
		X.ready = true;
		X.next = XBG_AT;
		copy_all();
	}
	for (int i = 0; i < X.n; ++i)
		if (X.bg[i].rom == xrom && X.bg[i].bg == bg) return X.bg[i].id;
	return fallback;
}

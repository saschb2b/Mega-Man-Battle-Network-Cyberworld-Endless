/* Portraits for Falzar's Navis (portrait.h). Gregar's mugshot numbers for
 * them point at an empty black face, but their battle sprites are there
 * (sprite list 0, 0x2E + the navi, uncompressed). A portrait is a 40 x 48
 * window of the battle sprite's standing frame (animation 0, its first
 * frame), laid into a copy of the HeelNavi's mugshot (one frame, four
 * objects over 30 tiles, one palette), on a ground of the guardian's
 * title-card colour, written into free space and listed at a mugshot
 * number no face takes in Gregar. Numbers only in the code: the pixels are
 * the player's ROM's (docs/ROM_DATA.md, Mugshots). */
#include "portrait.h"

#include <limits.h>
#include <stdbool.h>
#include <string.h>

#include "bytes.h"
#include "emu.h"
#include "guardians.h"
#include "rom.h"
#include "xnavi.h"

#define PORTRAIT_AT   (EMU_FREE + 0x166000)   /* (docs/EMULATION.md) */
#define PORTRAIT_SIZE 0x500
#define BUS           0x08000000u
#define LIST_BATTLE   0
#define LIST_FACES    8
#define TEMPLATE_FACE 0x43   /* the HeelNavi's mugshot */
#define EMPTY_FACE    0x07   /* a number Gregar points at its black face */
#define FACE_W        40
#define FACE_H        48

/* Per Falzar Navi (6 SpoutMan .. 10 DustMan): the window's top left from
 * his feet, chosen by eye around his head (SpoutMan whole, the others to
 * the chest), and the number his portrait takes: empty in Gregar, past the
 * townsfolk's faces (0x00-0x1F, their list-5 sprite less 0x20), and not
 * one the chat box gives a palette bank of its own (0x29, 0x31 and others:
 * TomahawkMan's at 0x31 drew in his palette's second bank, which it lacks;
 * docs/ROM_DATA.md, Mugshots). */
static const struct { int8_t wx, wy; uint8_t face; } P[5] = {
	{ -20, -40, 0x30 }, { -12, -50, 0x28 }, { -18, -78, 0x34 }, { -20, -62, 0x35 }, { -8, -56, 0x36 },
};

static const uint8_t dims[3][4][2] = {
	{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
	{ { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
	{ { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
};

/* The ROM offset of entry i of sprite list `list` */
static uint32_t list_entry(int list, int i) {
	return get32(R.data + R.layout->sprite_lists + (uint32_t)list * 4) - BUS + (uint32_t)i * 4;
}

/* An uncompressed sprite of the player's ROM: its header, the bytes
 * readable from it; NULL for a compressed or a bad pointer. */
static const uint8_t *sprite_at(int list, int i, uint32_t *n) {
	uint32_t p = get32(R.data + list_entry(list, i));
	if (p & 0x80000000u || p < BUS || p - BUS + 8 > ROM_SIZE) return NULL;
	*n = ROM_SIZE - (p - BUS);
	return R.data + (p - BUS);
}

/* The first frame of animation 0 of a sprite (its header `h`, `n` bytes
 * from it): the offsets from h of its tiles, its palette (each after its
 * length) and its object list, and the tiles' length; false past n. */
typedef struct { uint32_t tiles, tiles_len, pal, pal_len, obj; } Frame;

static bool first_frame(const uint8_t *h, uint32_t n, Frame *fr) {
	const uint8_t *b = h + 4;
	uint32_t m = n - 4, f = get32(b);
	if (f + 20 > m) return false;
	uint32_t t = get32(b + f), pl = get32(b + f + 4), mini = get32(b + f + 8), tab = get32(b + f + 12);
	if (t + 4 > m || pl + 4 > m || mini + 4 > m || tab + 4 > m) return false;
	uint32_t entry = mini + get32(b + mini);
	if (entry >= m) return false;
	uint32_t list = b[entry];
	if (tab + 4 * list + 4 > m) return false;
	fr->tiles = 4 + t + 4;
	fr->tiles_len = get32(b + t);
	fr->pal = 4 + pl + 4;
	fr->pal_len = get32(b + pl);
	fr->obj = 4 + tab + get32(b + tab + 4 * list);
	return fr->tiles + fr->tiles_len <= n && fr->pal + fr->pal_len <= n && fr->obj + 2 <= n;
}

/* The frame's objects into `px` (palette indices), the window's top left
 * at (wx, wy) from the sprite's origin; flips as each object says. */
static void draw_frame(uint8_t px[FACE_H][FACE_W], const uint8_t *h, uint32_t n, const Frame *fr, int wx, int wy) {
	const uint8_t *tiles = h + fr->tiles;
	int k = 0;
	for (uint32_t o = fr->obj; o + 5 <= n && !(h[o] == 0xFF && h[o + 1] == 0xFF) && k < 128; o += 5, ++k) {
		int shape = h[o + 4] & 3, size = h[o + 3] & 3;
		if (shape > 2) continue;
		int w = dims[shape][size][0], ht = dims[shape][size][1], tw = w / 8;
		bool hf = h[o + 3] & 0x40, vf = h[o + 3] & 0x80;
		for (int y = 0; y < ht; ++y)
			for (int x = 0; x < w; ++x) {
				int sx = hf ? w - 1 - x : x, sy = vf ? ht - 1 - y : y;
				uint32_t t = h[o] + (uint32_t)((sy / 8) * tw + sx / 8);
				if ((t + 1) * 32 > fr->tiles_len) continue;
				uint8_t v = tiles[t * 32 + (uint32_t)((sy % 8) * 4 + (sx % 8) / 2)];
				int ci = sx & 1 ? v >> 4 : v & 15;
				int X = (int8_t)h[o + 1] + x - wx, Y = (int8_t)h[o + 2] + y - wy;
				if (ci && X >= 0 && Y >= 0 && X < FACE_W && Y < FACE_H) px[Y][X] = (uint8_t)ci;
			}
	}
}

static long colour_gap(uint16_t a, uint16_t b) {
	long d = 0;
	for (int s = 0; s < 15; s += 5) {
		long c = (long)(a >> s & 31) - (long)(b >> s & 31);
		d += c * c;
	}
	return d;
}

/* A palette index for the ground: one the window leaves unused, else the
 * one it uses least, its few pixels given the nearest other colour. */
static int ground_index(uint8_t px[FACE_H][FACE_W], const uint16_t pal[16]) {
	int count[16] = { 0 };
	for (int y = 0; y < FACE_H; ++y)
		for (int x = 0; x < FACE_W; ++x) ++count[px[y][x]];
	int g = 1;
	for (int i = 2; i < 16; ++i)
		if (count[i] < count[g]) g = i;
	if (!count[g]) return g;
	int near = g == 1 ? 2 : 1;
	long best = LONG_MAX;
	for (int i = 1; i < 16; ++i)
		if (i != g && count[i] && colour_gap(pal[i], pal[g]) < best) { best = colour_gap(pal[i], pal[g]); near = i; }
	for (int y = 0; y < FACE_H; ++y)
		for (int x = 0; x < FACE_W; ++x)
			if (px[y][x] == g) px[y][x] = (uint8_t)near;
	return g;
}

/* `px` into the template's tiles, object by object as its list lays them
 * (the mugshot's origin at the window's middle) */
static void put_tiles(uint8_t *s, uint32_t n, const Frame *fr, uint8_t px[FACE_H][FACE_W]) {
	uint8_t *tiles = s + fr->tiles;
	int k = 0;
	for (uint32_t o = fr->obj; o + 5 <= n && !(s[o] == 0xFF && s[o + 1] == 0xFF) && k < 16; o += 5, ++k) {
		int shape = s[o + 4] & 3, size = s[o + 3] & 3;
		if (shape > 2) continue;
		int w = dims[shape][size][0], ht = dims[shape][size][1], tw = w / 8;
		int ox = (int8_t)s[o + 1] + FACE_W / 2, oy = (int8_t)s[o + 2] + FACE_H / 2;
		for (int y = 0; y < ht; ++y)
			for (int x = 0; x < w; ++x) {
				uint32_t t = s[o] + (uint32_t)((y / 8) * tw + x / 8);
				if ((t + 1) * 32 > fr->tiles_len) continue;
				int X = ox + x, Y = oy + y;
				uint8_t ci = X >= 0 && Y >= 0 && X < FACE_W && Y < FACE_H ? px[Y][X] : 0;
				uint8_t *v = tiles + t * 32 + (uint32_t)((y % 8) * 4 + (x % 8) / 2);
				*v = x & 1 ? (uint8_t)((*v & 0x0F) | ci << 4) : (uint8_t)((*v & 0xF0) | ci);
			}
	}
}

int portrait_face(int navi) {
	if (navi < 6 || navi > 10 || !R.data || !R.layout || !R.layout->sprite_lists) return -1;
	int k = navi - 6;
	/* (the number still black in the player's ROM, as Gregar has it) */
	if (get32(R.data + list_entry(LIST_FACES, P[k].face)) != get32(R.data + list_entry(LIST_FACES, EMPTY_FACE))) return -1;
	uint32_t tn, bn;
	const uint8_t *th = sprite_at(LIST_FACES, TEMPLATE_FACE, &tn), *bh = sprite_at(LIST_BATTLE, 0x2E + navi, &bn);
	if (!th || !bh) return -1;
	uint32_t len = xnavi_sprite_len(th, tn);
	Frame tf, bf;
	if (!len || len > PORTRAIT_SIZE || !first_frame(th, len, &tf) || !first_frame(bh, bn, &bf)) return -1;
	if (tf.tiles_len < FACE_W * FACE_H / 2 || tf.pal_len < 32 || bf.pal_len < 32) return -1;
	static uint8_t s[PORTRAIT_SIZE];
	memcpy(s, th, len);
	static uint8_t px[FACE_H][FACE_W];
	memset(px, 0, sizeof px);
	draw_frame(px, bh, bn, &bf, P[k].wx, P[k].wy);
	uint16_t pal[16];
	for (int i = 0; i < 16; ++i) pal[i] = (uint16_t)(bh[bf.pal + 2 * (uint32_t)i] | bh[bf.pal + 2 * (uint32_t)i + 1] << 8);
	int g = ground_index(px, pal);
	const Guardian *gd = guardian(navi);
	pal[g] = (uint16_t)((gd->r * 45 / 100) >> 3 | ((gd->g * 45 / 100) >> 3) << 5 | ((gd->b * 45 / 100) >> 3) << 10);
	for (int y = 0; y < FACE_H; ++y)
		for (int x = 0; x < FACE_W; ++x)
			if (!px[y][x]) px[y][x] = (uint8_t)g;
	put_tiles(s, len, &tf, px);
	for (int i = 0; i < 16; ++i) put16(s + tf.pal + 2 * (uint32_t)i, pal[i]);
	uint32_t at = PORTRAIT_AT + (uint32_t)k * PORTRAIT_SIZE;
	emu_write(at, s, len);
	emu_write32(BUS + list_entry(LIST_FACES, P[k].face), at);
	return P[k].face;
}

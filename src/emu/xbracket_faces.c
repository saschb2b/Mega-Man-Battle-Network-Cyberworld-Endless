/* The faces of BN6's and BN5's Navis on BN4's moved bracket
 * (xbracket_faces.h): tools/bn4_faces.py's plate_from in C, its rules and
 * ties as the oracle states them, checked byte for byte by its check. */
#include "xbracket_faces.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"
#include "emu.h"
#include "rom.h"
#include "xbracket.h"
#include "xbracket_bn4.h"
#include "xvirus.h"
#include "xvirus_tx.h"

#define BUS          0x08000000u
#define LIST_BATTLE  0
#define LIST_FACES   8
#define EMPTY_FACE   0x07     /* a number Gregar points at its black face (src/layer/portrait.c) */
#define FALZAR_FIRST 0x2E     /* list 0: 0x2E + the navi, his battle sprite (src/layer/guardians.c) */
#define MUG_W        40       /* a chat mugshot, list 8 */
#define MUG_H        48
#define HALF_W       (MUG_W / 2)
#define HALF_H       (MUG_H / 2)
#define WIN_W        BN4_PLATE_FACE_W
#define WIN_H        BN4_PLATE_FACE_H
#define KEPT         (BN4_PLATE_OUTER - 1)   /* the face's colours a plate keeps: indexes 1-13 */
#define MAX_OBJS     64
#define SHEET_READ   0x2400u  /* a sheet's bytes the dump reads back (the first sheet's 0x23C8) */

/* Each entrant's face (docs/multirom/bn4-tournament.md, The faces' sources): the navi, his game, his chat mugshot in its list 8,
 * or (mug -1: Falzar's five in Gregar, and EraseMan, whose halved mugshot lost his face) a head window of his battle sprite's
 * standing frame, its top left (wx, wy) from his feet, chosen by eye (tools/bn4_faces.py's HEAD_WINDOW) */
typedef struct { uint8_t navi; bool bn5; int16_t mug; int8_t wx, wy; } Source;
static const Source sources[XBF_FACES] = {
	{ 1, false, 0x47, 0, 0 }, { 2, false, 0x49, 0, 0 }, { 3, false, 0x4B, 0, 0 }, { 4, false, -1, 4, -55 }, { 5, false, 0x4F, 0, 0 },
	{ 6, false, -1, -11, -35 }, { 7, false, -1, 3, -47 }, { 8, false, -1, -5, -72 }, { 9, false, -1, -9, -58 }, { 10, false, -1, -8, -53 },
	{ 12, false, 0x51, 0, 0 }, { 13, false, 0x52, 0, 0 }, { 14, false, 0x54, 0, 0 }, { 15, false, 0x55, 0, 0 },
	{ 16, false, 0x56, 0, 0 }, { 18, false, 0x53, 0, 0 },
	{ 24, true, 69, 0, 0 }, { 25, true, 75, 0, 0 }, { 26, true, 71, 0, 0 }, { 27, true, 72, 0, 0 }, { 28, true, 73, 0, 0 },
	{ 29, true, 74, 0, 0 },
};

static const uint8_t dims[3][4][2] = {
	{ { 8, 8 }, { 16, 16 }, { 32, 32 }, { 64, 64 } },
	{ { 16, 8 }, { 32, 8 }, { 32, 16 }, { 64, 32 } },
	{ { 8, 16 }, { 8, 32 }, { 16, 32 }, { 32, 64 } },
};

/* ---- a sprite of BN's format: its bytes past the 4-byte header (an LZ77 one's past its size word too) */

typedef struct { const uint8_t *b; uint32_t n; uint8_t *dec; } Spr;
typedef struct { uint32_t tiles, tiles_len, pal, pal_len; int nobj; uint8_t obj[MAX_OBJS][5]; } Frame;

static bool word_at(const Spr *s, uint32_t a, uint32_t *v) {
	if (a > s->n || s->n - a < 4) return false;
	*v = get32(s->b + a);
	return true;
}

/* sprite `i` of list `list` of a ROM (its lists' offset `lists`) */
static bool spr_open(const uint8_t *rom, uint32_t lists, int list, int i, Spr *s) {
	memset(s, 0, sizeof *s);
	uint32_t l = get32(rom + lists + 4u * (uint32_t)list);
	if (l < BUS || l - BUS + 4u * (uint32_t)i + 4u > ROM_SIZE) return false;
	uint32_t p = get32(rom + (l - BUS) + 4u * (uint32_t)i), off = p & 0x1FFFFFFu;
	if (off + 8u > ROM_SIZE) return false;
	if (!(p & 0x80000000u)) {
		s->b = rom + off + 4;
		s->n = ROM_SIZE - off - 4;
		return true;
	}
	size_t len = 0;
	s->dec = lz77_decompress(rom + off, ROM_SIZE - off, &len);
	if (!s->dec || len < 8) return false;
	s->b = s->dec + 8;
	s->n = (uint32_t)len - 8;
	return true;
}

static void spr_close(Spr *s) {
	free(s->dec);
	s->dec = NULL;
}

/* animation `anim`'s first frame: its tiles and palette (offsets into s->b) and its objects */
static bool spr_frame(const Spr *s, int anim, Frame *fr) {
	uint32_t f, t, p, m, o, mi, ol;
	if (!word_at(s, 4u * (uint32_t)anim, &f) || !word_at(s, f, &t) || !word_at(s, f + 4, &p) || !word_at(s, f + 8, &m) ||
		!word_at(s, f + 12, &o) || !word_at(s, t, &fr->tiles_len) || !word_at(s, p, &fr->pal_len) || !word_at(s, m, &mi) ||
		m + mi >= s->n || !word_at(s, o + 4u * s->b[m + mi], &ol))
		return false;
	fr->tiles = t + 4;
	fr->pal = p + 4;
	if (fr->tiles_len > s->n - fr->tiles || fr->pal_len > s->n - fr->pal) return false;
	fr->nobj = 0;
	for (uint32_t a = o + ol; fr->nobj < MAX_OBJS; a += 5) {
		if (a + 5 > s->n) return false;
		if (s->b[a] == 0xFF && s->b[a + 1] == 0xFF) break;
		if ((s->b[a + 4] & 3) > 2) return false;
		memcpy(fr->obj[fr->nobj++], s->b + a, 5);
	}
	return fr->nobj > 0;
}

/* its objects' box from the origin: box[0], box[1] its top left, box[2], box[3] past its bottom right */
static void spr_box(const Frame *fr, int box[4]) {
	box[0] = box[1] = 1000;
	box[2] = box[3] = -1000;
	for (int k = 0; k < fr->nobj; ++k) {
		const uint8_t *o = fr->obj[k];
		int x = (int8_t)o[1], y = (int8_t)o[2], w = dims[o[4] & 3][o[3] & 3][0], h = dims[o[4] & 3][o[3] & 3][1];
		if (x < box[0]) box[0] = x;
		if (y < box[1]) box[1] = y;
		if (x + w > box[2]) box[2] = x + w;
		if (y + h > box[3]) box[3] = y + h;
	}
}

/* the frame's objects drawn in their order into px (w x h, palette indexes), its (0, 0) at (ox, oy) from the origin: a later
 * object's pixel over an earlier's where it is not 0, flips as each says, a tile past the frame's tiles left out */
static void spr_draw(const Spr *s, const Frame *fr, uint8_t *px, int w, int h, int ox, int oy) {
	for (int k = 0; k < fr->nobj; ++k) {
		const uint8_t *o = fr->obj[k];
		int ow = dims[o[4] & 3][o[3] & 3][0], oh = dims[o[4] & 3][o[3] & 3][1];
		for (int ty = 0; ty < oh / 8; ++ty)
			for (int tx = 0; tx < ow / 8; ++tx) {
				uint32_t t = o[0] + (uint32_t)(ty * (ow / 8) + tx);
				if ((t + 1) * 32 > fr->tiles_len) continue;
				const uint8_t *td = s->b + fr->tiles + t * 32;
				for (int i = 0; i < 64; ++i) {
					int ci = i & 1 ? td[i / 2] >> 4 : td[i / 2] & 15, X = tx * 8 + i % 8, Y = ty * 8 + i / 8;
					if (o[3] & 0x40) X = ow - 1 - X;
					if (o[3] & 0x80) Y = oh - 1 - Y;
					X += (int8_t)o[1] - ox;
					Y += (int8_t)o[2] - oy;
					if (ci && X >= 0 && Y >= 0 && X < w && Y < h) px[Y * w + X] = (uint8_t)ci;
				}
			}
	}
}

static void colours_of(const Spr *s, const Frame *fr, uint16_t pal[16]) {
	for (int i = 0; i < 16; ++i) pal[i] = 2u * (uint32_t)i + 1 < fr->pal_len ? get16(s->b + fr->pal + 2u * (uint32_t)i) : 0;
}

/* ---- a face from its own game: the window (WIN_W x WIN_H) and 16 colours */

static int lum(uint16_t c) { return (c & 31) + (c >> 5 & 31) + (c >> 10 & 31); }

/* a mugshot halved: each 2x2 block its commonest index, a tie to the darker, then to the lower; its middle columns and rows in the
 * window, a row of its ground (its edge's commonest index, a tie to the lower) above and below, its 0s the ground */
static void halve(const uint8_t px[MUG_H * MUG_W], const uint16_t pal[16], uint8_t win[WIN_H][WIN_W]) {
	static uint8_t half[HALF_H][HALF_W];
	for (int y = 0; y < HALF_H; ++y)
		for (int x = 0; x < HALF_W; ++x) {
			int cnt[16] = { 0 }, best = -1;
			for (int j = 0; j < 4; ++j) ++cnt[px[(2 * y + j / 2) * MUG_W + 2 * x + j % 2]];
			for (int c = 0; c < 16; ++c)
				if (cnt[c] && (best < 0 || cnt[c] > cnt[best] || (cnt[c] == cnt[best] && lum(pal[c]) < lum(pal[best])))) best = c;
			half[y][x] = (uint8_t)best;
		}
	int edge[16] = { 0 }, ground = 0;
	for (int x = 0; x < HALF_W; ++x) ++edge[half[0][x]], ++edge[half[HALF_H - 1][x]];
	for (int y = 0; y < HALF_H; ++y) ++edge[half[y][0]], ++edge[half[y][HALF_W - 1]];
	for (int c = 1; c < 16; ++c)
		if (edge[c] > edge[ground]) ground = c;
	for (int y = 0; y < WIN_H; ++y)
		for (int x = 0; x < WIN_W; ++x) {
			int c = y == 0 || y == WIN_H - 1 ? ground : half[y - 1][x + 1];
			win[y][x] = (uint8_t)(c ? c : ground);
		}
}

/* face `k` from its game's ROM: false where the ROM gives none (BN5's not read, a black mugshot, a sprite not as read) */
static bool face_of(int k, uint8_t win[WIN_H][WIN_W], uint16_t pal[16]) {
	const Source *src = &sources[k];
	const uint8_t *rom = src->bn5 ? XR[XROM_BN5_COLONEL_US].data : R.data;
	if (!rom || (src->bn5 ? !XR[XROM_BN5_COLONEL_US].layout : !R.layout)) return false;
	uint32_t lists = src->bn5 ? XR[XROM_BN5_COLONEL_US].layout->sprite_lists : R.layout->sprite_lists;
	Spr s, black;
	Frame fr;
	bool ok = false;
	if (src->mug >= 0) {
		if (!spr_open(rom, lists, LIST_FACES, src->mug, &s)) return false;
		/* (BN6's black face, as Gregar points its empty numbers: none) */
		bool empty = !src->bn5 && spr_open(rom, lists, LIST_FACES, EMPTY_FACE, &black) && black.b == s.b;
		if (!src->bn5) spr_close(&black);
		int box[4];
		if (!empty && spr_frame(&s, 0, &fr) && (spr_box(&fr, box), box[2] - box[0] == MUG_W && box[3] - box[1] == MUG_H)) {
			static uint8_t px[MUG_H * MUG_W];
			memset(px, 0, sizeof px);
			spr_draw(&s, &fr, px, MUG_W, MUG_H, box[0], box[1]);
			colours_of(&s, &fr, pal);
			halve(px, pal, win);
			ok = true;
		}
	} else if (spr_open(rom, lists, LIST_BATTLE, FALZAR_FIRST + src->navi, &s) && spr_frame(&s, 0, &fr)) {
		/* (a head window at its own scale, on a ground of his darkest colour, the first of a tie) */
		colours_of(&s, &fr, pal);
		int ground = 1;
		for (int i = 2; i < 16; ++i)
			if (lum(pal[i]) < lum(pal[ground])) ground = i;
		memset(win, ground, (size_t)WIN_W * WIN_H);
		spr_draw(&s, &fr, &win[0][0], WIN_W, WIN_H, src->wx, src->wy);
		ok = true;
	}
	spr_close(&s);
	return ok;
}

/* ---- the conversion: a window into a plate */

static long gap(uint16_t a, uint16_t b) {
	long d = 0;
	for (int sh = 0; sh < 15; sh += 5) {
		long c = (long)(a >> sh & 31) - (long)(b >> sh & 31);
		d += c * c;
	}
	return d;
}

/* the window's indexes, each its count, in the order first seen (the oracle's dict) */
typedef struct { int cnt[16], seen[16], nseen, into[16]; bool alive[16]; } Used;

/* the least used (the first seen of a tie) merged into its nearest (the first seen of a tie) while more than KEPT are left */
static void merge_colours(Used *u, const uint16_t pal[16]) {
	int *cnt = u->cnt, *seen = u->seen, nseen = u->nseen;
	bool *alive = u->alive;
	for (int left = nseen; left > KEPT; --left) {
		int least = -1, near = -1;
		for (int i = 0; i < nseen; ++i)
			if (alive[seen[i]] && (least < 0 || cnt[seen[i]] < cnt[least])) least = seen[i];
		for (int i = 0; i < nseen; ++i)
			if (alive[seen[i]] && seen[i] != least && (near < 0 || gap(pal[seen[i]], pal[least]) < gap(pal[near], pal[least]))) near = seen[i];
		cnt[near] += cnt[least];
		alive[least] = false;
		u->into[least] = near;
	}
}

/* the window's colours to at most KEPT slots (merge_colours); the kept by count (a tie by first seen) slots 1 on, a merged
 * index its target's slot. slot[c] for every index the window uses; colour[n] slot n's kept colour */
static void slots_of(uint8_t win[WIN_H][WIN_W], const uint16_t pal[16], uint8_t slot[16], uint16_t colour[16]) {
	Used u;
	memset(&u, 0, sizeof u);
	for (int y = 0; y < WIN_H; ++y)
		for (int x = 0; x < WIN_W; ++x) {
			int c = win[y][x];
			if (!u.alive[c]) u.alive[c] = true, u.seen[u.nseen++] = c;
			++u.cnt[c];
		}
	for (int c = 0; c < 16; ++c) u.into[c] = -1;
	merge_colours(&u, pal);
	memset(colour, 0, 16 * sizeof *colour);
	for (int n = 1, done = 0; done < u.nseen; ++done) {
		int pick = -1;
		for (int i = 0; i < u.nseen; ++i)
			if (u.alive[u.seen[i]] && (pick < 0 || u.cnt[u.seen[i]] > u.cnt[pick])) pick = u.seen[i];
		if (pick < 0) break;
		u.alive[pick] = false;
		slot[pick] = (uint8_t)n;
		colour[n++] = pal[pick];
	}
	for (int c = 0; c < 16; ++c) {
		int b = c;
		while (u.into[b] >= 0) b = u.into[b];
		if (b != c) slot[c] = slot[b];
	}
}

static uint16_t bgr(int r, int g, int b) {
	r = r < 0 ? 0 : r > 31 ? 31 : r;
	g = g < 0 ? 0 : g > 31 ? 31 : g;
	b = b < 0 ? 0 : b > 31 ? 31 : b;
	return (uint16_t)(r | g << 5 | b << 10);
}

/* v thousandths rounded to a whole, a half to the even (Python's round of the oracle's float, equal for every colour) */
static int round_even(long v) {
	long q = v / 1000, r = v % 1000;
	return (int)(r > 500 || (r == 500 && (q & 1)) ? q + 1 : q);
}

/* a variant's colour of a face colour: 0 and 3 it, 1 each channel plus 5, 2 a sepia grey o = 5 + 0.7 (0.25 r + 0.6 g + 0.15 b)
 * as (o + 1, o, o - 1) */
static uint16_t variant(int v, uint16_t c) {
	int r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
	if (v == BN4_PLATE_LIGHT) return bgr(r + 5, g + 5, b + 5);
	if (v != BN4_PLATE_GREY) return c;
	long o = 5000 + 7L * (r * 25 + g * 60 + b * 15);
	return bgr(round_even(o + 1000), round_even(o), round_even(o - 1000));
}

/* the plate's pixels packed as the two objects read their tiles: the 16x32's 8 (two a row of tiles), then the 8x32's 4 */
static void pack_tiles(uint8_t plate[BN4_PLATE_H][BN4_PLATE_W], uint8_t *t) {
	for (int k = 0; k < BN4_PLATE_TILES; ++k) {
		int x = k < 8 ? (k & 1) * 8 : 16, y = (k < 8 ? k / 2 : k - 8) * 8;
		for (int py = 0; py < 8; ++py)
			for (int p = 0; p < 8; p += 2) *t++ = (uint8_t)(plate[y + py][x + p] | plate[y + py][x + p + 1] << 4);
	}
}

/* the plate: BN4's frame (its outer and inner indexes), the window at its place, its tiles packed; its four palettes, the
 * frame's colours BN4's own (`frames`, per variant) */
static void plate_of(uint8_t win[WIN_H][WIN_W], const uint16_t pal[16], const uint16_t frames[BN4_PLATE_VARIANTS][2],
                     uint8_t out[XBF_PLATE_BYTES]) {
	uint8_t slot[16] = { 0 }, plate[BN4_PLATE_H][BN4_PLATE_W];
	uint16_t colour[16];
	slots_of(win, pal, slot, colour);
	memset(plate, 0, sizeof plate);
	for (int y = 0; y < 30; ++y)
		for (int x = 1; x < 23; ++x) plate[y][x] = y == 0 || y == 29 || x == 1 || x == 22 ? BN4_PLATE_OUTER : BN4_PLATE_INNER;
	for (int y = 0; y < WIN_H; ++y)
		for (int x = 0; x < WIN_W; ++x) plate[BN4_PLATE_FACE_Y + y][BN4_PLATE_FACE_X + x] = slot[win[y][x]];
	pack_tiles(plate, out);
	uint8_t *t = out + BN4_PLATE_TILES * 32;
	for (int v = 0; v < BN4_PLATE_VARIANTS; ++v)
		for (int i = 0; i < 16; ++i) {
			uint16_t c = i == 0 ? 0 : i == BN4_PLATE_OUTER ? frames[v][0] : i == BN4_PLATE_INNER ? frames[v][1] : variant(v, colour[i]);
			put16(t + 32 * v + 2 * i, c);
		}
}

/* ---- BN4's plates: where an id's tiles and palettes sit in its sheet */

typedef struct { uint32_t tiles, pal[BN4_PLATE_VARIANTS]; } Place;   /* offsets from the sheet's start */

/* the plate of BN4's id `id` in `sheet` (`n` bytes from it, a copy or BN4's): its animation and first palette BN4's tables' */
static bool place_of(const uint8_t *sheet, uint32_t n, int id, Place *pl) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	Spr s = { sheet + 4, n - 4, NULL };
	Frame fr;
	int anim = b4[BN4_PLATE_FRAMES - BUS + (uint32_t)id], base = b4[BN4_PLATE_PALETTES - BUS + (uint32_t)id];
	if (!spr_frame(&s, anim, &fr) || fr.tiles_len != BN4_PLATE_TILES * 32 || fr.nobj != 2) return false;
	pl->tiles = 4 + fr.tiles;
	for (int v = 0; v < BN4_PLATE_VARIANTS; ++v) {
		uint32_t at = fr.pal + 32u * (uint32_t)(base + v + (fr.obj[0][4] >> 4));
		if (at + 32 > s.n) return false;
		pl->pal[v] = 4 + at;
	}
	return true;
}

/* the sheet BN4's id `id` is drawn from, and its extent */
static const uint8_t *sheet_of(int id, uint32_t *n) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	bool a = b4[BN4_PLATE_SHEETS - BUS + (uint32_t)id] == 0;
	*n = a ? BN4_PLATE_SHEET_B - BN4_PLATE_SHEET_A : BN4_PLATE_SHEET_B_END - BN4_PLATE_SHEET_B;
	return b4 + ((a ? BN4_PLATE_SHEET_A : BN4_PLATE_SHEET_B) - BUS);
}

/* the template a face is written over: BN4's id of the same animation in its sheet (the first's 0x00-0x0F, the second's 0x10-) */
static int template_id(int k) { return k < XBF_FACES_BN6 ? k : 0x10 + (k - XBF_FACES_BN6); }

/* ---- the install */

static bool made[XBF_FACES];

static bool mutated(const char *what) {
	const char *m = getenv("CYBERWORLD_XBRACKET_MUTATE");
	return m && !strcmp(m, what);
}

/* the literal of BN4's at `lit` (its word `was`) in the moved copy, made `now` */
static void literal(uint32_t lit, uint32_t was, uint32_t now) {
	uint32_t at = xvirus_code_at(&xbracket_bn4, lit);
	if (!at || get32(XR[XROM_BN4_RED_SUN_US].data + (lit - BUS)) != was) {
		xv_fail("the plates' literal at %08x", (unsigned)lit);
		return;
	}
	uint8_t *p = xv_put(at, 4, false);
	if (p) put32(p, now);
}

/* the grown tables: BN4's own entries, the empty plate's past them, a face's id its template's animation and palette in its copy */
static void tables_stage(void) {
	const uint8_t *b4 = XR[XROM_BN4_RED_SUN_US].data;
	uint8_t *t = xv_put(XBF_FRAMES, XBF_TABLES_END - XBF_FRAMES, true);
	if (!t) return;
	uint8_t *frames = t, *pals = t + (XBF_PALETTES - XBF_FRAMES), *sheets = t + (XBF_SHEETS - XBF_FRAMES);
	for (int id = 0; id < XBF_IDS; ++id) {
		int from = id < BN4_PLATE_IDS ? id : BN4_PLATE_EMPTY_ID;
		frames[id] = b4[BN4_PLATE_FRAMES - BUS + (uint32_t)from];
		pals[id] = b4[BN4_PLATE_PALETTES - BUS + (uint32_t)from];
		sheets[id] = b4[BN4_PLATE_SHEETS - BUS + (uint32_t)from];
	}
	for (int k = 0; k < XBF_FACES; ++k) {
		if (!made[k]) continue;
		int id = XBR_PLATE_NAVI(sources[k].navi), from = template_id(k);
		frames[id] = b4[BN4_PLATE_FRAMES - BUS + (uint32_t)from];
		pals[id] = b4[BN4_PLATE_PALETTES - BUS + (uint32_t)from];
		sheets[id] = k < XBF_FACES_BN6 ? XBF_SHEET_VALUE_A : XBF_SHEET_VALUE_B;
	}
	uint8_t *sprites = t + (XBF_SPRITES - XBF_FRAMES);
	memcpy(sprites, b4 + (BN4_PLATE_SPRITES - BUS), BN4_PLATE_SHEET_VALUES);
	sprites[XBF_SHEET_VALUE_A] = XBF_SPRITE_A;
	sprites[XBF_SHEET_VALUE_B] = XBF_SPRITE_B;
	uint8_t *ptrs = t + (XBF_PTRS - XBF_FRAMES);
	for (int i = 0; i < BN4_MENU_SPRITE_PTRS_USED; ++i)
		put32(ptrs + 4 * i, xvirus_code_at(&xbracket_bn4, get32(b4 + (BN4_MENU_SPRITE_PTRS - BUS) + 4u * (uint32_t)i)));
	put32(ptrs + 4 * XBF_SPRITE_A, XBF_SHEET_A);
	put32(ptrs + 4 * XBF_SPRITE_B, XBF_SHEET_B);
}

/* BN4's frame colours, per variant: GutsMan's plate's (id 0x00), its outer and inner */
static bool frame_colours(uint16_t frames[BN4_PLATE_VARIANTS][2]) {
	uint32_t n;
	const uint8_t *sheet = sheet_of(0x00, &n);
	Place pl;
	if (!place_of(sheet, n, 0x00, &pl)) return false;
	for (int v = 0; v < BN4_PLATE_VARIANTS; ++v) {
		frames[v][0] = get16(sheet + pl.pal[v] + 2 * BN4_PLATE_OUTER);
		frames[v][1] = get16(sheet + pl.pal[v] + 2 * BN4_PLATE_INNER);
	}
	return true;
}

/* the two sheets copied, each face made written over its template's plate */
static void sheets_stage(const uint16_t frames[BN4_PLATE_VARIANTS][2]) {
	uint32_t na, nb;
	const uint8_t *a = sheet_of(0x00, &na), *b = sheet_of(0x10, &nb);
	uint8_t *ca = xv_put(XBF_SHEET_A, na, true), *cb = xv_put(XBF_SHEET_B, nb, true);
	if (!ca || !cb) return;
	memcpy(ca, a, na);
	memcpy(cb, b, nb);
	for (int k = 0; k < XBF_FACES; ++k) {
		static uint8_t win[WIN_H][WIN_W];
		uint16_t pal[16];
		made[k] = face_of(k, win, pal);
		if (!made[k]) continue;
		uint8_t plate[XBF_PLATE_BYTES], *sheet = k < XBF_FACES_BN6 ? ca : cb;
		Place pl;
		if (!place_of(sheet, k < XBF_FACES_BN6 ? na : nb, template_id(k), &pl)) {
			xv_fail("BN4's plate %#x is not as the faces read it", (unsigned)template_id(k));
			return;
		}
		plate_of(win, pal, frames, plate);
		memcpy(sheet + pl.tiles, plate, BN4_PLATE_TILES * 32);
		/* (CYBERWORLD_XBRACKET_MUTATE=facepal: the first face's palettes left BN4's, tools/bn4_faces.py check --mutation facepal) */
		if (k == 0 && mutated("facepal")) continue;
		for (int v = 0; v < BN4_PLATE_VARIANTS; ++v) memcpy(sheet + pl.pal[v], plate + BN4_PLATE_TILES * 32 + 32 * v, 32);
	}
}

bool xbracket_faces_stage(void) {
	if (xv_failed() || !XR[XROM_BN4_RED_SUN_US].data || !R.data) return false;
	uint16_t frames[BN4_PLATE_VARIANTS][2];
	if (!frame_colours(frames)) return xv_fail("BN4's plate 0x00 is not as the faces read it");
	sheets_stage(frames);
	tables_stage();
	literal(BN4_PLATE_LIT_FRAMES, BN4_PLATE_FRAMES, XBF_FRAMES);
	literal(BN4_PLATE_LIT_PALETTES, BN4_PLATE_PALETTES, XBF_PALETTES);
	literal(BN4_PLATE_LIT_SHEETS, BN4_PLATE_SHEETS, XBF_SHEETS);
	literal(BN4_PLATE_LIT_SPRITES, BN4_PLATE_SPRITES, XBF_SPRITES);
	literal(BN4_MENU_SPRITE_LIT_PTRS, BN4_MENU_SPRITE_PTRS, XBF_PTRS);
	return !xv_failed();
}

/* ---- the check's dump: read back as the bracket reads a plate */

static uint32_t moved_word(uint32_t lit) { return emu_read32(xvirus_code_at(&xbracket_bn4, lit)); }

void xbracket_faces_dump(void) {
	const char *path = getenv("CYBERWORLD_XBRACKET_FACES");
	FILE *f = path && *path ? fopen(path, "w") : NULL;
	if (!f) return;
	static uint8_t sheet[SHEET_READ];
	for (int k = 0; k < XBF_FACES; ++k) {
		if (!made[k]) continue;
		int id = XBR_PLATE_NAVI(sources[k].navi);
		uint8_t anim = emu_read8(moved_word(BN4_PLATE_LIT_FRAMES) + (uint32_t)id);
		uint8_t base = emu_read8(moved_word(BN4_PLATE_LIT_PALETTES) + (uint32_t)id);
		uint8_t value = emu_read8(moved_word(BN4_PLATE_LIT_SHEETS) + (uint32_t)id);
		uint8_t sprite = emu_read8(moved_word(BN4_PLATE_LIT_SPRITES) + value);
		uint32_t at = emu_read32(moved_word(BN4_MENU_SPRITE_LIT_PTRS) + 4u * sprite);
		for (uint32_t i = 0; i < SHEET_READ; ++i) sheet[i] = at + i < FREE_END ? emu_read8(at + i) : 0;
		Spr s = { sheet + 4, SHEET_READ - 4, NULL };
		Frame fr;
		fprintf(f, "%d ", sources[k].navi);
		if (spr_frame(&s, anim, &fr)) {
			for (uint32_t i = 0; i < BN4_PLATE_TILES * 32 && fr.tiles + i < s.n; ++i) fprintf(f, "%02x", s.b[fr.tiles + i]);
			for (int v = 0; v < BN4_PLATE_VARIANTS; ++v)
				for (uint32_t i = 0, p = fr.pal + 32u * (uint32_t)(base + v + (fr.obj[0][4] >> 4)); i < 32 && p + i < s.n; ++i)
					fprintf(f, "%02x", s.b[p + i]);
		}
		fprintf(f, "\n");
	}
	fclose(f);
	printf("xbracket: the faces' plates read back into %s\n", path);
}

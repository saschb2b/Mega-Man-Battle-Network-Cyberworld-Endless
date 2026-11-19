/* cross_rows.h. A row of CROSSSELECT is 9 x 2 tiles (72 x 16 pixels) in
 * one palette: the Cross Navi's face at the left in colours 1-8, then a
 * band (11, 12) under the name, its letters 9 in a box of 13, the frame 13
 * round it; its grey twin draws the face and the letters in the greys
 * 10, 13, 14, 15 alone (measured from Gregar's ten, docs/rom/souls.md,
 * CROSSSELECT). Falzar's are Gregar's SlashCross row, whose box holds five
 * letters, with the face and the name made anew: the face the Navi's own
 * battle sprite (his portrait's window, portrait.c), its colours cut to
 * eight on a ground of his title card's colour; the name in block letters
 * of the engine's own, 6 x 8 with strokes two pixels wide as the rows' own
 * are, drawn for this project. Numbers only in the code: the pixels are
 * the player's ROM's. */
#include "cross_rows.h"

#include <limits.h>
#include <string.h>

#include "bn6.h"
#include "crossselect.h"
#include "guardians.h"
#include "portrait.h"
#include "rom.h"

#define BUS        0x08000000u
#define ROW_W      72
#define ROW_H      16
#define TEMPLATE   2         /* Gregar's SlashCross row */
#define PORTRAIT_W 40        /* the portrait's window (portrait.c), the face cut from its top */
#define PORTRAIT_H 48
#define FACE_COLOURS 8       /* colours 1-8 */
#define BOX_TOP    3         /* the name's box, rows 3-12, its right column 69 */
#define BOX_BOTTOM 12
#define BOX_RIGHT  69
#define LETTER_W   6
#define LETTER_H   8
#define PITCH      8         /* a letter and the two columns of the box after it */
#define C_LETTER   9
#define C_BAND_B   11
#define C_BAND_C   12
#define C_BOX      13
#define C_GREY_LETTER 15

static const char *const names[CROSSSELECT_FALZAR] = { "SPOUT", "TMHWK", "TENGU", "GRND", "DUST" };

/* The engine's block letters, the ones the names take */
static const struct { char c; const char *rows[LETTER_H]; } letters[] = {
	{ 'D', { "#####.", "######", "##..##", "##..##", "##..##", "##..##", "######", "#####." } },
	{ 'E', { "######", "######", "##....", "#####.", "#####.", "##....", "######", "######" } },
	{ 'G', { ".#####", "######", "##....", "##.###", "##.###", "##..##", "######", ".#####" } },
	{ 'H', { "##..##", "##..##", "##..##", "######", "######", "##..##", "##..##", "##..##" } },
	{ 'K', { "##..##", "##.##.", "####..", "###...", "####..", "##.##.", "##..##", "##..##" } },
	{ 'M', { "##..##", "######", "######", "##..##", "##..##", "##..##", "##..##", "##..##" } },
	{ 'N', { "##..##", "###.##", "######", "######", "##.###", "##..##", "##..##", "##..##" } },
	{ 'O', { ".####.", "######", "##..##", "##..##", "##..##", "##..##", "######", ".####." } },
	{ 'P', { "#####.", "######", "##..##", "##..##", "######", "#####.", "##....", "##...." } },
	{ 'R', { "#####.", "######", "##..##", "######", "#####.", "##.##.", "##..##", "##..##" } },
	{ 'S', { ".#####", "######", "##....", "#####.", ".#####", "....##", "######", "#####." } },
	{ 'T', { "######", "######", "..##..", "..##..", "..##..", "..##..", "..##..", "..##.." } },
	{ 'U', { "##..##", "##..##", "##..##", "##..##", "##..##", "##..##", "######", ".####." } },
	{ 'W', { "##..##", "##..##", "##..##", "##..##", "######", "######", "##..##", "#....#" } },
};

static const char *const *letter(char c) {
	for (size_t i = 0; i < sizeof letters / sizeof *letters; ++i)
		if (letters[i].c == c) return letters[i].rows;
	return NULL;
}

/* a row's art (9 x 2 tiles of 4 bits a pixel) to and from its pixels */
static void decode(const uint8_t *art, uint8_t px[ROW_H][ROW_W]) {
	for (int y = 0; y < ROW_H; ++y)
		for (int x = 0; x < ROW_W; ++x) {
			uint8_t v = art[((y / 8) * 9 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2];
			px[y][x] = x & 1 ? v >> 4 : v & 15;
		}
}

static void encode(uint8_t px[ROW_H][ROW_W], uint8_t *art) {
	memset(art, 0, BN6_CROSS_ART_SIZE);
	for (int y = 0; y < ROW_H; ++y)
		for (int x = 0; x < ROW_W; ++x)
			art[((y / 8) * 9 + x / 8) * 32 + (y % 8) * 4 + (x % 8) / 2] |= (uint8_t)(x & 1 ? px[y][x] << 4 : px[y][x]);
}

static long gap(uint16_t a, uint16_t b) {
	long d = 0;
	for (int s = 0; s < 15; s += 5) {
		long c = (long)(a >> s & 31) - (long)(b >> s & 31);
		d += c * c;
	}
	return d;
}

static int light(uint16_t c) { return 3 * (c & 31) + 6 * (c >> 5 & 31) + (c >> 10 & 31); }

/* the face's left columns: in each row, up to the band's first pixel */
static void face_edge(uint8_t px[ROW_H][ROW_W], int edge[ROW_H]) {
	for (int y = 0; y < ROW_H; ++y) {
		edge[y] = 1;
		for (int x = 1; x < ROW_W / 2; ++x)
			if (px[y][x] == C_BAND_B || px[y][x] == C_BAND_C) { edge[y] = x; break; }
	}
}

/* the name in its box, its letters in `ink`, right-aligned at BOX_RIGHT,
 * the band under the old one first */
static void name(uint8_t px[ROW_H][ROW_W], const int edge[ROW_H], const char *s, uint8_t ink) {
	int n = (int)strlen(s), x0 = BOX_RIGHT + 1 - PITCH * n;
	for (int y = BOX_TOP; y <= BOX_BOTTOM; ++y)
		for (int x = edge[y]; x <= BOX_RIGHT; ++x) px[y][x] = x >= x0 ? C_BOX : C_BAND_C;
	for (int i = 0; i < n; ++i) {
		const char *const *rows = letter(s[i]);
		for (int y = 0; rows && y < LETTER_H; ++y)
			for (int x = 0; x < LETTER_W; ++x)
				if (rows[y][x] == '#') px[BOX_TOP + 1 + y][x0 + 1 + PITCH * i + x] = ink;
	}
}

/* the face's window in the portrait's: its top rows that hold the Navi,
 * centred on him across them */
static void face_window(uint8_t all[PORTRAIT_H][PORTRAIT_W], int fw, int *wx, int *wy) {
	int top = 0, lo = PORTRAIT_W, hi = -1;
	for (bool any = false; top < PORTRAIT_H - ROW_H; ++top) {
		for (int x = 0; x < PORTRAIT_W; ++x) any |= all[top][x] != 0;
		if (any) break;
	}
	for (int y = top; y < top + ROW_H && y < PORTRAIT_H; ++y)
		for (int x = 0; x < PORTRAIT_W; ++x)
			if (all[y][x]) { if (x < lo) lo = x; if (x > hi) hi = x; }
	int x = hi < 0 ? 0 : (lo + hi + 1) / 2 - fw / 2;
	*wx = x < 0 ? 0 : x > PORTRAIT_W - fw ? PORTRAIT_W - fw : x;
	*wy = top;
}

/* Falzar Cross k's face: his portrait's window cut to FACE_COLOURS colours
 * (`colours`, 1-8 in `out`), on his title card's colour darkened */
static bool face(int k, const int edge[ROW_H], uint8_t out[ROW_H][ROW_W], uint16_t colours[FACE_COLOURS + 1]) {
	enum { FW = 30 };
	static uint8_t all[PORTRAIT_H][PORTRAIT_W];
	uint16_t pal[16];
	if (!portrait_pixels(6 + k, 0, 0, PORTRAIT_W, PORTRAIT_H, &all[0][0], pal)) return false;
	int wx, wy;
	face_window(all, FW, &wx, &wy);
	uint8_t win[ROW_H][FW];
	memset(win, 0, sizeof win);
	for (int y = 0; y < ROW_H && wy + y < PORTRAIT_H; ++y) memcpy(win[y], &all[wy + y][wx], FW);
	const Guardian *g = guardian(6 + k);
	pal[0] = (uint16_t)((g->r * 45 / 100) >> 3 | ((g->g * 45 / 100) >> 3) << 5 | ((g->b * 45 / 100) >> 3) << 10);
	int count[16] = { 0 };
	for (int y = 0; y < ROW_H - 1; ++y)
		for (int x = 1; x < edge[y] && x < FW; ++x) ++count[win[y][x]];
	/* (the colours most used, then each pixel the nearest of them) */
	int pick[FACE_COLOURS], picked = 0;
	for (; picked < FACE_COLOURS; ++picked) {
		int best = -1;
		for (int i = 0; i < 16; ++i)
			if (count[i] && (best < 0 || count[i] > count[best])) best = i;
		if (best < 0) break;
		pick[picked] = best;
		count[best] = 0;
	}
	if (!picked) return false;
	for (int i = 0; i < picked; ++i) colours[1 + i] = pal[pick[i]];
	for (int i = picked; i < FACE_COLOURS; ++i) colours[1 + i] = colours[1];
	for (int y = 0; y < ROW_H - 1; ++y)
		for (int x = 1; x < edge[y]; ++x) {
			uint16_t c = x < FW ? pal[win[y][x]] : pal[0];
			int best = 0;
			long d = LONG_MAX;
			for (int i = 0; i < picked; ++i)
				if (gap(c, pal[pick[i]]) < d) { d = gap(c, pal[pick[i]]); best = i; }
			out[y][x] = (uint8_t)(1 + best);
		}
	return true;
}

/* the colours 1-8 as `ramp`'s (a row of Gregar's in its other palette), by lightness */
static void tinted(const uint16_t colours[FACE_COLOURS + 1], const uint8_t *ramp, uint16_t out[FACE_COLOURS + 1]) {
	uint16_t r[FACE_COLOURS];
	for (int i = 0; i < FACE_COLOURS; ++i) r[i] = (uint16_t)(ramp[2 + 2 * i] | ramp[3 + 2 * i] << 8);
	for (int i = 0; i < FACE_COLOURS; ++i)
		for (int j = i + 1; j < FACE_COLOURS; ++j)
			if (light(r[j]) < light(r[i])) { uint16_t t = r[i]; r[i] = r[j]; r[j] = t; }
	for (int i = 1; i <= FACE_COLOURS; ++i) {
		int rank = 0;
		for (int j = 1; j <= FACE_COLOURS; ++j) rank += light(colours[j]) < light(colours[i]);
		out[i] = r[rank];
	}
}

/* a colour 1-8 as the grey rows draw it: their four greys, darkest first */
static uint8_t grey_of(uint16_t c) {
	static const uint8_t greys[4] = { 13, 10, 14, 15 };
	int l = light(c) * 4 / (10 * 31 + 1);
	return greys[l > 3 ? 3 : l];
}

bool cross_rows_make(uint8_t *art, uint8_t *pals) {
	if (!R.data) return false;
	const uint8_t *rows = R.data + (BN6_CROSS_ART - BUS), *rp = R.data + (BN6_CROSS_PALS - BUS);
	uint8_t tpl[ROW_H][ROW_W], grey[ROW_H][ROW_W];
	decode(rows + TEMPLATE * BN6_CROSS_ART_SIZE, tpl);
	decode(rows + (BN6_CROSS_ROWS + TEMPLATE) * BN6_CROSS_ART_SIZE, grey);
	int edge[ROW_H];
	face_edge(tpl, edge);
	for (int k = 0; k < CROSSSELECT_FALZAR; ++k) {
		uint8_t px[ROW_H][ROW_W], gx[ROW_H][ROW_W];
		memcpy(px, tpl, sizeof px);
		memcpy(gx, grey, sizeof gx);
		uint16_t colours[FACE_COLOURS + 1] = { 0 }, chosen[FACE_COLOURS + 1] = { 0 };
		if (!face(k, edge, px, colours)) return false;
		for (int y = 0; y < ROW_H - 1; ++y)
			for (int x = 1; x < edge[y]; ++x) gx[y][x] = grey_of(colours[px[y][x]]);
		name(px, edge, names[k], C_LETTER);
		name(gx, edge, names[k], C_GREY_LETTER);
		encode(px, art + (size_t)k * BN6_CROSS_ART_SIZE);
		encode(gx, art + (size_t)(CROSSSELECT_FALZAR + k) * BN6_CROSS_ART_SIZE);
		/* (the palettes: the face's colours, the rest SlashCross's; chosen, the face tinted as Gregar's tints its own) */
		uint8_t *p = pals + (size_t)k * 0x20, *c = pals + (size_t)(CROSSSELECT_FALZAR + k) * 0x20;
		memcpy(p, rp + TEMPLATE * 0x20, 0x20);
		memcpy(c, rp + (BN6_CROSS_ROWS + TEMPLATE) * 0x20, 0x20);
		tinted(colours, rp + (BN6_CROSS_ROWS + TEMPLATE) * 0x20, chosen);
		for (int i = 1; i <= FACE_COLOURS; ++i) {
			p[2 * i] = (uint8_t)colours[i]; p[2 * i + 1] = (uint8_t)(colours[i] >> 8);
			c[2 * i] = (uint8_t)chosen[i]; c[2 * i + 1] = (uint8_t)(chosen[i] >> 8);
		}
	}
	return true;
}

void cross_rows_install(void) {
	static uint8_t art[CROSSSELECT_ART_BYTES], pals[CROSSSELECT_PAL_BYTES];
	if (!crossselect_falzar() && cross_rows_make(art, pals)) crossselect_install(art, pals);
}

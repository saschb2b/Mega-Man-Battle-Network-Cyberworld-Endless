/* BN5's own pieces of a liberation's board (liberation_pieces.h), each
 * laid out as BN5 lays its OBJs and BG0 (traced in BN5: docs/ROM_DATA.md,
 * BN5's Liberation board drawn in BN6), its tiles and colours read from
 * the player's BN5 ROM. Positions are the GBA picture's. */
#include "liberation_pieces.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn5_liberation.h"
#include "gfx.h"
#include "liberation_state.h"
#include "pet_look.h"
#include "platform.h"
#include "rom.h"
#include "xtiles.h"

#define HUD_TILES   28     /* the gauge's tiles in RAM: 20 of its frame, the bar's 6, the count's 2 */
#define BAR_FIRST   20
#define BAR_TILES   6
#define COUNT_FIRST 26
#define WINDOW_X    112    /* the command window's frame, on BG0 */
#define WINDOW_Y    104
#define WINDOW_W    16
#define WINDOW_H    7
#define LINE_X      128    /* ... its lines' capitals, a line 14 down */
#define LINE_Y      114
#define LINE_STEP   14
#define CURSOR_X    112
#define CURSOR_Y    112

/* BN5's bytes at bus address `a`, `n` of them; NULL where its ROM is not there */
static const uint8_t *bn5(uint32_t a, uint32_t n) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	uint32_t o = a - 0x08000000u;
	return x->data && a >= 0x08000000u && o + n <= ROM_SIZE ? x->data + o : NULL;
}

bool lib_pieces_on(void) { return bn5(BN5_LIB_HUD_FRAME, 32) != NULL; }

/* ---- the Order Points gauge ---- */

/* Each of the gauge's frame tiles: the piece of BN5_LIB_HUD_FRAME BN5
 * copies there (traced: its RAM 0x02039200 on) */
static const uint8_t hud_piece[BAR_FIRST + BAR_TILES] = { 0, 2, 2, 2, 1, 3, 3, 3, 2, 2, 2, 4, 3, 3, 3, 5, 4, 6, 5, 7, 8, 9, 10, 8, 9, 10 };
/* ... and its six OBJs (BN5_LIB_HUD_OAM): x, y, width and height in tiles, the first tile */
static const struct { uint8_t x, y, w, h, tile; } hud_objs[] = {
	{ 162, 0, 4, 2, 0 }, { 194, 0, 4, 2, 8 }, { 224, 0, 2, 2, 16 },
	{ 170, 4, 4, 1, BAR_FIRST }, { 202, 4, 2, 1, BAR_FIRST + 4 }, { 216, 4, 2, 1, COUNT_FIRST },
};

void lib_piece_gauge(int op) {
	const uint8_t *pieces = bn5(BN5_LIB_HUD_FRAME, 11 * 32), *digits = bn5(BN5_LIB_HUD_DIGITS, 10 * 32), *pal = bn5(BN5_LIB_HUD_PAL, 32);
	const uint8_t *masks = bn5(BN5_LIB_HUD_MASKS, (BN5_LIB_OP_MAX + 1) * 8);
	if (!pieces || !digits || !pal || !masks) return;
	if (op < 0) op = 0;
	if (op > BN5_LIB_OP_MAX) op = BN5_LIB_OP_MAX;
	uint8_t t[HUD_TILES][32];
	memset(t, 0, sizeof t);
	for (int i = 0; i < BAR_FIRST + BAR_TILES; ++i) memcpy(t[i], pieces + hud_piece[i] * 32, 32);
	/* the bar: its last (6 OP + 7) / 8 tiles kept, the first of them masked */
	int keep = (6 * op + 7) / 8;
	uint32_t mask = (uint32_t)masks[op * 8] | (uint32_t)masks[op * 8 + 1] << 8 | (uint32_t)masks[op * 8 + 2] << 16 | (uint32_t)masks[op * 8 + 3] << 24;
	for (int i = 0; i < BAR_TILES - keep; ++i) memset(t[BAR_FIRST + i], 0, 32);
	if (keep > 0)
		for (int r = 0; r < 8; ++r)
			for (int b = 0; b < 4; ++b) t[BAR_FIRST + BAR_TILES - keep][r * 4 + b] &= (uint8_t)(mask >> (8 * b));
	/* the count: tens where it has them, then ones */
	if (op >= 10) memcpy(t[COUNT_FIRST], digits + op / 10 * 32, 32);
	memcpy(t[COUNT_FIRST + 1], digits + op % 10 * 32, 32);
	for (size_t i = 0; i < sizeof hud_objs / sizeof *hud_objs; ++i)
		xtiles_draw(t[hud_objs[i].tile], pal, P.core_x + hud_objs[i].x, P.core_y + hud_objs[i].y, hud_objs[i].w, hud_objs[i].h, 0);
}

/* ---- a blow's harm ---- */

#define HARM_UP 29   /* the digits' top over the Navi's feet (BN5_LIB_HARM_PAL) */

void lib_piece_harm(int harm, int x, int y) {
	const uint8_t *digits = bn5(BN5_LIB_DIGITS, 10 * 0x40), *pal = bn5(BN5_LIB_HARM_PAL, 32);
	if (!digits || !pal || harm <= 0) return;
	char s[8];
	int n = snprintf(s, sizeof s, "%d", harm > 9999 ? 9999 : harm);
	for (int i = 0; i < n; ++i)
		xtiles_draw(digits + (uint32_t)(s[i] - '0') * 0x40, pal, P.core_x + x - 4 * n + 8 * i, P.core_y + y - HARM_UP, 1, 2, 0);
}

/* ---- the team's HP ---- */

void lib_piece_hp(int hp, int x, int y) {
	const uint8_t *digits = bn5(BN5_LIB_DIGITS, 10 * 0x40), *pal = bn5(BN5_LIB_DIGITS_PAL, 32);
	if (!digits || !pal) return;
	char s[8];
	int n = snprintf(s, sizeof s, "%d", hp < 0 ? 0 : hp > 9999 ? 9999 : hp);
	for (int i = 0; i < n; ++i)
		xtiles_draw(digits + (uint32_t)(s[i] - '0') * 0x40, pal, P.core_x + x - 4 * n + 8 * i, P.core_y + y - BN5_LIB_HP_DOWN, 1, 2, 0);
}

void lib_piece_hp_box(int hp) {
	const uint8_t *cap = bn5(BN5_LIB_HP_BOX, 0x40), *digits = bn5(BN5_LIB_HP_BOX_DIGITS, 10 * 0x40), *blank = bn5(BN5_LIB_HP_BOX_BLANK, 0x40);
	const uint8_t *pal = bn5(BN5_LIB_HUD_PAL, 32);
	if (!cap || !digits || !blank || !pal) return;
	char s[8];
	snprintf(s, sizeof s, "%4d", hp < 0 ? 0 : hp > 9999 ? 9999 : hp);
	xtiles_draw(cap, pal, P.core_x, P.core_y, 1, 2, 0);
	for (int i = 0; i < 4; ++i)
		xtiles_draw(s[i] == ' ' ? blank : digits + (uint32_t)(s[i] - '0') * 0x40, pal, P.core_x + 8 + 8 * i, P.core_y, 1, 2, 0);
	xtiles_draw(cap, pal, P.core_x + 40, P.core_y, 1, 2, 1);
}

/* ---- the L view ---- */

static void phase_number(int n, int x, int y) {
	const uint8_t *pal = bn5(BN5_LIB_HUD_PAL, 32);
	char s[8];
	int k = 0;
	if (n < 0) n = 0;
	if (n > 99) n = 99;
	if (n >= 10) s[k++] = (char)('0' + n / 10);
	s[k++] = (char)('0' + n % 10);
	for (int i = 0; i < k; ++i)
		xtiles_draw(bn5(BN5_LIB_PHASE_ZERO + (uint32_t)(s[i] - '0') * 32, 32), pal, P.core_x + x + 8 * i, P.core_y + y, 1, 1, 0);
}

/* The L view's arrows at the picture's edges, BN5's (traced: its OAM),
 * by the direction their tiles point: up first, then clockwise */
static const uint8_t arrow_at[8][2] = { { 116, 4 }, { 224, 8 }, { 228, 77 }, { 224, 144 }, { 116, 148 }, { 8, 144 }, { 4, 77 }, { 8, 8 } };

static void lview_arrows(int t) {
	const uint8_t *pal = bn5(BN5_LIB_HUD_PAL, 32), *cross = bn5(BN5_LIB_LVIEW_CROSS, 4 * 32);
	uint32_t look = (t & 31) < 24 ? 0 : BN5_LIB_LVIEW_BLINK;
	for (int k = 0; k < 8; ++k)
		xtiles_draw(bn5(BN5_LIB_LVIEW_ARROWS + look + 32u * (uint32_t)k, 32), pal, P.core_x + arrow_at[k][0], P.core_y + arrow_at[k][1], 1, 1, 0);
	xtiles_draw(cross, pal, P.core_x + 112, P.core_y + 72, 2, 2, 0);
}

void lib_piece_phases(int phase, int par, int t) {
	const uint8_t *words = bn5(BN5_LIB_PHASE_TILES, 9 * 32), *pal = bn5(BN5_LIB_PHASE_PAL, 32), *band = bn5(BN5_LIB_PHASE_WORDS_PAL, 32);
	if (!words || !pal || !band) return;
	/* (the panel behind them: BG0 in WIN1, blended as the banners' band) */
	xtiles_blend_rect(P.core_x + BN5_LIB_LVIEW_PANEL_X, P.core_y + BN5_LIB_LVIEW_PANEL_Y, BN5_LIB_LVIEW_PANEL_W, BN5_LIB_LVIEW_PANEL_H,
		(uint16_t)(band[2 * BN5_LIB_BAND_COLOUR] | band[2 * BN5_LIB_BAND_COLOUR + 1] << 8), 16, 8);
	lview_arrows(t);
	/* "PHASE" its five tiles, "TRGT" the four after them */
	xtiles_draw(words, pal, P.core_x + 54, P.core_y + 124, 5, 1, 0);
	phase_number(phase, 97, 124);
	xtiles_draw(words + 5 * 32, pal, P.core_x + 22, P.core_y + 136, 4, 1, 0);
	xtiles_draw(words, pal, P.core_x + 54, P.core_y + 136, 5, 1, 0);
	phase_number(par, 97, 136);
}

/* ---- the command window ---- */

void lib_piece_window(const char *const *lines, const bool *can, int n, int cursor) {
	const uint8_t *frame = bn5(BN5_LIB_WINDOW_TILES, 9 * 32), *bg = bn5(BN5_LIB_WINDOW_BG_PAL, 32);
	const uint8_t *arrow = bn5(BN5_LIB_CURSOR_TILES, 4 * 32), *arrow_pal = bn5(BN5_LIB_CURSOR_PAL, 32);
	if (!frame || !bg || !arrow || !arrow_pal) return;
	for (int ty = 0; ty < WINDOW_H; ++ty)
		for (int tx = 0; tx < WINDOW_W; ++tx) {
			int col = tx == 0 ? 0 : tx == WINDOW_W - 1 ? 2 : 1, row = ty == 0 ? 0 : ty == WINDOW_H - 1 ? 2 : 1;
			xtiles_draw(frame + (row * 3 + col) * 32, bg, P.core_x + WINDOW_X + tx * 8, P.core_y + WINDOW_Y + ty * 8, 1, 1, 0);
		}
	/* (its words in BN6's own menu font, as BN5's are in its own: white,
	 * a command that cannot be taken here dimmed) */
	for (int i = 0; i < n; ++i)
		look_text(P.core_x + LINE_X, P.core_y + LINE_Y + i * LINE_STEP - 2, lines[i], can[i] ? rgba(240, 248, 240, 255) : rgba(88, 200, 200, 255), rgba(0, 0, 0, 0), TEXT_LEFT);
	if (cursor >= 0 && cursor < n) xtiles_draw(arrow, arrow_pal, P.core_x + CURSOR_X, P.core_y + CURSOR_Y + cursor * LINE_STEP, 2, 2, 0);
}

/* ---- the banners ---- */

/* The affine's vertical scale, frame by frame, before and after its hold
 * at full size (256), BN5's, traced */
static const int16_t squash_in[] = { 768, 640, 512, 384, 256, 208, 224, 256 };
static const int16_t squash_out[] = { 224, 208, 256, 384, 512, 640, 768, 896 };
#define CHEER_HOLD (LIB_CHEER_FRAMES - (int)(sizeof squash_in / sizeof *squash_in) - (int)(sizeof squash_out / sizeof *squash_out))

static int squash(int t) {
	int in = (int)(sizeof squash_in / sizeof *squash_in);
	if (t < in) return squash_in[t];
	if (t < in + CHEER_HOLD) return 256;
	t -= in + CHEER_HOLD;
	return t < (int)(sizeof squash_out / sizeof *squash_out) ? squash_out[t] : 0;
}

/* A column's letter (8x16, two tiles): BN5's battle-message letters */
static const uint8_t *letter(char c) {
	if (c >= 'A' && c <= 'Z') return bn5(BN5_LIB_LETTERS + (uint32_t)(c - 'A') * 64, 64);
	if (c == '!') return bn5(BN5_LIB_LETTER_BANG, 64);
	if (c == '<') return bn5(BN5_LIB_LETTER_CAPS, 64);
	if (c == '>') return bn5(BN5_LIB_LETTER_CAPS + 64, 64);
	return bn5(BN5_LIB_LETTER_SPACE, 64);
}

void lib_piece_cheer(int cheer, int t) {
	/* (20 columns: '<' and '>' the caps) */
	static const char *const words[] = { "", "<LIBERATE SUCCESS!>", "<   TURN LIBERATE! >" };
	static const int left[] = { 0, 44, 40 };
	const uint8_t *pal = bn5(BN5_LIB_BANNER_PAL, 32);
	int pd = squash(t);
	if (cheer <= LIB_CHEER_NONE || cheer > LIB_CHEER_ONE_TURN || !pal || pd <= 0) return;
	const char *w = words[cheer];
	int cols = (int)strlen(w);
	for (int o = 0; o * 4 < cols; ++o) {
		uint8_t tiles[8][32];
		memset(tiles, 0, sizeof tiles);
		for (int c = 0; c < 4 && o * 4 + c < cols; ++c) {
			const uint8_t *l = letter(w[o * 4 + c]);
			if (!l) return;
			memcpy(tiles[c], l, 32);
			memcpy(tiles[4 + c], l + 32, 32);
		}
		xtiles_draw_squashed(tiles[0], pal, P.core_x + left[cheer] + o * 32, P.core_y + 64, 4, 2, pd);
	}
	/* (its turn's digit, 16x16 at (52, 64), the digit in its right column) */
	const uint8_t *one = bn5(BN5_LIB_BIG_DIGITS + 64, 64);
	if (cheer == LIB_CHEER_ONE_TURN && one) {
		uint8_t tiles[4][32];
		memset(tiles, 0, sizeof tiles);
		memcpy(tiles[1], one, 32);
		memcpy(tiles[3], one + 32, 32);
		xtiles_draw_squashed(tiles[0], pal, P.core_x + 52, P.core_y + 64, 2, 2, pd);
	}
}

/* ---- the phases' banners ---- */

/* The words' OBJs as BN5 lays them (traced: their OAM held in place, x
 * from the picture's left, y 64 or 72), each a block of the words' tiles
 * (BN5_LIB_PHASE_WORDS' tile n, OBJ tile 719 + n) */
typedef struct { uint8_t x, dy, w, h, tile, flip; } WordObj;
static const WordObj darkloid_phase[] = {
	{ 14, 0, 2, 2, 30, 0 },
	{ 30, 0, 1, 1, 9, 0 }, { 30, 8, 1, 1, 10, 0 }, { 38, 0, 1, 1, 9, 1 }, { 38, 8, 1, 1, 10, 1 },
	{ 46, 0, 1, 2, 19, 0 }, { 54, 0, 1, 2, 17, 0 },
	{ 62, 0, 2, 2, 34, 0 },
	{ 78, 0, 2, 2, 38, 0 },
	{ 94, 0, 1, 2, 42, 1 }, { 102, 0, 1, 2, 42, 0 },
	{ 112, 0, 1, 1, 44, 0 }, { 112, 8, 1, 1, 45, 0 },
	{ 120, 0, 2, 2, 30, 0 },
	{ 146, 0, 2, 2, 21, 0 },
	{ 162, 0, 1, 1, 25, 0 }, { 162, 8, 1, 1, 10, 0 }, { 170, 0, 1, 1, 25, 1 }, { 170, 8, 1, 1, 10, 1 },
	{ 178, 0, 1, 1, 9, 0 }, { 178, 8, 1, 1, 10, 0 }, { 186, 0, 1, 1, 9, 1 }, { 186, 8, 1, 1, 10, 1 },
	{ 194, 0, 2, 2, 26, 0 },
	{ 210, 0, 2, 2, 13, 0 },
};
static const WordObj player_phase[] = {
	{ 10, 0, 4, 2, 1, 0 },
	{ 42, 0, 1, 1, 9, 0 }, { 42, 8, 1, 1, 10, 0 }, { 50, 0, 1, 1, 9, 1 }, { 50, 8, 1, 1, 10, 1 },
	{ 58, 0, 1, 2, 11, 0 }, { 66, 0, 1, 2, 11, 1 },
	{ 74, 0, 2, 2, 13, 0 },
	{ 90, 0, 1, 2, 19, 0 }, { 98, 0, 1, 2, 17, 0 },
	{ 114, 0, 2, 2, 21, 0 },
	{ 130, 0, 1, 1, 25, 0 }, { 130, 8, 1, 1, 10, 0 }, { 138, 0, 1, 1, 25, 1 }, { 138, 8, 1, 1, 10, 1 },
	{ 146, 0, 1, 1, 9, 0 }, { 146, 8, 1, 1, 10, 0 }, { 154, 0, 1, 1, 9, 1 }, { 154, 8, 1, 1, 10, 1 },
	{ 162, 0, 2, 2, 26, 0 },
	{ 178, 0, 2, 2, 13, 0 },
};
#define WORDS_Y      64
#define DIGITS_X     200    /* PLAYER PHASE's two digits, 16 apart */
#define WORD_TILES   165

/* The words' tiles, decompressed from BN5's ROM once (a u32 of their size first) */
static const uint8_t *word_tiles(void) {
	static uint8_t *tiles;
	static const uint8_t *from;
	const uint8_t *lz = bn5(BN5_LIB_PHASE_WORDS, 4);
	if (!lz) return NULL;
	if (from != lz) {
		free(tiles);
		size_t n = 0;
		tiles = lz77_decompress(lz, ROM_SIZE - (BN5_LIB_PHASE_WORDS - 0x08000000u), &n);
		if (tiles && n < 4 + WORD_TILES * 32) { free(tiles); tiles = NULL; }
		from = lz;
	}
	return tiles ? tiles + 4 : NULL;
}

int lib_piece_banner_frames(void) { return lib_pieces_on() && word_tiles() ? BN5_LIB_BANNER_END : 0; }

/* The band's rows as WIN1 shows them: a row at its middle for two frames,
 * then 17, then BN5_LIB_BAND_ROWS; closing through 16 rows and none */
static void band_rows(int t, int *top, int *rows) {
	int mid = BN5_LIB_BAND_TOP + BN5_LIB_BAND_ROWS / 2;
	*top = BN5_LIB_BAND_TOP;
	*rows = BN5_LIB_BAND_ROWS;
	if (t < 2) { *top = mid; *rows = 1; }
	else if (t == 2) { *top = mid - 8; *rows = 17; }
	else if (t == BN5_LIB_BANNER_END - 2) { *top = mid - 8; *rows = 16; }
	else if (t >= BN5_LIB_BANNER_END - 1) *rows = 0;
}

/* The words' offset from their place, `t` frames after the band opened:
 * in from 240 to the right, 32 a frame, held, then out to the left; false
 * while they are not shown */
static bool words_offset(int t, int *dx) {
	int in = (240 + BN5_LIB_SLIDE_STEP - 1) / BN5_LIB_SLIDE_STEP, k = t - BN5_LIB_WORDS_AT;
	if (k < 0) return false;
	if (k < in) { *dx = 240 - k * BN5_LIB_SLIDE_STEP; if (*dx < 0) *dx = 0; return true; }
	k -= in + BN5_LIB_WORDS_HOLD;
	if (k < 0) { *dx = 0; return true; }
	*dx = -(k + 1) * BN5_LIB_SLIDE_STEP;
	return *dx > -240;
}

static void word_obj(const uint8_t *tiles, const uint8_t *pal, const WordObj *o, int dx) {
	int x = o->x + dx;
	if (x >= 240 || x + o->w * 8 <= 0 || o->tile + o->w * o->h > WORD_TILES) return;
	xtiles_draw(tiles + o->tile * 32, pal, P.core_x + x, P.core_y + WORDS_Y + o->dy, o->w, o->h, o->flip);
}

void lib_piece_phase_banner(bool darkloid, int phase, int t) {
	const uint8_t *tiles = word_tiles(), *pal = bn5(BN5_LIB_PHASE_WORDS_PAL + (darkloid ? 32u : 0u), 32);
	if (!tiles || !pal || t < 0 || t >= BN5_LIB_BANNER_END) return;
	int top, rows, dx;
	band_rows(t, &top, &rows);
	if (rows) xtiles_blend_rect(P.core_x, P.core_y + top, 240, rows, (uint16_t)(pal[2 * BN5_LIB_BAND_COLOUR] | pal[2 * BN5_LIB_BAND_COLOUR + 1] << 8), 16, 8);
	if (!rows || !words_offset(t, &dx)) return;
	/* (a letter half past the picture's left edge cut there, as the GBA's screen cuts it) */
	SDL_Rect core = { P.core_x, P.core_y, 240, 160 };
	gfx_clip(&core);
	const WordObj *w = darkloid ? darkloid_phase : player_phase;
	int n = darkloid ? (int)(sizeof darkloid_phase / sizeof *darkloid_phase) : (int)(sizeof player_phase / sizeof *player_phase);
	for (int i = 0; i < n; ++i) word_obj(tiles, pal, &w[i], dx);
	if (!darkloid) {
		if (phase < 0) phase = 0;
		if (phase > 99) phase = 99;
		WordObj d = { DIGITS_X, 0, 2, 2, (uint8_t)(BN5_LIB_PHASE_DIGIT + 4 * (phase / 10)), 0 };
		word_obj(tiles, pal, &d, dx);
		d.x = DIGITS_X + 16;
		d.tile = (uint8_t)(BN5_LIB_PHASE_DIGIT + 4 * (phase % 10));
		word_obj(tiles, pal, &d, dx);
	}
	gfx_clip(NULL);
}

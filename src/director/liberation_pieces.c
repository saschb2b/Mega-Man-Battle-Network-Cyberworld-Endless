/* BN5's own pieces of a liberation's board (liberation_pieces.h), each
 * laid out as BN5 lays its OBJs and BG0 (traced in BN5: docs/ROM_DATA.md,
 * BN5's Liberation board drawn in BN6), its tiles and colours read from
 * the player's BN5 ROM. Positions are the GBA picture's. */
#include "liberation_pieces.h"

#include <stdint.h>
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

void lib_piece_phases(int phase, int par) {
	const uint8_t *words = bn5(BN5_LIB_PHASE_TILES, 9 * 32), *pal = bn5(BN5_LIB_PHASE_PAL, 32);
	if (!words || !pal) return;
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

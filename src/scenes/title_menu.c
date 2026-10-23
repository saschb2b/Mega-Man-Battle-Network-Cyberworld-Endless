/* title_menu.h. BN6's title has NEW GAME and CONTINUE and nothing more:
 * OPTIONS is set from their letters and PRESS START's (letters.h), read
 * from the player's ROM the first time the menu is drawn. */
#include "title_menu.h"

#include <stdio.h>
#include <string.h>

#include "controls.h"
#include "game.h"
#include "gfx.h"
#include "letters.h"
#include "minifont.h"
#include "platform.h"
#include "rom.h"
#include "saves.h"
#include "text.h"

#define T (R.layout->title)
#define ROW_X 88          /* the rows' left, BN6's */
#define ARROW_X 73        /* the arrow's */
#define LABEL_X 170       /* CONTINUE's word on where the run goes on */
#define HINT_Y 154        /* the word on the keys: under the copyright line */
#define ENTER_Y 138
#define WORD_W 64         /* OPTIONS' pixels across, at most */
#define WORD_H 16
#define LETTERS_FILL 1    /* the title's letters' colour in their palettes */
#define PRESS_SHADE 2     /* PRESS START's shade in its palette, */
#define MENU_SHADE 3      /* ... the menu's in its */
#define PRESS_W 128       /* PRESS START: four OBJs of 32x16 from tile 1 */
#define CONTINUE_W 80     /* CONTINUE: 32x16, 32x16 and 16x16 from tile 55 */
#define NEW_GAME 35
#define CONTINUE 55

int title_menu_rows(bool has_save) { return has_save ? 3 : 2; }

/* A row's top in the picture: BN6's own two places, three closer from
 * higher up (the copyright line under them) */
static int row_top(int row, int rows) { return rows <= 2 ? 112 + 16 * row : 102 + 14 * row; }

/* The title's text tile n (its OBJ tile n: the block's first is 1) */
static uint32_t text_tile(int n) { return gfx_lz_ref(T.text_tiles) + 4 + (uint32_t)(n - 1) * 32; }

/* OBJs of `w[i]` x 2 tiles side by side from tile `first`, as colour
 * indices; false where a tile is missing */
static bool strip(int first, const int *w, int n, uint8_t *px, int width) {
	int x0 = 0;
	for (int o = 0; o < n; ++o) {
		for (int t = 0; t < 2 * w[o]; ++t) {
			const uint8_t *b = gfx_tile_bytes(text_tile(first + t));
			if (!b) return false;
			int tx = x0 + (t % w[o]) * 8, ty = (t / w[o]) * 8;
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) px[(ty + y) * width + tx + x] = (uint8_t)(b[y * 4 + x / 2] >> (x & 1 ? 4 : 0) & 15);
		}
		x0 += 8 * w[o];
		first += 2 * w[o];
	}
	return true;
}

/* OPTIONS in the menu's palette, its width; 0 where the letters are not
 * the ones read (PRESS START cut into 10, CONTINUE into 8) */
static int options_word(uint32_t argb[WORD_H][WORD_W]) {
	static uint8_t press[WORD_H][PRESS_W], cont[WORD_H][CONTINUE_W], word[WORD_H][WORD_W];
	static const int press_w[] = { 4, 4, 4, 4 }, cont_w[] = { 4, 4, 2 };
	if (!strip(1, press_w, 4, &press[0][0], PRESS_W) || !strip(CONTINUE, cont_w, 3, &cont[0][0], CONTINUE_W)) return 0;
	LetterStrip p = { &press[0][0], PRESS_W, WORD_H, LETTERS_FILL }, c = { &cont[0][0], CONTINUE_W, WORD_H, LETTERS_FILL };
	int at[16][2];
	if (letters_cut(&p, at, 16) != 10 || letters_cut(&c, at, 16) != 8) return 0;
	uint8_t from_press[16] = { 0 }, from_cont[16] = { 0 };
	from_press[LETTERS_FILL] = from_cont[LETTERS_FILL] = LETTERS_FILL;
	from_press[PRESS_SHADE] = from_cont[MENU_SHADE] = MENU_SHADE;
	/* (PRESS START stands higher in its OBJs than the menu's words) */
	int dy = letters_top(&c) - letters_top(&p);
	/* O, P, T, I, O, N, S: CONTINUE's letter n, or PRESS START's */
	static const struct { bool press; int n; } picks[] = {
		{ false, 1 }, { true, 0 }, { false, 3 }, { false, 4 }, { false, 1 }, { false, 2 }, { true, 3 },
	};
	memset(word, 0, sizeof word);
	LetterWord w = { &word[0][0], WORD_W, WORD_H };
	int x = 0;
	for (size_t i = 0; i < sizeof picks / sizeof *picks && x >= 0; ++i)
		x = picks[i].press ? letters_put(&p, picks[i].n, &w, x, dy, from_press) : letters_put(&c, picks[i].n, &w, x, 0, from_cont);
	if (x <= 0) return 0;
	const uint8_t *pal = R.data + T.menu_pal;
	for (int y = 0; y < WORD_H; ++y)
		for (int i = 0; i < WORD_W; ++i)
			argb[y][i] = word[y][i] ? bgr555((uint16_t)(pal[word[y][i] * 2] | pal[word[y][i] * 2 + 1] << 8)) : 0;
	return x;
}

/* OPTIONS' row: the word as a texture, made once; the game's battle
 * font where the letters are not found */
static void options_row(int x, int y) {
	static SDL_Texture *tex;
	static int w;
	static bool tried;
	if (!tried) {
		tried = true;
		static uint32_t argb[WORD_H][WORD_W];
		w = options_word(argb);
		if (w) tex = SDL_CreateTexture(P.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, WORD_H);
		if (tex) {
			SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
			SDL_UpdateTexture(tex, NULL, argb, WORD_W * 4);
		}
	}
	if (!tex) {
		text_draw(x, y + 2, "OPTIONS", WHITE, TEXT_LEFT);
		return;
	}
	SDL_Rect d = { x, y, w, WORD_H };
	SDL_RenderCopy(P.renderer, tex, NULL, &d);
}

/* CONTINUE's word on where the saved run goes on: its layer, or home (a
 * run saved at home said "Layer 7" to a playtester saved in AsterLand,
 * session 73) */
static void continue_label(const TitleMenu *m, int x, int y) {
	if (!m->has_save || !m->saved_depth) return;
	if (m->saved_home) text_draw(x, y, "Home", WHITE, TEXT_LEFT);
	else text_drawf(x, y, WHITE, TEXT_LEFT, "Layer %d", m->saved_depth);
}

void title_menu_draw(int x0, int y0, const TitleMenu *m) {
	uint32_t text = text_tile(0);
	int rows = title_menu_rows(m->has_save);
	/* NEW GAME (tiles 35-54) and CONTINUE (55-74): 32x16, 32x16, 16x16 */
	for (int i = 0; i < rows - 1; ++i) {
		uint32_t first = i == 0 ? NEW_GAME : CONTINUE;
		int y = y0 + row_top(i, rows);
		rom_tiles(text + first * 32, T.menu_pal, x0 + ROW_X, y, 4, 2, 0);
		rom_tiles(text + (first + 8) * 32, T.menu_pal, x0 + ROW_X + 32, y, 4, 2, 0);
		rom_tiles(text + (first + 16) * 32, T.menu_pal, x0 + ROW_X + 64, y, 2, 2, 0);
	}
	options_row(x0 + ROW_X, y0 + row_top(rows - 1, rows));
	continue_label(m, x0 + LABEL_X, y0 + row_top(1, rows) + 2);
	/* the arrow cycles three frames, 6 frames each */
	int f = (m->frame / 6) % 3;
	if (!m->asking) rom_tiles(T.arrow + (uint32_t)f * 4 * 32, T.arrow_pal, x0 + ARROW_X, y0 + row_top(m->cursor, rows) + 1, 2, 2, 0);
}

/* (with the menu: SAVES on L, kept off the menu's ROM-drawn rows, and the
 * ROMs on R where the launcher is, issue #97: no way to add BN5 had been
 * found; each on the player's own button, controls_word's "L", "Q") */
void title_hint_draw(int x0, int y0, bool menu, bool pressed, bool over) {
	SDL_Color grey = rgba(150, 160, 190, 255);
	if (!menu && !pressed && controls_keyboard_only()) minifont_draw_centered(x0 + CORE_W / 2, y0 + ENTER_Y, "ENTER", grey, 1);
	if (!menu || over) return;
	char hint[80];
#ifdef __3DS__
	snprintf(hint, sizeof hint, "L: %s", saves_word_hint());
#else
	/* (controls_word's own buffer, one word at a time) */
	char r[32];
	snprintf(r, sizeof r, "%s", controls_word(BTN_R));
	snprintf(hint, sizeof hint, "%s: %s", controls_word(BTN_L), saves_word_hint());
	if (launcher_here()) {
		size_t m = strlen(hint);
		snprintf(hint + m, sizeof hint - m, "  %s: ROMs", r);
	}
#endif
	minifont_draw_centered(x0 + CORE_W / 2, y0 + HINT_Y, hint, grey, 1);
}

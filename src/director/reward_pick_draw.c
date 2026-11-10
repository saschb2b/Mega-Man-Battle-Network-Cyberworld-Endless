/* The reward pick's picture (reward_pick.h), laid out as BN6's Library,
 * whose card a hardcore player reads at a glance: the row under the cursor
 * on the Library's card at the left (a chip's picture, its code as the
 * folder editor's card sets it, its element, power and description; a
 * program's shape in its colour where a chip's picture stands; HPMemory's
 * and the BugFrags' sums), the choices in the window at the right, a row
 * each, the skip's row under them, and the information box under the
 * window, KeyItem's, saying what the row adds. The screen's name on the
 * band. Drawing changes nothing. */
#include "reward_pick.h"

#include <stdio.h>
#include <string.h>

#include "bn6_chips.h"
#include "chips.h"
#include "data.h"
#include "director.h"
#include "gfx.h"
#include "library_draw.h"
#include "library_pages.h"
#include "meta.h"
#include "navicust.h"
#include "pet_look.h"
#include "platform.h"
#include "reward_pick_state.h"
#include "reward_pick_text.h"
#include "second_art.h"
#include "second_card.h"

/* where it draws, from the picture's corner: the Library's card, the
 * window of four rows (three choices and the skip) beside it, the
 * information box under the window to the body's foot */
#define CARD_X 7
#define CARD_Y 17
#define WIN_X 92
#define WIN_Y 17
#define WIN_W (CORE_W - WIN_X - 7)
#define ROW_H 16
#define WIN_H (2 * LOOK_FRAME + 4 + 4 * ROW_H)
#define INFO_Y (WIN_Y + WIN_H + 3)
#define INFO_H (CORE_H - LOOK_RAIL - 2 - INFO_Y)
#define CODE_INK rgba(255, 231, 0, 255)   /* a chip's code, yellow as BN6 sets it */

/* ---- the card ---- */

/* a program's colour variant (program * 4 + v) of colour `color` */
static int variant_of(int program, int color) {
	for (int v = 0; v < 4; ++v) {
		int kind, c;
		if (navicust_part(program * 4 + v, &kind, &c) && c == color) return program * 4 + v;
	}
	return program * 4;
}

/* a sum in the card's head, in heavy capitals, its label over it */
static void head_sum(int x, int y, const char *label, const char *sum) {
	look_caps(x + 40 - look_caps_width(label) / 2, y + 22, label, LOOK_PAGE, rgba(0, 0, 0, 0));
	look_title(x + 40 - look_title_width(sum) / 2, y + 32, sum, LOOK_GOLD);
}

static void chip_card(int x, int y, const PickChoice *c) {
	const uint8_t *rec = chips_record(c->id);
	int cls = rec && rec[BN6_CHIP_LIBRARY_TYPE] < LIB_CLASSES ? rec[BN6_CHIP_LIBRARY_TYPE] : LIB_STD;
	library_card(x, y, c->id, cls);
	/* (its code at the head's foot left of the element, as the folder
	 * editor's card sets it, where the Library's has none) */
	char code[2] = { c->code >= 26 ? '*' : (char)('A' + c->code), 0 };
	text_draw_cells(x + 12, y + 53, code, CODE_INK, LOOK_CARD_DARK);
}

static void program_card(int x, int y, const PickChoice *c) {
	library_card_blank(x, y, LIB_STD);
	NaviShape s;
	int v = variant_of(c->id, c->code), part = NAVI_PART, color = c->code;
	if (navicust_shape(v, &s)) second_program_shape(&s, x + 7 + (66 - SECOND_SHAPE_W) / 2, y + 2 + (66 - SECOND_SHAPE_W) / 2);
	navicust_part(v, &part, &color);
	char lines[64];
	snprintf(lines, sizeof lines, "%s\n%s", navicust_color_name(c->code), reward_pick_part(part));
	if (lines[0] >= 'a' && lines[0] <= 'z') lines[0] = (char)(lines[0] - 'a' + 'A');
	library_card_lines(x, y, lines);
}

static void card(int x, int y) {
	if (RP.cursor >= RP.o.n) {
		/* (the skip's row: its BugFrags) */
		char sum[12];
		snprintf(sum, sizeof sum, "%d", RP.o.frags);
		library_card_blank(x, y, LIB_STD);
		head_sum(x, y, "BUGFRAGS", sum);
		return;
	}
	const PickChoice *c = &RP.o.c[RP.cursor];
	if (c->what == PICK_CHIP) chip_card(x, y, c);
	else if (c->what == PICK_PROGRAM) program_card(x, y, c);
	else {
		char sum[12], lines[32];
		snprintf(sum, sizeof sum, "+%d", c->id * 20);
		library_card_blank(x, y, LIB_STD);
		head_sum(x, y, "MAX HP", sum);
		snprintf(lines, sizeof lines, "%s\nx%d", reward_pick_hp_name(), c->id);
		library_card_lines(x, y, lines);
	}
}

/* ---- the list ---- */

/* a row's name and its mark after it (a chip's code, HPMemory's count) */
static void row_words(int x, int y, const char *name, const char *mark) {
	text_draw_cells(x, y + 1, name, LOOK_TEXT, LOOK_SHADE);
	if (mark && *mark) text_draw_cells(x + 8 * ((int)strlen(name) + 1), y + 1, mark, CODE_INK, LOOK_SHADE);
}

static void row(SDL_Rect f, int r) {
	int y = f.y + 2 + r * ROW_H, x = f.x + 26;
	char name[24], mark[12] = "";
	if (r >= RP.o.n) {
		snprintf(mark, sizeof mark, "%d", RP.o.frags);
		row_words(x, y, reward_pick_skip_name(), mark);
	} else {
		const PickChoice *c = &RP.o.c[r];
		if (c->what == PICK_CHIP) {
			second_chip_icon(c->id, f.x + 8, y - 1);
			chips_name(c->id, name, sizeof name);
			mark[0] = c->code >= 26 ? '*' : (char)('A' + c->code);
			mark[1] = 0;
		} else if (c->what == PICK_PROGRAM) {
			second_color_swatch(c->code, f.x + 11, y + 3, 7);
			program_name(c->id, name, sizeof name);
		} else {
			snprintf(name, sizeof name, "%s", reward_pick_hp_name());
			snprintf(mark, sizeof mark, "x%d", c->id);
		}
		row_words(x, y, name, mark);
	}
	if (r == RP.cursor) look_cursor(f.x - 3, y + 2);
}

/* ---- the information box ---- */

static void info(int x, int y) {
	SDL_Rect r = look_info(x, y, WIN_W, INFO_H, reward_pick_label());
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	char s[200];
	if (RP.cursor >= RP.o.n) reward_pick_skip_info(s, sizeof s, RP.o.frags);
	else {
		const PickChoice *c = &RP.o.c[RP.cursor];
		if (c->what == PICK_CHIP) reward_pick_chip_info(s, sizeof s, meta_library_has(c->id));
		else if (c->what == PICK_PROGRAM) {
			/* (MegaMan's words of what it does, its name left off) */
			const char *about = navicust_about(c->id), *colon = about ? strchr(about, ':') : NULL;
			reward_pick_program_info(s, sizeof s, colon ? colon + 1 + (colon[1] == ' ') : about ? about : "", c->held);
			if (s[0] >= 'a' && s[0] <= 'z') s[0] = (char)(s[0] - 'a' + 'A');
		} else reward_pick_hp_info(s, sizeof s, c->id);
	}
	look_wrapped(s, r.x + 5, r.y + 1, r.w - 9, 3, LOOK_INK, LOOK_INK_SHADE);
}

void director_draw_pick(void) {
	if (!RP.up) return;
	int x0 = P.core_x, y0 = P.core_y;
	look_screen(x0, y0, CORE_W, CORE_H, reward_pick_title(RP.kind));
	card(x0 + CARD_X, y0 + CARD_Y);
	SDL_Rect f = look_window(x0 + WIN_X, y0 + WIN_Y, WIN_W, WIN_H, 0, false);
	for (int r = 0; r < reward_pick_rows(); ++r) row(f, r);
	info(x0 + WIN_X, y0 + INFO_Y);
	look_fade_in(x0, y0, CORE_W, CORE_H, RP.t);
}

/* The team pick's picture (liberation_pick.h), as the JACK-IN SETUP is
 * drawn (title_setup.c; docs/UI.md, Ours): BN6's screen frame over the
 * board, its band TEAM with the count of places filled as FOLDER EDIT's
 * 30/30, the held Navis in the window, the picked marked by the ChipFolder's
 * EQUIP corners, GO! under it as NaviCust's RUN!; at the right what the
 * cursor's Navi brings: his face, HP and his battle form's weakness in the
 * NAVIGATOR box's shape, that form, his command with its cost on a tag and
 * its shape on a slot's small grid of panels, faced right as BN5's board is
 * first faced. Drawing changes nothing. */
#include "liberation_pick.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "liberation_pick_state.h"
#include "liberation_pick_text.h"
#include "liberation_state.h"
#include "liberation_team.h"
#include "liberation_words.h"
#include "pet_look.h"
#include "pet_panel.h"
#include "platform.h"

/* where it draws, from the picture's corner: the list's window on the
 * left, wide enough for TomahawkMan in the chat font, its rows the font's
 * cell apart, GO! under it; at the right the Navi's box (the NAVIGATOR
 * box's shape: his face beside his HP and weakness), his battle form's and
 * his command's */
#define WIN_X 7
#define WIN_Y 17
#define WIN_W 112
#define ROW_H 13
#define WIN_H (2 * LOOK_FRAME + 4 + PICK_ROWS_SHOWN * ROW_H)
#define ROW_Y (WIN_Y + LOOK_FRAME + 2)
#define CURSOR_X 11
#define LABEL_X 26
#define GO_Y (WIN_Y + WIN_H + 4)
#define GO_H 13
#define SIDE_X (WIN_X + WIN_W + 3)
#define SIDE_W (CORE_W - SIDE_X - 7)
#define NAVI_Y WIN_Y
#define FORM_Y (NAVI_Y + LOOK_NAV_H + 2)
#define FORM_H 24
#define CMD_Y (FORM_Y + FORM_H + 2)
#define CMD_H (CORE_H - LOOK_RAIL - 1 - CMD_Y)
/* the command's grid: panels CELL wide, PITCH apart, GRID_W by GRID_H,
 * the Navi's panel in its first column (in its middle for a shield) */
#define CELL 5
#define PITCH 6
#define GRID_W 8
#define GRID_H 3

/* The Navi the information is about: the cursor's, or on GO! the first
 * picked */
static int shown_navi(void) {
	if (LP.cursor < LP.n) return LP.navi[LP.cursor];
	for (int i = 0; i < LP.n; ++i)
		if (LP.order[i] == 1) return LP.navi[i];
	return LP.n ? LP.navi[0] : MATE_MEGAMAN;
}

/* ---- the list ---- */

static void rows_draw(int x0, int y0) {
	SDL_Rect win = { x0 + WIN_X, y0 + WIN_Y, WIN_W, WIN_H };
	bool bar = LP.n > PICK_ROWS_SHOWN;
	look_window(win.x, win.y, win.w, win.h, 0, bar);
	for (int r = 0; r < PICK_ROWS_SHOWN && LP.first + r < LP.n; ++r) {
		int i = LP.first + r, y = y0 + ROW_Y + r * ROW_H;
		const char *name = lib_navi_name(LP.navi[i]);
		look_text(x0 + LABEL_X, y, name, LP.order[i] ? LOOK_GOLD : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		/* (a Navi picked: what is one's own, as the ChipFolder's EQUIP
		 * corners, round his name) */
		if (LP.order[i]) look_corners(x0 + LABEL_X - 4, y - 1, look_text_width(name) + 8, ROW_H + 1);
		if (i == LP.cursor) look_cursor(x0 + CURSOR_X, y + 2);
	}
	look_scroll(win, 0, LP.first, PICK_ROWS_SHOWN, LP.n);
}

/* GO!: NaviCust's RUN!, heavy white capitals on the bright green, the green
 * triangle before it when chosen; dimmed till every place is filled */
static void go_draw(int x0, int y0) {
	const char *s = lib_pick_word(PW_GO);
	int w = look_title_width(s) + 18, x = x0 + WIN_X + (WIN_W - w) / 2, y = y0 + GO_Y;
	look_button(x, y, w, GO_H);
	bool ready = lib_pick_count() == LP.places;
	look_title(x + 10, y + 4, s, rgba(41, 132, 148, 255));
	look_title(x + 9, y + 3, s, ready ? LOOK_PAGE : LOOK_DIM);
	if (LP.cursor == LP.n) look_choice(x - LOOK_CHOICE_W - 3, y);
}

/* ---- the Navi ---- */

/* The NAVIGATOR box's shape: his name for its label, his face on the left,
 * the white page with his HP and the element his battle form is weak to */
static void navi_draw(int x0, int y0, int navi) {
	char label[24], hp[16];
	snprintf(label, sizeof label, "%s", lib_navi_name(navi));
	for (char *c = label; *c; ++c) *c = (char)toupper((unsigned char)*c);
	int x = x0 + SIDE_X, y = y0 + NAVI_Y;
	look_info(x, y, SIDE_W, LOOK_NAV_H, label);
	SDL_Rect page = { x + 43, y + 8, SIDE_W - 45, 48 };
	fill_rect(page.x, page.y, page.w, page.h, LOOK_PAGE);
	fill_rect(x + 3, y + 8, 40, 48, LOOK_SLOT);
	Sprite *mug = lib_navi_face(navi);
	if (mug) sprite_draw_frame(mug, 0, 0, x + 3 + 20, y + 8 + 24, false, 0, 0);
	const char *weak = lib_navi_weakness(navi);
	const char *lines[3] = { lib_pick_hp(hp, sizeof hp, lib_navi_hp(navi, LB.b.act)), lib_pick_word(PW_WEAKNESS),
		weak && weak[0] ? weak : lib_pick_word(PW_NONE) };
	for (int l = 0; l < 3; ++l)
		look_text(page.x + 4, page.y + 1 + l * LOOK_NAV_LINE, lines[l], l == 1 ? LOOK_DIM : LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
}

/* The information box labelled BATTLE: the form his battles are fought in */
static void form_draw(int x0, int y0, int navi) {
	SDL_Rect r = look_info(x0 + SIDE_X, y0 + FORM_Y, SIDE_W, FORM_H, lib_pick_word(PW_BATTLE));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	look_text(r.x + 6, r.y, lib_navi_form(navi), LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
}

/* ---- his command ---- */

static void cell(int gx, int gy, int cx, int cy, SDL_Color c) {
	if (cx < 0 || cy < 0 || cx >= GRID_W || cy >= GRID_H) return;
	fill_rect(gx + cx * PITCH, gy + cy * PITCH, CELL, CELL, c);
}

/* The panels command `command` takes, faced right from the Navi's panel
 * (lib_command_panels on a board all dark round him), on a slot: the dark
 * panels round him, those the command takes lit, his own gold; KnightMan's
 * StoneBody the four panels beside him */
static void shape_draw(int gx, int gy, int command) {
	static LibBoard scratch;
	static bool made;
	if (!made) {
		memset(scratch.kind, LIB_DARK, sizeof scratch.kind);
		made = true;
	}
	bool shield = command == LIB_DO_NONE;
	int col = shield ? GRID_W / 2 - 1 : 0, row = GRID_H / 2, sx = MAP_W / 2, sy = MAP_H / 2;
	for (int cy = 0; cy < GRID_H; ++cy)
		for (int cx = 0; cx < GRID_W; ++cx) cell(gx, gy, cx, cy, LOOK_SHADE);
	if (shield) {
		static const int beside[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
		for (int d = 0; d < 4; ++d) cell(gx, gy, col + beside[d][0], row + beside[d][1], LOOK_EDGE_HI);
	} else {
		int xs[6], ys[6], n = lib_command_panels(&scratch, command, sx + 1, sy, 0, xs, ys);
		for (int j = 0; j < n; ++j) cell(gx, gy, col + xs[j] - sx, row + ys[j] - sy, LOOK_EDGE_HI);
	}
	cell(gx, gy, col, row, LOOK_GOLD);
}

/* The information box labelled COMMAND, its cost on its tag as KeyItem's
 * box carries KEYITEM: its name, its shape under it */
static void command_draw(int x0, int y0, int navi) {
	int command = lib_command_of(navi), x = x0 + SIDE_X, y = y0 + CMD_Y;
	SDL_Rect r = look_info(x, y, SIDE_W, CMD_H, lib_pick_word(PW_COMMAND));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	/* (no command: KnightMan's StoneBody, always on; another's none) */
	bool stone = command == LIB_DO_NONE && navi == MATE_KNIGHTMAN;
	if (command == LIB_DO_NONE && !stone) return;
	const char *tag = lib_pick_command_note(command);
	int tw = look_caps_width(tag) + 8;
	look_tag(x + SIDE_W - tw - 3, y + 1, tw, 7, tag, LOOK_BAND);
	const char *name = stone ? lib_pick_word(PW_STONEBODY) : lib_command_label(LIB_CMD_LONGSWRD, command);
	look_text(r.x + 6, r.y, name, LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
	int sw = GRID_W * PITCH + 5, sh = GRID_H * PITCH + 5;
	SDL_Rect f = look_slot(r.x + (r.w - sw) / 2, r.y + r.h - sh - 2, sw, sh);
	shape_draw(f.x + 1, f.y + 1, command);
}

void lib_pick_draw(void) {
	int x0 = P.core_x, y0 = P.core_y;
	char count[12];
	pet_panel_draw(x0, y0, lib_pick_word(PW_TITLE));
	bool full = lib_pick_count() == LP.places;
	look_band_note(x0, y0, CORE_W, lib_pick_places(count, sizeof count, lib_pick_count(), LP.places), full ? LOOK_GOLD : LOOK_PAGE);
	rows_draw(x0, y0);
	go_draw(x0, y0);
	int navi = shown_navi();
	navi_draw(x0, y0, navi);
	form_draw(x0, y0, navi);
	command_draw(x0, y0, navi);
	look_fade_in(x0, y0, CORE_W, CORE_H, LP.t);
}

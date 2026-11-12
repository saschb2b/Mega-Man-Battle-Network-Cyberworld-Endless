/* The Battle Lab's builder's picture (lab.h), in BN6's PET look
 * (docs/engine/ui.md), laid over BN6's own battle of the set, held with
 * its field up: the field, MegaMan and the foes are the game's, not
 * drawn here. The pages as Options' tabs, the page's list in a window,
 * what the cursor's item is in SubChip's flat panel, BATTLE! as
 * NaviCust's RUN!, the panel under the cursor in EQUIP's corners.
 * Drawing changes nothing. */
#include "lab.h"

#include <stdio.h>
#include <string.h>

#include "battle_data.h"
#include "bn6.h"
#include "bn6_panels.h"
#include "data.h"
#include "director.h"
#include "gfx.h"
#include "lab_navis.h"
#include "lab_stage.h"
#include "lab_state.h"
#include "lab_text.h"
#include "pet_look.h"
#include "platform.h"
#include "save.h"

/* where it draws, over BN6's own battle held with its field up (lab_battle.h):
 * the pages' tabs across the top; then, on FOES, STAGE and SETS, a panel
 * over MegaMan's side as BN6's Custom screen stands (the list, what the
 * cursor's item is, the button), the foes' side and all that stands on it
 * left clear; on FIELD a strip under the tabs alone, so that all eighteen
 * panels show. The panels are BN6's, at its places on the screen. */
#define BAND_H 16
#define PANEL_W 40           /* a battle panel on BN6's screen: its width, its rows' height, its first row's top */
#define PANEL_H 24
#define FIELD_TOP 72
#define SIDE_W 119           /* the panel over MegaMan's side */
#define WIN_X 3
#define WIN_Y (BAND_H + 3)
#define WIN_W (SIDE_W - 6)
#define ROW_H 13
#define WIN_H(rows) (2 * LOOK_FRAME + 4 + (rows) * ROW_H)
#define INFO_H (2 * LOOK_LINE + 2)
#define GO_H 12
#define FIELD_WIN_W 130      /* ... its row's window, the button and the count after it */
#define STRIP_H 46           /* FIELD's strip under the tabs: its one row, its line of information */

/* a foe's name in its version (a Navi's with LAB_NAVI in it) */
static void foe_name(int family, int version, char *out, size_t n) {
	out[0] = 0;
	if (version & LAB_NAVI) { lab_navi_name(family, version & ~LAB_NAVI, out, n); return; }
	int id = enemy_id(0, family, version);
	if (id >= 0) enemy_name(id, out, n);
}

static int row_version(const LabRow *r) { return r->version | (r->navi ? LAB_NAVI : 0); }

/* ... of the panel under the cursor: the foe on it, the one lifted over it, or its kind */
static void info_field(char *s, size_t n, const LabRow *r) {
	char name[24] = "";
	int foe = lab_foe_at(LS.col, LS.line);
	if (foe >= 0) foe_name(LS.set.foes[foe].family, LS.set.foes[foe].version, name, sizeof name);
	if (LS.lifted && foe < 0 && r && !r->navi && lab_set_navi()) lab_info_alone(s, n);
	else if (LS.lifted && foe < 0 && r) {
		foe_name(r->family, row_version(r), name, sizeof name);
		lab_info_lifted(s, n, name, r->navi ? 0 : LS.set.nfoes, LAB_FOES);
	} else lab_info_panel(s, n, lab_panel_name(LS.set.panels[LS.line * LAB_COLS + LS.col]), name);
}

/* ... of the roster's row: a family's or a Navi's in its version, or its gauge */
static void info_row(char *s, size_t n, const LabRow *r) {
	int hp = 0, damage = 0;
	if (r->navi && r->versions) lab_info_navi(s, n, lab_navi_hp(r->family, r->version), r->count);
	else if (r->navi) lab_info_gauge(s, n, r->count, lab_navi_need(r->family));
	else if (r->versions) {
		enemy_stats(enemy_id(0, r->family, r->version), &hp, &damage);
		lab_info_family(s, n, hp, r->count);
	} else lab_info_gauge(s, n, r->count, DATA_COMPLETE);
}

static void info_text(char *s, size_t n) {
	s[0] = 0;
	const LabRow *r = LS.nroster ? &LS.roster[LS.row[LAB_PAGE_FOES]] : NULL;
	if (LS.at == LAB_AT_GO) {
		if (!LS.set.nfoes) lab_info_empty(s, n);
		else if (LS.last.end != LAB_NONE) lab_info_result(s, n, &LS.last);
	} else if (LS.at == LAB_AT_FIELD) info_field(s, n, r);
	else if (LS.page == LAB_PAGE_SETS) {
		const LabSaved *k = &profile.lab_sets[LS.row[LAB_PAGE_SETS] % LAB_SETS];
		lab_info_set(s, n, k->set.nfoes, k->bust, k->frames, k->hp_lost);
	} else if (LS.page == LAB_PAGE_STAGE) {
		/* (the row's value in full: an area's or a song's name is wider than its row) */
		if (LS.row[LAB_PAGE_STAGE] == LAB_ROW_FOLDER) lab_info_folder(s, n, profile.lab_folder_on);
		else if (LS.row[LAB_PAGE_STAGE]) lab_stage_song_name(LS.set.song, s, n);
		else lab_stage_area_name(LS.set.area, s, n);
	} else if (LS.page == LAB_PAGE_FOES && r) info_row(s, n, r);
}

/* what the cursor's item is, in SubChip's flat panel: `lines` of it */
static void info_draw(int x, int y, int w, int lines) {
	char s[96];
	info_text(s, sizeof s);
	SDL_Rect r = look_panel(x, y, w, lines * LOOK_LINE + 2);
	look_wrapped(s, r.x + 4, r.y - 1, r.w - 8, lines, LOOK_PAGE, LOOK_PANEL_SHADE);
}

/* a row's value at the right, BN6's small arrows either side where Left and Right turn it */
static void value_draw(SDL_Rect f, int y, const char *v, bool turns, SDL_Color c) {
	int right = f.x + f.w - 9, w = look_text_width(v);
	look_text(right, y, v, c, LOOK_SHADE, TEXT_RIGHT);
	if (!turns) return;
	look_pointer(right - w - 8, y + 3, 5, true, LOOK_ORANGE);
	look_pointer(right + 3, y + 3, 5, false, LOOK_ORANGE);
}

/* a row of the page's list: FOES a family in its version (or its gauge), FIELD and STAGE a name and its value */
static void row_draw(SDL_Rect f, int i, int y, bool under) {
	char v[24], name[24];
	if (LS.page == LAB_PAGE_FOES) {
		const LabRow *r = &LS.roster[i];
		foe_name(r->family, row_version(r), name, sizeof name);
		look_text(f.x + 11, y, name, r->versions ? LOOK_TEXT : LOOK_DIM, LOOK_SHADE, TEXT_LEFT);
		if (r->versions) value_draw(f, y, r->navi ? lab_navi_version_word(r->family, r->version) : lab_version_word(r->version), under && LS.at == LAB_AT_LIST, LOOK_TEXT);
		else {
			lab_count_words(v, sizeof v, r->count, r->navi ? lab_navi_need(r->family) : DATA_COMPLETE);
			value_draw(f, y, v, false, LOOK_DIM);
		}
	} else if (LS.page == LAB_PAGE_FIELD) {
		look_text(f.x + 14, y, lab_row_preset, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		if (LS.preset) lab_preset_words(v, sizeof v, LS.preset);
		value_draw(f, y, LS.preset ? v : lab_plain_field, under && LS.at == LAB_AT_LIST, LOOK_TEXT);
	} else if (LS.page == LAB_PAGE_SETS) {
		/* (the set in hand in gold, as what is one's own; its foes as they stand now) */
		bool mine = i == profile.lab_slot % LAB_SETS;
		lab_set_words(name, sizeof name, i);
		look_text(f.x + 14, y, name, mine ? LOOK_GOLD : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		lab_set_value(v, sizeof v, mine ? LS.set.nfoes : profile.lab_sets[i].set.nfoes);
		look_text(f.x + f.w - 4, y, v, LOOK_TEXT, LOOK_SHADE, TEXT_RIGHT);
	} else {
		/* (what the row turns, its value in the information: a song's name is wider than the row) */
		look_text(f.x + 14, y, i == LAB_ROW_FOLDER ? lab_row_folder : i ? lab_row_song : lab_row_area, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		if (under && LS.at == LAB_AT_LIST) {
			look_pointer(f.x + f.w - 14, y + 3, 5, true, LOOK_ORANGE);
			look_pointer(f.x + f.w - 7, y + 3, 5, false, LOOK_ORANGE);
		}
	}
	if (under && LS.at == LAB_AT_LIST) look_cursor(f.x + 2, y + 2);
}

/* the page's list in a window at (x, y), `shown` rows of it; its height */
static int list_draw(int x, int y, int w, int shown) {
	SDL_Rect win = { x, y, w, WIN_H(shown) };
	SDL_Rect f = look_window(win.x, win.y, win.w, win.h, 0, false);
	int rows = lab_rows(), at = LS.row[LS.page], top = LS.page == LAB_PAGE_FOES ? LS.top : at >= shown ? at - shown + 1 : 0;
	for (int i = top; i < rows && i < top + shown; ++i) row_draw(f, i, f.y + 2 + (i - top) * ROW_H, i == at);
	if (rows > shown) look_scroll(win, 0, top, shown, rows);
	return win.h;
}

/* the button at (x, y), the count of foes set after it */
static void go_draw(int x, int y) {
	int w = look_title_width(lab_go) + 18;
	look_button(x + LOOK_CHOICE_W + 2, y, w, GO_H);
	look_title(x + LOOK_CHOICE_W + 12, y + 3, lab_go, rgba(41, 132, 148, 255));
	look_title(x + LOOK_CHOICE_W + 11, y + 2, lab_go, LOOK_PAGE);
	if (LS.at == LAB_AT_GO) look_choice(x, y);
	char count[8];
	lab_count_words(count, sizeof count, LS.set.nfoes, LAB_FOES);
	look_text(x + LOOK_CHOICE_W + w + 8, y - 1, count, LOOK_PAGE, LOOK_PANEL_SHADE, TEXT_LEFT);
}

void director_draw_lab(void) {
	if (!LS.up) return;
	int x0 = P.core_x, y0 = P.core_y;
	fill_rect(x0, y0, CORE_W, BAND_H, LOOK_BAND);
	look_tabs(x0 + 3, y0 + 1, CORE_W - 6, LAB_PAGES, LS.page, lab_tabs);
	if (LS.page == LAB_PAGE_FIELD) {
		/* (all eighteen panels in sight: the page's one row and its line in a strip under the tabs) */
		fill_rect(x0, y0 + BAND_H, CORE_W, STRIP_H, LOOK_GREEN);
		fill_rect(x0, y0 + BAND_H + STRIP_H, CORE_W, 1, LOOK_GREEN_LO);
		int h = list_draw(x0 + WIN_X, y0 + WIN_Y, FIELD_WIN_W, 1);
		go_draw(x0 + WIN_X + FIELD_WIN_W + 2, y0 + WIN_Y + 7);
		info_draw(x0 + WIN_X, y0 + WIN_Y + h + 2, CORE_W - 6, 1);
	} else {
		fill_rect(x0, y0 + BAND_H, SIDE_W, CORE_H - BAND_H, LOOK_GREEN);
		fill_rect(x0 + SIDE_W, y0 + BAND_H, 2, CORE_H - BAND_H, LOOK_METAL);
		fill_rect(x0 + SIDE_W + 2, y0 + BAND_H, 1, CORE_H - BAND_H, LOOK_METAL_LO);
		int h = list_draw(x0 + WIN_X, y0 + WIN_Y, WIN_W, LAB_ROWS_SHOWN);
		info_draw(x0 + WIN_X, y0 + WIN_Y + h + 3, WIN_W, 2);
		go_draw(x0 + WIN_X, y0 + WIN_Y + h + 3 + INFO_H + 5);
	}
	/* (the panel under the cursor, BN6's own, in EQUIP's corners) */
	if (LS.at == LAB_AT_FIELD) look_corners(x0 + LS.col * PANEL_W, y0 + FIELD_TOP + LS.line * PANEL_H, PANEL_W, PANEL_H);
}

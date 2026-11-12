/* The Battle Lab's builder's picture (lab.h), in BN6's PET look
 * (docs/engine/ui.md): the band with the count of foes set in its pill,
 * the three pages as Options' tabs, the field's eighteen panels in a slot
 * with the foes set on them, KeyItem's INFORMATION box beside it, the
 * page's list in the window under them, and BATTLE! as NaviCust's RUN!.
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
#include "guardians.h"
#include "jukebox_text.h"
#include "lab_navis.h"
#include "lab_state.h"
#include "lab_text.h"
#include "pet_look.h"
#include "platform.h"
#include "save.h"

/* where it draws, from the picture's corner */
#define TABS_X 7
#define TABS_Y 16
#define TABS_W (CORE_W - 14)
#define CELL_W 16
#define CELL_H 16
#define FIELD_X 8
#define FIELD_Y 34
#define FIELD_W (LAB_COLS * CELL_W + 4)
#define FIELD_H (LAB_ROWS * CELL_H + 4)
#define INFO_X (FIELD_X + FIELD_W + 3)
#define INFO_W (CORE_W - INFO_X - 7)
#define WIN_X 7
#define WIN_Y (FIELD_Y + FIELD_H + 2)
#define WIN_W (CORE_W - 14)
#define ROW_H 13
#define WIN_H (2 * LOOK_FRAME + 4 + LAB_ROWS_SHOWN * ROW_H)
#define GO_H 12
#define GO_Y (CORE_H - LOOK_RAIL - GO_H - 1)

/* a panel's colour: its side's where it is plain, its type's otherwise */
static SDL_Color panel_ink(int type, bool foes) {
	switch (type) {
	case BN6_PANEL_HOLE: return rgba(8, 24, 41, 255);
	case BN6_PANEL_POISON: return rgba(148, 66, 198, 255);
	case BN6_PANEL_HOLY: return rgba(255, 231, 99, 255);
	case BN6_PANEL_GRASS: return rgba(57, 189, 57, 255);
	case BN6_PANEL_ICE: return rgba(181, 239, 255, 255);
	case BN6_PANEL_VOLCANO: return rgba(214, 82, 24, 255);
	default: break;
	}
	if (type >= BN6_PANEL_CONVEYOR && type <= BN6_PANEL_CONVEYOR_END) return rgba(140, 148, 165, 255);
	return foes ? rgba(66, 132, 231, 255) : rgba(231, 90, 74, 255);
}

static void panel_draw(int x, int y, int type, bool foes) {
	fill_rect(x, y, CELL_W - 1, CELL_H - 1, panel_ink(type, foes));
	fill_rect(x, y, CELL_W - 1, 1, rgba(255, 255, 255, 70));
	/* (cracked: a break across it; a conveyor: which way it runs) */
	if (type == BN6_PANEL_CRACKED)
		for (int k = 0; k < 9; ++k) fill_rect(x + 3 + k, y + 2 + k, 2, 1, rgba(41, 41, 66, 255));
	if (type == BN6_PANEL_CONVEYOR_LEFT || type == BN6_PANEL_CONVEYOR_RIGHT) look_pointer(x + 4, y + 3, 7, type == BN6_PANEL_CONVEYOR_LEFT, LOOK_PAGE);
	if (type == BN6_PANEL_CONVEYOR_UP || type == BN6_PANEL_CONVEYOR_DOWN)
		for (int k = 0; k < 4; ++k) fill_rect(x + 7 - k, y + (type == BN6_PANEL_CONVEYOR_UP ? 4 + k : 9 - k), 1 + 2 * k, 1, LOOK_PAGE);
}

/* the field: MegaMan on his panel, each foe by its place in the set */
static void field_draw(int x0, int y0) {
	SDL_Rect s = look_slot(x0 + FIELD_X, y0 + FIELD_Y, FIELD_W, FIELD_H);
	for (int line = 0; line < LAB_ROWS; ++line)
		for (int col = 0; col < LAB_COLS; ++col) {
			int x = s.x + col * CELL_W, y = s.y + line * CELL_H, foe = lab_foe_at(col, line);
			panel_draw(x, y, LS.set.panels[line * LAB_COLS + col], col >= LAB_COLS / 2);
			char mark[2] = { foe >= 0 ? (char)('1' + foe) : col == 1 && line == 1 ? 'M' : 0, 0 };
			if (mark[0]) look_text(x + CELL_W / 2 - 1, y + 1, mark, LOOK_PAGE, LOOK_CARD_INK, TEXT_CENTER);
			if (LS.at == LAB_AT_FIELD && col == LS.col && line == LS.line) look_corners(x - 1, y - 1, CELL_W + 1, CELL_H + 1);
		}
}

/* a foe's name in its version (a Navi's with LAB_NAVI in it) */
static void foe_name(int family, int version, char *out, size_t n) {
	out[0] = 0;
	if (version & LAB_NAVI) { lab_navi_name(family, version & ~LAB_NAVI, out, n); return; }
	int id = enemy_id(0, family, version);
	if (id >= 0) enemy_name(id, out, n);
}

static int row_version(const LabRow *r) { return r->version | (r->navi ? LAB_NAVI : 0); }

static void info_text(char *s, size_t n) {
	s[0] = 0;
	if (LS.at == LAB_AT_GO) {
		if (!LS.set.nfoes) lab_info_empty(s, n);
		else if (LS.last.end != LAB_NONE) lab_info_result(s, n, &LS.last);
		return;
	}
	if (LS.at == LAB_AT_LIST && LS.page == LAB_PAGE_SETS) {
		const LabSaved *k = &profile.lab_sets[LS.row[LAB_PAGE_SETS] % LAB_SETS];
		lab_info_set(s, n, k->set.nfoes, k->bust, k->frames, k->hp_lost);
		return;
	}
	const LabRow *r = LS.nroster ? &LS.roster[LS.row[LAB_PAGE_FOES]] : NULL;
	char name[24] = "";
	if (LS.at == LAB_AT_FIELD) {
		int foe = lab_foe_at(LS.col, LS.line);
		if (foe >= 0) foe_name(LS.set.foes[foe].family, LS.set.foes[foe].version, name, sizeof name);
		if (LS.lifted && foe < 0 && r && !r->navi && lab_set_navi()) lab_info_alone(s, n);
		else if (LS.lifted && foe < 0 && r) {
			foe_name(r->family, row_version(r), name, sizeof name);
			lab_info_lifted(s, n, name, r->navi ? 0 : LS.set.nfoes, LAB_FOES);
		} else lab_info_panel(s, n, lab_panel_name(LS.set.panels[LS.line * LAB_COLS + LS.col]), name);
	} else if (LS.page == LAB_PAGE_FOES && r) {
		int hp = 0, damage = 0;
		if (r->navi && r->versions) lab_info_navi(s, n, lab_navi_hp(r->family, r->version), r->count);
		else if (r->navi) lab_info_gauge(s, n, r->count, lab_navi_need(r->family));
		else if (r->versions) {
			enemy_stats(enemy_id(0, r->family, r->version), &hp, &damage);
			lab_info_family(s, n, hp, r->count);
		} else lab_info_gauge(s, n, r->count, DATA_COMPLETE);
	}
}

static void info_draw(int x0, int y0) {
	SDL_Rect r = look_info(x0 + INFO_X, y0 + FIELD_Y, INFO_W, FIELD_H, lab_info_label);
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	char s[96];
	info_text(s, sizeof s);
	look_wrapped(s, r.x + 4, r.y + 1, r.w - 8, (r.h - 2) / LOOK_LINE, LOOK_INK, LOOK_INK_SHADE);
}

/* a row's value at the right, BN6's small arrows either side where Left and Right turn it */
static void value_draw(SDL_Rect f, int y, const char *v, bool turns, SDL_Color c) {
	int right = f.x + f.w - 12, w = look_text_width(v);
	look_text(right, y, v, c, LOOK_SHADE, TEXT_RIGHT);
	if (!turns) return;
	look_pointer(right - w - 8, y + 3, 5, true, LOOK_ORANGE);
	look_pointer(right + 3, y + 3, 5, false, LOOK_ORANGE);
}

static const char *song_name(int song) {
	for (int i = 0; i < jukebox_song_count; ++i)
		if (jukebox_songs[i].game == 0 && jukebox_songs[i].song == song) return jukebox_songs[i].title;
	return lab_own_song;
}

/* a row of the page's list: FOES a family in its version (or its gauge), FIELD and STAGE a name and its value */
static void row_draw(SDL_Rect f, int i, int y, bool under) {
	char v[24], name[24];
	if (LS.page == LAB_PAGE_FOES) {
		const LabRow *r = &LS.roster[i];
		foe_name(r->family, row_version(r), name, sizeof name);
		look_text(f.x + 14, y, name, r->versions ? LOOK_TEXT : LOOK_DIM, LOOK_SHADE, TEXT_LEFT);
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
		value_draw(f, y, v, false, LOOK_TEXT);
	} else {
		look_text(f.x + 14, y, i ? lab_row_song : lab_row_area, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		value_draw(f, y, i ? song_name(LS.set.song) : guardian_area_short(LS.set.area), under && LS.at == LAB_AT_LIST, LOOK_TEXT);
	}
	if (under && LS.at == LAB_AT_LIST) look_cursor(f.x + 2, y + 2);
}

static void list_draw(int x0, int y0) {
	SDL_Rect win = { x0 + WIN_X, y0 + WIN_Y, WIN_W, WIN_H };
	SDL_Rect f = look_window(win.x, win.y, win.w, win.h, 0, false);
	int rows = lab_rows(), top = LS.page == LAB_PAGE_FOES ? LS.top : LS.row[LS.page] >= LAB_ROWS_SHOWN ? LS.row[LS.page] - LAB_ROWS_SHOWN + 1 : 0;
	for (int i = top; i < rows && i < top + LAB_ROWS_SHOWN; ++i) row_draw(f, i, f.y + 2 + (i - top) * ROW_H, i == LS.row[LS.page]);
	if (rows > LAB_ROWS_SHOWN) look_scroll(win, 0, top, LAB_ROWS_SHOWN, rows);
}

static void go_draw(int x0, int y0) {
	int w = look_title_width(lab_go) + 18, x = x0 + CORE_W - 8 - w, y = y0 + GO_Y;
	look_button(x, y, w, GO_H);
	look_title(x + 10, y + 3, lab_go, rgba(41, 132, 148, 255));
	look_title(x + 9, y + 2, lab_go, LOOK_PAGE);
	if (LS.at == LAB_AT_GO) look_choice(x - LOOK_CHOICE_W - 3, y);
}

void director_draw_lab(void) {
	if (!LS.up) return;
	int x0 = P.core_x, y0 = P.core_y;
	look_screen(x0, y0, CORE_W, CORE_H, lab_title);
	char count[8];
	lab_count_words(count, sizeof count, LS.set.nfoes, LAB_FOES);
	look_band_note(x0, y0, CORE_W, count, LOOK_PAGE);
	look_tabs(x0 + TABS_X, y0 + TABS_Y, TABS_W, LAB_PAGES, LS.page, lab_tabs);
	field_draw(x0, y0);
	info_draw(x0, y0);
	list_draw(x0, y0);
	go_draw(x0, y0);
	look_fade_in(x0, y0, CORE_W, CORE_H, LS.t);
}

/* title_setup.h. Drawn as Options is (options_draw.c): the paused title
 * under BN6's screen frame, the band with the screen's name, the rows in
 * the window, the information on the row under the cursor in KeyItem's
 * box. Drawing changes nothing. */
#include "title_setup.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "meta.h"
#include "pet_panel.h"
#include "powers.h"
#include "run.h"
#include "save.h"
#include "title_setup_text.h"

enum { ROW_NET, ROW_FOLDER, ROW_CROSS, ROW_THREAT, ROW_HELPERS, ROW_GO, ROWS };

/* where it draws, from the picture's corner: the rows in a window with
 * JACK IN! at its foot, as NaviCust's RUN! at its list's, and the
 * information under it to the screen's foot. (The rows 12 apart, the chat
 * font's cell, so that the box keeps three lines: a Cross's two and "No
 * other Cross this run") */
#define WIN_Y 17
#define WIN_H 86
#define ROW_Y 23
#define ROW_H 12
#define PAGE_ROW_H 14   /* the helpers' page: five lines at most */
#define LABEL_X 24
#define CURSOR_X 12
#define VALUE_R (CORE_W - 30)   /* a value's right edge, its arrow after it */
#define GO_Y 84
#define GO_H 13
#define NOTE_Y 104
#define NOTE_H 50
#define NOTE_LINES 3
#define SHUT_MAX 48   /* a line of what is still shut, and how */
#define SHUT_LINES 4

static struct {
	bool shown, helpers_page;
	int row;      /* the cursor's row */
	int helper;   /* ... its line on the helpers' page */
	int t;        /* frames since the page came: its fade */
	SetupChoice c;
} S;

/* the rows the last summary's unlocks are on (profile.setup_new) */
static int row_new(int row) {
	static const int bits[ROWS] = { SETUP_NEW_NET, SETUP_NEW_FOLDER, SETUP_NEW_CROSS, SETUP_NEW_THREAT, SETUP_NEW_HELP, 0 };
	return profile.setup_new & bits[row];
}

void title_setup_reset(void) { S.c = (SetupChoice){ .net = RUN_SHORT, .folder = FOLDER_STANDARD }; }

bool title_setup_shown(void) { return S.shown; }

const SetupChoice *title_setup_choice(void) { return &S.c; }

void title_setup_open(void) {
	S.shown = true;
	S.helpers_page = false;
	S.t = 0;
	S.row = ROW_GO;
	meta_unlocks_unsaid();
	/* (on the row of what the last run opened, as a new unlock is spent
	 * here: a playtester's A went through a setup that showed none of his
	 * new SlashCross start, the cursor on JACK IN!; else JACK IN!, for the
	 * run again as before) */
	for (int r = ROW_GO - 1; r >= 0; --r) if (row_new(r)) S.row = r;
	S.c.net = profile.last_net == RUN_ENDLESS && meta_endless_open() ? RUN_ENDLESS : RUN_SHORT;
	S.c.folder = meta_folder_open(profile.last_folder) ? profile.last_folder : FOLDER_STANDARD;
	S.c.cross = meta_cross_open(profile.last_cross) ? profile.last_cross : 0;
	S.c.threat = profile.last_threat <= meta_threat_open() ? profile.last_threat : meta_threat_open();
	S.c.helpers = profile.last_helpers & ((1 << meta_helpers_shown()) - 1);
	/* (on BeastOut where it is the news: the Help row's only kind, the
	 * helper a profile earns, issue #99) */
	S.helper = row_new(ROW_HELPERS) && meta_beast_open() ? HELPER_BEAST : 0;
}

/* ---- its keys ---- */

static int value_of(int row) {
	return row == ROW_NET ? S.c.net : row == ROW_FOLDER ? S.c.folder : row == ROW_CROSS ? S.c.cross : S.c.threat;
}

/* Left or Right on a row with a value: the next open one that way */
static void turn(int d) {
	int was = value_of(S.row);
	if (S.row == ROW_NET && meta_endless_open()) S.c.net = S.c.net == RUN_SHORT ? RUN_ENDLESS : RUN_SHORT;
	if (S.row == ROW_FOLDER)   /* (the open ones only) */
		for (int k = 1; k < FOLDER_COUNT; ++k) {
			int f = (S.c.folder + d * k + FOLDER_COUNT * k) % FOLDER_COUNT;
			if (meta_folder_open(f)) { S.c.folder = f; break; }
		}
	if (S.row == ROW_CROSS)   /* (none, or an open one) */
		for (int k = 1; k < 6; ++k) {
			int c = (S.c.cross + d * k + 6 * k) % 6;
			if (!c || meta_cross_open(c)) { S.c.cross = c; break; }
		}
	if (S.row == ROW_THREAT) S.c.threat = (S.c.threat + d + meta_threat_open() + 1) % (meta_threat_open() + 1);
	if (value_of(S.row) != was) audio_sfx(SFX_CURSOR);
}

/* jacking in with these: they are the next setup's to start from */
static bool go(void) {
	profile.last_net = (uint8_t)S.c.net;
	profile.last_folder = (uint8_t)S.c.folder;
	profile.last_cross = (uint8_t)S.c.cross;
	profile.last_threat = (uint8_t)S.c.threat;
	profile.last_helpers = (uint8_t)S.c.helpers;
	profile.setup_new = 0;
	profile_save();
	S.shown = false;
	return true;
}

/* the helpers' page: each on or off, as Options' Display turns its lines;
 * B back to the rows */
static bool helpers_update(void) {
	int n = meta_helpers_shown();
	if (btn_pressed(BTN_START)) return go();
	if (btn_pressed(BTN_B)) {
		S.helpers_page = false;
		S.t = 0;
		audio_sfx(SFX_CANCEL);
		return false;
	}
	if (btn_repeat(BTN_UP)) { S.helper = (S.helper + n - 1) % n; audio_sfx(SFX_CURSOR); }
	if (btn_repeat(BTN_DOWN)) { S.helper = (S.helper + 1) % n; audio_sfx(SFX_CURSOR); }
	if (btn_pressed(BTN_A) || btn_pressed(BTN_LEFT) || btn_pressed(BTN_RIGHT)) {
		S.c.helpers ^= 1 << S.helper;
		audio_sfx(btn_pressed(BTN_A) ? SFX_SELECT : SFX_CURSOR);
	}
	return false;
}

bool title_setup_update(void) {
	++S.t;
	if (S.helpers_page) return helpers_update();
	if (btn_pressed(BTN_B)) { S.shown = false; audio_sfx(SFX_CANCEL); return false; }
	if (btn_repeat(BTN_UP)) { S.row = (S.row + ROWS - 1) % ROWS; audio_sfx(SFX_CURSOR); }
	if (btn_repeat(BTN_DOWN)) { S.row = (S.row + 1) % ROWS; audio_sfx(SFX_CURSOR); }
	int d = btn_repeat(BTN_RIGHT) ? 1 : btn_repeat(BTN_LEFT) ? -1 : 0;
	if (d && S.row < ROW_HELPERS) turn(d);
	if (btn_pressed(BTN_START)) return go();
	if (!btn_pressed(BTN_A)) return false;
	if (S.row == ROW_HELPERS) {
		S.helpers_page = true;
		S.t = 0;
		audio_sfx(SFX_SELECT);
		return false;
	}
	return go();   /* (A on another row: jack in with these) */
}

/* ---- the information ---- */

/* the Crosses a run may bring: the Cross row's arrows and its note */
static int crosses_open(void) {
	int n = 0;
	for (int c = 1; c <= 5; ++c) n += meta_cross_open(c);
	return n;
}

/* the Cross row's note, made into `buf` where it is: the Cross's strength
 * and its costs, or how Crosses open; that no other Cross comes, or that
 * more open, into `shut` */
static const char *cross_note(char *buf, size_t n, char shut[][SHUT_MAX], int *nshut) {
	int c = S.c.cross, open = crosses_open();
	const char *weak = c ? powers_cross_weakness(c) : NULL;
	if (c) snprintf(shut[(*nshut)++], SHUT_MAX, "%s", setup_word(SW_CROSS_ONE));
	else if (open && open < 5) snprintf(shut[(*nshut)++], SHUT_MAX, "%s", setup_word(SW_CROSS_MORE));
	if (weak) return setup_cross_note(buf, n, powers_cross_strength(c), weak);
	return setup_word(c ? SW_CROSS_CHOSEN_NOTE : open ? SW_CROSS_GUARDIANS_NOTE : SW_CROSS_NONE_NOTE);
}

/* what the row under the cursor means, a line or two (made into `buf`
 * where it is made), and under it, dim, what is still shut and how, or
 * what else to know (into `shut`) */
static const char *row_note(char *buf, size_t n, char shut[][SHUT_MAX], int *nshut) {
	/* (and what else it changes: All *'s Program Advances, on in the same
	 * lines, as the box has three) */
	if (S.helpers_page) return setup_helper_note(buf, n, meta_helper(S.helper)->about, meta_helper(S.helper)->more);
	switch (S.row) {
	case ROW_NET:
		return setup_word(S.c.net == RUN_ENDLESS ? SW_ENDLESS_NOTE : meta_endless_open() ? SW_SHORT_NOTE : SW_SHORT_WIN_NOTE);
	case ROW_FOLDER:
		/* (every folder still to open, and how: the telegraph comes first) */
		for (int f = 1; f < FOLDER_COUNT && *nshut < SHUT_LINES; ++f)
			if (!meta_folder_open(f) && meta_folder(f)->opens)
				setup_shut_folder(shut[(*nshut)++], SHUT_MAX, meta_folder(f)->name, meta_folder(f)->opens);
		return meta_folder(S.c.folder)->about;
	case ROW_CROSS: return cross_note(buf, n, shut, nshut);
	case ROW_THREAT:
		if (S.c.threat) return meta_threat_rule(S.c.threat);
		return setup_word(meta_threat_open() ? SW_THREAT0_NOTE : SW_THREAT0_WIN_NOTE);
	case ROW_HELPERS: return setup_word(SW_HELP_NOTE);
	default: return setup_word(SW_GO_NOTE);
	}
}

/* KeyItem's information box: its label, its white page, the note in the
 * chat font's ink and what is shut under it, dim, each where it fits
 * whole: BN6's box says its piece in three lines at most (docs/UI.md).
 * (The chat font is wide: "Storm: delete an Aqua guardian" takes two, so
 * Blade's two lines of style leave it to Standard's note, where the
 * folders still shut are told in their order) */
static void note_draw(int x0, int y0) {
	char buf[128], shut[SHUT_LINES][SHUT_MAX];
	int nshut = 0;
	const char *note = row_note(buf, sizeof buf, shut, &nshut);
	SDL_Rect r = look_info(x0 + 7, y0 + NOTE_Y, CORE_W - 14, NOTE_H, setup_word(SW_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	int x = r.x + 6, w = r.w - 12, end = r.y + 1 + NOTE_LINES * LOOK_LINE;
	int y = look_wrapped(note, x, r.y + 1, w, NOTE_LINES, LOOK_INK, LOOK_INK_SHADE);
	for (int i = 0; i < nshut; ++i) {
		int lines = look_text_width(shut[i]) <= w ? 1 : 2;
		if (y + lines * LOOK_LINE > end) break;
		y = look_wrapped(shut[i], x, y, w, lines, LOOK_DIM, LOOK_INK_SHADE);
	}
}

/* ---- the rows ---- */

/* what the last run opened, beside its row: NEW as BN6 marks a new mail,
 * orange capitals over their dark red */
static void new_draw(int x, int y) {
	look_caps(x, y + 3, setup_word(SW_NEW), rgba(255, 173, 41, 255), rgba(165, 49, 74, 255));
}

/* a row's name, NEW after it where `fresh` */
static void label_draw(int x0, int y, const char *s, bool fresh) {
	look_text(x0 + LABEL_X, y, s, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	if (fresh) new_draw(x0 + LABEL_X + look_text_width(s) + 5, y);
}

/* a value at the right, BN6's small orange arrows either side where the
 * d-pad turns it (as Options' lines) */
static void value_draw(int x0, int y, const char *v, bool turns) {
	int r = x0 + VALUE_R, l = r - look_text_width(v);
	look_text(l, y, v, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	if (!turns) return;
	look_pointer(l - 5, y, 4, true, PET_ORANGE);
	look_pointer(r + 4, y, 4, false, PET_ORANGE);
}

/* the Help row's value, as Options' sections show theirs before their
 * chevron: the helpers on by name, their count where the names run long */
static void helpers_value(int x0, int y, bool on) {
	char names[64] = "", count[16];
	int n = 0;
	for (int h = 0; h < meta_helpers_shown(); ++h)
		if (S.c.helpers >> h & 1) {
			size_t m = strlen(names);
			snprintf(names + m, sizeof names - m, "%s%s", m ? " " : "", meta_helper(h)->name);
			++n;
		}
	int room = VALUE_R - LABEL_X - look_text_width(setup_word(SW_HELP)) - 32;
	const char *v = !n ? setup_word(SW_NONE) : look_text_width(names) <= room ? names : setup_helpers_on(count, sizeof count, n);
	look_text(x0 + VALUE_R - look_text_width(v), y, v, LOOK_EDGE_HI, LOOK_SHADE, TEXT_LEFT);
	look_pointer(x0 + VALUE_R + 6, y + 1, 3, false, on ? LOOK_GOLD : LOOK_DIM);
}

/* JACK IN!: NaviCust's RUN!, heavy white capitals on the bright green,
 * the green triangle before it when chosen */
static void go_draw(int x0, int y0, bool on) {
	const char *s = setup_word(SW_JACK_IN);
	int w = look_title_width(s) + 18, x = x0 + (CORE_W - w) / 2, y = y0 + GO_Y;
	look_button(x, y, w, GO_H);
	look_title(x + 10, y + 4, s, rgba(41, 132, 148, 255));
	look_title(x + 9, y + 3, s, LOOK_PAGE);
	if (on) look_choice(x - LOOK_CHOICE_W - 3, y);
}

static void rows_draw(int x0, int y0) {
	static const SetupWord names[ROW_GO] = { SW_NET, SW_FOLDER, SW_CROSS, SW_THREAT, SW_HELP };
	for (int r = 0; r < ROW_GO; ++r) label_draw(x0, y0 + ROW_Y + r * ROW_H, setup_word(names[r]), row_new(r));
	int open_folders = 0;
	for (int f = 0; f < FOLDER_COUNT; ++f) open_folders += meta_folder_open(f);
	char v[16];
	snprintf(v, sizeof v, "%d", S.c.threat);
	const char *values[ROW_HELPERS] = { setup_word(S.c.net == RUN_ENDLESS ? SW_ENDLESS : SW_SHORT), meta_folder(S.c.folder)->name,
		S.c.cross ? powers_cross_name(S.c.cross) : setup_word(SW_NONE), v };
	bool turns[ROW_HELPERS] = { meta_endless_open(), open_folders > 1, crosses_open() > 0, meta_threat_open() > 0 };
	for (int r = 0; r < ROW_HELPERS; ++r) value_draw(x0, y0 + ROW_Y + r * ROW_H, values[r], turns[r]);
	helpers_value(x0, y0 + ROW_Y + ROW_HELPERS * ROW_H, S.row == ROW_HELPERS);
	go_draw(x0, y0, S.row == ROW_GO);
	/* (BN6's lists keep a line's colour under the cursor: the arrow alone says where it is) */
	if (S.row < ROW_GO) look_cursor(x0 + CURSOR_X, y0 + ROW_Y + S.row * ROW_H + 2);
}

/* the helpers' page: each by name, On or Off between the arrows */
static void helpers_draw(int x0, int y0) {
	for (int h = 0; h < meta_helpers_shown(); ++h) {
		int y = y0 + ROW_Y + h * PAGE_ROW_H;
		label_draw(x0, y, meta_helper(h)->name, h == HELPER_BEAST && row_new(ROW_HELPERS));
		value_draw(x0, y, setup_word(S.c.helpers >> h & 1 ? SW_ON : SW_OFF), true);
		if (h == S.helper) look_cursor(x0 + CURSOR_X, y + 2);
	}
}

void title_setup_draw(int x0, int y0) {
	pet_panel_draw(x0, y0, setup_word(S.helpers_page ? SW_HELP_TITLE : SW_TITLE));
	look_window(x0 + 7, y0 + WIN_Y, CORE_W - 14, WIN_H, 0, false);
	if (S.helpers_page) helpers_draw(x0, y0);
	else rows_draw(x0, y0);
	note_draw(x0, y0);
	look_fade_in(x0, y0, CORE_W, CORE_H, S.t);
}

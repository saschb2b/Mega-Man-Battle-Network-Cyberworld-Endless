/* title_setup.h. Drawn as Options is (options_draw.c): the paused title
 * under BN6's screen frame, the band with the screen's name, the tabs of
 * its pages over the rows in the window, the information on the row under
 * the cursor in KeyItem's box. Drawing changes nothing. */
#include "title_setup.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "extras.h"
#include "extras_text.h"
#include "meta.h"
#include "pet_panel.h"
#include "powers.h"
#include "run.h"
#include "save.h"
#include "title_setup_text.h"

/* its pages, each a tab (Options' pages, the owner's call of 5 November
 * 2026): the run's shape and what it brings, the helpers, the extras;
 * NEXT at the foot of each but the last, whose foot is JACK IN! */
enum { TAB_RUN, TAB_HELP, TAB_EXTRAS, TABS };
enum { ROW_NET, ROW_FOLDER, ROW_CROSS, ROW_THREAT, RUN_ROWS };
#define ROWS_MAX 8   /* a page's rows at most */

/* where it draws, from the picture's corner: the tabs under the band, the
 * rows in a window with NEXT or JACK IN! at its foot, as NaviCust's RUN! at
 * its list's, and the information under it to the screen's foot. (The
 * rows 12 apart, the chat font's cell; three in sight, the rest scrolled,
 * as Options' are, so that the box keeps its three lines: a Cross's two
 * and "No other Cross this run") */
#define TABS_Y 19
#define WIN_X 7
#define WIN_W (CORE_W - 14)
#define WIN_Y (TABS_Y + LOOK_PILL_H + 3)
#define NOTE_H 49
#define NOTE_Y (CORE_H - 6 - NOTE_H)
#define WIN_H (NOTE_Y - 2 - WIN_Y)
#define ROW_Y (WIN_Y + 6)
#define ROW_H 12
#define ROWS_SHOWN 3
#define LABEL_X 24
#define CURSOR_X 12
#define VALUE_R (CORE_W - 34)   /* a value's right edge, its arrow after it, the scroll bar beyond */
#define GO_H 13
#define GO_Y (WIN_Y + WIN_H - LOOK_FRAME - GO_H - 1)
#define NOTE_LINES 3
#define SHUT_MAX 48   /* a line of what is still shut, and how */
#define SHUT_LINES 4

/* the choices on the pages */
typedef struct {
	int net, folder, cross, threat, helpers;   /* run_setup's */
	int extras_off;                            /* run_setup_extras' */
} SetupChoice;

static struct {
	bool shown;
	int tab;
	int at[TABS];    /* each page's cursor: a row's place, or the rows' count on the foot's button */
	int top[TABS];   /* ... and its first row in sight */
	int t;           /* frames since the page came: its fade */
	SetupChoice c;
} S;

/* the rows the last summary's unlocks are on (profile.setup_new) */
static int row_new(int row) {
	static const int bits[RUN_ROWS] = { SETUP_NEW_NET, SETUP_NEW_FOLDER, SETUP_NEW_CROSS, SETUP_NEW_THREAT };
	return row >= 0 && row < RUN_ROWS ? profile.setup_new & bits[row] : 0;
}

/* a page's rows: the run's four; the helpers shown; the extras shown, by
 * EXTRA_* (the older nets only where another game's ROM is read) */
static int rows_of(int tab, int row[ROWS_MAX]) {
	int n = 0;
	if (tab == TAB_RUN)
		for (int r = 0; r < RUN_ROWS; ++r) row[n++] = r;
	if (tab == TAB_HELP)
		for (int h = 0; h < meta_helpers_shown(); ++h) row[n++] = h;
	if (tab == TAB_EXTRAS)
		for (int e = 0; e < EXTRAS; ++e)
			if (extra_shown(e)) row[n++] = e;
	return n;
}

static int rows_count(int tab) {
	int row[ROWS_MAX];
	return rows_of(tab, row);
}

/* the row under the cursor (its RUN row, helper or extra), -1 on the foot's button */
static int row_at(void) {
	int row[ROWS_MAX], n = rows_of(S.tab, row);
	return S.at[S.tab] < n ? row[S.at[S.tab]] : -1;
}

/* page `tab`'s cursor row in sight (on the foot's button, the rows as
 * they were: a page opens on its first) */
static void scroll(int tab) {
	int n = rows_count(tab), at = S.at[tab], *top = &S.top[tab];
	if (at >= n) return;
	if (at < *top) *top = at;
	if (at >= *top + ROWS_SHOWN) *top = at - ROWS_SHOWN + 1;
	if (*top < 0) *top = 0;
}

void title_setup_reset(void) { S.c = (SetupChoice){ .net = RUN_SHORT, .folder = FOLDER_STANDARD }; }

bool title_setup_shown(void) { return S.shown; }

void title_setup_apply(void) {
	run_setup(S.c.net, S.c.folder, S.c.threat, S.c.helpers, S.c.cross);
	run_setup_extras(S.c.extras_off);
}

void title_setup_open(void) {
	S.shown = true;
	S.t = 0;
	meta_unlocks_unsaid();
	S.c.net = profile.last_net == RUN_ENDLESS && meta_endless_open() ? RUN_ENDLESS : RUN_SHORT;
	S.c.folder = meta_folder_open(profile.last_folder) ? profile.last_folder : FOLDER_STANDARD;
	S.c.cross = meta_cross_open(profile.last_cross) ? profile.last_cross : 0;
	S.c.threat = profile.last_threat <= meta_threat_open() ? profile.last_threat : meta_threat_open();
	S.c.helpers = profile.last_helpers & ((1 << meta_helpers_shown()) - 1);
	S.c.extras_off = profile.last_extras_off & ((1 << EXTRAS) - 1);
	/* (each page's cursor on its foot, so that A goes page by page to
	 * JACK IN!; but on the row of what the last run opened, as a new
	 * unlock is spent here: a playtester's A went through a setup that
	 * showed none of his new SlashCross start; on BeastOut where it is
	 * the news, the helper a profile earns, issue #99) */
	S.tab = TAB_RUN;
	for (int t = 0; t < TABS; ++t) {
		S.at[t] = rows_count(t);
		S.top[t] = 0;
	}
	for (int r = RUN_ROWS - 1; r >= 0; --r) if (row_new(r)) S.at[TAB_RUN] = r;
	if (S.at[TAB_RUN] == RUN_ROWS && (profile.setup_new & SETUP_NEW_HELP) && meta_beast_open()) {
		S.tab = TAB_HELP;
		S.at[TAB_HELP] = HELPER_BEAST;
	}
	for (int t = 0; t < TABS; ++t) scroll(t);
}

/* ---- its keys ---- */

static int value_of(int row) {
	return row == ROW_NET ? S.c.net : row == ROW_FOLDER ? S.c.folder : row == ROW_CROSS ? S.c.cross : S.c.threat;
}

/* Left or Right on a RUN row: the next open value that way */
static void turn(int row, int d) {
	int was = value_of(row);
	if (row == ROW_NET && meta_endless_open()) S.c.net = S.c.net == RUN_SHORT ? RUN_ENDLESS : RUN_SHORT;
	if (row == ROW_FOLDER)   /* (the open ones only) */
		for (int k = 1; k < FOLDER_COUNT; ++k) {
			int f = (S.c.folder + d * k + FOLDER_COUNT * k) % FOLDER_COUNT;
			if (meta_folder_open(f)) { S.c.folder = f; break; }
		}
	if (row == ROW_CROSS)   /* (none, or an open one) */
		for (int k = 1; k < 6; ++k) {
			int c = (S.c.cross + d * k + 6 * k) % 6;
			if (!c || meta_cross_open(c)) { S.c.cross = c; break; }
		}
	if (row == ROW_THREAT) S.c.threat = (S.c.threat + d + meta_threat_open() + 1) % (meta_threat_open() + 1);
	if (value_of(row) != was) audio_sfx(SFX_CURSOR);
}

/* jacking in with these: they are the next setup's to start from */
static bool go(void) {
	profile.last_net = (uint8_t)S.c.net;
	profile.last_folder = (uint8_t)S.c.folder;
	profile.last_cross = (uint8_t)S.c.cross;
	profile.last_threat = (uint8_t)S.c.threat;
	profile.last_helpers = (uint8_t)S.c.helpers;
	profile.last_extras_off = (uint16_t)S.c.extras_off;
	profile.setup_new = 0;
	profile_save();
	S.shown = false;
	return true;
}

/* L or R, or NEXT: the page that way, its cursor where it was left */
static void turn_page(int d, int sfx) {
	S.tab = (S.tab + d + TABS) % TABS;
	S.t = 0;
	audio_sfx(sfx);
}

/* A, Left or Right on the row under the cursor: a RUN row's value turned
 * (A as Right), a helper or an extra on or off, as Options' Display turns
 * its lines */
static void row_turn(int row, int d, bool a) {
	if (S.tab == TAB_RUN) { turn(row, d); return; }
	if (S.tab == TAB_HELP) S.c.helpers ^= 1 << row;
	else S.c.extras_off ^= 1 << row;
	audio_sfx(a ? SFX_SELECT : SFX_CURSOR);
}

bool title_setup_update(void) {
	++S.t;
	if (btn_pressed(BTN_START)) return go();
	if (btn_pressed(BTN_B)) { S.shown = false; audio_sfx(SFX_CANCEL); return false; }
	if (btn_pressed(BTN_L) || btn_pressed(BTN_R)) { turn_page(btn_pressed(BTN_L) ? -1 : 1, SFX_CURSOR); return false; }
	int n = rows_count(S.tab), *at = &S.at[S.tab];
	if (btn_repeat(BTN_UP)) { *at = (*at + n) % (n + 1); scroll(S.tab); audio_sfx(SFX_CURSOR); }
	if (btn_repeat(BTN_DOWN)) { *at = (*at + 1) % (n + 1); scroll(S.tab); audio_sfx(SFX_CURSOR); }
	int row = row_at(), d = btn_repeat(BTN_RIGHT) ? 1 : btn_repeat(BTN_LEFT) ? -1 : 0;
	if (row >= 0 && (d || btn_pressed(BTN_A))) row_turn(row, d ? d : 1, !d);
	if (row >= 0 || !btn_pressed(BTN_A)) return false;
	if (S.tab == TABS - 1) return go();   /* (JACK IN!) */
	turn_page(1, SFX_SELECT);   /* (NEXT: the next page, on its foot as it was left) */
	return false;
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

/* a RUN row's note */
static const char *run_note(int row, char *buf, size_t n, char shut[][SHUT_MAX], int *nshut) {
	switch (row) {
	case ROW_NET:
		return setup_word(S.c.net == RUN_ENDLESS ? SW_ENDLESS_NOTE : meta_endless_open() ? SW_SHORT_NOTE : SW_SHORT_WIN_NOTE);
	case ROW_FOLDER:
		/* (every folder still to open, and how: the telegraph comes first) */
		for (int f = 1; f < FOLDER_COUNT && *nshut < SHUT_LINES; ++f)
			if (!meta_folder_open(f) && meta_folder(f)->opens)
				setup_shut_folder(shut[(*nshut)++], SHUT_MAX, meta_folder(f)->name, meta_folder(f)->opens);
		return meta_folder(S.c.folder)->about;
	case ROW_CROSS: return cross_note(buf, n, shut, nshut);
	default:
		if (S.c.threat) return meta_threat_rule(S.c.threat);
		return setup_word(meta_threat_open() ? SW_THREAT0_NOTE : SW_THREAT0_WIN_NOTE);
	}
}

/* what the row under the cursor means, a line or two (made into `buf`
 * where it is made), and under it, dim, what is still shut and how, or
 * what else to know (into `shut`) */
static const char *row_note(char *buf, size_t n, char shut[][SHUT_MAX], int *nshut) {
	static const SetupWord feet[TABS] = { SW_NEXT_HELP_NOTE, SW_NEXT_EXTRAS_NOTE, SW_GO_NOTE };
	int row = row_at();
	if (row < 0) return setup_word(feet[S.tab]);
	if (S.tab == TAB_RUN) return run_note(row, buf, n, shut, nshut);
	/* (and what else it changes: All *'s Program Advances, the tournament
	 * not in runs yet) */
	const HelperInfo *h = S.tab == TAB_HELP ? meta_helper(row) : extra_words(row);
	if (h->more && S.tab == TAB_EXTRAS) snprintf(shut[(*nshut)++], SHUT_MAX, "%s", h->more);
	return S.tab == TAB_HELP ? setup_helper_note(buf, n, h->about, h->more) : h->about;
}

/* KeyItem's information box: its label, its white page, the note in the
 * chat font's ink and what is shut under it, dim, each where it fits
 * whole: BN6's box says its piece in three lines at most (docs/engine/ui.md). */
static void note_draw(int x0, int y0) {
	char buf[128], shut[SHUT_LINES][SHUT_MAX];
	int nshut = 0;
	const char *note = row_note(buf, sizeof buf, shut, &nshut);
	SDL_Rect r = look_info(x0 + WIN_X, y0 + NOTE_Y, WIN_W, NOTE_H, setup_word(SW_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	int x = r.x + 6, w = r.w - 12, end = r.y + 1 + NOTE_LINES * LOOK_LINE;
	int y = look_wrapped(note, x, r.y + 1, w, NOTE_LINES, LOOK_INK, LOOK_INK_SHADE);
	for (int i = 0; i < nshut; ++i) {
		int lines = look_text_width(shut[i]) <= w ? 1 : 2;
		if (y + lines * LOOK_LINE > end) break;
		y = look_wrapped(shut[i], x, y, w, lines, LOOK_DIM, LOOK_INK_SHADE);
	}
}

/* ---- the pages ---- */

/* the pages' tabs over the list, as Options' are (look_tabs) */
static void tabs_draw(int x0, int y0) {
	const char *names[TABS] = { setup_word(SW_TAB_RUN), setup_word(SW_TAB_HELP), setup_word(SW_TAB_EXTRAS) };
	look_tabs(x0 + WIN_X, y0 + TABS_Y, WIN_W, TABS, S.tab, names);
}

/* a row's name, NEW after it where `fresh`: orange capitals over their
 * dark red, as BN6 marks a new mail */
static void label_draw(int x0, int y, const char *s, bool fresh) {
	look_text(x0 + LABEL_X, y, s, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	if (fresh) look_new(x0 + LABEL_X + look_text_width(s) + 5, y + 3, setup_word(SW_NEW));
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

/* a RUN row: its name and value */
static void run_row_draw(int x0, int y, int row) {
	static const SetupWord names[RUN_ROWS] = { SW_NET, SW_FOLDER, SW_CROSS, SW_THREAT };
	int open_folders = 0;
	for (int f = 0; f < FOLDER_COUNT; ++f) open_folders += meta_folder_open(f);
	char v[16];
	snprintf(v, sizeof v, "%d", S.c.threat);
	const char *values[RUN_ROWS] = { setup_word(S.c.net == RUN_ENDLESS ? SW_ENDLESS : SW_SHORT), meta_folder(S.c.folder)->name,
		S.c.cross ? powers_cross_name(S.c.cross) : setup_word(SW_NONE), v };
	bool turns[RUN_ROWS] = { meta_endless_open(), open_folders > 1, crosses_open() > 0, meta_threat_open() > 0 };
	label_draw(x0, y, setup_word(names[row]), row_new(row));
	value_draw(x0, y, values[row], turns[row]);
}

/* the foot's button: NEXT, or JACK IN! on the last page, NaviCust's RUN!,
 * heavy white capitals on the bright green, the green triangle before it
 * when chosen */
static void go_draw(int x0, int y0, bool on) {
	const char *s = setup_word(S.tab == TABS - 1 ? SW_JACK_IN : SW_NEXT);
	int w = look_title_width(s) + 18, x = x0 + (CORE_W - w) / 2, y = y0 + GO_Y;
	look_button(x, y, w, GO_H);
	look_title(x + 10, y + 4, s, rgba(41, 132, 148, 255));
	look_title(x + 9, y + 3, s, LOOK_PAGE);
	if (on) look_choice(x - LOOK_CHOICE_W - 3, y);
}

/* the page's rows in sight, each with its value, and its foot */
static void page_draw(int x0, int y0) {
	int row[ROWS_MAX], n = rows_of(S.tab, row), at = S.at[S.tab], top = S.top[S.tab];
	for (int i = top; i < n && i < top + ROWS_SHOWN; ++i) {
		int y = y0 + ROW_Y + (i - top) * ROW_H;
		if (S.tab == TAB_RUN) run_row_draw(x0, y, row[i]);
		else {
			bool on = S.tab == TAB_HELP ? S.c.helpers >> row[i] & 1 : !(S.c.extras_off >> row[i] & 1);
			const char *name = S.tab == TAB_HELP ? meta_helper(row[i])->name : extra_words(row[i])->name;
			label_draw(x0, y, name, S.tab == TAB_HELP && row[i] == HELPER_BEAST && (profile.setup_new & SETUP_NEW_HELP));
			value_draw(x0, y, setup_word(on ? SW_ON : SW_OFF), true);
		}
		/* (BN6's lists keep a line's colour under the cursor: the arrow alone says where it is) */
		if (i == at) look_cursor(x0 + CURSOR_X, y + 2);
	}
	go_draw(x0, y0, at >= n);
}

void title_setup_draw(int x0, int y0) {
	pet_panel_draw(x0, y0, setup_word(SW_TITLE));
	look_band_hint(x0, y0, CORE_W, "START", setup_word(SW_START_HINT));
	tabs_draw(x0, y0);
	SDL_Rect win = { x0 + WIN_X, y0 + WIN_Y, WIN_W, WIN_H };
	look_window(win.x, win.y, win.w, win.h, 0, true);
	page_draw(x0, y0);
	look_scroll(win, 0, S.top[S.tab], ROWS_SHOWN, rows_count(S.tab));
	note_draw(x0, y0);
	look_fade_in(x0, y0, CORE_W, CORE_H, S.t);
}

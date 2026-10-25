/* The controls screen's picture (controls_state.h): its tabs, its rows
 * with their MAIN and ALSO slots and their dots, the bottom line, the
 * note, and the box that asks for a press; where a tap lands on them.
 * Drawing changes nothing. */
#include "controls.h"

#include <stdio.h>
#include <string.h>

#include "buttons.h"
#include "controls_state.h"
#include "gfx.h"
#include "minifont.h"
#include "options_text.h"
#include "pads.h"
#include "pet_panel.h"
#include "platform.h"
#include "touch.h"

/* where it draws, from the picture's corner: the tabs and the device's
 * name on the body under the band, the rows in a window (MAIN and ALSO in
 * its band; ROWS_SHOWN of them, a row the list's letters' height and a
 * pixel either side, the rest a scroll away), the bottom line in it, the note and the keys under it */
#define HEAD_Y 18      /* the tabs, the device's name */
#define WIN_Y 29
#define WIN_TOP 7      /* MAIN and ALSO over their columns, in the window's band */
#define COL_Y (WIN_Y + LOOK_FRAME + 1)
#define ROW_Y (WIN_Y + LOOK_FRAME + WIN_TOP)
#define ROW_H 14
#define BOTTOM_Y (ROW_Y + ROWS_SHOWN * ROW_H)
#define WIN_H (2 * LOOK_FRAME + WIN_TOP + (ROWS_SHOWN + 1) * ROW_H + 1)
#define NOTE_Y 132
#define NAV_Y 148
#define LABEL_X 24
#define MAIN_X 82
#define ALSO_X 150
#define DOT_X (CORE_W - 24)
#define CELL_W 66

static const SDL_Color SLOT = { 16, 99, 107, 255 }, GREEN = { 74, 231, 115, 255 }, DARK = { 10, 58, 80, 255 };

/* ---- the words ---- */

/* `s` cut where it fits `w` pixels */
static void fit(char *s, int w) {
	size_t n = strlen(s);
	while (n > 1 && look_text_width(s) > w) s[--n] = 0;
}

const char *controls_key_word(int sc) {
	static char s[32];
	static const struct { const char *name, *word; } short_names[] = {
		{ "Return", "Enter" }, { "Backspace", "Bksp" }, { "Left Shift", "L Shift" }, { "Right Shift", "R Shift" },
		{ "Left Ctrl", "L Ctrl" }, { "Right Ctrl", "R Ctrl" }, { "Left Alt", "L Alt" }, { "Right Alt", "R Alt" },
		{ "CapsLock", "Caps" }, { "PageUp", "PgUp" }, { "PageDown", "PgDn" },
	};
	const char *name = SDL_GetScancodeName((SDL_Scancode)sc);
	for (size_t i = 0; i < sizeof short_names / sizeof *short_names; ++i)
		if (!strcmp(name, short_names[i].name)) return short_names[i].word;
	/* ("Keypad Enter" is "KP Enter") */
	if (!strncmp(name, "Keypad ", 7)) { snprintf(s, sizeof s, "KP %s", name + 7); return s; }
	return name;
}

void controls_slot_words(int gba, int slot, char *out, size_t n) {
	if (C.tab == TAB_PAD) {
		int in = C.pads.in[C.family][gba][slot];
		snprintf(out, n, "%s", in == PAD_NONE ? "-" : padmap_label_of(C.family, C.style, in));
	} else {
		int sc = C.keys.slot[gba][slot];
		snprintf(out, n, "%s", sc ? controls_key_word(sc) : "-");
	}
}

/* The four ways' keys as one: their letters as WASD reads (up, left,
 * down, right), the arrows as "Arrows", else UP's and an ellipsis */
static void dpad_keys(int slot, char *out, size_t n) {
	static const int order[4] = { 0, 2, 1, 3 };
	static const int arrows[4] = { SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT };
	bool letters = true, arrow = true;
	char s[8] = "";
	for (int k = 0; k < 4; ++k) {
		int sc = C.keys.slot[order[k]][slot];
		const char *name = sc ? SDL_GetScancodeName((SDL_Scancode)sc) : "";
		letters &= strlen(name) == 1;
		s[k] = name[0];
		arrow &= C.keys.slot[k][slot] == arrows[k];
	}
	if (!C.keys.slot[0][slot]) snprintf(out, n, "-");
	else if (letters) snprintf(out, n, "%s", s);
	else if (arrow) snprintf(out, n, "Arrows");
	else snprintf(out, n, "%s...", controls_key_word(C.keys.slot[0][slot]));
}

/* The four ways' inputs on a pad as one: their label without the way
 * ("D-pad up" "D-pad", "Hat up" "Hat", "Axis 2-" "Axis 2"), else UP's and
 * an ellipsis */
void controls_dpad_words(int slot, char *out, size_t n) {
	if (C.tab == TAB_KEYS) { dpad_keys(slot, out, n); return; }
	int in = C.pads.in[C.family][0][slot];
	if (in == PAD_NONE) { snprintf(out, n, "-"); return; }
	snprintf(out, n, "%s", padmap_label_of(C.family, C.style, in));
	char *sp = strrchr(out, ' ');
	size_t len = strlen(out);
	if (sp && !strcmp(sp, " up")) *sp = 0;
	else if (!strncmp(out, "Axis", 4) && len > 1 && (out[len - 1] == '-' || out[len - 1] == '+')) out[len - 1] = 0;
	else if (len + 3 < n) strcat(out, "...");
}

const char *controls_nav_word(int which) {
	static const char *const keys[] = { "Enter", "Esc", "Bksp", "Tab", "Tab" };
	if (touch_shown() && !C.has_pad) return which == NAV_A ? "Tap" : "";
	if (P.keyboard_last || !C.has_pad) return KEYS ? keys[which] : "";
	bool raw = C.family == PAD_FAMILY_RAW;
	static const int pad[] = { PAD_A, PAD_B, PAD_X, PAD_LEFTSHOULDER, PAD_RIGHTSHOULDER }, joy[] = { 0, 1, 2, 4, 5 };
	return padmap_label_of(C.family, C.style, raw ? joy[which] : pad[which]);
}

/* ---- the layout, for the taps too ---- */

static int tab_count(void) { return KEYS ? 2 : 1; }
static const char *tab_name(int t) { return t == TAB_PAD ? "Controller" : "Keyboard"; }

/* the tabs as BN6's tags on the body under the band, from its left */
static void tab_span(int t, int *x0, int *x1) {
	int x = 8;
	for (int k = 0; k < t; ++k) x += look_caps_width(tab_name(k)) + 10 + 4;
	*x0 = x;
	*x1 = x + look_caps_width(tab_name(t)) + 10;
}

static const char *item_name(int item) {
	static const char *const names[ITEMS] = { "Set all", "Defaults", "Done" };
	return names[item];
}

/* the bottom line's items spread across it */
static void item_spans(int x0[ITEMS], int x1[ITEMS]) {
	int total = 0;
	for (int i = 0; i < ITEMS; ++i) total += look_text_width(item_name(i));
	int gap = (CORE_W - 2 * LABEL_X - total) / (ITEMS - 1), x = LABEL_X;
	for (int i = 0; i < ITEMS; ++i) {
		x0[i] = x;
		x1[i] = x + look_text_width(item_name(i));
		x = x1[i] + gap;
	}
}

int controls_tap_tab(int x, int y) {
	if (y < HEAD_Y - 3 || y > HEAD_Y + 11) return -1;
	for (int t = 0; t < tab_count(); ++t) {
		int a, b;
		tab_span(t, &a, &b);
		if (x >= a - 2 && x <= b + 2) return t;
	}
	return -1;
}

int controls_tap_item(int x, int y) {
	if (y < BOTTOM_Y || y >= BOTTOM_Y + ROW_H) return -1;
	int x0[ITEMS], x1[ITEMS];
	item_spans(x0, x1);
	for (int i = 0; i < ITEMS; ++i)
		if (x >= x0[i] - 6 && x <= x1[i] + 6) return i;
	return -1;
}

bool controls_tap_cell(int x, int y, int *row, int *col) {
	if (y < ROW_Y || y >= BOTTOM_Y || x < 6 || x >= CORE_W - 6) return false;
	*row = C.top + (y - ROW_Y) / ROW_H;
	*col = x >= ALSO_X - 4 ? SLOT_ALSO : SLOT_MAIN;
	return true;
}

/* ---- the picture ---- */

/* the tabs, the one open dark as the band; the device's name at the
 * right; MAIN and ALSO in the window's band at their columns' right ends */
static void tabs_draw(int x0, int y0) {
	int end = 0;
	for (int t = 0; t < tab_count(); ++t) {
		int a, b;
		tab_span(t, &a, &b);
		look_tag(x0 + a, y0 + HEAD_Y, b - a, 9, tab_name(t), t == C.tab ? LOOK_BAND : LOOK_GREEN_LO);
		end = b;
	}
	char name[40];
	snprintf(name, sizeof name, "%s", C.tab == TAB_KEYS ? "US key positions" : C.has_pad ? C.name : "No controller");
	int right = CORE_W - 9;
	for (size_t n = strlen(name); n > 1 && minifont_width(name, 1) > right - end - 8; ) name[--n] = 0;
	look_note(x0 + right - minifont_width(name, 1), y0 + HEAD_Y + 2, name, C.tab == TAB_PAD && !C.has_pad ? LOOK_GREEN_HI : LOOK_PAGE);
	int main_x = MAIN_X + CELL_W - 8 - look_caps_width("Main"), also_x = ALSO_X + CELL_W - 8 - look_caps_width("Also");
	look_caps(x0 + main_x, y0 + COL_Y, "Main", LOOK_PAGE, LOOK_EDGE_LO);
	look_caps(x0 + also_x, y0 + COL_Y, "Also", LOOK_PAGE, LOOK_EDGE_LO);
}

static void preset_draw(int x0, int y) {
	if (C.tab == TAB_KEYS) { look_text(x0 + MAIN_X, y, "-", LOOK_DIM, LOOK_SHADE, TEXT_LEFT); return; }
	int p = padmap_preset_of(&C.pads, C.family);
	const char *v = padmap_preset_name(C.family, C.style, p);
	look_text(x0 + MAIN_X + 6, y, v, p < 0 ? LOOK_DIM : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	/* (the PET's small arrows: Left and Right change it) */
	int r = x0 + MAIN_X + 6 + look_text_width(v) + 3;
	for (int i = 0; i < 4; ++i) {
		fill_rect(x0 + MAIN_X + 3 - i, y + 2 + i, 1, 8 - 2 * i, PET_ORANGE);
		fill_rect(r + i, y + 2 + i, 1, 8 - 2 * i, PET_ORANGE);
	}
}

static void cell_draw(int x, int y, const char *words, bool focus) {
	char s[48];
	snprintf(s, sizeof s, "%s", words);
	fit(s, CELL_W - 4);
	look_text(x, y, s, focus ? LOOK_GOLD : !strcmp(s, "-") ? LOOK_DIM : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
}

static void rows_draw(int x0, int y0) {
	static const char *const names[ROWS] = { "A and B", "D-PAD", "A", "B", "L", "R", "START", "SELECT", "FAST" };
	bool idle = !C.ask && !C.confirm;
	/* (the cursor's slot under every row's words, so none is covered) */
	if (idle && C.row < ROWS && C.row != ROW_PRESET)
		fill_rect(x0 + (C.col == SLOT_MAIN ? MAIN_X : ALSO_X) - 3, y0 + ROW_Y + (C.row - C.top) * ROW_H, CELL_W, ROW_H, SLOT);
	for (int r = C.top; r < C.top + ROWS_SHOWN && r < ROWS; ++r) {
		int y = y0 + ROW_Y + (r - C.top) * ROW_H, gba = controls_gba_of(r);
		bool on = C.row == r && idle;
		const char *name = names[r];
		look_text(x0 + LABEL_X, y + 1, name, on ? LOOK_GOLD : r == ROW_PRESET && C.tab == TAB_KEYS ? LOOK_DIM : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		if (on) look_cursor(x0 + 12, y + 2);
		if (r == ROW_PRESET) { preset_draw(x0, y + 1); continue; }
		char words[48];
		for (int s = SLOT_MAIN; s <= SLOT_ALSO; ++s) {
			if (r == ROW_DPAD) controls_dpad_words(s, words, sizeof words);
			else controls_slot_words(gba, s, words, sizeof words);
			cell_draw(x0 + (s == SLOT_MAIN ? MAIN_X : ALSO_X), y + 1, words, on && C.col == s);
		}
		/* the dot: lit by a press of its button, through the map being set */
		bool lit = r == ROW_DPAD ? (C.lit[0] | C.lit[1] | C.lit[2] | C.lit[3]) > 0 : C.lit[gba] > 0;
		fill_rect(x0 + DOT_X, y + 4, 5, 5, lit ? GREEN : DARK);
	}
}

static void bottom_draw(int x0, int y0) {
	int a[ITEMS], b[ITEMS], y = y0 + BOTTOM_Y;
	item_spans(a, b);
	for (int i = 0; i < ITEMS; ++i) {
		bool on = C.row == ROW_BOTTOM && C.item == i && !C.ask && !C.confirm;
		look_text(x0 + a[i], y + 1, item_name(i), on ? LOOK_GOLD : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		if (on) look_cursor(x0 + a[i] - 11, y + 2);
	}
}

/* What the cursor is on does, or the word just said */
static void note_words(char *s, size_t n) {
	const char *a = controls_nav_word(NAV_A);
	int gba = controls_gba_of(C.row);
	if (C.said_t > 0) snprintf(s, n, "%s", C.said);
	else if (C.row == ROW_PRESET) snprintf(s, n, "%s", padmap_preset_about(C.family, C.style, padmap_preset_of(&C.pads, C.family)));
	else if (C.row == ROW_DPAD) snprintf(s, n, "%s: the D-pad's %s, way by way", a, C.col == SLOT_MAIN ? "MAIN" : "ALSO");
	else if (gba == PAD_FAST) snprintf(s, n, "%s: fast forward's %s %s", a, C.col == SLOT_MAIN ? "MAIN" : "ALSO", C.tab == TAB_PAD ? "button" : "key");
	else if (gba >= 0) snprintf(s, n, "%s: give %s its %s %s", a, padmap_gba_name(gba), C.col == SLOT_MAIN ? "MAIN" : "ALSO", C.tab == TAB_PAD ? "button" : "key");
	else if (C.item == ITEM_SETALL) snprintf(s, n, "Asks for every button in turn");
	else if (C.item == ITEM_DEFAULTS) snprintf(s, n, "The %s as it came", C.tab == TAB_PAD ? "controller" : "keyboard");
	else snprintf(s, n, "Keep them: press the new A");
}

bool controls_note_draw(int cx, int y, const char *s, SDL_Color c) {
	/* (the bold letters, narrower than the list's: a line of them, or two of the small ones) */
	if (text_width(s) <= CORE_W - 12) {
		text_draw(cx, y, s, c, TEXT_CENTER);
		return false;
	}
	char a[128], *second = NULL;
	snprintf(a, sizeof a, "%s", s);
	if (minifont_width(a, 1) > CORE_W - 20) {
		char *cut = NULL;
		for (char *sp = strchr(a, ' '); sp; sp = strchr(sp + 1, ' ')) {
			*sp = 0;
			bool fits = minifont_width(a, 1) <= CORE_W - 20;
			*sp = ' ';
			if (!fits) break;
			cut = sp;
		}
		if (cut) { *cut = 0; second = cut + 1; }
	}
	pet_panel_note(cx, y + 2, a, c);
	if (second) pet_panel_note(cx, y + 9, second, c);
	return second != NULL;
}

/* The box over the rows: what it asks for, and how long it waits */
static void ask_draw(int x0, int y0) {
	static const char *const ways[4] = { "UP", "DOWN", "LEFT", "RIGHT" };
	char line[64], sub[40], wait[48];
	const char *what = C.tab == TAB_PAD ? "button" : "key", *slot = C.ask_slot == SLOT_MAIN ? "MAIN" : "ALSO";
	if (C.ask == ASK_SLOT) {
		snprintf(line, sizeof line, "%s %s for %s", slot, what, padmap_gba_name(C.ask_gba));
		snprintf(sub, sizeof sub, "%s", C.tab == TAB_PAD ? "on the controller" : "on the keyboard");
	} else if (C.ask == ASK_DPAD) {
		snprintf(line, sizeof line, "%s D-pad: press %s", slot, ways[C.ask_step]);
		snprintf(sub, sizeof sub, "%d of 4", C.ask_step + 1);
	} else {
		snprintf(line, sizeof line, "Press the %s for %s", what, padmap_gba_name(C.ask_step));
		snprintf(sub, sizeof sub, "%d of %d", C.ask_step + 1, PAD_ROWS);
	}
	int secs = (C.ask_left + 59) / 60;
	/* (Set all: waiting skips the one button, Escape stops the rest) */
	if (C.ask == ASK_ALL) snprintf(wait, sizeof wait, "Wait %d to skip it%s", secs, KEYS ? ", Esc to stop" : "");
	else snprintf(wait, sizeof wait, "%sWait %d to cancel", KEYS ? "Esc or " : "", secs);
	look_window(x0 + 18, y0 + 40, CORE_W - 36, 62, 0, false);
	text_draw(x0 + CORE_W / 2, y0 + 48, line, LOOK_GOLD, TEXT_CENTER);
	look_text(x0 + CORE_W / 2, y0 + 63, sub, LOOK_TEXT, LOOK_SHADE, TEXT_CENTER);
	minifont_draw_centered(x0 + CORE_W / 2, y0 + 84, wait, LOOK_EDGE_HI, 1);
}

/* The keep box: the edited map's A, on the device used last */
static void confirm_draw(int x0, int y0) {
	char a[32], line[64], sub[48];
	if (C.tab == TAB_PAD && C.has_pad) controls_slot_words(4, SLOT_MAIN, a, sizeof a);
	else if (KEYS) snprintf(a, sizeof a, "%s", C.keys.slot[4][0] ? controls_key_word(C.keys.slot[4][0]) : "A");
	else snprintf(a, sizeof a, "A");
	snprintf(line, sizeof line, "Press %s to keep these", a);
	snprintf(sub, sizeof sub, "Not kept in %d", (C.confirm + 59) / 60);
	look_window(x0 + 18, y0 + 46, CORE_W - 36, 46, 0, false);
	text_draw(x0 + CORE_W / 2, y0 + 54, line, LOOK_GOLD, TEXT_CENTER);
	look_text(x0 + CORE_W / 2, y0 + 70, sub, LOOK_TEXT, LOOK_SHADE, TEXT_CENTER);
}

static void nav_draw(int cx, int y) {
	char nav[96];
	if (touch_shown() && !C.has_pad) snprintf(nav, sizeof nav, "Tap a slot to set it, a tab to switch");
	else if (KEYS)
		snprintf(nav, sizeof nav, "%s: set  %s: clear  %s: tab  %s: back", controls_nav_word(NAV_A), controls_nav_word(NAV_X),
			P.keyboard_last || !C.has_pad ? "Tab" : controls_nav_word(NAV_R), controls_nav_word(NAV_B));
	else snprintf(nav, sizeof nav, "%s: set  %s: clear  %s: back", controls_nav_word(NAV_A), controls_nav_word(NAV_X), controls_nav_word(NAV_B));
	pet_panel_note(cx, y, nav, LOOK_PAGE);
}

void controls_draw(void) {
	if (!C.open) return;
	int x0 = P.core_x, y0 = P.core_y, cx = x0 + CORE_W / 2;
	pet_panel_draw(x0, y0, options_word(OW_CONTROLS));
	SDL_Rect win = { x0 + 7, y0 + WIN_Y, CORE_W - 14, WIN_H };
	look_window(win.x, win.y, win.w, win.h, WIN_TOP, true);
	look_scroll(win, WIN_TOP, C.top, ROWS_SHOWN, ROWS);
	tabs_draw(x0, y0);
	rows_draw(x0, y0);
	bottom_draw(x0, y0);
	if (C.ask) ask_draw(x0, y0);
	else if (C.confirm) confirm_draw(x0, y0);
	char note[128];
	note_words(note, sizeof note);
	if (!C.ask && !C.confirm) {
		bool two = controls_note_draw(cx, y0 + NOTE_Y, note, C.said_t > 0 && C.said_warn ? LOOK_ORANGE : LOOK_PAGE);
		/* (under a note of two lines, its second line takes the keys'
		 * place: the note says what A does) */
		if (!two) nav_draw(cx, y0 + NAV_Y);
	}
}

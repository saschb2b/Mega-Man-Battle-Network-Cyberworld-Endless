/* The controls screen's picture (controls_state.h): its tabs, its rows
 * with their MAIN and ALSO slots and their dots, the bottom line, the
 * note, and the box that asks for a press; where a tap lands on them.
 * Drawing changes nothing. */
#include "controls.h"

#include <stdio.h>
#include <string.h>

#include "analytics.h"
#include "analytics_ask.h"
#include "analytics_text.h"
#include "buttons.h"
#include "controls_state.h"
#include "gfx.h"
#include "minifont.h"
#include "pads.h"
#include "platform.h"
#include "touch.h"

/* where it draws, from the picture's corner */
#define HEAD_Y 7       /* the tabs, the device's name */
#define COL_Y 16       /* MAIN and ALSO over their columns */
#define ROW_Y 24
#define ROW_H 11
#define BOTTOM_Y (ROW_Y + controls_rows() * ROW_H)
#define NOTE_Y (BOTTOM_Y + 13)
#define NAV_Y 149
#define LABEL_X 17
#define MAIN_X 72
#define ALSO_X 150
#define DOT_X (CORE_W - 14)
#define CELL_W 72

static const SDL_Color GOLD = { 255, 230, 90, 255 }, SKY = { 170, 200, 255, 255 }, ORANGE = { 255, 170, 40, 255 },
	DIM = { 120, 140, 170, 255 }, NAVY = { 16, 60, 90, 255 }, CYAN = { 66, 198, 231, 255 }, SLOT = { 16, 99, 107, 255 },
	GREEN = { 74, 231, 115, 255 }, DARK = { 12, 36, 56, 250 };

/* ---- the words ---- */

/* `s` cut where it fits `w` pixels */
static void fit(char *s, int w) {
	size_t n = strlen(s);
	while (n > 1 && text_width(s) > w) s[--n] = 0;
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

static void tab_span(int t, int *x0, int *x1) {
	int x = 10;
	for (int k = 0; k < t; ++k) x += minifont_width(tab_name(k), 1) + 12;
	*x0 = x;
	*x1 = x + minifont_width(tab_name(t), 1) + 8;
}

static const char *item_name(int item) {
	static const char *const names[ITEMS] = { "Set all", "Defaults", "Done" };
	return names[item];
}

/* the bottom line's items spread across it */
static void item_spans(int x0[ITEMS], int x1[ITEMS]) {
	int total = 0;
	for (int i = 0; i < ITEMS; ++i) total += text_width(item_name(i));
	int gap = (CORE_W - 34 - total) / (ITEMS - 1), x = 17;
	for (int i = 0; i < ITEMS; ++i) {
		x0[i] = x;
		x1[i] = x + text_width(item_name(i));
		x = x1[i] + gap;
	}
}

int controls_tap_tab(int x, int y) {
	if (y < HEAD_Y - 3 || y > HEAD_Y + 8) return -1;
	for (int t = 0; t < tab_count(); ++t) {
		int a, b;
		tab_span(t, &a, &b);
		if (x >= a - 2 && x <= b + 2) return t;
	}
	return -1;
}

int controls_tap_item(int x, int y) {
	if (y < BOTTOM_Y - 2 || y > BOTTOM_Y + ROW_H) return -1;
	int x0[ITEMS], x1[ITEMS];
	item_spans(x0, x1);
	for (int i = 0; i < ITEMS; ++i)
		if (x >= x0[i] - 6 && x <= x1[i] + 6) return i;
	return -1;
}

bool controls_tap_cell(int x, int y, int *row, int *col) {
	if (y < ROW_Y - 1 || y >= BOTTOM_Y - 1 || x < 6 || x >= CORE_W - 6) return false;
	*row = (y - ROW_Y + 1) / ROW_H;
	*col = x >= ALSO_X - 4 ? SLOT_ALSO : SLOT_MAIN;
	return true;
}

/* ---- the picture ---- */

static void arrow(int x, int y, SDL_Color c) {
	for (int i = 0; i < 4; ++i) fill_rect(x + i, y + 2 + i, 1, 8 - 2 * i, c);
}

static void tabs_draw(int x0, int y0) {
	for (int t = 0; t < tab_count(); ++t) {
		int a, b;
		tab_span(t, &a, &b);
		if (t == C.tab) fill_rect(x0 + a - 2, y0 + HEAD_Y - 2, b - a + 4, 9, CYAN);
		minifont_draw(x0 + a + 2, y0 + HEAD_Y, tab_name(t), t == C.tab ? NAVY : DIM, 1);
	}
	char name[40];
	snprintf(name, sizeof name, "%s", C.tab == TAB_KEYS ? "US key positions" : C.has_pad ? C.name : "No controller");
	int right = x0 + CORE_W - 10, a, b;
	tab_span(tab_count() - 1, &a, &b);
	for (size_t n = strlen(name); n > 1 && minifont_width(name, 1) > right - (x0 + b + 10); ) name[--n] = 0;
	minifont_draw(right - minifont_width(name, 1), y0 + HEAD_Y, name, C.tab == TAB_PAD && !C.has_pad ? DIM : SKY, 1);
	minifont_draw(x0 + MAIN_X, y0 + COL_Y, "Main", DIM, 1);
	minifont_draw(x0 + ALSO_X, y0 + COL_Y, "Also", DIM, 1);
}

static void preset_draw(int x0, int y) {
	if (C.tab == TAB_KEYS) { text_draw(x0 + MAIN_X, y, "-", DIM, TEXT_LEFT); return; }
	int p = padmap_preset_of(&C.pads, C.family);
	const char *v = padmap_preset_name(C.family, C.style, p);
	text_draw(x0 + MAIN_X + 6, y, v, p < 0 ? DIM : WHITE, TEXT_LEFT);
	/* (the PET's small arrows: Left and Right change it) */
	int r = x0 + MAIN_X + 6 + text_width(v) + 3;
	for (int i = 0; i < 4; ++i) {
		fill_rect(x0 + MAIN_X + 3 - i, y + 2 + i, 1, 8 - 2 * i, ORANGE);
		fill_rect(r + i, y + 2 + i, 1, 8 - 2 * i, ORANGE);
	}
}

static void cell_draw(int x, int y, const char *words, bool focus) {
	char s[48];
	snprintf(s, sizeof s, "%s", words);
	fit(s, CELL_W - 4);
	text_draw(x, y, s, focus ? GOLD : !strcmp(s, "-") ? DIM : WHITE, TEXT_LEFT);
}

static void rows_draw(int x0, int y0) {
	static const char *const names[ROW_STATS] = { "A and B", "D-PAD", "A", "B", "L", "R", "START", "SELECT" };
	bool idle = !C.ask && !C.confirm;
	/* (the cursor's slot under every row's words, so none is covered) */
	if (idle && C.row < ROW_STATS && C.row != ROW_PRESET)
		fill_rect(x0 + (C.col == SLOT_MAIN ? MAIN_X : ALSO_X) - 3, y0 + ROW_Y + C.row * ROW_H + 2, CELL_W, ROW_H, SLOT);
	for (int r = 0; r < controls_rows(); ++r) {
		int y = y0 + ROW_Y + r * ROW_H, gba = controls_gba_of(r);
		bool on = C.row == r && idle;
		const char *name = r == ROW_STATS ? analytics_word(AW_ROW) : names[r];
		text_draw(x0 + LABEL_X, y, name, on ? GOLD : r == ROW_PRESET && C.tab == TAB_KEYS ? DIM : WHITE, TEXT_LEFT);
		if (on) arrow(x0 + 9, y, ORANGE);
		if (r == ROW_PRESET) { preset_draw(x0, y); continue; }
		/* (the statistics' answer after its name: no slot of a device's) */
		if (r == ROW_STATS) {
			text_draw(x0 + LABEL_X + text_width(name) + 8, y, analytics_word(analytics_consent() == ANALYTICS_ON ? AW_ON : AW_OFF), WHITE, TEXT_LEFT);
			continue;
		}
		char words[48];
		for (int s = SLOT_MAIN; s <= SLOT_ALSO; ++s) {
			if (r == ROW_DPAD) controls_dpad_words(s, words, sizeof words);
			else controls_slot_words(gba, s, words, sizeof words);
			cell_draw(x0 + (s == SLOT_MAIN ? MAIN_X : ALSO_X), y, words, on && C.col == s);
		}
		/* the dot: lit by a press of its button, through the map being set */
		bool lit = r == ROW_DPAD ? (C.lit[0] | C.lit[1] | C.lit[2] | C.lit[3]) > 0 : C.lit[gba] > 0;
		fill_rect(x0 + DOT_X, y + 3, 5, 5, lit ? GREEN : DARK);
	}
}

static void bottom_draw(int x0, int y0) {
	int a[ITEMS], b[ITEMS], y = y0 + BOTTOM_Y;
	item_spans(a, b);
	for (int i = 0; i < ITEMS; ++i) {
		bool on = C.row == ROW_BOTTOM && C.item == i && !C.ask && !C.confirm;
		text_draw(x0 + a[i], y, item_name(i), on ? GOLD : WHITE, TEXT_LEFT);
		if (on) arrow(x0 + a[i] - 8, y, ORANGE);
	}
}

/* What the cursor is on does, or the word just said */
static void note_words(char *s, size_t n) {
	const char *a = controls_nav_word(NAV_A);
	int gba = controls_gba_of(C.row);
	if (C.said_t > 0) snprintf(s, n, "%s", C.said);
	else if (C.row == ROW_PRESET) snprintf(s, n, "%s", padmap_preset_about(C.family, C.style, padmap_preset_of(&C.pads, C.family)));
	else if (C.row == ROW_DPAD) snprintf(s, n, "%s: the D-pad's %s, way by way", a, C.col == SLOT_MAIN ? "MAIN" : "ALSO");
	else if (C.row == ROW_STATS) snprintf(s, n, "%s", analytics_word(AW_ROW_NOTE));
	else if (gba >= 0) snprintf(s, n, "%s: give %s its %s %s", a, padmap_gba_name(gba), C.col == SLOT_MAIN ? "MAIN" : "ALSO", C.tab == TAB_PAD ? "button" : "key");
	else if (C.item == ITEM_SETALL) snprintf(s, n, "Asks for every button in turn");
	else if (C.item == ITEM_DEFAULTS) snprintf(s, n, "The %s as it came", C.tab == TAB_PAD ? "controller" : "keyboard");
	else snprintf(s, n, "Keep them: press the new A");
}

/* The note in two lines at most: broken after the most words that fit;
 * whether it took two */
static bool note_draw(int cx, int y, const char *s, SDL_Color c) {
	char a[128], *second = NULL;
	snprintf(a, sizeof a, "%s", s);
	if (text_width(a) > CORE_W - 16) {
		char *cut = NULL;
		for (char *sp = strchr(a, ' '); sp; sp = strchr(sp + 1, ' ')) {
			*sp = 0;
			bool fits = text_width(a) <= CORE_W - 16;
			*sp = ' ';
			if (!fits) break;
			cut = sp;
		}
		if (cut) { *cut = 0; second = cut + 1; }
	}
	text_draw(cx, y, a, c, TEXT_CENTER);
	if (second) text_draw(cx, y + 11, second, c, TEXT_CENTER);
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
		snprintf(sub, sizeof sub, "%d of %d", C.ask_step + 1, PAD_GBA);
	}
	int secs = (C.ask_left + 59) / 60;
	snprintf(wait, sizeof wait, "%sWait %d to %s", KEYS ? "Esc or " : "", secs, C.ask == ASK_ALL ? "skip it" : "cancel");
	fill_rect(x0 + 18, y0 + 40, CORE_W - 36, 62, GOLD);
	fill_rect(x0 + 19, y0 + 41, CORE_W - 38, 60, DARK);
	text_draw(x0 + CORE_W / 2, y0 + 48, line, GOLD, TEXT_CENTER);
	text_draw(x0 + CORE_W / 2, y0 + 62, sub, SKY, TEXT_CENTER);
	minifont_draw_centered(x0 + CORE_W / 2, y0 + 82, wait, DIM, 1);
}

/* The keep box: the edited map's A, on the device used last */
static void confirm_draw(int x0, int y0) {
	char a[32], line[64], sub[48];
	if (C.tab == TAB_PAD && C.has_pad) controls_slot_words(4, SLOT_MAIN, a, sizeof a);
	else if (KEYS) snprintf(a, sizeof a, "%s", C.keys.slot[4][0] ? controls_key_word(C.keys.slot[4][0]) : "A");
	else snprintf(a, sizeof a, "A");
	snprintf(line, sizeof line, "Press %s to keep these", a);
	snprintf(sub, sizeof sub, "Not kept in %d", (C.confirm + 59) / 60);
	fill_rect(x0 + 18, y0 + 46, CORE_W - 36, 46, GOLD);
	fill_rect(x0 + 19, y0 + 47, CORE_W - 38, 44, DARK);
	text_draw(x0 + CORE_W / 2, y0 + 54, line, GOLD, TEXT_CENTER);
	text_draw(x0 + CORE_W / 2, y0 + 70, sub, SKY, TEXT_CENTER);
}

static void nav_draw(int cx, int y) {
	char nav[96];
	if (touch_shown() && !C.has_pad) snprintf(nav, sizeof nav, "Tap a slot to set it, a tab to switch");
	else if (KEYS)
		snprintf(nav, sizeof nav, "%s: set  %s: clear  %s: tab  %s: back", controls_nav_word(NAV_A), controls_nav_word(NAV_X),
			P.keyboard_last || !C.has_pad ? "Tab" : controls_nav_word(NAV_R), controls_nav_word(NAV_B));
	else snprintf(nav, sizeof nav, "%s: set  %s: clear  %s: back", controls_nav_word(NAV_A), controls_nav_word(NAV_X), controls_nav_word(NAV_B));
	minifont_draw_centered(cx, y, nav, DIM, 1);
}

void controls_draw(void) {
	if (analytics_ask_shown()) { analytics_ask_draw(); return; }
	if (!C.open) return;
	int x0 = P.core_x, y0 = P.core_y, cx = x0 + CORE_W / 2;
	/* the paused game dimmed under the PET's panel */
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 110));
	fill_rect(x0 + 4, y0 + 4, CORE_W - 8, CORE_H - 8, CYAN);
	fill_rect(x0 + 6, y0 + 6, CORE_W - 12, CORE_H - 12, rgba(16, 60, 90, 250));
	tabs_draw(x0, y0);
	rows_draw(x0, y0);
	bottom_draw(x0, y0);
	if (C.ask) ask_draw(x0, y0);
	else if (C.confirm) confirm_draw(x0, y0);
	char note[128];
	note_words(note, sizeof note);
	if (!C.ask && !C.confirm) {
		bool two = note_draw(cx, y0 + NOTE_Y, note, C.said_t > 0 && C.said_warn ? ORANGE : SKY);
		/* (under a note of two lines with every row there, its second line
		 * takes the keys' place: the note says what A does) */
		if (!(two && NOTE_Y + 11 + TEXT_H > NAV_Y)) nav_draw(cx, y0 + NAV_Y);
	}
}

#include "controls.h"

#include <stdio.h>
#include <string.h>

#include "analytics_ask.h"
#include "audio.h"
#include "buttons.h"
#include "controls_state.h"
#include "pads.h"
#include "platform.h"
#include "touch.h"

ControlsState C;

int controls_gba_of(int row) { return row >= ROW_A && row <= ROW_SELECT ? row - ROW_A + 4 : -1; }
int controls_rows(void) { return analytics_ask_here() ? ROWS : ROW_STATS; }

static void say(const char *s, bool warn) {
	snprintf(C.said, sizeof C.said, "%s", s);
	C.said_t = NOTE;
	C.said_warn = warn;
}

static void refresh_pad(void) {
	const char *name;
	C.has_pad = pads_last(&C.family, &C.style, &name);
	if (C.has_pad) snprintf(C.name, sizeof C.name, "%s", name);
	else { C.family = PAD_FAMILY_XBOX; C.style = PAD_STYLE_GENERIC; }
}

void controls_open(void) {
	memset(&C, 0, sizeof C);
	C.open = true;
	C.pads = C.pads_was = *pads_map();
	platform_keys_get(&C.keys);
	C.keys_was = C.keys;
	refresh_pad();
	/* (the keyboard's tab first where a key was the last thing used, or
	 * no pad is there) */
	C.tab = KEYS && (P.keyboard_last || !C.has_pad) ? TAB_KEYS : TAB_PAD;
	C.row = ROW_A;
	C.item = ITEM_DONE;
	/* (the finger that opened it presses nothing more) */
	touch_release();
}

/* (and the statistics' question in its place: analytics_ask.h) */
bool controls_shown(void) { return C.open || analytics_ask_shown(); }

bool controls_takes_buttons(void) { return !C.open && analytics_ask_shown(); }

static void leave(void) {
	C.open = false;
	audio_sfx(SFX_CANCEL);
}

static bool unchanged(void) { return padmap_same(&C.pads, &C.pads_was) && !memcmp(&C.keys, &C.keys_was, sizeof C.keys); }

static void keep(void) {
	if (!padmap_same(&C.pads, &C.pads_was)) {
		pads_set_map(&C.pads);
		pads_save();
	}
	if (memcmp(&C.keys, &C.keys_was, sizeof C.keys)) platform_keys_set(&C.keys);
	audio_sfx(SFX_CONFIRM);
	C.open = false;
}

/* The next preset for A and B, or the one before (a pad's: the keyboard
 * has none) */
static void cycle(int d) {
	if (C.tab == TAB_KEYS) return;
	int n = padmap_presets(C.family), p = padmap_preset_of(&C.pads, C.family);
	p = p < 0 ? (d > 0 ? 0 : n - 1) : (p + d + n) % n;
	padmap_preset_apply(&C.pads, C.family, p);
	audio_sfx(SFX_CURSOR);
}

/* ---- a slot set, and what it moved ---- */

/* the input in words on its own device */
static const char *input_word(int input, int family) {
	return family < 0 ? controls_key_word(input) : padmap_label_of(family, C.style, input);
}

/* the other GBA button an input is on, -1 for none */
static int owner_of(int gba, int input, int family) {
	for (int g = 0; g < PAD_GBA; ++g)
		for (int i = 0; g != gba && i < PAD_PER; ++i) {
			int in = family < 0 ? platform_keys_slot(&C.keys, g, i) : (C.pads.in[family][g][i] == PAD_NONE ? -1 : C.pads.in[family][g][i]);
			if (in == input) return g;
		}
	return -1;
}

/* `input` into GBA button `gba`'s slot, the move said in a line ("Swapped:
 * A now on B", "Taken from L, which keeps LT") or why it was not ("Start
 * is START's only button"); whether it was put */
static bool put(int gba, int slot, int input, int family) {
	char s[72];
	int owner = owner_of(gba, input, family);
	int old = family < 0 ? platform_keys_slot(&C.keys, gba, slot) : C.pads.in[family][gba][slot];
	int r = family < 0 ? platform_keys_set_slot(&C.keys, gba, slot, input) : padmap_set_slot(&C.pads, family, gba, slot, input);
	if (r == PAD_SLOT_REFUSED) {
		snprintf(s, sizeof s, "%s is %s's only %s", input_word(input, family), padmap_gba_name(owner), family < 0 ? "key" : "button");
		say(s, true);
		return false;
	}
	if (r < 0) return true;
	bool swapped = old != (family < 0 ? 0 : PAD_NONE) && owner_of(gba, old, family) == r;
	int kept = family < 0 ? platform_keys_slot(&C.keys, r, 0) : C.pads.in[family][r][0];
	if (swapped) snprintf(s, sizeof s, "Swapped: %s now on %s", padmap_gba_name(r), input_word(old, family));
	else snprintf(s, sizeof s, "Taken from %s, which keeps %s", padmap_gba_name(r), input_word(kept, family));
	say(s, false);
	return true;
}

/* ---- the box: a slot, the D-pad's four, Set all's ten ---- */

static void ask_start(int ask, int gba, int slot) {
	C.pads_before = C.pads;
	C.keys_before = C.keys;
	C.ask = ask;
	C.ask_gba = gba;
	C.ask_slot = slot;
	C.ask_step = 0;
	C.ask_left = ASK_TIME;
	C.settle = SETTLE;
	C.said_t = 0;
	audio_sfx(SFX_SELECT);
}

/* (a slot and the D-pad back as they were; Set all keeps what it set) */
static void ask_cancel(void) {
	if (C.ask != ASK_ALL) {
		C.pads = C.pads_before;
		C.keys = C.keys_before;
		say("Not set: as it was", false);
	} else
		say("Stopped: the rest as they were", false);
	C.ask = ASK_NONE;
	audio_sfx(SFX_CANCEL);
}

static bool anything_held(void) {
	bool held = pads_any_held();
#if KEYS
	int nkeys = 0;
	const Uint8 *ks = SDL_GetKeyboardState(&nkeys);
	for (int sc = 1; sc < nkeys && !held; ++sc) held = ks[sc] != 0;
#endif
	return held;
}

/* The first press of the tab's device (a pad's on CONTROLLER, a key on
 * KEYBOARD) once the last press is let go: its input and family (-1 for
 * a key). A pad's guide button is its system's. */
static bool take_press(int *input, int *family) {
	if (C.settle > 0) {
		if (!anything_held()) --C.settle;
		return false;
	}
	if (C.tab == TAB_PAD) {
		PadPress p[8];
		int n = pads_presses(p, 8);
		for (int i = 0; i < n; ++i)
			if (p[i].family == PAD_FAMILY_RAW || p[i].input != PAD_GUIDE) {
				*input = p[i].input;
				*family = p[i].family;
				return true;
			}
		return false;
	}
#if KEYS
	int k[8];
	int n = platform_keys_pressed(k, 8);
	for (int i = 0; i < n; ++i)
		if (platform_key_free(k[i])) {
			*input = k[i];
			*family = -1;
			return true;
		}
#endif
	return false;
}

/* the next step of the D-pad's or Set all's, or the end */
static void ask_next(int steps, const char *done) {
	C.settle = SETTLE;
	C.ask_left = ASK_TIME;
	if (++C.ask_step < steps) return;
	C.ask = ASK_NONE;
	say(done, false);
	audio_sfx(SFX_CONFIRM);
}

static void ask_update(void) {
	int input, family;
	if (take_press(&input, &family)) {
		if (C.ask == ASK_SLOT) {
			C.ask = ASK_NONE;
			audio_sfx(put(C.ask_gba, C.ask_slot, input, family) ? SFX_CONFIRM : SFX_CANCEL);
		} else if (C.ask == ASK_DPAD) {
			if (put(C.ask_step, C.ask_slot, input, family)) { audio_sfx(SFX_CURSOR); ask_next(4, "D-pad set: DONE keeps it"); }
		} else if (put(C.ask_step, SLOT_MAIN, input, family)) {
			audio_sfx(SFX_CURSOR);
			ask_next(PAD_GBA, "All set: DONE keeps them");
		}
		return;
	}
	if (--C.ask_left > 0) return;
	/* (waited out: Set all skips the button, the rest stop) */
	if (C.ask == ASK_ALL) ask_next(PAD_GBA, "All set: DONE keeps them");
	else ask_cancel();
}

/* ---- the list ---- */

/* A slot cleared: the D-pad's four, or a button's (MAIN cleared, ALSO
 * takes its place); never a button's last */
static void clear(void) {
	int gba = controls_gba_of(C.row);
	if (C.row != ROW_DPAD && gba < 0) return;
	bool ok = true;
	for (int g = C.row == ROW_DPAD ? 0 : gba; g <= (C.row == ROW_DPAD ? 3 : gba); ++g)
		ok &= C.tab == TAB_PAD ? padmap_clear_slot(&C.pads, C.family, g, C.col) : platform_keys_clear_slot(&C.keys, g, C.col);
	if (!ok) {
		char s[72];
		snprintf(s, sizeof s, "%s keeps one %s", C.row == ROW_DPAD ? "Each way" : padmap_gba_name(gba), C.tab == TAB_PAD ? "button" : "key");
		say(s, true);
	}
	audio_sfx(SFX_CANCEL);
}

static void defaults(void) {
	if (C.tab == TAB_PAD) padmap_default_family(&C.pads, C.family);
	else platform_keys_default(&C.keys);
	say("As it came: DONE keeps it", false);
	audio_sfx(SFX_SELECT);
}

/* Back, or DONE: away at once where nothing changed, else the keep box
 * (kept once the new A is pressed: a map whose A cannot be found goes back
 * to the old one by itself) */
static void finish(void) {
	if (unchanged()) leave();
	else {
		C.confirm = CONFIRM;
		audio_sfx(SFX_SELECT);
	}
}

static void choose(void) {
	if (C.row == ROW_PRESET) { cycle(1); return; }
	if (C.row == ROW_DPAD) { ask_start(ASK_DPAD, 0, C.col); return; }
	/* (the statistics' question over this screen, its cursor on the answer
	 * as it stands: what is sent is read before a yes) */
	if (C.row == ROW_STATS) { if (analytics_ask_open()) audio_sfx(SFX_SELECT); return; }
	if (C.row < ROWS) { ask_start(ASK_SLOT, controls_gba_of(C.row), C.col); return; }
	if (C.item == ITEM_SETALL) ask_start(ASK_ALL, 0, SLOT_MAIN);
	else if (C.item == ITEM_DEFAULTS) defaults();
	else finish();
}

static void switch_tab(void) {
	if (!KEYS) return;
	C.tab = !C.tab;
	if (C.row == ROW_PRESET && C.tab == TAB_KEYS) C.row = ROW_DPAD;
	audio_sfx(SFX_CURSOR);
}

static void step_item(int d) { C.item = (C.item + d + ITEMS) % ITEMS; }

/* the line above or below: the rows shown, then the bottom line */
static int line_from(int row, int d) {
	int first = C.tab == TAB_KEYS ? ROW_DPAD : ROW_PRESET, last = controls_rows() - 1;
	if (d < 0) return row == first ? ROW_BOTTOM : row == ROW_BOTTOM ? last : row - 1;
	return row == ROW_BOTTOM ? first : row == last ? ROW_BOTTOM : row + 1;
}

/* A key pressed this frame (Tab switches the tabs, Delete clears) */
static bool key_pressed(int sc) {
#if KEYS
	int k[8];
	int n = platform_keys_pressed(k, 8);
	for (int i = 0; i < n; ++i)
		if (k[i] == sc) return true;
#else
	(void)sc;
#endif
	return false;
}

static void nav_update(void) {
	uint32_t pr = P.menu_pressed, rep = P.menu_repeat;
	if (pr & BTN_B) { finish(); return; }
	if ((pr & (BTN_L | BTN_R)) || key_pressed(SDL_SCANCODE_TAB)) { switch_tab(); return; }
	int was = C.row * 16 + C.col * 4 + C.item;
	if (rep & BTN_UP) C.row = line_from(C.row, -1);
	else if (rep & BTN_DOWN) C.row = line_from(C.row, 1);
	int d = (rep & BTN_RIGHT) ? 1 : (rep & BTN_LEFT) ? -1 : 0;
	if (d && C.row == ROW_PRESET) cycle(d);
	else if (d && C.row == ROW_BOTTOM) step_item(d);
	else if (d) C.col = !C.col;
	if (C.row * 16 + C.col * 4 + C.item != was) audio_sfx(SFX_CURSOR);
	if (pr & (BTN_A | BTN_START)) choose();
	else if ((pr & BTN_SELECT) || key_pressed(SDL_SCANCODE_DELETE)) clear();
}

/* Each press lights the rows it presses through the map being set: it is
 * tried on the spot */
static void light(void) {
	PadPress p[8];
	int n = pads_presses(p, 8);
	for (int i = 0; i < n; ++i) {
		uint32_t bits = padmap_bits(&C.pads, p[i].family, p[i].input);
		for (int g = 0; g < PAD_GBA; ++g)
			if (bits >> g & 1) C.lit[g] = LIT;
	}
#if KEYS
	int k[8];
	n = platform_keys_pressed(k, 8);
	for (int i = 0; i < n; ++i)
		for (int g = 0; g < PAD_GBA; ++g)
			for (int s = 0; s < KEYS_PER; ++s)
				if (C.keys.slot[g][s] && C.keys.slot[g][s] == k[i]) C.lit[g] = LIT;
#endif
}

/* The edited map's A pressed, on a pad or a key */
static bool new_a_pressed(void) {
	PadPress p[8];
	int n = pads_presses(p, 8);
	for (int i = 0; i < n; ++i)
		if (padmap_bits(&C.pads, p[i].family, p[i].input) & BTN_A) return true;
#if KEYS
	int k[8];
	n = platform_keys_pressed(k, 8);
	for (int i = 0; i < n; ++i)
		for (int s = 0; s < KEYS_PER; ++s)
			if (C.keys.slot[4][s] && C.keys.slot[4][s] == k[i]) return true;
#endif
	return false;
}

/* the keep box: the new A keeps, Back leaves them, waiting goes back to
 * the list */
static void confirm_update(void) {
	if (new_a_pressed()) keep();
	else if (P.menu_pressed & BTN_B) leave();
	else if (--C.confirm == 0) {
		say("Not kept: as they were", true);
		audio_sfx(SFX_CANCEL);
	}
}

void controls_update(void) {
	if (analytics_ask_shown()) { analytics_ask_update(); return; }
	if (!C.open) return;
	++C.t;
	if (C.said_t > 0) --C.said_t;
	for (int g = 0; g < PAD_GBA; ++g)
		if (C.lit[g] > 0) --C.lit[g];
	refresh_pad();
	if (C.confirm) confirm_update();
	else if (C.ask) ask_update();
	else {
		light();
		nav_update();
	}
}

bool controls_back(void) {
	if (analytics_ask_shown()) return analytics_ask_back();
	if (!C.open) return false;
	if (C.ask) ask_cancel();
	else if (C.confirm) leave();
	else finish();
	return true;
}

/* ---- taps ---- */

static void tap(int x, int y) {
	if (C.ask) { ask_cancel(); return; }
	if (C.confirm) {
		if (controls_tap_item(x, y) >= 0) keep();
		return;
	}
	int tab = controls_tap_tab(x, y);
	if (tab >= 0) {
		if (tab != C.tab) switch_tab();
		return;
	}
	int row, col, item = controls_tap_item(x, y);
	if (item >= 0) {
		C.row = ROW_BOTTOM;
		C.item = item;
		choose();
	} else if (controls_tap_cell(x, y, &row, &col) && row < controls_rows() && !(row == ROW_PRESET && C.tab == TAB_KEYS)) {
		C.row = row;
		C.col = col;
		choose();
	}
}

void controls_finger(uint32_t type, SDL_FingerID id, int x, int y) {
	bool ask = analytics_ask_shown();
	if (!C.open && !ask) return;
	if (type == SDL_FINGERDOWN) {
		C.finger = id;
		C.finger_down = true;
	} else if (type == SDL_FINGERUP && C.finger_down && id == C.finger) {
		C.finger_down = false;
		if (ask) analytics_ask_tap(x - P.core_x, y - P.core_y);
		else tap(x - P.core_x, y - P.core_y);
	}
}

/* ---- the words for the other screens' prompts ---- */

const char *controls_word(uint32_t bit) {
	static char s[32];
	int gba = 0, family, style;
	const char *name;
	while (gba < PAD_GBA - 1 && !(bit >> gba & 1)) ++gba;
	if (touch_shown()) return padmap_gba_name(gba);
	if (!P.keyboard_last && pads_last(&family, &style, &name)) {
		int in = pads_map()->in[family][gba][0];
		return in == PAD_NONE ? padmap_gba_name(gba) : padmap_label_of(family, style, in);
	}
	if (!KEYS) return padmap_gba_name(gba);
	platform_keys_label(NULL, gba, s, sizeof s);
	char *comma = strchr(s, ',');
	if (comma) *comma = 0;
	return s[0] ? s : padmap_gba_name(gba);
}

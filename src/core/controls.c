#include "controls.h"

#include <stdio.h>
#include <string.h>

#include "analytics.h"
#include "analytics_ask.h"
#include "analytics_text.h"
#include "audio.h"
#include "buttons.h"
#include "gfx.h"
#include "minifont.h"
#include "pads.h"
#include "platform.h"
#include "touch.h"

/* (desktops and browsers have a keyboard to set as well) */
#if defined(CW_DESKTOP) || defined(__EMSCRIPTEN__)
#define KEYS 1
#else
#define KEYS 0
#endif

/* The rows: the preset for A and B, the six buttons, the anonymous
 * statistics where the build can send them (issue #104, analytics.h: on
 * or off, and A their question again, what is sent and Yes or No, kept at
 * once), then DEFAULTS and DONE side by side. (The D-pad
 * and the stick move, set in pad.ini and keys.ini only: a direction given
 * one button would lose the others.) */
enum { ROW_PRESET, ROW_A, ROW_B, ROW_L, ROW_R, ROW_START, ROW_SELECT, ROW_STATS, ROW_DEFAULTS, ROW_DONE };
#define LISTEN 300     /* frames "press a button" waits */
#define CONFIRM 600    /* ... and "press A to keep" */
#define NOTE 150       /* ... a passing word stays */
/* where it draws, from the picture's corner: with the statistics' row the
 * rows a pixel closer and DEFAULTS and DONE just under them, so the note
 * keeps its two lines over the keys' */
#define ROW_Y 27
#define ROW_H (stats_row() ? 11 : 12)
#define BOTTOM_Y (stats_row() ? 113 : 112)
#define LABEL_X 17
#define PAD_X 74
#define KEY_X 174

static struct {
	bool open;
	int row, listen, confirm, t;
	int family, style;     /* the pad shown: the one used last */
	bool has_pad;
	char name[40];
	PadMap pads, pads_was;
	char said[64];
	int said_t;
	SDL_FingerID finger;   /* (a finger that went down on it: only its lift taps) */
	bool finger_down;
} C;
static KeyMap keys, keys_was;

static int gba_of(int row) { return row - ROW_A + 4; }   /* (A is the GBA's fifth button, buttons.h) */
static bool button_row(int row) { return row >= ROW_A && row <= ROW_SELECT; }
static bool stats_row(void) { return analytics_ask_here(); }
/* the last row of the list: the statistics', where they are here */
static int last_row(void) { return stats_row() ? ROW_STATS : ROW_SELECT; }

static void say(const char *s) {
	snprintf(C.said, sizeof C.said, "%s", s);
	C.said_t = NOTE;
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
	platform_keys_get(&keys);
	keys_was = keys;
	refresh_pad();
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

static bool unchanged(void) { return padmap_same(&C.pads, &C.pads_was) && !memcmp(&keys, &keys_was, sizeof keys); }

static void keep(void) {
	if (!padmap_same(&C.pads, &C.pads_was)) {
		pads_set_map(&C.pads);
		pads_save();
	}
	if (memcmp(&keys, &keys_was, sizeof keys)) platform_keys_set(&keys);
	audio_sfx(SFX_CONFIRM);
	C.open = false;
}

/* The next preset for A and B, or the one before */
static void cycle(int d) {
	int n = padmap_presets(C.family), p = padmap_preset_of(&C.pads, C.family);
	p = p < 0 ? (d > 0 ? 0 : n - 1) : (p + d + n) % n;
	padmap_preset_apply(&C.pads, C.family, p);
	audio_sfx(SFX_CURSOR);
}

static void choose(int row) {
	C.row = row;
	if (row == ROW_PRESET) cycle(1);
	/* (the statistics' question over this screen, the cursor on the answer
	 * as it stands: what is sent is read before a yes) */
	else if (row == ROW_STATS) { if (analytics_ask_open()) audio_sfx(SFX_SELECT); }
	else if (button_row(row)) {
		C.listen = LISTEN;
		audio_sfx(SFX_SELECT);
	} else if (row == ROW_DEFAULTS) {
		padmap_default(&C.pads);
		platform_keys_default(&keys);
		say("All as they came: DONE keeps them");
		audio_sfx(SFX_SELECT);
	} else if (unchanged()) {
		leave();
	} else {
		/* (kept once the new A is pressed: a map whose A cannot be found
		 * goes back to the old one by itself) */
		C.confirm = CONFIRM;
		audio_sfx(SFX_SELECT);
	}
}

/* ---- the frame ---- */

/* A pad's press or a key taken for the row (the guide button is the
 * system's on many pads: Steam's overlay, a phone's home) */
static bool listen_take(void) {
	PadPress p[8];
	int n = pads_presses(p, 8);
	for (int i = 0; i < n; ++i)
		if (p[i].input != PAD_GUIDE) {
			padmap_bind(&C.pads, p[i].family, gba_of(C.row), p[i].input);
			return true;
		}
#if KEYS
	int k[8];
	n = platform_keys_pressed(k, 8);
	for (int i = 0; i < n; ++i)
		if (platform_key_free(k[i])) {
			platform_keys_bind(&keys, gba_of(C.row), k[i]);
			return true;
		}
#endif
	return false;
}

static void listen_update(void) {
	if (listen_take()) {
		C.listen = 0;
		audio_sfx(SFX_CONFIRM);
	} else if (--C.listen == 0) {
		say("Nothing pressed");
		audio_sfx(SFX_CANCEL);
	}
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
		if (k[i] >= 0 && k[i] < SDL_NUM_SCANCODES && (keys.bits[k[i]] & BTN_A)) return true;
#endif
	return false;
}

static void confirm_update(void) {
	if (new_a_pressed()) keep();
	else if (--C.confirm == 0) {
		say("Not kept: the controls stay as they were");
		audio_sfx(SFX_CANCEL);
	}
}

static int up_from(int row) { return row == ROW_PRESET ? ROW_DONE : row >= ROW_DEFAULTS ? last_row() : row - 1; }
static int down_from(int row) { return row == last_row() ? ROW_DONE : row >= ROW_DEFAULTS ? ROW_PRESET : row + 1; }

static void nav_update(void) {
	if (P.menu_pressed & BTN_B) { leave(); return; }
	int was = C.row, d = (P.menu_repeat & BTN_RIGHT) ? 1 : (P.menu_repeat & BTN_LEFT) ? -1 : 0;
	if (P.menu_repeat & BTN_UP) C.row = up_from(C.row);
	else if (P.menu_repeat & BTN_DOWN) C.row = down_from(C.row);
	if (d && C.row == ROW_PRESET) cycle(d);
	else if (d && C.row >= ROW_DEFAULTS) C.row = C.row == ROW_DONE ? ROW_DEFAULTS : ROW_DONE;
	if (C.row != was) audio_sfx(SFX_CURSOR);
	/* (a keyboard's Enter, START, chooses too) */
	if (P.menu_pressed & (BTN_A | BTN_START)) choose(C.row);
}

void controls_update(void) {
	if (analytics_ask_shown()) { analytics_ask_update(); return; }
	if (!C.open) return;
	++C.t;
	if (C.said_t > 0) --C.said_t;
	refresh_pad();
	if (C.confirm) confirm_update();
	else if (C.listen) listen_update();
	else nav_update();
}

bool controls_back(void) {
	if (analytics_ask_shown()) return analytics_ask_back();
	if (!C.open) return false;
	if (C.listen) {
		C.listen = 0;
		audio_sfx(SFX_CANCEL);
	} else if (C.confirm) {
		C.confirm = 0;
		say("Not kept");
		audio_sfx(SFX_CANCEL);
	} else leave();
	return true;
}

/* ---- taps ---- */

static int row_at(int x, int y) {
	if (x < 6 || x >= CORE_W - 6) return -1;
	if (y >= BOTTOM_Y - 2 && y < BOTTOM_Y + ROW_H) return x < CORE_W / 2 ? ROW_DEFAULTS : ROW_DONE;
	for (int r = ROW_PRESET; r <= last_row(); ++r)
		if (y >= ROW_Y + r * ROW_H - 1 && y < ROW_Y + (r + 1) * ROW_H - 1) return r;
	return -1;
}

static void tap(int x, int y) {
	int row = row_at(x, y);
	if (C.listen) {
		C.listen = 0;
		say("Nothing set");
		audio_sfx(SFX_CANCEL);
	} else if (C.confirm) {
		if (row >= ROW_DEFAULTS) keep();
	} else if (row >= 0) {
		choose(row);
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

/* ---- the words ---- */

/* `s` cut where its comma-parted words fit `w` pixels (the first kept) */
static void fit(char *s, int w) {
	while (text_width(s) > w) {
		char *comma = strrchr(s, ',');
		if (!comma) {
			size_t n = strlen(s);
			while (n > 1 && text_width(s) > w) s[--n] = 0;
			return;
		}
		*comma = 0;
	}
}

static void pad_words(int gba, char *out, size_t n, int w) {
	out[0] = 0;
	const uint8_t *in = C.pads.in[C.family][gba];
	for (int i = 0; i < PAD_PER && in[i] != PAD_NONE; ++i) {
		size_t m = strlen(out);
		snprintf(out + m, n - m, "%s%s", i ? ", " : "", padmap_label(C.style, in[i]));
	}
	if (!out[0]) snprintf(out, n, "-");
	fit(out, w);
}

/* The words for the screen's own choose (A) or back (B) on the device
 * used last: the pad's own button, the keyboard's key, or a tap. */
static const char *nav_word(bool back) {
	static char words[2][24];   /* (a word each: both go in one line) */
	char *s = words[back];
	if (touch_shown() && !C.has_pad) return back ? "Back" : "Tap";
	if (P.keyboard_last || !C.has_pad) {
		if (!KEYS) return back ? "B" : "A";
		platform_keys_label(NULL, back ? 5 : 4, s, sizeof words[0]);
		char *comma = strchr(s, ',');
		if (comma) *comma = 0;
		return s;
	}
	return padmap_label(C.style, back ? PAD_B : PAD_A);
}

const char *controls_word(uint32_t bit) {
	static char s[32];
	int gba = 0, family, style;
	const char *name;
	while (gba < PAD_GBA - 1 && !(bit >> gba & 1)) ++gba;
	if (touch_shown()) return padmap_gba_name(gba);
	if (!P.keyboard_last && pads_last(&family, &style, &name)) {
		int in = pads_map()->in[family][gba][0];
		return in == PAD_NONE ? padmap_gba_name(gba) : padmap_label(style, in);
	}
	if (!KEYS) return padmap_gba_name(gba);
	platform_keys_label(NULL, gba, s, sizeof s);
	char *comma = strchr(s, ',');
	if (comma) *comma = 0;
	return s[0] ? s : padmap_gba_name(gba);
}

/* The edited map's A, on the device used last */
static void new_a_word(char *s, size_t n) {
	if (C.has_pad && !P.keyboard_last) pad_words(4, s, n, 90);
	else if (KEYS) platform_keys_label(&keys, 4, s, n);
	else snprintf(s, n, "A");
	char *comma = strchr(s, ',');
	if (comma) *comma = 0;
}

/* What the row the cursor is on does, or what it waits for */
static void row_note(char *s, size_t n) {
	if (C.confirm) {
		char a[40];
		new_a_word(a, sizeof a);
		snprintf(s, n, "Press %s to keep them\nBack to the old ones in %d", a, (C.confirm + 59) / 60);
	} else if (C.listen) {
		snprintf(s, n, KEYS ? "Press the button or key for %s. Wait to cancel" : "Press the button for %s. Wait to cancel", padmap_gba_name(gba_of(C.row)));
	} else if (C.said_t > 0) {
		snprintf(s, n, "%s", C.said);
	} else if (C.row == ROW_PRESET) {
		snprintf(s, n, "%s", padmap_preset_about(C.family, C.style, padmap_preset_of(&C.pads, C.family)));
	} else if (C.row == ROW_STATS) {
		snprintf(s, n, "%s", analytics_word(AW_ROW_NOTE));
	} else if (button_row(C.row)) {
		snprintf(s, n, "%s: give %s its button%s", nav_word(false), padmap_gba_name(gba_of(C.row)), KEYS ? " or key" : "");
	} else {
		snprintf(s, n, "%s", C.row == ROW_DEFAULTS ? "Every button as it came" : "Keep these: it asks you to press the new A first");
	}
}

/* ---- the picture ---- */

static const SDL_Color GOLD = { 255, 230, 90, 255 }, SKY = { 170, 200, 255, 255 }, ORANGE = { 255, 170, 40, 255 },
	DIM = { 120, 140, 170, 255 };

static void arrow(int x, int y) {
	for (int i = 0; i < 4; ++i) fill_rect(x + i, y + 2 + i, 1, 8 - 2 * i, ORANGE);
}

static void preset_draw(int x0, int y) {
	int p = padmap_preset_of(&C.pads, C.family);
	const char *v = padmap_preset_name(C.family, C.style, p);
	text_draw(x0 + PAD_X + 8, y, v, p < 0 ? DIM : WHITE, TEXT_LEFT);
	/* (the PET's small arrows: Left and Right change it) */
	int r = x0 + PAD_X + 8 + text_width(v) + 4;
	for (int i = 0; i < 4; ++i) {
		fill_rect(x0 + PAD_X + 3 - i, y + 2 + i, 1, 8 - 2 * i, ORANGE);
		fill_rect(r + i, y + 2 + i, 1, 8 - 2 * i, ORANGE);
	}
}

static void button_draw(int x0, int y, int row) {
	char s[64];
	bool asking = C.listen && C.row == row;
	if (asking) {
		if ((C.t / 16) % 2 == 0) {
			text_draw(x0 + PAD_X, y, "Press a button", GOLD, TEXT_LEFT);
			if (KEYS) text_draw(x0 + KEY_X, y, "or a key", GOLD, TEXT_LEFT);
		}
		return;
	}
	pad_words(gba_of(row), s, sizeof s, KEYS ? KEY_X - PAD_X - 6 : CORE_W - PAD_X - 10);
	text_draw(x0 + PAD_X, y, s, C.has_pad ? WHITE : DIM, TEXT_LEFT);
#if KEYS
	platform_keys_label(&keys, gba_of(row), s, sizeof s);
	fit(s, CORE_W - KEY_X - 8);
	text_draw(x0 + KEY_X, y, s[0] ? s : "-", WHITE, TEXT_LEFT);
#endif
}

static void rows_draw(int x0, int y0) {
	static const char *const names[] = { "A and B", "A", "B", "L", "R", "START", "SELECT" };
	for (int r = ROW_PRESET; r <= last_row(); ++r) {
		int y = y0 + ROW_Y + r * ROW_H;
		const char *label = r == ROW_STATS ? analytics_word(AW_ROW) : names[r];
		text_draw(x0 + LABEL_X, y, label, C.row == r ? GOLD : WHITE, TEXT_LEFT);
		if (r == ROW_PRESET) preset_draw(x0, y);
		/* (the statistics' answer after its name: no controller's or key's column) */
		else if (r == ROW_STATS)
			text_draw(x0 + LABEL_X + text_width(label) + 8, y, analytics_word(analytics_consent() == ANALYTICS_ON ? AW_ON : AW_OFF), WHITE, TEXT_LEFT);
		else button_draw(x0, y, r);
		if (C.row == r && !C.confirm) arrow(x0 + 9, y);
	}
}

static void bottom_draw(int x0, int y0) {
	int y = y0 + BOTTOM_Y, l = x0 + CORE_W / 4, r = x0 + CORE_W * 3 / 4;
	if (C.confirm) {
		text_draw(x0 + CORE_W / 2, y, "Keep", GOLD, TEXT_CENTER);
		return;
	}
	text_draw(l, y, "Defaults", C.row == ROW_DEFAULTS ? GOLD : WHITE, TEXT_CENTER);
	text_draw(r, y, "Done", C.row == ROW_DONE ? GOLD : WHITE, TEXT_CENTER);
	if (C.row == ROW_DEFAULTS) arrow(l - text_width("Defaults") / 2 - 10, y);
	if (C.row == ROW_DONE) arrow(r - text_width("Done") / 2 - 10, y);
}

/* The note in two lines at most: broken where it says (\n), else after
 * the most words that fit */
static void note_draw(int cx, int y, const char *s) {
	char a[128], *second;
	snprintf(a, sizeof a, "%s", s);
	second = strchr(a, '\n');
	if (second) *second++ = 0;
	else if (text_width(a) > CORE_W - 16) {
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
	text_draw(cx, y, a, SKY, TEXT_CENTER);
	if (second) text_draw(cx, y + 11, second, SKY, TEXT_CENTER);
}

void controls_draw(void) {
	if (analytics_ask_shown()) { analytics_ask_draw(); return; }
	if (!C.open) return;
	int x0 = P.core_x, y0 = P.core_y, cx = x0 + CORE_W / 2;
	/* the paused game dimmed under the PET's panel */
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 110));
	fill_rect(x0 + 4, y0 + 4, CORE_W - 8, CORE_H - 8, rgba(66, 198, 231, 255));
	fill_rect(x0 + 6, y0 + 6, CORE_W - 12, CORE_H - 12, rgba(16, 60, 90, 250));
	text_draw(cx, y0 + 7, "CONTROLS", GOLD, TEXT_CENTER);
	/* (the pad's name as far as its column reaches) */
	char name[40];
	snprintf(name, sizeof name, "%s", C.has_pad ? C.name : "No controller");
	for (size_t n = strlen(name); n > 1 && minifont_width(name, 1) > (KEYS ? KEY_X - PAD_X - 6 : CORE_W - PAD_X - 10); ) name[--n] = 0;
	minifont_draw(x0 + PAD_X, y0 + 21, name, C.has_pad ? SKY : DIM, 1);
	if (KEYS) minifont_draw(x0 + KEY_X, y0 + 21, "Keyboard", SKY, 1);
	rows_draw(x0, y0);
	bottom_draw(x0, y0);
	char note[128];
	row_note(note, sizeof note);
	note_draw(cx, y0 + 124, note);
	char nav[64];
	if (touch_shown() && !C.has_pad) snprintf(nav, sizeof nav, "Tap a row to choose it");
	else snprintf(nav, sizeof nav, "%s: choose   %s: back", nav_word(false), nav_word(true));
	if (!C.listen && !C.confirm) minifont_draw_centered(cx, y0 + 149, nav, DIM, 1);
}

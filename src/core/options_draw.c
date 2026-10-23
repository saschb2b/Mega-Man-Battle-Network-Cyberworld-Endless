/* Options' picture (options_state.h), in BN6's PET screens' look as its
 * KeyItem screen (pet_look.h): the band with Options and the section open,
 * its lines with their values in a window, the note on the line under the
 * cursor in the information box, the keys' line; where a tap lands on
 * them. Drawing changes nothing. */
#include "options.h"

#include <stdio.h>

#include "analytics.h"
#include "analytics_ask.h"
#include "analytics_text.h"
#include "buttons.h"
#include "controls.h"
#include "display.h"
#include "gfx.h"
#include "minifont.h"
#include "options_state.h"
#include "options_text.h"
#include "pet_panel.h"
#include "platform.h"
#include "touch.h"

/* where it draws, from the picture's corner: the lines in a window, the
 * note in the information box under it, the keys on the body */
#define WIN_Y 18
#define WIN_H 84
#define ROW_Y 25
#define ROW_H 18   /* (14 for five lines) */
#define LABEL_X 24
#define VALUE_R (CORE_W - 30)   /* a value's right edge, the chevron after it */
#define NOTE_Y 103
#define NOTE_H 42
#define NAV_Y 147

static int row_h(int n) { return n > 4 ? 14 : ROW_H; }

int options_tap_line(int x, int y) {
	int kinds[LINES_MAX], n = options_lines(kinds), h = row_h(n);
	if (y < ROW_Y - 2 || y >= ROW_Y - 2 + n * h || x < 6 || x >= CORE_W - 6) return -1;
	return (y - ROW_Y + 2) / h;
}

/* the PET's small arrows: the cursor's and a section's chevron point right,
 * a value's pair either way */
static void pointer(int x, int y, int size, bool left, SDL_Color c) {
	for (int i = 0; i < size; ++i) fill_rect(left ? x - i : x + i, y + 2 + i, 1, 2 * size - 2 * i, c);
}

/* the head: the section open as the band's note (Options is its title) */
static void head_draw(int x0, int y0) {
	if (O.section != SECTION_HUB)
		look_band_note(x0, y0, CORE_W, options_word(O.section == SECTION_DISPLAY ? OW_DISPLAY : OW_AUDIO), LOOK_PAGE);
}

static const char *sound_word(void) { return options_word(sound_filter() == SOUND_SOFT ? OW_SOFT : OW_GBA); }

/* a turned line's value: the window's, the scale's, smooth motion's, the sound's */
static const char *value_word(int kind) {
	switch (kind) {
	case LINE_FULLSCREEN: return options_word(P.fullscreen ? OW_ON : OW_OFF);
	case LINE_SCALE: return options_word((OptionsWord)(OW_AUTO + display_scale()));
	case LINE_SMOOTH: return options_word(P.blend ? OW_ON : OW_OFF);
	default: return sound_word();
	}
}

/* a line: its name, its value at the right with a section's chevron, or
 * a value between its arrows */
static void line_draw(int x0, int y, int kind, bool on) {
	static const OptionsWord names[] = { [LINE_CONTROLS] = OW_CONTROLS, [LINE_DISPLAY] = OW_DISPLAY, [LINE_AUDIO] = OW_AUDIO,
		[LINE_FULLSCREEN] = OW_FULLSCREEN, [LINE_SCALE] = OW_SCALE, [LINE_SMOOTH] = OW_SMOOTH, [LINE_SOUND] = OW_SOUND, [LINE_DONE] = OW_DONE };
	const char *name = kind == LINE_STATS ? analytics_word(AW_ROW) : options_word(names[kind]);
	look_text(x0 + LABEL_X, y, name, on ? LOOK_GOLD : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	if (on) look_cursor(x0 + 12, y + 2);
	const char *value = kind == LINE_AUDIO ? sound_word()
		: kind == LINE_STATS ? analytics_word(analytics_consent() == ANALYTICS_ON ? AW_ON : AW_OFF) : NULL;
	if (options_line_turns(kind)) {
		const char *v = value_word(kind);
		int r = x0 + VALUE_R, l = r - look_text_width(v);
		look_text(l, y, v, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		pointer(l - 5, y, 4, true, PET_ORANGE);
		pointer(r + 4, y, 4, false, PET_ORANGE);
		return;
	}
	if (value) look_text(x0 + VALUE_R - look_text_width(value), y, value, LOOK_EDGE_HI, LOOK_SHADE, TEXT_LEFT);
	if (kind != LINE_DONE) pointer(x0 + VALUE_R + 6, y + 1, 3, false, on ? LOOK_GOLD : LOOK_DIM);
}

static const char *note_words(int kind) {
	switch (kind) {
	case LINE_CONTROLS: return options_word(OW_CONTROLS_NOTE);
	case LINE_DISPLAY: return options_word(OW_DISPLAY_NOTE);
	case LINE_AUDIO: return options_word(OW_AUDIO_NOTE);
	case LINE_STATS: return analytics_word(AW_ROW_NOTE);
	case LINE_FULLSCREEN: return options_word(P.fullscreen ? OW_FULLSCREEN_ON_NOTE : OW_FULLSCREEN_OFF_NOTE);
	case LINE_SCALE: return options_word((OptionsWord)(OW_AUTO_NOTE + display_scale()));
	case LINE_SMOOTH: return options_word(P.blend ? OW_SMOOTH_ON_NOTE : OW_SMOOTH_OFF_NOTE);
	case LINE_SOUND: return options_word(sound_filter() == SOUND_SOFT ? OW_SOFT_NOTE : OW_GBA_NOTE);
	default: return options_word(O.section != SECTION_HUB ? OW_BACK_NOTE : OW_DONE_NOTE);
	}
}

/* the note in the information box: its white page, the chat font's ink */
static void note_draw(int x0, int y, const char *s) {
	SDL_Rect r = look_info(x0 + 7, y, CORE_W - 14, NOTE_H, NULL);
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	look_wrapped(s, r.x + 6, r.y + 1, r.w - 12, 3, LOOK_INK, LOOK_INK_SHADE);
}

static void options_panel_draw(void) {
	int x0 = P.core_x, y0 = P.core_y, cx = x0 + CORE_W / 2;
	pet_panel_draw(x0, y0, options_word(OW_TITLE));
	head_draw(x0, y0);
	look_window(x0 + 7, y0 + WIN_Y, CORE_W - 14, WIN_H, 0, false);
	int kinds[LINES_MAX], n = options_lines(kinds), h = row_h(n);
	for (int i = 0; i < n; ++i) line_draw(x0, y0 + ROW_Y + i * h, kinds[i], i == O.at);
	note_draw(x0, y0 + NOTE_Y, note_words(kinds[O.at < n ? O.at : n - 1]));
	char keys[64];
	if (touch_shown() && !platform_pad_present()) snprintf(keys, sizeof keys, "%s", options_word(OW_TAP));
	else {
		/* (controls_word's own buffer, one word at a time) */
		char a[32];
		snprintf(a, sizeof a, "%s", controls_word(BTN_A));
		options_keys(a, controls_word(BTN_B), options_line_turns(kinds[O.at < n ? O.at : n - 1]), keys, sizeof keys);
	}
	pet_panel_note(cx, y0 + NAV_Y, keys, LOOK_PAGE);
}

/* the question over all, else the controls screen over Options, else it */
void options_draw(void) {
	if (analytics_ask_shown()) analytics_ask_draw();
	else if (controls_shown()) controls_draw();
	else if (O.open) options_panel_draw();
}

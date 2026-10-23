/* Options' picture (options_state.h): the PET's panel as the controls
 * screen draws it, its head (Options, and the section open), its lines
 * with their values, the note on the line under the cursor and the keys'
 * line; where a tap lands on them. Drawing changes nothing. */
#include "options.h"

#include <stdio.h>

#include "analytics.h"
#include "analytics_ask.h"
#include "analytics_text.h"
#include "buttons.h"
#include "controls.h"
#include "gfx.h"
#include "minifont.h"
#include "options_state.h"
#include "options_text.h"
#include "pet_panel.h"
#include "platform.h"
#include "touch.h"

/* where it draws, from the picture's corner */
#define HEAD_Y 7
#define ROW_Y 30
#define ROW_H 18
#define LABEL_X 17
#define VALUE_R (CORE_W - 26)   /* a value's right edge, the chevron after it */
#define NOTE_Y 112
#define NAV_Y 149

int options_tap_line(int x, int y) {
	int kinds[LINES_MAX], n = options_lines(kinds);
	if (y < ROW_Y - 3 || y >= ROW_Y - 3 + n * ROW_H || x < 6 || x >= CORE_W - 6) return -1;
	return (y - ROW_Y + 3) / ROW_H;
}

/* the PET's small arrows: the cursor's and a section's chevron point right,
 * the sound's pair either way */
static void pointer(int x, int y, int size, bool left, SDL_Color c) {
	for (int i = 0; i < size; ++i) fill_rect(left ? x - i : x + i, y + 2 + i, 1, 2 * size - 2 * i, c);
}

/* the head: Options, and the section open after it */
static void head_draw(int x0, int y0) {
	const char *names[2] = { options_word(OW_TITLE), options_word(OW_AUDIO) };
	int x = 10;
	for (int t = 0; t <= (int)O.audio; ++t) {
		int w = minifont_width(names[t], 1) + 8;
		bool on = t == (int)O.audio;
		if (on) fill_rect(x0 + x - 2, y0 + HEAD_Y - 2, w + 4, 9, PET_CYAN);
		minifont_draw(x0 + x + 2, y0 + HEAD_Y, names[t], on ? PET_NAVY : PET_DIM, 1);
		x += w + 12;
	}
}

static const char *sound_word(void) { return options_word(sound_filter() == SOUND_SOFT ? OW_SOFT : OW_GBA); }

/* a line: its name, its value at the right with a section's chevron, or
 * the sound between its arrows */
static void line_draw(int x0, int y, int kind, bool on) {
	static const OptionsWord names[] = { [LINE_CONTROLS] = OW_CONTROLS, [LINE_AUDIO] = OW_AUDIO, [LINE_SOUND] = OW_SOUND, [LINE_DONE] = OW_DONE };
	const char *name = kind == LINE_STATS ? analytics_word(AW_ROW) : options_word(names[kind]);
	text_draw(x0 + LABEL_X, y, name, on ? PET_GOLD : WHITE, TEXT_LEFT);
	if (on) pointer(x0 + 9, y, 4, false, PET_ORANGE);
	const char *value = kind == LINE_AUDIO ? sound_word()
		: kind == LINE_STATS ? analytics_word(analytics_consent() == ANALYTICS_ON ? AW_ON : AW_OFF) : NULL;
	if (kind == LINE_SOUND) {
		const char *v = sound_word();
		int r = x0 + VALUE_R, l = r - text_width(v);
		text_draw(l, y, v, WHITE, TEXT_LEFT);
		pointer(l - 5, y, 4, true, PET_ORANGE);
		pointer(r + 4, y, 4, false, PET_ORANGE);
		return;
	}
	if (value) text_draw(x0 + VALUE_R - text_width(value), y, value, PET_SKY, TEXT_LEFT);
	if (kind != LINE_DONE) pointer(x0 + VALUE_R + 6, y + 1, 3, false, on ? PET_GOLD : PET_DIM);
}

static const char *note_words(int kind) {
	switch (kind) {
	case LINE_CONTROLS: return options_word(OW_CONTROLS_NOTE);
	case LINE_AUDIO: return options_word(OW_AUDIO_NOTE);
	case LINE_STATS: return analytics_word(AW_ROW_NOTE);
	case LINE_SOUND: return options_word(sound_filter() == SOUND_SOFT ? OW_SOFT_NOTE : OW_GBA_NOTE);
	default: return options_word(O.audio ? OW_BACK_NOTE : OW_DONE_NOTE);
	}
}

static void options_panel_draw(void) {
	int x0 = P.core_x, y0 = P.core_y, cx = x0 + CORE_W / 2;
	pet_panel_draw(x0, y0);
	head_draw(x0, y0);
	int kinds[LINES_MAX], n = options_lines(kinds);
	for (int i = 0; i < n; ++i) line_draw(x0, y0 + ROW_Y + i * ROW_H, kinds[i], i == O.at);
	controls_note_draw(cx, y0 + NOTE_Y, note_words(kinds[O.at < n ? O.at : n - 1]), PET_SKY);
	char keys[64];
	if (touch_shown() && !platform_pad_present()) snprintf(keys, sizeof keys, "%s", options_word(OW_TAP));
	else {
		/* (controls_word's own buffer, one word at a time) */
		char a[32];
		snprintf(a, sizeof a, "%s", controls_word(BTN_A));
		options_keys(a, controls_word(BTN_B), kinds[O.at < n ? O.at : n - 1] == LINE_SOUND, keys, sizeof keys);
	}
	minifont_draw_centered(cx, y0 + NAV_Y, keys, PET_DIM, 1);
}

/* the question over all, else the controls screen over Options, else it */
void options_draw(void) {
	if (analytics_ask_shown()) analytics_ask_draw();
	else if (controls_shown()) controls_draw();
	else if (O.open) options_panel_draw();
}

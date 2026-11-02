/* Options' picture (options_state.h), in BN6's PET screens' look
 * (pet_look.h): the band named Options, START's hint at its end where the
 * page has lines to put back (E-Mail's START:SORT); the page's lines in a
 * window whose band names the page between the Library's green arrows,
 * three rows at a time; under them the note on the line under the cursor
 * in KeyItem's information box and, on the Display page, the scene
 * (options_scene.h) beside it, Valhalla's list, description and picture
 * (docs/engine/settings-screens.md); START's question in the box with
 * BN6's Yes and No. Where a tap lands on them. Drawing changes nothing. */
#include "options.h"

#include "analytics.h"
#include "analytics_ask.h"
#include "analytics_text.h"
#include "controls.h"
#include "display.h"
#include "fast_forward.h"
#include "feel_out.h"
#include "gfx.h"
#include "options_scene.h"
#include "options_state.h"
#include "options_text.h"
#include "pet_panel.h"
#include "platform.h"
#include "sound_heard.h"

/* where it draws, from the picture's corner: the list's window, its band
 * (the page's name) and rows; under it the note and the scene */
#define TABS_Y    19
#define WIN_X     7
#define WIN_Y     (TABS_Y + LOOK_PILL_H + 3)
#define WIN_W     (CORE_W - 14)
#define ROW_H     13
#define ROW_Y     (WIN_Y + LOOK_FRAME + 2)
#define WIN_H     (OPTIONS_ROWS * ROW_H + 12)
#define LABEL_X   22
#define VALUE_R   (CORE_W - 32)   /* a value's right edge, the chevron after it */
#define BOX_Y     (WIN_Y + WIN_H + 3)
#define BOX_H     (CORE_H - 6 - BOX_Y)
#define NOTE_W    110             /* the note's beside the scene; the whole width without it */
#define TABS_L    (WIN_X + 18)    /* the tabs' row, between the arrows */
#define TABS_R    (WIN_X + WIN_W - 18)
#define TAB_GAP   3

/* the hint's words and where it lies (look_band_hint's) */
static SDL_Rect hint_rect(int x0, int y0) {
	int ww = look_hint_width("START", options_word(OW_DEFAULT)), hx = x0 + CORE_W - 10 - ww;
	return (SDL_Rect){ hx - 2, y0 + 9, ww + 4, 11 };
}

/* the note's box: beside the scene on the Display page */
static SDL_Rect box_rect(int x0, int y0) {
	return (SDL_Rect){ x0 + WIN_X, y0 + BOX_Y, O.page == PAGE_DISPLAY ? NOTE_W : WIN_W, BOX_H };
}

/* Yes and No on the question's last line in the box's page `r` */
static SDL_Rect answer_rect(SDL_Rect r, bool yes) {
	int y = r.y + r.h - LOOK_LINE - 1, x = r.x + 6 + LOOK_CHOICE_W + 2;
	if (!yes) x += look_text_width(analytics_word(AW_YES)) + LOOK_CHOICE_W + 12;
	return (SDL_Rect){ x - LOOK_CHOICE_W - 3, y - 1, (yes ? look_text_width(analytics_word(AW_YES)) : look_text_width(analytics_word(AW_NO))) +
		LOOK_CHOICE_W + 6, LOOK_LINE + 2 };
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h; }

int options_tap(int x, int y) {
	SDL_Rect box = box_rect(0, 0);
	if (O.asking) {
		SDL_Rect page = { box.x + 2, box.y + 8, box.w - 4, box.h - 10 };
		return inside(answer_rect(page, true), x, y) ? TAP_YES : inside(answer_rect(page, false), x, y) ? TAP_NO : TAP_NONE;
	}
	if (options_page_resets() && inside(hint_rect(0, 0), x, y)) return TAP_DEFAULT;
	/* (the tabs' row: an arrow's end turns a page, a tab shows its own) */
	if (y >= TABS_Y - 2 && y < TABS_Y + LOOK_PILL_H + 2 && x >= WIN_X && x < WIN_X + WIN_W)
		return x < TABS_L ? TAP_PREV : x >= TABS_R ? TAP_NEXT : TAP_PAGE - (x - TABS_L) * PAGE_COUNT / (TABS_R - TABS_L);
	int kinds[LINES_MAX], n = options_lines(kinds), row = (y - ROW_Y + 2) / ROW_H;
	if (y < ROW_Y - 2 || row >= OPTIONS_ROWS || O.top + row >= n || x < WIN_X || x >= WIN_X + WIN_W) return TAP_NONE;
	return O.top + row;
}

static const char *page_word(int page) {
	static const OptionsWord names[PAGE_COUNT] = { OW_DISPLAY, OW_AUDIO, OW_GAME };
	return options_word(names[page]);
}

static const char *sound_word(void) { return options_word(sound_filter() == SOUND_SOFT ? OW_SOFT : OW_GBA); }

/* the colour correction's words and notes, by LcdColour */
static const OptionsWord colour_words[LCD_COLOUR_COUNT] = { OW_OFF, OW_GBA, OW_VBA, OW_NDS, OW_PSP };
static const OptionsWord colour_notes[LCD_COLOUR_COUNT] = { OW_COLOUR_OFF_NOTE, OW_COLOUR_GBA_NOTE, OW_COLOUR_VBA_NOTE, OW_COLOUR_NDS_NOTE,
	OW_COLOUR_PSP_NOTE };

/* a turned line's value: the window's, the scale's, smooth motion's, the GBA's screen's, the music's, the effects',
 * fast forward's, the vibration's, the trigger effects', the sound's */
static const char *value_word(int kind) {
	switch (kind) {
	case LINE_FULLSCREEN: return options_word(P.fullscreen ? OW_ON : OW_OFF);
	case LINE_SCALE: return options_word((OptionsWord)(OW_AUTO + display_scale()));
	case LINE_SMOOTH: return options_word(P.blend ? OW_ON : OW_OFF);
	case LINE_COLOUR: return options_word(colour_words[lcd_colour()]);
	case LINE_BLEND: return options_word(lcd_blend() == LCD_BLEND_OFF ? OW_OFF : (OptionsWord)(OW_SIMPLE + lcd_blend() - LCD_BLEND_SIMPLE));
	case LINE_LCD3X: return options_word(lcd_grid() ? OW_ON : OW_OFF);
	case LINE_MUSIC: return options_word(sound_heard_music() ? OW_ON : OW_OFF);
	case LINE_EFFECTS: return options_word(sound_heard_effects() ? OW_ON : OW_OFF);
	case LINE_FAST: return fast_speed() > FAST_OFF ? fast_speed_word(fast_speed()) : options_word(OW_OFF);
	case LINE_VIBRATION: return options_word(feel_level() == FEEL_OFF ? OW_OFF : (OptionsWord)(OW_WEAK + feel_level() - FEEL_WEAK));
	case LINE_TRIGGERS: return options_word(feel_triggers() ? OW_ON : OW_OFF);
	default: return sound_word();
	}
}

/* a line: its name, its value at the right between the PET's arrows, or
 * a screen's chevron (Controls, Statistics) */
static void line_draw(int x0, int y, int kind, bool on) {
	static const OptionsWord names[] = { [LINE_CONTROLS] = OW_CONTROLS, [LINE_FULLSCREEN] = OW_FULLSCREEN, [LINE_SCALE] = OW_SCALE,
		[LINE_SMOOTH] = OW_SMOOTH, [LINE_SOUND] = OW_SOUND, [LINE_MUSIC] = OW_MUSIC, [LINE_EFFECTS] = OW_EFFECTS, [LINE_FAST] = OW_FAST,
		[LINE_VIBRATION] = OW_VIBRATION, [LINE_TRIGGERS] = OW_TRIGGERS, [LINE_COLOUR] = OW_COLOUR, [LINE_BLEND] = OW_BLEND, [LINE_LCD3X] = OW_LCD3X };
	const char *name = kind == LINE_STATS ? analytics_word(AW_ROW) : options_word(names[kind]);
	/* (BN6's lists keep a line's colour under the cursor: the arrow alone says where it is) */
	look_text(x0 + LABEL_X, y, name, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	if (on) look_cursor(x0 + 10, y + 2);
	if (options_line_turns(kind)) {
		const char *v = value_word(kind);
		int r = x0 + VALUE_R, l = r - look_text_width(v);
		look_text(l, y, v, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		look_pointer(l - 5, y, 4, true, PET_ORANGE);
		look_pointer(r + 4, y, 4, false, PET_ORANGE);
		return;
	}
	const char *value = kind == LINE_STATS ? analytics_word(analytics_consent() == ANALYTICS_ON ? AW_ON : AW_OFF) : NULL;
	if (value) look_text(x0 + VALUE_R - look_text_width(value), y, value, LOOK_EDGE_HI, LOOK_SHADE, TEXT_LEFT);
	look_pointer(x0 + VALUE_R + 6, y + 1, 3, false, on ? LOOK_GOLD : LOOK_DIM);
}

/* the Display page's lines' notes */
static const char *display_note(int kind) {
	switch (kind) {
	case LINE_FULLSCREEN: return options_word(P.fullscreen ? OW_FULLSCREEN_ON_NOTE : OW_FULLSCREEN_OFF_NOTE);
	case LINE_SCALE: return options_word((OptionsWord)(OW_AUTO_NOTE + display_scale()));
	case LINE_SMOOTH: return options_word(P.blend ? OW_SMOOTH_ON_NOTE : OW_SMOOTH_OFF_NOTE);
	case LINE_COLOUR: return options_word(colour_notes[lcd_colour()]);
	case LINE_BLEND: return options_word((OptionsWord)(OW_BLEND_OFF_NOTE + lcd_blend()));
	case LINE_LCD3X: return options_word(lcd_grid() ? OW_LCD3X_ON_NOTE : OW_LCD3X_OFF_NOTE);
	default: return "";
	}
}

static const char *note_words(int kind) {
	switch (kind) {
	case LINE_CONTROLS: return options_word(OW_CONTROLS_NOTE);
	case LINE_STATS: return analytics_word(AW_ROW_NOTE);
	case LINE_SOUND: return options_word(sound_filter() == SOUND_SOFT ? OW_SOFT_NOTE : OW_GBA_NOTE);
	case LINE_MUSIC: return options_word(sound_heard_music() ? OW_MUSIC_ON_NOTE : OW_MUSIC_OFF_NOTE);
	case LINE_EFFECTS: return options_word(sound_heard_effects() ? OW_EFFECTS_ON_NOTE : OW_EFFECTS_OFF_NOTE);
	case LINE_FAST: return options_word(fast_speed() > FAST_OFF ? OW_FAST_ON_NOTE : OW_FAST_OFF_NOTE);
	case LINE_VIBRATION: return options_word(feel_level() == FEEL_OFF ? OW_VIBRATION_OFF_NOTE : OW_VIBRATION_ON_NOTE);
	case LINE_TRIGGERS: return options_word(feel_triggers() ? OW_TRIGGERS_ON_NOTE : OW_TRIGGERS_OFF_NOTE);
	default: return display_note(kind);
	}
}

/* the pages' tabs over the list, STATUS's pills, the page shown NaviCust's
 * green RUN! (as the gate's buttons and SAVES' are, the one under the
 * cursor lit), each named in a screen's heavy capitals; the Library's
 * green arrows at the row's ends with L and R in a hint's small capitals
 * (Valhalla's tab row and its Q and E: docs/engine/settings-screens.md) */
static void tabs_draw(int x0, int y0) {
	int y = y0 + TABS_Y, w = (TABS_R - TABS_L - (PAGE_COUNT - 1) * TAB_GAP) / PAGE_COUNT;
	for (int p = 0; p < PAGE_COUNT; ++p) {
		int x = x0 + TABS_L + p * (w + TAB_GAP);
		if (p == O.page) look_button(x, y, w - 1, LOOK_PILL_H - 1);
		else look_pill(x, y, w);
		const char *name = page_word(p);
		look_title(x + (w - look_title_width(name)) / 2, y + 4, name, p == O.page ? LOOK_PAGE : LOOK_TEXT);
	}
	look_caps(x0 + WIN_X + 1, y + 5, "L", LOOK_PAGE, LOOK_GREEN_LO);
	look_arrow(x0 + WIN_X + 7, y + 4, false);
	look_arrow(x0 + TABS_R + 3, y + 4, true);
	look_caps(x0 + WIN_X + WIN_W - 6, y + 5, "R", LOOK_PAGE, LOOK_GREEN_LO);
}

/* the pages' tabs, then the page's lines in their window */
static void list_draw(int x0, int y0) {
	int kinds[LINES_MAX], n = options_lines(kinds), wy = y0 + WIN_Y;
	tabs_draw(x0, y0);
	look_window(x0 + WIN_X, wy, WIN_W, WIN_H, 0, true);
	for (int i = 0; i < OPTIONS_ROWS && O.top + i < n; ++i) line_draw(x0, y0 + ROW_Y + i * ROW_H, kinds[O.top + i], O.top + i == O.at[O.page]);
	look_scroll((SDL_Rect){ x0 + WIN_X, wy, WIN_W, WIN_H }, 0, O.top, OPTIONS_ROWS, n);
}

/* KeyItem's information box: the note on the cursor's line, or START's
 * question with BN6's Yes and No, the green triangle before the one chosen */
static void box_draw(int x0, int y0) {
	static const OptionsWord asks[PAGE_COUNT] = { OW_ASK_DISPLAY, OW_ASK_AUDIO, OW_ASK_GAME };
	SDL_Rect b = box_rect(x0, y0), r = look_info(b.x, b.y, b.w, b.h, options_word(OW_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	if (!O.asking) {
		look_wrapped(note_words(options_line_kind()), r.x + 5, r.y + 1, r.w - 10, 4, LOOK_INK, LOOK_INK_SHADE);
		return;
	}
	look_wrapped(options_word(asks[O.page]), r.x + 5, r.y + 1, r.w - 10, 3, LOOK_INK, LOOK_INK_SHADE);
	for (int k = 0; k < 2; ++k) {
		SDL_Rect a = answer_rect(r, k == 0);
		look_text(a.x + LOOK_CHOICE_W + 3, a.y + 1, analytics_word(k == 0 ? AW_YES : AW_NO), LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
		if ((k == 0) == O.yes) look_choice(a.x, a.y);
	}
}

static void options_panel_draw(void) {
	int x0 = P.core_x, y0 = P.core_y;
	options_scene_grab();
	pet_panel_draw(x0, y0, options_word(OW_TITLE));
	if (options_page_resets()) look_band_hint(x0, y0, CORE_W, "START", options_word(OW_DEFAULT));
	list_draw(x0, y0);
	box_draw(x0, y0);
	if (O.page == PAGE_DISPLAY) {
		int sx = x0 + WIN_X + NOTE_W + 3;
		options_scene_draw(look_slot(sx, y0 + BOX_Y, x0 + WIN_X + WIN_W - sx, BOX_H));
	}
	look_fade_in(x0, y0, CORE_W, CORE_H, O.t);
}

/* the question over all, else the controls screen over Options, else it */
void options_draw(void) {
	if (analytics_ask_shown()) analytics_ask_draw();
	else if (controls_shown()) controls_draw();
	else if (O.open) options_panel_draw();
}

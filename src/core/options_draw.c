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

#include <stdio.h>

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
#include "update.h"
#include "update_text.h"

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
#define SCENE_W   106             /* the scene's slot, where KeyItem's box holds its tag */
#define TEXT_IN   8               /* the note's letters in from the page's left, as KeyItem's */
#define TABS_L    (WIN_X + LOOK_TABS_IN)   /* the tabs' row, between the arrows (look_tabs') */
#define TABS_R    (WIN_X + WIN_W - LOOK_TABS_IN)

/* the hint's words and where it lies (look_band_hint's) */
static SDL_Rect hint_rect(int x0, int y0) {
	int ww = look_hint_width("START", options_word(OW_DEFAULT)), hx = x0 + CORE_W - 10 - ww;
	return (SDL_Rect){ hx - 2, y0 + 9, ww + 4, 11 };
}

/* KeyItem's information box across the foot */
static SDL_Rect box_rect(int x0, int y0) { return (SDL_Rect){ x0 + WIN_X, y0 + BOX_Y, WIN_W, BOX_H }; }

/* ... its white page under the label, to the right of the scene on the
 * Display page (where KeyItem's tag stands), the box's width elsewhere */
static SDL_Rect page_rect(int x0, int y0) {
	SDL_Rect b = box_rect(x0, y0);
	int px = b.x + 2 + (O.page == PAGE_DISPLAY ? SCENE_W + 3 : 0);
	return (SDL_Rect){ px, b.y + 2 + LOOK_CAPS_H + 2, b.x + b.w - 2 - px, b.h - LOOK_CAPS_H - 7 };
}

/* Yes and No on the question's last line in the box's page `r` */
static SDL_Rect answer_rect(SDL_Rect r, bool yes) {
	int y = r.y + r.h - LOOK_LINE - 1, x = r.x + TEXT_IN + LOOK_CHOICE_W + 2;
	if (!yes) x += look_text_width(analytics_word(AW_YES)) + LOOK_CHOICE_W + 12;
	return (SDL_Rect){ x - LOOK_CHOICE_W - 3, y - 1, (yes ? look_text_width(analytics_word(AW_YES)) : look_text_width(analytics_word(AW_NO))) +
		LOOK_CHOICE_W + 6, LOOK_LINE + 2 };
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h; }

int options_tap(int x, int y) {
	if (O.asking) {
		SDL_Rect page = page_rect(0, 0);
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

static const char *sound_word(void) {
	static const OptionsWord filters[SOUND_FILTER_COUNT] = { OW_GBA, OW_SOFT, OW_MGBA };
	return options_word(filters[sound_filter()]);
}

/* the colour correction's words and notes, by LcdColour */
static const OptionsWord colour_words[LCD_COLOUR_COUNT] = { OW_OFF, OW_GBA, OW_VBA, OW_NDS, OW_PSP };
static const OptionsWord colour_notes[LCD_COLOUR_COUNT] = { OW_COLOUR_OFF_NOTE, OW_COLOUR_GBA_NOTE, OW_COLOUR_VBA_NOTE, OW_COLOUR_NDS_NOTE,
	OW_COLOUR_PSP_NOTE };

/* a turned line's value: the window's, the scale's, smooth motion's, the GBA's screen's, the music's, the effects', the output's,
 * fast forward's, the vibration's, the trigger effects', the sound's */
static const char *value_word(int kind) {
	switch (kind) {
	case LINE_FULLSCREEN: return options_word(P.fullscreen ? OW_ON : OW_OFF);
	case LINE_SCALE: return options_word((OptionsWord)(OW_AUTO + display_scale()));
	case LINE_SMOOTH: return options_word(P.blend ? OW_ON : OW_OFF);
	case LINE_COLOUR: return options_word(colour_words[lcd_colour()]);
	case LINE_BLEND: return options_word(lcd_blend() == LCD_BLEND_OFF ? OW_OFF : (OptionsWord)(OW_SIMPLE + lcd_blend() - LCD_BLEND_SIMPLE));
	case LINE_LCD3X: return options_word(lcd_grid() ? OW_ON : OW_OFF);
	case LINE_VOXEL: return options_word(display_voxel() ? OW_ON : OW_OFF);
	case LINE_MUSIC: return options_level_word(sound_heard_music());
	case LINE_EFFECTS: return options_level_word(sound_heard_effects());
	case LINE_OUTPUT: return options_word((OptionsWord)(OW_SPEAKERS + sound_output()));
	case LINE_FAST: return fast_speed() > FAST_OFF ? fast_speed_word(fast_speed()) : options_word(OW_OFF);
	case LINE_VIBRATION: return options_word(feel_level() == FEEL_OFF ? OW_OFF : (OptionsWord)(OW_WEAK + feel_level() - FEEL_WEAK));
	case LINE_TRIGGERS: return options_word(feel_triggers() ? OW_ON : OW_OFF);
	case LINE_UPDATES: return options_word(update_on() ? OW_ON : OW_OFF);
	default: return sound_word();
	}
}

/* the update's line: "Update to 0.22.0" */
static const char *update_name(void) {
	static char s[48];
	snprintf(s, sizeof s, "%s %s", update_word(UW_GET), update_version());
	return s;
}

/* ... its value: the download's share in, or its failure (none else) */
static const char *update_value(void) {
	static char s[8];
	if (update_state() == UPDATE_FAILED) return update_word(UW_FAILED);
	if (update_state() != UPDATE_GETTING && update_state() != UPDATE_READY) return NULL;
	snprintf(s, sizeof s, "%d%%", update_percent());
	return s;
}

/* a line's name */
static const char *line_name(int kind) {
	static const OptionsWord names[] = { [LINE_CONTROLS] = OW_CONTROLS, [LINE_FULLSCREEN] = OW_FULLSCREEN, [LINE_SCALE] = OW_SCALE,
		[LINE_SMOOTH] = OW_SMOOTH, [LINE_SOUND] = OW_SOUND, [LINE_MUSIC] = OW_MUSIC, [LINE_EFFECTS] = OW_EFFECTS, [LINE_FAST] = OW_FAST,
		[LINE_VIBRATION] = OW_VIBRATION, [LINE_TRIGGERS] = OW_TRIGGERS, [LINE_COLOUR] = OW_COLOUR, [LINE_BLEND] = OW_BLEND, [LINE_LCD3X] = OW_LCD3X,
		[LINE_OUTPUT] = OW_OUTPUT, [LINE_VOXEL] = OW_VOXEL };
	switch (kind) {
	case LINE_STATS: return analytics_word(AW_ROW);
	case LINE_UPDATE: return update_name();
	case LINE_UPDATES: return update_word(UW_ROW);
	default: return options_word(names[kind]);
	}
}

/* a line: its name, its value at the right between the PET's arrows, or
 * a screen's chevron (Controls, Statistics, the update where A does something) */
static void line_draw(int x0, int y, int kind, bool on) {
	const char *name = line_name(kind);
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
	const char *value = kind == LINE_STATS ? analytics_word(analytics_consent() == ANALYTICS_ON ? AW_ON : AW_OFF) :
		kind == LINE_UPDATE ? update_value() : NULL;
	if (value) look_text(x0 + VALUE_R - look_text_width(value), y, value, LOOK_EDGE_HI, LOOK_SHADE, TEXT_LEFT);
	bool chevron = kind != LINE_UPDATE || (update_state() != UPDATE_GETTING && update_state() != UPDATE_READY &&
		(update_way() == UPDATE_WAY_INSTALL || update_way() == UPDATE_WAY_PAGE));
	if (chevron) look_pointer(x0 + VALUE_R + 6, y + 1, 3, false, on ? LOOK_GOLD : LOOK_DIM);
}

/* the update's note: what A does, by how this copy updates, or how the
 * download goes */
static const char *update_note(void) {
	static const UpdateWord ways[] = { [UPDATE_WAY_NONE] = UW_RELEASES_NOTE, [UPDATE_WAY_INSTALL] = UW_INSTALL_NOTE,
		[UPDATE_WAY_PAGE] = UW_PAGE_NOTE, [UPDATE_WAY_RELEASES] = UW_RELEASES_NOTE, [UPDATE_WAY_ITCH] = UW_ITCH_NOTE };
	UpdateState s = update_state();
	return update_word(s == UPDATE_FAILED ? UW_FAILED_NOTE : s == UPDATE_GETTING ? UW_GETTING_NOTE : s == UPDATE_READY ? UW_READY_NOTE : ways[update_way()]);
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
	case LINE_VOXEL: return options_word(display_voxel() ? OW_VOXEL_ON_NOTE : OW_VOXEL_OFF_NOTE);
	default: return "";
	}
}

/* Music's or Effects' note: at the game's own level, under it, or off */
static OptionsWord level_note(int level, OptionsWord full, OptionsWord low, OptionsWord off) {
	return level >= SOUND_LEVELS ? full : level > 0 ? low : off;
}

static const char *note_words(int kind) {
	switch (kind) {
	case LINE_CONTROLS: return options_word(OW_CONTROLS_NOTE);
	case LINE_STATS: return analytics_word(AW_ROW_NOTE);
	case LINE_UPDATE: return update_note();
	case LINE_UPDATES: return update_word(update_on() ? UW_ON_NOTE : UW_OFF_NOTE);
	case LINE_SOUND: return options_word(sound_filter() == SOUND_MGBA ? OW_MGBA_NOTE : sound_filter() == SOUND_SOFT ? OW_SOFT_NOTE : OW_GBA_NOTE);
	case LINE_OUTPUT: return options_word((OptionsWord)(OW_SPEAKERS_NOTE + sound_output()));
	case LINE_MUSIC: return options_word(level_note(sound_heard_music(), OW_MUSIC_ON_NOTE, OW_MUSIC_LOW_NOTE, OW_MUSIC_OFF_NOTE));
	case LINE_EFFECTS: return options_word(level_note(sound_heard_effects(), OW_EFFECTS_ON_NOTE, OW_EFFECTS_LOW_NOTE, OW_EFFECTS_OFF_NOTE));
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
	const char *names[PAGE_COUNT];
	for (int p = 0; p < PAGE_COUNT; ++p) names[p] = page_word(p);
	look_tabs(x0 + WIN_X, y0 + TABS_Y, WIN_W, PAGE_COUNT, O.page, names);
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
	SDL_Rect b = box_rect(x0, y0), r = page_rect(x0, y0);
	look_info(b.x, b.y, b.w, b.h, NULL);
	look_caps(r.x + 2, b.y + 2, options_word(OW_INFORMATION), LOOK_PAGE, (SDL_Color){ 0, 0, 0, 0 });
	fill_rect(r.x, r.y, r.w, r.h, LOOK_PAGE);
	/* (the scene in STATUS's slot, the box's height, beside the page) */
	if (O.page == PAGE_DISPLAY) options_scene_draw(look_slot(b.x + 2, b.y + 2, SCENE_W, b.h - 5));
	int most = (r.h - 2) / LOOK_LINE;
	if (!O.asking) {
		look_wrapped(note_words(options_line_kind()), r.x + TEXT_IN, r.y + 3, r.w - 2 * TEXT_IN, most, LOOK_INK, LOOK_INK_SHADE);
		return;
	}
	look_wrapped(options_word(asks[O.page]), r.x + TEXT_IN, r.y + 3, r.w - 2 * TEXT_IN, most - 1, LOOK_INK, LOOK_INK_SHADE);
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
	look_fade_in(x0, y0, CORE_W, CORE_H, O.t);
}

/* the question over all, else the controls screen over Options, else it */
void options_draw(void) {
	if (analytics_ask_shown()) analytics_ask_draw();
	else if (controls_shown()) controls_draw();
	else if (O.open) options_panel_draw();
}

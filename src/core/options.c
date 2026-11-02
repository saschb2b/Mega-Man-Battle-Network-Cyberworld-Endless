#include "options.h"

#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "analytics_ask.h"
#include "audio.h"
#include "buttons.h"
#include "controls.h"
#include "display.h"
#include "fast_forward.h"
#include "feel_out.h"
#include "feel.h"
#include "game.h"
#include "options_scene.h"
#include "options_state.h"
#include "options_text.h"
#include "platform.h"
#include "settings_line.h"
#include "sound_heard.h"
#include "touch.h"

OptionsState O;
/* (the page last shown and each page's cursor, kept from one opening to
 * the next, as Apple's HIG asks: docs/engine/settings-screens.md) */
static int kept_page, kept_at[PAGE_COUNT];

int options_lines(int kinds[LINES_MAX]) {
	int n = 0;
	if (O.page == PAGE_AUDIO) {
		kinds[n++] = LINE_MUSIC;
		kinds[n++] = LINE_EFFECTS;
		kinds[n++] = LINE_OUTPUT;
		kinds[n++] = LINE_SOUND;
		if (feel_here()) kinds[n++] = LINE_VIBRATION;
		if (feel_dualsense()) kinds[n++] = LINE_TRIGGERS;
	} else if (O.page == PAGE_DISPLAY) {
		if (display_has_window()) kinds[n++] = LINE_FULLSCREEN;
		if (display_has_scale()) kinds[n++] = LINE_SCALE;
		if (display_has_smooth()) kinds[n++] = LINE_SMOOTH;
		kinds[n++] = LINE_COLOUR;
		kinds[n++] = LINE_BLEND;
		if (display_has_lcd3x()) kinds[n++] = LINE_LCD3X;
	} else {
#ifndef __3DS__
		kinds[n++] = LINE_CONTROLS;
#endif
		kinds[n++] = LINE_FAST;
		if (analytics_ask_here()) kinds[n++] = LINE_STATS;
	}
	/* (no Done: B leaves, as everywhere in BN6) */
	return n;
}

int options_line_kind(void) {
	int kinds[LINES_MAX], n = options_lines(kinds);
	return kinds[O.at[O.page] < n ? O.at[O.page] : n - 1];
}

/* (the Game page's Controls and Statistics are their own screens' to
 * set: START puts back the lines whose value Options turns) */
bool options_page_resets(void) {
	int kinds[LINES_MAX], n = options_lines(kinds);
	for (int i = 0; i < n; ++i)
		if (options_line_turns(kinds[i]) && kinds[i] != LINE_FULLSCREEN) return true;
	return false;
}

/* the list scrolled to keep the cursor's row shown */
static void scroll(void) {
	int kinds[LINES_MAX], n = options_lines(kinds), at = O.at[O.page];
	if (at < O.top) O.top = at;
	if (at >= O.top + OPTIONS_ROWS) O.top = at - OPTIONS_ROWS + 1;
	if (O.top > n - OPTIONS_ROWS) O.top = n - OPTIONS_ROWS;
	if (O.top < 0) O.top = 0;
}

void options_open(void) {
	memset(&O, 0, sizeof O);
	O.open = true;
	O.page = kept_page;
	memcpy(O.at, kept_at, sizeof O.at);
	scroll();
	/* (the finger that opened it presses nothing more) */
	touch_release();
}

void options_first_start(void) { analytics_ask_offer(); }

bool options_shown(void) { return O.open || controls_shown() || analytics_ask_shown(); }

/* The screen shown, in a word, for a headless run's state (screen_name.c): NULL where none */
const char *options_screen(void) {
	static const char *const pages[PAGE_COUNT] = { "options-display", "options-audio", "options-game" };
	if (controls_shown()) return "controls";
	if (O.open) return pages[O.page];
	return analytics_ask_shown() ? "statistics-question" : NULL;
}

bool options_takes_buttons(void) { return analytics_ask_shown() && !O.open && !controls_shown(); }

/* ---- the sound ---- */

void options_sound_keep(SoundFilter f, bool tell_page) {
	char path[600];
	sound_filter_set(f);
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	settings_line_write(path, "sound", sound_filter_word(f), options_sound_note());
	platform_persist();
#ifdef __EMSCRIPTEN__
	if (tell_page) {
		char js[64];
		snprintf(js, sizeof js, "if (Module.cwSound) Module.cwSound('%s');", sound_filter_word(f));
		emscripten_run_script(js);
	}
#else
	(void)tell_page;
#endif
}

/* A press of A or a tap on a line (TURN_PRESS): its value the next, round
 * from the last to the first; Left and Right are -1 and +1 */
#define TURN_PRESS 2

/* a level turned (`dir`): Left and Right stop at Off and the game's own,
 * so neither leaps from silence to the loudest; a press goes round from
 * the loudest to Off */
static int level_next(int level, int dir) {
	if (dir == TURN_PRESS) return level >= SOUND_LEVELS ? 0 : level + 1;
	level += dir < 0 ? -1 : 1;
	return level < 0 ? 0 : level > SOUND_LEVELS ? SOUND_LEVELS : level;
}

/* the music's or the effects' (`kind`'s) level set to `level` and kept */
static void heard_keep(int kind, int level) {
	char path[600], word[12];
	int music = kind == LINE_MUSIC ? level : sound_heard_music(), effects = kind == LINE_EFFECTS ? level : sound_heard_effects();
	sound_heard_set(music, effects);
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	snprintf(word, sizeof word, "%d", kind == LINE_MUSIC ? sound_heard_music() : sound_heard_effects());
	if (kind == LINE_MUSIC) settings_line_write(path, "music", word, options_music_note());
	else settings_line_write(path, "effects", word, options_effects_note());
	platform_persist();
}

/* the output set to `o` and kept */
static void output_keep(SoundOutput o) {
	char path[600];
	sound_output_set(o);
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	settings_line_write(path, "output", sound_output_word(o), options_output_note());
	platform_persist();
}

/* fast forward's speed set to `speed` and kept */
static void fast_keep(int speed) {
	char path[600];
	fast_speed_set(speed);
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	settings_line_write(path, "fast_forward", fast_speed_word(fast_speed()), options_fast_note());
	platform_persist();
}

/* the vibration set to `level`, or the trigger effects to `level` (0 or
 * 1), and kept */
static void feel_keep(int kind, int level) {
	char path[600];
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	if (kind == LINE_VIBRATION) {
		feel_set_level(level);
		settings_line_write(path, "vibration", feel_level_word(feel_level()), options_vibration_note());
		/* (the new strength felt at once, as a hit of a quarter of his HP) */
		feel_beat(FEEL_HIT, 255);
	} else {
		feel_set_triggers(level != 0);
		settings_line_write(path, "trigger_effects", feel_triggers() ? "on" : "off", options_triggers_note());
	}
	platform_persist();
}

#ifdef __EMSCRIPTEN__
/* The page's Sound button (web/play/app.js): set here as Options sets it
 * (`filter` a SoundFilter: 0 gba, 1 soft, 2 mgba). */
void cw_set_sound(int filter);
EMSCRIPTEN_KEEPALIVE void cw_set_sound(int filter) { options_sound_keep((unsigned)filter < SOUND_FILTER_COUNT ? (SoundFilter)filter : SOUND_GBA, false); }
#endif

/* A line's value turned (`dir`: +1 Right, TURN_PRESS A or a tap, -1 Left) and kept */
static void turn(int kind, int dir) {
	switch (kind) {
	case LINE_FULLSCREEN: display_fullscreen_keep(!P.fullscreen); break;
	case LINE_SCALE: display_scale_keep(display_scale_next(dir)); break;
	case LINE_SMOOTH: display_smooth_keep(!P.blend, true); break;
	case LINE_COLOUR: display_colour_keep(display_colour_next(dir)); break;
	case LINE_BLEND: display_blend_keep(display_blend_next(dir)); break;
	case LINE_LCD3X: display_lcd3x_keep(!lcd_grid()); break;
	case LINE_MUSIC:
	case LINE_EFFECTS: heard_keep(kind, level_next(kind == LINE_MUSIC ? sound_heard_music() : sound_heard_effects(), dir)); break;
	case LINE_OUTPUT: output_keep((SoundOutput)((sound_output() + SOUND_OUTPUT_COUNT + (dir < 0 ? -1 : 1)) % SOUND_OUTPUT_COUNT)); break;
	case LINE_FAST: fast_keep(fast_speed_next(fast_speed(), dir)); break;
	case LINE_VIBRATION: feel_keep(kind, feel_level_next(feel_level(), dir)); break;
	case LINE_TRIGGERS: feel_keep(kind, !feel_triggers()); break;
	default: options_sound_keep((SoundFilter)((sound_filter() + SOUND_FILTER_COUNT + (dir < 0 ? -1 : 1)) % SOUND_FILTER_COUNT), true); break;
	}
	audio_sfx(SFX_CURSOR);
}

/* START's Yes: the page's lines as they came, each kept (Fullscreen, the
 * window's, and the Game page's own screens left as they are, as Lyra's
 * reset leaves what it marks: docs/engine/settings-screens.md) */
static void defaults(void) {
	if (O.page == PAGE_DISPLAY) {
		if (display_has_scale()) display_scale_keep(display_scale_default());
		if (display_has_smooth() && P.blend) display_smooth_keep(false, true);
		display_colour_keep(LCD_COLOUR_OFF);
		display_blend_keep(LCD_BLEND_OFF);
		if (display_has_lcd3x() && lcd_grid()) display_lcd3x_keep(false);
	} else if (O.page == PAGE_AUDIO) {
		if (sound_heard_music() != SOUND_LEVELS) heard_keep(LINE_MUSIC, SOUND_LEVELS);
		if (sound_heard_effects() != SOUND_LEVELS) heard_keep(LINE_EFFECTS, SOUND_LEVELS);
		if (sound_output() != OUTPUT_SPEAKERS) output_keep(OUTPUT_SPEAKERS);
		if (sound_filter() != SOUND_GBA) options_sound_keep(SOUND_GBA, true);
		if (feel_here() && feel_level() != FEEL_DEFAULT) feel_keep(LINE_VIBRATION, FEEL_DEFAULT);
		if (feel_dualsense() && !feel_triggers()) feel_keep(LINE_TRIGGERS, 1);
	} else if (fast_speed() != FAST_DEFAULT) fast_keep(FAST_DEFAULT);
}

/* ---- the lines ---- */

/* B: away (no hub above the pages); the cursor's page and lines kept */
static void leave(void) {
	audio_sfx(SFX_CANCEL);
	kept_page = O.page;
	memcpy(kept_at, O.at, sizeof kept_at);
	O.open = false;
}

/* page `p` shown (a tab tapped), its own cursor, fading in */
static void show_page(int p) {
	O.page = p;
	O.top = 0;
	scroll();
	O.t = 0;
	audio_sfx(SFX_CURSOR);
}

/* L or R: the next page (`dir`) */
static void turn_page(int dir) { show_page((O.page + PAGE_COUNT + dir) % PAGE_COUNT); }

static void choose(void) {
	switch (options_line_kind()) {
	case LINE_CONTROLS: controls_open(); audio_sfx(SFX_SELECT); break;
	/* (the statistics' question over the page, its cursor on the answer as
	 * it stands: what is sent is read before a yes) */
	case LINE_STATS: if (analytics_ask_open()) audio_sfx(SFX_SELECT); break;
	default: turn(options_line_kind(), TURN_PRESS); break;
	}
}

/* START's question asked, or answered (`yes`) */
static void ask(void) {
	O.asking = true;
	O.yes = false;
	audio_sfx(SFX_SELECT);
}

static void answer(bool yes) {
	O.asking = false;
	if (yes) defaults();
	audio_sfx(yes ? SFX_SELECT : SFX_CANCEL);
}

static void ask_update(uint32_t pr) {
	if (pr & BTN_B) answer(false);
	else if (pr & (BTN_A | BTN_START)) answer(O.yes);
	else if (pr & (BTN_LEFT | BTN_RIGHT | BTN_UP | BTN_DOWN)) {
		O.yes = !O.yes;
		audio_sfx(SFX_CURSOR);
	}
}

static void nav_update(void) {
	uint32_t pr = P.menu_pressed, rep = P.menu_repeat;
	if (O.asking) { ask_update(pr); return; }
	int kinds[LINES_MAX], n = options_lines(kinds), *at = &O.at[O.page];
	if (pr & BTN_B) { leave(); return; }
	if (pr & (BTN_L | BTN_R)) { turn_page(pr & BTN_L ? -1 : 1); return; }
	if ((pr & BTN_START) && options_page_resets()) { ask(); return; }
	int was = *at;
	if (rep & BTN_UP) *at = (*at + n - 1) % n;
	else if (rep & BTN_DOWN) *at = (*at + 1) % n;
	if (*at != was) {
		scroll();
		audio_sfx(SFX_CURSOR);
	}
	/* (Left and Right turn a value, as the PET's small arrows say) */
	if ((pr & (BTN_LEFT | BTN_RIGHT)) && options_line_turns(kinds[*at])) turn(kinds[*at], pr & BTN_LEFT ? -1 : 1);
	else if (pr & BTN_A) choose();
}

/* ---- the overlay: the question, the controls screen, then Options ---- */

void options_update(void) {
	if (analytics_ask_shown()) { analytics_ask_update(); return; }
	if (controls_shown()) { controls_update(); return; }
	if (O.open) {
		++O.t;
		nav_update();
	}
	options_scene_update();
}

bool options_back(void) {
	if (analytics_ask_shown()) return analytics_ask_back();
	if (controls_shown()) return controls_back();
	if (!O.open) return false;
	if (O.asking) answer(false);
	else leave();
	return true;
}

/* a tap's place (options_tap's) acted on */
static void tapped(int at) {
	if (O.asking) {
		if (at == TAP_YES || at == TAP_NO) answer(at == TAP_YES);
		return;
	}
	if (at == TAP_PREV || at == TAP_NEXT) turn_page(at == TAP_PREV ? -1 : 1);
	else if (at <= TAP_PAGE && at > TAP_PAGE - PAGE_COUNT) {
		if (TAP_PAGE - at != O.page) show_page(TAP_PAGE - at);
	}
	else if (at == TAP_DEFAULT && options_page_resets()) ask();
	else if (at >= 0) {
		O.at[O.page] = at;
		scroll();
		choose();
	}
}

void options_finger(uint32_t type, SDL_FingerID id, int x, int y) {
	bool ask = analytics_ask_shown();
	if (!ask && controls_shown()) { controls_finger(type, id, x, y); return; }
	if (!ask && !O.open) return;
	if (type == SDL_FINGERDOWN) {
		O.finger = id;
		O.finger_down = true;
		return;
	}
	if (type != SDL_FINGERUP || !O.finger_down || id != O.finger) return;
	O.finger_down = false;
	if (ask) { analytics_ask_tap(x - P.core_x, y - P.core_y); return; }
	tapped(options_tap(x - P.core_x, y - P.core_y));
}

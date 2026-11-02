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
#include "game.h"
#include "options_state.h"
#include "options_text.h"
#include "platform.h"
#include "settings_line.h"
#include "sound_heard.h"
#include "touch.h"

OptionsState O;

int options_lines(int kinds[LINES_MAX]) {
	int n = 0;
	if (O.section == SECTION_AUDIO) {
		kinds[n++] = LINE_MUSIC;
		kinds[n++] = LINE_EFFECTS;
		kinds[n++] = LINE_OUTPUT;
		kinds[n++] = LINE_SOUND;
		if (feel_here()) kinds[n++] = LINE_VIBRATION;
		if (feel_dualsense()) kinds[n++] = LINE_TRIGGERS;
	} else if (O.section == SECTION_DISPLAY) {
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
		kinds[n++] = LINE_DISPLAY;
		kinds[n++] = LINE_AUDIO;
		kinds[n++] = LINE_FAST;
		if (analytics_ask_here()) kinds[n++] = LINE_STATS;
	}
	/* (no Done: B leaves, as everywhere in BN6) */
	return n;
}

static int line_kind(void) {
	int kinds[LINES_MAX], n = options_lines(kinds);
	return kinds[O.at < n ? O.at : n - 1];
}

void options_open(void) {
	memset(&O, 0, sizeof O);
	O.open = true;
	/* (the finger that opened it presses nothing more) */
	touch_release();
}

void options_first_start(void) { analytics_ask_offer(); }

bool options_shown(void) { return O.open || controls_shown() || analytics_ask_shown(); }

/* The screen shown, in a word, for a headless run's state (screen_name.c): NULL where none */
const char *options_screen(void) {
	if (controls_shown()) return "controls";
	if (O.open) return O.section == SECTION_DISPLAY ? "options-display" : O.section == SECTION_AUDIO ? "options-audio" : "options";
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

/* the music's or the effects' (`kind`'s) level turned (`dir`) and kept */
static void heard_keep(int kind, int dir) {
	char path[600], word[12];
	int music = sound_heard_music(), effects = sound_heard_effects();
	if (kind == LINE_MUSIC) music = level_next(music, dir);
	else effects = level_next(effects, dir);
	sound_heard_set(music, effects);
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	snprintf(word, sizeof word, "%d", kind == LINE_MUSIC ? music : effects);
	if (kind == LINE_MUSIC) settings_line_write(path, "music", word, options_music_note());
	else settings_line_write(path, "effects", word, options_effects_note());
	platform_persist();
}

/* the output turned (`dir`) and kept */
static void output_keep(int dir) {
	char path[600];
	sound_output_set((SoundOutput)((sound_output() + SOUND_OUTPUT_COUNT + (dir < 0 ? -1 : 1)) % SOUND_OUTPUT_COUNT));
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	settings_line_write(path, "output", sound_output_word(sound_output()), options_output_note());
	platform_persist();
}

/* fast forward's speed turned (`dir`) and kept */
static void fast_keep(int dir) {
	char path[600];
	fast_speed_set(fast_speed_next(fast_speed(), dir));
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	settings_line_write(path, "fast_forward", fast_speed_word(fast_speed()), options_fast_note());
	platform_persist();
}

/* the vibration turned (`dir`) or the trigger effects switched, and kept */
static void feel_keep(int kind, int dir) {
	char path[600];
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	if (kind == LINE_VIBRATION) {
		feel_set_level(feel_level_next(feel_level(), dir));
		settings_line_write(path, "vibration", feel_level_word(feel_level()), options_vibration_note());
		/* (the new strength felt at once, as a hit of a quarter of his HP) */
		feel_beat(FEEL_HIT, 255);
	} else {
		feel_set_triggers(!feel_triggers());
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
	case LINE_EFFECTS: heard_keep(kind, dir); break;
	case LINE_OUTPUT: output_keep(dir); break;
	case LINE_FAST: fast_keep(dir); break;
	case LINE_VIBRATION:
	case LINE_TRIGGERS: feel_keep(kind, dir); break;
	default: options_sound_keep((SoundFilter)((sound_filter() + SOUND_FILTER_COUNT + (dir < 0 ? -1 : 1)) % SOUND_FILTER_COUNT), true); break;
	}
	audio_sfx(SFX_CURSOR);
}

/* ---- the lines ---- */

/* B: from a section to the hub, its line under the cursor, the hub
 * fading in again; from the hub away */
static void back(void) {
	audio_sfx(SFX_CANCEL);
	if (O.section != SECTION_HUB) {
		O.section = SECTION_HUB;
		O.at = O.hub_at;
		O.t = 0;
	} else O.open = false;
}

static void choose(void) {
	switch (line_kind()) {
	case LINE_CONTROLS: controls_open(); audio_sfx(SFX_SELECT); break;
	case LINE_DISPLAY:
	case LINE_AUDIO:
		O.hub_at = O.at;
		O.section = line_kind() == LINE_DISPLAY ? SECTION_DISPLAY : SECTION_AUDIO;
		O.at = 0;
		O.t = 0;
		audio_sfx(SFX_SELECT);
		break;
	/* (the statistics' question over the hub, its cursor on the answer as
	 * it stands: what is sent is read before a yes) */
	case LINE_STATS: if (analytics_ask_open()) audio_sfx(SFX_SELECT); break;
	default: turn(line_kind(), TURN_PRESS); break;
	}
}

static void nav_update(void) {
	uint32_t pr = P.menu_pressed, rep = P.menu_repeat;
	int kinds[LINES_MAX], n = options_lines(kinds);
	if (pr & BTN_B) { back(); return; }
	int was = O.at;
	if (rep & BTN_UP) O.at = (O.at + n - 1) % n;
	else if (rep & BTN_DOWN) O.at = (O.at + 1) % n;
	if (O.at != was) audio_sfx(SFX_CURSOR);
	/* (Left and Right turn a value, as the PET's small arrows say) */
	if ((pr & (BTN_LEFT | BTN_RIGHT)) && options_line_turns(kinds[O.at])) turn(kinds[O.at], pr & BTN_LEFT ? -1 : 1);
	else if (pr & (BTN_A | BTN_START)) choose();
}

/* ---- the overlay: the question, the controls screen, then Options ---- */

void options_update(void) {
	if (analytics_ask_shown()) { analytics_ask_update(); return; }
	if (controls_shown()) { controls_update(); return; }
	if (O.open) {
		++O.t;
		nav_update();
	}
}

bool options_back(void) {
	if (analytics_ask_shown()) return analytics_ask_back();
	if (controls_shown()) return controls_back();
	if (!O.open) return false;
	back();
	return true;
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
	int at = options_tap_line(x - P.core_x, y - P.core_y);
	if (at < 0) return;
	O.at = at;
	choose();
}

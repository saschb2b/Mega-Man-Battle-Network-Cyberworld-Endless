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
		kinds[n++] = LINE_SOUND;
	} else if (O.section == SECTION_DISPLAY) {
		if (display_has_window()) kinds[n++] = LINE_FULLSCREEN;
		if (display_has_scale()) kinds[n++] = LINE_SCALE;
		if (display_has_smooth()) kinds[n++] = LINE_SMOOTH;
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
	settings_line_write(path, "sound", f == SOUND_SOFT ? "soft" : "gba", options_sound_note());
	platform_persist();
#ifdef __EMSCRIPTEN__
	if (tell_page) emscripten_run_script(f == SOUND_SOFT ? "if (Module.cwSound) Module.cwSound(true);" : "if (Module.cwSound) Module.cwSound(false);");
#else
	(void)tell_page;
#endif
}

/* the music or the effects (`kind`'s) turned on or off and kept */
static void heard_keep(int kind) {
	char path[600];
	bool music = sound_heard_music() ^ (kind == LINE_MUSIC), effects = sound_heard_effects() ^ (kind == LINE_EFFECTS);
	sound_heard_set(music, effects);
	snprintf(path, sizeof path, "%s/settings.ini", g_data_dir);
	if (kind == LINE_MUSIC) settings_line_write(path, "music", music ? "on" : "off", options_music_note());
	else settings_line_write(path, "effects", effects ? "on" : "off", options_effects_note());
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

#ifdef __EMSCRIPTEN__
/* The page's Sound button (web/play/app.js): set here as Options sets it. */
void cw_set_sound(int soft);
EMSCRIPTEN_KEEPALIVE void cw_set_sound(int soft) { options_sound_keep(soft ? SOUND_SOFT : SOUND_GBA, false); }
#endif

/* A line's value turned (`dir`: +1 Right, A or a tap; -1 Left) and kept */
static void turn(int kind, int dir) {
	switch (kind) {
	case LINE_FULLSCREEN: display_fullscreen_keep(!P.fullscreen); break;
	case LINE_SCALE: display_scale_keep(display_scale_next(dir)); break;
	case LINE_SMOOTH: display_smooth_keep(!P.blend, true); break;
	case LINE_MUSIC:
	case LINE_EFFECTS: heard_keep(kind); break;
	case LINE_FAST: fast_keep(dir); break;
	default: options_sound_keep(sound_filter() == SOUND_SOFT ? SOUND_GBA : SOUND_SOFT, true); break;
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
	default: turn(line_kind(), 1); break;
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

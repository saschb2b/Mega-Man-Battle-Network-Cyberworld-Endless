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
#include "game.h"
#include "options_state.h"
#include "options_text.h"
#include "platform.h"
#include "settings_line.h"
#include "touch.h"

OptionsState O;

int options_lines(int kinds[LINES_MAX]) {
	int n = 0;
	if (O.audio) kinds[n++] = LINE_SOUND;
	else {
#ifndef __3DS__
		kinds[n++] = LINE_CONTROLS;
#endif
		kinds[n++] = LINE_AUDIO;
		if (analytics_ask_here()) kinds[n++] = LINE_STATS;
	}
	kinds[n++] = LINE_DONE;
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

#ifdef __EMSCRIPTEN__
/* The page's Sound button (web/play/app.js): set here as Options sets it. */
void cw_set_sound(int soft);
EMSCRIPTEN_KEEPALIVE void cw_set_sound(int soft) { options_sound_keep(soft ? SOUND_SOFT : SOUND_GBA, false); }
#endif

static void sound_turn(void) {
	options_sound_keep(sound_filter() == SOUND_SOFT ? SOUND_GBA : SOUND_SOFT, true);
	audio_sfx(SFX_CURSOR);
}

/* ---- the lines ---- */

/* B, or Done: from a section to the hub, its line under the cursor; from
 * the hub away */
static void back(void) {
	audio_sfx(SFX_CANCEL);
	if (O.audio) {
		O.audio = false;
		O.at = O.hub_at;
	} else O.open = false;
}

static void choose(void) {
	switch (line_kind()) {
	case LINE_CONTROLS: controls_open(); audio_sfx(SFX_SELECT); break;
	case LINE_AUDIO:
		O.hub_at = O.at;
		O.audio = true;
		O.at = 0;
		audio_sfx(SFX_SELECT);
		break;
	/* (the statistics' question over the hub, its cursor on the answer as
	 * it stands: what is sent is read before a yes) */
	case LINE_STATS: if (analytics_ask_open()) audio_sfx(SFX_SELECT); break;
	case LINE_SOUND: sound_turn(); break;
	default: back(); break;
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
	/* (Left and Right turn the sound, as the PET's small arrows say) */
	if ((pr & (BTN_LEFT | BTN_RIGHT)) && kinds[O.at] == LINE_SOUND) sound_turn();
	else if (pr & (BTN_A | BTN_START)) choose();
}

/* ---- the overlay: the question, the controls screen, then Options ---- */

void options_update(void) {
	if (analytics_ask_shown()) { analytics_ask_update(); return; }
	if (controls_shown()) { controls_update(); return; }
	if (O.open) nav_update();
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

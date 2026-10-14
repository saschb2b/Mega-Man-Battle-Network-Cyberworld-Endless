#include "title_access.h"

#include <stdio.h>
#include <string.h>

#include "analytics_ask.h"
#include "audio.h"
#include "controls.h"
#include "game.h"
#include "gfx.h"
#include "minifont.h"
#include "network.h"
#include "platform.h"
#include "saves.h"

/* SELECT, on the title and its menu: the controls screen (issue #37); on
 * the 3DS, whose buttons are the console's own, the anonymous statistics'
 * question, its one setting (issue #104), where the build can send; with
 * L held, the developer's sprite gallery, which SELECT alone opened
 * before. R: the ROMs' screen, the launcher, to add BN5 after the first
 * start (issue #97), where the build has one. L alone: SAVES (saves.h),
 * kept off the menu's ROM-drawn rows. True when it took the press. */
bool title_access_keys(void) {
	if (btn_pressed(BTN_R) && launcher_open()) {
		audio_sfx(SFX_SELECT);
		return true;
	}
	if (saves_key()) return true;
	if (!btn_pressed(BTN_SELECT)) return false;
#ifndef __3DS__
	if (!btn_held(BTN_L)) {
		controls_open();
		audio_sfx(SFX_SELECT);
		return true;
	}
#else
	if (!btn_held(BTN_L) && analytics_ask_open()) {
		audio_sfx(SFX_SELECT);
		return true;
	}
#endif
	scene_set(&scene_gallery);
	return true;
}

/* The word under the picture: (a keyboard's Start, where no controller is:
 * a PC player had no word of which key it is) ENTER before the menu, and
 * with the menu where the controls are set, on the player's own SELECT
 * (controls_word: "R", "Minus", "SELECT"), SAVES on L, and the ROMs, on
 * R, where the launcher is (issue #97: no way to add BN5 had been found);
 * on the 3DS its SELECT's statistics, where the build can send them */
void title_access_hint(int x0, int y0, bool menu, bool pressed, bool confirm) {
	SDL_Color grey = rgba(150, 160, 190, 255);
	if (!menu && !pressed && controls_keyboard_only()) minifont_draw_centered(x0 + CORE_W / 2, y0 + 138, "ENTER", grey, 1);
#ifdef __3DS__
	if (!menu || confirm) return;
	char hint[64];
	snprintf(hint, sizeof hint, "%sL: %s", analytics_ask_here() ? "SELECT: Statistics   " : "", saves_word_hint());
	minifont_draw_centered(x0 + CORE_W / 2, y0 + 107, hint, grey, 1);
#else
	if (!menu || confirm) return;
	/* (controls_word's own buffer, one word at a time) */
	char hint[80], l[32];
	snprintf(l, sizeof l, "%s", controls_word(BTN_L));
	snprintf(hint, sizeof hint, "%s: Controls  %s: %s", controls_word(BTN_SELECT), l, saves_word_hint());
	if (launcher_here()) {
		size_t m = strlen(hint);
		snprintf(hint + m, sizeof hint - m, "  %s: ROMs", controls_word(BTN_R));
	}
	/* (in the rows between the logo and the menu: under NEW GAME it ran
	 * into the build's line and the best layer's once SAVES joined it,
	 * and CONTINUE's row hides it there) */
	minifont_draw_centered(x0 + CORE_W / 2, y0 + 107, hint, grey, 1);
#endif
}

void title_access_network(void) { network_open(); }

void title_access_offer(void) { analytics_ask_offer(); }

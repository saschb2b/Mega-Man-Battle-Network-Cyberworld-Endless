#include "director_pc.h"

#include <stdio.h>

#include "audio.h"
#include "bn6_fields.h"
#include "director_hold.h"
#include "director_save.h"
#include "director_state.h"
#include "gfx.h"
#include "lan_house.h"
#include "network.h"
#include "pc_text.h"
#include "pixfont.h"
#include "platform.h"
#include "save_queue.h"
#include "second_frame.h"

static bool opened, requested, r_held, failed;
static int selected, jack_in;

static bool at_pc(void) {
	return D.active && D.town && D.home && !D.warping &&
		emu_read8(BN6_MAP_GROUP) == LAN_HOUSE_GROUP && emu_read8(BN6_MAP_NUMBER) == LAN_ROOM &&
		lan_room_on_pc(bn6_player_x(), bn6_player_y());
}

static bool close_menu(void) {
	opened = false;
	platform_own_taps(false);
	platform_on_back(NULL);
	return true;
}

void director_pc_reset(void) {
	if (opened) close_menu();
	requested = r_held = failed = false;
	jack_in = 0;
}

static SDL_Rect option_rect(int row) {
	return (SDL_Rect){ P.core_x + 42, P.core_y + 48 + row * 21, 156, 19 };
}

uint32_t director_pc_keys(uint32_t keys) {
	/* BN6 samples its pad after the core's next VBlank. Keep this menu's
	 * deliberate jack-in for a short press, as the physical button is. */
	if (jack_in > 0) { --jack_in; return KEY_R; }
	bool pressed = (keys & KEY_R) && !r_held;
	r_held = (keys & KEY_R) != 0;
	if (!opened) {
		if (!pressed || !at_pc() || !hold_save_ok()) return keys;
		opened = true;
		selected = 0;
		failed = false;
		platform_own_taps(true);
		platform_on_back(close_menu);
		return 0;
	}
	if (btn_pressed(BTN_B)) { close_menu(); return 0; }
	if (btn_repeat(BTN_UP)) selected = (selected + 2) % 3;
	if (btn_repeat(BTN_DOWN)) selected = (selected + 1) % 3;
	bool choose = btn_pressed(BTN_A) || btn_pressed(BTN_START);
	int x, y;
	if (platform_tap(&x, &y))
		for (int i = 0; i < 3; ++i) {
			SDL_Rect r = option_rect(i);
			if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) { selected = i; choose = true; }
		}
	if (!choose) return 0;
	audio_sfx(selected == 2 ? SFX_CANCEL : SFX_SELECT);
	if (selected == 1) { requested = true; return 0; }
	close_menu();
	if (!selected) jack_in = 5;
	return selected == 0 ? KEY_R : 0;
}

bool director_pc_update(void) {
	if (!requested) return false;
	requested = false;
	if (!at_pc() || !hold_save_ok()) { failed = true; return false; }
	/* Observe every write in this checkpoint, including the worker's
	 * result. A previous failure does not prevent a successful retry. */
	save_queue_wait();
	unsigned before = save_write_failures();
	D.checkpoint_here = true;
	save_checkpoint();
	D.checkpoint_here = false;
	save_queue_wait();
	if (save_write_failures() != before) { failed = true; return false; }
	D.home_save_due = false;
	D.saved_at = pc_saved;
	fprintf(stderr, "online: PC checkpoint saved; opening Network\n");
	close_menu();
	network_open_checkpoint();
	return true;
}

void director_pc_draw(void) {
	if (!opened) return;
	int x = P.core_x + 30, y = P.core_y + 29;
	fill_rect(x, y, 180, 109, PET_LINE);
	fill_rect(x + 1, y + 1, 178, 107, PET_EXIT);
	pixfont_draw(x + 90, y + 5, pc_title, PET_GOLD, PET_EXIT, PIXFONT_CENTER, 1);
	for (int i = 0; i < 3; ++i) {
		SDL_Rect r = option_rect(i);
		fill_rect(r.x, r.y, r.w, r.h, selected == i ? PET_GOLD : PET_LINE);
		fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, selected == i ? PET_NAVY : PET_SLOT);
		pixfont_draw(r.x + r.w / 2, r.y + 5, pc_options[i], PET_WHITE, PET_EXIT, PIXFONT_CENTER, 1);
	}
	char lines[2][96];
	int n = pixfont_wrap(failed ? pc_failed : pc_hint, 172, 1, lines, 2);
	for (int i = 0; i < n; ++i)
		pixfont_draw(x + 90, y + 84 + i * PIXFONT_LINE, lines[i], failed ? PET_GOLD : PET_WHITE, PET_EXIT, PIXFONT_CENTER, 1);
}

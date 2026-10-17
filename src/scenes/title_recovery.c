#include "title_recovery.h"

#include "audio.h"
#include "game.h"
#include "online_trade_gate.h"
#include "pixfont.h"
#include "platform.h"
#include "save.h"
#include "save_queue.h"
#include "second_frame.h"
#include "title_recovery_text.h"

static bool failed, confirm;
static int choice;

void title_recovery_enter(void) {
	failed = confirm = false;
	choice = 0;
	/* The initial town can end before its first checkpoint. A title
	 * re-entry ends that live session; its intent needs an explicit choice. */
	if (online_trade_gate_state() == TRADE_GATE_NEW_RUN) {
		save_queue_wait();
		(void)online_trade_gate_recover();
		save_init();
	}
}

void title_recovery_failed(void) { failed = true; confirm = false; choice = 0; }
bool title_recovery_active(void) { return failed || !save_playable(); }

static int page(void) {
	if (confirm) return TITLE_RECOVERY_DISCARD;
	if (failed && save_playable()) return TITLE_RECOVERY_FAILED;
	if (online_trade_gate_state() == TRADE_GATE_UNBOUND) return TITLE_RECOVERY_START;
	if (online_trade_gate_state() == TRADE_GATE_PENDING) return TITLE_RECOVERY_TRADE;
	return TITLE_RECOVERY_BLOCKED;
}

static void back_to_title(void) {
	title_menu = true;
	scene_set(&scene_title);
}

static void recover(bool discard) {
	save_queue_wait();
	bool ok = discard ? online_trade_gate_discard() : online_trade_gate_recover();
	confirm = false;
	choice = 0;
	if (!ok) return;
	/* Import recovery and receipt admission follow transaction recovery.
	 * A restored older receipt can still prevent continuation here. */
	save_init();
	if (save_playable()) back_to_title();
}

bool title_recovery_update(void) {
	if (!title_recovery_active()) return false;
	if (btn_pressed(BTN_UP) || btn_pressed(BTN_DOWN)) {
		choice ^= 1;
		audio_sfx(SFX_CURSOR);
	}
	if (btn_pressed(BTN_B) && confirm) { confirm = false; choice = 0; return true; }
	if (!btn_pressed(BTN_A) && !btn_pressed(BTN_START)) return true;
	audio_sfx(SFX_SELECT);
	if (confirm) {
		if (choice) recover(true);
		else confirm = false;
		choice = 0;
	} else if (choice) P.quit = true;
	else if (page() == TITLE_RECOVERY_START) { confirm = true; choice = 0; }
	else if (page() == TITLE_RECOVERY_FAILED) back_to_title();
	else recover(false);
	return true;
}

bool title_recovery_draw(void) {
	if (!title_recovery_active()) return false;
	int x = P.core_x, y = P.core_y, current = page();
	fill_rect(x, y, CORE_W, CORE_H, PET_EXIT);
	fill_rect(x, y, CORE_W, 18, PET_GREEN);
	pixfont_draw(x + 8, y + 6, title_recovery_heading, PET_WHITE, PET_GREEN, PIXFONT_LEFT, 1);
	for (int i = 0; i < 3; ++i)
		pixfont_draw(x + 8, y + 33 + i * 14, title_recovery_lines[current][i], PET_WHITE, PET_EXIT, PIXFONT_LEFT, 1);
	for (int i = 0; i < 2; ++i) {
		SDL_Color background = choice == i ? PET_NAVY : PET_SLOT_EDGE;
		fill_rect(x + 6, y + 89 + i * 23, CORE_W - 12, 21, background);
		pixfont_draw(x + 12, y + 96 + i * 23, title_recovery_choices[current][i], choice == i ? PET_GOLD : PET_WHITE, background, PIXFONT_LEFT, 1);
	}
	pixfont_draw(x + 8, y + 149, title_recovery_help, PET_WHITE, PET_EXIT, PIXFONT_LEFT, 1);
	return true;
}

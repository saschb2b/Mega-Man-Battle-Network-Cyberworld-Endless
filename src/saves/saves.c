#include "saves.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "audio.h"
#include "director.h"
#include "mirror.h"
#include "pick.h"
#include "rivals.h"
#include "rom.h"
#include "save.h"
#include "saves_state.h"
#include "saves_text.h"
#include "touch.h"

Saves SV;

void saves_init(void) { (void)pick_kinds(); }

bool saves_comparing(void) {
	return SV.view == SAVES_COMPARE || (SV.view == SAVES_DETAIL && SV.details_back == SAVES_COMPARE);
}

void saves_refresh(void) {
	memset(&SV.here, 0, sizeof SV.here);
	SV.have = backup_local_info(g_data_dir, &SV.here);
	SV.undo = backup_can_undo(g_data_dir);
	SV.resume = R.data && SV.here.has_run && backup_check(&SV.here, XR[XROM_BN5_COLONEL_US].data != NULL) == BACKUP_OK;
	if (!pick_saves_place(SV.place, sizeof SV.place)) SV.place[0] = 0;
}

void saves_reload(void) {
	if (R.data) { save_init(); rivals_load(); }
	platform_persist();
	saves_refresh();
}

void saves_clear_file(void) {
	free(SV.bytes);
	SV.bytes = NULL;
	SV.size = 0;
	SV.undo_preview = false;
	mirror_hold(false);
}

void saves_feedback(const char *text) {
	snprintf(SV.feedback, sizeof SV.feedback, "%s", text);
	SV.feedback_until = SDL_GetTicks() + 5000;
}

void saves_result(const char *text, bool undo) {
	snprintf(SV.note, sizeof SV.note, "%s", text);
	SV.view = SAVES_RESULT;
	SV.result_undo = undo;
	SV.result_resume = false;
	SV.focus = SAVES_DONE;
}

void saves_open(void) {
	if (scene_current() == &scene_saves) return;
	SV.back = scene_current() == &scene_launcher ? &scene_launcher : &scene_title;
	if (scene_current() == &scene_emu) director_suspend();
	SV.focus = SAVES_EXPORT;
	SV.view = SAVES_HOME;
	SV.busy = false;
	SV.note[0] = SV.feedback[0] = 0;
	scene_set(&scene_saves);
}

void saves_import(const char *path) {
	SV.busy = false;
	if (!path || !*path) { if (!saves_comparing()) mirror_hold(false); return; }
	saves_open();
	free(SV.bytes);
	SV.size = 0;
	SV.bytes = backup_read_file(path, &SV.size);
	snprintf(SV.path, sizeof SV.path, "%s", path);
	memset(&SV.file, 0, sizeof SV.file);
	SV.status = SV.bytes && backup_info(SV.bytes, SV.size, &SV.file)
		? backup_check(&SV.file, XR[XROM_BN5_COLONEL_US].data != NULL) : BACKUP_DAMAGED;
	saves_refresh();
	SV.view = SAVES_COMPARE;
	SV.undo_preview = false;
	SV.guard_input = true;
	SV.focus = SAVES_KEEP;
	mirror_hold(true);
	if (SV.status != BACKUP_OK) saves_refusal(SV.status, &SV.file, SV.note, sizeof SV.note);
	else if (SV.have && SV.here.hash == SV.file.hash) {
		saves_clear_file();
		saves_result("This file holds the same saves. Your progress is already here.", false);
	} else saves_loss(&SV.here, &SV.file, SV.note, sizeof SV.note);
}

bool saves_back(void) {
	if (SV.busy) return true;
	if (SV.view == SAVES_DETAIL) {
		SV.view = SV.details_back;
		SV.focus = SV.details_focus;
		return true;
	}
	if (SV.view == SAVES_TRANSFER) {
		SV.view = SAVES_HOME;
		SV.focus = SAVES_OPTIONS;
		return true;
	}
	if (SV.view == SAVES_COMPARE) { saves_answer(false); return true; }
	scene_set(SV.back ? SV.back : &scene_title);
	return true;
}

static void enter(void) {
	platform_own_taps(true);
	platform_on_back(saves_back);
	platform_second_screen(NULL, NULL);
	touch_release();
	saves_refresh();
}

static void leave(void) {
	platform_own_taps(false);
	platform_on_back(NULL);
}

static void update(void) {
	bool routed = saves_picked();
	if (routed || SV.guard_input) {
		int x, y;
		SV.guard_input = false;
		(void)platform_tap(&x, &y);
		return;
	}
	if (SV.busy || saves_pointer()) return;
	if (btn_pressed(BTN_B)) { saves_back(); return; }
	if (btn_repeat(BTN_UP)) saves_move_focus(0, -1);
	if (btn_repeat(BTN_DOWN)) saves_move_focus(0, 1);
	if (btn_repeat(BTN_LEFT)) saves_move_focus(-1, 0);
	if (btn_repeat(BTN_RIGHT)) saves_move_focus(1, 0);
	if (btn_pressed(BTN_A) || btn_pressed(BTN_START)) saves_activate(SV.focus);
}

const Scene scene_saves = { "saves", enter, update, saves_draw, leave };

void saves_tick(void) {
	const Scene *scene = scene_current();
	if (scene != &scene_title && scene != &scene_intro && scene != &scene_saves) return;
	if (SV.busy || pick_busy() || pick_saves_busy()) return;
	char path[1100];
	if (platform_dropped(path, sizeof path)) {
		size_t n = strlen(path);
		if (n > 7 && !SDL_strcasecmp(path + n - 7, ".cwsave")) saves_import(path);
	}
	if (saves_comparing() || SV.busy) return;
	if (mirror_scan(path, sizeof path)) saves_import(path);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void cw_saves_open(void) { saves_open(); }
EMSCRIPTEN_KEEPALIVE
void cw_saves_import(const char *path) { saves_import(path); }
#endif

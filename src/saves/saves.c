/* saves.h. The screen's course: opened over the title or the ROMs screen,
 * home first; a file to import compared with this device's saves, focus
 * on the side with more progress; B goes back a view, and leaves the
 * comparison keeping this device's saves. Its buttons' work is
 * saves_actions.c's, its picture saves_draw.c's. */
#include "saves.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "audio.h"
#include "mirror.h"
#include "pick.h"
#include "platform.h"
#include "rom.h"
#include "saves_actions.h"
#include "saves_draw.h"
#include "saves_state.h"
#include "saves_text.h"
#include "touch.h"

Saves SV;

void saves_refresh(void) {
	SV.have = mirror_local(&SV.here);
	SV.unreadable = !SV.have && SV.here.status != BACKUP_OK;
	SV.undo = backup_can_undo(g_data_dir);
}

void saves_clear_file(void) {
	free(SV.bytes);
	SV.bytes = NULL;
	SV.size = 0;
	SV.from_place = false;
}

void saves_result(const char *text) {
	if (text != SV.note) snprintf(SV.note, sizeof SV.note, "%s", text);
	SV.view = SAVES_RESULT;
	SV.offer_undo = SV.undone = SV.resume = false;
	SV.focus = SAVES_DONE;
}

static void home(void) {
	SV.view = SAVES_HOME;
	SV.focus = SV.have ? SAVES_EXPORT : SAVES_IMPORT;
}

bool saves_open(void) {
	const Scene *now = scene_current();
	if (now == &scene_saves) return true;
	if (now == &scene_emu) return false;
	SV.back = now == &scene_launcher ? &scene_launcher : &scene_title;
	SV.busy = false;
	SV.said[0] = 0;
	saves_clear_file();
	/* (the player asked: the copy's place looked at, a desktop's Downloads
	 * too) */
	mirror_look();
	saves_refresh();
	home();
	scene_set(&scene_saves);
	return true;
}

/* (a record's deepest layer: its best, or the run on now where deeper) */
static int deepest(const BackupInfo *info) { return info->best > info->run_depth ? info->best : info->run_depth; }

/* (the side with more progress: more runs, then the deeper layer, this
 * device's at a tie; a reinstall's empty saves never) */
static bool file_ahead(void) {
	if (!SV.have) return true;
	if (SV.file.runs != SV.here.runs) return SV.file.runs > SV.here.runs;
	return deepest(&SV.file) > deepest(&SV.here);
}

void saves_compare(uint8_t *bytes, size_t n, bool from_place) {
	if (!saves_open()) {
		free(bytes);
		return;
	}
	saves_clear_file();
	SV.bytes = bytes;
	SV.size = n;
	SV.from_place = from_place;
	saves_refresh();
	memset(&SV.file, 0, sizeof SV.file);
	SV.status = bytes && backup_info(bytes, n, &SV.file) ? backup_check(&SV.file, XR[XROM_BN5_COLONEL_US].data != NULL) : BACKUP_DAMAGED;
	SV.guard_input = true;
	if (SV.status == BACKUP_OK && SV.have && SV.here.hash == SV.file.hash) {
		if (from_place) mirror_answer(true);
		saves_clear_file();
		saves_result(saves_same());
		return;
	}
	SV.view = SAVES_COMPARE;
	if (SV.status != BACKUP_OK) {
		saves_refusal(SV.status, &SV.file, SV.note, sizeof SV.note);
		SV.focus = SAVES_DONE;
	} else {
		MirrorStatus m;
		mirror_status(&m);
		bool place = from_place && m.state != MIRROR_OFF;
		saves_compare_note(&SV.here, SV.have, SV.unreadable, place ? m.place : NULL, SV.note, sizeof SV.note);
		SV.focus = file_ahead() ? SAVES_TAKE : SV.have ? SAVES_KEEP : SAVES_DONE;
	}
	if (R.data) audio_sfx(SFX_REVEAL);
}

bool saves_compare_held(void) {
	const uint8_t *held;
	size_t n;
	if (!mirror_held(&held, &n)) return false;
	uint8_t *bytes = malloc(n ? n : 1);
	if (!bytes) return false;
	memcpy(bytes, held, n);
	saves_compare(bytes, n, true);
	return true;
}

void saves_import(const char *path) {
	if (!path || !*path) return;
	size_t n = 0;
	uint8_t *bytes = backup_read_file(path, &n);
	saves_compare(bytes, bytes ? n : 0, false);
}

/* KEEP THESE (the place's file: the copy may write over it now) or DONE
 * and B (the place's file: not opened by itself again, no copy over it) */
void saves_leave_compare(bool keep_these) {
	if (SV.from_place) mirror_answer(keep_these);
	saves_clear_file();
	home();
}

static bool back(void) {
	if (SV.busy) {
		/* (the browser's wait for its page's file let go) */
		pick_saves_cancel();
		return true;
	}
	if (SV.view == SAVES_COMPARE) saves_leave_compare(false);
	else if (SV.view == SAVES_RESULT) home();
	else {
		if (SV.back == &scene_title) title_menu = true;
		scene_set(SV.back ? SV.back : &scene_title);
	}
	if (R.data) audio_sfx(SFX_CANCEL);
	return true;
}

static void enter(void) {
	/* (--scene saves, a capture's start: as from the title) */
	if (!SV.back) {
		SV.back = &scene_title;
		mirror_look();
		saves_refresh();
		home();
	}
	platform_own_taps(true);
	platform_on_back(back);
	platform_second_screen(NULL, NULL);
	touch_release();
}

static void leave(void) {
	platform_own_taps(false);
	platform_on_back(NULL);
	saves_clear_file();
}

static void update(void) {
	saves_tick();
	if (saves_picked() || SV.guard_input) {
		int x, y;
		SV.guard_input = false;
		(void)platform_tap(&x, &y);
		return;
	}
	if (btn_pressed(BTN_B)) { back(); return; }
	if (SV.busy || saves_pointer(back)) return;
	if (btn_repeat(BTN_UP)) saves_move_focus(0, -1);
	if (btn_repeat(BTN_DOWN)) saves_move_focus(0, 1);
	if (btn_repeat(BTN_LEFT)) saves_move_focus(-1, 0);
	if (btn_repeat(BTN_RIGHT)) saves_move_focus(1, 0);
	if (btn_pressed(BTN_A) || btn_pressed(BTN_START)) saves_activate(SV.focus);
}

const Scene scene_saves = { "saves", enter, update, saves_draw, leave };

bool saves_key(void) {
	static bool alone;
	if (btn_pressed(BTN_L)) alone = true;
	if (btn_held(BTN_L) && btn_pressed(BTN_SELECT)) alone = false;
	if (!btn_released(BTN_L) || !alone) return false;
	alone = false;
	if (saves_open() && R.data) audio_sfx(SFX_SELECT);
	return true;
}

void saves_tick(void) {
	const Scene *scene = scene_current();
	if (scene != &scene_title && scene != &scene_launcher && scene != &scene_saves) return;
	if (SV.busy || pick_busy() || pick_saves_busy() || SV.view == SAVES_COMPARE) return;
	char path[1100];
	/* (the ROMs screen takes its own drops: a ROM or a .cwsave) */
	if (scene != &scene_launcher && platform_dropped(path, sizeof path)) {
		size_t n = strlen(path);
		if (n > 7 && !SDL_strcasecmp(path + n - 7, ".cwsave")) {
			saves_import(path);
			return;
		}
	}
	/* (the copy's place holding saves not answered: compared at once, once
	 * a look) */
	if (scene != &scene_saves && mirror_incoming()) saves_compare_held();
}

#ifdef __EMSCRIPTEN__
/* The browser page's menu (web/play/app.js): SAVES over the title, 0 in a
 * run (the page says to go to the title first) */
EMSCRIPTEN_KEEPALIVE
int cw_saves_open(void) { return saves_open() ? 1 : 0; }
/* ... and the file its chooser gave, "" for none */
EMSCRIPTEN_KEEPALIVE
void cw_saves_import(const char *path) {
	if (!saves_open()) return;
	pick_saves_page(path);
}
#endif

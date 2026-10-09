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

void saves_refresh(void) {
	memset(&SV.here, 0, sizeof SV.here);
	SV.have = backup_local_info(g_data_dir, &SV.here);
	SV.undo = backup_can_undo(g_data_dir);
	if (!pick_saves_place(SV.place, sizeof SV.place)) SV.place[0] = 0;
}

static void reload(void) {
	if (R.data) { save_init(); rivals_load(); }
	platform_persist();
	saves_refresh();
}

static void clear_file(void) {
	free(SV.bytes);
	SV.bytes = NULL;
	SV.size = 0;
	SV.comparing = false;
	mirror_hold(false);
}

void saves_open(void) {
	if (scene_current() == &scene_saves) return;
	SV.back = scene_current() == &scene_launcher ? &scene_launcher : &scene_title;
	if (scene_current() == &scene_emu) director_suspend();
	SV.focus = SAVES_EXPORT;
	SV.busy = false;
	SV.note[0] = 0;
	scene_set(&scene_saves);
}

void saves_import(const char *path) {
	SV.busy = false;
	if (!path || !*path) { if (!SV.comparing) mirror_hold(false); return; }
	saves_open();
	free(SV.bytes);
	SV.size = 0;
	SV.bytes = backup_read_file(path, &SV.size);
	memset(&SV.file, 0, sizeof SV.file);
	SV.status = SV.bytes && backup_info(SV.bytes, SV.size, &SV.file)
		? backup_check(&SV.file, XR[XROM_BN5_COLONEL_US].data != NULL) : BACKUP_DAMAGED;
	saves_refresh();
	SV.comparing = true;
	SV.focus = SAVES_KEEP;
	mirror_hold(true);
	if (SV.status != BACKUP_OK) saves_refusal(SV.status, &SV.file, SV.note, sizeof SV.note);
	else if (SV.have && SV.here.hash == SV.file.hash) {
		snprintf(SV.note, sizeof SV.note, "This file holds the same saves. Nothing needs to change.");
		clear_file();
	}
	else saves_loss(&SV.here, &SV.file, SV.note, sizeof SV.note);
}

void saves_answer(bool take) {
	if (take && SV.status != BACKUP_OK) return;
	if (take) {
		BackupStatus status = backup_restore(g_data_dir, SV.bytes, SV.size, XR[XROM_BN5_COLONEL_US].data != NULL);
		if (status != BACKUP_OK) {
			SV.status = status;
			saves_refusal(status, &SV.file, SV.note, sizeof SV.note);
			return;
		}
		reload();
		snprintf(SV.note, sizeof SV.note, "Imported. Undo last import brings back this device's previous saves.");
	} else snprintf(SV.note, sizeof SV.note, "Kept this device's saves.");
	clear_file();
	SV.focus = SAVES_IMPORT;
	if (R.data) audio_sfx(take ? SFX_GOT : SFX_CANCEL);
}

bool saves_back(void) {
	if (SV.busy) return true;
	if (SV.comparing) { saves_answer(false); return true; }
	scene_set(SV.back ? SV.back : &scene_title);
	return true;
}

static void saves_exported(const char *where) {
	snprintf(SV.note, sizeof SV.note, "Exported %s to %s.", BACKUP_NAME, where && *where ? where : SV.place);
}

static void export_file(void) {
	size_t n = 0;
	uint8_t *bytes = backup_pack(g_data_dir, 0, &n);
	char path[700];
	snprintf(path, sizeof path, "%s/export.cwsave", g_data_dir);
	bool ok = bytes && backup_write_file(path, bytes, n);
	free(bytes);
	if (!ok) { snprintf(SV.note, sizeof SV.note, "There are no saves to export, or the file could not be written."); return; }
	if (!pick_saves_export(path)) { snprintf(SV.note, sizeof SV.note, "The export could not be written to your folder."); return; }
	SV.busy = pick_saves_busy();
	if (SV.busy) mirror_hold(true);
	if (!SV.busy) saves_exported(SV.place);
	saves_refresh();
}

void saves_activate(int action) {
	if (SV.busy) return;
	SV.operation = action;
	if (action == SAVES_EXPORT) { export_file(); return; }
	if (action == SAVES_IMPORT) {
		if (pick_saves_import()) { SV.busy = true; mirror_hold(true); return; }
		char path[700];
		snprintf(path, sizeof path, "%s/found.cwsave", g_data_dir);
		if (pick_saves_find(path)) saves_import(path);
		else snprintf(SV.note, sizeof SV.note, "Put %s in %s, then choose Import.", BACKUP_NAME, SV.place);
		return;
	}
	if (action == SAVES_AUTO) {
		if (!(pick_saves_capabilities() & SAVES_CAN_AUTO)) { snprintf(SV.note, sizeof SV.note, "Use Export to download a saves file. This browser cannot keep an export folder."); return; }
		pick_saves_auto(!pick_saves_auto_enabled());
		if (pick_saves_auto_enabled()) mirror_new_folder();
		snprintf(SV.note, sizeof SV.note, "Auto-export %s: %s", pick_saves_auto_enabled() ? "on" : "off", SV.place);
		return;
	}
	if (action == SAVES_UNDO) {
		if (!SV.undo) { snprintf(SV.note, sizeof SV.note, "There is no import to undo."); return; }
		if (!backup_undo(g_data_dir)) { saves_refusal(BACKUP_IO, &SV.here, SV.note, sizeof SV.note); return; }
		reload();
		snprintf(SV.note, sizeof SV.note, "Brought back the saves replaced by your last import.");
		return;
	}
	if (action == SAVES_FOLDER) {
		if (pick_saves_choose_folder()) { SV.busy = true; mirror_hold(true); }
		else snprintf(SV.note, sizeof SV.note, "Your transfer folder is %s.", SV.place);
	}
}

static void picked(void) {
	PickResult result;
	if (!pick_saves_done(&result)) return;
	SV.busy = false;
	if (!SV.comparing) mirror_hold(false);
	if (result.status < 0) {
		snprintf(SV.note, sizeof SV.note, "%s", result.status == -1 ? "Cancelled. Your saves are kept." : result.text);
		return;
	}
	if (SV.operation == SAVES_IMPORT) { saves_import(result.text); return; }
	saves_refresh();
	if (SV.operation == SAVES_EXPORT) saves_exported(result.text);
	else {
		mirror_new_folder();
		snprintf(SV.note, sizeof SV.note, "Transfer folder: %s", SV.place);
	}
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
	picked();
	if (SV.busy) return;
	if (saves_pointer()) return;
	if (!SV.comparing && SV.note[0]) {
		if (btn_pressed(BTN_A) || btn_pressed(BTN_B) || btn_pressed(BTN_START)) SV.note[0] = 0;
		return;
	}
	int count = SV.comparing ? 2 : SAVES_ACTIONS;
	if (btn_repeat(BTN_UP) || btn_repeat(BTN_LEFT)) SV.focus = (SV.focus + count - 1) % count;
	if (btn_repeat(BTN_DOWN) || btn_repeat(BTN_RIGHT)) SV.focus = (SV.focus + 1) % count;
	if (SV.comparing && SV.status != BACKUP_OK) SV.focus = SAVES_KEEP;
	if (btn_pressed(BTN_B)) { saves_back(); return; }
	if (btn_pressed(BTN_A) || btn_pressed(BTN_START)) {
		if (SV.comparing) saves_answer(SV.focus == SAVES_TAKE);
		else saves_activate(SV.focus);
	}
}

const Scene scene_saves = { "saves", enter, update, saves_draw, leave };

void saves_tick(void) {
	const Scene *scene = scene_current();
	if (scene != &scene_title && scene != &scene_intro && scene != &scene_saves) return;
	char path[1100];
	if (platform_dropped(path, sizeof path)) {
		size_t n = strlen(path);
		if (n > 7 && !strcmp(path + n - 7, ".cwsave")) saves_import(path);
	}
	if (SV.comparing || SV.busy) return;
	if (mirror_scan(path, sizeof path)) saves_import(path);
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void cw_saves_open(void) { saves_open(); }
EMSCRIPTEN_KEEPALIVE
void cw_saves_import(const char *path) { saves_import(path); }
#endif

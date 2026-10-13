/* saves_actions.h. Nothing here changes the saves but USE THE FILE'S and
 * UNDO IMPORT (backup.h's transactions), and the copy's answers mirror.h
 * keeps. */
#include "saves_actions.h"

#include <stdio.h>
#include <stdlib.h>

#include "audio.h"
#include "mirror.h"
#include "pick.h"
#include "platform.h"
#include "rivals.h"
#include "rom.h"
#include "save.h"
#include "saves_state.h"
#include "saves_text.h"

static int picking;   /* the picker open: SAVES_EXPORT, SAVES_IMPORT, SAVES_FOLDER */

static bool bn5(void) { return XR[XROM_BN5_COLONEL_US].data != NULL; }

int saves_actions(int *out) {
	int n = 0;
	unsigned caps = pick_saves_capabilities();
	if (SV.view == SAVES_HOME) {
		if (SV.have) out[n++] = SAVES_EXPORT;
		out[n++] = SAVES_IMPORT;
		if (SV.undo) out[n++] = SAVES_UNDO;
		if (caps & SAVES_CAN_AUTO) out[n++] = SAVES_AUTO;
		if (caps & SAVES_CAN_FOLDER) out[n++] = SAVES_FOLDER;
	} else if (SV.view == SAVES_COMPARE) {
		if (SV.status != BACKUP_OK) out[n++] = SAVES_DONE;
		else {
			out[n++] = SV.have ? SAVES_KEEP : SAVES_DONE;
			out[n++] = SAVES_TAKE;
		}
	} else {
		out[n++] = SV.resume ? SAVES_CONTINUE : SAVES_DONE;
		if (SV.offer_undo) out[n++] = SAVES_UNDO;
	}
	return n;
}

static void reload(void) {
	/* (the profile and the rival's record read again over the title; the
	 * launcher's game reads them as it begins) */
	if (R.data) {
		save_init();
		rivals_load();
	}
	platform_persist();
	saves_refresh();
}

/* the result's CONTINUE: where the run on this device can go on */
static bool can_continue(void) {
	return R.data && SV.here.has_run && backup_check(&SV.here, bn5()) == BACKUP_OK;
}

static void take(void) {
	BackupInfo before = SV.here;
	bool had = SV.have, from_place = SV.from_place;
	BackupStatus status = backup_restore(g_data_dir, SV.bytes, SV.size, bn5());
	if (status != BACKUP_OK) {
		saves_clear_file();
		saves_refusal(status, &SV.file, SV.note, sizeof SV.note);
		saves_result(SV.note);
		return;
	}
	if (from_place) mirror_answer(true);
	saves_clear_file();
	reload();
	char text[300];
	saves_imported(&SV.here, &before, had, text, sizeof text);
	saves_result(text);
	SV.offer_undo = SV.undo;
	SV.undone = false;
	SV.resume = can_continue();
	SV.focus = SV.resume ? SAVES_CONTINUE : SAVES_DONE;
	if (R.data) audio_sfx(SFX_GOT);
}

/* UNDO IMPORT: the two folders swapped at once, the result offering the
 * swap again (UNDO: the imported saves back) */
static void undo(void) {
	BackupInfo previous, was = SV.here;
	bool redo = SV.view == SAVES_RESULT && SV.undone;
	BackupStatus status = backup_undo_info(g_data_dir, &previous) ? backup_check(&previous, bn5()) : previous.status;
	if (status == BACKUP_OK && !backup_undo(g_data_dir)) status = BACKUP_IO;
	if (status != BACKUP_OK) {
		saves_refusal(status, &previous, SV.note, sizeof SV.note);
		saves_result(SV.note);
		return;
	}
	reload();
	char text[300];
	if (redo) saves_imported(&SV.here, &was, true, text, sizeof text);
	else saves_undone(&SV.here, SV.have, text, sizeof text);
	saves_result(text);
	SV.undone = !redo;
	SV.offer_undo = true;
	SV.resume = can_continue();
	SV.focus = SV.resume ? SAVES_CONTINUE : SAVES_DONE;
	if (R.data) audio_sfx(SFX_GOT);
}

/* (the data folder's file an export hands its picker, gone once it ends) */
static void export_path(char *out, size_t n) { snprintf(out, n, "%s/saves-export.tmp", g_data_dir); }

static void export_file(void) {
	size_t n = 0;
	uint8_t *bytes = backup_pack(g_data_dir, 0, &n);
	char path[700];
	export_path(path, sizeof path);
	bool ok = bytes && backup_write_file(path, bytes, n);
	if (ok) mirror_wrote(bytes, n);
	free(bytes);
	if (!ok || !pick_saves_export(path)) {
		remove(path);
		saves_result(saves_not_written());
		return;
	}
	SV.busy = true;
	picking = SAVES_EXPORT;
}

static void import_file(void) {
	/* (the copy's place holding saves not answered: those first) */
	if (pick_saves_auto_enabled() && saves_compare_held()) return;
	SV.busy = pick_saves_import();
	picking = SAVES_IMPORT;
}

static void continue_run(void) {
	if (!load_run()) {
		saves_result(saves_no_continue());
		return;
	}
	emu_resume_requested = true;
	scene_set(&scene_emu);
}

void saves_activate(int action) {
	if (SV.busy) return;
	if (R.data && action != SAVES_TAKE && action != SAVES_UNDO) audio_sfx(action == SAVES_KEEP || action == SAVES_DONE ? SFX_CANCEL : SFX_SELECT);
	switch (action) {
	case SAVES_EXPORT: export_file(); break;
	case SAVES_IMPORT: import_file(); break;
	case SAVES_UNDO: undo(); break;
	case SAVES_AUTO:
		pick_saves_auto(!pick_saves_auto_enabled());
		mirror_new_folder();
		break;
	case SAVES_FOLDER:
		SV.busy = pick_saves_choose_folder();
		picking = SAVES_FOLDER;
		break;
	case SAVES_KEEP: saves_leave_compare(true); break;
	case SAVES_TAKE: take(); break;
	case SAVES_CONTINUE: continue_run(); break;
	default:
		if (SV.view == SAVES_COMPARE) saves_leave_compare(false);
		else {
			SV.view = SAVES_HOME;
			SV.focus = SV.have ? SAVES_EXPORT : SAVES_IMPORT;
		}
		break;
	}
}

static void import_picked(const char *path) {
	size_t n = 0;
	uint8_t *bytes = backup_read_file(path, &n);
	/* (a phone's or the page's copy of the file chosen: gone once read) */
	char staged[700];
	snprintf(staged, sizeof staged, "%s/saves-import.cwsave", g_data_dir);
	remove(staged);
	saves_compare(bytes, bytes ? n : 0, false);
}

bool saves_picked(void) {
	PickResult r;
	if (!pick_saves_done(&r)) return false;
	SV.busy = false;
	char staged[700];
	export_path(staged, sizeof staged);
	remove(staged);
	if (r.status == -1) return true;
	if (r.status < 0) { saves_result(r.text); return true; }
	if (r.path) import_picked(r.text);
	else if (picking == SAVES_FOLDER) {
		/* (a folder chosen: looked at now, its saves compared where it has
		 * some not answered yet) */
		mirror_new_folder();
		mirror_look();
		saves_refresh();
		if (mirror_incoming()) saves_compare_held();
	} else {
		char text[1200];
		saves_exported(r.text, text, sizeof text);
		saves_result(text);
	}
	return true;
}

/* Choices and picker outcomes for the portable saves screen. */
#include "saves_state.h"

#include <stdio.h>
#include <stdlib.h>

#include "audio.h"
#include "mirror.h"
#include "pick.h"
#include "rom.h"
#include "save.h"
#include "saves_text.h"

bool saves_disabled(int action) {
	if (SV.busy) return true;
	unsigned caps = pick_saves_capabilities();
	switch (action) {
	case SAVES_EXPORT: return !SV.have;
	case SAVES_UNDO: return !SV.undo;
	case SAVES_AUTO: return !(caps & SAVES_CAN_AUTO);
	case SAVES_FOLDER: return !(caps & SAVES_CAN_FOLDER);
	case SAVES_RETRY: return !(caps & SAVES_CAN_AUTO) || !pick_saves_auto_enabled() || !SV.place[0] || !SV.have;
	case SAVES_CONTINUE: return !SV.resume || !SV.result_resume;
	case SAVES_TAKE: return SV.status != BACKUP_OK;
	case SAVES_PREV: return SV.details_page <= 0;
	case SAVES_NEXT: return SV.details_page >= saves_details_pages() - 1;
	default: return false;
	}
}

static void undo_preview(void) {
	saves_refresh();
	SV.status = backup_undo_info(g_data_dir, &SV.file) ? backup_check(&SV.file, XR[XROM_BN5_COLONEL_US].data != NULL) : SV.file.status;
	SV.undo_preview = true;
	SV.view = SAVES_COMPARE;
	SV.focus = SAVES_KEEP;
	SV.path[0] = 0;
	mirror_hold(true);
	if (SV.status != BACKUP_OK) saves_refusal(SV.status, &SV.file, SV.note, sizeof SV.note);
	else saves_undo_loss(SV.note, sizeof SV.note);
}

static BackupStatus restore_previous(void) {
	BackupInfo previous;
	if (!backup_undo_info(g_data_dir, &previous)) return previous.status;
	BackupStatus status = backup_check(&previous, XR[XROM_BN5_COLONEL_US].data != NULL);
	if (status != BACKUP_OK) return status;
	if (previous.hash != SV.file.hash) return BACKUP_IO;
	return backup_undo(g_data_dir) ? BACKUP_OK : BACKUP_IO;
}

void saves_answer(bool take) {
	if (!saves_comparing() || (take && SV.status != BACKUP_OK)) return;
	bool undo = SV.undo_preview;
	int previous_layer = SV.here.run_depth;
	if (take) {
		BackupStatus status = undo ? restore_previous() : backup_restore(g_data_dir, SV.bytes, SV.size, XR[XROM_BN5_COLONEL_US].data != NULL);
		if (status != BACKUP_OK) {
			SV.status = status;
			saves_refusal(status, &SV.file, SV.note, sizeof SV.note);
			SV.view = SAVES_COMPARE;
			SV.focus = SAVES_KEEP;
			return;
		}
		saves_reload();
	}
	saves_clear_file();
	if (!take) {
		SV.view = SAVES_HOME;
		SV.focus = undo ? SAVES_UNDO : SAVES_IMPORT;
		saves_feedback("Kept this device's saves.");
	} else {
		char text[350];
		saves_success(undo, previous_layer, text, sizeof text);
		saves_result(text, !undo);
		SV.result_resume = SV.resume;
		if (SV.result_resume) SV.focus = SAVES_CONTINUE;
	}
	if (R.data) audio_sfx(take ? SFX_GOT : SFX_CANCEL);
}

static void exported(const char *where) {
	char text[1100];
	snprintf(text, sizeof text, "Exported %s to %s.", BACKUP_NAME, where && *where ? where : SV.place);
	saves_result(text, false);
}

static void export_file(void) {
	size_t n = 0;
	uint8_t *bytes = backup_pack(g_data_dir, 0, &n);
	char path[700];
	snprintf(path, sizeof path, "%s/export.cwsave", g_data_dir);
	bool ok = bytes && backup_write_file(path, bytes, n);
	free(bytes);
	if (!ok) { saves_result("The export could not be written. Check your device's free space and try Export again.", false); return; }
	if (!pick_saves_export(path)) { saves_result("The export could not be written to your folder. Check its access and free space, then try Export again.", false); return; }
	SV.busy = pick_saves_busy();
	if (SV.busy) mirror_hold(true);
	else exported(SV.place);
	saves_refresh();
}

static void import_file(void) {
	if (pick_saves_import()) { SV.busy = true; mirror_hold(true); return; }
	char path[700];
	snprintf(path, sizeof path, "%s/found.cwsave", g_data_dir);
	if (pick_saves_find(path)) saves_import(path);
	else {
		char text[1100];
		snprintf(text, sizeof text, "Put %s in %s, then choose Import a file.", BACKUP_NAME, SV.place);
		saves_result(text, false);
	}
}

static void show_details(bool file) {
	SV.details_back = SV.view;
	SV.details_focus = SV.focus;
	SV.details_file = file;
	SV.details_page = 0;
	SV.view = SAVES_DETAIL;
	SV.focus = SAVES_NEXT;
}

static void transfer_action(int action) {
	if (action == SAVES_AUTO) {
		pick_saves_auto(!pick_saves_auto_enabled());
		if (pick_saves_auto_enabled()) mirror_retry();
		saves_feedback(pick_saves_auto_enabled() ? "Auto-export on. The folder is checked before copying." : "Auto-export off. Use Export a copy when needed.");
	} else if (action == SAVES_RETRY) {
		mirror_retry();
		saves_feedback("Retrying. The folder is checked before copying.");
	} else if (pick_saves_choose_folder()) {
		SV.busy = true;
		mirror_hold(true);
	}
}

static void continue_run(void) {
	if (!R.data || !load_run()) { saves_result("This checkpoint could not be loaded. Your saves are kept. Return to SAVES to inspect it.", false); return; }
	emu_resume_requested = true;
	scene_set(&scene_emu);
}

void saves_activate(int action) {
	if (SV.busy) return;
	SV.feedback[0] = 0;
	if (saves_disabled(action)) {
		char reason[200];
		saves_action_hint(action, reason, sizeof reason);
		saves_feedback(reason);
		return;
	}
	SV.operation = action;
	switch (action) {
	case SAVES_EXPORT: export_file(); break;
	case SAVES_IMPORT: import_file(); break;
	case SAVES_UNDO: undo_preview(); break;
	case SAVES_OPTIONS: SV.view = SAVES_TRANSFER; SV.focus = SAVES_AUTO; break;
	case SAVES_DETAILS: case SAVES_HERE_DETAILS: show_details(false); break;
	case SAVES_FILE_DETAILS: show_details(true); break;
	case SAVES_AUTO: case SAVES_FOLDER: case SAVES_RETRY: transfer_action(action); break;
	case SAVES_CONTINUE: continue_run(); break;
	case SAVES_DONE: SV.view = SAVES_HOME; SV.focus = SAVES_IMPORT; break;
	case SAVES_KEEP: case SAVES_TAKE: saves_answer(action == SAVES_TAKE); break;
	case SAVES_PREV: --SV.details_page; break;
	case SAVES_NEXT: ++SV.details_page; break;
	default: break;
	}
}

bool saves_picked(void) {
	PickResult result;
	if (!pick_saves_done(&result)) return false;
	SV.busy = false;
	if (!saves_comparing()) mirror_hold(false);
	if (result.status < 0) {
		if (result.status == -1) saves_feedback("Cancelled. Your saves are kept.");
		else saves_result(result.text, false);
		return true;
	}
	if (SV.operation == SAVES_IMPORT) { saves_import(result.text); return true; }
	saves_refresh();
	if (SV.operation == SAVES_EXPORT) exported(result.text);
	else { mirror_new_folder(); saves_feedback("Transfer folder selected. Checked before copying."); }
	return true;
}

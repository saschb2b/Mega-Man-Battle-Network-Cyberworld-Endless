#include "saves_text.h"

#include <stdio.h>
#include <time.h>

#include "guardians.h"
#include "mirror.h"
#include "pick.h"
#include "rom.h"
#include "saves_state.h"

const char *saves_label(int action) {
	static const char *const labels[] = {
		"Export a copy", "Import a file", "Undo last import", "Options", "Details",
		"Auto-export", "Choose folder", "Retry copy", "Done", "Continue",
		"Keep this device", "Use file", "Local details", "File details", "Previous", "Next"
	};
	if (action == SAVES_TAKE && SV.undo_preview) return "Restore";
	if (action == SAVES_FILE_DETAILS && SV.undo_preview) return "Undo details";
	return action >= 0 && action < SAVES_ACTIONS ? labels[action] : "SAVES";
}

const char *saves_hint(int action) {
	switch (action) {
	case SAVES_EXPORT: return "Copy your progress and checkpoint to a file.";
	case SAVES_IMPORT: return "Compare a file before replacing your saves.";
	case SAVES_UNDO: return "Preview the saves replaced by your import.";
	case SAVES_OPTIONS: return "Automatic copies, folder and retry.";
	case SAVES_DETAILS: case SAVES_HERE_DETAILS: return "Read the full save details and folder.";
	case SAVES_FILE_DETAILS: return "Read the other copy's full details.";
	case SAVES_AUTO: return "Keep a fresh file in your folder after each save.";
	case SAVES_FOLDER: return "Choose the folder used for automatic copies and incoming saves.";
	case SAVES_RETRY: return "Check the folder for incoming saves, then try copying again.";
	case SAVES_CONTINUE: return "Resume this device's checkpoint.";
	case SAVES_KEEP: return "Keep this device's current progress.";
	case SAVES_TAKE: return "Replace this device's progress. Undo keeps the current copy.";
	default: return "Your device settings stay here.";
	}
}

static void date_text(uint64_t stamp, bool full, char *out, size_t n) {
	time_t seconds = (time_t)stamp;
	struct tm *date = stamp ? localtime(&seconds) : NULL;
	if (date && full) strftime(out, n, "%Y-%m-%d %H:%M", date);
	else if (date) strftime(out, n, "%m-%d %H:%M", date);
	else snprintf(out, n, "unknown");
}

void saves_summary(const BackupInfo *info, int row, char *out, size_t n) {
	*out = 0;
	if (row == 0) { snprintf(out, n, "%d runs / best %d", info->runs, info->best); return; }
	if (row == 1) {
		if (info->run_depth) snprintf(out, n, "Layer %d / act %d", info->run_depth, info->run_act);
		else snprintf(out, n, "No run in progress");
		return;
	}
	if (row == 2 && info->run_depth) {
		const NetAreaDef *a = R.layout && info->run_area >= 0 ? net_area_def(info->run_area) : NULL;
		const char *name = a && a->name ? a->name : info->needs_bn5 ? "Older net" : guardian_area_original_name(info->run_area);
		snprintf(out, n, "%s%s", name, info->needs_bn5 ? " (BN5)" : "");
		return;
	}
	if (row == 3) { date_text(info->stamp, true, out, n); return; }
	if (row == 4) snprintf(out, n, "%s%s%s", info->system[0] ? info->system : "Device unknown", info->device[0] ? " / " : "", info->device);
	if (row == 5) snprintf(out, n, "Build: %s", info->version[0] ? info->version : "unknown (old file)");
	if (row == 6) date_text(info->stamp, false, out, n);
}

void saves_refusal(BackupStatus status, const BackupInfo *info, char *out, size_t n) {
	switch (status) {
	case BACKUP_NEWER_BUILD:
	case BACKUP_NEWER_FORMAT:
	case BACKUP_INCOMPATIBLE_RUN:
		snprintf(out, n, "Made by %s: update this device first. Your saves are kept.", info->version[0] ? info->version : "a newer build"); break;
	case BACKUP_NEEDS_BN5: snprintf(out, n, "Needs BN5 Team Colonel (USA). Add its ROM first. Your saves are kept."); break;
	case BACKUP_DAMAGED: snprintf(out, n, "This copy is damaged or is not a saves file. Your saves are kept."); break;
	case BACKUP_IO: snprintf(out, n, "The saves could not be read or written. Check access and free space. Your saves are kept."); break;
	default: snprintf(out, n, "Your controls, screen, volumes and statistics answer stay here."); break;
	}
}

void saves_loss(const BackupInfo *here, const BackupInfo *file, char *out, size_t n) {
	char replace[120];
	if (P.w < 230) snprintf(replace, sizeof replace, "Use file. Undo available.");
	else if (here->run_depth && file->run_depth) snprintf(replace, sizeof replace, "Replace layer %d with %d. Undo available.", here->run_depth, file->run_depth);
	else if (here->run_depth) snprintf(replace, sizeof replace, "Replace your layer %d run. Undo available.", here->run_depth);
	else snprintf(replace, sizeof replace, "Replace this device's progress. Undo available.");
	snprintf(out, n, "%s\nSettings stay here.%s", replace, pick_saves_auto_enabled() && (pick_saves_capabilities() & SAVES_CAN_AUTO)
		? "\nKeep copies to folder (Auto on)." : "");
}

void saves_undo_loss(char *out, size_t n) {
	if (P.w < 230) snprintf(out, n, "Restore previous saves.\nUndo keeps current saves.\nSettings stay here.");
	else if (SV.file.run_depth) snprintf(out, n, "Restore layer %d. Undo keeps current saves.\nSettings stay here.", SV.file.run_depth);
	else snprintf(out, n, "Restore previous progress (no active run).\nUndo keeps current saves. Settings stay here.");
}

void saves_action_hint(int action, char *out, size_t n) {
	const char *hint = saves_hint(action);
	unsigned caps = pick_saves_capabilities();
	if (action == SAVES_EXPORT && !SV.have) hint = "Start a game before exporting its saves.";
	else if (action == SAVES_UNDO && !SV.undo) hint = "No import to undo yet.";
	else if (action == SAVES_AUTO && !(caps & SAVES_CAN_AUTO)) hint = "This browser uses manual downloads. Use Export a copy.";
	else if (action == SAVES_FOLDER && !(caps & SAVES_CAN_FOLDER)) hint = caps & SAVES_CAN_AUTO
		? "This system uses the folder shown above. Details shows its full path." : "This browser uses downloads and uploads, without a transfer folder.";
	else if (action == SAVES_RETRY && (!(caps & SAVES_CAN_AUTO) || !pick_saves_auto_enabled())) hint = caps & SAVES_CAN_AUTO
		? "Turn Auto-export on to retry a folder copy." : "Use Export a copy to download again in this browser.";
	else if (action == SAVES_RETRY && !SV.place[0]) hint = "Choose a transfer folder before trying an automatic copy.";
	else if (action == SAVES_RETRY && !SV.have) hint = "Start a game first. Its progress can then be copied.";
	else if (action == SAVES_TAKE && SV.status != BACKUP_OK) hint = "This copy cannot be used. Read the reason above or its Details.";
	snprintf(out, n, "%s", hint);
}

static const char *copy_state(MirrorState state) {
	switch (state) {
	case MIRROR_PENDING: return "Copy pending";
	case MIRROR_COPIED: return "Copied to folder";
	case MIRROR_FAILED: return "Copy failed - Retry in Options";
	case MIRROR_INCOMING: return "Incoming saves - compare first";
	default: return "No automatic copy yet";
	}
}

void saves_transfer_status(char *out, size_t n) {
	if (!(pick_saves_capabilities() & SAVES_CAN_AUTO)) {
		snprintf(out, n, "Browser: manual downloads\nKeep a copy before clearing storage");
		return;
	}
	if (!SV.have) {
		snprintf(out, n, "No saves to copy yet\nFolder: %s", SV.place[0] ? SV.place : "not chosen");
		return;
	}
	MirrorStatus status;
	mirror_status(&status);
	char date[64];
	date_text(status.copied_at, false, date, sizeof date);
	snprintf(out, n, "%s%s%s\nFolder: %s", pick_saves_auto_enabled() ? copy_state(status.state) : "Auto-export off",
		status.copied_at && status.state == MIRROR_COPIED ? " " : "", status.copied_at && status.state == MIRROR_COPIED ? date : "",
		SV.place[0] ? SV.place : "not chosen");
}

void saves_details_text(char *out, size_t n) {
	const BackupInfo *info = SV.details_file ? &SV.file : &SV.here;
	char progress[128], area[128], runs[128], played[64], source[160], build[128];
	saves_summary(info, 1, progress, sizeof progress);
	saves_summary(info, 2, area, sizeof area);
	saves_summary(info, 0, runs, sizeof runs);
	saves_summary(info, 3, played, sizeof played);
	saves_summary(info, 4, source, sizeof source);
	saves_summary(info, 5, build, sizeof build);
	const char *label = SV.details_file ? SV.undo_preview ? "Previous local saves" : "Incoming file" : "This device";
	const char *message = SV.details_back == SAVES_RESULT || (SV.details_file && SV.status != BACKUP_OK) ? SV.note : "";
	int used = snprintf(out, n, "%s%s%s\n%s\n%s\n%s\nLast played: %s\n%s\n%s\n", message, *message ? "\n" : "", label, progress, area, runs, played, source, build);
	if (used < 0 || (size_t)used >= n) return;
	if (SV.details_file) {
		snprintf(out + used, n - (size_t)used, "%s", SV.undo_preview ? "Kept before the last import. Restore swaps this copy with the current saves. Device settings stay here."
			: SV.path[0] ? SV.path : "File selected for import");
		return;
	}
	MirrorStatus status;
	mirror_status(&status);
	char copied[64];
	date_text(status.copied_at, true, copied, sizeof copied);
	snprintf(out + used, n - (size_t)used, "Auto-export: %s\n%s\nLast successful automatic copy: %s\nTransfer folder: %s\nA folder copy does not confirm cloud sync. Your controls, screen, volumes and statistics answer stay on this device.",
		pick_saves_auto_enabled() ? "on" : "off", copy_state(status.state), copied, SV.place[0] ? SV.place : "unavailable (manual downloads)");
}

const char *saves_panel_title(bool file) {
	return file ? SV.undo_preview ? "Previous saves" : "The file" : "This device";
}

const char *saves_view_title(void) {
	switch (SV.view) {
	case SAVES_TRANSFER: return "OPTIONS";
	case SAVES_DETAIL: return "DETAILS";
	case SAVES_COMPARE: return SV.undo_preview ? "UNDO IMPORT" : "COMPARE SAVES";
	default: return "SAVES";
	}
}

void saves_page_label(int page, int pages, char *out, size_t n) {
	snprintf(out, n, "Page %d / %d", page + 1, pages);
}

void saves_success(bool undo, int previous_layer, char *out, size_t n) {
	char progress[96], recovery[100];
	saves_summary(&SV.here, 1, progress, sizeof progress);
	if (undo) snprintf(recovery, sizeof recovery, "Current saves remain available in Undo.");
	else if (previous_layer) snprintf(recovery, sizeof recovery, "Undo brings back your previous layer %d run.", previous_layer);
	else snprintf(recovery, sizeof recovery, "Undo brings back your previous progress.");
	const char *next = SV.resume ? " Continue resumes this checkpoint." : SV.here.has_run
		? " Open the game with its ROM to continue." : " Your progress is ready for the next run.";
	snprintf(out, n, "%s %s. %s%s", undo ? "Previous saves restored." : "Imported.", progress, recovery, next);
}

#include "saves_text.h"

#include <stdio.h>
#include <time.h>

#include "rom.h"
#include "guardians.h"
#include "saves_state.h"

const char *saves_label(int action) {
	static const char *const labels[] = { "Export", "Import", "Auto-export", "Undo last import", "Choose folder" };
	return action >= 0 && action < SAVES_ACTIONS ? labels[action] : "SAVES";
}

const char *saves_hint(int action) {
	static const char *const hints[] = {
		"Export one file to take your saves to another device.",
		"Import a file. Compare before replacing this device's saves.",
		"Keep a fresh export in your folder after each save.",
		"Bring back the saves replaced by your last import.",
		"Keep a folder for exports and look for imports there at start."
	};
	return action >= 0 && action < SAVES_ACTIONS ? hints[action] : "Your controls, screen, volumes and statistics answer stay on this device.";
}

void saves_summary(const BackupInfo *info, int row, char *out, size_t n) {
	*out = 0;
	if (row == 0) { snprintf(out, n, "%d runs / best %d", info->runs, info->best); return; }
	if (row == 1) {
		if (info->run_depth) snprintf(out, n, "Run: layer %d / act %d", info->run_depth, info->run_act);
		else snprintf(out, n, "No run in progress");
		return;
	}
	if (row == 2 && info->run_depth) {
		const NetAreaDef *a = R.layout && info->run_area >= 0 ? net_area_def(info->run_area) : NULL;
		const char *name = a && a->name ? a->name : info->needs_bn5 ? "Older net" : guardian_area_original_name(info->run_area);
		snprintf(out, n, "%s%s", name, info->needs_bn5 ? " (BN5)" : "");
		return;
	}
	if (row == 3) {
		time_t stamp = (time_t)info->stamp;
		struct tm *date = info->stamp ? localtime(&stamp) : NULL;
		if (date) strftime(out, n, "%Y-%m-%d %H:%M", date);
		else snprintf(out, n, "Last played: unknown");
		return;
	}
	if (row == 4) snprintf(out, n, "%s%s%s", info->system[0] ? info->system : "Device unknown", info->device[0] ? " / " : "", info->device);
	if (row == 5) snprintf(out, n, "Build: %s", info->version[0] ? info->version : "unknown (old file)");
}

void saves_refusal(BackupStatus status, const BackupInfo *info, char *out, size_t n) {
	switch (status) {
	case BACKUP_NEWER_BUILD:
	case BACKUP_NEWER_FORMAT:
	case BACKUP_INCOMPATIBLE_RUN:
		snprintf(out, n, "Made by %s: update this device first. Your saves are kept.", info->version[0] ? info->version : "a newer build"); break;
	case BACKUP_NEEDS_BN5: snprintf(out, n, "Needs BN5 Team Colonel (USA). Add its ROM first. Your saves are kept."); break;
	case BACKUP_DAMAGED: snprintf(out, n, "This file is damaged or is not a saves file. Your saves are kept."); break;
	case BACKUP_IO: snprintf(out, n, "The saves could not be written. Your saves are kept."); break;
	default: snprintf(out, n, "Your controls, screen, volumes and statistics answer stay here."); break;
	}
}

void saves_loss(const BackupInfo *here, const BackupInfo *file, char *out, size_t n) {
	if (here->run_depth && file->run_depth) snprintf(out, n, "Replace layer %d run with layer %d. Undo brings it back.", here->run_depth, file->run_depth);
	else if (here->run_depth) snprintf(out, n, "Replace layer %d run. Undo brings it back.", here->run_depth);
	else snprintf(out, n, "Replace this device's progress. Undo available. Settings stay here.");
}

/* saves_text.h. Menu words: short, plain, in capitals on the buttons as
 * on the ROMs screen; one name for each thing: this device, the file, the
 * folder, the build, Layer N and Act N (story_words.c's capitals). */
#include "saves_text.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "guardians.h"
#include "pick.h"
#include "rom.h"
#include "saves.h"
#include "saves_state.h"

const char *saves_word_title(void) { return "SAVES"; }

const char *saves_word_hint(void) { return "Saves"; }

const char *saves_word_back(void) { return "BACK"; }

const char *saves_label(int action) {
	switch (action) {
	case SAVES_EXPORT: return "EXPORT";
	case SAVES_IMPORT: return "IMPORT";
	case SAVES_UNDO: return SV.undone ? "UNDO" : "UNDO IMPORT";
	case SAVES_AUTO: return pick_saves_auto_enabled() ? "AUTO-COPY: ON" : "AUTO-COPY: OFF";
	case SAVES_FOLDER: return "FOLDER";
	case SAVES_KEEP: return "KEEP THESE";
	case SAVES_TAKE: return SV.have || SV.unreadable ? "USE THE FILE'S" : "BRING BACK";
	case SAVES_CONTINUE: return "CONTINUE";
	default: return "DONE";
	}
}

const char *saves_panel_title(bool file) { return file ? "THE FILE" : "THIS DEVICE"; }

static void when(uint64_t stamp, char *out, size_t n) {
	time_t seconds = (time_t)stamp;
	struct tm *date = stamp ? localtime(&seconds) : NULL;
	if (!date || !strftime(out, n, "%m-%d %H:%M", date)) out[0] = 0;
}

static const char *area_name(const BackupInfo *info) {
	const NetAreaDef *a = R.layout && !info->needs_bn5 && info->run_area >= 0 ? net_area_def(info->run_area) : NULL;
	if (a && a->name) return a->name;
	return info->needs_bn5 ? "The older Net (BN5)" : guardian_area_original_name(info->run_area);
}

int saves_rows(const BackupInfo *info, bool damaged, bool source, char rows[4][64]) {
	if (damaged) {
		snprintf(rows[0], 64, "Damaged");
		return 1;
	}
	int k = 0, best = info->best > info->run_depth ? info->best : info->run_depth;
	if (info->run_depth > 0) {
		snprintf(rows[k++], 64, "Layer %d, Act %d", info->run_depth, info->run_act);
		const char *area = area_name(info);
		if (area && *area) snprintf(rows[k++], 64, "%s", area);
	}
	if (info->runs > 0) snprintf(rows[k++], 64, "%d run%s, best Layer %d", info->runs, info->runs == 1 ? "" : "s", best);
	else if (info->run_depth <= 0 && best > 0) snprintf(rows[k++], 64, "Best Layer %d", best);
	if (!k) snprintf(rows[k++], 64, "No runs yet");
	if (source && k < 4) {
		char date[32];
		when(info->stamp, date, sizeof date);
		const char *system = info->system[0] ? info->system : "";
		snprintf(rows[k++], 64, "%s%s%s", system, *system && *date ? ", " : "", date);
		if (!rows[k - 1][0]) --k;
	}
	return k;
}

void saves_status_line(const MirrorStatus *m, bool blocked, bool can_choose, char *out, size_t n) {
	out[0] = 0;
	if (blocked) { snprintf(out, n, "Not saving: savedata could not be put back."); return; }
	const char *p = m->place;
	char date[32];
	switch (m->state) {
	case MIRROR_OFF: if (*p) snprintf(out, n, "Folder: %s", p); break;
	case MIRROR_WAITING: snprintf(out, n, "Copies go to %s", p); break;
	case MIRROR_COPIED:
		when(m->copied_at, date, sizeof date);
		snprintf(out, n, "Copied to %s %s", p, date);
		break;
	case MIRROR_FAILED: snprintf(out, n, can_choose ? "No copy in %s: choose it again" : "No copy in %s: it could not be written", p); break;
	case MIRROR_HELD: snprintf(out, n, "New saves in %s: IMPORT shows them", p); break;
	case MIRROR_UNREAD: snprintf(out, n, "No copy: the file in %s could not be read", p); break;
	}
}

static void phrase(const BackupInfo *info, char *out, size_t n) {
	int best = info->best > info->run_depth ? info->best : info->run_depth;
	if (info->run_depth > 0) snprintf(out, n, "a run on Layer %d", info->run_depth);
	else if (info->runs > 0) snprintf(out, n, "%d run%s, best Layer %d", info->runs, info->runs == 1 ? "" : "s", best);
	else snprintf(out, n, "no runs yet");
}

void saves_compare_note(const BackupInfo *here, bool have, bool unreadable, const char *place, char *out, size_t n) {
	int used;
	if (unreadable) used = snprintf(out, n, "This device's saves cannot be read. UNDO IMPORT brings them back.");
	else if (!have) used = snprintf(out, n, "This device has no saves yet.");
	else if (here->run_depth > 0) used = snprintf(out, n, "USE THE FILE'S replaces your Layer %d run. UNDO IMPORT brings it back.", here->run_depth);
	else used = snprintf(out, n, "USE THE FILE'S replaces your saves. UNDO IMPORT brings them back.");
	if (place && have && used > 0 && (size_t)used < n)
		snprintf(out + used, n - (size_t)used, "\nKEEP THESE: the copy in %s becomes this device's.", place);
}

void saves_refusal(BackupStatus status, const BackupInfo *file, char *out, size_t n) {
	const char *why;
	char made[160];
	switch (status) {
	case BACKUP_NEWER_BUILD:
	case BACKUP_NEWER_FORMAT:
	case BACKUP_INCOMPATIBLE_RUN:
		snprintf(made, sizeof made, "Made by %s, newer than this one: update the game first.", file->version[0] ? file->version : "a newer build");
		why = made;
		break;
	case BACKUP_NEEDS_BN5: why = "This run needs BN5 Team Colonel (USA)."; break;
	case BACKUP_IO: why = "The saves could not be written here."; break;
	default: why = "Not a saves file, or damaged."; break;
	}
	snprintf(out, n, "%s\nYour saves are kept.", why);
}

void saves_imported(const BackupInfo *now, const BackupInfo *before, bool had, char *out, size_t n) {
	char got[64];
	phrase(now, got, sizeof got);
	if (had && before->run_depth > 0) snprintf(out, n, "Saves imported: %s. UNDO IMPORT brings back your Layer %d run.", got, before->run_depth);
	else if (had) snprintf(out, n, "Saves imported: %s. UNDO IMPORT brings back your saves.", got);
	else snprintf(out, n, "Saves imported: %s. UNDO IMPORT takes them out again.", got);
}

void saves_undone(const BackupInfo *now, bool have, char *out, size_t n) {
	char got[64];
	phrase(now, got, sizeof got);
	if (have) snprintf(out, n, "Your saves are back: %s. UNDO swaps them again.", got);
	else snprintf(out, n, "The import is undone. UNDO swaps it back.");
}

void saves_exported(const char *where, char *out, size_t n) { snprintf(out, n, "Exported to %s.", where); }

const char *saves_same(void) { return "The file holds the saves already here."; }

const char *saves_no_continue(void) { return "The checkpoint could not be loaded. Your saves are kept."; }

const char *saves_not_written(void) { return "The file could not be written."; }

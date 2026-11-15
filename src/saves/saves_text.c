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
#include "saves_cloud.h"
#include "saves_state.h"

static const char *word_title(void) { return "SAVES"; }

const char *saves_word_hint(void) { return "Saves"; }


/* (RECEIVE's keys: saves_code.c) */
static const char *code_key(int action) {
	static const char *const digits[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
	if (action >= SAVES_DIGIT && action < SAVES_DIGIT + 10) return digits[action - SAVES_DIGIT];
	return action == SAVES_ERASE ? "ERASE" : action == SAVES_GET ? "OK" : NULL;
}

const char *saves_label(int action) {
	const char *key = code_key(action);
	if (key) return key;
	switch (action) {
	case SAVES_UNDO: return SV.undone ? "UNDO" : "UNDO IMPORT";
	case SAVES_KEEP: return "KEEP THESE";
	/* (the comparison's other side, whatever brought it: THE FILE, THE CODE, THE CLOUD) */
	case SAVES_TAKE: return SV.have || SV.unreadable ? "USE THOSE" : "BRING BACK";
	case SAVES_CONTINUE: return "CONTINUE";
	default: return "DONE";
	}
}

const char *saves_panel_title(bool file) {
	if (!file) return "THIS DEVICE";
	return SV.from_cloud ? "THE CLOUD" : SV.from_code ? "THE CODE" : "THE FILE";
}

/* (the band's name: a view opened from a line takes its line's) */
const char *saves_view_title(void) {
	if (SV.view == SAVES_SEND_CODE) return SV.code_link ? "ADD A DEVICE" : "SEND";
	if (SV.view == SAVES_RECEIVE_CODE) return SV.linking ? "LINK" : "RECEIVE";
	return word_title();
}

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
	case MIRROR_OFF: break;
	case MIRROR_WAITING: snprintf(out, n, "No copy yet. It comes after the next save."); break;
	case MIRROR_COPIED:
		when(m->copied_at, date, sizeof date);
		snprintf(out, n, "Copied to %s %s", p, date);
		break;
	case MIRROR_FAILED: snprintf(out, n, can_choose ? "The copy failed. Choose the folder again." : "The copy in %s could not be written.", p); break;
	case MIRROR_HELD: snprintf(out, n, "New saves in %s. Import shows them.", p); break;
	case MIRROR_UNREAD: snprintf(out, n, "The copy in %s could not be read.", p); break;
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
	else if (here->run_depth > 0) used = snprintf(out, n, "USE THOSE replaces your Layer %d run. UNDO IMPORT brings it back.", here->run_depth);
	else used = snprintf(out, n, "USE THOSE replaces your saves. UNDO IMPORT brings them back.");
	if (place && have && used > 0 && (size_t)used < n)
		snprintf(out + used, n - (size_t)used, "\nKEEP THESE writes this device's saves over the copy in %s.", place);
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

const char *saves_waiting(int action) {
	return action == SAVES_EXPORT ? "Choose where it goes, in the picker." : action == SAVES_IMPORT ? "Choose the file, in the picker." :
		"Choose the folder, in the picker.";
}

const char *saves_cancelled(int action) {
	return action == SAVES_EXPORT ? "Nothing exported." : action == SAVES_IMPORT ? "Nothing imported." : "The folder is as it was.";
}

/* (saves sent by a code: SEND's and RECEIVE's words) */

void saves_code_shown(const char *code, char *out, size_t n) { snprintf(out, n, "%.5s %.5s", code, strlen(code) > 5 ? code + 5 : ""); }

void saves_code_typed(const char *code, char *out, size_t n) {
	char all[11];
	size_t k = strlen(code);
	for (size_t i = 0; i < 10; ++i) all[i] = i < k ? code[i] : '-';
	all[10] = 0;
	saves_code_shown(all, out, n);
}

void saves_code_left(uint32_t ms, char *out, size_t n) {
	unsigned minutes = (ms + 59999) / 60000;
	if (ms < 60000) snprintf(out, n, "Works once, for under a minute.");
	else snprintf(out, n, "Works once, for %u more minutes.", minutes);
}

const char *saves_code_prompt(void) { return "Type the code your other device shows."; }

static const char *trouble(const TransferState *s, char *busy, size_t n) {
	switch (s->trouble) {
	case TRANSFER_UNREACHED: return "The server could not be reached.";
	case TRANSFER_BUSY: {
		unsigned minutes = (s->wait_s + 59) / 60;
		snprintf(busy, n, "The server is busy. Try again in %u minute%s.", minutes ? minutes : 1, minutes > 1 ? "s" : "");
		return busy;
	}
	case TRANSFER_NO_SAVES: return "Nothing waits under that code. A code works once, for 15 minutes.";
	case TRANSFER_OFF: return "This server takes no saves yet.";
	case TRANSFER_DAMAGED: return "What came could not be opened.";
	case TRANSFER_TOO_BIG: return "These saves are too big to send.";
	default: return "Not enough memory for this.";
	}
}

void saves_code_words(const TransferState *s, bool receiving, bool expired, char *out, size_t n) {
	char busy[96];
	int percent = s->size ? (int)((uint64_t)s->moved * 100 / s->size) : 0;
	out[0] = 0;
	switch (s->phase) {
	case TRANSFER_WORKING:
		if (!s->moved) snprintf(out, n, "%s", receiving ? "Asking the server..." : "Sending...");
		else snprintf(out, n, "%s: %d%%", receiving ? "Receiving" : "Sending", percent);
		break;
	case TRANSFER_SENT:
		if (expired) snprintf(out, n, "%s", SV.code_link ? "This code's time is up. Add a device again for a new one." : "This code's time is up. Send again for a new one.");
		else snprintf(out, n, "%s", SV.code_link ? "On your other device: SAVES, CLOUD, Link with a code." : "On your other device: SAVES, CODE, Receive.");
		break;
	case TRANSFER_FAILED:
		snprintf(out, n, "%s%s", trouble(s, busy, sizeof busy), receiving ? "\nYour saves are kept." : "");
		break;
	default: break;
	}
}

/* (saves kept in step: the cloud's words) */

void saves_cloud_compare_note(const BackupInfo *here, bool have, bool unreadable, char *out, size_t n) {
	if (unreadable) snprintf(out, n, "This device's saves cannot be read. UNDO IMPORT brings them back.");
	else if (!have) snprintf(out, n, "This device has no saves yet.");
	else if (here->run_depth > 0) snprintf(out, n, "USE THOSE replaces your Layer %d run. UNDO IMPORT brings it back.\nKEEP THESE sends this device's saves to the cloud. Your other device asks at its next start.", here->run_depth);
	else snprintf(out, n, "USE THOSE replaces your saves. UNDO IMPORT brings them back.\nKEEP THESE sends this device's saves to the cloud. Your other device asks at its next start.");
}

void saves_cloud_refused(BackupStatus status, const BackupInfo *cloud, char *out, size_t n) {
	if (status == BACKUP_NEWER_BUILD || status == BACKUP_NEWER_FORMAT || status == BACKUP_INCOMPATIBLE_RUN)
		snprintf(out, n, "Cloud: made by %s. Update first.", cloud->version[0] ? cloud->version : "a newer build");
	else if (status == BACKUP_NEEDS_BN5) snprintf(out, n, "Cloud: its run needs BN5 Team Colonel (USA).");
	else snprintf(out, n, "Cloud: its saves could not be taken in.");
}

void saves_cloud_from(const BackupInfo *cloud, char *out, size_t n) {
	char date[32], what[32];
	when(cloud->stamp, date, sizeof date);
	if (cloud->run_depth > 0) snprintf(what, sizeof what, "Layer %d", cloud->run_depth);
	else snprintf(what, sizeof what, "Saves");
	snprintf(out, n, "Cloud: %s from %s%s%s", what, cloud->system[0] ? cloud->system : "another device", date[0] ? ", " : "", date);
}

const char *saves_cloud_trouble(CloudTrouble trouble) {
	switch (trouble) {
	case CLOUD_FINE: return "";
	case CLOUD_UNREACHED: return "Not reached. Your saves are kept.";
	case CLOUD_BUSY: return "The server is busy. It tries again later.";
	case CLOUD_GONE:
	case CLOUD_DENIED: return "The cloud saves are gone. Turn on to start again.";
	case CLOUD_OFF: return "This server keeps no saves yet.";
	case CLOUD_CONFLICT: return "Another device's saves are newer.";
	case CLOUD_DAMAGED: return "What came could not be opened.";
	case CLOUD_TOO_BIG: return "These saves are too big for it.";
	default: return "Not enough memory.";
	}
}

void saves_cloud_titled(CloudTrouble trouble, char *out, size_t n) {
	const char *t = saves_cloud_trouble(trouble);
	snprintf(out, n, "Cloud: %c%s", t[0] >= 'A' && t[0] <= 'Z' ? t[0] - 'A' + 'a' : t[0], t[0] ? t + 1 : "");
}

const char *saves_cloud_syncing(void) { return "Cloud: syncing..."; }
const char *saves_cloud_deleted(void) { return "Cloud saves deleted. Your other devices stop syncing at their next start."; }
const char *saves_cloud_unlinked(void) { return "Unlinked. Your saves stay here."; }
const char *saves_cloud_waiting(void) { return "Waiting for the sync to end."; }

void saves_cloud_status(const CloudState *s, char *out, size_t n) {
	char date[32];
	out[0] = 0;
	if (!s->linked) return;
	if (s->doing == CLOUD_PULLING) snprintf(out, n, "Syncing...");
	else if (s->doing == CLOUD_PUSHING) snprintf(out, n, "Sending %d%%", s->size ? (int)((uint64_t)s->moved * 100 / s->size) : 0);
	else if (s->trouble != CLOUD_FINE) snprintf(out, n, "%s", saves_cloud_trouble(s->trouble));
	else if (s->synced_at) {
		when(s->synced_at, date, sizeof date);
		snprintf(out, n, "Synced %s", date);
	} else snprintf(out, n, "Not synced yet");
}

/* (home's pages and lines: docs/engine/saves-screens.md) */

const char *saves_page_name(int page) {
	static const char *const names[SAVES_PAGES] = { "CLOUD", "CODE", "FILE" };
	return page >= 0 && page < SAVES_PAGES ? names[page] : "";
}

const char *saves_word_undo(void) { return "UNDO IMPORT"; }

const char *saves_line_label(int action) {
	switch (action) {
	case SAVES_TURN_ON: return "Turn on";
	case SAVES_LINK: return "Link with a code";
	case SAVES_ADD: return "Add a device";
	case SAVES_UNLINK: return "Unlink this device";
	case SAVES_DELETE: return "Delete cloud saves";
	case SAVES_SEND: return "Send";
	case SAVES_RECEIVE: return "Receive";
	case SAVES_EXPORT: return "Export";
	case SAVES_IMPORT: return "Import";
	case SAVES_AUTO: return "Auto-copy";
	case SAVES_FOLDER: return "Folder";
	default: return saves_label(action);
	}
}

/* (a folder's last part: "Downloads" of ".../Downloads") */
static const char *last_part(const char *path) {
	const char *slash = strrchr(path, '/'), *back = strrchr(path, '\\');
	const char *at = slash > back ? slash : back;
	return at && at[1] ? at + 1 : path;
}

bool saves_line_value(int action, char *out, size_t n) {
	MirrorStatus m;
	if (action == SAVES_AUTO) snprintf(out, n, "%s", pick_saves_auto_enabled() ? "On" : "Off");
	else if (action == SAVES_FOLDER && (mirror_status(&m), m.place[0])) snprintf(out, n, "%s", last_part(m.place));
	else return false;
	return true;
}

/* (what a line does: one line of the box where a status follows, two on CODE) */
static const char *line_does(int action) {
	switch (action) {
	case SAVES_TURN_ON: return "Keeps these saves synced on every device you add.";
	case SAVES_LINK: return "Joins the cloud saves of your other device.";
	case SAVES_ADD: return "Shows a code for your other device.";
	case SAVES_UNLINK: return "Stops syncing here. Saves stay.";
	case SAVES_DELETE: return "Deletes the saves on the server.";
	case SAVES_SEND: return "Shows a code to type on your other device. It works once, for 15 minutes.";
	case SAVES_RECEIVE: return "Type the code your other device shows. Both saves are shown first.";
	case SAVES_EXPORT: return "Writes these saves to one .cwsave file.";
	case SAVES_IMPORT: return "Choose a .cwsave file to compare.";
	case SAVES_AUTO: return pick_saves_auto_enabled() ? "A copy a few seconds after each save." : "No copy is kept outside the game.";
	case SAVES_FOLDER: return "Where the copy goes.";
	default: return "";
	}
}

void saves_line_note(int action, char *out, size_t n) {
	char status[1200] = "";
	MirrorStatus m;
	mirror_status(&m);
	if (backup_blocked()) saves_status_line(&m, true, false, status, sizeof status);
	else if (SV.page == SAVES_PAGE_CLOUD) saves_cloud_line(status, sizeof status);
	else if (SV.page == SAVES_PAGE_FILE) saves_status_line(&m, false, pick_saves_capabilities() & SAVES_CAN_FOLDER, status, sizeof status);
	snprintf(out, n, "%s%s%s", line_does(action), status[0] ? "\n" : "", status);
}

void saves_ask_words(int action, char *out, size_t n) {
	BackupInfo before;
	switch (action) {
	case SAVES_UNDO:
		if (backup_undo_info(g_data_dir, &before) && before.run_depth > 0) snprintf(out, n, "Bring back your Layer %d run from before the import?", before.run_depth);
		else snprintf(out, n, "Bring back the saves from before the import?");
		break;
	case SAVES_TURN_ON: snprintf(out, n, "Turn on cloud saves? These saves go up now."); break;
	case SAVES_UNLINK: snprintf(out, n, "Stop syncing here? To join again takes a code."); break;
	case SAVES_DELETE: snprintf(out, n, "Delete the cloud saves? Each device keeps its own."); break;
	default: out[0] = 0; break;
	}
}

const char *saves_word_yes(void) { return "Yes"; }
const char *saves_word_no(void) { return "No"; }
const char *saves_word_device(void) { return "THIS DEVICE"; }
const char *saves_word_information(void) { return "INFORMATION"; }

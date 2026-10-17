/* backup.h. Every imported file is staged and flushed before anything is
 * replaced; the prior complete folder stays for Undo, and the one Undo
 * held before it is kept as a file. A failed install rolls its renames
 * back, and backup_recover puts right one a power cut stopped. */
#include "backup_internal.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "compat.h"
#include "online_trade_admit.h"
#include "online_trade_gate.h"
#include "save.h"
#include "save_format.h"
#include "save_queue.h"

#define REPLACED_KEPT 3   /* saves-replaced-1.cwsave to -3.cwsave */

static bool blocked;

bool backup_blocked(void) { return blocked; }

static bool is_dir(const char *path) {
	struct stat st;
	return !stat(path, &st) && S_ISDIR(st.st_mode);
}

static bool missing_profile(const char *path) {
	struct stat st;
	return stat(path, &st) != 0 && errno == ENOENT;
}

void backup_remove_dir(const char *dir) {
	DIR *d = opendir(dir);
	if (!d) return;
	char path[1400];
	for (struct dirent *e; (e = readdir(d));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		remove(path);
	}
	closedir(d);
	cw_rmdir(dir);
}

typedef struct { const char *stage; uint8_t *profile; size_t profile_n; bool ok; } Unpack;

static void stage_file(const char *name, const uint8_t *data, uint32_t size, void *user) {
	Unpack *u = user;
	if (!backup_saved_name(name) || !u->ok) return;
	char path[1100];
	snprintf(path, sizeof path, "%s/%s", u->stage, name + 9);
	uint8_t *copy = NULL;
	if (!strcmp(name, "savedata/profile.sav")) {
		uint32_t write_size = size < 48 ? 48 : size;
		copy = calloc(1, write_size);
		if (!copy) { u->ok = false; return; }
		memcpy(copy, data, size);
		if (write_size != size) {
			backup_put32(copy + 4, write_size - 12);
			backup_put32(copy + 8, backup_fnv(copy + 12, write_size - 12, 2166136261u));
		}
		size = write_size;
		backup_normalize_profile(copy, size, u->profile, u->profile_n);
		data = copy;
	}
	u->ok = backup_write_file(path, data, size);
	free(copy);
}

static void paths(const char *data_dir, char dir[1100], char old[1100], char undo[1100]) {
	snprintf(dir, 1100, "%s/savedata", data_dir);
	snprintf(old, 1100, "%s/savedata.old", data_dir);
	snprintf(undo, 1100, "%s/savedata.undo", data_dir);
}

/* (the hash of the saves an install is putting in, written before its
 * renames and removed once they are in and read back) */
static void pending_path(const char *data_dir, char out[1100]) { snprintf(out, 1100, "%s/savedata.pending", data_dir); }

static bool pending_hash(const char *data_dir, uint32_t *hash) {
	char path[1100];
	pending_path(data_dir, path);
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	bool ok = b && n == 4;
	if (ok) *hash = backup_get32(b);
	free(b);
	return b != NULL;
}

/* savedata/ holds the saves with `hash` */
static bool installed(const char *data_dir, uint32_t hash) {
	char dir[1100];
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	size_t n = 0;
	bool empty;
	BackupInfo info;
	uint8_t *b = backup_pack_dir(dir, &n, &empty, &info);
	free(b);
	return b && info.hash == hash;
}

/* the install's savedata/ taken out again, the one before it put back */
static bool roll_back(const char *data_dir) {
	char dir[1100], old[1100], undo[1100], bad[1100];
	paths(data_dir, dir, old, undo);
	snprintf(bad, sizeof bad, "%s/savedata.bad", data_dir);
	if (!is_dir(old)) return false;
	backup_remove_dir(bad);
	if (is_dir(dir) && !cw_rename(dir, bad)) return false;
	if (!cw_rename(old, dir)) return false;
	if (is_dir(undo)) cw_rename(undo, old);
	backup_remove_dir(bad);
	cw_fsync_dir(data_dir);
	return true;
}

static bool recover_swap(const char *dir, const char *old, const char *swap) {
	bool ok = true;
	if (is_dir(swap)) {
		if (!is_dir(dir) && is_dir(old) && !cw_rename(old, dir)) ok = false;
		else if (!is_dir(dir)) ok = cw_rename(swap, dir);
		else if (!is_dir(old)) ok = cw_rename(swap, old);
		else ok = false;
	}
	return ok;
}

/* If power stopped between the final renames, or the folder they put in
 * is not the one staged, the saves before the import come back. Called
 * before anything reads the saves at a start, and after a failed install. */
bool backup_recover(const char *data_dir) {
	save_queue_wait();
	online_trade_gate_start(data_dir);
	if (!online_trade_gate_transfer(data_dir)) { blocked = true; return false; }
	char dir[1100], old[1100], undo[1100], stage[1100], swap[1100], pending[1100];
	paths(data_dir, dir, old, undo);
	snprintf(stage, sizeof stage, "%s/savedata.import", data_dir);
	snprintf(swap, sizeof swap, "%s/savedata.swap", data_dir);
	pending_path(data_dir, pending);
	bool ok = recover_swap(dir, old, swap);
	uint32_t hash = 0;
	if (ok && pending_hash(data_dir, &hash) && !installed(data_dir, hash)) ok = roll_back(data_dir) || !is_dir(old);
	if (ok && !is_dir(dir) && is_dir(old)) ok = cw_rename(old, dir);
	if (ok && !is_dir(old) && is_dir(undo)) ok = cw_rename(undo, old);
	if (ok) {
		remove(pending);
		backup_remove_dir(undo);
		backup_remove_dir(stage);
		cw_fsync_dir(data_dir);
	}
	/* (the saves beside a missing savedata/: no fresh one over them) */
	blocked = !is_dir(dir) && (is_dir(old) || is_dir(swap) || is_dir(undo));
	if (ok && !blocked && !online_trade_gate_admit(data_dir)) blocked = true;
	return ok && !blocked;
}

/* The saves Undo held, about to be replaced: kept as the newest of the
 * last three saves-replaced-N.cwsave in the data folder */
static void keep_replaced(const char *data_dir, const char *old) {
	size_t n = 0;
	bool empty;
	BackupInfo info;
	uint8_t *b = backup_pack_dir(old, &n, &empty, &info);
	if (!b) return;
	char from[1100], to[1100];
	for (int i = REPLACED_KEPT; i > 1; --i) {
		snprintf(from, sizeof from, "%s/saves-replaced-%d.cwsave", data_dir, i - 1);
		snprintf(to, sizeof to, "%s/saves-replaced-%d.cwsave", data_dir, i);
		struct stat st;
		if (!stat(from, &st)) cw_rename(from, to);
	}
	snprintf(to, sizeof to, "%s/saves-replaced-1.cwsave", data_dir);
	backup_write_file(to, b, n);
	free(b);
}

static bool install(const char *data_dir, const char *stage, uint32_t hash) {
	char dir[1100], old[1100], undo[1100], pending[1100];
	paths(data_dir, dir, old, undo);
	pending_path(data_dir, pending);
	bool had_old = is_dir(old), had_dir = is_dir(dir);
	if (had_old) keep_replaced(data_dir, old);
	/* (the staged files are on the disk before any rename counts on them) */
	cw_fsync_dir(stage);
	uint8_t mark[4];
	backup_put32(mark, hash);
	if (!backup_write_file(pending, mark, sizeof mark)) return false;
	cw_fsync_dir(data_dir);
	if (had_old && !cw_rename(old, undo)) { remove(pending); return false; }
	if (had_dir && !cw_rename(dir, old)) {
		if (had_old) cw_rename(undo, old);
		remove(pending);
		return false;
	}
	if (!had_dir && cw_mkdir(old) != 0) {
		if (had_old) cw_rename(undo, old);
		remove(pending);
		return false;
	}
	if (!cw_rename(stage, dir)) {
		if (had_dir) cw_rename(old, dir);
		else backup_remove_dir(old);
		if (had_old) cw_rename(undo, old);
		remove(pending);
		return false;
	}
	cw_fsync_dir(data_dir);
	/* (read back: the folder in place is the one staged) */
	if (!installed(data_dir, hash)) return false;
	backup_remove_dir(undo);
	remove(pending);
	cw_fsync_dir(data_dir);
	return true;
}

BackupStatus backup_restore(const char *data_dir, const uint8_t *bytes, size_t n, bool bn5) {
	BackupInfo info;
	if (!backup_info(bytes, n, &info)) return BACKUP_DAMAGED;
	BackupStatus check = backup_check(&info, bn5);
	if (check != BACKUP_OK) return check;
	if (!backup_recover(data_dir)) return BACKUP_IO;
	char stage[1100], profile_path[1100];
	snprintf(stage, sizeof stage, "%s/savedata.import", data_dir);
	snprintf(profile_path, sizeof profile_path, "%s/savedata/profile.sav", data_dir);
	Unpack u = { .stage = stage, .ok = true };
	u.profile = backup_read_file(profile_path, &u.profile_n);
	if (!u.profile && !missing_profile(profile_path)) return BACKUP_IO;
	if (cw_mkdir(stage) != 0) { free(u.profile); return BACKUP_IO; }
	backup_walk(bytes, n, stage_file, &u);
	free(u.profile);
	BackupInfo staged;
	bool empty;
	size_t sn = 0;
	uint8_t *sb = u.ok ? backup_pack_dir(stage, &sn, &empty, &staged) : NULL;
	free(sb);
	bool ok = sb && online_trade_admit_check(data_dir, stage) && install(data_dir, stage, staged.hash) && online_trade_gate_admit(data_dir);
	if (!ok) {
		backup_remove_dir(stage);
		backup_recover(data_dir);
	}
	return ok ? BACKUP_OK : BACKUP_IO;
}

bool backup_unpack(const char *data_dir, const uint8_t *bytes, size_t n) { return backup_restore(data_dir, bytes, n, true) == BACKUP_OK; }

bool backup_can_undo(const char *data_dir) {
	if (!online_trade_gate_transfer(data_dir)) return false;
	save_queue_wait();
	char path[1100];
	snprintf(path, sizeof path, "%s/savedata.old", data_dir);
	return is_dir(path);
}

static bool keep_volumes(const char *dir, const char *old) {
	save_queue_wait();
	char from[1400], to[1400];
	snprintf(from, sizeof from, "%s/profile.sav", dir);
	snprintf(to, sizeof to, "%s/profile.sav", old);
	size_t a_n = 0, b_n = 0;
	uint8_t *a = backup_read_file(from, &a_n), *b = backup_read_file(to, &b_n);
	if ((!a && !missing_profile(from)) || (!b && !missing_profile(to))) {
		free(a);
		free(b);
		return false;
	}
	bool ok = true;
	if (!b) {
		b_n = 12 + sizeof(Profile);
		b = calloc(1, b_n);
		if (b) {
			backup_put32(b, PROFILE_MAGIC);
			backup_put32(b + 4, sizeof(Profile));
			backup_put32(b + 8, backup_fnv(b + 12, sizeof(Profile), 2166136261u));
		} else ok = false;
	}
	if (b && b_n >= 12 && b_n < 48 && backup_get32(b) == PROFILE_MAGIC && backup_get32(b + 4) == b_n - 12 &&
		backup_get32(b + 8) == backup_fnv(b + 12, b_n - 12, 2166136261u)) {
		uint8_t *grown = realloc(b, 48);
		if (grown) {
			b = grown;
			memset(b + b_n, 0, 48 - b_n);
			b_n = 48;
			backup_put32(b + 4, 36);
			backup_put32(b + 8, backup_fnv(b + 12, 36, 2166136261u));
		} else ok = false;
	}
	if (b && ok) { backup_normalize_profile(b, b_n, a, a_n); ok = backup_write_file(to, b, b_n); }
	free(a);
	free(b);
	return ok;
}

bool backup_undo(const char *data_dir) {
	save_queue_wait();
	if (!online_trade_gate_transfer(data_dir)) return false;
	char dir[1100], old[1100], swap[1100];
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	snprintf(old, sizeof old, "%s/savedata.old", data_dir);
	snprintf(swap, sizeof swap, "%s/savedata.swap", data_dir);
	if (!is_dir(dir) || !is_dir(old) || is_dir(swap)) return false;
	if (!online_trade_admit_check(data_dir, old)) return false;
	if (!keep_volumes(dir, old)) return false;
	cw_fsync_dir(old);
	if (!cw_rename(dir, swap)) return false;
	if (!cw_rename(old, dir)) { cw_rename(swap, dir); return false; }
	if (!cw_rename(swap, old)) { cw_rename(dir, old); cw_rename(swap, dir); return false; }
	cw_fsync_dir(data_dir);
	return online_trade_gate_admit(data_dir);
}

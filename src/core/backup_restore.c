/* Stage every imported file before replacing anything. The prior complete
 * folder remains available for Undo; a failed install rolls its renames back. */
#include "backup_internal.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "compat.h"
#include "save.h"
#include "save_format.h"

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

/* If power stopped between the two final renames, recover the saves that
 * were there before the import. Call before reading progress at startup. */
bool backup_recover(const char *data_dir) {
	char dir[1100], old[1100], undo[1100], stage[1100], swap[1100];
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	snprintf(old, sizeof old, "%s/savedata.old", data_dir);
	snprintf(undo, sizeof undo, "%s/savedata.undo", data_dir);
	snprintf(stage, sizeof stage, "%s/savedata.import", data_dir);
	snprintf(swap, sizeof swap, "%s/savedata.swap", data_dir);
	if (is_dir(swap)) {
		if (!is_dir(dir) && is_dir(old) && !cw_rename(old, dir)) return false;
		if (!is_dir(dir)) { if (!cw_rename(swap, dir)) return false; }
		else if (!is_dir(old)) { if (!cw_rename(swap, old)) return false; }
		else return false;
	}
	if (!is_dir(dir) && is_dir(old) && !cw_rename(old, dir)) return false;
	if (!is_dir(old) && is_dir(undo) && !cw_rename(undo, old)) return false;
	backup_remove_dir(undo);
	backup_remove_dir(stage);
	return true;
}

static bool install(const char *data_dir, const char *stage) {
	char dir[1100], old[1100], undo[1100];
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	snprintf(old, sizeof old, "%s/savedata.old", data_dir);
	snprintf(undo, sizeof undo, "%s/savedata.undo", data_dir);
	bool had_old = is_dir(old), had_dir = is_dir(dir);
	if (had_old && !cw_rename(old, undo)) return false;
	if (had_dir && !cw_rename(dir, old)) {
		if (had_old) cw_rename(undo, old);
		return false;
	}
	if (!had_dir && cw_mkdir(old) != 0) {
		if (had_old) cw_rename(undo, old);
		return false;
	}
	if (!cw_rename(stage, dir)) {
		if (had_dir) cw_rename(old, dir);
		else backup_remove_dir(old);
		if (had_old) cw_rename(undo, old);
		return false;
	}
	backup_remove_dir(undo);
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
	bool ok = u.ok && install(data_dir, stage);
	if (!ok) backup_remove_dir(stage);
	return ok ? BACKUP_OK : BACKUP_IO;
}

bool backup_unpack(const char *data_dir, const uint8_t *bytes, size_t n) { return backup_restore(data_dir, bytes, n, true) == BACKUP_OK; }

bool backup_can_undo(const char *data_dir) {
	char path[1100];
	snprintf(path, sizeof path, "%s/savedata.old", data_dir);
	return is_dir(path);
}

static bool keep_volumes(const char *dir, const char *old) {
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
	char dir[1100], old[1100], swap[1100];
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	snprintf(old, sizeof old, "%s/savedata.old", data_dir);
	snprintf(swap, sizeof swap, "%s/savedata.swap", data_dir);
	if (!is_dir(dir) || !is_dir(old) || is_dir(swap)) return false;
	if (!keep_volumes(dir, old)) return false;
	if (!cw_rename(dir, swap)) return false;
	if (!cw_rename(old, dir)) { cw_rename(swap, dir); return false; }
	if (!cw_rename(swap, old)) { cw_rename(dir, old); cw_rename(swap, dir); return false; }
	return true;
}

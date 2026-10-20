/* backup.h. The v1 carrier stays readable: "CWSAVE1\n", u64 stamp,
 * u32 count, then (u16 name length, name, u32 size, bytes), and its FNV-1a
 * checksum, all little-endian. V2 adds transfer.manifest among its files.
 * Only progress travels; preferences, consent, ROMs and boot states do not. */
#include "backup.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "backup_internal.h"
#include "compat.h"
#include "save_queue.h"

#define MAGIC "CWSAVE1\n"

uint32_t backup_fnv(const uint8_t *p, size_t n, uint32_t h) {
	for (size_t i = 0; i < n; ++i) h = (h ^ p[i]) * 16777619u;
	return h;
}

uint32_t backup_get32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
void backup_put32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (8 * i)); }

uint8_t *backup_read_file(const char *path, size_t *n) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	uint8_t *buf = NULL;
	long size = fseek(f, 0, SEEK_END) == 0 ? ftell(f) : -1;
	if (size >= 0 && (unsigned long)size <= BACKUP_MAX && fseek(f, 0, SEEK_SET) == 0) {
		buf = malloc(size ? (size_t)size : 1);
		if (buf && fread(buf, 1, (size_t)size, f) != (size_t)size) { free(buf); buf = NULL; }
	}
	fclose(f);
	if (buf) *n = (size_t)size;
	return buf;
}

bool backup_write_file(const char *path, const uint8_t *bytes, size_t n) {
	char tmp[1400];
	if (snprintf(tmp, sizeof tmp, "%s.tmp", path) >= (int)sizeof tmp) return false;
	FILE *f = fopen(tmp, "wb");
	if (!f) return false;
	bool ok = fwrite(bytes, 1, n, f) == n;
	ok = cw_fsync_file(f) && ok;
	ok = fclose(f) == 0 && ok;
	if (ok) ok = cw_rename(tmp, path);
	if (!ok) remove(tmp);
	return ok;
}

const char *const save_progress_names[SAVE_PROGRESS_NAMES] = {
	"profile.sav", "rivals.sav", "battle.sav", "run.sav", "run.state", "run.make", "run.area",
	"run.seen", "run.folder", "run.act", "run.dark", "run.souls", "run.board", "run.tally"
};

/* (the normal slot's files, the week's net's beside them after "weekly-",
 * and weekly.sav, its record: weekly.h, docs/SAVES.md) */
bool backup_progress_name(const char *name) {
	if (strncmp(name, "savedata/", 9)) return false;
	const char *file = name + 9;
	if (!strcmp(file, "weekly.sav")) return true;
	if (!strncmp(file, "weekly-", 7)) file += 7;
	for (size_t i = 0; i < SAVE_PROGRESS_NAMES; ++i)
		if (!strcmp(file, save_progress_names[i])) return true;
	return false;
}

bool backup_saved_name(const char *name) { return backup_progress_name(name) || !strcmp(name, BACKUP_MANIFEST); }

static bool legacy_setting(const char *name) {
	return !strcmp(name, "settings.ini") || !strcmp(name, "keys.ini") || !strcmp(name, "pad.ini") || !strcmp(name, "touch.ini");
}

/* Validate all boundaries and names before handing any file to a writer. */
bool backup_walk(const uint8_t *b, size_t n, BackupEach each, void *user) {
	if (!b || n < BACKUP_HEAD + 4 || n > BACKUP_MAX || memcmp(b, MAGIC, 8) ||
		backup_get32(b + n - 4) != backup_fnv(b, n - 4, 2166136261u)) return false;
	uint32_t count = backup_get32(b + 16);
	if (!count || count > BACKUP_FILES_MAX) return false;
	char seen[BACKUP_FILES_MAX][200];
	size_t at = BACKUP_HEAD;
	for (uint32_t i = 0; i < count; ++i) {
		if (at + 2 > n - 4) return false;
		size_t len = (size_t)b[at] | (size_t)b[at + 1] << 8;
		if (!len || len >= sizeof seen[0] || at + 6 + len > n - 4 || memchr(b + at + 2, 0, len)) return false;
		uint32_t size = backup_get32(b + at + 2 + len);
		if (size > BACKUP_FILE_MAX || size > n - 4 - (at + 6 + len)) return false;
		memcpy(seen[i], b + at + 2, len);
		seen[i][len] = 0;
		if (!backup_saved_name(seen[i]) && !legacy_setting(seen[i])) return false;
		for (uint32_t j = 0; j < i; ++j) if (!strcmp(seen[i], seen[j])) return false;
		if (each) each(seen[i], b + at + 6 + len, size, user);
		at += 6 + len + size;
	}
	return at == n - 4;
}

typedef struct { uint8_t *buf; size_t n, cap; uint64_t latest; uint32_t count; bool ok; } Pack;

static bool grow(Pack *p, size_t more) {
	if (more > BACKUP_MAX - p->n) return false;
	if (p->n + more <= p->cap) return true;
	size_t cap = p->cap ? p->cap : 1u << 16;
	while (cap < p->n + more) cap *= 2;
	uint8_t *b = realloc(p->buf, cap);
	if (!b) return false;
	p->buf = b;
	p->cap = cap;
	return true;
}

static void add_bytes(Pack *p, const char *name, const uint8_t *data, size_t size) {
	size_t len = strlen(name);
	if (size > BACKUP_FILE_MAX || p->count >= BACKUP_FILES_MAX || !grow(p, 6 + len + size)) { p->ok = false; return; }
	p->buf[p->n] = (uint8_t)len;
	p->buf[p->n + 1] = (uint8_t)(len >> 8);
	memcpy(p->buf + p->n + 2, name, len);
	backup_put32(p->buf + p->n + 2 + len, (uint32_t)size);
	memcpy(p->buf + p->n + 6 + len, data, size);
	p->n += 6 + len + size;
	++p->count;
}

static void add_file(Pack *p, const char *dir, const char *name) {
	char path[1400];
	snprintf(path, sizeof path, "%s/%.190s", dir, name + 9);
	size_t size = 0;
	uint8_t *data = backup_read_file(path, &size);
	if (!data) { p->ok = false; return; }
	struct stat st;
	if (!stat(path, &st) && st.st_mtime > 0 && (uint64_t)st.st_mtime > p->latest) p->latest = (uint64_t)st.st_mtime;
	if (!strcmp(name, "savedata/profile.sav")) backup_normalize_profile(data, size, NULL, 0);
	add_bytes(p, name, data, size);
	free(data);
}

static int by_name(const void *a, const void *b) { return strcmp(a, b); }

static bool add_progress(Pack *p, const char *dir) {
	char names[BACKUP_FILES_MAX][200];
	DIR *d = opendir(dir);
	if (!d) return false;
	int k = 0;
	for (struct dirent *e; (e = readdir(d));) {
		char name[300];
		snprintf(name, sizeof name, "savedata/%s", e->d_name);
		if (!backup_progress_name(name)) continue;
		if (k >= BACKUP_FILES_MAX) { p->ok = false; break; }
		snprintf(names[k++], sizeof names[0], "%.199s", name);
	}
	closedir(d);
	qsort(names, (size_t)k, sizeof names[0], by_name);
	for (int i = 0; i < k; ++i) add_file(p, dir, names[i]);
	return p->ok;
}

static bool finish(Pack *p, uint64_t stamp) {
	if (!p->ok || !grow(p, 4)) return false;
	for (int i = 0; i < 8; ++i) p->buf[8 + i] = (uint8_t)(stamp >> (8 * i));
	backup_put32(p->buf + 16, p->count);
	backup_put32(p->buf + p->n, backup_fnv(p->buf, p->n, 2166136261u));
	return true;
}

uint8_t *backup_pack(const char *data_dir, uint64_t stamp, size_t *n) {
	save_queue_wait();   /* (the files a checkpoint queued, all written) */
	Pack p = { .ok = true };
	if (!grow(&p, BACKUP_HEAD)) return NULL;
	memcpy(p.buf, MAGIC, 8);
	p.n = BACKUP_HEAD;
	BackupInfo info;
	char manifest[BACKUP_MANIFEST_MAX], dir[1100];
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	bool ok = add_progress(&p, dir) && finish(&p, stamp) && backup_info(p.buf, p.n + 4, &info) &&
		info.has_profile && info.status == BACKUP_OK && backup_manifest_update(data_dir, stamp ? stamp : p.latest, &info, manifest, sizeof manifest);
	if (ok) {
		add_bytes(&p, BACKUP_MANIFEST, (const uint8_t *)manifest, strlen(manifest));
		ok = finish(&p, info.stamp);
	}
	if (!ok) { free(p.buf); return NULL; }
	*n = p.n + 4;
	return p.buf;
}

/* A folder of saves as a .cwsave's bytes, read only: its manifest goes
 * with them where it describes these very files (a later save outdates
 * it). NULL with `empty` where the folder holds no progress at all. */
uint8_t *backup_pack_dir(const char *dir, size_t *n, bool *empty, BackupInfo *info) {
	memset(info, 0, sizeof *info);
	info->status = BACKUP_IO;
	*empty = false;
	Pack p = { .ok = true };
	if (!grow(&p, BACKUP_HEAD)) return NULL;
	memcpy(p.buf, MAGIC, 8);
	p.n = BACKUP_HEAD;
	char path[1200];
	snprintf(path, sizeof path, "%s/transfer.manifest", dir);
	size_t mn = 0;
	uint8_t *manifest = NULL;
	BackupInfo recorded;
	bool ok = false;
	if (!add_progress(&p, dir)) goto done;
	manifest = backup_read_file(path, &mn);
	if (!manifest) {
		struct stat st;
		if (stat(path, &st) == 0 || errno != ENOENT) goto done;
	} else if (!backup_manifest_read(manifest, mn, &recorded)) { info->status = BACKUP_DAMAGED; goto done; }
	if (!p.count) {
		*empty = !manifest;
		info->format = 1;
		info->status = manifest ? BACKUP_DAMAGED : BACKUP_OK;
		goto done;
	}
	if (!finish(&p, p.latest) || !backup_info(p.buf, p.n + 4, info)) goto done;
	if (manifest && recorded.hash == info->hash) {
		add_bytes(&p, BACKUP_MANIFEST, manifest, mn);
		if (!finish(&p, recorded.stamp) || !backup_info(p.buf, p.n + 4, info)) goto done;
	}
	ok = true;
done:
	free(manifest);
	if (!ok) {
		if (info->status == BACKUP_OK && !*empty) info->status = BACKUP_IO;
		free(p.buf);
		return NULL;
	}
	*n = p.n + 4;
	return p.buf;
}

static bool dir_info(const char *data_dir, const char *name, BackupInfo *info, bool empty_ok) {
	save_queue_wait();
	char dir[1100];
	snprintf(dir, sizeof dir, "%s/%s", data_dir, name);
	size_t n = 0;
	bool empty;
	uint8_t *b = backup_pack_dir(dir, &n, &empty, info);
	free(b);
	if (b) return true;
	return empty && empty_ok;
}

bool backup_undo_info(const char *data_dir, BackupInfo *info) { return dir_info(data_dir, "savedata.old", info, true); }

/* (read only: nothing is written, the manifest neither; saves not yet
 * exported are this system's) */
bool backup_local_info(const char *data_dir, BackupInfo *info) {
	bool ok = dir_info(data_dir, "savedata", info, false) && info->has_profile;
	if (!ok) {
		/* (its status kept: BACKUP_OK for no saves, else saves that cannot
		 * be read) */
		BackupStatus status = info->status;
		memset(info, 0, sizeof *info);
		info->status = status;
	} else if (!info->system[0]) snprintf(info->system, sizeof info->system, "%s", backup_system_name());
	return ok;
}

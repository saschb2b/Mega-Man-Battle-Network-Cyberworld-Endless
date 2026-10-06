/* backup.h. A .cwsave, all little-endian:
 *
 *   "CWSAVE1\n"            8 bytes, the format
 *   u64 stamp              when it was made, seconds since 1970
 *   u32 count              then, count times:
 *     u16 length, name     its place under the data folder ("savedata/run.sav", "keys.ini")
 *     u32 size, bytes
 *   u32 FNV-1a             of every byte before it
 *
 * A file's name is its place under the data folder; only savedata/'s
 * files and the four settings files are written back. Its profile and run
 * are read for what a player is told of it (runs, the best layer, a run's
 * layer) by their blobs (save.c's: magic, size, checksum, the struct):
 * the profile's runs and best layer are its first two ints, a run's
 * active flag and layer at 0 and 8 in every run format since the second
 * ("CWE2"). */
#include "backup.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"

#define MAGIC "CWSAVE1\n"
#define HEAD 20               /* the magic, the stamp, the count */
#define FILE_MAX (4u << 20)   /* a save file packed (a run's state: 420 KB) */
#define FILES_MAX 64
#define PROFILE_BLOB 0x43575032u   /* "CWP2" (save.c's PROFILE_MAGIC) */
#define RUN_BLOB 0x43574500u       /* "CWE" and the format's digit (save.c's RUN_MAGIC) */

/* the settings beside savedata/ that travel with the saves */
static const char *const settings[] = { "settings.ini", "keys.ini", "pad.ini", "touch.ini" };

static uint32_t fnv(const uint8_t *p, size_t n, uint32_t h) {
	for (size_t i = 0; i < n; ++i) h = (h ^ p[i]) * 16777619u;
	return h;
}

static uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static void put32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (8 * i)); }

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
	char tmp[1100];
	snprintf(tmp, sizeof tmp, "%s.tmp", path);
	FILE *f = fopen(tmp, "wb");
	if (!f) return false;
	bool ok = fwrite(bytes, 1, n, f) == n;
	ok = fclose(f) == 0 && ok;
	if (ok) ok = cw_rename(tmp, path);
	if (!ok) remove(tmp);
	return ok;
}

/* ---- packing ---- */

typedef struct {
	uint8_t *buf;
	size_t n, cap;
	uint32_t count;
} Pack;

static bool grow(Pack *p, size_t more) {
	if (p->n + more <= p->cap) return true;
	size_t cap = p->cap ? p->cap : 1 << 16;
	while (cap < p->n + more) cap *= 2;
	uint8_t *b = realloc(p->buf, cap);
	if (!b) return false;
	p->buf = b;
	p->cap = cap;
	return true;
}

/* one file under data_dir, as `name` */
static void add(Pack *p, const char *data_dir, const char *name) {
	char path[1100];
	snprintf(path, sizeof path, "%s/%s", data_dir, name);
	size_t size = 0, len = strlen(name);
	uint8_t *data = backup_read_file(path, &size);
	if (!data) return;
	if (size <= FILE_MAX && len < 200 && p->count < FILES_MAX && grow(p, 6 + len + size)) {
		p->buf[p->n] = (uint8_t)len;
		p->buf[p->n + 1] = (uint8_t)(len >> 8);
		memcpy(p->buf + p->n + 2, name, len);
		put32(p->buf + p->n + 2 + len, (uint32_t)size);
		memcpy(p->buf + p->n + 6 + len, data, size);
		p->n += 6 + len + size;
		++p->count;
	}
	free(data);
}

static int by_name(const void *a, const void *b) { return strcmp(a, b); }

static bool packed_name(const char *name) {
	size_t n = strlen(name);
	return name[0] != '.' && n < 100 && (n < 4 || strcmp(name + n - 4, ".tmp"));
}

uint8_t *backup_pack(const char *data_dir, uint64_t stamp, size_t *n) {
	char dir[1100];
	snprintf(dir, sizeof dir, "%s/savedata/profile.sav", data_dir);
	FILE *f = fopen(dir, "rb");
	if (!f) return NULL;   /* (no profile: nothing played to keep) */
	fclose(f);
	Pack p = { 0 };
	if (!grow(&p, HEAD)) return NULL;
	memcpy(p.buf, MAGIC, 8);
	for (int i = 0; i < 8; ++i) p.buf[8 + i] = (uint8_t)(stamp >> (8 * i));
	p.n = HEAD;
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	DIR *d = opendir(dir);
	if (d) {
		/* (sorted, so the same saves pack to the same bytes) */
		char names[FILES_MAX][270];
		int k = 0;
		for (struct dirent *e; (e = readdir(d)) && k < FILES_MAX;)
			if (packed_name(e->d_name)) snprintf(names[k++], sizeof names[0], "savedata/%s", e->d_name);
		closedir(d);
		qsort(names, (size_t)k, sizeof names[0], by_name);
		for (int i = 0; i < k; ++i) add(&p, data_dir, names[i]);
	}
	for (size_t i = 0; i < sizeof settings / sizeof *settings; ++i) add(&p, data_dir, settings[i]);
	if (!grow(&p, 4)) { free(p.buf); return NULL; }
	put32(p.buf + 16, p.count);
	put32(p.buf + p.n, fnv(p.buf, p.n, 2166136261u));
	p.n += 4;
	*n = p.n;
	return p.buf;
}

/* ---- reading ---- */

/* Each file in `b` (checked whole first) handed to `each`; false for bytes
 * that are no .cwsave */
typedef void (*EachFile)(const char *name, const uint8_t *data, uint32_t size, void *user);

static bool walk(const uint8_t *b, size_t n, EachFile each, void *user) {
	if (!b || n < HEAD + 4 || n > BACKUP_MAX || memcmp(b, MAGIC, 8) || get32(b + n - 4) != fnv(b, n - 4, 2166136261u)) return false;
	uint32_t count = get32(b + 16);
	size_t at = HEAD;
	for (uint32_t i = 0; i < count; ++i) {
		if (at + 2 > n - 4) return false;
		size_t len = (size_t)b[at] | (size_t)b[at + 1] << 8;
		if (!len || len >= 200 || at + 6 + len > n - 4) return false;
		uint32_t size = get32(b + at + 2 + len);
		if (size > n - 4 - (at + 6 + len)) return false;
		char name[200];
		memcpy(name, b + at + 2, len);
		name[len] = 0;
		if (each) each(name, b + at + 6 + len, size, user);
		at += 6 + len + size;
	}
	return at == n - 4;
}

/* a save blob's struct (save.c's: magic, size, checksum, then it) */
static const uint8_t *blob(const uint8_t *data, uint32_t size, uint32_t magic, uint32_t mask, uint32_t *len) {
	if (size < 12 || (get32(data) & mask) != magic) return NULL;
	uint32_t n = get32(data + 4), sum = 2166136261u;
	if (n > size - 12) return NULL;
	for (uint32_t i = 0; i < n; ++i) sum = (sum ^ data[12 + i]) * 16777619u;
	if (sum != get32(data + 8)) return NULL;
	*len = n;
	return data + 12;
}

static void read_saves(const char *name, const uint8_t *data, uint32_t size, void *user) {
	BackupInfo *info = user;
	uint32_t len;
	const uint8_t *p;
	++info->files;
	if (!strcmp(name, "savedata/profile.sav") && (p = blob(data, size, PROFILE_BLOB, 0xFFFFFFFFu, &len)) && len >= 8) {
		info->runs = (int)get32(p);
		info->best = (int)get32(p + 4);
	}
	/* (a run's format digit "2" on: active at 0, the layer at 8) */
	if (!strcmp(name, "savedata/run.sav") && (p = blob(data, size, RUN_BLOB, 0xFFFFFF00u, &len)) && len >= 12 && data[0] >= '2' && p[0])
		info->run_depth = (int)get32(p + 8);
}

bool backup_info(const uint8_t *bytes, size_t n, BackupInfo *info) {
	memset(info, 0, sizeof *info);
	if (!walk(bytes, n, read_saves, info)) return false;
	info->stamp = 0;
	for (int i = 0; i < 8; ++i) info->stamp |= (uint64_t)bytes[8 + i] << (8 * i);
	/* (its files alone: the same saves packed at another time hash the same) */
	info->hash = fnv(bytes + 16, n - 20, 2166136261u);
	return true;
}

bool backup_local_info(const char *data_dir, BackupInfo *info) {
	size_t n = 0;
	uint8_t *b = backup_pack(data_dir, 0, &n);
	bool ok = b && backup_info(b, n, info);
	free(b);
	if (!ok) memset(info, 0, sizeof *info);
	return ok;
}

/* ---- unpacking ---- */

/* a name a .cwsave may write: a file in savedata/, or a settings file */
static bool writable(const char *name) {
	if (!strncmp(name, "savedata/", 9)) return packed_name(name + 9) && !strchr(name + 9, '/') && !strchr(name + 9, '\\');
	for (size_t i = 0; i < sizeof settings / sizeof *settings; ++i)
		if (!strcmp(name, settings[i])) return true;
	return false;
}

/* every file of `dir`, then the folder itself, gone */
static void remove_dir(const char *dir) {
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

typedef struct {
	const char *data_dir;
	bool ok;
} Unpack;

static void write_one(const char *name, const uint8_t *data, uint32_t size, void *user) {
	Unpack *u = user;
	if (!writable(name)) return;
	char path[1100];
	snprintf(path, sizeof path, "%s/%s", u->data_dir, name);
	if (!backup_write_file(path, data, size)) u->ok = false;
}

bool backup_unpack(const char *data_dir, const uint8_t *bytes, size_t n) {
	if (!walk(bytes, n, NULL, NULL)) return false;
	char dir[1100], old[1100];
	snprintf(dir, sizeof dir, "%s/savedata", data_dir);
	snprintf(old, sizeof old, "%s/savedata.old", data_dir);
	/* (what was here, kept aside once: the last restore's undone by hand) */
	remove_dir(old);
	cw_rename(dir, old);
	cw_mkdir(dir);
	Unpack u = { data_dir, true };
	walk(bytes, n, write_one, &u);
	return u.ok;
}

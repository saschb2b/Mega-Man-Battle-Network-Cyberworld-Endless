#include "online_trade_files.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sha256.h"

#if !defined(_WIN32) && !defined(__3DS__) && !defined(__EMSCRIPTEN__)
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILES_MAX 64
#define NAME_MAXIMUM 64
typedef struct { char name[FILES_MAX][NAME_MAXIMUM]; unsigned count; } FileList;

#ifdef CW_TRADE_FILES_TEST
static bool (*test_hook)(const char *point);
void trade_files_test_hook(bool (*hook)(const char *point)) { test_hook = hook; }
static bool point(const char *name) { return !test_hook || test_hook(name); }
#else
static bool point(const char *name) { (void)name; return true; }
#endif

bool trade_files_supported(void) { return true; }

bool trade_files_path(char out[TRADE_PATH_MAX], const char *dir, const char *name) {
	if (!dir || !*dir || !name || !*name || !strcmp(name, ".") || !strcmp(name, "..") || strchr(name, '/') || strchr(name, '\\')) return false;
	int n = snprintf(out, TRADE_PATH_MAX, "%s/%s", dir, name);
	return n > 0 && n < TRADE_PATH_MAX;
}

int trade_files_kind(const char *path) {
	struct stat st;
	if (lstat(path, &st)) return errno == ENOENT ? 0 : -1;
	return S_ISREG(st.st_mode) ? 1 : S_ISDIR(st.st_mode) ? 2 : -1;
}

int trade_files_lock(const char *root) {
	char path[TRADE_PATH_MAX];
	if (!trade_files_path(path, root, "trade.lock")) return -1;
	int fd = open(path, O_CREAT | O_RDWR | O_NOFOLLOW, 0600);
	struct stat st;
	if (fd >= 0 && (fstat(fd, &st) || !S_ISREG(st.st_mode) || flock(fd, LOCK_EX | LOCK_NB))) { close(fd); return -1; }
	return fd;
}

void trade_files_unlock(int fd) { if (fd >= 0) close(fd); }

bool trade_files_sync(const char *dir) {
	if (!point("before-directory-flush")) return false;
	int fd = open(dir, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
	if (fd < 0) return false;
	bool ok = fsync(fd) == 0;
	if (close(fd)) ok = false;
	return ok && point("after-directory-flush");
}

bool trade_files_remove(const char *path) {
	int kind = trade_files_kind(path);
	if (!kind) return true;
	return kind == 1 && point("before-remove") && !unlink(path) && point("after-remove");
}

bool trade_files_mkdir(const char *path) {
	return point("before-mkdir") && !mkdir(path, 0700) && point("after-mkdir");
}

bool trade_files_move(const char *from, const char *to) {
	/* Directory installs require an absent destination. In particular,
	 * never use a platform's remove-then-rename replacement fallback. */
	return trade_files_kind(to) == 0 && point("before-move") && !rename(from, to) && point("after-move");
}

static bool write_bytes(int fd, const uint8_t *bytes, size_t n) {
	if (!point("before-write")) return false;
	while (n) {
		ssize_t wrote = write(fd, bytes, n);
		if (wrote < 0 && errno == EINTR) continue;
		if (wrote <= 0) return false;
		bytes += wrote; n -= (size_t)wrote;
	}
	return point("after-write");
}

bool trade_files_write(const char *path, const uint8_t *data, size_t n) {
	if (!data || !n || n > TRADE_FILE_LIMIT) return false;
	char tmp[TRADE_PATH_MAX];
	int len = snprintf(tmp, sizeof tmp, "%s.tmp", path);
	if (len < 0 || len >= (int)sizeof tmp || !trade_files_remove(tmp)) return false;
	int fd = open(tmp, O_CREAT | O_EXCL | O_WRONLY | O_NOFOLLOW, 0600);
	if (fd < 0) return false;
	bool ok = write_bytes(fd, data, n);
	if (ok) ok = point("before-file-flush") && !fsync(fd) && point("after-file-flush");
	if (close(fd)) ok = false;
	if (ok) ok = point("before-file-rename") && !rename(tmp, path) && point("after-file-rename");
	/* The caller flushes the containing directory before treating the
	 * new name as durable. Failed writes remain for recovery to inspect. */
	if (!ok) return false;
	size_t read_n;
	uint8_t *readback = trade_files_read(path, &read_n);
	ok = readback && read_n == n && !memcmp(data, readback, n);
	free(readback);
	return ok;
}

uint8_t *trade_files_read(const char *path, size_t *n) {
	*n = 0;
	if (!point("before-read")) return NULL;
	int fd = open(path, O_RDONLY | O_NOFOLLOW);
	if (fd < 0) return NULL;
	struct stat st;
	bool ok = !fstat(fd, &st) && S_ISREG(st.st_mode) && st.st_size > 0 && st.st_size <= TRADE_FILE_LIMIT;
	size_t size = ok ? (size_t)st.st_size : 0;
	uint8_t *bytes = size ? malloc(size) : NULL;
	size_t got = 0;
	while (bytes && got < size) {
		ssize_t take = read(fd, bytes + got, size - got);
		if (take < 0 && errno == EINTR) continue;
		if (take <= 0) break;
		got += (size_t)take;
	}
	uint8_t extra;
	ok = bytes && got == size && read(fd, &extra, 1) == 0;
	if (close(fd)) ok = false;
	if (!ok || !point("after-read")) { free(bytes); return NULL; }
	*n = size;
	return bytes;
}

static int by_name(const void *a, const void *b) { return strcmp(a, b); }

static bool saved_name(const char *name) {
	static const char *const allowed[] = {
		"profile.sav", "rivals.sav", "battle.sav", "run.sav", "run.state", "run.make", "run.area",
		"run.seen", "run.folder", "run.act", "run.dark", "run.souls", "run.board", "run.trade", "transfer.manifest"
	};
	for (unsigned i = 0; i < sizeof allowed / sizeof *allowed; ++i)
		if (!strcmp(name, allowed[i])) return true;
	return false;
}

static bool file_list(const char *dir, FileList *list) {
	memset(list, 0, sizeof *list);
	if (trade_files_kind(dir) != 2) return false;
	DIR *stream = opendir(dir);
	if (!stream) return false;
	bool ok = true;
	for (;;) {
		errno = 0;
		struct dirent *entry = readdir(stream);
		if (!entry) { if (errno) ok = false; break; }
		const char *name = entry->d_name;
		if (!strcmp(name, ".") || !strcmp(name, "..")) continue;
		size_t n = strlen(name);
		char path[TRADE_PATH_MAX];
		if (!n || n >= NAME_MAXIMUM || !saved_name(name) ||
			list->count == FILES_MAX || !trade_files_path(path, dir, name) || trade_files_kind(path) != 1) { ok = false; break; }
		memcpy(list->name[list->count++], name, n + 1);
	}
	if (closedir(stream)) ok = false;
	qsort(list->name, list->count, sizeof list->name[0], by_name);
	return ok;
}

bool trade_files_hash(const char *dir, bool content, uint8_t out[32]) {
	FileList list;
	if (!file_list(dir, &list) || !list.count) return false;
	CwSha256 hash;
	cw_sha256_init(&hash);
	static const char domain[] = "Cyberworld checkpoint 1";
	cw_sha256_update(&hash, domain, sizeof domain);
	for (unsigned i = 0; i < list.count; ++i) {
		const char *name = list.name[i];
		if (content && (!strcmp(name, "run.trade") || !strcmp(name, "transfer.manifest"))) continue;
		char path[TRADE_PATH_MAX];
		if (!trade_files_path(path, dir, name)) return false;
		size_t n;
		uint8_t *bytes = trade_files_read(path, &n);
		if (!bytes) return false;
		uint8_t length[4];
		for (unsigned j = 0; j < 4; ++j) length[j] = (uint8_t)(n >> (24 - j * 8));
		cw_sha256_update(&hash, name, strlen(name) + 1);
		cw_sha256_update(&hash, length, sizeof length);
		cw_sha256_update(&hash, bytes, n);
		free(bytes);
	}
	cw_sha256_final(&hash, out);
	return true;
}

bool trade_files_copy(const char *from, const char *to) {
	FileList list;
	if (!file_list(from, &list) || !list.count) return false;
	for (unsigned i = 0; i < list.count; ++i) {
		char source[TRADE_PATH_MAX], target[TRADE_PATH_MAX];
		if (!trade_files_path(source, from, list.name[i]) || !trade_files_path(target, to, list.name[i])) return false;
		size_t n;
		uint8_t *bytes = trade_files_read(source, &n);
		bool ok = bytes && trade_files_write(target, bytes, n);
		free(bytes);
		if (!ok) return false;
	}
	return trade_files_sync(to);
}

static bool sync_file(const char *path) {
	int fd = open(path, O_RDONLY | O_NOFOLLOW);
	if (fd < 0) return false;
	struct stat st;
	bool ok = !fstat(fd, &st) && S_ISREG(st.st_mode) && point("before-file-flush") &&
		!fsync(fd) && point("after-file-flush");
	if (close(fd)) ok = false;
	return ok;
}

bool trade_files_certify(const char *dir) {
	FileList list;
	if (!file_list(dir, &list) || !list.count) return false;
	for (unsigned i = 0; i < list.count; ++i) {
		char path[TRADE_PATH_MAX];
		if (!trade_files_path(path, dir, list.name[i]) || !sync_file(path)) return false;
	}
	return trade_files_sync(dir);
}

static bool recovery_name(const char *name) {
	if (saved_name(name)) return true;
	size_t n = strlen(name);
	if (n < 5 || n >= NAME_MAXIMUM || strcmp(name + n - 4, ".tmp")) return false;
	char plain[NAME_MAXIMUM];
	memcpy(plain, name, n - 4); plain[n - 4] = 0;
	return saved_name(plain);
}

bool trade_files_remove_dir(const char *path) {
	if (!trade_files_kind(path)) return true;
	/* Recovery may remove a half-written .tmp, but never follows a link
	 * or removes nested folders, even in this private staging directory. */
	if (trade_files_kind(path) != 2) return false;
	DIR *stream = opendir(path);
	if (!stream) return false;
	bool ok = true;
	for (struct dirent *e; (e = readdir(stream));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		char file[TRADE_PATH_MAX];
		if (!recovery_name(e->d_name) || !trade_files_path(file, path, e->d_name) || !trade_files_remove(file)) { ok = false; break; }
	}
	if (closedir(stream)) ok = false;
	return ok && point("before-rmdir") && !rmdir(path) && point("after-rmdir");
}

#else
/* These platforms need their own acknowledged persistence/rename
 * backend before they may advertise the safe trade capability. */
bool trade_files_supported(void) { return false; }
bool trade_files_path(char out[TRADE_PATH_MAX], const char *dir, const char *name) { return false; }
int trade_files_lock(const char *root) { return -1; }
void trade_files_unlock(int fd) {}
bool trade_files_sync(const char *dir) { return false; }
bool trade_files_write(const char *path, const uint8_t *data, size_t n) { return false; }
uint8_t *trade_files_read(const char *path, size_t *n) { *n = 0; return NULL; }
int trade_files_kind(const char *path) { return -1; }
bool trade_files_mkdir(const char *path) { return false; }
bool trade_files_move(const char *from, const char *to) { return false; }
bool trade_files_remove(const char *path) { return false; }
bool trade_files_remove_dir(const char *path) { return false; }
bool trade_files_hash(const char *dir, bool content, uint8_t out[32]) { return false; }
bool trade_files_copy(const char *from, const char *to) { return false; }
bool trade_files_certify(const char *dir) { return false; }
#endif

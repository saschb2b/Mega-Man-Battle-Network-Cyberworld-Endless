/* The automatic copy (src/launcher/mirror.c) where the transfer place is
 * the data folder itself (pick_none.c: PortMaster, the 3DS), with saves of
 * invented bytes: it writes only where the place's file is absent, its
 * own, or answered; holds over another device's; keeps an answer across a
 * restart (this program run again); and its look writes nothing. */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "backup.h"
#include "backup_internal.h"
#include "compat.h"
#include "game.h"
#include "mirror.h"
#include "pick.h"
#include "platform.h"
#include "save_format.h"
#include "save.h"

char g_data_dir[512];
static Uint32 ticks = 10000;
static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

Uint32 SDL_GetTicks(void) { return ticks; }
void platform_persist(void) { mirror_note(); }

static void remove_tree(const char *path) {
	DIR *dir = opendir(path);
	if (!dir) { remove(path); return; }
	for (struct dirent *e; (e = readdir(dir));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		char child[1400];
		snprintf(child, sizeof child, "%s/%s", path, e->d_name);
		remove_tree(child);
	}
	closedir(dir);
	cw_rmdir(path);
}

static bool blob(const char *path, uint32_t magic, const void *value, uint32_t size) {
	uint8_t *bytes = malloc(12 + size);
	if (!bytes) return false;
	backup_put32(bytes, magic);
	backup_put32(bytes + 4, size);
	backup_put32(bytes + 8, backup_fnv(value, size, 2166136261u));
	memcpy(bytes + 12, value, size);
	bool ok = backup_write_file(path, bytes, size + 12);
	free(bytes);
	return ok;
}

/* invented progress in `dir`: `runs` runs */
static bool progress(const char *dir, int runs) {
	char path[700];
	snprintf(path, sizeof path, "%s/savedata", dir);
	cw_mkdir(path);
	Profile p = { .runs = runs, .best_depth = 3 + runs };
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	return blob(path, PROFILE_MAGIC, &p, sizeof p);
}

/* the saves of another device with `runs` runs, as a .cwsave in `path` */
static bool foreign(const char *base, const char *path, int runs) {
	char other[700];
	snprintf(other, sizeof other, "%s/other", base);
	remove_tree(other);
	cw_mkdir(other);
	size_t n = 0;
	uint8_t *b = progress(other, runs) ? backup_pack(other, 1600000000u + (uint64_t)runs, &n) : NULL;
	bool ok = b && backup_write_file(path, b, n);
	free(b);
	remove_tree(other);
	return ok;
}

static uint8_t *read_all(const char *path, size_t *n) { return backup_read_file(path, n); }

static bool same_bytes(const char *path, const uint8_t *b, size_t n) {
	size_t m = 0;
	uint8_t *c = read_all(path, &m);
	bool same = c && m == n && !memcmp(b, c, n);
	free(c);
	return same;
}

/* the saves rest, then a frame */
static void rest(void) {
	ticks += 4000;
	mirror_tick();
}

/* (a folder's files, their sizes and times: a look must change none) */
typedef struct { char name[256]; long long size, mtime; } Entry;
static int listing(const char *dir, Entry *out, int most) {
	DIR *d = opendir(dir);
	int n = 0;
	if (!d) return 0;
	for (struct dirent *e; (e = readdir(d)) && n < most;) {
		if (e->d_name[0] == '.') continue;
		char path[1200];
		struct stat st;
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		if (stat(path, &st)) continue;
		snprintf(out[n].name, sizeof out[n].name, "%s", e->d_name);
		out[n].size = (long long)st.st_size;
		out[n++].mtime = (long long)st.st_mtime;
	}
	closedir(d);
	return n;
}

static bool same_listing(const Entry *a, int na, const Entry *b, int nb) {
	if (na != nb) return false;
	for (int i = 0; i < na; ++i) {
		bool found = false;
		for (int j = 0; j < nb; ++j)
			found |= !strcmp(a[i].name, b[j].name) && a[i].size == b[j].size && a[i].mtime == b[j].mtime;
		if (!found) return false;
	}
	return true;
}

/* This program run again on `dir`, as a restart: its exit status says
 * whether its first look found saves to compare (1), or none (0) */
static int fresh_look(const char *self, const char *dir) {
	pid_t pid = fork();
	if (pid == 0) {
		execl(self, self, "--look", dir, (char *)NULL);
		_exit(99);
	}
	int status = 0;
	if (pid < 0 || waitpid(pid, &status, 0) < 0 || !WIFEXITED(status)) return -1;
	return WEXITSTATUS(status);
}

static void test_copy(const char *self, const char *base) {
	snprintf(g_data_dir, sizeof g_data_dir, "%s/device", base);
	remove_tree(g_data_dir);
	cw_mkdir(g_data_dir);
	char ini[700], place_file[700];
	snprintf(ini, sizeof ini, "%s/saves.ini", g_data_dir);
	CHECK(backup_write_file(ini, (const uint8_t *)"auto=1\nfolder=\n", 15), "transfer: the copy turned on");
	snprintf(place_file, sizeof place_file, "%s/%s", g_data_dir, BACKUP_NAME);
	CHECK(progress(g_data_dir, 2), "transfer: local progress made");

	/* absent: written */
	mirror_rescan();
	mirror_note();
	rest();
	size_t n = 0;
	uint8_t *mine = read_all(place_file, &n);
	MirrorStatus m;
	mirror_status(&m);
	CHECK(mine && m.state == MIRROR_COPIED && m.copied_at, "transfer: no file in the place: the copy made (state %d)", (int)m.state);
	CHECK(fresh_look(self, g_data_dir) == 0, "transfer: after a restart this device's own copy is no incoming saves");

	/* another device's file: held, never written over */
	CHECK(foreign(base, place_file, 9), "transfer: another device's file in the place");
	size_t fn = 0;
	uint8_t *theirs = read_all(place_file, &fn);
	mirror_rescan();
	CHECK(mirror_incoming(), "transfer: another device's file is offered once");
	CHECK(!mirror_incoming(), "transfer: ... and not again in the same look");
	CHECK(progress(g_data_dir, 3), "transfer: local progress changes");
	mirror_note();
	rest();
	mirror_flush();
	mirror_status(&m);
	CHECK(theirs && same_bytes(place_file, theirs, fn) && m.state == MIRROR_HELD,
		"transfer: the copy holds over another device's file (state %d)", (int)m.state);
	CHECK(fresh_look(self, g_data_dir) == 1, "transfer: unanswered, a restart offers it again");

	/* answered without leave (DONE, B): kept so across a restart, still held */
	mirror_answer(false);
	CHECK(fresh_look(self, g_data_dir) == 0, "transfer: an answer is kept on disk: a restart does not reopen it");
	mirror_note();
	rest();
	CHECK(same_bytes(place_file, theirs, fn), "transfer: DONE gives no leave to write over it");

	/* KEEP THESE: leave given, the next copy writes over it */
	mirror_rescan();
	(void)mirror_incoming();
	mirror_answer(true);
	rest();
	mirror_status(&m);
	CHECK(!same_bytes(place_file, theirs, fn) && m.state == MIRROR_COPIED, "transfer: KEEP THESE lets the save held back write over it");
	CHECK(fresh_look(self, g_data_dir) == 0, "transfer: and the copy is this device's after a restart");
	free(theirs);
	free(mine);

	/* a look writes nothing, with another device's file there */
	CHECK(foreign(base, place_file, 11), "transfer: another device's file again");
	Entry before[64], after[64];
	int nb = listing(g_data_dir, before, 64);
	mirror_rescan();
	(void)mirror_incoming();
	mirror_look();
	int na = listing(g_data_dir, after, 64);
	CHECK(same_listing(before, nb, after, na), "transfer: the look at the place wrote nothing (%d files, then %d)", nb, na);

	/* the place's file unreadable: no copy over it */
	remove(place_file);
	CHECK(cw_mkdir(place_file) == 0, "transfer: a folder where the file goes");
	mirror_rescan();
	mirror_note();
	rest();
	mirror_status(&m);
	struct stat st;
	CHECK(m.state == MIRROR_UNREAD && !stat(place_file, &st) && S_ISDIR(st.st_mode), "transfer: an unreadable place holds the copy (state %d)", (int)m.state);
	cw_rmdir(place_file);

	/* an export's bytes are this device's own where they turn up again */
	size_t en = 0;
	uint8_t *exported = backup_pack(g_data_dir, 0, &en);
	if (exported) mirror_wrote(exported, en);
	CHECK(exported && backup_write_file(place_file, exported, en), "transfer: an export in the place");
	CHECK(progress(g_data_dir, 5), "transfer: progress after the export");
	CHECK(fresh_look(self, g_data_dir) == 0, "transfer: the device's own older export is no incoming saves");
	free(exported);
	char receipts[700];
	snprintf(receipts, sizeof receipts, "%s/saves-receipts.txt", g_data_dir);
	CHECK(access(receipts, F_OK) == 0, "transfer: the receipts kept in the data folder");
}

int main(int argc, char **argv) {
	if (argc == 3 && !strcmp(argv[1], "--look")) {
		snprintf(g_data_dir, sizeof g_data_dir, "%s", argv[2]);
		return mirror_incoming() ? 1 : 0;
	}
	char base[128];
	snprintf(base, sizeof base, "build/host/test-transfer-%d", (int)getpid());
	remove_tree(base);
	cw_mkdir(base);
	test_copy(argv[0], base);
	remove_tree(base);
	if (failures) return 1;
	printf("transfer: the copy writes only its own, holds over another device's, keeps answers, looks without writing\n");
	return 0;
}

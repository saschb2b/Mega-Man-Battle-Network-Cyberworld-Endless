/* A desktop's transfer place (pick_saves.c with CW_DESKTOP) under the
 * automatic copy's look (mirror.c), with invented saves: Downloads asked
 * of the system once and never made until a write; not looked in until
 * the player asks (SAVES opened) or turns the copy on; a file unchanged
 * by size and time not read again; a folder chosen looked at anew. */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#include "backup.h"
#include "backup_internal.h"
#include "compat.h"
#include "desktop.h"
#include "game.h"
#include "mirror.h"
#include "pick.h"
#include "pick_saves.h"
#include "platform.h"
#include "save.h"
#include "save_format.h"

char g_data_dir[512];
static char downloads[600];
static int asked_downloads;
static Uint32 ticks = 10000;
static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

Uint32 SDL_GetTicks(void) { return ticks; }
void platform_persist(void) { mirror_note(); }
bool desktop_big_screen(void) { return false; }
bool desktop_can_choose(void) { return true; }
bool desktop_saves_default(char *path, size_t n) {
	++asked_downloads;
	snprintf(path, n, "%s", downloads);
	return true;
}
bool pick_busy(void) { return false; }
bool pick_saves_busy(void) { return false; }

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

static bool progress(const char *dir, int runs) {
	char path[700];
	snprintf(path, sizeof path, "%s/savedata", dir);
	cw_mkdir(path);
	Profile p = { .runs = runs, .best_depth = 4 };
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	return blob(path, PROFILE_MAGIC, &p, sizeof p);
}

/* another device's saves with `runs` runs as a .cwsave at `path` */
static bool foreign(const char *base, const char *path, int runs) {
	char other[700];
	snprintf(other, sizeof other, "%s/other", base);
	remove_tree(other);
	cw_mkdir(other);
	size_t n = 0;
	uint8_t *b = progress(other, runs) ? backup_pack(other, 1600000000u, &n) : NULL;
	bool ok = b && backup_write_file(path, b, n);
	free(b);
	remove_tree(other);
	return ok;
}

static int held_runs(void) {
	const uint8_t *b;
	size_t n;
	BackupInfo info;
	return mirror_held(&b, &n) && backup_info(b, n, &info) ? info.runs : -1;
}

static void test_place(const char *base) {
	snprintf(g_data_dir, sizeof g_data_dir, "%s/data", base);
	snprintf(downloads, sizeof downloads, "%s/Downloads", base);
	cw_mkdir(g_data_dir);
	CHECK(progress(g_data_dir, 1), "discovery: local progress made");
	struct stat st;
	MirrorStatus m;
	mirror_status(&m);
	mirror_status(&m);
	CHECK(!strcmp(m.place, downloads) && asked_downloads == 1 && stat(downloads, &st) != 0,
		"discovery: Downloads asked once, never made by a look at the status (asked %d)", asked_downloads);

	/* not looked in until asked */
	cw_mkdir(downloads);
	char file[800];
	snprintf(file, sizeof file, "%s/%s", downloads, BACKUP_NAME);
	CHECK(foreign(base, file, 7), "discovery: another device's file in Downloads");
	CHECK(!mirror_incoming() && held_runs() < 0, "discovery: the copy off, Downloads is not looked in at the title");
	mirror_look();
	CHECK(held_runs() == 7, "discovery: SAVES opened, the player asked: Downloads looked in (%d runs)", held_runs());

	/* unchanged by size and time: not read again */
	struct stat was;
	CHECK(!stat(file, &was), "discovery: the file's time read");
	CHECK(foreign(base, file, 8), "discovery: a file of the same size put there");
	struct utimbuf back = { was.st_atime, was.st_mtime };
	CHECK(!utime(file, &back), "discovery: its time put back");
	mirror_look();
	CHECK(held_runs() == 7, "discovery: same size and time: the place's file is not read again");
	struct utimbuf later = { was.st_atime + 10, was.st_mtime + 10 };
	CHECK(!utime(file, &later), "discovery: its time moved on");
	mirror_look();
	CHECK(held_runs() == 8, "discovery: a changed time: read again (%d runs)", held_runs());

	/* a folder chosen: the place, looked at anew */
	char chosen[700];
	snprintf(chosen, sizeof chosen, "%s/chosen", base);
	cw_mkdir(chosen);
	CHECK(saves_folder_keep(chosen), "discovery: a folder kept");
	mirror_new_folder();
	mirror_look();
	mirror_status(&m);
	CHECK(!strcmp(m.place, chosen) && held_runs() < 0, "discovery: the chosen folder is the place, nothing held there");
	pick_saves_auto(true);
	mirror_note();
	ticks += 4000;
	mirror_tick();
	snprintf(file, sizeof file, "%s/%s", chosen, BACKUP_NAME);
	mirror_status(&m);
	CHECK(!stat(file, &st) && m.state == MIRROR_COPIED, "discovery: the copy made in the chosen folder (state %d)", (int)m.state);

	/* Downloads made at the first write there, not before */
	remove_tree(downloads);
	CHECK(saves_folder_keep(""), "discovery: back to Downloads");
	mirror_new_folder();
	mirror_note();
	ticks += 4000;
	mirror_tick();
	snprintf(file, sizeof file, "%s/%s", downloads, BACKUP_NAME);
	CHECK(!stat(file, &st), "discovery: Downloads made by the copy's write, the copy in it");
}

int main(void) {
	char base[128];
	snprintf(base, sizeof base, "build/host/test-discovery-%d", (int)getpid());
	remove_tree(base);
	cw_mkdir(base);
	test_place(base);
	remove_tree(base);
	if (failures) return 1;
	printf("discovery: Downloads asked once and made at a write; looked in when asked; unchanged files not read again\n");
	return 0;
}

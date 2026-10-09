/* Three ordinary-file places behind desktop picker adapters, with invented
 * progress only: a skipped first candidate must not mask a later import. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <SDL.h>

#include "backup_fixture.h"
#include "backup_internal.h"
#include "compat.h"
#include "desktop.h"
#include "game.h"
#include "mirror.h"
#include "pick.h"
#include "pick_saves.h"
#include "platform.h"

char g_data_dir[512];
static char downloads[600];
static Uint32 ticks = 5000;
static bool rom_picker, saves_picker;

Uint32 SDL_GetTicks(void) { return ticks; }
void platform_persist(void) { mirror_note(); }
bool desktop_big_screen(void) { return false; }
bool desktop_can_choose(void) { return true; }
bool pick_busy(void) { return rom_picker; }
bool pick_saves_busy(void) { return saves_picker; }
bool desktop_saves_default(char *path, size_t n) {
	snprintf(path, n, "%s", downloads);
	return true;
}

static bool place_file(char *path, size_t n) {
	char folder[1024];
	if (!pick_saves_place(folder, sizeof folder)) return false;
	snprintf(path, n, "%s/%s", folder, BACKUP_NAME);
	return true;
}

bool pick_saves_get(const char *to) {
	char path[1100];
	return place_file(path, sizeof path) && saves_stage(path, to);
}

bool pick_saves_put(const char *from) {
	char path[1100];
	return place_file(path, sizeof path) && saves_copy(from, path);
}

static void progress(const char *dir, unsigned runs) {
	char path[600];
	snprintf(path, sizeof path, "%s/savedata/profile.sav", dir);
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	assert(b && n >= 20);
	backup_put32(b + 12, runs);
	backup_put32(b + 8, backup_fnv(b + 12, backup_get32(b + 4), 2166136261u));
	assert(backup_write_file(path, b, n));
	free(b);
}

static void same_bytes(const char *path, const uint8_t *expected, size_t length) {
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	assert(b && n == length && !memcmp(b, expected, n));
	free(b);
}

static void picker_protection(const char *folder, const uint8_t *incoming, size_t n) {
	char path[1100], staged[600];
	snprintf(path, sizeof path, "%s/%s", folder, BACKUP_NAME);
	assert(backup_write_file(path, incoming, n));
	pick_saves_auto(true);
	for (int kind = 0; kind < 2; ++kind) {
		bool *busy = kind ? &saves_picker : &rom_picker;
		*busy = true;
		/* A picker worker retains its new destination before handing its
		 * result to the frame loop. Neither discovery nor writes may race it. */
		assert(saves_folder_keep(folder));
		mirror_new_folder();
		ticks += 4000;
		mirror_tick();
		assert(!mirror_scan(staged, sizeof staged));
		mirror_flush();
		same_bytes(path, incoming, n);
		*busy = false;
		assert(mirror_scan(staged, sizeof staged));
		same_bytes(staged, incoming, n);
		mirror_flush();
		same_bytes(path, incoming, n);
		mirror_hold(false);
	}
}

int main(void) {
	char base[] = "build/host/test-discovery-XXXXXX", kept[600], remote[600];
	char own[700], downloaded[700], data_file[600], staged[600], manual[600];
	assert(mkdtemp(base));
	snprintf(g_data_dir, sizeof g_data_dir, "%s/local", base);
	snprintf(kept, sizeof kept, "%s/kept", base);
	snprintf(downloads, sizeof downloads, "%s/downloads", base);
	snprintf(remote, sizeof remote, "%s/remote", base);
	assert(!cw_mkdir(g_data_dir) && !cw_mkdir(kept) && !cw_mkdir(downloads) && !cw_mkdir(remote));
	assert(backup_restore(g_data_dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK);
	assert(backup_restore(remote, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK);
	assert(saves_folder_keep(kept));
	pick_saves_auto(true);
	mirror_new_folder();
	mirror_flush();
	assert(mirror_last() == 1);
	snprintf(own, sizeof own, "%s/%s", kept, BACKUP_NAME);
	snprintf(downloaded, sizeof downloaded, "%s/%s", downloads, BACKUP_NAME);
	snprintf(data_file, sizeof data_file, "%s/%s", g_data_dir, BACKUP_NAME);
	snprintf(manual, sizeof manual, "%s/manual.cwsave", g_data_dir);
	size_t own_n = 0, first_n = 0, second_n = 0;
	uint8_t *own_bytes = backup_read_file(own, &own_n);
	assert(own_bytes);
	pick_saves_auto(false);
	progress(g_data_dir, 3);
	progress(remote, 1);
	uint8_t *first = backup_pack(remote, 0, &first_n);
	assert(first && backup_write_file(downloaded, first, first_n));

	/* The recognized old output in the kept folder cannot hide Downloads.
	 * Manual Import can still explicitly choose that older own output. */
	mirror_new_folder();
	assert(mirror_scan(staged, sizeof staged));
	same_bytes(staged, first, first_n);
	same_bytes(own, own_bytes, own_n);
	assert(pick_saves_find(manual));
	same_bytes(manual, own_bytes, own_n);

	/* Keep dismisses only those exact Downloads bytes: the data folder's
	 * next candidate is still found, with auto-export disabled. */
	mirror_hold(false);
	progress(remote, 2);
	uint8_t *second = backup_pack(remote, 0, &second_n);
	assert(second && backup_write_file(data_file, second, second_n));
	ticks += 4000;
	assert(mirror_scan(staged, sizeof staged));
	same_bytes(staged, second, second_n);
	mirror_flush();
	same_bytes(own, own_bytes, own_n);
	same_bytes(downloaded, first, first_n);
	same_bytes(data_file, second, second_n);
	BackupInfo local;
	assert(backup_local_info(g_data_dir, &local) && local.runs == 3);
	mirror_hold(false);
	picker_protection(remote, second, second_n);
	free(own_bytes);
	free(first);
	free(second);
	puts("discovery: known places and manual import retained; both picker workers protect incoming files");
	return 0;
}

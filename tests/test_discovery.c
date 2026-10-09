/* Three ordinary-file places behind desktop picker adapters, with invented
 * progress only: a skipped first candidate must not mask a later import. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
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
static bool rom_picker, saves_picker, fail_put;
static time_t copy_time = 1700000000;

Uint32 SDL_GetTicks(void) { return ticks; }
time_t time(time_t *out) { if (out) *out = copy_time; return copy_time; }
void platform_persist(void) { mirror_note(); }
bool desktop_big_screen(void) { return false; }
bool desktop_can_choose(void) { return true; }
bool pick_busy(void) { return rom_picker; }
bool pick_saves_busy(void) { return saves_picker; }
bool desktop_saves_default(char *path, size_t n) {
	snprintf(path, n, "%s", downloads);
	return downloads[0] != 0;
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
	return !fail_put && place_file(path, sizeof path) && saves_copy(from, path);
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

static int receipt_check(const char *dir, const char *stamp, const char *destination) {
	snprintf(g_data_dir, sizeof g_data_dir, "%s", dir);
	MirrorStatus status;
	mirror_status(&status);
	assert(!status.copied_at && status.state == MIRROR_IDLE);
	mirror_tick();
	mirror_status(&status);
	uint64_t expected = strtoull(stamp, NULL, 10);
	assert(status.copied_at == expected && !strcmp(status.destination, destination));
	assert(status.state == (expected ? MIRROR_COPIED : MIRROR_IDLE));
	return 0;
}

static void receipt_restart(uint64_t stamp, const char *destination) {
	char expected[32];
	snprintf(expected, sizeof expected, "%llu", (unsigned long long)stamp);
	pid_t child = fork();
	assert(child >= 0);
	if (!child) {
		execl("/proc/self/exe", "test_discovery", "--receipt-check", g_data_dir, expected, destination, (char *)NULL);
		_exit(127);
	}
	int status;
	assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void receipt_status(void) {
	char base[] = "build/host/test-receipt-XXXXXX", first[600], next[600], path[1100], receipt_path[600];
	assert(mkdtemp(base));
	snprintf(g_data_dir, sizeof g_data_dir, "%s/local", base);
	snprintf(first, sizeof first, "%s/first", base);
	snprintf(next, sizeof next, "%s/next", base);
	snprintf(downloads, sizeof downloads, "%s/downloads", base);
	assert(!cw_mkdir(g_data_dir) && !cw_mkdir(first) && !cw_mkdir(next) && !cw_mkdir(downloads));
	assert(backup_restore(g_data_dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK);
	assert(saves_folder_keep(first));
	pick_saves_auto(true);
	mirror_new_folder();
	mirror_flush();
	MirrorStatus status;
	mirror_status(&status);
	assert(status.state == MIRROR_COPIED && status.copied_at == (uint64_t)copy_time && !strcmp(status.destination, first));
	char initial[sizeof g_data_dir], other[600];
	memcpy(initial, g_data_dir, sizeof initial);
	snprintf(other, sizeof other, "%s/other", base);
	assert(!cw_mkdir(other));
	snprintf(g_data_dir, sizeof g_data_dir, "%s/other", base);
	assert(saves_folder_keep(first));
	mirror_status(&status);
	assert(!status.copied_at && status.state == MIRROR_IDLE);
	memcpy(g_data_dir, initial, sizeof g_data_dir);
	mirror_status(&status);
	receipt_restart(status.copied_at, first);
	snprintf(receipt_path, sizeof receipt_path, "%s/saves-export.receipt", g_data_dir);
	size_t receipt_n = 0;
	uint8_t *receipt_bytes = backup_read_file(receipt_path, &receipt_n);
	assert(receipt_bytes);
	uint64_t previous = status.copied_at;
	progress(g_data_dir, 4);
	mirror_note();
	mirror_status(&status);
	assert(status.state == MIRROR_PENDING && status.copied_at == previous);
	fail_put = true;
	ticks += 4000;
	mirror_tick();
	mirror_status(&status);
	assert(status.state == MIRROR_FAILED && status.copied_at == previous && !strcmp(status.destination, first));
	same_bytes(receipt_path, receipt_bytes, receipt_n);
	/* Retry copies the current progress and updates its receipt only after
	 * the adapter succeeds, even after a cold restart retained the old time. */
	receipt_restart(previous, first);
	fail_put = false;
	copy_time += 60;
	mirror_retry();
	mirror_status(&status);
	assert(status.state == MIRROR_PENDING && status.copied_at == previous);
	mirror_tick();
	mirror_status(&status);
	assert(status.state == MIRROR_COPIED && status.copied_at == (uint64_t)copy_time);
	previous = status.copied_at;
	snprintf(path, sizeof path, "%s/%s", first, BACKUP_NAME);
	assert(backup_write_file(path, backup_fixture, sizeof backup_fixture));
	mirror_note();
	ticks += 4000;
	mirror_tick();
	mirror_status(&status);
	assert(status.state == MIRROR_INCOMING && status.copied_at == previous);
	mirror_retry();
	mirror_flush();
	same_bytes(path, backup_fixture, sizeof backup_fixture);
	mirror_hold(false);
	mirror_flush();
	/* Selecting a folder elsewhere in the launcher can change the scope
	 * without mirror_new_folder. The same progress still needs a new copy. */
	assert(saves_folder_keep(next));
	mirror_status(&status);
	assert(!status.copied_at && status.state == MIRROR_PENDING && !strcmp(status.destination, next));
	copy_time += 60;
	mirror_flush();
	mirror_status(&status);
	assert(status.state == MIRROR_COPIED && status.copied_at == (uint64_t)copy_time && !strcmp(status.destination, next));
	snprintf(path, sizeof path, "%s/%s", next, BACKUP_NAME);
	size_t n = 0;
	uint8_t *copied = backup_read_file(path, &n);
	BackupInfo info;
	assert(copied && backup_info(copied, n, &info) && info.runs == 4);
	free(copied);
	receipt_restart(status.copied_at, next);
	assert(backup_write_file(receipt_path, (const uint8_t *)"damaged", 7));
	receipt_restart(0, next);
	assert(saves_folder_keep(""));
	downloads[0] = 0;
	mirror_status(&status);
	assert(!status.copied_at && status.state != MIRROR_COPIED && !status.destination[0]);
	free(receipt_bytes);
}

int main(int argc, char **argv) {
	if (argc == 5 && !strcmp(argv[1], "--receipt-check")) return receipt_check(argv[2], argv[3], argv[4]);
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
	receipt_status();
	puts("discovery: fallback, picker protection and scoped copy receipts/retry passed");
	return 0;
}

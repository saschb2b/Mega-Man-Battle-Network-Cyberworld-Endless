/* The no-picker transfer store, with a deterministic clock and synthetic
 * progress: a synced-in file must survive until the player's decision. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <SDL.h>

#include "backup_fixture.h"
#include "backup_internal.h"
#include "compat.h"
#include "game.h"
#include "mirror.h"
#include "pick.h"
#include "platform.h"
#include "version.h"

char g_data_dir[512];
static Uint32 ticks = 5000;

Uint32 SDL_GetTicks(void) { return ticks; }
void platform_persist(void) { mirror_note(); }

static void advance(void) { ticks += 4000; mirror_tick(); }

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

static BackupInfo file_info(const char *path) {
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	BackupInfo info;
	assert(b && backup_info(b, n, &info));
	free(b);
	return info;
}

static void same_bytes(const char *path, const uint8_t *expected, size_t length) {
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	assert(b && n == length && !memcmp(b, expected, n));
	free(b);
}

static uint8_t *manifest_bytes(uint8_t *b, size_t n) {
	size_t at = BACKUP_HEAD;
	while (at + 6 < n - 4) {
		size_t len = (size_t)b[at] | (size_t)b[at + 1] << 8;
		uint32_t size = backup_get32(b + at + 2 + len);
		if (strlen(BACKUP_MANIFEST) == len && !memcmp(b + at + 2, BACKUP_MANIFEST, len)) return b + at + 6 + len;
		at += 6 + len + size;
	}
	return NULL;
}

static void future_file(const char *path, bool build) {
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	assert(b);
	BackupInfo local, future;
	assert(backup_info(b, n, &local));
	uint8_t *manifest = manifest_bytes(b, n);
	assert(manifest);
	if (build) manifest[strlen("format=2\nversion=")] = '9';
	else manifest[7] = '3';
	backup_put32(b + n - 4, backup_fnv(b, n - 4, 2166136261u));
	assert(backup_info(b, n, &future) && future.hash == local.hash &&
		future.status == (build ? BACKUP_NEWER_BUILD : BACKUP_NEWER_FORMAT));
	assert(backup_write_file(path, b, n));
	/* Downgrading the app must not treat its previously exported newer
	 * carrier as safe to overwrite just because its fingerprint is known. */
	char fingerprint[600];
	uint8_t known[4];
	backup_put32(known, backup_fnv(b, n, 2166136261u));
	snprintf(fingerprint, sizeof fingerprint, "%s/saves-export.hash", g_data_dir);
	assert(backup_write_file(fingerprint, known, sizeof known));
	mirror_new_folder();
	advance();
	char staged[600];
	assert(mirror_scan(staged, sizeof staged));
	mirror_flush();
	same_bytes(path, b, n);
	mirror_hold(false);
	mirror_flush();
	assert(file_info(path).status == BACKUP_OK);
	free(b);
}

static void parent_fork(void) {
	char base[] = "build/host/test-parent-fork-XXXXXX", remote[600], path[600], staged[600];
	assert(mkdtemp(base));
	snprintf(g_data_dir, sizeof g_data_dir, "%s/local", base);
	snprintf(remote, sizeof remote, "%s/remote", base);
	assert(cw_mkdir(g_data_dir) == 0 && cw_mkdir(remote) == 0);
	assert(backup_restore(g_data_dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK);
	pick_saves_auto(true);
	mirror_new_folder();
	mirror_flush();
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	size_t n = 0, incoming_n = 0;
	uint8_t *first = backup_read_file(path, &n);
	assert(first && backup_restore(remote, first, n, true) == BACKUP_OK);
	free(first);

	/* Two devices independently make revision 2 from the same revision 1. */
	progress(g_data_dir, 10);
	uint8_t *local = backup_pack(g_data_dir, 0, &n);
	BackupInfo local_parent, file, latest;
	assert(local && backup_info(local, n, &local_parent));
	free(local);
	progress(remote, 20);
	uint8_t *incoming = backup_pack(remote, 0, &incoming_n);
	assert(incoming && backup_info(incoming, incoming_n, &file));
	assert(!strcmp(file.save_id, local_parent.save_id) && file.revision == local_parent.revision && file.hash != local_parent.hash);
	/* Local advances again before the other revision 2 arrives by sync. */
	progress(g_data_dir, 30);
	assert(backup_local_info(g_data_dir, &latest) && latest.parent_revision == file.revision);
	assert(backup_write_file(path, incoming, incoming_n));
	mirror_new_folder();   /* clear volatile recognition, preserving the on-disk export fingerprint */
	advance();
	assert(mirror_scan(staged, sizeof staged));
	mirror_flush();
	same_bytes(path, incoming, incoming_n);
	/* A new process has no volatile mirror state to recognize the file by. */
	pid_t child = fork();
	assert(child >= 0);
	if (!child) {
		execl("/proc/self/exe", "test_transfer", "--restart-check", g_data_dir, (char *)NULL);
		_exit(127);
	}
	int status;
	assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);

	/* Restart-style reset must not turn the parent-number fork into our output. */
	mirror_hold(false);
	mirror_new_folder();
	advance();
	assert(mirror_scan(staged, sizeof staged));
	mirror_flush();
	same_bytes(path, incoming, incoming_n);
	mirror_hold(false);
	mirror_flush();
	assert(file_info(path).runs == 30);
	free(incoming);
}

static int restart_check(const char *dir) {
	snprintf(g_data_dir, sizeof g_data_dir, "%s", dir);
	mirror_new_folder();
	advance();
	char path[600], staged[600];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	assert(mirror_scan(staged, sizeof staged));
	mirror_flush();
	assert(file_info(path).runs == 20);
	return 0;
}

static void export_results(const char *path, const uint8_t *bytes, size_t n) {
	char from[600];
	snprintf(from, sizeof from, "%s/export.cwsave", g_data_dir);
	assert(backup_write_file(from, bytes, n));
	assert(pick_saves_export(from) && pick_saves_busy());
	PickResult result;
	assert(pick_saves_done(&result) && result.status == 1 && !result.path);
	assert(!pick_saves_busy() && !pick_saves_done(&result));
	same_bytes(path, bytes, n);
	/* A directory at the destination forces the atomic write to fail.
	 * Completion must report failure, never leave a success or bare path. */
	assert(remove(path) == 0 && cw_mkdir(path) == 0);
	assert(pick_saves_export(from) && pick_saves_busy());
	assert(pick_saves_done(&result) && result.status == -2 && !result.path);
	assert(strstr(result.text, "could not be exported") && !pick_saves_busy());
	assert(!pick_saves_done(&result));
	struct stat st;
	assert(stat(path, &st) == 0 && S_ISDIR(st.st_mode));
	same_bytes(from, bytes, n);
	assert(rmdir(path) == 0);
}

int main(int argc, char **argv) {
	if (argc == 3 && !strcmp(argv[1], "--restart-check")) return restart_check(argv[2]);
	char base[] = "build/host/test-transfer-XXXXXX", remote[600], path[600], staged[600];
	assert(mkdtemp(base));
	snprintf(g_data_dir, sizeof g_data_dir, "%s/local", base);
	snprintf(remote, sizeof remote, "%s/remote", base);
	assert(cw_mkdir(g_data_dir) == 0 && cw_mkdir(remote) == 0);
	assert(backup_restore(g_data_dir, backup_fixture, sizeof backup_fixture, true) == BACKUP_OK);
	pick_saves_auto(true);
	mirror_new_folder();
	mirror_flush();
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	assert(mirror_last() == 1 && file_info(path).runs == 0);

	/* The local checkpoint refreshes our own output without a question. */
	progress(g_data_dir, 1);
	mirror_new_folder();   /* also clears the last-output memory, as a restart does */
	advance();
	assert(!mirror_scan(staged, sizeof staged));
	assert(file_info(path).runs == 1);

	/* A second device continues the same save, then places its file here. */
	size_t n = 0, incoming_n = 0;
	uint8_t *local = backup_read_file(path, &n);
	assert(local && backup_restore(remote, local, n, true) == BACKUP_OK);
	free(local);
	progress(remote, 2);
	uint8_t *incoming = backup_pack(remote, 2000000000u, &incoming_n);
	assert(incoming && backup_write_file(path, incoming, incoming_n));
	mirror_note();
	advance();
	assert(mirror_scan(staged, sizeof staged));
	mirror_flush();
	same_bytes(path, incoming, incoming_n);
	same_bytes(staged, incoming, incoming_n);

	/* Keep this device is the explicit decision that permits replacing it. */
	mirror_hold(false);
	mirror_flush();
	assert(file_info(path).runs == 1 && !mirror_scan(staged, sizeof staged));

	/* A damaged file gets the same protection, including on backgrounding. */
	incoming[0] ^= 1;
	assert(backup_write_file(path, incoming, incoming_n));
	mirror_note();
	advance();
	assert(mirror_scan(staged, sizeof staged));
	mirror_flush();
	same_bytes(path, incoming, incoming_n);
	mirror_hold(false);
	mirror_flush();
	assert(file_info(path).runs == 1);
	incoming[0] ^= 1;

	/* Compatibility preflight applies even when the payload matches ours. */
	future_file(path, false);
	unsigned int major, minor, patch;
	if (sscanf(CW_VERSION, "%u.%u.%u", &major, &minor, &patch) == 3 && major < 9) future_file(path, true);

	/* Too-large input is refused without erasing the source either. */
	FILE *large = fopen(path, "wb");
	assert(large && fseek(large, BACKUP_MAX, SEEK_SET) == 0 && fputc(1, large) == 1);
	assert(fclose(large) == 0);
	mirror_note();
	advance();
	assert(mirror_scan(staged, sizeof staged));
	mirror_flush();
	struct stat st;
	assert(stat(path, &st) == 0 && (uint64_t)st.st_size > BACKUP_MAX);
	mirror_hold(false);
	mirror_flush();
	assert(file_info(path).runs == 1);

	/* Import without a picker stages bytes; it never writes local progress. */
	assert(backup_write_file(path, incoming, incoming_n));
	assert(pick_saves_import());
	PickResult picked;
	assert(pick_saves_done(&picked) && picked.status == 1 && picked.path);
	assert(!pick_saves_done(&picked));
	same_bytes(picked.text, incoming, incoming_n);
	BackupInfo here;
	assert(backup_local_info(g_data_dir, &here) && here.runs == 1);
	export_results(path, incoming, incoming_n);
	free(incoming);
	parent_fork();
	puts("transfer: auto-export, divergent-parent and damaged-file protection, known-place import passed");
	return 0;
}

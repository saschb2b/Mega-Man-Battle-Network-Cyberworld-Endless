/* mirror.h. The saves are packed (backup_pack) into the data folder's own
 * .cwsave first, then the platform copies that into the folder kept
 * (pick_saves_put: the kept folder or known place on each system); the
 * same saves packed again are not copied again. */
#include "mirror.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "backup.h"
#include "game.h"
#include "pick.h"
#include "platform.h"

#define REST_MS 3000   /* the saves untouched this long: a checkpoint's files all written */

static bool dirty, held;
static uint32_t written_at;     /* SDL_GetTicks of the last write */
static uint32_t put_hash;       /* the saves last copied */
static int last;
static bool scanned, pending;
static uint32_t scan_at, pending_hash, ignored_hash;
static char pending_path[600];

void mirror_note(void) {
	dirty = true;
	written_at = SDL_GetTicks();
}

void mirror_hold(bool on) {
	held = on;
	if (!on && pending) {
		ignored_hash = pending_hash;
		pending = false;
		put_hash = 0;
		dirty = true;
		written_at = SDL_GetTicks() - REST_MS;
	}
}

void mirror_new_folder(void) {
	last = 0;
	put_hash = 0;
	scanned = false;
	ignored_hash = 0;
	dirty = true;
	written_at = SDL_GetTicks() - REST_MS;
}

int mirror_last(void) { return last; }

static uint32_t file_hash(const char *path) {
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	uint32_t hash = 2166136261u;
	for (size_t i = 0; b && i < n; ++i) hash = (hash ^ b[i]) * 16777619u;
	free(b);
	return hash;
}

/* Revision numbers alone cannot identify our output: two devices can
 * make different revision 2s from revision 1. Keep the fingerprint of the
 * bytes we successfully exported, on this device, across app restarts. */
static uint32_t exported_hash(void) {
	char path[600];
	snprintf(path, sizeof path, "%s/saves-export.hash", g_data_dir);
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	uint32_t hash = b && n == 4 ? (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24 : 0;
	free(b);
	return hash;
}

static void remember_export(uint32_t hash) {
	char path[600];
	uint8_t bytes[4];
	for (int i = 0; i < 4; ++i) bytes[i] = (uint8_t)(hash >> (8 * i));
	snprintf(path, sizeof path, "%s/saves-export.hash", g_data_dir);
	if (backup_write_file(path, bytes, sizeof bytes)) platform_persist();
}

static void scan(void) {
	scanned = true;
	scan_at = SDL_GetTicks();
	if (held || pending) return;
	snprintf(pending_path, sizeof pending_path, "%s/saves-found.cwsave", g_data_dir);
	if (!pick_saves_discover(pending_path, ignored_hash, exported_hash())) return;
	pending_hash = file_hash(pending_path);
	pending = true;
	held = true;
}

bool mirror_scan(char *path, size_t n) {
	if (!scanned || (!held && SDL_GetTicks() - scan_at >= REST_MS)) scan();
	if (pending && n) snprintf(path, n, "%s", pending_path);
	return pending;
}

static void put(void) {
	char folder[1024];
	if (!dirty || held || !pick_saves_auto_enabled() || !pick_saves_place(folder, sizeof folder)) return;
	/* A provider may have received another device's file since the title.
	 * Read it before writing ours, even when this frame is backgrounding. */
	scan();
	if (pending) return;
	dirty = false;
	size_t n = 0;
	uint8_t *b = backup_pack(g_data_dir, 0, &n);
	BackupInfo info;
	if (!b || !backup_info(b, n, &info) || info.hash == put_hash) { free(b); return; }
	char path[600];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	bool ok = backup_write_file(path, b, n) && pick_saves_put(path);
	free(b);
	last = ok ? 1 : -1;
	if (ok) {
		put_hash = info.hash;
		ignored_hash = file_hash(path);
		remember_export(ignored_hash);
	}
	else fprintf(stderr, "saves: the copy in %s could not be written\n", folder);
}

void mirror_tick(void) {
	if (dirty && SDL_GetTicks() - written_at >= REST_MS) put();
}

void mirror_flush(void) { put(); }

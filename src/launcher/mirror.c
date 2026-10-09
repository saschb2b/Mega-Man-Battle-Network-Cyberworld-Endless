/* mirror.h. The saves are packed (backup_pack) into the data folder's own
 * .cwsave first, then the platform copies that into the folder kept
 * (pick_saves_put: the kept folder or known place on each system); the
 * same saves packed again are not copied again. */
#include "mirror.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
static char receipt_dir[512];
static char attempted_scope[1024], attempted_destination[1024];
static struct { uint64_t stamp; uint32_t hash; char destination[1024], scope[1024]; } receipt;

static uint32_t bytes_hash(const uint8_t *b, size_t n) {
	uint32_t hash = 2166136261u;
	for (size_t i = 0; i < n; ++i) hash = (hash ^ b[i]) * 16777619u;
	return hash;
}

static uint32_t get32(const uint8_t *b) { return (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24; }
static void put32(uint8_t *b, uint32_t v) { for (int i = 0; i < 4; ++i) b[i] = (uint8_t)(v >> (8 * i)); }

static void receipt_load(void) {
	if (!strcmp(receipt_dir, g_data_dir)) return;
	snprintf(receipt_dir, sizeof receipt_dir, "%s", g_data_dir);
	memset(&receipt, 0, sizeof receipt);
	char path[600];
	snprintf(path, sizeof path, "%s/saves-export.receipt", g_data_dir);
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	if (b && n >= 24 && !memcmp(b, "CWEX", 4) && get32(b + n - 4) == bytes_hash(b, n - 4)) {
		size_t scope = (size_t)b[16] | (size_t)b[17] << 8, dest = (size_t)b[18] | (size_t)b[19] << 8;
		if (scope < sizeof receipt.scope && dest && dest < sizeof receipt.destination && n == 24 + scope + dest &&
			!memchr(b + 20, 0, scope + dest)) {
			for (int i = 0; i < 8; ++i) receipt.stamp |= (uint64_t)b[4 + i] << (8 * i);
			receipt.hash = get32(b + 12);
			memcpy(receipt.scope, b + 20, scope);
			memcpy(receipt.destination, b + 20 + scope, dest);
		}
	}
	free(b);
}

void mirror_status(MirrorStatus *out) {
	memset(out, 0, sizeof *out);
	pick_saves_place(out->destination, sizeof out->destination);
	if (strcmp(receipt_dir, g_data_dir)) return;
	char scope[1024];
	bool scoped = pick_saves_scope(scope, sizeof scope);
	bool copied_here = scoped && !strcmp(scope, receipt.scope);
	if (copied_here) out->copied_at = receipt.stamp;
	if (pending) out->state = MIRROR_INCOMING;
	else if (last < 0 && ((scoped && !strcmp(scope, attempted_scope)) ||
		(!scoped && !attempted_scope[0] && !strcmp(out->destination, attempted_destination)))) out->state = MIRROR_FAILED;
	else if (dirty && pick_saves_auto_enabled()) out->state = MIRROR_PENDING;
	else if (copied_here && (out->copied_at || last > 0)) out->state = MIRROR_COPIED;
}

void mirror_retry(void) {
	dirty = true;
	put_hash = 0;
	last = 0;
	written_at = SDL_GetTicks() - REST_MS;
}

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
	uint32_t hash = b ? bytes_hash(b, n) : 0;
	free(b);
	return hash;
}

/* Revision numbers alone cannot identify our output: two devices can
 * make different revision 2s from revision 1. Keep the fingerprint of the
 * bytes we successfully exported, on this device, across app restarts. */
static uint32_t exported_hash(void) {
	receipt_load();
	if (receipt.hash) return receipt.hash;
	char path[600];
	snprintf(path, sizeof path, "%s/saves-export.hash", g_data_dir);
	size_t n = 0;
	uint8_t *b = backup_read_file(path, &n);
	uint32_t hash = b && n == 4 ? (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24 : 0;
	free(b);
	return hash;
}

static void remember_export(uint32_t hash, const char *destination) {
	receipt_load();
	receipt.hash = hash;
	time_t now = time(NULL);
	receipt.stamp = now > 0 ? (uint64_t)now : 0;
	snprintf(receipt.destination, sizeof receipt.destination, "%s", destination);
	if (!pick_saves_scope(receipt.scope, sizeof receipt.scope)) receipt.scope[0] = 0;
	char path[600];
	uint8_t bytes[2100];
	size_t scope = strlen(receipt.scope), dest = strlen(receipt.destination), n = 24 + scope + dest;
	memcpy(bytes, "CWEX", 4);
	for (int i = 0; i < 8; ++i) bytes[4 + i] = (uint8_t)(receipt.stamp >> (8 * i));
	put32(bytes + 12, hash);
	bytes[16] = (uint8_t)scope;
	bytes[17] = (uint8_t)(scope >> 8);
	bytes[18] = (uint8_t)dest;
	bytes[19] = (uint8_t)(dest >> 8);
	memcpy(bytes + 20, receipt.scope, scope);
	memcpy(bytes + 20 + scope, receipt.destination, dest);
	put32(bytes + n - 4, bytes_hash(bytes, n - 4));
	snprintf(path, sizeof path, "%s/saves-export.receipt", g_data_dir);
	if (backup_write_file(path, bytes, n)) platform_persist();
}

static void scan(void) {
	if (pick_busy() || pick_saves_busy()) return;
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
	char folder[1024], scope[1024];
	if (!dirty || held || pick_busy() || pick_saves_busy() || !pick_saves_auto_enabled() || !pick_saves_place(folder, sizeof folder)) return;
	receipt_load();
	if (!pick_saves_scope(scope, sizeof scope)) scope[0] = 0;
	if (!scope[0] || strcmp(scope, receipt.scope)) put_hash = 0;
	/* A provider may have received another device's file since the title.
	 * Read it before writing ours, even when this frame is backgrounding. */
	scan();
	if (pending) return;
	snprintf(attempted_scope, sizeof attempted_scope, "%s", scope);
	snprintf(attempted_destination, sizeof attempted_destination, "%s", folder);
	dirty = false;
	size_t n = 0;
	uint8_t *b = backup_pack(g_data_dir, 0, &n);
	BackupInfo info;
	if (!b || !backup_info(b, n, &info)) { free(b); last = -1; return; }
	if (info.hash == put_hash) { free(b); return; }
	char path[600];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	bool ok = backup_write_file(path, b, n) && pick_saves_put(path);
	free(b);
	last = ok ? 1 : -1;
	if (ok) {
		put_hash = info.hash;
		ignored_hash = file_hash(path);
		remember_export(ignored_hash, folder);
		dirty = false;
	}
	else fprintf(stderr, "saves: the copy in %s could not be written\n", folder);
}

void mirror_tick(void) {
	receipt_load();
	if (dirty && SDL_GetTicks() - written_at >= REST_MS) put();
}

void mirror_flush(void) { put(); }

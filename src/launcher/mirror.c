/* mirror.h. The saves are packed (backup_pack) into the data folder's own
 * .cwsave first, then the platform copies that into the folder kept
 * (pick_saves_put: Android's document tree, iOS's bookmarked folder); the
 * same saves packed again are not copied again. */
#include "mirror.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>

#include "backup.h"
#include "game.h"
#include "pick.h"

#define REST_MS 3000   /* the saves untouched this long: a checkpoint's files all written */

static bool dirty, held;
static uint32_t written_at;     /* SDL_GetTicks of the last write */
static uint32_t put_hash;       /* the saves last copied */
static int last;

void mirror_note(void) {
	dirty = true;
	written_at = SDL_GetTicks();
}

void mirror_hold(bool on) { held = on; }

void mirror_new_folder(void) {
	last = 0;
	put_hash = 0;
	dirty = true;
	written_at = SDL_GetTicks() - REST_MS;
}

int mirror_last(void) { return last; }

static void put(void) {
	char folder[256];
	if (!dirty || held || !(pick_kinds() & PICK_FOLDER) || !pick_folder(folder, sizeof folder)) return;
	dirty = false;
	size_t n = 0;
	uint8_t *b = backup_pack(g_data_dir, (uint64_t)time(NULL), &n);
	BackupInfo info;
	if (!b || !backup_info(b, n, &info) || info.hash == put_hash) { free(b); return; }
	char path[600];
	snprintf(path, sizeof path, "%s/%s", g_data_dir, BACKUP_NAME);
	bool ok = backup_write_file(path, b, n) && pick_saves_put(path);
	free(b);
	last = ok ? 1 : -1;
	if (ok) put_hash = info.hash;
	else fprintf(stderr, "saves: the copy in %s could not be written\n", folder);
}

void mirror_tick(void) {
	if (dirty && SDL_GetTicks() - written_at >= REST_MS) put();
}

void mirror_flush(void) { put(); }

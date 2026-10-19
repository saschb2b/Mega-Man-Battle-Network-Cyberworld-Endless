/* The Arena's rollback window on this system (arena_window.h). */
#include "arena_window.h"

#include <stdbool.h>
#include <stdio.h>

#include "arena_sync.h"
#include "compat.h"
#include "game.h"
#include "platform.h"

static void path_of(char *out, size_t size) { snprintf(out, size, "%s/arena.ini", g_data_dir); }

void arena_window_keep(double frame_ms) {
	if (!(frame_ms > 0) || frame_ms > 1000) return;
	char path[600], temp[610];
	path_of(path, sizeof path);
	snprintf(temp, sizeof temp, "%s.tmp", path);
	FILE *file = fopen(temp, "w");
	if (!file) return;
	fprintf(file, "frame_us = %u\n", (unsigned)(frame_ms * 1000.0 + 0.5));
	bool ok = !ferror(file);
	if (fclose(file)) ok = false;
	if (ok && cw_rename(temp, path)) platform_persist();
	else remove(temp);
}

unsigned arena_window_kept(void) {
	char path[600], line[80];
	path_of(path, sizeof path);
	FILE *file = fopen(path, "r");
	unsigned us = 0;
	while (file && fgets(line, sizeof line, file))
		if (sscanf(line, " frame_us = %u", &us) == 1) break;
	if (file) fclose(file);
	return us ? arena_window_for(us / 1000.0) : ARENA_WINDOW_FIRST;
}

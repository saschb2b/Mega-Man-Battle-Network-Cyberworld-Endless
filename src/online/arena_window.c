/* The Arena's rollback window on this system (arena_window.h). */
#include "arena_window.h"

#include <stdbool.h>
#include <stdio.h>

#include "arena_sync.h"
#include "compat.h"
#include "emu.h"
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

/* (a pair frame costs its two cores, the cable between them and their
 * pictures: 2.4 to 2.7 of the run's core's frame in the Square in the
 * scenario's battles here, 1.19 to 1.36 ms against 0.45 to 0.50, issue
 * #180; rollbench's 2.2 here and 1.5 on the Nova without the pictures. The
 * larger and some taken, so a guess errs to a smaller window. A second's
 * frames at least, and a guess no larger than the Nova's own) */
#define PAIR_OF_CORE   2.8
#define TIMED_FRAMES   60
#define GUESSED_MOST   8

unsigned arena_window_kept(void) {
	char path[600], line[80];
	path_of(path, sizeof path);
	FILE *file = fopen(path, "r");
	unsigned us = 0, frames = 0;
	while (file && fgets(line, sizeof line, file))
		if (sscanf(line, " frame_us = %u", &us) == 1) break;
	if (file) fclose(file);
	if (us) return arena_window_for(us / 1000.0);
	/* (none kept, a first NetBattle on this system: guessed from the run's
	 * own core, measured since the start, issue #180) */
	double core = emu_core_frame_ms(&frames);
	if (frames < TIMED_FRAMES || !(core > 0)) return ARENA_WINDOW_FIRST;
	unsigned w = arena_window_for(core * PAIR_OF_CORE);
	if (w > GUESSED_MOST) w = GUESSED_MOST;
	printf("arena: no battle's cost kept here; the core's %.2f ms a frame (%u frames) guess a window of %u\n", core, frames, w);
	return w;
}

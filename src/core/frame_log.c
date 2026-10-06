/* frame_log.h. Each second: the frames shown and played, the gaps
 * between those shown, and where a frame's time went (the update, the
 * GBA's part of it, the drawing, the present, the second screen). */
#include "frame_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "emu.h"
#include "platform.h"
#include "platform_second.h"

bool platform_frame_log;

/* CYBERWORLD_FRAME_LOG: a line a second of how the frames reached the
 * display (shown, played, the gaps between shown ones), for a player's
 * pacing or lag */
/* (the frame log's split: the game's update and its drawing, summed over
 * the frames played, and the present, over those shown) */
static uint64_t part_update, part_draw, part_present;
static uint64_t part_longest;   /* (the longest frame's update and drawing together: a hitch the means hide) */
static int part_played;

void platform_frame_parts(uint64_t update, uint64_t draw) {
	part_update += update;
	part_draw += draw;
	if (update + draw > part_longest) part_longest = update + draw;
	++part_played;
}

void frame_log_present(uint64_t present) {
	part_present += present;
	static int log_on = -1;
	if (log_on < 0) log_on = platform_frame_log || getenv("CYBERWORLD_FRAME_LOG") != NULL;
	if (!log_on) return;
	static uint64_t last, second;
	static int shown, lo = 1 << 30, hi, gaps[4];
	uint64_t now = SDL_GetPerformanceCounter(), hz = SDL_GetPerformanceFrequency();
	if (last) {
		int us = (int)((now - last) * 1000000 / hz);
		if (us < lo) lo = us;
		if (us > hi) hi = us;
		++gaps[us < 12500 ? 0 : us < 20000 ? 1 : us < 30000 ? 2 : 3];
	}
	last = now;
	++shown;
	if (!second) second = now;
	if (now - second >= hz) {
		double ms = 1000.0 / (double)hz, played = part_played ? part_played : 1;
		extern uint64_t emu_core_ticks, emu_core_unshown_ticks;
		extern int emu_core_unshown;
		int drawn = part_played - emu_core_unshown;
		char bottom[160] = "";
		uint64_t second_ticks;
		int seconds = platform_second_parts(&second_ticks);
		if (seconds) snprintf(bottom, sizeof bottom, " (the bottom screen's picture %.1f ms of it, %d times)", second_ticks * ms / seconds, seconds);
		/* (and reads of the game that waited, drawing, for the next frame) */
		if (emu_draw_waits) {
			size_t k = strlen(bottom);
			snprintf(bottom + k, sizeof bottom - k, "; %d reads of the game waited while drawing", emu_draw_waits);
			emu_draw_waits = 0;
		}
		printf("frames: %d shown, %llu played, gaps %.1f-%.1f ms (<12.5: %d, <20: %d, <30: %d, more: %d)%s;"
			" a frame's update %.1f ms (the GBA %.1f drawing its picture, %.1f in %d without), drawing %.1f ms, present %.1f ms,"
			" the longest update and drawing %.1f ms%s\n",
			shown, (unsigned long long)P.frame, lo / 1000.0, hi / 1000.0, gaps[0], gaps[1], gaps[2], gaps[3], P.blend ? " smooth" : "",
			part_update * ms / played, drawn > 0 ? (emu_core_ticks - emu_core_unshown_ticks) * ms / drawn : 0.0,
			emu_core_unshown ? emu_core_unshown_ticks * ms / emu_core_unshown : 0.0, emu_core_unshown,
			part_draw * ms / played, part_present * ms / shown, part_longest * ms, bottom);
		emu_core_ticks = emu_core_unshown_ticks = 0;
		emu_core_unshown = 0;
		fflush(stdout);
		shown = 0; hi = 0; lo = 1 << 30; gaps[0] = gaps[1] = gaps[2] = gaps[3] = 0;
		part_update = part_draw = part_present = part_longest = 0;
		part_played = 0;
		second = now;
	}
}

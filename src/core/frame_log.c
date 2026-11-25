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
#ifdef __3DS__
#include "start_3ds.h"
#endif

bool platform_frame_log;

#ifdef __PSP__
static void (*heap_of)(char *out, size_t n);
void frame_log_heap_from(void (*heap)(char *out, size_t n)) { heap_of = heap; }
#endif

/* CYBERWORLD_FRAME_LOG: a line a second of how the frames reached the
 * display (shown, played, the gaps between shown ones), for a player's
 * pacing or lag */
/* (the frame log's split: the game's update and its drawing, summed over
 * the frames played, and the present, over those shown) */
static uint64_t part_update, part_draw, part_present;
static uint64_t part_longest;   /* (the longest frame's update and drawing together: a hitch the means hide) */
static int part_played;

/* (depth lighting's frames and their time, and the scale of its last) */
static uint64_t depth_ticks;
static int depth_frames, depth_scale;

void frame_log_depth(uint64_t ticks, int scale) {
	depth_ticks += ticks;
	++depth_frames;
	depth_scale = scale;
}

/* (the parts named, their time summed over the second) */
#define PARTS_MOST 64
static struct { const char *name; uint64_t ticks; } parts[PARTS_MOST];
static int nparts;

/* (a frame by itself, for the seconds whose means hide it: the last
 * frame's update and drawing, its wait for the screen's refresh inside the
 * present, and of the second the frames whose own work ran past a refresh
 * with the longest's parts, and those shown late with less work than that:
 * held up by something the main thread waited for) */
#define OWN_LONG_MS 16.0
static uint64_t last_update, last_draw, last_wait;
static int own_long, late_short;
static uint64_t worst_own, worst_update, worst_draw, worst_present;

void frame_log_part(const char *name, uint64_t ticks) {
	if (!strcmp(name, "frame wait")) last_wait += ticks;
	for (int i = 0; i < nparts; ++i)
		if (parts[i].name == name) { parts[i].ticks += ticks; return; }
	if (nparts == PARTS_MOST) return;
	parts[nparts].name = name;
	parts[nparts++].ticks = ticks;
}

/* The dearest parts, a tenth of a millisecond a frame or more, eight at most, after `out` */
static void parts_say(char *out, size_t n, double ms, double played) {
	for (int k = 0; k < 8; ++k) {
		int best = -1;
		for (int i = 0; i < nparts; ++i)
			if (parts[i].ticks && (best < 0 || parts[i].ticks > parts[best].ticks)) best = i;
		if (best < 0 || parts[best].ticks * ms / played < 0.1) break;
		size_t at = strlen(out);
		snprintf(out + at, n - at, "%s%s %.1f", k ? ", " : "; the parts a frame: ", parts[best].name, parts[best].ticks * ms / played);
		parts[best].ticks = 0;
	}
	for (int i = 0; i < nparts; ++i) parts[i].ticks = 0;
}

/* (the GPU's time a frame, and the counts named, summed over the second) */
static double gpu_ms;
static int gpu_frames;
#define COUNTS_MOST 8
static struct { const char *name; long sum; int frames; } counts[COUNTS_MOST];
static int ncounts;

#ifdef __3DS__
void frame_log_gpu(float ms) {
	gpu_ms += ms;
	++gpu_frames;
}

void frame_log_count(const char *name, int n) {
	int i = 0;
	while (i < ncounts && counts[i].name != name) ++i;
	if (i == ncounts) {
		if (ncounts == COUNTS_MOST) return;
		counts[ncounts].name = name;
		counts[ncounts].sum = 0;
		counts[ncounts++].frames = 0;
	}
	counts[i].sum += n;
	++counts[i].frames;
}
#endif

/* The GPU's mean and the counts' after `out`, and begun afresh */
static void extras_say(char *out, size_t n) {
	size_t at = strlen(out);
	if (gpu_frames) snprintf(out + at, n - at, "; the GPU %.1f ms a frame", gpu_ms / gpu_frames);
	for (int i = 0; i < ncounts; ++i) {
		at = strlen(out);
		if (counts[i].frames) snprintf(out + at, n - at, "%s %s %.1f", i ? "," : "; a frame's", counts[i].name, (double)counts[i].sum / counts[i].frames);
		counts[i].sum = 0;
		counts[i].frames = 0;
	}
	gpu_ms = 0;
	gpu_frames = 0;
}

void platform_frame_parts(uint64_t update, uint64_t draw) {
	last_update = update;
	last_draw = draw;
	part_update += update;
	part_draw += draw;
	if (update + draw > part_longest) part_longest = update + draw;
	++part_played;
}

void frame_log_drawn(void) { emu_drawing = false; }

/* A line said. On the 3DS a line printed goes to the SD card at once, on
 * the thread the frames are paced on: 11 to 29 ms a line, measured on a New
 * 3DS, so every second logged lost a frame and its sound ran dry two or
 * three times, and the log showed a stutter of its own making (issue #897).
 * There the lines are kept in memory and written where the loop is held
 * anyway: after a second with a frame of 100 ms or more in it (a layer
 * made, a start), as the room fills (two minutes of lines), and as the game
 * ends (flush, at exit). A game that stops without ending loses the lines
 * kept. Elsewhere a line is printed as it is said, for the tools that read
 * the log as it grows. */
#ifdef __3DS__
#define KEPT_BYTES (128 * 1024)
static char *kept;
static size_t nkept;

static void flush(void) {
	if (nkept) fwrite(kept, 1, nkept, stdout);
	nkept = 0;
	fflush(stdout);
}
#endif

static void say(const char *line, bool held) {
#ifdef __3DS__
	size_t n = strlen(line);
	if (!kept && (kept = malloc(KEPT_BYTES)) != NULL) atexit(flush);
	if (kept && n < KEPT_BYTES) {
		if (nkept + n > KEPT_BYTES) flush();
		memcpy(kept + nkept, line, n);
		nkept += n;
		if (held) flush();
		return;
	}
#else
	(void)held;
#endif
	fputs(line, stdout);
	fflush(stdout);
}

/* (the longest present of the second, and what the line before took to
 * say: on the 3DS written to the SD card only where say() writes) */
static uint64_t present_longest, wrote;

/* The line's end: the core's sound, the longest present, what the line
 * before took, and the reads of the game that waited while drawing */
static void tail_say(char *out, size_t n, double ms) {
	/* (the core's sound: the device's reads its ring could not fill, each
	 * a crack heard, and the least it held as a read began) */
	int dry, silence, least;
	emu_sound_dry(&dry, &silence, &least);
	if (least >= 0) {
		size_t k = strlen(out);
		if (dry) snprintf(out + k, n - k, "; the sound ran dry %d times, %d samples of silence", dry, silence);
		else snprintf(out + k, n - k, "; the sound's buffer %d samples at least", least);
	}
	{
		size_t k = strlen(out);
		snprintf(out + k, n - k, "; the longest present %.1f ms, the line before said in %.1f ms", present_longest * ms, wrote * ms);
		present_longest = 0;
	}
	{
		extern uint64_t emu_core_longest, emu_video_longest;
		size_t k = strlen(out);
		snprintf(out + k, n - k, "; %d frames' own work past %.0f ms (the longest: update %.1f, drawing %.1f, the present's work %.1f), %d shown late"
			" with less; the GBA's longest frame %.1f ms, its picture waited for %.1f ms at most", own_long, OWN_LONG_MS, worst_update * ms,
			worst_draw * ms, worst_present * ms, late_short, emu_core_longest * ms, emu_video_longest * ms);
		emu_video_longest = 0;
		own_long = late_short = 0;
		worst_own = worst_update = worst_draw = worst_present = 0;
		emu_core_longest = 0;
	}
	/* (and reads of the game that waited, drawing, for the next frame) */
	if (emu_draw_waits) {
		size_t k = strlen(out);
		snprintf(out + k, n - k, "; %d reads of the game waited while drawing%s%s", emu_draw_waits, emu_draw_waited ? ", the last in " : "",
			emu_draw_waited ? emu_draw_waited : "");
		emu_draw_waits = 0;
		emu_draw_waited = NULL;
	}
}

/* a frame shown `us` after the one before: its own work (the update, the
 * drawing and the present less its wait for the refresh) counted */
static void own_frame(uint64_t present, int us, uint64_t hz) {
	uint64_t work = present > last_wait ? present - last_wait : 0, own = last_update + last_draw + work;
	if ((double)own * 1000.0 / (double)hz >= OWN_LONG_MS) ++own_long;
	else if (us >= 30000) ++late_short;
	if (own > worst_own) { worst_own = own; worst_update = last_update; worst_draw = last_draw; worst_present = work; }
}

void frame_log_present(uint64_t present) {
	part_present += present;
	present_longest = present > present_longest ? present : present_longest;
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
		own_frame(present, us, hz);
	}
	last_wait = 0;
	last = now;
	++shown;
	if (!second) second = now;
	if (now - second >= hz) {
		double ms = 1000.0 / (double)hz, played = part_played ? part_played : 1;
		extern uint64_t emu_core_ticks, emu_core_unshown_ticks;
		extern int emu_core_unshown;
		int drawn = part_played - emu_core_unshown;
#ifdef __3DS__
		char bottom[1100] = "";
#else
		char bottom[680] = "";
#endif
		uint64_t second_ticks;
		int seconds = platform_second_parts(&second_ticks);
		if (seconds) snprintf(bottom, sizeof bottom, " (the bottom screen's picture %.1f ms of it, %d times)", second_ticks * ms / seconds, seconds);
		parts_say(bottom, sizeof bottom, ms, played);
		extras_say(bottom, sizeof bottom);
		if (depth_frames) {
			size_t k = strlen(bottom);
			snprintf(bottom + k, sizeof bottom - k, "; depth lighting %.1f ms at %dx", depth_ticks * ms / depth_frames, depth_scale);
			depth_ticks = 0;
			depth_frames = 0;
		}
		tail_say(bottom, sizeof bottom, ms);
#ifdef __3DS__
		/* (and the heap, whose high-water mark a layer's learning raises:
		 * docs/engine/emulation/3ds-memory.md) */
		char heap[120];
		start_3ds_heap(heap, sizeof heap);
		size_t at = strlen(bottom);
		snprintf(bottom + at, sizeof bottom - at, "; %s", heap);
#elif defined(__PSP__)
		if (heap_of) {
			char heap[120];
			heap_of(heap, sizeof heap);
			size_t at = strlen(bottom);
			snprintf(bottom + at, sizeof bottom - at, "; %s", heap);
		}
#endif
		uint64_t write0 = SDL_GetPerformanceCounter();
		char line[1600];
		snprintf(line, sizeof line, "frames: %d shown, %llu played, gaps %.1f-%.1f ms (<12.5: %d, <20: %d, <30: %d, more: %d)%s;"
			" a frame's update %.1f ms (the GBA %.1f drawing its picture, %.1f in %d without), drawing %.1f ms, present %.1f ms,"
			" the longest update and drawing %.1f ms%s\n",
			shown, (unsigned long long)P.frame, lo / 1000.0, hi / 1000.0, gaps[0], gaps[1], gaps[2], gaps[3], P.blend ? " smooth" : "",
			part_update * ms / played, drawn > 0 ? (emu_core_ticks - emu_core_unshown_ticks) * ms / drawn : 0.0,
			emu_core_unshown ? emu_core_unshown_ticks * ms / emu_core_unshown : 0.0, emu_core_unshown,
			part_draw * ms / played, part_present * ms / shown, part_longest * ms, bottom);
		emu_core_ticks = emu_core_unshown_ticks = 0;
		emu_core_unshown = 0;
		say(line, part_longest * ms >= 100.0);
		wrote = SDL_GetPerformanceCounter() - write0;
		shown = 0; hi = 0; lo = 1 << 30; gaps[0] = gaps[1] = gaps[2] = gaps[3] = 0;
		part_update = part_draw = part_present = part_longest = 0;
		part_played = 0;
		second = now;
	}
}

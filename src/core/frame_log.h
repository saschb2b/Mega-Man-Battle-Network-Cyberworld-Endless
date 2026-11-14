/* The frame log (frame_log.c): --frame-log, settings.ini's frame_log or
 * CYBERWORLD_FRAME_LOG, a line a second of how the frames reached the
 * display, for a player's report of pacing or lag. platform.h declares the
 * switch (platform_frame_log) and a played frame's parts. */
#ifndef CW_FRAME_LOG_H
#define CW_FRAME_LOG_H

#include <stddef.h>
#include <stdint.h>

#ifdef __PSP__
/* The PSP's heap in words for each line ("heap N KB in use, ..."), given
 * by its start (start_psp.c), which this layer may not name */
void frame_log_heap_from(void (*heap)(char *out, size_t n));
#endif

/* Depth lighting's part of a frame (docs/engine/voxel.md, Its cost):
 * `ticks` performance-counter ticks at the picture's `scale` */
void frame_log_depth(uint64_t ticks, int scale);

/* A part of a frame's work (`name` a string that lives as long as the
 * game: its address is its key), `ticks` performance-counter ticks long:
 * the line names the dearest parts' means a frame, for finding what keeps
 * a system from its 60 frames (docs/devtools/frame-log.md) */
void frame_log_part(const char *name, uint64_t ticks);

/* A frame shown, its present `present` performance-counter ticks long
 * (0 where it was not measured): the line, once a second */
void frame_log_present(uint64_t present);

#endif

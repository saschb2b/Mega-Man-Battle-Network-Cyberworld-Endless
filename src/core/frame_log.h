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

/* A frame shown, its present `present` performance-counter ticks long
 * (0 where it was not measured): the line, once a second */
void frame_log_present(uint64_t present);

#endif

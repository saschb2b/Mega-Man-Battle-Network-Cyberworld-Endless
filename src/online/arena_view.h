/* The Arena on the screen (issue #120): my core's picture whole in the
 * canvas's game rectangle, a note over its foot (a heading and words
 * broken to fit), the way in's progress, and the way in run at the
 * frame's pace (a slice of frames a tick without a window, so that a
 * capture's frames are the same on every machine; a time budget in one).
 * --dev arena's scene and the Net Square's battles share it. */
#ifndef CW_ARENA_VIEW_H
#define CW_ARENA_VIEW_H

#include <stdbool.h>
#include <stdint.h>

#include "arena.h"

/* A tick of the way in */
void arena_view_connect(Arena *a);
void arena_view_core(const uint32_t *video);
/* `line` in white, `second` (NULL: none) broken into lines under it */
void arena_view_note(const char *line, const char *second);
/* The way in's picture: the title, "connecting" and its bar at `progress` (0-100) */
void arena_view_connecting(int progress);
/* The screen cleared to black, before the rest */
void arena_view_clear(void);
/* The battle's end (`result`: 1 won, 2 lost, else over), and the way back
 * (`to_square`: to the Net Square, else to the title) */
void arena_view_result(unsigned result, bool to_square);
/* The battle stopped, and why; the peer awaited */
void arena_view_stopped(const char *why);
void arena_view_waiting(void);
/* (the picture's texture, given back as the scene leaves) */
void arena_view_free(void);

#endif

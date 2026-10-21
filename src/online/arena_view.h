/* The Arena on the screen (issue #120): my core's picture whole in the
 * canvas's game rectangle, a note over its foot (a heading and words
 * broken to fit), over the battle the other's name plate on the Custom
 * screen, the link's badge and a stall's note (issue #167), and the way
 * in run at the frame's pace (a slice of frames a tick without a window,
 * so that a capture's frames are the same on every machine; a time
 * budget in one). --dev arena's scene and the Net Square's battles share
 * it; the way in's panel and the result card are arena_card.h's. */
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
/* The screen cleared to black, before the rest */
void arena_view_clear(void);
/* Over the battle: the other's `name` on a small plate below BN6's own
 * label, while the Custom screen is open (`custom`), the link's badge
 * beside it (bars alone: three steady, two rolling back, one blinking
 * stalled; at the corner once the Custom screen has closed); `t` the
 * frames, for the blink */
void arena_view_over(const char *name, bool custom, ArenaLinkState link, uint32_t t);
/* A stall's note over the picture's foot: whose PET the battle waits for,
 * a bar running down to the wait's end (`left` of `total`), and once it
 * has (`way_out`) the key that leaves */
void arena_view_stall(const char *name, uint32_t left, uint32_t total, bool way_out);
/* (the picture's texture, given back as the scene leaves) */
void arena_view_free(void);

#endif

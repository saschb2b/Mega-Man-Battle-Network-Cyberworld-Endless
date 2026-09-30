/* The 3DS's present (issue #9, 3ds/): the game draws its canvas into
 * memory the GPU reads, and the GPU puts it on the top screen, whole or
 * filling the screen's height. Empty on every other target. */
#ifndef PRESENT_3DS_H
#define PRESENT_3DS_H

#include <stdbool.h>

#include <SDL.h>

/* After SDL's video starts: a surface of w x h (at most 256 x 256) for the
 * software renderer to draw into, or NULL when the GPU did not start. */
SDL_Surface *present3ds_init(int w, int h);
/* The surface's picture on the top screen: at 1x in the middle, or at the
 * screen's height (1.5x) with its pixels mixed at their edges. */
void present3ds_frame(bool fill);
void present3ds_exit(void);
/* The bottom screen's picture (issue #9): SECOND_W x SECOND_H pixels,
 * RGBA8888, `pitch` bytes a row, in memory the GPU reads; NULL where it has
 * none. present3ds_bottom_show(true) puts it on the screen at the next
 * frame, false leaves the screen black. */
void *present3ds_bottom(int *pitch);
void present3ds_bottom_show(bool on);

#endif

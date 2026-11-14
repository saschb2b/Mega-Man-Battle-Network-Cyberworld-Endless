/* The 3DS's present (issue #9, 3ds/): the game draws its canvas into
 * memory the GPU reads, and the GPU puts it on the top screen, whole,
 * filling the screen's height, or stretched over the whole of it. Empty on every other target. */
#ifndef PRESENT_3DS_H
#define PRESENT_3DS_H

#include <stdbool.h>

#include <SDL.h>

/* After SDL's video starts: a surface of w x h (at most 256 x 256) for the
 * software renderer to draw into, or NULL when the GPU did not start. */
SDL_Surface *present3ds_init(int w, int h);
/* The surface's picture on the top screen: at 1x in the middle, or at the
 * screen's height (1.5x) with its pixels mixed at their edges, `stretch`ed
 * over its width too (display.h); with `sync`, at the screen's next
 * refresh, the frame paced by it. */
void present3ds_frame(bool fill, bool stretch, bool sync);
/* The canvas's surface (RGBA8888 in memory the GPU reads), NULL before the
 * present starts: for a picture written into it straight */
SDL_Surface *present3ds_canvas(void);
void present3ds_exit(void);
/* The bottom screen's picture (issue #9): SECOND_W x SECOND_H pixels,
 * RGBA8888, `pitch` bytes a row, in memory the GPU reads; NULL where it has
 * none. present3ds_bottom_show(true) puts it on the screen at the next
 * frame, false leaves the screen black. */
void *present3ds_bottom(int *pitch);
void present3ds_bottom_show(bool on);

/* The 3D screen (docs/engine/stereo.md), with Options' 3D depth on: each
 * eye's picture is the canvas's own, drawn by the GPU a few rows at a
 * time, each row's spans of the world moved by `shift` (the right eye's,
 * in the canvas's pixels; the left's the other way) and its flat spans
 * where they are. A row's span i runs from x[i] to x[i + 1] (the last to
 * the canvas's edge). */
#define PRESENT3DS_SPANS 8
typedef struct {
	uint8_t n;
	int16_t x[PRESENT3DS_SPANS];
	uint8_t flat[PRESENT3DS_SPANS];
} Present3dsRow;
typedef struct {
	Present3dsRow row[160];
	float shift[160];
} Present3dsStereo;
/* What says each frame's spans, false where the frame is to be flat in both eyes */
void present3ds_stereo_from(bool (*make)(Present3dsStereo *out));
/* The 3D slider, 0 to 1 (0 where the screen is not in 3D: Options' 3D depth off) */
float present3ds_slider(void);

#endif

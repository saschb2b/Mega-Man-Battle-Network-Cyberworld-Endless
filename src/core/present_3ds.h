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
void present3ds_exit(void);
/* The bottom screen's picture (issue #9): SECOND_W x SECOND_H pixels,
 * RGBA8888, `pitch` bytes a row, in memory the GPU reads; NULL where it has
 * none. present3ds_bottom_show(true) puts it on the screen at the next
 * frame, false leaves the screen black. */
void *present3ds_bottom(int *pitch);
void present3ds_bottom_show(bool on);

/* The 3D screen (docs/engine/stereo.md), with Options' 3D depth on: the
 * frame in three layers of the canvas's size (the world, the bodies that
 * stand, what stays flat), each eye's picture drawn from them, the world's
 * rows and the bodies moved apart by their depth. `shift` is the right
 * eye's, in the canvas's pixels (the left's the other way); a body is its
 * box on the canvas (x0, y0, x1, y1), drawn in their order. */
#define PRESENT3DS_BODIES 48
typedef struct {
	float row_shift[160];
	int16_t box[PRESENT3DS_BODIES][4];
	float box_shift[PRESENT3DS_BODIES];
	int nbodies;
} Present3dsStereo;
/* Layer n's memory (0 the world, 1 the bodies, 2 the flat), `pitch`
 * pixels a row, the canvas's colours (0 clear); NULL where it has none */
uint32_t *present3ds_layer(int n, int *pitch);
/* What makes each frame's layers from the canvas, false where the frame
 * is to be flat in both eyes */
void present3ds_stereo_from(bool (*make)(const uint32_t *canvas, int pitch, Present3dsStereo *out));
/* The 3D slider, 0 to 1 (0 where the screen is not in 3D: Options' 3D depth off) */
float present3ds_slider(void);

#endif

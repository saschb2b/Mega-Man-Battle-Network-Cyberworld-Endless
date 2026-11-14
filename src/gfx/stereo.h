/* The 3DS's 3D screen (docs/engine/stereo.md): the frame parted by depth.
 * The world (the map's backgrounds and what lies on its floor) goes back
 * into the screen row by row, its lowest row at the screen and its top the
 * deepest; a sprite that stands (joined with those it touches into a
 * body) stands at the depth of its feet; the HUD, an open chat box and
 * whatever the engine draws over the game stay flat at the screen. Nothing
 * comes out of it, so the HUD is never behind the world. The colours are
 * the frame's own: nothing of the game's is kept. */
#ifndef CW_STEREO_H
#define CW_STEREO_H

#include <stdbool.h>
#include <stdint.h>

#include "gba_layers.h"

#define STEREO_W 240
#define STEREO_H 160
#define STEREO_N (STEREO_W * STEREO_H)
#define STEREO_BODIES 48

/* What a pixel is to the 3D screen: the world's, flat, or a body's
 * (STEREO_BODY + its number, from 0) */
enum { STEREO_WORLD = 0, STEREO_FLAT = 1, STEREO_BODY = 2 };

/* A body's pixels' box, its last column and row x1 and y1: its feet stand on y1 */
typedef struct { int16_t x0, y0, x1, y1; } StereoBox;

typedef struct {
	uint8_t what[STEREO_N];
	StereoBox body[STEREO_BODIES];
	int nbodies;
} StereoFrame;

/* The frame sorted from its video memory and its sprites (gba_layers_sprites).
 * `hud_rows`: the rows whose sprites are the HUD's, as voxel_lift's (an
 * open chat box's from that row down, or, negative, that many from the top). */
void stereo_sort(const GbaVideo *v, const GbaLayers *l, int hud_rows, StereoFrame *out);

/* How deep a row of the frame lies, from 0 (its last row, at the screen) to 1 (its first) */
float stereo_depth(int row);
/* ... and where an eye (-1 the left, +1 the right) sees what lies at it,
 * in pixels from where the frame has it, `sep` the two eyes' parting at
 * the deepest */
float stereo_shift(int eye, float sep, int row);

/* The three layers drawn at their depths, each `pitch` pixels a row, the
 * colours as the canvas has them (0: none). `canvas` is the screen's
 * picture with the engine's drawing over the game's, `picture` the game's
 * alone, the frame's (0, 0) at the canvas's (`x`, `y`): a pixel of the
 * canvas that differs from the game's is the engine's, flat. Where a body
 * or a flat pixel stood, the world takes the colour beside it (its eyes
 * see a little round them). `row_world`: 1 for a row of the canvas with
 * any of the world in it. */
typedef struct {
	uint32_t *world, *bodies, *flat;
	int pitch;
	uint32_t alpha;   /* the colours' alpha bits: a clear pixel has none (its colour may be its neighbour's) */
	int x, y;         /* the frame's (0, 0) on the canvas */
	uint8_t row_world[STEREO_H];
} StereoLayers;
void stereo_layers(const StereoFrame *f, const uint32_t *canvas, int canvas_pitch, const uint32_t *picture, StereoLayers *out);

/* One eye's picture of the layers (240x160 at `scale`, nearest), as the
 * 3DS's GPU draws it: for a look at the depth on a system without a 3D
 * screen (CYBERWORLD_STEREO_SHOT) and the tests */
void stereo_compose(const StereoLayers *l, const StereoFrame *f, int eye, float sep, int scale, uint32_t *out);

#endif

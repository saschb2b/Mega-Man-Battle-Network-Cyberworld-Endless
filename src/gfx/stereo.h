/* The 3DS's 3D screen (docs/engine/stereo.md): the frame parted by depth.
 * The world goes back into the screen row by row, its last row at the
 * screen and its first the deepest, as the floor goes back under BN6's
 * camera; what stands on it goes back with its rows (a sprite's head under
 * a pixel deeper than its feet at the most). The HUD stays flat at the
 * screen: the sprites in the HP's corner, in an open chat box's rows and
 * in a battle's top rows, and the tiles of BG0 (the chat's text) and of a
 * BG3 in front of the map (a battle's Custom screen). Nothing comes out of
 * the screen, so the HUD is never behind the world. Only which parts of a
 * row are flat is worked out here, from the sprites' table and two
 * backgrounds' maps: each eye's picture is the frame's own, its rows'
 * spans moved apart (by the 3DS's GPU, or stereo_compose). Nothing of the
 * game's is kept. */
#ifndef CW_STEREO_H
#define CW_STEREO_H

#include <stdbool.h>
#include <stdint.h>

#include "gba_layers.h"

#define STEREO_W 240
#define STEREO_H 160
#define STEREO_N (STEREO_W * STEREO_H)
#define STEREO_RECTS 64
#define STEREO_SPANS 8

/* What of the frame is flat: the HUD's sprites' boxes (x1 and y1 past
 * their last), and the frame's 8x8 cells a HUD background has a pixel in */
typedef struct {
	int nrects;
	struct { int16_t x0, y0, x1, y1; } rect[STEREO_RECTS];
	uint8_t cell[STEREO_H / 8][STEREO_W / 8];
} StereoFlat;

/* ... read from the frame's video memory (OAM, the IO registers and VRAM's
 * backgrounds; v->pal unused). `hud_rows`: the rows whose sprites are the
 * HUD's, as voxel_lift's (an open chat box's from that row down, or,
 * negative, that many from the top). */
void stereo_flat(const GbaVideo *v, int hud_rows, StereoFlat *out);

/* A row's spans, left to right: span i from x[i] to x[i + 1] (the last to
 * the frame's edge), flat or the world's; at most STEREO_SPANS (what is
 * past them flat) */
typedef struct {
	uint8_t n;
	int16_t x[STEREO_SPANS];
	uint8_t flat[STEREO_SPANS];
} StereoRow;
void stereo_row(const StereoFlat *f, int y, StereoRow *out);

/* How deep a row of the frame lies, from 0 (its last row, at the screen) to 1 (its first) */
float stereo_depth(int row);
/* ... and how far an eye (-1 the left, +1 the right) sees the world's
 * pixels of the row moved, `sep` the two eyes' parting at the deepest */
float stereo_shift(int eye, float sep, int row);

/* One eye's picture (240x160 at `scale`, nearest) of the frame's own
 * `picture` (its rows STEREO_W apart), drawn as the 3DS's GPU draws it:
 * each row's world spans moved, the gap a span leaves beside a flat one
 * filled with its edge's colour, the flat spans unmoved. For a look at the
 * depth on a system without a 3D screen (CYBERWORLD_STEREO_SHOT) and the
 * tests. */
void stereo_compose(const uint32_t *picture, const StereoFlat *f, int eye, float sep, int scale, uint32_t *out);

#endif

/* The 3DS's 3D screen (docs/engine/stereo.md): the frame's parts at depths
 * of their own. The moving background (BG3 behind the map, which mGBA's
 * picture tags pixel for pixel) lies far behind, flat; the map is a floor
 * tilted back between it and the screen, its last row the nearest; what
 * stands on it (the sprites joined into bodies where they touch) stands
 * upright at the depth of the floor at its feet; the HUD stays flat at the
 * screen (the sprites in the HP's corner, in an open chat box's rows and in
 * a battle's top rows, BG0's tiles and a BG3 in front of the map). Nothing
 * comes out of the screen, so the HUD is never behind the world. Each
 * eye's picture is the frame's own, its parts moved apart (by the 3DS's
 * GPU, or stereo_compose). Nothing of the game's is kept. */
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
	/* a frame a quarter of whose cells or more are the HUD's backgrounds'
	 * (a battle's Custom screen, a result screen): a screen of menus, flat
	 * whole (the chips' sprites over its window moved with the field's
	 * rows behind it, and its spans made anew each frame cost the 3DS its
	 * 60 frames) */
	bool whole;
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

/* What stands: its pixels (`mask`, 1 a standing sprite's) and its bodies,
 * the boxes of the sprites that showed joined where they touch (x1 and y1
 * past their last), from the sprites' table and their tiles; the HUD's
 * sprites (as stereo_flat's) left out */
#define STEREO_BODIES 48
typedef struct { int16_t x0, y0, x1, y1; } StereoBox;
typedef struct {
	int n;
	StereoBox box[STEREO_BODIES];
} StereoBodies;
void stereo_sprites(const GbaVideo *v, int hud_rows, uint8_t *mask, StereoBodies *out);

/* A pixel's part: the map's (and what lies on it), the moving background's, a standing sprite's */
enum { STEREO_MAP, STEREO_SKY, STEREO_SPRITE };

/* How far up the frame a row lies, from 0 (its last row) to 1 (its first) */
float stereo_depth(int row);
/* The depths, from 0 (at the screen) to 1 (the deepest): the moving
 * background's and the map's at a row, each tilted back by its rows, the
 * background behind the map at every row, and a body's (the map's at its
 * feet) */
float stereo_sky_depth(int row);
float stereo_map_depth(int row);
float stereo_body_depth(const StereoBox *b);
/* ... and how far an eye (-1 the left, +1 the right) sees what lies at a
 * depth moved, `sep` the two eyes' parting at the deepest */
float stereo_shift(int eye, float sep, float depth);

/* One eye's picture (240x160 at `scale`, nearest) of the frame, drawn as
 * the 3DS's GPU draws it: the background at its rows' depth (where `sky`),
 * the map at its rows' (the gap a part leaves filled from beside it), each
 * body at its own, the flat spans unmoved. For a look at the depth on a
 * system without a 3D screen (CYBERWORLD_STEREO_SHOT) and the tests. */
typedef struct {
	const uint32_t *picture;   /* the frame's colours, its rows STEREO_W apart */
	const uint8_t *part;       /* each pixel's part (STEREO_MAP, ...) */
	const StereoFlat *flat;
	const StereoBodies *bodies;
	bool sky;                  /* the background behind (a layer's, a battle's) */
} StereoPicture;
void stereo_compose(const StereoPicture *p, int eye, float sep, int scale, uint32_t *out);

#endif

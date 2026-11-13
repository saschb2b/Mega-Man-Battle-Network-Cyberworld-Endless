/* Voxel mode (docs/engine/voxel.md): the net's picture rebuilt as columns
 * under a camera and a sun. The map's art (gba_world.h, more of it than
 * the frame holds) and the frame's sprites are sorted by what they show (a
 * floor's top, a slab's side, a sprite standing on it, the sky behind) and
 * given their place in the world from the layer's own floor; each pixel is
 * then a face of a column, lit (cast shadows, shaded sides) and drawn
 * through a camera that can lean lower than BN6's, at the screen's own
 * scale, with a soft sky, light blooming off the bright panels and what
 * is far from MegaMan out of focus. Numbers only: the colours are the
 * frame's, nothing of the game's is kept. Off by default (settings.ini's
 * voxel). */
#ifndef CW_VOXEL_H
#define CW_VOXEL_H

#include <stdbool.h>
#include <stdint.h>

#include "gba_layers.h"

#define VOX_W 240
#define VOX_H 160
#define VOX_N (VOX_W * VOX_H)

/* The world's window on the frame's lattice (column x, row y, the frame's
 * own pixel (0, 0) at its (VOX_LEFT, VOX_UP)): the frame and what lies
 * about it, which a lower camera brings in */
#define VOX_LEFT   96
#define VOX_UP     144
#define VOX_DOWN   64
#define VOX_WW     (VOX_W + 2 * VOX_LEFT)
#define VOX_WH     (VOX_H + VOX_UP + VOX_DOWN)
#define VOX_WN     (VOX_WW * VOX_WH)

/* What a pixel of the map shows */
typedef enum { VOX_SKY, VOX_TOP, VOX_WALL } VoxKind;

/* The floor over the lattice by ground row gy, the row a point of the
 * world shows in were it at height 0 (one h high shows h rows above its
 * ground row): each level's panels, over the window and a margin about it */
#define VOX_MARGIN  48
#define VOX_GW      (VOX_WW + 2 * VOX_MARGIN)
#define VOX_GH      (VOX_WH + 4 * VOX_MARGIN)
#define VOX_LEVELS  2
typedef struct {
	uint8_t floor[VOX_GW * VOX_GH];   /* bit L: level L has floor at window (x, gy) + VOX_MARGIN */
	uint8_t ramp[VOX_GW * VOX_GH];    /* a stair there: its height in rows, plus 1 (0 none) */
	int z[VOX_LEVELS];                /* each level's height in rows */
} VoxGround;

/* (x, gy) in the frame's own coordinates */
static inline unsigned vox_floor(const VoxGround *g, int x, int gy) {
	x += VOX_LEFT + VOX_MARGIN;
	gy += VOX_UP + VOX_MARGIN;
	return x < 0 || gy < 0 || x >= VOX_GW || gy >= VOX_GH ? 0u : g->floor[gy * VOX_GW + x];
}

static inline int vox_ramp(const VoxGround *g, int x, int gy) {
	x += VOX_LEFT + VOX_MARGIN;
	gy += VOX_UP + VOX_MARGIN;
	return x < 0 || gy < 0 || x >= VOX_GW || gy >= VOX_GH ? 0 : g->ramp[gy * VOX_GW + x];
}

/* A sprite as it stands: its pixels' box in the frame, the ground row of
 * its feet and the floor's height there */
#define VOX_BODIES 48
typedef struct { int x0, y0, x1, y1, gy, z; } VoxBody;

typedef struct {
	/* the window: the map's colour (0 none) and the sky's behind it, each
	 * map pixel's kind and ground row (a top's own, a side's the edge it
	 * hangs from) */
	uint32_t map[VOX_WN], sky[VOX_WN];
	uint8_t kind[VOX_WN];
	int16_t gy[VOX_WN];
	/* the frame: its sprites' colours (0 none), each one's body (0 none),
	 * and 1 where the HUD is (the canvas keeps it) */
	uint32_t sprite[VOX_N];
	uint8_t body[VOX_N], hud[VOX_N];
	VoxBody bodies[VOX_BODIES + 1];
	int nbodies;
} VoxScene;

/* `scene`'s map sorted and placed over `ground`, and the frame's sprites
 * taken from `frame` (its colours as shown) by `layers` (gba_layers.h).
 * `hud_rows`: the rows whose sprites are the HUD's, from that row down (a
 * chat box open: 96) or, negative, that many from the top (a battle); 0
 * none. A sprite on BG0 is the HUD's wherever it is. */
void voxel_lift(const GbaLayers *layers, int hud_rows, const uint32_t *frame, const VoxGround *ground, VoxScene *scene);

/* What the sun and the lens are set to */
typedef struct {
	float sun_x, sun_g, sun_h;   /* a step toward the sun: columns left, ground rows nearer, rows up */
	float ambient;               /* a shadow's light, of 1 */
	float wall;                  /* a side's light */
	float bloom;                 /* how much the bright panels glow */
	float focus;                 /* how fast what is nearer or farther than MegaMan blurs (0 none) */
	float sky;                   /* the sky's light */
	float bevel;                 /* each voxel's edges, lit and shaded */
	int debug;                   /* 1: the kinds as flat colours */
} VoxLook;
VoxLook voxel_look_default(void);

/* The scene under the sun: each map pixel's and each sprite pixel's
 * colour as lit (0xFFBBGGRR, 0 none), VOX_WN and VOX_N of them. */
void voxel_light(const VoxScene *scene, const VoxGround *ground, const VoxLook *look, uint32_t *map, uint32_t *sprite);

/* The camera: BN6's looks down at 30 degrees, a ground row a row of the
 * screen. `pitch` its angle here (lower: the floor's rows closer, what
 * stands taller), `zoom` its scale, `depth` how much what is near grows
 * over what is far (0: none, as BN6); the point (x, gy, h) of the lattice
 * stays where BN6 shows it. */
typedef struct { float pitch, zoom, depth; float x, gy, h; } VoxCamera;

/* The picture's last passes run over bands of its rows: `run` calls
 * `band(from, to, ctx)` for bands that together are rows 0 to `rows`, on
 * as many threads as it likes (NULL: one band, here). */
typedef void (*VoxBand)(int from, int to, void *ctx);
void voxel_bands(void (*run)(VoxBand band, int rows, void *ctx));
/* ... `band` over rows 0 to `rows` by it */
void voxel_in_bands(VoxBand band, int rows, void *ctx);

/* The lit scene through `cam` at `scale` (1-8) into `out`, VOX_W * scale
 * wide and VOX_H * scale high, 0xFFBBGGRR. False where it has no memory. */
bool voxel_draw(const VoxScene *scene, const uint32_t *map, const uint32_t *sprite, const VoxLook *look, const VoxCamera *cam, int scale, uint32_t *out);

#endif

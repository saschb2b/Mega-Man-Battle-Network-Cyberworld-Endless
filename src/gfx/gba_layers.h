/* Which of the GBA's layers shows at each pixel of a frame, read from its
 * video memory as the frame ends: the four backgrounds (mode 0's tile
 * maps) and the sprites (OAM), by their priorities. mGBA's own picture
 * keeps a pixel's layer only where nothing blends (BN6's glass panels
 * do), so voxel mode (voxel.h) asks here. Its scroll and controls are the
 * frame's last: a game that changes them down the screen is not followed,
 * nor are windows and mosaic. */
#ifndef CW_GBA_LAYERS_H
#define CW_GBA_LAYERS_H

#include <stdbool.h>
#include <stdint.h>

#define GBA_LAYER_OBJ   4
#define GBA_LAYER_NONE  5
#define GBA_OBJS        128

typedef struct {
	const uint8_t *vram;   /* 0x06000000, 0x18000 bytes */
	const uint8_t *oam;    /* 0x07000000, 0x400 */
	const uint8_t *io;     /* 0x04000000, 0x60 */
	const uint8_t *pal;    /* 0x05000000, 0x400 (NULL: no colours asked for) */
} GbaVideo;

typedef struct {
	uint8_t top[240 * 160];     /* the layer on top: a background's number, GBA_LAYER_OBJ or GBA_LAYER_NONE */
	uint8_t under[240 * 160];   /* the background on top of the others, sprites aside (or GBA_LAYER_NONE) */
	uint8_t obj[240 * 160];     /* the sprite on top of the others, its OAM entry + 1 (0 none), whatever covers it */
	uint8_t hud[240 * 160];     /* 1 where a background of the HUD lies under the sprites: BG0, or BG3 in front (gba_bg3_front) */
	uint8_t sheer[240 * 160];   /* 1 where that sprite is a blended one (a shadow, a light) */
	uint8_t prio[240 * 160];    /* that sprite's priority */
	/* the sprites BN6 lays on the floor under everyone, those of the last
	 * priority (3: a warp pad) and the blobs under who stands (one colour,
	 * wider than high): the colour of the first of them at each pixel, 0xFFBBGGRR (0 none), whatever stands on it; they are no
	 * part of `top` and `obj` */
	uint32_t lying[240 * 160];
	/* each sprite's box on the screen (w 0: not shown) */
	int16_t x[GBA_OBJS], y[GBA_OBJS];
	uint8_t w[GBA_OBJS], h[GBA_OBJS];
} GbaLayers;

/* Whether BG3 stands in front of the map's two backgrounds (BG1, BG2) by
 * its priority: a window of the HUD then (a battle's Custom screen), not
 * the sky behind the world (a net map's) */
bool gba_bg3_front(const uint8_t *io);

/* A sprite's box on the screen as OAM sets it (w 0: not shown) */
typedef struct { int16_t x, y; uint8_t w, h; } GbaBox;
/* ... each of the 128, their pixels unread (v->oam alone read) */
void gba_obj_boxes(const GbaVideo *v, GbaBox box[GBA_OBJS]);
/* The pixels of the sprites that stand, of those `want` names (1 each):
 * 1 in `mask` (240x160, cleared first) where one shows, `drew` 1 for each
 * that showed any. One that lies on the floor (priority 3, or a blob) and
 * a blended one (a shadow, a light) are left out; whatever covers them is
 * not asked. Only the entries wanted are read, a pixel at a time. */
void gba_obj_pixels(const GbaVideo *v, const uint8_t want[GBA_OBJS], uint8_t *mask, uint8_t drew[GBA_OBJS]);

/* False in a mode other than 0 (its backgrounds are not tile maps alone). */
bool gba_layers(const GbaVideo *v, GbaLayers *out);
/* ... in two steps: the sprites, then the backgrounds' rows `from` to `to`
 * (each band of rows by itself, on threads if a caller likes) */
bool gba_layers_sprites(const GbaVideo *v, GbaLayers *out);
void gba_layers_rows(const GbaVideo *v, GbaLayers *out, int from, int to);

#endif

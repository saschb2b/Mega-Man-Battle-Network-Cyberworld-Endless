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
} GbaVideo;

typedef struct {
	uint8_t top[240 * 160];     /* the layer on top: a background's number, GBA_LAYER_OBJ or GBA_LAYER_NONE */
	uint8_t under[240 * 160];   /* the background on top of the others, sprites aside (or GBA_LAYER_NONE) */
	uint8_t obj[240 * 160];     /* the sprite on top of the others, its OAM entry + 1 (0 none), whatever covers it */
	uint8_t sheer[240 * 160];   /* 1 where that sprite is a blended one (a shadow, a light) */
	/* each sprite's box on the screen (w 0: not shown) */
	int16_t x[GBA_OBJS], y[GBA_OBJS];
	uint8_t w[GBA_OBJS], h[GBA_OBJS];
} GbaLayers;

/* False in a mode other than 0 (its backgrounds are not tile maps alone). */
bool gba_layers(const GbaVideo *v, GbaLayers *out);

#endif

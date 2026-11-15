/* The map's picture beyond the frame: BN6's two map backgrounds (BG1 and
 * BG2) drawn from the map's whole tile map, and what lies behind them
 * (BG3, wrapped as the GBA wraps it, or the backdrop), over any window of
 * the frame's lattice. The colours are the palette's as mGBA shows them,
 * a fade (BLDY) on them; nothing here is kept past the frame. For voxel
 * mode (voxel.h), which draws more of the world than the frame holds. */
#ifndef CW_GBA_WORLD_H
#define CW_GBA_WORLD_H

#include <stdint.h>

typedef struct {
	const uint8_t *vram;      /* 0x06000000, 0x18000 bytes */
	const uint8_t *pal;       /* 0x05000000, 0x400 */
	const uint8_t *io;        /* 0x04000000, 0x60 */
	const uint16_t *map[2];   /* BG1's and BG2's tile maps, tw x th screen entries each (NULL: none, the frame alone
	                           * from their screen maps) */
	int tw, th;
	int ox, oy;               /* the map's pixel at the frame's (0, 0) */
	const uint32_t *lut;      /* each of the 32768 colours as the window shows it (the screen's correction, a tint), 0xFFBBGGRR;
	                           * NULL: as mGBA shows them (gba_shown) */
} GbaWorld;

/* 15 bits as mGBA shows them, 0xFFBBGGRR (5 bits a channel, its top bits again below) */
uint32_t gba_shown(unsigned c);

/* A window w x h whose corner is the frame's (x0, y0): `map` the map's
 * colour where BG1 or BG2 has a pixel (0xFFBBGGRR, else 0), `sky` what is
 * behind it, their rows `stride` pixels apart */
typedef struct {
	int x0, y0, w, h, stride;
	uint32_t *map, *sky;
} GbaWindow;

void gba_world(const GbaWorld *g, const GbaWindow *win);

#endif

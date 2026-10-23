/* Another game's 4bpp tiles drawn on the canvas: its ROM's bytes read as
 * data (or bytes made from them as the game makes them), in sixteen of its
 * colours, BN5's Liberation pieces (src/director/liberation_pieces.c).
 * Nothing is kept but the textures made of them as the game runs. */
#ifndef CW_XTILES_H
#define CW_XTILES_H

#include <stdint.h>

/* A block of w x h tiles in an OBJ's order (row by row, 32 bytes a tile),
 * in palette `pal` (16 BGR555 colours, colour 0 clear), at canvas (x, y);
 * flip: bit 0 across, bit 1 down. Kept as a texture by its bytes. */
void xtiles_draw(const uint8_t *tiles, const uint8_t *pal, int x, int y, int w, int h, int flip);
/* ... as an affine OBJ that is not double-sized scales it: its height by
 * 256 / pd about its middle, kept within its own (BN5's banners' squash) */
void xtiles_draw_squashed(const uint8_t *tiles, const uint8_t *pal, int x, int y, int w, int h, int pd);

/* A rectangle of one of its colours (BGR555) blended over the picture as
 * the GBA blends a first target over a second: the colour times eva/16
 * over what lies beneath times evb/16 (BN5's bands, BLDALPHA's 16 and 8) */
void xtiles_blend_rect(int x, int y, int w, int h, uint16_t colour, int eva, int evb);

#endif

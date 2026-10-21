/* A pad as it is drawn: where MegaMan stands on it to the eye, which BN6's
 * trigger cells behind the art need not match (docs/ROM_DATA.md, Lan's HP
 * and Pads as drawn). One zone for every pad: Lan's HP's pink pad and
 * link squares, a layer's exit pad, the Net Square's pad home. */
#ifndef CW_PAD_ART_H
#define CW_PAD_ART_H

#include <stdbool.h>

/* A pad's art: its middle in world units, and how far it reaches across
 * (half its width) and down (half its height) the screen, in pixels; a
 * world step (dx, dy) goes dx + dy across and (dy - dx) / 2 down. */
typedef struct { int x, y, across, down; } PadArt;

/* The pads' art as measured, their middle at their place: a layer's exit
 * pad and a way back's (sprite list 7's 0x22 and 0x81) a ring 56 by 30,
 * BN6's link marker (list 7's 0x88) a diamond 56 by 28. */
#define PAD_RING_ACROSS 28
#define PAD_RING_DOWN 15
#define PAD_MARK_ACROSS 28
#define PAD_MARK_DOWN 14

/* Whether MegaMan's feet at world (x, y) stand on pad `p` as drawn, or
 * within `reach` pixels of it: on its art, his shoes' width round it, or
 * in front of it as far as his sprite, standing there, covers it. */
bool pad_art_on(const PadArt *p, int x, int y, int reach);

#endif

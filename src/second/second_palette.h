/* The Net Square's palette on the second screen (second_palette.c, issue
 * #178; its course and its buttons' places in src/online/emote_palette.c):
 * every emote as MegaMan's pose with its name, every phrase on a plate,
 * the favourites marked with their way of L, the button tapped lit. */
#ifndef CW_SECOND_PALETTE_H
#define CW_SECOND_PALETTE_H

#include "emote_palette.h"

/* The whole screen, w x h: the frame without the run's strip, the palette in its body */
void second_palette_draw(int w, int h, const PaletteView *v);
/* ... the two tabs alone, over the NetPass's body (the other tab's) */
void second_palette_tabs(int w, int h, const PaletteView *v);

#endif

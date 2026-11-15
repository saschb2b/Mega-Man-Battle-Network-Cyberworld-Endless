/* The shared dive's finds (issue #589; docs/online/shared-dive.md, As
 * built): a Mystery Data MegaMan opens on a dive layer told to the server
 * (DIVE found) by its index on the layer, the same on every diver's game
 * since the layer's seed places them; once BN6's own box of what he got
 * has closed, the band names the week's first to open it. */
#ifndef CW_DIVE_FINDS_H
#define CW_DIVE_FINDS_H

/* Each frame on a dive layer. */
void dive_finds_frame(void);

#endif

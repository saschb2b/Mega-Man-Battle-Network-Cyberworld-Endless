/* BN5's lava and sea in BN6's own battle engine (xpanels.c; issue #82,
 * docs/MULTIROM.md, One net; docs/ROM_DATA.md, BN5's lava and sea in
 * BN6's engine), at panel types BN6 leaves free, so that BN5's
 * battlefields with them translate (xfields.c):
 * - their flags (BN6's table grown to 16 words, its IWRAM literal pointed
 *   at it);
 * - what they do to what stands on them: an object on lava not of Fire,
 *   floating or invulnerable takes 50, doubled where Fire beats it, and
 *   the lava cools plain under a spark; a Fire object on sea takes
 *   poison's damage (hooks on BN6's code, BN5's rules in C and a little
 *   Thumb of the project's own);
 * - an Aqua hit cools lava, and a chip's lava or sea runs out after
 *   BN5's frames;
 * - how they look (xpanels_look.c): BN5's tiles and colours.
 * Numbers alone here: BN5's tiles and colours come from the player's ROM
 * as the core starts. */
#ifndef CW_XPANELS_H
#define CW_XPANELS_H

#include <stdbool.h>

#define XPANEL_LAVA 13   /* BN6_PANEL_TYPE: BN5's lava (BN5_PANEL_LAVA), at BN6's first free type (BN6_PANEL_TYPES) */
#define XPANEL_SEA  14   /* ... BN5's sea (BN5_PANEL_SEA) */

/* Into the core's copy as it starts, where BN5's ROM is read (xfields.c
 * calls it before it translates a field): each patch site checked to be
 * BN6's own first; all or nothing, the log saying why not. */
void xpanels_install(int xrom);
/* Whether BN6's engine has lava and sea now */
bool xpanels_ready(void);

#endif

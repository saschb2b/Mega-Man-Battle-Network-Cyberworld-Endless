/* How BN5's lava, sea and magnet panel and BN4's Hole look in BN6's battles
 * (xpanels_look.c, for xpanels.c): their tiles in VRAM after BN6's
 * panels', BN6's tile table grown to their types, and their colours where
 * their games give them in the panel palettes, by their animations, in a
 * battle that shows them. Also where the module's data stands. */
#ifndef CW_XPANELS_LOOK_H
#define CW_XPANELS_LOOK_H

#include <stdbool.h>
#include <stdint.h>

#include "emu.h"

/* Where it all goes, at fixed addresses: the plan's FREE_PANELS
 * (free_space.h; docs/engine/emulation/) */
#define XP_AT        FREE_PANELS
#define XP_FLAGS     XP_AT                 /* BN6's panel flags table grown to XPANEL_TYPES words, 0x80 bytes' room (xpanels.c) */
#define XP_STUB      (XP_AT + 0x80u)       /* the lava's cooling, Thumb, and its veneers on to BN6's routines (xpanels.c) */
#define XP_TILES     (XP_AT + 0x100u)      /* BN6's panel tile table grown to XPANEL_TYPES types (lava, sea, the magnet panel, the Hole), */
#define XP_TILES_SIZE 0xCC0u               /* ... 6 entries of 32 bytes a type */
#define XP_PULL      (XP_AT + 0xF00u)      /* the magnet panel's pull, BN5's step for it (xpanels.c, pull_make): Thumb, its veneers and
                                              BN5's directions' table after it */
#define XP_END       FREE_PANELS_END

/* The kinds of one game's ROM (XRomId: BN5's lava, sea and magnet panel, BN4's Hole)
 * read and what is to be written made, BN6's table grown with them; false,
 * the log saying why, where either ROM is not as expected */
bool xpanels_look_stage(int xrom);
/* ... the table written into the core's copy with every kind staged so
 * far, the game's drawing pointed at it (its pointer checked to be BN6's
 * own or the table already) */
void xpanels_look_write(void);
/* The hook on the panels' draw that puts their tiles in VRAM and their
 * colours in the palettes, set once; false where it cannot be set */
bool xpanels_look_hook(void);

#endif

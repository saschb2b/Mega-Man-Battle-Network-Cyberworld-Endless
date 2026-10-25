/* How BN5's lava and sea look in BN6's battles (xpanels_look.c, for
 * xpanels.c): BN5's tiles of them in VRAM after BN6's panels', BN6's tile
 * table grown to their types, and their colours where BN5 gives them in
 * the panel palettes, by BN5's animations, in a battle that shows them.
 * Also where the module's data stands. */
#ifndef CW_XPANELS_LOOK_H
#define CW_XPANELS_LOOK_H

#include <stdbool.h>
#include <stdint.h>

#include "emu.h"

/* Where it all goes, at fixed addresses: the plan's FREE_PANELS
 * (free_space.h; docs/engine/emulation/) */
#define XP_AT        FREE_PANELS
#define XP_FLAGS     XP_AT                 /* BN6's panel flags table grown to 16 words (xpanels.c) */
#define XP_STUB      (XP_AT + 0x40u)       /* the lava's cooling, Thumb, and its veneers on to BN6's routines (xpanels.c) */
#define XP_TILES     (XP_AT + 0x100u)      /* BN6's panel tile table grown to 15 types, */
#define XP_TILES_SIZE 0xB40u               /* ... 6 entries of 32 bytes a type */
#define XP_END       FREE_PANELS_END

/* Both ROMs read and what is to be written made; false, the log saying
 * why, where either is not as expected */
bool xpanels_look_stage(void);
/* ... written into the core's copy, the game's drawing pointed at it (its
 * pointer checked to be BN6's own first) */
void xpanels_look_write(void);
/* The hook on the panels' draw that puts their tiles in VRAM and their
 * colours in the palettes; false where it cannot be set */
bool xpanels_look_hook(void);

#endif

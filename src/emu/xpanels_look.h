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

/* Where it all goes, at fixed addresses (docs/EMULATION.md): EMU_FREE +
 * 0x4C0000 to + 0x4D0000 */
#define XP_AT        (EMU_FREE + 0x4C0000u)
#define XP_FLAGS     XP_AT                 /* BN6's panel flags table grown to 16 words (xpanels.c) */
#define XP_STUB      (XP_AT + 0x40u)       /* the lava's cooling, Thumb, and its veneers on to BN6's routines (xpanels.c) */
#define XP_TILES     (XP_AT + 0x100u)      /* BN6's panel tile table grown to 15 types (0xB40 bytes) */
#define XP_END       (XP_AT + 0x10000u)

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

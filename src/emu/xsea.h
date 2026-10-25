/* BN5's sea's submerge in BN6's battle engine (issue #82; docs/multirom/one-net.md,
 * The sea's submerge; docs/rom/): an object whose AI's traits hold
 * BN6_TRAIT_SUBMERGES (ToadMan's, ToadSoul's) standing on BN5's sea
 * (XPANEL_SEA) goes under, as BN5's status step of MegaMan and the Navis
 * has it (bn5.h: BN5_SEA_SUBMERGE, BN5_HIDE_STEP, BN5_HIT_HIDDEN): it is
 * not drawn, a ripple follows it (BN5's marker, the fourth kind's 0x3E,
 * moved: xnavi_objects.h), and only an Elec attack or one of the flags
 * 0x1008 hits it. BN5 keeps that in the object's collision data (+0x2C
 * 0xFFFF, flag 0x80000000, the marker at +0x50), where BN6 keeps its
 * bubble: here it is kept apart, in the RAM issue #82's plan gives it
 * (RAM_SEA, cleared with each battle's), by each collision data's own bit
 * (BN6_COLL_INDEX_BIT). BN6's code reaches it through hooks at the places
 * BN5's steps stand (bn6_panels.h), a few instructions of the project's own
 * Thumb (the marker's spawn, the hit gate's test) and the hit gate's test
 * in IWRAM patched to call it, kept so in RAM (emu_keep).
 *
 * A moved family that goes under in its own code (BN5's sets flag
 * 0x80000000 itself) sets and clears its bit in XSEA_MASK instead; the
 * marker and the gate follow it. */
#ifndef CW_XSEA_H
#define CW_XSEA_H

#include <stdint.h>

#include "free_space.h"

#define XSEA_MASK     (RAM_SEA + 0x00u)   /* u32, the submerged: each one's collision data's bit */
#define XSEA_MARKERS  (RAM_SEA + 0x04u)   /* each one's marker by that bit's index, 32 words, 0 none */
#define XSEA_RAM_END  (RAM_SEA + 0x84u)   /* (to RAM_DARK_LISTS spare) */

/* Into the core's copy as it starts, where BN5's lava and sea are in BN6's
 * engine (xpanels.h) and its moved objects with them (xnavi_objects.h):
 * each site checked to be BN6's own first; all or nothing, the log saying
 * why not. */
void xsea_install(void);

#endif

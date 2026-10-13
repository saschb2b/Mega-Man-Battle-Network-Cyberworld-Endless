/* The families made by hand, as recipes of numbers (xvirus_families.c): the
 * control, and BN5's first two, which tests/test_core.c holds the
 * generator's (xvirus_bn5.h, the families installed) to field for field. */
#ifndef CW_XVIRUS_FAMILIES_H
#define CW_XVIRUS_FAMILIES_H

#include "xvirus.h"

/* The control (--dev xcontrol): BN6's own Mettaur, AI 1, moved through the
 * same steps as another game's family, as AI 32 with enemy ids 0x200-0x205;
 * it must play frame for frame as Mettaur does (tools/xcontrol.py). */
extern const XVirusFamily xvirus_control;
/* BN5's CanGard (issue #82's first), as its recipe was made by hand and
 * compared frame by frame with BN5's own: its code, its reticle's, its
 * sprites and records copied from the player's BN5 ROM, as AI 32 with enemy
 * ids 0x200-0x205 (CanGard, CanGrdEX, CanRaid, CanRadEX, Canlada, CanladEX) */
extern const XVirusFamily xvirus_cangard;
/* BN5's Handi (issue #82's second, the first that draws random numbers and
 * moves), as made by hand: its code and records copied from the player's
 * BN5 ROM, as AI 33 with enemy ids 0x206-0x20B (Handi, HandiEX, Hando,
 * HandoEX, Handum, HandumEX) */
extern const XVirusFamily xvirus_handi;

#endif

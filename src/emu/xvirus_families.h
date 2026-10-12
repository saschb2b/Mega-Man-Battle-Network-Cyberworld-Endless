/* The families xvirus.c can install, as recipes of numbers (xvirus_families.c). */
#ifndef CW_XVIRUS_FAMILIES_H
#define CW_XVIRUS_FAMILIES_H

#include "xvirus.h"

/* The control (--dev xcontrol): BN6's own Mettaur, AI 1, moved through the
 * same steps as another game's family, as AI 32 with enemy ids 0xBB-0xC0;
 * it must play frame for frame as Mettaur does (tools/xcontrol.py). */
extern const XVirusFamily xvirus_control;
/* BN5's CanGard (issue #82's first): its code, its reticle's, its sprites
 * and records copied from the player's BN5 ROM, as AI 32 with enemy ids
 * 0xBB-0xC0 (CanGard, CanGrdEX, CanRaid, CanRadEX, Canlada, CanladEX) */
extern const XVirusFamily xvirus_cangard;
/* BN5's Handi (issue #82's second, the first that draws random numbers and
 * moves): its code and records copied from the player's BN5 ROM, as AI 33
 * with enemy ids 0xC1-0xC6 (Handi, HandiEX, Hando, HandoEX, Handum,
 * HandumEX) */
extern const XVirusFamily xvirus_handi;

#endif

/* FireSoul on lava in BN6's engine (docs/multirom/bn4-souls.md, FireSoul in
 * BN6's engine): where BN4's lava (type 8) would hurt an actor or spare a
 * Fire one, it heals FireSoul 50 and turns plain (BN4_LAVA_STEP's form 5,
 * BN4_LAVA_FIRE_HEAL). BN6's engine's lava is BN5's (xpanels.c); here
 * FireSoul takes it BN4's way, through xpanels_lava_own. */
#ifndef CW_SOULLAVA_H
#define CW_SOULLAVA_H

#include <stdbool.h>

/* The Thumb (SOULS_BN4_LAVA) and its route through the lava's sites, where
 * FireSoul is moved and BN4's code is the one read; true either way */
bool soullava_install(void);

#endif

/* RollSoul's heal in BN6's engine (docs/multirom/bn4-souls.md, RollSoul in
 * BN6's engine): BN4's chip use heals him a tenth of his max HP after a chip
 * of another kind than Recovery's where his form is RollSoul's
 * (BN4_ROLL_HEAL_FORM), the heal's effect at him and its sound; here a hook
 * at the same place of BN6's chip use (BN6_CHIP_USE_HEAL) heals him the
 * same where he stands in such a Soul (soulbn4_heal), and a stub of the
 * Souls' room spawns the effect and plays the sound as BN4's, moved. */
#ifndef CW_SOULROLL_H
#define CW_SOULROLL_H

#include <stdbool.h>

/* (after xchips_bn4_code_install) The hook and its stub (SOULS4_HEAL), where
 * a moved BN4 Soul heals and breaks (soulpassive.h's) leave out no Soul's
 * passives; true where nothing failed */
bool soulroll_install(int breaks);

#endif

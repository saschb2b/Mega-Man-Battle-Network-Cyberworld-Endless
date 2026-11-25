/* The BN4 Souls no hit's status takes, in BN6's engine (docs/multirom/bn4-souls.md,
 * WoodSoul in BN6's engine): BN4's player's frame passes over the status a hit
 * gave him where his form is WoodSoul's (BN4_STATUS_GUARD_FORM); here a hook on
 * BN6's player's frame passes over its call of BN6_STATUS_TAKEN the same way
 * where he stands in such a Soul (soulbn4_guarded). */
#ifndef CW_SOULGUARD_H
#define CW_SOULGUARD_H

#include <stdbool.h>

/* The hook (BN6_PLAYER_STATUS_CALL), where a moved BN4 Soul is guarded and
 * breaks (soulpassive.h's) leave out no Soul's passives; true where nothing
 * failed */
bool soulguard_install(int breaks);

#endif

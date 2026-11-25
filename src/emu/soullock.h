/* A BN4 Soul's B and left's lock in BN6's engine (docs/multirom/bn4-souls.md,
 * ProtSoul in BN6's engine): BN4's power attack sets the lock as it begins
 * (BN4_AI_LEFT_HOLD, ProtSoul's shield 0x28 frames) and it runs on past the
 * attack's end; BN6's attack's end sets it anew from the attack's +5 for a
 * B and left's attack (BN6_ATTACK_END_LEFT). A hook passes over that store
 * where he stands in a BN4 Soul, his lock running on as BN4's. */
#ifndef CW_SOULLOCK_H
#define CW_SOULLOCK_H

#include <stdbool.h>

/* The hook (BN6_ATTACK_END_LEFT), where a moved BN4 Soul has a B and left's
 * power attack; true where nothing failed */
bool soullock_install(void);

#endif

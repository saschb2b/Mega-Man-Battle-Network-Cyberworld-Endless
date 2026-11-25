/* WindSoul's shoes and wind in BN6's engine (docs/multirom/bn4-souls.md,
 * WindSoul in BN6's engine): BN4's taking of the form ORs his collision
 * flags (floating, BN6's bits too) and spawns his side's wind
 * (BN4_WIND_ENTER); every form's state set clears them and ends the wind
 * again (BN4_FORM_RESET). Here, as he enters the proxy form with such a
 * Soul, the flags are ORed and BN6's own wind spawned (BN6_WIND_SPAWN, the
 * twin WindMan's move pairs it with) at the end of his next frame, as BN4's
 * comes a frame later on the fight's clock, and as he leaves they are cleared and
 * the wind of his side he spawned told to end (BN6_WIND_END). */
#ifndef CW_SOULWIND_H
#define CW_SOULWIND_H

#include <stdbool.h>
#include <stdint.h>

/* The hook on the end of his frame (BN6_PLAYER_LAST_CALL) and its stub
 * (SOULS4_WIND), where a moved BN4 Soul has a wind and breaks (soulpassive.h's)
 * leave out no Soul's passives; true where nothing failed */
bool soulwind_install(int breaks);
/* (soullife.c's, MegaMan `obj` entering the proxy form with BN4's k-th
 * Soul) his flags and his wind counted to the end of his next frame, where
 * the Soul has one */
void soulwind_enter(uint32_t obj, int k);
/* (... leaving it) his flags cleared and his side's wind ended where the
 * Soul has one */
void soulwind_leave(uint32_t obj, int k);

#endif

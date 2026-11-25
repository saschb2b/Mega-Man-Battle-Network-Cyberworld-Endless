/* WindSoul's shoes and wind in BN6's engine (docs/multirom/bn4-souls.md,
 * WindSoul in BN6's engine): BN4's taking of the form ORs his collision
 * flags (floating, BN6's bits too) and spawns his side's wind
 * (BN4_WIND_ENTER); every form's state set clears them and ends the wind
 * again (BN4_FORM_RESET). Here, as he enters the proxy form with such a
 * Soul, the flags are ORed and BN6's own wind spawned (BN6_WIND_SPAWN, the
 * twin WindMan's move pairs it with) at the end of his next frame, as BN4's
 * comes a frame later on the fight's clock, and as he leaves they are cleared and
 * the wind of his side he spawned told to end (BN6_WIND_END). A Soul whose
 * taking of the form spawns an object that lasts the fight (JunkSoul's,
 * soulbn4_late) has that routine's copy run at the same frame's end, so its
 * object steps on the fight's clock as BN4's. */
#ifndef CW_SOULWIND_H
#define CW_SOULWIND_H

#include <stdbool.h>
#include <stdint.h>

/* (after xchips_bn4_code_install) The hook on the end of his frame
 * (BN6_PLAYER_LAST_CALL) and its stubs (SOULS4_WIND, SOULS4_LATE), where a
 * moved BN4 Soul has a wind or a late taking of the form and breaks
 * (soulpassive.h's) leave out no Soul's passives; true where nothing failed */
bool soulwind_install(int breaks);
/* (soullife.c's, MegaMan `obj` entering the proxy form with BN4's k-th
 * Soul) his flags and his wind, or the Soul's late taking of the form,
 * counted to the end of his next frame, where the Soul has one */
void soulwind_enter(uint32_t obj, int k);
/* whether BN4's k-th Soul's taking of the form waits for that frame's end
 * here, rather than running as he enters the proxy form */
bool soulwind_late(int k);
/* (... leaving it) his flags cleared and his side's wind ended where the
 * Soul has one */
void soulwind_leave(uint32_t obj, int k);

#endif

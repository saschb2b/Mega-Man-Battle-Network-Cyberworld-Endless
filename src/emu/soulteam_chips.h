/* Team ProtoMan's Souls' rules for the chips MegaMan uses in BN6's engine
 * (docs/game/team-protoman.md, unit 8; docs/multirom/souls.md, Team
 * ProtoMan's Souls): NapalmSoul's lava step (BN5's 0x080125FA: on lava the
 * hand's next attack chip 10 stronger, the panel made plain), GyroSoul's
 * propeller (BN5's 0x080102CA: a Wind chip sets it; 0x08010264: the next
 * Wind or plain attack chip doubled by it; his overlay spun while it is
 * set). Their hooks and the Thumb they jump to (soul_room.h's
 * SOULS_TEAM_THUMB), installed with the Souls' chips (soulchips.c). */
#ifndef CW_SOULTEAM_CHIPS_H
#define CW_SOULTEAM_CHIPS_H

#include <stdbool.h>

#include "hook.h"

/* (soulchips.c's hook where the chip's damage is modified, BN6_CHIP_MODIFIED, GyroSoul united) his propeller's part of the
 * modifier: HOOK_JUMP to its spending where it doubles the chip, HOOK_CONTINUE (r0 and r1 set as BN5's leave them) else */
HookAct soulteam_chips_modified(HookRegs *r, int chip);
/* Their hooks and Thumb, once a core; `breaks` soulbreak's chips' (soulchips.h); false (nothing hooked) where a byte they
 * stand on is not Gregar's as expected */
bool soulteam_chips_install(int breaks);

#endif

/* MeddySoul's capsules on BN6's own Custom screen (docs/multirom/souls.md, Team ProtoMan's Souls, MeddySoul's capsules;
 * docs/rom/souls.md, MeddySoul's capsules; src/emu/bn5_capsules.h the addresses): BN5's Soul button for its Soul 6 on the
 * elements Gregar kept for it, 8 and 9 of types 6 and 7, which BN6's own code draws as the chips their entries name (BN6's
 * 382-384, the registry's 734 and 735), gives back on B and copies into the chip at OK; what Gregar dropped comes back:
 * the set-up and the lit rule (BN5's, through soulcolonel.c's hooks on DustCross's, one hook an address), A on a capsule
 * and mode 0x3C (BN5's phases on BN6's own UNITE flight), and in the fight the yellow and purple bits and the pink heal. */
#ifndef CW_SOULCAPSULE_H
#define CW_SOULCAPSULE_H

#include <stdbool.h>

#include "hook.h"

/* (soulcolonel.c's set-up hook, MeddySoul united on the screen's side: r4 the elements, r5 the screen) BN5's set-up for
 * Soul 6 (0x08023D1C): elements 8 and 9 the two capsules, grey, two drawn while the screen holds none */
HookAct soulcapsule_setup(HookRegs *r);
/* (soulcolonel.c's lit-rule hook, the same: r5 the screen) BN5's rule (0x08024C78): both lit while the last pick is a
 * chip with no capsule whose record shows its power */
HookAct soulcapsule_light(HookRegs *r);

/* The hooks, the Thumb and the tables (soul_room.h's SOULS_CAPSULES), once a core, after the Souls' code (soulcode_install:
 * mode 0x3C's last two phases moved); false (nothing written) where a byte they stand on is not Gregar's or BN5's as
 * expected. `breaks` soulbreak's: none yet (the differential's mutations poke the tables) */
bool soulcapsule_install(int breaks);

#endif

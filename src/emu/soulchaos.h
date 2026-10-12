/* Chaos Unison in BN6's engine (issue #82, the Souls track's S6;
 * docs/MULTIROM.md, Transplanted Souls; docs/ROM_DATA.md, Chaos Unison): a
 * Soul united by a DarkChip of its kind charges B as BN5's does. Its time
 * by the Soul's chaos power attack (his loadout's sixth, AIData +0x11) and
 * the chaos level (BN5's 0x08010654); once full, a cycle of phases (BN5's
 * 0x080105F0) the glow shows in BN5's dark charge sprite, its animation the
 * phase (0x080E0E44); released in phase 2 it is the Soul's DarkChip
 * attack (DrkSonic, DarkInvs, DarkPlus, DrkLance, DarkDril or DarkWide,
 * the level one up: 0x080F0436), released otherwise the failure (BN5's
 * family 0x39): the Dark Soul on the other side, MegaMan out of the Soul.
 * Hooks on BN6's charge, release, requests, the attack's end and the glow,
 * the project's Thumb at SOULS_CHAOS (soulchaos_thumb.h) for what runs in
 * the game's calls. The Dark Soul's chip lists, where the battle gave it
 * none, are the folder's chips, the most of a chip first (BN5 makes them
 * from the chips MegaMan uses most, which BN6 keeps no record of). */
#ifndef CW_SOULCHAOS_H
#define CW_SOULCHAOS_H

#include <stdbool.h>
#include <stdint.h>

/* Who its bytes are (claims.h) */
#define SOULCHAOS_CLAIMS "Chaos Unison (soulchaos.c)"

/* (soulbreak's chaos, SOULBREAK_POWER's 8: the differential's own test, tests/xdiff/mutations.json) */
enum { SOULCHAOS_BREAK_PHASE = 8 };   /* the phase's cycle a frame longer than BN5's */

/* Chaos Unison as the Souls' abilities go in (souls_own, before
 * soulpower.c, whose tables name its power attacks): its Thumb, the failure's
 * family, its hooks. Once a core; false, nothing hooked, where a byte it
 * stands on is not as expected (the log says which). */
bool soulchaos_install(int breaks);
/* The Thumb address of BN5's chaos power attack `bn5_id` (0x31-0x3A, a
 * Soul's loadout's sixth), 0 for another or where it is not installed */
uint32_t soulchaos_power(int bn5_id);

#endif

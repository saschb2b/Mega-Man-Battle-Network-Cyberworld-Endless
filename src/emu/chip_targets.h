/* The player's chips aimed at the foes moved into BN6's engine (issue
 * #523; docs/rom/chips-code.md, The chips' targets by NameID). BN6 finds
 * some chips' targets by NameID, a range of them at a time
 * (object_getEnemyByNameRange, BN6_BY_NAME_RANGE: the other side's four
 * actors, those whose NameID is in the range, in their slots' order):
 * AquaNdl's needles ask for the viruses, 0-0xAE, then the Navis, 0x100-
 * 0x1C3; YoYo's strike the same with 0-0xBA; the reticle over a foe
 * 0-0x1C3. The families moved into the engine take 0x200-0x37F
 * (XV_FIRST_ID), the moved Navis 0x1C4-0x1FF and 0x380-0x3FF (XN_FIRST_ID,
 * XN_BN4_FIRST_ID): past every one of those ranges. AquaNdl thrown at
 * BN5's or BN4's viruses found none, and its needles fell where the
 * stack's leftover words pointed: nowhere on the field.
 *
 * A hook on each of those five calls looks the side up as BN6's routine
 * does, a moved family's NameID read as BN6's first virus's and a moved
 * Navi's as its first Navi's, so each range finds the moved foes of its
 * kind among BN6's own, in the slots' order. BN6's own foes are found as
 * before, and a battle with none of the moved runs as it did. */
#ifndef CW_CHIP_TARGETS_H
#define CW_CHIP_TARGETS_H

#include <stdint.h>

#include "xnavi_recipe.h"
#include "xvirus_recipe.h"

#define CHIP_TARGETS_VIRUS 0x001   /* BN6's first virus's NameID (Mettaur), in every viruses' range BN6's chips ask for */
#define CHIP_TARGETS_NAVI  0x100   /* BN6's first Navi's (the Navis 0x100-0x190), in every Navis' range, in no viruses' */
/* (the families' ids end where BN4's Navis' begin, the moved Navis' two runs either side of them) */
#define CHIP_TARGETS_FAMILIES_END (XV_FIRST_ID + XV_FAMILIES * XV_VERSIONS)
_Static_assert(XN_IDS_END <= XV_FIRST_ID && CHIP_TARGETS_FAMILIES_END <= XN_BN4_FIRST_ID && XN_BN4_FIRST_ID < XN_MORE_IDS_END,
	"chip_targets: the moved ids' runs as chip_targets_name reads them");

/* The NameID a look-up reads for `id`: CHIP_TARGETS_VIRUS for a moved
 * family's, CHIP_TARGETS_NAVI for a moved Navi's, else `id` itself */
static inline uint16_t chip_targets_name(uint16_t id) {
	if (id >= XV_FIRST_ID && id < CHIP_TARGETS_FAMILIES_END) return CHIP_TARGETS_VIRUS;
	if ((id >= XN_FIRST_ID && id < XN_IDS_END) || (id >= XN_BN4_FIRST_ID && id < XN_MORE_IDS_END)) return CHIP_TARGETS_NAVI;
	return id;
}

/* The hooks, once the core is up (game_hooks.c); CYBERWORLD_EMU_DEBUG
 * says what a call found where a moved foe is among it and it changed */
void chip_targets_install(void);

#endif

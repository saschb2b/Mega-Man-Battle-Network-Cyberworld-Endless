/* liberation_state.h: the Darkloid of a dark board as BN5's own where its
 * ROM brought him into BN6's engine (docs/game/liberation/building/stage-4-darkloids.md). */
#include "liberation_state.h"

#include <stdio.h>

#include "debug.h"
#include "devtools.h"
#include "encounter.h"
#include "guardians.h"
#include "loot.h"
#include "run.h"
#include "save.h"
#include "xguardian.h"

void lib_darkloid_dress(void) {
	/* (BN5's Darkloid in his own sprite and face, copied in as BN5's
	 * guardians' are: guardian_objs.c) */
	if (guardian_darkloid(LB.darkloid)) guardian_set_face(LB.darkloid, xguardian_slot(LB.darkloid));
}

Encounter lib_darkloid_battle(void) {
	if (!guardian_darkloid(LB.darkloid)) return make_boss(run.depth, run.biome, LB.darkloid);
	/* (his moved id at the version the act's band takes, his HP held to
	 * the band as BN5's guardians' are: boss.c) */
	int v = xguardian_version(LB.darkloid, run.depth);
	Encounter e = xguardian_encounter(LB.darkloid, v);
	emu_battle_hp(xguardian_hp(LB.darkloid, v), xguardian_hp_fought(LB.darkloid, v, run.depth));
	if (emu_debug_on()) fprintf(stderr, "liberation: darkloid %d, enemy id %#x, version %d\n", LB.darkloid, e.foes[0].id, v);
	return e;
}

void lib_darkloid_beaten(void) {
	/* (BN5's own, deleted on his board: the version beaten noted for the Battle Lab, as a guardian's is, boss.c;
	 * lab_navis.h) */
	if (!guardian_darkloid(LB.darkloid)) return;
	int v = xguardian_version(LB.darkloid, run.depth);
	if (v < 0 || v >= 8) return;
	profile.navi_beaten[LB.darkloid] |= (uint8_t)(1u << v);
	profile_save();
	if (emu_debug_on()) fprintf(stderr, "liberation: darkloid %d deleted at version %d, noted for the Battle Lab\n", LB.darkloid, v);
}

void lib_darkloid_hp(int *hp, int *max) {
	/* (the HP his battle gives him: BN5's own's held to the act's band,
	 * else make_boss's version's) */
	if (guardian_darkloid(LB.darkloid)) {
		int v = xguardian_version(LB.darkloid, run.depth);
		*max = xguardian_hp_fought(LB.darkloid, v, run.depth);
	} else {
		Encounter e = make_boss(run.depth, run.biome, LB.darkloid);
		*max = navi_hp(e.foes[0].family, e.foes[0].version);
	}
	/* (--dev libhp=N: he begins at N, his heal and his battle from the
	 * board's HP seen) */
	*hp = dev.lib_hp > 0 && dev.lib_hp < *max ? dev.lib_hp : *max;
}

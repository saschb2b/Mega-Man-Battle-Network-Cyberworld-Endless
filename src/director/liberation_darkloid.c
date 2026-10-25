/* liberation_state.h: the Darkloid of a dark board as BN5's own where its
 * ROM brought him into BN6's engine (docs/LIBERATION.md, stage 4). */
#include "liberation_state.h"

#include <stdio.h>

#include "debug.h"
#include "encounter.h"
#include "guardians.h"
#include "loot.h"
#include "run.h"
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

/* A Glitch layer's copy, its fight (glitch.h; docs/game/glitch-layer.md):
 * its forms, the guardians the run_glitch_forms names, in one BN6 battle,
 * each swapped on its panel for the next as its share of one pool of HP
 * runs out (battle_swap.h, the true swap); deleted, the glitch settles and
 * its pick of their Navi chips waits for a talk. */
#include "glitch.h"

#include <stdio.h>

#include "battle_swap.h"
#include "data.h"
#include "debug.h"
#include "director_layer.h"
#include "director_state.h"
#include "encounter.h"
#include "flags.h"
#include "layer_objs.h"
#include "loot.h"
#include "pacing.h"
#include "run.h"

static struct {
	bool fighting;
	bool swapped;       /* the fight is the true swap's */
} C;

void glitch_copy_fight(void) {
	int forms[GLITCH_FORMS], n = run_glitch_forms(run.depth, forms);
	if (n <= 0) return;
	/* (one pool of HP, one and a half of the act's guardians, a share a
	 * form: docs/game/glitch-layer.md) */
	int lo, hi;
	pacing_guardian_band(pacing_act(run.depth), &lo, &hi);
	int pool = (lo + hi) * 3 / 4;
	Encounter e = make_boss(run.depth, run.biome, forms[0]);
	SwapForm plan[SWAP_FORMS];
	int m = 0;
	for (int k = 0; k < n && m < SWAP_FORMS; ++k) {
		Encounter f = make_boss(run.depth, run.biome, forms[k]);
		int id = enemy_id(1, forms[k], f.foes[0].version);
		if (id > 0) plan[m++] = (SwapForm){ id, 0 };
	}
	for (int k = 0; k < m; ++k) plan[k].hp = pool / m;
	set_encounter(&e, true);
	/* (the true swap between its forms inside one battle; with one form,
	 * or a plan the swap refuses, that form alone at the pool's HP) */
	C.swapped = m >= 2 && battle_swap_arm(plan, m);
	int own = navi_hp(forms[0], e.foes[0].version);
	if (!C.swapped && own > 0) emu_battle_hp(own, pool);
	if (emu_debug_on()) fprintf(stderr, "glitch: the copy's fight, %d forms (%s), %d HP\n", m, C.swapped ? "swapped" : "one", pool);
	C.fighting = true;
	D.challenge = true;
}

bool glitch_copy_fighting(void) { return C.fighting; }

void glitch_copy_over(bool won) {
	C.fighting = false;
	if (C.swapped) battle_swap_disarm();
	else emu_battle_unwatch();
	C.swapped = false;
	if (!won) return;
	glitch_settle();
	flag_set(LAYER_COPY_WON_FLAG);
	D.copy_due = true;
}

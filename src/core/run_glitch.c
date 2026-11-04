/* The run's Glitch layer (run.h; docs/game/glitch-layer.md): which layer
 * holds its warp, from the run's seed, and the copy's forms, from what the
 * run has fought and what MegaMan's records remember. */
#include "run.h"

#include "guardians.h"
#include "net.h"
#include "pacing.h"
#include "rivals.h"
#include "super_boss.h"

bool run_debug_glitch;

bool run_glitch_at(int depth) {
	if (depth <= 3 || depth > CYCLE_LAYERS || (depth - 1) % 3) return false;
	int act = (depth - 1) / 3, acts = run.mode == RUN_SHORT ? 2 : 4;
	if (act > acts) return false;
	if (run_debug_glitch) return true;
	uint32_t h = (run.seed ^ 0x474C5443u) * 2654435761u;   /* "GLTC" */
	if ((h >> 8) % 3) return false;
	return act == 1 + (int)((h >> 16) % (unsigned)acts);
}

/* (a form not taken yet, and BN6's own: no super boss, and none moved from
 * another net, whose deletion the swap has not been proven on,
 * docs/rom/navis.md, A Navi swapped mid-battle) */
static int form_add(int forms[GLITCH_FORMS], int n, int navi, int ahead) {
	if (n >= GLITCH_FORMS || navi <= 0 || navi == ahead || super_boss(navi) || guardian_older(navi)) return n;
	for (int k = 0; k < n; ++k) if (forms[k] == navi) return n;
	forms[n] = navi;
	return n + 1;
}

/* (the Navi who last deleted MegaMan, in the latest run a record names) */
static int last_deleter(void) {
	int best = 0, at = 0;
	for (int navi = 1; navi < RIVAL_NAVIS; ++navi) {
		const Rival *r = rival(navi);
		if (r->last == RIVAL_NAVI_WON && r->run > at) { at = r->run; best = navi; }
	}
	return best;
}

int run_glitch_forms(int depth, int forms[GLITCH_FORMS]) {
	int act = pacing_act(depth), n = 0;
	int ahead = act < 6 ? run_guardian(run.biome_order[act]) : 0;
	for (int a = act - 1; a >= 0; --a) n = form_add(forms, n, run_guardian(run.biome_order[a]), ahead);
	n = form_add(forms, n, last_deleter(), ahead);
	/* (the guardians the run set in areas off its way, from its seed) */
	uint32_t h = (run.seed ^ 0x434F5059u) * 2654435761u;   /* "COPY" */
	for (int k = 0; k < BIOME_COUNT && n < GLITCH_FORMS; ++k) {
		int b = (int)((h >> 8) + (uint32_t)k * 7u) % BIOME_COUNT;
		if (b == BIOME_SECRET || b == BIOME_NEST) continue;
		n = form_add(forms, n, run.boss_order[b], ahead);
	}
	return n;
}

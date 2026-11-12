/* The tournament's rounds in the pacing report (pacing_bracket.h). */
#include "pacing_bracket.h"

#include "bracket.h"
#include "guardians.h"
#include "pacing.h"
#include "rivals.h"
#include "run.h"

/* The acts a bracket can stand in: the short net's third, the endless
 * net's third to fifth, each cycle (run_bracket_at) */
static int bracket_act(FILE *out, int mode, int act, int loop) {
	int seen[3][RIVAL_NAVIS][4] = { { { 0 } } }, outside[3] = { 0 }, flagged = 0;
	int depth = loop * CYCLE_LAYERS + act * 3 + 3;
	for (uint32_t seed = 1; seed <= 500; ++seed) {
		run_new(seed * 2654435761u);
		run.mode = mode;
		run.depth = depth;
		run.biome = run.biome_order[act];
		int final = run_guardian(run.biome);
		if (final <= 0) continue;
		RunBracket b;
		bracket_draw(depth, final, &b);
		for (int r = 0; r < 3; ++r) {
			int lo, hi;
			pacing_round_band(act, r, &lo, &hi);
			if (b.navi[r] > 0 && b.navi[r] < RIVAL_NAVIS && b.version[r] >= 0 && b.version[r] < 4) seen[r][b.navi[r]][b.version[r]]++;
			/* (the rounds held to their bands on the first cycle, as the
			 * guardians are; later ones fight a version under SP) */
			if (loop == 0 && (b.hp[r] < lo || b.hp[r] > hi || b.navi[r] == b.navi[(r + 1) % 3])) ++outside[r];
		}
	}
	static const char *const marks[4] = { "", " EX", " SP", "" }, *const older[4] = { "", " V2", " V3", " SP" };
	for (int r = 0; r < 3; ++r) {
		int lo, hi;
		pacing_round_band(act, r, &lo, &hi);
		fprintf(out, "%s act %d cycle %d %s (band %d-%d):", mode == RUN_SHORT ? "short" : "endless", act + 1, loop + 1,
			r == 2 ? "final" : r == 0 ? "round 1" : "round 2", lo, hi);
		for (int n = 0; n < RIVAL_NAVIS; ++n)
			for (int v = 0; v < 4; ++v)
				if (seen[r][n][v]) fprintf(out, " %s%s (%d)", guardian(n)->name, guardian_moved(n) ? older[v] : marks[v], seen[r][n][v]);
		fprintf(out, "%s\n", outside[r] ? "  OUTSIDE" : "");
		flagged += outside[r];
	}
	return flagged;
}

int pacing_bracket_report(FILE *out) {
	fprintf(out, "\nThe tournament's brackets over 500 runs (docs/game/tournament.md): each round's Navi and version, how often;\n"
		"OUTSIDE where one lies past his round's band on the first cycle, or meets the same Navi twice.\n");
	int flagged = bracket_act(out, RUN_SHORT, 2, 0);
	for (int loop = 0; loop < 2; ++loop)
		for (int act = 2; act <= 4; ++act) flagged += bracket_act(out, RUN_ENDLESS, act, loop);
	return flagged;
}

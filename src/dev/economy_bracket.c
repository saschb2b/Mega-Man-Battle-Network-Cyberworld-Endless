/* The tournament's rounds in the economy report (economy_report.h). */
#include "economy_report.h"

#include "bracket.h"
#include "guardians.h"
#include "loot.h"
#include "run.h"
#include "save.h"
#include "xguardian.h"

int economy_bracket_rounds(int depth, int biome, Encounter enc[2], int chip[2], int code[2]) {
	/* (a profile past the Nest's: where the coin puts a bracket for one,
	 * the screen's faces aside) */
	int best = profile.best_depth;
	profile.best_depth = SHORT_LAYERS;
	bool stands = run_bracket_at(depth);
	profile.best_depth = best;
	int final = run_guardian(biome);
	if (!stands || final <= 0) return 0;
	RunBracket b;
	bracket_draw(depth, final, &b);
	for (int r = 0; r < 2; ++r) {
		int navi = b.navi[r];
		bool other = guardian_older(navi) || guardian_bn4(navi);
		enc[r] = guardian_older(navi) ? xguardian_encounter(navi, b.version[r])
			: guardian_bn4(navi) ? xguardian_bn4_encounter(navi, b.version[r]) : make_boss(depth, biome, navi);
		if (!other && enc[r].nfoes) enc[r].foes[0].version = b.version[r];
		chip[r] = bracket_chip(navi, b.version[r], depth, &code[r]);
	}
	return 2;
}

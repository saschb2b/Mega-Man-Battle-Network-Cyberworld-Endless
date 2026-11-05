/* The tournament's acts (run.h; docs/game/tournament.md, the owner's calls
 * 1 and 2 of 5 November 2026): where a bracket of three Navis stands in an
 * act's arena in place of its guardian, from the run's seed. The bracket
 * itself is still being built: it hooks in here, so a run whose extras
 * leave the tournament out (docs/game/meta/extras.md) never meets one. */
#include "run.h"

#include "save.h"

bool run_bracket_at(int depth) {
	if (depth < 1 || !run_extra(EXTRA_TOURNEY) || profile.best_depth < SHORT_LAYERS) return false;
	int loop = (depth - 1) / CYCLE_LAYERS, p = (depth - 1) % CYCLE_LAYERS, act = p / 3;
	if (p >= 18 || (run.mode == RUN_SHORT && (loop || act > 2))) return false;
	/* (the short net's act 3 in half the runs; the endless net's act 3, 4
	 * or 5, each as likely, every cycle) */
	uint32_t h = (run.seed ^ 0x54524E59u ^ (uint32_t)loop * 0x9E3779B9u) * 2654435761u;   /* "TRNY" */
	int at = run.mode == RUN_SHORT ? (h >> 8 & 1 ? 2 : -1) : 2 + (int)((h >> 8) % 3);
	/* (never the act of the run's Glitch layer: there the act keeps its
	 * single guardian) */
	return act == at && !run_glitch_at(loop * CYCLE_LAYERS + act * 3 + 1);
}

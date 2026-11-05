/* The dev economy report: what a run hands out, layer by layer and act by
 * act, against its bands, without the game (economy_report.c,
 * docs/game/economy/, docs/devtools/). */
#ifndef CW_ECONOMY_REPORT_H
#define CW_ECONOMY_REPORT_H

#include "foes.h"

/* Writes the report to `spec`'s path (PATH[:RUNS]); 0 when written. */
int economy_report_run(const char *spec);
/* The tournament's rounds at the arena of `depth` in area `biome`, where
 * the coin puts a bracket for a profile past the Nest
 * (docs/game/tournament.md): each round's battle and the Navi chip it pays
 * (economy_bracket.c); how many, 0 where the act keeps its guardian */
int economy_bracket_rounds(int depth, int biome, Encounter enc[2], int chip[2], int code[2]);

#endif

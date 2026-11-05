/* The pacing report's rows for the tournament's brackets (pacing_bracket.c;
 * docs/game/tournament.md, Measured). */
#ifndef CW_PACING_BRACKET_H
#define CW_PACING_BRACKET_H

#include <stdio.h>

/* Each round's Navis over 500 drawn runs, per act a bracket can stand in,
 * against its band; how many past it */
int pacing_bracket_report(FILE *out);

#endif

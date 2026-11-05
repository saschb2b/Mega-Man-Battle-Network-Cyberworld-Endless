/* A guardian's title card, and BN4's bracket screen between a tournament's
 * rounds (boss_rounds.c), for the guardian's sequence (boss.c;
 * docs/game/tournament.md). */
#ifndef CW_BOSS_ROUNDS_H
#define CW_BOSS_ROUNDS_H

#include <stdbool.h>

#include "run.h"

/* Navi `navi`'s title card at `version`: "Guardian of" his area over his
 * name, or a tournament's round (0, 1, 2 the final; -1 none) */
void boss_title(int navi, int version, int round);
/* BN4's bracket screen, moved into BN6 (xbracket.h), with `won` of
 * MegaMan's rounds settled: off the map until its fade back, which brings
 * MegaMan back where he stands; random battles held from here
 * (emu_encounters_hold) */
void boss_bracket_open(const RunBracket *b, int won);
/* Whether it has come and gone, `t` the frames the staging has counted on
 * the map since it opened (frames off the map since, then a beat back on
 * it; or it never showed): the map's arrival it changed put back */
bool boss_bracket_back(int t);

#endif

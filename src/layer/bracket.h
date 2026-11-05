/* The tournament's bracket as a layer holds it (docs/game/tournament.md):
 * whether an act's arena holds one, its draw from the Navis moved of the
 * games present (run.h's run_bracket_draw), and its eight entrants as
 * BN4's bracket screen, moved into BN6, takes them (xbracket.h). */
#ifndef CW_BRACKET_H
#define CW_BRACKET_H

#include <stdbool.h>

#include "run.h"
#include "xbracket.h"

/* Whether the act of the normal layer at `depth` ends in the bracket:
 * run_bracket_at, where BN4's ROM is read, as its screen shows every
 * bracket and the tournament waits for it (the owner's call 6) */
bool bracket_here(int depth);
/* The act's bracket at `depth`, its final the act's guardian `final` as
 * the layer draws him; false where none stands */
bool bracket_of(int depth, int final, RunBracket *b);
/* A round's Navi chip, as his Guardian Data would give it: his chip at the
 * version fought (BN5's xguardian_chip), its code into *code; 0 none */
int bracket_chip(int navi, int version, int depth, int *code);
/* BN4's tournament whose title the screen shows for act `act` (0-based):
 * The Den Battle Tournament in the third, The Eagle Tournament in the
 * fourth, The Red Sun Tournament in the fifth */
int bracket_tournament(int act);
/* The eight entrants, `won` rounds of MegaMan's settled (0 before the
 * first, 3 after the final): MegaMan first, the first round's Navi beside
 * him, the second's in the next pair, the final's in the other half; the
 * four others lose their matches. */
void bracket_entrants(const RunBracket *b, int won, XbrEntrant e[BN4_ENTRANTS]);

#endif

/* What a liberation's battle stakes, in words (liberation_battle_words.c):
 * the menu's lines before it starts. */
#ifndef CW_LIBERATION_BATTLE_WORDS_H
#define CW_LIBERATION_BATTLE_WORDS_H

#include <stddef.h>

#include "liberation_battle.h"

/* Into `a` the turns and what a win does, into `b` what a win in the
 * first turn adds ("" none) */
void lib_stakes_words(const LibStakes *s, char *a, size_t na, char *b, size_t nb);
/* Into `c` the battle's field where BN5 surrounds the Navi ("" else) */
void lib_surrounded_words(const LibStakes *s, char *c, size_t nc);

#endif

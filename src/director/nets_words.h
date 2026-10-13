/* MegaMan and Lan where the nets meet (nets_words.c), for the director's other parts. */
#ifndef CW_NETS_WORDS_H
#define CW_NETS_WORDS_H

#include "foes.h"

/* The arrival's words where the nets meet, the first time a profile comes
 * there (after the arrival's own, into D.beat) */
void nets_arrival_words(void);
/* A battle over: the first visitor a profile met, its words due */
void nets_battle_note(const Encounter *e);
/* Once a frame on the map: the words due (the first visitor; after a
 * CONTINUE without BN5's ROM, the chips and Souls of BN5's that wait for
 * it, D.resumed) */
void nets_words_due(void);

#endif

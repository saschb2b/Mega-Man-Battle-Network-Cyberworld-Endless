/* MegaMan and Lan on the older net and where the nets meet (guest_words.c), for the director's other parts. */
#ifndef CW_GUEST_WORDS_H
#define CW_GUEST_WORDS_H

#include <stddef.h>

#include "foes.h"

int out_names(char *s, size_t n);
void older_net_words(void);
void guest_words(void);
void old_chips_words(void);
void nets_arrival_words(void);
void nets_battle_note(const Encounter *e);

#endif

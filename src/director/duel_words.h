/* Chaud's lines (duel_words.c), for the director's other parts. */
#ifndef CW_DUEL_WORDS_H
#define CW_DUEL_WORDS_H

#include <stdbool.h>
#include <stddef.h>

int verdict_result(char *out, size_t n, bool won, bool beat, int rung, int mine, int his);
const char *duel_call_words(void);

#endif

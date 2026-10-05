/* Chaud's lines (duel_words.c), for the director's other parts. */
#ifndef CW_DUEL_WORDS_H
#define CW_DUEL_WORDS_H

#include <stdbool.h>
#include <stddef.h>

const char *duel_call_words(void);
/* a duel's end, as Chaud's verdict speaks of it */
typedef struct {
	bool won, beat, opened;   /* the battle won; ProtoMan's time beaten (or him); the gate beside opened */
	int rung, mine, his;      /* the duel's rung; the two times, in frames */
	int before, after;        /* Chaud's clearance before the duel and after */
} DuelVerdict;
void verdict_words(char *out, int size, const DuelVerdict *v);

#endif

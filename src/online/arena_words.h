/* What MegaMan says of the Arena in the Net Square (arena_words.c,
 * square_arena.c, docs/VOICE.md): the Battle choice for another player's
 * MegaMan, a challenge sent and answered, the question a challenge asks,
 * a MegaMan who cannot go, and the word back in the Square. */
#ifndef CW_ARENA_WORDS_H
#define CW_ARENA_WORDS_H

#include <stdbool.h>

/* The peer menu's choice */
const char *arena_battle_option(void);
/* A challenge sent to `name`; the answer No, or the battle gone before it began */
const char *arena_sent_words(const char *name);
const char *arena_declined_words(void);
const char *arena_gone_words(void);
/* Challenged by `name`: the box, then the question (its end a '\n') */
const char *arena_challenged_words(const char *name);
const char *arena_challenged_question(void);
/* Our MegaMan can't go into a plain BN6 NetBattle: `souls` his Souls, else another game's chips */
const char *arena_cant_words(bool souls);
/* Back in the Square: won, lost, over with no result, stopped */
const char *arena_won_words(void);
const char *arena_lost_words(void);
const char *arena_over_words(void);
const char *arena_stopped_words(void);

#endif

/* What MegaMan says of the Arena in the Net Square (arena_words.c,
 * square_arena.c, docs/VOICE.md): the Battle choice for another player's
 * MegaMan, a challenge sent and answered, the question a challenge asks,
 * a MegaMan who cannot go, and the word back in the Square, a rematch
 * asked with it. */
#ifndef CW_ARENA_WORDS_H
#define CW_ARENA_WORDS_H

#include <stdbool.h>

/* The peer menu's choice */
const char *arena_battle_option(void);
/* A challenge sent to `name`; the answer No, or the battle gone before it
 * began */
const char *arena_sent_words(const char *name);
const char *arena_declined_words(void);
const char *arena_gone_words(void);
/* Challenged by `name`: the boxes, then the question (its end a '\n') */
const char *arena_challenged_words(const char *name);
/* The folder to carry: folder k's name as BN6 names it; MegaMan's words
 * before the choice; no folder holds a chip */
const char *arena_folder_name(int k);
const char *arena_folder_words(void);
const char *arena_no_folder_words(void);
const char *arena_challenged_question(void);
/* Back in the Square after a result: another with `name`? (its end a '\n') */
const char *arena_again_question(const char *name);
/* Our MegaMan can't go into a NetBattle: a chip or a number no pair core
 * holds (the last guard, arena_loadout_check) */
const char *arena_cant_words(void);
/* Back in the Square: won, lost, over with no result, stopped */
const char *arena_won_words(void);
const char *arena_lost_words(void);
const char *arena_over_words(void);
const char *arena_stopped_words(void);
/* ... stopped before it began, the two games' installs apart */
const char *arena_installs_apart_words(void);

#endif

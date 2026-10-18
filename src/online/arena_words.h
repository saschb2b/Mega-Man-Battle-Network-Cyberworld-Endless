/* What MegaMan says of the Arena in the Net Square (arena_words.c,
 * square_arena.c, docs/VOICE.md): the Battle choice for another player's
 * MegaMan, a challenge sent and answered, the question a challenge asks,
 * a MegaMan who cannot go, and the word back in the Square. */
#ifndef CW_ARENA_WORDS_H
#define CW_ARENA_WORDS_H

#include <stdbool.h>

/* The peer menu's choice */
const char *arena_battle_option(void);
/* A challenge sent to `name` by `format`'s rules (ONLINE_FORMAT_*), named
 * (`souls_home`: under BN6's, his Souls left home); the answer No, or the
 * battle gone before it began */
const char *arena_sent_words(const char *name, unsigned format, bool souls_home);
const char *arena_declined_words(void);
const char *arena_gone_words(void);
/* Challenged by `name` to `format`'s rules, named: the boxes, then the
 * question (its end a '\n') */
const char *arena_challenged_words(const char *name, unsigned format);
/* The folder to carry: folder k's name as BN6 names it; MegaMan's words
 * before the choice (`standins` first, the folders' BN5 chips that sit
 * out, arena_standin_words; `souls_home` his Souls left home); no folder
 * fits (`bn5`: every one holds BN5's chips) */
const char *arena_folder_name(int k);
const char *arena_folder_words(unsigned format, const char *standins, bool souls_home);
/* ... a folder's BN5 chips sitting out under BN6's rules: `n` of them,
 * the first `gone` and the chip of the Pack's standing in for it, `in` */
const char *arena_standin_words(const char *folder, int n, const char *gone, const char *in);
const char *arena_no_folder_words(bool bn5);
const char *arena_challenged_question(void);
/* Our MegaMan can't go into a plain BN6 NetBattle: `souls` his Souls, else another game's chips (the
 * last guard, after the rules left them home) */
const char *arena_cant_words(bool souls);
/* Back in the Square: won, lost, over with no result, stopped */
const char *arena_won_words(void);
const char *arena_lost_words(void);
const char *arena_over_words(void);
const char *arena_stopped_words(void);

#endif

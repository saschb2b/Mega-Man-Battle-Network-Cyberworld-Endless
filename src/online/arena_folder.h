/* The questions a NetBattle from the Net Square asks in BN6's chat box
 * (square_arena.c; docs/ONLINE_ARENA.md, Formats), as the Square's other
 * choices are: a challenge taken or not, its rules named, and the folder
 * MegaMan carries. MegaMan names the rules and each folder they leave home
 * (BN5's chips in it under BN6's rules, his Souls too), and Lan picks among
 * the folders that fit, B none. Asked where Lan has a choice or one stays
 * home; where MegaMan has one folder and it fits, it goes unasked. Each
 * option's script sets an event flag of the Square's, read as the chat
 * closes. */
#ifndef CW_ARENA_FOLDER_H
#define CW_ARENA_FOLDER_H

#include <stdbool.h>

#include "arena_format.h"

/* Challenged by `name` to `format`'s rules: the question opened (the
 * cursor on No); false where a chat is open. As it closes: Yes said. */
bool arena_folder_challenge(const char *name, unsigned format);
bool arena_folder_challenge_yes(void);
/* Back in the Square after a result: `said` (MegaMan's word on it, ""
 * none), then whether to go again with `name` (the cursor on No); false
 * where a chat is open. As it closes: arena_folder_challenge_yes. */
bool arena_folder_again(const char *said, const char *name);
/* The first folder of `fits` (a bit a folder), 0 where none */
int arena_folder_first(unsigned fits);
/* Whether Lan is asked: `l`'s folders under `format`, `fits` those that fit */
bool arena_folder_asked(const OnlineLoadout *l, unsigned format, unsigned fits);
/* The question opened; false where it could not be */
bool arena_folder_ask(const OnlineLoadout *l, unsigned format, unsigned fits);
/* As its chat closes: the folder picked (0-2), -1 none; its flags cleared */
int arena_folder_picked(void);
/* The MegaMan carried into the NetBattle: arena_format_carry's, its folder's
 * BN5 chips stood in for under BN6's rules (arena_standin.h), each said in
 * the log where `say` */
void arena_folder_carry(const OnlineLoadout *all, unsigned format, int k, OnlineLoadout *out, bool say);
/* The questions' flags cleared, as MegaMan arrives in the Square */
void arena_folder_reset(void);

#endif

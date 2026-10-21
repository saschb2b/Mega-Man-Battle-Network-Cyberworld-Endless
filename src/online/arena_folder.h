/* The questions a NetBattle from the Net Square asks in BN6's chat box
 * (square_arena.c; docs/ONLINE_ARENA.md, The folder), as the Square's
 * other choices are: a challenge taken or not, and the folder MegaMan
 * carries, Lan's pick among those that hold chips, B none. Asked where Lan
 * has a choice; where MegaMan has one folder, it goes unasked. Each
 * option's script sets an event flag of the Square's, read as the chat
 * closes. */
#ifndef CW_ARENA_FOLDER_H
#define CW_ARENA_FOLDER_H

#include <stdbool.h>

#include "arena_format.h"

/* Challenged by `name`: the question opened (the cursor on No); false
 * where a chat is open. As it closes: Yes said. */
bool arena_folder_challenge(const char *name);
bool arena_folder_challenge_yes(void);
/* The first folder of `fits` (a bit a folder), 0 where none */
int arena_folder_first(unsigned fits);
/* Whether Lan is asked: more than one folder of `fits` */
bool arena_folder_asked(unsigned fits);
/* The question opened; false where it could not be */
bool arena_folder_ask(unsigned fits);
/* As its chat closes: the folder picked (0-2), -1 none; its flags cleared */
int arena_folder_picked(void);
/* The questions' flags cleared, as MegaMan arrives in the Square */
void arena_folder_reset(void);

#endif

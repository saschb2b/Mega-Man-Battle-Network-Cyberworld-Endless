/* The Battle Lab's own folder (docs/game/battle-lab.md, The sandbox folder):
 * thirty chips of those a profile has held in any run, set in BN6's own
 * folder editor and kept in the profile. The editor runs inside a snapshot
 * of the game, as a test battle does (lab_battle.h): the pack is filled
 * with the Library's chips and the folder with the Lab's, BN6's PET is
 * opened on its folder editor, and as the player leaves it the folder is
 * read and the game put back. Nothing of it reaches the run. */
#ifndef CW_LAB_FOLDER_H
#define CW_LAB_FOLDER_H

#include <stdbool.h>
#include <stdint.h>

/* The editor asked for, from the builder (no battle of its own held: left
 * first): false where the game's state could not be kept */
bool lab_folder_edit(void);
/* Whether it is under way, and still on its way to BN6's editor (its
 * frames run through at once, as a set's field coming up) */
bool lab_folder_on(void);
bool lab_folder_opening(void);
/* One frame of it, from the director's update through lab_battle_frame, in
 * the place of everything else: false where none is under way */
bool lab_folder_frame(void);
/* The keys BN6 gets in it: the way into the editor pressed for the player,
 * then the player's own, none as it leaves */
uint32_t lab_folder_keys(uint32_t keys);
/* Whether the Lab has a folder of its own to fight with (the profile's
 * switch on and a folder made), and that folder written into the game's
 * first, its chips owned: a test battle's, inside its snapshot */
bool lab_folder_used(void);
void lab_folder_into_game(void);

#endif

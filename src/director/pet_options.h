/* The PET's Options (docs/game/pet.md, Options): BN6's Comm row, which a run
 * greys, lit and named Options in the menu's own lettering, and A on it
 * opening the game's Options (options.h) over the PET, which waits under
 * it. */
#ifndef CW_PET_OPTIONS_H
#define CW_PET_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Once, on the core's ROM copy (pet_install): the label made from the
 * labels' letters, the grey lifted; false where the ROM's labels are not
 * the ones read, which leaves Comm as BN6 has it. */
bool pet_options_install(void);
/* Comm is Options: its A is the engine's (pet.c's hook on the menu's input) */
bool pet_options_here(void);
/* The ninth row's label, Forms, made beside Options in the same copy of the
 * labels' tiles: its 9 x 2 tile entries (Save's, the word's columns ours),
 * for pet_row.c; false while the labels are BN6's */
bool pet_options_forms_map(uint16_t out[2 * 9]);
/* The state in words, in the PET: `blanktile zero` while BN6's blank tile
 * (0x2FF of BG character block 2, BG0's every entry on the Chip Folder's
 * screens) is clear of the labels' copy, `drawn` where a screen of BN6's
 * would show it; reads only */
void pet_options_describe(FILE *f);
/* Each game frame: Options taken from the menu opens. */
void pet_options_update(void);
/* The player's keys: those held as Options closes kept from the game
 * until let go (the A or B that closed it, else the menu's again). */
uint32_t pet_options_keys(uint32_t keys);

#endif

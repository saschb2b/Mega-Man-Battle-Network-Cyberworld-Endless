/* The PET's Options (docs/PET.md, Options): BN6's Comm row, which a run
 * greys, lit and named Options in the menu's own lettering, and A on it
 * opening the game's Options (options.h) over the PET, which waits under
 * it. */
#ifndef CW_PET_OPTIONS_H
#define CW_PET_OPTIONS_H

#include <stdbool.h>
#include <stdint.h>

/* Once, on the core's ROM copy (pet_install): the label made from the
 * labels' letters, the grey lifted; false where the ROM's labels are not
 * the ones read, which leaves Comm as BN6 has it. */
bool pet_options_install(void);
/* Comm is Options: its A is the engine's (pet.c's hook on the menu's input) */
bool pet_options_here(void);
/* Each game frame: Options taken from the menu opens. */
void pet_options_update(void);
/* The player's keys: those held as Options closes kept from the game
 * until let go (the A or B that closed it, else the menu's again). */
uint32_t pet_options_keys(uint32_t keys);

#endif

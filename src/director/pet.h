/* The PET's Save as the run's (docs/game/pet.md): A on it, disabled in a run,
 * closes the PET and saves the run where MegaMan stands. Comm is the
 * game's Options (pet_options.h); the dive's report and records are
 * Dad's mail (pet_text.h). The KeyItem screen's NetPass and Jukebox open
 * their screens over it. */
#ifndef CW_PET_H
#define CW_PET_H

#include <stdbool.h>
#include <stdint.h>

/* Once, on the core's ROM copy: Save and Options lit and taken by the engine. */
void pet_install(void);
/* Each game frame: Save chosen closes the PET, then saves on the map. */
void pet_update(void);
/* The player's keys, but for a Save closing the PET, which presses B; A
 * on the NetPass key item opens the pass (netpass_screen.h), on the
 * Jukebox the Jukebox (jukebox.h), the A kept from the game. */
uint32_t pet_keys(uint32_t keys);
/* The key item under the KeyItem screen's cursor while it takes input,
 * -1 elsewhere */
int pet_key_item(void);
/* Over the game's picture in a run: the NetPass while it is up, and its
 * strip while MegaMan's question at the Net Square's gate is open; the
 * Jukebox while it is up */
void pet_draw(void);
/* The frame's course of a PET screen that holds the game still under it
 * (the Jukebox, its music with it): true while it does, the core's frame
 * not run */
bool pet_holds(void);
/* The game's scene left: such a screen put away */
void pet_leave(void);

#endif

/* The PET's Save as the run's (docs/PET.md): A on it, disabled in a run,
 * closes the PET and saves the run where MegaMan stands. Comm keeps BN6's
 * grey; the dive's report and records are Dad's mail (pet_text.h). */
#ifndef CW_PET_H
#define CW_PET_H

#include <stdbool.h>
#include <stdint.h>

/* Once, on the core's ROM copy: Save lit and taken by the engine. */
void pet_install(void);
/* Each game frame: Save chosen closes the PET, then saves on the map. */
void pet_update(void);
/* The player's keys, but for a Save closing the PET, which presses B; A
 * on the NetPass key item opens the pass (netpass_screen.h), the A kept
 * from the game. */
uint32_t pet_keys(uint32_t keys);
/* The key item under the KeyItem screen's cursor while it takes input,
 * -1 elsewhere */
int pet_key_item(void);
/* Over the game's picture in a run: the NetPass while it is up, and its
 * strip while MegaMan's question at the Net Square's gate is open */
void pet_draw(void);

#endif

/* The PET's entries the run gives a use of its own (docs/PET.md): the
 * SciLab link behind Comm, the lab's view of the dive, drawn by the engine
 * over the PET while the game holds still. */
#ifndef CW_PET_H
#define CW_PET_H

#include <stdbool.h>
#include <stdint.h>

/* The PET menu's Comm and Save as the run's (the patches: A on either,
 * disabled in a run, leaves the entry for the engine instead of a
 * buzzer, and neither is greyed). Once, on the core's ROM copy. */
void pet_install(void);
/* Each game frame: Comm chosen opens the SciLab link, Save closes the PET
 * and saves the run on the map. */
void pet_update(void);
/* The SciLab link: open (its first page), and whether it shows. */
void pet_link_open(void);
bool pet_link_is_open(void);
/* The player's keys: the link takes them while it is open (LEFT and RIGHT
 * turn its pages, A or B closes it, and the game gets neither till they
 * are let go); a Save closing the PET presses B for it. */
uint32_t pet_keys(uint32_t keys);
void pet_link_draw(void);

#endif

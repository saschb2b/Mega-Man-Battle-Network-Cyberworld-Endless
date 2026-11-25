/* The PET's screens of ours that open from its start menu (pet_menu.h):
 * Forms (pet_forms.h) and Colours (src/looks/), each over the PET's menu
 * with the game held under it, for pet.c in one place. */
#ifndef CW_PET_SCREENS_H
#define CW_PET_SCREENS_H

#include <stdbool.h>
#include <stdio.h>

/* Each frame in the PET: what the screens keep in step with the run */
void pet_screens_update(void);
/* The frame's course of whichever is up: true while one holds the game */
bool pet_screens_holds(void);
/* Whether the second screen follows one (the forms screen's cursor) */
bool pet_screens_second(void);
void pet_screens_draw(void);
/* Put away at once: the game's scene left */
void pet_screens_leave(void);
/* The state in words (docs/devtools/scenarios.md); reads only */
void pet_screens_describe(FILE *f);

#endif

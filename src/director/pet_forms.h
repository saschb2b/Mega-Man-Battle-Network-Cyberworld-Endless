/* The forms screen in a run (src/forms/; docs/game/meta/forms-screen.md, issue
 * #774): what the run holds of MegaMan's forms handed to it as it opens
 * from the PET, and the five Crosses marked for CROSSSELECT, which the
 * engine's CROSSSELECT lists (crossselect_active) in BN6's order. */
#ifndef CW_PET_FORMS_H
#define CW_PET_FORMS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* The forms screen opened over the PET's menu */
void pet_forms_open(void);
/* Each game frame: the five handed to CROSSSELECT (none marked again in
 * another run) */
void pet_forms_update(void);
/* The player's keys on the PET's menu: R opens the forms screen, the R
 * kept from the game (the caller's outside the Net Square, where R is the
 * NetPass's: square_pet.h) */
uint32_t pet_forms_keys(uint32_t keys);
/* The screen's course while it is up (forms_screen.h's): true while it holds the game */
bool pet_forms_holds(void);
bool pet_forms_shown(void);
/* Over the game's picture: the screen while it is up, and R's hint at the
 * PET menu's band's right end while the menu takes input */
void pet_forms_draw(void);
/* The game's scene left: put away */
void pet_forms_leave(void);
/* The state in words: `crossesfive` the five CROSSSELECT lists (those
 * marked that MegaMan holds, then the first held in BN6's order until five
 * are; BN6's first five held while none is marked) (`none` where no Cross
 * is held), and CROSSSELECT's rows as it lists them while it is open in a
 * battle (`crosslist`); the forms screen's while it is up; reads only */
void pet_forms_describe(FILE *f);

#endif

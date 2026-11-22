/* The PET's own screens on the second screen (second_pet.c, issue #77):
 * MegaMan's status, the Library, and the forms screen (src/forms/). */
#ifndef CW_SECOND_PET_H
#define CW_SECOND_PET_H

#include "gfx.h"

/* In the body `b` of the PET's frame */
void second_status_draw(SDL_Rect b);
void second_library_draw(SDL_Rect b);
void second_forms_draw(SDL_Rect b);
/* Whether the forms screen is up over the PET's menu: its panel then, read
 * from it each frame (S2.forms) */
bool second_forms_up(void);
void second_read_forms(void);

#endif

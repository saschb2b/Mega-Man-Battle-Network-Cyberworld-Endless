/* The PET's own screens on the second screen (second_pet.c, issue #77):
 * MegaMan's status and the Library. */
#ifndef CW_SECOND_PET_H
#define CW_SECOND_PET_H

#include "gfx.h"

/* In the body `b` of the PET's frame */
void second_status_draw(SDL_Rect b);
void second_library_draw(SDL_Rect b);

#endif

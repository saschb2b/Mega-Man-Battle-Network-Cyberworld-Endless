/* The PET at home (second_home.c, issue #79): MegaMan's face with the
 * run's next step in the town, and with the profile's record on the
 * title (second.h's second_title_draw). */
#ifndef CW_SECOND_HOME_H
#define CW_SECOND_HOME_H

#include "gfx.h"

/* The town's panel in the body `b` of the PET's frame (and the PET's
 * menu's and its screens' without one of their own) */
void second_home_draw(SDL_Rect b);
/* MegaMan's face twice as large in its slot from (x, y), SECOND_FACE_W
 * wide (SECOND_FACE_H tall): the PET's screens' */
#define SECOND_FACE_W 86
#define SECOND_FACE_H 102
void second_face(int x, int y);
/* ... another's, BN6's mugshot `face` (a mail's sender) */
void second_mugshot(int face, int x, int y);

#endif

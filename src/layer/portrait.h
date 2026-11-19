/* Faces for the guardians Gregar has none of (docs/game/bosses/): a portrait
 * from MegaMan's battle data, made from the Navi's own battle sprite. */
#ifndef CW_PORTRAIT_H
#define CW_PORTRAIT_H

#include <stdbool.h>
#include <stdint.h>

/* The mugshot number guardian `navi` speaks with where Gregar has no face
 * of him (Falzar's Navis, 6 SpoutMan to 10 DustMan): his portrait, written
 * into the core's ROM copy (again on every call: a layer that needs it
 * calls it as it is made); -1 for any other navi, or where it cannot be
 * made (the player's ROM not as expected). */
int portrait_face(int navi);
/* The pixels of `navi`'s portrait window (Falzar's Navis' alone) from
 * (x, y), w x h of its 40 x 48, into `px` (palette indices, 0 none, row by
 * row) and his battle sprite's palette into `pal`; false where it cannot
 * be read. CROSSSELECT's rows (cross_rows.c) are made from it. */
bool portrait_pixels(int navi, int x, int y, int w, int h, uint8_t *px, uint16_t pal[16]);

#endif

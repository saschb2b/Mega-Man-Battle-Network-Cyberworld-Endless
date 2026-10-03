/* Faces for the guardians Gregar has none of (docs/BOSSES.md): a portrait
 * from MegaMan's battle data, made from the Navi's own battle sprite. */
#ifndef CW_PORTRAIT_H
#define CW_PORTRAIT_H

/* The mugshot number guardian `navi` speaks with where Gregar has no face
 * of him (Falzar's Navis, 6 SpoutMan to 10 DustMan): his portrait, written
 * into the core's ROM copy (again on every call: a layer that needs it
 * calls it as it is made); -1 for any other navi, or where it cannot be
 * made (the player's ROM not as expected). */
int portrait_face(int navi);

#endif

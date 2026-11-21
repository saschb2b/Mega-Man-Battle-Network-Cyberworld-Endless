/* A name over a head on BN6's map: where a place of the map shows in the
 * picture, by BN6's displayed camera, and the dark plate a name stands on.
 * The Net Square's players (online_labels.h) and a dive's divers
 * (dive_people.h) are named by it. Our own drawing over the game's
 * picture (at P.core_x, P.core_y); it reads the game and changes nothing. */
#ifndef CW_MAP_LABEL_H
#define CW_MAP_LABEL_H

#include <stdbool.h>

#include <SDL.h>

#define MAP_LABEL_ROW 12   /* a plate's rows and the gap to the next one over it */

/* BN6's displayed camera (false: none to read, no map showing). */
bool map_label_camera(int camera[3]);
/* The picture's point over the head of one who stands at (x, y, z). */
void map_label_at(int x, int y, int z, const int camera[3], int *px, int *py);
/* Whether a label at that point shows: inside the picture, off its top
 * rows (BN6's HP box) and its foot's (the bands). */
bool map_label_shown(int px, int py);
/* `text`'s middle once it is held inside the picture's sides. */
int map_label_x(int px, const char *text);
/* `text` on its plate, its middle at map_label_x(px, text), which is
 * returned (a mark over it stands there too). */
int map_label_plate(int px, int py, const char *text, SDL_Color colour);

#endif

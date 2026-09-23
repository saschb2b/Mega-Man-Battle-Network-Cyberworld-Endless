/* Net floors drawn from the original areas' panels (see net_floor.c). */
#ifndef NET_FLOOR_H
#define NET_FLOOR_H

#include <stdbool.h>

#include <SDL.h>

#define FLOOR_AREAS 8

/* Neighbour bits of a panel, clockwise from the upper left. */
enum { NB_NW, NB_N, NB_NE, NB_E, NB_SE, NB_S, NB_SW, NB_W };

/* Learns area's pieces from the ROM on first use; false if it cannot. */
bool floor_ready(int area);
/* The panel centred at (sx, sy) with neighbours nb (1 << NB_*). */
void floor_draw(int area, int sx, int sy, unsigned nb, SDL_Color mod);

#endif

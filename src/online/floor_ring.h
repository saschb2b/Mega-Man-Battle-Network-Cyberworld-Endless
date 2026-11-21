/* A ring on BN6's map floor, drawn over the game's picture (issue #716):
 * a circle of the world around one who stands there, as BN6's projection
 * shows it (an ellipse twice as wide as tall, map_label.h), two pixels
 * wide with a dark edge round it, so it reads on a light floor, a dark one
 * and one of its own colour. Our own
 * drawing; it reads the game's camera and changes nothing. */
#ifndef CW_FLOOR_RING_H
#define CW_FLOOR_RING_H

#include <stdbool.h>

#include <SDL.h>

typedef struct {
	SDL_Color colour;
	bool dotted;     /* every other point: one about to be (a crew forming) */
	bool around;     /* round a MegaMan's feet: its back left out where his body stands */
	int bottom;      /* the picture's rows from here down left alone (BN6's chat box), EMU_H none */
} FloorRing;

/* A ring of `r` world units round (x, y, z), by BN6's displayed camera. */
void floor_ring(int x, int y, int z, int r, const int camera[3], const FloorRing *how);

#endif

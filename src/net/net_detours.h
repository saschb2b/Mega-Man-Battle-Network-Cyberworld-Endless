/* A layer's detours (net_detours.c), for the generator's other files. */
#ifndef CW_NET_DETOURS_H
#define CW_NET_DETOURS_H

#include <stdbool.h>
#include <stdint.h>

#include "net_shapes.h"

extern int16_t ng_detour[MAP_H][MAP_W];

/* (a blue data's detour at least, three panels there and three back; the
 * long walk that holds the best) */
#define DETOUR_BLUE 3

#define DETOUR_FAR 8

bool ng_in_arena(int x, int y);
void ng_measure_detours(void);
void ng_pad_middle(int *x, int *y);

/* The far end of a detour: the farthest free cell, `min` panels off the
 * way or more, of a branch not `taken` yet (which it then takes); false
 * where none is left. One at a time, as each object placed changes where
 * the next may stand. */
typedef struct { int x, y, d; } DetourEnd;

bool ng_detour_end(int min, uint8_t *taken, DetourEnd *out);
void ng_fill_empty_detours(void);

#endif

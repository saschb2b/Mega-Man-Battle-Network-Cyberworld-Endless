/* An area's maps learned (netmap_learn.c), for the map's other files. */
#ifndef CW_NETMAP_LEARN_H
#define CW_NETMAP_LEARN_H

#include <stdbool.h>

#include "netmap_parts.h"

int nm_floordiv(int a, int b);
bool nm_learn(int area, Learned *L);
/* The area's piece in a walled hole (Learned.hole) for its layers' generator. */
void nm_kit_hole(const Learned *L, LayerKit *kit);

#endif

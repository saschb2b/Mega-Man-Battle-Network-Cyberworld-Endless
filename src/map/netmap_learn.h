/* An area's maps learned (netmap_learn.c), for the map's other files. */
#ifndef CW_NETMAP_LEARN_H
#define CW_NETMAP_LEARN_H

#include <stdbool.h>

#include "netmap_parts.h"

int nm_floordiv(int a, int b);
bool nm_learn(int area, Learned *L);

#endif

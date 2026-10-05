/* The town's ports beyond its landmark's (town_ports.c), for town.c. */
#ifndef CW_TOWN_PORTS_H
#define CW_TOWN_PORTS_H

#include <stdbool.h>

#include "area_src.h"

/* A new plan: no ports yet. */
void ports_begin(void);
/* A cell (world units) of port 1 or 2, and its trigger in the original:
 * one of the original's jack-in points (0x40 + n), or 0 for the ground
 * before a check made a port; false where the port holds no more. */
bool ports_cell(int port, int x, int y, int value);
/* Port 1 or 2 a check makes (its trigger `value`): of the original's
 * trigger cells (`c`, `n`), those of that check joined to the one nearest
 * cell (sx, sy), the landmark's, are the port's; ports_in_region says
 * whether cell (cx, cy) is one. One place a port: Green Town's lily ponds
 * are two, a town apart, and one check. */
void ports_region(const CoordCell *c, int n, int port, int value, int sx, int sy);
bool ports_in_region(int port, int cx, int cy);
/* What a trigger cell at (x, y), planned as `value`, is written as: a
 * closed port's ground before a check none (0), the original's own
 * jack-in points jacking in to the first way, as before the ports. */
int ports_value(int x, int y, int value);

#endif

/* The way on over a layer's floor, for the arrow and MegaMan's words: the
 * shortest walk from MegaMan to a panel, and the pad's way (0 right, then
 * clockwise: down-right, down, down-left, left, up-left, up, up-right) to a
 * point a few panels along it. Positions are grid panels, a panel's centre
 * at whole numbers; the grid lies turned on the screen (netmap.c): +x
 * down-right, +y down-left. */
#ifndef NET_ROUTE_H
#define NET_ROUTE_H

#include <stdbool.h>
#include <stdint.h>

#include "net.h"

/* The pad's way of a step (dx, dy) on the grid. */
int route_grid_way(double dx, double dy);

/* The layer's objects gone from its floor (bit i: layer.obj[i]), as a
 * Mystery Data taken: route_way walks through their panels. Whoever asks
 * for the way sets it first; 0 takes every object as standing. */
extern uint64_t route_gone;

/* The way along the floor from MegaMan at (px, py) to panel (tx, ty),
 * around the layer's solid objects; -1 when either is off the floor. *len:
 * the walk's panels. */
int route_way(double px, double py, int tx, int ty, int *len);

/* The walk alone, as route_way's (route_walk, route_walk_len), from a
 * search kept while the target, `key` (the layer's) and route_gone stand:
 * the second screen's map, drawn again as MegaMan walks; its turns as
 * least_turns' (the arrow's own search, route_way, stays as it was). 0,
 * or -1 where none reaches the target. */
int route_walk_kept(double px, double py, int tx, int ty, uint32_t key, int *len);

/* Whether MegaMan walks from panel (sx, sy) to (ax, ay) in a straight line
 * over the floor, clear of what stands on it (as of the last route_way). */
bool route_floor_line(int sx, int sy, int ax, int ay);

/* The last route_way's walk (panels y * MAP_W + x, from the target back)
 * and the panel it aimed at (-1 for none), for the playtest state's map. */
extern int16_t route_walk[MAP_W * MAP_H];
extern int route_walk_len, route_walk_aim;

/* Whether the last route_way's way is still `shown`'s, one of the eight:
 * within `margin` eighths of its middle (a half is its own edge; more holds
 * a way a little past it, so the arrow does not wobble between two). */
bool route_way_holds(int shown, double margin);

#endif

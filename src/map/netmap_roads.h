/* The way's walkways drawn as roads (netmap_roads.c), for the map's other files. */
#ifndef CW_NETMAP_ROADS_H
#define CW_NETMAP_ROADS_H

#include <stdbool.h>

struct TileBook;

/* The current layer's way from its arrival to its exit marked again over
 * its floor as it now stands, where the area has roads: `books`, its
 * roads' art (nbooks of them, 0 none), which says where a road may turn. */
void nm_mark_roads(const struct TileBook *books, int nbooks);
/* Whether grid cell (x, y) lies on that way and is drawn as road. */
bool nm_road(int x, int y);

#endif

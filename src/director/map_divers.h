/* The other divers on the layer's map (the shared dive, issue #728;
 * docs/online/shared-dive.md, Dots on the map): who lends them (the
 * online dive, dive_dots.h, while it is linked) and what the map draws
 * of each (director_map.c), a dot where he stands as MegaMan's is. */
#ifndef CW_MAP_DIVERS_H
#define CW_MAP_DIVERS_H

#include <stdbool.h>

#include <SDL.h>

#define MAP_DIVERS 8   /* a room's eight */

/* One diver: where he stands (world units, as BN6 places MegaMan), and
 * whether he is in MegaMan's crew, his dot then ringed in `colour`. */
typedef struct {
	int x, y;
	bool crew;
	SDL_Color colour;
} MapDiver;

/* The divers on MegaMan's layer, up to `most` into `out`: their count.
 * NULL while no one lends them (no dive linked). Read as the map is
 * drawn: it changes nothing. */
extern int (*map_divers)(MapDiver *out, int most);

#endif

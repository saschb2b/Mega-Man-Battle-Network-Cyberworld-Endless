/* The dev tour: the game shows every room of each area's layer, warped to
 * in turn and saved (tour.c, docs/DEVTOOLS.md). */
#ifndef CW_TOUR_H
#define CW_TOUR_H

#include <stdbool.h>

/* `spec` DIR[:BIOMES]; turns the tour on (it also sets dev's quiet and god). */
bool tour_parse(const char *spec);
bool tour_on(void);
/* Once a game frame, after the director. */
void tour_update(void);

#endif

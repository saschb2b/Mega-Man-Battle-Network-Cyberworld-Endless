/* Home's indoor places, for the director (home_places.c, docs/HOME.md):
 * Lan's house and room, AsterLand, the Cyber Academy's open maps. */
#ifndef CW_HOME_PLACES_H
#define CW_HOME_PLACES_H

#include <stdbool.h>

/* Which of them map (group, number) is */
enum { HOME_PLACE_NONE, HOME_PLACE_ROOM, HOME_PLACE_HOUSE, HOME_PLACE_ASTER, HOME_PLACE_ACADEMY };
int home_places_at(int group, int number);

/* All of them installed, Lan's PC jacking in to world (x, y) of map
 * (to_group, to_number). False where one cannot be. */
bool home_places_install(int to_group, int to_number, int x, int y);
/* The name the map's label shows ("AsterLand"); NULL for none of them. */
const char *home_places_name(int group, int number);
/* The way on in one of them: the PC in Lan's room, the stairs up in his
 * house, elsewhere the way out (world units); false for none. */
bool home_places_way(int group, int number, int *x, int *y);
/* Each frame in one of them: the doors it keeps shut. */
void home_places_frame(int group, int number);
/* What A reads from world (x, y) facing `face` (0-7) in one of them where
 * the place answers instead of BN6 (AsterLand's Number Trader); NULL
 * elsewhere. */
const char *home_places_check(int group, int number, int x, int y, int face);

#endif

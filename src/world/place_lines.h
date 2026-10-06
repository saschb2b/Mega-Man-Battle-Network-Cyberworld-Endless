/* The people of home's indoor places and what is said there
 * (place_lines.c), for indoors.c. */
#ifndef CW_PLACE_LINES_H
#define CW_PLACE_LINES_H

#include "town_lines.h"   /* Folk, FACE_* */

/* Someone in AsterLand or the Cyber Academy: the map they stand on (its
 * group and number), where, which way, who and what they say (Folk, in the
 * map's own world units), for one behind a counter how far towards +y the
 * counter's front is, where they are spoken to (0 none), for one who posts
 * requests (jobs.h) which asker, plus one (0 none), and for one who keeps
 * a shop BN6's shop, plus one (0 none): their words its greeting. */
typedef struct {
	int group, number;
	Folk folk;
	int counter;
	int asker;
	int shop;
} PlaceFolk;
extern const PlaceFolk place_folk[];
extern const int place_nfolk;

/* What A at AsterLand's Number Trader reads: it is off in a run (docs/
 * HOME.md, piece 10). */
extern const char *const place_number_trader;
/* What AsterLand's clerk says once this visit's order is made. */
extern const char *const place_order_closed;

#endif

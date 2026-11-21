/* A crewmate pulled back (issue #718; docs/online/shared-dive.md, A
 * deletion in a crew): a crew's battle won with a MegaMan of it deleted,
 * each crewmate still standing gives a quarter of the HP he ends with, and
 * the fallen is back on the layer with the sum, half his own max HP at
 * most and 1 at least; once a dive. Worked out alike on every game of the
 * field from its seats as the battle ended, so no message carries it. */
#ifndef CW_CREW_PULL_H
#define CW_CREW_PULL_H

#include <stdbool.h>

#define CREW_PULL_SEATS 4

typedef struct {
	unsigned n;                           /* the field's seats */
	unsigned hp[CREW_PULL_SEATS];         /* each MegaMan's HP as the field ended (0 deleted) */
	unsigned max_hp[CREW_PULL_SEATS];     /* ... and his max */
	bool spent[CREW_PULL_SEATS];          /* his dive's pull-back used already */
	bool gone[CREW_PULL_SEATS];           /* his game left the field */
} CrewPullIn;

typedef struct {
	unsigned hp[CREW_PULL_SEATS];         /* each MegaMan's HP after: a giver's less his gift, a fallen's back */
	unsigned gave[CREW_PULL_SEATS];       /* what each gave */
	bool pulled[CREW_PULL_SEATS];         /* each fallen pulled back */
} CrewPullOut;

/* The gifts after a crew's battle (`won`): the fallen in seat order, each
 * standing seat giving in seat order a quarter of his HP as it stands,
 * the last gift cut to the fallen's half. A lost battle, one won with
 * nobody standing, a seat whose pull-back is spent or whose game left: as
 * the field ended. */
void crew_pull(const CrewPullIn *in, bool won, CrewPullOut *out);

#endif

/* Home after an act (director_home.c, docs/HOME.md), for the director's
 * other parts. */
#ifndef CW_DIRECTOR_HOME_H
#define CW_DIRECTOR_HOME_H

#include <stdbool.h>

/* Whether the exit MegaMan steps on now leads home: an act's guardian
 * deleted, on the act's last layer (not the short net's Nest). */
bool home_due(void);
/* The exit's warp turned home, the next act's first layer built and
 * `beaten` the guardian deleted on the way: the run's town installed with
 * that layer behind its port, and the warp BN6's own jack-out to where Lan
 * jacked in. False where the town cannot be built (the warp goes on into
 * the net). */
bool home_begin(const char *beaten);

/* Whether map (group, number) is the run's town. */
bool home_map(int group, int number);
/* The town's frame, at the run's start and at home: Lan's words, the walk
 * to the port, and the next layer's arrival after his jack-in. */
void home_update(void);

/* A CONTINUE at home, before its state loads: the town installed again
 * with the next layer (built) behind its port. */
bool home_rebuild(void);
/* ... and after it: Lan entered into the town where he stood. */
void home_resume(void);

#endif

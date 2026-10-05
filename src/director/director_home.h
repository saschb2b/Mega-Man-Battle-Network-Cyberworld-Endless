/* Home after an act (director_home.c, docs/HOME.md), for the director's
 * other parts. */
#ifndef CW_DIRECTOR_HOME_H
#define CW_DIRECTOR_HOME_H

#include <stdbool.h>

#include "run.h"

/* Whether the exit MegaMan steps on now leads home: an act's guardian
 * deleted, on the act's last layer (not the short net's Nest). */
bool home_due(void);
/* The exit's warp turned home, the next act's first layer built and
 * `beaten` the guardian deleted on the way: the run's town installed, its
 * jack-in to Lan's HP, Lan's HP with the next act's portals, the first
 * way's to that layer, and the warp to Lan's HP's arrival. False where
 * they cannot be built (the warp goes on into the net). */
bool home_begin(const char *beaten);
/* ... and at a run's start, the first layer built: the town planned and
 * installed, its jack-in to Lan's HP, the portal to the layer, and Lan at
 * the town's start (`abandoned` that the run before was given up, which
 * its people speak of). False where they cannot be built. */
bool home_run_start(bool abandoned);

/* The next act's ways at home (Lan's HP's portals, docs/HOME.md): how many
 * are open, and whether a dark way stands sealed. */
const RunWay *home_ways(int *n, bool *dark_sealed);
/* Portal `k` taken: its way's first layer built where it was another's,
 * and the portal pointed at it. */
bool home_take_way(int k);
/* Whether map (group, number) is home: the run's town or Lan's HP
 * (director_town.c, as the rest below); whether MegaMan is in Lan's HP. */
bool home_map(int group, int number);
bool home_in_hp(void);
/* The place's name for the map's label: Lan's HP, or the town's. */
const char *home_place_name(void);
/* The home's frame: Lan's and MegaMan's words, the portals, and the next
 * layer's arrival; home_entered after the game enters a map (its portals'
 * flags). */
void home_update(void);
void home_entered(void);

/* A CONTINUE at home, before its state loads: the town installed again
 * with the next layer (built) behind its port. */
bool home_rebuild(void);
/* ... and after it: Lan entered into the town where he stood. */
void home_resume(void);

#endif

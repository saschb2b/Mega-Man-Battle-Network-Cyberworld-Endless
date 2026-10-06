/* Home after an act (director_home.c, docs/HOME.md), for the director's
 * other parts. */
#ifndef CW_DIRECTOR_HOME_H
#define CW_DIRECTOR_HOME_H

#include <stdbool.h>
#include <stdint.h>

#include "director_jobs.h"   /* (jobs at home, its other part) */
#include "run.h"

/* Whether the exit MegaMan steps on now leads home: an act's guardian
 * deleted, on the act's last layer (not the short net's Nest). */
bool home_due(void);
/* The exit's warp turned home, the next act's first layer built and
 * `beaten` the guardian deleted on the way: the run's town and Lan's house
 * installed, his PC's jack-in to Lan's HP, Lan's HP with the next act's
 * portals, the first way's to that layer, and the warp to Lan's HP's
 * arrival. False where they cannot be built (the warp goes on into the
 * net). */
bool home_begin(const char *beaten);
/* ... and after a trip back, its exit taken and the run's own layer made
 * again (director.c): home with the trip's words. */
bool home_return(void);
/* ... and at a run's start, the first layer built: the town planned and
 * installed with Lan's house, his PC's jack-in to Lan's HP, the portal to
 * the layer, and Lan at the top of his room's stairs (`abandoned` that the
 * run before was given up, which its people speak of). False where they
 * cannot be built. */
bool home_run_start(bool abandoned);

/* The next act's ways at home (Lan's HP's portals, docs/HOME.md): how many
 * are open, and whether a dark way stands sealed. */
const RunWay *home_ways(int *n, bool *dark_sealed);
/* Portal `k` taken: its way's first layer built where it was another's,
 * and the portal pointed at it; the older portals open again for the next
 * visit. */
bool home_take_way(int k);
/* The portals lit in Lan's HP (a bit each, lanhp_lit): the ways, and the
 * older portals not yet taken this visit. */
unsigned home_lit(void);
/* The area older portal `k` goes back to (docs/HOME.md, going back), -1
 * where `k` is none, or taken this visit. */
int home_older(int k);
/* Older portal `k` taken: a notch on the Net's clock, the trip's layer
 * built (the older act's band, LAYER_BACK) and the portal pointed at it;
 * the run comes home to its own depth (run.home_depth). False where none
 * could be made. */
bool home_go_back(int k);
/* Whether map (group, number) is home: the run's town, Lan's house or
 * room, or Lan's HP (director_town.c, as the rest below); whether MegaMan
 * is in Lan's HP. */
bool home_map(int group, int number);
bool home_in_hp(void);
/* The place's name for the map's label: Lan's HP, his house or room,
 * AsterLand, the Cyber Academy, or the town's. */
const char *home_place_name(void);
/* Whether Lan stands in one of home's indoor places (his house and room,
 * AsterLand, the Cyber Academy). */
bool home_indoors(void);
/* What A reads ahead of Lan where home's place answers instead of BN6
 * (home_places_check); NULL elsewhere. */
const char *home_check(void);
/* Whether A ahead of Lan talks to a keeper across a counter (AsterLand's
 * clerk and SubChip seller, home_places_counter): their talk's archive and
 * script. */
bool home_counter(uint32_t *archive, int *script);
/* Whether AsterLand's order of this visit is still to make, and the zenny
 * held pays for one. */
bool home_order_open(void);
/* The way on at home from where Lan or MegaMan stands (way_to's word,
 * way_last its direction, `far` how far): the PC in Lan's room, the room
 * up the house's stairs, the way out of AsterLand and the Academy, home
 * from the town (a town not home: its port), the pink pad in Lan's HP. */
const char *home_way(int *far);
/* L at home: MegaMan's word on that way. */
const char *home_status(void);
/* The town's hour at this visit (director_town_hour). */
int home_hour(void);
/* The home's frame: Lan's and MegaMan's words, the portals, and the next
 * layer's arrival; home_entered after the game enters a map (its portals'
 * flags). */
void home_update(void);
void home_entered(void);

/* A CONTINUE at home, before its state loads: the town and Lan's house
 * installed again, the next layer (built) behind Lan's HP's portal. */
bool home_rebuild(void);
/* ... and after it: Lan or MegaMan entered into the home's map where he
 * stood. */
void home_resume(void);

#endif

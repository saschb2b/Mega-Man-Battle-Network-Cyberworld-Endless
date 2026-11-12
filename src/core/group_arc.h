/* An act's arc of encounter groups (docs/game/progression/encounter-groups.md,
 * the design's third point; groups.h): each act of a run is about one
 * virus family, its signature, drawn by the run's seed. The act's first
 * layer meets it alone (one of it, eased), the second in a pair
 * with a partner, the third in its constellation; the Server's challenge
 * is the hardest constellation the signature stands in. One group battle
 * a layer; the layer's other battles are the area's own.
 *
 * A signature is one of the area's own families in a share of runs that
 * falls by act (the owner's call 1: three quarters of act 1's, half of
 * act 2's, a third of act 3's, a quarter of act 4's; the deep free), and
 * a family can only be one where it can stand alone inside the eased
 * limits and some template pairs it.
 *
 * Numbers only, no ROM: the pool and the limits reach it through its
 * caller (group_pool.h). The build's step 2: behind `--dev arc`. */
#ifndef CW_GROUP_ARC_H
#define CW_GROUP_ARC_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"
#include "groups.h"

/* The beats: an act's three layers, in order */
enum { ARC_ALONE, ARC_PAIR, ARC_CONSTELLATION, ARC_BEATS };

/* What an act's arc is rolled under */
typedef struct {
	GroupLimits eased;       /* the first beat's limits (groups.h's GROUP_EASED) */
	GroupLimits plain;       /* the pair's and the constellation's */
	GroupLimits challenge;   /* the Server's (GROUP_CHALLENGE) */
	FamilyMask home;         /* the area's own families (BN6's AI indexes) */
	int home_share;          /* how many runs in ARC_SHARE_OF take a signature of the area's own; -1 any family (the deep) */
	bool second;             /* the partner stands in the constellation too, where one holds both (from the third act) */
} ArcRules;
#define ARC_SHARE_OF 12
/* The share of act `act` (0-5, 6 the Nest): 9, 6, 4 and 3 of twelve for
 * the surface's four acts, -1 after them */
int arc_home_share(int act);

/* An act's arc: the signature and its partner (the pool's indexes; -1 no
 * partner), whether the signature is one of the area's own, each beat's
 * group, and the Server's challenge where a constellation holds the
 * signature inside the challenge's limits */
typedef struct {
	int16_t signature, partner;
	bool home, has_challenge;
	GroupFill beat[ARC_BEATS];
	GroupFill challenge;
} GroupArc;

/* The layer an act's arc is made at, whichever of its layers asks: the
 * act's first. What an arc is made under must be the act's alone: an older
 * net's area's own families are its records' of the layer (xbattles.h), so
 * rules read at the layer asked made a game started on an act's third
 * layer (a CONTINUE) name another signature than the act's card had. */
int group_arc_depth(int depth);

/* The arc `roll` (the run's seed and the act mixed) gives from `pool`
 * under `r`: the signature each candidate as likely as any of its side
 * (the area's own or not, by the share; the other side where its own has
 * none), the pair one of the two-family fillings that hold it, the
 * constellation one of the three-family ones (with the partner where
 * `second` and one holds both), else three of it, else the hardest pair
 * with another partner, else the pair again; the challenge the filling of
 * most pressure that holds it. No random numbers drawn. False where no
 * family can be a signature. */
bool group_arc_make(const GroupFam *pool, int n, const ArcRules *r, uint32_t roll, GroupArc *out);

#endif

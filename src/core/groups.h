/* Encounter groups (docs/game/progression/encounter-groups.md): one pool of
 * virus families over every ROM read, each tagged by what it does, and
 * the authored constellations (templates) a battle is built from in
 * place of an area's own record: a wall and its artillery, a panel war,
 * two elements that clash. A template's slots say what a family must be
 * to stand in each; a group is filled from the act's pool at the versions
 * that keep it inside the act's band (its HP together, each one's hit)
 * and under the act's pressure cap, the third limit, measured
 * (docs/game/progression/encounter-groups-measured.md).
 *
 * The build's step 1: behind `--dev groups` (group_pool.h), no run of a
 * player's holds one. Numbers only, no ROM: the pool reaches it through
 * its caller, so it builds and tests without one. */
#ifndef CW_GROUPS_H
#define CW_GROUPS_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"
#include "pacing.h"

/* Where a family's own game sets it most (its lean): the back where two
 * of five of its placements or more are in the enemy's last column and no
 * column holds more, the front likewise for the first, else neither. The
 * visitors' role (visitors.h) asks three in four, which gives seven
 * families of 65 one. */
enum { GROUP_FREE, GROUP_BACK, GROUP_FRONT };
int group_lean(int front, int middle, int back);

/* What a family does that a battle is built around (its tricks, measured
 * alone: tools/encounter_danger.py families) */
enum {
	GROUP_BREAKS = 1,    /* changes the panels of MegaMan's side */
	GROUP_SHAPES = 2,    /* takes panels, or changes its own side's */
	GROUP_SUMMONS = 4,   /* brings more enemies than the battle set */
	GROUP_TOUGH = 8,     /* the starting folder does not delete its V1, alone, in 50 seconds */
};

/* The nets a family is of, as visitor_families.h's VISIT_* (0 BN6's, 1
 * BN5's, 2 BN4's), and a set of them a bit each */
#define GROUP_NETS 3
#define GROUP_NETS_ALL 7u

#define GROUP_VERSIONS 6     /* BN6's four (V1 to SP), the older nets' six (V1 to V3EX) */
#define GROUP_POOL_MAX 128
#define GROUP_SIZE 3         /* a group's families at most (a swarm: three of one) */

/* One version of a family: its enemy id (-1: none), HP, hardest hit and
 * element (data.h's ELEM_*, 0 none) */
typedef struct { int16_t id, hp, hit; int8_t elem; } GroupVer;

/* A family of the pool */
typedef struct {
	int16_t family;      /* BN6's AI index (BN5's moved from 32, BN4's from 64) */
	int8_t net;          /* 0 BN6's, 1 BN5's, 2 BN4's */
	int8_t lean;         /* GROUP_FREE, GROUP_BACK, GROUP_FRONT */
	int8_t stage;        /* the stage it first meets MegaMan in its own net (loot.h's loot_family_stage) */
	uint8_t tricks;      /* GROUP_BREAKS ... */
	int16_t pressure;    /* the HP a standing MegaMan loses in 20 seconds to its V1 alone; -1 not measured */
	GroupVer ver[GROUP_VERSIONS];
} GroupFam;

/* A family's measured tags (group_tags.c, written by tools/encounter_tags.py
 * engine from tools/encounter_data/families.tsv): numbers alone */
typedef struct { int8_t net; int16_t family; int16_t pressure; uint8_t tricks; } GroupTag;
extern const GroupTag group_tags[];
extern const int group_tags_count;
/* ... of net `net`'s family `family`, NULL where it was not measured */
const GroupTag *group_tag(int net, int family);
/* Two families that press harder together than their own pressures add
 * up to (the pairs' pass, tools/encounter_danger.py pairs: every two of
 * the pool fought once at V1), and by how much in 20 seconds */
typedef struct { int8_t net_a; int16_t family_a; int8_t net_b; int16_t family_b; int16_t extra; } GroupPair;
extern const GroupPair group_pairs[];
extern const int group_pairs_count;
/* ... of the two families (either way round), 0 where they do not */
int group_pair_extra(int net_a, int family_a, int net_b, int family_b);

/* The templates */
enum {
	GROUP_WALL,          /* a front-row or tough family, and a back-row shooter behind it */
	GROUP_PANEL_WAR,     /* one that changes MegaMan's panels, and one that roams */
	GROUP_SUMMONER,      /* one that brings more enemies, behind a front or tough one */
	GROUP_CLASH,         /* two elements next to each other on the wheel */
	GROUP_ONE_ELEMENT,   /* two families of one element */
	GROUP_PINCER,        /* the front, the back and a roamer */
	GROUP_SWARM,         /* three of one family */
	GROUP_HELD_GROUND,   /* one that takes or reshapes panels, and a back-row shooter */
	GROUP_GLASS,         /* two that hit near the act's cap, their HP in the band's lower half */
	GROUP_BULWARK,       /* one three fifths of the battle's HP hitting softly, one hitting near the cap */
	GROUP_NETS_MEET,     /* a roamer of each of two nets */
	GROUP_TEMPLATES
};
const char *group_template_name(int t);

/* What a group at a depth is held to: the act's band (its HP together
 * from lo to hi, each hit to cap), the highest version each net's
 * families take (BN6's the act's, the older nets' by tier: BN6's V1 lets
 * their V1EX), the stage the run has reached, the nets present and the
 * pressure cap */
typedef struct {
	PacingBand band;
	int8_t top[GROUP_NETS];
	int8_t stage;
	uint8_t nets;
	int pressure;
} GroupLimits;
/* ... at `depth`, the run at stage `stage` (loot.h's loot_stage_allowed),
 * with nets `nets` */
GroupLimits group_limits(int depth, int stage, unsigned nets);
/* The pressure cap of act `act` (0-5, 6 the Nest) on the first cycle: the
 * three-quarter mark of the pressure predicted of the act's areas' own
 * battles (docs/game/progression/encounter-groups.md, Danger). On later
 * cycles group_limits scales it with the band's cap on one hit, as the
 * prediction scales with the hit (assumed, not measured). */
int group_pressure_cap(int act);

/* A template filled: the pool's families (their indexes, a swarm's the
 * same three times), the version of each, the group's HP together and its
 * pressure as its families' own predict it (each one's measured alone at
 * V1, by its hit at the version fought over its V1's; and for two the
 * pairs' pass found pressing harder together, that excess by the larger
 * of their hits over their V1's) */
typedef struct {
	int8_t template, n;
	int16_t fam[GROUP_SIZE];
	int8_t ver[GROUP_SIZE];
	int hp, pressure;
} GroupFill;

/* Whether pool family `f` may be in a battle under `l` at all: its net
 * present, its stage reached, a version of it inside the band */
bool group_in_pool(const GroupFam *f, const GroupLimits *l);
/* Every way `pool` (n families) fills template `t` under `l`, a family
 * once a group and a pair of alike slots once: each handed to `each`
 * (NULL: counted alone) in a fixed order; how many. No random numbers. */
int group_fillings(const GroupFam *pool, int n, const GroupLimits *l, int t, void (*each)(const GroupFill *g, void *ctx), void *ctx);
/* One of them, each as likely as any, by `roll` (any 32 bits: a layer's
 * seed and the battle's index mixed, so a checkpoint rolls the same):
 * false where the pool fills the template in no way */
bool group_pick(const GroupFam *pool, int n, const GroupLimits *l, int t, uint32_t roll, GroupFill *out);
/* ... where the caller has counted them (`count`: group_fillings's) */
bool group_pick_counted(const GroupFam *pool, int n, const GroupLimits *l, int t, int count, uint32_t roll, GroupFill *out);
/* ... of any template the pool fills, each template as likely as any */
bool group_pick_any(const GroupFam *pool, int n, const GroupLimits *l, uint32_t roll, GroupFill *out);
/* The template `roll` takes among those with a filling (counts[t] over
 * 0), -1 where none has */
int group_template_roll(const int counts[GROUP_TEMPLATES], uint32_t roll);

/* The group on the enemy's panels, as `e`'s foes (its field and the rest
 * left as they are): the front's in the first column, the back's in the
 * last, a roamer's between, on the rows 2, 1, 3; a swarm's one a column */
void group_place(const GroupFam *pool, const GroupFill *g, Encounter *e);

#endif

/* The virus families of each net as their own game sets them, for the
 * visitors (visitors.h; issue #82, docs/MULTIROM.md, One net: the design):
 * BN6's from its areas' random battles, BN5's from the records of the
 * areas it lends a run (NetAreaDef.xbattles), fought in BN6's engine.
 * Each family's role (where its game sets it: the back, the front, or
 * either), its stage (the depth it first meets MegaMan in its own net),
 * whether its game sets it on special panels alone, and whether it may
 * visit the other net's battles. Read from the ROM files and the families
 * installed in the core, once; none where BN5's ROM is not read or its
 * families are not in the engine. */
#ifndef CW_VISITOR_FAMILIES_H
#define CW_VISITOR_FAMILIES_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* The nets: a battle's own, and the visitor's the other */
enum { VISIT_BN6, VISIT_BN5 };

typedef struct {
	int family;          /* BN6's AI index (BN5's moved families from 32) */
	int8_t role;         /* VISITOR_* (visitors.h): where its own game's records set it */
	int8_t stage;        /* the stage it first meets MegaMan in its own net (loot.h's loot_family_stage: 0 the surface, 1 the Undernet, 2 the Graveyard) */
	bool bound;          /* its game sets it on special panels alone (none plain or cracked): a sea's, lava's */
	bool visits;         /* one of its net's own the other has none of by name, not bound: it may visit */
	int16_t placed;      /* its placements in its game's records, which the role is of */
} VisitorKin;

#define VISITOR_KIN_MAX 64

/* Net `net`'s families as its records set them (into *out), how many; 0
 * where BN5's ROM is not read or none of its families is installed */
int visitor_kin(int net, const VisitorKin **out);
/* The role of `family` in net `net`'s records (VISITOR_*), VISITOR_FREE
 * where they set it nowhere */
int visitor_role_in(int net, int family);
/* The enemy id of net `net`'s family `family` at `version` (BN6's 0-3,
 * V1 to SP; BN5's 0-5, V1 to V3EX, as installed), -1 none */
int visitor_id(int net, int family, int version);
/* The panel's type at column `col`, row `row` (from 0) of battle `e`'s
 * field, as its own game numbers it (2 plain, 3 cracked in both): BN5's
 * field for one of its records (Encounter.xrecord), BN6's else; -1 none */
int visitor_panel(const Encounter *e, int col, int row);

#endif

/* Where a family of another net stands and what MegaMan says of the
 * first one a profile meets (docs/multirom/one-net.md; issue #82). The
 * one-in-four visitor that took a place in a finished battle from the
 * second act, and the deep's coin between the nets, are gone since the
 * encounter groups (docs/game/progression/encounter-groups.md, What it
 * replaces): a family of another net comes as an act's signature or its
 * partner, in a battle shaped for it (group_arc.h). What the visitors
 * measured, each family's place in its own game's records
 * (visitor_families.h), is the groups' first tag. */
#ifndef CW_VISITORS_H
#define CW_VISITORS_H

#include <stdbool.h>
#include <stdint.h>

#include "foes.h"

/* A family's role, where its own game's records set it: the back where
 * three of four of its placements are in the last column, the front where
 * most are in the first enemy column, else either */
enum { VISITOR_FREE, VISITOR_BACK, VISITOR_FRONT };
/* ... from its placements in the enemy's columns, first to last */
int visitor_role(int front, int middle, int back);

/* What MegaMan says of the first family of another net a profile meets, once (the telegraph:
 * docs/multirom/one-net.md; docs/multirom/bn4.md, the owner's call 5): BN5's
 * and BN4's families are both the older Net's, so one of either in a
 * battle of BN6's is the older Net's, one of BN6's in a battle of BN5's
 * is ours, and one of BN4's in a battle of BN5's is of no other net:
 * nothing said, and the once kept */
enum { VISITOR_UNSAID, VISITOR_OLDER_NETS, VISITOR_OURS };
/* ... of a visitor of net `net` in a battle of net `home` (both
 * visitor_families.h's VISIT_*: a battle's BN5's where it is one of its
 * records, Encounter.xrecord) */
int visitor_telegraph(int home, int net);

#endif

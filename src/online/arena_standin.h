/* Stand-ins under BN6's rules (docs/ONLINE_ARENA.md, Formats). A run has
 * one folder, Folder 1, so a chip of BN5's in it would keep its MegaMan
 * from a NetBattle with a game that has no BN5. For that battle alone each
 * such chip sits out and one of BN6's own from the run's Pack stands in:
 * the same kind (standard, Mega, Giga) and element where the Pack has a
 * spare, its power the nearest, within BN6's folder rules (four of a chip,
 * the run's Mega and Giga limits, one of each); BN6's first standard chip
 * the folder has room for, Cannon, where the Pack has none. The carried
 * copy changes, never the run's folder. The carrying game alone picks
 * them: the other reads the folder as it is sent. */
#ifndef CW_ARENA_STANDIN_H
#define CW_ARENA_STANDIN_H

#include <stdint.h>

#include "online_snapshot.h"

/* A chip that sat out and the one that stood in: folder entries (chip |
 * code << 9) */
typedef struct {
	uint16_t out, in;
} ArenaStandIn;

/* `l`'s folder with each of BN5's chips stood in for, its Regular and
 * TagChips let go where one of theirs did; the stand-ins in `list`, at
 * most `cap` (it may be NULL), their count returned */
int arena_standins(OnlineLoadout *l, ArenaStandIn *list, int cap);

#endif

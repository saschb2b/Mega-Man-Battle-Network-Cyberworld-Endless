/* --dev arena2v2: a partner's NaviCust compiled by BN6's own code (epic
 * #167, docs/ONLINE_ARENA.md, His NaviCust, compiled by BN6). BN6 makes a
 * NaviStats' abilities from the NaviCust's board in its menus
 * (BN6_NAVICUST_RELOAD: the stats put back to the Navi's own, then each
 * program's routine run over them, then the bugs), and in a NetBattle
 * each GBA's lead has them made so on his own GBA on the way in. A
 * partner has no GBA in the pair, so his own client runs the same routine
 * on a core of its own: his checkpoint's state loaded into a core of the
 * plain ROM, his loadout written in as the Arena writes a lead's
 * (arena_loadout.h), BN6's routine called once in the game's update
 * (the game's own call, as gamecall.c's), and the NaviStats it leaves
 * read out. What crosses is that record, the same on every client. */
#ifndef CW_ARENA_TEAM_NAVICUST_H
#define CW_ARENA_TEAM_NAVICUST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"

/* `l`'s NaviStats as BN6 compiles them (BN6_BATTLE_NAVI_SIZE bytes into
 * `out`, its HP whole as a NetBattle begins), on a core of the player's
 * ROM loaded from `state` (n bytes); false, the reason in `why`, where
 * the core, the loadout or BN6's call failed */
bool team_navicust_compile(const uint8_t *state, size_t n, const OnlineLoadout *l, uint8_t *out, char *why, size_t wn);

#endif

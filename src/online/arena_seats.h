/* Two MegaMen a side over the network (protocol v15, issue #181,
 * docs/ONLINE_ARENA.md, Two MegaMen a side over the server): what every
 * game of a 2v2, and its spectators, sets alike before the pair is built.
 * The pair's two MegaMen a side (arena_team.h's hooks) are the dev flag's
 * (--dev arena2v2) or a 2v2's; the GBAs' MegaMen are seats 0 and 1's,
 * built into the pair as a NetBattle's, and the partners' are seats 2 and
 * 3's: each one's folder dealt into his own deck (arena_team_custom.h),
 * his HP and buster in his own NaviStats (arena_team_navi.h), the same on
 * every client, the pair's hash over them. */
#ifndef CW_ARENA_SEATS_H
#define CW_ARENA_SEATS_H

#include <stdbool.h>

#include "online_snapshot.h"

/* the pair holds two MegaMen a side: --dev arena2v2's, or a 2v2's (set below) */
bool arena_seats_on(void);
/* a 2v2's pair to be built (true), or a NetBattle of two's (false) */
void arena_seats_set(bool on);
/* the pair a 2v2's over the network (or a spectator's of one), not the dev flag's */
bool arena_seats_net(void);
/* teampa=1 on a partner's own game: the MegaMan he carries into a 2v2
 * given Cannon A, B and C as his folder's first chips, Regular chip and
 * TagChips (dealt first: a Program Advance in his first hand), his own
 * folder as it crosses to every game */
void arena_seats_pa_folder(OnlineLoadout *l);
/* the partners' MegaMen (side 0's, side 1's; NULL: the game's start's,
 * his lead's folder and numbers), before the pair is built */
void arena_seats_partners(const OnlineLoadout *partner[2]);

#endif

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
#include <stddef.h>
#include <stdint.h>

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
/* teamunite=1 on a partner's own game: the MegaMan he carries in holds
 * NumberSoul and has Atk+10 * as his folder's first chip and Regular chip
 * (dealt first: a chip that lights NumberSoul in his first hand), his own
 * as he crosses to every game */
void arena_seats_unite_folder(OnlineLoadout *l);
/* the partners' MegaMen (side 0's, side 1's; NULL: the game's start's,
 * his lead's folder and numbers), before the pair is built */
void arena_seats_partners(const OnlineLoadout *partner[2]);
/* ... one side's MegaMan for the way in to compile (NULL none: his
 * record his side's with his loadout's numbers); the dev flag's
 * stood-in partners' too */
void arena_seats_partner(unsigned side, const OnlineLoadout *l);
/* From the way in (arena_boot.c), over `base`, the boot's own state (n
 * bytes: the game's start, the same bytes on every game and spectator):
 * each given partner's NaviCust compiled by BN6's own reload
 * (arena_team_navicust.h), his record from it; false (the reason in
 * `why`) where one could not be, and the way in fails on every game
 * alike, no game left with another record */
bool arena_seats_compile(const uint8_t *base, size_t n, char *why, size_t wn);

#endif

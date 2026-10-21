/* --dev arena2v2: the partners' own clients, stood in for on this machine
 * (epic #167, docs/ONLINE_ARENA.md, A partner's own Custom screen). Each
 * partner's client has BN6's own Custom screen on a plain copy of the pair
 * of its own (arena_team_screen.h), made as his side's GBA first opens its
 * Custom screen; while that GBA's screen is open his keys drive it (a
 * script by the frame's number), and once his OK is pressed, or that
 * GBA's screen closes first, his picks go out in his stream as pick
 * messages (arena_team_custom.h), which the local link carries as a
 * network would. Never part of the pair, its hash or a spectator's game.
 * `teamseat=N` shows seat N's screen (arena_team_view.h). */
#ifndef CW_ARENA_TEAM_CLIENT_H
#define CW_ARENA_TEAM_CLIENT_H

#include <stdint.h>

#include "arena.h"
#include "online_snapshot.h"

/* the battle begun: side 0's partner's folder the carried MegaMan's next
 * folder that holds chips after the lead's (`all` NULL, or none: his
 * lead's, as side 1's partner's is); teamseat's GBA shown */
void team_client_begin(Arena *a, const OnlineLoadout *all);
/* the clients let go (a battle's end, the scene's) */
void team_client_free(void);
/* player's (2 or 3) stream for `frame`: his keys, released on his Custom
 * screen, which they drive, or a pick message */
uint16_t team_client_keys(Arena *a, unsigned player, uint32_t frame);
/* teamseat's screen: its GBA's picture (a partner's BN6's own Custom
 * screen on his copy of the pair while it is open) and the seat's own
 * drawing over it */
void team_client_draw(Arena *a);
/* said in the output: the two GBAs compared (arena_team.h), and each
 * client's screen, its frames and their cost */
void team_client_report(void);

#endif

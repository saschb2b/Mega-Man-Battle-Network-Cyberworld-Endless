/* The Net Square built into the running game (online_square.c,
 * docs/ONLINE.md): its plan from the server's seed (square_plan.h), BN6's
 * own map engine drawing it and walking the other players' MegaMen. */
#ifndef CW_ONLINE_SQUARE_H
#define CW_ONLINE_SQUARE_H

#include <stdbool.h>
#include <stdint.h>

#include "online_types.h"

/* The peers a channel shows: every player of it but MegaMan himself. */
#define SQUARE_PEERS (ONLINE_MAX_PLAYERS - 1)

/* The Square for seed `seed` installed in place of the run's maps and
 * MegaMan warped to its arrival, as player `player_id`. */
bool online_square_build(uint32_t seed, uint32_t player_id);
bool online_square_arrived(void);
/* The plan's and the drawn tiles' hash (two clients' Squares compared). */
uint32_t online_square_hash(void);
/* The channel keeper's words: a text archive (his talk its script 0). */
bool online_square_keeper_say(const uint8_t *archive, int n);
/* Room for the Square's chats: a text archive of TALK_ROOM bytes. */
uint32_t online_square_talk_room(void);
#define SQUARE_TALK_ROOM 2048
/* Whether world (x, y) stands on the pad home. */
bool online_square_on_exit(int x, int y);
/* The peers to show, slot by slot, moved by BN6 between frames. */
void online_square_peers(const OnlineSnapshot *shown);
/* The slot of the peer MegaMan at (x, y) facing `face` (0-7) talks to
 * with A; -1 none. */
int online_square_peer_ahead(int x, int y, int face);
/* Where BN6 actually placed this peer's NPC last frame, for the trace. */
bool online_square_peer_position(unsigned slot, int *x, int *y, int *z);

#endif

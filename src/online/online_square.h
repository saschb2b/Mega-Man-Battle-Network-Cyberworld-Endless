/* The Net Square built into the running game (online_square.c,
 * docs/ONLINE.md): its plan from the server's seed (square_plan.h), BN6's
 * own map engine drawing it and walking the other players' MegaMen. */
#ifndef CW_ONLINE_SQUARE_H
#define CW_ONLINE_SQUARE_H

#include <stdbool.h>
#include <stdint.h>

#include "online_types.h"
#include "square_plan.h"

/* The peers a channel shows: every player of it but MegaMan himself. */
#define SQUARE_PEERS (ONLINE_MAX_PLAYERS - 1)

/* The Square for seed `seed` installed in place of the run's maps and
 * MegaMan warped to its arrival, as player `player_id`. */
bool online_square_build(uint32_t seed, uint32_t player_id);
bool online_square_arrived(void);
/* The plan's and the drawn tiles' hash (two clients' Squares compared). */
uint32_t online_square_hash(void);
/* The plan the Square was built from (its stage and ring stand raised
 * where the area's maps have stairs). */
const SquarePlan *online_square_plan(void);
/* The Square's Mr.Progs: the channel keeper by the plaza's way in, the
 * greeter in the log-in plaza, the referee by the Arena's ring; each
 * one's words a text archive (his talk its script 0). */
enum { SQUARE_PROG_KEEPER, SQUARE_PROG_GREETER, SQUARE_PROG_REFEREE, SQUARE_PROGS };
bool online_square_prog_say(int who, const uint8_t *archive, int n);
bool online_square_keeper_say(const uint8_t *archive, int n);
/* Trade Central's clerks' words (trade_central.c), as the keeper's. */
bool online_square_trade_say(const uint8_t *archive, int n);
/* Room for the Square's chats: a text archive of TALK_ROOM bytes. */
uint32_t online_square_talk_room(void);
#define SQUARE_TALK_ROOM 2048
/* Whether world (x, y) stands on the pad home. */
bool online_square_on_exit(int x, int y);
/* The peers to show, slot by slot, moved by BN6 between frames. */
void online_square_peers(const OnlineSnapshot *shown);
/* The slot of the peer MegaMan at (x, y) facing `face` (0-7) talks to
 * with A; -1 none, -2 one of the Square's own (a Mr.Prog, the desk: BN6
 * talks to them). */
int online_square_peer_ahead(int x, int y, int face);
/* The ambient actors (square_life.c moves them): the two walkers, BN6's
 * log-in beam and the "!" over a head, each shown or hidden in animation
 * `anim` at world (x, y, z) from the next frame; a sound (BN6's id)
 * played from one once. */
enum { SQUARE_ACTOR_WALKER0, SQUARE_ACTOR_WALKER1, SQUARE_ACTOR_BEAM, SQUARE_ACTOR_MARK, SQUARE_ACTORS };
void online_square_actor(int k, bool visible, int anim, int x, int y, int z);
void online_square_actor_sound(int k, int sound);
/* The walker MegaMan at (x, y) facing `face` talks to with A; -1 none. */
int online_square_actor_ahead(int x, int y, int face);
/* Where BN6 actually placed this peer's NPC last frame, for the trace. */
bool online_square_peer_position(unsigned slot, int *x, int *y, int *z);

#endif

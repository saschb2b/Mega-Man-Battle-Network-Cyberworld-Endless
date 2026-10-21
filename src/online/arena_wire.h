/* The Arena's messages on the wire (issue #120, shared/online/README.md,
 * protocol v3): ARENA (a challenge, its answer, a leave, the end), the
 * server's ARENA_STATE, and the three it relays between the two players:
 * ARENA_LOADOUT (a carried MegaMan's bytes), ARENA_INPUT (keys by frame)
 * and ARENA_HASH (the pair's state now and then). Big-endian, each shape
 * checked as the server checks it. A carried MegaMan's bytes are this
 * file's too: what arena_loadout_install writes, and nothing more.
 * ROM-free: tests/test_arena.c. */
#ifndef CW_ARENA_WIRE_H
#define CW_ARENA_WIRE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"
#include "protocol.h"

/* ARENA's actions */
enum { ARENA_ACT_CHALLENGE = 1, ARENA_ACT_ACCEPT, ARENA_ACT_LEAVE, ARENA_ACT_OVER, ARENA_ACT_SEEN };   /* (SEEN, v13: the question on the challenged's screen) */
/* ARENA_STATE's stages */
enum { ARENA_WIRE_CHALLENGED = 1, ARENA_WIRE_WAITING, ARENA_WIRE_START, ARENA_WIRE_ENDED, ARENA_WIRE_SEEN };   /* (SEEN, v13: the challenger told his question is up) */

#define ARENA_LATENCY_MOST 5000u   /* ms: the server refuses past it */
#define ARENA_INPUT_HEAD 10        /* ARENA_INPUT before its keys */
#define ARENA_HASH_BYTES 17
#define ARENA_LOADOUT_HEAD 7
#define ARENA_LOADOUT_VERSION 2     /* (v12: the HP he goes in with, a Team NetBattle's winner's left from his last bout) */

typedef struct {
	uint8_t stage;
	uint32_t match, other;   /* the battle's id; the other player's session */
	uint8_t identity[ONLINE_IDENTITY_BYTES];
	uint8_t side, delay;     /* START's: my side of the cable, the frames of input delay */
} ArenaWireState;

/* ARENA: `action` on `ref` (a challenge's target session, else the
 * battle's id), my latency in ms (clamped); its size, 0 where refused.
 * Every battle is by full rules (protocol v9: no rules byte); the server
 * refuses a game without the NetPass's Arena stamp, NOTICE 28 */
size_t arena_wire_action(uint8_t out[ONLINE_ARENA_BYTES], unsigned action, uint32_t ref, uint32_t latency_ms);
bool arena_wire_state(const uint8_t *data, size_t size, ArenaWireState *out);

/* ARENA_INPUT: `count` (1..ONLINE_ARENA_INPUT_MAX) frames' keys from `frame` */
size_t arena_wire_input(uint8_t *out, size_t cap, uint32_t match, uint32_t frame, const uint16_t *keys, unsigned count);
bool arena_wire_read_input(const uint8_t *data, size_t size, uint32_t *match, uint32_t *frame,
	uint16_t keys[ONLINE_ARENA_INPUT_MAX], unsigned *count);
size_t arena_wire_hash(uint8_t out[ARENA_HASH_BYTES], uint32_t match, uint32_t frame, uint64_t hash);
bool arena_wire_read_hash(const uint8_t *data, size_t size, uint32_t *match, uint32_t *frame, uint64_t *hash);
/* ARENA_LOADOUT around `n` bytes (1..ONLINE_ARENA_LOADOUT_MAX) */
size_t arena_wire_loadout(uint8_t *out, size_t cap, uint32_t match, const uint8_t *bytes, size_t n);
bool arena_wire_read_loadout(const uint8_t *data, size_t size, uint32_t *match, const uint8_t **bytes, size_t *n);

/* TEAM (protocol v12, epic #167): its actions; TEAM_STATE's stages and
 * its result (the recipient's team's) */
enum { TEAM_ACT_PROPOSE = 1, TEAM_ACT_ANSWER, TEAM_ACT_SPLIT, TEAM_ACT_ORDER };
enum { TEAM_ASKED = 1, TEAM_WAITING, TEAM_FORMED, TEAM_SPLIT_UP, TEAM_MATCH, TEAM_ORDERED, TEAM_BOUT, TEAM_FIGHT, TEAM_OVER };
enum { TEAM_WON = 1, TEAM_LOST, TEAM_BROKE_OFF, TEAM_DRAWN };

typedef struct {
	uint8_t stage;
	uint32_t partner;        /* my partner (asked: the one who asks; waiting: the one asked) */
	uint32_t match, bout;    /* the team match, its bout under way (an ARENA battle's id) */
	uint32_t players[4];     /* my team's two, then theirs, each MegaMan up now first (before the order: captain first) */
	uint8_t left[2];         /* MegaMen standing: ours, theirs */
	uint8_t number, result;  /* the bout called; OVER's result */
	uint16_t hp;             /* the staying MegaMan's HP, as the last bout's report left it (0 whole) */
} TeamWireState;

/* TEAM: `action` on `ref` (a player's session; the lead's for the order;
 * 0 for a split), `value` an answer's yes; its size, 0 where refused */
size_t arena_wire_team(uint8_t out[ONLINE_TEAM_BYTES], unsigned action, uint32_t ref, bool value);
bool arena_wire_team_state(const uint8_t *data, size_t size, TeamWireState *out);

/* A carried MegaMan as bytes (NULL: the game's start's MegaMan): the
 * first folder, the NaviCust's placed programs and grid, HP, the buster,
 * Regular and TagChips, the Crosses, Souls and Beast Out; the pack stays
 * home (a NetBattle fights the folder). 0 where it does not fit. */
size_t arena_loadout_pack(const OnlineLoadout *l, uint8_t *out, size_t cap);
/* ... and back: false, the reason in `why`, where the bytes are not one;
 * *carried false for the game's start's MegaMan */
bool arena_loadout_unpack(const uint8_t *in, size_t n, OnlineLoadout *out, bool *carried, char *why, size_t whyn);

#endif

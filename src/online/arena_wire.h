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
enum { ARENA_ACT_CHALLENGE = 1, ARENA_ACT_ACCEPT, ARENA_ACT_LEAVE, ARENA_ACT_OVER };
/* ARENA_STATE's stages */
enum { ARENA_WIRE_CHALLENGED = 1, ARENA_WIRE_WAITING, ARENA_WIRE_START, ARENA_WIRE_ENDED };

#define ARENA_LATENCY_MOST 5000u   /* ms: the server refuses past it */
#define ARENA_INPUT_HEAD 10        /* ARENA_INPUT before its keys */
#define ARENA_HASH_BYTES 17
#define ARENA_LOADOUT_HEAD 7
#define ARENA_LOADOUT_VERSION 1

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

/* A carried MegaMan as bytes (NULL: the game's start's MegaMan): the
 * first folder, the NaviCust's placed programs and grid, HP, the buster,
 * Regular and TagChips, the Crosses, Souls and Beast Out; the pack stays
 * home (a NetBattle fights the folder). 0 where it does not fit. */
size_t arena_loadout_pack(const OnlineLoadout *l, uint8_t *out, size_t cap);
/* ... and back: false, the reason in `why`, where the bytes are not one;
 * *carried false for the game's start's MegaMan */
bool arena_loadout_unpack(const uint8_t *in, size_t n, OnlineLoadout *out, bool *carried, char *why, size_t whyn);

#endif

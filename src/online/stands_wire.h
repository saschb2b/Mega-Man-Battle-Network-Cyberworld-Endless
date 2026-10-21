/* The stands' messages on the wire (issue #128, shared/online/README.md,
 * protocol v7): the channel's NetBattles as the server tells every game
 * (STANDS), the report the cable's first game sends of its battle
 * (ARENA_STATUS), and a battle watched: the ask (WATCH), its setup and
 * end (WATCH_STATE), the players' messages handed on (WATCH_RELAY: their
 * MegaMen and hashes) and both players' keys from the first frame
 * (WATCH_KEYS). Big-endian, each shape checked as the server checks it.
 * ROM-free: tests/test_arena.c. */
#ifndef CW_STANDS_WIRE_H
#define CW_STANDS_WIRE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "protocol.h"

/* A battle's report: each side's HP, the last chip it used (0 none) and
 * how many, the winner (0 none yet, 1 the cable's first, 2 its second) */
typedef struct {
	uint16_t hp[2], max_hp[2], chip[2];
	uint8_t used[2], winner;
} ArenaReport;

/* A STANDS entry's stage */
enum { STANDS_WAY_IN = 1, STANDS_FIGHTING, STANDS_OVER, STANDS_STOPPED };

typedef struct {
	uint32_t match, side[2];                  /* the battle; the cable's first and second players' sessions */
	char name[2][ONLINE_NAME_BYTES + 1];
	uint8_t stage, watchers, cheers;
	ArenaReport report;
	uint8_t bout;                             /* a Team NetBattle's bout (protocol v12), 0 a lone battle */
} StandsEntry;

/* WATCH's actions, WATCH_STATE's stages */
enum { WATCH_ACT_WATCH = 1, WATCH_ACT_STOP };
enum { WATCH_ON = 1, WATCH_OVER };

typedef struct {
	uint8_t stage;
	uint32_t match, side[2];
	uint8_t delay;
} WatchState;

/* STANDS: its entries into out (ONLINE_STANDS_MAX at most), how many; -1 malformed */
int stands_wire_read(const uint8_t *data, size_t size, StandsEntry out[ONLINE_STANDS_MAX]);
/* ARENA_STATUS of `match`; 0 where the report is out of bounds */
size_t stands_wire_status(uint8_t out[ONLINE_ARENA_STATUS_BYTES], uint32_t match, const ArenaReport *r);
size_t stands_wire_watch(uint8_t out[ONLINE_WATCH_BYTES], unsigned action, uint32_t match);
bool stands_wire_state(const uint8_t *data, size_t size, WatchState *out);
/* WATCH_KEYS: side's keys from frame *first, *count of them, big-endian at *keys */
bool stands_wire_keys(const uint8_t *data, size_t size, uint32_t *match, unsigned *side, uint32_t *first, const uint8_t **keys,
	unsigned *count);
/* WATCH_RELAY: the side whose message it is, and the message */
bool stands_wire_relay(const uint8_t *data, size_t size, unsigned *side, const uint8_t **inner, size_t *n);

#endif

/* A watched NetBattle's keys (issue #128, arena_watch.h): both players'
 * keys by frame from the battle's first, as long as it runs, as the
 * server hands them on (WATCH_KEYS, big-endian), and the frames whose keys
 * and the three after them are known for both. ROM-free:
 * tests/test_arena.c. */
#ifndef CW_WATCH_KEYS_H
#define CW_WATCH_KEYS_H

#include <stdbool.h>
#include <stdint.h>

#include "arena_input.h"

/* Both players' keys from the battle's first frame, as long as it runs */
typedef struct {
	uint16_t *keys[2];
	uint32_t known[2], cap[2];   /* side's keys known below known[side] */
} WatchKeys;

/* side's keys from `first` (big-endian u16, `count` of them): false where
 * one went missing between (first past the known) or memory ran out; a
 * stretch already known is left as it was */
bool watch_keys_put(WatchKeys *k, unsigned side, uint32_t first, const uint8_t *be, unsigned count);
/* the frames whose keys and the three after them are known for both sides */
uint32_t watch_keys_runnable(const WatchKeys *k);
/* frame's keys and the three after it: out[k][side] */
void watch_keys_take(const WatchKeys *k, uint32_t frame, uint16_t out[ARENA_LOOKAHEAD][2]);
void watch_keys_free(WatchKeys *k);

#endif

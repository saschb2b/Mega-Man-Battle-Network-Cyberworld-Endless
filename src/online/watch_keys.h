/* A watched NetBattle's keys (issue #128, arena_watch.h): both players'
 * keys by frame from the battle's first, as long as it runs, as the
 * server hands them on (WATCH_KEYS, big-endian), and the frames whose keys
 * and the one after them are known for both. ROM-free:
 * tests/test_arena.c. */
#ifndef CW_WATCH_KEYS_H
#define CW_WATCH_KEYS_H

#include <stdbool.h>
#include <stdint.h>

#include "arena_input.h"

/* Both players' keys from the battle's first frame, as long as it runs
 * (a 2v2's four seats', protocol v15: watch_keys_seats) */
typedef struct {
	uint16_t *keys[ARENA_PLAYERS];
	uint32_t known[ARENA_PLAYERS], cap[ARENA_PLAYERS];   /* side's keys known below known[side] */
	unsigned seats;              /* the streams: 2 (0 as made), or ARENA_PLAYERS */
} WatchKeys;

/* a 2v2's four streams, before any is put: a partner's keeps its sixteen bits */
void watch_keys_seats(WatchKeys *k, unsigned seats);
/* frame's keys and the one after it, every stream's: out[k][seat] (a
 * battle of two's partners' columns released) */
void watch_keys_take_all(const WatchKeys *k, uint32_t frame, uint16_t out[ARENA_LOOKAHEAD][ARENA_PLAYERS]);

/* side's keys from `first` (big-endian u16, `count` of them): false where
 * one went missing between (first past the known) or memory ran out; a
 * stretch already known is left as it was */
bool watch_keys_put(WatchKeys *k, unsigned side, uint32_t first, const uint8_t *be, unsigned count);
/* the frames whose keys and the one after them are known for every stream */
uint32_t watch_keys_runnable(const WatchKeys *k);
void watch_keys_free(WatchKeys *k);

#endif

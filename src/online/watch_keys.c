/* A watched NetBattle's keys (watch_keys.h). */
#include "watch_keys.h"

#include <stdlib.h>
#include <string.h>

bool watch_keys_put(WatchKeys *k, unsigned side, uint32_t first, const uint8_t *be, unsigned count) {
	if (side > 1 || first > k->known[side]) return false;
	uint32_t end = first + count;
	if (end <= k->known[side]) return true;
	if (end > k->cap[side]) {
		uint32_t cap = k->cap[side] ? k->cap[side] : 4096;
		while (cap < end) cap *= 2;
		uint16_t *grown = realloc(k->keys[side], cap * sizeof *grown);
		if (!grown) return false;
		k->keys[side] = grown;
		k->cap[side] = cap;
	}
	for (uint32_t f = k->known[side]; f < end; ++f) {
		const uint8_t *b = be + 2 * (f - first);
		k->keys[side][f] = (uint16_t)(((unsigned)b[0] << 8 | b[1]) & ARENA_KEYS_MASK);
	}
	k->known[side] = end;
	return true;
}

uint32_t watch_keys_runnable(const WatchKeys *k) {
	uint32_t known = k->known[0] < k->known[1] ? k->known[0] : k->known[1];
	return known >= ARENA_LOOKAHEAD - 1 ? known - (ARENA_LOOKAHEAD - 1) : 0;
}

void watch_keys_take(const WatchKeys *k, uint32_t frame, uint16_t out[ARENA_LOOKAHEAD][2]) {
	for (unsigned i = 0; i < ARENA_LOOKAHEAD; ++i)
		for (unsigned side = 0; side < 2; ++side) out[i][side] = k->keys[side][frame + i];
}

void watch_keys_free(WatchKeys *k) {
	for (unsigned side = 0; side < 2; ++side) free(k->keys[side]);
	memset(k, 0, sizeof *k);
}

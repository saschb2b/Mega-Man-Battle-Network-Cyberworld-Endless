#include "arena_input.h"

#include <string.h>

void arena_input_init(ArenaInput *in, unsigned delay) {
	memset(in, 0, sizeof *in);
	if (delay < ARENA_DELAY_MIN) delay = ARENA_DELAY_MIN;
	if (delay > ARENA_DELAY_MAX) delay = ARENA_DELAY_MAX;
	in->delay = delay;
	/* (frames before the first key sampled: released, on every client) */
	in->known[0] = in->known[1] = delay;
}

ArenaPut arena_input_put(ArenaInput *in, unsigned player, uint32_t frame, uint16_t keys) {
	if (player > 1) return ARENA_PUT_GAP;
	if (frame < in->known[player]) return ARENA_PUT_OLD;
	if (frame > in->known[player]) return ARENA_PUT_GAP;
	/* (a slot is free once its frame and the three before the next are run) */
	if (frame >= in->next + ARENA_RING) return ARENA_PUT_FULL;
	in->keys[frame & (ARENA_RING - 1)][player] = keys & ARENA_KEYS_MASK;
	++in->known[player];
	return ARENA_PUT_OK;
}

unsigned arena_input_runnable(const ArenaInput *in) {
	uint32_t known = in->known[0] < in->known[1] ? in->known[0] : in->known[1];
	return known >= in->next + ARENA_LOOKAHEAD ? known - (ARENA_LOOKAHEAD - 1) - in->next : 0;
}

bool arena_input_ready(const ArenaInput *in) { return arena_input_runnable(in) > 0; }

bool arena_input_take(ArenaInput *in, uint16_t out[ARENA_LOOKAHEAD][2]) {
	if (!arena_input_ready(in)) return false;
	for (unsigned k = 0; k < ARENA_LOOKAHEAD; ++k)
		for (unsigned p = 0; p < 2; ++p) out[k][p] = in->keys[(in->next + k) & (ARENA_RING - 1)][p];
	++in->next;
	return true;
}

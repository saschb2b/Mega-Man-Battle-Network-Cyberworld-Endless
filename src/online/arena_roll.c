/* The Arena's rollback tried against the local stand-in (arena_roll.h). */
#include "arena_roll.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ArenaRoll {
	ArenaPair *pair;
	unsigned window;
	ArenaPairSnap *ring[ARENA_ROLL_MOST];       /* the snapshot before each of the last `window` frames */
	uint16_t keys[ARENA_ROLL_MOST][ARENA_LOOKAHEAD][ARENA_PLAYERS];
	ArenaPairSnap *straight, *again;
	uint32_t rolls, same;
	bool said;
};

ArenaRoll *arena_roll_new(ArenaPair *pair, unsigned window) {
	if (!pair || window < 1 || window > ARENA_ROLL_MOST) return NULL;
	ArenaRoll *r = calloc(1, sizeof *r);
	if (!r) return NULL;
	r->pair = pair;
	r->window = window;
	bool ok = (r->straight = arena_pair_snap_new(pair)) && (r->again = arena_pair_snap_new(pair));
	for (unsigned i = 0; ok && i < window; ++i) ok = (r->ring[i] = arena_pair_snap_new(pair)) != NULL;
	if (!ok) { arena_roll_free(r); return NULL; }
	printf("arena: rolling back %u frames every %u, the game's state checked each time\n", window, window);
	return r;
}

void arena_roll_say(ArenaRoll *r) {
	if (!r || r->said) return;
	r->said = true;
	if (r->rolls) printf("arena: %u rollbacks of %u frames, the game's state the same after %u\n", r->rolls, r->window, r->same);
}

void arena_roll_free(ArenaRoll *r) {
	if (!r) return;
	arena_roll_say(r);
	for (unsigned i = 0; i < ARENA_ROLL_MOST; ++i) arena_pair_snap_free(r->ring[i]);
	arena_pair_snap_free(r->straight);
	arena_pair_snap_free(r->again);
	free(r);
}

bool arena_roll_before(ArenaRoll *r, uint32_t frame, const uint16_t keys[ARENA_LOOKAHEAD][ARENA_PLAYERS]) {
	memcpy(r->keys[frame % r->window], keys, sizeof r->keys[0]);
	return arena_pair_save(r->pair, r->ring[frame % r->window]);
}

bool arena_roll_after(ArenaRoll *r, uint32_t frame) {
	if ((frame + 1) % r->window) return true;
	uint32_t from = frame + 1 - r->window;
	if (!arena_pair_save(r->pair, r->straight) || !arena_pair_load(r->pair, r->ring[from % r->window])) return true;
	for (uint32_t f = from; f <= frame; ++f) {
		arena_pair_save(r->pair, r->ring[f % r->window]);
		if (!arena_pair_step_all(r->pair, (const uint16_t (*)[ARENA_PLAYERS])r->keys[f % r->window])) return false;
	}
	++r->rolls;
	/* (the first few apart said in full) */
	if (arena_pair_save(r->pair, r->again) && !arena_pair_snap_diff(r->straight, r->again, frame, r->rolls - r->same <= 3)) ++r->same;
	return true;
}

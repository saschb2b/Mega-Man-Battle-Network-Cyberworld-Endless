#include "arena_input.h"

#include <string.h>

#define SLOT(frame) ((frame) & (ARENA_RING - 1))

void arena_input_init(ArenaInput *in, unsigned delay) {
	memset(in, 0, sizeof *in);
	if (delay < ARENA_DELAY_MIN) delay = ARENA_DELAY_MIN;
	if (delay > ARENA_DELAY_MAX) delay = ARENA_DELAY_MAX;
	in->delay = delay;
	in->players = 2;
	/* (frames before the first key sampled: released, on every client) */
	for (unsigned p = 0; p < ARENA_PLAYERS; ++p) in->known[p] = in->guessed[p] = delay;
	in->wrong = ARENA_NO_FRAME;
}

void arena_input_players(ArenaInput *in, unsigned players) {
	in->players = players < 2 ? 2 : players > ARENA_PLAYERS ? ARENA_PLAYERS : players;
}

uint32_t arena_input_settled(const ArenaInput *in) {
	uint32_t known = in->known[0];
	for (unsigned p = 1; p < in->players; ++p)
		if (in->known[p] < known) known = in->known[p];
	return known >= ARENA_LOOKAHEAD - 1 ? known - (ARENA_LOOKAHEAD - 1) : 0;
}

ArenaPut arena_input_put(ArenaInput *in, unsigned player, uint32_t frame, uint16_t keys) {
	if (player >= in->players) return ARENA_PUT_GAP;
	if (frame < in->known[player]) return ARENA_PUT_OLD;
	if (frame > in->known[player]) return ARENA_PUT_GAP;
	/* (a slot is free once no frame that reads it can run again) */
	uint32_t settled = arena_input_settled(in), base = settled < in->next ? settled : in->next;
	if (frame >= base + ARENA_RING) return ARENA_PUT_FULL;
	/* (a partner's stream carries his Custom screen's picks past the GBA's ten buttons, arena_team_custom.h) */
	keys &= player < 2 ? ARENA_KEYS_MASK : ARENA_PARTNER_MASK;
	if (frame < in->guessed[player]) {
		if (in->keys[SLOT(frame)][player] != keys && frame < in->wrong) in->wrong = frame;
	} else {
		in->guessed[player] = frame + 1;
	}
	in->keys[SLOT(frame)][player] = keys;
	++in->known[player];
	return ARENA_PUT_OK;
}

bool arena_input_ready(const ArenaInput *in, unsigned window) {
	return in->next < arena_input_settled(in) + window;
}

bool arena_input_take(ArenaInput *in, unsigned window, uint16_t out[ARENA_LOOKAHEAD][ARENA_PLAYERS]) {
	if (!arena_input_ready(in, window)) return false;
	for (unsigned p = 0; p < in->players; ++p) {
		/* (each key not yet come guessed as the player's last known, held) */
		uint16_t last = in->keys[SLOT(in->known[p] - 1)][p];
		for (; in->guessed[p] < in->next + ARENA_LOOKAHEAD; ++in->guessed[p]) in->keys[SLOT(in->guessed[p])][p] = last;
	}
	for (unsigned k = 0; k < ARENA_LOOKAHEAD; ++k)
		for (unsigned p = 0; p < ARENA_PLAYERS; ++p) out[k][p] = p < in->players ? in->keys[SLOT(in->next + k)][p] : 0;
	++in->next;
	return true;
}

uint32_t arena_input_wrong(const ArenaInput *in) {
	return in->wrong == ARENA_NO_FRAME ? ARENA_NO_FRAME : in->wrong >= ARENA_LOOKAHEAD - 1 ? in->wrong - (ARENA_LOOKAHEAD - 1) : 0;
}

void arena_input_rewind(ArenaInput *in, uint32_t frame) {
	if (frame < in->next) in->next = frame;
	for (unsigned p = 0; p < ARENA_PLAYERS; ++p) in->guessed[p] = in->known[p];
	in->wrong = ARENA_NO_FRAME;
}

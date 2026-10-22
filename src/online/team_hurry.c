/* Two MegaMen a side: the turn's one limit (team_hurry.h). */
#include "team_hurry.h"

#include <stdio.h>

#include <mgba/core/core.h>

#include "arena_input.h"
#include "arena_team_custom.h"
#include "bn6.h"
#include "emu.h"

#define PRESS 4   /* frames a key is held, then as many let go: BN6's screen takes a press on its edge */

unsigned team_hurry_clock(Arena *a) {
	unsigned most = 0;
	for (unsigned side = 0; side < 2; ++side) {
		unsigned t = arena_team_custom_clock_of(arena_core(a, side));
		if (t > most) most = t;
	}
	return most;
}

/* side's lead past OK on his GBA (OK pressed: its show, a Program Advance's, the wait for the cable), or no turn */
static bool lead_done(struct mCore *c) {
	if (!c || c->rawRead8(c, BN6_BATTLE_PHASE, -1) != BN6_PHASE_CUSTOM) return true;
	unsigned mode = (unsigned)c->rawRead8(c, BN6_CUSTOM_MODE, -1) & 0xFFu;
	return mode == BN6_MODE_OK || mode == BN6_MODE_PA || mode == BN6_MODE_WAIT;
}

/* whether `seat`'s player is still picking this turn (a lead's Custom screen not past OK; a partner's turn open) */
static bool late(Arena *a, unsigned seat) {
	unsigned side = seat < 2 ? seat & 1u : (seat - 2) & 1u;
	struct mCore *c = arena_core(a, side);
	return seat < 2 ? !lead_done(c) : arena_team_custom_picking(c, side);
}

bool team_hurry_shown(Arena *a, unsigned seat) {
	unsigned t = team_hurry_clock(a);
	return t + TEAM_TURN_HURRY >= TEAM_TURN_LIMIT && late(a, seat);
}

uint16_t team_hurry_lead(Arena *a, unsigned side, uint16_t keys, uint32_t tick) {
	static unsigned said[2];   /* (the turn's clock as the output last said it, once a turn) */
	struct mCore *c = arena_core(a, side & 1u);
	unsigned t = team_hurry_clock(a);
	if (t < TEAM_TURN_LIMIT || lead_done(c)) {
		if (!t) said[side & 1u] = 0;
		return keys;
	}
	if (!said[side & 1u]) {
		said[side & 1u] = t;
		printf("arena2v2: the turn's limit, %u frames after its first OK: side %u's lead's own game presses OK for him, his picks "
			"so far\n", t, side & 1u);
	}
	if (tick % (2 * PRESS) >= PRESS) return 0;
	unsigned mode = (unsigned)c->rawRead8(c, BN6_CUSTOM_MODE, -1) & 0xFFu, cursor = (unsigned)c->rawRead8(c, BN6_CUSTOM_CURSOR, -1) & 0xFFu;
	/* (CROSSSELECT open: B shuts it, his Cross not taken; on the chips: right to OK, up from the emblem under it, then A) */
	if (mode != BN6_MODE_CHIPS) return mode >= BN6_MODE_CROSS_OPEN && mode <= BN6_MODE_CROSS_TAKEN ? KEY_B : 0;
	return cursor == BN6_CUSTOM_OK ? KEY_A : cursor == BN6_CUSTOM_EMBLEM ? KEY_UP : KEY_RIGHT;
}

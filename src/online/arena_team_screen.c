/* --dev arena2v2: a partner's own Custom screen, BN6's own (arena_team_screen.h). */
#include "arena_team_screen.h"

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include <mgba/core/core.h>

#include "arena_pair.h"
#include "arena_team_custom.h"
#include "arena_team_navi.h"
#include "bn6.h"
#include "emu.h"

#define NONE16  0xFFFFu
#define CROSSES 5   /* Gregar's, their flags from BN6_FLAG_HEAT_CROSS on */

struct TeamScreen {
	ArenaPair *pair;         /* the plain copy, both GBAs on their own cable */
	ArenaPairSnap *snap;     /* the pair copied each turn */
	unsigned side;           /* his side's GBA there */
	bool open;               /* a turn under way, his OK not yet pressed */
	uint8_t mode;            /* BN6_CUSTOM_MODE the frame before */
	bool pa;                 /* his OK made a Program Advance: his hand read once BN6 has shown it */
	uint16_t built[BN6_HAND_MAX];   /* the hand BN6 built at his last OK (its chips), */
	uint16_t pa_id;          /* ... the Program Advance in it (0 none) */
	uint8_t crosses;         /* his own Crosses (bit i Gregar's Cross i, BN6_FLAG_HEAT_CROSS's order) */
	uint32_t used;           /* ... those he has chosen this battle (BN6_CROSSES_USED's), kept from his last OK */
	double ms, worst, copy, copy_worst;
	unsigned frames, copies;
};

static double since(uint64_t t) {
	return (double)(SDL_GetPerformanceCounter() - t) * 1000.0 / (double)SDL_GetPerformanceFrequency();
}

TeamScreen *team_screen_new(const uint8_t *rom, size_t size, ArenaPair *pair, unsigned side) {
	TeamScreen *s = calloc(1, sizeof *s);
	if (!s || !pair) { free(s); return NULL; }
	s->side = side & 1u;
	uint8_t *state[2] = { NULL, NULL };
	size_t n[2] = { 0, 0 };
	for (unsigned g = 0; g < 2; ++g) state[g] = arena_core_save(arena_pair_core(pair, g), &n[g]);
	const uint8_t *from[2] = { state[0], state[1] };
	if (state[0] && state[1]) s->pair = arena_pair_open_plain(rom, size, from, n);
	free(state[0]);
	free(state[1]);
	if (s->pair) s->snap = arena_pair_snap_new(pair);
	if (!s->pair || !s->snap) { team_screen_free(s); return NULL; }
	/* (his side's picture alone: the other GBA's is never shown) */
	arena_pair_draw(s->pair, 1u << s->side);
	return s;
}

void team_screen_free(TeamScreen *s) {
	if (!s) return;
	arena_pair_snap_free(s->snap);
	arena_pair_close(s->pair);
	free(s);
}

bool team_screen_open(TeamScreen *s, ArenaPair *pair, const uint16_t *deck, unsigned n) {
	uint64_t t = SDL_GetPerformanceCounter();
	if (!arena_pair_save(pair, s->snap) || !arena_pair_load(s->pair, s->snap)) return false;
	struct mCore *c = arena_pair_core(s->pair, s->side);
	for (unsigned i = 0; i < BN6_DECK_SIZE; ++i) c->rawWrite16(c, BN6_BATTLE_DECK + 2 * i, -1, (uint16_t)(i < n ? deck[i] : NONE16));
	/* (his own record as his side's battle NaviStats there: BN6's screen deals his hand by his own Custom level, his NaviCust's) */
	uint8_t rec[BN6_BATTLE_NAVI_SIZE];
	if (arena_team_navi_record(c, s->side, rec))
		for (unsigned i = 0; i < BN6_BATTLE_NAVI_SIZE; ++i) c->rawWrite8(c, BN6_BATTLE_NAVI + s->side * BN6_BATTLE_NAVI_SIZE + i, -1, rec[i]);
	/* (his own Crosses there, his lead's off, and those he has used this battle; Beast Out off: not his yet) */
	for (unsigned f = BN6_FLAG_BEAST_OUT; f < BN6_FLAG_HEAT_CROSS + CROSSES; ++f) {
		uint32_t at = BN6_EVENT_FLAGS + f / 8;
		uint8_t bit = (uint8_t)(0x80u >> (f & 7)), was = (uint8_t)c->rawRead8(c, at, -1);
		bool his = f >= BN6_FLAG_HEAT_CROSS && (s->crosses >> (f - BN6_FLAG_HEAT_CROSS) & 1u);
		if (f == BN6_FLAG_BEAST_OUT || f >= BN6_FLAG_HEAT_CROSS) c->rawWrite8(c, at, -1, (uint8_t)(his ? was | bit : was & ~bit));
	}
	c->rawWrite32(c, BN6_CROSSES_USED, -1, s->used);
	/* (the Custom screen's phase at its first step: BN6 sets the screen up anew the next frame, its hand dealt from the deck) */
	c->rawWrite8(c, BN6_BATTLE_STEP, -1, 0);
	arena_pair_anchor(s->pair);
	double took = since(t);
	s->copy += took;
	if (took > s->copy_worst) s->copy_worst = took;
	++s->copies;
	s->open = true;
	s->mode = (uint8_t)c->rawRead8(c, BN6_CUSTOM_MODE, -1);
	return true;
}

bool team_screen_frame(TeamScreen *s, uint16_t keys, uint8_t *picks, unsigned *n) {
	*n = 0;
	if (!s->open) return false;
	uint64_t t = SDL_GetPerformanceCounter();
	uint16_t k[ARENA_LOOKAHEAD][2] = { { 0, 0 } };
	k[0][s->side] = keys;
	bool ran = arena_pair_step(s->pair, (const uint16_t (*)[2])k, false);
	arena_pair_mute(s->pair);
	double ms = since(t);
	s->ms += ms;
	if (ms > s->worst) s->worst = ms;
	++s->frames;
	if (!ran) { s->open = false; return false; }
	struct mCore *c = arena_pair_core(s->pair, s->side);
	uint8_t mode = (uint8_t)c->rawRead8(c, BN6_CUSTOM_MODE, -1), was = s->mode;
	s->mode = mode;
	/* (his OK; where it made a Program Advance, BN6 shows it first: his hand is whole once the screen waits for the cable) */
	bool ok = mode != was && mode == BN6_MODE_OK, pa = c->rawRead8(c, BN6_CUSTOM_NEXT, -1) == BN6_MODE_PA;
	if (ok && pa) s->pa = true;
	if (!(ok && !pa) && !(s->pa && mode != was && mode == BN6_MODE_WAIT)) return false;
	*n = team_screen_picks(s, picks);
	/* (the hand BN6 built, BN6_CUSTOM_BUILT, and the Program Advance in it) */
	s->pa_id = 0;
	for (unsigned i = 0; i < BN6_HAND_MAX; ++i) {
		s->built[i] = (uint16_t)c->rawRead16(c, BN6_CUSTOM_BUILT + BN6_HAND_CHIPS + 2 * i, -1);
		if (s->pa && !s->pa_id && s->built[i] >= BN6_PA_FIRST && s->built[i] <= BN6_PA_LAST) s->pa_id = s->built[i];
	}
	s->pa = false;
	s->open = false;
	return true;
}

unsigned team_screen_picks(const TeamScreen *s, uint8_t *picks) {
	struct mCore *c = arena_pair_core(s->pair, s->side);
	unsigned picked = (unsigned)c->rawRead8(c, BN6_CUSTOM_PICKED, -1);
	if (picked > TEAM_CUSTOM_PICKS) picked = TEAM_CUSTOM_PICKS;
	for (unsigned i = 0; i < picked; ++i) picks[i] = (uint8_t)c->rawRead8(c, BN6_CUSTOM_PICKS + i, -1);
	return picked;
}

void team_screen_crosses(TeamScreen *s, uint8_t crosses) {
	if (s) s->crosses = crosses;
}

uint8_t team_screen_cross(TeamScreen *s) {
	struct mCore *c = arena_pair_core(s->pair, s->side);
	uint8_t form = (uint8_t)c->rawRead8(c, BN6_CROSS_RECORD, -1);
	/* (the Crosses he has chosen, kept for his next turn's CROSSSELECT) */
	s->used = (uint32_t)c->rawRead32(c, BN6_CROSSES_USED, -1);
	return form && form < BN6_FORMS ? form : 0;
}

uint16_t team_screen_pa(const TeamScreen *s, uint16_t *built) {
	for (unsigned i = 0; i < BN6_HAND_MAX; ++i) built[i] = s->built[i];
	return s->pa_id;
}

void team_screen_close(TeamScreen *s) { s->open = false; }
const uint32_t *team_screen_video(const TeamScreen *s) { return arena_pair_video(s->pair, s->side); }
bool team_screen_open_now(const TeamScreen *s) { return s && s->open; }
double team_screen_ms(const TeamScreen *s, bool worst) { return worst ? s->worst : s->frames ? s->ms / s->frames : 0; }
double team_screen_copy_ms(const TeamScreen *s, bool worst) { return worst ? s->copy_worst : s->copies ? s->copy / s->copies : 0; }

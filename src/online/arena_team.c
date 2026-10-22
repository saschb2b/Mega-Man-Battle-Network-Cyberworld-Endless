/* --dev arena2v2 (arena_team.h). */
#include "arena_team.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_input.h"
#include "arena_team_cross.h"
#include "arena_team_custom.h"
#include "arena_team_link.h"
#include "arena_team_navi.h"
#include "arena_team_rules.h"
#include "arena_team_wait.h"
#include "bn6.h"
#include "darkinvs.h"
#include "devtools.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"

#define ITERATIONS 8192
#define MUTATED    6      /* iterations the mutation flips a key for */
#define SIDES      2
#define MEGAMEN    2      /* a side's: its player, then its partner */

/* (allocated as the hooks go on: what a battle of the flag's is compared by, ~0.5 MB nobody else pays for) */
typedef struct {
	int slot[SIDES];                         /* each GBA's core's hook slot (hook_slot_of); the rest by GBA, 0 or 1 */
	uint32_t digest[SIDES][ITERATIONS];      /* each GBA's, by the loop's iteration */
	uint8_t partner[SIDES][ITERATIONS][SIDES];   /* each side's partner's panel, row << 4 | column */
	uint16_t hp[SIDES][ITERATIONS][SIDES * MEGAMEN];
	uint16_t keys[SIDES][ITERATIONS][SIDES];     /* each partner's keys taken */
	uint8_t at[SIDES][ITERATIONS][SIDES * MEGAMEN];   /* each MegaMan's hand's chip up next (a partner's his own) */
	uint32_t last[SIDES];                    /* the last iteration each met */
	uint32_t parted;                         /* the first iteration they were met apart (0 none yet) */
} TeamLog;
static TeamLog *T;

/* what a NetBattle spawns instead of its own list (one MegaMan a side, and
 * rocks in some): two MegaMen a side, no rocks (the study's list) */
static const uint8_t team_list[] = {
	0x00, 0x22, 0, 0,   /* side 0's player, column 2 row 2 */
	0x00, 0x31, 0, 0,   /* side 0's partner, column 1 row 3 */
	0x01, 0x25, 0, 0,   /* side 1's player, column 5 row 2 */
	0x01, 0x16, 0, 0,   /* side 1's partner, column 6 row 1 */
	BN6_ENTITY_END, 0, 0, 0,
};

static HookAct spawn(HookRegs *r, void *user) {
	(void)user;
	/* (a NetBattle's list, whichever of them: its first two MegaMen, side 0's and side 1's) */
	if (hook_read8(r->r[0]) != BN6_ENTITY_MEGAMAN || hook_read8(r->r[0] + BN6_ENTITY_SIZE) != (BN6_ENTITY_MEGAMAN | 1)) return HOOK_CONTINUE;
	for (unsigned i = 0; i < sizeof team_list; ++i) hook_write8(RAM_TEAM + i, team_list[i]);
	r->r[0] = RAM_TEAM;
	/* (the partners' words emptied with the battle: the count starts again at 0, RAM_TEAM_TICK cleared with the battle's RAM) */
	team_link_begin();
	/* (each partner's own NaviStats and hand, from his side's as the battle set them up; his deck and Custom screen anew) */
	arena_team_navi_begin();
	arena_team_custom_begin();
	/* (teambeast=3: the leads' Beast Out too, its flag on both GBAs as the battle spawns, the partners' control) */
	if (dev.team_beast >= 3) {
		uint32_t at = BN6_EVENT_FLAGS + BN6_FLAG_BEAST_OUT / 8;
		hook_write8(at, (uint8_t)(hook_read8(at) | 0x80u >> (BN6_FLAG_BEAST_OUT & 7)));
	}
	return HOOK_CONTINUE;
}

void arena_team_sample(struct mCore *core[2], unsigned side, uint16_t value) { team_link_put(core[side & 1u], side & 1u, value); }

uint16_t arena_team_script(unsigned player, uint32_t frame) {
	if (player == 2) {
		/* side 0's partner: up to row 1 (column 1), charged shots along it */
		unsigned t = frame % 480;
		if (t < 4 || (t >= 16 && t < 20)) return KEY_UP;
		if (t >= 330 && t < 380) return t % 8 < 2 ? KEY_A : 0;   /* (a chip of his own hand: A tapped till he is free to use it) */
		return (t >= 40 && t < 150) || (t >= 200 && t < 310) ? KEY_B : 0;
	}
	if (player == 3) {
		/* side 1's partner: charged shots along row 1 (column 6), a step down and back between */
		unsigned t = (frame + 100) % 400;
		if (t >= 300 && t < 304) return KEY_DOWN;
		if (t >= 266 && t < 298) return t % 8 < 2 ? KEY_A : 0;
		if (t >= 340 && t < 344) return KEY_UP;
		return t < 110 || (t >= 150 && t < 260) ? KEY_B : 0;
	}
	return 0;
}

static uint32_t fnv(uint32_t h, uint32_t v) {
	for (unsigned i = 0; i < 4; ++i) h = (h ^ ((v >> (8 * i)) & 0xFFu)) * 16777619u;
	return h;
}

/* the battle's state both GBAs must agree on, as BN6's own 1v1 has them
 * agree (the research harness's comparison, .build/research/battles) */
static uint32_t digest(int g, uint32_t tick) {
	uint32_t h = 2166136261u;
	for (unsigned side = 0; side < SIDES; ++side)
		for (unsigned k = 0; k < MEGAMEN; ++k) {
			uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + k * BN6_SIDE_PARTNER);
			if (!obj) { h = fnv(h, 0); continue; }
			uint32_t ai = hook_read32(obj + BN6_T1_AI_DATA);
			uint16_t hp = hook_read16(obj + BN6_T1_HP);
			uint8_t panel = (uint8_t)(hook_read8(obj + BN6_T1_PANEL_Y) << 4 | hook_read8(obj + BN6_T1_PANEL_X));
			h = fnv(h, panel | (uint32_t)hook_read8(obj + BN6_T1_ACTION) << 8 | (uint32_t)hook_read8(obj + BN6_T1_ALLIANCE) << 16);
			h = fnv(h, hp | (uint32_t)hook_read8(obj + BN6_T1_FUTURE_X) << 16 | (uint32_t)hook_read8(obj + BN6_T1_FUTURE_Y) << 24);
			if (ai) h = fnv(h, hook_read16(ai + BN6_AI_HELD) | (uint32_t)hook_read16(ai + BN6_AI_PRESSED) << 16);
			if (tick < ITERATIONS) {
				T->hp[g][tick][side * MEGAMEN + k] = hp;
				if (k) T->partner[g][tick][side] = panel;
			}
		}
	return arena_team_navi_digest(fnv(fnv(h, hook_read32(BN6_RNG_PRIMARY)), hook_read32(BN6_BATTLE_TIMER)));
}

/* the first iteration apart said as it runs, for a battle the two GBAs no
 * longer end together (a mutation's; with a rollback window a guessed
 * iteration may be said and run again, so the report at the end is the
 * one to read) */
static void apart_now(int g, uint32_t tick) {
	int other = g ^ 1;
	if (T->parted || T->last[other] < tick || T->digest[other][tick] == T->digest[g][tick]) return;
	T->parted = tick;
	printf("arena2v2: the two GBAs part at iteration %u, said as it ran (the partners' keys taken there: %04X %04X and %04X %04X)\n",
		(unsigned)tick, T->keys[g][tick][0], T->keys[g][tick][1], T->keys[other][tick][0], T->keys[other][tick][1]);
}

/* side 0's copy (r0 0), the loop's iteration counted: the partners' keys
 * as their sides' packets last brought them (arena_team_link.h) written
 * as BN6_KEY_COPY writes a player's from the packets' pads, then the
 * state recorded */
static HookAct keys(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] != 0) return HOOK_CONTINUE;
	uint32_t tick = hook_read32(RAM_TEAM_TICK) + 1;
	hook_write32(RAM_TEAM_TICK, tick);
	/* (each partner's own Custom screen's picks into his own hand, at the iteration they were set for; teampick=-1 a chip short
	 * on the second GBA alone, the mutation) */
	int slot = hook_slot();
	const uint32_t hands[SIDES] = { arena_team_navi_hand(0), arena_team_navi_hand(1) };
	arena_team_custom_take(tick, hands, T && slot == T->slot[1]);
	int g = T && slot >= 0 ? (slot == T->slot[0] ? 0 : slot == T->slot[1] ? 1 : -1) : -1;
	bool kept = g >= 0 && tick < ITERATIONS;
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER);
		uint32_t ai = obj ? hook_read32(obj + BN6_T1_AI_DATA) : 0;
		if (!ai) continue;
		uint16_t k = team_link_keys(side);
		/* (the mutation: one key of side 0's partner flipped on the second GBA alone) */
		if (dev.arena_2v2 > 1 && T && slot == T->slot[1] && side == 0 && tick >= (uint32_t)dev.arena_2v2 && tick < (uint32_t)dev.arena_2v2 + MUTATED)
			k ^= KEY_RIGHT;
		if (kept) T->keys[g][tick][side] = k;
		k |= BN6_KEYS_VALID;
		uint16_t held = hook_read16(ai + BN6_AI_HELD);
		hook_write16(ai + BN6_AI_LAST, held);
		hook_write16(ai + BN6_AI_HELD, k);
		hook_write16(ai + BN6_AI_PRESSED, (uint16_t)(~held & k));
		hook_write16(ai + BN6_AI_UP, (uint16_t)(~k & held));
	}
	if (kept) {
		for (unsigned m = 0; m < SIDES * MEGAMEN; ++m) T->at[g][tick][m] = arena_team_navi_at(m / MEGAMEN, m % MEGAMEN);
		T->digest[g][tick] = digest(g, tick);
		if (tick > T->last[g]) T->last[g] = tick;
		apart_now(g, tick);
	}
	return HOOK_CONTINUE;
}

bool arena_team_hooks(struct mCore *core, unsigned side) {
	if (side >= SIDES) return false;
	if (!side) {
		free(T);
		if (!(T = calloc(1, sizeof *T))) return false;
	}
	if (!T) return false;
	HookCore *c = hook_core(core);
	bool ok = c && hook_on(c, BN6_SPAWN_ENTITIES, spawn, NULL) && hook_on(c, BN6_KEY_COPY, keys, NULL) &&
		arena_team_navi_hooks(c, side, hook_slot_of(core)) && arena_team_wait_hooks(c) &&
		arena_team_cross_hooks(c, side, hook_slot_of(core)) && team_link_hooks(c, side == 1);
	/* (a partner's own NaviStats told to the Souls' and the mood's hooks: arena_team_rules.h) */
	arena_team_rules_begin();
	/* (under full rules BN5's chip AI gives side 0's player its block at his set-up, the others rooms of their own as it takes
	 * them: darkinvs_room.h) */
	darkinvs_two_a_side(true);
	T->slot[side] = hook_slot_of(core);
	/* (the side's GBA's core, from which his lead's folder is read as his deck is dealt: the first deal comes in the way in,
	 * before the pair hands a GBA its partner's first frame) */
	arena_team_custom_sampled(core, side);
	if (!ok) printf("arena2v2: the hooks (%08X, %08X, the NaviStats') not all taken on side %u's core\n", BN6_SPAWN_ENTITIES, BN6_KEY_COPY, side);
	return ok;
}

/* how the partners' keys went in over the iterations both GBAs ran */
static void report_keys(int a, int b, uint32_t n) {
	uint32_t pressed[SIDES] = { 0 }, keys_apart = 0;
	for (uint32_t t = 1; t <= n; ++t) {
		keys_apart += memcmp(T->keys[a][t], T->keys[b][t], sizeof T->keys[a][t]) != 0;
		for (unsigned s = 0; s < SIDES; ++s) pressed[s] += T->keys[a][t][s] != 0;
	}
	printf("arena2v2: the partners' keys from their sides' packets: %u iterations taken apart; held in %u and %u iterations "
		"(side 0's, side 1's)\n", (unsigned)keys_apart, (unsigned)pressed[0], (unsigned)pressed[1]);
	team_link_report();
}

bool arena_team_report(void) {
	if (!T) return false;
	const int a = 0, b = 1;
	if (T->slot[0] < 0 || T->slot[1] < 0) return false;
	uint32_t n = T->last[a] < T->last[b] ? T->last[a] : T->last[b], differing = 0, first = 0, moved = 0;
	for (uint32_t t = 1; t <= n; ++t) {
		if (T->digest[a][t] != T->digest[b][t]) { if (!differing) first = t; ++differing; }
		if (t > 1 && T->partner[a][t][0] != T->partner[a][t - 1][0]) ++moved;
	}
	printf("arena2v2: iterations %u and %u, compared %u, differing %u (first %u); side 0's partner moved %u times\n",
		(unsigned)T->last[a], (unsigned)T->last[b], (unsigned)n, (unsigned)differing, (unsigned)first, (unsigned)moved);
	report_keys(a, b, n);
	arena_team_navi_report();
	arena_team_custom_report();
	arena_team_wait_report();
	arena_team_cross_report();
	for (unsigned g = 0; g < SIDES && n; ++g) {
		int s = g ? b : a;
		const uint16_t *hp = T->hp[s][n];
		printf("arena2v2: GBA %u at iteration %u: partners at %02X and %02X, HP %u %u | %u %u\n", g, (unsigned)n,
			T->partner[s][n][0], T->partner[s][n][1], hp[0], hp[1], hp[2], hp[3]);
	}
	/* (each hit as the first GBA had it: whose HP fell when, each MegaMan's own; and each chip used, from whose hand) */
	static const char *const who[SIDES * MEGAMEN] = { "side 0's player", "side 0's partner", "side 1's player", "side 1's partner" };
	for (uint32_t t = 2; t <= n; ++t)
		for (unsigned m = 0; m < SIDES * MEGAMEN; ++m) {
			if (T->at[a][t][m] != T->at[a][t - 1][m])
				printf("arena2v2: iteration %u: %s's hand's chip up next %u to %u (the others' %u %u %u %u)\n", (unsigned)t, who[m],
					T->at[a][t - 1][m], T->at[a][t][m], T->at[a][t][0], T->at[a][t][1], T->at[a][t][2], T->at[a][t][3]);
			if (T->hp[a][t][m] != T->hp[a][t - 1][m])
				printf("arena2v2: iteration %u: %s's HP %u to %u (the others' %u %u %u %u)\n", (unsigned)t, who[m], T->hp[a][t - 1][m],
					T->hp[a][t][m], T->hp[a][t][0], T->hp[a][t][1], T->hp[a][t][2], T->hp[a][t][3]);
		}
	fflush(stdout);
	return n && !differing;
}

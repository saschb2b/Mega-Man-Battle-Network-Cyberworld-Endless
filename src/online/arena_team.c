/* --dev arena2v2 (arena_team.h). */
#include "arena_team.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "darkinvs.h"
#include "devtools.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"

#define ITERATIONS 4096
#define MUTATED    6      /* iterations the mutation flips a key for */
#define SIDES      2
#define MEGAMEN    2      /* a side's: its player, then its partner */

static struct {
	int slot[SIDES];                         /* each GBA's core's hook slot (hook_slot_of) */
	uint32_t digest[HOOK_CORES][ITERATIONS]; /* each core's, by the loop's iteration */
	uint8_t partner[HOOK_CORES][ITERATIONS][SIDES];   /* each side's partner's panel, row << 4 | column */
	uint16_t hp[HOOK_CORES][ITERATIONS][SIDES * MEGAMEN];
	uint32_t last[HOOK_CORES];               /* the last iteration each met */
} T;

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
	return HOOK_CONTINUE;
}

/* a partner's keys at an iteration: side 0's steps up twice to row 1 and
 * fires charged shots along it; side 1's steps down once and does the same
 * along row 2 */
static uint16_t partner_keys(uint32_t tick, unsigned side) {
	if (side == 0) {
		if ((tick >= 130 && tick < 136) || (tick >= 150 && tick < 156)) return KEY_UP;
		if (tick >= 180 && (tick - 180) % 160 < 110) return KEY_B;
		return 0;
	}
	if (tick >= 140 && tick < 146) return KEY_DOWN;
	if (tick >= 220 && (tick - 220) % 160 < 110) return KEY_B;
	return 0;
}

static uint32_t fnv(uint32_t h, uint32_t v) {
	for (unsigned i = 0; i < 4; ++i) h = (h ^ ((v >> (8 * i)) & 0xFFu)) * 16777619u;
	return h;
}

/* the battle's state both GBAs must agree on, as BN6's own 1v1 has them
 * agree (the research harness's comparison, .build/research/battles) */
static uint32_t digest(int slot, uint32_t tick) {
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
				T.hp[slot][tick][side * MEGAMEN + k] = hp;
				if (k) T.partner[slot][tick][side] = panel;
			}
		}
	return fnv(fnv(h, hook_read32(BN6_RNG_PRIMARY)), hook_read32(BN6_BATTLE_TIMER));
}

/* side 0's copy (r0 0), the loop's iteration counted: the partners' keys
 * written as BN6_KEY_COPY writes a player's, then the state recorded */
static HookAct keys(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] != 0) return HOOK_CONTINUE;
	uint32_t tick = hook_read32(RAM_TEAM_TICK) + 1;
	hook_write32(RAM_TEAM_TICK, tick);
	int slot = hook_slot();
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER);
		uint32_t ai = obj ? hook_read32(obj + BN6_T1_AI_DATA) : 0;
		if (!ai) continue;
		uint16_t k = partner_keys(tick, side);
		/* (the mutation: one key of side 0's partner flipped on the second GBA alone) */
		if (dev.arena_2v2 > 1 && slot == T.slot[1] && side == 0 && tick >= (uint32_t)dev.arena_2v2 && tick < (uint32_t)dev.arena_2v2 + MUTATED)
			k ^= KEY_RIGHT;
		k |= BN6_KEYS_VALID;
		uint16_t held = hook_read16(ai + BN6_AI_HELD);
		hook_write16(ai + BN6_AI_LAST, held);
		hook_write16(ai + BN6_AI_HELD, k);
		hook_write16(ai + BN6_AI_PRESSED, (uint16_t)(~held & k));
		hook_write16(ai + BN6_AI_UP, (uint16_t)(~k & held));
	}
	if (slot >= 0 && slot < HOOK_CORES && tick < ITERATIONS) {
		T.digest[slot][tick] = digest(slot, tick);
		if (tick > T.last[slot]) T.last[slot] = tick;
	}
	return HOOK_CONTINUE;
}

bool arena_team_hooks(struct mCore *core, unsigned side) {
	if (side >= SIDES) return false;
	if (!side) memset(&T, 0, sizeof T);
	HookCore *c = hook_core(core);
	bool ok = c && hook_on(c, BN6_SPAWN_ENTITIES, spawn, NULL) && hook_on(c, BN6_KEY_COPY, keys, NULL);
	/* (under full rules BN5's chip AI gives side 0's player a block of its own: the first MegaMan's alone) */
	darkinvs_two_a_side(true);
	T.slot[side] = hook_slot_of(core);
	if (!ok) printf("arena2v2: the hooks at %08X and %08X not taken on side %u's core\n", BN6_SPAWN_ENTITIES, BN6_KEY_COPY, side);
	return ok;
}

bool arena_team_report(void) {
	int a = T.slot[0], b = T.slot[1];
	if (a < 0 || b < 0 || a >= HOOK_CORES || b >= HOOK_CORES) return false;
	uint32_t n = T.last[a] < T.last[b] ? T.last[a] : T.last[b], differing = 0, first = 0, moved = 0;
	for (uint32_t t = 1; t <= n; ++t) {
		if (T.digest[a][t] != T.digest[b][t]) { if (!differing) first = t; ++differing; }
		if (t > 1 && T.partner[a][t][0] != T.partner[a][t - 1][0]) ++moved;
	}
	printf("arena2v2: iterations %u and %u, compared %u, differing %u (first %u); side 0's partner moved %u times\n",
		(unsigned)T.last[a], (unsigned)T.last[b], (unsigned)n, (unsigned)differing, (unsigned)first, (unsigned)moved);
	for (unsigned g = 0; g < SIDES && n; ++g) {
		int s = g ? b : a;
		const uint16_t *hp = T.hp[s][n];
		printf("arena2v2: GBA %u at iteration %u: partners at %02X and %02X, HP %u %u | %u %u\n", g, (unsigned)n,
			T.partner[s][n][0], T.partner[s][n][1], hp[0], hp[1], hp[2], hp[3]);
	}
	fflush(stdout);
	return n && !differing;
}

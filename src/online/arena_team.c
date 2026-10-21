/* --dev arena2v2 (arena_team.h). */
#include "arena_team.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_input.h"
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
/* the partners' rings in each core's RAM (RAM_TEAM_RING): a side's RING
 * slots of a u16 of keys and the u16 iteration they are for */
#define RING       8      /* a power of two past DELAY and the GBAs' skew (two iterations at most, engine-2v2) */
#define SLOT_SIZE  4
#define DELAY      4      /* iterations from a partner's keys sampled to taken: BN6's own link's, a key pressed at a GBA's frame t
                             in both GBAs' key array at t + 4 (the engine study), so a partner's keys are as late as his lead's */

/* (allocated as the hooks go on: what a battle of the flag's is compared by, ~0.5 MB nobody else pays for) */
typedef struct {
	int slot[SIDES];                         /* each GBA's core's hook slot (hook_slot_of) */
	uint32_t digest[HOOK_CORES][ITERATIONS]; /* each core's, by the loop's iteration */
	uint8_t partner[HOOK_CORES][ITERATIONS][SIDES];   /* each side's partner's panel, row << 4 | column */
	uint16_t hp[HOOK_CORES][ITERATIONS][SIDES * MEGAMEN];
	uint16_t keys[HOOK_CORES][ITERATIONS][SIDES];     /* each partner's keys taken */
	uint8_t late[HOOK_CORES][ITERATIONS];    /* the sides whose partner's slot held no keys for the iteration */
	uint32_t last[HOOK_CORES];               /* the last iteration each met */
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

static uint32_t ring_at(unsigned side, uint32_t iteration) {
	return RAM_TEAM_RING + side * RING * SLOT_SIZE + (iteration & (RING - 1)) * SLOT_SIZE;
}

static HookAct spawn(HookRegs *r, void *user) {
	(void)user;
	/* (a NetBattle's list, whichever of them: its first two MegaMen, side 0's and side 1's) */
	if (hook_read8(r->r[0]) != BN6_ENTITY_MEGAMAN || hook_read8(r->r[0] + BN6_ENTITY_SIZE) != (BN6_ENTITY_MEGAMAN | 1)) return HOOK_CONTINUE;
	for (unsigned i = 0; i < sizeof team_list; ++i) hook_write8(RAM_TEAM + i, team_list[i]);
	r->r[0] = RAM_TEAM;
	/* (the rings emptied with the battle: the count starts again at 0, RAM_TEAM_TICK cleared with the battle's RAM) */
	for (uint32_t a = RAM_TEAM_RING; a < RAM_TEAM_RING_END; a += 4) hook_write32(a, 0);
	return HOOK_CONTINUE;
}

void arena_team_sample(struct mCore *core[2], unsigned side, uint16_t keys) {
	struct mCore *own = core[side & 1];
	uint32_t at = own->rawRead32(own, RAM_TEAM_TICK, -1) + DELAY, slot = ring_at(side & 1, at);
	for (unsigned g = 0; g < SIDES; ++g) {
		core[g]->rawWrite16(core[g], slot, -1, (uint16_t)(keys & ARENA_KEYS_MASK));
		core[g]->rawWrite16(core[g], slot + 2, -1, (uint16_t)at);
	}
}

uint16_t arena_team_script(unsigned player, uint32_t frame) {
	if (player == 2) {
		/* side 0's partner: up to row 1 (column 1), charged shots along it */
		unsigned t = frame % 480;
		if (t < 4 || (t >= 16 && t < 20)) return KEY_UP;
		return (t >= 40 && t < 150) || (t >= 200 && t < 310) ? KEY_B : 0;
	}
	if (player == 3) {
		/* side 1's partner: charged shots along row 1 (column 6), a step down and back between */
		unsigned t = (frame + 100) % 400;
		if (t >= 300 && t < 304) return KEY_DOWN;
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
				T->hp[slot][tick][side * MEGAMEN + k] = hp;
				if (k) T->partner[slot][tick][side] = panel;
			}
		}
	return fnv(fnv(h, hook_read32(BN6_RNG_PRIMARY)), hook_read32(BN6_BATTLE_TIMER));
}

/* a side's partner's keys for the loop's iteration `tick`, from the ring
 * (his last held where the slot holds none for it: the battle's first
 * DELAY iterations, or a sample late) */
static uint16_t taken(unsigned side, uint32_t tick, uint32_t ai, uint8_t *late) {
	uint32_t at = ring_at(side, tick);
	if (hook_read16(at + 2) == (uint16_t)tick) return hook_read16(at);
	if (tick > DELAY) *late |= (uint8_t)(1u << side);
	return hook_read16(ai + BN6_AI_HELD) & ARENA_KEYS_MASK;
}

/* side 0's copy (r0 0), the loop's iteration counted: the partners' keys
 * taken from the ring and written as BN6_KEY_COPY writes a player's, then
 * the state recorded */
static HookAct keys(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] != 0) return HOOK_CONTINUE;
	uint32_t tick = hook_read32(RAM_TEAM_TICK) + 1;
	hook_write32(RAM_TEAM_TICK, tick);
	int slot = hook_slot();
	bool kept = T && slot >= 0 && slot < HOOK_CORES && tick < ITERATIONS;
	uint8_t late = 0;
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER);
		uint32_t ai = obj ? hook_read32(obj + BN6_T1_AI_DATA) : 0;
		if (!ai) continue;
		uint16_t k = taken(side, tick, ai, &late);
		/* (the mutation: one key of side 0's partner flipped on the second GBA alone) */
		if (dev.arena_2v2 > 1 && T && slot == T->slot[1] && side == 0 && tick >= (uint32_t)dev.arena_2v2 && tick < (uint32_t)dev.arena_2v2 + MUTATED)
			k ^= KEY_RIGHT;
		if (kept) T->keys[slot][tick][side] = k;
		k |= BN6_KEYS_VALID;
		uint16_t held = hook_read16(ai + BN6_AI_HELD);
		hook_write16(ai + BN6_AI_LAST, held);
		hook_write16(ai + BN6_AI_HELD, k);
		hook_write16(ai + BN6_AI_PRESSED, (uint16_t)(~held & k));
		hook_write16(ai + BN6_AI_UP, (uint16_t)(~k & held));
	}
	if (kept) {
		T->late[slot][tick] = late;
		T->digest[slot][tick] = digest(slot, tick);
		if (tick > T->last[slot]) T->last[slot] = tick;
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
	bool ok = c && hook_on(c, BN6_SPAWN_ENTITIES, spawn, NULL) && hook_on(c, BN6_KEY_COPY, keys, NULL);
	/* (under full rules BN5's chip AI gives side 0's player a block of its own: the first MegaMan's alone) */
	darkinvs_two_a_side(true);
	T->slot[side] = hook_slot_of(core);
	if (!ok) printf("arena2v2: the hooks at %08X and %08X not taken on side %u's core\n", BN6_SPAWN_ENTITIES, BN6_KEY_COPY, side);
	return ok;
}

/* how the partners' keys went in over the iterations both GBAs ran */
static void report_keys(int a, int b, uint32_t n) {
	uint32_t late = 0, pressed[SIDES] = { 0 }, keys_apart = 0;
	for (uint32_t t = 1; t <= n; ++t) {
		late += (T->late[a][t] | T->late[b][t]) != 0;
		keys_apart += memcmp(T->keys[a][t], T->keys[b][t], sizeof T->keys[a][t]) != 0;
		for (unsigned s = 0; s < SIDES; ++s) pressed[s] += T->keys[a][t][s] != 0;
	}
	printf("arena2v2: the partners' keys from their rings: %u iterations late on either GBA, %u taken apart; "
		"held in %u and %u iterations (side 0's, side 1's)\n", (unsigned)late, (unsigned)keys_apart, (unsigned)pressed[0],
		(unsigned)pressed[1]);
}

bool arena_team_report(void) {
	if (!T) return false;
	int a = T->slot[0], b = T->slot[1];
	if (a < 0 || b < 0 || a >= HOOK_CORES || b >= HOOK_CORES) return false;
	uint32_t n = T->last[a] < T->last[b] ? T->last[a] : T->last[b], differing = 0, first = 0, moved = 0;
	for (uint32_t t = 1; t <= n; ++t) {
		if (T->digest[a][t] != T->digest[b][t]) { if (!differing) first = t; ++differing; }
		if (t > 1 && T->partner[a][t][0] != T->partner[a][t - 1][0]) ++moved;
	}
	printf("arena2v2: iterations %u and %u, compared %u, differing %u (first %u); side 0's partner moved %u times\n",
		(unsigned)T->last[a], (unsigned)T->last[b], (unsigned)n, (unsigned)differing, (unsigned)first, (unsigned)moved);
	report_keys(a, b, n);
	for (unsigned g = 0; g < SIDES && n; ++g) {
		int s = g ? b : a;
		const uint16_t *hp = T->hp[s][n];
		printf("arena2v2: GBA %u at iteration %u: partners at %02X and %02X, HP %u %u | %u %u\n", g, (unsigned)n,
			T->partner[s][n][0], T->partner[s][n][1], hp[0], hp[1], hp[2], hp[3]);
	}
	/* (each hit as the first GBA had it: whose HP fell when, each MegaMan's own) */
	static const char *const who[SIDES * MEGAMEN] = { "side 0's player", "side 0's partner", "side 1's player", "side 1's partner" };
	for (uint32_t t = 2; t <= n; ++t)
		for (unsigned m = 0; m < SIDES * MEGAMEN; ++m)
			if (T->hp[a][t][m] != T->hp[a][t - 1][m])
				printf("arena2v2: iteration %u: %s's HP %u to %u (the others' %u %u %u %u)\n", (unsigned)t, who[m], T->hp[a][t - 1][m],
					T->hp[a][t][m], T->hp[a][t][0], T->hp[a][t][1], T->hp[a][t][2], T->hp[a][t][3]);
	fflush(stdout);
	return n && !differing;
}

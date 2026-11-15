/* party_field.h. The battle's list grown with the partners, and their keys,
 * as --dev arena2v2's do for a NetBattle (arena_team.c), on the run's own
 * core and its own battles. */
#include "party_field.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bn6_link.h"
#include "bytes.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"

#define LIST_MOST  ((RAM_TEAM_END - RAM_TEAM) / BN6_ENTITY_SIZE)   /* entries the grown list holds, its end among them */
#define SIDE_MOST  4                                               /* BN6's actors a side (bn6f BATTLE_ACTORS_PER_SIDE) */

static struct {
	int n;                        /* each battle's MegaMen (1 alone) */
	int hp_set, end;              /* the test's partners' HP and its enemies' last iteration (0 none) */
	int standing;                 /* the battle under way's MegaMen spawned (0 none) */
	uint16_t hp[2][SIDE_MOST];    /* each side's actors' HP as last seen, for the log */
	uint32_t obj[2][SIDE_MOST];   /* ... and their objects */
	uint32_t tick;
} P;

/* the panels a partner takes on side 0, in order: the back column first,
 * then the middle's free ones (row << 4 | column, as the list writes them) */
static const uint8_t panels[] = { 0x11, 0x31, 0x21, 0x12, 0x32, 0x13, 0x33, 0x23, 0x22 };

static bool taken(uint8_t panel, const uint8_t *list, int count) {
	for (int i = 0; i < count; ++i) {
		const uint8_t *e = list + i * BN6_ENTITY_SIZE;
		if (!(e[BN6_ENTITY_KIND] & 1) && e[BN6_ENTITY_PANEL] == panel) return true;
	}
	return false;
}

static HookAct spawn(HookRegs *r, void *user) {
	(void)user;
	int n = P.n;
	uint32_t at = r->r[0];
	/* (a battle of the run's own: MegaMan first, no MegaMan on side 1, so never a NetBattle's) */
	if (n < 2 || hook_read8(at) != BN6_ENTITY_MEGAMAN || hook_read8(at + BN6_ENTITY_SIZE) == (BN6_ENTITY_MEGAMAN | 1)) return HOOK_CONTINUE;
	uint8_t old[LIST_MOST * BN6_ENTITY_SIZE];
	int count = 0;
	while (count < (int)LIST_MOST && hook_read8(at + (uint32_t)count * BN6_ENTITY_SIZE) != BN6_ENTITY_END) {
		for (unsigned b = 0; b < BN6_ENTITY_SIZE; ++b) old[count * BN6_ENTITY_SIZE + (int)b] = hook_read8(at + (uint32_t)count * BN6_ENTITY_SIZE + b);
		++count;
	}
	/* (the grown list: the player, the partners on free panels, then the rest as it was, as many as RAM_TEAM holds) */
	uint8_t list[LIST_MOST * BN6_ENTITY_SIZE];
	int len = 0, added = 0;
	memcpy(list, old, BN6_ENTITY_SIZE);
	++len;
	for (unsigned p = 0; p < sizeof panels && added < n - 1 && len < (int)LIST_MOST - 1; ++p) {
		if (taken(panels[p], old, count)) continue;
		uint8_t *e = list + len * BN6_ENTITY_SIZE;
		memset(e, 0, BN6_ENTITY_SIZE);
		e[BN6_ENTITY_KIND] = BN6_ENTITY_MEGAMAN;
		e[BN6_ENTITY_PANEL] = panels[p];
		++len;
		++added;
	}
	for (int i = 1; i < count && len < (int)LIST_MOST - 1; ++i, ++len) memcpy(list + len * BN6_ENTITY_SIZE, old + i * BN6_ENTITY_SIZE, BN6_ENTITY_SIZE);
	memset(list + len * BN6_ENTITY_SIZE, 0, BN6_ENTITY_SIZE);
	list[len * BN6_ENTITY_SIZE] = BN6_ENTITY_END;
	for (unsigned b = 0; b < (unsigned)(len + 1) * BN6_ENTITY_SIZE; ++b) hook_write8(RAM_TEAM + b, list[b]);
	r->r[0] = RAM_TEAM;
	P.standing = 1 + added;
	P.tick = 0;
	memset(P.hp, 0, sizeof P.hp);
	memset(P.obj, 0, sizeof P.obj);
	printf("party: a battle with %d MegaMen (%d of its %d entries kept)\n", P.standing, len - added, count);
	for (int i = 0; i < len; ++i) {
		const uint8_t *e = list + i * BN6_ENTITY_SIZE;
		printf("party:   entry %d: kind %02X at column %u row %u, id %03X\n", i, e[BN6_ENTITY_KIND], e[BN6_ENTITY_PANEL] & 15u,
			(unsigned)e[BN6_ENTITY_PANEL] >> 4, (unsigned)get16(e + BN6_ENTITY_ID));
	}
	return HOOK_CONTINUE;
}

/* the spike's partners: each his own pattern of steps and charged shots, so
 * that they stand on different rows at different times */
static uint16_t scripted(int k, uint32_t tick) {
	uint32_t t = (tick + (uint32_t)k * 97u) % 360u;
	if (k == 1) return t < 4 ? KEY_UP : (t >= 180 && t < 184) ? KEY_DOWN : (t % 120 < 90 ? KEY_B : 0);
	if (k == 2) return t < 4 ? KEY_DOWN : (t >= 180 && t < 184) ? KEY_UP : (t % 100 < 70 ? KEY_B : 0);
	return (t >= 60 && t < 64) ? KEY_RIGHT : (t >= 240 && t < 244) ? KEY_LEFT : (t % 90 < 60 ? KEY_B : 0);
}

static void watch(uint32_t tick) {
	for (unsigned side = 0; side < 2; ++side)
		for (unsigned k = 0; k < SIDE_MOST; ++k) {
			uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + k * BN6_SIDE_PARTNER);
			uint16_t hp = obj ? hook_read16(obj + BN6_T1_HP) : 0;
			if (obj != P.obj[side][k])
				printf("party: iteration %u: side %u's actor %u %s (%s %03X, HP %u)\n", (unsigned)tick, side, k, obj ? "in" : "gone",
					side ? "id" : "name", obj ? hook_read16(obj + BN6_T1_NAME_ID) : 0, hp);
			else if (obj && hp != P.hp[side][k])
				printf("party: iteration %u: side %u's actor %u HP %u to %u, at column %u row %u\n", (unsigned)tick, side, k,
					P.hp[side][k], hp, hook_read8(obj + BN6_T1_PANEL_X), hook_read8(obj + BN6_T1_PANEL_Y));
			P.obj[side][k] = obj;
			P.hp[side][k] = hp;
		}
}

/* side 0's copy (r0 0): each partner's keys into his AI data as the copy
 * writes the player's */
static HookAct keys(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] != 0 || P.standing < 2) return HOOK_CONTINUE;
	uint32_t tick = ++P.tick;
	for (int k = 1; k < P.standing && k < SIDE_MOST; ++k) {
		uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + (uint32_t)k * BN6_SIDE_PARTNER);
		uint32_t ai = obj ? hook_read32(obj + BN6_T1_AI_DATA) : 0;
		if (!ai) continue;
		uint16_t key = (uint16_t)(scripted(k, tick) | BN6_KEYS_VALID);
		uint16_t held = hook_read16(ai + BN6_AI_HELD);
		hook_write16(ai + BN6_AI_LAST, held);
		hook_write16(ai + BN6_AI_HELD, key);
		hook_write16(ai + BN6_AI_PRESSED, (uint16_t)(~held & key));
		hook_write16(ai + BN6_AI_UP, (uint16_t)(~key & held));
	}
	/* (the spike's deletion test: the partners' HP set as the battle's first iteration runs) */
	if (tick == 1 && P.hp_set > 0)
		for (int k = 1; k < P.standing && k < SIDE_MOST; ++k) {
			uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + (uint32_t)k * BN6_SIDE_PARTNER);
			if (obj) hook_write16(obj + BN6_T1_HP, (uint16_t)P.hp_set);
		}
	if (P.end > 0 && tick == (uint32_t)P.end)
		for (unsigned k = 0; k < SIDE_MOST; ++k) {
			uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + BN6_BATTLE_PLAYER_STEP + k * BN6_SIDE_PARTNER);
			if (obj && hook_read16(obj + BN6_T1_HP) > 1) hook_write16(obj + BN6_T1_HP, 1);
		}
	watch(tick);
	return HOOK_CONTINUE;
}

void party_field_install(int n, int hp, int end) {
	static bool done;
	P.n = n < 1 ? 1 : n > PARTY_MOST ? PARTY_MOST : n;
	P.hp_set = hp;
	P.end = end;
	if (done) return;
	done = true;
	if (!emu_hook(BN6_SPAWN_ENTITIES, spawn, NULL) || !emu_hook(BN6_KEY_COPY, keys, NULL))
		printf("party: the hooks (%08X, %08X) not taken\n", BN6_SPAWN_ENTITIES, BN6_KEY_COPY);
}

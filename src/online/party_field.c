/* party_field.h. The battle's list grown with the partners, and their keys,
 * as --dev arena2v2's do for a NetBattle (arena_team.c), on the run's own
 * core (the spike) or the party's own (party_core.c). */
#include "party_field.h"

#include <stdio.h>
#include <string.h>

#include "arena_team_custom.h"
#include "arena_team_navi.h"
#include "arena_team_wait.h"
#include "bn6.h"
#include "bn6_link.h"
#include "bytes.h"
#include "emu.h"
#include "free_space.h"

/* (the grown list in RAM_TEAM, --dev arena2v2's: on the party's core no
 * NetBattle ever runs, and the spike is a dev flag's on the run's core) */
#define LIST_MOST  ((RAM_TEAM_END - RAM_TEAM) / BN6_ENTITY_SIZE)   /* entries the grown list holds, its end among them */
#define SIDE_MOST  4                                               /* BN6's actors a side (bn6f BATTLE_ACTORS_PER_SIDE) */
#define PARTNER_KEYS (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_B)   /* a partner's with no hand of his own: no A */
#define OWN_KEYS     (PARTNER_KEYS | KEY_A)                                /* ... with his own (f->own): A uses his chip */
#define AIM_TURN     240                                           /* the battle loop's iterations each MegaMan in turn draws the enemies' aim */

_Static_assert(SIDE_MOST == PARTY_MOST, "a party is a side's actors");
#define DEL_ENDS 60   /* partydel's: its enemies deleted so many iterations after its MegaMen */

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

static void say_list(const PartyField *f, const uint8_t *list, int len, int added, int count) {
	if (f->quiet) return;
	printf("party: a battle with %d MegaMen (%d of its %d entries kept)\n", f->standing, len - added, count);
	for (int i = 0; i < len; ++i) {
		const uint8_t *e = list + i * BN6_ENTITY_SIZE;
		printf("party:   entry %d: kind %02X at column %u row %u, id %03X\n", i, e[BN6_ENTITY_KIND], e[BN6_ENTITY_PANEL] & 15u,
			(unsigned)e[BN6_ENTITY_PANEL] >> 4, (unsigned)get16(e + BN6_ENTITY_ID));
	}
}

static HookAct spawn(HookRegs *r, void *user) {
	PartyField *f = user;
	int n = f->n;
	uint32_t at = r->r[0];
	/* (a battle of the run's own: MegaMan first, no MegaMan on side 1, so never a NetBattle's; a field of one, the dev's
	 * solo on the party's core, tools/party_guardian.py, its list as it was) */
	if (n < 1 || hook_read8(at) != BN6_ENTITY_MEGAMAN || hook_read8(at + BN6_ENTITY_SIZE) == (BN6_ENTITY_MEGAMAN | 1)) return HOOK_CONTINUE;
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
	f->standing = 1 + added;
	f->tick = 0;
	memset(f->seen_hp, 0, sizeof f->seen_hp);
	memset(f->seen_obj, 0, sizeof f->seen_obj);
	memset(f->seen_at, 0xFF, sizeof f->seen_at);
	say_list(f, list, len, added, count);
	/* (the partners' own NaviStats and hands in their rooms, their decks and turns emptied: arena_team_navi.h, arena_team_custom.h) */
	if (f->own) {
		hook_write32(RAM_TEAM_TICK, 0);
		arena_team_navi_begin();
		arena_team_custom_begin();
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

static void watch(PartyField *f, uint32_t tick) {
	for (unsigned side = 0; side < 2; ++side)
		for (unsigned k = 0; k < SIDE_MOST; ++k) {
			uint32_t obj = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + k * BN6_SIDE_PARTNER);
			uint16_t hp = obj ? hook_read16(obj + BN6_T1_HP) : 0;
			if (f->quiet) {
			} else if (obj != f->seen_obj[side][k])
				printf("party: iteration %u: side %u's actor %u %s (%s %03X, HP %u)\n", (unsigned)tick, side, k, obj ? "in" : "gone",
					side ? "id" : "name", obj ? hook_read16(obj + BN6_T1_NAME_ID) : 0, hp);
			else if (obj && hp != f->seen_hp[side][k])
				printf("party: iteration %u: side %u's actor %u HP %u to %u, at column %u row %u\n", (unsigned)tick, side, k,
					f->seen_hp[side][k], hp, hook_read8(obj + BN6_T1_PANEL_X), hook_read8(obj + BN6_T1_PANEL_Y));
			f->seen_obj[side][k] = obj;
			f->seen_hp[side][k] = hp;
		}
}

/* side 0's actor k's battle object, 0 none */
static uint32_t actor(unsigned side, int k) { return hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + (uint32_t)k * BN6_SIDE_PARTNER); }

/* the battle's first iteration: each MegaMan's HP going in, and the spike's partners' */
static void first_iteration(const PartyField *f) {
	for (int k = 0; k < f->standing && k < SIDE_MOST; ++k) {
		uint32_t obj = actor(0, k);
		if (!obj) continue;
		if (f->max_hp[k]) hook_write16(obj + BN6_T1_MAX_HP, f->max_hp[k]);
		if (f->hp[k]) hook_write16(obj + BN6_T1_HP, f->hp[k]);
		if (k && f->hp_set > 0) hook_write16(obj + BN6_T1_HP, (uint16_t)f->hp_set);
	}
}

/* each partner's own hand as it changes (his picks in, a chip used), said in the log */
static void hands_watch(PartyField *f, uint32_t tick) {
	for (int k = 1; k < f->standing && k < SIDE_MOST; ++k) {
		uint32_t h = arena_team_navi_hand((unsigned)k - 1);
		uint8_t at = h ? hook_read8(h + BN6_HAND_AT) : 0xFF;
		if (at == f->seen_at[k]) continue;
		f->seen_at[k] = at;
		if (f->quiet || !h) continue;
		uint16_t chip = at < BN6_HAND_MAX ? hook_read16(h + BN6_HAND_CHIPS + 2u * at) : 0xFFFF;
		printf("party: iteration %u: partner %d's own hand: chip up next %u (%03X), his lead's %u\n", (unsigned)tick, k, at, chip,
			hook_read8(BN6_BATTLE_HAND + BN6_HAND_AT));
	}
}

/* the dev's partyend: the enemies' HP set to 1 at its iteration; with
 * partydel the party's MegaMen of its mask deleted then, as a hit would,
 * and the enemies a second later, so that the battle ends whatever its
 * foes (a crew's pull-back's test, issue #718) */
static void dev_end(const PartyField *f, uint32_t tick) {
	if (tick == (uint32_t)f->end) {
		for (unsigned k = 0; k < SIDE_MOST; ++k) {
			uint32_t obj = actor(1, (int)k);
			if (obj && hook_read16(obj + BN6_T1_HP) > 1) hook_write16(obj + BN6_T1_HP, 1);
		}
		for (int k = 0; k < f->standing && k < SIDE_MOST; ++k) {
			uint32_t gone = f->del >> k & 1 ? actor(0, k) : 0;
			if (gone) hook_write16(gone + BN6_T1_HP, 0);
		}
	}
	if (f->del > 0 && tick == (uint32_t)f->end + DEL_ENDS)
		for (int k = 0; k < SIDE_MOST; ++k) {
			uint32_t obj = actor(1, k);
			if (obj) hook_write16(obj + BN6_T1_HP, 0);
		}
}

/* side 0's copy (r0 0): each partner's keys into his AI data as the copy
 * writes the player's */

static void trade(unsigned k);

static HookAct keys(HookRegs *r, void *user) {
	PartyField *f = user;
	if (r->r[0] != 0 || f->standing < 1) return HOOK_CONTINUE;
	uint32_t tick = ++f->tick;
	/* (each partner's own Custom screen's picks into his own hand, at the iteration they were set for: arena_team_custom.h) */
	if (f->own) {
		hook_write32(RAM_TEAM_TICK, tick);
		const uint32_t hands[TEAM_PARTNERS] = { arena_team_navi_hand(0), arena_team_navi_hand(1), arena_team_navi_hand(2) };
		arena_team_custom_take(tick, hands, false);
	}
	for (int k = 1; k < f->standing && k < SIDE_MOST; ++k) {
		uint32_t obj = actor(0, k);
		uint32_t ai = obj ? hook_read32(obj + BN6_T1_AI_DATA) : 0;
		if (!ai) continue;
		uint16_t key = (uint16_t)((f->still ? 0 : f->scripted ? scripted(k, tick) : f->keys[k] & (f->own ? OWN_KEYS : PARTNER_KEYS)) | BN6_KEYS_VALID);
		uint16_t held = hook_read16(ai + BN6_AI_HELD);
		hook_write16(ai + BN6_AI_LAST, held);
		hook_write16(ai + BN6_AI_HELD, key);
		hook_write16(ai + BN6_AI_PRESSED, (uint16_t)(~held & key));
		hook_write16(ai + BN6_AI_UP, (uint16_t)(~key & held));
	}
	if (tick == 1) first_iteration(f);
	/* (the viruses' bench: the lead's slot and partner 1's traded for good at this iteration, issue #777) */
	if (f->swap_at > 0 && tick == (uint32_t)f->swap_at && f->standing > 1) trade(1);
	if (f->own) hands_watch(f, tick);
	if (f->end > 0) dev_end(f, tick);
	watch(f, tick);
	return HOOK_CONTINUE;
}

/* An enemy's spawn, as it is given its HP and MaxHP (r2): the guardian's
 * raised for the party, as encounter.c's emu_battle_hp raises the run's */
static HookAct spawned(HookRegs *r, void *user) {
	PartyField *f = user;
	if (f->foe_from > 0 && hook_read8(r->r[5] + BN6_T1_ALLIANCE) == 1 && r->r[2] == (uint32_t)f->foe_from) r->r[2] = (uint32_t)f->foe_to;
	return HOOK_CONTINUE;
}

/* side 0's slot k, its object */
static uint32_t slot(unsigned k) { return BN6_BATTLE_PLAYERS + k * BN6_SIDE_PARTNER; }

static void trade(unsigned k) {
	uint32_t lead = hook_read32(slot(0)), other = hook_read32(slot(k));
	hook_write32(slot(0), other);
	hook_write32(slot(k), lead);
}

/* the MegaMan whose turn it is to draw the enemies' aim: the standing ones in turn, AIM_TURN iterations each */
static unsigned aimed(const PartyField *f) {
	unsigned up[SIDE_MOST], n = 0;
	for (unsigned k = 0; k < (unsigned)f->standing && k < SIDE_MOST; ++k) {
		uint32_t obj = hook_read32(slot(k));
		if (obj && hook_read16(obj + BN6_T1_HP)) up[n++] = k;
	}
	return n ? up[(f->tick / AIM_TURN) % n] : 0;
}


/* an object's routine about to run (r5 the object): one of side 1's sees the MegaMan in turn in side 0's first slot */
static HookAct logic_in(HookRegs *r, void *user) {
	PartyField *f = user;
	f->swapped = 0;
	if (!f->spread || f->standing < 2 || !r->r[5] || hook_read8(r->r[5] + BN6_T1_ALLIANCE) != 1) return HOOK_CONTINUE;
	unsigned k = aimed(f);
	if (k) { trade(k); f->swapped = k; }
	return HOOK_CONTINUE;
}

/* ... and returned: the lead's object back in the first slot */
static HookAct logic_back(HookRegs *r, void *user) {
	(void)r;
	PartyField *f = user;
	if (f->swapped) trade(f->swapped);
	f->swapped = 0;
	return HOOK_CONTINUE;
}

bool party_field_aim(HookCore *core, PartyField *f) {
	return hook_on(core, BN6_OBJECT_LOGIC_CALL, logic_in, f) && hook_on(core, BN6_OBJECT_LOGIC_BACK, logic_back, f);
}

bool party_field_hooks(HookCore *core, PartyField *f) {
	return hook_on(core, BN6_SPAWN_ENTITIES, spawn, f) && hook_on(core, BN6_KEY_COPY, keys, f) &&
		(f->foe_from <= 0 || f->foe_to == f->foe_from || hook_on(core, BN6_SPAWN_HP, spawned, f)) &&
		(!f->own || (arena_team_navi_hooks(core, TEAM_PARTNERS, -1) && arena_team_wait_hooks(core)));
}


void party_field_install(int n, int hp, int end, int aim, int swap_at) {
	static PartyField spike;
	static bool done;
	spike.n = n < 1 ? 1 : n > PARTY_MOST ? PARTY_MOST : n;
	spike.scripted = true;
	spike.hp_set = hp;
	spike.end = end;
	spike.spread = aim & 1;
	spike.still = (aim & 2) != 0;
	spike.swap_at = swap_at;
	if (done) return;
	done = true;
	if (!emu_hook(BN6_SPAWN_ENTITIES, spawn, &spike) || !emu_hook(BN6_KEY_COPY, keys, &spike) ||
		((aim & 1) && (!emu_hook(BN6_OBJECT_LOGIC_CALL, logic_in, &spike) || !emu_hook(BN6_OBJECT_LOGIC_BACK, logic_back, &spike))))
		printf("party: the hooks (%08X, %08X) not taken\n", BN6_SPAWN_ENTITIES, BN6_KEY_COPY);
}

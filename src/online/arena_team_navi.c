/* --dev arena2v2: a partner's own battle NaviStats (arena_team_navi.h). */
#include "arena_team_navi.h"

#include <stdio.h>

#include "bn6.h"
#include "devtools.h"
#include "hook.h"

#define KINDS 3   /* the objects a spawn makes: the first kind's (actors), the third's (attacks), the fourth's (effects) */
#define SIDES 2
#define HAND_SIZE 0x50   /* a side's hand (BN6_BATTLE_HAND): in a partner's room after his NaviStats */
#define ROOM  30  /* the fourth kind's slot side 0's partner's record takes, side 1's the next: the pool's last, which a spawn
                     takes only when the 30 before them are in play */

/* what the hooks keep a core between calls (a rollback puts it back,
 * hook_keep_slots): the rooms taken, whose the object spawning now is,
 * and whose each object slot of each kind is (0 none, 1 + the side of
 * the partner) */
typedef struct {
	uint8_t taken;
	uint8_t pending;
	uint8_t mark[KINDS][32];
	uint8_t seen[SIDES][HAND_SIZE];   /* each lead's hand as the partner's was last dealt from it */
} Kept;
static Kept kept[HOOK_CORES];
static int second_slot = -1;   /* the second GBA's core's slot: teamhp's mutation (negative) there alone */
/* the records' answers by the asker (a partner's own object, one of each kind he spawned) and the objects marked a
 * partner's, every core's, a rollback's frames run again among them: what the output says */
static uint32_t answered[SIDES][1 + KINDS], hands[SIDES][1 + KINDS], dealt[SIDES], marked;

/* side's partner's room: the fourth kind's slot ROOM + side, from its node (BN6_T4_SIZE bytes: his NaviStats, then his hand) */
static uint32_t room(unsigned side) { return BN6_T4_OBJECTS - BN6_OBJECT_NODE + (ROOM + side) * BN6_T4_SIZE; }
static uint32_t hand(unsigned side) { return room(side) + BN6_BATTLE_NAVI_SIZE; }
static uint32_t room_bit(unsigned side) { return 1u << (31 - (ROOM + side)); }

/* an object's kind and slot from its address; false where it is none of them */
static bool slot_of(uint32_t obj, unsigned *kind, unsigned *slot) {
	static const uint32_t base[KINDS] = { BN6_T1_OBJECTS, BN6_T3_OBJECTS, BN6_T4_OBJECTS };
	static const uint32_t size[KINDS] = { BN6_T1_SIZE, BN6_T1_SIZE, BN6_T4_SIZE };
	for (unsigned k = 0; k < KINDS; ++k)
		if (obj >= base[k] && obj < base[k] + 32 * size[k] && (obj - base[k]) % size[k] == 0) {
			*kind = k;
			*slot = (obj - base[k]) / size[k];
			return true;
		}
	return false;
}

/* whose an object is: 1 + the side of the partner it is, or he spawned,
 * or they spawned in turn; 0 neither. *how: 0 his own object, 1 + its kind */
static unsigned whose(uint32_t obj, unsigned *how) {
	int s = hook_slot();
	*how = 0;
	if (!obj || s < 0 || s >= HOOK_CORES || !kept[s].taken) return 0;
	for (unsigned side = 0; side < SIDES; ++side)
		if (obj == hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER)) return 1 + side;
	unsigned kind, slot;
	if (!slot_of(obj, &kind, &slot)) return 0;
	*how = 1 + kind;
	return kept[s].mark[kind][slot];
}

/* every spawn: whose the object spawning is */
static HookAct spawning(HookRegs *r, void *user) {
	(void)user;
	int s = hook_slot();
	unsigned how;
	if (s >= 0 && s < HOOK_CORES) kept[s].pending = (uint8_t)whose(r->r[5], &how);
	return HOOK_CONTINUE;
}

/* ... and the object made marked so */
static HookAct made(HookRegs *r, void *user) {
	(void)user;
	int s = hook_slot();
	unsigned kind, slot;
	if (s >= 0 && s < HOOK_CORES && slot_of(r->r[5], &kind, &slot)) {
		kept[s].mark[kind][slot] = kept[s].pending;
		marked += kept[s].pending != 0;
	}
	return HOOK_CONTINUE;
}

/* a battle NaviStats asked for by object: a partner's own record for his */
static HookAct stats(HookRegs *r, void *user) {
	(void)user;
	uint32_t from = r->lr & ~1u;
	if (from < BN6_NAVI_BY_OBJECT || from >= BN6_NAVI_BY_OBJECT_END) return HOOK_CONTINUE;
	unsigned how, who = whose(r->r[5], &how);
	/* (his side asked for: a partner's object's own alliance) */
	if (!who || r->r[0] != who - 1) return HOOK_CONTINUE;
	++answered[who - 1][how];
	r->r[0] = room(who - 1);
	r->r[1] = BN6_BATTLE_NAVI;   /* (as the routine leaves it) */
	return HOOK_RETURN;
}

/* a side's hand asked for with a partner's object in r5 (the chip up next, a chip used): his own */
static HookAct hand_of(HookRegs *r, void *user) {
	(void)user;
	unsigned how, who = whose(r->r[5], &how);
	if (!who || r->r[0] != who - 1) return HOOK_CONTINUE;
	++hands[who - 1][how];
	r->r[0] = hand(who - 1);
	r->r[1] = BN6_BATTLE_HAND;   /* (as the routine leaves it) */
	return HOOK_RETURN;
}

bool arena_team_navi_hooks(struct HookCore *core, unsigned side, int slot) {
	if (side == 1) second_slot = slot;
	if (!hook_keep_slots(kept, sizeof *kept)) return false;
	return hook_on(core, BN6_SPAWN_COMMON, spawning, NULL) && hook_on(core, BN6_SPAWN_MADE, made, NULL) &&
		hook_on(core, BN6_NAVI_STATS_ADDR, stats, NULL) && hook_on(core, BN6_HAND_ADDR, hand_of, NULL);
}

void arena_team_navi_begin(void) {
	int s = hook_slot();
	if (s < 0 || s >= HOOK_CORES) return;
	kept[s] = (Kept){ 0 };
	/* (the rooms: two slots of the fourth kind's 32 taken in BN6's own pool as the battle has just set it up, never run or
	 * freed since nothing links them into the run order; BN6 clears them with the pool as the next battle sets up) */
	uint32_t active = hook_read32(BN6_T4_ACTIVE), bits = room_bit(0) | room_bit(1);
	if (active & bits) {
		printf("arena2v2: the partners' rooms in the fourth kind's pool are taken (%08X): their NaviStats their sides'\n", (unsigned)active);
		return;
	}
	hook_write32(BN6_T4_ACTIVE, active | bits);
	kept[s].taken = 1;
	/* (teamhp=N: each partner's own max HP, as his own loadout would give it; -N the second GBA's alone, a mutation that must
	 * part them) */
	int hp = dev.team_hp < 0 && s == second_slot ? -dev.team_hp : dev.team_hp;
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t to = room(side), from = BN6_BATTLE_NAVI + side * BN6_BATTLE_NAVI_SIZE;
		for (uint32_t i = 0; i < BN6_BATTLE_NAVI_SIZE; i += 4) hook_write32(to + i, hook_read32(from + i));
		/* (his hand his side's as the battle sets it up, empty) */
		for (unsigned i = 0; i < HAND_SIZE; ++i) {
			kept[s].seen[side][i] = hook_read8(BN6_BATTLE_HAND + side * HAND_SIZE + i);
			hook_write8(hand(side) + i, kept[s].seen[side][i]);
		}
		if (hp > 0)
			for (uint32_t f = BN6_NS_BASE_HP; f <= BN6_NS_MAX_HP; f += 2) hook_write16(to + f, (uint16_t)hp);
	}
}

void arena_team_navi_deal(void) {
	int s = hook_slot();
	if (s < 0 || s >= HOOK_CORES || !kept[s].taken) return;
	for (unsigned side = 0; side < SIDES; ++side) {
		/* (a lead's hand dealt anew by the Custom screen: its next chip the first, and other than last seen) */
		uint32_t lead = BN6_BATTLE_HAND + side * HAND_SIZE;
		uint8_t now[HAND_SIZE];
		bool other = false;
		for (unsigned i = 0; i < HAND_SIZE; ++i) {
			now[i] = hook_read8(lead + i);
			other |= now[i] != kept[s].seen[side][i];
		}
		if (!other || now[BN6_HAND_AT]) continue;
		for (unsigned i = 0; i < HAND_SIZE; ++i) {
			kept[s].seen[side][i] = now[i];
			hook_write8(hand(side) + i, now[i]);
		}
		++dealt[side];
	}
}

uint8_t arena_team_navi_at(unsigned side, bool partner) {
	return hook_read8(partner ? hand(side & 1) + BN6_HAND_AT : BN6_BATTLE_HAND + (side & 1) * HAND_SIZE + BN6_HAND_AT);
}

uint32_t arena_team_navi_digest(uint32_t h) {
	for (unsigned side = 0; side < SIDES; ++side) {
		for (uint32_t f = BN6_NS_BASE_HP; f <= BN6_NS_MAX_HP; f += 2) h = (h ^ hook_read16(room(side) + f)) * 16777619u;
		h = (h ^ hook_read8(hand(side) + BN6_HAND_AT)) * 16777619u;
	}
	return h;
}

void arena_team_navi_report(void) {
	for (unsigned side = 0; side < SIDES; ++side)
		printf("arena2v2: side %u's partner's own NaviStats answered %u times for him, %u for his attacks (the third kind), %u for "
			"his actors and %u for his effects\n", side, (unsigned)answered[side][0], (unsigned)answered[side][2],
			(unsigned)answered[side][1], (unsigned)answered[side][3]);
	for (unsigned side = 0; side < SIDES; ++side)
		printf("arena2v2: side %u's partner's own hand answered %u times for him, %u for his attacks, %u for his actors and %u for "
			"his effects; dealt %u times from his lead's\n", side, (unsigned)hands[side][0], (unsigned)hands[side][2],
			(unsigned)hands[side][1], (unsigned)hands[side][3], (unsigned)dealt[side]);
	printf("arena2v2: %u objects marked a partner's as they were made\n", (unsigned)marked);
}

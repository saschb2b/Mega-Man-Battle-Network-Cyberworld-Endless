/* --dev arena2v2: a partner's own battle NaviStats (arena_team_navi.h). */
#include "arena_team_navi.h"

#include <stdio.h>
#include <string.h>

#include <mgba/core/core.h>

#include "bn6.h"
#include "devtools.h"
#include "hook.h"
#include "team_shape.h"

#define KINDS 3   /* the objects a spawn makes: the first kind's (actors), the third's (attacks), the fourth's (effects) */
#define PARTNERS TEAM_PARTNERS
#define HAND_SIZE BN6_HAND_SIZE   /* a side's hand (BN6_BATTLE_HAND): in a partner's room after his NaviStats */
#define SLOTS 32  /* the fourth kind's slots: the partners' records take the pool's last (the Arena's 30 and 31, side 0's partner's
                     first; a party's 29 to 31), which a spawn takes only when the slots before them are in play */

/* what the hooks keep a core between calls (a rollback puts it back,
 * hook_keep_slots): the rooms taken, whose the object spawning now is,
 * and whose each object slot of each kind is (0 none, 1 + the partner:
 * the Arena's his side, team_shape.h) */
typedef struct {
	uint8_t taken;
	uint8_t pending;
	uint8_t mark[KINDS][32];
} Kept;
static Kept kept[HOOK_CORES];
static int second_slot = -1;   /* the second GBA's core's slot: teamhp's mutation (negative) there alone */
/* each partner's own loadout's numbers, the same on every client (arena_team_navi_loadout) */
static struct { bool given; uint16_t max_hp, base_hp; uint8_t attack, speed, charge; } own[PARTNERS];
/* ... and his NaviStats as BN6 compiled them from his NaviCust's board on his own client (arena_team_navicust.h) */
static struct { bool given; uint8_t stats[BN6_BATTLE_NAVI_SIZE]; } compiled[PARTNERS];
/* the records' answers by the asker (a partner's own object, one of each kind he spawned) and the objects marked a
 * partner's, every core's, a rollback's frames run again among them: what the output says */
static uint32_t answered[PARTNERS][1 + KINDS], hands[PARTNERS][1 + KINDS], marked;

/* partner p's room of n's: the fourth kind's slot SLOTS - n + p, from its node (BN6_T4_SIZE bytes: his NaviStats, then his hand) */
static uint32_t room_of(unsigned n, unsigned p) { return BN6_T4_OBJECTS - BN6_OBJECT_NODE + (SLOTS - n + p) * BN6_T4_SIZE; }
static uint32_t rooms_bits(unsigned n) { return ((1u << n) - 1u); }   /* (slot i bit 31 - i: the pool's last n, its low bits) */
/* ... from a hook */
static uint32_t room(unsigned p) { return room_of(team_partners(), p); }
static uint32_t hand(unsigned p) { return room(p) + BN6_BATTLE_NAVI_SIZE; }

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
	for (unsigned p = 0, n = team_partners(); p < n; ++p)
		if (obj == hook_read32(team_partner_slot(p))) return 1 + p;
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
	if (!who || r->r[0] != team_partner_side(who - 1)) return HOOK_CONTINUE;
	++answered[who - 1][how];
	r->r[0] = room(who - 1);
	r->r[1] = BN6_BATTLE_NAVI;   /* (as the routine leaves it) */
	return HOOK_RETURN;
}

/* a side's hand asked for with a partner's object in r5 (the chip up next, a chip used): his own */
static HookAct hand_of(HookRegs *r, void *user) {
	(void)user;
	unsigned how, who = whose(r->r[5], &how);
	if (!who || r->r[0] != team_partner_side(who - 1)) return HOOK_CONTINUE;
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

/* partner p's record made from his side's (s the core's hook slot, hp teamhp's) */
static void record_made(unsigned p, int s, int hp) {
	unsigned side = team_partner_side(p);
	uint32_t to = room(p), from = BN6_BATTLE_NAVI + side * BN6_BATTLE_NAVI_SIZE;
	for (uint32_t i = 0; i < BN6_BATTLE_NAVI_SIZE; i += 4) hook_write32(to + i, hook_read32(from + i));
	/* (his NaviCust's programs whole: his NaviStats as BN6 compiled them, but for the two bytes a link battle's start sets in
	 * every MegaMan's alike, kept from his side's; teamcust=-1 the second GBA's his side's copy, a mutation that must part them) */
	if (compiled[p].given && !(dev.team_cust == -1 && s == second_slot)) {
		uint8_t start = hook_read8(from + BN6_NS_START_FORM), beast = hook_read8(from + BN6_NS_BEAST_TURNS);
		for (uint32_t i = 0; i < BN6_BATTLE_NAVI_SIZE; ++i) hook_write8(to + i, compiled[p].stats[i]);
		hook_write8(to + BN6_NS_START_FORM, start);
		hook_write8(to + BN6_NS_BEAST_TURNS, beast);
	}
	/* (his hand his side's as the battle sets it up, empty, until his own Custom screen's picks: arena_team_custom.h) */
	for (unsigned i = 0; i < HAND_SIZE; ++i) hook_write8(hand(p) + i, hook_read8(BN6_BATTLE_HAND + side * HAND_SIZE + i));
	/* (his own loadout's HP and buster, as the Arena carries a lead's: BN6 copies a side's battle NaviStats from his menu's) */
	if (own[p].given && !compiled[p].given) {
		if (own[p].max_hp) {
			hook_write16(to + BN6_NS_BASE_HP, own[p].base_hp);
			hook_write16(to + BN6_NS_CUR_HP, own[p].max_hp);
			hook_write16(to + BN6_NS_MAX_HP, own[p].max_hp);
		}
		hook_write8(to + BN6_NS_ATTACK, own[p].attack);
		hook_write8(to + BN6_NS_SPEED, own[p].speed);
		hook_write8(to + BN6_NS_CHARGE, own[p].charge);
	}
	if (hp > 0)
		for (uint32_t f = BN6_NS_BASE_HP; f <= BN6_NS_MAX_HP; f += 2) hook_write16(to + f, (uint16_t)hp);
}

void arena_team_navi_begin(void) {
	int s = hook_slot();
	if (s < 0 || s >= HOOK_CORES) return;
	kept[s] = (Kept){ 0 };
	/* (the rooms: the fourth kind's last slots taken in BN6's own pool as the battle has just set it up, never run or freed
	 * since nothing links them into the run order; BN6 clears them with the pool as the next battle sets up) */
	unsigned n = team_partners();
	uint32_t active = hook_read32(BN6_T4_ACTIVE), bits = rooms_bits(n);
	if (active & bits) {
		printf("arena2v2: the partners' rooms in the fourth kind's pool are taken (%08X): their NaviStats their sides'\n", (unsigned)active);
		return;
	}
	hook_write32(BN6_T4_ACTIVE, active | bits);
	kept[s].taken = 1;
	/* (teamhp=N: each partner's own max HP, as his own loadout would give it; -N the second GBA's alone, a mutation that must
	 * part them; the Arena's flag alone) */
	int hp = team_shape_is_party() ? 0 : dev.team_hp < 0 && s == second_slot ? -dev.team_hp : dev.team_hp;
	for (unsigned p = 0; p < n; ++p) record_made(p, s, hp);
}

void arena_team_navi_loadout(unsigned p, uint16_t max_hp, uint16_t base_hp, const uint8_t buster[3]) {
	if (p >= PARTNERS) return;
	own[p].given = buster != NULL;
	if (!buster) return;
	own[p].max_hp = max_hp;
	own[p].base_hp = base_hp;
	own[p].attack = buster[0];
	own[p].speed = buster[1];
	own[p].charge = buster[2];
}

void arena_team_navi_compiled(unsigned p, const uint8_t *stats) {
	if (p >= PARTNERS) return;
	compiled[p].given = stats != NULL;
	if (stats) memcpy(compiled[p].stats, stats, sizeof compiled[p].stats);
}

bool arena_team_navi_record(struct mCore *c, unsigned p, uint8_t *out) {
	unsigned n = team_partners_of(hook_slot_of(c));
	uint32_t bits = rooms_bits(n);
	if (!c || p >= n || ((uint32_t)c->rawRead32(c, BN6_T4_ACTIVE, -1) & bits) != bits) return false;
	for (uint32_t i = 0; i < BN6_BATTLE_NAVI_SIZE; ++i) out[i] = (uint8_t)c->rawRead8(c, room_of(n, p) + i, -1);
	return true;
}

uint32_t arena_team_navi_record_at(unsigned p) {
	int s = hook_slot();
	return s >= 0 && s < HOOK_CORES && kept[s].taken && p < team_partners() ? room(p) : 0;
}

uint32_t arena_team_navi_hand(unsigned p) {
	int s = hook_slot();
	return s >= 0 && s < HOOK_CORES && kept[s].taken && p < team_partners() ? hand(p) : 0;
}

uint8_t arena_team_navi_at(unsigned side, bool partner) {
	return hook_read8(partner ? hand(side & 1) + BN6_HAND_AT : BN6_BATTLE_HAND + (side & 1) * HAND_SIZE + BN6_HAND_AT);
}

uint32_t arena_team_navi_digest(uint32_t h) {
	for (unsigned p = 0, n = team_partners(); p < n; ++p) {
		for (uint32_t f = BN6_NS_BASE_HP; f <= BN6_NS_MAX_HP; f += 2) h = (h ^ hook_read16(room(p) + f)) * 16777619u;
		h = (h ^ hook_read8(hand(p) + BN6_HAND_AT)) * 16777619u;
		h = (h ^ hook_read8(room(p) + BN6_NAVI_FORM)) * 16777619u;
		for (unsigned i = 0; i < BN6_HAND_MAX; ++i) h = (h ^ hook_read16(hand(p) + BN6_HAND_CHIPS + 2 * i)) * 16777619u;
	}
	return h;
}

void arena_team_navi_report(void) {
	for (unsigned side = 0; side < TEAM_ARENA; ++side)
		printf("arena2v2: side %u's partner's own NaviStats answered %u times for him, %u for his attacks (the third kind), %u for "
			"his actors and %u for his effects\n", side, (unsigned)answered[side][0], (unsigned)answered[side][2],
			(unsigned)answered[side][1], (unsigned)answered[side][3]);
	for (unsigned side = 0; side < TEAM_ARENA; ++side)
		printf("arena2v2: side %u's partner's own hand answered %u times for him, %u for his attacks, %u for his actors and %u for "
			"his effects\n", side, (unsigned)hands[side][0], (unsigned)hands[side][2], (unsigned)hands[side][1],
			(unsigned)hands[side][3]);
	printf("arena2v2: %u objects marked a partner's as they were made\n", (unsigned)marked);
}

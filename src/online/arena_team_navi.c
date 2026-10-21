/* --dev arena2v2: a partner's own battle NaviStats (arena_team_navi.h). */
#include "arena_team_navi.h"

#include <stdio.h>

#include "bn6.h"
#include "devtools.h"
#include "free_space.h"
#include "hook.h"

#define KINDS 3   /* the objects a spawn makes: the first kind's (actors), the third's (attacks), the fourth's (effects) */

/* what the hooks keep a core between calls (a rollback puts it back,
 * hook_keep_slots): whether the object spawning now is the partner's, and
 * for every object slot of each kind whether it is */
typedef struct {
	uint8_t pending;
	uint8_t mark[KINDS][32];
} Kept;
static Kept kept[HOOK_CORES];
static int second_slot = -1;   /* the second GBA's core's slot: teamhp's mutation (negative) there alone */
/* the record's answers by the asker (his own object, one of each kind he spawned) and the objects marked his, every core's,
 * a rollback's frames run again among them: what the output says */
static uint32_t answered[1 + KINDS], marked;

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

/* side 0's partner (0), or an object he spawned, or one they did in turn
 * (1 + its kind); -1 neither */
static int whose(uint32_t obj) {
	int s = hook_slot();
	if (!obj || s < 0 || s >= HOOK_CORES) return -1;
	if (obj == hook_read32(BN6_BATTLE_PLAYERS + BN6_SIDE_PARTNER)) return 0;
	unsigned kind, slot;
	return slot_of(obj, &kind, &slot) && kept[s].mark[kind][slot] ? 1 + (int)kind : -1;
}
static bool his(uint32_t obj) { return whose(obj) >= 0; }

/* every spawn: whether the object spawning is his */
static HookAct spawning(HookRegs *r, void *user) {
	(void)user;
	int s = hook_slot();
	if (s >= 0 && s < HOOK_CORES) kept[s].pending = his(r->r[5]);
	return HOOK_CONTINUE;
}

/* ... and the object made marked so */
static HookAct made(HookRegs *r, void *user) {
	(void)user;
	int s = hook_slot();
	unsigned kind, slot;
	if (s >= 0 && s < HOOK_CORES && slot_of(r->r[5], &kind, &slot)) {
		kept[s].mark[kind][slot] = kept[s].pending;
		marked += kept[s].pending;
	}
	return HOOK_CONTINUE;
}

/* a battle NaviStats asked for by object: his own record for his */
static HookAct stats(HookRegs *r, void *user) {
	(void)user;
	uint32_t from = r->lr & ~1u;
	int who = from < BN6_NAVI_BY_OBJECT || from >= BN6_NAVI_BY_OBJECT_END || r->r[0] != 0 ? -1 : whose(r->r[5]);
	if (who < 0) return HOOK_CONTINUE;
	++answered[who];
	r->r[0] = RAM_TEAM_NAVI;
	r->r[1] = BN6_BATTLE_NAVI;   /* (as the routine leaves it) */
	return HOOK_RETURN;
}

bool arena_team_navi_hooks(struct HookCore *core, unsigned side, int slot) {
	if (side == 1) second_slot = slot;
	if (!hook_keep_slots(kept, sizeof *kept)) return false;
	return hook_on(core, BN6_SPAWN_COMMON, spawning, NULL) && hook_on(core, BN6_SPAWN_MADE, made, NULL) &&
		hook_on(core, BN6_NAVI_STATS_ADDR, stats, NULL);
}

void arena_team_navi_begin(void) {
	int s = hook_slot();
	if (s >= 0 && s < HOOK_CORES) kept[s] = (Kept){ 0 };
	for (uint32_t i = 0; i < BN6_BATTLE_NAVI_SIZE; i += 4) hook_write32(RAM_TEAM_NAVI + i, hook_read32(BN6_BATTLE_NAVI + i));
	/* (teamhp=N: his own max HP, as his own loadout would give it; -N the second GBA's alone, a mutation that must part them) */
	int hp = dev.team_hp < 0 && s == second_slot ? -dev.team_hp : dev.team_hp;
	if (hp > 0)
		for (uint32_t f = BN6_NS_BASE_HP; f <= BN6_NS_MAX_HP; f += 2) hook_write16(RAM_TEAM_NAVI + f, (uint16_t)hp);
}

uint32_t arena_team_navi_digest(uint32_t h) {
	for (uint32_t f = BN6_NS_BASE_HP; f <= BN6_NS_MAX_HP; f += 2) h = (h ^ hook_read16(RAM_TEAM_NAVI + f)) * 16777619u;
	return h;
}

void arena_team_navi_report(void) {
	printf("arena2v2: side 0's partner's own NaviStats answered %u times for him, %u for his attacks (the third kind), %u for "
		"his actors and %u for his effects; %u objects marked his as they were made\n", (unsigned)answered[0], (unsigned)answered[2],
		(unsigned)answered[1], (unsigned)answered[3], (unsigned)marked);
}

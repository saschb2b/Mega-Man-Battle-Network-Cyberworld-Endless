/* --dev arena2v2: a partner's own Cross (arena_team_cross.h). */
#include "arena_team_cross.h"

#include <stdio.h>

#include "arena_team_custom.h"
#include "bn6.h"
#include "devtools.h"
#include "hook.h"

#define SIDES 2
#define NONE  0xFFu   /* a pending record's form: none */

static int second_slot = -1;   /* the second GBA's core's slot: teamcross=-N's mutation there alone */
/* every core's, a rollback's frames run again among them: what the output says */
static unsigned forced, asked[SIDES], waited, answered[SIDES];

/* side's partner's object where he stands on the field, else 0 */
static uint32_t partner(unsigned side) {
	uint32_t o = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER);
	return o >= BN6_EWRAM && o < BN6_EWRAM_END && hook_read16(o + BN6_T1_HP) ? o : 0;
}

/* the form side's partner chose this turn, 0 none (teamcross=-N: none on the second GBA, a mutation that must part them) */
static uint8_t chosen(unsigned side) {
	if (dev.team_cross < 0 && hook_slot() == second_slot) return 0;
	return arena_team_custom_cross(side);
}

/* (BN6_TURN_FORMS_SET) the controller sent to its crossing where a partner on the field has a Cross, as where a record has */
static HookAct turn(HookRegs *r, void *user) {
	(void)user;
	if (r->r[4]) return HOOK_CONTINUE;
	for (unsigned side = 0; side < SIDES; ++side)
		if (chosen(side) && partner(side)) {
			r->r[4] = 1;
			++forced;
			break;
		}
	return HOOK_CONTINUE;
}

/* (BN6_CROSS_ACTIVATE, r5 the controller) its first frame: each partner with a Cross asked to cross as BN6 asks the
 * records' objects; after it, BN6's wait held while a partner still crosses */
static HookAct activate(HookRegs *r, void *user) {
	(void)user;
	bool first = hook_read8(r->r[5] + BN6_CROSS_STEP) == 0;
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t o = partner(side), ai = o ? hook_read32(o + BN6_T1_AI_DATA) : 0;
		if (!ai) continue;
		if (first && chosen(side)) {
			hook_write32(ai + BN6_AI_FLAGS, hook_read32(ai + BN6_AI_FLAGS) | BN6_AI_CROSS_ASK);
			++asked[side];
		} else if (!first && (hook_read32(ai + BN6_AI_MODES) & BN6_AI_CROSSING)) {
			++waited;
			/* (back to the caller, a Thumb mov lr,pc; bx r1 that left lr's Thumb bit clear: its pop {pc} as the routine's own) */
			r->r[12] = r->lr | 1u;
			return HOOK_JUMP;
		}
	}
	return HOOK_CONTINUE;
}

/* (BN6_CROSS_PENDING_GET, r0 a side, r5 the crossing object) a partner's own pending form for his own crossing */
static HookAct pending(HookRegs *r, void *user) {
	(void)user;
	unsigned side = r->r[0];
	if (side >= SIDES || !r->r[5] || r->r[5] != partner(side)) return HOOK_CONTINUE;
	uint8_t form = chosen(side);
	++answered[side];
	r->r[0] = form ? form : NONE;
	r->r[1] = 0;   /* (+1: 0 for a Cross, BN6_RECORD_CHAOS) */
	r->r[3] = 0;   /* (+3: a Cross's turns, which no code of Gregar's reads) */
	return HOOK_RETURN;
}

bool arena_team_cross_hooks(struct HookCore *core, unsigned side, int slot) {
	if (side == 1) second_slot = slot;
	return hook_on(core, BN6_TURN_FORMS_SET, turn, NULL) && hook_on(core, BN6_CROSS_ACTIVATE, activate, NULL) &&
		hook_on(core, BN6_CROSS_PENDING_GET, pending, NULL);
}

void arena_team_cross_report(void) {
	printf("arena2v2: the partners' Crosses: the turn's crossing begun for a partner alone %u times, side 0's partner asked to "
		"cross %u times and side 1's %u, BN6's wait held for a partner still crossing %u frames, their own pending forms "
		"answered %u and %u times, every core's\n", forced, asked[0], asked[1], waited, answered[0], answered[1]);
}

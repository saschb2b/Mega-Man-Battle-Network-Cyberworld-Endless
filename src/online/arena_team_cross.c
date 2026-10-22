/* --dev arena2v2: a partner's own Cross (arena_team_cross.h). */
#include "arena_team_cross.h"

#include <stdint.h>
#include <stdio.h>

#include "arena_team_custom.h"
#include "arena_team_navi.h"
#include "bn6.h"
#include "devtools.h"
#include "free_space.h"
#include "hook.h"

#define SIDES 2
#define NONE  0xFFu   /* a pending record's form: none */

static int second_slot = -1;   /* the second GBA's core's slot: teamcross=-N's and teambeast=-1's mutation there alone */
/* every core's, a rollback's frames run again among them: what the output says */
static unsigned forced, asked[SIDES], waited, answered[SIDES], counted[SIDES], beast_turns[SIDES], checked[SIDES], ended[SIDES];
static unsigned left_waited;

/* what the turn check's hook keeps a core between its calls (BN6's routines called from it come back to it): where its run
 * for the partners stands, and the registers BN6's own check goes on with; never past the frame, so a rollback finds it idle */
typedef struct {
	uint8_t step, side;
	uint32_t r4, r5;
} Kept;
static Kept kept[HOOK_CORES];
enum { CHECK_IDLE, CHECK_ENDED, CHECK_LEFT };

/* side's partner's object where he stands on the field, else 0 */
static uint32_t partner(unsigned side) {
	uint32_t o = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + BN6_SIDE_PARTNER);
	return o >= BN6_EWRAM && o < BN6_EWRAM_END && hook_read16(o + BN6_T1_HP) ? o : 0;
}

/* the side whose player `o` is, SIDES none */
static unsigned player_side(uint32_t o) {
	for (unsigned side = 0; o && side < SIDES; ++side)
		if (o == hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP)) return side;
	return SIDES;
}

/* the form side's partner chose this turn, a Cross or Beast Out, 0 none (teamcross=-N, teambeast=-1: none on the second GBA, a
 * mutation that must part them) */
static uint8_t chosen(unsigned side) {
	if ((dev.team_cross < 0 || dev.team_beast < 0) && hook_slot() == second_slot) return 0;
	return arena_team_custom_cross(side);
}

/* BN6's routine run for an object from the turn check's hook, back to the hook's own instruction */
static HookAct call_back(HookRegs *r, uint32_t routine, uint32_t object) {
	r->r[0] = object;
	r->lr = BN6_TURN_FORMS_SET | 1u;
	r->r[12] = routine | 1u;
	return HOOK_JUMP;
}

/* teambeast: each MegaMan's form and BeastOutCounter as a turn's check meets them, said in the output (the first GBA's, a
 * rollback's frames run again among them) */
static void say_turn(void) {
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t o = partner(side), ns = arena_team_navi_record_at(side), lead = BN6_BATTLE_NAVI + side * BN6_BATTLE_NAVI_SIZE;
		uint8_t record = hook_read8(BN6_CROSS_SIDES + BN6_CROSS_SIDE_RECORD + side * BN6_CROSS_PENDING_SIZE);
		if (!o || !ns) continue;
		printf("arena2v2: the turn's check at iteration %u: side %u's lead in form %u, his BeastOutCounter %u, his form chosen %u | "
			"his partner in form %u, his BeastOutCounter %u, his form chosen %u\n", (unsigned)hook_read32(RAM_TEAM_TICK), side,
			hook_read8(lead + BN6_NAVI_FORM), hook_read8(lead + BN6_NS_BEAST_TURNS), record, hook_read8(ns + BN6_NAVI_FORM),
			hook_read8(ns + BN6_NS_BEAST_TURNS), chosen(side));
	}
}

/* (BN6_TURN_FORMS_SET, the turn's form check's end) each partner without a form this turn asked whether his Beast Out has
 * ended and, where it has, asked to leave it, as BN6 asks a record's (BN6_FORM_END_CHECK, BN6_FORM_LEAVE_ASK, BN6's own code
 * for his object); then the controller sent to its crossing where a partner on the field has a form, as where a record has */
static HookAct turn(HookRegs *r, void *user) {
	(void)user;
	int s = hook_slot();
	if (s < 0 || s >= HOOK_CORES) return HOOK_CONTINUE;
	Kept *k = &kept[s];
	if (k->step == CHECK_IDLE) {
		k->side = 0;
		k->r4 = r->r[4];
		k->r5 = r->r[5];
		if (dev.team_beast && s != second_slot) say_turn();
	} else if (k->step == CHECK_ENDED && r->r[0]) {
		k->step = CHECK_LEFT;
		++ended[k->side];
		return call_back(r, BN6_FORM_LEAVE_ASK, partner(k->side));
	} else ++k->side;
	for (; k->side < SIDES; ++k->side) {
		uint32_t p = partner(k->side);
		if (!p || chosen(k->side)) continue;
		k->step = CHECK_ENDED;
		++checked[k->side];
		return call_back(r, BN6_FORM_END_CHECK, p);
	}
	k->step = CHECK_IDLE;
	r->r[4] = k->r4;
	r->r[5] = k->r5;
	if (r->r[4]) return HOOK_CONTINUE;
	for (unsigned side = 0; side < SIDES; ++side)
		if (chosen(side) && partner(side)) {
			r->r[4] = 1;
			++forced;
			break;
		}
	return HOOK_CONTINUE;
}

/* (BN6_FORM_TURN_DOWN_END, BN6_BEAST_TURN_END, r5 the object done) a side's player's turn counted as BN6 counts it: his
 * partner's too, BN6's own body run again for his object, which comes back here with r5 his */
static HookAct turned(HookRegs *r, void *user) {
	uint32_t body = (uint32_t)(uintptr_t)user;
	unsigned side = player_side(r->r[5]);
	uint32_t p = side < SIDES ? partner(side) : 0;
	if (!p) return HOOK_CONTINUE;
	++(body == BN6_BEAST_TURN_BODY ? beast_turns : counted)[side];
	r->r[5] = p;
	if (body == BN6_BEAST_TURN_BODY) r->r[7] = 0;   /* (BN6_BEAST_TURN's answer, 0 throughout; BN6_FORM_TURN_DOWN keeps no r7) */
	r->r[12] = body | 1u;
	return HOOK_JUMP;
}

/* (BN6_CROSS_RESTORE, the controller's mode 8) BN6's wait held while a partner leaves his form, as for a record's object */
static HookAct restore(HookRegs *r, void *user) {
	(void)user;
	/* (his pending form taken, as the crossing's step takes the records' once their objects have crossed: the next turn's
	 * checks meet him as they meet a player who chose none) */
	for (unsigned side = 0; side < SIDES; ++side) arena_team_custom_cross_taken(side);
	for (unsigned side = 0; side < SIDES; ++side) {
		uint32_t o = partner(side), ai = o ? hook_read32(o + BN6_T1_AI_DATA) : 0;
		if (ai && ((hook_read32(ai + BN6_AI_MODES) & BN6_AI_LEAVING) || (hook_read32(ai + BN6_AI_FLAGS) & BN6_AI_LEAVE_ASK))) {
			++left_waited;
			r->r[12] = r->lr | 1u;   /* (a mov lr,pc; bx r1 caller, as BN6_CROSS_ACTIVATE's) */
			return HOOK_JUMP;
		}
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
	if (!hook_keep_slots(kept, sizeof *kept)) return false;
	return hook_on(core, BN6_TURN_FORMS_SET, turn, NULL) && hook_on(core, BN6_CROSS_ACTIVATE, activate, NULL) &&
		hook_on(core, BN6_CROSS_PENDING_GET, pending, NULL) && hook_on(core, BN6_CROSS_RESTORE, restore, NULL) &&
		hook_on(core, BN6_FORM_TURN_DOWN_END, turned, (void *)(uintptr_t)BN6_FORM_TURN_DOWN_BODY) &&
		hook_on(core, BN6_BEAST_TURN_END, turned, (void *)(uintptr_t)BN6_BEAST_TURN_BODY);
}

void arena_team_cross_report(void) {
	printf("arena2v2: the partners' Crosses: the turn's crossing begun for a partner alone %u times, side 0's partner asked to "
		"cross %u times and side 1's %u, BN6's wait held for a partner still crossing %u frames, their own pending forms "
		"answered %u and %u times, every core's\n", forced, asked[0], asked[1], waited, answered[0], answered[1]);
	printf("arena2v2: the partners' turns as BN6 counts a player's: counted down %u and %u times, their Beast Out's turns %u and "
		"%u, their form's end asked %u and %u times and found %u and %u, BN6's wait held for a partner leaving his form %u "
		"frames, every core's\n", counted[0], counted[1], beast_turns[0], beast_turns[1], checked[0], checked[1], ended[0],
		ended[1], left_waited);
}

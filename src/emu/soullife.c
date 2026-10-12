/* A Soul's life on the proxy form (soullife.h). */
#include "soullife.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "emu.h"
#include "soulmood.h"

#define THUMB_BX_LR 0x4770u   /* a stub's instruction, which its hook stands on */
#define FREE_HALF   0xFFFFu   /* the free space as emu_init pads it */
#define BL_HI(op)   (((op) & 0xF800u) == 0xF000u)

uint32_t soul_field(int side, uint32_t field) { return BN6_SOULS + (uint32_t)side * BN6_SOULS_SIDE + field; }

/* (from a hook) the side of the battle object at obj */
static int soul_side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }

bool soul_united(int side) { return hook_read8(soul_field(side, BN6_SOUL_ID)) != 0; }

static uint8_t navi_byte(int side, uint32_t field) {
	return hook_read8(BN6_BATTLE_NAVI + (uint32_t)side * BN6_BATTLE_NAVI_SIZE + field);
}

/* the proxy's own overlay routines (its entries 8 as soulproxy.c left them: HeatCross's) */
static uint32_t enter_to, leave_to;
/* a Soul's own way in and out (soullife_routes), and whether it is set */
static uint32_t soul_enter[SOULS_BN5_FORMS], soul_leave[SOULS_BN5_FORMS];
static bool routed;

void soullife_routes(const uint32_t enter[SOULS_BN5_FORMS], const uint32_t leave[SOULS_BN5_FORMS]) {
	memcpy(soul_enter, enter, sizeof soul_enter);
	memcpy(soul_leave, leave, sizeof soul_leave);
	routed = true;
}

/* on to `to` (a routine of BN6's, or a Soul's), or where it is none BN6's routine of no overlay (form 0's entry, a mov
 * pc,lr: the tables' callers come by mov lr,pc; bx, and a bx lr would leave Thumb) */
static HookAct go(HookRegs *r, uint32_t to) {
	r->r[12] = (to ? to : BN6_FORM_OVERLAY_NONE) | 1;
	return HOOK_JUMP;
}
static bool keep_soul;   /* (SOULUNITE_BREAK_LEAVE) */

/* (r5 MegaMan's object, entering form 8: BN5 takes the Soul pending as the
 * form byte is written, 0x08012088; here the overlay's spawn, just after
 * BN6 writes it, 0x08014C5C) the Soul committed, its chaos and turns */
static HookAct entered(HookRegs *r, void *user) {
	(void)user;
	int side = soul_side_of(r->r[5]);
	uint8_t next = hook_read8(soul_field(side, BN6_SOUL_NEXT));
	if (next) {
		hook_write8(soul_field(side, BN6_SOUL_ID), next);
		hook_write8(soul_field(side, BN6_SOUL_CHAOS), hook_read8(soul_field(side, BN6_SOUL_NEXT_CHAOS)));
		hook_write8(soul_field(side, BN6_SOUL_TURNS), hook_read8(soul_field(side, BN6_SOUL_NEXT_TURNS)));
		hook_write8(soul_field(side, BN6_SOUL_NEXT), 0);
		hook_write8(soul_field(side, BN6_SOUL_NEXT_CHAOS), 0);
		hook_write8(soul_field(side, BN6_SOUL_NEXT_TURNS), 0);
		soulmood_unison(side);   /* (BN5's calm as he unites, 0x08012146) */
		/* (no Cross's overlay: what BN5's Soul spawns as he enters it, NumberSoul's) */
		if (routed && next < SOULS_BN5_FORMS) return go(r, soul_enter[next]);
	}
	return go(r, enter_to);
}

/* (r5 MegaMan's object, leaving form 8: as BN5's form goes to 0, 0x0801226A)
 * the Soul gone: its id, chaos, turns and what later milestones keep */
static HookAct left(HookRegs *r, void *user) {
	(void)user;
	int side = soul_side_of(r->r[5]);
	uint8_t soul = hook_read8(soul_field(side, BN6_SOUL_ID));
	if (soul) soulmood_unison(side);   /* (BN5's calm as the Soul ends, 0x08012272) */
	if (soul && !keep_soul) {
		static const uint32_t fields[] = { BN6_SOUL_ID, BN6_SOUL_CHAOS, BN6_SOUL_TURNS, BN6_SOUL_PHASE, BN6_SOUL_LEVEL, BN6_SOUL_SCREEN };
		for (size_t i = 0; i < sizeof fields / sizeof *fields; ++i) hook_write8(soul_field(side, fields[i]), 0);
		hook_write16(soul_field(side, BN6_SOUL_CHIP), 0);
	}
	/* (what BN5's Soul takes away as he leaves it, NumberSoul's) */
	if (soul && routed && soul < SOULS_BN5_FORMS) return go(r, soul_leave[soul]);
	return go(r, leave_to);
}

/* (r0 a side's player object, as BN6 counts its own down for it: the
 * Custom screen just opened) BN5's countdown (0x08012484): its Navi
 * MegaMan, its turns neither 0 nor 0xFF */
static HookAct counted(HookRegs *r, void *user) {
	(void)user;
	if (!r->r[0]) return HOOK_CONTINUE;
	int side = soul_side_of(r->r[0]);
	uint32_t turns = soul_field(side, BN6_SOUL_TURNS);
	uint8_t t = hook_read8(turns);
	if (soul_united(side) && !navi_byte(side, BN6_NAVI_SIDE_NAVI) && t && t != 0xFF) hook_write8(turns, (uint8_t)(t - 1));
	return HOOK_CONTINUE;
}

/* (r0 a side's player object, as a turn starts with no form pending; user
 * its side's way to the end) BN5's end (0x08012464): a Soul with no turns
 * left ends, through BN6's request to leave the form (0x08015994) in place
 * of Beast Out's end check. BN6 runs this check as a turn's change begins,
 * before its Custom screen (0x0800840C, for Beast Out), and again as the
 * turn starts after it (0x080084A2); BN5 only after it in a battle not
 * linked (0x08007D0C; its run before, 0x0800773C, skips there through
 * 0x0802D510): a Soul's end waits for a Custom screen to have opened
 * (BN6_SOUL_SCREEN, which soulunite.c's stand sets), so the last screen
 * opens with the Soul still on, as BN5's does. */
static HookAct ending(HookRegs *r, void *user) {
	if (!r->r[0]) return HOOK_CONTINUE;
	int side = soul_side_of(r->r[0]);
	uint32_t screen = soul_field(side, BN6_SOUL_SCREEN);
	if (!soul_united(side)) return HOOK_CONTINUE;
	bool shown = hook_read8(screen) != 0;
	if (shown) hook_write8(screen, 0);
	if (!shown || hook_read8(soul_field(side, BN6_SOUL_TURNS)) || navi_byte(side, BN6_NAVI_FORM) != BN6_FORM_PROXY) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* (r5 the object a weakness hit asks to break) no break for a Soul: BN5's
 * never break on an element hit (it asks only of a DarkChip used united) */
static HookAct breaking(HookRegs *r, void *user) {
	(void)user;
	if (!soul_united(soul_side_of(r->r[5]))) return HOOK_CONTINUE;
	r->r[12] = BN6_FORM_BREAK_NOT | 1;
	return HOOK_JUMP;
}

static bool code_is(uint32_t at, uint16_t want) { return emu_read16(at) == want; }

/* every byte a Soul's life stands on as Gregar has it, the proxy's entries (enter_to, leave_to) as soulproxy.c wrote
 * them (HeatCross's, Gregar's own routines), its two stubs' place free */
static bool expected(uint32_t stubs) {
	bool entries = enter_to == emu_read32(BN6_FORM_OVERLAYS + 4u * BN6_FORM_HEAT_CROSS) &&
		leave_to == emu_read32(BN6_FORM_OVERLAYS_END + 4u * BN6_FORM_HEAT_CROSS) && (enter_to & 1) && (leave_to & 1) &&
		enter_to < EMU_FREE && leave_to < EMU_FREE && emu_read32(BN6_FORM_OVERLAYS) == (BN6_FORM_OVERLAY_NONE | 1);
	bool code = BL_HI(emu_read16(BN6_TURN_COUNT)) && BL_HI(emu_read16(BN6_TURN_COUNT_1)) && BL_HI(emu_read16(BN6_TURN_END)) &&
		BL_HI(emu_read16(BN6_TURN_END_1)) && code_is(BN6_TURN_END_GO, 0x68B0) && code_is(BN6_TURN_END_GO_1, 0x68B8) &&
		code_is(BN6_FORM_BREAK, 0x8D28) && code_is(BN6_FORM_OVERLAY_NONE, 0x46F7);
	return entries && code && code_is(stubs, FREE_HALF) && code_is(stubs + 4, FREE_HALF);
}

bool soullife_install(uint32_t stubs, bool keep) {
	static bool done;
	if (done) return true;
	keep_soul = keep;
	uint32_t enter_at = BN6_FORM_OVERLAYS + 4u * BN6_FORM_PROXY, leave_at = BN6_FORM_OVERLAYS_END + 4u * BN6_FORM_PROXY;
	enter_to = emu_read32(enter_at);
	leave_to = emu_read32(leave_at);
	if (!expected(stubs)) {
		fprintf(stderr, "soullife: the proxy's entries or the code a Soul's life stands on are not as expected: none of it placed\n");
		return false;
	}
	done = true;
	const uint8_t bx[2] = { THUMB_BX_LR & 0xFF, THUMB_BX_LR >> 8 };
	emu_write(stubs, bx, 2);
	emu_write(stubs + 4, bx, 2);
	/* (the entries pointed at the stubs only once both are hooked: an unhooked stub would return before the proxy's own routine) */
	bool stubbed = emu_hook(stubs, entered, NULL) && emu_hook(stubs + 4, left, NULL);
	if (stubbed) {
		emu_write32(enter_at, stubs | 1);
		emu_write32(leave_at, (stubs + 4) | 1);
	}
	bool hooked = stubbed && emu_hook(BN6_TURN_COUNT, counted, NULL) &&
		emu_hook(BN6_TURN_COUNT_1, counted, NULL) && emu_hook(BN6_TURN_END, ending, (void *)(uintptr_t)BN6_TURN_END_GO) &&
		emu_hook(BN6_TURN_END_1, ending, (void *)(uintptr_t)BN6_TURN_END_GO_1) && emu_hook(BN6_FORM_BREAK, breaking, NULL);
	if (!hooked) fprintf(stderr, "soullife: a hook of a Soul's life could not be placed\n");
	return hooked;
}

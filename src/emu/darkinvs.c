/* BN5's DarkInvs in BN6's engine: the player's chip AI (darkinvs.h). */
#include "darkinvs.h"

#include <stdio.h>

#include "bn5_chips.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "darkinvs_room.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"
#include "soulchaos.h"
#include "soullife.h"
#include "soulmood.h"
#include "soulunite.h"
#include "xchips_code.h"
#include "xnavi_bn5.h"
#include "xchips_moved.h"
#include "xnavi_install.h"
#include "xnavi_room.h"
#include "xvirus.h"

#define POOL_AT (RAM_PLAYER_AI + BN5_AI_DATA_SIZE)   /* (his own AI data's place in BN6's pool while the block holds it) */
#define ENTER_AT 120                                 /* (--dev chipai: two seconds into a battle) */
#define DEV_FRAMES 600                               /* (... for DarkInvs's 600 frames) */

static bool dev_on;
static bool two_a_side;   /* (--dev arena2v2: darkinvs_two_a_side) */
static int room_break = -1;   /* (chipaibreak=N: darkinvs_room_break) */
static uint32_t last_timer;
static bool entered;

/* where BN5's address `a` is moved to in the Dark Soul's code (its Thumb bit kept), 0 where it is not there */
static uint32_t dark_code(uint32_t a) {
	if (xnavi_bn6_id(BN5_DARK_SOUL_ID) < 0) return 0;
	for (int i = 0; i < xnavi_bn5_count; ++i)
		if (xnavi_bn5[i]->code.src_ai == BN5_NAVI_DARK_SOUL) return xvirus_code_at(&xnavi_bn5[i]->code, a);
	return 0;
}

static uint32_t machine(void) { return dark_code(BN5_CHIP_AI_PLAYER | 1u); }

/* (the Navis' copy of BN5's buster arm at its ldrb r1,[r7,#0x12], BN5_ARM_CHAOS_READ: MegaMan in a form, r5 him) his Chaos
 * Unison as BN5's AI data holds it, which BN6's holds in the Souls' block (BN6_SOUL_CHAOS, as soulcode_bn5.c's fields answer
 * the Souls' own reads of it): the arm's flash BN5's chaos one (BN5_FLASH_CHAOS) where the machine fires the Navis' buster
 * in a Chaos Unison (chaos_shadow_45) */
static HookAct arm_chaos(HookRegs *r, void *user) {
	/* (a side's partner never in his side's Chaos Unison: soul_partner) */
	bool own = soulunite_ready() && !soul_partner(r->r[5]);
	r->r[1] = own ? hook_read8(soul_field(hook_read8(r->r[5] + BN6_T1_ALLIANCE) & 1, BN6_SOUL_CHAOS)) : 0;
	r->r[12] = (uint32_t)(uintptr_t)user | 1u;
	return HOOK_JUMP;
}

/* another MegaMan in play holds the block already: --dev arena2v2's two a side, the block side 0's first's (his set-up comes
 * before his partner's), whose AI data BN6's key copy (BN6_KEY_COPY) writes */
static bool block_taken(uint32_t me) {
	for (uint32_t o = BN6_T1_OBJECTS; o < BN6_T1_OBJECTS + BN6_T1_COUNT * BN6_T1_SIZE; o += BN6_T1_SIZE)
		if (o != me && (hook_read8(o + BN6_T1_IN_PLAY) & 1) && hook_read32(o + BN6_T1_AI_DATA) == RAM_PLAYER_AI) return true;
	return false;
}

/* `me`'s seat in a NetBattle (darkinvs_room.h: side x 2, + 1 the partner), -1 where he is none of BN6_BATTLE_PLAYERS' */
static int seat_of(uint32_t me) {
	for (uint32_t seat = 0; me && seat < DARKINVS_SEATS; ++seat)
		if (hook_read32(BN6_BATTLE_PLAYERS + (seat >> 1) * BN6_BATTLE_PLAYER_STEP + (seat & 1) * BN6_SIDE_PARTNER) == me) return (int)seat;
	return -1;
}

/* an AI data of BN5's size: side 0's player's block, or a seat's room */
static bool bn5_sized(uint32_t ai) {
	if (ai == RAM_PLAYER_AI) return true;
	for (unsigned seat = 1; seat < DARKINVS_SEATS; ++seat)
		if (ai == darkinvs_room(seat)) return true;
	return false;
}

/* (issue #182) seat's room taken from BN6's AI data's pool (darkinvs_room.h), 0 where an actor holds an entry of it */
static uint32_t room_take(int seat) {
	uint32_t used, rooms;
	if (hook_slot() == room_break) return 0;
	if (!darkinvs_room_take((unsigned)seat, hook_read32(BN6_AI_POOL_USED), hook_read32(RAM_CHIP_AI_ROOMS), &used, &rooms)) return 0;
	hook_write32(BN6_AI_POOL_USED, used);
	hook_write32(RAM_CHIP_AI_ROOMS, rooms);
	return darkinvs_room((unsigned)seat);
}

/* his AI data moved into BN5's 0x140 bytes (BN6's 0x100: the chip AI keeps its state past them, BN6_DARK_AI's reason): side
 * 0's player (a run's MegaMan) into RAM_PLAYER_AI, his pool's place kept; a NetBattle's other MegaMen each into a room of his
 * own (issue #182: side 1's player moved into side 0's block, both reading one AI data). His entry in the pool stays his (its
 * bit, which the copy's +0x7C names, let go with him); none where no room is free */
static void adopt(uint32_t me) {
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA);
	if (bn5_sized(ai) || ai < BN6_EWRAM || ai >= BN6_EWRAM_END) return;
	int seat = seat_of(me);
	/* (at his set-up BN6_BATTLE_PLAYERS may not name him yet: side 0's MegaMan is the first set up) */
	if (seat < 0 && !(hook_read8(me + BN6_T1_ALLIANCE) & 1)) seat = 0;
	uint32_t to = seat == 0 ? (two_a_side && block_taken(me) ? 0 : RAM_PLAYER_AI) : seat > 0 ? room_take(seat) : 0;
	if (emu_debug_on() && seat > 0)
		fprintf(stderr, "darkinvs: core %d: seat %d's AI data %08x %s %08x\n", hook_slot(), seat, (unsigned)ai,
			to ? "moved into his room" : "kept: an actor holds the pool's entries of his room", (unsigned)darkinvs_room((unsigned)seat));
	if (!to) return;
	for (uint32_t k = 0; k < BN5_AI_DATA_SIZE; k += 4) hook_write32(to + k, k < BN6_AI_DATA_SIZE ? hook_read32(ai + k) : 0);
	if (to == RAM_PLAYER_AI) hook_write32(POOL_AT, ai);
	hook_write32(me + BN6_T1_AI_DATA, to);
}

/* (BN6_PLAYER_INIT_AI, r5 him: his set-up, his AI data laid out, before anything of his has named it) side 0's MegaMan's AI
 * data BN5's size from the start, as BN5's MegaMan's is, so that nothing holds its old place when the chip AI takes him (the
 * others' as the chip AI first takes them: a NetBattle without it is the battle it was) */
static HookAct init(HookRegs *r, void *user) {
	(void)user;
	int seat = seat_of(r->r[5]);
	if (seat == 0 || (seat < 0 && !hook_read8(r->r[5] + BN6_T1_ALLIANCE))) adopt(r->r[5]);
	return HOOK_CONTINUE;
}

/* a MegaMan whose AI data stays BN6's 0x100 bytes (no room free: an actor holds its entries; --dev arena2v2's side 0's partner
 * at his set-up) is left out of BN5's chip AI, whose state past BN6's 0x100 would land in the pool's next entry */
static bool held_short(uint32_t me) { return !bn5_sized(hook_read32(me + BN6_T1_AI_DATA)); }

/* BN5's 0x0802C03E: the chip AI's state cleared */
static void reset(uint32_t me) {
	adopt(me);
	if (held_short(me)) return;
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA);
	for (uint32_t k = 0; k < 0x40; k += 4) hook_write32(ai + BN5_CHIP_AI_STATE + k, 0);
}

void darkinvs_enter(uint32_t me, uint16_t frames) {
	reset(me);
	if (held_short(me)) return;
	/* (BN5's 0x0802C114 with the time: under way, the time, the flag) */
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA);
	hook_write16(ai + BN5_AI_UNDER_TIME, frames);
	hook_write8(ai + BN5_CHIP_AI_STATE, 1);
	hook_write32(ai + BN6_AI_MODES, hook_read32(ai + BN6_AI_MODES) | BN5_AI_UNDER);
	soulchaos_lists_fill(hook_read8(me + BN6_T1_ALLIANCE) & 1);
	if (emu_debug_on()) {
		fprintf(stderr, "darkinvs: core %d: side %u's MegaMan under BN5's chip AI for %u frames; AI data:", hook_slot(),
			(unsigned)(hook_read8(me + BN6_T1_ALLIANCE) & 1), (unsigned)frames);
		for (uint32_t side = 0; side < 2; ++side)
			for (uint32_t k = 0; k < 2; ++k) {
				uint32_t o = hook_read32(BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP + k * BN6_SIDE_PARTNER);
				if (o) fprintf(stderr, " side %u%s %08x", (unsigned)side, k ? "'s partner" : "", (unsigned)hook_read32(o + BN6_T1_AI_DATA));
			}
		fprintf(stderr, "\n");
	}
}

void darkinvs_leave(uint32_t me) {
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA);
	if (!(hook_read32(ai + BN6_AI_MODES) & BN5_AI_UNDER)) return;
	hook_write16(ai + BN5_AI_UNDER_TIME, 0);
	hook_write8(ai + BN5_CHIP_AI_STATE, 0);
	hook_write32(ai + BN6_AI_MODES, hook_read32(ai + BN6_AI_MODES) & ~(uint32_t)BN5_AI_UNDER);
	if (emu_debug_on()) fprintf(stderr, "darkinvs: MegaMan out of BN5's chip AI, a Soul's unison begun or ended\n");
}

/* (--dev chipai) entered once a battle, two seconds in; its time and BN5's end are DarkInvs's (timer, reaction) */
static void dev_step(uint32_t me) {
	uint32_t t = hook_read32(BN6_BATTLE_TIMER);
	if (t < last_timer) entered = false;   /* (a new battle) */
	last_timer = t;
	if (!entered && t >= ENTER_AT) {
		entered = true;
		darkinvs_enter(me, DEV_FRAMES);
	}
}

/* ---- BN5's DarkInvs (tools/xchiprecipe.py's UNIT_STUBS: its calls BN6 has no twin of, at the chips' stubs) ---- */

/* (XC_STUBS + XC_STUB_DARKINVS, r5 him: BN5's 0x0802C03E) */
static HookAct stub_reset(HookRegs *r, void *user) {
	(void)user;
	reset(r->r[5]);
	return HOOK_CONTINUE;
}

/* (... + 4, r5 him, r0 the time: BN5's 0x0802C114) */
static HookAct stub_enter(HookRegs *r, void *user) {
	(void)user;
	darkinvs_enter(r->r[5], (uint16_t)r->r[0]);
	return HOOK_CONTINUE;
}

/* (... + 8, r5 him: BN5's 0x08017060) his statuses cleared, at BN6's places of the collision data (legit.json's layout: BN5's
 * +0x1E-+0x25 BN6's +0x1C-+0x23); then BN5's calm of his mood as BN5 keeps it (the call after it, 0x08011A8C, BN6's twin
 * 0x080143A6 for the rest: soulmood_unison, where he is not dark) */
static HookAct stub_statuses(HookRegs *r, void *user) {
	(void)user;
	uint32_t me = r->r[5], coll = hook_read32(me + BN6_T1_COLLISION);
	hook_write32(coll + BN6_COLL_FLAGS, hook_read32(coll + BN6_COLL_FLAGS) & ~BN5_STATUS_FLAGS);
	hook_write32(coll + BN6_COLL_STRUCK, hook_read32(coll + BN6_COLL_STRUCK) & ~BN5_STATUS_FLAGS2);
	for (uint32_t k = 0; k < 8; k += 2) hook_write16(coll + BN6_COLL_PARALYSIS + k, 0);   /* (paralysis, confusion, blindness, held) */
	soulmood_calm_of(me);
	return HOOK_CONTINUE;
}

/* (BN6_PLAYER_FRAME_END, r5 him: the end of his frame, where BN5's calls 0x0800DBD8) BN5's time under the chip AI: counted
 * down where time runs and the battle is not paused, at 0 his request 0x80000, BN5's end of it (family 0x49) */
static HookAct timer(HookRegs *r, void *user) {
	(void)user;
	uint32_t me = r->r[5], ai = hook_read32(me + BN6_T1_AI_DATA);
	/* (a MegaMan of BN5's size alone: none other is under the chip AI, issue #182's side 1 among them) */
	if (!bn5_sized(ai) || !(hook_read32(ai + BN6_AI_MODES) & BN5_AI_UNDER)) return HOOK_CONTINUE;
	if ((hook_read16(BN6_BATTLE_FLAGS) & BN6_TIME_STOP) || hook_read8(hook_read32(BN6_TOOLKIT + BN6_TOOLKIT_PAUSE) + BN6_PAUSED)) return HOOK_CONTINUE;
	uint16_t t = hook_read16(ai + BN5_AI_UNDER_TIME);
	if (t == 0xFFFF || !t) return HOOK_CONTINUE;
	hook_write16(ai + BN5_AI_UNDER_TIME, --t);
	if (!t) hook_write32(ai + BN6_AI_FLAGS, hook_read32(ai + BN6_AI_FLAGS) | BN5_AI_UNDER_END);
	return HOOK_CONTINUE;
}

/* (BN6_SUPPORT_MARK, r7 the attack's variables) BN5's support family ends its attack in the frame it runs the chip's routine
 * (0x080EC448), BN6's at its next (legit.json's "BN6's support family's extra frame"): for BN5's DarkInvs, whose chip AI
 * takes MegaMan from his next idle frame, BN5's frame, so that the machine runs from the frame BN5's does */
static HookAct support_end(HookRegs *r, void *user) {
	if (hook_read8(r->r[7] + BN6_ATTACK_SUB) != BN6_SUPPORT_DARKINVS) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1u;
	return HOOK_JUMP;
}

/* (BN6_AI_DRAIN_TEST, r5 him) BN6's own use of AIData +0x36 (BN6_AI_DRAIN: while it is not 0, 1 HP a frame lost), where BN5
 * keeps the chip AI's time (BN5_AI_UNDER_TIME), which drains nothing: none while the chip AI has him. (BN6's other readers of
 * it, which hold his mood as it is while it is not 0, BN5's twins do alike.) */
static HookAct drain(HookRegs *r, void *user) {
	if (!(hook_read32(hook_read32(r->r[5] + BN6_T1_AI_DATA) + BN6_AI_MODES) & BN5_AI_UNDER)) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1u;
	return HOOK_JUMP;
}

/* (BN6_REACTION_START, r5 him: his request 0x80000 answered, movs r0,#0x49 before the attack's start) BN5's family 0x49
 * where the chip AI has him (BN5_AI_UNDER_END: its time's end), as BN5's twin starts it (0x080EB0F0): BN5's code, moved as
 * BN6's 0x50 + 0x49 (the chips' Reaction49), which gives him back (the flag and the invisibility cleared); BN6's own family
 * of the number another */
static HookAct reaction(HookRegs *r, void *user) {
	if (!(hook_read32(hook_read32(r->r[5] + BN6_T1_AI_DATA) + BN6_AI_MODES) & BN5_AI_UNDER)) return HOOK_CONTINUE;
	r->r[0] = BN6_REACTION_BN5;
	r->r[12] = (uint32_t)(uintptr_t)user | 1u;
	return HOOK_JUMP;
}

/* (BN6_PLAYER_IDLE_FREE, r5 him) his chip AI where it stands: on to the stub, which runs it and leaves the state */
static HookAct idle(HookRegs *r, void *user) {
	uint32_t me = r->r[5], ai = hook_read32(me + BN6_T1_AI_DATA);
	/* (a MegaMan of BN5's size alone, side 0's player or another's room: issue #182) */
	if (!bn5_sized(ai)) return HOOK_CONTINUE;
	if (dev_on && ai == RAM_PLAYER_AI) dev_step(me);
	ai = hook_read32(me + BN6_T1_AI_DATA);
	if (!(hook_read32(ai + BN6_AI_MODES) & BN5_AI_UNDER)) {
		/* (BN5's idle state clears the chip AI's state each frame it does not run it, 0x080F04D6) */
		for (uint32_t k = 0; k < 0x40; k += 4) hook_write32(ai + BN5_CHIP_AI_STATE + k, 0);
		return HOOK_CONTINUE;
	}
	r->r[12] = (uint32_t)(uintptr_t)user | 1u;
	return HOOK_JUMP;
}

void darkinvs_dev(bool on) { dev_on = on; }
void darkinvs_two_a_side(bool on) { two_a_side = on; }
void darkinvs_room_break(int slot) { room_break = slot; }

bool darkinvs_install(void) {
	uint32_t m = machine();
	if (!m) return false;
	/* (ldr r3, the machine; bl to bx r3; movs r0, #1; pop {r4, r6, r7, pc}: BN5's 0x080F04DC-0x080F04E2) */
	static const uint16_t code[] = { 0x4B02, 0xF000, 0xF802, 0x2001, 0xBDD0, 0x4718, 0, 0 };
	uint8_t b[sizeof code];
	for (size_t i = 0; i < sizeof code / 2; ++i) put16(b + 2 * i, code[i]);
	put32(b + 0x0C, m);
	claims_begin("the player's chip AI (DarkInvs)");
	_Static_assert(sizeof b <= FREE_CHIP_AI_END - FREE_CHIP_AI, "the stub past its room");
	emu_write(FREE_CHIP_AI, b, sizeof b);
	bool ok = emu_hook(BN6_PLAYER_IDLE_FREE, idle, (void *)(uintptr_t)FREE_CHIP_AI) && emu_hook(BN6_PLAYER_FRAME_END, timer, NULL) &&
		emu_hook(BN6_PLAYER_INIT_AI, init, NULL) && emu_hook(BN6_AI_DRAIN_TEST, drain, (void *)(uintptr_t)BN6_AI_DRAIN_DONE) &&
		emu_hook(BN6_SUPPORT_MARK, support_end, (void *)(uintptr_t)BN6_SUPPORT_END) &&
		emu_hook(BN6_REACTION_START, reaction, (void *)(uintptr_t)BN6_REACTION_CALL);
	uint32_t arm = dark_code(BN5_ARM_CHAOS_READ | 1u) & ~1u;
	claims_end();
	/* (the hooks on another track's bytes claimed as that track's, as xchips_flash.c's are: the Navis' copy of the arm, the
	 * chips' stubs) */
	claims_begin(XN_CLAIMS);
	ok = ok && arm && emu_hook(arm, arm_chaos, (void *)(uintptr_t)(arm + 2));
	claims_end();
	claims_begin(XC_CLAIMS);
	/* (BN5's DarkInvs's calls, where its unit is installed: xchips_code.c's stubs) */
	if (ok && xchips_moved_of(BN6_DARKINVS))
		ok = emu_hook(XC_STUBS + XC_STUB_DARKINVS, stub_reset, NULL) && emu_hook(XC_STUBS + XC_STUB_DARKINVS + 4, stub_enter, NULL) &&
			emu_hook(XC_STUBS + XC_STUB_DARKINVS + 8, stub_statuses, NULL);
	claims_end();
	if (emu_debug_on()) fprintf(stderr, "darkinvs: the player's chip AI at %08x, %s\n", (unsigned)m, ok ? "its hook set" : "NOT set");
	return ok;
}

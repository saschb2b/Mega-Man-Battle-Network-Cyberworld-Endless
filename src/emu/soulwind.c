/* WindSoul's shoes and wind, and a Soul's taking of the form at his frame's end (soulwind.h). */
#include "soulwind.h"

#include "bn6.h"
#include "bn6_forms.h"
#include "bn6_navis.h"
#include "bn6_panels.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soul_room.h"
#include "soulbn4.h"
#include "soullife.h"
#include "soulpassive.h"
#include "thumb_reloc.h"

#define BUS 0x08000000u
#define SOUL_WIND 0x1A      /* (BN6_SOULS' block, past BN6_SOUL_WINDOW) u8 the frames' ends till his wind is spawned, 0 none */
#define WIND_WAIT 2         /* ... as he enters: BN4 spawns it as the form's state is set in the unison (BN4_FORM_STATE), its
                             * first frame a frame after BN6's proxy form is entered, so the end of his next frame */
#define WIND_ENDING 8       /* BN6_WIND_END's state (+8) */
#define WIND_SPAWNER 0x4C   /* the wind's spawner, which BN6_WIND_HOLD reads (BN6_WIND_SPAWN's str r5,[r0,#0x4c]) */
#define WIND_LEVEL 0x60     /* ... and the word BN6_WIND_END clears */
#define WIND_STATE 0x08

static bool done;
static bool late[SOULS_BN4];   /* each BN4 Soul whose taking of the form waits for his frame's end (soulbn4_late) */

static void flags_set(uint32_t obj, uint32_t flags, bool on) {
	uint32_t coll = hook_read32(obj + BN6_T1_COLLISION);
	if (!coll) return;
	uint32_t f = hook_read32(coll + BN6_COLL_FLAGS);
	hook_write32(coll + BN6_COLL_FLAGS, on ? f | flags : f & ~flags);
}

void soulwind_enter(uint32_t obj, int k) {
	if (!done || k < 0 || k >= SOULS_BN4 || (!soulbn4_wind(k) && !late[k])) return;
	hook_write8(soul_field_of(obj, SOUL_WIND), WIND_WAIT);
}

bool soulwind_late(int k) { return done && k >= 0 && k < SOULS_BN4 && late[k]; }

void soulwind_leave(uint32_t obj, int k) {
	uint32_t flags = soulbn4_wind(k);
	if (!done || !flags) return;
	flags_set(obj, flags, false);
	hook_write8(soul_field_of(obj, SOUL_WIND), 0);
	/* (his side's wind: BN4's AI +0x5C's, BN6's side slot's, his side's alone a wind of his) */
	uint32_t slot = BN6_BATTLE_STATE + BN6_WIND_SLOTS + 4u * (hook_read8(obj + BN6_T1_ALLIANCE) & 1);
	uint32_t wind = hook_read32(slot);
	if (!wind) return;
	hook_write32(wind + WIND_STATE, WIND_ENDING);
	hook_write32(wind + WIND_LEVEL, 0);
}

/* (BN6_PLAYER_LAST_CALL, r5 him) his flags ORed and his wind spawned at the end of the frame its count runs out in, then the
 * call */
static HookAct last(HookRegs *r, void *user) {
	uint32_t at = soul_field_of(r->r[5], SOUL_WIND);
	uint8_t n = hook_read8(at);
	if (!n) return HOOK_CONTINUE;
	hook_write8(at, (uint8_t)(n - 1));
	if (n > 1) return HOOK_CONTINUE;
	int k = soulbn4_k(soul_id_of(r->r[5]));
	if (k >= 0 && late[k]) {   /* (... or his taking of the form, run then: JunkSoul's) */
		r->r[12] = SOULS4_LATE | 1;
		return HOOK_JUMP;
	}
	/* (his flags with it: BN6's form's state, set after the proxy form is entered, sets his collision's flags anew) */
	flags_set(r->r[5], soulbn4_wind(soulbn4_k(soul_id_of(r->r[5]))), true);
	r->r[12] = (uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* (BN6_WIND_HOLD's first instruction, r5 a wind) a wind with no spawner, WindSoul's (BN4's spawn keeps none), steps as BN4's
 * state 4 does, holding for nothing: on to its step, a tail call */
static HookAct hold(HookRegs *r, void *user) {
	(void)user;
	if (hook_read32(r->r[5] + WIND_SPAWNER)) return HOOK_CONTINUE;
	r->r[12] = BN6_WIND_STEP | 1;
	return HOOK_JUMP;
}

/* SOULS4_LATE: the Soul's taking of the form's copy (`enter`, r5 him), then the call the hook stands on, then back past it, each
 * through a bx r3 (past a bl's reach) */
static bool late_stage(uint32_t enter) {
	static const uint16_t code[] = {
		0x4B04,   /* 00 ldr r3,[pc,#0x10]: the taking of the form's copy */
		0, 0,     /* 02 bl 0x10 */
		0x4B04,   /* 06 ldr r3,[pc,#0x10]: BN6_PLAYER_LAST */
		0, 0,     /* 08 bl 0x10 */
		0x4B03,   /* 0C ldr r3,[pc,#0xc]: back */
		0x4718,   /* 0E bx r3 */
		0x4718,   /* 10 bx r3 */
		0x46C0,   /* 12 nop */
		0, 0,     /* 14 the copy */
		0, 0,     /* 18 BN6_PLAYER_LAST */
		0, 0,     /* 1C BN6_PLAYER_FRAME_END */
	};
	_Static_assert(sizeof code <= SOULS4_LATE_END - SOULS4_LATE, "the late taking's room");
	uint8_t out[sizeof code];
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + 0x14, enter | 1);
	put32(out + 0x18, BN6_PLAYER_LAST | 1);
	put32(out + 0x1C, BN6_PLAYER_FRAME_END | 1);
	if (!thumb_bl_write(out, 0x02, SOULS4_LATE, SOULS4_LATE + 0x10) || !thumb_bl_write(out, 0x08, SOULS4_LATE, SOULS4_LATE + 0x10))
		return false;
	claims_begin("a Soul's taking of the form at his frame's end (soulwind.c)");
	emu_write(SOULS4_LATE, out, sizeof out);
	claims_end();
	return true;
}

bool soulwind_install(int breaks) {
	if (done) return true;
	bool any = false, lates = false;
	uint32_t enter = 0;
	for (int k = 0; k < SOULS_BN4; ++k) {
		any = any || soulbn4_wind(k);
		if (soulbn4_late(k) && (enter = soulbn4_enter_copy(k)) != 0) lates = late[k] = true;
	}
	if ((!any && !lates) || (breaks & SOULPASSIVE_BREAK_READY)) return true;
	int level = any ? soulbn4_wind_level() : 0;
	SoulBn4CollSet set = { 0 };
	if (level < 0 || (any && !soulbn4_wind_coll(&set)) || (lates && !late_stage(enter))) return false;
	uint32_t to;
	if (!thumb_bl_read(R.data + (BN6_PLAYER_LAST_CALL - BUS), 0, BN6_PLAYER_LAST_CALL, &to) || to != BN6_PLAYER_LAST ||
		get16(R.data + (BN6_WIND_HOLD - BUS)) != 0xB500 /* push {lr} */ || get16(R.data + (BN6_WIND_HOLD_SPAWNER - BUS)) != 0x6CE8 /* ldr r0,[r5,#0x4c] */)
		return false;
	/* (the wind's spawn, then the call the hook stands on, then back past it: each through a bx r3, past a bl's reach) */
	/* (BN4's spawn keeps no spawner at the wind's +0x4C, so BN4's wind never holds for his state (BN6_WIND_HOLD) and gusts as it
	 * starts: BN6's spawner word cleared after BN6's spawn, as BN4's. The spawn's r4, its +4 to +7, BN4's level 1 (movs r4,#1 before
	 * BN4_WIND_SPAWN_CALL), which the step's first phase reads (+6, 0x080E65F2): his r4 kept round it) */
	/* (and first his collision set up again with BN4's arguments, BN4's 0x08012ED0 before the spawn, BN6's twin BN6_COLL_SET_UP) */
	static const uint16_t code[] = {
		0xB410,   /* 00 push {r4} */
		0x2100,   /* 02 movs r1,#region */
		0x2200,   /* 04 movs r2,#region2 */
		0x2300,   /* 06 movs r3,#arg */
		0, 0,     /* 08 bl 0x28: his collision set up */
		0x2400,   /* 0C movs r4,#level */
		0, 0,     /* 0E bl 0x30: the wind's spawn */
		0xBC10,   /* 12 pop {r4} */
		0x2800,   /* 14 cmp r0,#0 */
		0xD001,   /* 16 beq 0x1C */
		0x2100,   /* 18 movs r1,#0 */
		0x64C1,   /* 1A str r1,[r0,#0x4c] */
		0, 0,     /* 1C bl 0x38: the call the hook stands on */
		0x4B00,   /* 20 ldr r3,[pc,#0]: back */
		0x4718,   /* 22 bx r3 */
		0, 0,     /* 24 its word */
		0x4C00,   /* 28 ldr r4,[pc,#0]: r3 is the set-up's argument, r4 pushed */
		0x4720,   /* 2A bx r4 */
		0, 0,     /* 2C BN6_COLL_SET_UP */
		0x4B00,   /* 30 ldr r3,[pc,#0] */
		0x4718,   /* 32 bx r3 */
		0, 0,     /* 34 BN6_WIND_SPAWN */
		0x4B00,   /* 38 ldr r3,[pc,#0] */
		0x4718,   /* 3A bx r3 */
		0, 0,     /* 3C BN6_PLAYER_LAST */
	};
	uint8_t out[SOULS4_WIND_END - SOULS4_WIND];
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put16(out + 0x02, (uint16_t)(0x2100 | set.region));
	put16(out + 0x04, (uint16_t)(0x2200 | set.region2));
	put16(out + 0x06, (uint16_t)(0x2300 | set.arg));
	put16(out + 0x0C, (uint16_t)(0x2400 | level));
	put32(out + 0x24, BN6_PLAYER_FRAME_END | 1);   /* (past the call the hook stands on: the frame's pop) */
	put32(out + 0x2C, BN6_COLL_SET_UP | 1);
	put32(out + 0x34, BN6_WIND_SPAWN | 1);
	put32(out + 0x3C, BN6_PLAYER_LAST | 1);
	if (!thumb_bl_write(out, 0x08, SOULS4_WIND, SOULS4_WIND + 0x28) || !thumb_bl_write(out, 0x0E, SOULS4_WIND, SOULS4_WIND + 0x30) ||
		!thumb_bl_write(out, 0x1C, SOULS4_WIND, SOULS4_WIND + 0x38))
		return false;
	claims_begin("WindSoul's wind (soulwind.c)");
	emu_write(SOULS4_WIND, out, sizeof out);
	bool ok = emu_hook(BN6_PLAYER_LAST_CALL, last, (void *)(uintptr_t)SOULS4_WIND) && emu_hook(BN6_WIND_HOLD, hold, NULL);
	claims_end();
	return done = ok;
}

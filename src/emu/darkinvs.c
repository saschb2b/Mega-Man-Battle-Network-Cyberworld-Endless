/* BN5's DarkInvs in BN6's engine: the player's chip AI (darkinvs.h). */
#include "darkinvs.h"

#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "debug.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"
#include "soulchaos.h"
#include "xnavi_bn5.h"
#include "xnavi_install.h"
#include "xvirus.h"

#define POOL_AT (RAM_PLAYER_AI + BN5_AI_DATA_SIZE)   /* (his own AI data's place in BN6's pool while the block holds it) */
#define ENTER_AT 120                                 /* (--dev chipai: two seconds into a battle) */
#define DEV_FRAMES 600                               /* (... for DarkInvs's 600 frames) */

static bool dev_on;
static uint32_t last_timer, entered_at = UINT32_MAX;

/* the machine's place in the Dark Soul's moved code, 0 where it is not there */
static uint32_t machine(void) {
	if (xnavi_bn6_id(BN5_DARK_SOUL_ID) < 0) return 0;
	for (int i = 0; i < xnavi_bn5_count; ++i)
		if (xnavi_bn5[i]->code.src_ai == BN5_NAVI_DARK_SOUL) return xvirus_code_at(&xnavi_bn5[i]->code, BN5_CHIP_AI_PLAYER | 1u);
	return 0;
}

void darkinvs_enter(uint32_t me, uint16_t frames) {
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA);
	/* (BN6's 0x100 bytes into BN5's 0x140: the chip AI keeps its state past BN6's, BN6_DARK_AI's reason) */
	if (ai != RAM_PLAYER_AI) {
		for (uint32_t k = 0; k < BN5_AI_DATA_SIZE; k += 4) hook_write32(RAM_PLAYER_AI + k, k < BN6_AI_DATA_SIZE ? hook_read32(ai + k) : 0);
		hook_write32(POOL_AT, ai);
		hook_write32(me + BN6_T1_AI_DATA, RAM_PLAYER_AI);
	}
	/* (BN5's 0x0802C03E, then 0x0802C114 with the time: the state cleared, under way, the time, the flag) */
	for (uint32_t k = 0; k < 0x40; k += 4) hook_write32(RAM_PLAYER_AI + BN5_CHIP_AI_STATE + k, 0);
	hook_write16(RAM_PLAYER_AI + BN5_AI_UNDER_TIME, frames);
	hook_write8(RAM_PLAYER_AI + BN5_CHIP_AI_STATE, 1);
	hook_write32(RAM_PLAYER_AI + BN6_AI_MODES, hook_read32(RAM_PLAYER_AI + BN6_AI_MODES) | BN5_AI_UNDER);
	soulchaos_lists_fill(hook_read8(me + BN6_T1_ALLIANCE) & 1);
	if (emu_debug_on()) fprintf(stderr, "darkinvs: MegaMan under BN5's chip AI for %u frames\n", (unsigned)frames);
}

/* (--dev chipai alone: BN5's end, its timer and the engine's exits, is not in) out of it, his own AI data back */
static void leave(uint32_t me) {
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA), pool = hook_read32(POOL_AT);
	hook_write32(ai + BN6_AI_MODES, hook_read32(ai + BN6_AI_MODES) & ~(uint32_t)BN5_AI_UNDER);
	hook_write8(ai + BN5_CHIP_AI_STATE, 0);
	if (ai == RAM_PLAYER_AI && pool) {
		for (uint32_t k = 0; k < BN6_AI_DATA_SIZE; k += 4) hook_write32(pool + k, hook_read32(RAM_PLAYER_AI + k));
		hook_write32(me + BN6_T1_AI_DATA, pool);
		hook_write32(POOL_AT, 0);
	}
	if (emu_debug_on()) fprintf(stderr, "darkinvs: MegaMan out of BN5's chip AI\n");
}

static void dev_step(uint32_t me) {
	uint32_t t = hook_read32(BN6_BATTLE_TIMER);
	if (t < last_timer) entered_at = UINT32_MAX;   /* (a new battle) */
	last_timer = t;
	if (entered_at == UINT32_MAX && t >= ENTER_AT) {
		entered_at = t;
		darkinvs_enter(me, DEV_FRAMES);
	} else if (entered_at != UINT32_MAX && entered_at != 0 && t >= entered_at + DEV_FRAMES) {
		entered_at = 0;
		leave(me);
	}
}

/* (BN6_PLAYER_IDLE_FREE, r5 him) his chip AI where it stands: on to the stub, which runs it and leaves the state */
static HookAct idle(HookRegs *r, void *user) {
	uint32_t me = r->r[5];
	if (hook_read8(me + BN6_T1_ALLIANCE)) return HOOK_CONTINUE;
	if (dev_on) dev_step(me);
	uint32_t ai = hook_read32(me + BN6_T1_AI_DATA);
	if (!(hook_read32(ai + BN6_AI_MODES) & BN5_AI_UNDER)) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1u;
	return HOOK_JUMP;
}

bool darkinvs_install(bool dev) {
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
	bool ok = emu_hook(BN6_PLAYER_IDLE_FREE, idle, (void *)(uintptr_t)FREE_CHIP_AI);
	claims_end();
	dev_on = ok && dev;
	if (emu_debug_on()) fprintf(stderr, "darkinvs: the player's chip AI at %08x, %s\n", (unsigned)m, ok ? "its hook set" : "NOT set");
	return ok;
}

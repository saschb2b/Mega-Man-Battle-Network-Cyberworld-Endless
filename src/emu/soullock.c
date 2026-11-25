/* A BN4 Soul's B and left's lock (soullock.h). */
#include "soullock.h"

#include <stdint.h>

#include "bn6_attacks.h"
#include "bytes.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soulbn4.h"
#include "soullife.h"

#define BUS 0x08000000u
#define OP_STRB_LOCK 0x7548u   /* strb r0,[r1,#0x15] */

/* (BN6_ATTACK_END_LEFT, r5 him) on past the lock's store where he stands in a BN4 Soul */
static HookAct end(HookRegs *r, void *user) {
	(void)user;
	if (soulbn4_k(soul_id_of(r->r[5])) < 0) return HOOK_CONTINUE;
	r->r[12] = BN6_ATTACK_END_LEFT_PAST | 1;
	return HOOK_JUMP;
}

bool soullock_install(void) {
	static bool done;
	if (done) return true;
	bool any = false;
	for (int k = 0; k < SOULS_BN4; ++k) {
		SoulBn4 s;
		any = any || (soulbn4_read(k, &s) && s.left_routine);
	}
	if (!any) return true;
	if (get16(R.data + (BN6_ATTACK_END_LEFT - BUS)) != OP_STRB_LOCK) return false;
	return done = emu_hook(BN6_ATTACK_END_LEFT, end, NULL);
}

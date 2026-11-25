/* The BN4 Souls no hit's status takes (soulguard.h). */
#include "soulguard.h"

#include <stdint.h>

#include "bn5_capsules.h"
#include "bn6_panels.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "soulbn4.h"
#include "soullife.h"
#include "soulpassive.h"
#include "thumb_reloc.h"

#define BUS 0x08000000u

static bool guarded[SOULS_BN4];   /* each BN4 Soul's guard, by k */

/* (BN6_PLAYER_STATUS_CALL, r5 him) on past the status's call where he stands in a guarded BN4 Soul */
static HookAct status(HookRegs *r, void *user) {
	(void)user;
	int k = soulbn4_k(soul_id_of(r->r[5]));
	if (k < 0 || !guarded[k]) return HOOK_CONTINUE;
	r->r[12] = BN6_PLAYER_STATUS_PAST | 1;
	return HOOK_JUMP;
}

bool soulguard_install(int breaks) {
	static bool done;
	if (done) return true;
	int n = 0;
	for (int k = 0; k < SOULS_BN4; ++k) {
		guarded[k] = soulbn4_guarded(k);
		n += guarded[k];
	}
	if (!n || (breaks & SOULPASSIVE_BREAK_READY)) return done = true;
	/* (BN6's code the hook stands on: the bl to the status's routine) */
	uint32_t to;
	if (!thumb_bl_read(R.data + (BN6_PLAYER_STATUS_CALL - BUS), 0, BN6_PLAYER_STATUS_CALL, &to) || to != BN6_STATUS_TAKEN) return false;
	return done = emu_hook(BN6_PLAYER_STATUS_CALL, status, NULL);
}

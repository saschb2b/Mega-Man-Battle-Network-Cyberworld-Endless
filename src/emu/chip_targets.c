/* chip_targets.h: the player's chips aimed at the moved foes too. */
#include "chip_targets.h"

#include <stdbool.h>
#include <stdio.h>

#include "bn6.h"
#include "bn6_families.h"
#include "bn6_forms.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"
#include "rom.h"
#include "thumb_reloc.h"

#define BUS 0x08000000u

/* the calls hooked (bn6_chips.h) */
static const uint32_t SITES[] = {
	BN6_AQUANDL_FIND_VIRUSES, BN6_AQUANDL_FIND_NAVIS, BN6_YOYO_FIND_VIRUSES, BN6_YOYO_FIND_NAVIS, BN6_RETICLE_FIND,
};
#define SITE_COUNT (sizeof SITES / sizeof *SITES)
static uint32_t last_said[SITE_COUNT];

/* (at the call's bl: r0 the list it fills, r1 and r2 the range, r3 the side, r10 eToolkit.) What the call would leave:
 * the side's actors in the range, in their slots' order, at r0; r0 how many, r3 the slots walked, every other register
 * as it was, lr the call's return; then on past the bl, as BN6's routine returns */
static HookAct find(HookRegs *r, void *user) {
	uint32_t *said = user;   /* (what CYBERWORLD_EMU_DEBUG last said of this call: the reticle's looks every frame) */
	uint32_t list = r->r[0];
	int32_t lo = (int32_t)r->r[1], hi = (int32_t)r->r[2];
	uint32_t slots = hook_read32(r->r[10] + BN6_TOOLKIT_BATTLE) + BN6_BATTLE_ACTORS + r->r[3] * BN6_BATTLE_PLAYER_STEP;
	uint32_t n = 0, moved = 0;
	for (uint32_t k = 0; k < BN6_BATTLE_PLAYER_SLOTS; ++k) {
		uint32_t o = hook_read32(slots + 4 * k);
		if (!o) continue;
		uint16_t id = hook_read16(o + BN6_T1_NAME_ID), as = chip_targets_name(id);
		if (as < lo || as > hi) continue;
		hook_write32(list + 4 * n++, o);
		if (as != id) moved |= 1u << k;
	}
	if (moved && emu_debug_on() && *said != (n << 8 | moved))
		fprintf(stderr, "chip_targets: %08x: %u of NameIDs %x-%x, the moved foes of slots %x among them\n", (unsigned)r->pc, (unsigned)n, (unsigned)lo,
			(unsigned)hi, (unsigned)moved);
	*said = n << 8 | moved;
	r->r[0] = n;
	r->r[3] = BN6_BATTLE_PLAYER_SLOTS;
	r->lr = (r->pc + 4) | 1;
	return HOOK_RETURN;
}

void chip_targets_install(void) {
	static bool done;
	if (done || !R.data || !emu_ready()) return;
	done = true;
	for (unsigned i = 0; i < SITE_COUNT; ++i) {
		uint32_t at = SITES[i], off = at - BUS, to = 0;
		/* (only where the game is the one read: the call the hook stands on) */
		if (!thumb_bl_decode(at, rom_u16(off), rom_u16(off + 2), &to) || to != BN6_BY_NAME_RANGE) {
			fprintf(stderr, "chip_targets: %08x is no call of %08x: its chip finds BN6's foes alone\n", (unsigned)at, (unsigned)BN6_BY_NAME_RANGE);
			continue;
		}
		if (!emu_hook(at, find, &last_said[i])) fprintf(stderr, "chip_targets: no hook at %08x: its chip finds BN6's foes alone\n", (unsigned)at);
	}
}

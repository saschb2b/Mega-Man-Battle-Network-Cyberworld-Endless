/* save_mask.h: BN6's save's mask marked by hooks at its two ends. A hook
 * runs on the core's own thread where it has one; the mark is read after
 * emu_sync, the frame ended. */
#include "save_mask.h"

#include <stdio.h>

#include "bn6_save.h"
#include "emu.h"

static bool masked;
static unsigned masks;   /* since the start: one a save, where nothing wrote into it masked */

static HookAct mask_begins(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	masked = true;
	++masks;
	return HOOK_CONTINUE;
}

static HookAct mask_ends(HookRegs *r, void *user) {
	(void)r;
	(void)user;
	masked = false;
	return HOOK_CONTINUE;
}

/* (the instruction checked first: a ROM that reads otherwise there has no
 * hook, and the run reads its RAM through the save as before) */
static void hook_at(uint32_t addr, uint16_t op, EmuHook fn) {
	if (emu_read16(addr) != op || !emu_hook(addr, fn, NULL)) fprintf(stderr, "save mask: no hook at %08x\n", (unsigned)addr);
}

void save_mask_install(void) {
	static bool done;
	if (done) return;
	done = true;
	hook_at(BN6_SAVE_MASK_AT, BN6_SAVE_MASK_AT_OP, mask_begins);
	hook_at(BN6_SAVE_UNMASKED, BN6_SAVE_UNMASKED_OP, mask_ends);
	hook_at(BN6_SAVE_GAVE_UP, BN6_SAVE_UNMASKED_OP, mask_ends);
}

bool save_masked(void) {
	emu_sync();
	return masked;
}

void save_mask_loaded(void) { masked = false; }

void save_mask_describe(FILE *f) {
	bool now = save_masked();
	fprintf(f, "savemasks %u\nsavemasked %s\n", masks, now ? "yes" : "no");
}

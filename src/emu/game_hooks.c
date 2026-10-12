/* game_hooks.h. Each hook's own file says what it hooks and why
 * (docs/EMULATION.md); this one keeps their order. */
#include "game_hooks.h"

#include "claims.h"
#include "debug.h"
#include "encounter.h"
#include "events.h"
#include "gamecall.h"
#include "idle.h"
#include "npc.h"
#include "pet.h"
#include "pet_text.h"
#include "protoman_cross.h"
#include "scripts.h"
#include "xchips_install.h"

void game_hooks_install(void) {
	/* (BN6's chip tables grown, BN5's chips in them or their placeholders,
	 * and the hook on the deck's deal: before the boot, whose NEW GAME
	 * counts its folder by the records; the same at every boot,
	 * docs/MULTIROM.md, The chip registry) */
	xchips_install();
	gamecall_install();
	idle_install();
	npc_reach_install();
	chat_marks_install();
	pet_install();
	pet_text_install();
	if (emu_debug_on()) claims_report();
}

void game_hooks_after_boot(void) {
	emu_encounters_install();
	protoman_cross_install();
	events_install();
}

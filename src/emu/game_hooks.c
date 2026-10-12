/* game_hooks.h. Each hook's own file says what it hooks and why
 * (docs/EMULATION.md); this one keeps their order. */
#include "game_hooks.h"

#include "devtools.h"
#include "encounter.h"
#include "events.h"
#include "gamecall.h"
#include "idle.h"
#include "npc.h"
#include "pet.h"
#include "pet_text.h"
#include "protoman_cross.h"
#include "scripts.h"
#include "soulproxy.h"
#include "souls.h"
#include "soulunite.h"

void game_hooks_install(void) {
	gamecall_install();
	idle_install();
	npc_reach_install();
	chat_marks_install();
	pet_install();
	pet_text_install();
	/* (a test's: the Souls' proxy form, --dev soulproxy, and the Souls on it
	 * in BN6's battles, which --dev souls=MASK asks for too: the run's Souls
	 * reach it as each battle begins, souls.c) */
	if (dev.soulproxy || souls_dev_mask) {
		bool proxy = soulproxy_install(dev.proxybreak);
		if (proxy) soulunite_install(dev.unitebreak);
	}
}

void game_hooks_after_boot(void) {
	emu_encounters_install();
	protoman_cross_install();
	events_install();
}

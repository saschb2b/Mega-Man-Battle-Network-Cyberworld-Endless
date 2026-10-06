/* game_hooks.h. Each hook's own file says what it hooks and why
 * (docs/EMULATION.md); this one keeps their order. */
#include "game_hooks.h"

#include "encounter.h"
#include "events.h"
#include "gamecall.h"
#include "idle.h"
#include "npc.h"
#include "pet.h"
#include "pet_text.h"
#include "protoman_cross.h"
#include "scripts.h"

void game_hooks_install(void) {
	gamecall_install();
	idle_install();
	npc_reach_install();
	chat_marks_install();
	pet_install();
	pet_text_install();
}

void game_hooks_after_boot(void) {
	emu_encounters_install();
	protoman_cross_install();
	events_install();
}

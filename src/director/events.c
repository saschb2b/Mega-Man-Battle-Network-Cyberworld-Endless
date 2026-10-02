/* The game's map, flag and key item events, by hook (events.h; docs/
 * EMULATION.md, Hooks; issue #31): a map entered, a layer's choice taken,
 * a key item given, each queued for director_update as it happens. */
#include "events.h"

#include <stdbool.h>

#include "bn6.h"
#include "emu.h"
#include "layer_objs.h"

/* SetEventFlag runs for every flag the game sets, many a frame: an event
 * only for a layer's choice, which a Yes in its chat sets (the chat's
 * EA 00 command) */
static HookAct flag_set(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] >= LAYER_FLAG_BASE && r->r[0] < LAYER_FLAG_BASE + LAYER_MAX_CHOICES) hook_post(r, EV_CHOICE);
	return HOOK_CONTINUE;
}

void events_install(void) {
	static bool done;
	if (done) return;
	done = true;
	emu_hook_event(BN6_ENTER_MAP, EV_MAP_ENTER);
	emu_hook(BN6_SET_EVENT_FLAG, flag_set, NULL);
	emu_hook_event(BN6_GIVE_ITEM, EV_ITEM_GIVEN);
}

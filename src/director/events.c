/* The game's map, flag and key item events, by hook (events.h; docs/
 * docs/engine/emulation/hooks.md; issue #31): a map entered, a layer's choice taken,
 * a key item given, each queued for director_update as it happens. */
#include "events.h"

#include <stdbool.h>
#include <stdio.h>

#include "battle_data.h"
#include "bn6.h"
#include "bn6_bbs.h"
#include "debug.h"
#include "director.h"
#include "emu.h"
#include "job_words.h"
#include "layer_objs.h"
#include "order_sort.h"
#include "weekly.h"
#include "xchips_wide.h"

/* SetEventFlag runs for every flag the game sets, many a frame: an event
 * only for a layer's choice, which a Yes in its chat sets (the chat's
 * EA 00 command), and for a request taken or settled at home, on BN6's
 * Request BBS too */
static HookAct flag_set(HookRegs *r, void *user) {
	(void)user;
	if (r->r[0] >= LAYER_FLAG_BASE && r->r[0] < LAYER_FLAG_BASE + LAYER_MAX_CHOICES) hook_post(r, EV_CHOICE);
	else if ((r->r[0] >= JOB_TAKE_FLAG && r->r[0] < JOB_TAKE_FLAG + JOB_ASKERS) || r->r[0] == JOB_SETTLED_FLAG ||
		(r->r[0] >= BN6_FLAG_BBS_TAKEN && r->r[0] < BN6_FLAG_BBS_TAKEN + JOB_ASKERS))
		hook_post(r, EV_JOB);
	return HOOK_CONTINUE;
}

/* BN6's dying state, for every battle object that dies with its triple
 * read: an event for a virus (actor type 0), by its AI index and version,
 * with the side it fought on */
static HookAct foe_dying(HookRegs *r, void *user) {
	(void)user;
	if (hook_read8(r->r[0] + 1)) return HOOK_CONTINUE;
	HookRegs ev = *r;
	ev.r[0] = hook_read8(r->r[0] + 2);
	ev.r[1] = hook_read8(r->r[0]);
	ev.r[2] = hook_read8(r->r[5] + BN6_T1_ALLIANCE);
	hook_post(&ev, EV_FOE_DELETED);
	return HOOK_CONTINUE;
}

void events_foe_deleted(const uint32_t r[3], bool counted) {
	if (!counted || !r[2] || weekly_on()) return;
	battle_data_deleted((int)r[0], (int)r[1]);
	if (emu_debug_on())
		fprintf(stderr, "battle data: family %u version %u deleted, %d of %d held, versions %02x%s, %d families complete\n", (unsigned)r[0], (unsigned)r[1],
			battle_data_count((int)r[0]), DATA_COMPLETE, battle_data_versions((int)r[0]), battle_data_complete((int)r[0]) ? ", complete" : "",
			battle_data_families_complete());
}

void events_install(void) {
	static bool done;
	if (done) return;
	done = true;
	emu_hook_event(BN6_ENTER_MAP, EV_MAP_ENTER);
	emu_hook(BN6_SET_EVENT_FLAG, flag_set, NULL);
	emu_hook_event(BN6_GIVE_ITEM, EV_ITEM_GIVEN);
	xchips_wide_given(EV_CHIP_GIVEN_WIDE);
	/* (AsterLand's Order Service sorts by MegaMan's copies: order_sort.h) */
	order_sort_pack(director_pack_now);
	/* (the instruction checked first: a ROM that reads otherwise there keeps its battles, and the battle data stays as it is) */
	if (emu_read16(BN6_FOE_DYING) != BN6_FOE_DYING_OP || !emu_hook(BN6_FOE_DYING, foe_dying, NULL))
		fprintf(stderr, "events: no hook at %08x: no battle data counted\n", (unsigned)BN6_FOE_DYING);
}

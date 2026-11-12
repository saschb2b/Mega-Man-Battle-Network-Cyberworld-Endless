/* grab_search.h: BN6's search for AreaGrab's column, ended. */
#include "grab_search.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_panels.h"
#include "debug.h"
#include "emu.h"
#include "hook.h"

/* (r5 the grabbing object, r4 the walk's step, r6 its column) */
static HookAct turn(HookRegs *r, void *user) {
	(void)user;
	uint32_t o = r->r[5];
	if (!grab_search_lost((int)(int32_t)r->r[4], hook_read8(o + BN6_T1_ALLIANCE), hook_read8(o + BN6_T1_FLIP))) return HOOK_CONTINUE;
	r->r[6] = hook_read8(o + BN6_T1_PANEL_X);
	if (emu_debug_on()) fprintf(stderr, "grab: no column wholly side %d's: the search ended on column %u\n", hook_read8(o + BN6_T1_ALLIANCE), (unsigned)r->r[6]);
	r->r[12] = BN6_GRAB_FOUND | 1;
	return HOOK_JUMP;
}

void grab_search_install(void) {
	if (!emu_hook(BN6_GRAB_TURN, turn, NULL)) fprintf(stderr, "grab: no hook at %08x: AreaGrab's search can run on for good\n", (unsigned)BN6_GRAB_TURN);
}

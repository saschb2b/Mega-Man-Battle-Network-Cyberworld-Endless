/* --dev arena2v2: the leads wait for their partners (arena_team_wait.h). */
#include "arena_team_wait.h"

#include <stdio.h>

#include "arena_team_custom.h"
#include "bn6.h"
#include "hook.h"

static unsigned held, sent;   /* every core's, a rollback's frames run again among them */

/* (BN6_LINK_SEND's entry, from mode 0x14's first frame) this GBA's packet held back while its side's partner picks: owed */
static HookAct send(HookRegs *r, void *user) {
	(void)user;
	uint32_t from = r->lr & ~1u;
	if (from < BN6_CUSTOM_WAIT || from >= BN6_CUSTOM_WAIT_END) return HOOK_CONTINUE;
	unsigned side = hook_read8(BN6_BATTLE_SIDE) & 1u;
	if (!arena_team_custom_holds(side)) return HOOK_CONTINUE;
	arena_team_custom_owe(side, true);
	++held;
	return HOOK_RETURN;
}

/* (BN6_CUSTOM_WAIT_LOOP, each frame of mode 0x14) the packet owed and the hold over: sent now, BN6's own routine run from
 * here and back here, then its wait for both packets as ever */
static HookAct waiting(HookRegs *r, void *user) {
	(void)user;
	unsigned side = hook_read8(BN6_BATTLE_SIDE) & 1u;
	if (!arena_team_custom_owed(side) || arena_team_custom_holds(side)) return HOOK_CONTINUE;
	arena_team_custom_owe(side, false);
	++sent;
	r->lr = BN6_CUSTOM_WAIT_LOOP | 1u;
	r->r[12] = BN6_LINK_SEND | 1u;
	return HOOK_JUMP;
}

bool arena_team_wait_hooks(struct HookCore *core) {
	return hook_on(core, BN6_LINK_SEND, send, NULL) && hook_on(core, BN6_CUSTOM_WAIT_LOOP, waiting, NULL);
}

void arena_team_wait_report(void) {
	printf("arena2v2: a side's Custom screen's packet held back for its partner %u times and sent once he was in %u times, "
		"every core's\n", held, sent);
}

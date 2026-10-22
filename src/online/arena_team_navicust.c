/* --dev arena2v2: a partner's NaviCust compiled by BN6's own code (arena_team_navicust.h). */
#include "arena_team_navicust.h"

#include <stdio.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_loadout.h"
#include "arena_pair.h"
#include "bn6.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"
#include "rom.h"

#define CALL_FRAMES 120   /* frames the call may wait for the game's own update (gamecall.c's) */

/* the game's own call (gamecall.c's, arena_boot.c's): at the game mode's
 * update BN6's routine runs in its place, returning into the free space
 * past the game, where the second hook goes back to the main loop */
typedef struct {
	bool queued, called;
	uint32_t lr, kept[8];
} Call;

static HookAct update(HookRegs *r, void *user) {
	Call *c = user;
	if (!c->queued) return HOOK_CONTINUE;
	c->queued = false;
	c->lr = r->lr;
	memcpy(c->kept, r->r + 4, sizeof c->kept);
	r->r[0] = r->r[1] = r->r[2] = 0;
	r->lr = FREE_CALL_RETURN | 1;
	r->r[12] = BN6_NAVICUST_RELOAD | 1;
	return HOOK_JUMP;
}

static HookAct returned(HookRegs *r, void *user) {
	Call *c = user;
	c->called = true;
	memcpy(r->r + 4, c->kept, sizeof c->kept);
	r->r[12] = c->lr | 1;
	return HOOK_JUMP;
}

static uint32_t core_read32(void *context, uint32_t address) {
	struct mCore *c = context;
	return c->rawRead32(c, address, -1);
}

static void core_write8(void *context, uint32_t address, uint8_t value) {
	struct mCore *c = context;
	c->rawWrite8(c, address, -1, value);
}

bool team_navicust_compile(const uint8_t *state, size_t n, const OnlineLoadout *l, uint8_t *out, char *why, size_t wn) {
	static uint32_t video[ARENA_W * ARENA_H];
	if (why && wn) *why = 0;
	struct mCore *c = R.data && state && l && out ? arena_core_new(R.data, ROM_SIZE, video, false) : NULL;
	if (!c) {
		if (why && wn) snprintf(why, wn, "no core for the compile");
		return false;
	}
	const ArenaMemory m = { core_read32, core_write8, c };
	bool ok = arena_core_load(c, state, n) && arena_loadout_install(&m, l, why, wn);
	Call call = { .queued = true };
	HookCore *h = ok ? hook_core(c) : NULL;
	ok = h && hook_on(h, BN6_OW_HOOK, update, &call) && hook_on(h, FREE_CALL_RETURN, returned, &call);
	for (unsigned f = 0; ok && !call.called && f < CALL_FRAMES; ++f) {
		c->setKeys(c, 0);
		c->runFrame(c);
		emu_audio_drop_from(c);
	}
	if (ok && !call.called && why && wn) snprintf(why, wn, "BN6's NaviCust reload did not run in %u frames", CALL_FRAMES);
	ok = ok && call.called;
	if (ok) {
		for (uint32_t i = 0; i < BN6_BATTLE_NAVI_SIZE; ++i) out[i] = (uint8_t)c->rawRead8(c, BN6_NAVI_STATS + i, -1);
		/* (a NetBattle begins whole, as the Arena writes a lead's) */
		memcpy(out + BN6_NS_CUR_HP, out + BN6_NS_MAX_HP, sizeof(uint16_t));
	}
	if (h) hook_detach(c);
	arena_core_free(c);
	return ok;
}

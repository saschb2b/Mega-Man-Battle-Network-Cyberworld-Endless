/* Calls into the game's own routines (gamecall.h), through two standing
 * hooks (docs/EMULATION.md, Hooks; issue #33): at cbGameState, the game
 * mode's state update the main loop runs once a frame, a queued routine
 * runs in the update's place; where it returns, in the free ROM space, a
 * second hook keeps its r0 and r1 and goes back to the main loop, as the
 * update would have. The frame is the call's, as it was the borrowed
 * update's: the game skips one update a call. */
#include "gamecall.h"

#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "emu.h"

#define WARP_DATA (EMU_FREE + 0x000)  /* a warp record and a one-entry warp list */
#define RETURN_AT (EMU_FREE + 0x100)  /* where a called routine returns: a hook */

/* The call: set between frames, run and answered by the hooks */
static struct {
	bool queued, done;
	uint32_t fn, arg[3], out[2];
	uint32_t lr, kept[8];   /* the main loop's return, and r4-r11 as the update found them */
} call;

static HookAct state_update(HookRegs *r, void *user) {
	(void)user;
	if (!call.queued) return HOOK_CONTINUE;
	call.queued = false;
	call.lr = r->lr;
	memcpy(call.kept, r->r + 4, sizeof call.kept);
	memcpy(r->r, call.arg, sizeof call.arg);
	r->lr = RETURN_AT | 1;
	r->r[12] = call.fn | 1;
	return HOOK_JUMP;
}

static HookAct returned(HookRegs *r, void *user) {
	(void)user;
	call.out[0] = r->r[0];
	call.out[1] = r->r[1];
	call.done = true;
	memcpy(r->r + 4, call.kept, sizeof call.kept);
	/* (the main loop's mov lr,pc; bx r0 left lr's Thumb bit clear) */
	r->r[12] = call.lr | 1;
	return HOOK_JUMP;
}

void gamecall_install(void) {
	static bool done;
	if (done) return;
	done = true;
	emu_hook(BN6_OW_HOOK, state_update, NULL);
	emu_hook(RETURN_AT, returned, NULL);
}

bool game_call_ret(uint32_t fn, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t out[2]) {
	emu_sync();
	memset(&call, 0, sizeof call);
	call.fn = fn;
	call.arg[0] = r0;
	call.arg[1] = r1;
	call.arg[2] = r2;
	call.queued = true;
	/* (the update runs only in the game's own mode) */
	for (int i = 0; i < 120 && !call.done; ++i) {
		emu_frame(0);
		emu_sync();
	}
	call.queued = false;
	if (out) {
		out[0] = call.out[0];
		out[1] = call.out[1];
	}
	return call.done;
}

bool game_call(uint32_t fn, uint32_t r0, uint32_t r1) { return game_call_ret(fn, r0, r1, 0, NULL); }

/* the game starts a map's song only when it is not the one it last started:
 * after the intro, or a state whose song had stopped, forget it */
static void restart_music(void) {
	if (emu_read32(BN6_MUSIC_STATUS) & 0x80000000u) emu_write8(BN6_SONG_PLAYING, 0xFF);
}

void emu_warp(int group, int number, int x, int y, int facing) {
	restart_music();
	/* Warp2011bb0: the destination, warp index 1 of a list holding it */
	uint8_t data[32] = { (uint8_t)group, (uint8_t)number, 0, (uint8_t)facing };
	put32(data + 4, (uint32_t)x << 16);
	put32(data + 8, (uint32_t)y << 16);
	data[17] = 1;
	put32(data + 20, WARP_DATA);
	emu_write(WARP_DATA, data, sizeof data);
	emu_write(BN6_WARP, data, sizeof data);
	game_call(BN6_ENTER_MAP_ON_WARP, 0, 0);
}

void emu_warp_out(void) {
	restart_music();
	/* as the warp pad's trigger leaves it: under way, index 1, same group kind */
	emu_write8(BN6_WARP_PENDING, 1);
	emu_write8(BN6_WARP_INDEX, 1);
	emu_write8(BN6_WARP_GROUP_KIND, 0);
	game_call(BN6_WARP_DEPART_JACK_OUT, 0, 0);
}

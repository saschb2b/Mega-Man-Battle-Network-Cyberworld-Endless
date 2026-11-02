/* core_boot.h. */
#include "core_boot.h"

#include <string.h>

#include <mgba/core/core.h>

#include "bn6.h"
#include "emu.h"
#include "free_space.h"

#define CALL_FRAMES 120   /* frames a game call may wait for the game's own mode (gamecall.c) */
#define SEASIDE_1   0x91  /* the boot's warp: Seaside Area 1, as boot.c's */

enum { ACT_RUN, ACT_WARP, ACT_END_INTRO };
typedef struct { uint16_t frames, keys; uint8_t act; } Step;

/* src/emu/boot.c's sequence: the title, NEW GAME, the intro's first line,
 * the warp, the intro ended as its own end commands end it */
static const Step steps[] = {
	{ 400, 0, ACT_RUN }, { 4, KEY_START, ACT_RUN }, { 100, 0, ACT_RUN }, { 4, KEY_A, ACT_RUN }, { 200, 0, ACT_RUN },
	{ CALL_FRAMES, 0, ACT_WARP }, { 120, 0, ACT_RUN }, { 0, 0, ACT_END_INTRO }, { 30, 0, ACT_RUN },
};
#define NSTEPS (sizeof steps / sizeof *steps)

/* ---- the game's own call (gamecall.c's) ---- */

static HookAct state_update(HookRegs *r, void *user) {
	CoreBoot *b = user;
	if (!b->call.queued) return HOOK_CONTINUE;
	b->call.queued = false;
	b->call.lr = r->lr;
	memcpy(b->call.kept, r->r + 4, sizeof b->call.kept);
	r->r[0] = r->r[1] = r->r[2] = 0;
	r->lr = FREE_CALL_RETURN | 1;
	r->r[12] = BN6_ENTER_MAP_ON_WARP | 1;
	return HOOK_JUMP;
}

static HookAct returned(HookRegs *r, void *user) {
	CoreBoot *b = user;
	b->call.called = true;
	memcpy(r->r + 4, b->call.kept, sizeof b->call.kept);
	/* (the main loop's mov lr,pc; bx r0 left lr's Thumb bit clear) */
	r->r[12] = b->call.lr | 1;
	return HOOK_JUMP;
}

static void write_bytes(struct mCore *c, uint32_t at, const uint8_t *data, size_t n) {
	for (size_t i = 0; i < n; ++i) c->rawWrite8(c, at + (uint32_t)i, -1, data[i]);
}

/* emu_warp's: the destination as warp index 1 of a one-entry list, the
 * map's song started again, the game's warp routine queued */
static void warp(CoreBoot *b) {
	struct mCore *c = b->core;
	if (c->rawRead32(c, BN6_MUSIC_STATUS, -1) & BN6_MUSIC_STOPPED) c->rawWrite8(c, BN6_SONG_PLAYING, -1, 0xFF);
	uint8_t data[32] = { SEASIDE_1, 0, 0, 5 };
	data[17] = 1;
	for (unsigned i = 0; i < 4; ++i) data[20 + i] = (uint8_t)(FREE_WARP >> (8 * i));
	write_bytes(c, FREE_WARP, data, sizeof data);
	write_bytes(c, BN6_WARP, data, sizeof data);
	b->call.queued = true;
	b->call.called = false;
}

/* boot.c's end of the intro: no cutscene script, the chat box closed */
static void end_intro(struct mCore *c) {
	c->rawWrite32(c, BN6_CUTSCENE_POS, -1, 0);
	c->rawWrite32(c, BN6_CUTSCENE_POS0, -1, 0);
	c->rawWrite32(c, BN6_CHATBOX_FLAGS, -1, c->rawRead32(c, BN6_CHATBOX_FLAGS, -1) & ~0xC8u);
	c->rawWrite8(c, BN6_CHATBOX, -1, 0);
	c->rawWrite8(c, BN6_CHATBOX_STATE, -1, 0);
}

/* ---- the course ---- */

bool core_boot_begin(CoreBoot *b, struct mCore *core) {
	memset(b, 0, sizeof *b);
	b->core = core;
	b->hooks = hook_core(core);
	return b->hooks && hook_on(b->hooks, BN6_OW_HOOK, state_update, b) && hook_on(b->hooks, FREE_CALL_RETURN, returned, b);
}

int core_boot_frame(CoreBoot *b) {
	if (b->step >= NSTEPS) return 0;
	const Step *s = &steps[b->step];
	if (s->act == ACT_END_INTRO) end_intro(b->core);
	else {
		if (s->act == ACT_WARP && b->in_step == 0) warp(b);
		b->core->setKeys(b->core, s->keys);
		b->core->runFrame(b->core);
		emu_audio_drop_from(b->core);   /* (the boot is not heard) */
		++b->ran;
	}
	bool over = ++b->in_step >= s->frames || (s->act == ACT_WARP && b->call.called);
	if (over && s->act == ACT_WARP && !b->call.called) return -1;
	if (over) { b->in_step = 0; ++b->step; }
	return b->step < NSTEPS;
}

void core_boot_end(CoreBoot *b) {
	if (b->hooks) hook_detach(b->core);
	b->hooks = NULL;
}

unsigned core_boot_total(void) {
	unsigned total = 0;
	for (size_t i = 0; i < NSTEPS; ++i) total += steps[i].act == ACT_WARP ? 1 : steps[i].frames;
	return total;
}

/* The Arena's way in (arena_boot.h).
 *
 * The boot core has two hooks of its own: the game's own call (gamecall.c's:
 * at cbGameState a queued routine runs in the update's place, and returns
 * into the free space past the game, where the second hook goes back to
 * the main loop) takes MegaMan through the game's warp routine as
 * src/emu/boot.c does. The state it ends in holds RAM alone; the pair's
 * cores load it over a ROM no hook of the boot's has touched (under full
 * rules BN5's installed bytes in both, the boot's ROM's too, and BN5's
 * hooks on the pair's alone: the boot meets none of them). */
#include "arena_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include <mgba/core/core.h>

#include "arena_loadout.h"
#include "bn6.h"
#include "emu.h"
#include "hook.h"
#include "protocol.h"
#include "xinstall_log.h"

#if CW_ARENA

#define CALL_FRAMES 120   /* frames a game call may wait for the game's own mode (gamecall.c) */
#define SEASIDE_1   0x91  /* the boot's warp: Seaside Area 1, as boot.c's */

enum { ACT_RUN, ACT_WARP, ACT_END_INTRO };
typedef struct { uint16_t frames, keys; uint8_t act; } Step;

/* src/emu/boot.c's sequence: the title, NEW GAME, the intro's first line,
 * the warp, the intro ended as its own end commands end it */
static const Step boot_steps[] = {
	{ 400, 0, ACT_RUN }, { 4, KEY_START, ACT_RUN }, { 100, 0, ACT_RUN }, { 4, KEY_A, ACT_RUN }, { 200, 0, ACT_RUN },
	{ CALL_FRAMES, 0, ACT_WARP }, { 120, 0, ACT_RUN }, { 0, 0, ACT_END_INTRO }, { 30, 0, ACT_RUN },
};

/* Both GBAs' keys through BN6's menus, from the boot's overworld (the
 * research's, tools/online_link_script.c, docs/ONLINE_LINK.md): the PET,
 * Comm (two rows up), then Link Cable, the save and its message, NetBattle,
 * Single Battle and Practice, each its menu's first row (bn6f
 * CompText86DA1B8, scripts 17/25: Practice above Battle), confirmed; then
 * the connection begun and its negotiation waited through to the Custom
 * screen */
#define CONFIRM { 4, KEY_A, ACT_RUN }, { 80, 0, ACT_RUN }
static const Step entry_steps[] = {
	{ 30, 0, ACT_RUN }, { 4, KEY_START, ACT_RUN }, { 40, 0, ACT_RUN },
	{ 4, KEY_UP, ACT_RUN }, { 12, 0, ACT_RUN }, { 4, KEY_UP, ACT_RUN }, { 12, 0, ACT_RUN },
	{ 4, KEY_A, ACT_RUN }, { 100, 0, ACT_RUN },
	CONFIRM, CONFIRM, CONFIRM, CONFIRM, CONFIRM, CONFIRM,
	{ 4, KEY_A, ACT_RUN }, { 240, 0, ACT_RUN },
};
#undef CONFIRM

#define NSTEPS(a) (sizeof a / sizeof *a)

struct ArenaBoot {
	ArenaBootStage stage;
	const uint8_t *rom;
	size_t rom_size;

	OnlineLoadout loadout[2];
	bool carried[2];
	struct mCore *core;     /* the boot core, until the pair is made */
	HookCore *hooks;
	uint32_t video[ARENA_W * ARENA_H];
	ArenaPair *pair;
	unsigned step, in_step, done;
	double pair_ms, pair_worst;   /* the pair's frames through the menus, timed */
	unsigned pair_frames;
	struct { bool queued, called; uint32_t lr, kept[8]; } call;
	char error[160];
};

static ArenaBootStage fail(ArenaBoot *b, const char *why) {
	snprintf(b->error, sizeof b->error, "%s", why);
	return b->stage = ARENA_BOOT_FAILED;
}

/* ---- the game's own call, on the boot core (gamecall.c's) ---- */

static HookAct state_update(HookRegs *r, void *user) {
	ArenaBoot *b = user;
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
	ArenaBoot *b = user;
	b->call.called = true;
	memcpy(r->r + 4, b->call.kept, sizeof b->call.kept);
	/* (the main loop's mov lr,pc; bx r0 left lr's Thumb bit clear) */
	r->r[12] = b->call.lr | 1;
	return HOOK_JUMP;
}

static void write_bytes(struct mCore *c, uint32_t at, const uint8_t *data, size_t n) {
	for (size_t i = 0; i < n; ++i) c->rawWrite8(c, at + (uint32_t)i, -1, data[i]);
}

static void put32(uint8_t *p, uint32_t v) {
	for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (8 * i));
}

/* emu_warp's: the destination as warp index 1 of a one-entry list, the
 * map's song started again, the game's warp routine queued */
static void warp(ArenaBoot *b) {
	struct mCore *c = b->core;
	if (c->rawRead32(c, BN6_MUSIC_STATUS, -1) & BN6_MUSIC_STOPPED) c->rawWrite8(c, BN6_SONG_PLAYING, -1, 0xFF);
	uint8_t data[32] = { SEASIDE_1, 0, 0, 5 };
	data[17] = 1;
	put32(data + 20, FREE_WARP);
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

/* ---- a carried MegaMan into a copy of the boot ---- */

static uint32_t core_read32(void *context, uint32_t address) {
	struct mCore *c = context;
	return c->rawRead32(c, address, -1);
}

static void core_write8(void *context, uint32_t address, uint8_t value) {
	struct mCore *c = context;
	c->rawWrite8(c, address, -1, value);
}

/* the boot's state, a copy for each side with its MegaMan written in, and
 * the pair made of them */
static ArenaBootStage make_pair(ArenaBoot *b) {
	hook_detach(b->core);
	b->hooks = NULL;
	size_t base_size;
	uint8_t *base = arena_core_save(b->core, &base_size);
	uint8_t *state[2] = { NULL, NULL };
	size_t size[2] = { 0, 0 };
	bool ok = base != NULL;
	char why[128] = "";
	for (unsigned side = 0; ok && side < 2; ++side) {
		if (!b->carried[side]) continue;
		ArenaMemory m = { core_read32, core_write8, b->core };
		ok = arena_core_load(b->core, base, base_size) && arena_loadout_install(&m, &b->loadout[side], why, sizeof why) &&
			(state[side] = arena_core_save(b->core, &size[side])) != NULL;
	}
	arena_core_free(b->core);
	b->core = NULL;
	if (ok) {
		const uint8_t *from[2] = { state[0] ? state[0] : base, state[1] ? state[1] : base };
		const size_t n[2] = { state[0] ? size[0] : base_size, state[1] ? size[1] : base_size };
		const unsigned souls[2] = { b->carried[0] ? b->loadout[0].souls : 0u, b->carried[1] ? b->loadout[1].souls : 0u };
		b->pair = arena_pair_open(b->rom, b->rom_size, from, n, souls);
	}
	free(base);
	free(state[0]);
	free(state[1]);
	if (!ok) return fail(b, *why ? why : "the boot's state could not be kept");
	if (!b->pair) return fail(b, "the two cores could not be made (too little memory?)");
	b->step = b->in_step = 0;
	return b->stage = ARENA_BOOT_ENTRY;
}

/* ---- the course ---- */

ArenaBoot *arena_boot_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2]) {
	ArenaBoot *b = calloc(1, sizeof *b);
	if (!b) return NULL;
	b->rom = rom;
	b->rom_size = size;
	for (unsigned side = 0; side < 2; ++side) {
		if (!loadout || !loadout[side]) continue;
		b->loadout[side] = *loadout[side];
		b->carried[side] = true;
		if (!arena_loadout_check(loadout[side], b->error, sizeof b->error)) { b->stage = ARENA_BOOT_FAILED; return b; }
	}
	/* (full rules: BN5's installs as this game's core made them, which a game without them cannot give) */
	if (!xlog_hash()) { fail(b, "BN5's installs are not in this game's core"); return b; }
	printf("arena: full rules: BN5's installs in every core, %zu bytes and %d hooks (record %016llx)\n", xlog_bytes(), xlog_hooks(),
		(unsigned long long)xlog_hash());
	b->core = arena_core_new(rom, size, b->video, true);
	if (!b->core) { fail(b, "the boot core could not be made (too little memory?)"); return b; }
	b->hooks = hook_core(b->core);
	if (!b->hooks || !hook_on(b->hooks, BN6_OW_HOOK, state_update, b) || !hook_on(b->hooks, FREE_CALL_RETURN, returned, b))
		fail(b, "the boot core's hooks could not be set");
	return b;
}

void arena_boot_free(ArenaBoot *b) {
	if (!b) return;
	if (b->core) {
		if (b->hooks) hook_detach(b->core);
		arena_core_free(b->core);
	}
	arena_pair_close(b->pair);
	free(b);
}

/* a frame of the boot core's steps: false once they are done */
static bool boot_frame(ArenaBoot *b) {
	const Step *s = &boot_steps[b->step];
	if (s->act == ACT_END_INTRO) end_intro(b->core);
	else {
		if (s->act == ACT_WARP && b->in_step == 0) warp(b);
		b->core->setKeys(b->core, s->keys);
		b->core->runFrame(b->core);
		emu_audio_drop_from(b->core);   /* (the way in is not heard) */
		++b->done;
	}
	bool over = ++b->in_step >= s->frames || (s->act == ACT_WARP && b->call.called);
	if (over && s->act == ACT_WARP && !b->call.called) { fail(b, "the game's warp did not run"); return false; }
	if (over) { b->in_step = 0; ++b->step; }
	return b->step < NSTEPS(boot_steps);
}

/* a frame of the pair through the menus, both cores holding the step's keys */
static bool entry_frame(ArenaBoot *b) {
	const Step *s = &entry_steps[b->step];
	if (b->in_step == 0) arena_pair_anchor(b->pair);
	uint16_t keys[ARENA_LOOKAHEAD][2] = { { s->keys, s->keys } };
	uint64_t t = SDL_GetPerformanceCounter();
	if (!arena_pair_step(b->pair, keys, false)) { fail(b, "the cable stopped in BN6's menus"); return false; }
	double ms = (double)(SDL_GetPerformanceCounter() - t) * 1000.0 / (double)SDL_GetPerformanceFrequency();
	b->pair_ms += ms;
	if (ms > b->pair_worst) b->pair_worst = ms;
	++b->pair_frames;
	arena_pair_mute(b->pair);
	++b->done;
	if (++b->in_step >= s->frames) { b->in_step = 0; ++b->step; }
	return b->step < NSTEPS(entry_steps);
}

static ArenaBootStage at_custom(ArenaBoot *b) {
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = arena_pair_core(b->pair, side);
		if (c->rawRead8(c, BN6_BATTLE_PHASE, -1) != BN6_PHASE_CUSTOM) return fail(b, "the Practice battle's Custom screen did not open");
	}
	arena_pair_anchor(b->pair);
	arena_pair_begin(b->pair);
	return b->stage = ARENA_BOOT_READY;
}

ArenaBootStage arena_boot_run(ArenaBoot *b, unsigned frames) {
	for (unsigned i = 0; i < frames; ++i) {
		if (b->stage == ARENA_BOOT_POWER && !boot_frame(b) && b->stage == ARENA_BOOT_POWER) make_pair(b);
		else if (b->stage == ARENA_BOOT_ENTRY && !entry_frame(b) && b->stage == ARENA_BOOT_ENTRY) at_custom(b);
		if (b->stage == ARENA_BOOT_READY || b->stage == ARENA_BOOT_FAILED) break;
	}
	return b->stage;
}

static unsigned steps_frames(const Step *s, size_t n) {
	unsigned total = 0;
	for (size_t i = 0; i < n; ++i) total += s[i].act == ACT_WARP ? 1 : s[i].frames;
	return total;
}

unsigned arena_boot_total(void) { return steps_frames(boot_steps, NSTEPS(boot_steps)) + steps_frames(entry_steps, NSTEPS(entry_steps)); }

ArenaPair *arena_boot_take(ArenaBoot *b) {
	if (b->stage != ARENA_BOOT_READY) return NULL;
	ArenaPair *p = b->pair;
	b->pair = NULL;
	return p;
}

double arena_boot_frame_ms(const ArenaBoot *b, bool worst) {
	return worst ? b->pair_worst : b->pair_frames ? b->pair_ms / b->pair_frames : 0;
}

#else   /* no Arena in this build (arena_pair.h, CW_ARENA) */

struct ArenaBoot { ArenaBootStage stage; };
double arena_boot_frame_ms(const ArenaBoot *b, bool worst) { (void)b; (void)worst; return 0; }

ArenaBoot *arena_boot_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2]) {
	(void)rom; (void)size; (void)loadout;
	ArenaBoot *b = calloc(1, sizeof *b);
	if (b) b->stage = ARENA_BOOT_FAILED;
	return b;
}
void arena_boot_free(ArenaBoot *b) { free(b); }
ArenaBootStage arena_boot_run(ArenaBoot *b, unsigned frames) { (void)frames; return b->stage; }
unsigned arena_boot_total(void) { return 1; }
ArenaPair *arena_boot_take(ArenaBoot *b) { (void)b; return NULL; }

#endif

ArenaBootStage arena_boot_stage(const ArenaBoot *b) { return b->stage; }

const char *arena_boot_error(const ArenaBoot *b) {
#if CW_ARENA
	return b->error;
#else
	(void)b;
	return "This build has no Arena.";
#endif
}

unsigned arena_boot_done(const ArenaBoot *b) {
#if CW_ARENA
	return b->done;
#else
	(void)b;
	return 0;
#endif
}

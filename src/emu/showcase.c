/* showcase.h. A scene's way: its record written into the core's ROM copy
 * (BN6's BattleSettings, as encounter.c writes the game's), the battle
 * forced from the overworld's check straight to StartBattle (encounter.c's
 * forced hook, on this core's own table), its opening keys from the Custom
 * screen to where the loop begins, that moment kept, then the loop's keys
 * and the kept moment again. */
#include "showcase.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__3DS__) && !defined(CW_GPSP)

#include <mgba/core/core.h>

#include "arena_pair.h"
#include "bn6.h"
#include "bn6_panels.h"
#include "core_boot.h"
#include "data.h"
#include "emu.h"
#include "free_space.h"
#include "hook.h"
#include "rom.h"

/* a stretch of frames with these keys held */
typedef struct { uint16_t frames, keys; } Beat;

#define FOES 3
typedef struct {
	uint8_t field, backdrop, player;   /* the record's battlefield, background and MegaMan's panel (row << 4 | column, from 1) */
	struct { uint8_t family, version, panel; } foe[FOES];   /* (family 0: none) */
	const Beat *open;   /* from the Custom screen to the loop's start */
	const Beat *loop;   /* the loop, its last beat's frames 0 */
	int fx, fy;         /* the action's middle in the picture */
} Scene;

/* (each list ends with a beat of 0 frames) */
/* the Custom screen, once it has slid in: its first chip (Cannon in the
 * game's first folder) taken; then Right and A in turn till OK is pressed
 * (open_frame) */
static const Beat open_custom[] = { { 90, 0 }, { 6, KEY_A }, { 20, 0 }, { 0, 0 } };
/* MegaMan's Cannon, then the buster from each row in turn */
static const Beat loop_colour[] = { { 10, 0 }, { 6, KEY_A }, { 50, 0 }, { 6, KEY_B }, { 30, 0 }, { 6, KEY_UP }, { 24, 0 }, { 6, KEY_B },
	{ 30, 0 }, { 6, KEY_DOWN }, { 24, 0 }, { 6, KEY_B }, { 30, 0 }, { 6, KEY_DOWN }, { 24, 0 }, { 6, KEY_B }, { 30, 0 }, { 6, KEY_UP }, { 24, 0 }, { 0, 0 } };
/* MegaMan stands in the Mettaur's row: its wave hits him, and he flickers */
static const Beat loop_flicker[] = { { 300, 0 }, { 0, 0 } };
/* MegaMan across his row at a steady beat, the buster at each stop */
#define HOP(k) { 6, k }, { 8, 0 }, { 4, KEY_B }, { 6, 0 }
static const Beat loop_motion[] = { HOP(KEY_RIGHT), HOP(KEY_LEFT), HOP(KEY_LEFT), HOP(KEY_RIGHT), HOP(KEY_RIGHT), HOP(KEY_LEFT), HOP(KEY_LEFT), HOP(KEY_RIGHT), { 0, 0 } };
#undef HOP

static const Scene scenes[SCENE_COUNT] = {
	[SCENE_COLOUR] = { 0, 0x07, 0x23, { { 1, 0, 0x24 }, { 2, 0, 0x15 }, { 14, 0, 0x34 } }, open_custom, loop_colour, 120, 92 },
	[SCENE_FLICKER] = { 0, 0x0B, 0x23, { { 1, 0, 0x25 }, { 0, 0, 0 }, { 0, 0, 0 } }, open_custom, loop_flicker, 110, 96 },
	[SCENE_MOTION] = { 0, 0x04, 0x22, { { 1, 0, 0x35 }, { 2, 0, 0x16 }, { 0, 0, 0 } }, open_custom, loop_motion, 66, 86 },
};

/* frames after BATTLE START!'s banner has gone where a scene's loop begins */
#define SETTLE 110

enum { ST_NONE, ST_BOOT, ST_STAGE, ST_OPEN, ST_LOOP, ST_FAILED };

static struct {
	int stage;
	struct mCore *core;
	CoreBoot boot;
	HookCore *hooks;
	uint32_t video[EMU_W * EMU_H];
	uint8_t *base;      /* the boot's end: MegaMan on the net (RAM alone) */
	size_t base_size;
	uint8_t *loop[SCENE_COUNT];   /* each scene's loop start, kept once staged */
	size_t loop_size[SCENE_COUNT];
	int want, staged;   /* the scene wanted, and the one staged (-1 none) */
	int beat, in_beat;  /* where its keys are */
	int frames;         /* frames of the staging so far */
	int fight;          /* frames since the fight began (BN6_PHASE_FIGHT) */
	uint32_t made;      /* pictures made */
	bool forcing;
} S = { .staged = -1 };

/* frames of the boot or the staging run in one of the game's: the way in
 * as fast as it goes without the panel stalling */
#define SLICE 24
#define RECORD (FREE_RECORDS + 0x100u)   /* (the game's two records' room: this core runs none of the game's) */

static void put32(uint8_t *p, uint32_t v) {
	for (unsigned i = 0; i < 4; ++i) p[i] = (uint8_t)(v >> (8 * i));
}

/* the overworld's check straight to StartBattle with the scene's record
 * (encounter.c's forced) */
static HookAct forced(HookRegs *r, void *user) {
	(void)user;
	if (!S.forcing) return HOOK_CONTINUE;
	r->r[0] = RECORD;
	r->r[12] = BN6_ENCOUNTER_START;
	return HOOK_JUMP;
}

static void write_record(const Scene *sc) {
	struct mCore *c = S.core;
	uint8_t list[4 * (FOES + 1) + 1], *p = list;
	*p++ = 0x00; *p++ = sc->player; *p++ = 0; *p++ = 0;
	for (int i = 0; i < FOES; ++i) {
		if (!sc->foe[i].family) continue;
		int id = enemy_id(0, sc->foe[i].family, sc->foe[i].version);
		if (id < 0) continue;
		*p++ = 0x11; *p++ = sc->foe[i].panel; *p++ = (uint8_t)id; *p++ = (uint8_t)(id >> 8);
	}
	*p++ = 0xF0;
	uint8_t s[16] = { sc->field, 0x36, 0x15, 0x00, sc->backdrop, 0x00, BN6_SIDES_PLAIN, 0x00 };
	put32(s + 8, 0x000049E2);
	put32(s + 12, RECORD + 0x20);
	for (size_t i = 0; i < sizeof s; ++i) c->rawWrite8(c, RECORD + (uint32_t)i, -1, s[i]);
	for (long i = 0; i < p - list; ++i) c->rawWrite8(c, RECORD + 0x20 + (uint32_t)i, -1, list[i]);
}

static void fail(const char *why) {
	printf("showcase: %s\n", why);
	S.stage = ST_FAILED;
}

static void run(uint32_t keys) {
	S.core->setKeys(S.core, keys);
	S.core->runFrame(S.core);
	emu_audio_drop_from(S.core);
	++S.made;
}

static bool make(void) {
	if (!R.data) return false;
	if (!(S.core = arena_core_new(R.data, ROM_SIZE, S.video, false))) return false;
	if (!core_boot_begin(&S.boot, S.core)) return false;
	S.stage = ST_BOOT;
	return true;
}

/* the boot's end kept, and the battles' hook set */
static void booted(void) {
	core_boot_end(&S.boot);
	S.base = arena_core_save(S.core, &S.base_size);
	S.hooks = hook_core(S.core);
	if (!S.base || !S.hooks || !hook_on(S.hooks, BN6_ENCOUNTER_CHECK, forced, NULL)) { fail("its battles could not be set up"); return; }
	S.stage = ST_STAGE;
	S.staged = -1;
}

/* the wanted scene from its loop's start where it was staged before,
 * else begun from the boot's end */
static void stage(void) {
	S.staged = S.want;
	S.beat = S.in_beat = 0;
	if (S.loop[S.want]) {
		if (!arena_core_load(S.core, S.loop[S.want], S.loop_size[S.want])) fail("the scene's loop did not load");
		return;
	}
	if (!arena_core_load(S.core, S.base, S.base_size)) { fail("the boot's state did not load"); return; }
	write_record(&scenes[S.want]);
	S.forcing = true;
	S.frames = 0;
	S.fight = 0;
	S.stage = ST_OPEN;
}

/* a frame of the scene's beats: false once its list is over */
static bool beat_frame(const Beat *b) {
	if (!b[S.beat].frames) return false;
	run(b[S.beat].keys);
	if (++S.in_beat >= b[S.beat].frames) { S.in_beat = 0; ++S.beat; }
	return true;
}

/* the battle's way to its Custom screen, then the opening's keys, then the loop's start kept */
static void open_frame(void) {
	if (S.forcing) {
		run(0);
		if (S.core->rawRead8(S.core, BN6_BATTLE_PHASE, -1) == BN6_PHASE_CUSTOM) S.forcing = false;
		else if (++S.frames > 900) fail("its battle did not begin");
		return;
	}
	if (beat_frame(scenes[S.staged].open)) return;
	/* (the cursor on along the hand and A, till OK closes the Custom
	 * screen: chips taken on the way, the Custom screen slid out and
	 * BATTLE START! gone) */
	if (S.fight < SETTLE) {
		int phase = S.core->rawRead8(S.core, BN6_BATTLE_PHASE, -1), t = S.frames % 32;
		run(phase != BN6_PHASE_CUSTOM ? 0 : t < 6 ? KEY_RIGHT : t >= 16 && t < 22 ? KEY_A : 0);
		if (phase == BN6_PHASE_FIGHT) ++S.fight;
		else if (++S.frames > 1800) fail("its fight did not begin");
		return;
	}
	if (!(S.loop[S.staged] = arena_core_save(S.core, &S.loop_size[S.staged]))) { fail("the scene's loop could not be kept"); return; }
	S.beat = S.in_beat = 0;
	S.stage = ST_LOOP;
}

static void loop_frame(void) {
	if (beat_frame(scenes[S.staged].loop)) return;
	/* (its end: the loop's start again, and its first frame) */
	if (!arena_core_load(S.core, S.loop[S.staged], S.loop_size[S.staged])) { fail("the scene's loop did not load"); return; }
	S.beat = S.in_beat = 0;
	beat_frame(scenes[S.staged].loop);
}

bool showcase_here(void) { return true; }

void showcase_want(ShowScene s) {
	if ((unsigned)s >= SCENE_COUNT) return;
	S.want = s;
	if (S.stage == ST_NONE && !make()) fail("its core could not be made (too little memory?)");
}

void showcase_frame(void) {
	switch (S.stage) {
	case ST_BOOT:
		for (int i = 0; i < SLICE && S.stage == ST_BOOT; ++i) {
			int more = core_boot_frame(&S.boot);
			if (more < 0) fail("the game's warp did not run");
			else if (!more) booted();
		}
		break;
	case ST_STAGE: stage(); break;
	case ST_OPEN:
		for (int i = 0; i < SLICE && S.stage == ST_OPEN; ++i) open_frame();
		break;
	case ST_LOOP:
		if (S.want != S.staged) stage();
		else loop_frame();
		break;
	default: break;
	}
}

const uint32_t *showcase_picture(uint32_t *made) {
	*made = S.made;
	return S.stage == ST_LOOP ? S.video : NULL;
}

void showcase_focus(int *x, int *y) {
	const Scene *sc = &scenes[S.staged >= 0 ? S.staged : S.want];
	*x = sc->fx;
	*y = sc->fy;
}

void showcase_close(void) {
	if (S.core) {
		if (S.hooks || S.stage == ST_BOOT) hook_detach(S.core);
		arena_core_free(S.core);
	}
	free(S.base);
	for (int k = 0; k < SCENE_COUNT; ++k) free(S.loop[k]);
	memset(&S, 0, sizeof S);
	S.staged = -1;
}

#else   /* (the 3DS, and a build on gpSP: no second core) */

bool showcase_here(void) { return false; }
void showcase_want(ShowScene s) { (void)s; }
void showcase_close(void) {}
void showcase_frame(void) {}
const uint32_t *showcase_picture(uint32_t *made) { *made = 0; return NULL; }
void showcase_focus(int *x, int *y) { *x = 120; *y = 80; }

#endif

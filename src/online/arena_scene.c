/* --dev arena (issue #120, docs/ONLINE_ARENA.md): the Arena's engine on
 * this machine. The player is the cable's first GBA, from the keyboard or
 * pad; the second is a script over the local link (arena_link_local),
 * its keys held back `arenalag` ticks as a network would hold them. The
 * player's core's picture shows whole at the window's scale; the battle's
 * end, or the reset chord, goes back to the title. */
#include "arena.h"

#include <stdio.h>

#include <SDL.h>

#include "arena_carry.h"
#include "arena_view.h"
#include "arena_pair.h"
#include "arena_text.h"
#include "game.h"
#include "audio.h"
#include "bn6.h"
#include "devtools.h"
#include "emu.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"

#define DELAY_DEFAULT  6    /* frames of input delay: 100 ms */
#define LAG_DEFAULT    3    /* the local link's one way: 50 ms */
#define OVER_TICKS     300  /* the result shown before the title */
#define STALL_SHOWN    20   /* ticks waiting for the peer before it is said */

static struct {
	Arena *arena;
	ArenaLocalLink local;
	OnlineLoadout carried;
	int over_t, stalled;
} S;

/* The second player: idle in the battle; at the Custom screen, the
 * cursor taken right to OK and OK pressed, again every 160 frames until
 * the screen closes (the starter folder's five chips, then OK: the
 * research's script, tools/online_link_script.c) */
static uint16_t peer_keys(void *context, uint32_t frame) {
	(void)context;
	if (!S.arena || arena_phase(S.arena, 1) != BN6_PHASE_CUSTOM) return 0;
	unsigned t = frame % 160;
	if (t >= 40 && t < 100) return (t - 40) % 12 < 4 ? KEY_RIGHT : 0;
	return t >= 100 && t < 104 ? KEY_A : 0;
}

/* the player's buttons as the GBA's keys, bit for bit */
static uint16_t pad_keys(void) {
	static const int buttons[10] = { BTN_A, BTN_B, BTN_SELECT, BTN_START, BTN_RIGHT, BTN_LEFT, BTN_UP, BTN_DOWN, BTN_R, BTN_L };
	uint16_t k = 0;
	for (int i = 0; i < 10; ++i)
		if (btn_held((uint32_t)buttons[i])) k |= (uint16_t)(1u << i);
	return k;
}

static void enter(void) {
	S.over_t = S.stalled = 0;
	unsigned delay = dev.arena_delay > 0 ? (unsigned)dev.arena_delay : DELAY_DEFAULT;
	unsigned lag = dev.arena_lag >= 0 ? (unsigned)dev.arena_lag : LAG_DEFAULT;
	ArenaLink *link = arena_link_local(&S.local, delay, lag, peer_keys, NULL);
	/* (the run's MegaMan from its checkpoint where there is one; the
	 * second player the game's start's) */
	const OnlineLoadout *loadout[2] = { arena_carry_read(&S.carried) ? &S.carried : NULL, NULL };
	/* (arenahp=N: the carried MegaMan's max HP made N, the installer seen in the battle) */
	if (loadout[0] && dev.arena_hp > 0) S.carried.max_hp = S.carried.base_hp = (uint16_t)dev.arena_hp;
	printf("arena: delay %u frames, the local link's lag %u ticks, MegaMan %s\n", link->delay, lag,
		loadout[0] ? "carried from the checkpoint" : "the game's start's");
	S.arena = arena_new(R.data, ROM_SIZE, loadout, link);
	emu_audio_open();
	audio_external(emu_audio_read);
}

static void leave(void) {
	audio_external(NULL);
	arena_free(S.arena);
	S.arena = NULL;
	arena_view_free();
}

static bool reset_chord(void) {
	const uint32_t chord = BTN_A | BTN_B | BTN_SELECT | BTN_START;
	return (P.held & chord) == chord;
}

static void update(void) {
	if (!S.arena || reset_chord()) { scene_set(&scene_title); return; }
	switch (arena_stage(S.arena)) {
	case ARENA_CONNECTING: arena_view_connect(S.arena); break;
	case ARENA_BATTLE: {
		uint32_t stalls = arena_stalls(S.arena);
		arena_tick(S.arena, pad_keys());
		arena_sound(S.arena);
		S.stalled = arena_stalls(S.arena) != stalls ? S.stalled + 1 : 0;
		break;
	}
	case ARENA_OVER:
		if (++S.over_t >= OVER_TICKS || btn_pressed(BTN_A)) scene_set(&scene_title);
		break;
	case ARENA_FAILED:
		if (btn_pressed(BTN_A) || btn_pressed(BTN_B)) scene_set(&scene_title);
		break;
	}
}

static void draw(void) {
	arena_view_clear();
	if (!S.arena) return;
	ArenaStage stage = arena_stage(S.arena);
	const uint32_t *v = arena_video(S.arena);
	if (stage == ARENA_CONNECTING || (stage == ARENA_FAILED && !v)) {
		if (stage == ARENA_CONNECTING) arena_view_connecting(arena_progress(S.arena));
		else arena_view_stopped(arena_error(S.arena));
		return;
	}
	if (v) arena_view_core(v);
	if (stage == ARENA_OVER) arena_view_result(arena_result(S.arena), false);
	else if (stage == ARENA_FAILED) arena_view_stopped(arena_error(S.arena));
	else if (S.stalled >= STALL_SHOWN) arena_view_waiting();
}

const Scene scene_arena = { "arena", enter, update, draw, leave };

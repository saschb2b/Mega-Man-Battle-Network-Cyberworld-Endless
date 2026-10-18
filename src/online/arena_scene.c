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
#define HEADLESS_SLICE 64   /* the way in's frames a tick without a window (a capture's frames stay the same on every machine) */
#define SLICE          8
#define SLICE_MS       14   /* the way in's time a tick in a window */
#define OVER_TICKS     300  /* the result shown before the title */
#define STALL_SHOWN    20   /* ticks waiting for the peer before it is said */

static struct {
	Arena *arena;
	ArenaLocalLink local;
	OnlineLoadout carried;
	SDL_Texture *tex;
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
	printf("arena: delay %u frames, the local link's lag %u ticks, MegaMan %s\n", link->delay, lag,
		loadout[0] ? "carried from the checkpoint" : "the game's start's");
	S.arena = arena_new(R.data, ROM_SIZE, loadout, link);
	if (emu_ready()) audio_external(emu_audio_read);
}

static void leave(void) {
	if (emu_ready()) audio_external(NULL);
	arena_free(S.arena);
	S.arena = NULL;
	if (S.tex) SDL_DestroyTexture(S.tex);
	S.tex = NULL;
}

static void connect(void) {
	if (P.headless) { arena_connect(S.arena, HEADLESS_SLICE); return; }
	uint64_t start = SDL_GetPerformanceCounter(), budget = SDL_GetPerformanceFrequency() * SLICE_MS / 1000;
	while (arena_connect(S.arena, SLICE) == ARENA_CONNECTING && SDL_GetPerformanceCounter() - start < budget) {}
}

static bool reset_chord(void) {
	const uint32_t chord = BTN_A | BTN_B | BTN_SELECT | BTN_START;
	return (P.held & chord) == chord;
}

static void update(void) {
	if (!S.arena || reset_chord()) { scene_set(&scene_title); return; }
	switch (arena_stage(S.arena)) {
	case ARENA_CONNECTING: connect(); break;
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

static void draw_core(const uint32_t *v) {
#ifdef __3DS__
	const Uint32 format = SDL_PIXELFORMAT_ARGB8888;
#else
	const Uint32 format = SDL_PIXELFORMAT_ABGR8888;
#endif
	if (!S.tex) {
		S.tex = SDL_CreateTexture(P.renderer, format, SDL_TEXTUREACCESS_STREAMING, ARENA_W, ARENA_H);
		if (!S.tex) return;
		SDL_SetTextureScaleMode(S.tex, SDL_ScaleModeNearest);
		SDL_SetTextureBlendMode(S.tex, SDL_BLENDMODE_NONE);
	}
	/* (mGBA keeps layer flags in the top byte, which a renderer reads as alpha) */
	static uint32_t px[ARENA_W * ARENA_H];
	for (int i = 0; i < ARENA_W * ARENA_H; ++i) px[i] = v[i] | 0xFF000000u;
	SDL_UpdateTexture(S.tex, NULL, px, ARENA_W * 4);
	SDL_Rect dst = { P.core_x, P.core_y, ARENA_W, ARENA_H };
	SDL_RenderCopy(P.renderer, S.tex, NULL, &dst);
}

static void draw_note(const char *line, const char *second) {
	int y = P.core_y + ARENA_H - (second ? 28 : 16);
	fill_rect(P.core_x, y, ARENA_W, second ? 28 : 16, rgba(0, 0, 0, 200));
	text_draw(P.w / 2, y + 3, line, WHITE, TEXT_CENTER);
	if (second) text_draw(P.w / 2, y + 15, second, rgba(170, 200, 255, 255), TEXT_CENTER);
}

static void draw_connecting(void) {
	int cx = P.w / 2, y = P.core_y + 52;
	text_draw(cx, y, arena_title, rgba(170, 200, 255, 255), TEXT_CENTER);
	text_draw(cx, y + 16, arena_connecting, WHITE, TEXT_CENTER);
	int w = 160, done = w * arena_progress(S.arena) / 100;
	fill_rect(cx - w / 2, y + 36, w, 6, rgba(40, 60, 90, 255));
	fill_rect(cx - w / 2, y + 36, done, 6, rgba(120, 200, 255, 255));
}

static void draw(void) {
	fill_rect(0, 0, P.w, P.h, rgba(0, 0, 0, 255));
	if (!S.arena) return;
	ArenaStage stage = arena_stage(S.arena);
	const uint32_t *v = arena_video(S.arena);
	if (stage == ARENA_CONNECTING || (stage == ARENA_FAILED && !v)) {
		if (stage == ARENA_CONNECTING) draw_connecting();
		else draw_note(arena_stopped, arena_error(S.arena));
		return;
	}
	if (v) draw_core(v);
	unsigned result = arena_result(S.arena);
	if (stage == ARENA_OVER) draw_note(result == 1 ? arena_won : result == 2 ? arena_lost : arena_ended, arena_back);
	else if (stage == ARENA_FAILED) draw_note(arena_stopped, arena_error(S.arena));
	else if (S.stalled >= STALL_SHOWN) draw_note(arena_waiting, NULL);
}

const Scene scene_arena = { "arena", enter, update, draw, leave };

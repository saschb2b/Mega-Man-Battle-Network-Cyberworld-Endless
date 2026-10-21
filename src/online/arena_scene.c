/* --dev arena (issue #120, docs/ONLINE_ARENA.md): the Arena's engine on
 * this machine. The player is the cable's first GBA, from the keyboard or
 * pad; the second is a script over the local link (arena_link_local),
 * its keys held back `arenalag` ticks as a network would hold them. The
 * player's core's picture shows whole at the window's scale, the way in
 * and the result on the Square's PET screens (arena_card.h), the battle
 * with the Square's plate, badge and stall note (arena_view.h); the
 * card's A, or the reset chord, goes back to the title. */
#include "arena.h"

#include <stdio.h>

#include <SDL.h>

#include "arena_card.h"
#include "arena_carry.h"
#include "arena_folder.h"
#include "arena_format.h"
#include "arena_report.h"
#include "arena_team.h"
#include "arena_view.h"
#include "arena_text.h"
#include "game.h"
#include "audio.h"
#include "bn6.h"
#include "devtools.h"
#include "emu.h"
#include "gfx.h"
#include "platform.h"
#include "protocol.h"
#include "rom.h"
#include "xinstall.h"
#include "xinstall_log.h"

#define DELAY_DEFAULT  6    /* frames of input delay: 100 ms */
#define LAG_DEFAULT    3    /* the local link's one way: 50 ms */
#define OVER_TICKS     300  /* the result shown before the title */
#define FADE           12   /* frames of a fade, as the Square's (square_arena.c) */
#define STALL_NOTE     60   /* ticks waiting for the peer before it is said */
#define STALL_END      1200

static struct {
	Arena *arena;
	ArenaLocalLink local;
	OnlineLoadout carried;
	int over_t;
	uint32_t t;            /* frames of the screen shown: the panel, the battle, the card */
	ArenaStage was;
} S;

/* arenabusy's second player, by the frame's number alone (what the game
 * shows never asked, so that a run that rolls back and one that does not
 * hand him the same keys): every four seconds the cursor taken right to
 * OK and OK pressed (the Custom screen's, where it is open), between them
 * a step, a shot or the buster held a few frames at a time */
static uint16_t busy_keys(uint32_t frame) {
	unsigned t = frame % 240;
	if (t < 60) return t % 12 < 4 ? KEY_RIGHT : 0;
	if (t < 64) return KEY_A;
	static const uint16_t moves[8] = { 0, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, 0, KEY_B, KEY_B };
	uint32_t x = (frame / 6) * 2654435761u;
	x ^= x >> 13;
	return moves[(x * 2246822519u) >> 29];
}

/* The second player: idle in the battle; at the Custom screen, the
 * cursor taken right to OK and OK pressed, again every 160 frames until
 * the screen closes (the starter folder's five chips, then OK: the
 * research's script, tools/online_link_script.c) */
static uint16_t peer_keys(void *context, uint32_t frame) {
	(void)context;
	if (dev.arena_busy) return busy_keys(frame);
	if (!S.arena || arena_phase(S.arena, 1) != BN6_PHASE_CUSTOM) return 0;
	unsigned t = frame % 160;
	if (t >= 40 && t < 100) return (t - 40) % 12 < 4 ? KEY_RIGHT : 0;
	return t >= 100 && t < 104 ? KEY_A : 0;
}

/* --dev arena2v2's partners, players 2 and 3: arena_team's script, by the frame's number alone */
static uint16_t partner_keys(void *context, unsigned player, uint32_t frame) {
	(void)context;
	return arena_team_script(player, frame);
}

/* the player's buttons as the GBA's keys, bit for bit */
static uint16_t pad_keys(void) {
	static uint32_t tick;
	if (dev.arena_both) return S.arena ? arena_bot_keys(S.arena, 0, tick++) : 0;
	static const int buttons[10] = { BTN_A, BTN_B, BTN_SELECT, BTN_START, BTN_RIGHT, BTN_LEFT, BTN_UP, BTN_DOWN, BTN_R, BTN_L };
	uint16_t k = 0;
	for (int i = 0; i < 10; ++i)
		if (btn_held((uint32_t)buttons[i])) k |= (uint16_t)(1u << i);
	return k;
}

/* Full rules, every NetBattle's: BN5's installs made on the game's core
 * first as a run's start makes them (scene_emu.c, game_hooks.c), for the
 * pair to take (xinstall_log.h); without BN5's ROM the way in stops,
 * saying so */
static void installs(void) {
	/* (made once: --dev rollbench,arena made them before) */
	if (!XR[XROM_BN5_COLONEL_US].data || !emu_init(R.data, ROM_SIZE) || (!xlog_hash() && !xinstall_bn5()))
		printf("arena: BN5's installs not made: a NetBattle needs BN5's ROM beside BN6's\n");
}

static void enter(void) {
	S.over_t = 0;
	S.t = 0;
	S.was = ARENA_CONNECTING;
	unsigned delay = dev.arena_delay > 0 ? (unsigned)dev.arena_delay : DELAY_DEFAULT;
	unsigned lag = dev.arena_lag >= 0 ? (unsigned)dev.arena_lag : LAG_DEFAULT;
	ArenaLink *link = arena_link_local(&S.local, delay, lag, peer_keys, NULL);
	/* (--dev arena2v2: both partners on the local link too, a stream each) */
	if (dev.arena_2v2 > 0) arena_link_local_partners(&S.local, partner_keys, NULL);
	/* (the run's MegaMan from its checkpoint where there is one; the
	 * second player the game's start's) */
	installs();
	const OnlineLoadout *loadout[2] = { NULL, NULL };
	/* (the first folder that holds chips, as the Net Square's challenge carries him) */
	static OnlineLoadout all;
	if (arena_carry_read(&all)) {
		arena_format_carry(&all, arena_folder_first(arena_folders_fitting(&all)), &S.carried);
		loadout[0] = &S.carried;
	}
	/* (arenahp=N: the carried MegaMan's max HP made N, the installer seen in the battle) */
	if (loadout[0] && dev.arena_hp > 0) S.carried.max_hp = S.carried.base_hp = (uint16_t)dev.arena_hp;
	printf("arena: delay %u frames, the local link's lag %u ticks, MegaMan %s\n", link->delay, lag,
		loadout[0] ? "carried from the checkpoint" : "the game's start's");
	S.arena = arena_new(R.data, ROM_SIZE, loadout, link);
	/* (the stands' report read as the Square's battles read it, said in the log at the end) */
	arena_report_begin(1);
	/* (the local link's round trip: its one way there and back) */
	if (S.arena) arena_rtt(S.arena, lag * 2 * 1000 / 60);
	if (S.arena && dev.arena_roll >= 0) arena_rollback(S.arena, dev.arena_roll);
	if (S.arena) arena_window(S.arena, dev.arena_window);
	if (S.arena && dev.arena_2v2 > 0) arena_players(S.arena, ARENA_PLAYERS);
	if (S.arena && dev.arena_other) arena_show(S.arena, 1);
	emu_audio_open();
	audio_external(emu_audio_read);
	/* (the way in's panel opened as the Square's opens) */
	arena_card_cue(ARENA_CUE_OPEN);
}

static void leave(void) {
	audio_external(NULL);
	arena_free(S.arena);
	S.arena = NULL;
	arena_view_free();
}

/* the battle over: the title, or the game's end (quitend) */
static void done(void) {
	if (dev.quit_end) P.quit = true;
	else scene_set(&scene_title);
}

static bool reset_chord(void) {
	const uint32_t chord = BTN_A | BTN_B | BTN_SELECT | BTN_START;
	return (P.held & chord) == chord;
}

static void update(void) {
	if (!S.arena || reset_chord()) { scene_set(&scene_title); return; }
	ArenaStage stage = arena_stage(S.arena);
	if (stage != S.was) {
		S.was = stage;
		S.t = 0;
		arena_card_cue(stage == ARENA_BATTLE ? ARENA_CUE_BATTLE : ARENA_CUE_OPEN);
	}
	++S.t;
	switch (stage) {
	case ARENA_CONNECTING: arena_view_connect(S.arena); break;
	case ARENA_BATTLE:
		arena_tick(S.arena, pad_keys());
		arena_sound(S.arena);
		arena_report_frame(S.arena);
		if (arena_stage(S.arena) == ARENA_OVER) arena_report_end(arena_result(S.arena));
		/* (--dev arena2v2: the two GBAs' battles compared, iteration by iteration) */
		if (arena_stage(S.arena) != ARENA_BATTLE && dev.arena_2v2 > 0) arena_team_report();
		break;
	case ARENA_OVER:
		if (++S.over_t >= OVER_TICKS || btn_pressed(BTN_A)) done();
		break;
	case ARENA_FAILED:
		if (dev.quit_end || btn_pressed(BTN_A) || btn_pressed(BTN_B)) done();
		break;
	}
}

/* the result card: the battle's tally (the player the cable's first), or the stop's line */
static void card(ArenaStage stage) {
	ArenaTally t;
	arena_tally(S.arena, &t);
	ArenaCardView c = { .name = arena_other, .t = S.t, .tally = t.seen, .frames = t.fought };
	if (stage == ARENA_FAILED) {
		c.result = arena_video(S.arena) ? 4 : 5;
		c.why = arena_stop_line(arena_apart(S.arena) ? ARENA_STOP_APART : ARENA_STOP_ERROR, arena_other);
	} else c.result = arena_result(S.arena) == 1 || arena_result(S.arena) == 2 ? arena_result(S.arena) : 3;
	for (unsigned k = 0; k < 2; ++k) {
		c.hp[k] = t.hp[k];
		c.max_hp[k] = t.max_hp[k];
		c.used[k] = t.used[k];
	}
	arena_card_result(&c);
	arena_card_fade(255 - (int)S.t * 255 / FADE, false);
}

static void draw(void) {
	arena_view_clear();
	if (!S.arena) return;
	ArenaStage stage = arena_stage(S.arena);
	const uint32_t *v = arena_video(S.arena);
	if (stage == ARENA_CONNECTING) {
		ArenaWayView w = { .name = { arena_you, arena_other }, .known = { true, true }, .t = S.t,
			.wait = { ARENA_SIDE_LINKING, ARENA_SIDE_LINKING }, .progress = arena_progress(S.arena) };
		w.loadout[0] = S.carried.max_hp ? &S.carried : NULL;
		arena_card_way(&w);
		arena_card_fade(255 - (int)S.t * 255 / FADE, true);
		return;
	}
	if (stage != ARENA_BATTLE || !v) { card(stage); return; }
	arena_view_core(v);
	arena_view_over(arena_other, arena_phase(S.arena, 0) == BN6_PHASE_CUSTOM, arena_link_state(S.arena), S.t);
	uint32_t waited = arena_waited(S.arena);
	if (waited >= STALL_NOTE) arena_view_stall(arena_other, waited < STALL_END ? STALL_END - waited : 0, STALL_END - STALL_NOTE, false);
	arena_card_fade(255 - (int)S.t * 255 / FADE, true);
}

const Scene scene_arena = { "arena", enter, update, draw, leave };

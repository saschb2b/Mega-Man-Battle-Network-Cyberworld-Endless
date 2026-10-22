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
#include "arena_team_client.h"
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
	unsigned bouts;        /* fought before this one (arenaagain=N) */
	uint32_t bot_tick;     /* arenaboth's first player's clock, each bout's from 0 */
	uint64_t again_from;   /* a bout from the last one's pair: when it was asked for */
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

/* --dev arena2v2: on the Custom screen a lead first takes the chip under
 * the cursor, so that his hand holds one, before the script takes the
 * cursor to OK; in the fight side 0's lead taps R every 600 frames, which
 * opens the next turn's Custom screen once the gauge is full */
static uint16_t chip_first(unsigned side, uint32_t t, uint16_t keys) {
	if (dev.arena_2v2 <= 0 || !S.arena) return keys;
	if (arena_phase(S.arena, side) != BN6_PHASE_CUSTOM) return !side && t % 600 < 4 ? (uint16_t)(keys | KEY_R) : keys;
	/* (teambeast=3: the leads too take Beast Out each turn, their partners' control: the cursor right to OK, down to the
	 * emblem, A, then up and OK, again every 300 frames until the screen closes) */
	if (dev.team_beast >= 3) {
		unsigned b = t % 300;
		return b >= 40 && b < 100 ? ((b - 40) % 12 < 4 ? KEY_RIGHT : 0) : b >= 104 && b < 107 ? KEY_DOWN :
			(b >= 116 && b < 119) || (b >= 212 && b < 215) ? KEY_A : b >= 200 && b < 203 ? KEY_UP : 0;
	}
	return t % 160 >= 20 && t % 160 < 24 ? KEY_A : keys;
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
	return chip_first(1, frame, t >= 100 && t < 104 ? KEY_A : 0);
}

/* --dev arena2v2's partners, players 2 and 3: their own clients stood in
 * for (arena_team_client.h): BN6's own Custom screen on a core of each
 * one's while his side's GBA's is open, his picks in his stream; in the
 * battle arena_team's script, by the frame's number alone */
static uint16_t partner_keys(void *context, unsigned player, uint32_t frame) {
	(void)context;
	return S.arena ? team_client_keys(S.arena, player, frame) : 0;
}

/* the player's buttons as the GBA's keys, bit for bit */
static uint16_t pad_keys(void) {
	if (dev.arena_both && S.arena) {
		uint32_t t = S.bot_tick++;
		return chip_first(0, t, arena_bot_keys(S.arena, 0, t));
	}
	if (dev.arena_both) return 0;
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
/* the checkpoint's MegaMan as read, every folder of his (--dev arena2v2's partner takes another) */
static OnlineLoadout carried_all;

static void installs(void) {
	/* (made once: --dev rollbench,arena made them before) */
	if (!XR[XROM_BN5_COLONEL_US].data || !emu_init(R.data, ROM_SIZE) || (!xlog_hash() && !xinstall_bn5()))
		printf("arena: BN5's installs not made: a NetBattle needs BN5's ROM beside BN6's\n");
}

/* a bout: its link, the Arena from the last bout's pair where one is kept
 * (arenaagain=N), its seed (arenaseed=N, one more each bout) */
static void bout(ArenaKept *kept) {
	S.over_t = 0;
	S.t = 0;
	S.bot_tick = 0;
	S.was = ARENA_CONNECTING;
	unsigned delay = dev.arena_delay > 0 ? (unsigned)dev.arena_delay : DELAY_DEFAULT;
	unsigned lag = dev.arena_lag >= 0 ? (unsigned)dev.arena_lag : LAG_DEFAULT;
	ArenaLink *link = arena_link_local(&S.local, delay, lag, peer_keys, NULL);
	/* (--dev arena2v2: both partners on the local link too, a stream each) */
	if (dev.arena_2v2 > 0) arena_link_local_partners(&S.local, partner_keys, NULL);
	const OnlineLoadout *loadout[2] = { S.carried.max_hp ? &S.carried : NULL, NULL };
	uint32_t seed = dev.arena_seed ? (uint32_t)dev.arena_seed + S.bouts : 0;
	printf("arena: bout %u: delay %u frames, the local link's lag %u ticks, MegaMan %s, seed %u\n", S.bouts + 1, link->delay, lag,
		loadout[0] ? "carried from the checkpoint" : "the game's start's", (unsigned)seed);
	S.arena = arena_new_bout(R.data, ROM_SIZE, loadout, link, seed, kept);
	/* (the stands' report read as the Square's battles read it, said in the log at the end) */
	arena_report_begin(1);
	/* (the local link's round trip: its one way there and back) */
	if (S.arena) arena_rtt(S.arena, lag * 2 * 1000 / 60);
	if (S.arena && dev.arena_roll >= 0) arena_rollback(S.arena, dev.arena_roll);
	if (S.arena) arena_window(S.arena, dev.arena_window);
	if (S.arena && dev.arena_2v2 > 0) arena_players(S.arena, ARENA_PLAYERS);
	if (S.arena && dev.arena_other) arena_show(S.arena, 1);
	/* (--dev arena2v2: the partners' folders, a carried MegaMan's other folder for side 0's; teamseat=N's GBA shown) */
	if (S.arena && dev.arena_2v2 > 0) team_client_begin(S.arena, loadout[0] ? &carried_all : NULL);
	/* (the way in's panel opened as the Square's opens) */
	arena_card_cue(ARENA_CUE_OPEN);
}

static void enter(void) {
	S.bouts = 0;
	S.carried.max_hp = 0;
	/* (the run's MegaMan from its checkpoint where there is one; the
	 * second player the game's start's) */
	installs();
	/* (the first folder that holds chips, as the Net Square's challenge carries him) */
	if (arena_carry_read(&carried_all)) {
		arena_format_carry(&carried_all, arena_folder_first(arena_folders_fitting(&carried_all)), &S.carried);
		/* (arenahp=N: the carried MegaMan's max HP made N, the installer seen in the battle) */
		if (dev.arena_hp > 0) S.carried.max_hp = S.carried.base_hp = (uint16_t)dev.arena_hp;
		/* (a NetBattle begins whole; arenacarry=N: with N HP, as a Team NetBattle's winner goes on) */
		S.carried.hp = dev.arena_carry > 0 && dev.arena_carry < S.carried.max_hp ? (uint16_t)dev.arena_carry : S.carried.max_hp;
	}
	bout(NULL);
	emu_audio_open();
	audio_external(emu_audio_read);
}

/* arenaagain=N: the next bout from the last one's pair, kept as it began */
static void next_bout(void) {
	ArenaKept *kept = arena_keep(S.arena);
	arena_free(S.arena);
	S.arena = NULL;
	++S.bouts;
	uint64_t t = SDL_GetPerformanceCounter();
	bout(kept);
	S.again_from = t;
}

static void leave(void) {
	audio_external(NULL);
	team_client_free();
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
		if (stage == ARENA_BATTLE && S.again_from)
			printf("arena: bout %u ready %u ticks after the last one's end (%.0f ms here)\n", S.bouts + 1, (unsigned)S.t,
				(double)(SDL_GetPerformanceCounter() - S.again_from) * 1000.0 / (double)SDL_GetPerformanceFrequency());
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
		if (arena_stage(S.arena) != ARENA_BATTLE && dev.arena_2v2 > 0) {
			team_client_report();
		}
		break;
	case ARENA_OVER:
		if (S.bouts < (unsigned)(dev.arena_again > 0 ? dev.arena_again : 0)) next_bout();
		else if (++S.over_t >= OVER_TICKS || btn_pressed(BTN_A)) done();
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
	/* (--dev arena2v2: the seat's picture, a partner's BN6's own Custom screen on his client's copy of the pair while it is
	 * open, and the seat's own drawing over it) */
	if (dev.arena_2v2 > 0) team_client_draw(S.arena);
	else arena_view_core(v);
	arena_view_over(arena_other, arena_phase(S.arena, 0) == BN6_PHASE_CUSTOM, arena_link_state(S.arena), S.t);
	uint32_t waited = arena_waited(S.arena);
	if (waited >= STALL_NOTE) arena_view_stall(arena_other, waited < STALL_END ? STALL_END - waited : 0, STALL_END - STALL_NOTE, false);
	arena_card_fade(255 - (int)S.t * 255 / FADE, true);
}

const Scene scene_arena = { "arena", enter, update, draw, leave };

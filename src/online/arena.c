/* The Arena's local engine (arena.h).
 *
 * A key sampled at tick t is frame t + D's (D the link's delay); the
 * first D frames are released keys on both sides, on every client. Frame
 * N of the pair is due at tick N and needs both players' keys for N and
 * the frame after it (arena_input.h), each core taking those of its own
 * frame clock. With a rollback window W (issue #128, docs/ONLINE_ARENA.md,
 * Rollback) frame N runs at its tick with up to W frames on a guess: the
 * other player's keys not yet come are his last held, the pair's snapshot
 * kept before each frame that ran on one, and a key that comes otherwise
 * than guessed loads the snapshot before the first frame that read it and
 * runs the frames again, unheard, to where they were. Whatever a frame
 * left (its hash, the battle's end) is taken up only once both players'
 * keys for it are known: every client and spectator agrees on those. W 0
 * is the delay alone: a frame runs once its keys have all come.
 *
 * A frame due that may not run yet is a stall; the frames behind are
 * caught up CATCH_UP a tick. A stall moves my clock: the tick after it
 * samples nothing and runs the frame that waited, so a wait shows one
 * frame twice and never skips one; and the game that leads holds a tick
 * now and then, spread out, until it no longer does (arena_sync.h): a
 * stall is then a game truly fallen behind. */
#include "arena.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include <mgba/core/core.h>

#include "arena_boot.h"
#include "arena_pair.h"
#include "arena_roll.h"
#include "arena_sync.h"
#include "arena_window.h"
#include "bn6.h"

#define CATCH_UP 2       /* frames of the pair a tick may run */
/* frames my keys may run past the pair's next one: past it a tick samples
 * none and waits, so the other game held still (its app sent to the
 * background, its window dragged) stalls the battle rather than filling
 * the ring (ARENA_RING) and stopping it four seconds on */
#define LEAD     (ARENA_RING / 2)
#define PENDING  8       /* my hashes waiting for the peer's */
#define SETTLE   300     /* frames of the battle after which the clocks have settled (arena_sync.h): its uneven ticks counted apart */
#define LOG      64      /* frames whose outcome waits to be taken up: past the largest window and a tick's catch-up */

typedef struct { uint32_t frame; uint64_t hash; } Hashed;

/* what a frame of the pair left, read as it ran (and again where it ran again) */
typedef struct {
	uint64_t hash;      /* the pair's, at every ARENA_HASH_EVERY-th frame */
	bool fight, gone;   /* my core's battle under way; both cores' gone, back at the Comm menu */
	uint8_t result[2];  /* mine and the other's (BN6_BATTLE_RESULT) */
	ArenaReading reading;   /* both MegaMen as the cable's first core has them */
} Ran;

struct Arena {
	ArenaStage stage;
	ArenaLink *link;
	ArenaBoot *boot;
	ArenaPair *pair;
	ArenaInput input;
	ArenaSync sync;
	uint32_t tick, stalls, hashes;
	uint32_t ticks, uneven;    /* the battle's ticks, and those that ran no frame or more than one */
	uint32_t settled;          /* ... of them, once the clocks have settled (past SETTLE frames) */
	bool owed;                 /* a stall last tick: this one holds my clock and runs the frame that waited */
	int roll_window;           /* --dev arena's rollback tried (arena_roll.h): its frames; 0 the game's hash said alone; -1 none */
	ArenaRoll *roll;
	int window_asked;          /* arena_window: the rollback window set, -1 this system's own */
	unsigned window;           /* frames that may run on a guess: 0 the delay alone */
	ArenaPairSnap *snap[ARENA_WINDOW_MOST];   /* the pair before each frame that ran on a guess, by frame % window */
	Ran ran[LOG];              /* by frame % LOG */
	uint32_t done;             /* the frames taken up: their keys all known, their hashes sent, the end looked for */
	uint32_t rollbacks, again, deepest;   /* the rollbacks, the frames they ran again, the most at once */
	double cost, worst;        /* the pair's frames timed, ms */
	uint32_t costed;
	double tick_worst;         /* a tick of the battle's longest, ms */
	uint32_t late;             /* ticks longer than a 60th of a second */
	Hashed pending[PENDING];
	unsigned npending;
	ArenaReading reading;      /* the last settled frame's, for the stands' report */
	bool read;                 /* ... once one is */
	bool fought, apart;
	unsigned result;
	char error[160];
};

static ArenaStage fail(Arena *a, const char *why) {
	snprintf(a->error, sizeof a->error, "%s", why);
	printf("arena: stopped at frame %" PRIu32 ": %s\n", a->input.next, a->error);
	return a->stage = ARENA_FAILED;
}

Arena *arena_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2], ArenaLink *link) {
	Arena *a = calloc(1, sizeof *a);
	if (!a) return NULL;
	a->link = link;
	a->roll_window = -1;
	a->window_asked = -1;
	a->boot = arena_boot_new(rom, size, loadout);
	if (!a->boot) { free(a); return NULL; }
	if (arena_boot_stage(a->boot) == ARENA_BOOT_FAILED) fail(a, arena_boot_error(a->boot));
	return a;
}

void arena_free(Arena *a) {
	if (!a) return;
	arena_boot_free(a->boot);
	arena_roll_free(a->roll);
	for (unsigned i = 0; i < ARENA_WINDOW_MOST; ++i) arena_pair_snap_free(a->snap[i]);
	arena_pair_close(a->pair);
	if (a->link && a->link->close) a->link->close(a->link);
	free(a);
}

/* the window this battle rolls back: --dev arena's rollback tried has its
 * own; one set (arena_window); else what a frame of the pair costs here,
 * as the way in timed it; a snapshot made for each of its frames */
static bool open_window(Arena *a) {
	double ms = arena_boot_frame_ms(a->boot, false);
	if (a->roll) a->window = 0;
	else if (a->window_asked >= 0) a->window = (unsigned)a->window_asked < ARENA_WINDOW_MOST ? (unsigned)a->window_asked : ARENA_WINDOW_MOST;
	else a->window = arena_window_for(ms * ARENA_MENUS_TO_BATTLE);
	for (unsigned i = 0; i < a->window; ++i)
		if (!(a->snap[i] = arena_pair_snap_new(a->pair))) return false;
	printf("arena: a frame of the pair %.2f ms here (the worst %.2f): rolling back %u frames at most\n", ms,
		arena_boot_frame_ms(a->boot, true), a->window);
	return true;
}

ArenaStage arena_connect(Arena *a, unsigned frames) {
	if (a->stage != ARENA_CONNECTING) return a->stage;
	ArenaBootStage s = arena_boot_run(a->boot, frames);
	if (s == ARENA_BOOT_FAILED) return fail(a, arena_boot_error(a->boot));
	if (s != ARENA_BOOT_READY) return a->stage;
	a->pair = arena_boot_take(a->boot);
	printf("arena: both cores at the Custom screen after %u frames, %u cable transfers; delay %u frames\n",
		arena_boot_done(a->boot), arena_pair_transfers(a->pair), a->link->delay);
	for (unsigned side = 0; side < 2; ++side) {
		struct mCore *c = arena_pair_core(a->pair, side);
		uint32_t folder = c->rawRead32(c, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, -1);
		printf("arena: side %u: HP %u, the folder's first chips %04x %04x %04x\n", side, c->rawRead16(c, BN6_NAVI_MAX_HP, -1),
			c->rawRead16(c, folder, -1), c->rawRead16(c, folder + 2, -1), c->rawRead16(c, folder + 4, -1));
	}
	arena_input_init(&a->input, a->link->delay);
	arena_sync_init(&a->sync);
	if (a->roll_window > 0 && !(a->roll = arena_roll_new(a->pair, (unsigned)a->roll_window))) return fail(a, "no room for the rollback's snapshots");
	bool room = open_window(a);
	arena_boot_free(a->boot);
	a->boot = NULL;
	if (!room) return fail(a, "no room for the rollback's snapshots");
	return a->stage = ARENA_BATTLE;
}

static struct mCore *core(const Arena *a, unsigned side) { return a->pair ? arena_pair_core(a->pair, side) : NULL; }

/* the core back at the PET's Comm menu, where a NetBattle returns */
static bool at_comm(struct mCore *c) {
	uint32_t main = c->rawRead32(c, BN6_TOOLKIT, -1), menu = c->rawRead32(c, BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU, -1);
	return c->rawRead8(c, main, -1) == BN6_MODE_SUBMENU && c->rawRead8(c, menu, -1) == BN6_SUBMENU_COMM;
}

/* the peer's keys as they come, each its stream's next */
static bool take_peer(Arena *a) {
	unsigned peer = a->link->side ^ 1;
	uint16_t keys;
	while (a->link->poll(a->link, a->input.known[peer], &keys)) {
		/* (its lag: my tick now, less the tick the other game sampled it on) */
		arena_sync_arrived(&a->sync, (int32_t)(a->tick - 1 - (a->input.known[peer] - a->input.delay)));
		if (arena_input_put(&a->input, peer, a->input.known[peer], keys) != ARENA_PUT_OK) return false;
	}
	return true;
}

/* a core whose NetBattle is decided and gone, back at the PET's Comm menu
 * (which it reads through the battle too: the result byte, 0 until the
 * battle is decided, and the battle's mode say when it has ended) */
static bool battle_gone(struct mCore *c) {
	return c->rawRead8(c, BN6_BATTLE_RESULT, -1) && c->rawRead8(c, BN6_BATTLE_MODE, -1) == BN6_BATTLE_GONE && at_comm(c);
}

/* both MegaMen as core `c` has them (each side's player object, BN6_BATTLE_PLAYERS) */
static void read_sides(struct mCore *c, ArenaReading *out) {
	memset(out, 0, sizeof *out);
	for (unsigned side = 0; side < 2; ++side) {
		uint32_t o = c->rawRead32(c, BN6_BATTLE_PLAYERS + side * BN6_BATTLE_PLAYER_STEP, -1);
		if (o < BN6_EWRAM || o >= BN6_EWRAM_END - BN6_T1_SIZE) continue;
		out->seen[side] = true;
		out->hp[side] = (uint16_t)c->rawRead16(c, o + BN6_T1_HP, -1);
		out->max_hp[side] = (uint16_t)c->rawRead16(c, o + BN6_T1_MAX_HP, -1);
		out->action[side] = (uint8_t)c->rawRead8(c, o + BN6_T1_ACTION, -1);
		out->chip[side] = (uint16_t)c->rawRead16(c, o + BN6_T1_CHIP, -1);
	}
}

/* what frame `frame` left, noted as it ran */
static void note(Arena *a, uint32_t frame) {
	Ran *r = &a->ran[frame % LOG];
	struct mCore *mine = arena_pair_core(a->pair, a->link->side), *theirs = arena_pair_core(a->pair, a->link->side ^ 1);
	r->fight = mine->rawRead8(mine, BN6_BATTLE_PHASE, -1) == BN6_PHASE_FIGHT;
	r->gone = battle_gone(mine) && battle_gone(theirs);
	r->result[0] = mine->rawRead8(mine, BN6_BATTLE_RESULT, -1);
	r->result[1] = theirs->rawRead8(theirs, BN6_BATTLE_RESULT, -1);
	r->hash = (frame + 1) % ARENA_HASH_EVERY == 0 ? arena_pair_hash(a->pair) : 0;
	read_sides(arena_pair_core(a->pair, 0), &r->reading);
}

/* the pair's next frame: its snapshot kept where it runs on a guess, its
 * keys taken (the other's not yet come guessed), what it left noted */
static bool run_frame(Arena *a) {
	uint32_t frame = a->input.next;
	if (a->window && frame >= arena_input_settled(&a->input) && !arena_pair_save(a->pair, a->snap[frame % a->window])) return false;
	uint16_t keys[ARENA_LOOKAHEAD][2];
	if (!arena_input_take(&a->input, a->window, keys)) return false;
	if (a->roll) arena_roll_before(a->roll, frame, (const uint16_t (*)[2])keys);
	uint64_t t = SDL_GetPerformanceCounter();
	if (!arena_pair_step(a->pair, (const uint16_t (*)[2])keys, true)) return false;
	double ms = (double)(SDL_GetPerformanceCounter() - t) * 1000.0 / (double)SDL_GetPerformanceFrequency();
	a->cost += ms;
	if (ms > a->worst) a->worst = ms;
	++a->costed;
	if (a->roll && !arena_roll_after(a->roll, frame)) return false;
	note(a, frame);
	return true;
}

/* a key come otherwise than guessed: the pair back before the first frame
 * that read it, and the frames since run again, unheard, with what is
 * known now; false where the pair could not be (a snapshot or the cable) */
static bool roll_back(Arena *a) {
	uint32_t from = arena_input_wrong(&a->input), until = a->input.next;
	if (from == ARENA_NO_FRAME) return true;
	arena_input_rewind(&a->input, from);
	if (from >= until) return true;
	if (!a->window || from < a->done || !arena_pair_load(a->pair, a->snap[from % a->window])) return false;
	while (a->input.next < until)
		if (!run_frame(a)) return false;
	arena_pair_mute(a->pair);
	++a->rollbacks;
	a->again += until - from;
	if (until - from > a->deepest) a->deepest = until - from;
	return true;
}

/* my hash for a frame settled, and the peer's for the ones before */
static void hash_out(Arena *a, uint32_t frame, uint64_t h) {
	a->link->send_hash(a->link, frame, h);
	if (a->npending == PENDING) {   /* (the oldest given up: a peer that sends none is not a difference) */
		memmove(a->pending, a->pending + 1, sizeof a->pending - sizeof *a->pending);
		--a->npending;
	}
	a->pending[a->npending++] = (Hashed){ frame, h };
	printf("arena: frame %" PRIu32 " hash %016" PRIx64 "\n", frame, h);
	/* (--dev arena's rollback tried compares these: the game's state, without the sound's fields) */
	if (a->roll_window >= 0) printf("arena: frame %" PRIu32 " the game's hash %016" PRIx64 "\n", frame, h);
}

static void hashes_in(Arena *a) {
	for (unsigned i = 0; i < a->npending && a->stage == ARENA_BATTLE;) {
		uint64_t theirs;
		if (!a->link->poll_hash(a->link, a->pending[i].frame, &theirs)) { ++i; continue; }
		if (theirs != a->pending[i].hash) {
			char why[96];
			snprintf(why, sizeof why, "the two games went apart by frame %" PRIu32, a->pending[i].frame);
			a->apart = true;
			fail(a, why);
			return;
		}
		++a->hashes;
		memmove(a->pending + i, a->pending + i + 1, (a->npending - i - 1) * sizeof *a->pending);
		--a->npending;
	}
}

/* the battle seen under way, then both games out of it: over */
static void over(Arena *a, uint32_t frame, const Ran *r) {
	a->result = r->result[0];
	printf("arena: over at frame %" PRIu32 ": result %u and the other's %u (1 won, 2 lost), %" PRIu32 " ticks stalled, %" PRIu32
		" held, %" PRIu32 " of %" PRIu32 " uneven (%" PRIu32 " after frame %u), %" PRIu32 " hashes agreed, %u cable transfers\n",
		frame, a->result, r->result[1], a->stalls, a->sync.held, a->uneven, a->ticks, a->settled, SETTLE, a->hashes,
		arena_pair_transfers(a->pair));
	printf("arena: window %u, delay %u: %" PRIu32 " rollbacks, %" PRIu32 " frames run again, %" PRIu32
		" the most at once; a frame of the pair %.2f ms (the worst %.2f), a tick at worst %.2f ms, %" PRIu32 " past a 60th\n",
		a->window, a->input.delay, a->rollbacks, a->again,
		a->deepest, a->costed ? a->cost / a->costed : 0, a->worst, a->tick_worst, a->late);
	/* (what a frame costs here, kept for the window the next challenge asks by) */
	if (a->costed) arena_window_keep(a->cost / a->costed);
	a->stage = ARENA_OVER;
}

/* the frames whose keys are all known, taken up in order: their hashes
 * sent, the battle's end looked for; then the peer's hashes compared */
static void settle(Arena *a) {
	uint32_t until = arena_input_settled(&a->input);
	if (until > a->input.next) until = a->input.next;
	for (; a->done < until && a->stage == ARENA_BATTLE; ++a->done) {
		const Ran *r = &a->ran[a->done % LOG];
		a->reading = r->reading;
		a->read = true;
		if ((a->done + 1) % ARENA_HASH_EVERY == 0) hash_out(a, a->done, r->hash);
		if (r->fight) a->fought = true;
		if (a->fought && r->gone) over(a, a->done + 1, r);
	}
	hashes_in(a);
}

/* my clock held this tick, nothing sampled: after a stall, or the leading game's */
static bool hold(Arena *a) {
	bool held = a->owed;
	if (held) arena_sync_shift(&a->sync);
	else if ((held = arena_sync_hold(&a->sync)))
		printf("arena: a tick held at frame %" PRIu32 ", the other's keys' lag past %u\n", a->input.next, arena_sync_target(&a->sync));
	a->owed = false;
	return held;
}

static ArenaStage tick(Arena *a, uint16_t keys) {
	ArenaLink *l = a->link;
	l->tick(l);
	++a->ticks;
	bool held = hold(a);
	uint32_t now = a->tick - 1;
	if (!held && a->tick + a->input.delay < a->input.next + LEAD) {
		uint32_t frame = (now = a->tick++) + a->input.delay;
		keys &= ARENA_KEYS_MASK;
		if (arena_input_put(&a->input, l->side, frame, keys) != ARENA_PUT_OK || !l->send(l, frame, keys))
			return fail(a, "my keys could not be sent");
	}
	if (!take_peer(a)) return fail(a, "the other player's keys came out of order");
	const char *broken = l->broken(l);
	if (broken) return fail(a, broken);
	if (!roll_back(a)) return fail(a, "the cable between the two cores stuck");
	bool settled = a->input.next >= SETTLE;
	uint32_t ran = a->input.next;
	if (a->input.next <= now && !arena_input_ready(&a->input, a->window)) { ++a->stalls; a->owed = true; }
	for (unsigned i = 0; i < CATCH_UP && a->input.next <= now && arena_input_ready(&a->input, a->window); ++i)
		if (!run_frame(a)) return fail(a, "the cable between the two cores stuck");
	settle(a);
	if (a->input.next - ran != 1) { ++a->uneven; a->settled += settled; }
	return a->stage;
}

ArenaStage arena_tick(Arena *a, uint16_t keys) {
	if (a->stage != ARENA_BATTLE) return a->stage;
	uint64_t t = SDL_GetPerformanceCounter();
	ArenaStage s = tick(a, keys);
	double ms = (double)(SDL_GetPerformanceCounter() - t) * 1000.0 / (double)SDL_GetPerformanceFrequency();
	if (ms > a->tick_worst) a->tick_worst = ms;
	a->late += ms > 1000.0 / 60;
	return s;
}

void arena_rtt(Arena *a, uint32_t ms) { arena_sync_rtt(&a->sync, ms); }
void arena_rollback(Arena *a, int window) { a->roll_window = window; }
void arena_window(Arena *a, int window) { a->window_asked = window; }

ArenaStage arena_stage(const Arena *a) { return a->stage; }
const char *arena_error(const Arena *a) { return a->error; }

int arena_progress(const Arena *a) {
	if (a->stage != ARENA_CONNECTING) return 100;
	unsigned done = arena_boot_done(a->boot), total = arena_boot_total();
	return done >= total ? 99 : (int)(done * 100 / total);
}

const uint32_t *arena_video(const Arena *a) { return a->pair ? arena_pair_video(a->pair, a->link->side) : NULL; }

unsigned arena_phase(const Arena *a, unsigned side) {
	struct mCore *c = core(a, side);
	return c ? c->rawRead8(c, BN6_BATTLE_PHASE, -1) : 0;
}

unsigned arena_result(const Arena *a) { return a->result; }
bool arena_apart(const Arena *a) { return a->apart; }
uint32_t arena_stalls(const Arena *a) { return a->stalls; }

const ArenaReading *arena_reading(const Arena *a) { return a->read ? &a->reading : NULL; }

uint64_t arena_hash(Arena *a) { return a->pair ? arena_pair_hash(a->pair) : 0; }
uint64_t arena_record(const Arena *a) { return a->pair ? arena_pair_record(a->pair) : 0; }

void arena_sound(Arena *a) {
	if (a->pair) arena_pair_sound(a->pair, a->link->side);
}

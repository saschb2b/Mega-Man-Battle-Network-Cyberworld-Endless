/* A NetBattle watched from the stands (arena_watch.h).
 *
 * The pair runs as a player's does (arena.c): frame N once both players'
 * keys for N and the frame after it are known (ARENA_LOOKAHEAD), each core taking
 * those of its own frame clock. A spectator's keys come all at once where
 * he came in late, so it runs as many frames a tick as the frame's time
 * allows while it is more than BEHIND_KEPT frames behind the keys known, then one
 * a tick, and waits within BEHIND_LEAST of them. Catching up that gains nothing in
 * TRY_TICKS ticks gives up: this machine cannot run the pair fast enough. */
#include "arena_watch.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <mgba/core/core.h>

#include "arena_boot.h"
#include "arena_loadout.h"
#include "arena_pair.h"
#include "arena_seats.h"
#include "arena_team.h"
#include "arena_wire.h"
#include "bn6.h"
#include "devtools.h"

#define BEHIND_KEPT            90    /* frames behind the keys known it keeps: a second and a half */
#define BEHIND_LEAST           20    /* ... nearer than this, it waits for more */
#define FAST_MS        12    /* a tick's time for catching up, in a window */
#define FAST_HEADLESS  8     /* ... frames a tick without one (a capture's frames the same on every machine) */
#define SLICE          8     /* the way in's frames between two looks at the clock */
#define SLICE_MS       14
#define HEADLESS_SLICE 64
#define TRY_TICKS      600   /* catching up for so long without gaining: too slow */
#define HASH_EVERY     60    /* (arena.h's ARENA_HASH_EVERY: the players' hashes come at these frames) */
#define READY_FRAME    0xFFFFFFFFu
#define RECORD_FRAME   0xFFFFFFFEu

/* hashes by their frame (60 apart), mine and the cable's first's */
typedef struct { uint64_t *hash; uint8_t *known; uint32_t cap; } Hashes;

static bool hashes_put(Hashes *h, uint32_t at, uint64_t hash, uint8_t who) {
	if (at >= h->cap) {
		uint32_t cap = h->cap ? h->cap * 2 : 256;
		while (cap <= at) cap *= 2;
		uint64_t *grown = realloc(h->hash, cap * 2 * sizeof *grown);
		uint8_t *known = grown ? realloc(h->known, cap) : NULL;
		if (grown) h->hash = grown;
		if (!known) return false;
		memset(known + h->cap, 0, cap - h->cap);
		/* (two a slot: mine at 2*at, theirs at 2*at + 1) */
		h->known = known;
		h->cap = cap;
	}
	h->hash[2 * at + (who >> 1)] = hash;
	h->known[at] |= who;
	return true;
}

struct ArenaWatch {
	WatchingStage stage;
	const uint8_t *rom;
	size_t size;

	uint32_t match;
	unsigned seats;   /* 2, or a 2v2's 4 */
	uint8_t loadout[ONLINE_SEATS][ONLINE_ARENA_LOADOUT_MAX];
	size_t loadout_n[ONLINE_SEATS];
	bool loadout_in[ONLINE_SEATS], record_in, ready_in, ended, fought, fast;
	unsigned shown;   /* the side whose picture is drawn (arena_watch_show) */
	uint64_t record, ready;
	ArenaBoot *boot;
	ArenaPair *pair;
	uint64_t pair_ready;
	WatchKeys keys;
	Hashes hashes;
	uint32_t next, checked, fast_ticks, fast_from, fast_at;
	unsigned result;
	char error[160];
};

static WatchingStage fail(ArenaWatch *w, const char *why) {
	snprintf(w->error, sizeof w->error, "%s", why);
	printf("watch: stopped at frame %" PRIu32 ": %s\n", w->next, w->error);
	if (w->pair) arena_pair_mute(w->pair);
	return w->stage = WATCHING_FAILED;
}

ArenaWatch *arena_watch_new(const uint8_t *rom, size_t size, uint32_t match) {
	ArenaWatch *w = calloc(1, sizeof *w);
	if (!w) return NULL;
	w->match = match;
	w->rom = rom;
	w->size = size;
	w->seats = 2;

	return w;
}

void arena_watch_four(ArenaWatch *w) {
	w->seats = ONLINE_SEATS;
	watch_keys_seats(&w->keys, ONLINE_SEATS);
}

void arena_watch_free(ArenaWatch *w) {
	if (!w) return;
	arena_boot_free(w->boot);
	arena_pair_close(w->pair);
	watch_keys_free(&w->keys);
	free(w->hashes.hash);
	free(w->hashes.known);
	free(w);
}

static void loadout_in(ArenaWatch *w, unsigned side, const uint8_t *bytes, size_t n) {
	uint32_t match;
	const uint8_t *body;
	size_t len;
	if (side >= w->seats || !arena_wire_read_loadout(bytes, n, &match, &body, &len) || match != w->match || len > sizeof w->loadout[side]) return;
	memcpy(w->loadout[side], body, len);
	w->loadout_n[side] = len;
	w->loadout_in[side] = true;
}

/* a hash of mine or theirs for the frame: compared where both are in */
static void hashed(ArenaWatch *w, uint32_t frame, uint64_t hash, uint8_t who) {
	if (w->stage == WATCHING_FAILED || (frame + 1) % HASH_EVERY) return;
	uint32_t at = frame / HASH_EVERY;
	if (!hashes_put(&w->hashes, at, hash, who)) return;
	if (w->hashes.known[at] != 3) return;
	if (w->hashes.hash[2 * at] != w->hashes.hash[2 * at + 1]) {
		char why[96];
		snprintf(why, sizeof why, "the games went apart by frame %" PRIu32, frame);
		fail(w, why);
		return;
	}
	++w->checked;
}

static void hash_in(ArenaWatch *w, unsigned side, uint32_t frame, uint64_t hash) {
	if (frame == RECORD_FRAME) { if (!side) { w->record = hash; w->record_in = true; } return; }
	if (frame == READY_FRAME) { if (!side) { w->ready = hash; w->ready_in = true; } return; }
	if (!side) hashed(w, frame, hash, 2);
}

void arena_watch_relayed(ArenaWatch *w, unsigned side, const uint8_t *message, size_t n) {
	uint32_t match, frame;
	uint64_t hash;
	if (n && message[0] == ONLINE_ARENA_LOADOUT) loadout_in(w, side, message, n);
	else if (arena_wire_read_hash(message, n, &match, &frame, &hash) && match == w->match) hash_in(w, side, frame, hash);
}

/* (arenadesync=N on a spectator: the cable's first's keys from frame N for
 * a second turned to other directions and buttons here, as a spectator's
 * copy gone wrong: the hashes' stop tested) */
#define DESYNC_FRAMES 60
#define DESYNC_KEYS   0x33u
static const uint8_t *desynced(unsigned side, uint32_t first, const uint8_t *be, unsigned count, uint8_t *out) {
	uint32_t at = (uint32_t)dev.arena_desync;
	if (dev.arena_desync <= 0 || side || first + count <= at || first >= at + DESYNC_FRAMES) return be;
	memcpy(out, be, 2u * count);
	for (unsigned i = 0; i < count; ++i)
		if (first + i >= at && first + i < at + DESYNC_FRAMES) out[2 * i + 1] ^= DESYNC_KEYS;
	if (first <= at) printf("watch: (arenadesync) the first side's keys from frame %u changed here\n", at);
	return out;
}

void arena_watch_keys(ArenaWatch *w, unsigned side, uint32_t first, const uint8_t *be, unsigned count) {
	uint8_t changed[2 * ONLINE_WATCH_KEYS_MAX];
	if (count > ONLINE_WATCH_KEYS_MAX) return;
	be = desynced(side, first, be, count, changed);
	if (w->stage != WATCHING_FAILED && !watch_keys_put(&w->keys, side, first, be, count)) fail(w, "the players' keys came apart");
}

void arena_watch_ended(ArenaWatch *w) { w->ended = true; }

static void build(ArenaWatch *w) {
	static OnlineLoadout l[ONLINE_SEATS];
	const OnlineLoadout *carried[ONLINE_SEATS];
	for (unsigned side = 0; side < w->seats; ++side) {
		bool is;
		char why[120];
		if (!arena_loadout_unpack(w->loadout[side], w->loadout_n[side], &l[side], &is, why, sizeof why)) { fail(w, why); return; }
		carried[side] = is ? &l[side] : NULL;
	}
	/* (a 2v2: the partners' MegaMen seats 2 and 3's, as every player's game sets them) */
	if (w->seats > 2) arena_seats_partners(carried + 2);
	arena_seats_set(w->seats > 2);
	w->boot = arena_boot_new(w->rom, w->size, carried);
	if (!w->boot) { fail(w, "too little memory for the two PETs"); return; }
	if (arena_boot_stage(w->boot) == ARENA_BOOT_FAILED) { fail(w, arena_boot_error(w->boot)); return; }
	/* (the bout's seed, its match, as both players' games write it: a
	 * rematch they went into from their kept pair is the same bout here,
	 * the whole way in from the start) */
	arena_boot_seed(w->boot, w->match);
	printf("watch: both MegaMen in, the way in begins\n");
	w->stage = WATCHING_IN;
}

/* the pair at the Custom screen against the players': the same, or watching stops */
static void ready(ArenaWatch *w) {
	if (!w->ready_in) return;
	if (w->pair_ready != w->ready) {
		bool installs = w->record_in && w->record != arena_pair_record(w->pair);
		fail(w, installs ? "this PET's BN5 is not theirs" : "the two PETs here came out apart from theirs");
		return;
	}
	printf("watch: the pair agrees with the players' at the Custom screen (%016" PRIx64 ")\n", w->ready);
	w->stage = WATCHING_ON;
}

static void way_in(ArenaWatch *w, bool headless) {
	if (w->pair) { ready(w); return; }
	ArenaBootStage s;
	if (headless) s = arena_boot_run(w->boot, HEADLESS_SLICE);
	else {
		uint64_t start = SDL_GetPerformanceCounter(), budget = SDL_GetPerformanceFrequency() * SLICE_MS / 1000;
		while ((s = arena_boot_run(w->boot, SLICE)) != ARENA_BOOT_READY && s != ARENA_BOOT_FAILED &&
		       SDL_GetPerformanceCounter() - start < budget) {}
	}
	if (s == ARENA_BOOT_FAILED) { fail(w, arena_boot_error(w->boot)); return; }
	if (s != ARENA_BOOT_READY) return;
	w->pair = arena_boot_take(w->boot);
	arena_boot_free(w->boot);
	w->boot = NULL;
	arena_pair_mute(w->pair);
	w->pair_ready = arena_pair_hash(w->pair);
	ready(w);
}

/* a core whose NetBattle is decided and gone, back at the PET's Comm menu */
static bool gone(struct mCore *c) {
	uint32_t main = c->rawRead32(c, BN6_TOOLKIT, -1), menu = c->rawRead32(c, BN6_TOOLKIT + BN6_TOOLKIT_SUBMENU, -1);
	return c->rawRead8(c, BN6_BATTLE_RESULT, -1) && c->rawRead8(c, BN6_BATTLE_MODE, -1) == BN6_BATTLE_GONE &&
		c->rawRead8(c, main, -1) == BN6_MODE_SUBMENU && c->rawRead8(c, menu, -1) == BN6_SUBMENU_COMM;
}

static bool step(ArenaWatch *w) {
	uint16_t k[ARENA_LOOKAHEAD][ARENA_PLAYERS];
	watch_keys_take_all(&w->keys, w->next, k);
	if (!arena_pair_step_all(w->pair, (const uint16_t (*)[ARENA_PLAYERS])k)) { fail(w, "the cable between the two cores stuck"); return false; }
	++w->next;
	if (w->next % HASH_EVERY == 0) hashed(w, w->next - 1, arena_pair_hash(w->pair), 1);
	if (w->stage != WATCHING_ON) return false;
	struct mCore *first = arena_pair_core(w->pair, 0), *second = arena_pair_core(w->pair, 1);
	if (first->rawRead8(first, BN6_BATTLE_PHASE, -1) == BN6_PHASE_FIGHT) w->fought = true;
	if (!w->fought || !gone(first) || !gone(second)) return true;
	w->result = first->rawRead8(first, BN6_BATTLE_RESULT, -1);
	printf("watch: over at frame %" PRIu32 ": the cable's first's result %u, %" PRIu32 " hashes checked\n", w->next, w->result, w->checked);
	/* (side by side: the pair's two GBAs compared iteration by iteration, as each player's game does) */
	if (arena_seats_net()) arena_team_report();
	w->stage = WATCHING_OVER;
	return false;
}

/* frames to run this tick: all that time allows where far behind (or the
 * keys have ended), one where near, none within BEHIND_LEAST */
static void run(ArenaWatch *w, bool headless) {
	uint32_t runnable = watch_keys_runnable(&w->keys);
	uint32_t behind = runnable > w->next ? runnable - w->next : 0;
	if (!behind && w->ended) { fail(w, "the NetBattle broke off"); return; }
	bool fast = w->ended || behind > BEHIND_KEPT + BEHIND_LEAST;
	if (fast && !w->fast) {
		w->fast_ticks = 0;
		w->fast_from = behind;
		w->fast_at = w->next;
		printf("watch: catching up from frame %" PRIu32 ", %" PRIu32 " frames behind the keys\n", w->next, behind);
	}
	bool was = w->fast;
	w->fast = fast || (w->fast && behind > BEHIND_KEPT);
	if (was && !w->fast) printf("watch: caught up at frame %" PRIu32 " (%" PRIu32 " frames run since frame %" PRIu32 "), %" PRIu32 " behind\n",
		w->next, w->next - w->fast_at, w->fast_at, behind);
	arena_pair_draw(w->pair, 1u << w->shown);
	if (!w->fast) {
		if (behind >= BEHIND_LEAST) step(w);
		return;
	}
	if (!w->ended && ++w->fast_ticks >= TRY_TICKS) {
		if (behind >= w->fast_from) { fail(w, "this PET is too slow to catch up"); return; }
		w->fast_ticks = 0;
		w->fast_from = behind;
	}
	arena_pair_mute(w->pair);
	uint64_t start = SDL_GetPerformanceCounter(), budget = SDL_GetPerformanceFrequency() * FAST_MS / 1000;
	for (unsigned n = 0; w->next < runnable && (w->ended || runnable - w->next > BEHIND_KEPT); ++n) {
		if (headless ? n >= FAST_HEADLESS : SDL_GetPerformanceCounter() - start >= budget) break;
		/* (a tick's first frame drawn alone: "Catching up..." shows over it) */
		if (n == 1) arena_pair_draw(w->pair, 0);
		if (!step(w)) break;
	}
}

WatchingStage arena_watch_tick(ArenaWatch *w, bool headless) {
	switch (w->stage) {
	case WATCHING_SETUP: {
		bool all = true;
		for (unsigned s = 0; s < w->seats; ++s) all = all && w->loadout_in[s];
		if (all) build(w);
		break;
	}
	case WATCHING_IN: way_in(w, headless); break;
	case WATCHING_ON: run(w, headless); break;
	default: break;
	}
	return w->stage;
}

const char *arena_watch_error(const ArenaWatch *w) { return w->error; }

int arena_watch_progress(const ArenaWatch *w) {
	if (w->stage == WATCHING_SETUP) return 0;
	if (w->stage != WATCHING_IN || !w->boot) return 100;
	unsigned done = arena_boot_done(w->boot), total = arena_boot_total();
	return done >= total ? 99 : (int)(done * 100 / total);
}

bool arena_watch_catching_up(const ArenaWatch *w) { return w->stage == WATCHING_ON && w->fast && !w->ended; }

void arena_watch_show(ArenaWatch *w, unsigned side) { w->shown = side & 1u; }

const uint32_t *arena_watch_video(const ArenaWatch *w, unsigned side) { return w->pair ? arena_pair_video(w->pair, side & 1u) : NULL; }

void arena_watch_sound(ArenaWatch *w, unsigned side) {
	if (w->pair && !w->fast && w->stage == WATCHING_ON) arena_pair_sound(w->pair, side & 1u);
}

unsigned arena_watch_result(const ArenaWatch *w) { return w->result; }

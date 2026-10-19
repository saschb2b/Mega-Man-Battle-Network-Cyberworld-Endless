/* The Arena's local engine (arena.h).
 *
 * A key sampled at tick t is frame t + D's (D the link's delay); the
 * first D frames are released keys on both sides, on every client. Frame
 * N of the pair is due at tick N, and runs once both players' keys for N
 * and the three after it are in (arena_input.h), each core taking those
 * of its own frame clock, as the research found BN6's linked cores need
 * (docs/ONLINE_FRAME.md): a delay of three frames and the link's one-way
 * latency never waits. A frame due whose keys have not come is a stall;
 * the frames behind are caught up CATCH_UP a tick. */
#include "arena.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mgba/core/core.h>

#include "arena_boot.h"
#include "arena_pair.h"
#include "bn6.h"

#define CATCH_UP 2       /* frames of the pair a tick may run */
#define PENDING  8       /* my hashes waiting for the peer's */

typedef struct { uint32_t frame; uint64_t hash; } Hashed;

struct Arena {
	ArenaStage stage;
	ArenaLink *link;
	ArenaBoot *boot;
	ArenaPair *pair;
	ArenaInput input;
	uint32_t tick, stalls, hashes;
	Hashed pending[PENDING];
	unsigned npending;
	bool fought, apart;
	unsigned result;
	char error[160];
};

static ArenaStage fail(Arena *a, const char *why) {
	snprintf(a->error, sizeof a->error, "%s", why);
	printf("arena: stopped at frame %" PRIu32 ": %s\n", a->input.next, a->error);
	return a->stage = ARENA_FAILED;
}

Arena *arena_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2], ArenaLink *link, unsigned format) {
	Arena *a = calloc(1, sizeof *a);
	if (!a) return NULL;
	a->link = link;
	a->boot = arena_boot_new(rom, size, loadout, format);
	if (!a->boot) { free(a); return NULL; }
	if (arena_boot_stage(a->boot) == ARENA_BOOT_FAILED) fail(a, arena_boot_error(a->boot));
	return a;
}

void arena_free(Arena *a) {
	if (!a) return;
	arena_boot_free(a->boot);
	arena_pair_close(a->pair);
	if (a->link && a->link->close) a->link->close(a->link);
	free(a);
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
	arena_boot_free(a->boot);
	a->boot = NULL;
	arena_input_init(&a->input, a->link->delay);
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
	while (a->link->poll(a->link, a->input.known[peer], &keys))
		if (arena_input_put(&a->input, peer, a->input.known[peer], keys) != ARENA_PUT_OK) return false;
	return true;
}

/* my hash for the frame just run, and the peer's for the ones before */
static void compare_hashes(Arena *a) {
	uint32_t frame = a->input.next - 1;
	if (a->input.next % ARENA_HASH_EVERY == 0) {
		uint64_t h = arena_pair_hash(a->pair);
		a->link->send_hash(a->link, frame, h);
		if (a->npending == PENDING) {   /* (the oldest given up: a peer that sends none is not a difference) */
			memmove(a->pending, a->pending + 1, sizeof a->pending - sizeof *a->pending);
			--a->npending;
		}
		a->pending[a->npending++] = (Hashed){ frame, h };
		printf("arena: frame %" PRIu32 " hash %016" PRIx64 "\n", frame, h);
	}
	for (unsigned i = 0; i < a->npending;) {
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

/* a core whose NetBattle is decided and gone, back at the PET's Comm menu
 * (which it reads through the battle too: the result byte, 0 until the
 * battle is decided, and the battle's mode say when it has ended) */
static bool battle_gone(struct mCore *c) {
	return c->rawRead8(c, BN6_BATTLE_RESULT, -1) && c->rawRead8(c, BN6_BATTLE_MODE, -1) == BN6_BATTLE_GONE && at_comm(c);
}

/* the battle seen under way, then both games out of it: over */
static void watch_end(Arena *a) {
	struct mCore *mine = arena_pair_core(a->pair, a->link->side), *theirs = arena_pair_core(a->pair, a->link->side ^ 1);
	if (mine->rawRead8(mine, BN6_BATTLE_PHASE, -1) == BN6_PHASE_FIGHT) a->fought = true;
	if (!a->fought || !battle_gone(mine) || !battle_gone(theirs)) return;
	a->result = mine->rawRead8(mine, BN6_BATTLE_RESULT, -1);
	printf("arena: over at frame %" PRIu32 ": result %u and the other's %u (1 won, 2 lost), %" PRIu32 " ticks stalled, %" PRIu32
		" hashes agreed, %u cable transfers\n", a->input.next, a->result, theirs->rawRead8(theirs, BN6_BATTLE_RESULT, -1),
		a->stalls, a->hashes, arena_pair_transfers(a->pair));
	a->stage = ARENA_OVER;
}

ArenaStage arena_tick(Arena *a, uint16_t keys) {
	if (a->stage != ARENA_BATTLE) return a->stage;
	ArenaLink *l = a->link;
	l->tick(l);
	uint32_t now = a->tick++, frame = now + a->input.delay;
	keys &= ARENA_KEYS_MASK;
	if (arena_input_put(&a->input, l->side, frame, keys) != ARENA_PUT_OK || !l->send(l, frame, keys))
		return fail(a, "my keys could not be sent");
	if (!take_peer(a)) return fail(a, "the other player's keys came out of order");
	const char *broken = l->broken(l);
	if (broken) return fail(a, broken);
	if (a->input.next <= now && !arena_input_ready(&a->input)) { ++a->stalls; return a->stage; }
	for (unsigned i = 0; i < CATCH_UP && a->stage == ARENA_BATTLE && a->input.next <= now && arena_input_ready(&a->input); ++i) {
		uint16_t frame_keys[ARENA_LOOKAHEAD][2];
		arena_input_take(&a->input, frame_keys);
		if (!arena_pair_step(a->pair, frame_keys, true)) return fail(a, "the cable between the two cores stuck");
		compare_hashes(a);
		if (a->stage == ARENA_BATTLE) watch_end(a);
	}
	return a->stage;
}

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

uint64_t arena_hash(Arena *a) { return a->pair ? arena_pair_hash(a->pair) : 0; }
uint64_t arena_record(const Arena *a) { return a->pair ? arena_pair_record(a->pair) : 0; }

void arena_sound(Arena *a) {
	if (a->pair) arena_pair_sound(a->pair, a->link->side);
}

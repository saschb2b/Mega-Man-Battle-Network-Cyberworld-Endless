/* The Arena (issue #120, docs/ONLINE_ARENA.md): a NetBattle of BN6's own
 * between two players, each client running both linked cores and only
 * the players' keys crossing between them (arena_link.h). This is its
 * local engine: the way in (arena_boot.h), then the battle in lockstep,
 * frame by frame as both players' keys come, its state hashed now and
 * then and the hashes compared, a difference said and the battle stopped.
 * Its scene is --dev arena's (arena_scene.c). */
#ifndef CW_ARENA_H
#define CW_ARENA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arena_link.h"
#include "arena_sync.h"   /* (the round trips kept for the delay asked for: arena_pings_*) */

#include "online_snapshot.h"

#define ARENA_HASH_EVERY 60   /* frames of the pair between two hashes compared */

typedef enum {
	ARENA_CONNECTING,   /* the way in: arena_connect until it is done */
	ARENA_BATTLE,       /* the battle: arena_tick once a tick */
	ARENA_OVER,         /* back at the Comm menu: arena_result says how it went */
	ARENA_FAILED,       /* (the reason: arena_error) */
} ArenaStage;

typedef struct Arena Arena;

/* An Arena over `link` (mine: link->side) by `format`'s rules
 * (ONLINE_FORMAT_*, arena_format.h), loadout[side] each side's MegaMan
 * (NULL: the game's start's); NULL where memory ran out */
Arena *arena_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2], ArenaLink *link, unsigned format);
void arena_free(Arena *a);
/* Up to `frames` frames of the way in */
ArenaStage arena_connect(Arena *a, unsigned frames);
/* One tick of the battle with my keys: the link's messages, then the
 * pair's frames whose keys have all come (none: a stall, counted) */
ArenaStage arena_tick(Arena *a, uint16_t keys);
ArenaStage arena_stage(const Arena *a);
const char *arena_error(const Arena *a);
/* the way in's progress, 0-100 */
int arena_progress(const Arena *a);
/* my core's picture (ARENA_W x ARENA_H), NULL before the battle */
const uint32_t *arena_video(const Arena *a);
/* side's core's battle phase (BN6_BATTLE_PHASE), 0 before the battle */
unsigned arena_phase(const Arena *a, unsigned side);
/* my result once OVER: 1 won, 2 lost (BN6_BATTLE_RESULT), 0 none */
unsigned arena_result(const Arena *a);
/* FAILED because the two pairs' hashes differed (both clients see it) */
bool arena_apart(const Arena *a);
/* the ticks stalled, waiting for the peer's keys */
uint32_t arena_stalls(const Arena *a);
/* my round trip to the server as the transport measures it, a tick at a
 * time (the local link's: twice its one way), for the time sync that runs
 * the leading game a little slower (arena_sync.h) */
void arena_rtt(Arena *a, uint32_t ms);
/* --dev arena's rollback tried, before the way in is done (arena_roll.h):
 * `window` frames rolled back every `window`, the game's state checked;
 * and every 60th frame the game's hash said (without the sound's fields),
 * which a run that rolls back must say alike; 0 the hash alone */
void arena_rollback(Arena *a, int window);
/* my core's sound into the game's */
void arena_sound(Arena *a);
/* the pair's hash now (both cores' states, the cable's numbers), 0 before
 * the battle: the same on both clients at the Custom screen, before the
 * first frame (the network's link says it as its ready) */
uint64_t arena_hash(Arena *a);
/* the installs' record the pair's cores were made with (arena_pair_record):
 * 0 under BN6's rules, or before the pair is built */
uint64_t arena_record(const Arena *a);

#endif

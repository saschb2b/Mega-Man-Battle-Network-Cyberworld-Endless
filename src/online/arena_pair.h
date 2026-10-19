/* The Arena's pair (issue #120, docs/ONLINE_ARENA.md): two BN6 cores
 * linked by mGBA's own cable (its GBASIOLockstep) in one process, each
 * client running both. Under BN6's rules the cores are the player's ROM
 * as it is: no hook, no install, no other game's data, so that every
 * client builds the same two whatever ROMs each player has. Under full
 * rules (two games with BN5's ROM) each core takes BN5's installs as this
 * game's own core has them (src/emu/xinstall_log.h): their bytes in its
 * ROM copy, their hooks on its own table, the same on every client whose
 * ROMs and build are the same, which the pair's hash holds to. Their
 * flash saves live in memory. */
#ifndef CW_ARENA_PAIR_H
#define CW_ARENA_PAIR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "arena_input.h"

/* (the Arena runs two cores beside the game's: not on the 3DS, whose heap
 * has no room for them, nor yet in the browser, whose build has no threads
 * and a frame budget a second core would break) */
#if defined(__3DS__) || defined(__EMSCRIPTEN__)
#define CW_ARENA 0
#else
#define CW_ARENA 1
#endif

#define ARENA_W 240
#define ARENA_H 160

struct mCore;
typedef struct ArenaPair ArenaPair;

/* A core of the ROM's bytes (`size` of them, padded to the 16 MB the
 * game's own core maps), BN5's installed bytes in it where `bn5`
 * (xlog_apply; their hooks are the pair's), its flash save in memory and
 * its picture into `video` (ARENA_W x ARENA_H): reset, not yet run. NULL
 * where it could not be made (too little memory, or no Arena in this
 * build). */
struct mCore *arena_core_new(const uint8_t *rom, size_t size, uint32_t *video, bool bn5);
void arena_core_free(struct mCore *core);
/* a core's whole state into memory (malloc'd, *size its length), and back */
uint8_t *arena_core_save(struct mCore *core, size_t *size);
bool arena_core_load(struct mCore *core, const uint8_t *state, size_t size);

/* The pair: two cores of the ROM, each loaded with its state, then
 * linked; with BN5's installs where `bn5` (their bytes, and their hooks
 * on each core's own table), souls[side] that side's player's Souls for
 * UNITE (NULL none), each Soul committed carried to the other core
 * (arena_souls.h); NULL where it could not be made */
ArenaPair *arena_pair_open(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2], bool bn5,
	const unsigned souls[2]);
void arena_pair_close(ArenaPair *pair);
/* Each core's frame counter as the clock's start: the next frame of the
 * pair is frame 0 of a stretch of keys (the entry's steps, the battle) */
void arena_pair_anchor(ArenaPair *pair);
/* One frame of the pair: each core run to its next frame, the cable's
 * transfers between. `own_clock` false: both cores hold keys[0] for the
 * frame (the entry's scripted steps); true: each takes the keys of its
 * own frame clock, keys[k] k frames ahead of the pair's (the battle).
 * A core ARENA_LOOKAHEAD frames ahead waits for its partner. False where
 * the cable stuck: the frame not done in STEP_LIMIT instructions. */
bool arena_pair_step(ArenaPair *pair, const uint16_t keys[ARENA_LOOKAHEAD][2], bool own_clock);
/* A hash of both cores' whole state, their flash saves and the cable's,
 * and under full rules the installs' record they were made with: two
 * clients whose pairs agree have the same; 0 where it failed */
uint64_t arena_pair_hash(ArenaPair *pair);
/* The installs' record its cores were made with (xinstall_log.h's hash): 0
 * under BN6's rules */
uint64_t arena_pair_record(const ArenaPair *pair);
/* side's core, and its picture (ARENA_W x ARENA_H, mGBA's colour order) */
struct mCore *arena_pair_core(ArenaPair *pair, unsigned side);
const uint32_t *arena_pair_video(const ArenaPair *pair, unsigned side);
/* the cable's transfers so far */
unsigned arena_pair_transfers(const ArenaPair *pair);
/* the cores' sound: side's into the game's sound (emu_audio_from), the
 * other's dropped; or both dropped (the way in, unheard) */
void arena_pair_sound(ArenaPair *pair, unsigned side);
void arena_pair_mute(ArenaPair *pair);

#endif

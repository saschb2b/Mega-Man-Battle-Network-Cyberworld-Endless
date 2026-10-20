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
 * linked, with BN5's installs (full rules, every NetBattle's: their bytes,
 * and their hooks on each core's own table), souls[side] that side's player's Souls for
 * UNITE (NULL none), each Soul committed carried to the other core
 * (arena_souls.h); NULL where it could not be made */
ArenaPair *arena_pair_open(const uint8_t *rom, size_t size, const uint8_t *state[2], const size_t state_size[2], const unsigned souls[2]);
void arena_pair_close(ArenaPair *pair);
/* Each core's frame counter as the clock's start: the next frame of the
 * pair is frame 0 of a stretch of keys (the entry's steps, the battle) */
void arena_pair_anchor(ArenaPair *pair);
/* The battle begun, both cores at its first Custom screen: under full
 * rules what BN5's link exchange carries at a battle's setup and BN6's
 * does not, given to both (arena_souls_lists) */
void arena_pair_begin(ArenaPair *pair);
/* One frame of the pair: each core run to its next frame, the cable's
 * transfers between. `own_clock` false: both cores hold keys[0] for the
 * frame (the entry's scripted steps); true: each takes the keys of its
 * own frame clock, keys[k] k frames ahead of the pair's (the battle).
 * A core ARENA_LOOKAHEAD frames ahead waits for its partner. False where
 * the cable stuck: the frame not done in STEP_LIMIT run loops. */
bool arena_pair_step(ArenaPair *pair, const uint16_t keys[ARENA_LOOKAHEAD][2], bool own_clock);
/* A hash of both cores' state, their flash saves and the cable's, and
 * under full rules the installs' record they were made with: two clients
 * (and a spectator) whose pairs agree have the same, whether either rolled
 * back or not; 0 where it failed. It leaves out the sound's fields, which
 * a state loaded leaves otherwise than run straight (arena_pair_snap_diff;
 * docs/ONLINE_ARENA.md, Rollback) */
uint64_t arena_pair_hash(ArenaPair *pair);
/* ... with the sound's fields too: the whole state (--dev rollbench) */
uint64_t arena_pair_hash_whole(ArenaPair *pair);
/* The installs' record its cores were made with (xinstall_log.h's hash): 0
 * under BN6's rules */
uint64_t arena_pair_record(const ArenaPair *pair);
/* side's core, and its picture (ARENA_W x ARENA_H, mGBA's colour order) */
struct mCore *arena_pair_core(ArenaPair *pair, unsigned side);
const uint32_t *arena_pair_video(const ArenaPair *pair, unsigned side);
/* the cable's transfers so far */
unsigned arena_pair_transfers(const ArenaPair *pair);

/* A snapshot of the pair, all a rollback puts back (docs/ONLINE_ARENA.md,
 * Rollback): both cores' raw states and what no core's state holds, the
 * cable's lockstep (its numbers, each node's own and its event's place in
 * its core's timing), each GBA's registers as they read and what else it
 * keeps outside its state (the SIO's words, mGBA's idle loop detection),
 * the pair's scheduling and frame clock, the Souls carried and, under full
 * rules, what BN5's hooks keep between their calls (hook_keep_slots). Not
 * the flash saves (a battle writes none). */
typedef struct ArenaPairSnap ArenaPairSnap;
/* room for one snapshot of `pair`; NULL where memory ran out */
ArenaPairSnap *arena_pair_snap_new(ArenaPair *pair);
void arena_pair_snap_free(ArenaPairSnap *snap);
/* the bytes one holds: both cores' states, then the pair's own */
size_t arena_pair_snap_bytes(const ArenaPairSnap *snap);
size_t arena_pair_snap_core_bytes(const ArenaPairSnap *snap);
/* the pair, between two of its frames, into `snap`; and back */
bool arena_pair_save(ArenaPair *pair, ArenaPairSnap *snap);
bool arena_pair_load(ArenaPair *pair, const ArenaPairSnap *snap);
/* Where two snapshots differ outside the sound's fields (which mGBA's lazy
 * PSG leaves otherwise after a load than run straight): the stretches of
 * the game's state apart, and said in the output where `say` (a rollback
 * that came back apart at `frame`: --dev rollbench); 0 the same */
unsigned arena_pair_snap_diff(const ArenaPairSnap *a, const ArenaPairSnap *b, unsigned frame, bool say);
/* the cores' sound: side's into the game's sound (emu_audio_from), the
 * other's dropped; or both dropped (the way in, unheard) */
void arena_pair_sound(ArenaPair *pair, unsigned side);
void arena_pair_mute(ArenaPair *pair);

#endif

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
#include "arena_window.h" /* (the window this system affords, kept for the delay asked for) */

#include "online_snapshot.h"

struct mCore;

#define ARENA_HASH_EVERY 60   /* frames of the pair between two hashes compared */

typedef enum {
	ARENA_CONNECTING,   /* the way in: arena_connect until it is done */
	ARENA_BATTLE,       /* the battle: arena_tick once a tick */
	ARENA_OVER,         /* back at the Comm menu: arena_result says how it went */
	ARENA_FAILED,       /* (the reason: arena_error) */
} ArenaStage;

typedef struct Arena Arena;

/* An Arena over `link` (mine: link->side) by full rules (arena_boot.h),
 * loadout[side] each side's MegaMan
 * (NULL: the game's start's); NULL where memory ran out */
Arena *arena_new(const uint8_t *rom, size_t size, const OnlineLoadout *loadout[2], ArenaLink *link);
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
/* the ticks stalled in a row, waiting for the peer's keys, up to this tick: 0 where it ran */
uint32_t arena_waited(const Arena *a);
/* The link as the badge shows it (arena_view.h): steady; rolling back
 * (a guess undone, or a moment's wait, in the last ARENA_LINK_SHAKY
 * ticks); stalled (waiting on the other's keys now, ARENA_LINK_STALL
 * ticks in a row) */
#define ARENA_LINK_SHAKY 60
#define ARENA_LINK_STALL 10
typedef enum { ARENA_LINK_STEADY, ARENA_LINK_ROLLING, ARENA_LINK_STALLED } ArenaLinkState;
ArenaLinkState arena_link_state(const Arena *a);
/* What the result card reads of the battle (arena_card.h), from its
 * settled frames alone: the frames fought (BN6's fight under way, the
 * Custom screen's left out), each side's HP at the last, and the chips
 * each side used (counted as the stands' report counts them,
 * stands_report.h); by the cable's sides */
typedef struct {
	bool seen;
	uint32_t fought;
	uint16_t hp[2], max_hp[2];
	uint8_t used[2];
} ArenaTally;
void arena_tally(const Arena *a, ArenaTally *out);
/* the battle's network as counted, for the statistics' NetBattle
 * (analytics_online.h): the delay and the window, the rollbacks and the
 * most frames at once, the ticks stalled, a tick's longest, the frames
 * settled */
typedef struct {
	unsigned delay, window;
	uint32_t rollbacks, deepest, stalls, frames;
	double worst_tick_ms;
} ArenaNumbers;
void arena_numbers(const Arena *a, ArenaNumbers *out);
/* what the stands' report reads of the battle (arena_report.h): each
 * side's MegaMan as the cable's first core has him, at the last frame whose
 * keys are all known, so a frame run on a guess and rolled back is never
 * reported */
typedef struct {
	bool seen[2];               /* the side's player object read */
	uint16_t hp[2], max_hp[2], chip[2];
	uint8_t action[2];
} ArenaReading;
/* NULL before the battle's first frame is settled */
const ArenaReading *arena_reading(const Arena *a);
/* my round trip to the server as the transport measures it, a tick at a
 * time (the local link's: twice its one way), for the time sync that runs
 * the leading game a little slower (arena_sync.h) */
void arena_rtt(Arena *a, uint32_t ms);
/* --dev arenaboth's player on `side`, by the tick: the Custom screen's
 * OK (the cursor taken right to it), in the battle charged shots from
 * where he stands (B held 100 frames, the second GBA 104, let go): a NetBattle that plays
 * itself, on a device or between browsers */
uint16_t arena_bot_keys(const Arena *a, unsigned side, uint32_t tick);
/* --dev arena's rollback tried, before the way in is done (arena_roll.h):
 * `window` frames rolled back every `window`, the game's state checked;
 * and every 60th frame the game's hash said (without the sound's fields),
 * which a run that rolls back must say alike; 0 the hash alone */
void arena_rollback(Arena *a, int window);
/* The rollback window, before the way in is done (docs/ONLINE_ARENA.md,
 * Rollback): up to `window` frames run on a guess of the other player's
 * keys and rolled back where it was wrong (0: the delay alone, a frame
 * run once its keys have all come); -1, as it starts, what a frame of the
 * pair costs here affords (arena_window.h) */
void arena_window(Arena *a, int window);
/* The streams the battle carries, before the way in is done: 2, or
 * ARENA_PLAYERS with each side's partner's (--dev arena2v2, epic #167:
 * the partners' keys polled from the link, guessed and rolled back as the
 * other player's, and queued into both GBAs by the battle's iteration,
 * arena_team.h); a link that carries no partners keeps 2 */
void arena_players(Arena *a, unsigned players);
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

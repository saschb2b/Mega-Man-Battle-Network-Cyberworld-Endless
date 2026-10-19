/* The Arena's time sync (issue #128, docs/ONLINE_ARENA.md, Smoothed
 * delay): a NetBattle that feels like the player's own presses, the
 * leading game running a little slower for a moment instead of stopping a
 * whole frame now and then.
 *
 * Each game starts its ticks when the other's ready has come, so the one
 * ready last leads by up to the link's one way, and at the delay's edge it
 * waited for the other's keys again and again (one frame in eight on the
 * deployed server). The lag of a key (my tick as it comes, less the tick
 * the other game sampled it on) is the two clocks' offset plus the trip;
 * the two games' lags add up to the two round trips through the server,
 * so each game holds its own at its own round trip and a tick: when its
 * least lag over the last second is past that, it holds a tick (neither
 * samples nor runs a frame), one at most every ARENA_SYNC_SPREAD ticks,
 * until it is not. No message changes: each game judges by the keys it
 * gets and the round trip its transport measures. A game that waited
 * for a key holds its clock the tick after (arena.c), which moves it the
 * same way (arena_sync_shift). ROM-free: tests/test_arena.c. */
#ifndef CW_ARENA_SYNC_H
#define CW_ARENA_SYNC_H

#include <stdbool.h>
#include <stdint.h>

#define ARENA_SYNC_WINDOW 60   /* the peer's frames a lead is judged by: a second's */
#define ARENA_SYNC_JUDGED 30   /* ... once half a second's have come */
#define ARENA_SYNC_SPREAD 20   /* ticks at least between two held: the game at most 5% slower */
#define ARENA_SYNC_PINGS  8    /* round trips kept for the delay asked for */
#define ARENA_SYNC_PING_MS 2000 /* (the transport's ping, one each) */

typedef struct {
	int16_t lag[ARENA_SYNC_WINDOW];   /* ticks, by my clock as it is now */
	unsigned n, at;
	uint32_t rtt_ms;                  /* my least round trip to the server seen, 0 none */
	unsigned since;                   /* ticks since the last held */
	uint32_t held;                    /* ticks held in all */
	uint32_t shifted;                 /* ticks my clock was held after a stall */
} ArenaSync;

void arena_sync_init(ArenaSync *s);
/* my round trip to the server, as the transport measures it (the least is kept) */
void arena_sync_rtt(ArenaSync *s, uint32_t ms);
/* a key of the other player's come, `lag` ticks after the tick it was sampled on, by my clock */
void arena_sync_arrived(ArenaSync *s, int32_t lag);
/* the lag the game holds itself at: ticks of its least round trip, and one
 * for the tick a key waits to be taken */
unsigned arena_sync_target(const ArenaSync *s);
/* a tick begun: true where it is held (my clock waits a tick) */
bool arena_sync_hold(ArenaSync *s);
/* my clock held a tick for another reason (a stall's): the lags seen a tick less by it */
void arena_sync_shift(ArenaSync *s);

/* The round trips the transport measured in the Square, for the delay the
 * server picks (ARENA's latency field, unchanged on the wire): kept one a
 * ping, said as their mean and twice their spread, so that a link that
 * jitters asks for the delay it needs */
typedef struct {
	uint16_t ms[ARENA_SYNC_PINGS];
	unsigned n, at;
	uint32_t last;   /* when the last was kept (SDL_GetTicks), 0 never */
} ArenaPings;

void arena_pings_note(ArenaPings *p, uint32_t rtt_ms, uint32_t now_ms);
uint32_t arena_pings_latency(const ArenaPings *p);

/* The rollback window (issue #128, docs/ONLINE_ARENA.md, Rollback): the
 * frames a tick can run again after one snapshot loaded, and the new one,
 * each saved, within three quarters of a 60th of a second (a quarter kept
 * for the drawing, the picture's present and the system), from a battle
 * frame of the pair's mean cost here and a tenth more; ARENA_WINDOW_MOST
 * at most. The way in's frames, BN6's menus, cost less: a battle's is
 * taken as ARENA_MENUS_TO_BATTLE of theirs (measured: 0.65 ms and 1.0
 * here, docs/ONLINE_ARENA.md) */
#define ARENA_WINDOW_MOST  10   /* frames rolled back at most: past them a guess shows too long */
#define ARENA_WINDOW_FIRST 4    /* ... where no frame was measured here yet */
#define ARENA_MENUS_TO_BATTLE 1.6
unsigned arena_window_for(double frame_ms);
/* The latency a game says in its challenge or Yes (ARENA's, protocol v8):
 * its round trip with its jitter, less what its rollback window covers.
 * The server's delay is the lockstep's frame, the tick a key waits to be
 * taken, and the longer of the two said (arena.py's delay_for) */
uint32_t arena_ask_ms(uint32_t latency_ms, unsigned window);

#endif

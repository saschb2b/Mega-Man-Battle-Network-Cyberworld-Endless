/* The Arena's time sync (issue #128, docs/online/arena/smoothed-delay.md, Smoothed
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
 * until it is not. A game that waited for a key holds its clock the tick
 * after (arena.c), which moves it the same way (arena_sync_shift).
 *
 * Balanced (issue #167, protocol v11): held at its own round trip, the
 * game with the worse link kept the larger lag and saw the deeper
 * rollbacks. Each game now says its least lag of the last second to the
 * other once a second (ARENA_HASH's lag note), and the one whose lag is
 * two ticks or more past the other's holds a tick: each hold takes one
 * from its lag and gives one to the other's, so the two meet, the waits
 * split evenly. A hold waits for a tick nobody sees (the Custom screen,
 * or my keys unchanged a while: arena.c's `quiet`), ARENA_SYNC_LOUD at
 * most, and after it the game waits for the other's lag to show it
 * (ARENA_SYNC_SETTLE). Before the first note, the old rule: my own round
 * trip and a tick. ROM-free: tests/test_arena.c. */
#ifndef CW_ARENA_SYNC_H
#define CW_ARENA_SYNC_H

#include <stdbool.h>
#include <stdint.h>

#define ARENA_SYNC_WINDOW 60   /* the peer's frames a lead is judged by: a second's */
#define ARENA_SYNC_JUDGED 30   /* ... once half a second's have come */
#define ARENA_SYNC_SPREAD 20   /* ticks at least between two held: the game at most 5% slower */
#define ARENA_SYNC_PINGS  8    /* round trips kept for the delay asked for */
#define ARENA_SYNC_PING_MS 2000 /* (the transport's ping, one each) */
#define ARENA_SYNC_EVEN   2    /* my least lag past the other's by this much: I lead, and hold */
#define ARENA_SYNC_SETTLE 180  /* ticks after a hold before the next by the other's lag: his second of keys judged anew, and said */
#define ARENA_SYNC_LOUD   120  /* ticks a hold waits for a quiet one at most */
#define ARENA_SYNC_NOTE   60   /* ticks between my lag notes to the other */

typedef struct {
	int16_t lag[ARENA_SYNC_WINDOW];   /* ticks, by my clock as it is now */
	unsigned n, at;
	uint32_t rtt_ms;                  /* my least round trip to the server seen, 0 none */
	unsigned since;                   /* ticks since the last held */
	uint32_t held;                    /* ticks held in all */
	uint32_t shifted;                 /* ticks my clock was held after a stall */
	int16_t peer_lag;                 /* the other's least lag, as he last said it */
	bool peer_known;                  /* ... once he has */
	unsigned due;                     /* ticks a hold has waited for a quiet one */
} ArenaSync;

void arena_sync_init(ArenaSync *s);
/* my round trip to the server, as the transport measures it (the least is kept) */
void arena_sync_rtt(ArenaSync *s, uint32_t ms);
/* a key of the other player's come, `lag` ticks after the tick it was sampled on, by my clock */
void arena_sync_arrived(ArenaSync *s, int32_t lag);
/* the lag the game holds itself at: ticks of its least round trip, and one
 * for the tick a key waits to be taken */
unsigned arena_sync_target(const ArenaSync *s);
/* a tick begun: true where it is held (my clock waits a tick); `quiet`
 * where a hold now would not be seen */
bool arena_sync_hold(ArenaSync *s, bool quiet);
/* the other game's least lag, as his note says it (protocol v11) */
void arena_sync_peer(ArenaSync *s, int lag);
/* my least lag of the last second for my note to him: false before half a second's keys have come */
bool arena_sync_least(const ArenaSync *s, int *lag);
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

/* The rollback window (issue #128, docs/online/arena/rollback/overview.md): the
 * frames a tick can run again after one snapshot loaded, and the new one,
 * each saved, within three quarters of a 60th of a second (a quarter kept
 * for the drawing, the picture's present and the system), from a battle
 * frame of the pair's mean cost here and a tenth more; ARENA_WINDOW_MOST
 * at most. The way in's frames, BN6's menus, cost less: a battle's is
 * taken as ARENA_MENUS_TO_BATTLE of theirs (measured: 0.65 ms and 1.0
 * here, docs/online/arena/) */
#define ARENA_WINDOW_MOST  10   /* frames rolled back at most: past them a guess shows too long */
#define ARENA_WINDOW_FIRST 4    /* ... where no frame was measured here yet */
#define ARENA_MENUS_TO_BATTLE 1.6
unsigned arena_window_for(double frame_ms);
/* The latency a game says in its challenge or Yes (ARENA's, protocol v8):
 * its round trip with its jitter, less what its rollback window covers but
 * a frame, the tick a key waits to be taken (a game with no window says a
 * frame more for it). The server's delay is the lockstep's frame and the
 * longer of the two said (arena.py's delay_for): 1 where both windows
 * cover the trip (issue #167; 2 before, the server adding that tick) */
uint32_t arena_ask_ms(uint32_t latency_ms, unsigned window);

#endif

/* A NetBattle watched from the stands (issue #128, docs/ONLINE_ARENA.md,
 * The stands). The spectator's game builds the same pair as the two
 * players' games (the way in from the ROM with both carried MegaMen, by
 * the battle's rules, BN5's installs under full rules), and runs it from
 * both players' keys as the server hands them on, behind the players: it
 * catches up faster than real time where it came in late, then keeps a
 * second or two behind the keys known, so it rarely waits. It never sends
 * a key. The cable's first game's hashes are checked against the pair's
 * own at the same frames: a difference, or a pair built apart at the
 * Custom screen, stops the watching, said why. */
#ifndef CW_ARENA_WATCH_H
#define CW_ARENA_WATCH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "watch_keys.h"

typedef enum {
	WATCHING_SETUP,     /* both MegaMen awaited */
	WATCHING_IN,        /* the way in, slices a tick */
	WATCHING_ON,        /* the battle, from the keys */
	WATCHING_OVER,      /* both cores out of the battle: arena_watch_result */
	WATCHING_FAILED,    /* (arena_watch_error says why) */
} WatchingStage;

typedef struct ArenaWatch ArenaWatch;

/* A battle by `format`'s rules (ONLINE_FORMAT_*); NULL where memory ran out */
ArenaWatch *arena_watch_new(const uint8_t *rom, size_t size, unsigned format, uint32_t match);
void arena_watch_free(ArenaWatch *w);
/* What the server hands on: a player's MegaMen (ARENA_LOADOUT's bytes),
 * the cable's first's hashes (the installs' record, the ready, every 60
 * frames: arena_link_net.h's frames), the keys; and the end: no more keys
 * come (the battle over, or it broke off); `match` the battle's */
void arena_watch_relayed(ArenaWatch *w, unsigned side, const uint8_t *message, size_t n);
void arena_watch_keys(ArenaWatch *w, unsigned side, uint32_t first, const uint8_t *be, unsigned count);
void arena_watch_ended(ArenaWatch *w);
/* A tick: the way in, or the battle's frames due (`fast` frames at most
 * where it is behind, one a tick near the keys' end, none where it is
 * too near) */
WatchingStage arena_watch_tick(ArenaWatch *w, bool headless);
const char *arena_watch_error(const ArenaWatch *w);
/* the way in's progress (0-100); far behind the keys known, catching up */
int arena_watch_progress(const ArenaWatch *w);
bool arena_watch_catching_up(const ArenaWatch *w);
/* side's core's picture, NULL before the battle; its sound into the game's */
const uint32_t *arena_watch_video(const ArenaWatch *w, unsigned side);
void arena_watch_sound(ArenaWatch *w, unsigned side);
/* the cable's first player's result once OVER: 1 won, 2 lost, else none */
unsigned arena_watch_result(const ArenaWatch *w);

#endif

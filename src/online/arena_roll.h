/* The Arena's rollback tried against the local stand-in (issue #128,
 * docs/online/arena/rollback/overview.md): `--dev arena,arenaroll=W` keeps the
 * pair's snapshot before each of the last W frames and, every W frames,
 * rolls back to the first of them and runs them again with the same keys,
 * as a rollback over a wrong guess would, then goes on from there; each
 * time the game's state must come back as it was run straight. The real
 * Arena does not roll back: this is the groundwork's proof, the engine's
 * own path (the Souls carried, BN5's hooks) under it. */
#ifndef CW_ARENA_ROLL_H
#define CW_ARENA_ROLL_H

#include <stdbool.h>
#include <stdint.h>

#include "arena_input.h"
#include "arena_pair.h"

typedef struct ArenaRoll ArenaRoll;

/* a rollback of `window` frames (1 to ARENA_ROLL_MOST) over `pair`; NULL where memory ran out */
#define ARENA_ROLL_MOST 30
ArenaRoll *arena_roll_new(ArenaPair *pair, unsigned window);
/* its count said (the rollbacks, and those that came back to the same game state), and let go */
void arena_roll_free(ArenaRoll *roll);
/* ... said alone, once (the battle over, the game ending before it is let go: quitend) */
void arena_roll_say(ArenaRoll *roll);
/* frame `frame` of the pair about to run with `keys` (every stream's): its snapshot and keys kept */
bool arena_roll_before(ArenaRoll *roll, uint32_t frame, const uint16_t keys[ARENA_LOOKAHEAD][ARENA_PLAYERS]);
/* ... and run: every `window` frames those rolled back and run again; false where the cable stuck */
bool arena_roll_after(ArenaRoll *roll, uint32_t frame);

#endif

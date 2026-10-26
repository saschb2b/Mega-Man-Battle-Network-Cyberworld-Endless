/* The run's Souls (docs/game/meta/souls.md; issues #69, #82): which of BN5's
 * Team Colonel Navis the run has beaten as a guardian (docs/game/bosses/bn5-navis.md,
 * BN5's Navis) and holds the Soul of, for every battle of the run, the
 * Souls standing in BN6's engine (docs/multirom/souls.md);
 * kept beside the run's checkpoint. A new run holds none. */
#ifndef CW_SOULS_H
#define CW_SOULS_H

#include <stdbool.h>
#include <stdint.h>

#define SOULS 6   /* Colonel, ShadowMan, NumberMan, TomahawkMan, KnightMan, ToadMan: BN5's Souls 7-12 */

/* A new run's: none, or the dev flag's (--dev souls=MASK, a bit a Soul) */
void souls_new_run(uint32_t seed);
/* ... where what is held is another run's (a run begun in the net) */
void souls_begin(uint32_t seed);
/* Saved beside the checkpoint; taken back on CONTINUE, for the run whose
 * seed it names. */
void souls_save(void);
void souls_load(uint32_t seed);
/* The Souls held, bit k BN5's Soul 7 + k (soulunite.h's, handed to BN6's
 * engine as each battle begins: encounter.c) */
unsigned souls_held(void);
/* The Souls held as the layer about to be made is made (the director's,
 * with the folder's codes), kept beside the checkpoint: what its finds
 * and stock lean to (loot_souls.h), the same on a CONTINUE's rebuild, a
 * Soul won on the layer counted from the next */
void souls_note_layer(void);
unsigned souls_layer(void);
/* Whether the run holds guardian `navi`'s Soul (guardians.h, guardian_older),
 * and his given */
bool soul_held(int navi);
void soul_give(int navi);
/* The dev flag's mask, for a new run's first Souls (devtools) */
extern uint8_t souls_dev_mask;

#endif

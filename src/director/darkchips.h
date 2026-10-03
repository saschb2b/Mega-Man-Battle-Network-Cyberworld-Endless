/* The run's DarkChips (docs/META.md, DarkChips in BN5 territory; issue
 * #64): which of BN5's twelve it holds, kept beside the run's checkpoint,
 * and the price of one used: one HPMemory of max HP for the rest of the
 * run. A new run holds none. */
#ifndef CW_DARKCHIPS_H
#define CW_DARKCHIPS_H

#include <stdbool.h>
#include <stdint.h>

#define DARK_KINDS 12              /* BN5's DarkChips, ids 187 (DarkCirc) to 198 (DrkSonic) */
#define DARK_FIRST_BN5_ID 187
#define DARK_PRICE 20              /* max HP a battle with one used costs: one HPMemory (the owner's) */

/* A new run's: none, or the dev flag's (--dev darkchips=MASK, a bit a
 * kind). */
void dark_new_run(uint32_t seed);
/* ... where what is held is another run's (a run begun in the net) */
void dark_begin(uint32_t seed);
/* Saved beside the checkpoint; taken back on CONTINUE, for the run whose
 * seed it names. */
void dark_save(void);
void dark_load(uint32_t seed);
/* How many of kind `k` (0-11) the run holds (one at most, as BN5's folder
 * takes one of each); it held. */
int dark_count(int k);
void dark_give(int k);
/* After a guest battle: what BN5 left of the run's DarkChips (counts by
 * kind, BN5 using them up as it plays them). */
void dark_set_counts(const uint8_t counts[DARK_KINDS]);
/* The dev flag's mask, for a new run's first DarkChips (devtools). */
extern uint16_t dark_dev_mask;

#endif

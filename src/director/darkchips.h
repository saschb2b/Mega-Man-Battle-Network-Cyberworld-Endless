/* The run's DarkChips (docs/META.md, DarkChips in BN5 territory and BN6's
 * own DarkChips; issues #64, #70): which of BN5's twelve kinds it holds,
 * kept beside the run's checkpoint, and the price of one used: one HPMemory
 * of max HP for the rest of the run, for every battle a DarkChip's dark
 * power ran in, in either net. A new run holds none.
 *
 * BN6 keeps five of BN5's twelve, its records 286-290, each BN5's DarkChip
 * of its name: a kind the run holds is one chip, played by BN5's rule in
 * BN5's battles and from BN6's folder in BN6's. DarkPlus, which never joins
 * the chip before it in BN6, sits out of BN6's battles. */
#ifndef CW_DARKCHIPS_H
#define CW_DARKCHIPS_H

#include <stdbool.h>
#include <stdint.h>

#define DARK_KINDS 12              /* BN5's DarkChips, ids 187 (DarkCirc) to 198 (DrkSonic) */
#define DARK_FIRST_BN5_ID 187
#define DARK_PRICE 20              /* max HP a battle with one used costs: one HPMemory (the owner's) */
#define DARK_BN6_FIRST 286         /* BN6's DrkSword, DarkThnd, DrkRecov, DarkInvs, DarkPlus: ids 286-290, DarkChipID 0-4 */
#define DARK_BN6_COUNT 5
#define DARK_BN6_PLAYED 4          /* ... the first four, which BN6's battles play and its flames offer */

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

/* BN6's DarkChip `id`'s kind (0-11), -1 for any other chip (DarkPlus's
 * too: it is BN5's kind 3). */
int dark_bn6_kind(int id);
/* BN6's chip of kind `k` that its battles play (DrkSword, DarkThnd,
 * DrkRecov, DarkInvs), 0 for a kind they do not. */
int dark_bn6_id(int k);

/* The kind a flame of darkness on layer `depth` offers: the first the run
 * lacks (`held`, counts by kind) from the run's seed and the act on, of
 * BN5's twelve, or of the four BN6's battles play (`bn6`); -1 where it holds
 * every one. */
int dark_flame_pick(uint32_t seed, int depth, bool bn6, const uint8_t held[DARK_KINDS]);
/* ... the layer's flame as the run keeps it: picked as the layer is first
 * made, and the same when a CONTINUE makes it again (the run holding that
 * kind by then, where it was taken before the save). */
int dark_flame_of(uint32_t seed, int depth, int side, bool bn6);

/* The price of a battle a DarkChip's dark power ran in, on MegaMan's base
 * max HP (HPMemory counts into it), max HP and HP: `out` the three after it
 * (one HPMemory off the base, never under 10; the max with it, the HP at
 * most the max). */
void dark_price_hp(int base, int max, int hp, int out[3]);

#endif

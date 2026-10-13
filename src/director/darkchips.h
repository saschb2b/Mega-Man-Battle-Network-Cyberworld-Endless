/* The run's DarkChips (docs/META.md, DarkChips; issues #64, #70, #82):
 * which of BN5's twelve kinds it holds, kept beside the run's checkpoint,
 * and the price of one used: one HPMemory of max HP for the rest of the
 * run, for every battle a DarkChip's dark power ran in. A new run holds
 * none.
 *
 * Every kind is one chip of BN6's tables (the chip registry,
 * xchips_registry.h), played in BN6's battles from the folder: BN6 keeps
 * five of BN5's twelve, its records 286-290, each BN5's DarkChip of its
 * name, played by BN6's own rule (a BugFrag a use); BN5's seven of its own
 * (registry ids 214-220) come into the hand by BN5's worried rule, its
 * mood brought into BN6's battles (soulmood.h). DarkPlus, BN6's 290, plays
 * BN5's code by BN5's rule where BN5's ROM is read (xchips_moved.c), and
 * BN5's flame alone offers it there (the owner's call of 13 October 2026:
 * BN6's US DarkPlus does nothing). */
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
/* The dev flag's mask, for a new run's first DarkChips (devtools). */
extern uint16_t dark_dev_mask;

/* BN6's DarkChip `id`'s kind (0-11), -1 for any other chip (DarkPlus's
 * too: it is BN5's kind 3). */
int dark_bn6_kind(int id);
/* BN6's chip of kind `k` that its battles play by BN6's rule (DrkSword,
 * DarkThnd, DrkRecov, DarkInvs), 0 for a kind they do not. */
int dark_bn6_id(int k);
/* Kind `k`'s chip in BN6's tables, whichever rule plays it (BN6's 286-290,
 * the registry's 214-220), 0 none; and chip `id`'s kind, -1 for no
 * DarkChip */
int dark_chip_id(int k);
int dark_chip_kind(int id);
/* The kinds a flame offers, a bit each: BN6's own (`bn5` false) its four,
 * never DarkPlus; BN5's every kind (BN6's four among them; BN5's own seven
 * by BN5's rule, where its mood is in BN6's battles), DarkPlus only where
 * BN5's code plays it (`plus`) */
#define DARK_PLUS_KIND 3
#define DARK_INVS_KIND 2           /* (BN5's DarkInvs: 600 frames under its chip AI, src/emu/darkinvs.h) */
unsigned dark_kinds_offered(bool bn5, bool plus);

/* The kind a flame of darkness on layer `depth` offers: the first the run
 * lacks (`held`, counts by kind) from the run's seed and the act on, of
 * BN5's flame's kinds, or of the four BN6's battles play (`bn6`), each of
 * `offered` (a bit a kind, dark_kinds_offered); -1 where it holds every
 * one. */
int dark_flame_pick(uint32_t seed, int depth, bool bn6, const uint8_t held[DARK_KINDS], unsigned offered);
/* ... the layer's flame as the run keeps it: picked as the layer is first
 * made, and the same when a CONTINUE makes it again (the run holding that
 * kind by then, where it was taken before the save). */
int dark_flame_of(uint32_t seed, int depth, int side, bool bn6, unsigned offered);

/* The price of a battle a DarkChip's dark power ran in, on MegaMan's base
 * max HP (HPMemory counts into it), max HP and HP: `out` the three after it
 * (one HPMemory off the base, never under 10; the max with it, the HP at
 * most the max). */
void dark_price_hp(int base, int max, int hp, int out[3]);

/* Chip `id` (BN6's tables) one of the run's DarkChips, either net's: the
 * registry's dark rows, BN6's 286-290 and BN5's own 214-220 (dark_chip_id
 * of every kind); the test the price's hook makes of every chip MegaMan
 * runs (darkbn6.c). */
bool dark_chip_is(int id);
/* A battle's DarkChips that ran, a bit each: BN6's rule's by their
 * DarkChipID (1 << 0-4, a BugFrag paid), and DARK_RAN_BN5 for one by
 * BN5's (its record's DarkChipID 0xFF: BN5's own seven, and DarkPlus
 * where BN5's plays in BN6's record of its name) */
#define DARK_RAN_BN6 0x1Fu
#define DARK_RAN_BN5 0x20u
/* ... `ran` with chip `id` run for MegaMan (BN6_DARK_AFTER's chip: a
 * DarkChip of BN6's only where its dark power ran, its base chip's id
 * where it fell back) and `dark_id` its record's DarkChipID: unchanged
 * for any chip but a DarkChip. A DarkChip given up for a Chaos Unison
 * never runs so (its attack goes through the chip writer alone): none. */
unsigned dark_ran_add(unsigned ran, int id, int dark_id);

#endif

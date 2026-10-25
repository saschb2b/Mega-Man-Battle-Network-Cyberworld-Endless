/* The finds and the Net Dealers' chips leaned to the run's Souls
 * (docs/META.md, Souls: the key with the lock; the owner's call of 25
 * October 2026). A Soul unites with a chip of its kind (soulkinds.h), and
 * a run that cannot farm, its starting folder holding almost none of them,
 * won Souls it had nothing to unite with. So a share of the Mystery Data's
 * chips are of a held Soul's kind, each of the tier it rolled (no Giga
 * early: the counts and tiers stay as they were), and one of a Net
 * Dealer's four chips; the kind the folder holds fewest chips of first.
 * Read from the Souls held as the layer was made (souls_layer), as the
 * folder's codes are, so a checkpoint rebuilds the same stock. Nothing
 * is drawn where the run holds no Soul. DarkChips are never leaned to. */
#ifndef CW_LOOT_SOULS_H
#define CW_LOOT_SOULS_H

#include <stdbool.h>

#define LOOT_SOUL_LEAN 25   /* % of the Mystery Data's chips of a held Soul's kind */

/* The Souls held as the layer is made (souls.h: bit k BN5's Soul 7 + k,
 * 0 none): their kinds, where both ROMs are read (the folder's chips of
 * each counted at a pick, as loot_folder_counts was given them) */
void loot_soul_souls(unsigned souls);
/* Soul k's kind (BN5_CHIP_KIND, as soulkinds.h numbers a chip's), -1
 * where BN5's ROM is not read */
int loot_soul_kind_of(int soul);
/* Whether chip `id` is of Soul k's kind, and no DarkChip (the economy
 * report's count) */
bool loot_soul_chip(int id, int soul);
/* The kind a chip leans to: of the held Souls' kinds, the one the folder
 * holds fewest chips of (a draw among those tied); -1 none */
int loot_soul_kind(void);
/* A find's chip (`*id`, `*code`): LOOT_SOUL_LEAN% of the time where the run
 * holds a Soul, one of loot_soul_kind's of the same tier, where its tier
 * has one; whether it was */
bool loot_soul_find(int *id, char *code);
/* A dealer's chip of loot_soul_kind's: of `tier`, else the nearest under
 * it, else over it up to a rare standard (ECON_GOOD_TIER); never one the
 * folder holds as many of as it may; its code into `code`; -1 none */
int loot_soul_stock(int tier, char *code);

#endif

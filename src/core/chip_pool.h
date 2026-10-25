/* The chips a run can find or buy: every standard, Mega and Giga chip of
 * the ROM's chip data, in tiers by rarity and class; and, where BN5's ROM
 * is read, the older net's own catalogue beside them (issue #82, docs/
 * docs/multirom/one-net.md): every chip of the registry that
 * plays in BN6's battles, those both games name by BN6's id. */
#ifndef CW_CHIP_POOL_H
#define CW_CHIP_POOL_H

#include <stdbool.h>

#define CHIP_TIERS 5

/* The nets a chip pool is of: BN6's chips, or BN5's catalogue (the
 * registry's chips that play, xchips_registry.h) */
enum { CHIP_NET_BN6, CHIP_NET_BN5 };

/* Chip `rom_id`'s tier (0 common standard .. 4 Giga), or -1 when a run
 * cannot hold it (Program Advances, unused records, a DarkChip, a chip of
 * BN5's that sits out of BN6's battles). */
int chip_pool_tier(int rom_id);
/* A random chip of `tier` from BN6's chips (the nearest tier that has
 * some); -1 without a ROM. */
int chip_pool_pick(int tier);
/* ... from net `net`'s pool (CHIP_NET_*), of `tier` alone; -1 where it has
 * none of it (BN5's without its ROM) */
int chip_pool_pick_net(int tier, int net);
/* ... of those of `tier` alone that `want` takes; -1 where it has none */
int chip_pool_pick_if(int tier, int net, bool (*want)(int rom_id));
/* Chip `rom_id`'s Library class (0 standard, 1 Mega, 2 Giga), or -1 when a
 * run cannot hold it; and how many chips of a class a run can hold. BN6's
 * chips alone: the classes' marks are BN6's (docs/game/meta/). */
int chip_pool_class(int rom_id);
int chip_pool_class_count(int cls);
/* Whether chip `rom_id` is one of the older net's own a run can hold (an
 * id of the registry's own, a standard, Mega or Giga chip, no DarkChip and
 * no Program Advance), whether it plays now or sits out; and how many of
 * them play now, which a run can find (0 without BN5's ROM). The profile's
 * Library counts them apart, under the older net. */
bool chip_pool_older(int rom_id);
int chip_pool_older_count(void);

#endif

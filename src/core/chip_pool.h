/* The chips a run can find or buy: every standard, Mega and Giga chip of
 * the ROM's chip data, in tiers by rarity and class; and, where another
 * game's ROM is read, the older Net's catalogues beside them (issue #82,
 * docs/multirom/one-net.md; docs/multirom/bn4-chips.md, unit 7): each
 * game's chips as a run has them, those BN6 names by BN6's id, its own by
 * the registry's where they play in BN6's battles. */
#ifndef CW_CHIP_POOL_H
#define CW_CHIP_POOL_H

#include <stdbool.h>

#define CHIP_TIERS 5

/* The nets a chip pool is of: BN6's chips; BN5's catalogue (the registry's
 * chips that play, xchips_registry.h); BN4's (the run's chip of each of
 * BN4's, xchips_bn4_paid: BN6's or the registry's of its name, its own
 * rows that play, none of them a dark chip, xreg4_dark: a find tells no
 * price, and no flame offers one yet); and the older Net's whole, every catalogue read, BN5's
 * first, a chip once (the deep's: its half draws from the games present) */
enum { CHIP_NET_BN6, CHIP_NET_BN5, CHIP_NET_BN4, CHIP_NET_OLDER, CHIP_NETS };

/* Chip `rom_id`'s tier (0 common standard .. 4 Giga), or -1 when a run
 * cannot hold it (Program Advances, unused records, a DarkChip, a chip of
 * BN5's or BN4's that sits out of BN6's battles). */
int chip_pool_tier(int rom_id);
/* A random chip of `tier` from BN6's chips (the nearest tier that has
 * some); -1 without a ROM. */
int chip_pool_pick(int tier);
/* ... from net `net`'s pool (CHIP_NET_*), of `tier` alone; -1 where it has
 * none of it (a game's without its ROM) */
int chip_pool_pick_net(int tier, int net);
/* ... of those of `tier` alone that `want` takes; -1 where it has none */
int chip_pool_pick_if(int tier, int net, bool (*want)(int rom_id));
/* How many chips net `net`'s pool holds of `tier` (-1: of every tier) */
int chip_pool_count(int net, int tier);
/* Net `net`'s chips in the order they were listed (BN6's by id; BN5's by
 * its rows; BN4's by BN4's ids; the whole BN5's, then BN4's others), into
 * `out` (`most` at most): how many */
int chip_pool_list(int net, short *out, int most);
/* The catalogue of another game's area (`xrom`: rom.h's XRomId), -1 none */
int chip_pool_net_of(int xrom);
/* Whether net `net`'s chips can be read now: its game's ROM (the whole's:
 * either's) */
bool chip_pool_net_read(int net);
/* Chip `rom_id`'s Library class (0 standard, 1 Mega, 2 Giga), or -1 when a
 * run cannot hold it; and how many chips of a class a run can hold. BN6's
 * chips alone: the classes' marks are BN6's (docs/game/meta/). */
int chip_pool_class(int rom_id);
int chip_pool_class_count(int cls);
/* Whether chip `rom_id` is one of the older Net's own a run can hold (an
 * id of the registry's own, BN5's rows' or BN4's: a standard, Mega or Giga
 * chip, no DarkChip and no Program Advance), whether it plays now or sits
 * out. The Library keeps them (library_pages.h lists those that play). */
bool chip_pool_older(int rom_id);

#endif

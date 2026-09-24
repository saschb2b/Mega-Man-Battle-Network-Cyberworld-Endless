/* The chips a run can find or buy: every standard, Mega and Giga chip of
 * the ROM's chip data, in tiers by rarity and class. */
#ifndef CW_CHIP_POOL_H
#define CW_CHIP_POOL_H

#define CHIP_TIERS 5

/* Chip `rom_id`'s tier (0 common standard .. 4 Giga), or -1 when a run
 * cannot hold it (Program Advances, unused records). */
int chip_pool_tier(int rom_id);
/* A random chip of `tier`; -1 without a ROM. */
int chip_pool_pick(int tier);

#endif

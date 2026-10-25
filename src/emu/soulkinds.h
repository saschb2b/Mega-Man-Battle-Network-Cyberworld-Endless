/* BN6's chips as BN5's UNITE reads them (the Souls track's S2; docs/
 * docs/multirom/souls.md): a Soul unites with the last chip
 * picked of its kind, and BN5 tells a chip's kind and whether it is a
 * DarkChip from its own record (BN5_CHIP_KIND, its +9's 0x20). A chip of
 * BN6's goes into BN5's battles as BN5's chip of its name (the guest
 * battles' rule, xchips.h), so its kind there is BN5's chip's: BN6's own
 * kind is taken only where BN5 has no chip of its name, read by name
 * (BN6 has neither Recovery nor Invisible, and numbers the rest otherwise:
 * soulkinds_bn5). The same chip lights the same Soul in both nets. */
#ifndef CW_SOULKINDS_H
#define CW_SOULKINDS_H

#include <stdbool.h>

#define SOULKIND_NONE 12   /* BN5_CHIP_KIND: no kind (BN5's 12) */

/* BN6's kind (BN6_CHIP_ELEMENT: 0 Fire, 1 Aqua, 2 Elec, 3 Wood, 4 Plus,
 * 5 Sword, 6 Cursor, 7 Obstacle, 8 Wind, 9 Break, 10 none) as BN5 numbers
 * the kind of that name (BN5_CHIP_KIND); SOULKIND_NONE for none and for
 * any other number */
int soulkinds_bn5(int bn6_kind);

/* Each chip a folder holds (1-511: BN6's, and the registry's of BN5's,
 * chips.h) its kind and DarkChip as BN5 has them, from both ROMs' records
 * and names (R, XR; the records as the game reads them): once, after both
 * are read; false where BN5's ROM is not */
bool soulkinds_make(void);
/* (after soulkinds_make) BN6 chip `chip`'s kind as BN5 has it, SOULKIND_NONE
 * where none; and whether BN5 counts it a DarkChip */
int soulkinds_of(int chip);
bool soulkinds_dark(int chip);

#endif

/* Ten-bit chip ids in the core's copy (unit 1 of docs/multirom/bn4-chips.md,
 * Past 511): BN6's code that takes a chip's id in nine bits patched to
 * ten, the census's sites (tools/chip_ids.py, docs/rom/chips-code.md,
 * Ten-bit chip ids), and the preset folders' entries written in ten bits;
 * and what an id past 511 needs beyond its bits (docs/rom/chips-code.md,
 * The pack past 511): the folder editor's items as id << 6 | code, every
 * reader of a name or a description over the four archives, the Library's
 * flag left unset and a reward's wide form split.
 * Where chip_entry.h's CHIP_ID_BITS is 9, nothing. All or nothing: every
 * halfword is read first and must be BN6's own, or already ours. Not in a
 * state: every boot writes the same. */
#ifndef CW_XCHIPS_WIDE_H
#define CW_XCHIPS_WIDE_H

#include <stdbool.h>

/* Once a core as it starts, after the chip registry (xchips_install.c);
 * false where the core's copy holds neither BN6's code nor ours. */
bool xchips_wide_install(void);

/* A chip past 511 given to the pack or with a folder (BN6's GiveChips,
 * which would set a flag past the Library's 512): told as an event of
 * `kind`, its r0 the chip's id, for whoever keeps the Library; 0 none. */
void xchips_wide_given(int kind);

#endif

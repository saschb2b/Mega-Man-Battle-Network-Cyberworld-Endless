/* Ten-bit chip ids in the core's copy (unit 1 of docs/multirom/bn4-chips.md,
 * Past 511): BN6's code that takes a chip's id in nine bits patched to
 * ten, the census's sites (tools/chip_ids.py, docs/rom/chips-code.md,
 * Ten-bit chip ids), and the preset folders' entries written in ten bits.
 * Where chip_entry.h's CHIP_ID_BITS is 9, nothing. All or nothing: every
 * halfword is read first and must be BN6's own, or already ours. Not in a
 * state: every boot writes the same. */
#ifndef CW_XCHIPS_WIDE_H
#define CW_XCHIPS_WIDE_H

#include <stdbool.h>

/* Once a core as it starts, after the chip registry (xchips_install.c);
 * false where the core's copy holds neither BN6's code nor ours. */
bool xchips_wide_install(void);

#endif

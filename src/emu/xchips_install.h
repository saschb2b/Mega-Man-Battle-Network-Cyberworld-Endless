/* BN6's chip tables grown and moved in the core's ROM copy (issue #82;
 * docs/multirom/chip-registry.md; docs/engine/emulation/): the chips'
 * image (src/core/chips.h: 512 records, names and descriptions to 511,
 * BN5's art) at CHIPS_FREE, the word naming the records and the 23 naming
 * the archives pointed at it; BN6's pack routines split so ids 320-511
 * count in its extension (BN6_PACK_EXT), its list and totals taken to 511
 * and its clear clearing the extension too; its test of a pseudo-chip kept
 * to its own (0x170-0x19A), so a chip of 411 on is no pseudo-chip; and a
 * hook taking the chips that sit out of the battle's deck as it is dealt
 * (chips_sits_out).
 *
 * All or nothing: every word and halfword changed is read first and must
 * be BN6's own (or already the install's), the place the image goes must
 * be free (0xFF, as emu.c pads it) or the install's; then it is written.
 * Not in a state: every boot with the same ROMs writes the same. */
#ifndef CW_XCHIPS_INSTALL_H
#define CW_XCHIPS_INSTALL_H

#include <stdbool.h>

/* Once a core as it starts, and again where the ROMs read changed since
 * (BN5's put in on the ROMs screen: its chips for their placeholders), BN5's
 * chips' moved code installed first where its ROM is read (xchips_code.h);
 * true where the core's copy holds the chips' tables. */
bool xchips_install(void);
/* BN5's DarkChips of their own (the registry's XREG_DARK) dealt into the
 * battle's deck (on) where BN5's mood and its DarkChip rule shelve them
 * (soulmood.c), or sat out of it as before (off) */
void xchips_dark_shelved(bool on);

#endif

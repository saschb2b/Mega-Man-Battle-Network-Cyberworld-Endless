/* Battle Network 4: Red Sun (USA): its chips' texts and art, for the chip
 * registry's rows of BN4's (docs/rom/bn4.md, Its chips' texts and art;
 * docs/multirom/bn4-chips.md, The build plan's unit 2), beside bn4.h's
 * records and names, which includes it. */
#ifndef CW_BN4_CHIPS_H
#define CW_BN4_CHIPS_H

#define BN4_CHIP_DESCS    0x0874CBE0u /* text archive: the chips' descriptions, ids 0-255: E7 02, F0 00 00, three lines by E8, E6 01, */
#define BN4_CHIP_DESCS_HI 0x0874F03Cu /* ... its end 0xE5 as every entry of BN4's; ids 256-368, none past them */
#define BN4_CHIP_DESC_LAST 368        /* the last id with a description */
#define BN4_TEXT_END      0xE5        /* an entry's end in BN4's archives (BN6's and BN5's 0xE6), */
#define BN4_TEXT_LINE     0xE8        /* ... and a new line in one (theirs 0xE9) */
#define BN4_CHIP_ICON_PAL 0x08747FF8u /* the chips' icons' palette, 16 colours: 0-8 and 10-15 BN5's, 9 its own outline's */
#define BN4_CHIP_KEY_XOR  0x64        /* the gate's key (BN4_CHIP_GATE, bn4.h): a chip's byte of the save's copy is its key byte XOR this */

#define BN4_CHIP_LIBRARY_ORDER 0x1C /* a chip record's +0x1C, u16: its place in BN4's Library (1 Cannon to 150 ColorPt, its secret standard chips to 186, the
                                    * Megas 201-280, the Gigas 281-290), as BN5's (tools/library_pages.py) */

#endif

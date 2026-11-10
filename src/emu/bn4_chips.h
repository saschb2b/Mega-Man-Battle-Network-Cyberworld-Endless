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

/* A chip's alignment, BN4's own rule (no field of BN6's: its +0x15 is the Library's number): as a chip is used
 * (0x0800D5D6: the side's karma, BN4_NAVI_KARMA, against BN4_KARMA_DARK) a light chip under it and a dark chip at it or
 * over play the dud instead, family 0x20 with its subfamily 4 (movs r0,#4; strb r0,[r7,#3]; movs r0,#0x20 at
 * 0x0800D5F6). 63 records light (the SP Navi chips, HolyPanl, Snctuary), 31 dark (Hole, DrkLine, the DS Navi chips);
 * 0x0800D608 the same test at 480 for a chip held (r0 its id), 1 where it may be used. A run takes no karma
 * (docs/multirom/bn4-chips.md, The DarkChips): the differential's reference is given it (karma4) */
#define BN4_CHIP_ALIGN        0x15        /* record +0x15: 0 any, 1 light, 2 dark */
#define BN4_CHIP_ALIGN_LIGHT  1           /* ... light: played at a karma of BN4_KARMA_DARK or over */
#define BN4_CHIP_ALIGN_DARK   2           /* ... dark: played under it */
#define BN4_CHIP_USE_TEST     0x0800D5D6u /* the test as a chip is used, in its family's choice (to 0x0800D606) */
#define BN4_CHIP_HELD_TEST    0x0800D608u /* ... of a chip held (r0), at 480 */
#define BN4_CHIP_DUD_FAMILY   0x20        /* the dud's family, its subfamily 4 */
#define BN4_NAVI_KARMA        0x36        /* NaviStats +0x36, u16: MegaMan's karma (a side's read by 0x0800F5A6, written by 0x0800F5B0); the
                                           * battle's copy the one the test reads */
#define BN4_KARMA_DARK         0x1D6       /* dark under it: 470, BN5's dark meter's number; the steps of 0x0800F5BC at 480 and 500 too */

/* A barrier or aura set on an object, BN6_BARRIER_SET's twin (r0 its kind; its collision data's +4 the kind, and from
 * BN4_BARRIER_KINDS' row +0x20 its HP, +0x22 its aura and +0x24 its frames): BN4's kinds are numbered otherwise, its 6
 * (1, 200, 3000) BN6's 9, LifeAura and LifeAur one chip by their rows; its 7 (1, 300, 3000), BlakBarr's, no kind of BN6's */
#define BN4_BARRIER_SET       0x08012DCEu
#define BN4_BARRIER_KINDS     0x080185E0u /* three u16 a kind, as BN6_BARRIER_KINDS: 1 Barrier's 10, 2 100, 3 200, 4 150, 6 LifeAura's, 7 BlakBarr's */
#define BN4_COLL_BARRIER_KIND  0x04        /* collision data: u8 its barrier's kind (BN6's +6) */
#define BN4_COLL_BARRIER_HP    0x20        /* ... u16 its HP (BN6's +0x16, a u8) */
#define BN4_COLL_BARRIER_AURA  0x22        /* ... u16 the damage that breaks an aura (BN6's +0x17, a u8) */
#define BN4_COLL_BARRIER_FRAMES 0x24       /* ... u16 its frames (BN6's +0x1A) */

#endif

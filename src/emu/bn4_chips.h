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

/* Its support chips' code (docs/rom/bn4.md, Its support chips' code; docs/multirom/bn4-chips.md): MegaMan's support state
 * runs the chip's routine from one table by its attack's subfamily, support chips and Navi chips alike, where BN6 has a
 * table each; a routine spawns a fourth-kind object of BN6's support routines' shape, the element at BN4's place */
#define BN4_CHIP_SUPPORT_FAMILY 0x0C      /* a support or Navi chip's family (its record's +0xB): MegaMan's state BN4_SUPPORT_RUN */
#define BN4_SUPPORT_RUN       0x080EB9EAu /* that state (BN4_PLAYER_STATES' entry 0x0C): the chip's routine, then the attack's end in the same
                                           * frame (BN6_SUPPORT_RUN ends it a frame later) */
#define BN4_CHIP_ROUTINES     0x080220E0u /* its routines by subfamily, BN4_CHIP_ROUTINE_COUNT words (BN6_CHIP_ROUTINES' twin) */
#define BN4_CHIP_ROUTINE_COUNT 0x4E
/* A support chip's object's generic states, which every such type's tables name (BN6's the same places of its own types'
 * tables, BN6_SUPPORT_OBJECT_*: tools/xchiprecipe_bn4.py TWINS, each pair read in both ROMs) */
#define BN4_SUPPORT_OBJECT_BEGIN 0x08008BD8u /* its state 0 (+8): its side's turn of the time freeze asked for */
#define BN4_SUPPORT_OBJECT_WAIT 0x08008BF8u /* its step 0 (+9): the other side's turn waited out */
#define BN4_SUPPORT_OBJECT_NAME 0x08008C2Au /* its step 4: the chip's name shown (BN6's plays sound 0x173 there, BN4's none) */
#define BN4_SUPPORT_OBJECT_DONE 0x08008EB8u /* its step 12: its own work done, its state 8 */
#define BN4_SUPPORT_OBJECT_END 0x08008EEEu /* its state 8: its turn given back, the object freed */
#define BN4_PANEL_BREAK_FREE  0x08009BACu /* r0, r1 a panel: broken (type 1) where it is a floor (0x10) no occupant's flag (0x0F8A0080) is on, sound
                                           * 0x95, r0 1; else r0 0. BN6_PANEL_BREAK_FREE's twin but for its test of flag 0x20000, BN4's type 5's */
/* PanlSht1-3 (its chips 178-180, support subfamily 0x26), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_PANLSHT           0x080E7A58u /* its routine: the fourth kind's BN4_T4_PANLSHT spawned */
#define BN4_T4_PANLSHT        0x4F        /* its object, its routine 0x080E79B8 (to 0x080E7BF0): by its +4 (the record's first parameter) the panel
                                           * ahead, the panels of the row, or the four around MegaMan, each a shot (0x080E7A9C, 0x080E7AD2, 0x080E7B3C) */
#define BN4_PANLSHT_SHOT      0x080DCC2Au /* a shot spawned (r0, r1 its panel, r2 its element, r4 its wait, r6 its damage, r7 its flag's byte) */
#define BN4_T3_PANLSHT_SHOT   0x7B        /* the shot, the third kind's (0x080DCAA4 to 0x080DCC5C): its panel broken (BN4_PANEL_BREAK_FREE), it rises 16
                                           * pixels, waits, flies off down its row at 10 pixels a frame, ending on its first hit or off the screen;
                                           * sounds 0x1C6 as it is made and 0xBF as it flies, sprite list 3's 0x38 (BN6's 0x33 the same bytes) */
/* ShotStar (its chip 210, support subfamily 0x19), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_SHOTSTAR          0x080E5D78u /* its routine: the fourth kind's BN4_T4_SHOTSTAR spawned */
#define BN4_T4_SHOTSTAR       0x34        /* its object (0x080E5C90 to 0x080E5E38): thirty stars, one every ten frames, each on the next panel
                                           * of the other side's that holds something (0x080E5D9A, its masks by side at 0x080E5E10) */
#define BN4_STAR_SPAWN        0x080D879Au /* a star spawned (r0, r1 its panel, r2 its element, r4 its kind, r6 its damage): through 0x080D877C */
#define BN4_T3_STAR           0x56        /* the star, the third kind's (0x080D85AA to 0x080D87B0): it falls from 16 pixels a frame up and right
                                           * of its panel, strikes there, and bursts (effect 6, sound 0x70) where it struck nothing; sound
                                           * 0x146 as it is made */
#define BN4_STAR_KINDS        0x080D8580u /* ... its kinds, 6 bytes each, 7 of them: its sprite's number in list 3, its animation, its collision's
                                           * two arguments; ShotStar's is kind 1 (list 3's 0x31) */
#define BN4_T4_STAR_MARK      0x35        /* the star's mark on its panel, the fourth kind's (0x080E5E38 to 0x080E5EF4), its spawner 0x080E5EEA */
/* Its misc chips' code: MegaMan's state 0x20 runs the chip's routine by its subfamily and ends the attack in the same frame
 * (subfamily 5 after 0x28 frames), BN6_MISC_RUN's twin instruction for instruction */
#define BN4_CHIP_MISC_FAMILY  0x20
#define BN4_MISC_RUN          0x080EC972u
#define BN4_MISC_ROUTINES     0x080EC9CCu /* its routines by subfamily, BN4_MISC_ROUTINE_COUNT words */
#define BN4_MISC_ROUTINE_COUNT 0x12
#define BN4_BATTLE_OBSTACLES  0x68        /* the battle state's obstacles by side, three words a side (BN6's +0xA0) */
#define BN4_OBSTACLE_ADD      0x0800B230u /* BN6_OBSTACLE_ADD's twin but for that offset */
#define BN4_REPAIR            0x0800C720u /* Repair's routine (its chip 115, misc subfamily 0x0B): each of its side's obstacles with HP left given
                                           * its full HP (+0x26 into +0x24), effect 7 over it, sound 0x9A */
/* Counter1-3 (its chips 60-62, misc subfamily 0), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_COUNTER           0x080CFE02u /* its routine, in its hit's code: each of the other side's players (the battle state's +0x44 on, by
                                           * its side ^ 1 ^ MegaMan's) whose collision data's counter byte (BN4_COLL_COUNTER) is set is struck
                                           * on its panel through 0x080CFDE4 */
#define BN4_T3_COUNTER_HIT    0x17        /* the hit, the third kind's (0x080CFD48 to 0x080CFE5C): sprite list 5's 0x10, sound 0xBE, the hit
                                           * area spawned (0x080CD7E2) with its damage, gone after 0x25 frames */
#define BN4_AI_LOCKOUT        0x3A        /* MegaMan's AI data: u16 the frames he cannot act after a chip, its record's +0x14, stored as the
                                           * attack ends (BN4_CHIP_ATTACK_END; BN6's +0x19, a byte) */
#define BN4_CHIP_ATTACK_END   0x0800C9FCu /* a chip's attack ended: the lock-out kept, his action back to 6 (BN6_CHIP_ATTACK_END) */
/* WhitWeb1-3 (its chips 72-74, misc subfamilies 8, 9 and 0xA), moved as one unit (src/emu/xchips_bn4.c) */
#define BN4_WHITWEB           0x080D4F20u /* WhitWeb1's routine, WhitWeb2's 0x080D4F2A and WhitWeb3's 0x080D4F34 after it: each hands a row
                                           * (1, 2, 3) to 0x080D4EC4, which lays a web on each of the other side's panels of that row */
#define BN4_T3_WEB            0x3D        /* the web, the third kind's (0x080D4D30 to 0x080D4F40, the three routines in its code): on its
                                           * panel it strikes what stands there; sound 0xF7 */
/* Boomer1-3 (its chips 75-77, misc subfamily 2), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_BOOMER            0x080D344Eu /* its routine, in the boomerang's code: the boomerang spawned (0x080D3430) */
#define BN4_T3_BOOMERANG      0x32        /* the boomerang, the third kind's (0x080D3234 to 0x080D3468): it flies round the field's edge by its
                                           * row of BN4_BOOMERANG_FLIGHTS, striking what it meets; sprite list 4's 0x14, sound 0xAB */
#define BN4_BOOMERANG_FLIGHTS 0x080D31ECu /* ... its flights, six rows of three words before its routine: its timer's two bytes, its speed along
                                           * the rows, its speed across them (16.16) */
/* Meteors1-3 (its chips 163-165, misc subfamily 0x11), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_METEORS           0x080EAE92u /* its routine, after its controller's code: a panel aimed at (BN4_ROW_PANEL_ACROSS) and, where one
                                           * is found, the controller spawned on it (0x080EAE6C: its panel, element and damage) */
#define BN4_T4_METEORS        0x81        /* its controller, the fourth kind's last type (0x080EADF4 to 0x080EAE6C): five meteors (+0x22), one
                                           * every 0x14 frames (+0x20), each BN4_T3_STAR's kind 6 through 0x080D877C on its own panel */
#define BN4_ROW_PANEL_ACROSS  0x0800FC48u /* r5 an object: a panel of its row on the side across found by BN4's 0x0800A1AE with a required and a
                                           * refused word of panel flags by its side (0x0800FC84, then 0x0800FC94, eight bytes a side); r0 0
                                           * none (BN6_ROW_PANEL_ACROSS the same instruction for instruction, the same words) */

#endif

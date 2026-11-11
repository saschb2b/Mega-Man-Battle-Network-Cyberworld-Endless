/* Battle Network 4: Red Sun (USA): its Hole panel, a panel's step and blink, the panel changer behind every chip that
 * sets panels and a DS Navi chip's power, which counts the field's Holes (docs/rom/bn4.md, The panel changer and the
 * Hole panel; docs/multirom/bn4-chips.md, The dark units, read), beside bn4.h's panels, which includes it. */
#ifndef CW_BN4_PANELS_H
#define CW_BN4_PANELS_H

#define BN4_PANEL_HOLE    0x0B        /* its type 0xB, the Hole panel (Hole's, DrkLine's and DrkStage's: BN4_PANEL_CHANGER_KINDS' rows 9, 0xA, 0xC):
                                       * plain's word (0x10010), so a floor like any plain panel; set by the changer and at 0x080CB4C4,
                                       * 0x080CB4F2, 0x080D5572, 0x080DC912 and 0x080E80BA; read at BN4_HOLE_CLEANSE, BN4_ROW_HAS_HOLE,
                                       * BN4_ROW_HAS_FLOOR, 0x08023B80 (a random panel picker leaves a Hole out) and 0x080EDF76 (three panels
                                       * of a column turned plain again), and counted for a DS Navi chip's power (BN4_DS_POWER); never timed
                                       * (BN4_PANEL_STEP); no type of BN6's */
#define BN4_HOLE_CLEANSE  0x0801332Au /* an object (r5) standing on a Hole whose side's karma (0x0800F5A6) is over 499: the panel plain again
                                       * (BN4_SET_PANEL_TYPE with 2, at 0x08013354), after two tests not read (0x0800D7CA's, 0x0800F49C's) */
#define BN4_ROW_HAS_HOLE  0x08024830u /* whether row r1 has a Hole among its columns 1-6 (1 where it has); and */
#define BN4_ROW_HAS_FLOOR 0x08024850u /* ... whether it has a panel that is none: two random panel pickers' tests (0x080247A0, 0x080247F0) */
#define BN4_PANEL_COUNT   0x08009F40u /* how many panels of type r0 the side r1 holds (rows 1-3, columns 1-6) */
#define BN4_DS_POWER      0x08019518u /* a DS Navi chip's power (r0 its id): the field's Holes, both sides' by BN4_PANEL_COUNT, 10 at most; the
                                       * record's power less BN4_DS_POWER_FIRST its row of BN4_DS_POWERS, 10 less the Holes its column */
#define BN4_DS_POWERS     0x08018B48u /* ... 20 rows of 11 u16, RollDS (1023) to VideMnDS (1042): the first column ten Holes' power, the last
                                       * none's (RollDS 80 to 30, GutsMnDS 200 to 100, MetlMnDS 300 to 200), a step a Hole */
#define BN4_DS_POWER_FIRST 1023       /* ... the first DS chip's computed power's number (a record's +0x1A) */
#define BN4_PANEL_STEP    0x08009740u /* a panel's own step each frame (r0 column, r1 row), by its type: a hole (1) closes when its +0xC has
                                       * counted 600 down, a cracked one (3) by what stands on it, its type 8 turns plain when its +0xE
                                       * has counted 0x3C0, a holding one (0xA) when BN4_PANEL_TIMER has; every other type (poison, holy, the
                                       * Hole) only has those counters set again: it stays until something sets another */
#define BN4_PANEL_BLINK_SET 0x08009E38u /* a panel (r0 column, r1 row) drawn as type r2 on side r3 this frame, a changer's blink before the type
                                       * is set: BN4_PANEL_BLINKS' byte's low nibble the type | 0x80, the byte 0x28 on the side | 0x80; BN6
                                       * keeps both in the panel (BN6_PANEL_BLINK_SET: its +8 the type, +9 the side, +0xD 1) */
#define BN4_PANEL_BLINKS  0x02037B36u /* ... a byte a panel (row * 8 + column), the sides' 0x28 after */
/* The panel changer (docs/rom/bn4.md, The panel changer and the Hole panel): one fourth-kind object behind every chip that
 * sets panels, its kind (+4) a row of BN4_PANEL_CHANGER_KINDS: the panels found by the row's shape, blinked while its
 * frames run (BN4_PANEL_BLINK_SET every fourth frame), then set (BN4_SET_PANEL_TYPE) */
#define BN4_T4_PANEL_CHANGER 0x1F     /* its type */
#define BN4_PANEL_CHANGER 0x080E4354u /* its routine (to 0x080E4584); its states' table at 0x080E4368 */
#define BN4_PANEL_CHANGER_SPAWN 0x080E4568u /* one spawned (r0 column, r1 row, r4 its kind, r3 a byte it clears as it ends) */
#define BN4_PANEL_CHANGER_KINDS 0x080E423Cu /* its kinds, BN4_PANEL_CHANGER_KIND_COUNT rows of 0x14 bytes: +0 the panel type, +1 the shape (its
                                       * routine's place in the table at 0x080E43B8: 0 every panel of a side, 4 the object's row, 8 the panel
                                       * ahead, 0xC its own), +2 whether the blink skips frames, +3 its frames, +4 two words a side, the panel
                                       * search's flags (0x08009EAC). Its rows: 0 plain (PnlRetrn), 6 holy ahead (HolyPanl), 7 holy on its side
                                       * (Snctuary), 8 poison under it, 9 a Hole ahead (Hole), 0xA Holes along its row (DrkLine), 0xC Holes on
                                       * its whole side and 0xD poison on the other side's (DrkStage's two) */
#define BN4_PANEL_CHANGER_KIND_COUNT 14
#define BN4_T4_PANEL_CHIP 0x20        /* the panel chips' object (0x080E4584, support subfamily 0x0B's routine 0x080E45F6 spawns it): one
                                       * changer of the kind its +4 holds, the chip's first parameter (Hole 9, DrkLine 0xA) */
#define BN4_T4_DRKSTAGE   0x6A        /* DrkStage's object (0x080E9838, support subfamily 0x3B's routine 0x080E98BC): two changers at once,
                                       * kinds 0xC and 0xD: Holes on MegaMan's side, poison on the other */
#define BN4_T4_BLAKWEAP   0x7E        /* BlakWeap's object (0x080EABFC, support subfamily 0x4B's routine 0x080EAC76): one third-kind 0x5A
                                       * spawned by 0x080CCA9C; its record (289) has no Library number and BN4's blank picture */

#endif

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
/* A Navi chip (docs/rom/bn4.md, Its Navi chips): its routine there spawns a fourth-kind object of the Navi's own, BN6's
 * one Navi chip object's twin state for state (BN6_CHIP_NAVI_OBJECT), which steps MegaMan out, spawns the Navi as his chip
 * shows him (a first-kind type of his own, not the type he fights as), waits for his word and steps MegaMan in */
#define BN4_CHIP_NAVI_STEP    0x080E2D26u /* MegaMan stepped out (r1 1) or in (r1 0): r0 his side; the fourth kind's 0x0C (0x080E2C24),
                                             BN6_CHIP_NAVI_STEP's part */
#define BN4_REGION_SPAWN_FROZEN 0x080CD81Eu /* a collision region spawned (0x080CD7E2) and its +0 given bit 0x10 (BN6_REGION_SPAWN_FROZEN) */
#define BN4_GUTSMAN_CHIP_OBJECT 0x080E31E8u /* GutsMan's chip's object (the fourth kind's 0x10; its routine BN4_CHIP_ROUTINES' 5, 0x080E3304) */
#define BN4_GUTSMAN_CHIP_NAVI 0x080B999Cu /* ... and the chip's GutsMan (the first kind's 0x14): he appears (sound 0xB0), strikes the
                                             ground at frame 20 of 90 (his shockwave's spawner, 0x080D0894: the third kind's 0x0B, the
                                             Navi's own; kind 1, or 2 for his SP and DS by his +4) and a hit before him, and is gone */
#define BN4_GUTSMAN_CHIP_SPAWN 0x080B9B1Cu /* ... his spawner: r0, r1 his panel, r2 his element, r5 his user, r6 his power, r7 the byte he
                                             sets to 1 and clears when he is done */
/* GutPnch1-3 (its chips 154-156, support subfamily 0x36): a Navi chip by its shape, moved as a unit of kind navi */
#define BN4_GUTPNCH_OBJECT    0x080E90CCu /* its object (the fourth kind's 0x64; its routine BN4_CHIP_ROUTINES' 0x36, 0x080E91DC): GutsMan's
                                             chip's object state for state, but that it hands its Navi the parameters whole and plays
                                             BN4_GUTPNCH_IN_SOUND as it steps MegaMan in */
#define BN4_GUTPNCH_IN_SOUND  0x080E91BCu /* ... movs r0,#0xb0 in its last state, after its step: a sound BN6's Navi object has none of */
#define BN4_GUTPNCH_NAVI      0x080C83F8u /* ... and the chip's Navi (the first kind's 0x42, to 0x080C85E8): GutsMan's sprite and palette, he
                                             appears (sound 0xB0), punches the panel ahead at frame 0xC of his state (sound 0xA4;
                                             BN4_REGION_SPAWN_FROZEN, its bits BN4_GUTPNCH_BITS) and is gone */
#define BN4_GUTPNCH_BITS      0x080C855Au /* ... movs r7,#0x21: the punch's bits, struck and pushed (BN4_STRUCK_PUSHED) */
#define BN4_GUTPNCH_SPAWN     0x080C85C2u /* ... his spawner, GutsMan's chip's shape (BN4_GUTSMAN_CHIP_SPAWN) */
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
#define BN4_MISC_WAIT         0x080EC9ACu /* ... its wait: cmp r0,#5 on the subfamily, then movs r0,#0x28 into the misc state's +0x10
                                           * (0x080EC9B0), counted down a frame each to 0 before the attack's end (SidBmbo's alone) */
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
#define BN4_BOOMER_SPAWN      0x080D3430u /* ... its spawner (r0 the column, r1 the row, r2 the element, r3 the height, r4 its parameters: the
                                           * flight's row in the low byte, r6 the power word): Boomer's routine's call, and PitRng's launcher's,
                                           * whose unit copies it alone and spawns Boomer's copy of the type (tools/xchiprecipe_bn4.py SHARED3) */
/* PitRng1-3 (its Program Advances 332-334, misc subfamily 0x0D: three Boomer1, 2 or 3 of codes in a row), moved as a unit */
#define BN4_PITRNG            0x080E9A36u /* its routine, in its launcher's code: the launcher spawned (movs r0,#0x6c; BN4_SPAWN_4), its element
                                           * from r2 at +0x17, the power word from r6 at +0x2C, its side its user's; shorter than the support
                                           * routines' common spawner, as the launcher takes no panel */
#define BN4_T4_PITRNG         0x6C        /* the launcher, the fourth kind's (0x080E999C to 0x080E9A50), drawn by nothing: BN4_PITRNG_COUNT
                                           * boomerangs (BN4_BOOMER_SPAWN), one each BN4_PITRNG_EVERY + 1 frames from its side's first column,
                                           * the bottom row and the top row in turn, each on its own flight (its count less one), then gone */
#define BN4_PITRNG_COUNT      0x080E99E4u /* ... movs r0,#3 into its +0x22: three boomerangs */
#define BN4_PITRNG_EVERY      0x080E99F8u /* ... movs r0,#0xf into its +0x20, counted down a frame each: sixteen frames apart */
/* Meteors1-3 (its chips 163-165, misc subfamily 0x11), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_METEORS           0x080EAE92u /* its routine, after its controller's code: a panel aimed at (BN4_ROW_PANEL_ACROSS) and, where one
                                           * is found, the controller spawned on it (0x080EAE6C: its panel, element and damage) */
#define BN4_T4_METEORS        0x81        /* its controller, the fourth kind's last type (0x080EADF4 to 0x080EAE6C): five meteors (+0x22), one
                                           * every 0x14 frames (+0x20), each BN4_T3_STAR's kind 6 through 0x080D877C on its own panel */
#define BN4_ROW_PANEL_ACROSS  0x0800FC48u /* r5 an object: a panel of its row on the side across found by BN4's 0x0800A1AE with a required and a
                                           * refused word of panel flags by its side (0x0800FC84, then 0x0800FC94, eight bytes a side); r0 0
                                           * none (BN6_ROW_PANEL_ACROSS the same instruction for instruction, the same words) */
/* MokoRus1-3 (its chips 130-132, misc subfamily 1), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_MOKORUS           0x080D252Au /* its routine, in the Moko's code: the three rows shuffled (BN4_SHUFFLE), a Moko spawned on each
                                           * (0x080D250C), the k-th to set off 10 x k frames on (+0x20) */
#define BN4_T3_MOKO           0x2C        /* the Moko, the third kind's (0x080D235C to 0x080D2568): it runs down its row from the field's edge,
                                           * striking what it meets, struck and pushed back (its collision set up with the hit's bits 0x21 at
                                           * BN4_MOKO_HIT_BITS); sprite list 1's 0x11, sound 0xC7 as it sets off */
#define BN4_MOKO_HIT_BITS     0x080D23E0u /* ... its init's movs r3,#0x21 before its collision set up (BN4_COLL_SETUP_ARG): BN4_STRUCK_PUSHED's
                                           * bits, 0x61 in its copy (BN6_STRUCK_PUSHED's) */
#define BN4_SHUFFLE           0x08000AAAu /* bytes shuffled (r0 them, r1 how many, r2 the swaps): BN4_RNG_DRAW_POSITIVE twice a swap, each
                                           * modulo the count, the two bytes swapped (BN6_SHUFFLE, the same instructions); MokoRus's rows' */
#define BN4_STRUCK_PUSHED     0x0801360Cu /* the struck pass of the first kind (from 0x080135E6, r4 the struck's hit bits, BN4_COLL_HIT_BITS):
                                           * a push bit (0xFC) with bit 0x01 gives the status 0x100 and clears the flinch's 4 (to 0x08013622),
                                           * one without it the push alone's 0x10 (0x08013628-0x08013634). BN6_STRUCK_PUSHED: bit 0x40 where
                                           * BN4 reads bit 0x01 */
/* SidBmbo1-3 (its chips 78-80, misc subfamily 5), moved as a unit (src/emu/xchips_bn4.c), the one BN4's misc run waits for */
#define BN4_SIDBMBO           0x080D5DDAu /* its routine, in the bamboo's code: the bamboo spawned (0x080D5DBA) */
#define BN4_T3_SIDBMBO        0x44        /* the bamboo, the third kind's (0x080D5C50 to 0x080D5DF4); sprite list 1's 0xC, sound 0x8E */
/* Its attack families: a chip's family (its record's +0xB) is a state of MegaMan's (BN4_PLAYER_STATES by the number), run with
 * his object in r5; the state makes r7 the attack's variables itself (his AI data's +0x70: BN6's RunAIAttack hands a family
 * its own, his AI data's +0xA0). One moved whole is an attack unit (src/emu/xchips_bn4.h XC4_FIRST_ATTACK) */
#define BN4_COUNTER_WINDOW    0x0800BA66u /* MegaMan's counter window opened as a chip's attack begins: 0x10 to his collision data's counter byte
                                           * (BN4_COLL_COUNTER, through 0x0800AFBC) where the battle's kind (the battle state's +0xF) is 0x46 or
                                           * over (BN6_COUNTER_WINDOW) */
#define BN4_ELEMENT_SET       0x0801288Cu /* r0 an element: the object's +0x17 and its collision data's BN4_COLL_ELEMENT (BN6_ELEMENT_SET) */
#define BN4_COLL_ELEMENT      0x02        /* collision data: u8 its element (BN6_COLL_ELEMENT, the same place) */
#define BN4_T3_ARM            0x02        /* MegaMan's arm while he uses a chip, the third kind's (0x080CD5F8 to 0x080CD728): its kind (+4) a
                                           * row of BN4_T3_02_KINDS, its animation +5, drawn at its parent's place (+0x4C) until the slot it
                                           * was given (+0x60: its parent's +0x4C) reads 0. BN6's is a first-kind object (BN6_ARM_SPAWN) */
#define BN4_ARM_SPAWN         0x080CD6E0u /* ... its spawner (r4 the animation << 8 | the kind, r7 the slot), in its code */
/* The spread chips (Spreader 18, HeatShot 19 to BublSide 24, DrkSpred 192; HeatSprd 321 and BubSprd 322: family 0x1E, subfamily 0
 * plain, 1 heat, 2 bubble, 3 dark), moved as a unit (src/emu/xchips_bn4.c) */
#define BN4_CHIP_SPREAD_FAMILY 0x1E
#define BN4_SPREAD_STATE      0x080EC760u /* the state (0x080EC760 to 0x080EC842): phase 0 his animation 0x0E, the counter window, the arm, then
                                           * after BN4_SPREAD_WAITS' frames the shot's sound 0x8D and the shot; phase 4 0x14 frames, then the
                                           * attack's end. BN6's family 0x25 (BN6_SPREAD_FAMILY) is its twin but for the arm */
#define BN4_SPREAD_ARMS       0x080EC814u /* ... its arm's kind by subfamily: 0x0A, 0x06, 0x0E, 0x32 */
#define BN4_SPREAD_WAITS      0x080EC818u /* ... the frames before the shot by subfamily: 1, 10, 10, 1 */
#define BN4_SPREAD_HEIGHTS    0x080EC81Cu /* ... the shot's height by subfamily (<< 16): 0x10, 0x10, 0, 0x10 */
#define BN4_T3_SPREAD_SHOT    0x12        /* the shot, the third kind's (0x080CF420 to 0x080CF5B8; BN6's 0x12 too, BN6_T3_SPREAD_SHOT): unseen,
                                           * a panel a frame along its row; where it strikes, its damage, then BN4_EFFECT_PATTERN on the panels
                                           * of its kind's shape with its kind's effect and sound */
#define BN4_SPREAD_SHOT_SPAWN 0x080CF596u /* ... its spawner, in its code (r0, r1 its panel, r2 its element, r3 its height, r4 its kind, the
                                           * chip's first parameter, r6 its damage) */
#define BN4_SPREAD_SHOT_KINDS 0x080CF3DCu /* ... its kinds, 17 rows of 4 bytes before its routine: the burst's shape (2 the panel behind, 0x0D
                                           * the two behind a row up and down, 4 above and below, 0x0F all round), its collision's argument,
                                           * the burst's effect, its sound; 0-3 plain, 4-7 bubbles, 8-0xB fire, 0xF DrkSpred's */
/* FlmLine1-3 (its chips 12-14) and FlmCros1-3 (329-331: family 0x22, subfamily 0 the line, 1 the cross), moved as a unit */
#define BN4_CHIP_FLMLINE_FAMILY 0x22
#define BN4_FLMLINE_STATE     0x080ECAAEu /* the state (0x080ECAAE to 0x080ECB74): phase 0 his animation 0x0D, the counter window, 0x20 frames
                                           * counted, the arm (kind 0x0C); at 0x1B left his animation 0x11 and the flames two columns ahead
                                           * (BN4_FLAME_LINE, or BN4_FLAME_CROSS for subfamily 1); phase 4 10 frames, then the attack's end */
#define BN4_T3_FLAME          0x19        /* a flame, the third kind's (0x080CFF80 to 0x080D014E): on its panel, where there is one, for its
                                           * +5 frames (the chip's second parameter, 0x28), striking what stands there; sprite list 4's 0x0A,
                                           * sound 0x9D */
#define BN4_FLAME_LINE        0x080D00D8u /* ... three of them, on (r0, r1) and the panels above and below it, in its code */
#define BN4_FLAME_CROSS       0x080D00F8u /* ... five: those three and the panels before and behind (FlmCros's) */
/* Thunder1-3 (its chips 57-59: family 0x26), moved as a unit with BN4's own thunder (bn4.h BN4_THUNDER, the third kind's 0x2A,
 * which Billy's recipe runs BN6's object for: tools/xchiprecipe_bn4.py UNIT_MOVES) */
#define BN4_CHIP_THUNDER_FAMILY 0x26
#define BN4_THUNDER_STATE     0x080ECE14u /* the state (0x080ECE14 to 0x080ECEAE): his animation 0x12, the counter window; on its second frame
                                           * the thunder a column ahead (BN4_SPAWN_THUNDER: r4 the chip's parameters, r3 its height 0xA << 16);
                                           * after five frames phase 4, 0x1E frames, then the attack's end. No arm */
/* TwnFng1-3 (its chips 69-71) and BstFang1-3 (335-337: family 0x34, subfamily 0 one volley, 1 eight), moved as a unit */
#define BN4_CHIP_TWNFNG_FAMILY 0x34
#define BN4_TWNFNG_STATE      0x080ED4C6u /* the state (0x080ED4C6 to 0x080ED572): phase 0 his animation 0x12, the counter window, its volleys
                                           * counted (7 x its subfamily + 1); phase 4 a volley every 10 frames, two fangs from his panel
                                           * (BN4_FANG_SPAWN, kinds 0 and 1); phase 8 0x1E frames, then the attack's end. No arm */
#define BN4_T3_FANG           0x51        /* a fang, the third kind's (0x080D79B8 to 0x080D7B64): from his panel to the row below (kind 0) or
                                           * above (kind 1), 0x40000 a frame, then along it, 0xA0000 a frame, striking what it meets; sprite
                                           * list 4's 0x27, sound 0xA6; let go in effect 0x15 as the battle ends (0x08007A12, BN6's 0x0800A18E) */
#define BN4_FANG_SPAWN        0x080D7B3Au /* ... its spawner, in its code (r0, r1 its panel, r2 its element, r4 its kind, r6 its damage) */
/* MagBolt1-3 and MagShok1-3 (its chips 35-37 and 341-343, family 0x59), moved as an attack unit */
#define BN4_MAGBOLT_STATE     0x080EE012u /* MegaMan's state 0x59 (to 0x080EE1A2): phase 0 the arm (kind 0x23 + the subfamily, less 3 from 3 on)
                                           * and after 6 frames the bolt on the panel ahead (three from subfamily 3 on), sounds 0xF8 and 0xB9;
                                           * phase 4 the pull, 0x1E frames; phase 8 ten frames, then the attack's end */
#define BN4_MAGBOLT_PULL      0x080EE140u /* ... its pull's hit word, 0x1E05FF08: a hit area a frame over the row ahead (0x080CD7E2), its bits
                                           * 0x08, a push bit alone (BN4_STRUCK_PUSHED), toward him; MagShok's by his side at 0x080EE148 */
#define BN4_T3_BOLT           0x75        /* the bolt, the third kind's (0x080DBCA0 to 0x080DBD94): 0x1E frames at its parent's panel and its
                                           * offset, striking what stands there; sprite list 4's, let go as the battle ends */
#define BN4_BOLT_SPAWN        0x080DBD72u /* ... its spawner, in its code */
#define BN4_BOLT_HIT_BITS     0x080DBCD2u /* ... its init's movs r3,#3 before its collision set up: with the pull's push bit BN4_STRUCK_PUSHED's
                                           * bits, 0x43 in its copy (BN6_STRUCK_PUSHED's bit 0x40) */
#define BN4_STRUCK_STEP       0x08010610u /* strb r2,[r5,#0xb] in the struck-and-pushed state's first phase (the engine's state 5, 0x080105DC):
                                           * its step's panels to go, BN4's step routine's (0x0800ACAA) whatever the panel; the state waits
                                           * 0x18 frames where the panel cannot be entered (0x0801064E) */
#define BN4_RUNNER_SLIDE      0x08013E3Cu /* a virus's runner: a pushed object's slide run (status 0x10, or the slide's flag 0x1000: bl
                                           * 0x08010F00 at 0x08013E4C and 0x08013E5E), before: */
#define BN4_RUNNER_STRUCK     0x08013E68u /* ... status 0x100 made its state 5. BN6's runner the other way round (BN6_RUNNER_STRUCK) */
/* PropBom1-3 (its chips 157-159, family 0x62), moved as an attack unit */
#define BN4_PROPBOM_STATE     0x080EE532u /* MegaMan's state 0x62 (to 0x080EE5EA): 4 frames in his throw, then sound 0x8D and the bomb on the
                                           * panel ahead with its record's power; 0x1E frames more, then the attack's end */
#define BN4_PROPBOM_SPEEDS    0x080EE5BCu /* ... the bomb's speed by the battle NaviStats' Navi, 13 bytes of 0x14: 0x14000 a frame */
#define BN4_T3_PROPBOM        0x8C        /* the bomb, the third kind's (0x080DEB70 to 0x080DED60): 10 HP, its side's obstacle slot
                                           * (BN4_OBSTACLE_ADD with r2 1), sound 0x133; a hit taken ends it in effect 0 and sound 0xC8, past
                                           * the far column it blasts the two back columns; sprite list 4's 0x2B */
#define BN4_PROPBOM_SPAWN     0x080DED30u /* ... its spawner, in its code */
#define BN4_PROPBOM_BLAST     0x080DED2Cu /* ... its blast's hit word, 0x0405FF11 (a hit area over a pattern, 0x080CD7E2 with r7 3, centred
                                           * on column 5 by its side), with effect 0 over the same panels (BN4_EFFECT_PATTERN, pattern 0x11) */
#define BN4_OBSTACLE_REMOVE   0x0800B272u /* the object in r5 taken out of the battle state's six obstacle words (+0x68; BN6_OBSTACLE_REMOVE,
                                           * the same instructions at +0xA0) */
/* SandRing's family (its chip 47) and ThunSoul's charged shot (docs/multirom/bn4-souls.md, ThunSoul in BN6's engine), moved as
 * one unit for ThunSoul's kind alone (src/emu/xchips_bn4.c's ThunSoul) */
#define BN4_CHIP_RING_FAMILY  0x47        /* MegaMan's state 0x47 (BN4_PLAYER_STATES): the arm (none for kind 3), then the ring of the
                                           * attack's +0xC's kind (0x080EDCE2) */
#define BN4_RING_STATE        0x080EDC5Au /* the state (0x080EDC5A to 0x080EDD2E), its variables AI data +0x70 as every attack state's */
#define BN4_POWER_THUN        0x0800D298u /* power attack 0x58 (BN4_POWER_ATTACKS), ThunSoul's charged shot (BN4_FORM_LOADOUTS' form 6
                                           * +3): the attack's +2 and +3 3, +0xC 0 (the ring's kind 0), its damage 20 at +8, its
                                           * +0xA 0x8A, back BN4_CHIP_RING_FAMILY; no call, BN6's power attacks' shape */
#define BN4_T3_RING           0x6D        /* the ring, the third kind's (0x080DAFBC to 0x080DB0EC): its sprite 3:0x37, kind 3's 3:0x3A */
#define BN4_RING_KINDS        0x080DAFB0u /* ... its kinds, 4 rows of 3 bytes before its routine, read by its init by +4: its speed, a
                                           * flag (0x08013212 called where set), its sound (0x080DB05C plays it); kinds 0-2 sound
                                           * 0x91, 3 sound 0x8D; ThunSoul's ring kind 0 (seen in both traces) */
/* ... and TimeBomb (support 0x11, the chip 97): its placer, its bomb and the blasts */
#define BN4_T4_TIMEBOMB       0x27        /* its placer, the fourth kind's (0x080E4C3C to 0x080E4F5C; spawned by the subfamily's routine
                                           * 0x080E4CA6): a column of the foe's side (0x080E4D94 by the side's masks, their front
                                           * column in the specs), a row of it drawn among its free panels (0x080E4CC8: rows 3 to 1
                                           * listed, BN4_RNG_DRAW_POSITIVE at 0x080E4D4C, BN4_REMAINDER by their count), the bomb
                                           * spawned there with the chip's parameter as its HP (0x080E4E38). With no free panel its
                                           * way on (0x080E4DF2) reads two words at 0x00010010 + 8 x the side, no address of the
                                           * cartridge's or the RAM's (the literal at 0x080E4E60, a panel mask where 0x080E4E26
                                           * reads it): BN4's own, moved as it is, reached by no spec */
#define BN4_REMAINDER         0x08111888u /* swi 6; adds r0,r1,#0; bx lr: r0 mod r1 (BN6 has the three only inline, 0x08010C00) */
#define BN4_T3_TIMEBOMB       0x4B        /* the bomb, the third kind's (0x080D6BF8 to 0x080D6ED4): an actor, its update (0x080D6CA0)
                                           * BN4_STEP_BEGIN then BN4_ACTOR_RUN with its states; 50 HP, its side's obstacle slot, sound
                                           * 0xB0; its count 3 steps (60, 60 and 30 frames, sounds 0xE3, 0xE3, 0xE4), then a hit area
                                           * over the foe's area and the blasts (BN4_T4_BLASTS, sound 0xBC); a hit taken ends it in
                                           * effect 0 and sound 0x70; sprite list 3's 0x23 */
#define BN4_TIMEBOMB_STATES   0x080D6CCCu /* ... its seven states: its own the first, the third and the seventh (0x080D6CE8,
                                           * 0x080D6E6C, 0x080D6D58), the engine's generic 1, 3 and 4 and BN4_OBSTACLE_PUSHED the rest */
#define BN4_TIMEBOMB_SPAWN    0x080D6EA4u /* ... its spawner, in its code: r4 its HP */
#define BN4_ACTOR_RUN         0x08013F3Eu /* an actor's state run from the table in r0 by its CurAction (+9), after the engine's tests of
                                           * a time freeze and its statuses: the engine's states 0-5, an object's own from 6
                                           * (BN6_ACTOR_RUN: 0-7 and from 8) */
#define BN4_OBSTACLE_PUSHED   0x080112C4u /* an obstacle's state 5, pushed: three steps on +0xA, its slide 0x80000 a frame
                                           * (BN6_OBSTACLE_PUSHED: 0x60000, and its moves counted) */
#define BN4_T4_BLASTS         0x24        /* blasts over a list of panels, the fourth kind's (0x080E4A0A to 0x080E4AD4): one every +0x22
                                           * frames from the list at its +0x60, each an effect and a sound by its kind (+4) */
#define BN4_BLASTS_KINDS      0x080E4A00u /* ... its kinds, five of 2 bytes: the effect, the sound (0 none); TimeBomb's kind 1: 0x00, none */
#define BN4_BLASTS_SPAWN      0x080E4A9Cu /* ... its spawner: r0 the pattern (its low seven bits a row of BN4_PANEL_MASKS, bit 7 set
                                           * for any), r1 the frames between two, r4 the kind */
#define BN4_PANEL_MASKS       0x08012618u /* masks of the field's panels, rows of two words (BN6_PANEL_MASKS, the same 0x50 bytes) */

/* The bombs (its chips of family 9: MiniBomb, EnergBom, MegEnBom, Binder1-3, BugBomb, Ball, Geyser, BlkBomb, VDoll,
 * SeekBom1-3, DarkBomb): one state, a subfamily's spawner each. SeekBom1-3 (160-162, subfamily 9) moved as an attack unit
 * of its own with a copy of the state (tools/xchiprecipe_bn4.py ATTACK_SUBS) */
#define BN4_BOMBS_STATE       0x080EB3FEu /* MegaMan's state 9 (to 0x080EB51E): his animation 6, the arm (BN4_BOMBS_ARMS; none for Navis 5
                                           * and 0xA), sound 0x90; at its count's 9 the subfamily's entry of BN4_BOMBS_SPAWNERS called with
                                           * his place, the power and the element; 0x15 counts, then five more and the attack's end */
#define BN4_BOMBS_SPAWNERS    0x080EB4C4u /* ... its spawners by subfamily, ten words: 0-2 0x080CE2B8, 3 0x080D2748, 4 0x080D3A58,
                                           * 5 0x080D5B62, 6 0x080D645E, 7 0x080DC30C, 8 0x080DCA7C, 9 BN4_SEEKBOM_THROW */
#define BN4_BOMBS_ARMS        0x080EB4ECu /* ... its arm's word by subfamily (u16: the animation << 8 | the kind; 0x2F subfamily 9's) */
#define BN4_T3_SEEKBOM        0x8D        /* SeekBom's bomb, the third kind's (0x080DED60 to 0x080DEFEC): thrown onto the panel its throw
                                           * found, it strikes what stands there and spawns the explosions' maker; sprite list 4's 0x24 */
#define BN4_SEEKBOM_THROW     0x080DEEF2u /* ... subfamily 9's entry, in its code: the nearest foe's panel ahead looked for (0x080DEF1A),
                                           * the bomb spawned for it (0x080DEED6) */
#define BN4_T4_EXPLOSIONS     0x11        /* the explosions' maker, the fourth kind's (0x080E3328 to 0x080E33E8): its +4 its kind (its
                                           * frames by 0x080E3368: 0x20, 0x5A, 0x20), its +5 the effect it spawns every four frames around
                                           * its parent's place, its +6 whether it stays silent (sound 0x70) */
#define BN4_EXPLOSIONS_SPAWN  0x080E33D4u /* ... its spawner, in its code (r4 its +4 to +7: SeekBom's bomb's movs r4,#2) */
#define BN4_EXPLOSIONS_EFFECT 0x080E33B4u /* ... its ldrb r4,[r5,#5] before BN4_SPAWN_EFFECT (0x080E33B6): no immediate names the id; the
                                           * copy's call goes through a shim that gives r4 BN6's (XvEffectCall, xvirus_shim.h) */
/* Ligtnin1-3, AquaUp1-3 and GreenWd1-3 (its chips 166-168 and 181-186, misc subfamily 0x0C), moved as one unit */
#define BN4_SWEEP             0x080DF98Cu /* their routine, in the sweep's code: the sweep spawned with the record's first parameter its
                                           * kind (0 AquaUp, 1 GreenWd, 2 Ligtnin), its element and its damage */
#define BN4_T3_SWEEP          0x93        /* the sweep, the third kind's (0x080DF7F8 to 0x080DF9B0): from its side's edge a column every so
                                           * many frames (BN4_SWEEP_WAITS by kind), each row's panel tried by its kind's routine (0x080DF898:
                                           * a cracked panel, type 3, of the other side's with a foe on it; a grass one, type 6, likewise; any
                                           * panel that holds an obstacle, its flags' 0x03800000); a column that answered ends its turn */
#define BN4_SWEEP_WAITS       0x080DF7F4u /* ... the frames it waits before each turn, a byte a kind (10, 10, 10) */
#define BN4_T3_GEYSER         0x92        /* AquaUp's geyser and GreenWd's tower, the third kind's (0x080DF68C to 0x080DF7F8, its spawner
                                           * 0x080DF7D0): its kind's row of BN4_GEYSER_KINDS, three animations by its frames */
#define BN4_GEYSER_KINDS      0x080DF674u /* ... its two kinds, 12 bytes each: its sprite's list's offset and number (list 3's 0x25, 0x0B), its
                                           * animations and their frames, its collision's argument, and its sound, a u16 (0x120, 0x13E) */
#define BN4_T3_LIGTNIN_BOLT   0x8E        /* Ligtnin's bolt, the third kind's (0x080DEFEC to 0x080DF128, its spawner 0x080DF108): it strikes
                                           * its panel and the eight around it in its first frame (its collision's +1 0x10), a spark (effect
                                           * 0x6B) on each of the eight in turn; sprite list 5's 0x14, sound 0x143 */
#define BN4_T3_SPRITE_STEP    0x08014446u /* a third-kind object's sprite each frame (BN6's 0x0801BBF4; the first kind's 0x080143FC) */
/* ... and the Plus chips (support 0x4C, chips 285-288: FirePlus, ThunPlus, AquaPowr, WoodPowr) and Marking (misc 0x0F) */
#define BN4_T4_PLUS           0x7F        /* their object, the fourth kind's (0x080EACDC to 0x080EAD2E; its spawner 0x080EAD2E, the
                                           * subfamily's routine): lives a frame; BN4_AI_OF_SIDE's data's four timers written (its
                                           * kind's, obj +4, 480, the rest 0) and, kind 0 alone, the stage's changer spawned with kind
                                           * 3, grass (0x080EAD10, BN4_PANEL_CHANGER_SPAWN) */
#define BN4_AI_PLUS_TIMERS    0x3C        /* the AI data's four element timers, u16 each to +0x42 (fire, the next three by the kind):
                                           * counted down by BN4_PLAYER_TICK, read by BN4_HUD_PLUS_ICON alone (searched: every
                                           * caller of BN4_AI_OF_SIDE, every halfword at these offsets after a load of an object's
                                           * +0x58). No chip's damage changes by one (measured, docs/multirom/bn4-chips.md, call 12) */
#define BN4_PLAYER_TICK       0x08022E3Cu /* the player's timers each frame: BN4_AI_PLUS_TIMERS; then, where the side's record's first
                                           * byte is not 0 (0x08022E62), that record's +0x2E, +0x3A, +0x3C and +0x16; then the AI
                                           * data's +0x3A and its byte +0x1A */
#define BN4_HUD_PLUS_ICON     0x08014CC0u /* the HUD's icon for a running element timer: the first of the four not 0 picks its tiles
                                           * (pointers at 0x08014CF8, 0x180 bytes) for OBJ VRAM 0x06017480; BN6 has no such icon */
#define BN4_MARKING           0x08022F5Au /* Marking's routine (misc 0x0F, 16 bytes): BN4_SIDE_RECORD's +0x16 = 600, which
                                           * BN4_PLAYER_TICK counts only in a mode that sets the record's first byte: nothing in a
                                           * plain battle (measured, call 13). Where it counts, each frame 0x0800B018 with r0 2: the
                                           * user's collision data +0x1C 2 and flag 8, the status whose tick (0x0800AF62) spawns the
                                           * mark over an object's head, the fourth kind's 0x06 */
#define BN4_SIDE_RECORD       0x08022D8Au /* r0 a side: 0x020374B0 + 0x1B0 a side, a record of a side's own (BN6's another layout) */

#define BN4_T3_HAMMER         0x4A        /* BigHamr's and GodHammr's hammer, the third kind's (0x080D68CC to 0x080D6BF8): an actor of 100 HP
                                           * on the panel before MegaMan, run by BN4_ACTOR_RUN with BN4_HAMMER_STATES; set down with sound 0xB0
                                           * (0x080D69AE), a wait, the swing: sound 0x122 (0x080D6A72), BN4_CAMERA_SHAKE_SET for 30 frames
                                           * (0x080D6AB8), sound 0xA2 (0x080D6ABE) and its hit area (0x080CD7E2 at 0x080D6AD6) */
#define BN4_HAMMER_STATES     0x080D698C  /* ... its eight states: its own the first, the third, the seventh and the eighth (0x080D69AC,
                                           * 0x080D6B42, 0x080D69BE, 0x080D69E0), the engine's the rest; its set-down and wait store 6 and 7
                                           * (0x080D69B4, 0x080D69D6), BN6's runner's 8 and 9 in the unit's copy */
#define BN4_T4_HAMMER         0x26        /* ... its support object, the fourth kind's (0x080E4B88 to 0x080E4C3C), which the chip's routine
                                           * spawns and which sets the hammer down */

#endif

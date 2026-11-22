/* BN4 Red Sun's Souls' own code and art past bn4.h's tables by form
 * (docs/rom/bn4.md, Souls in battle; docs/multirom/bn4-souls.md): the
 * unison's picture, and what a Soul's routine as its form is taken does. */
#ifndef CW_BN4_SOULS_H
#define CW_BN4_SOULS_H

#define BN4_T4_SOUL_PICTURE 0x13      /* the fourth kind's: the unison's picture, spawned by its phase 1 (0x080EBB20, 0x080E37D8: its +4 the
                                       * pending form), its init 0x080E363C: */
#define BN4_SOUL_PICTURE_LIST 0x080E363Eu /* ... movs r1,#8: the picture's sprite list offset (list 2), */
#define BN4_SOUL_PICTURE_NUMBER 0x080E3640u /* ... ldrb r2,[r5,#4]: its number the form, so a Soul's picture is his Navi's battle
                                       * sprite (ThunSoul's list 2 number 6, 36,388 bytes; GutsSoul's 60,988 the largest) */
#define BN4_SOUL_PICTURE_DECOS 0x080E36ACu /* ... an arm object (BN4_ARM_SPAWN's kind) on the picture by form, 0xFF none: WindSoul's
                                       * 0x34, FireSoul's 0x12, NumSoul's 0x14, AquaSoul's 0x13 */
#define BN4_GUTS_ENTER    0x0800E0E4u /* GutsSoul's (form 2's): r0 his side, on to his hand's pass, */
#define BN4_GUTS_PASS     0x0800E1CAu /* ... which adds to each chip of the side's hand from the one up next (0x0800BFEA's hand: +0 the
                                       * index, +2 the ids, their powers 0x24 on) whose record's +9 holds 0x02 (its power shown): */
#define BN4_GUTS_PASS_KIND 0x0800E1F0u /* ... cmp r1,#0xc: of the kind none (BN4_CHIP_KIND), */
#define BN4_GUTS_PASS_KIND2 0x0800E1F6u /* ... cmp r1,#8: or panel-cracking, */
#define BN4_GUTS_PASS_ADD 0x0800E1FEu /* ... adds r1,#0x1e: 30 */

#endif

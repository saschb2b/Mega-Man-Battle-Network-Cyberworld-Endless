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
#define BN4_GUTS_PASS     0x0800E1CAu /* ... which adds to the bonus of each chip of the side's hand from the one up next (0x0800BFEA's
                                       * hand: +0 the index, +2 the ids, their bonuses 0x24 on, +0x26: BN6's BN6_HAND_BONUS, the attack's
                                       * +6 as it is used) whose record's +9 holds 0x02 (its power shown): */
#define BN4_GUTS_PASS_KIND 0x0800E1F0u /* ... cmp r1,#0xc: of the kind none (BN4_CHIP_KIND), */
#define BN4_GUTS_PASS_KIND2 0x0800E1F6u /* ... cmp r1,#8: or panel-cracking, */
#define BN4_GUTS_PASS_ADD 0x0800E1FEu /* ... adds r1,#0x1e: 30 */
/* GutsSoul's rapid buster: B pressed six times in his form (the key handler 0x0800BDE0, r4 his AI data, r6 his requests at
 * its start) asks his state 0x10, eight shots of his buster (soulrapid.c) */
#define BN4_GUTS_RAPID_FORM 0x0800BE50u /* cmp r7,#2: his form (NaviStats +0x24) GutsSoul's, and his request 0x20 not standing
                                         * (tst r6, #0x20): then each B pressed (AI +0x1E bit 2) counts at his AI +0x12, */
#define BN4_GUTS_RAPID_WINDOW 0x0800BE68u /* ... movs r0,#0xa: its window at his AI +0x11, ten frames from a press, the count
                                           * 0 as it runs out (0x0800BE6C-0x0800BE7A), */
#define BN4_GUTS_RAPID_AT 0x0800BE7Eu /* ... cmp r0,#6: at six or more the window the count (0x0800BE84), request 0x20 asked
                                       * (0x0800BE8A), his kept charge levels set as B let go sets them (+0x14, +0x15), */
#define BN4_GUTS_RAPID_ASK 0x080EEC04u /* ... movs r0,#0x10: which his ready state's requests (0x080EEBEE, before a plain or charged
                                        * B's) answer with his state 0x10, */
#define BN4_GUTS_RAPID 0x080EBDC0u     /* ... its routine (BN4_PLAYER_STATES' 0x10): eight buster shots (0x0800B590) with sound
                                        * 0xA7, the count and window cleared at its end (0x080EBE84-0x080EBE88) */

#endif

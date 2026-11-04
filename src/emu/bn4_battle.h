/* Battle Network 4: Red Sun (USA)'s battle structures past bn4.h's: its
 * panels' fields its field init writes (docs/rom/bn4.md, The battle's
 * panels against BN6's), apart from bn4.h, which includes it. */
#ifndef CW_BN4_BATTLE_H
#define CW_BN4_BATTLE_H

/* A panel's (BN4_PANEL_SIZE), as BN4's field init writes them (0x08009120: the 40 panels cleared, then each one's three
 * u16 at 0x0800913A-0x0800913E; 0x080091BC: the field's types into +0 from its record's nibbles, 0x080091F4: the sides
 * into +1 from BN4_PANEL_SIDES): its type and side bn4.h's BN4_PANEL_TYPE and BN4_PANEL_ALLIANCE */
#define BN4_PANEL_STATE   0x0C        /* u16 0x258 at the field's start, as BN6's +0xE (BN6_PANEL_STATE, its field init 0x0800C562) */
#define BN4_PANEL_TIMER_A 0x0E        /* u16 0x3C0 at the field's start (BN4_PANEL_TIMER_A_IDLE), idle through a plain battle */
#define BN4_PANEL_TIMER_B 0x12        /* u16 0x80BE at the field's start (BN4_PANEL_TIMER_B_IDLE), idle through a plain battle; BN6
                                       * keeps one timer at +0x12, 0x708 idle (BN6_PANEL_TIMER, BN6_PANEL_TIMER_IDLE) */
#define BN4_PANEL_TIMER_A_IDLE 0x3C0
#define BN4_PANEL_TIMER_B_IDLE 0x80BE
#define BN4_PANEL_SIDES   0x08009218u /* a panel's side at the field's start, a byte a panel (0x080091FA's literal): 0 on the border
                                       * columns, where BN6's per-column side reads 0xFF (0x0800C8C2's table) */

/* MegaMan's AI data (BN4_AI_DATA, a side's): three of his power attacks, each read by its dispatcher into BN4's table
 * (0x0800C9D2 +0xD, 0x0800CA60 +0xF, 0x0800C9E8 +0, the attack variables at +0x70 in r7), called in that order by his
 * attack state (0x080EEC1E, 0x080EEC3E, 0x080EEC5E) as BN6's by his (BN6_AI_POWER_A, _CHARGE, _C: +6, +7, +8); the charged
 * shot's setup (0x0800D836) writes +0xF and +0 from his loadout record (0x0800D8CE-0x0800D8D8), as BN6's (0x08013892) +7
 * and +8 (0x08013918-0x0801391E) */
#define BN4_AI_POWER_A    0x0D        /* (0x0800C9D2's; BN6_AI_POWER_A, +6) */
#define BN4_AI_POWER_CHARGE 0x0F      /* the charged shot's (0x0800CA60's; BN6_AI_POWER_CHARGE, +7) */
#define BN4_AI_POWER_C    0x00        /* (0x0800C9E8's; BN6_AI_POWER_C, +8) */

/* ... and its fields paired with BN6's by both games' code (docs/devtools/differential.md, MegaMan's AI data and his object) */
#define BN4_AI_POWER_CHIP 0x0E        /* u8 the chip charge's power attack (0xFF none: 0x0800BD76 for form 0, else the form's loadout at
                                       * 0x0800BD5E), the one the charge time BN4_AI_CHARGE_TIMES +0 is read for (0x0800BCFA); BN6's +5
                                       * (BN6_AI_POWER_CHIP), whose charge time BN6 reads for r7 5 (0x08012F6C) */
#define BN4_AI_KEYS       0x1C        /* u16 x 8 his keys, as the update 0x0800BA78 writes them (r4 his AI data): held +0x1C, pressed +0x1E,
                                       * let go +0x20, the last held +0x22, then the second set +0x24 to +0x2B; BN6's twin 0x08012DFC writes
                                       * the same at +0x22 to +0x31 (BN6_AI_HELD, store by store) */
#define BN4_AI_KEYS_SIZE  0x10        /* ... their bytes, both sets */
#define BN4_AI_MODES      0x4C        /* u32 his flags: test 0x0800C1D6, set 0x0800C1E8, clear 0x0800C1F2, each BN6's at +0x48
                                       * (0x08010300, 0x08010312, 0x0801031C: BN6_AI_MODES) instruction for instruction */
#define BN4_AI_HP_BUG     0x03        /* u8 the HP bug's frames counted (0x0800C164: HP above 1, its period NaviStats +0xE, one HP lost
                                       * through 0x0800AB92 at the period, then 0); BN6's +9 (0x08010230, BN6_AI_HP_BUG) */
#define BN4_AI_DAMAGE_TAKEN 0x06     /* u16 the damage he has taken, at most 0xFFFF: added by 0x0800C75C, read by 0x0800C776 (by side);
                                       * BN6's +0x20 (0x08010548, 0x0801055E: BN6_AI_DAMAGE_TAKEN), instruction for instruction */
#define BN4_AI_STRUCK_FRAMES 0x54     /* u32 the frames his collision flags hold 0xC00 (0x0800C518: 0x0800C500 counts, 0x0800C50A
                                       * clears, 0x0800C512 reads); BN6's +0x4C (0x080143FC: 0x08014432, 0x08014424, 0x0801442C) */
#define BN4_AI_ANGER      0x2E        /* u16 his anger's frames: a struck word's 0x200 sets flag 0x200000 and 600 here (0x0800C594, its
                                       * emotion 0x80 through 0x0800F4DE), counted down after (0x0800C5C6); BN6's +0x34 (0x08014350,
                                       * BN6_AI_ANGER), instruction for instruction */
#define BN4_AI_LOADOUT_PAD 0x0C       /* u8 the loadout's first byte, 0xFF in every form (0x080184F0's +0, 0x0800BD74 for form 0) and read
                                       * by no code of BN4's through an AI data pointer (written only at 0x0800BD5E and 0x0800BD74) */
#define BN4_AI_CHARGE_TIMES 0x34      /* u16 x 2 the charge times of BN4_AI_POWER_CHIP's and the charged shot's (0xFF none), cached as the
                                       * loadout is set (0x0800BCF4, from BN4_CHARGE_TIMES by his Charge) and read by the charge levels
                                       * (0x0800BB56, 0x0800BB6C); BN6 caches none, reading its table at each test (0x08012F62) */
#define BN4_AI_CONTROL    0x50        /* u32 his side's control block (BN4_CONTROL_BLOCKS): the AI data's clear (0x0800B7CE) points side 0's
                                       * at 0x02035A70 and side 1's at 0x02035AB0; his ready routine (0x080EED48) runs its state from it
                                       * (+0 set up, 0x0800EB86 / 0x0800EB8E; +1 its state, 0x0800E4F8) */
#define BN4_CONTROL_BLOCKS 0x02035A70u /* the sides' control blocks, 0x40 each (cleared by 0x0800E8FE); BN6's ready (0x080F1694) keeps none */

#endif

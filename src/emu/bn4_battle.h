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

#endif

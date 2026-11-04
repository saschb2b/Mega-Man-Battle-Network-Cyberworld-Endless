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

#endif

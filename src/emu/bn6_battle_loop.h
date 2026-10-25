/* BN6's battle loop, the one a random battle and a boss's run (bn6f
 * sub_800801C, its state at BN6_LOOP: the phase it is in, its step, and
 * what it came to), and the banners it shows over the field (BN6's battle
 * messages: BN5's table kept whole, its Liberation Missions' among them).
 * Read and answered by a liberation's battle (src/director/
 * liberation_battle.c; docs/ROM_DATA.md, BN6's battle loop and banners). */
#ifndef CW_BN6_BATTLE_LOOP_H
#define CW_BN6_BATTLE_LOOP_H

#define BN6_LOOP              0x0203CA70u /* u32: the battle loop's phase (+0, a word: a store sets its step to 0 with it), its step (+3), what it came
                                             to (+4: 1 won, 2 lost, 3 drawn, 6 on to the Custom screen), read by the fight's state (0x0800938A) */
#define BN6_LOOP_STEP         0x0203CA73u /* u8, the loop state's fourth byte: its step, 0 as a phase begins (its first frame), 4 after */
#define BN6_LOOP_DRAW         0x14        /* BN6_LOOP's phase: a draw (0x080082DC): the battle stopped, its banner, then result 3 (no reward) */
#define BN6_FIGHT_BANNER      0x080080A0u /* movs r0,#0 (BATTLE START!) as each turn's fight starts (0x08008064), where a NetBattle's TURN n
                                             START! is not (the options' bit 8), */
#define BN6_FIGHT_BANNER_SHOW 0x080080AEu /* ... its bl to the banner (0x0801E792: r0 the message, r1 its number) */
#define BN6_FIGHT_ASK         0x08008144u /* cmp r0,#0xff in the fight's frame (0x080080D2) with no outcome yet, before the Custom screen's request */
#define BN6_FIGHT_RETURN      0x080081A2u /* ... the routine's pop {pc} */
#define BN6_FIGHT_CUSTOM      0x0800819Au /* ... its way to the Custom screen as L or R asks it (bl 0x0800A028, then the loop's phase 0x20) */
#define BN6_STATE_FLAGS       0x32        /* BattleState +0x32, u16 the battle's flags (bn6f battle_getFlags 0x0800A2F0, battle_setFlags 0x0800A2D8): */
#define BN6_FLAG_GAUGE_FULL   0x02        /* ... the Custom gauge full (BN5's liberation ask opens the screen on it alone, 0x08008BD2), */
#define BN6_FLAG_HELD         0x04        /* ... the battle held (no screen asked: 0x0800A098), */
#define BN6_FLAG_ASKED        0x10        /* ... L or R pressed with the gauge full (0x0801306A; the ask 0x0800A1D0 opens on it) */
#define BN6_STATE_SIDE0_LEFT  0x12        /* BattleState +0x12, u8 not 0 while side 0 has someone left, */
#define BN6_STATE_SIDE1_LEFT  0x13        /* ... +0x13 side 1, */
#define BN6_STATE_ENDING      0x0B        /* ... and +0x0B 0: only then is a Custom screen asked (0x0800A18E) */
#define BN6_WIN_SONG          0x08008206u /* adds r0,r2,#0 in the win (0x080081A4): r2 its song (0x19 for the kinds 4, 5, 8 or the options' bit 2, else 0x1F), begun at the next bl */
#define BN6_WIN_BANNER        0x0800822Cu /* adds r0,r6,#0 in the win (0x080081A4): r6 its banner (4, ENEMY DELETED!), shown at the next bl */
#define BN6_DRAW_BANNER       0x080082F8u /* movs r0,#0x1c (DRAW!) in the draw's first step, */
#define BN6_DRAW_BANNER_SHOW  0x080082FAu /* ... its bl to the banner */
#define BN6_DRAW_SHOWN        0x0800830Cu /* ldrh r0,[r5,#8] in the draw, its banner gone */
#define BN6_DRAW_RETURN       0x08008348u /* ... the routine's pop {pc} */
#define BN6_GAUGE_FULL        0x4000      /* BN6_CUSTOM_GAUGE (bn6.h): full */
/* The banners (BN6's battle messages, their table at 0x0801EF84: BN5's at
 * 0x0801B80C, the same up to its Navis' names) */
#define BN6_MSG_TURN_START    0x0C        /* TURN n START! (n the banner's number) */
#define BN6_MSG_FINAL_TURN    0x10        /* FINAL TURN! */
#define BN6_MSG_TIME_UP       0x54        /* TIME UP! */
#define BN6_MSG_LIB_SUCCESS   0x58        /* LIBERATE SUCCESS! */
#define BN6_MSG_LIB_FAILED    0x5C        /* LIBERATE FAILED! */
#define BN6_MSG_TURN_LIBERATE 0x60        /* n TURN LIBERATE! */

#include "bn6_battler.h"   /* the Virus Battler's mode, its own loop: the BtlrCard's data, the five Battlers' records (research) */

#endif

/* Battle Network 5: Team Colonel (USA): its Liberation Missions' rules a
 * panel follows as it is liberated (docs/game/liberation/): a liberation
 * battle, the battle a panel's dark chooses, and a BonusPanel. Split from
 * bn5_liberation.h, which includes it. */
#ifndef CW_BN5_LIBERATION_RULES_H
#define CW_BN5_LIBERATION_RULES_H

/* A liberation battle */
#define BN5_BATTLE_TYPE      0x0802B3E0u /* the battle's record's +3 (BattleSettings' BattleType): */
#define BN5_BATTLE_LIBERATION 6          /* ... 6 a liberation battle (all 860 of BN5_LIB_BATTLES) */
#define BN5_BATTLE_TURN      0x08009016u /* BattleState +7: the turn, one more as each Custom screen opens (1 at the first) */
#define BN5_LIB_TURNS_UP     0x08009190u /* true once BattleState +7 reaches 3, read by BN5_LIB_TURN_BANNER alone */
#define BN5_LIB_TURN_BANNER  0x08007954u /* as each turn's fight starts: FINAL TURN! (message 0x10) where BN5_LIB_TURNS_UP, else TURN n START! (0x0C, n the turn) */
#define BN5_LIB_TIME_ASK     0x08008BD2u /* the Custom screen asked for as the gauge is full (BattleState +0x32 bit 2, not the L or R request */
                                         /* ... 0x10 of the other battles' 0x08008BB0): below the third turn it opens by itself, from it (+7 >= 3) */
#define BN5_LIB_TIME_UP      0x08007C24u /* ... the loop's phase 0x18: TIME UP! (0x54), 30 frames, LIBERATE FAILED! (0x5C), the battle over */
#define BN5_LIB_WIN_BANNER   0x08007A50u /* the win's first step: 1 TURN LIBERATE! (0x90, number 1) where +7 is 1, else LIBERATE SUCCESS! (0x58) */
#define BN5_BATTLE_MESSAGES  0x0801B80Cu /* the battle messages' table, a word each (a message's number is its offset), read by 0x0801B02A */
#define BN5_LIB_LATE_DOUBLE  0x0800901Eu /* true in a battle of type 1 (no liberation battle's) from its eleventh turn: */
#define BN5_LIB_DOUBLE_HIT   0x0800C7CCu /* ... an object's hits this frame summed (its collision data's five) and doubled */

/* A BonusPanel (docs/game/liberation/bn5/bonus-panels.md; traced in play, a panel of mission 1 made one):
 * liberated by the Liberation command, no battle, the Navi's action not spent; its roulette on the
 * phase banner's band, stopped by A, the slot stopped on given */
#define BN5_LIB_PTR_BONUS    8           /* the record's BonusPanels: 24 bytes each (x, y, u16 the frames a slot, five */
                                         /* ... outcomes' pointers, a slot each), 0 ends; an outcome a list of 8 bytes */
                                         /* ... (kind, code, u16 chip, u16 amount, u8 weight; 0xFF ends) */
#define BN5_LIB_BONUS_FIND   0x08052D4Eu /* a panel's id (y << 8 | x) to its BonusPanel's index and record */
#define BN5_LIB_BONUS_PICK   0x08052D74u /* ... slot r1's outcome: one of its list by their weights (the chips' 0x20 each) */
#define BN5_LIB_BONUS_STATE  0x0200ABB0u /* the roulette: +0 its routine (BN5_LIB_BONUS_STEPS), +1 the icon's place (8 */
                                         /* ... pixels a unit from the middle), +2 a wait, +4 frames on the slot, +6 the slot */
                                         /* ... (BN5_LIB_BONUS_ROLL's first), +7 the frames a slot (0 once stopped) */
#define BN5_LIB_BONUS_STEPS  0x08054EF4u /* its ten routines: the band opened, the icon in, the spin, A, held, out, closed */
#define BN5_LIB_BONUS_FRAME  0x08055110u /* each frame: the slot one on every +7 frames (the record's u16, 4), mod 5 */
#define BN5_LIB_BONUS_GIVEN  0x0804C754u /* the slot stopped on: the player's phase's step (its table 0x0804BF5C, routine 2) */
#define BN5_LIB_BONUS_SLOTS  5           /* ... in this order: */
#define BN5_LIB_BONUS_SHIELD 0           /* ... "1PHS INVIN": every Navi's unit +0x0E |= 0x80 (0x08054620), sound 0x93, cleared */
                                         /* ... at the next player's phase (0x08054648): no harm on the board for a phase */
#define BN5_LIB_BONUS_HP     1           /* ... "HP Rec!": item kind 0, 100 (x 10): the acting Navi's HP full, his alone */
#define BN5_LIB_BONUS_OP     2           /* ... "Order Pt!": item kind 2, 5 Order Points */
#define BN5_LIB_BONUS_CHIP   3           /* ... "BattleChip!": item kind 5, one of the mission's four (equal weights) */
#define BN5_LIB_BONUS_HIT    4           /* ... "Major Hit!": the nearest guardian (kinds 0x11-0x13, 0x08055276) deleted; */
                                         /* ... none, the nearest Darkloid (0x0805527A) a fifth of his max HP off */
#define BN5_LIB_BONUS_NEAREST 0x08054290u /* the unit nearest the acting Navi of a kind list (r0: 1 guardians, 2 Darkloids) */
#define BN5_LIB_BONUS_ICONS  0x08126D4Cu /* the icon a slot shows (object 0x23, 0x08126CA8): 4 bytes a slot, its sprite of */
#define BN5_LIB_BONUS_ICON_LIST 9        /* ... this list, its animation, its palette; at x (+1 + 15) x 8, y 72 */
#define BN5_LIB_BONUS_ICON_Y 72        /* ... the icon's origin's y */
#define BN5_LIB_BONUS_WORDS  25          /* the words under it (object 0x24, 0x08126D68): list 9's 25 (BN5_LIB_PHASE_WORDS' */
#define BN5_LIB_BONUS_PRESS  4           /* ... sprite), animation 4 "PRESS A BUTTON" while it spins, 6 + the slot its name */
#define BN5_LIB_BONUS_NAMES  6           /* ... once stopped, */
#define BN5_LIB_BONUS_WORDS_X 120        /* ... its origin here */
#define BN5_LIB_BONUS_WORDS_Y 98       /* ... and its y */
#define BN5_LIB_BONUS_FAR    30          /* the icon's place as the band opens (+1, offscreen right), */
#define BN5_LIB_BONUS_STEP   4           /* ... in by this a frame once BN5_LIB_BONUS_IN_AT frames on (32 pixels), */
#define BN5_LIB_BONUS_IN_AT  16          /* ... then BN5_LIB_BONUS_LIVE_AT frames more before A stops it; */
#define BN5_LIB_BONUS_LIVE_AT 8          /* ... out the same way to -30 after the hold, the band shut 4 later */
#define BN5_LIB_BONUS_HOLD   88          /* frames the slot stopped on shows, its name under it */
#define BN5_LIB_BONUS_FRAMES 4           /* frames a slot (every record's BonusPanel's u16) */
#define BN5_LIB_BONUS_TICK   0x80        /* the sound at each slot's turn while the icon is in place */
#define BN5_LIB_BONUS_STOP   0x92        /* ... as A stops it */
#define BN5_LIB_BONUS_GUARD  0x93        /* ... "1PHS INVIN" given */
#define BN5_LIB_BONUS_FLASH  0xD7        /* before it, three white flashes over the board, this sound each (steps 0x16-0x18): */
                                         /* ... 2 frames white, then 14/16 down by 2/16 a frame; the second 26 frames after */
                                         /* ... the first, the third 10 after that; the band opens 30 after the third */

/* A liberation battle's choice (docs/game/liberation/building/stage-2-battle.md): the dark round the faced panel weighed by where the Navi
 * stands, its level choosing the battle, and so the battle's field (its record's +6, bn6_panels.h's BN6_SIDES_*) */
#define BN5_LIB_LEVEL        0x08051D9Eu /* r0 the faced panel's id, r1 the Navi's: the eight round it each add their weight (BN5_LIB_WEIGHTS) */
                                         /* ... where BN5_LIB_DARK_COUNTS has their kind, into BN5_LIB_STATE +0x0D: 0 none, 1 to 9, 2 to 19, */
                                         /* ... 3 to 30, 4 above */
#define BN5_LIB_WEIGHTS      0x08051E58u /* ... 8 bytes by the way he faces it (+0 to y+1, +8 to x-1, +0x10 to y-1, +0x18 to x+1), the eight round it */
                                         /* ... from (x, y-1) on to (x+1, y-1) and round: beyond it 20, its two sides 10, the diagonals beside */
                                         /* ... the Navi 2, the far ones and his own 0 */
#define BN5_LIB_DARK_COUNTS  0x080502F4u /* ... a byte per panel kind, not 0 where it counts: 1 dark, 3 an item's or trap's, 0x0A a DarkHole */
#define BN5_LIB_LEVEL_STATE  (BN5_LIB_STATE + 0x0D) /* u8: that level */
#define BN5_LIB_BATTLE_OF    0x08051F7Au /* the faced panel's battle: a DarkHole's by its level (r1 3), a unit's (r1 2, r2 the unit), else a */
                                         /* ... dark panel's by weight from its level's list (r1 1); r1 and r2 to BN5_LIB_STATE +8, +9 */
#define BN5_LIB_BATTLE       0x08008EEAu /* battle r0's record: BN5_LIB_BATTLES + 16 r0 */
#define BN5_LIB_BATTLES      0x0811488Cu /* the liberations' 860 BattleSettings: type 6, field 0, +6 the column pattern by the level */
                                         /* ... (0 0x30, 1 0x38, 2 0x3C, 3 and 4 0x33 Surrounded; a Darkloid's never Surrounded, 0x3C from 2) */
#define BN5_LIB_SURROUNDED_AT 0x23       /* MegaMan's panel in a Surrounded battle (row 2, column 3), its foes in columns 1-2 and 5-6 */
#define BN5_LIB_SURROUNDED_SAY 0x08008960u /* the battle's opening phase 0x28 (from 0x080086FC, the pattern 0x31, 0x23 or 0x33 and flag 3:0x0E */
                                         /* ... clear): the Navi's "Surrounded!" line, archive 0x0875351C script 0x0B + the Navi, then the flag */
                                         /* ... set: once a save. BN6 keeps neither the phase nor the line */
#define BN5_LIB_DARKLOID     0x08051A1Eu /* the mission's record +8, +9 and the par (BN5_LIB_STATE +5, +0x0A, +0x16): +9 the Darkloid's */
                                         /* ... kind (the units' first, all nine), read to know his unit (0x0805528E), his phase and his line */
#define BN5_LIB_UNIT_HP_IN   0x08053B9Eu /* a unit's battle (BN5_LIB_STATE +8 == 2): its HP from the board's copy 0x02000C20 (0x28 each, */
#define BN5_LIB_UNIT_HP_OUT  0x08053BF4u /* ... +9 the unit, +0x14 HP) as its enemy spawns (0x08006862); the HP left written back at the end */
                                         /* ... (0x0800906A, 0x080138CA). The battle reads nothing else of the board */

#endif

/* The Virus Battler in BN6 Cybeast Gregar (USA), part of bn6.h's addresses
 * (bn6_battle_loop.h includes this): the BtlrCard and the rare viruses'
 * data it holds, the battle mode in which two of them fight a Battler's
 * pair on their own, and the five Battler Mr.Progs' series of records,
 * which a run's Battler Mr.Prog fights by BN6's own code
 * (src/director/director_battler.c; docs/world/level-design/virus-battler.md, The Virus
 * Battler; docs/rom/virus-battler.md, The Virus Battler; docs/research/sources.md). */
#ifndef CW_BN6_BATTLER_H
#define CW_BN6_BATTLER_H

/* The card and the data on it */
#define BN6_ITEM_BTLRCARD     0x40        /* the key item BtlrCard (BN6_KEY_NAMES' entry 0x40); only the seller's chat checks it */
#define BN6_HEELNAVI_TEXT     0x0879D0F4u /* Class 1-2's blackboard HeelNavi's archive (compressed): its entry 11 takes 2000 zenny, sets
                                             BN6_FLAG_BTLRCARD, BN6_FLAG_VIRUS_DATA + 1 and + 23 (Mettaur's, Gunner's) and gives the card */
#define BN6_FLAG_BTLRCARD     0x01D8      /* the card held: a rare's deletion registers it (BN6_DELETE_RARE), and BN6's random battles of
                                             kind 1 (rare) may come (BN6_RARE_BATTLES) */
#define BN6_FLAG_VIRUS_DATA   0x0196      /* + an AI index: that family's data on the card (1-29 but 10 and 13: BN6_CARD_FLAGS) */
#define BN6_CARD_FLAGS        0x0802B2E0u /* the VIRUS LIST's families in its order: a u32 flag each, BN6_CARD_FAMILIES read (0x0802B2A0) */
#define BN6_CARD_FAMILIES     27          /* ... AI 1-9, 11, 12, 14-29 (not 10 Trumpy, 13 Kettle) */
#define BN6_RARE_PENDING      0x0203EAEAu /* u8: the AI index BN6_DELETE_RARE put for this battle's results, 0 none */
#define BN6_NEW_VIRUS_DATA    0x0802C2D0u /* the results' step that runs BN6_NEW_VIRUS_DATA_TEXT's entry BN6_RARE_PENDING (at 0x0802C2E8) */
#define BN6_NEW_VIRUS_DATA_TEXT 0x08736058u /* "You got new Virus Data: ...!": entry n sets BN6_FLAG_VIRUS_DATA + n, the card's one writer */
#define BN6_RARE_BATTLES      0x080ABF5Cu /* the random battles' pick of a kind 1 (rare) record: none without BN6_FLAG_BTLRCARD */

/* The battle mode (BN6_BATTLE_SETTINGS' +3) */
#define BN6_MODE_VIRUS_BATTLER 6
#define BN6_MODE_LOOPS        0x08007B50u /* the battle loop of each mode, a Thumb pointer each (12), by BattleSettings +3 */
#define BN6_VB_LOOP           0x0800961Cu /* ... mode 6's: by BN6_BATTLE_PHASE, 8 its VIRUS LIST and placing (BN6_VB_PICK), 0x0C the fight */
#define BN6_VB_PICK           0x0802A87Cu /* the VIRUS LIST's step, each frame of phase 8, by BN6_VB_LIST's +0 (it sets BN6_FLAG_VIRUS_DATA
                                             + 1 and + 23 as it opens); its +4 set once the team is placed and confirmed */
#define BN6_VB_LIST           0x020364C0u /* the VIRUS LIST's state (bn6f eS20364C0): */
#define BN6_VB_LIST_PHASE     (BN6_VB_LIST + 0x01) /* u8: 4 the list, 8 OK pressed, 0x0C placing, 0x14 "Fight with this set?" */
#define BN6_VB_LIST_PLACED    (BN6_VB_LIST + 0x05) /* u8: viruses placed, 0-2 */
#define BN6_VB_LIST_CURSOR_X  (BN6_VB_LIST + 0x08) /* u8: the panel the one placing stands on, column 1-3 */
#define BN6_VB_LIST_CURSOR_Y  (BN6_VB_LIST + 0x09) /* u8: ... row 1-3 */
#define BN6_VB_LIST_PICK1     (BN6_VB_LIST + 0x10) /* u8: the list's entry picked first, 0xFF none */
#define BN6_VB_LIST_PICK2     (BN6_VB_LIST + 0x11) /* u8: ... second, 0xFF none (a team of one may fight) */
#define BN6_VB_FAMILY_DATA    0x0802B5FCu /* 8 bytes an AI index: u16 HP, attack and MB (the list's DATA: the V1's numbers), 0 */
#define BN6_VB_MB_LIMIT       50          /* the team's MB at most (0x0802AB54's and 0x0802B39E's cmp #0x32) */
#define BN6_VB_SPAWN          0x0802AEEAu /* r0 an enemy id: a picked family placed as its V1 (id (AI - 1) * 6 + 1) on MegaMan's side */
#define BN6_VB_CONFIRM_TEXT   0x08734CB0u /* the archive whose entry 0x28 asks "Fight with this set?" (0x0802AD0A) */
#define BN6_VB_GAUGE_STEP     13          /* the time limit: BN6_CUSTOM_GAUGE rises 13 a frame from 0 (about 21 s); full, TIME UP */
#define BN6_RESULT_WON        1           /* BN6_BATTLE_RESULT: every enemy deleted */
#define BN6_RESULT_LOST       2           /* ... the team deleted */
#define BN6_RESULT_DRAW       7           /* ... a time up with both sides standing (TIME UP!, DRAW!) */

/* The Battlers' records and their series */
#define BN6_BATTLE_LIST0      0x080B06E0u /* BattleSettings list 0, 16 bytes a record by index (getBattleSettingsFromList0 0x0800A71E) */
#define BN6_VB_FIRST_RECORD   0x1A        /* the Battlers' records in it, mode 6 all: 0x1A-0x1C, 0x1D-0x21, 0x22-0x26, 0x27-0x30, 0x31-0x3A;
                                             +5 the round (BATTLE n START!), +2 the music (0x16 a series' last), +8 0x80A0, +12 the pair */
#define BN6_VB_RECORDS        33          /* ... their count, the five series in a row */
#define BN6_VB_START_SCENE    0x0809A5F0u /* the cutscene a Battler's map script starts, its record's index the parameter's low half */
#define BN6_VB_START_BATTLE   0x0809A674u /* ... its step: StartBattle (BN6_START_BATTLE) on list 0's record, r1 1 */
#define BN6_FLAG_VB_SERIES    0x01D9      /* + 2 a Battler (0-4): his series under way (the chat's Yes sets it, the map's script
                                             starts each round while it is); + 1 more, set as the series is won */
#define BN6_FLAG_VB_ROUNDS    0x01F1      /* the rounds won so far, a flag each: 0x1F1, 0x1F4, 0x1F9, 0x1FE, 0x208 the five series' first */
#define BN6_FLAG_VB_BEATEN    0x01D4      /* + 0-3: Battlers 1-4 won once (the fifth has none) */
#define BN6_FLAG_VB_FIRST_PRIZE 0x01EC    /* + 0-4: each Battler's first prize given; later wins pay zenny or BugFrags */
#define BN6_VB_TEXT_1         0x0879B9B0u /* the five Battlers' archives (compressed): RoboDog's comp, Central Town */
#define BN6_VB_TEXT_2         0x0879F4DCu /* ... the water comp, Seaside */
#define BN6_VB_TEXT_3         0x0879F080u /* ... the punishment chair's comp, Green Town */
#define BN6_VB_TEXT_4         0x087A10BCu /* ... the oxygen tank's comp, Sky Town */
#define BN6_VB_TEXT_5         0x08782844u /* ... Central Area 1 */
#define BN6_VB_BATTLERS       5
/* Each Battler's own Mr.Prog, never the generic green one (sprite list 6,
 * as his map's NPC script sets it: bn6f's Comps1, Comps2 and CentralArea,
 * each shown once the Battler before is beaten, BN6_FLAG_VB_BEATEN) */
#define BN6_VB_SPRITE_1       0x5C
#define BN6_VB_SPRITE_2       0x5D
#define BN6_VB_SPRITE_3       0x5F
#define BN6_VB_SPRITE_4       0x5E
#define BN6_VB_SPRITE_5       0x3D
/* Each Battler's first prize, as his archive gives it (ts_item_give_chip
 * F4 10 and ts_item_give_navi_cust_program EF 1B, read from the five
 * archives above): Battler 1's WindRack *, 2's HP+500 (pink), 3's BugFix *,
 * 4's FldrPak2 (yellow), 5's SuprVulc V; a chip's id and code (0-25, 26
 * '*'), a program's give id (its name's index times four) and colour */
#define BN6_VB_PRIZE_CHIP_1   0x50
#define BN6_VB_PRIZE_PROGRAM_2 0xB8
#define BN6_VB_PRIZE_COLOR_2  6
#define BN6_VB_PRIZE_CHIP_3   0xB0
#define BN6_VB_PRIZE_PROGRAM_4 0x78
#define BN6_VB_PRIZE_COLOR_4  3
#define BN6_VB_PRIZE_CHIP_5   0x08
#define BN6_VB_PRIZE_CODE_5   21

#endif

/* Battle Network 5: Team Colonel (USA): the addresses the guest core uses
 * (guest.c; docs/ROM_DATA.md, BN5 guest battles). BN5 shares BN6's engine:
 * its toolkit, game state and battle records are laid out as BN6's are. */
#ifndef CW_BN5_H
#define CW_BN5_H

#define BN5_TOOLKIT       0x0200A440u /* eToolkit: +0 points at the main mode's index (4 the game, 0x14 its GAME OVER) */
#define BN5_TOOLKIT_GAMESTATE 0x3C    /* eToolkit +0x3C: the game state */
#define BN5_GAMESTATE     0x02002940u /* +0 the sub-mode: 4 on the map, 8 a battle beginning, 0xC in battle; +0x1C the battle's record */
#define BN5_ROLL          0x0810F6B4u /* the encounter roll the check calls each frame on a net map: returns a BattleSettings* (0 none) */
#define BN5_ENEMY_IDS     0x08014C8Cu /* (version, type, AI) per enemy id, 3 bytes, as BN6's GetVerActorTyAndAIIdx table: read by the routine before it (0x08014C7C) */
#define BN5_ENEMY_STATS   0x0800D17Cu /* per type a pointer, per AI a pointer, per version 6 bytes: u16 element << 12 | HP, version, flags, u16 element << 12 | damage */
#define BN5_BATTLE_TABLES 0x0801C968u /* two pointers, the real world's map groups and the net's (group - 0x80): per group a pointer per map,
                                       * per map 16-byte BattleSettings records, byte 0 0xFF ending them, +7 a condition */

#define BN5_NAVI_STATS    0x020052A8u /* MegaMan's NaviStats (eToolkit +0x74): 0x60 bytes, as BN6's 0x64 laid out */
#define BN5_NAVI_BASE_MAX_HP (BN5_NAVI_STATS + 0x3E) /* u16: the max HP before HPMemory and programs */
#define BN5_NAVI_HP       (BN5_NAVI_STATS + 0x40) /* u16 CurHP, then +0x42 MaxHP */
#define BN5_NAVI_MAX_HP   (BN5_NAVI_STATS + 0x42)
#define BN5_BATTLE_RESULT 0x0200AEE8u /* +1 how the last battle ended: 1 won, 2 lost, 4 escaped (BN6's 0x0200A008) */
#define BN5_REWARD        (BN5_BATTLE_RESULT + 4) /* u16: what the results screen gave, as BN6's rewards are (bits 14-15: 0 a chip, its id and code << 9; 1 zenny) */
#define BN5_REWARD_ROWS   0x081100D4u /* the reward rows: 20 u16 per enemy id, 0x28 apart, as BN6's are (its chooser 0x0810FB18 matches BN6's 0x080AC150) */
#define BN5_RESULT_WON     1
#define BN5_RESULT_LOST    2
#define BN5_RESULT_ESCAPED 4
#define BN5_OPT_GAME_OVER  0x100   /* BattleSettings +8 options: a loss plays GAME OVER (GetBattleEffects 0x0802B3F2) */
#define BN5_RECORD_BACKDROP 4      /* BattleSettings +4: its background's number, 0xFF the map's (its routine 0x0808CAE8, a byte per map at 0x0808CB1C) */
#define BN5_FREE          0x08800000u /* the guest's ROM copy past BN5's 8 MB: its record copies */
#define BN5_NAVI_FOLDER   (BN5_NAVI_STATS + 0x2D) /* u8: the folder MegaMan fights with (0 the first) */
#define BN5_TOOLKIT_CHIPS 0x48        /* eToolkit +0x48: the folders (0x02002DF4), 30 u16 each (chip | code << 9), 0x3C apart */
#define BN5_TOOLKIT_CHIP_MARKS 0x80   /* eToolkit +0x80 (0x02005CC4): a byte per chip id, its key XOR BN5_CHIP_KEY_XOR where owned */
#define BN5_CHIP_KEYS     0x02001440u /* a key byte per chip id */
#define BN5_CHIP_KEY_XOR  0x81
#define BN5_CHIP_NAMES_LOW  0x736084u /* (ROM offsets) the chip names' text archives: ids 0-255, */
#define BN5_CHIP_NAMES_HIGH 0x736AB8u /* ... and 256 on */
#define BN5_CHIPS         424         /* its chip ids */
#define BN5_CHIP_RECORDS  0x01E210u   /* (ROM offset) its chips' records, 0x2C each in BN6's ChipData layout: +0..3 the chip's four codes (0-25 A-Z, 26 *, 0xFF none) */
#define BN5_PA_STAR_LIMIT 0x080252B2u /* a Program Advance of one chip in codes in a row: cmp r2, #1, one * of its three at most, as BN6's (BN6_PA_STAR_LIMIT) */
/* its DarkChips (docs/ROM_DATA.md, BN5 guest battles; .build research):
 * ids 187-198, folder chips as any, at most three in a folder (one of each) */
#define BN5_DARK_FIRST    187
#define BN5_COMPACT       0x080250EAu /* the deck's compaction at each Custom screen, after the worried rule (0x08025118): replaced by the shelf */
#define BN5_DARK_USED     0x0203E542u /* + side * 8: set to 1 as a DarkChip is used, cleared at battle start; gone as the battle leaves (latched) */
#define BN5_NAVI_MOOD     (BN5_NAVI_STATS + 0x0E) /* u8: the mood a battle starts from where 0xFF; 0x80 calm */
#define BN5_NAVI_METER    (BN5_NAVI_STATS + 0x44) /* u16: the dark meter, 500 neutral (below 470 battles start dark) */
#define BN5_TOOLKIT_METER_CHECK 0x94  /* eToolkit +0x94: the meter XOR the u32 at BN5_METER_KEY, or the meter resets to 500 */
#define BN5_METER_KEY     0x02002338u
#define BN5_BATTLE_SIDE   (BN5_BATTLE_STATE + 0x0D) /* u8: MegaMan's side */
#define BN5_BATTLE_MOOD   0x0203C88Eu /* u8: MegaMan's mood in battle (side 1's 0x60 on): 1-0x40 worried, 0 dark, 0xFF Full Synchro */

#define BN5_BATTLE_STATE  0x02034A90u /* BattleState: +1 the battle's phase (8 the Custom screen, 0xC fighting, 0x10 over) */
#define BN5_PHASE_CUSTOM  0x08
#define BN5_PHASE_FIGHT   0x0C
#define BN5_CUSTOM_GAUGE  0x02035700u /* u16, the Custom gauge, full at 0x4000 (its SetCustGauge 0x0801A88E: 0x020356E0 + 0x20, as BN6's) */
#define BN5_BATTLE_HP     (BN5_BATTLE_STATE + 0x34) /* u16: MegaMan's HP as the battle ends, to copy back (0 deleted) */
#define BN5_FIGHT_HP      0x0203B224u /* u16: MegaMan's HP while the battle runs (his battle object's), then +2 its max (the worried rule reads them) */

#define BN5_MODE_GAME      0x04
#define BN5_MODE_GAME_OVER 0x14
#define BN5_SUB_MAP        0x04
#define BN5_SUB_BATTLE_INIT 0x08
#define BN5_SUB_BATTLE     0x0C

#endif

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
#define BN5_REWARD        (BN5_BATTLE_RESULT + 4) /* u16: what the results screen gave, as BN6's rewards are (bits 14-15: 0 a chip, its id and code << 9; 1 zenny; */
#define BN5_REWARD_HP     2           /* ... 2 HP restored on the results screen, into BattleState +0x34 too: "HP+50"; */
#define BN5_REWARD_BUGFRAGS 3         /* ... 3 BugFrags; 0 and 0xFFFF none), given as the screen ends (0x08028E48: its GiveChips 0x0801DF56, GiveZenny 0x0803C2FC, GiveBugfrags 0x0803C384) */
#define BN5_REWARD_ZENNY  1           /* BN5_REWARD: its kind (bits 14-15) for zenny */
#define BN5_REWARD_FIND   (BN5_BATTLE_RESULT + 6) /* u16: the screen's second reward, the find of a green Mystery Data still on the field as the battle is won
                                       * (BN5_FIND_ROWS; its BattleState +0x36, 0x080DB616), set as the first is (0x08028690) and given after it (0x08028E5A) */
#define BN5_REWARD_ROWS   0x081100D4u /* the reward rows: 20 u16 per enemy id, 0x28 apart, as BN6's are (its chooser 0x0810FB18 matches BN6's 0x080AC150) */
#define BN5_FIND_ROWS     0x0801D62Cu /* the battlefield Mystery Data's finds: a row of eight u16 per its entity's byte 2 >> 4, one drawn by RNG & 0xE as it appears (0x080DB692) */
#define BN5_FIND_ROW_COUNT 6          /* ... the rows its records name: 1000-3000 zenny, 1-3 BugFrags, from row 1 a chip or two */
#define BN5_ENTITY_FIND   2           /* a BattleSettings entity's kind (byte 0 >> 4): a green Mystery Data, there where its byte 2 beats RNG % 15 (0x08006874) */
#define BN5_RNG_PRIMARY   0x02001D40u /* u32, its random numbers (bn6f ePrimaryRngSeed, BN6's 0x020013F0): GetRNG 0x08001490 steps it, once a frame on a map
                                       * and in battle (0x08004CA8); a battle keeps it as it begins (0x080098D8) and puts it back after its opening (0x08009A0A),
                                       * its rolls from there: the Mystery Data and its find, the reward, the viruses (docs/ROM_DATA.md, BN5's random numbers) */
#define BN5_RNG_SECONDARY 0x02001C94u /* u32, its second ones (bn6f eSecondaryRngSeed, BN6's 0x02001120): GetRNGSecondary 0x080014C0 steps it twice a
                                       * frame (0x0800031C, 0x08004CAC); a battle's folder is shuffled from it as the battle begins (0x08008D82): the first hand */
#define BN5_RNG_XOR       0x873CA9E5u /* both words' step: x = ((x rotl 1) + 1) ^ this; SeedRNG 0x08001488 sets the first to 0xA338244F at the start */
#define BN5_RESULT_WON     1       /* BN5_BATTLE_RESULT (its +1): won */
#define BN5_RESULT_LOST    2       /* BN5_BATTLE_RESULT (its +1): lost */
#define BN5_RESULT_ESCAPED 4       /* BN5_BATTLE_RESULT (its +1): escaped */
#define BN5_OPT_GAME_OVER  0x100   /* BattleSettings +8 options: a loss plays GAME OVER (GetBattleEffects 0x0802B3F2) */
#define BN5_RECORD_BACKDROP 4      /* BattleSettings +4: its background's number, 0xFF the map's (its routine 0x0808CAE8, a byte per map at 0x0808CB1C) */
#define BN5_FREE          0x08800000u /* the guest's ROM copy past BN5's 8 MB: its record copies */
#define BN5_NAVI_FOLDER   (BN5_NAVI_STATS + 0x2D) /* u8: the folder MegaMan fights with (0 the first) */
#define BN5_NAVI_ATTACK   (BN5_NAVI_STATS + 0x01) /* u8 Attack, Speed and Charge: the buster's levels less one (0-4), as BN6's (BN6_NAVI_ATTACK): */
#define BN5_NAVI_SPEED    (BN5_NAVI_STATS + 0x02) /* a shot does Attack + 1 (0x0800F538), a charged one ten times that (0x0800F5B4); Charge picks */
#define BN5_NAVI_CHARGE   (BN5_NAVI_STATS + 0x03) /* the charge's frames, 100 down to 60 (0x08010682, its table 0x0801CA68); its boot state 0, 0, 0 */
#define BN5_BUSTER_MAX    4           /* the buster's highest level less one */
#define BN5_BATTLE_NAVI   0x0203C880u /* the battle's copies of the NaviStats, BN5_BATTLE_NAVI_SIZE a side (0x08010D18), made from them as it begins: */
#define BN5_BATTLE_NAVI_SIZE 0x60     /* ... its buster read from there (0x08010DEE, the object's side), its fields at the NaviStats' offsets */
#define BN5_TOOLKIT_CHIPS 0x48        /* eToolkit +0x48: the folders (0x02002DF4), 30 u16 each (chip | code << 9), 0x3C apart */
#define BN5_TOOLKIT_CHIP_MARKS 0x80   /* eToolkit +0x80 (0x02005CC4): a byte per chip id, its key XOR BN5_CHIP_KEY_XOR where owned */
#define BN5_CHIP_KEYS     0x02001440u /* a key byte per chip id */
#define BN5_CHIP_KEY_XOR  0x81        /* the marks' XOR (BN6's 0x17) */
#define BN5_CHIP_NAMES_LOW  0x736084u /* (ROM offsets) the chip names' text archives: ids 0-255, */
#define BN5_CHIP_NAMES_HIGH 0x736AB8u /* ... and 256 on */
#define BN5_CHIPS         424         /* its chip ids */
#define BN5_CHIP_RECORDS  0x01E210u   /* (ROM offset) its chips' records, 0x2C each in BN6's ChipData layout: +0..3 the chip's four codes (0-25 A-Z, 26 *, 0xFF none) */
#define BN5_PA_STAR_LIMIT 0x080252B2u /* a Program Advance of one chip in codes in a row: cmp r2, #1, one * of its three at most, as BN6's (BN6_PA_STAR_LIMIT) */
/* its DarkChips (docs/ROM_DATA.md, BN5 guest battles; .build research):
 * ids 187-198, folder chips as any, at most three in a folder (one of each) */
#define BN5_DARK_FIRST    187
#define BN5_COMPACT       0x080250EAu /* the deck's compaction at each Custom screen, after the worried rule (0x08025118): replaced by the shelf (its deck BN5_BATTLE_DECK) */
#define BN5_DARK_USED     0x0203E542u /* + side * 8: set to 1 as a DarkChip is used, cleared at battle start; gone as the battle leaves (latched) */
#define BN5_NAVI_MOOD     (BN5_NAVI_STATS + 0x0E) /* u8: the mood a battle starts from where 0xFF; 0x80 calm */
#define BN5_NAVI_METER    (BN5_NAVI_STATS + 0x44) /* u16: the dark meter, 500 neutral (below 470 battles start dark) */
#define BN5_TOOLKIT_METER_CHECK 0x94  /* eToolkit +0x94: the meter XOR the u32 at BN5_METER_KEY, or the meter resets to 500 */
#define BN5_METER_KEY     0x02002338u /* the u32 the meter's check is XORed with */
#define BN5_BATTLE_SIDE   (BN5_BATTLE_STATE + 0x0D) /* u8: MegaMan's side */
#define BN5_BATTLE_MOOD   0x0203C88Eu /* u8: MegaMan's mood in battle (side 1's 0x60 on): 1-0x40 worried, 0 dark, 0xFF Full Synchro */

#define BN5_BATTLE_STATE  0x02034A90u /* BattleState: +1 the battle's phase (8 the Custom screen, 0xC fighting, 0x10 over) */
#define BN5_BATTLE_PHASE  (BN5_BATTLE_STATE + 0x01) /* the battle's phase: 8 the Custom screen, 0xC fighting, 0x10 over */
#define BN5_PHASE_INTRO   0x00        /* BN5_BATTLE_PHASE: the battle's opening, which sets the mood to 0x80, calm, once its first frames are done */
#define BN5_PHASE_CUSTOM  0x08        /* BN5_BATTLE_PHASE: the Custom screen */
#define BN5_PHASE_FIGHT   0x0C        /* BN5_BATTLE_PHASE: fighting */
#define BN5_CUSTOM_GAUGE  0x02035700u /* u16, the Custom gauge, full at 0x4000 (its SetCustGauge 0x0801A88E: 0x020356E0 + 0x20, as BN6's) */
#define BN5_BATTLE_HP     (BN5_BATTLE_STATE + 0x34) /* u16: MegaMan's HP as the battle ends, to copy back (0 deleted) */
#define BN5_FIGHT_HP      0x0203B224u /* u16: MegaMan's HP while the battle runs (his battle object's), then +2 its max (the worried rule reads them) */

/* Its battle as the autopilot and the dev switches read it (docs/ROM_DATA.md,
 * BN5's battle for the autopilot): BN6's structures (bn6.h's fields), at its
 * own addresses, but where named here */
#define BN5_BATTLE_TIMER  (BN5_BATTLE_STATE + 0x40) /* u32, the frames fought: held on the Custom screen, through BATTLE START! and the results (BN6_BATTLE_TIMER) */
#define BN5_T1_OBJECTS    0x0203B200u /* its viruses and MegaMan (BattleObject, BN6_T1_SIZE each, BN6_T1_* its fields; MegaMan the first), */
#define BN5_T1_COUNT      0x10        /* ... 16 of them where BN6 keeps 32 (its list's descriptor at 0x080032E4) */
#define BN5_T3_OBJECTS    0x0203CA40u /* its attacks and effects (BN6_T3_OBJECTS), BN6_T1_SIZE each, */
#define BN5_T3_COUNT      0x20        /* 32 of them */
#define BN5_MEGAMAN_READY 0x06        /* MegaMan's action (BN6_T1_ACTION) when he can act, BN6's 0x08 (0x10 a step, 0x14 a chip, 0x03 hit); his chip (BN6_T1_CHIP) is 0xFFFF as its use begins */
#define BN5_FIELD_PANELS  0x0203A100u /* its field's panels, 8 a row from column 0, rows 0-4, each */
#define BN5_PANEL_SIZE    0x24        /* ... this long (BN6's 0x20): type and alliance at BN6_PANEL_TYPE and BN6_PANEL_ALLIANCE, */
#define BN5_PANEL_FLAGS   0x18        /* ... and its flags here, u32 (BN6's at +0x14), BN6_PANEL_STRUCK while an enemy's attack is on it */
#define BN5_CUSTOM_SCREEN 0x02036B10u /* the Custom screen's state, BN6_CUSTOM_SCREEN's: */
#define BN5_CUSTOM_HAND   (BN5_CUSTOM_SCREEN + 0x06) /* the chips it offers, the first of BN5_BATTLE_DECK */
#define BN5_CUSTOM_CURSOR (BN5_CUSTOM_SCREEN + 0x07) /* the slot under the cursor, 0-4 the top row, BN6_CUSTOM_OK on OK */
#define BN5_CUSTOM_PICKED (BN5_CUSTOM_SCREEN + 0x08) /* the chips picked */
#define BN5_CUSTOM_WINDOW 0x020356F2u /* 0x020356E0 + 0x12: the Custom screen's window as it slides in, 0 to BN5_WINDOW_OPEN (BN6_CUSTOM_WINDOW) */
#define BN5_WINDOW_OPEN   0x78        /* BN5_CUSTOM_WINDOW: open */
#define BN5_BATTLE_DECK   0x0203C830u /* the battle's folder as it is drawn, u16 chip | code << 9 (BN6_BATTLE_DECK) */

/* Its Navis as a territory's guardians (docs/BOSSES.md, BN5's Navis;
 * docs/ROM_DATA.md, BN5's Navis): a Navi is type BN5_NAVI_TYPE in the ids
 * table, his AI index a family of six versions (V1, V2, V3, SP, DS, a
 * sixth), each with its 6-byte stats row (BN5_ENEMY_STATS) */
#define BN5_NAVI_TYPE     1           /* a Navi's type in the ids table */
#define BN5_STORY_BATTLES 0x08113CD8u /* its story's battle records by number, 16 bytes each (its Navis' V1-V3 among them: KnightMan V1 0x21) */
#define BN5_STORY_BATTLE_COUNT 0x69 /* the records there */
#define BN5_OPT_RESULTS   0x02     /* BattleSettings +8 options: its results screen and reward after a win (a V1 story record lacks it) */
#define BN5_OPT_RUN       0x20     /* BattleSettings +8 options: running allowed (its random battles), as BN6's */
#define BN5_CHIP_KIND     6        /* a chip record's +6: its kind, 0 Fire, 1 Aqua, 2 Elec, 3 Wood, 4 Recovery, 5 Plus, 6 Sword, 7 Invisible,
                                    * 8 Cursor, 9 Obstacle, 10 Wind, 11 Break, 12 none */
#define BN5_SOUL_KINDS    0x08024BE4u /* a byte per Soul 1-12 (its Navi's AI index): the chip kind it unites with (the UNITE check 0x08024B2C) */
/* Its Souls (docs/META.md, Souls in BN5 territory; docs/ROM_DATA.md, BN5's
 * Souls): the Custom screen's UNITE command, its Double Soul */
#define BN5_UNITE_CHECK   0x08024B2Cu /* the check of the last chip picked: its kind (BN5_CHIP_KIND) a Soul's, that Soul's flag set, not used this battle */
#define BN5_SOUL_FLAGS    0x08024BF4u /* a byte per Soul 1-12: its event flag, 0xFF none (Team ProtoMan's six in this version) */
#define BN5_TOOLKIT_FLAGS 0x44        /* eToolkit +0x44: its event flags (0x020029F8), flag f in byte f >> 3, bit 0x80 >> (f & 7) */
#define BN5_FLAG_DOUBLE_SOUL 0x0000   /* Double Soul learned: UNITE stands under OK (0x08023C8C), while MegaMan is neither worried nor dark */
#define BN5_FLAG_SOUL     0x0008      /* + k: Team Colonel's Soul k held (0 Colonel's .. 5 ToadMan's: BN5_SOUL_FLAGS' 7-12) */
#define BN5_FLAG_CHAOS    0x0236      /* Chaos Unison: a DarkChip of a held Soul's kind unites too (the check at 0x08024B80) */
#define BN5_SOULS_USED    0x02034E10u /* u32: bit k Soul k united this battle, bit 16 + k its Chaos Unison (each once a battle) */
#define BN5_CUSTOM_UNITE  0x02036C94u /* the Custom screen's UNITE (its element 0xB, 12 bytes): +0 2 where it stands, +5 the Soul the last chip picked
                                       * unites, +6 1 for Chaos Unison, +7 0 ready, 1 grey, 2 chosen */
#define BN5_BATTLE_SOUL   (BN5_BATTLE_NAVI + 0x2C) /* u8, MegaMan's side's: the Soul he is united with (its Navi's AI index), 0 none */
/* Team Colonel's Navis, by AI index: their Souls are the six its MegaMan
 * unites with (BN5_SOUL_FLAGS) */
enum { BN5_NAVI_COLONEL = 7, BN5_NAVI_SHADOWMAN, BN5_NAVI_NUMBERMAN, BN5_NAVI_TOMAHAWKMAN, BN5_NAVI_KNIGHTMAN, BN5_NAVI_TOADMAN };

#define BN5_MODE_GAME      0x04   /* BN5_TOOLKIT: the game (the main mode, the index eToolkit +0 points at) */
#define BN5_MODE_GAME_OVER 0x14   /* BN5_TOOLKIT: its GAME OVER */
#define BN5_SUB_MAP        0x04   /* BN5_GAMESTATE: on the map */
#define BN5_SUB_BATTLE_INIT 0x08  /* BN5_GAMESTATE: a battle beginning */
#define BN5_SUB_BATTLE     0x0C   /* BN5_GAMESTATE: in battle */

#endif

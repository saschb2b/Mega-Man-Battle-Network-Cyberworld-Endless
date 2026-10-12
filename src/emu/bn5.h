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
/* Its battle engine's routines the differential hooks (src/dev/xdiff_events.c), BN6's in bn6.h: the same code */
#define BN5_RNG_DRAW      0x08001490u /* GetRNG (BN6_RNG_DRAW) */
#define BN5_RNG_DRAW_POSITIVE 0x080014A6u /* GetPositiveSignedRNG */
#define BN5_RNG_DRAW_SECOND 0x080014C0u /* GetRNGSecondary */
#define BN5_RNG_DRAW_SECOND_POSITIVE 0x080014D6u /* GetPositiveSignedRNGSecondary */
#define BN5_SPAWN_COMMON  0x08003098u /* SpawnBattleObjectCommon (BN6_SPAWN_COMMON) */
#define BN5_SPAWN_COMMON_END 0x080030E8u /* ... its end: r5 the object spawned, 0 none */
#define BN5_PLAY_SOUND    0x08000598u /* PlaySoundEffect: r0 the song */
#define BN5_SPAWN_EFFECT  0x080DFFD2u /* an effect spawned: r4's low byte its id (BN6_SPAWN_EFFECT) */
#define BN5_SPRITE_LOAD   0x08002650u /* sprite_load: r5 the object whose sprite state is set up anew (BN6_SPRITE_LOAD) */
#define BN5_SET_ATTACK    0x0800F21Cu /* setAttack's common part (setAttack0-6 at 0x0800F200 + 4N): r0 the CurAction, r2 N, r5 the object */
#define BN5_RESERVE_PANEL_DONE 0x0801866Eu /* object_reservePanel +0x1A (it is at 0x08018654), once the panel is reserved: r5 the object (no bit set in it) */
#define BN5_BLINK_FLASHING 0x080137A8u /* the blink step of an object flashing: bit 1 of its flash timer (+0x26) clear hides it (BN6's opposite) */
#define BN5_OBJECT_RUNNER_RET 0x08003020u /* RunBattleObjectLogic +0x54 (it is at 0x08002FCC): the return address an object's call chain ends at */
#define BN5_OBJECT_RUNNING 0x0200BBF0u /* u32 the node of the object running, 0 outside the runner (0x08002FE2) */
#define BN5_MOOD_GAIN     0x080127FAu /* a side's battle mood raised by r1 (r0 the side: not at 0, dark, nor 0xFF, Full Synchro; 0xFE at most), as
                                       * the hit pass lands a hit that is no counter (0x08017018); BN6's hit pass has no such call (0x0801A228) */
#define BN5_RESULT_WON     1       /* BN5_BATTLE_RESULT (its +1): won */
#define BN5_RESULT_LOST    2       /* BN5_BATTLE_RESULT (its +1): lost */
#define BN5_RESULT_ESCAPED 4       /* BN5_BATTLE_RESULT (its +1): escaped */
#define BN5_OPT_GAME_OVER  0x100   /* BattleSettings +8 options: a loss plays GAME OVER (GetBattleEffects 0x0802B3F2) */
#define BN5_RECORD_BACKDROP 4      /* BattleSettings +4: its background's number, 0xFF the map's (its routine 0x0808CAE8, a byte per map at 0x0808CB1C) */
#define BN5_FREE          0x08800000u /* the guest's ROM copy past BN5's 8 MB: its record copies */
#define BN5_NAVI_FOLDER   (BN5_NAVI_STATS + 0x2D) /* u8: the folder MegaMan fights with (0 the first) */
#define BN5_NAVI_FOLDER1_REG (BN5_NAVI_STATS + 0x2E) /* u8: the first folder's Regular chip, its entry, 0xFF none (BN6_NAVI_FOLDER1_REG's: read as the
                                       * battle builds its deck, 0x08008D64); BN5 has no TagChips (no reader of its NaviStats +0x56) */
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
#define BN5_CHIP_DESCS_LOW  0x737018u /* (ROM offsets) the chips' descriptions' archives, ids 0-255 (as BN6's: E8 06 01 01, F1 00 00, three lines by E9, E7 01, E6), */
#define BN5_CHIP_DESCS_HIGH 0x739744u /* ... and 256 on */
#define BN5_CHIP_ICON_PAL 0x74BDBCu   /* (ROM offset) the chips' icons' palette, 16 colours: 0-8 BN6's, 9 its icons' grey outline (BN6's teal) */
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
#define BN5_T4_OBJECTS    0x02036F00u /* its effects (BN6_T4_OBJECTS), BN6_T4_SIZE each, 32 of them (the kind's record at 0x08003130) */
#define BN5_OBJECT_LIST   0x0200A410u /* its battle's objects in the order they run (BN6_OBJECT_LIST), */
#define BN5_OBJECT_LIST_END 0x0200ABA0u /* ... to this node */
#define BN5_COLL_SIZE     0xA0        /* a collision data's bytes (BN6_COLL_SIZE, 8 more): laid out otherwise from +0x14 (tests/xdiff/legit.json's layout) */
#define BN5_COLL_FLASH    0x26        /* u16 frames left flashing after a hit (BN6_COLL_FLASH, at +0x24 there) */
#define BN5_COLL_MASKS    0x08016364u /* its collision data's mask words by alliance and region (BN6_COLL_MASKS), */
#define BN5_COLL_MASK_WORDS 160        /* ... 80 regions, BN6's 89 */
#define BN5_MPLAYS        0x08155A74u /* its music players (BN6_MPLAYS) */
#define BN5_MEGAMAN_READY 0x06        /* MegaMan's action (BN6_T1_ACTION) when he can act, BN6's 0x08 (0x10 a step, 0x14 a chip, 0x03 hit); his chip (BN6_T1_CHIP) is 0xFFFF as its use begins */
#define BN5_FIELDS        0x0800BD64u /* its battlefields, laid out as BN6's (BN6_FIELDS: 12 bytes each, a nibble a panel), its own
                                       * numbers (its records' byte 0), read through the literal at 0x0800AF84; its panel types numbered
                                       * as BN6's but for 5, 9 and 0xA (BN5_PANEL_FLAGS_OF; docs/ROM_DATA.md, Battlefields) */
#define BN5_PANEL_FLAGS_OF 0x0800BCE8u /* the flags each panel type gives (+0x18), a u32 a type, 11 of them (BN6_PANEL_FLAGS_OF's) */
#define BN5_PANEL_HOLY    9           /* its panel type: holy (BN6_PANEL_HOLY's flag; damage halved, 0x0800C80C) */
#define BN5_PANEL_LAVA    8           /* its panel type: lava (none in BN6: a non-Fire object on it takes 50 and it cools plain, 0x08016DA2;
                                       * BN6's 8 another panel: the differential's MegaMan stepped onto it unhurt, an attack rose from it) */
#define BN5_PANEL_SEA     0x0A        /* its panel type: sea (none in BN6: a Fire object on it hurt, 0x08016EC4; a Wood hit dries it) */
#define BN5_FIELD_PANELS  0x0203A100u /* its field's panels, 8 a row from column 0, rows 0-4, each */
#define BN5_PANEL_SIZE    0x24        /* ... this long (BN6's 0x20): type and alliance at BN6_PANEL_TYPE and BN6_PANEL_ALLIANCE, */
#define BN5_PANEL_FLAGS   0x18        /* ... and its flags here, u32 (BN6's at +0x14), BN6_PANEL_STRUCK while an enemy's attack is on it */
#define BN5_PANEL_LAVA_TIMER 0x10     /* ... lava's frames left, u16, counted down on lava, 0x3C0 written while the panel is another (BN6 keeps one timer, */
#define BN5_PANEL_SEA_TIMER  0x14     /* ... BN6_PANEL_TIMER); and the sea's, the same */
/* Its lava and sea, as src/emu/xpanels.c brings them into BN6's engine
 * (docs/ROM_DATA.md, BN5's lava and sea in BN6's engine): what they do,
 * BN6's routines' twins, and how they are drawn */
#define BN5_PANEL_STEP    0x08016C76u /* what the panel under an actor does to it (BN6_PANEL_STEP): poison, and a Fire object on sea the same (0x08016C9E) */
#define BN5_LAVA_STEP     0x08016D78u /* an object on lava, from the actors' pre-steps before their status step and four obstacles': 50 damage << the
                                       * weakness unless Fire, floating or invulnerable, then the panel plain and a spark over it (0x08016DA2) */
#define BN5_LAVA_STEP_MEGAMAN 0x08016E10u /* ... MegaMan's (his pre-step, 0x0801790E), its tests in another order */
#define BN5_PANEL_HIT     0x08016D0Cu /* a panel hit by an element (BN6_PANEL_HIT): lava cooled plain by Aqua (0x08016D48) */
#define BN5_PANEL_UPDATE  0x0800A990u /* the per-panel update (BN6_PANEL_UPDATE): lava and sea count their timers down to plain, blinking their last 60 frames */
#define BN5_SET_PANEL_TYPE 0x0800B2A6u /* object_setPanelType (BN6_SET_PANEL_TYPE): lava and sea made with 0x3C0 frames */
#define BN5_PANEL_TILESET 0x086F4E5Cu /* its panels' tiles (LZ77) to VRAM from tile 0xA3, 842 of them: lava's from 0x2F5, the sea's from 0x393 */
#define BN5_PANEL_TILES   0x086F6F6Cu /* its tile table (BN6_PANEL_TILES'), 11 types: 8 lava (palettes 2 and 6), 0xA sea (3 and 7) */
#define BN5_PANEL_PALETTES 0x086F7C2Cu /* its panels' palettes 1-8 (BN6_PANEL_PALETTES'): 2 lava (colours 6-10) and poison, 3 the sea (11-14) and ice */
#define BN5_PANEL_ANIMS   0x0800A7A4u /* its panel palettes' animations (BN6_PANEL_ANIMS'): 2 and 6 lava's glow, 10 frames of 16; 3 and 7 the sea's, 8 */
#define BN5_BG_PALETTE    0x03003960u /* IWRAM: its BG palette buffer (BN6_BG_PALETTE's), the panels' animations' destinations */
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
#define BN5_SOUL_PENDING  0x0203B1D0u /* the Soul each side unites with as its next turn starts, 16 bytes a side (+0 the Soul, 0xFF none; +1 1 for
                                       * Chaos Unison; +3 its turns): BN6_CROSS_PENDING's twin, read by 0x080123FA; the Custom screen's record
                                       * of it (0x0203C940, 0x080123E8 writes it at OK) copied there as the turn starts */
#define BN5_CUSTOM_UNITE  0x02036C94u /* the Custom screen's UNITE (its element 0xB, 12 bytes): +0 2 where it stands, +5 the Soul the last chip picked
                                       * unites, +6 1 for Chaos Unison, +7 0 ready, 1 grey, 2 chosen */
#define BN5_BATTLE_SOUL   (BN5_BATTLE_NAVI + 0x2C) /* u8, MegaMan's side's: the Soul he is united with (its Navi's AI index), 0 none */
/* UNITE as BN6's engine takes it over (src/emu/soulunite.c, the Souls track's
 * S2; docs/ROM_DATA.md, UNITE on BN6's Custom screen): its rule, read here and
 * done there, and what of it is copied from the player's ROM */
#define BN5_UNITE_STAND   0x08023C8Cu /* the rule it stands by as the Custom screen opens: its Navi 0, neither worried (BN5_MOOD_WORRIED) nor
                                       * dark (0), Double Soul learned; then grey (+7 1) until a pick lights it */
#define BN5_MOOD_WORRIED  0x40        /* BN5_BATTLE_MOOD: worried from 1 to this, dark at 0 (its emotion getter 0x08012738: 1 worried, 5 dark) */
#define BN5_UNITE_A       0x08024976u /* A on UNITE, BN5_UNITE_A_SIZE bytes: lit, sound 0x81 and the Custom screen's mode 0x24; grey, 0x69. Moved into
                                       * BN6's copy whole, its two calls to BN5_PLAY_SOUND aimed at BN6_PLAY_SOUND */
#define BN5_UNITE_A_SIZE  0x24
#define BN5_UNITE_A_CRC   0xF347D9BBu /* (CRC32 of its bytes in the player's ROM: the move checks it first) */
#define BN5_UNITE_COMMIT  0x08024FFA  /* OK's commit of UNITE (from 0x08024DF0, the pick of type 2): turns 3 + NaviStats +0x32 (1-9), 1 for Chaos
                                       * Unison; the record (0x080123E8: the Soul, chaos, turns); the used bit (BN5_SOULS_USED) */
#define BN5_UNITE_TAKE_BACK 0x08024D48u /* B with UNITE the last pick: the chip given up back in its place (element 0xB's +4), UNITE lit (+7 0) */
#define BN5_TURN_COUNT    0x08012484u /* a side's Soul one turn less (its AIData +0x0F, unless 0 or 0xFF; its NaviStats +0x29 0), called for both sides
                                       * (0x0800778C, 0x08007796) right after the Custom screen opens (0x0802CA68), in the step 0x08007774 */
#define BN5_TURN_END      0x08012464u /* as a turn starts with no form pending (0x08011E02): a form and no turns left end it (0x08012432) */
#define BN5_UNITE_ART_TILES 0x08024574u /* the word naming UNITE's art in the chip's window: 0x087335A4, BN5_UNITE_ART_SIZE bytes (0x08024544) */
#define BN5_UNITE_ART_PALS  0x08024578u /* ... its palettes: 0x08736004, 0x20 bytes each by element 0xB's +6 (0 a Soul, 1 Chaos Unison) */
#define BN5_UNITE_ART_SIZE  0x540
#define BN5_SOUL_PICTURES 0x08023358u /* mode 0x24's word naming the Souls' pictures: 0x0874B2BC, 0x80 bytes by Soul 1-12, 0xD Chaos Unison */
#define BN5_SOUL_PICTURE_PAL 0x08023350u /* ... and their palette: 0x0874BDBC, 0x20 bytes */
/* Team Colonel's Navis, by AI index: their Souls are the six its MegaMan
 * unites with (BN5_SOUL_FLAGS) */
enum { BN5_NAVI_COLONEL = 7, BN5_NAVI_SHADOWMAN, BN5_NAVI_NUMBERMAN, BN5_NAVI_TOMAHAWKMAN, BN5_NAVI_KNIGHTMAN, BN5_NAVI_TOADMAN };

#define BN5_MODE_GAME      0x04   /* BN5_TOOLKIT: the game (the main mode, the index eToolkit +0 points at) */
#define BN5_MODE_GAME_OVER 0x14   /* BN5_TOOLKIT: its GAME OVER */
#define BN5_SUB_MAP        0x04   /* BN5_GAMESTATE: on the map */
#define BN5_SUB_BATTLE_INIT 0x08  /* BN5_GAMESTATE: a battle beginning */
#define BN5_SUB_BATTLE     0x0C   /* BN5_GAMESTATE: in battle */

/* Its virus families as src/emu/xvirus.c moves them into BN6's engine
 * (issue #82; docs/MULTIROM.md, Transplanted families): the tables they
 * are found through, laid out as BN6's (src/emu/bn6_families.h) */
#define BN5_FAMILY_TABLES 0x081045D4u /* nine per-family tables, BN5_FAMILY_STEP apart, BN5_FAMILIES words each, by AI index: 0 the state
                                       * tables (generic states 0-5, the family's own from 6), 1 struct1, 2 struct2, 3 each frame, then the
                                       * hooks 4 spawning, 5 spawned, 6 paralysed, 7 flinching, 8 dragged (BN6's tables 4, 5, 6, 9 and 10) */
#define BN5_FAMILY_STEP   0x8C        /* ... the bytes from one table to the next */
#define BN5_FAMILIES      35          /* ... their entries */
#define BN5_TRAITS_SHADOW 5           /* struct1 +5: a shadow drawn under it (BN6's at BN6_TRAITS_SHADOW, its +5 an element) */
#define BN5_ENEMY_NAMES   0x0873C0B4u /* its viruses' names, a text archive by enemy id (BN6's RomLayout.enemy_names[0]) */
#define BN5_T1_TYPES      0x08003958u /* the first kind's routines by type (BN6_T1_TYPES; its word in the kinds' table 0x08003044), */
#define BN5_TYPES_T1_COUNT 95         /* ... its types */
#define BN5_T3_TYPES      0x08003B80u /* its attacks' routines by type (BN6_T3_TYPES; 0x0800304C), */
#define BN5_TYPES_T3_COUNT 204        /* ... its types */
#define BN5_T4_TYPES      0x08003F60u /* the fourth kind's routines by type (BN6_T4_TYPES; 0x08003050), */
#define BN5_TYPES_T4_COUNT 145        /* ... its types */
/* Its spawners (each r0 the type, r1-r4 its words, r5 back the object or 0), whose calls in a moved family's code the
 * second-level dispatch takes (src/emu/xvirus_dispatch.c, XV_SPAWN_*): each BN6's of the same kind and the same place
 * in the run order (the routine its spawn ends with: after the object running, at the list's end, at its start) */
#define BN5_SPAWN_1       0x08003140u /* the first kind's, run after the object running (0x08003204; BN6_SPAWN_1) */
#define BN5_SPAWN_1_LAST  0x0800315Cu /* the first kind's, at the list's end (0x0800322C; BN6_SPAWN_1_LAST) */
#define BN5_SPAWN_1_LAST2 0x080031E8u /* ... a second of the same code (BN6_SPAWN_1_LAST) */
#define BN5_SPAWN_3       0x08003178u /* the third kind's, after the object running (BN6_SPAWN_3) */
#define BN5_SPAWN_3_LAST  0x08003194u /* ... at the list's end (BN6_SPAWN_3_LAST) */
#define BN5_SPAWN_3_FIRST 0x080031B0u /* ... at its start (0x08003244; BN6_SPAWN_3_FIRST) */
#define BN5_SPAWN_4       0x080031CCu /* the fourth kind's, after the object running (BN6_SPAWN_4); BN5 has no other of the kind */
#define BN5_EFFECTS       0x080DFD7Cu /* its effects, 4 bytes an id as BN6_EFFECTS', BN5_EFFECT_COUNT of them */
#define BN5_EFFECT_COUNT  108         /* ... its effect ids, the effects' code after them (0x080DFF2C) */

/* The Souls' look and abilities in BN6's engine (src/emu/soullook.c, soulpower.c, soulcode.c; the Souls track's S3-S5) */
#define BN5_SOUL_LOADOUTS 0x0801CA18u /* each form's loadout, 6 bytes (AI data +4-+8, +0x11; BN6_FORM_LOADOUTS'): the Souls' 7-12, */
#define BN5_SOUL_LOADOUTS_REF 0x0800DCFCu /* ... the word naming it */
#define BN5_CHARGE_TIMES  0x0801CA68u /* the power attacks' charge times, 5 halfwords an id by the buster's Charge (BN6_CHARGE_TIMES'), */
#define BN5_CHARGE_TIMES_REF 0x080106E0u /* ... the word naming it */
#define BN5_SOUL_ELEMENTS 0x0800E62Cu /* each form's element (0x0800E5F8): TomahawkSoul's 4 Wood, ToadSoul's 2 Aqua, the rest of Colonel's none */
#define BN5_SOUL_ENTER    0x0800F030u /* each form's routine as he enters it (0x0800F01C, from the unison's phase 2): NumberSoul's 0x0800F075, */
#define BN5_SOUL_LEAVE    0x0800F094u /* ... as he leaves it (0x0800F080): NumberSoul's 0x0800F0D7, */
#define BN5_FORM_NULLSUB  0x0800F195u /* ... the other Souls' empty one */
#define BN5_SOUL_READY    0x08011BA4u /* each form's passives as he unites (0x08011B8A, after his loadout): ShadowSoul's FloatShoe 0x08011C79, */
#define BN5_SOUL_REAPPLY  0x08011CCCu /* ... and as a form's state is set again (0x08011CB4, from 0x08011144): ShadowSoul's 0x08011D2B; */
#define BN5_THUMB_RETURN  0x46F7u     /* ... a form with none: its entry a routine of mov pc,lr alone */
#define BN5_ARM_LISTS     0x080151B8u /* a word a list: its rows of an arm's points by sprite number, 0x3C bytes a row (0x080151A8, through its */
#define BN5_ARM_ROW_SIZE  0x3C        /* ... literal 0x080151B4; BN6's rows 0x44, by a row byte for list 0): list 0's the Souls' at their numbers 7-12 */
#define BN5_UNITE_IMAGE_PHASE 0x08012008u /* the unison's phase 1 (its table 0x08011F8C): the Soul's image (0x080C060C), sounds 0x8D/0x143 */
#define BN5_SOUL_FACES_REF 0x080197F0u /* the word naming the Souls' faces in the emotion window (0x087417FC, 0x100 bytes by the Soul - 7), */
#define BN5_FACE_PALS_REF 0x08019800u /* ... the faces' palettes (0x08742AFC, 0x20 bytes by the face: the Soul's at the Soul - 2), */
#define BN5_FACE_DIGITS_REF 0x08019854u /* ... the count's digits under them (0x08741DFC, 0x80 bytes by 10 - the count) */
#define BN5_FACE_SOUL     5           /* the face of the first Soul, 7 (BN5's 0x0801AF68: the Soul - 2) */
#define BN5_FLASH_SOUL    0x23        /* the arm flash's kind of MegaMan's buster in a Soul (0x080EBBCA), its frame the Soul; */
#define BN5_FLASH_CHAOS   0x33        /* ... in Chaos Unison (0x080EBC04) */
#define BN5_FLASH_SPAWN   0x080B9CAEu /* the arm flash's spawner (r4 its word, r5 its owner, r7 where its object goes): the first kind's */
                                      /* type 5, BN5's object (0x080B9B81) with BN5's kinds (0x080B9A5C), moved with the Souls' code */

#endif

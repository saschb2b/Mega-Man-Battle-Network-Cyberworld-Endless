/* Battle Network 5: Team Colonel (USA): the addresses the differential's
 * second core reads (guest.c, src/dev/xdiff.c; docs/ROM_DATA.md, BN5 guest
 * battles) and what is moved from it into BN6's engine. BN5 shares BN6's engine:
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
#define BN5_MOOD_SET      0x080127CEu /* a side's battle mood set to r1 (r0 the side; not at 0, dark), BN6_MOOD_SET's twin */
#define BN5_MOOD_CALM     0x08011A8Cu /* the mood of r5 (him) calmed to 0x80 where it is not 0, dark (BN6_MOOD_CALM's twin) */
#define BN5_RESULT_WON     1       /* BN5_BATTLE_RESULT (its +1): won */
#define BN5_RESULT_LOST    2       /* BN5_BATTLE_RESULT (its +1): lost */
#define BN5_RESULT_ESCAPED 4       /* BN5_BATTLE_RESULT (its +1): escaped */
#define BN5_OPT_GAME_OVER  0x100   /* BattleSettings +8 options: a loss plays GAME OVER (GetBattleEffects 0x0802B3F2) */
#define BN5_RECORD_BACKDROP 4      /* BattleSettings +4: its background's number, 0xFF the map's (its routine 0x0808CAE8, a byte per map at 0x0808CB1C) */
#define BN5_FREE          0x08800000u /* the second core's ROM copy past BN5's 8 MB: its record copies */
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
#define BN5_ARM_VIRUSES_REF   0x080151BCu /* an object's arm points (0x08015140, BN6's 0x08018810: its tables by struct1's list offset at
                                             0x080151B8): the viruses' records' word (sprite list 1's numbers: 0x080154D0) */
#define BN5_ARM_VIRUSES_COUNT 31          /* ... its records, the Navis' table after them (0x08015C14) */
#define BN5_ARM_VIRUS_SIZE    0x3C        /* ... a record: 30 points, x and y signed bytes */
#define BN5_CHIP_POWERS   0x0800E728u /* the computed powers' routines by power - 1000 (BN6_CHIP_POWERS'), 76 of them, read at 0x0800E70E */
#define BN5_CHIP_POWERS_COUNT 76          /* ... its entries (1000-1075), BN6 growing its own table with those a helper moves */
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
#define BN5_COLL_PARALYSIS 0x1E       /* u16 frames left paralysed (BN6_COLL_PARALYSIS, at +0x1C there): its status step 0x0800CB48 counts it down */
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
/* Its sea's submerge (none in BN6's engine: xsea.c makes it again) */
#define BN5_SEA_TEST      0x08017028u /* on sea (BN5_PANEL_SEA) with the AI's traits (+0x48) 0x20: the collision data's +0x2C 0xFFFF, else 0 */
#define BN5_SEA_SUBMERGE  0x0800DF52u /* ... from MegaMan's and the Navis' status step (0x08017D7C): flag 0x80000000 where +0x2C holds 0xFFFF and
                                       * none of the flags 0x00500C00 */
#define BN5_HIDE_STEP     0x0800DEAAu /* what is hidden (0x80000004: invisible or submerged) not drawn, its marker spawned and kept at the
                                       * collision data's +0x50 (BN5_SEA_MARKER_SPAWN), ended as it shows; from 0x08017DB4 */
#define BN5_HIT_HIDDEN    0x08016948u /* the hit gate's test: a hidden target hit only by an attack of flags 0x1008, or of Elec */
#define BN5_PANEL_ARRIVED 0x08017156u /* what the panel an object comes to does, from each actor's pre-step: on sea, a non-Aqua object not
                                       * floating nor going under held 0x14 frames (BN5_IMMOBILIZE) under a splash (effect 0x63, 0x080171E0) */
#define BN5_IMMOBILIZE    0x0800CDA2u /* r0 frames at the collision data's +0x24, flag 0x4000 */
#define BN5_SEA_MARKER_SPAWN 0x080E4D1Eu /* the marker, the fourth kind's 0x3E (a ripple, list 3's 0x2D), spawned: r5 the object, r0 it */
#define BN5_PANEL_SEA     0x0A        /* its panel type: sea (none in BN6: a Fire object on it hurt as on poison, BN5_PANEL_STEP at 0x08016C9E; a Wood hit dries it) */
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
#define BN5_PANEL_BONUS   0x08016AEEu /* what a hit adds for the panel its victim stands on (BN6_PANEL_BONUS), from the hit pass at 0x08016AD6 and
                                         0x080174C4: the attack's damage once more for Fire on grass (6) and Elec on sea (0xA) */
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
#define BN5_CHIP_DARK_GATE 0x15    /* a chip record's +0x15: 0 none; 1 usable only while his dark meter is 470 or more (not dark: HolyDrem, a Navi's
                                    * V1 and SP chips), else only below it (dark: BlakWing, the DS chips); his chip use's test 0x08010110 */
#define BN5_DARK_AFTER    0x0802C938u /* a chip used (its chip use's call, 0x08010086): one of its DarkChips (187-198) runs its routine by
                                       * id from 0x0802C978 on the user's battle NaviStats (src/emu/soulmood.c) */
#define BN5_DARK_AFTER_END 0x0802CA68u /* ... the end of its routines and their table, the bytes the port reads */
#define BN5_DARK_AFTER_CRC 0x25D4DB46u /* ... their CRC32, the port's source */
#define BN5_SOUL_KINDS    0x08024BE4u /* a byte per Soul 1-12 (its Navi's AI index): the chip kind it unites with (the UNITE check 0x08024B2C) */
/* Its Souls (docs/META.md, Souls; docs/ROM_DATA.md, BN5's
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
#define BN5_COLONEL_ID    0x125       /* Colonel's enemy ids, V1, V2, V3 and SP, from here (the differential's colonel_v1: 0x125) */

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
/* What its families' code asks of the engine that BN6's families' never does (issue #82's A2b; docs/ROM_DATA.md, The
 * families' shims and pre-steps) */
#define BN5_BY_NAME_RANGE 0x0800CE54u /* object_getEnemyByNameRange (BN6_BY_NAME_RANGE: the same arguments, BN5's NameIDs) */
#define BN5_COUNT_BY_NAME 0x08104B1Cu /* the shared virus code's count of the enemies of a NameID range (r1, r2, handed on to
                                         BN5_BY_NAME_RANGE) whose word at r0 is not 0, back r0 the count and r1 their sum (BN6's twin
                                         0x0810A960, BN6_COUNT_BY_NAME, the same code) */
#define BN5_VIRUS_DISPATCH 0x08104550u /* the virus dispatcher (BN6's 0x0810A29C), each virus each frame: before the state, cases by
                                          NameID for two families, which the installer ports as their pre-steps (xvirus_shim.h): */
#define BN5_VIRUS_DISPATCH_END 0x081045CCu /* ... its code's end (its literals after), */
#define BN5_VIRUS_DISPATCH_CRC 0x025440F5u /* ... the CRC32 of its code, the port's source */
#define BN5_PRESTEP_APPLEY 0x0810455Cu  /* ... its case for Appley's ids (0x9D-0xA2): while the virus's +0x60 and its collision data's
                                          +0x7C are 0, its pending hit taken by BN5_PRESTEP_TAKE (sound 0x6E where there is one) */
#define BN5_PRESTEP_DOMINERD 0x08104588u /* ... for Dominerd's (0x19-0x1E): while its +0x68 is 0 and a hit is pending (+0x78), the hit
                                          taken by BN5_PRESTEP_TAKE_MIN unless the hit's flags (+0x68) hold 2; its +0x64 set */
#define BN5_PRESTEP_TAKE  0x0800CD1Eu /* a pending hit taken: its statuses (flags2 0xFE) cleared, its damage less the hit's five parts
                                          (+0x7A-+0x82, cleared, and +0x84) left at +0x78 */
#define BN5_PRESTEP_TAKE_MIN 0x0800CD44u /* ... the same with flags2 0x1FE and +0x86 cleared, at least 1 left (+0x78 and +0x7A 1 where
                                          none is) */
#define BN5_PRESTEP_TAKE_END 0x0800CD80u /* ... their code's end, */
#define BN5_PRESTEP_TAKE_CRC 0x0D9E4CF8u /* ... its CRC32, the port's source */

/* Its Navis as src/emu/xnavi_install.c moves them into BN6's engine
 * (issue #82; docs/MULTIROM.md, Transplanted Navis; docs/ROM_DATA.md, The
 * Navis' tables): the tables they are found through by AI index, laid out
 * as BN6's (src/emu/bn6_navis.h) but for the three BN6 added (hook A,
 * frozen, bubbled) */
#define BN5_NAVI_TABLES   0x080F2374u /* ten per-AI tables, BN5_NAVI_TABLE_STEP apart, BN5_NAVIS words each: 0 the state tables (generic
                                       * 0-5, its own from 6), 1 the pre-step, 2 struct1, 3 struct2, 4 the post hook, 5 spawn start, 6 spawn end,
                                       * 7 paralysed, 8 flinching, 9 dragged (BN6's 0, 1, 3, 4, 5, 6, 7, 8, 11, 12) */
#define BN5_NAVI_TABLE_STEP 0x5C      /* ... from one to the next */
#define BN5_NAVIS         23          /* ... their entries: AI 0 (a MegaMan of the story's) to 22 (the Dark Soul's) */
#define BN5_NAVI_NULLSUB  0x080F2749u /* the Navis' empty hook (BN6_NAVI_NULLSUB's) */
#define BN5_NAVI_FRAME    0x080F2334u /* the Navis' per frame (BN6_NAVI_FRAME's): pre-step, state, post, then a counter-hit tally of its own
                                       * (0x080F270C, for the busting level: BN6's battles keep theirs) */
#define BN5_NAVI_HELD     0x08113C78u /* the Navis' held chips' words (BN6_NAVI_HELD_REF's table; its triple 0x08113BE0): Team Colonel's six empty */
#define BN5_NAVI_COMPANION 0x0800EDB4u /* the routine a Navi's companion begins by, a word an AI (BN6_NAVI_COMPANION_REF's table; triple 0x0800ED1C) */
#define BN5_NAVI_COMPANION_END 0x0800EF64u /* ... ends by (triple 0x0800EECC) */
#define BN5_NAVI_HIDE     0x0800F134u /* ... an attack of its hides the overlay by (0x0800F114: BN6_NAVI_HIDE_REF's table), a word an AI: */
#define BN5_OVERLAY_HIDE  0x080C3836u /* ... the overlay's hide, the others the empty companion (0x0800F194) */
#define BN5_ARM_LISTS     0x080151B8u /* its arm points' tables by struct1's list offset (BN6_ARM_LISTS'): MegaMan's 0x080151C4, the viruses',
                                       * the Navis' 0x08015C14, BN5_ARM_RECORD bytes a record: */
#define BN5_ARM_RECORD    0x3C        /* ... 30 points (BN6_ARM_POINTS_BN5) */
#define BN5_NAVI_NAMES    0x0873C978u /* its Navis' names, a text archive by NameID - 0x100 (BN6_NAVI_NAMES_REF's; 145 entries) */
/* Its Navis' records: per AI index a block of the ROM, struct1 first, its code after; Team Colonel's six and the Dark Soul's */
#define BN5_NAVI_ID_FIRST 0x100       /* its Navis' enemy ids from here, six versions an AI (V1, V2, V3, SP, DS, a sixth unused) */
#define BN5_DARK_SOUL_ID  0x18D       /* the Dark Soul's id (AI 0x16, one version): what a Chaos Unison released at the wrong time spawns */
#define BN5_NAVI_DARK_SOUL 0x16       /* its AI index */
#define BN5_DARK_SOUL_NAME 0x087F4404u /* "DarkSoul", a word of its script: the Dark Soul's name in BN6's engine. Its id's entry
                                       * in BN5_NAVI_NAMES reads DrkCloud (BN5's own first Custom screen names the id so), and a
                                       * failed Chaos Unison brings it later, past the one screen that names the foes */
#define BN5_DARK_SOUL_NAME_LEN 8   /* ... its letters, no end of its own (the script runs on) */
/* What BN6's engine lacks, which a Navi's moved code calls: stubbed */
#define BN5_CLAIM_SPAWN   0x080EA08Cu /* a panel claim spawned (the fourth kind's 0x83) where the battle is a gate's (0x080090E8: BattleState
                                       * +0x32 bit 0x40, set by 0x0802D594 alone), else r0 0: the stub's answer, its result unread by its callers */
#define BN5_CLAIM_SPAWN2  0x080EA0BAu /* ... its other spawner, the same */
#define BN5_CHIP_AI_TEST  0x0802C10Au /* the DS version's chip AI: whether it is under way (AIData +0xF0; set only by BN5_CHIP_AI_ENTER) */
#define BN5_CHIP_AI_ENTER 0x0802C114u /* ... entered, from a Navi's own code where its version (AIData +0x16) is 4, the DS */
#define BN5_CHIP_AI_RUN   0x0802B804u /* ... run each frame while under way */
#define BN5_CHIP_AI_DARK  0x0802BA18u /* ... the Dark Soul's own (its state AIData +0xF0: +1 its step, +2 and +3 their phases, +4 its
                                       * misses, +5 and +0xC its target's, +0x24 the chip under way) */
#define BN5_DARK_REGION   0x08013C64u /* its actor's init: the Dark Soul's id's test (ldr r0,=BN5_DARK_SOUL_ID), its region 0x13 */
#define BN5_DARK_PLATE    0x08013394u /* its arrival into a fight under way (AIData +2 not 0, 0x080131A0: substate 0x08013387): the Dark
                                       * Soul's id's test, 0x0801A5BC(0, 0), which lists a foe and its HP, skipped */
#define BN5_AI_DATA_SIZE  0x140       /* an AI's data, BN5's (BN6_AI_DATA_SIZE 0x100: BN6 dropped the DS's chip AI, +0xF0 to +0x13F) */
#define BN5_AI_FLASH      0x3C        /* AIData +0x3C, u16: the green flash's frames left (DarkPlus's: bn5_chips.h; BN6's +0x3C his resting height) */
#define BN5_DARK_LISTS    0x02034C20u /* the chip AI's lists, BN5_DARK_LIST_SIZE a side: its chips (u16 each, 0x2A of them: a chip's
                                       * id, or 0x8000 + a sequence's number; 0xFFFF none), +0x54 their count (u32), +0x58 eight
                                       * sequences of 16 bytes (its place's offsets from the target, s8 each, then up to seven chips,
                                       * 0xFFFF after them); copied as each battle is set up (0x08009B92) from what the battle's
                                       * exchange carried, made of the save's usage record (0x0802C7C2, eToolkit +0x78) */
#define BN5_DARK_LIST_SIZE 0xE0u     /* ... a side's bytes */
#define BN5_DARK_LIST_COUNT 0x54u    /* ... the chips' count (u32) in a side's */
#define BN5_DARK_SEQUENCES 0x58u     /* ... its sequences in a side's */
/* A chip record's bytes the Dark Soul's chip AI reads (by 0x0801DF0C, BN6's twin the same code; its attack, 0x0802C098, reads
 * +4, +0xA-+0xC, +0x10 and +0x14, which BN6's records hold the same way), which BN5's records give a meaning BN6's do not */
#define BN5_CHIP_DS_FLAGS  0x09       /* bit 0: a chip it plays from a step of its own (0x0802B8CC, 0x0802BAEE: state 0x10 or 0x18) */
#define BN5_CHIP_DS_USAGE  0x0D       /* the approach it takes (0x08029B54: its routines' table 0x08029B6C; bn6f's DarkSoulUsage) */
#define BN5_CHIP_DS_PICKS  0x0E       /* bits 0 and 1: a chip its list's search moves to the front (0x0802BE80 bit 0, 0x0802BECC bit 1) */
#define BN5_CHIP_DS_REGION 0x17       /* the panels it aims from (0x08029D24, 0x08029E30, 0x0802AC2E; BN5's DS region table 0x08022AF8;
                                       * BN6's byte here its lock-on type, BN6_CHIP_LOCK_ON_TYPE) */
#define BN5_BATTLE_BITS   0x5C        /* BattleState +0x5C, a word of bits (0x0800131E sets, 0x0800132A clears, 0x08001336 reads): 1, 2,
                                       * 4 and 8 the sides' (0x0C as each battle is set up, 0x08008F64), 0x10 and 0x20 a Colonel's on
                                       * side 0 and 1 (BN5_COLONEL_SIDE_SET; read by the obstacles' dark hole, BN5_DARK_HOLE_TEST), 0x40
                                       * the field's slide as a battle ends (BN6's 0x10) */
#define BN5_COLONEL_SIDE_SET 0x080CAD06u /* a Colonel's side's bit set (0x10 side 0, 0x20 side 1): from Colonel's spawn's end (his table 6,
                                       * 0x080F8500, at 0x080F8524) after his table (0x080CACE0), and ColonelSoul's (0x08011C42) */
#define BN5_DARK_HOLE_TEST 0x080CABEAu /* an obstacle's panel the side opposite a Colonel's, his bit set, a panel beside it his side's
                                       * (0x080CAC42, 0x080CAC98): from the obstacles' states (0x08018014, 0x0801817C, 0x080182E8,
                                       * 0x08018444), a T3 0x30 spawned there (0x080CABCA) */
#define BN5_COLONEL_TABLE 0x02034000u /* Colonel's two 8-byte entries by alliance (written by his AI and the engine's 0x08011C48, read by his
                                       * attacks' objects): BN6_COLONEL_TABLE in BN6 */
#define BN5_CHAOS_FAIL_SPAWN 0x08006AAEu /* where a failed Chaos Unison spawns the Dark Soul (BN5_DARK_SOUL_ID) on the other side */

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

/* ColonelSoul's button and strike (src/emu/soulcolonel.c, moved by tools/xsoul.py) */
#define BN5_COLONEL_PHASE0 0x080236BCu /* mode 0x38's phases (its table 0x080236B4): the last pick into the button, its icon to OBJ 0x06016EC0, */
#define BN5_COLONEL_PHASE1 0x080236E8u /* ... its blink, 30 frames, and back to mode 4 (the lit rules again, 0x08024A2A) */
#define BN5_COLONEL_DRAW  0x080254F8u /* the loaded chip drawn on the button (an OBJ, its tile 0x376, palette 10), modes 4 and 0x24 each frame */
#define BN5_COLONEL_ART   0x08024180u /* the word naming ColonelSoul's button's tiles on the chips' screen (type 8's draw, 0x0802415E):
                                         0x086FBA88, 0x180 bytes lit and chosen, then 0x180 grey */
#define BN5_COLONEL_INFO_ART 0x080245C4u /* ... its picture in the chip's window (0x080245A4): 0x087508BC, 0x540 bytes, */
#define BN5_COLONEL_INFO_PAL 0x080245C8u /* ... its palette: 0x08750E1C, 0x20 bytes, */
#define BN5_COLONEL_NAME_PAL 0x08024650u /* ... and its name line's (0x0802460A): 0x086FC1E8, 0x20 bytes */
#define BN5_COLONEL_STRIKE_SPAWN 0x080CABCAu /* the obstacle strike's spawn: the third kind's 0x30 at r0, r1, its side r3 (0x08018026) */
#include "bn5_expression.h"   /* what BN5 has to show a mood where BN6 has the like or lacks it */
#include "bn5_liberation.h"   /* its Liberation Missions' records and board (docs/LIBERATION.md) */
#endif

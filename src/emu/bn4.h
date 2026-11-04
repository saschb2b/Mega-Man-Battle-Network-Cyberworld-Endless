/* Battle Network 4: Red Sun (USA): what is mapped of it, read for research
 * alone so far (docs/rom/bn4.md; docs/multirom/bn4.md, the plan). BN4's
 * engine is BN5's and BN6's elder: its toolkit is BN6's from +0x3C shifted
 * by 4, its chip records 0x2C bytes, its battle records and enemy data its
 * own. Its save block (the game state, NaviStats, folders, flags) moves in
 * RAM at each change of mode: read through eToolkit, never as a constant. */
#ifndef CW_BN4_H
#define CW_BN4_H

#define BN4_TOOLKIT       0x02009E70u /* eToolkit, set up by `ldr r0,=eToolkit; mov r10,r0` (0x080060D8) as BN5's and BN6's are */
#define BN4_TOOLKIT_GAMESTATE 0x40    /* eToolkit +0x40: the game state (BN5's and BN6's at +0x3C), in the moving block */
#define BN4_TOOLKIT_BATTLE 0x18       /* eToolkit +0x18: the battle state, BN4_BATTLE_STATE (fixed) */
#define BN4_TOOLKIT_FLAGS 0x48        /* eToolkit +0x48: the event flags (flag f in byte f >> 3, bit 0x80 >> (f & 7)), in the moving block */
#define BN4_TOOLKIT_FOLDERS 0x4C      /* eToolkit +0x4C: the folders, 30 u16 each (chip | code << 9), in the moving block */
#define BN4_TOOLKIT_NAVI_STATS 0x78   /* eToolkit +0x78: MegaMan's NaviStats, 0x40 bytes (BN5's 0x60), in the moving block */
#define BN4_NAVI_HP       0x30        /* NaviStats +0x30: HP (a battle ended at 37 began the next at 37), +0x32 and +0x34 the max HPs */
#define BN4_NAVI_MAX_HPS  0x32        /* NaviStats +0x32 and +0x34: the max HPs, u16 each (which is the base assumed; the guest's battles write both) */
#define BN4_MODE_MAP      4           /* eToolkit [+0]'s main mode on the map (the game state's +0 too) */
#define BN4_MODE_BATTLE   8           /* ... in a battle, a main mode of its own in BN4 */
#define BN4_MODE_GAME_OVER 0x18       /* ... at GAME OVER */
#define BN4_BATTLE_STATE  0x02035810u /* BattleState: +1 the phase (8 the Custom screen, 0xC fighting, 0x10 over, 0x14 ending), +0xD the side,
                                       * +0x32 its flags (u16), +0x3C the record fought */
#define BN4_BATTLE_SIDE   (BN4_BATTLE_STATE + 0x0D) /* u8: MegaMan's side (BN6_BATTLE_SIDE's twin), as the state's comment has it */
#define BN4_BATTLE_TIMER  (BN4_BATTLE_STATE + 0x40) /* u32, the frames fought: 0 through BATTLE START!, held on the Custom screen, one more
                                       * a fight frame (BN5_BATTLE_TIMER's place; seen counting so in xdiff's game=bn4 battle) */
#define BN4_CUSTOM_WINDOW 0x02036406u /* the Custom screen's window as it slides in, 0 to 0x78 open (BN6_WINDOW_OPEN): the gauge's struct
                                       * 0x020363F0 + 0x16, BN5's +0x12 shifted by 4 as its gauge (+0x24, BN5's +0x20) is; seen
                                       * 0x78 while it shows, 0 once OK closed it and in the fight */
#define BN4_BATTLE_RESULT 0x0203BCB4u /* the battle's result, 1 won, 2 lost (a fixed copy of the game state's +0x10) */
#define BN4_BATTLE_REWARD 0x0203BD4Cu /* u16: the reward the results screen gives (a Mettaur's: 0x0058, Guard1 A), the Busting level at -7 */
#define BN4_BATTLE_NAVI   0x0203BEC0u /* the battle's NaviStats, 0x40 a side (GetBattleNaviStatsAddr, 0x0800D742), HP at +0x30 */
#define BN4_NAVI_SIZE     0x40        /* ... a side's (BN6_BATTLE_NAVI_SIZE's twin) */
#define BN4_CUSTOM_GAUGE 0x02036414u /* u16, full at 0x4000 (SetCustGauge, 0x08016810: its struct 0x020363F0 + 0x24) */
#define BN4_OBJECT_KINDS  0x080031B8u /* the object runner's (0x0800314C) table by kind: the first kind's types BN4_T1_TYPES, the third's
                                       * 0x08003C40 (165), the fourth's 0x08003F78 (130); each type run with `mov lr,pc; bx` */
#define BN4_T1_TYPES      0x08003A24u /* the first kind's 95 types, a routine each: the virus families, the Navis, MegaMan. A family's routine
                                       * runs a jump table on +8 (0 its init, 4 its update: the pre-step 0x080137BC, the state runner
                                       * 0x08013C48 with its state table, the collision's present 0x08012D62; 8 the shared dying state):
                                       * BN4 has none of BN6's per-family tables */
#define BN4_T3_TYPES      0x08003C40u /* the third kind's 165 types (BN4_OBJECT_KINDS) */
#define BN4_TYPES_T3_COUNT 165       /* ... how many */
#define BN4_T4_TYPES      0x08003F78u /* the fourth kind's 130 */
#define BN4_TYPES_T4_COUNT 130       /* ... how many */
#define BN4_TYPES_T1_COUNT 95        /* the first kind's types (BN4_T1_TYPES), how many */
#define BN4_TYPE_INIT     0x00        /* a family's routine's jump table (its first literal of three words): +0 its init, */
#define BN4_TYPE_UPDATE   0x04        /* ... +4 its update (BN4_PRESTEP, BN4_STATE_RUNNER with its state table, the collision's present), */
#define BN4_TYPE_DYING    0x08        /* ... +8 the shared dying state, BN4_DYING */
#define BN4_DYING         0x0801052Du
#define BN4_PRESTEP       0x080137BCu /* a family's update's first call (Billy's 0x080B9568) */
#define BN4_STATE_RUNNER  0x08013C48u /* its second: r0 the family's state table, run by CurAction (+9) */
/* A family's state table opens with BN4's generic states 0-5 (BN5's 0-5 as BN5's families' do; BN6's 0-5 of the same places,
 * tools/bn4_twins.py GENERIC4), its own from 6 (Billy's 0x080B9580: 6-9) */
#define BN4_STATE_GENERIC_0 0x0800FCADu
#define BN4_STATE_GENERIC_1 0x0801013Bu
#define BN4_STATE_GENERIC_2 0x0801014Du
#define BN4_STATE_GENERIC_3 0x08010277u
#define BN4_STATE_GENERIC_4 0x080104ADu
#define BN4_STATE_GENERIC_5 0x080105BDu
#define BN4_BY_NAME_RANGE 0x080B6568u /* the enemies of a NameID range (r0 a list it fills, r1 and r2 the bounds): BN6_BY_NAME_RANGE's work, its side
                                       * its own (BN6's takes it in r3) */
/* Its spawners (each r0 the type, r1-r4 its words, r5 back the object or 0), whose calls in a moved family's code the
 * second-level dispatch takes (src/emu/xvirus_dispatch.c, XV_SPAWN_*): each BN6's of the same kind (its movs r0) and the
 * same place in the run order (the routine its spawn ends with: 0x0800337C after the object running, 0x080033A4 at the
 * list's end, 0x080033BC at its start), as BN5's are */
#define BN4_SPAWN_COMMON  0x08003210u /* SpawnBattleObjectCommon (BN6_SPAWN_COMMON's twin): r0 the kind, r1 the spawner's five words */
#define BN4_SPAWN_1       0x080032B8u /* the first kind's, run after the object running (BN6_SPAWN_1) */
#define BN4_SPAWN_1_LAST  0x080032D4u /* ... at the list's end (BN6_SPAWN_1_LAST) */
#define BN4_SPAWN_1_LAST2 0x08003360u /* ... a second of the same code (BN6_SPAWN_1_LAST) */
#define BN4_SPAWN_3       0x080032F0u /* the third kind's, after the object running (BN6_SPAWN_3): Spikey's and Shrimpy's attacks' */
#define BN4_SPAWN_3_LAST  0x0800330Cu /* ... at the list's end (BN6_SPAWN_3_LAST) */
#define BN4_SPAWN_3_FIRST 0x08003328u /* ... at its start (BN6_SPAWN_3_FIRST) */
#define BN4_SPAWN_4       0x08003344u /* the fourth kind's, after the object running (BN6_SPAWN_4) */
#define BN4_SPAWN_EFFECT  0x080E18EEu /* an effect spawned: r4's low byte its id (BN6_SPAWN_EFFECT's twin by body); BN4's effect table not mapped */
#define BN4_PLAY_SOUND    0x08000560u /* PlaySoundEffect: r0 the sound (BN6_PLAY_SOUND's twin by body), BN4's own numbers (its songs at
                                       * RomLayout.song_table 0x155FF8, compared with BN6's by how they sound: tools/xrecipe_bn4.py) */
#define BN4_TIME_STOP     0x08007948u /* battle_isTimeStop: the battle's flags' bit 4 (BN6's 0x0800A098, the same body) */
#define BN4_HIGHLIGHT_PANEL 0x08009DA0u /* a panel (r0 column, r1 row) lit where an attack will strike: its byte of BN4_PANEL_HIGHLIGHTS */
#define BN4_SET_PANEL_TYPE 0x08009DC4u /* a panel's type set (r0 column, r1 row, r2 type) unless it is a hole, for BN4_PANEL_TIMER's 0xBE frames */
#define BN4_ENEMY_PLAYER_ROW 0x0800A17Au /* the row of the other side's player (BN4_FIND_PLAYER), or its own (BN6's 0x0800D048, its future row
                                       * where his row is off the field) */
#define BN4_DIRECTION_TO  0x0800AFB0u /* the direction from its panel to another (0x0800AF90: r0-r3, 0 the same panel, 1-4) into
                                       * BN4_COLL_DIRECTION; BN6's 0x0800E994 answers neighbours alone */
#define BN4_FIND_PLAYER   0x0800C456u /* battle_findPlayer: the player object of side r0 (BN6's 0x080103BC, its own search) */
#define BN4_OBJECT_DESTROY 0x08010561u /* object_genericDestroy, a moved object's jump table's third entry (BN6's 0x08016C8B) */
#define BN4_HIT_TAKEN     0x080CDCE4u /* an object's hit taken off its HP (r0 0), -1 back where none is left (BN6's 0x0801156A) */
#define BN4_T3_0B_KINDS   0x080CE5B0u /* the third kind's 0x0B's kinds, 16 bytes each: its collision set-up, element (+3), hit effect, sprite
                                       * (+5 list offset, +6 number), speed (+0xC); CirKill's 10-12 */
#define BN4_TILE_BUFFER   0x02037810u /* a buffer of sprite tiles copied each frame (its end at +0, its fill at +4): Elmperor's aura draws
                                       * through it (the fourth kind's 0x4C); no BN6 twin mapped */
/* Walla's, Elmperor's and CirKill's types (bn4_twins.TYPES, docs/rom/bn4.md), each moved through the second-level dispatch */
#define BN4_T3_WALLA_RING 0x3C        /* the third kind's: Walla's rings (two a throw), flying at a speed of their own, shot down by their HP */
#define BN4_T3_ELMPEROR_STRIKE 0x79   /* ... Elmperor's strike, the panel it lands on given its element's type */
#define BN4_T3_CIRKILL_SHOT 0x0B      /* ... CirKill's shot, of BN4_T3_0B_KINDS' kind 10 + its version / 2 */
#define BN4_T4_ELMPEROR_AURA 0x4C     /* the fourth kind's: Elmperor's aura over it (not moved: an interim) */
#define BN4_ID_RAISE      0x0800B036u /* an enemy id raised by the level ([eToolkit +0x44] +0xD, at most 2): a virus's by 2 a level (to its
                                       * version 5), a Navi's by 1; that the level is the playthrough's is assumed */
#define BN4_COLL_SIZE     0x90        /* an object's collision data (BN5's 0xA0, BN6's 0xA8), its offsets otherwise */
#define BN4_T1_OBJECTS    0x0203B180u /* the battle's first kind of objects, BN4_T1_SIZE each, 8 (BN5 16, BN6 32): MegaMan first */
#define BN4_T1_SIZE       0xD8
#define BN4_T1_COUNT      8           /* (the kinds' descriptor at 0x08003440: 0x6C0 bytes) */
#define BN4_T1_TYPE       0x01        /* its type (BN4_T1_TYPES), an enemy's its record's BN4_ENEMY_TYPE; MegaMan 0 */
#define BN4_T1_NAME_ID    0x0F        /* u8: its enemy id (BN6's NAME_ID is a u16 at +0x28, 0 on every BN4 object) */
#define BN4_T1_PANEL_X    0x12        /* its panel's column, then its row at +0x13, the ones it moves to at +0x14 and +0x15, as BN6's */
#define BN4_T1_ALLIANCE   0x16        /* its side, as BN6's */
#define BN4_T1_ELEMENT    0x17        /* its element (BN6's +0x0E; BN6's flip at +0x17, which BN4 has none of) */
#define BN4_T1_HP         0x24        /* u16: an object's HP (MegaMan's 0x0203B1A4, the first enemy's 0x0203B27C; poked, the screen followed);
                                       * its max at +0x26, as BN6's */
#define BN4_T1_X          0x34        /* 16.16: its place, Y at +0x38 and Z at +0x3C, as BN6's */
#define BN4_T1_COLLISION  0x54        /* its collision data, as BN6's */
#define BN4_T1_AI_DATA    0x58        /* its AI data, as BN6's: 0 on a virus */
#define BN4_T3_OBJECTS    0x0203C080u /* the third kind's objects, 32 of 0xD8 (the buster's shots among them) */
#define BN4_T4_OBJECTS    0x02037D10u /* the fourth kind's, 32 of 0xC8 */
#define BN4_OBJECT_LIST   0x02009E40u /* the run list's head, its end 0x0200A4C0 (BN6's 0x02009380, 0x02009AB0) */
#define BN4_OBJECT_LIST_END 0x0200A4C0u /* ... its end, a node's +4 there ending the run order (BN6_OBJECT_LIST_END's twin) */
#define BN4_TOOLKIT_COLLISION 0x30    /* eToolkit +0x30: the collision data's pool, 32 of BN4_COLL_SIZE (0x02039A10) */
#define BN4_COLL_REGION   0x01        /* collision data: its region's kind (BN6's +1, the same place: BN4's 0x08009EE4 and BN6's
                                       * object_highlightPanelRegion 0x0800CCBE read it there) */
#define BN4_COLL_ALLIANCE 0x03        /* collision data: its side (BN6's +4) */
#define BN4_COLL_HIT_EFFECT 0x07      /* ... its hit's effect (BN6's +9): BN4's setter 0x08012F74 is BN6's object_setCollisionHitEffect
                                       * (0x0801A140) but for this offset */
#define BN4_COLL_PANEL_X  0x08        /* ... its panel's column, its row at +9 (BN6's +0xA, +0xB) */
#define BN4_COLL_PARALYSIS 0x10       /* ... u16: paralysis's frames (BN6's +0x1C); confusion +0x12, blindness +0x14, the hold +0x16 */
#define BN4_COLL_CONFUSE  0x12        /* ... u16: confusion's frames (BN6_COLL_CONFUSE, +0x1E) */
#define BN4_COLL_BLIND    0x14        /* ... u16: blindness's frames (BN6_COLL_BLIND, +0x20) */
#define BN4_COLL_HOLD     0x16        /* ... u16: the hold's frames (BN6_COLL_IMMOBILE, +0x22) */
#define BN4_COLL_FLASH    0x18       /* ... u16: the flashing after a hit (BN6's +0x24): 0x78, less 1 a frame, blinking by bit 2 */
#define BN4_COLL_INVULNERABLE 0x1C    /* ... u16: invulnerability's frames (BN6's +0x28); BN4 has no freeze or bubble */
#define BN4_COLL_FINAL_DAMAGE 0x28    /* ... u16: the HP a hit takes, subtracted at 0x08013C58 (BN6's +0x80) */
#define BN4_COLL_FROM_COLLISION 0x54  /* ... u32: the flags its hits left it (BN6's FlagsFromCollision +0x70): the thunder's update
                                       * (the third kind's 0x2A, 0x080D20C8) is BN6's (0x080CAD2C) but for this offset (0x080D215E) */
#define BN4_COLL_FLAGS    0x64        /* ... u32: ObjectFlags1, BN6's bits (BN6's +0x3C) */
#define BN4_COLL_STRUCK   0x68        /* ... u32: ObjectFlags2, BN6's bits (BN6's +0x40) */
#define BN4_COLL_POISON_TIMER 0x06    /* ... u8: frames to poison's next point on a poison panel (BN6's +8): 6, less 1 a frame there
                                       * (0x0801301E, BN6's 0x0801A1AA); Elmperor holds it at 0xFF, poison never striking it */
#define BN4_COLL_DIRECTION 0x0A       /* ... u8: the direction it strikes in, 0-4 (BN6's +0xC; BN4_DIRECTION_TO writes it) */
#define BN4_COLL_MASKS    0x08012660u /* the collision masks, two words a region (r0 the side), read by 0x08012EF6: BN6_COLL_MASKS' words but
                                       * for its bit 0x80, region for region (the first 60 compared) */
#define BN4_AI_DATA       0x02035B20u /* the AI data: one a side (MegaMan's, a Navi's), 0xC0 each, no pool (BN6 8 of 0x100) */
#define BN4_AI_DATA_SIZE  0xC0        /* (0x0800BFF4: `movs r1,#0xc0; muls`, then BN4_AI_DATA) */
#define BN4_AI_ATTACK_VARS 0x70       /* AI data: the attack's variables, 0x50 (BN6's +0xA0); the damage at +0x78, the chip at +0x84 */
#define BN4_FIELD_PANELS  0x0203AC70u /* the panels, BN4_PANEL_SIZE each, (row * 8 + column) << 5 */
#define BN4_PANEL_SIZE    0x20
#define BN4_PANEL_TYPE    0x00        /* a panel's type, 2 plain (BN6's +2) */
#define BN4_PANEL_ALLIANCE 0x01       /* its side (BN6's +3) */
#define BN4_PANEL_FLAGS   0x04        /* u32: its type, flags, side and occupants (BN6's +0x14) */
#define BN4_PANEL_RESERVER 0x14       /* the object that reserved it (BN6's +0x1C) */
#define BN4_PANEL_TIMER   0x12        /* u16: the frames a panel set by BN4_SET_PANEL_TYPE keeps its type (0xBE there; BN6's +0x12 counts
                                       * for its conveyors alone) */
#define BN4_PANEL_HIGHLIGHTS 0x02037ABEu /* a byte a panel (row * 8 + column): 1 where an attack will strike, BN4_HIGHLIGHT_PANEL's; BN6
                                       * keeps it in the panel's +1 (object_highlightPanel, 0x0800CBD8) */
#define BN4_RNG_PRIMARY   0x02001790u /* the main RNG's state (GetRNG's literal) */
#define BN4_RNG_SECONDARY 0x020015D4u /* the second RNG's state: stepped by GetRNG's twin at 0x08001B30, the same code on its own word */
#define BN4_FREE          0x08800000u /* the guest core's ROM copy past BN4's 8 MB: its records (src/emu/guest.c) */
#define BN4_PLAY_MUSIC    0x08000568u /* PlayMusic: r0 the song (0xFF stops it), kept at the game state's +0x19; not again while it plays */
#define BN4_MPLAYS        0x08155E78u /* the music players' table (m4aSongNumStart, 0x08112848); songs 0x00-0x21 on player 31 */
#define BN4_SOUND_INFO    0x030036A0u /* MP2K's SoundInfo (BN6's 0x030045C0), sound mode 0x0093040D as BN6's, 32 players */
#define BN4_MAP_MUSIC     0x0802AFD0u /* four lists by the game state's +7, 8 bytes an entry (the group, a pointer to a song per map), 0xFF
                                       * ending them, as BN5's (0x080361C8); read at 0x0802BCCC */
#define BN4_MODE_SONGS    0x08007510u /* the songs of battle modes 0x46 on (BN4_RECORD_MODE - 0x46): 14 14 15 15 17 17 17 */
#define BN4_SONG_TITLE    0x00        /* the title's */
#define BN4_SONG_NET      0x12        /* the net areas' theme (BN4_MAP_MUSIC: ACDC, Town, Park Area and the countries') */
#define BN4_SONG_UNDERNET 0x13        /* the Undernet's and Black Earth's */
#define BN4_SONG_BATTLE   0x14        /* random battles' */
#define BN4_SONG_BOSS     0x15        /* the Navis' (the roaming SP Navis', the story's ShadeMan, LaserMan and Bass) */
#define BN4_SONG_DUO      0x16        /* Duo's */
#define BN4_SONG_TOURNAMENT 0x17      /* the tournament's records' (their opponents; that it is its theme is assumed) */
#define BN4_GAMESTATE_GROUP  0x04     /* the game state's +4: the map group (0x80 on the net) */
#define BN4_GAMESTATE_NUMBER 0x05     /* ... +5: the map's number in its group */
#define BN4_GAMESTATE_RECORD 0x2C     /* ... +0x2C: the battle record the encounter check chose, which the battle's setup reads (0x08006680) */
#define BN4_ENCOUNTER_CHECK 0x080F5370u /* the encounter check the map loop calls each frame (0x080053E6): on the net, its steps counted, a
                                       * record rolled (BN4_ROLL), stored at BN4_GAMESTATE_RECORD and returned (0 none); a record
                                       * returned starts its battle (0x080053F4). Patched to `ldr r0,=record; bx lr`, it starts that battle */
#define BN4_ROLL          0x080F5448u /* the roll: [[[BN4_BATTLE_TABLES + 4 on the net] + group * 4] + (number + its story offset) * 4], the
                                       * records whose BN4_RECORD_MASK meets r2's drawn by BN4_RECORD_WEIGHT (GetPositiveSignedRNG) */
#define BN4_STORY_OFFSET  0x080F54B8u /* the offset a net group's routine (one per group, by group - 0x80) adds to the map's number:
                                       * 0, or 0x10, 0x20 or 0x30 by story flags in the areas (ACDC Area's 0x080F5568) */
#define BN4_ENCOUNTER_RATES 0x080189E8u /* 16 bytes per net group (group - 0x80), one per map: its rate's class, 7 none */
#define BN4_BATTLE_TABLES 0x08017FB8u /* two pointers, the real world's map groups and the net's (group - 0x80, 21): per group a pointer
                                       * per map index (16 a story stage), per index a list of BN4_RECORD_SIZE records, 0xFF ending them */
#define BN4_RECORD_SIZE   12          /* a battle record: 8 bytes, then the pointer to its entities (BN5's and BN6's are 16 bytes) */
#define BN4_RECORD_WEIGHT 0x00        /* its weight in the roll (0x20 on ACDC Area's) */
#define BN4_RECORD_MODE   0x04        /* the battle's mode, copied to the battle state's +0xF (0x08007ADE): 0, 0x11 in 22 of the net's records;
                                       * from 0x46 its music is BN4_MODE_SONGS' */
#define BN4_RECORD_BACKDROP 0x05      /* the battle's background (BN4_BATTLE_BGS' index), 0xFF the map's (BN4_MAP_BACKDROPS): the net's random ones all 0xFF */
#define BN4_RECORD_MUSIC  0x03        /* the song its battle plays (0x080074A4), 0xFF none: BN4_SONG_BATTLE on every virus record of the net */
#define BN4_RECORD_MASK   0x06        /* tested against the roll's r2 (a progress byte, 0x17 of eToolkit +0x78's): 0x1F every one */
#define BN4_RECORD_ENTITIES 0x08      /* u32: its entity list, BN4_ENTITY_SIZE bytes each, 0xFF ending it */
#define BN4_ENTITY_SIZE   4           /* an entity: the enemy id (0 MegaMan), the panel's column and row, its kind */
#define BN4_ENTITY_KIND   0x03        /* ... its kind: 0 MegaMan, 1 an enemy, 5 an obstacle (id 1 a rock) */
#define BN4_ENEMY_STATS   0x08011888u /* a record per enemy id, BN4_ENEMY_STAT_SIZE bytes (0 MegaMan, 1-126 the viruses, 128 on the Navis) */
#define BN4_ENEMY_STAT_SIZE 12
#define BN4_ENEMY_COUNT   253         /* ids 0-252, as its enemy names archive (BN4_ENEMY_NAMES) holds them */
#define BN4_ENEMY_HP      0x00        /* u16: element << 12 | HP (Mettaur 40, VolGear Fire 80), as BN6's stats' first word */
#define BN4_ENEMY_TYPE    0x02        /* its first-kind object type (BN4_T1_TYPES): its family's own code (Mettaur 0x02, Billy 0x13, Spikey 0x17) */
#define BN4_ENEMY_KIND    0x03        /* its sprite's list, as an offset (4 list 1, the viruses'; 8 list 2, the Navis'), read at 0x08002B0E */
#define BN4_ENEMY_FAMILY  0x04        /* its sprite's number in that list: the virus's family (1 Mettaur, 3 Billy) or the Navi's (1 Roll) */
#define BN4_ENEMY_VERSION 0x05        /* its version in its family: 0-5 (V1, EX, V2, EX, V3, EX) */
#define BN4_ENEMY_ATTACK  0x0A        /* u16: its attack's damage (Mettaur 10, Mettaur3 160) */
#define BN4_CHIP_RECORDS  0x080197ECu /* its chip records, 0x2C bytes each as BN6's: codes at +0, power at +0x1A, +4 and +5 as BN6's; +6 to +8 not */
#define BN4_CHIP_MB       0x06        /* its MB (Cannon 8, the DarkChips 99): BN6's +8 */
#define BN4_CHIP_KIND     0x07        /* its kind, the icon's and the one a Soul unites by (0 Fire .. 11 Break, 12 none, 15 a Soul) */
#define BN4_CHIP_CLASS    0x08        /* 0 standard, 1 Mega, 2 Giga, 3 secret, 4 a Program Advance: BN6's +7 */
#define BN4_CHIP_DARK     0x20        /* its +9 (BN6's effect flags): a DarkChip, ids 187-194 (DrkCanon to DrkRecov), as BN6's mark */
#define BN4_NAVI_CHIPS    221         /* the Navi chips: 20 Navis from Roll to VideoMan, three each (his own, SP, DS) */
#define BN4_SOUL_CHIPS    0x160       /* the Souls' pseudo-chips, kind 15: 0x160 + the Soul (1-12), MegaSoul 0x160, DarkSoul 0x16F */
#define BN4_SOUL_KINDS    0x08020008u /* a byte per Soul 1-12: the chip kind it unites by (Roll 4 Recovery .. Wood 3), as BN5_SOUL_KINDS */
#define BN4_SOUL_FLAGS    0x08020018u /* a byte per Soul: its event flag, 0x17-0x1C Red Sun's six, 0xFF Blue Moon's six (refused) */
#define BN4_UNITE_CHECK   0x0801FF84u /* the last pick's kind a Soul's, its flag set, not used this battle (0x0203BF88's bit): BN5's twin */
#define BN4_FLAG_DOUBLE_SOUL 0x0014   /* UNITE stands only with it set and MegaMan neither worried nor dark (0x0801E0B4) */
#define BN4_CHIP_COUNT    389         /* ids 0-388 */
#define BN4_GET_CHIP      0x080190D0u /* getChip: BN6's (0x08021AA4) found by its bytes, its literal BN4_CHIP_RECORDS */
#define BN4_CHIP_NAMES    0x087481D8u /* text archive: the chips' names, ids 0-255 (letters and digits spelt as BN6's) */
#define BN4_CHIP_NAMES_HI 0x08748C14u /* ... ids 256 on */
#define BN4_ENEMY_NAMES   0x0874A780u /* text archive: the enemies' names by id (a Navi's four versions in a row from 128) */
#define BN4_TEXT_END      0xE5        /* ... each entry ending with it (BN6's 0xE6), its letters BN6's codes (Billy 0C 2E 31 31 3E) */
#define BN4_GET_RNG       0x08001B00u /* GetRNG: BN6's (0x0800151C) found by its calls and bytes */
#define BN4_BATTLE_BGS    0x0808563Cu /* its battle backgrounds' records, a pointer each, 27 (BN6 22), read by the loader at 0x08085430 */
#define BN4_BATTLE_BG_COUNT 27        /* the backgrounds, ids 0x00-0x1A (3 and 9 share a record) */
#define BN4_BG_RECORD_SIZE 0x24       /* a background's record: 9 words (BN5's and BN6's 7): tiles, their EWRAM buffer, VRAM; the map, its
                                       * buffer, its place; the palette, its RAM (BN4_BG_PALETTE_RAM) and size */
#define BN4_BG_PALETTE_RAM 0x03002A50u /* the record's +0x1C: where its colours go (BN5's 0x03003960, BN6's 0x03001960) */
#define BN4_BATTLE_BG_ANIMS 0x08085A50u /* per background its animation scripts' list, 0xFFFFFFFF ending it (14 empty), BN6's scripts' header */
#define BN4_BATTLE_BG_SCROLL 0x08085488u /* per background its scroll callbacks, 16 bytes (the second word the motion, as BN6's) */
#define BN4_MAP_BACKDROPS 0x08085BD0u /* per net group (group - 0x80) a pointer to a byte per map: the background of a record whose
                                       * BN4_RECORD_BACKDROP is 0xFF (0x08085BAC): ACDC Area 4, Town Area 5, Park Area 6, the Undernet 8 */
#define BN4_MAP_ENTERS    0x08026634u /* the net's groups' EnterMapGroup routines (21, 0x80-0x94; stubs where a group has no maps), its
                                       * dispatch 0x080266E8 as BN5's; LoadBGAnim the routine's fifth call, not its first as BN5's */
#define BN4_MAP_ANIMS     0x08026688u /* ... their LoadGFXAnims routines (dispatch 0x08026718), called before EnterMapGroup as in BN6 */

#endif

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
#define BN4_EFFECTS       0x080E1698u /* the effects a battle draws (the fourth kind's type 0, 0x080E1854, by the effect id at its +4; its
                                       * literal 0x080E18FC), 4 bytes an id as BN6_EFFECTS': list << 2, sprite, animation, palette */
#define BN4_EFFECT_COUNT  111         /* ... their ids (0-0x6E), its type 0's code after them (BN6's 108) */
#define BN4_SPAWN_EFFECT  0x080E18EEu /* an effect spawned, r4's low byte its id (BN6_SPAWN_EFFECT's twin by body) */
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
#define BN4_SPAWN_EFFECT  0x080E18EEu /* an effect spawned: r4's low byte its id (BN6_SPAWN_EFFECT's twin by body), its row BN4_EFFECTS' */
#define BN4_THUNDER       0x080D2000u /* the third kind's 0x2A, the thunder Billy's attack spawns: its routine (to 0x080D2224), BN6's
                                       * BN6_THUNDER but for its collision offsets and its init's r3 (docs/rom/bn4.md, The thunder's copy) */
#define BN4_THUNDER_ARG   0x080D208Cu /* ... its init's movs r3,#0 before its collision set up (bl 0x080129AC, BN4_COLL_SETUP_ARG) */
#define BN4_SPAWN_THUNDER 0x080D2156u /* ... its spawner, Billy's call (r1 column, r2 row, r3 element): BN6's BN6_SPAWN_THUNDER's copy */
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
#define BN4_COLL_OBJ_2E   0x05        /* ... u8, its object's +0x2E (BN6_COLL_OBJ_2E, +7): the setup 0x080129AC at 0x080129C8, BN6's 0x08019FDA */
#define BN4_COLL_PANEL_BYTE 0x0A      /* ... u8, what 0x0800AF90 gives of its panel, written by the collision panels' update 0x08012D9A at
                                       * 0x08012DAE (BN6_COLL_PANEL_BYTE, +0xC: 0x0801A04C's 0x0800E994 at 0x0801A062) */
#define BN4_COLL_COUNTER  0x0B        /* ... u8, its counter time (its window to be countered), set by 0x0800AFBC at 0x0800AFD6
                                       * (BN6_COLL_COUNTER, +0xD: its twin 0x0800E9DC at 0x0800E9F6) */
/* (the hit pass: BN4's from 0x08012BC0, its twin BN6's from 0x081D7830, store by store: r7 the attacker's collision data, r6 the
 * struck's) */
#define BN4_COLL_HIT_KIND 0x07        /* ... u8 set by 0x08012F74 (BN6_COLL_HIT_KIND, +9: 0x0801A140); the thunder's init gives 3 in both */
#define BN4_COLL_HIT_BITS 0x0D        /* ... u8 the struck's hit bits, the attacker's +0xC ORed in at 0x08012C40-0x08012C46 (BN6_COLL_HIT) */
#define BN4_COLL_STATUS_GIVEN 0x0E    /* ... u8 the status a hit gives, from its object's +6 (0x08013212), passed at 0x08012BEC
                                       * (BN6_COLL_STATUS_GIVEN, +0x10: 0x0801A258, 0x081D784E) */
#define BN4_COLL_STATUS_TAKEN 0x0F    /* ... u8 the struck's, 0x08012BF2 (BN6_COLL_STATUS_TAKEN, +0x11: 0x081D7854) */
#define BN4_COLL_MOOD_LOSS 0x36       /* ... u16 the struck's, the attacker's +5 & 0x7F added at 0x08012C2A (BN6_COLL_MOOD_LOSS, +0x8E) */
#define BN4_COLL_MOOD_GAIN 0x38       /* ... u16 the struck's, at 0x08012C24, 0xFF for a counter (BN6_COLL_MOOD_GAIN, +0x90: 0x8000 for one) */
#define BN4_COLL_HITS_TAKEN 0x3A      /* ... u16 one more a hit of the bit at 0x08012C36 (BN6_COLL_HITS_TAKEN, +0x92) */
#define BN4_COLL_HITS_WORD 0x54       /* ... u32 the struck's, the attacker's alliance word ORed in at 0x08012BE8 (BN6's +0x70) */
#define BN4_COLL_MET      0x4C        /* ... u32 the index bits of those it met, each pair's ORed in at 0x08012AB2-0x08012AC4 (BN6_COLL_MET,
                                       * +0x68: 0x081D7B1E-0x081D7B30) */
#define BN4_COLL_HITS_BACK 0x50       /* ... u32 the other's alliance word ORed in at 0x08012D3C-0x08012D42 (BN6_COLL_HITS_BACK, +0x6C:
                                       * 0x081D7BD2-0x081D7BD8) */
#define BN4_COLL_PUSH_TAKEN 0x48     /* ... u16 the struck's, the attacker's BN4_COLL_PUSH where its low byte is not 0 (0x08012C54; BN6's +0xA4) */
#define BN4_COLL_PUSH     0x74        /* ... u16 a hit's (BN6_COLL_PUSH, +0x12: 0x081D78BA; BN6 sets it by the damage word's bit 13) */
#define BN4_COLL_SETUP_ARG 0x0C    /* ... u8, the setup's r3 (BN6_COLL_SETUP_ARG, +0xE): 0x080129B2, BN6's 0x08019FBA */
#define BN4_COLL_DAMAGE   0x1E        /* ... u16, an attack's damage from its object's +0x2C (BN6_COLL_DAMAGE, +0x2E): 0x080129CC, BN6's 0x08019FDE */
#define BN4_COLL_HIT_MASKS 0x58       /* ... two u32, the alliance words (BN6_COLL_HIT_MASKS, +0x30): 0x080129D6 and 0x080129F2 from
                                       * 0x08012EF6, BN6's 0x08019FE8 and 0x0801A004 from 0x0801A0BA */
#define BN4_COLL_OBJECT   0x60        /* ... u32, its object (BN6_COLL_OBJECT, +0x38): 0x080129B0, BN6's 0x08019FB8 */
#define BN4_COLL_INDEX_BIT 0x78       /* ... u32, its own bit of the 32 (BN6_COLL_INDEX_BIT, +0x44): the pool's maker 0x08012488, BN6's 0x08019884 */
#define BN4_COLL_PARALYSIS 0x10      /* ... u16: paralysis's frames (BN6's +0x1C); confusion +0x12, blindness +0x14, the hold +0x16 */
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
/* Its panel types (BN4_PANEL_TYPE), by the flags each gives (BN4_PANEL_FLAGS_OF) and the code that tests them, each against
 * BN6's (bn6_panels.h; docs/rom/bn4.md, Panel types): 0 none, 1 hole, 2 plain, 3 cracked, 4 poison, 6 grass and 7 ice
 * BN6's numbers and words (ice the generic move's slide, 0x08010328, BN6's 0x08016754); 8 the word of BN6's volcano and
 * BN5's lava (0x11010), which of them unread; the rest below */
#define BN4_SET_PANEL_TYPE 0x08009DC4u /* object_setPanelType (r0 column, r1 row, r2 type): a panel of type 0 refused, its +0x12 given 0xBE */
#define BN4_PANEL_UPDATE_FLAGS 0x0800A376u /* a panel's flags made (BN6's _object_updatePanelParameters): its type's word, side and occupants */
#define BN4_PANEL_FLAGS_OF 0x0800A3A8u /* ... the word each type gives, 12 of them (BN6's BN6_PANEL_FLAGS_OF, 13) */
#define BN4_PANEL_TYPES   12          /* ... how many types (0-0xB) */
#define BN4_PANEL_TYPE5   5           /* its type 5: 0x30010, BN5's sea's word (BN5_PANEL_SEA, its 0xA), no type of BN6's; its code unread */
#define BN4_PANEL_HOLY    9           /* holy: 0x12010, BN6's 5's word (BN6's and BN5's damage-halving bit) */
#define BN4_PANEL_HOLDING 0x0A        /* a panel that holds what stands on it: a non-floating object's move stopped as it arrives (the
                                       * generic move, 0x0801033E) and its can-move check refused while it stands there (0x0800AD42,
                                       * 0x0800AD6C, 0x0800AD96), floating its object flags' 0x20; set by the third kind's type 0x6D
                                       * (0x080DB08A); its word 0x10210 BN6's conveyors' (9-12), which push instead: none in BN6 */
#define BN4_PANEL_PLAIN_B 0x0B        /* its type 0xB: plain's word (0x10010), set by four objects (0x080CB4F4 and others); its meaning unread */
#define BN4_CAN_MOVE      0x0800AD2Au /* an object's can-move check (r5): none of its flags 0x5040, and not on BN4_PANEL_HOLDING unless
                                       * floating; BN6's 0x0800E5E2 the flags test alone (BN6 has no holding panel) */
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
#define BN4_SOUND_CGB_CHANS 0x030046E0u /* its PSG's four channels, SoundInfo +0x1C (BN6's 0x03005600) */
#define BN4_MUSIC_PLAYER  0x02011AA0u /* the music player's MusicPlayerInfo, BN4_MPLAYS' row 31 (BN6's 0x02010890) */
#define BN4_MUSIC_TRACKS  0x02008680u /* ... its eight tracks, 0x50 bytes each (BN6's 0x02007EE8) */
#define BN4_M4A_SOUND_MAIN 0x08111B14u /* SoundMain: BN6's MP2K (0x0814F754) built otherwise, its seven tables BN6's byte for byte */
#define BN4_M4A_SOUND_MODE 0x08000530u /* the word its start gives m4aSoundMode, 0x0093040D as BN6's (0x0800059C) */
#define BN4_M4A_INIT_MODE 0x0811282Cu /* m4aSoundInit's own m4aSoundMode word, 0x0093C400: master volume 12, where BN6's (0x08150458,
                                       * 0x0093F400) sets 15; BN6_SOUND_INFO +0x07 */
#define BN4_DRUM_KIT      0x08153CD4u /* the music's one drum kit: keys 37-73 played, 0-35 a run of other pointers (xsong.c copies the
                                       * keys a song plays: XRomLayout.kit_keys_played) */
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
#define BN4_ENTITY_COL    0x01        /* ... its panel's column, 1-6 (the enemy's 4-6); its row, 1-3, at +2 */
#define BN4_NET_GROUP     0x80        /* the net's first map group: BN4_BATTLE_TABLES' second list and XRomLayout.map_table from it */
#define BN4_STORY_MAPS    16          /* a group's map indices a story stage: the map's number, 16 more each stage */
#define BN4_GROUP_DEEP    0x94        /* the group of the Undernet 1-6 (its maps 0-5) and Black Earth 1-2 (6 and 7), dark green */
#define BN4_MAP_BLACK_EARTH 6         /* ... Black Earth's first: NavShado and NavBlack, its own, in maps 6 and 7's records alone */
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
/* A Soul in battle (docs/rom/bn4.md, Souls in battle; docs/multirom/bn4-souls.md; tools/bn4_souls.py reads each Soul's rows) */
#define BN4_SOULS_USED    0x0203BF88u /* u32: the Souls used this battle, bit n - 1 the Soul n (zeroed with the battle, 0x0801594E) */
#define BN4_UNITE_COMMIT  0x0801F0A0u /* OK's picks loop, a pick of UNITE (its slot under 0x20 the Soul): the used bit (0x0801F1FE), then
                                       * BN4_SOUL_CHIPS + n into the picks, through BN4_PICK_GATE; the hand keeps no kind-15 chip */
#define BN4_PICK_GATE     0x08020306u /* a pick's chip let through or refused (back 0x3784: dropped): 0x1FF passes, the rest by BN4_CHIP_GATE
                                       * and the folder's limits */
#define BN4_CHIP_GATE     0x080063C8u /* a chip's check against the save: BN4_CHIP_KEYS + id XOR 0x64 equal to eToolkit [+0x84] + id, else
                                       * refused; a Soul's pseudo-chip passes only where the game wrote its byte as it gave the Soul */
#define BN4_CHIP_KEYS     0x02000670u /* ... its key bytes, a byte a chip id (fixed) */
#define BN4_TOOLKIT_CHIP_COPY 0x84    /* eToolkit +0x84: the save's chip bytes BN4_CHIP_GATE checks against, in the moving block */
#define BN4_NAVI_FORM     0x24        /* NaviStats +0x24: MegaMan's form, the Soul 1-12 (BN6's and BN5's +0x2C), 0xF DarkSoul */
#define BN4_NAVI_NAVI     0x23        /* NaviStats +0x23: whose NaviStats, 0 MegaMan (the sprite's row, BN4_STATS_SPRITES) */
#define BN4_NAVI_BYTE     0x0800D7CAu /* a battle NaviStats byte of the object in r5 (r1 its offset): BN6's by object */
#define BN4_NAVI_BYTE_SIDE 0x0800D772u /* ... of a side (r0), r1 its offset */
#define BN4_NAVI_BYTE_SET 0x0800D7AAu /* ... written for the object in r5: r1 its offset, r2 the byte */
#define BN4_FORM_DARK     0x0F        /* DarkSoul's form: timed by frames (BN4_FORM_FRAMES), not turns */
#define BN4_FORM_TURN_START 0x0800B658u /* as MegaMan's turn starts (0x08013918): the hand's next chip a Soul's pseudo-chip makes it the pending
                                       * form (AI data +0x10); else a Soul whose turns are spent (+0x18 >= +0x19) ends (pending 0, request
                                       * 0x40), DarkSoul when its frames run out; 0xFF none */
#define BN4_FORM_STATE    0x0800E05Cu /* the form's state set as it changes: its enter routine (BN4_FORM_ENTERS), turns, loadout, charge times,
                                       * element */
#define BN4_TURN_COUNT    0x0800BA20u /* as each Custom screen opens (0x08007192): both sides' AI data +0x18 one more, unless 0xFF */
#define BN4_AI_PENDING_FORM 0x10      /* AI data +0x10: the form the change is to take, 0xFF none (BN4_FORM_TURN_START) */
#define BN4_AI_SOUL_TURNS_USED 0x18   /* AI data +0x18: the Soul's turns counted, 0xFF where UNITE may not stand (a form taken) */
#define BN4_AI_SOUL_TURNS 0x19        /* AI data +0x19: the Soul's turns (BN4_FORM_TURNS); the emotion window shows +0x19 - +0x18 (0x08014B94) */
#define BN4_AI_LOADOUT    0x0C        /* AI data +0xC: the form's loadout, 4 bytes (+0xD its plain shot, +0xE chip charge, +0xF charged shot) */
#define BN4_AI_DARK_FRAMES 0x2C       /* AI data +0x2C: a timed form's frames left (DarkSoul's) */
#define BN4_AI_REQUESTS   0x44        /* AI data +0x44: the requests (0x40 a form's end, 0x20 GutsSoul's rapid buster), BN6's place */
#define BN4_FORM_LOADOUTS 0x080184F0u /* by form (0-15), 4 bytes: +1 the plain shot's power attack, +2 the chip charge's, +3 the charged shot's
                                       * (0xFF none; BN5's 6 bytes at 0x0801CA18) */
#define BN4_FORM_LOADOUT_SIZE 4       /* ... a form's bytes */
#define BN4_FORM_ENTERS   0x0800E0A0u /* by form, a routine as it is taken: eight Souls' (a hand pass, an object, a flag), a nullsub the rest's */
#define BN4_FORM_TURNS    0x080180A8u /* by form, a byte: its turns (3 every Soul's) */
#define BN4_FORM_FRAMES   0x08018088u /* by form, a u16 into AI data +0x2C: a timed form's frames (DarkSoul 960; the Souls' 600 unused) */
#define BN4_FORM_ELEMENTS 0x0800C964u /* by form, a byte: his element (FireSoul 1, ThunSoul 3, AquaSoul 2, WoodSoul 4) */
#define BN4_FORM_COMPANIONS 0x08018068u /* by form, a u16: an object spawned beside him (FireSoul's 0x0010, NumSoul's 0x0110), 0 none */
#define BN4_STATS_SPRITES 0x08017F98u /* by BN4_NAVI_NAVI, his sprite's list and number (2 bytes): MegaMan's list 0, number the form (the
                                       * Souls' sprites list 0's 1-12, BN5's 7-12); 1-13 list 8, the Soul Navis the unison draws */
#define BN4_POWER_ATTACKS 0x0800CA7Cu /* the power attacks by id, 108 (BN5_POWER_ATTACKS' and BN6_POWER_ATTACKS' twin): each sets the attack's
                                       * variables (AI data +0x70) and returns its family, a state of MegaMan's (BN4_PLAYER_STATES) */
#define BN4_POWER_ATTACK_COUNT 108    /* ... how many */
#define BN4_CHARGE_TIMES  0x080180B8u /* a power attack's charge times, 5 halfwords by his Charge (BN6_CHARGE_TIMES' shape; the Souls' the same
                                       * at every Charge: AquaSoul 30 to NumSoul 120) */
#define BN4_FAMILY_UNITE  0x0D        /* MegaMan's state (BN4_PLAYER_STATES) for the unison: its phases, the Soul Navi's image, the form written (0x080EBB80) */
#define BN4_FAMILY_LEAVE  0x11        /* ... for the Soul's end: form 0 written (0x080EBEF2) */
#define BN4_CHIP_ATTACK   0x0800D406u /* a power attack's chip (r0): its numbers from its record, its family the record's +0xB (WindSoul's
                                       * AirShot and WindRack, ProtSoul's WideSwrd, JunkSoul's the battle's chip) */
#define BN4_CHIP_FAMILY   0x0B        /* a chip record's +0xB: its attack family */
#define BN4_CHIP_COUNT    389         /* ids 0-388 */
#define BN4_GET_CHIP      0x080190D0u /* getChip: BN6's (0x08021AA4) found by its bytes, its literal BN4_CHIP_RECORDS */
#define BN4_CHIP_NAMES    0x087481D8u /* text archive: the chips' names, ids 0-255 (letters and digits spelt as BN6's) */
#define BN4_CHIP_NAMES_HI 0x08748C14u /* ... ids 256 on */
#define BN4_ENEMY_NAMES   0x0874A780u /* text archive: the enemies' names by id (a Navi's four versions in a row from 128) */
#define BN4_TEXT_END      0xE5        /* ... each entry ending with it (BN6's 0xE6), its letters BN6's codes (Billy 0C 2E 31 31 3E) */
#define BN4_GET_RNG       0x08001B00u /* GetRNG: BN6's (0x0800151C) found by its calls and bytes */
/* The differential's hooks on BN4's routines (src/dev/xdiff_events.c; docs/rom/bn4.md, The differential's hooks): each
 * BN6's twin (bn6.h) by its body, instruction for instruction but for its literals' places */
#define BN4_RNG_DRAW      BN4_GET_RNG /* GetRNG: the first word stepped, its value in r0 (push {r7,lr}: lr its caller; BN6_RNG_DRAW) */
#define BN4_RNG_DRAW_POSITIVE 0x08001B16u /* GetPositiveSignedRNG: the same, bit 31 cleared (BN6_RNG_DRAW_POSITIVE) */
#define BN4_RNG_DRAW_SECOND 0x08001B30u /* GetRNGSecondary: BN4_RNG_SECONDARY stepped (BN6_RNG_DRAW_SECOND) */
#define BN4_RNG_DRAW_SECOND_POSITIVE 0x08001B46u /* GetPositiveSignedRNGSecondary: the same, bit 31 cleared */
#define BN4_SPAWN_COMMON  0x08003210u /* SpawnBattleObjectCommon: r0 the kind, r1 its spawner's five words (the type first; its caller
                                       * at r1 + 0x18: spawnType1 0x080032B8, spawnType3 0x080032F0, spawnType4 0x08003344 and
                                       * their twins push {r7,lr}; sub sp,#0x14 as BN6's); its kinds' descriptors at 0x08003268 */
#define BN4_SPAWN_COMMON_END 0x08003260u /* ... its end (add sp,#4): r5 the object spawned, 0 none (BN6_SPAWN_COMMON_END) */
#define BN4_PLAY_SOUND    0x08000560u /* PlaySoundEffect: r0 the song (BN6_PLAY_SOUND; BN4_PLAY_MUSIC is the next routine) */
#define BN4_SPAWN_EFFECT  0x080E18EEu /* an effect spawned (the fourth kind's type 0): r4's low byte its id (BN6_SPAWN_EFFECT) */
#define BN4_SPRITE_LOAD   0x0800287Cu /* sprite_load: r5 the object, r1 its sprite list's offset, r2 its number (BN6_SPRITE_LOAD) */
#define BN4_SPRITE_CACHED 0x08002ACAu /* ... its look-up first (0x08002894): r0 the key, list offset << 8 | number; the cache's sprite, 0 none */
#define BN4_SPRITE_PRELOAD 0x08002AECu /* the battle's preload (from 0x08006C9A): each foe's compressed sprite LZ77'd into the cache */
#define BN4_SPRITE_CACHE  0x0200F720u /* the cache: its count (u8 +0), keys, sprites and next free byte (+0x34, each buffer the LZ77 */
#define BN4_SPRITE_CACHE_SIZE 0x38u   /* ... header's size on, below 0x02033000); its bytes */
#define BN4_SPRITE_CACHE_SLOTS 8      /* ... its keys and sprites (BN6's twin holds 12: bn6_navis.h, BN6_SPRITE_DECOMPRESS) */
#define BN4_CACHE_KEYS    0x04        /* ... its keys, a u16 a slot (list offset << 8 | number) */
#define BN4_CACHE_SPRITES 0x14        /* ... its sprites, a word a slot: the buffer + 4, past the LZ77 data's size word */
#define BN4_VIRUS_PRESENT 0x08010736u /* a virus's after-state, called at its own update's tail (Spikey's 0x080BA410, Shrimpy's
                                       * 0x080BA0B8): unless +0 has 0x08, time is stopped, its HP is 0 or its flags or struck word stop it, its
                                       * animation set where +0x10 is not +0x11 (0x08002E10), then BN4_SPRITE_UPDATE (0x08010774); no pause
                                       * test, so a virus animates through the paused opening (BN6_VIRUS_PRESENT's twin skips it there) */
#define BN4_SPIKEY_INIT   0x080BA428u /* Spikey's init (its type 0x17's state 0): its sprite, its collision, then BN4_SPIKEY_MOVES
                                       * (0x080BA46C), run in the battle's paused opening */
#define BN4_SPIKEY_MOVES  0x080BA5BCu /* ... its moves before its attack: 4 + GetPositiveSignedRNG & 3 at its +0x88, from the init and after
                                       * each attack (0x080BA5B0): a draw made before the fight is seeded */
#define BN4_SHOT_KINDS_LIT 0x080CE83Cu /* the literal of the third kind's 0x0B's init (0x080CE70C: ldr r7,=BN4_T3_0B_KINDS): its kind's
                                       * record, kind x 16 from the table, kept at the object's +0x4C (0x080CE710) */
#define BN4_RESERVE_PANEL 0x080143A8u /* object_reservePanel: the panel's reserver (+0x14) and its flags' 0x80 set, no bit in the object's
                                       * +0 (BN6's 0x0801BB1C also sets the object's 0x20, as BN5's does not) */
#define BN4_RESERVE_PANEL_DONE 0x080143C2u /* ... once the panel is reserved: r5 the object (BN6_RESERVE_PANEL_DONE) */
#define BN4_BLINK_FLASHING 0x08010450u /* the blink step of an object flashing: bit 1 of its flash timer (BN4_COLL_FLASH, +0x18) clear
                                        * hides it, as BN5's (BN6's set hides it: BN6_BLINK_FLASHING) */
#define BN4_OBJECT_RUNNER 0x0800314Cu /* RunBattleObjectLogic: each object in the run list by mov lr,pc; bx r0 (BN6's 0x080031AC) */
#define BN4_OBJECT_RUNNER_RET 0x0800319Cu /* ... the return address an object's call chain ends at (BN6_OBJECT_RUNNER_RET) */
#define BN4_SUBSYSTEM_BYTE 0x020015E1u /* u8 that 0x08044AEC returns tested (ldr r0,=0x020015E0; ldrb r0,[r0,#1]; tst r0,r0): Billy's
                                        * code asks it, its recipe aiming the call at BN6's routine that returns 0 (0x080011EC);
                                        * read 0 at every frame of a plain battle (xdiff's BN4 watch, xd4_watch) */
#define BN4_OBJECT_RUNNING 0x0200B48Cu /* u32 the node of the object running, 0 outside the runner (its literal at 0x080031CC, written at
                                        * 0x08003160 and 0x080031A4; BN6_OBJECT_RUNNING) */
#define BN4_BATTLE_BGS    0x0808563Cu /* its battle backgrounds' records, a pointer each, 27 (BN6 22), read by the loader at 0x08085430 */
#define BN4_BATTLE_BG_COUNT 27        /* the backgrounds, ids 0x00-0x1A (3 and 9 share a record) */
#define BN4_BG_RECORD_SIZE 0x24       /* a background's record: 9 words (BN5's and BN6's 7): tiles, their EWRAM buffer, VRAM; the map, its
                                       * buffer, its place; the palette, its RAM (BN4_BG_PALETTE_RAM) and size */
#define BN4_BG_PALETTE_RAM 0x03002A50u /* the record's +0x1C: where its colours go (BN5's 0x03003960, BN6's 0x03001960) */
#define BN4_BATTLE_BG_ANIMS 0x08085A50u /* per background its animation scripts' list, 0xFFFFFFFF ending it (14 empty), BN6's scripts' header */
#define BN4_BATTLE_BG_SCROLL 0x08085488u /* per background its scroll callbacks, 16 bytes (the second word the motion, as BN6's) */
#define BN4_MAP_BACKDROPS 0x08085BD0u /* per net group (group - 0x80) a pointer to a byte per map: the background of a record whose
                                       * BN4_RECORD_BACKDROP is 0xFF (0x08085BAC): ACDC Area 4, Town Area 5, Park Area 6, the Undernet 8 */
#define BN4_GROUP_ACDC    0x90        /* its net's large areas, a map group each, its maps from 0 (docs/rom/bn4.md, Net areas in BN6): ACDC Area 1-3, */
#define BN4_GROUP_TOWN    0x91        /* ... Town Area 1-4, */
#define BN4_GROUP_PARK    0x92        /* ... Park Area 1-3, */
#define BN4_GROUP_COUNTRIES 0x93      /* ... the four countries' areas, a look each, */
#define BN4_GROUP_UNDERNET 0x94       /* ... the Undernet 1-6 and Black Earth 1-2 (maps 6 and 7). Every map's tiles are 256-colour
                                       * (BN6_TILEMAP_COLOURS), their colours in banks 1-12 */
#define BN4_MUGSHOT_FACE  0x61        /* sprite list 8's (XRomLayout.sprite_lists 0x0802793C) Duo: BN4's face on the ROMs screen's cartridge */
#define BN4_MAP_ENTERS    0x08026634u /* the net's groups' EnterMapGroup routines (21, 0x80-0x94; stubs where a group has no maps), its
                                       * dispatch 0x080266E8 as BN5's; LoadBGAnim the routine's fifth call, not its first as BN5's */
#define BN4_MAP_ANIMS     0x08026688u /* ... their LoadGFXAnims routines (dispatch 0x08026718), called before EnterMapGroup as in BN6 */

#include "bn4_navis.h"   /* BN4's Navis: their types, routines and the engine's they share (docs/rom/bn4.md) */
#include "bn4_battle.h"  /* BN4's battle structures past these: its panels' fields, its rewards (docs/rom/bn4.md) */

#endif

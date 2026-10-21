/* Addresses in BN6 Cybeast Gregar (USA), the supported ROM (checked by
 * SHA-1 in rom.c). Names follow the bn6f disassembly; see docs/ROM_DATA.md. */
#ifndef CW_BN6_H
#define CW_BN6_H

/* EWRAM, and the structures in it with the fields the engine reads (bn6f's
 * names in the comments: include/structs/) */
#define BN6_EWRAM             0x02000000u
#define BN6_EWRAM_END         0x02040000u
#define BN6_TOOLKIT           0x020093B0u /* eToolkit: +0 points at the main mode (subsystem) index */
#define BN6_TOOLKIT_CHIPS     0x48        /* eToolkit +0x48: the chips' data (0x02002178): folders of 30 u16 from +0, 0x3C each (bn6f sub_8021AB4) */
#define BN6_TOOLKIT_STEPS     0x40        /* eToolkit S2001c04_Ptr: the encounter roll's walk (bn6f sub_80AA4C0): +0x12 the distance walked since the
                                             last battle (its chance rises with it, a check each 0x40), +0x14 where it last checked; both cleared as a map is entered */
#define BN6_STEPS_WALKED      0x12        /* S2001c04 +0x12: the distance walked since the last battle */
#define BN6_STEPS_CHECKED     0x14        /* S2001c04 +0x14: where the roll last checked */
#define BN6_FOLDER_ENTRIES    30          /* the chips in a folder */
/* each enemy id's reward row in ROM, 20 u16 entries (docs/ROM_DATA.md) */
#define BN6_DROP_ROWS         0x080AC718u
#define BN6_BATTLE_REWARD     0x0200A00Cu /* the reward picked as the battle ended (u16, a row's entry) */
#define BN6_TOOLKIT_KEY_ITEMS 0x50        /* eToolkit KeyItemsPtr: a count per key item */
#define BN6_TOOLKIT_KEY_CHECK 0x78        /* eToolkit Unk2004a8c_Ptr: per key item, its seed ^ 0x55 once given (CheckKeyItem reads 0 where it differs) */
#define BN6_KEY_ITEM_SEEDS    0x020004E0u /* ... the seeds, a byte per key item (Gregar's code 0x006E3C writes the check, 0x006E50 tests it) */
#define BN6_TOOLKIT_SHOP_DATA 0x54        /* eToolkit ShopDataPtr: 8-byte stock entries of every shop */
#define BN6_GAMESTATE         0x02001B80u /* eGameState: +0 the sub-mode (SubsystemIndex: 4 on the map, 8/0xC battle) */
#define BN6_MAP_GROUP         (BN6_GAMESTATE + 0x04) /* MapGroup */
#define BN6_MAP_NUMBER        (BN6_GAMESTATE + 0x05) /* MapNumber: the two read as a u16 are MapId */
#define BN6_MAP_ID            BN6_MAP_GROUP          /* (MapId: MapGroup and MapNumber as a u16) */
#define BN6_LAST_MAP          (BN6_GAMESTATE + 0x0C) /* LastMapGroup and its number, a u16 map id */
#define BN6_MAP_PAUSED        (BN6_GAMESTATE + 0x0A) /* BattlePaused: the map's objects stand still, MegaMan too (none updates without its pause flag) */
#define BN6_NPC_CHAT          (BN6_GAMESTATE + 0x10) /* Unk_10: 1 from A at an NPC till its chat ends (bn6f sub_809DF00, sub_809D7D8): START waits */
#define BN6_SAVED_X           (BN6_GAMESTATE + 0x34) /* SavedRealWorldX: where Lan jacked in, 16.16, which BN6's jack-out returns him to */
#define BN6_SAVED_Y           (BN6_GAMESTATE + 0x38) /* SavedRealWorldY, 16.16 */
#define BN6_SAVED_Z           (BN6_GAMESTATE + 0x3C) /* SavedRealWorldZ, 16.16 */
#define BN6_SAVED_FACING      (BN6_GAMESTATE + 0x40) /* SavedRealWorldFacingDirection: his facing there */
#define BN6_SAVED_MAP         (BN6_GAMESTATE + 0x44) /* SavedRealWorldMapId: its group, then its number */
#define BN6_SONG_PLAYING      (BN6_GAMESTATE + 0x0F) /* BGMusicIndicator */
#define BN6_ZENNY             (BN6_GAMESTATE + 0x5C) /* ProtectedZenny, u32 */
#define BN6_BUGFRAGS          (BN6_GAMESTATE + 0x60) /* ProtectedBugfrags, u32 */
#define BN6_EVENT_FLAGS       0x02001C88u /* eEventFlags: flag n is bit 0x80 >> (n & 7) of byte n / 8 */
#define BN6_EVENT_FLAG_BYTES  1448        /* its size (bn6f ewram.s): flags 0-0x2D3F */
#define BN6_CHATBOX           0x02009CD0u /* eChatbox: +0 Visible */
#define BN6_CHATBOX_STATE     (BN6_CHATBOX + 0x04)  /* TextScriptState_04 */
#define BN6_CHATBOX_OPEN      (BN6_CHATBOX + 0x10)  /* OpenState_10 */
#define BN6_CHATBOX_JUMP      (BN6_CHATBOX + 0x11)  /* JumpTableOffset_11 */
#define BN6_CHATBOX_ARCHIVE   (BN6_CHATBOX + 0x30)  /* TextScriptPtr: the archive its script runs from (a trader's, BN6_TRADER_TEXT) */
#define BN6_CHATBOX_OPTIONS   (BN6_CHATBOX + 0x12)  /* the options on the page so far (bn6f's ts_option counts them; a page clears it): 2 and up, a choice is shown */
#define BN6_CHATBOX_CURSOR    (BN6_CHATBOX + 0x13)  /* the choice's option under the cursor, 0 first (bn6f ChoiceCursorPos) */
#define BN6_CHATBOX_SCRIPT_AT (BN6_CHATBOX + 0x2C)  /* TextScriptCursorPtr: where the script reads */
#define BN6_CHATBOX_BOX_FLAGS (BN6_CHATBOX + 0x3E)  /* flags_3E (0x0100 the box hidden), u16 */
#define BN6_CHATBOX_WORD0     (BN6_CHATBOX + 0x4C)  /* Unk_4C and ... */
#define BN6_CHATBOX_WORD1     (BN6_CHATBOX + 0x50)  /* ... Unk_50: the words a script prints (a trader's prize chip and its code) */
#define BN6_CHATBOX_FLAGS     0x02009F38u /* eFlags2009F38, u32 */
#define BN6_CHAT_ACTIVE       0x80        /* BN6_CHATBOX_FLAGS: a chat under way (bn6f chatbox_mask_eFlags2009F38): START and the encounter roll wait */
#define BN6_PLAYER            0x02009F40u /* the overworld player object (OverworldPlayerObject) */
#define BN6_PLAYER_MAIN       (BN6_PLAYER + 0x08)  /* Unk_08: 0 just spawned, 4 once its update runs (bn6f owPlayer_main) */
#define BN6_PLAYER_STATE      (BN6_PLAYER + 0x09)  /* JumptableIndex_09: 0 stands, 4 walks, 8 runs (the pad read), 0x0C and 0x10 a talk, 0x14 BN6's conveyor */
#define BN6_PLAYER_FACING     (BN6_PLAYER + 0x10)  /* FacingDirection, 0-7 */
#define BN6_PLAYER_ANIM       (BN6_PLAYER + 0x14)  /* AnimationSelect: the facing it is drawn with */
#define BN6_PLAYER_LOCKED     (BN6_PLAYER + 0x17)  /* InteractionLocked */
#define BN6_PLAYER_X          (BN6_PLAYER + 0x1C)  /* X, Y and Z in world units, 16.16 (bn6_player_x and the others read them) */
#define BN6_PLAYER_Y          (BN6_PLAYER + 0x20)
#define BN6_PLAYER_Z          (BN6_PLAYER + 0x24)
#define BN6_PLAYER_NEXT_X     (BN6_PLAYER + 0x28)  /* NextX and NextY: where this frame's step goes */
#define BN6_PLAYER_NEXT_Y     (BN6_PLAYER + 0x2C)
#define BN6_PLAYER_PAD        (BN6_PLAYER + 0x4C)  /* Unk_4c, u16: the pad as BN6 took it this frame, BN6_PAD_STEP a walk or run; 0 while held (bn6f sub_809D9E0) */
#define BN6_PAD_STEP          0x30        /* BN6_PLAYER_PAD: 0x10 a walk, 0x20 a run */
/* The overworld's NPC objects (eOverworldNPCObjects): 16 of 0xD8 bytes,
 * fields from each one's start */
#define BN6_NPC_OBJECTS       0x020057B0u
#define BN6_NPC_SIZE          0xD8
#define BN6_NPC_COUNT         16
#define BN6_NPC_IN_USE        0x00        /* bit 0 in use */
#define BN6_NPC_STATE         0x08        /* CurState */
#define BN6_NPC_RADIUS        0x0C        /* CollisionRadius: 0 none, which the engine sets to let MegaMan through */
#define BN6_NPC_Z_REACH       0x0D        /* ZReach */
#define BN6_NPC_CENTER_X      0x11        /* CenterOffsetX and Y: its collision centre's offsets, s8 */
#define BN6_NPC_CENTER_Y      0x12
#define BN6_NPC_LOCKED        0x17        /* InteractionLocked */
#define BN6_NPC_SCRIPT        0x1C        /* TextScriptIndex */
#define BN6_NPC_X16           0x26        /* X16, Y16, Z16: its position's whole world units (s16) */
#define BN6_NPC_Y16           0x2A
#define BN6_NPC_Z16           0x2E
#define BN6_MUSIC_PLAYER      0x02010890u /* MP2K MusicPlayerInfo of the music (player 31) */
#define BN6_MUSIC_STATUS      (BN6_MUSIC_PLAYER + 4)  /* its status: bit 31 stopped */
#define BN6_MUSIC_STOPPED     (1u << 31)  /* BN6_MUSIC_STATUS: stopped */
#define BN6_BATTLE_RESULT     0x0200A009u /* last battle: 1 won */
#define BN6_BATTLE_STATE      0x02034880u /* the battle's state (docs/ROM_DATA.md): */
#define BN6_BATTLE_SIDE       (BN6_BATTLE_STATE + 0x0D) /* u8: MegaMan's side, his battle NaviStats' (0x08026874 reads it so) */
#define BN6_BATTLE_BUSTING    (BN6_BATTLE_STATE + 0x1E) /* u8: the busting level, 1-10 and 11 for S, which the reward pick takes (bn6f sub_802C8FA; docs/ROM_DATA.md, Battle rewards) */
#define BN6_BATTLE_PHASE      (BN6_BATTLE_STATE + 0x01) /* 8 while the Custom screen is open (and slides out), 0x0C from BATTLE START! on (the pause too) */
#define BN6_PHASE_CUSTOM      0x08        /* BN6_BATTLE_PHASE: the Custom screen open */
#define BN6_CUSTOM_SCREEN     0x020364C0u /* the Custom screen's state: */
#define BN6_CUSTOM_HAND       (BN6_CUSTOM_SCREEN + 0x06) /* the chips it offers, the first of BN6_BATTLE_DECK */
#define BN6_CUSTOM_CURSOR     (BN6_CUSTOM_SCREEN + 0x07) /* the slot under the cursor, 0-4 the top row, BN6_CUSTOM_OK on OK */
#define BN6_CUSTOM_PICKED     (BN6_CUSTOM_SCREEN + 0x08) /* the chips picked */
#define BN6_CUSTOM_OK         10          /* BN6_CUSTOM_CURSOR: on OK */
#define BN6_CUSTOM_EMBLEM     11          /* BN6_CUSTOM_CURSOR: on the Beast Out emblem under OK */
#define BN6_CUSTOM_MODE       (BN6_CUSTOM_SCREEN + 0x01) /* what it shows: BN6_MODE_CHIPS, CROSSSELECT (BN6_MODE_CROSS_*), 0x48 Beast Out chosen */
#define BN6_MODE_CHIPS        0x04        /* BN6_CUSTOM_MODE: the chips */
#define BN6_MODE_CROSS_OPEN   0x4C        /* BN6_CUSTOM_MODE: CROSSSELECT sliding open, ... */
#define BN6_MODE_CROSS_SHUT   0x50        /* ... sliding shut, */
#define BN6_MODE_CROSS        0x54        /* ... open, */
#define BN6_MODE_CROSS_TAKEN  0x5C        /* ... a Cross chosen in it */
#define BN6_CUSTOM_CROSS_ROW  (BN6_CUSTOM_SCREEN + 0x1B) /* CROSSSELECT's cursor: its row, of the Crosses MegaMan has in Gregar's order (BN6_FLAG_HEAT_CROSS on),
                                             less each chosen this battle (BN6_CROSSES_USED) and the one he began it in (0x08029EF6 lists them) */
#define BN6_BATTLE_DECK       0x0203CDB0u /* the battle's folder as it is drawn, u16 chip | code << 9 (26 *); used chips leave it, 0xFFFF after the rest */
#define BN6_PA_STAR_LIMIT     0x08029606u /* bn6f sub_80295C8, a Program Advance of one chip in codes in a row: cmp r2, #1 (0x2A01), one * of its three at most */
#define BN6_PA_STAR_ANY       0x2A03      /* ... cmp r2, #3: three *, the All * helper's (docs/ROM_DATA.md) */
#define BN6_FIELD_PANELS      0x02039AE0u /* the field's panels (bn6f PanelData), 8 a row from column 0, rows 0-4, each: */
#define BN6_PANEL_SIZE        0x20        /* a panel's size */
#define BN6_PANEL_TYPE        0x02        /* Type: 1 a hole, 2 a plain panel, 4 poison, 0x0B and 0x0C a conveyor left and right */
#define BN6_PANEL_ALLIANCE    0x03        /* Alliance: 0 MegaMan's side, 1 the enemies' */
#define BN6_PANEL_FLAGS       0x14        /* Flags, u32: BN6_PANEL_STRUCK while an enemy's attack is on it (BN6 lights it yellow) */
#define BN6_PANEL_HOLE        1           /* BN6_PANEL_TYPE: a hole */
#define BN6_PANEL_PLAIN       2           /* BN6_PANEL_TYPE: a plain panel */
#define BN6_PANEL_POISON      4           /* BN6_PANEL_TYPE: poison */
#define BN6_FIELD_COLUMNS     8           /* BN6_FIELD_PANELS' panels a row (the field's 6 and a border each side) */
#define BN6_PANEL_STRUCK      0x40000000u /* BN6_PANEL_FLAGS: an enemy's attack on the panel */
#define BN6_PANEL_CRACKED     3           /* BN6_PANEL_TYPE: cracked (a step breaks it to a hole: object_crackPanel 0x0800C938 writes it) */
#define BN6_PANEL_HOLY        5           /* BN6_PANEL_TYPE: holy (damage on it halved, 0x0800E3F0) */
#define BN6_PANEL_GRASS       6           /* BN6_PANEL_TYPE: grass (heals a Wood object on it, 0x0801A1D2; a Fire hit burns it plain, 0x03007704) */
#define BN6_PANEL_ICE         7           /* BN6_PANEL_TYPE: ice (an object not of Aqua slides on it, 0x08016754) */
#define BN6_PANEL_FLAGS_OF    0x081D7E20u /* the flags each panel type gives (+0x14), a u32 a type, 13 of them (IWRAM 0x03007920, copied from here) */
#define BN6_FIELDS            0x0800D730u /* the battlefields: 12 bytes each, three u32 rows (rows 1-3), a nibble a column (1-6), the panel's
                                             type (BN6_PANEL_TYPE), read by bn6f sub_800C4BC through sub_800C6D6's literal 0x0800C8D4 */
#define BN6_FIELD_COUNT       237         /* ... its fields: 0-236, up to the code at 0x0800E24C (a BattleSettings' byte 0 names one) */
#define BN6_FIELD_SIZE        12          /* ... a field's bytes */
#define BN6_BATTLE_TIMER      0x020348C0u /* u32, the frames a battle has run (held on the Custom screen and in the pause): the results screen's DeleteTime (docs/ROM_DATA.md) */
#define BN6_T1_OBJECTS        0x0203A9B0u /* eT1BattleObject0: viruses and navis in a battle (BattleObject), */
#define BN6_T1_SIZE           0xD8        /* ... each this long, its fields from its start: */
#define BN6_T1_COUNT          0x20        /* 32 of them */
#define BN6_T1_IN_PLAY        0x00        /* bit 0 in play */
#define BN6_T1_TYPE           0x01        /* its type, the routine its kind's table runs it by (BN6_T1_TYPES, BN6_T3_TYPES, BN6_T4_TYPES): written by
                                             the spawn alone (0x080032BE), read by the runner (0x080031F4) and by the third kind's type 0x10,
                                             which looks for its own among the third kind's objects (0x080C802E) */
#define BN6_T1_SLOT           0x03        /* its slot among its kind's 32, written once as the battle sets the slots up (0x080034BE) */
#define BN6_T1_ACTION         0x09        /* CurAction: MegaMan's BN6_MEGAMAN_READY when he can act, 0x10 a step, 0x12-0x16 a chip */
#define BN6_MEGAMAN_READY     0x08        /* BN6_T1_ACTION: MegaMan's when he can act */
#define BN6_T1_PANEL_X        0x12        /* PanelX and PanelY: its column and row, 0 off the field */
#define BN6_T1_PANEL_Y        0x13
#define BN6_T1_FUTURE_X       0x14        /* FuturePanelX and Y: the panel it is headed for (a thrown attack's landing), 0 none */
#define BN6_T1_FUTURE_Y       0x15
#define BN6_T1_ALLIANCE       0x16        /* Alliance: 0 MegaMan's side, 1 the enemies' */
#define BN6_T1_FLIP           0x17        /* DirectionFlip: its facing turned (BN6's 0x0800E2CA: +1 or -1 a step by it and the alliance) */
#define BN6_T1_HP             0x24        /* HP and MaxHP, u16 */
#define BN6_T1_MAX_HP         0x26
#define BN6_T1_CHIP           0x2A        /* the chip it uses next (u16 id), 0xFFFF none */
#define BN6_T1_NAME_ID        0x28        /* NameID, u16: its name, the viruses' (RomLayout.enemy_names[0]) to 0xFF, the Navis' past it */
#define BN6_T1_ELEMENT        0x0E        /* Element */
#define BN6_T1_COLLISION      0x54        /* CollisionDataPtr: its collision data, in EWRAM (docs/ROM_DATA.md, Statuses as a hit leaves them), its fields BN6_COLL_* */
#define BN6_T1_AI_DATA        0x58        /* AIDataPtr: its AI's data in EWRAM, BN6_AI_DATA_SIZE bytes: +1 its AI index, +0x80 its state, +0xA0 its attack's variables */
#define BN6_AI_DATA_SIZE      0x100       /* an AI's data (BN6_T1_AI_DATA), the same in BN5 */
#define BN6_COLL_SIZE         0xA8        /* a collision data's bytes (BN5_COLL_SIZE: BN5's are 8 fewer) */
#define BN6_COLL_MASKS        0x08019C7Cu /* the words a collision data's +0x30 and +0x34 are set from, by alliance and region (two a region), BN6_COLL_MASK_WORDS of them; */
#define BN6_COLL_MASK_WORDS   178         /* ... BN5_COLL_MASKS' first 160 with bit 0x80 or as they are, but region 61's (tests/xdiff/legit.json) */
#define BN6_COLL_REGION       0x01        /* in its collision data (CollisionDataPtr +0x54, bn6f collision_data_struct), Region: the panels it is struck on and strikes, bn6f PanelOffsetListsPointerTable's index, 0 none (nothing touches it), 1 its own panel */
#define BN6_COLL_PARALYSIS    0x1C        /* u16 frames left paralysed, one less each frame (0x0800E730, the status step every virus runs) */
#define BN6_COLL_CONFUSE      0x1E        /* u16 frames left confused, the same (its mark over it, the fourth kind's 6 of kind 0, at +0x48) */
#define BN6_COLL_BLIND        0x20        /* u16 frames left blinded, the same (its mark, kind 1, at +0x4C) */
#define BN6_COLL_FLASH        0x24        /* u16 frames left flashing after a hit (BN5_COLL_FLASH): the blink step reads its bit 1 (BN6_BLINK_FLASHING) */
#define BN6_COLL_FREEZE       0x2A        /* u16 frames left frozen in an ice block, the same */
#define BN6_COLL_BUBBLE       0x2C        /* u16 frames left in a bubble, the same */
#define BN6_COLL_FLAGS        0x3C        /* u32 its object's flags, ORed in by 0x0801A152 (BN5's 0x08016C42), read by object_getFlag (0x0801A166):
                                             BN6_OBJ_INVULNERABLE among them; poison spares 0x08000028, lava 0x20 and 0x88000206 (bn6_panels.h) */
#define BN6_OBJ_INVULNERABLE  0x08        /* BN6_COLL_FLAGS: no hit takes it (a hit's invulnerable frames set it, 0x0800EAFA; the same bit in BN5) */
#define BN6_COLL_STRUCK       0x40        /* u32 what the hits of a frame left it, acted on by its next (0x0801B1C4, a virus's every frame), each bit cleared there: */
#define BN6_STRUCK_PARALYSIS  0x00000008u /* ... paralysed, BN6_COLL_PARALYSIS frames: CurAction 4 */
#define BN6_STRUCK_SLIDE      0x00000010u /* ... pushed along a panel (it slides; CurAction kept) */
#define BN6_STRUCK_BLIND      0x00000020u /* ... blinded, BN6_COLL_BLIND frames, its mark spawned over it at its arm point 5 (0x0800E920-0x0800E93E) */
#define BN6_STRUCK_CONFUSE    0x00000080u /* ... confused, BN6_COLL_CONFUSE frames, its other statuses' timers cleared, its mark over it
                                             (0x0800E8B0-0x0800E8DE) */
#define BN6_STRUCK_DRAG       0x00000100u /* ... dragged: CurAction 5 */
#define BN6_STRUCK_FREEZE     0x00010000u /* ... frozen, BN6_COLL_FREEZE frames, an ice block spawned round it (BN6_ICE_SIZES): CurAction 6 */
#define BN6_STRUCK_BUBBLE     0x00020000u /* ... bubbled, BN6_COLL_BUBBLE frames: CurAction 7 */
#define BN6_BATTLE_HAND       0x020349C0u /* MegaMan's hand after OK (bn6f getBattleHandAddr_8010018; the enemies' 0x50 on): */
#define BN6_HAND_AT           0x00        /* ... the chip up next, an index */
#define BN6_HAND_CHIPS        0x02        /* ... the chips' ids, u16, six at most, 0xFFFF after them */
#define BN6_HAND_POWERS       0x0E        /* ... their powers, u16 */
#define BN6_HAND_BONUS        0x1A        /* ... the power Atk+ chips add to each, u16 (the HUD's "Sword 80+10") */
#define BN6_HAND_MAX          6           /* ... six chips at most */
#define BN6_CUSTOM_PICKS      0x02036508u /* the Custom screen's chips picked: their slots in its hand, in order (BN6_CUSTOM_PICKED of them) */
#define BN6_BATTLE_NAVI       0x0203CE00u /* eBattleNaviStats0: MegaMan's NaviStats as a battle copies them (bn6f GetBattleNaviStatsAddr), */
#define BN6_BATTLE_NAVI_SIZE  0x64        /* ... BN6_BATTLE_NAVI_SIZE a side (BN5's 0x60, laid out the same to +0x5F) */
#define BN6_BATTLE_MOOD       (BN6_BATTLE_NAVI + 0x0E) /* Mood: BN6_MOOD_SYNCHRO Full Synchro, 0 dark (bn6f's emotion getter, Falzar 0x08015B64) */
#define BN6_MOOD_SYNCHRO      0xFF        /* BN6_BATTLE_MOOD: Full Synchro, the next chip's power x2 (BN6's tutorial) */
#define BN6_BATTLE_BEAST_TURNS (BN6_BATTLE_NAVI + 0x21) /* BeastOutCounter: the EmotionCounter, 3 as a battle starts, one less each Beast Out turn; 0 tired */
#define BN6_BATTLE_FORM       (BN6_BATTLE_NAVI + 0x2C) /* Transformation: 0 none, 1-5 Gregar's Crosses (BN6_FLAG_HEAT_CROSS's order), BN6_FORM_BEAST, a Cross's Beast Out, BN6_FORM_BEAST_OVER */
#define BN6_FORM_BEAST        11          /* BN6_BATTLE_FORM: Beast Out; 12 + a Cross: Beast Out in that Cross */
#define BN6_FORM_CROSS_BEAST  12          /* BN6_BATTLE_FORM: this + the Cross (1-5), Beast Out in a Cross */
#define BN6_FORM_BEAST_OVER   23          /* BN6_BATTLE_FORM: BeastOver, the Cybeast's power out of hand */
#define BN6_T3_OBJECTS        0x0203CFE0u /* eT3BattleObject0: the battle's attacks and effects, BN6_T1_SIZE each, its fields as T1's */
#define BN6_T3_COUNT          0x20        /* 32 of them */
#define BN6_T4_OBJECTS        0x02036870u /* the battle's fourth kind of object (effects: a muzzle's blast, an ice block; the kind's record at 0x08003310), */
#define BN6_T4_SIZE           0xC8        /* ... each this long, its fields as T1's to +0x5C, */
#define BN6_T4_COUNT          0x20        /* ... 32 of them */
#define BN6_OBJECT_NODE       0x10        /* the list node before each battle object (+4 the next node; its kinds' sizes count it) */
#define BN6_OBJECT_LIST       0x02009380u /* the battle's objects in the order they run each frame: a head whose +4 is the first node, a node the 16 bytes before */
#define BN6_OBJECT_LIST_END   0x02009AB0u /* ... its object (+4 the next), to this one, the list's end */
#define BN6_RNG_PRIMARY       0x020013F0u /* u32, the random numbers' first word (BN5_RNG_PRIMARY's): its step at 0x0800151C, run once a frame (0x080050FC) */
#define BN6_RNG_SECONDARY     0x02001120u /* u32, the second (BN5_RNG_SECONDARY's): its step at 0x0800154C, run twice a frame (0x08005100, 0x0800031C) */
/* The battle engine's routines the differential hooks on both cores (src/dev/xdiff_events.c; BN5's in bn5.h), each a draw, a call or a step it logs with its caller */
#define BN6_RNG_DRAW          0x0800151Cu /* GetRNG: the first word stepped, its value in r0 (push {r7,lr} first: lr its caller) */
#define BN6_RNG_DRAW_POSITIVE 0x08001532u /* GetPositiveSignedRNG: the same, bit 31 cleared */
#define BN6_RNG_DRAW_SECOND   0x0800154Cu /* GetRNGSecondary: the second word stepped */
#define BN6_RNG_DRAW_SECOND_POSITIVE 0x08001562u /* GetPositiveSignedRNGSecondary: the same, bit 31 cleared */
#define BN6_SPAWN_COMMON      0x08003278u /* SpawnBattleObjectCommon: r0 the kind (1, 3, 4), r1 its spawner's five words (the type first; its caller at r1 + 0x18) */
#define BN6_SPAWN_COMMON_END  0x080032C8u /* ... its end (add sp,#4): r5 the object spawned, 0 none */
#define BN6_PLAY_SOUND        0x080005CCu /* PlaySoundEffect: r0 the song */
#define BN6_SPAWN_EFFECT      0x080E1932u /* an effect spawned (the fourth kind's type 0): r4's low byte the effect's id */
#define BN6_SPRITE_LOAD       0x080026E4u /* sprite_load: r5 the object whose sprite state (past what its spawn clears) is set up anew */
#define BN6_SET_ATTACK        0x08011698u /* setAttack's common part (setAttack0-5 at 0x08011680 + 4N branch to it): r0 the CurAction, r2 N, r5 the object */
#define BN6_RESERVE_PANEL_DONE 0x0801BB3Eu /* object_reservePanel (0x0801BB1C) once the panel is reserved and BN6_OBJ_RESERVED set: r5 the object */
#define BN6_OBJ_RESERVED      0x20        /* a battle object's +0: set as it first reserves a panel (0x0801BB36), never cleared while it lives (BN5 has no such bit) */
#define BN6_BLINK_FLASHING    0x08016954u /* the blink step of an object flashing (its flags 0x202): bit 1 of its collision data's flash timer (+0x24) set hides it */
#define BN6_OBJECT_RUNNER_RET 0x08003200u /* RunBattleObjectLogic (0x080031AC) calls each object by mov lr,pc; bx r0: the return address an object's call chain ends at */
#define BN6_OBJECT_RUNNING    0x0200AF70u /* u32 the node of the object running (its object 0x10 on), 0 outside the runner (0x080031C2, 0x0800320A) */
#define BN6_PHASE_FIGHT       0x0C        /* BN6_BATTLE_PHASE: fighting (from BATTLE START! on, the pause too) */
#define BN6_PHASE_RESULTS     0x10        /* BN6_BATTLE_PHASE: over, its results (the reward picked from the enemies' drop rows) */
#define BN6_WINDOW_OPEN       0x78        /* BN6_CUSTOM_WINDOW: open */
#define BN6_MPLAYS            0x08159DC8u /* the sound's music players (MP2K), BN6_MPLAY_COUNT of 12 bytes: +0 its MusicPlayerInfo in EWRAM (+0 the song's */
#define BN6_MPLAY_COUNT       32          /* ... header, +0xC its clock, 0 as a song starts); a song table entry names its player (RomLayout.song_table) */
#define BN6_NAVI_STATS        0x020047CCu /* eNaviStats0: MegaMan's (NaviStats) */
#define BN6_NAVI_HP           (BN6_NAVI_STATS + 0x40) /* CurHP and MaxHP, u16 */
#define BN6_NAVI_MAX_HP       (BN6_NAVI_STATS + 0x42)
#define BN6_NAVI_BASE_MAX_HP  (BN6_NAVI_STATS + 0x3E) /* MaxBaseHP, u16: HPMemory counts into it, programs on top (bn6f NaviStats) */
#define BN6_NAVI_REG          (BN6_NAVI_STATS + 0x09) /* RegUP: Reg memory in MB, made again from the RegUP items as one is given */
#define BN6_NAVI_MEGA_LEVEL   (BN6_NAVI_STATS + 0x0B) /* MegaLevel and GigaLevel: the Megas and Gigas a folder may hold ("You can use only N MegaChips.", bn6f CompText86CF1A8) */
#define BN6_NAVI_GIGA_LEVEL   (BN6_NAVI_STATS + 0x0C) /* GigaLevel: the Gigas a folder may hold */
#define BN6_NAVI_FOLDER1_REG  (BN6_NAVI_STATS + 0x2E) /* Folder1Reg: the first folder's Regular chip, its entry; 0xFF none */
#define BN6_NAVI_FOLDER1_TAG  (BN6_NAVI_STATS + 0x56) /* Folder1Tag1 and Tag2: its TagChips, their entries; 0xFF none */
#define BN6_NAVI_FOLDER1_TAG2 (BN6_NAVI_STATS + 0x57) /* Folder1Tag2: its second TagChip's entry; 0xFF none */
#define BN6_NAVI_ATTACK       (BN6_NAVI_STATS + 0x01) /* Attack, Speed and Charge: the buster's levels less one (0-4), as the NaviCust's RUN ... */
#define BN6_NAVI_SPEED        (BN6_NAVI_STATS + 0x02) /* ... makes them (bn6f applyNavicustPrograms_813C684: Attack+1 to ChargMAX, BustPack, ... */
#define BN6_NAVI_CHARGE       (BN6_NAVI_STATS + 0x03) /* ... each capped at 4); a battle reads its copy's (a shot's damage Attack + 1, sub_801265A) */
#define BN6_NAVICUST_BUGS     0x0200431Cu /* the NaviCust's bug counts, one byte per type 0-15 (docs/NAVICUST.md) */
#define BN6_NAVICUST_PLACED   0x02004190u /* the programs on the NaviCust's board, 8 bytes each: +0 u16 program * 4 + colour variant, +3 column, +4 row, +5 turns; 0 ends */
#define BN6_NAVICUST_PLACED_MAX 25        /* the most programs the board holds */
#define BN6_NAVICUST_SLOTS    49          /* the list's room, as the game's compile walks it (holes where a program was taken off) */
#define BN6_NAVICUST_GRID     0x0200414Cu /* the board as a 7x7 grid, a byte a cell: its placed program's index + 1, 0 none (bn6f sub_813B9B4; docs/NAVICUST.md) */
#define BN6_PROGRAM_ITEMS     0x90        /* key item 0x90 + program * 4 + variant: how many of it MegaMan has, on the board or not */
#define BN6_WARP              0x02011BB0u /* Warp2011bb0: the next map's warp data */
#define BN6_WARP_PENDING      (BN6_WARP + 0x10)    /* Unk_10: 1 while a trigger's warp is under way */
#define BN6_WARP_INDEX        (BN6_WARP + 0x11)    /* WarpIndex */
#define BN6_WARP_GROUP_KIND   (BN6_WARP + 0x12)    /* MapGroupTransitionType: 1 internet to real world, to BN6_SAVED_X's place */
#define BN6_CUTSCENE          0x02011C50u /* CutsceneState */
#define BN6_CUTSCENE_POS      (BN6_CUTSCENE + 0x1C) /* CutsceneScriptPos */
#define BN6_CUTSCENE_POS0     (BN6_CUTSCENE + 0x40) /* originalCutsceneScriptPos_40 */
#define BN6_SCREEN_FADE       0x0200A440u /* eScreenFade, the map's (bn6f SetScreenFade's first): +0 1 while its routine runs (a fade out keeps the dark so) */
#define BN6_FADE_ACTIVE       (BN6_SCREEN_FADE + 0x03) /* 1 while it fades (bn6f IsScreenFadeActive): the pad, START and EnterMap wait for it */

/* A map's tile map decompresses to 0x02013A00 (12-byte header, then the entries) and its coordinate
 * data to 0x02027A00, which the game reads in place: the raw entries may take up to here, not a byte more. */
#define BN6_TILEMAP_MAX       0x13FF4
#define BN6_BG_PALETTE        0x03001960u /* IWRAM: the game's BG palette buffer */

/* Event flags */
#define BN6_FLAG_NO_PET_SAVE  0x1706      /* EVENT_PET_COMM_SAVE_DISABLED: the PET's Comm and Save buzz */
#define BN6_FLAG_PET_OFF      0x1707      /* EVENT_PET_DISABLED: START opens nothing (EnterMap clears it) */
#define BN6_FLAG_CONVEYOR     0x1717      /* EVENT_1717_PLAYER_ADVANCE_FORWARD: BN6's conveyor carries MegaMan (map triggers 0x48-0x4F), the pad and START off */
#define BN6_FLAG_COMPRESSED   0x2660      /* + a NaviCust program's variant: compressed by its code (docs/ROM_DATA.md, NaviCust) */
#define BN6_FLAG_NO_JACK      0x1727      /* R neither jacks in nor out (the jack routine's first check) */
#define BN6_FLAG_NAVI_IN_PET  0x171D      /* EVENT_PET_NAVI_ACTIVE: MegaMan is in the PET; clear, a jack-in says "MegaMan isn't in the PET..." (bn6f CompText87385CC), and only BN6's own jack-out sets it again */
#define BN6_FLAG_WARP_OFF     0x16F0      /* + n: the map's warp trigger n does nothing */
#define BN6_FLAG_NO_ENCOUNTERS 0x1700     /* checkThenStartBattle skips random battles while set (a BBS request sets it); cleared on entering a map */
#define BN6_FLAG_NAVICUST     0x00F2      /* MegaMan's STATUS in the PET offers the NaviCust (found by setting flags there) */
#define BN6_FLAG_BEAST_OUT    0x00E0      /* Beast Out in the Custom screen (unless 0x163 is set) */
#define BN6_FLAG_LIBRARY      0x1E20      /* + chip id: the chip is in the Library (docs/ROM_DATA.md) */
#define BN6_TOOLKIT_CHIP_MARKS 0x7C       /* eToolkit: a byte per chip id, the chip's key XOR BN6_CHIP_KEY_XOR where owned */
#define BN6_TOOLKIT_PACK      0x4C        /* eToolkit: the pack, 12 bytes a chip id: a count (99 at most) per code of its record's four */
#define BN6_CHIP_KEYS         0x020008A0u /* a key byte per chip id */
#define BN6_CHIP_KEY_XOR      0x17        /* (Gregar; Falzar 0x81) */
#define BN6_FLAG_HEAT_CROSS   0x00E2      /* CROSSSELECT entries, Gregar's five: HeatCross */
#define BN6_FLAG_ELEC_CROSS   0x00E3      /* ElecCross */
#define BN6_FLAG_SLASH_CROSS  0x00E4      /* SlashCross */
#define BN6_FLAG_ERASE_CROSS  0x00E5      /* EraseCross */
#define BN6_FLAG_CHARGE_CROSS 0x00E6      /* ChargeCross */

/* Per internet group tables (index group - 0x80) */
#define BN6_NPC_LISTS         0x080347E0u /* NPCList_maps80: per-map lists of NPC script pointers */
#define BN6_MAP_SCRIPTS       0x08034670u /* (on enter, continuous) per-map map script lists */
#define BN6_ENTER_GROUP       0x0803093Cu /* EnterMap_InternetMapGroupJumptable: per-group map loaders */
#define BN6_OBJ_SPAWNERS      0x0803483Cu /* InternetSpawnMapObjectJumptable: per-group spawn routines */
#define BN6_SONG_NAVI_CUSTOMIZER 0x1E      /* the NaviCust screen's theme (bn6f's SONG_NAVI_CUSTOMIZER, played at 0x08137590): the Net Square's music */
#define BN6_MAP_MUSIC         0x080360E4u /* per chapter (GameState+7): (group, per-map song bytes) entries of 8 bytes, ending 0xFF */
#define BN6_MAP_MUSIC_LISTS   3           /* its lists, one a chapter */
#define BN6_MYSTERY_DATA      0x080A484Cu /* InternetMysteryDataMapGroupEntries: (group, per-map lists), ends with 1 */
#define BN6_MYSTERY_PICKS     0x02004348u /* per flag 0x1400+n: chosen placement and content */

/* Shops */
#define BN6_CHAT_FONT         0x086AACACu /* the chat box's font: 16x12 4bpp glyphs, 0x60 bytes per character code */
#define BN6_CHAT_FONT_WIDTHS  0x08043C74u /* ... its widths in pixels, a byte per code */
#define BN6_TALK_PROBES       0x0809F164u /* the player's 8 facing probes (bn6f byte_809DC2C): s32 x, y, z offsets (16.16), u8 radius, u8 z reach, flags; 24 bytes each */
#define BN6_TALK_PROBE_SIZE   24          /* a probe's size */
#define BN6_TALK_PROBE_Y      4           /* its y offset (s32, 16.16) */
#define BN6_TALK_PROBE_RADIUS 12          /* its radius (u8) */
#define BN6_DIALOGUE_LOCK     0x0200ACE0u /* eStruct200ace0 +0: 1 while a non-NPC dialogue holds the player (no talking to NPCs) */
#define BN6_SCRIPTED_WALK     (BN6_DIALOGUE_LOCK + 0x15) /* fixOWPlayerAnim_15: MegaMan walks by a cutscene's pad, not the player's (bn6f owPlayer_809E0FC; owPlayer_809E114 ends it) */
#define BN6_FLAG_PLAYER_CAN_MOVE 0x1714  /* EVENT_PLAYER_CAN_MOVE */
#define BN6_FLAG_DIALOGUE_1718   0x1718  /* set by the game's non-NPC dialogue lock */
#define BN6_FLAG_DIALOGUE_1719   0x1719  /* cleared by its unlock */
#define BN6_PET_MAP_NAMES_PTR 0x08120E9Cu /* the PET's PLACE line's literal: the same archive */
#define BN6_MAP_NAMES_PTR     0x08033F34u /* RenderMapName's literal: the map-name label's archive (TextScriptMapNames, 244 names of 12) */
#define BN6_SHOP_DESCS        0x08046B68u /* per shop: currency (0 zenny, 1 BugFrags, 2 Chip Order), text, data offset, entries */
#define BN6_VENDOR_FACE       0x087F046Au /* shop 3's keeper text (LZ77 at 0x087F0420): the literal face byte of its first F5 00, which its other six copy (0x42) */
#define BN6_SHOP_INIT         0x08047D70u /* the shop data a new game copies to ShopDataPtr */

/* Chip Traders (src/layer/trader.c) */
#define BN6_TRADER_TEXT       0x086C3758u /* TextScriptChipTrader: 0 Chip Trader, 6 Special, 12 BugFrag Trader */
#define BN6_TRADER_POOLS      0x0804CDE0u /* 5 x (map key, chip list, rarity weights): prizes by the trader's map */
#define BN6_TRADER_KINDS      0x0809B7E4u /* 6 x (map key, u8 trader script): which lines the trade screen shows */
#define BN6_TRADER_MODES      0x0804BDCCu /* 2 weights: the prize one the Library has (192) or a new one (64) */
/* the BugFrag Trader's trade, which BN6's trader machine makes on the
 * Undernet's map (bn6f sub_809A078; director_folder.c bugfrag_trade) */
#define BN6_TRADER_STATE      0x0200AC80u /* eS200AC80 */
#define BN6_TRADER_STATE_PRIZE (BN6_TRADER_STATE + 0x04) /* the prize's chip, then its code (u16 each) */
#define BN6_TRADER_STATE_30   (BN6_TRADER_STATE + 0x30) /* u16 the machine clears with the prize after a trade */
#define BN6_TRADER_RESET      0x0804B0ADu /* bn6f sub_804A2E8: clears the trade's state and the submenu's */
#define BN6_TRADER_PRIZE      0x0804CAC5u /* bn6f sub_804BD00: a prize from the map's pool, r0 its chip, r1 its code */
#define BN6_GIVE_CHIPS        0x08021AEFu /* GiveChips (chip, code, count) */
#define BN6_TAKE_BUGFRAGS     0x0803D09Du /* TakeBugfrags (count) */
#define BN6_SAVE_GAME         0x0803F76Du /* bn6f sub_803F798: the game saved to the cartridge, as after every trade */
#define BN6_FLAG_TRADER_HOWL  0xF6        /* EVENT_F6: the machine's howl, set by the trade's script; the machine clears it */

/* ROM code */
#define BN6_AWAIT_FRAME_LOOP  0x080003A6u /* bn6f main_awaitFrame (0x080003A0): its loop polling DISPSTAT for VBlank, ldrh r1,[r0] */
#define BN6_ENTER_MAP_ON_WARP 0x08005C05u /* map_triggerEnterMapOnWarp (Thumb) */
/* Map, flag and key item events, by hook (src/director/events.c; Gregar's as Falzar's where not said) */
#define BN6_ENTER_MAP         0x08005152u /* bn6f EnterMap (game state 0x00, 0x08005148) past its wait for the fade: once a map is entered, the map flags 0x1640-0x16FF cleared just after */
#define BN6_SET_EVENT_FLAG    0x0802F114u /* SetEventFlag: r0 the flag (SetEventFlagFromImmediate, 0x0802F110, falls into it); the chat's EA 00 command calls it */
#define BN6_GIVE_ITEM         0x0803CD6Cu /* GiveItem (Falzar 0x0803CD98): r0 the key item, r1 how many */
#define BN6_CHAT_RUN_SCRIPT   0x08040359u /* chatbox_runScript (archive, script index) */
#define BN6_GIVE_BUGFRAGS     0x0803D055u /* GiveBugfrags (count): the protected count and its checks, capped at 9999 (--talk bugfrags) */
#define BN6_GIVE_ZENNY        0x0803CFCDu /* GiveZenny (amount): ProtectedZenny and its checks, capped at 999999 (Falzar 0x0803CFF9) */
#define BN6_WARP_DEPART_JACK_OUT 0x080059B5u /* warp departure 8: the jack-out cutscene, then warp */
#define BN6_OW_HOOK           0x080050ECu /* cbGameState_80050EC, run every frame of the game mode: a hook runs the engine's calls in its place */
/* Random battles (src/director/encounter.c): bn6f checkThenStartBattle
 * (0x08005A8C, Gregar's as Falzar's) tests MegaMan on the map, then the
 * flags, fades and chat that hold a battle, calls the roll (0x080ABD30)
 * and branches on the flags it leaves; then StartBattle */
#define BN6_ENCOUNTER_CHECK   0x08005A98u /* its first test after the map's */
#define BN6_ENCOUNTER_ROLLED  0x08005AE2u /* the beq after the roll's bl: r0 the roll's BattleSettings* */
#define BN6_ENCOUNTER_START   0x08005AE5u /* movs r1,#1; bl StartBattle, r0 the record (Thumb) */
#define BN6_ENCOUNTER_SKIP    0x08005AF3u /* the check's pop {r5,pc}: its battle not begun (Thumb) */
/* Battles, by hook (src/director/encounter.c, src/emu/protoman_cross.c; Gregar's as Falzar's where not said) */
#define BN6_START_BATTLE      0x08005BC8u /* StartBattle: r0 the BattleSettings*, every battle's */
#define BN6_SPAWN_HP          0x08007740u /* in an enemy's spawn (its entry sub_800768C branches to the body bn6f sub_80076A0 holds): strh r2,[r5,#0x24], r2 its HP and MaxHP, r5 its BattleObject */
#define BN6_SUBTRACT_HP       0x0800E2D8u /* object_subtractHP: r5 the BattleObject, r0 the damage (every object's, every frame, mostly 0) */
#define BN6_REWARD_PICK       0x080AC180u /* bn6f sub_80AA910 (Falzar + 0x1870), as a battle ends: r0 the enemies' u16 ids, r1 their count */
#define BN6_PROTOMAN_CROSS_END 0x080FDFA0u /* ProtoMan landing from his cross (bn6f sub_80FCC4C + 0x2C; issue #55), his collision data just restored: strh r0,[r2,#0x2E], r2 his collision data */

/* BN6's own DarkChips (docs/ROM_DATA.md, BN6's own DarkChips; docs/META.md;
 * issue #70): the routines every chip use runs, the battle's BugFrags
 * (Gregar's as Falzar's); a chip record's fields are bn6_chips.h's */
#define BN6_DARK_FIRST_ID     0x11E       /* DrkSword, DarkThnd, DrkRecov, DarkInvs, DarkPlus: chips 0x11E-0x122 */
#define BN6_DARK_AFTER        0x0800B79Au /* bn6f sub_800B79A, a chip's after-effects as it runs: r0 the chip that ran (a DarkChip's id only where
                                             its dark power ran, a BugFrag paid), r5 its user; the NaviCust's HP bug its table at 0x0800B7BC adds */
#define BN6_DARK_CHECK        0x08010D58u /* bn6f sub_8010D58, every chip use: r0 its record's DarkChipID (0xFF for most), r5 its user; with a BugFrag
                                             in the battle's count it spends one (0x0800F4B2), with none the base chip runs */
#define BN6_DARK_BASES        0x08010D98u /* ... the base chips' routines, 10 bytes each, by DarkChipID: push {lr}; movs r0,#chip (Sword 0x47, Thunder
                                             0x1E, Recov10 0x9A, Invisibl 0xB1, Atk+10 0xC0) */
#define BN6_DARK_BASE_STEP    10          /* the bytes from one base chip's routine to the next in BN6_DARK_BASES */
#define BN6_BATTLE_BUGFRAGS   0x0203F7E0u /* the battle's BugFrags, u32 a side (+ alliance x 4), the save's copied as it begins (bn6f dword_203F7E0) */
#define BN6_RECOVERY_AMOUNTS  0x080EDBB0u /* u16 by a recovery chip's subfamily: 10, 30 .. 300, 1000 (DrkRecov's, the ninth) */
#define BN6_BBS_ARCHIVES      0x0813FE2Cu /* the BBS's text archives (bn6f off_813E04C), 12 words; the fourth, LZ77 (Falzar's CompText87E9578), */
#define BN6_BBS_DARK_ARCHIVE  3           /* the fourth of BN6_BBS_ARCHIVES, the DarkChip post's */
#define BN6_BBS_DARK_SCRIPT   27          /* ... its script 27 a post on DarkChips, "A dar...DarkChip!?" */
#define BN6_FLAME_SPRITE      0x3C        /* sprite list 7's blue flame (uncompressed, 1612 bytes), which no map object lists */
#define BN6_FLAME_SLOT        0x54        /* ... and a list-7 number Gregar leaves on its placeholder sprite, the purple copy's */

/* Main modes (main_subsystemJumpTable) and game-state sub-modes */
#define BN6_MODE_START_SCREEN 0x00   /* BN6_TOOLKIT: the start screen (the main mode, the index eToolkit +0 points at) */
#define BN6_MODE_GAME         0x04   /* BN6_TOOLKIT: the game */
#define BN6_MODE_GAME_OVER    0x14   /* BN6_TOOLKIT: its GAME OVER */
#define BN6_MODE_SUBMENU      0x28   /* BN6_TOOLKIT: a PET screen (bn6f SubMenuControl), which BN6_SUBMENU's first byte names */
#define BN6_MODE_SHOP         0x2C   /* BN6_TOOLKIT: a shop (bn6f ShopControl) */
/* (a shop's screen, its state and list: bn6_shop.h) */
#define BN6_MODE_TRADER       0x34   /* BN6_TOOLKIT: the Chip Trader (bn6f ChipTraderControl) */
#define BN6_MODE_MAIL         0x48   /* BN6_TOOLKIT: E-Mail on its own (bn6f HandleEmailMenu81279F8) */
#define BN6_SUB_ENTER         0x00   /* BN6_GAMESTATE: a map being entered (bn6f EnterMap, which waits out a fade first) */
#define BN6_SUB_MAP           0x04   /* BN6_GAMESTATE: on the map */
#define BN6_SUB_BATTLE_INIT   0x08   /* BN6_GAMESTATE: a battle beginning */
#define BN6_SUB_BATTLE        0x0C   /* BN6_GAMESTATE: in battle */
#define BN6_SUB_PET           0x18   /* BN6_GAMESTATE: the PET's menu, opened by START on the map (bn6f sub_8005AF4) */
#define BN6_CUSTOM_GAUGE      0x020352A0u /* u16, the Custom gauge: full at 0x4000 (bn6f SetCustGauge, eStruct2035280 + 0x20) */
#define BN6_CUSTOM_WINDOW     0x02035292u /* eStruct2035280 + 0x12: the Custom screen's window as it slides in, 0 (closed, or hidden by SELECT to see the field) to 0x78 open */

#include "bn6_attacks.h"   /* the attack families, power attacks and charge times, as the engine grows them (issue #82) */
#include "bn6_chips.h"   /* the chip tables grown and moved, the pack's extension (issue #82), BN6's sorts (issue #124) */
#include "bn6_families.h"   /* the battle engine's tables a virus family is found through (issue #82) */
#include "bn6_link.h"   /* the link battle's keys into a player, the Practice NetBattle's list (epic #167) */
#include "bn6_forms.h"   /* MegaMan's forms: the tables by form, the code that reads it, a Cross chosen (issue #82) */
#include "bn6_navis.h"   /* the battle engine's Navis: their tables by AI index, the routines they name, their arm points (issue #82) */
#include "bn6_pet.h"   /* the PET: its menu, its screens' state, KeyItem's and E-Mail's words */
#include "bn6_shop.h"   /* a shop's screen: its state and its list (issue #124) */
#include "bn6_panels.h"   /* the battlefield's panels as the engine updates, draws and acts on them (issue #82) */
#endif

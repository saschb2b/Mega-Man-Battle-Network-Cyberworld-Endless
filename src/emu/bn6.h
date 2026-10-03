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
#define BN6_FOLDER_ENTRIES    30
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
#define BN6_SONG_PLAYING      (BN6_GAMESTATE + 0x0F) /* BGMusicIndicator */
#define BN6_ZENNY             (BN6_GAMESTATE + 0x5C) /* ProtectedZenny, u32 */
#define BN6_BUGFRAGS          (BN6_GAMESTATE + 0x60) /* ProtectedBugfrags, u32 */
#define BN6_EVENT_FLAGS       0x02001C88u /* eEventFlags: flag n is bit 0x80 >> (n & 7) of byte n / 8 */
#define BN6_EVENT_FLAG_BYTES  1448        /* its size (bn6f ewram.s): flags 0-0x2D3F */
#define BN6_CHATBOX           0x02009CD0u /* eChatbox: +0 Visible */
#define BN6_CHATBOX_STATE     (BN6_CHATBOX + 0x04)  /* TextScriptState_04 */
#define BN6_CHATBOX_OPEN      (BN6_CHATBOX + 0x10)  /* OpenState_10 */
#define BN6_CHATBOX_JUMP      (BN6_CHATBOX + 0x11)  /* JumpTableOffset_11 */
#define BN6_CHATBOX_OPTIONS   (BN6_CHATBOX + 0x12)  /* the options on the page so far (bn6f's ts_option counts them; a page clears it): 2 and up, a choice is shown */
#define BN6_CHATBOX_CURSOR    (BN6_CHATBOX + 0x13)  /* the choice's option under the cursor, 0 first (bn6f ChoiceCursorPos) */
#define BN6_CHATBOX_SCRIPT_AT (BN6_CHATBOX + 0x2C)  /* TextScriptCursorPtr: where the script reads */
#define BN6_CHATBOX_ARCHIVE   (BN6_CHATBOX + 0x30)  /* TextScriptPtr: the archive it runs */
#define BN6_CHATBOX_BOX_FLAGS (BN6_CHATBOX + 0x3E)  /* flags_3E (0x0100 the box hidden), u16 */
#define BN6_CHATBOX_WORD0     (BN6_CHATBOX + 0x4C)  /* Unk_4C and ... */
#define BN6_CHATBOX_WORD1     (BN6_CHATBOX + 0x50)  /* ... Unk_50: the words a script prints (a trader's prize chip and its code) */
#define BN6_CHATBOX_FLAGS     0x02009F38u /* eFlags2009F38 */
#define BN6_PLAYER            0x02009F40u /* the overworld player object (OverworldPlayerObject) */
#define BN6_PLAYER_STATE      (BN6_PLAYER + 0x09)  /* JumptableIndex_09 */
#define BN6_PLAYER_FACING     (BN6_PLAYER + 0x10)  /* FacingDirection, 0-7 */
#define BN6_PLAYER_ANIM       (BN6_PLAYER + 0x14)  /* AnimationSelect: the facing it is drawn with */
#define BN6_PLAYER_LOCKED     (BN6_PLAYER + 0x17)  /* InteractionLocked */
#define BN6_PLAYER_X          (BN6_PLAYER + 0x1C)  /* X, Y and Z in world units, 16.16 (bn6_player_x and the others read them) */
#define BN6_PLAYER_Y          (BN6_PLAYER + 0x20)
#define BN6_PLAYER_Z          (BN6_PLAYER + 0x24)
#define BN6_PLAYER_NEXT_X     (BN6_PLAYER + 0x28)  /* NextX and NextY: where this frame's step goes */
#define BN6_PLAYER_NEXT_Y     (BN6_PLAYER + 0x2C)
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
#define BN6_BATTLE_RESULT     0x0200A009u /* last battle: 1 won */
#define BN6_BATTLE_STATE      0x02034880u /* the battle's state (docs/ROM_DATA.md): */
#define BN6_BATTLE_PHASE      (BN6_BATTLE_STATE + 0x01) /* 8 while the Custom screen is open (and slides out), 0x0C from BATTLE START! on (the pause too) */
#define BN6_PHASE_CUSTOM      0x08
#define BN6_CUSTOM_SCREEN     0x020364C0u /* the Custom screen's state: */
#define BN6_CUSTOM_HAND       (BN6_CUSTOM_SCREEN + 0x06) /* the chips it offers, the first of BN6_BATTLE_DECK */
#define BN6_CUSTOM_CURSOR     (BN6_CUSTOM_SCREEN + 0x07) /* the slot under the cursor, 0-4 the top row, BN6_CUSTOM_OK on OK */
#define BN6_CUSTOM_PICKED     (BN6_CUSTOM_SCREEN + 0x08) /* the chips picked */
#define BN6_CUSTOM_OK         10
#define BN6_BATTLE_DECK       0x0203CDB0u /* the battle's folder as it is drawn, u16 chip | code << 9 (26 *); used chips leave it, 0xFFFF after the rest */
#define BN6_FIELD_PANELS      0x02039AE0u /* the field's panels (bn6f PanelData), 8 a row from column 0, rows 0-4, each: */
#define BN6_PANEL_SIZE        0x20
#define BN6_PANEL_TYPE        0x02        /* Type: 1 a hole, 2 a plain panel, 4 poison, 0x0B and 0x0C a conveyor left and right */
#define BN6_PANEL_ALLIANCE    0x03        /* Alliance: 0 MegaMan's side, 1 the enemies' */
#define BN6_PANEL_FLAGS       0x14        /* Flags, u32: BN6_PANEL_STRUCK while an enemy's attack is on it (BN6 lights it yellow) */
#define BN6_PANEL_HOLE        1
#define BN6_PANEL_PLAIN       2
#define BN6_PANEL_POISON      4
#define BN6_PANEL_STRUCK      0x40000000u
#define BN6_BATTLE_TIMER      0x020348C0u /* u32, the frames a battle has run (held on the Custom screen and in the pause): the results screen's DeleteTime (docs/ROM_DATA.md) */
#define BN6_T1_OBJECTS        0x0203A9B0u /* eT1BattleObject0: viruses and navis in a battle (BattleObject), */
#define BN6_T1_SIZE           0xD8        /* ... each this long, its fields from its start: */
#define BN6_T1_COUNT          0x20
#define BN6_T1_IN_PLAY        0x00        /* bit 0 in play */
#define BN6_T1_ACTION         0x09        /* CurAction: MegaMan's BN6_MEGAMAN_READY when he can act, 0x10 a step, 0x12-0x16 a chip */
#define BN6_MEGAMAN_READY     0x08
#define BN6_T1_PANEL_X        0x12        /* PanelX and PanelY: its column and row, 0 off the field */
#define BN6_T1_PANEL_Y        0x13
#define BN6_T1_FUTURE_X       0x14        /* FuturePanelX and Y: the panel it is headed for (a thrown attack's landing), 0 none */
#define BN6_T1_FUTURE_Y       0x15
#define BN6_T1_ALLIANCE       0x16        /* Alliance: 0 MegaMan's side, 1 the enemies' */
#define BN6_T1_HP             0x24        /* HP and MaxHP, u16 */
#define BN6_T1_MAX_HP         0x26
#define BN6_T1_CHIP           0x2A        /* the chip it uses next (u16 id), 0xFFFF none */
#define BN6_T3_OBJECTS        0x0203CFE0u /* eT3BattleObject0: the battle's attacks and effects, BN6_T1_SIZE each, its fields as T1's */
#define BN6_T3_COUNT          0x20
#define BN6_NAVI_STATS        0x020047CCu /* eNaviStats0: MegaMan's (NaviStats) */
#define BN6_NAVI_HP           (BN6_NAVI_STATS + 0x40) /* CurHP and MaxHP, u16 */
#define BN6_NAVI_MAX_HP       (BN6_NAVI_STATS + 0x42)
#define BN6_NAVI_REG          (BN6_NAVI_STATS + 0x09) /* RegUP: Reg memory in MB, made again from the RegUP items as one is given */
#define BN6_NAVICUST_BUGS     0x0200431Cu /* the NaviCust's bug counts, one byte per type 0-15 (docs/NAVICUST.md) */
#define BN6_NAVICUST_PLACED   0x02004190u /* the programs on the NaviCust's board, 8 bytes each: +0 u16 program * 4 + colour variant, +3 column, +4 row, +5 turns; 0 ends */
#define BN6_NAVICUST_PLACED_MAX 25
#define BN6_NAVICUST_SLOTS    49          /* the list's room, as the game's compile walks it (holes where a program was taken off) */
#define BN6_NAVICUST_GRID     0x0200414Cu /* the board as a 7x7 grid, a byte a cell: its placed program's index + 1, 0 none (bn6f sub_813B9B4; docs/NAVICUST.md) */
#define BN6_PROGRAM_ITEMS     0x90        /* key item 0x90 + program * 4 + variant: how many of it MegaMan has, on the board or not */
#define BN6_WARP              0x02011BB0u /* Warp2011bb0: the next map's warp data */
#define BN6_WARP_PENDING      (BN6_WARP + 0x10)    /* Unk_10: 1 while a trigger's warp is under way */
#define BN6_WARP_INDEX        (BN6_WARP + 0x11)    /* WarpIndex */
#define BN6_WARP_GROUP_KIND   (BN6_WARP + 0x12)    /* MapGroupTransitionType */
#define BN6_CUTSCENE          0x02011C50u /* CutsceneState */
#define BN6_CUTSCENE_POS      (BN6_CUTSCENE + 0x1C) /* CutsceneScriptPos */
#define BN6_CUTSCENE_POS0     (BN6_CUTSCENE + 0x40) /* originalCutsceneScriptPos_40 */

/* A map's tile map decompresses to 0x02013A00 (12-byte header, then the
 * entries) and its coordinate data to 0x02027A00, which the game reads in
 * place: the raw entries may take up to here, not a byte more. */
#define BN6_TILEMAP_MAX       0x13FF4

/* IWRAM */
#define BN6_BG_PALETTE        0x03001960u /* the game's BG palette buffer */

/* Event flags */
#define BN6_FLAG_NO_PET_SAVE  0x1706      /* EVENT_PET_COMM_SAVE_DISABLED: the PET's Comm and Save buzz */
#define BN6_FLAG_COMPRESSED   0x2660      /* + a NaviCust program's variant: compressed by its code (docs/ROM_DATA.md, NaviCust) */
/* The PET menu (docs/ROM_DATA.md, the PET): ePETMenuData, +0 its state, +4
 * the cursor (6 Comm, 7 Save), +5 bit 0 open, +0xF the entry the engine's
 * hook on its input took (7 Save); the state table's pointer to its input
 * handler, and that handler; the grey's store to Save's colour */
#define BN6_PET_MENU          0x0200DF20u
#define BN6_PET_CURSOR        (BN6_PET_MENU + 0x04)
#define BN6_PET_MENU_OPEN     (BN6_PET_MENU + 0x05)
#define BN6_PET_HOLD          (BN6_PET_MENU + 0x09) /* a count that holds its input off */
#define BN6_PET_MENU_TAKEN    (BN6_PET_MENU + 0x0F)
#define BN6_KEYS_PRESSED      0x0200A272u           /* eJoypad +2: the keys just pressed (u16, the GBA's bits) */
#define BN6_PET_INPUT_PTR     0x08120B1Cu
#define BN6_PET_INPUT         0x08120B91u
#define BN6_PET_GREY_SAVE     0x08120F26u
/* The KeyItem screen's words (docs/ROM_DATA.md, the PET): the names'
 * archive (uncompressed, an entry per id) and the literals that point at
 * it (the KeyItem screen's, the text scripts', the shop's, SubChip's); the
 * descriptions' (LZ77) and the literal the screen runs them through */
#define BN6_KEY_NAMES         0x0873B938u
#define BN6_KEY_NAMES_PTRS    { 0x08042034u, 0x0804761Cu, 0x081265FCu, 0x0812A870u }
#define BN6_KEY_DESCS         0x0873BD88u
#define BN6_KEY_DESC_PTR      0x0812A9C4u
/* The E-Mail screen's (docs/ROM_DATA.md, the PET): senders (script 2n) and
 * subjects (2n + 1), and bodies (n), both LZ77, each read through a literal
 * holding its unpacked buffer; a row of 4 bytes a mail (its icon, bit 7 Lan's
 * HP, its places in the sorts); the list of mail ids, newest first, and its
 * count; the flags per mail: received, unread, read */
#define BN6_MAIL_TEXT         0x086D10D8u
#define BN6_MAIL_TEXT_PTR     0x08129E90u
#define BN6_MAIL_TEXT_BUF     0x02025700u
#define BN6_MAIL_BODIES       0x086CE598u
#define BN6_MAIL_BODY_PTR     0x0812A12Cu
#define BN6_MAIL_BODY_BUF     0x0201C700u
#define BN6_MAIL_TABLE        0x0812A2F8u
#define BN6_MAIL_LIST         0x02006530u
#define BN6_MAIL_COUNT        0x02001140u
#define BN6_FLAG_MAIL_GOT     0x1CA0
#define BN6_FLAG_MAIL_NEW     0x1D20
#define BN6_FLAG_MAIL_READ    0x1DA0
#define BN6_FLAG_NO_JACK      0x1727      /* R neither jacks in nor out (the jack routine's first check) */
#define BN6_FLAG_WARP_OFF     0x16F0      /* + n: the map's warp trigger n does nothing */
#define BN6_FLAG_NO_ENCOUNTERS 0x1700     /* checkThenStartBattle skips random battles while set (a BBS request sets it); cleared on entering a map */

#define BN6_FLAG_NAVICUST     0x00F2      /* MegaMan's STATUS in the PET offers the NaviCust (found by setting flags there) */
#define BN6_FLAG_BEAST_OUT    0x00E0      /* Beast Out in the Custom screen (unless 0x163 is set) */
#define BN6_FLAG_LIBRARY      0x1E20      /* + chip id: the chip is in the Library (docs/ROM_DATA.md) */
#define BN6_TOOLKIT_CHIP_MARKS 0x7C       /* eToolkit: a byte per chip id, the chip's key XOR BN6_CHIP_KEY_XOR where owned */
#define BN6_TOOLKIT_PACK      0x4C        /* eToolkit: the pack, 12 bytes a chip id: a count (99 at most) per code of its record's four */
#define BN6_CHIP_KEYS         0x020008A0u /* a key byte per chip id */
#define BN6_CHIP_KEY_XOR      0x17        /* (Gregar; Falzar 0x81) */
#define BN6_FLAG_HEAT_CROSS   0x00E2      /* CROSSSELECT entries, Gregar's five */
#define BN6_FLAG_ELEC_CROSS   0x00E3
#define BN6_FLAG_SLASH_CROSS  0x00E4
#define BN6_FLAG_ERASE_CROSS  0x00E5
#define BN6_FLAG_CHARGE_CROSS 0x00E6

/* Per internet group tables (index group - 0x80) */
#define BN6_NPC_LISTS         0x080347E0u /* NPCList_maps80: per-map lists of NPC script pointers */
#define BN6_MAP_SCRIPTS       0x08034670u /* (on enter, continuous) per-map map script lists */
#define BN6_ENTER_GROUP       0x0803093Cu /* EnterMap_InternetMapGroupJumptable: per-group map loaders */
#define BN6_OBJ_SPAWNERS      0x0803483Cu /* InternetSpawnMapObjectJumptable: per-group spawn routines */
#define BN6_MAP_MUSIC         0x080360E4u /* per chapter (GameState+7): (group, per-map song bytes) entries of 8 bytes, ending 0xFF */
#define BN6_MAP_MUSIC_LISTS   3
#define BN6_MYSTERY_DATA      0x080A484Cu /* InternetMysteryDataMapGroupEntries: (group, per-map lists), ends with 1 */
#define BN6_MYSTERY_PICKS     0x02004348u /* per flag 0x1400+n: chosen placement and content */

/* Shops */
#define BN6_CHAT_FONT         0x086AACACu /* the chat box's font: 16x12 4bpp glyphs, 0x60 bytes per character code */
#define BN6_CHAT_FONT_WIDTHS  0x08043C74u /* ... its widths in pixels, a byte per code */
#define BN6_TALK_PROBES       0x0809F164u /* the player's 8 facing probes (bn6f byte_809DC2C): s32 x, y, z offsets (16.16), u8 radius, u8 z reach, flags; 24 bytes each */
#define BN6_TALK_PROBE_SIZE   24
#define BN6_TALK_PROBE_Y      4
#define BN6_TALK_PROBE_RADIUS 12
#define BN6_DIALOGUE_LOCK     0x0200ACE0u /* eStruct200ace0 +0: 1 while a non-NPC dialogue holds the player (no talking to NPCs) */
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
 * Undernet's map (bn6f sub_809A078; director.c bugfrag_trade) */
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
/* Map, flag and key item events, by hook (src/director/events.c; Gregar's
 * as Falzar's where not said) */
#define BN6_ENTER_MAP         0x08005152u /* bn6f EnterMap (game state 0x00, 0x08005148) past its wait for the fade: once a map is entered, the map flags 0x1640-0x16FF cleared just after */
#define BN6_SET_EVENT_FLAG    0x0802F114u /* SetEventFlag: r0 the flag (SetEventFlagFromImmediate, 0x0802F110, falls into it); the chat's EA 00 command calls it */
#define BN6_GIVE_ITEM         0x0803CD6Cu /* GiveItem (Falzar 0x0803CD98): r0 the key item, r1 how many */
#define BN6_CHAT_RUN_SCRIPT   0x08040359u /* chatbox_runScript (archive, script index) */
#define BN6_GIVE_BUGFRAGS     0x0803D055u /* GiveBugfrags (count): the protected count and its checks, capped at 9999 (--talk bugfrags) */
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
/* Battles, by hook (src/director/encounter.c; Gregar's as Falzar's where
 * not said) */
#define BN6_START_BATTLE      0x08005BC8u /* StartBattle: r0 the BattleSettings*, every battle's */
#define BN6_SPAWN_HP          0x08007740u /* in bn6f sub_800768C, an enemy's spawn: strh r2,[r5,#0x24], r2 its HP and MaxHP, r5 its BattleObject */
#define BN6_SUBTRACT_HP       0x0800E2D8u /* object_subtractHP: r5 the BattleObject, r0 the damage (every object's, every frame, mostly 0) */
#define BN6_REWARD_PICK       0x080AC180u /* bn6f sub_80AA910 (Falzar + 0x1870), as a battle ends: r0 the enemies' u16 ids, r1 their count */


/* Main modes (main_subsystemJumpTable) and game-state sub-modes */
#define BN6_MODE_START_SCREEN 0x00
#define BN6_MODE_GAME         0x04
#define BN6_MODE_GAME_OVER    0x14
#define BN6_SUB_MAP           0x04
#define BN6_SUB_BATTLE_INIT   0x08
#define BN6_SUB_BATTLE        0x0C
#define BN6_SUB_PET           0x18   /* the PET's menu, opened by START on the map (bn6f sub_8005AF4) */
#define BN6_CUSTOM_GAUGE      0x020352A0u /* u16, the Custom gauge: full at 0x4000 (bn6f SetCustGauge, eStruct2035280 + 0x20) */
#define BN6_CUSTOM_WINDOW     0x02035292u /* eStruct2035280 + 0x12: the Custom screen's window as it slides in, 0 (closed, or hidden by SELECT to see the field) to 0x78 open */

#endif

/* Addresses in BN6 Cybeast Gregar (USA), the supported ROM (checked by
 * SHA-1 in rom.c). Names follow the bn6f disassembly; see docs/ROM_DATA.md. */
#ifndef CW_BN6_H
#define CW_BN6_H

/* EWRAM */
#define BN6_TOOLKIT           0x020093B0u /* eToolkit: +0 points at the main mode (subsystem) index */
#define BN6_TOOLKIT_CHIPS     0x48        /* eToolkit +0x48: the chips' data (0x02002178): folders of 30 u16 from +0, 0x3C each (bn6f sub_8021AB4) */
#define BN6_FOLDER_ENTRIES    30
#define BN6_TOOLKIT_BATTLE    0x18        /* eToolkit BattleStatePtr: +0x3C its BattleSettings* */
/* ... +0x09 the enemies spawned as the battle began, +0x54 their ids (u16);
 * each id's reward row in ROM, 20 u16 entries (docs/ROM_DATA.md) */
#define BN6_BATTLE_ENEMY_COUNT 0x09
#define BN6_BATTLE_ENEMY_IDS  0x54
#define BN6_DROP_ROWS         0x080AC718u
#define BN6_BATTLE_REWARD     0x0200A00Cu /* the reward picked as the battle ended (u16, a row's entry) */
#define BN6_TOOLKIT_KEY_ITEMS 0x50        /* eToolkit KeyItemsPtr: a count per key item */
#define BN6_TOOLKIT_KEY_CHECK 0x78        /* eToolkit Unk2004a8c_Ptr: per key item, its seed ^ 0x55 once given (CheckKeyItem reads 0 where it differs) */
#define BN6_KEY_ITEM_SEEDS    0x020004E0u /* ... the seeds, a byte per key item (Gregar's code 0x006E3C writes the check, 0x006E50 tests it) */
#define BN6_TOOLKIT_SHOP_DATA 0x54        /* eToolkit ShopDataPtr: 8-byte stock entries of every shop */
#define BN6_GAMESTATE         0x02001B80u /* eGameState: +0 sub-mode (4 on the map, 8/0xC battle), +4 map group, +5 map number, +0xF song playing (BGMusicIndicator) */
#define BN6_EVENT_FLAGS       0x02001C88u /* eEventFlags: flag n is bit 0x80 >> (n & 7) of byte n / 8 */
#define BN6_CHATBOX           0x02009CD0u /* eChatbox: +0 Visible, +4 script state */
#define BN6_CHATBOX_FLAGS     0x02009F38u /* eFlags2009F38 */
#define BN6_PLAYER            0x02009F40u /* overworld player object: +0x1C X, +0x20 Y (16.16) */
#define BN6_MUSIC_PLAYER      0x02010890u /* MP2K MusicPlayerInfo of the music (player 31): +4 status, bit 31 stopped */
#define BN6_BATTLE_RESULT     0x0200A009u /* last battle: 1 won */
#define BN6_T1_OBJECTS        0x0203A9B0u /* eT1BattleObject0: viruses and navis in a battle */
#define BN6_T1_SIZE           0xD8        /* ... each: flag bit 0 in play, +0x16 alliance (0 MegaMan), +0x24 HP, +0x26 max HP */
#define BN6_T1_COUNT          0x20
#define BN6_NAVI_STATS        0x020047CCu /* eNaviStats0: MegaMan's, +0x40 HP, +0x42 max HP */
#define BN6_NAVICUST_BUGS     0x0200431Cu /* the NaviCust's bug counts, one byte per type 0-15 (docs/NAVICUST.md) */
#define BN6_NAVICUST_PLACED   0x02004190u /* the programs on the NaviCust's board, 8 bytes each: +0 u16 program * 4 + colour variant, +3 column, +4 row, +5 turns; 0 ends */
#define BN6_NAVICUST_PLACED_MAX 25
#define BN6_PROGRAM_ITEMS     0x90        /* key item 0x90 + program * 4 + variant: how many of it MegaMan has, on the board or not */
#define BN6_WARP              0x02011BB0u /* Warp2011bb0: the next map's warp data; +0x10 1 while a trigger's warp is under way, +0x11 its warp index */
#define BN6_CUTSCENE          0x02011C50u /* CutsceneState: +0x1C script pos, +0x40 original pos */

#define BN6_ENGINE_MARK       0x0203FFF0u /* past everything the game uses: the engine's stubs signal here */
/* A map's tile map decompresses to 0x02013A00 (12-byte header, then the
 * entries) and its coordinate data to 0x02027A00, which the game reads in
 * place: the raw entries may take up to here, not a byte more. */
#define BN6_TILEMAP_MAX       0x13FF4

/* Event flags */
#define BN6_FLAG_NO_PET_SAVE  0x1706      /* EVENT_PET_COMM_SAVE_DISABLED: the PET's Comm and Save buzz */
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
#define BN6_DIALOGUE_LOCK     0x0200ACE0u /* eStruct200ace0 +0: 1 while a non-NPC dialogue holds the player (no talking to NPCs) */
#define BN6_FLAG_PLAYER_CAN_MOVE 0x1714  /* EVENT_PLAYER_CAN_MOVE */
#define BN6_FLAG_DIALOGUE_1718   0x1718  /* set by the game's non-NPC dialogue lock */
#define BN6_FLAG_DIALOGUE_1719   0x1719  /* cleared by its unlock */
#define BN6_PET_MAP_NAMES_PTR 0x08120E9Cu /* the PET's PLACE line's literal: the same archive */
#define BN6_MAP_NAMES_PTR     0x08033F34u /* RenderMapName's literal: the map-name label's archive (TextScriptMapNames, 244 names of 12) */
#define BN6_SHOP_DESCS        0x08046B68u /* per shop: currency (0 zenny, 1 BugFrags, 2 Chip Order), text, data offset, entries */
#define BN6_SHOP_INIT         0x08047D70u /* the shop data a new game copies to ShopDataPtr */

/* Chip Traders (src/layer/trader.c) */
#define BN6_TRADER_TEXT       0x086C3758u /* TextScriptChipTrader: 0 Chip Trader, 6 Special, 12 BugFrag Trader */
#define BN6_TRADER_POOLS      0x0804CDE0u /* 5 x (map key, chip list, rarity weights): prizes by the trader's map */
#define BN6_TRADER_KINDS      0x0809B7E4u /* 6 x (map key, u8 trader script): which lines the trade screen shows */
#define BN6_TRADER_MODES      0x0804BDCCu /* 2 weights: the prize one the Library has (192) or a new one (64) */

/* ROM code */
#define BN6_ENTER_MAP_ON_WARP 0x08005C05u /* map_triggerEnterMapOnWarp (Thumb) */
#define BN6_CHAT_RUN_SCRIPT   0x08040359u /* chatbox_runScript (archive, script index) */
#define BN6_WARP_DEPART_JACK_OUT 0x080059B5u /* warp departure 8: the jack-out cutscene, then warp */
#define BN6_OW_HOOK           0x080050ECu /* cbGameState_80050EC, run every frame of the game mode: the engine borrows it for a frame */


/* Main modes (main_subsystemJumpTable) and game-state sub-modes */
#define BN6_MODE_START_SCREEN 0x00
#define BN6_MODE_GAME         0x04
#define BN6_MODE_GAME_OVER    0x14
#define BN6_SUB_MAP           0x04
#define BN6_SUB_BATTLE_INIT   0x08
#define BN6_SUB_BATTLE        0x0C
#define BN6_CUSTOM_GAUGE      0x020352A0u /* u16, the Custom gauge: full at 0x4000 (bn6f SetCustGauge, eStruct2035280 + 0x20) */

#endif

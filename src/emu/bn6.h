/* Addresses in BN6 Cybeast Gregar (USA), the supported ROM (checked by
 * SHA-1 in rom.c). Names follow the bn6f disassembly; see docs/ROM_DATA.md. */
#ifndef CW_BN6_H
#define CW_BN6_H

/* EWRAM */
#define BN6_TOOLKIT           0x020093B0u /* eToolkit: +0 points at the main mode (subsystem) index */
#define BN6_TOOLKIT_KEY_ITEMS 0x50        /* eToolkit KeyItemsPtr: a count per key item */
#define BN6_TOOLKIT_SHOP_DATA 0x54        /* eToolkit ShopDataPtr: 8-byte stock entries of every shop */
#define BN6_GAMESTATE         0x02001B80u /* eGameState: +0 sub-mode (4 on the map, 8/0xC battle), +4 map group, +5 map number, +0xF song playing (BGMusicIndicator) */
#define BN6_EVENT_FLAGS       0x02001C88u /* eEventFlags: flag n is bit 0x80 >> (n & 7) of byte n / 8 */
#define BN6_CHATBOX           0x02009CD0u /* eChatbox: +0 Visible, +4 script state */
#define BN6_CHATBOX_FLAGS     0x02009F38u /* eFlags2009F38 */
#define BN6_PLAYER            0x02009F40u /* overworld player object: +0x1C X, +0x20 Y (16.16) */
#define BN6_MUSIC_PLAYER      0x02010890u /* MP2K MusicPlayerInfo of the music (player 31): +4 status, bit 31 stopped */
#define BN6_BATTLE_RESULT     0x0200A009u /* last battle: 1 won */
#define BN6_WARP              0x02011BB0u /* Warp2011bb0: the next map's warp data; +0x10 1 while a trigger's warp is under way, +0x11 its warp index */
#define BN6_CUTSCENE          0x02011C50u /* CutsceneState: +0x1C script pos, +0x40 original pos */

#define BN6_ENGINE_MARK       0x0203FFF0u /* past everything the game uses: the engine's stubs signal here */

/* Event flags */
#define BN6_FLAG_NO_PET_SAVE  0x1706      /* EVENT_PET_COMM_SAVE_DISABLED: the PET's Comm and Save buzz */
#define BN6_FLAG_NO_JACK      0x1727      /* R neither jacks in nor out (the jack routine's first check) */
#define BN6_FLAG_WARP_OFF     0x16F0      /* + n: the map's warp trigger n does nothing */

#define BN6_FLAG_BEAST_OUT    0x00E0      /* Beast Out in the Custom screen (unless 0x163 is set) */
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
#define BN6_SHOP_DESCS        0x08046B68u /* per shop: currency (0 zenny, 1 BugFrags, 2 Chip Order), text, data offset, entries */
#define BN6_SHOP_INIT         0x08047D70u /* the shop data a new game copies to ShopDataPtr */

/* ROM code */
#define BN6_ENTER_MAP_ON_WARP 0x08005C05u /* map_triggerEnterMapOnWarp (Thumb) */
#define BN6_OW_HOOK           0x080050ECu /* cbGameState_80050EC, run every frame of the game mode: the engine borrows it for a frame */


/* Main modes (main_subsystemJumpTable) and game-state sub-modes */
#define BN6_MODE_START_SCREEN 0x00
#define BN6_MODE_GAME         0x04
#define BN6_MODE_GAME_OVER    0x14
#define BN6_SUB_MAP           0x04
#define BN6_SUB_BATTLE_INIT   0x08
#define BN6_SUB_BATTLE        0x0C

#endif

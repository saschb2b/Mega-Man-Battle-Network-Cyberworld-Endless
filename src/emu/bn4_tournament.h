/* Battle Network 4: Red Sun (USA)'s tournaments as its own code presents them: the main modes, the bracket screen and
 * what drives it, the tournaments' records in RAM, the cutscene and text commands that open them (docs/rom/bn4.md,
 * The tournaments; docs/multirom/bn4-tournament.md, the screens and the two ways in). Found by code reading with
 * the Thumb decoder and shown in romlab: the bracket of each tournament drawn by BN4 from the entrants poked in.
 * Apart from bn4.h, which includes it. */
#ifndef CW_BN4_TOURNAMENT_H
#define CW_BN4_TOURNAMENT_H

/* The main modes: eToolkit [+0]'s byte indexes this table (the main loop, 0x080002F2), 16 modes 0x00-0x3C; the map's
 * own sub-states (the game state's +0, BN4_MAP_STATES) fade the map out and enter each of 0x0C and 0x1C-0x34 */
#define BN4_MAIN_MODES    0x0800032Cu /* the main loop's mode routines (BN6's main_subsystemJumpTable's twin) */
#define BN4_MAP_STATES    0x08004BB0u /* the map mode's (0x08004B90) routines by the game state's +0: 0x10 the jack-in (mode 0x0C),
                                       * 0x14 the PET (0x1C), 0x18 the city map (0x20), 0x1C the bracket (0x24), 0x28 the Chip Trader
                                       * (0x30), 0x2C the Free Tournament's bracket (0x34): each seen in romlab from Lan's room */
#define BN4_MODE_BRACKET  0x24        /* the bracket screen's main mode (BN4_BRACKET_SCREEN) */
#define BN4_MODE_BRACKET_FREE 0x34    /* ... the Free Tournament's (0x080450AC: BN4_BRACKET_SCREEN with BN4_SUBSYSTEM_BYTE's struct
                                       * 0x020015E0; that it is the link's tournament is assumed) */
#define BN4_GAMESTATE_TO_BRACKET 0x1C /* the game state's +0 that fades the map out and enters BN4_MODE_BRACKET (0x08004FEE: the map's
                                       * objects freed, BN4_BRACKET_SETUP, the mode written) */
#define BN4_GAMESTATE_TO_FREE 0x2C    /* ... and BN4_MODE_BRACKET_FREE (0x080051C8) */

/* The bracket screen */
#define BN4_OPEN_BRACKET  0x08005792u /* r0 the tournament, r1 1 where a script waits on it, r2 the script's argument: its entered byte
                                       * (BN4_TOURNAMENT_ENTERED) set, the three into BN4_BRACKET, MegaMan's place kept in the game
                                       * state (+0x38 to +0x44), its +0 made BN4_GAMESTATE_TO_BRACKET */
#define BN4_OPEN_FREE_BRACKET 0x080057C8u /* ... the Free Tournament's: where 0x020014F0's list holds an entrant, tournament 3 opened
                                       * through BN4_GAMESTATE_TO_FREE */
#define BN4_BRACKET_SETUP 0x080438F8u /* as the map leaves: BN4_BRACKET's first 0x2C bytes cleared but its +5 to +7, the
                                       * tournament's marks cleared (0x08043ED0), BN4_NEXT_OPPONENT */
#define BN4_BRACKET_SCREEN 0x0804395Cu /* BN4_MODE_BRACKET's routine: BN4_BRACKET_STATES by BN4_BRACKET's +0 */
#define BN4_BRACKET_STATES 0x08043970u /* 0 its start (0x08043980: palette, tiles, three BG maps, the plates and lines, the song,
                                       * the fade in), 4 shown (0x08043A88: A, B or START fades it out), 8 its end (BN4_BRACKET_LEAVE) */
#define BN4_BRACKET_LINES 0x08043D70u /* the lit lines: for each round's bit (4, 2, 1) each entrant that won it, a menu sprite */
#define BN4_BRACKET_LEAVE 0x08043BA8u /* the out-this-round bits (BN4_ENTRANT_OUT) cleared, then the script resumed (0x08005664)
                                       * where BN4_BRACKET_FROM_SCRIPT is set, the map entered again (0x0802DF2C) */
#define BN4_BRACKET       0x0200ACC0u /* the bracket screen's state: +0 BN4_BRACKET_STATES' state, +1 its step in state 4, +4 the
                                       * plates' progress, +8 frames before the plates move (10 from a script) */
#define BN4_BRACKET_TOURNAMENT 0x05   /* BN4_BRACKET +5: the tournament shown (BN4_TOURNAMENT_DEN to BN4_TOURNAMENT_FREE) */
#define BN4_BRACKET_FROM_SCRIPT 0x06  /* ... +6: 1 where a cutscene or text waits on it */
#define BN4_BRACKET_SCRIPT_ARG 0x07   /* ... +7: what it hands the script back (0x08005664's r1) */
#define BN4_BRACKET_MAPS  0x08043AFCu /* its BG maps' sources: three drawn, the third the title's by the tournament (entries 2-5) */
#define BN4_BRACKET_PALETTE 0x086C5B0Cu /* its BG colours, 0xC0 bytes to BN4_BG_PALETTE_RAM */
#define BN4_BRACKET_TILES 0x086C4E40u /* its BG tiles, LZ77 (9,924 bytes), to 0x06008020 */
#define BN4_PALETTE_ANIM  0x08002048u /* a palette's cycle started (r0 its list: the lit lines' flash 0x08043814, a tournament's title
                                       * colours 0x0804383C-0x0804386C), 0x18 bytes a cycle at 0x02009F50 */
#define BN4_SONG_BRACKET  0x09        /* the song the bracket's start plays (0x08043A50) */

/* The tournaments in RAM: eToolkit +0x3C, fixed (the toolkit's set-up 0x08006124), 0x20 bytes each, eight entrants of four
 * bytes in bracket order (a first round's pairs 0-1, 2-3, 4-5, 6-7); cleared to 0xFF by BN4_CLEAR_TOURNAMENTS */
#define BN4_TOOLKIT_TOURNAMENTS 0x3C  /* eToolkit +0x3C: BN4_TOURNAMENTS */
#define BN4_TOURNAMENTS   0x02001460u
#define BN4_TOURNAMENT_SIZE 0x20      /* a tournament's bytes: BN4_ENTRANTS of BN4_ENTRANT_SIZE */
#define BN4_ENTRANT_SIZE  4           /* an entrant's bytes */
#define BN4_ENTRANTS      8           /* a tournament's entrants, MegaMan among them */
#define BN4_TOURNAMENT_DEN 0          /* the tournaments by their titles, each seen drawn: The Den Battle Tournament, */
#define BN4_TOURNAMENT_EAGLE 1        /* ... The Eagle Tournament, */
#define BN4_TOURNAMENT_RED_SUN 2      /* ... The Red Sun Tournament, */
#define BN4_TOURNAMENT_FREE 3         /* ... Free Tournament (its plates and lines BN4_BRACKET_LINES' kin 0x080455C0, 0x080455D0) */
#define BN4_ENTRANT_ID    0x00        /* an entrant: its id: 0x00-0x16 a Navi's plate (0x12 and 0x13 numbered
                                       * placeholders with no battle, 0x16 0x14's plate), BN4_ENTRANT_MEGAMAN, 0xFF none */
#define BN4_ENTRANT_FLAGS 0x01        /* ... its rounds won (BN4_ENTRANT_ROUNDS) and BN4_ENTRANT_IN or BN4_ENTRANT_OUT */
#define BN4_ENTRANT_RANK  0x02        /* ... its seed: of two Navis the lower wins their match (BN4_ADVANCE_ROUND) */
#define BN4_ENTRANT_MARK  0x03        /* ... 1 in, 2 out this round, as the screen last showed it (0x08043EA0) */
#define BN4_ENTRANT_ROUNDS 0x07       /* BN4_ENTRANT_FLAGS: a bit a round won (1 the first, 2 the second, 4 the final) */
#define BN4_ENTRANT_IN    0x40        /* ... still in: its plate in colour */
#define BN4_ENTRANT_OUT   0x80        /* ... out this round: shown so once, the bit cleared as the screen leaves; neither, out
                                       * before: its plate grey */
#define BN4_ENTRANT_MEGAMAN 0x1E      /* MegaMan's id: the draw puts him first, he wins each match of his (BN4_ADVANCE_ROUND) */
#define BN4_TOURNAMENT_POOLS 0x020073A0u /* the candidates a tournament draws from, BN4_POOL_SIZE bytes each (0xFF none) */
#define BN4_POOL_SIZE     12
#define BN4_TOURNAMENT_ENTERED 0x02006130u /* a byte a tournament: 1 once its bracket was opened (BN4_MEGAMAN_ROUNDS answers 4 before) */
#define BN4_CLEAR_TOURNAMENTS 0x080438C4u /* all four cleared to 0xFF, the pools too, the entered bytes 0 (a new game, 0x08004A14) */
#define BN4_START_TOURNAMENT 0x08043FA0u /* r0 the tournament: its pool filled (0x08043FE4, 0x08044058, 0x0804422C, 0x0804428C), the
                                       * bracket drawn (BN4_DRAW_BRACKET), every entrant BN4_ENTRANT_IN (0x08043EEC), the next
                                       * opponent (BN4_NEXT_OPPONENT) */
#define BN4_DRAW_BRACKET  0x080442ECu /* the draw: MegaMan first, the pool's places 0, 2, 7 and 9 dealt to the halves by draws,
                                       * the rest shuffled (0x08001B60 its draws; read, not proved) */
#define BN4_ADVANCE_ROUND 0x08044640u /* r0 the tournament: each pair of entrants still in settled (MegaMan's won, two Navis' by the lower
                                       * BN4_ENTRANT_RANK, then an id under 0x14 over one above, then a draw),
                                       * the winner a round's bit more */
#define BN4_NEXT_OPPONENT 0x08044744u /* MegaMan's pair's other entrant: BN4_OPPONENT_BATTLES[id] + 0-2 (0x08000C44, assumed the
                                       * playthrough) into [eToolkit +0x44] +0xA, which BN4_CS_OPPONENT_ARENA and his battle read */
#define BN4_OPPONENT_BATTLES 0x080447DCu /* by entrant id, the first of its three battles (0xFF for ids 0x12 and 0x13) */
#define BN4_MEGAMAN_ROUNDS 0x08043F4Cu /* r0 the tournament: MegaMan's rounds won, 0-3, 4 where it was never entered */

/* The bracket's plates: menu sprites (24 of 0x58 at BN4_MENU_SPRITES, run by 0x08053FA8 and drawn by 0x08053FE4), the
 * entrants' its type 1 */
#define BN4_MENU_SPRITES  0x02008BB0u
#define BN4_MENU_SPRITE_SIZE 0x58
#define BN4_MENU_SPRITE_COUNT 24
#define BN4_MENU_SPRITE_TYPES 0x08053E24u /* by type, a routine */
#define BN4_MENU_SPRITE_SPAWN 0x08054056u /* r0 the type, r1 the slot, r2 its word */
#define BN4_BRACKET_PLATE 0x08103FFCu /* type 1: an entrant's plate (its kind 0), or a lit line (kinds 1 and 2) */
#define BN4_PLATE_FRAMES  0x0810414Du /* by entrant id (0x00-0x1E), the plate's animation in its sprite */
#define BN4_PLATE_SHEETS  0x08104127u /* by entrant id, 0 (ids 0x00-0x0F) or 0x11 (the rest): BN4_PLATE_SPRITES' index */
#define BN4_PLATE_SPRITES 0x0810433Fu /* ... the plate's sprite in BN4_MENU_SPRITE_PTRS: 4 (16 plates), 5 (seven: ids 0x10-0x15 and
                                       * MegaMan's, 6) */
#define BN4_MENU_SPRITE_PTRS 0x080281D0u /* the menu sprites' sprite pointers (0x08002908's r0): the plates 0x084BB768 and 0x084BDB30 */

/* What opens them in a script */
#define BN4_CUTSCENE_COMMANDS 0x0802C140u /* the cutscene engine's command routines by command byte (dispatched at 0x0802D05C) */
#define BN4_CS_START_TOURNAMENT 0x52  /* `52 t`: BN4_START_TOURNAMENT (0x0802CE1E) */
#define BN4_CS_ADVANCE_ROUND 0x53     /* `53 t`: BN4_ADVANCE_ROUND, then BN4_NEXT_OPPONENT (0x0802CE32) */
#define BN4_CS_SHOW_BRACKET 0x54      /* `54 t script argument`: BN4_OPEN_BRACKET (0x0802CE4A) */
#define BN4_CS_OPPONENT_ARENA 0x55    /* `55 0`: the next opponent's net map into the game state's +6 and +7, kept; `55 1` back */
#define BN4_TEXT_COMMANDS 0x0804E900u /* the text engine's commands by byte - 0xE5 (0x0804E13C) */
#define BN4_TEXT_TOURNAMENT 0xFC      /* its sub-commands by the next byte (0x08050434): `FC 01 t script argument` BN4_OPEN_BRACKET */

/* The bracket moved into BN6 (src/emu/xbracket.h; docs/multirom/bn4-tournament.md, Moved into BN6): what the move reads besides the
 * screen's own, found by tools/xrecipe_bracket.py's walk and proved by tools/xbracket.py */
#define BN4_MAP_LEAVE     0x08005040u /* the map left for a screen (the sub-state BN4_GAMESTATE_TO_BRACKET's call before the set-up): its
                                       * sounds stopped, BN4_BG_PRESETS_SET with BN4_BRACKET_BG_PRESET among its calls, DISPCNT 0x40 its last */
#define BN4_BG_PRESETS_SET 0x08001BE8u /* r0 a preset: its 8 bytes (BG0CNT-BG3CNT) from BN4_BG_PRESETS into the render info ([eToolkit +8] +4) */
#define BN4_BG_PRESETS    0x08001C04u /* the BG controls' presets, 8 bytes each */
#define BN4_BRACKET_BG_PRESET 0x13    /* ... the bracket's (BN4_MAP_LEAVE's movs r0,#0x13) */
#define BN4_BRACKET_BLEND 0x02009440u /* the bracket's line blend values, 0x140 bytes, sent to BLDALPHA line by line by the HBlank DMA */
#define BN4_BRACKET_TILES_RAM 0x0202E000u /* BN4_BRACKET_TILES unpacked (its LZ77 header's size, 0x26C4), copied to 0x06008020 */
#define BN4_HBLANK_DMA_SET 0x080060A4u /* an HBlank DMA set: r0 its source, r1 its register, r2 its count, r3 its control into BN4_HBLANK_DMA */
#define BN4_HBLANK_DMA    0x02008920u /* ... its state (+0 on, +1 count, +2 control, +4 source, +8 register; 0x080060BC stops it) */
#define BN4_MENU_SPRITE_SET 0x08002908u /* a menu sprite's sprite (r0 its index in BN4_MENU_SPRITE_PTRS, r5 the sprite) */
#define BN4_BG_FILL_RECT  0x08001E18u /* a BG map's rect filled with r3 (r0, r1 its place, r2 the BG, r4, r5 its size): a veneer to IWRAM
                                       * 0x03005A5D, whose body BN6's 0x03005EBB has (its veneer 0x080018D0) */
#define BN4_BRACKET_CYCLES 0x08043814u /* the palette cycles' lists BN4_PALETTE_ANIM takes: the lit lines' flash, then the titles' (to 0x08043884) */
#define BN4_BRACKET_SONG_SITE 0x08043A50u /* movs r0,#BN4_SONG_BRACKET before the music's start, in the screen's start */
#define BN4_FREE_SPAWN_LISTS 0x08044844u /* the Free Tournament's menu sprites' spawn lists (types 0x3C and 0x3D), 4 bytes an entry, 0xFF ending */
#define BN4_BRACKET_ART_END 0x086C792Cu /* the end of the bracket's art: BN4_BRACKET_TILES, BN4_BRACKET_PALETTE and its cycles' colours, the six
                                       * maps of BN4_BRACKET_MAPS (0x4B0 bytes each) one after another */

#endif

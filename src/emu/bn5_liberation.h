/* Battle Network 5: Team Colonel (USA): its Liberation Missions, as its ROM keeps
 * them (docs/LIBERATION.md, docs/ROM_DATA.md's BN5's Liberation Missions). No
 * engine code reads them yet: the names stand for the generator's design and
 * for tools/liberation_bn5.py, which reads the same records. */
#ifndef CW_BN5_LIBERATION_H
#define CW_BN5_LIBERATION_H

/* The board's state while a mission runs */
#define BN5_LIB_STATE        0x020022F0u /* the liberation state: +0 the board's routine (an offset into BN5_LIB_BOARD_STEPS) */
#define BN5_LIB_MISSION      (BN5_LIB_STATE + 0x04) /* u8: the mission under way, 1-9 (0 none); kept on the map too */
#define BN5_LIB_ORDER_POINTS (BN5_LIB_STATE + 0x07) /* u8: the team's Order Points (the mission record's +0xB at the start) */
#define BN5_LIB_PANEL_COUNT  (BN5_LIB_STATE + 0x0E) /* u16: the board's panels (100 on mission 1) */
#define BN5_LIB_PAR          (BN5_LIB_STATE + 0x16) /* u8: the target phases ("TRGT PHASE"), the record's +0xA */
#define BN5_LIB_PHASE        (BN5_LIB_STATE + 0x17) /* u8: the phase under way, from 1 */
#define BN5_LIB_RECORD       (BN5_LIB_STATE + 0x24) /* u32: the mission's record (BN5_LIB_MISSIONS) */
#define BN5_LIB_PANEL_LIST   (BN5_LIB_STATE + 0x28) /* u32: its panels in RAM (0x02030000), 8 bytes each: the kind, */
#define BN5_LIB_PANEL_SIZE   8           /* ... +1 flags (0x10 a unit stands on it), +2 x, +3 y, +4-+7 its edges' looks */
#define BN5_LIB_UNITS        0x0203E000u /* the board's units, 0x28 bytes each: the team, then the Darkloid and guardians */
#define BN5_LIB_UNIT_SIZE    0x28        /* ... +5 the Navi or enemy, +0x0A x, +0x0B y, +0x14 HP, +0x16 its max */
#define BN5_LIB_OP_ADD       0x080519D0u /* Order Points raised by r0, no higher than BN5_LIB_OP_MAX */
#define BN5_LIB_OP_SPEND     0x080519E0u /* Order Points lowered by r0, no lower than 0 */
#define BN5_LIB_OP_MAX       8           /* the most Order Points the team holds */
#define BN5_LIB_ON_MAP       0x0805198Au /* true where a mission is under way and its map is the map entered */
#define BN5_ENTER_MAP_BOARD  0x08004DE2u /* EnterMap's choice: the game state's sub-mode BN5_SUBMODE_BOARD there, 4 elsewhere */
#define BN5_SUBMODE_BOARD    0x18        /* the game state's sub-mode of a mission's board (its routine 0x08004FB6) */
#define BN5_LIB_BOARD_STEP   0x0804BD94u /* the board's frame: the routine BN5_LIB_STATE +0 names, then the camera and units */
#define BN5_LIB_BOARD_STEPS  0x0804BDB4u /* ... its table of eight routines (+0 the mission's setup 0x0804BDD8) */
#define BN5_LIB_PANEL_PLACE  0x08051BA8u /* a panel's id (y << 8 | x) to world units: 32 a panel from 0x80, less the record's +6, +7 */
#define BN5_LIB_ITEM_FIND    0x08051F20u /* the item panel at r0's id: r1 its kind, r2 its amount, r3 its chip and code (0 none) */
#define BN5_LIB_BONUS_ROLL   0x08054F78u /* a BonusPanel's roulette set going: its first slot (RNG & 0xF) % 5 of five */

/* The board's look (docs/LIBERATION.md, How the board looks and moves): the map on BG1, each panel's
 * kind laid over it on BG2 as an isometric diamond of tiles. Located by their bytes in a capture of
 * mission 1 (map 0x86:00); the drawing routine and the other maps' sets are still to be traced. */
#define BN5_LIB_MAP_CHARS    0x0861C510u /* map 0x86:00's tiles, uncompressed: tile t at this + 32 t */
#define BN5_LIB_PANEL_TILES  0x08623130u /* ... from its tile 865: the overlay's (clean, dark, DarkHole, the teal stud) */
#define BN5_LIB_LOOKS        0x08624310u /* the twelve panel looks, 0x40 bytes each (8 x 4 tile-map entries, the diamond): clean, dark, */
#define BN5_LIB_LOOK_SIZE    0x40        /* ... dark paled (the dissolve), item, "+5", BarrierPanels 1-4, BonusPanel, DarkHole, a grey grid */
#define BN5_LIB_LOOK_OF      0x0804F0D4u /* a panel's look: the look block's pointer at +0x10 + 4 x its kind (the kinds are the looks' order: */
                                         /* ... 2 a panel mid-dissolve, 4 the "+5", 0x0B the Darkloid's ring, grey until the DarkHoles fall) */
#define BN5_LIB_BOARD_SETUP  0x0804EEE2u /* the board drawn: the overlay's tiles copied, then each panel's look and its edges */
#define BN5_LIB_CORNERS      0x0804ECF0u /* a panel's four look bytes made from its neighbours: each corner's three (its two sides, 1 on x */
                                         /* ... and 4 on y, the diagonal 2) that are walkable for the Navi in play: +4 W NW N, +5 E NE N, */
                                         /* ... +7 E SE S, +6 W SW S; a neighbour counts where its flags +1 lack 0x10 and its kind's bit */
                                         /* ... is in the Navi's unit record +0x0C (MegaMan's 0x1001: clean and start) */
#define BN5_LIB_EDGES        0x0804F204u /* a panel's borders in the map's collision: its four look bytes (+4-+7), one a corner, each */
#define BN5_LIB_EDGE_TABLES  0x0804FB94u /* ... indexing that corner's table: lists of height modifiers (0x8800 + n at cell dx, dy; 0xFF ends) */
#define BN5_LIB_Z_LIST       0x02030804u /* ... gathered here (u16 cell y * 0xFE + x, u16 value; the count at -4), from the panel's corner */
                                         /* ... cell ((px - 0x80) * 4 + 0x7F + record +2, (py - 0x80) * 4 + 0x7F + record +3), 8 units a cell */
#define BN5_LIB_Z_INSTALL    0x080315D4u /* ... handed to the map (its state 0x02013770), read by the height check 0x080320EA (bn6f */
                                         /* ... checkZCoordModifiers_8031612) through 0x080315F8: so a Navi stops at a panel he cannot walk */
#define BN5_LIB_PTR_LOOK     0           /* the record's look (the same in all nine): +0 the overlay tiles' source, +4 their VRAM, +8 their size, */
                                         /* ... +0x10 the twelve looks' pointers (BN5_LIB_LOOKS) */
#define BN5_LIB_PAL_CLEAN    0x08614EB4u /* palette 8: the clean panels' cyan */
#define BN5_LIB_PAL_DARK     0x08614B84u /* palette 9 at rest: the dark panels' swirl, held 120 frames; then BN5_LIB_DARK_FRAMES' three */
#define BN5_LIB_DARK_FRAMES  0x08614B24u /* ... below it (0x20 apart, from here), down and up: 0x08614B64 16 frames, 0x08614B44 16, */
                                         /* ... this 32, 0x08614B44 16, 0x08614B64 16, then the rest again (216 frames round) */
#define BN5_LIB_PAL_HOLE     0x08614BC8u /* palette 10 as the capture held it; its four frames 0x08614BA8-0x08614C08 stepped */
#define BN5_LIB_HOLE_STEP    20          /* ... there and back, this many frames a step */
#define BN5_LIB_PULSE        0x08614C2Cu /* palette 11 (the barriers') at rest; every 132 frames a glint: the next five palettes, */
#define BN5_LIB_PULSE_FRAMES 6           /* ... 0x20 apart, held 6, 2, 2, 1, 1 frames, then this again */
#define BN5_LIB_BREATH       0x08614CF0u /* palette 12 (the BonusPanel's): six palettes 0x20 apart stepped there and back, 12 frames */
                                         /* ... each (8 at the last); a palette's working copy at 0x03003960 + 0x20 n, then 0x03003B60 + 0x20 n */
#define BN5_LIB_BLINK        4           /* a freed panel's look toggles between its old kind and clean every this many frames, */
#define BN5_LIB_BLINKS       14          /* ... this many times, once the board has faded back in (27 frames), then stays clean */
#define BN5_LIB_EVENT        0x02001AB0u /* the board's events, BN5_LIB_EVENT_SIZE each: +0 a kind, +1, +4 u32 (its low byte the slot, */
#define BN5_LIB_EVENT_SIZE   12          /* ... the next its text script: an item's find), +8 u16 the panel */
#define BN5_LIB_EVENT_QUEUE  0x08055D68u /* an event queued: r0 its kind, r1, r2 the word, r3 the panel */

/* The board's sprites (BN5's sprite lists, docs/ROM_DATA.md, BN5's Navis: list, index) */
#define BN5_LIB_OUTLINE_SPRITE 0x0848A5E0u /* list 7 index 15: the faced panel's outline, two mirrored 32x32 OBJs (OBJ palette 2), */
                                         /* ... anim 0's five frames held 20, 2, 2, 2, 8; shown only while the command window is */
                                         /* ... open over a dark panel faced, from 6 frames after A */
#define BN5_LIB_OUTLINE_PAL  0x0848B160u /* ... its palette */
#define BN5_LIB_SPRITES_7    0x08032D04u /* sprite list 7's table (the lists' at 0x0803272C): entry k a u32 at +4k */
#define BN5_LIB_ICONS_SPRITE 0x0848DF40u /* list 7 index 24, the heart that pops over a freed panel; each item kind's icon is the list */
                                         /* ... 7 index at BN5_LIB_ITEM_KINDS +4: 0x18 the heart, 0x19 a key 0x0848E1F8, 0x25 zenny */
                                         /* ... 0x084907AC, 0x26 BugFrags 0x08490A20, 0x29 Order Points 0x08492EF8, 0x53 a chip */
                                         /* ... 0x0849D508; anim 0, the shadow its own OBJ */
#define BN5_LIB_ICONS_PAL    0x0848E0FCu /* ... the heart's palette (OBJ palette 3) */
#define BN5_LIB_BANNER_TILES 0x0873CEF4u /* the battle messages' letters, 8x16 (0x40 bytes): A-Z from here, space 0x0873D574, "!" */
                                         /* ... 0x0873D5B4, the caps 0x0873D5F4 and 0x0873D634; five 32x16 affine OBJs at y 64 */
#define BN5_LIB_BANNER_DIGITS 0x0873D974u /* ... their digits, 8x16, 0x40 bytes each ("1 TURN LIBERATE!", "TURN n START!") */
#define BN5_LIB_BANNER_PAL   0x0873E4B4u /* ... their palette (OBJ palette 11) */
#define BN5_LIB_BANNER_HOLD  43          /* ... frames at full size, after 8 of opening and before 8 of closing (the scale's pd */
                                         /* ... 768, 640, 512, 384, 256, 208, 224, then 256; out 224, 208, 256, 384 ... 896) */
#define BN5_LIB_MESSAGES     0x0801B8D4u /* ... the messages' table, each a list of its letters' graphics (their pointers) */

/* The board's HUD and window (docs/LIBERATION.md, How the board looks and moves) */
#define BN5_LIB_HUD_OAM      0x080531F4u /* the Order Points gauge: six OAM entries (u32 attributes 0 and 1, u16 attribute 2, u32 1): */
                                         /* ... 32x16 at (162,0) and (194,0), 16x16 at (224,0), the bar 32x8 at (170,4) and 16x8 at */
                                         /* ... (202,4), the count 16x8 at (216,4); OBJ palette 12 (BN5_LIB_HUD_PAL), tiles 624 on */
#define BN5_LIB_HUD_LOADS    0x080531C8u /* ... their tiles' copies from RAM (0x02039200 0x280 bytes, 0x02039580 0xC0, 0x02039540 0x40) */
#define BN5_LIB_HUD_DIGITS   0x087E4704u /* the HUD's digits (8x8, 0x40 bytes a digit) */
#define BN5_LIB_HUD_FRAME    0x087E4884u /* the gauge's frame, copied to 0x02039200 */
#define BN5_LIB_HUD_BAR      0x087E4984u /* the gauge's bar, masked by the count */
#define BN5_LIB_HUD_MASKS    0x08053240u /* ... the bar's six tiles (6 px an Order Point) copied, the first 6 - (6 OP + 7) / 8 */
                                         /* ... cleared and the next one's rows ANDed with the u32 at +8 OP */
#define BN5_LIB_HUD_PAL      0x086D5904u /* OBJ palette 12: BN5's HUD palette (the HP box's too) */
#define BN5_LIB_PHASE_TILES  0x087E4534u /* the L view's "PHASE" and "TRGT PHASE" (OBJ palette 13 at 0x087E4658), its digits from */
#define BN5_LIB_PHASE_DIGITS 0x087E4E04u /* ... here (palette 12), its corner arrows from 0x087E4CC4 */
#define BN5_LIB_WINDOW_PAL   0x087E4FA4u /* OBJ palette 15: the command window, 21 OBJs of 16x16 at x 128-239, y 114-157, drawn into */
                                         /* ... RAM (0x0203A400) by BN5's code each time it opens; its cursor tile 704 (0x086BA484), */
                                         /* ... OBJ palette 14 (0x086CCD24), at (112,112), 14 px a line */
#define BN5_LIB_WINDOW_TILES 0x087E4FC4u /* the window's frame on BG0: nine tiles (corners, edges, body), 16 x 7 tiles at tile (14,13) */
#define BN5_LIB_WINDOW_BG_PAL 0x087E5144u /* ... its BG palette 14 */
#define BN5_LIB_BACKDROP_PAL 0x0861BF20u /* BG palette 0 in a mission: the map's own animated backdrop (its tiles rebuilt in RAM at */
                                         /* ... 0x0200E960) in purple; map 0x86:00's own palette 0 otherwise 0x086203CC */

/* The missions */
#define BN5_LIB_MISSIONS     0x0804F628u /* the nine missions' records, BN5_LIB_MISSION_SIZE each: +0 the map (group, number), */
#define BN5_LIB_MISSION_SIZE 0x5C        /* ... +6 and +7 the panels' origin (s8), +0xA the par, +0xB the Order Points at the start, */
#define BN5_LIB_MISSION_COUNT 9          /* ... +0xC twenty pointers (BN5_LIB_PTR_*) */
#define BN5_LIB_PTR_PANELS   1           /* the panels: u16 kind (BN5_PANEL_*), u8 x, u8 y; a kind's low byte 0xFF ends them */
#define BN5_LIB_PTR_HOLES    2           /* the DarkHoles' panels in their order, x and y a byte each */
#define BN5_LIB_PTR_TEAM     3           /* the team: 20 bytes a Navi (+5 its id, BN5_NAVI_*; +8, +0xA its place in world units) */
#define BN5_LIB_PTR_ENEMIES  4           /* the Darkloid, then the guardians: 20 bytes each (+0 its DarkHole, 0xFF none; +5 its kind; +8 x, +9 y) */
#define BN5_LIB_PTR_BONUS    8           /* the BonusPanels: x, y, u16, five outcomes' records; a u16 0 ends them */
#define BN5_LIB_PTR_TRAPS    9           /* the traps: kind (0 damage, its amount times 10; 1 paralysis), amount, x, y; 0xFFFFFFFF ends them */
#define BN5_LIB_PTR_ITEMS    10          /* the item panels: kind (BN5_ITEM_*), code, u16 chip, u16 amount, x, y; 0xFFFFFFFF ends them */
#define BN5_LIB_PTR_REWARDS  19          /* the rewards: kind (0 a chip, 1 zenny), code, phases from, phases to, u16 chip or zenny */
#define BN5_LIB_NAVI_HP      0x0804F968u /* per team Navi: u16 his id, then his HP in each of the nine missions */
#define BN5_LIB_ITEM_KINDS   0x08051924u /* per item kind 8 bytes: +2 the amount's multiplier (hearts 10, zenny 100); handlers 0x0804D8B4 */

/* A panel's kind */
#define BN5_LIB_PANEL_FREE       0x00        /* clean: the team walks it */
#define BN5_LIB_PANEL_DARK       0x01        /* a DarkPanel: liberated by a battle */
#define BN5_LIB_PANEL_ITEM       0x03        /* a DarkPanel holding an item or a trap */
#define BN5_LIB_PANEL_BARRIER1   0x05        /* a BarrierPanel, opened by key 1 (6 key 2, 7 key 3) */
#define BN5_LIB_PANEL_BONUS      0x09        /* a BonusPanel ("?") */
#define BN5_LIB_PANEL_HOLE       0x0A        /* a DarkHole */
#define BN5_LIB_PANEL_BOSS_RING  0x0B        /* the eight panels around the Darkloid */
#define BN5_LIB_PANEL_START      0x0C        /* the team's start */

/* An item panel's kind */
#define BN5_ITEM_HEART       0           /* HP, its amount times 10 */
#define BN5_ITEM_KEY         1           /* a key: its BarrierPanels opened */
#define BN5_ITEM_ORDER_POINTS 2          /* Order Points */
#define BN5_ITEM_ZENNY       3           /* zenny, its amount times 100 */
#define BN5_ITEM_BUGFRAGS    4           /* BugFrags */
#define BN5_ITEM_CHIP        5           /* a chip and its code */

/* The team's Navis in Team Colonel (a team record's +5; BN5_LIB_NAVI_HP's ids) */
#define BN5_NAVI_MEGAMAN     0x00        /* MegaMan: his own HP */
#define BN5_NAVI_COLONEL     0x07        /* Colonel: ScrenDiv */
#define BN5_NAVI_SHADOWMAN   0x08        /* ShadowMan: SneakAttack */
#define BN5_NAVI_NUMBERMAN   0x09        /* NumberMan: NumberCheck */
#define BN5_NAVI_TOMAHAWKMAN 0x0A        /* TomahawkMan: TomahawkSwing */
#define BN5_NAVI_KNIGHTMAN   0x0B        /* KnightMan: no command of his own */
#define BN5_NAVI_TOADMAN     0x0C        /* ToadMan: LifeMelody */

/* A liberation battle */
#define BN5_BATTLE_TYPE      0x0802B3E0u /* the battle's record's +3 (BattleSettings' BattleType): */
#define BN5_BATTLE_LIBERATION 1          /* ... 1 a liberation battle */
#define BN5_BATTLE_TURN      0x08009016u /* BattleState +7: the turns the Custom Gauge has filled */
#define BN5_LIB_TURNS_UP     0x08009190u /* true once BattleState +7 reaches 3: the battle loop (0x08007954) ends the battle */
#define BN5_LIB_LATE_DOUBLE  0x0800901Eu /* true in a liberation battle from its eleventh turn: a hit's damage doubled (0x0800C7CC) */

#endif

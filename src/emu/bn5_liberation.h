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
#define BN5_LIB_EDGES        0x0804F204u /* a panel's edges: its four look bytes (+4-+7), one a side, each a list from BN5_LIB_EDGE_TABLES */
#define BN5_LIB_EDGE_TABLES  0x0804FB94u /* ... four sides' tables of lists (tile 0x88xx, x, y; 0xFF ends them) */
#define BN5_LIB_PTR_LOOK     0           /* the record's look (the same in all nine): +0 the overlay tiles' source, +4 their VRAM, +8 their size, */
                                         /* ... +0x10 the twelve looks' pointers (BN5_LIB_LOOKS) */
#define BN5_LIB_PAL_CLEAN    0x08614EB4u /* palette 8: the clean panels' cyan */
#define BN5_LIB_PAL_DARK     0x08614B84u /* palette 9 as the capture held it: the dark panels' swirl */
#define BN5_LIB_PULSE        0x08614C4Cu /* the pulse: six palettes of 0x20 bytes copied in turn into palette 9's */
#define BN5_LIB_PULSE_FRAMES 6           /* ... buffer, each held one to six frames (the map's palette animation, assumed) */
#define BN5_LIB_PAL_HOLE     0x08614BC8u /* palette 10 */
#define BN5_LIB_EVENT        0x02001AB0u /* the board's events, BN5_LIB_EVENT_SIZE each: +0 a kind, +1, +4 u32 (its low byte the slot, */
#define BN5_LIB_EVENT_SIZE   12          /* ... the next its text script: an item's find), +8 u16 the panel */
#define BN5_LIB_EVENT_QUEUE  0x08055D68u /* an event queued: r0 its kind, r1, r2 the word, r3 the panel */

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

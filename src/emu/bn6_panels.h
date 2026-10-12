/* BN6's battlefield panels as its engine keeps, updates, draws and acts on
 * them, where BN5's lava and sea come into its free panel types (src/emu/
 * xpanels.c; docs/ROM_DATA.md, BN5's lava and sea in BN6's engine; issue
 * #82). The panels themselves are bn6.h's (BN6_FIELD_PANELS, BN6_PANEL_*);
 * Gregar's addresses, found by the panels' routines in the ROM. */
#ifndef CW_BN6_PANELS_H
#define CW_BN6_PANELS_H

/* A panel's fields past bn6.h's, and its types the engine names */
#define BN6_PANEL_ANIM        0x06        /* Drawn as: the type a panel is drawn as (its type; 1 or 2 while a hole closes or a conveyor runs out) */
#define BN6_PANEL_TIMER       0x12        /* Timer, u16: a conveyor's frames left (0x708 as one is made); 0x708 written every frame on most others */
#define BN6_PANEL_TIMER_IDLE  0x708       /* BN6_PANEL_TIMER: what it holds where it is not counting (a field's start writes it, 0x0800C564) */
#define BN6_PANEL_VOLCANO     8           /* BN6_PANEL_TYPE: a volcano (an eruption every 140 frames, 0x0800C438; an Aqua hit cools it plain) */
#define BN6_PANEL_CONVEYOR    9           /* BN6_PANEL_TYPE: the first conveyor, ... */
#define BN6_PANEL_CONVEYOR_END 12         /* ... to the last (9-12: they push what stands on them, 0x0801A39C, and run out to plain) */
#define BN6_PANEL_TYPES       13          /* BN6_PANEL_TYPE: past its last, how many it has (0-12); none of its fields or code names 13-15, which the engine gives BN5's lava and sea */

/* The flags each type gives, in IWRAM: _object_updatePanelParameters
 * (0x030078DC, from 0x0800C928 and the hit pass) ORs its type's word in
 * (bn6.h's BN6_PANEL_FLAGS_OF, the image of the table); the IWRAM
 * routines are copied from their image in ROM as the game powers on */
#define BN6_IWRAM_IMAGE       0x081D6000u /* the IWRAM routines' image in ROM, copied to BN6_IWRAM_CODE */
#define BN6_IWRAM_CODE        0x03005B00u /* ... where they run (an IWRAM address less this, plus the image, is its byte in ROM) */
#define BN6_PANEL_FLAGS_REF   0x0300791Cu /* the literal naming the table (ldr r3,[pc,#0x28] at 0x030078F0), image 0x081D7E1C: */
#define BN6_PANEL_FLAGS_IW    0x03007920u /* ... the table's copy in IWRAM, 13 words, getPanelDataOffset's code after it (0x03007954) */

/* A panel hit by an attack of an element (IWRAM 0x03007704, run by the hit
 * pass for each panel an attack's region covers, 0x030075B0): grass burns
 * plain under Fire, a volcano cools plain under Aqua, a conveyor stops
 * under Wood */
#define BN6_PANEL_HIT         0x03007704u /* the routine (r0 column, r1 row, r2 the attack's collision data): +2 its element */
#define BN6_PANEL_HIT_COOL    0x03007742u /* ... its cmp r0,#8 (the volcano's), from where the volcano's and the conveyors' tests run to the */
#define BN6_HIT_COOL_SPAN     0x30        /* ... last pop (0x03007770): their bytes, the image's at 0x081D7C42 */
#define BN6_SET_PANEL_TYPE_IW 0x030079A0u /* bn6f object_setPanelType in IWRAM (r0 column, r1 row, r2 type): a panel of type 0 refused, 9-12 given 0x708 */
#define BN6_SET_PANEL_TYPE    0x0800CC0Au /* ... its ROM entry, every chip's and attack's: push {r4,r5,r6,lr}, then BN6_SET_PANEL_TYPE_IW */
#define BN6_WEAKNESS          0x03007440u /* IWRAM: a byte by element x 5 + the attacker's element (0-4), the damage's shift: 1 where it is weak (0x0300742E) */
/* What a hit adds for the panel its victim stands on (IWRAM 0x03007668,
 * from the hit pass at 0x03007416 and 0x030076F6, each result added to the
 * victim's +0x82 or +0x94): r0 the victim's collision data, r1 the
 * attack's; back in r0 the attack's damage once more for Fire on grass,
 * else 0. BN5's twin (BN5_PANEL_BONUS) doubles Elec on sea as well: BN6's
 * branches past that test, the body it took left behind it */
#define BN6_PANEL_BONUS       0x03007668u /* the routine, push {r4,lr}; its */
#define BN6_PANEL_BONUS_TESTS 0x03007678u /* ... tests, cmp r1,#1 (r0 the panel's type, r1 the attack's element, r4 its collision data), to */
#define BN6_PANEL_BONUS_SPAN  0x16        /* ... its last pop (0x0300768C): their bytes, the image's at 0x081D7B78 */

/* A panel's update each frame (0x0800C380, from the field's loop
 * 0x0800BFC4, r7 the panel), and what stands on one (0x0801A186, from each
 * actor's pre-step after its status step) */
#define BN6_PANEL_UPDATE      0x0800C380u /* the per-panel update: holes close, cracks break, volcanoes erupt, conveyors run out; its */
#define BN6_PANEL_UPDATE_CONVEYOR 0x0800C3B8u /* ... cmp r0,#9, r0 the type, where 9-12 go to their timer's count (0x0800C45C), the rest have 0x708 written */
#define BN6_PANEL_STEP        0x0801A186u /* what the panel under an actor does to it (r5 the object): poison and grass; */
#define BN6_PANEL_STEP_POISON 0x0801A1AAu /* ... its cmp r0,#4, r0 the panel's type, r7 the object's collision data: poison's 1 HP a 7 frames */
#define BN6_OBJECT_STATUS     0x0801A802u /* each actor's status step (paralysis, freeze, bubble), push {r4,r6,lr}: its pre-steps' first call,
                                           * where BN5's call its lava step first (the six of 0x0801A9E2-0x0801AC96) */
#define BN6_STEP_OBSTACLE_A   0x0801AD3Cu /* an obstacle's pre-step (0x0801AD12, 12 callers): ldrb r4,[r7,#0xf], where BN5's twin steps it on lava */
#define BN6_STEP_OBSTACLE_B   0x0801AD94u /* ... another's (0x0801AD6A): bl 0x0800E420, the same */
#define BN6_STEP_OBSTACLE_C   0x0801ADC8u /* ... another's (0x0801AD9E, 4 callers): ldrb r4,[r7,#0xf], the same */
#define BN6_STEP_OBSTACLE_D   0x0801AE24u /* ... another's (0x0801ADFA): bl 0x0801AE56, the same */
#define BN6_STEP_OBJECT       0x080DA5FAu /* an object's routine (0x080DA5EE, holy halving its damage): bl 0x0800E3BE, the same */
#define BN6_PANEL_COORDS      0x0800E276u /* panel (r0 column, r1 row) as the screen's x and y, 16.16, in r0 and r1 */
#define BN6_PLACE_JITTER      0x0801BDDEu /* (r1, r3) moved by a random amount in r0's range, a random number drawn (0x0800151C) */
#define BN6_SPAWN_SPARK       0x080E1C00u /* the fourth kind's type 4, the spark a buster shot strikes with, at (r1, r2, r3), r4 its parameter, r5 its owner */
#define BN6_BATTLE_FLAGS      (BN6_BATTLE_STATE + 0x32) /* u16, BN6_TIME_STOP while time is stopped (bn6f's test 0x0800A098, from 0x0800A2F0) */
#define BN6_TIME_STOP         4           /* BN6_BATTLE_FLAGS: time stopped (a Navi chip's) */

/* What a panel's code reads of a collision data (BN6_T1_COLLISION, its
 * fields as bn6.h's BN6_COLL_*) */
#define BN6_COLL_ELEMENT      0x02        /* Element: 0 none, 1 Fire, 2 Aqua, 3 Elec, 4 Wood */
#define BN6_COLL_PANEL_X      0x0A        /* the panel it stands on: its column, */
#define BN6_COLL_PANEL_Y      0x0B        /* ... and its row */
#define BN6_COLL_HIT          0x0F        /* the hit's bits (3: struck): BN5 ORs them into its +0x18 and +0x19 too, which it ORs here, BN6 here alone */
#define BN6_COLL_DAMAGE       0x2E        /* u16, an attack's damage, what each of its hits deals (BN5's +0x2E too) */
/* (BN6_COLL_FLAGS, bn6.h's, its object's flags as bn6f's object_getFlag reads them, 0x0801A166: poison spares 0x08000028,
 * lava 0x20 and 0x88000206, its damage 9) */
#define BN6_COLL_DAMAGE_ADD   0x84        /* u16, damage taken outside a hit (lava's), taken off at 0x0800EAC4 */
#define BN6_COLL_POISON       0x8C        /* u16, poison's damage, a point each 7 frames (the sea's on a Fire object, as xpanels.c brings it) */
#define BN6_COLL_FIRE_DAMAGE  0x96        /* u16, the frame's Fire damage (+0x94 to +0x9C the five elements', summed at 0x0801A916) */

/* How the panels are drawn: each frame (0x0800C5E0, from the battle's
 * three loops) the panel palettes' animations step (0x0800C192), then
 * each panel's 5x3 tiles go to BG2's map from the tile table by its
 * animation, alliance and row (0x0800C01C); the tiles and palettes are
 * loaded as a battle begins (0x080075CA) */
#define BN6_PANEL_DRAW_ANIMATED 0x0800C5E6u /* the panels' draw, past its palette step (ldr r7,[pc,#0x2e4]): the palettes as they will be shown */
#define BN6_PANEL_TILES       0x086DDA1Cu /* the tile table: 32 bytes per type x 6 + alliance x 3 + row - 1 (5x3 BG map entries), BN6_PANEL_TYPES types; */
#define BN6_PANEL_TILES_REF   0x0800C07Cu /* ... its one literal (0x0800C060's ldr r6) */
#define BN6_PANEL_TILESET     0x086DBB24u /* the panels' tiles (LZ77), 498 of them to VRAM from tile 0xA3 (0x06001460), BG2's char base 0; */
#define BN6_PANEL_TILESET_REF 0x0800761Cu /* ... its one literal (0x080075E2's ldr r0); tiles 0x295-0x3FF free in a battle */
#define BN6_TILE_PANELS       0xA3        /* ... the first tile they take, */
#define BN6_VRAM_PANELS       0x06001460u /* ... at this address */
#define BN6_PANEL_PALETTES    0x086DE87Cu /* the panels' palettes 1-8 (copied to BN6_BG_PALETTE + 0x20 from the loader's list 0x080075F0):
                                           * 1 plain, holes, cracks and grass, 2 poison and volcanoes, 3 ice and conveyors, 4 holy; 5-8 the enemies' side's */
#define BN6_PANEL_ANIMS       0x0800C1DCu /* the panel palettes' six animations (0x0800C192): 2, 6, 3, 7, 4, 8; */
#define BN6_PANEL_ANIM_FRAMES 0x02036740u /* ... each one's frame (+2, +4 .. +0xC) and the frames to its next (+3, +5 .. +0xD), reset at a battle's start */
#define BN6_FRAME_COUNT       0x0200A210u /* u16, one more each frame of the main loop (0x08000300, through eToolkit +0x24; bn6f iCurrFrame) */

#endif

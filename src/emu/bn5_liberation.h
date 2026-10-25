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
                                         /* ... +0x10 the looks' pointers (BN5_LIB_LOOKS; past the twelve the start's, 0x0C, all blank, */
                                         /* ... then two on palette 7 no record's kind takes) */
#define BN5_LIB_PALETTE      0x08614DB4u /* the board's BG colours, banks 0-12, loaded over its map's (the frames grey; bank 8 */
#define BN5_LIB_PALETTE_SIZE 0x1A0       /* ... BN5_LIB_PAL_CLEAN): found as 0x20-apart banks ending in palettes 8-10's, same size as 0x86:00's */
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
#define BN5_LIB_ANIMS        0x08073E48u /* the mission maps' GFXAnim lists (group 0x86's, a u32 a map; LoadGFXAnims' table 0x08031468 */
                                         /* ... entry 6, its literal at 0x08073E44): 0x86:00's at 0x08073E94 runs palettes 9-12 */
#define BN5_LIB_ANIM_DARK    0x08073B20u /* ... palette 9's script (to 0x03003A80, 0x20 bytes; then palette and delay pairs, 1 loops) */
#define BN5_LIB_ANIM_HOLE    0x08073B60u /* ... palette 10's (to 0x03003AA0) */
#define BN5_LIB_ANIM_PULSE   0x08073BA0u /* ... palette 11's (to 0x03003AC0) */
#define BN5_LIB_ANIM_BREATH  0x08073BE0u /* ... palette 12's (to 0x03003AE0); the list also holds 0x08073AB8 and 0x0807FB8C */
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
                                         /* ... 7 index at BN5_LIB_ITEM_KINDS +5: 0x18 the heart, 0x19 a key 0x0848E1F8, 0x25 zenny */
                                         /* ... 0x084907AC, 0x26 BugFrags 0x08490A20, 0x29 Order Points 0x08492EF8, 0x53 a chip */
                                         /* ... 0x0849D508; anim 0, the shadow its own OBJ */
#define BN5_LIB_ICONS_PAL    0x0848E0FCu /* ... the heart's palette (OBJ palette 3) */
#define BN5_LIB_SPRITE_LIST  7           /* the board's sprites' list, */
#define BN5_LIB_OUTLINE      15          /* ... the outline's index: animations 0 a ripple (20, 2, 2, 2, 8 frames, looped), shown */
#define BN5_LIB_OUTLINE_RIPPLE 0         /* ... 6 frames after A while the command window is open over a liberable panel alone; */
#define BN5_LIB_OUTLINE_DELAY 6          /* ... 1 a blink (24 lit, 24 not), yellow; 2 and 3 the same in green; 4 still */
#define BN5_LIB_ICON_OF      5           /* an item kind's icon: BN5_LIB_ITEM_KINDS +5, its index in list 7 (a heart's 24, a key's */
                                         /* ... 25, zenny's 37, BugFrags' 38, Order Points' 41, a chip's 83), animation 0 (its */
                                         /* ... shadow its own; its tiles turn every 24 frames), its own palette */
#define BN5_LIB_ICON_POP     3           /* ... popped over the freed panel: up 8 after this many frames, down 2 after 5 more, held */
#define BN5_LIB_ITEM_SOUND   0x16D       /* the jingle as the Navi's line of an item found begins (its text's, 0x080B2444; BN6's 0x16D the same) */
/* The battle messages' letters ("LIBERATE SUCCESS!", "1 TURN LIBERATE!"): 8x16, two tiles (0x40 bytes) each, laid in
 * 20 columns of five 32x16 affine OBJs on OBJ palette 11 at y 64 */
#define BN5_LIB_LETTERS      0x0873CEF4u /* A-Z from here */
#define BN5_LIB_LETTER_SPACE 0x0873D574u /* " " */
#define BN5_LIB_LETTER_BANG  0x0873D5B4u /* "!" */
#define BN5_LIB_LETTER_CAPS  0x0873D5F4u /* the left cap, then the right */
#define BN5_LIB_BIG_DIGITS   0x0873D974u /* "0"-"9", in the right column of a 16x16 OBJ */
#define BN5_LIB_BANNER_PAL   0x0873E4B4u /* ... their palette */
#define BN5_LIB_BANNER_HOLD  43          /* ... frames at full size, after 8 of opening and before 8 of closing (the scale's pd */
                                         /* ... 768, 640, 512, 384, 256, 208, 224, then 256; out 224, 208, 256, 384 ... 896) */
#define BN5_LIB_MESSAGES     0x0801B8D4u /* the messages' letters' graphics (their pointers) */

/* The phases' banners ("DARKLOID PHASE", "PLAYER PHASE 02"): a band of BG0 across the board under OBJs of letters'
 * pieces on OBJ palette 15, the words sliding in from the right and out to the left (docs/LIBERATION.md) */
#define BN5_LIB_PHASE_WORDS  0x087E5A10u /* their tiles, LZ77 (a u32 size, then 165 tiles), decompressed to RAM 0x0203E400 by */
#define BN5_LIB_PHASE_LOAD   0x08052FF4u /* ... this load list's entry (0x887E5A10: compressed), copied to OBJ tile 719 on */
#define BN5_LIB_PHASE_WORDS_PAL 0x087E5500u /* ... OBJ palette 15 and BG palette 14 for the band: PLAYER PHASE's (its digits'
                                         ... colours 11-15 its own), DARKLOID PHASE's the next (0x087E5520) */
#define BN5_LIB_PHASE_DIGIT  65          /* ... "0" the block's tile 65, a 16x16 OBJ, each digit 4 tiles on */
#define BN5_LIB_BAND_COLOUR  3           /* the band: BG0 filled with one tile of this colour of BN5_LIB_PHASE_WORDS_PAL, shown */
#define BN5_LIB_BAND_TOP     56          /* ... in WIN1 from this row, BN5_LIB_BAND_ROWS high, blended over the board (BLDCNT */
#define BN5_LIB_BAND_ROWS    32          /* ... 0x3F41, BLDALPHA 0x0810: the band whole, the board at half) */
#define BN5_LIB_PHASE_SOUND  0xD6        /* the sound as the words slide in (0x08054DAE, from the routine 0x08054DA2; BN6's 0xD6 the same) */
#define BN5_LIB_SLIDE_STEP   32          /* the words' slide: pixels a frame, from 240 to the right of their place to it, then on */
#define BN5_LIB_WORDS_AT     18          /* ... frames after the band opens they begin */
#define BN5_LIB_WORDS_HOLD   65          /* ... frames they hold in place */
#define BN5_LIB_BANNER_END   104         /* ... frames from the band's opening to its end: open 3, words 18-97, closed at 103 */
/* The L view's panels: BG0 as the band, in WIN1 behind PHASE and TRGT PHASE and WIN0 behind its keys' help */
#define BN5_LIB_LVIEW_PANEL_X 20         /* ... WIN1's left, x 20-107 */
#define BN5_LIB_LVIEW_PANEL_Y 120        /* ... its top, y 120-147 */
#define BN5_LIB_LVIEW_PANEL_W 88         /* ... its width */
#define BN5_LIB_LVIEW_PANEL_H 28         /* ... its height (WIN0, 123-219 by 12-42, behind the keys' help, is not drawn) */

/* The board's camera (its BG1 scroll read frame by frame): a pan to the unit about to act about 20 pixels a frame, then a
 * quarter of what is left, the banner 3 frames after it arrives; the L view's D-pad moves it */
#define BN5_LIB_CAMERA_STEP  4           /* ... this many pixels a frame, */
#define BN5_LIB_CAMERA_FAST  8           /* ... this many with B held ("MoveCam(F)") */

/* The board's units: the guardians BN5's sprite list 6's 83 BigBrute (0x08474848), 84 TinHawk, 85 Bladia */
#define BN5_LIB_GUARD_SPRITE 83
#define BN5_LIB_GUARD_ANIM   0           /* ... the animations they stand in on the board: 0-7 by the eighth faced (1 up-right, */
                                         /* ... 3 down-right, 5 down-left, 7 up-left), a loop of 2 or 4 frames; */
#define BN5_LIB_BRUTE_STRIKE 33          /* ... BigBrute's strike (BeastBreath) 33 + the eighth, a loop of 2 frames, 3 each; */
#define BN5_LIB_HAWK_STRIKE  25          /* ... TinHawk's (HawkAttack) 25 facing left, 26 right, 10 frames a frame; */
#define BN5_LIB_BLADIA_STRIKE 25         /* ... Bladia's (DarkSlash) 25-28 by the eighth, 1 then clockwise, 10, 3, 3, 10 frames */

/* The Darkloids' phase (mission 1 in romlab: MegaMan's place poked beside a BigBrute and beside BlizzardMan): each unit's
 * turn the camera's pan to it and its rest (BN5_LIB_UNIT_HOLD), the Darkloid first, then each guardian in its order */
#define BN5_LIB_UNIT_WORLD_X 0x1C        /* the board's units (BN5_LIB_UNITS): +1 the Navi or kind, +0x1C and +0x20 its world */
#define BN5_LIB_UNIT_WORLD_Y 0x20        /* ... place, 16.16 (MegaMan's poked, the guardians turned on him) */
#define BN5_LIB_WARP_FX      0x080AF3F8u /* a guardian's move, all its steps at once: a warp effect spawned (0x08004508) and */
#define BN5_LIB_WARP_SOUND   0xB4        /* ... this sound, at the panel it leaves and 46 frames on at the one it takes */
#define BN5_LIB_WARP_SPRITE  59          /* ... the effect: sprite list 7's 59 (0x08389E68), a puff and a beam's lines, 2 frames */
#define BN5_LIB_WARP_OUT     2           /* ... each: the guardian's out (puff, lines; animation 0 the large, 4 the small) and */
#define BN5_LIB_WARP_IN      3           /* ... in (lines, puff; 1 and 5), the camera's pan between; the guardian shows 4 frames */
#define BN5_LIB_WARP_SHOWS   4           /* ... into the warp in, */
#define BN5_LIB_APPEAR_SOUND 0x94        /* ... with this sound (0x080B051E) */
#define BN5_LIB_HEAL_FX      0x08055208u /* the Darkloid's heal: effect 0x78 (rising lines) over him, sound 0x8A, his HP up at */
#define BN5_LIB_HEAL_SOUND   0x8A        /* ... once, after DARKLOID PHASE where dark panels stand round him */
#define BN5_LIB_REACH_BLINK  0x080529F0u /* a unit's reach in reach of a Navi: its panels in look 0x0D (red, BG palette 7), lit 9 */
#define BN5_LIB_REACH_SOUND  0x91        /* ... frames, dark 7, lit 9, dark 7, then lit, this sound as each lights; */
#define BN5_LOOK_REACH       0x0D        /* ... the look, */
#define BN5_LIB_REACH_ON     9           /* ... frames lit, */
#define BN5_LIB_REACH_OFF    7           /* ... frames dark, twice, */
#define BN5_LIB_REACH_LINE   68          /* ... frames from the first light to the unit's line (its attack's name in a chat box: */
                                         /* ... the Darkloid's with his mugshot, a guardian's with none) */
#define BN5_LIB_HARM         0x08053EBAu /* a blow on a Navi: sound 0x6B where it harms him, 0x6E where nothing does (0x08053EFC) */
#define BN5_LIB_HIT_SOUND    0x6B        /* ... the blow's sound where it harms, */
#define BN5_LIB_NO_HIT_SOUND 0x6E        /* ... where nothing does (a BonusPanel's shield) */
#define BN5_LIB_FLAME_SPRITE 56          /* BigBrute's BeastBreath on its Navi: sprite list 7's 56 (0x0833A1B4), animation 0 */
                                         /* ... rising (3 x 2 frames), 1 burning (looped), 2 falling (3 x 1) */
#define BN5_LIB_DIGITS       0x086F7DECu /* the units' HP under them and a blow's harm over its Navi: digits 8x16, 0x40 bytes each */
#define BN5_LIB_DIGITS_PAL   0x086CCD24u /* ... white (an HP, OBJ palette 14), */
#define BN5_LIB_HARM_PAL     0x087E4678u /* ... yellow (a harm, OBJ palette 15), its top 29 pixels over the Navi's feet, centred */

/* The units' HP on the board (BN5_LIB_UNITS +0x14 its HP, +0x16 its max, +0x18 the HP shown), drawn each frame after the
 * board's (0x08004FF0) for every unit in play with HP shown: the team's but the Navi in play's (his in the HP box) while
 * BN5_LIB_STATE +0x18 lacks 0x100, the Darkloid's and guardians' while it lacks 0x80 (a command's animation sets both) */
#define BN5_LIB_TEAM_HP      0x080533A0u /* the team's digits under them, */
#define BN5_LIB_ENEMY_HP     0x0805341Au /* ... the Darkloid's and the guardians', */
#define BN5_LIB_HP_DIGITS    0x08053482u /* ... each drawn: BN5_LIB_DIGITS' digit d at OBJ tile 664 + 2d, an 8x16 OBJ (priority 2) a */
#define BN5_LIB_HP_STEP      8           /* ... digit, this far apart, the number centred on the unit's place on the picture, */
                                         /* ... its top there (0x08003498: x + y + 120 across, (y - x) / 2 - z + 82 down) */
#define BN5_LIB_HP_ROLL      4           /* ... the HP shown stepped this much a frame toward its HP (0x080533D8) */
#define BN5_LIB_HP_TILE      664         /* ... (OBJ palette 14, BN5_LIB_DIGITS_PAL) */
#define BN5_LIB_DARKLOID_HEAL 0x080560ACu /* the Darkloid's turn's heal, as DARKLOID PHASE's band closes: where his HP is below */
#define BN5_LIB_HEAL         0x08053D04u /* ... his max and dark panels stand round him, his max / 10 for each (weighed by */
#define BN5_LIB_DARK_WEIGHT  0x08050334u /* ... this table, a byte per panel kind: 1 for dark, an item's, a DarkHole and his */
                                         /* ... ring, 0 else; 0x08051E7C sums his eight), his HP and the HP shown up at once, */
                                         /* ... his max at most; then BN5_LIB_HEAL_FX (variant 2) and its sound, */
#define BN5_LIB_HEAL_WAIT    20          /* ... and this many frames before his turn goes on (his reach lit 21 frames later) */
#define BN5_LIB_HEAL_SPRITE  120         /* ... the effect: sprite list 7's 120 (0x082FF3AC), rising lines, 15 frames, at his place, */
#define BN5_LIB_HEAL_ANIM    0           /* ... this animation (its first frame's 32x32 OBJ 16 left of and 24 over his place), */
#define BN5_LIB_HEAL_PAL     2           /* ... in the palette its variant names, 2 (a Navi's rest's 0): the frame's palette block's */
                                         /* ... third, past the one it counts (0x08300268, loaded in OBJ palette 2) */
#define BN5_LIB_PASS_HEAL    0x08053CBEu /* a Navi's rest (Pass) and the like: his max / 10 times a byte of 0x08053CF8 (2, 2, 3, 3, */
                                         /* ... 4, 4, 5, 5, 5) by the event flags 0x300-0x308 set from the first (0x08052EE8), */
                                         /* ... his max at most: a fifth on mission 1's state, none of them set */
/* A board unit's HP through its battle: as the battle's enemy marked by its record (bit 2 of its entry's first byte) spawns
 * (0x08006862) it takes the HP and max of the unit fought (BN5_LIB_STATE +8 = 2, +9 its index; 0x08053B9E); as the battle
 * ends (0x08007120) each such enemy's HP is written back into it (0x08009050, 0x08053BF4), so a unit not deleted keeps on the
 * board what its battle left (one left at 0 by a battle not won given back a quarter of its max, 0x0804D2EC) */
#define BN5_LIB_FOE_HP       0x08053B9Eu /* the fought unit's HP and max for its battle's marked enemy, */
#define BN5_LIB_FOE_HP_BACK  0x08009050u /* ... and its HP back into the unit as the battle ends */

/* The L view's arrows and cross (OBJ palette 12, BN5_LIB_HUD_PAL; read in its OAM) */
#define BN5_LIB_LVIEW_ARROWS 0x087E4B64u /* eight 8x8 tiles, up, up-right, right, down-right, down, down-left, left, up-left, */
#define BN5_LIB_LVIEW_BLINK  0x100       /* ... and their second look this far on, shown 8 frames of every 32 */
#define BN5_LIB_LVIEW_CROSS  0x087E4F24u /* the cross at the camera's middle, a 16x16 OBJ at (112, 72) */
#define BN5_LIB_UNIT_HOLD    15          /* frames the camera rests on a unit in the Darkloids' phase before its move, after its pan */

/* The board's music (BN5's song table 0x08155BF4, the BGM's player 31 at 0x02011470: +0 the song's header) */
#define BN5_SONG_LIBERATION  0x22        /* the board's own, through both phases (its clock runs on), BN5_LIB_STATE +0 = 8 */
#define BN5_SONG_MISSION_OPEN 0x1C       /* the mission's opening (the board's routine 1, 0x0804BEE0, its tour of the board) */

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
#define BN5_LIB_PHASE_DIGITS 0x087E4E04u /* ... here (palette 12) "1"-"9", its corner arrows from 0x087E4CC4 */
#define BN5_LIB_PHASE_ZERO   0x087E4DE4u /* ... and its "0", the arrows' tenth tile */
#define BN5_LIB_PHASE_PAL    0x087E4658u /* ... the words' palette: "PHASE" tiles 0-4 at (54, 124) and its number at (97, 124); */
                                         /* ... "TRGT" tiles 5-8 at (22, 136), "PHASE" after it, its number at (97, 136) */
#define BN5_LIB_WINDOW_PAL   0x087E4FA4u /* OBJ palette 15: the command window's words, 21 OBJs of 16x16 at x 128-239, y 114-157, */
                                         /* ... drawn into RAM (0x0203A400) by BN5's code each time it opens, a line 14 down */
#define BN5_LIB_WINDOW_TILES 0x087E4FC4u /* ... its frame on BG0: nine tiles (top left, top, top right, left, body, right, bottom */
#define BN5_LIB_WINDOW_BG_PAL 0x087E5144u /* ... left, bottom, bottom right), 16 x 7 tiles at (112, 104), BG palette 14 */
#define BN5_LIB_CURSOR_TILES 0x086BA484u /* ... its cursor, 16x16 at (112, 112), OBJ tile 704 */
#define BN5_LIB_CURSOR_PAL   0x086CCD24u /* ... OBJ palette 14 */
#define BN5_LIB_BACKDROP_PAL 0x0861BF20u /* BG palette 0 in a mission: the map's own animated backdrop (its tiles rebuilt in RAM at */
                                         /* ... 0x0200E960) in purple; map 0x86:00's own palette 0 otherwise 0x086203CC */
#define BN5_LIB_BACKDROP     0x080741B8u /* ... the mission's BGAnimData record that loads it (0x0861BF1C: a size word, then these), */
                                         /* ... map 0x86:00's own tiles and map: group 0x86's LoadBGAnim (0x08074098) takes its records */
                                         /* ... from 0x0807416C while BN5_LIB_ON_MAP, else 0x08074194 (0x080741D4, palette 0x086203C8), */
                                         /* ... and its LoadGFXAnims (0x08073E2A) its lists from 0x08073E70, else 0x08073E48 */

/* The missions */
#define BN5_LIB_MISSIONS     0x0804F628u /* the nine missions' records, BN5_LIB_MISSION_SIZE each: +0 the map (group, number), */
#define BN5_LIB_MISSION_SIZE 0x5C        /* ... +6 and +7 the panels' origin (s8), +0xA the par, +0xB the Order Points at the start, */
#define BN5_LIB_MISSION_COUNT 9          /* ... +0xC twenty pointers (BN5_LIB_PTR_*) */
#define BN5_LIB_MAP_GROUP    0x86        /* the missions' maps' group (each record's +0: mission n's map 0x86:n-1) */
#define BN5_LIB_PTR_PANELS   1           /* the panels: u16 kind (BN5_PANEL_*), u8 x, u8 y; a kind's low byte 0xFF ends them */
#define BN5_LIB_PTR_HOLES    2           /* the DarkHoles' panels in their order, x and y a byte each */
#define BN5_LIB_PTR_TEAM     3           /* the team: 20 bytes a Navi (+5 its id, BN5_NAVI_*; +8, +0xA its place in world units) */
#define BN5_LIB_PTR_ENEMIES  4           /* the Darkloid, then the guardians: 20 bytes each (+0 its DarkHole, 0xFF none; +5 its kind; +8 x, +9 y) */
#define BN5_LIB_PTR_BONUS    8           /* the BonusPanels: x, y, u16, five outcomes' records; a u16 0 ends them */
#define BN5_LIB_PTR_TRAPS    9           /* the traps: kind (0 damage, its amount times 10; 1 paralysis), amount, x, y; 0xFFFFFFFF ends them */
#define BN5_LIB_PTR_ITEMS    10          /* the item panels: kind (BN5_ITEM_*), code, u16 chip, u16 amount, x, y; 0xFFFFFFFF ends them */
#define BN5_LIB_PTR_REWARDS  19          /* the rewards: kind (0 a chip, 1 zenny), code, phases from, phases to, u16 chip or zenny */
#define BN5_LIB_NAVI_HP      0x0804F968u /* per team Navi: u16 his id, then his HP in each of the nine missions */
#define BN5_LIB_ITEM_KINDS   0x08051924u /* per item kind 8 bytes: +2 the amount's multiplier (hearts 10, zenny 100), +5 its icon */
                                         /* ... (BN5_LIB_ICON_OF); handlers 0x0804D8B4 */

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
#define BN5_LIB_NAVI_MEGAMAN     0x00  /* MegaMan: his own HP */
#define BN5_LIB_NAVI_COLONEL     0x07  /* Colonel: ScrenDiv */
#define BN5_LIB_NAVI_SHADOWMAN   0x08  /* ShadowMan: SneakAttack */
#define BN5_LIB_NAVI_NUMBERMAN   0x09  /* NumberMan: NumberCheck */
#define BN5_LIB_NAVI_TOMAHAWKMAN 0x0A  /* TomahawkMan: TomahawkSwing */
#define BN5_LIB_NAVI_KNIGHTMAN   0x0B  /* KnightMan: no command of his own */
#define BN5_LIB_NAVI_TOADMAN     0x0C  /* ToadMan: LifeMelody */

/* A liberation battle */
#define BN5_BATTLE_TYPE      0x0802B3E0u /* the battle's record's +3 (BattleSettings' BattleType): */
#define BN5_BATTLE_LIBERATION 1          /* ... 1 a liberation battle */
#define BN5_BATTLE_TURN      0x08009016u /* BattleState +7: the turn, one more as each Custom screen opens (1 at the first) */
#define BN5_LIB_TURNS_UP     0x08009190u /* true once BattleState +7 reaches 3, read by BN5_LIB_TURN_BANNER alone */
#define BN5_LIB_TURN_BANNER  0x08007954u /* as each turn's fight starts: FINAL TURN! (message 0x10) where BN5_LIB_TURNS_UP, else TURN n START! (0x0C, n the turn) */
#define BN5_LIB_TIME_ASK     0x08008BD2u /* the Custom screen asked for (BattleState +0x32 bit 2): below the third turn it opens, from it (+7 >= 3) */
#define BN5_LIB_TIME_UP      0x08007C24u /* ... the loop's phase 0x18: TIME UP! (0x54), 30 frames, LIBERATE FAILED! (0x5C), the battle over */
#define BN5_LIB_WIN_BANNER   0x08007A50u /* the win's first step: 1 TURN LIBERATE! (0x90, number 1) where +7 is 1, else LIBERATE SUCCESS! (0x58) */
#define BN5_BATTLE_MESSAGES  0x0801B80Cu /* the battle messages' table, a word each (a message's number is its offset), read by 0x0801B02A */
#define BN5_LIB_LATE_DOUBLE  0x0800901Eu /* true in a liberation battle from its eleventh turn: */
#define BN5_LIB_DOUBLE_HIT   0x0800C7CCu /* ... an object's hits this frame summed (its collision data's five) and doubled */

#endif

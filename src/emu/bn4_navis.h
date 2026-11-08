/* Battle Network 4: Red Sun (USA)'s Navis: their types, routines and the
 * engine's they share (docs/rom/bn4.md, BN4's Navis; docs/multirom/bn4.md,
 * Phase 2), apart from bn4.h, which includes it. */
#ifndef CW_BN4_NAVIS_H
#define CW_BN4_NAVIS_H

/* BN4's Navis (docs/rom/bn4.md, BN4's Navis; docs/multirom/bn4-navis.md): each a first-kind type of its own, as its
 * families are, its routine's jump table a family's (BN4_TYPE_INIT, _UPDATE, _DYING); its update the Navi pre-step and
 * the Navi state runner with its state table, BN4's six generic Navi states first, its own from 6. Ten (Red Sun's and
 * Blue Moon's Soul Navis) take their side's AI data at their init and name MegaMan's states in their table (their
 * attacks); the others keep their variables in the object, as the families do */
#define BN4_NAVI_FIRST_ID 128         /* Roll's V1: a Navi's four enemy ids in a row from here (V1, V2, V3, SP; BN4_ENEMY_VERSION 0-3) */
#define BN4_NAVI_SPRITES  8           /* a Navi's BN4_ENEMY_KIND: its battle sprite in list 2, its BN4_ENEMY_FAMILY the number (GutsMan 2) */
#define BN4_NAVI_TYPE_ROLL       0x1D /* Roll's (ids 128-131) first-kind type (BN4_ENEMY_TYPE of its records), Red Sun's Soul Navis first */
#define BN4_NAVI_TYPE_GUTSMAN    0x0A /* GutsMan's (132-135) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_WINDMAN    0x30 /* WindMan's (136-139) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_SEARCHMAN  0x2B /* SearchMan's (140-143) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_FIREMAN    0x0C /* FireMan's (144-147) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_THUNDERMAN 0x32 /* ThunderMan's (148-151) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_METALMAN   0x3F /* MetalMan's (160-163) first-kind type (BN4_ENEMY_TYPE of its records), Blue Moon's Soul Navis, fought in Red Sun too */
#define BN4_NAVI_TYPE_JUNKMAN    0x47 /* JunkMan's (164-167) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_AQUAMAN    0x0D /* AquaMan's (168-171) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_WOODMAN    0x46 /* WoodMan's (172-175) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_TOPMAN     0x09 /* TopMan's (176-179) first-kind type (BN4_ENEMY_TYPE of its records), the others: no AI data, their states all their own */
#define BN4_NAVI_TYPE_BURNERMAN  0x22 /* BurnerMan's (184-187) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_COLDMAN    0x4A /* ColdMan's (188-191) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_SPARKMAN   0x0B /* SparkMan's (192-195) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_LASERMAN   0x29 /* LaserMan's (208-211) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_KENDOMAN   0x2D /* KendoMan's (212-215) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_VIDEOMAN   0x2F /* VideoMan's (216-219) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_TYPE_DUO        0x2A /* Duo's (224-227) first-kind type (BN4_ENEMY_TYPE of its records) */
#define BN4_NAVI_PRESTEP  0x080135A8u /* a Navi's update's first call (BN4_PRESTEP's), Duo's BN4_NAVI_PRESTEP_DUO */
#define BN4_NAVI_PRESTEP_DUO 0x0801374Au /* ... Duo's: the same chores in 41 instructions to its 65 */
#define BN4_NAVI_RUNNER   0x08013DB4u /* its second: r0 the Navi's state table, run by CurAction (+9) with r6 its collision data and r7 its
                                       * AI data (BN4_STATE_RUNNER's work for a Navi; BN6's status dispatcher 0x0801AF44 another body) */
#define BN4_NAVI_DYING    0x08010F61u /* a Navi's jump table's +8 (WindMan's BN4_DYING, the families') */
/* BN4's generic Navi states 0-5, every Navi's table's first six where it does not put its own (BN4_STATE_GENERIC_* the
 * families'); 2 is the families' 2, 5 BN6's and BN5's Navi state 5 by body, the others BN4's own */
#define BN4_NAVI_GENERIC_0 0x08010D31u
#define BN4_NAVI_GENERIC_1 0x08010FA7u
#define BN4_NAVI_GENERIC_2 0x08010D4Du
#define BN4_NAVI_GENERIC_3 0x08010E89u
#define BN4_NAVI_GENERIC_4 0x08010F09u
#define BN4_NAVI_GENERIC_5 0x08010FAFu
#define BN4_MEGAMAN       0x080EAEA8u /* MegaMan's routine (the first kind's type 0 calls it): his jump table, his update with his state table */
#define BN4_PLAYER_STATES 0x080EAEFCu /* MegaMan's state table (his update's literal): his generic 0-5, his own from 6; 44-111 among them
                                       * the Soul Navis' attacks, a Navi's table naming them after its own (GutsMan's 7-10 his 67-70) */
#define BN4_PLAYER_STATE_COUNT 116  /* ... how many (the table ends where a word is no Thumb routine's) */
#define BN4_AI_TAKE       0x0800C09Au /* a Soul Navi's init: its side's AI data (BN4_AI_OF_SIDE by +0x16) into its +0x58 */
#define BN4_AI_OF_SIDE    0x0800BFF4u /* r0 a side: BN4_AI_DATA + side * BN4_AI_DATA_SIZE, so one Navi with AI data a side */
#define BN4_AI_NAVI       0x64        /* AI data: the Navi's own 12 bytes before the attack's variables, its decision's sub-state first
                                       * (BN6's +0x80, 0x20 bytes there: HeatMan's decision reads its sub-state so) */
#define BN4_SET_ACTION    0x0800C9ACu /* r0 a state: CurAction set, its sub-state and the attack's variables' first u16 cleared */
#define BN4_ATTACK_END    0x0800CA28u /* a Soul Navi's attack's end: AI data +0x44's bits 0x3F and +9 to +0xB, +0x30, +0x32 cleared, CurAction 6;
                                       * 0x0800C9FC the same with the attack's +5 kept at +0x3A, 0x0800CA4A the CurAction alone */
/* (their spawns: BN4_SPAWN_1, BN4_SPAWN_3 and BN4_SPAWN_4 above, r0 the type, r1 to r3 the place, r4 the parameters) */
#define BN4_ENEMY_RECORD  0x08011878u /* r0 an enemy id: its BN4_ENEMY_STATS record */
#define BN4_SPAWN_ENEMY   0x080F00F4u /* r0 an enemy id: its type spawned on a free panel of the side (Roll's state 11, MegaMan's 48) */
#define BN4_PANEL_CRACK   0x08009AECu /* r0, r1 a panel: cracked (its type 3), or broken (1) where it was cracked, sound 0x95 (ColdMan, Duo;
                                       * GutsMan's moved objects call it too, tools/xnavirecipe_bn4.py) */
/* GutsMan, the first BN4 Navi derived (tools/xnavirecipe_bn4.py; docs/multirom/bn4-navis.md, GutsMan derived): found by his
 * type, each checked by the generator against his ROM */
#define BN4_GUTSMAN       0x080B73C8u /* his type's routine (BN4_T1_TYPES[BN4_NAVI_TYPE_GUTSMAN]): jump table init 0x080B73ED, update
                                       * 0x080B7443, dying BN4_NAVI_DYING; his block to 0x080B7734 */
#define BN4_GUTSMAN_STATES 0x080B7458u /* his state table (his update's literal for BN4_NAVI_RUNNER): BN4's generic Navi 0-5, his
                                       * decision 6 (0x080B7485), MegaMan's 67-70 as his 7-10 */
#define BN4_GUTSMAN_TABLES 0x080B73ACu /* his tables of bytes below his routine (his states' literals 0x080B73AC-0x080B73C4), past the
                                       * previous type's pool (0x080B73A0-0x080B73AB) */
/* The engine's routines GutsMan's states reach (tools/xnavirecipe_bn4.py's NAVI_COPIED and bn4_twins.py's pairs; docs/rom/bn4.md,
 * GutsMan derived) */
#define BN4_NAVI_PALETTE  0x0800B950u /* r0 a number: its byte of BN4_NAVI_PALETTE_NUMBERS times the version's (+4) of BN4_NAVI_PALETTE_VERSIONS, a
                                       * Navi's init's palette (GutsMan's 2 and FireMan's 5: V1-V3 0, SP 2; BN6's step 0 gives 0 to all) */
#define BN4_ATTACK_END_CLEAR 0x0800BDAAu /* BN4_ATTACK_END's: AI data +9, +0xA, +0xB, +0x30 and +0x32 zeroed */
#define BN4_AI_REQUESTS_CLEAR 0x0800BFC0u /* r0 bits: cleared in AI data +0x44 (BN4_AI_REQUESTS; BN4_ATTACK_END's 0x3F) */
#define BN4_SET_STATUS_GIVEN 0x08013212u /* r0 a status: the collision data's BN4_COLL_STATUS_GIVEN (BN6's twin 0x0801A258, +0x10) */
#define BN4_ON_FLOOR      0x08009ECCu /* its own panel (+0x12, +0x13) a floor: its flags' 0x10 (BN6's twin 0x0800CCA6, flags at +0x14) */
#define BN4_FREE_PANELS   0x0800A244u /* r0 a side, r2 and r3 the flags wanted and refused, r7 a buffer: its free panels, a byte each
                                       * (column | row << 4), r0 their count; its panel's side read at BN4_PANEL_ALLIANCE */
#define BN4_RANDOM_FREE_PANEL 0x0800FAD0u /* one of BN4_FREE_PANELS' for its side (+0x16) by the RNG (0x08001B16): r0 column, r1 row */
#define BN4_RANDOM_PANEL  0x0800F734u /* a panel of its side by BN4_RANDOM_PANEL_MASKS and the RNG, else its own: the callback GutsMan's
                                       * decision hands his step (MegaMan's 67) in the attack's variables' +0x1C */
#define BN4_RANDOM_PANEL_MASKS 0x0800F770u /* ... its masks by side, two words a side (0x10, 0x0F8800A0; 0x30, 0x0F880080), 16 bytes */
#define BN4_T3_1E_KINDS   0x080D0A60u /* the third kind's 0x1E's kinds (GutsMan's objects), 6 bytes each to its code at 0x080D0A74: its
                                       * speed, its +0x64, animation, collision's, the sound its init plays (+4: 0xA5, 0xA6, 0xA6), the
                                       * status it gives; read by its init by +4 (0x080D0AAE) */
#define BN4_NAVI_PALETTE_NUMBERS  0x0800B9B1u /* BN4_NAVI_PALETTE's byte by its r0 (GutsMan's 2 and FireMan's 5 give 1), ... */
#define BN4_NAVI_PALETTE_VERSIONS 0x0800B9ACu /* ... times its byte by the version (+4): 0, 0, 0, 2 (V1-SP) */
/* FireMan, the second BN4 Navi derived (tools/xnavirecipe_bn4.py; docs/multirom/bn4-navis.md, FireMan built) */
#define BN4_FIREMAN       0x080B7DB4u /* his type's routine (BN4_T1_TYPES[BN4_NAVI_TYPE_FIREMAN]): its jump table alone, no after-step;
                                       * init 0x080B7DD5 (its tail spawns his companion and runs his update), update 0x080B7E3F */
#define BN4_FIREMAN_STATES 0x080B7E5Cu /* his state table: BN4's generic Navi 0-2, his own 3-5 (his companion told, then BN4's generic),
                                       * his decision 6 (0x080B7EC9), MegaMan's 72-76 as his 7-11 */
#define BN4_FIREMAN_POST  0x080B81B0u /* his update's call after the runner: his companion told (BN4_FIRE_COMPANION_TELL) where his
                                       * animation changes (+0x10 against +0x11) */
#define BN4_FIREMAN_TABLES 0x080B7D98u /* his tables of bytes below his routine, to BN4_FIREMAN */
#define BN4_FIRE_COMPANION_TYPE 0x57  /* his companion: a first-kind type (no enemy) that draws list 4's 8 on him, his animation's + its +7 */
#define BN4_FIRE_COMPANION 0x080CC3D8u /* ... its routine; its modes +9: 0 and 4 wait on the battle's appear (4 through BN4's tile buffer
                                       * 0x02037810), 8 drawn; 8 where the battle state's +0x32 bit 0 is set */
#define BN4_FIRE_COMPANION_SPAWN 0x080CC5F4u /* ... spawned (r4 its word: list offset, number), its parent at its +0x4C, back r0 */
#define BN4_FIRE_COMPANION_TELL 0x080CC61Au /* r0 the companion: its +0x11 0xFF, its animation set up again */
#define BN4_FIREMAN_COMPANION 0x74    /* FireMan's object: his companion */
#define BN4_ATTACK_END_KEEP 0x0800C9FCu /* BN4_ATTACK_END's kin: the attack's +5 kept at AI data +0x3A first (FireMan's, WoodMan's) */
#define BN4_SPRITE_STEP   0x08014446u /* unless paused: a new animation set up (+0x11 its +0x10), else the sprite updated; BN6's nearest
                                       * 0x0801BBF4 updates it in the frame of the set-up too */
#define BN4_PAUSE_BYTE    0x080078ECu /* the battle's pause byte ([eToolkit +0x40] +9) in r0 (BN6_PAUSE_TEST's twin, flags unset) */
#define BN4_T3_1A_KINDS   0x080D013Cu /* the third kind's 0x1A's kinds (FireMan's), 6 bytes each, its sprite's list 4 number first */
#define BN4_FIREMAN_MASKS 0x080F1DB4u /* panels' masks by side his MegaMan state 0x080F1D98 reads (BN4_RANDOM_PANEL_MASKS' words) */
/* FireMan proved (docs/multirom/bn4-navis.md, FireMan built: fireman_v1) */
#define BN4_FIRE_COMPANION_SPRITE 0x080CC5D0u /* his companion's modes' tail: its parent's sprite fields copied, then, where its +6 is 0,
                                       * the after-step (0x080143FC, BN6's twin BN6_AFTER_STEP) unless time is stopped (BN4_TIME_STOP) */
#define BN4_FIRE_FLAME    0x080CE2D8u /* the third kind's 0x09 (his flame): its init (0x080CE309) gives its palette (BN4_T3_PALETTE) to
                                       * its sprite (0x080CE358, BN4_SPRITE_PALETTE_SET) */
#define BN4_FIRE_FLAME_SPAWN 0x080CE49Cu /* ... spawned: r0-r2 its column, row and element (+0x12, +0x13, +0x17), r7 its palette, r6 +0x2C */
#define BN4_T3_PALETTE    0x0E        /* a third-kind object's palette, its sprite's +4: BN6's +0xE is its element (BN4's +0x17), so a moved
                                       * object keeps it at BN6_T3_PALETTE (tools/xrecipe_bn4.py palette_imms) */
#define BN4_SPRITE_PALETTE_SET 0x08002DECu /* r0 into the sprite's +4 (BN6_SPRITE_PALETTE_SET, the same body) */
/* ThunderMan, the second (tools/xnavirecipe_bn4.py, his companion read by tools/xnavirecipe_bn4_init.py, his objects' needs
 * by tools/xnavirecipe_bn4_objects.py; docs/multirom/bn4-navis.md, ThunderMan built) */
#define BN4_THUNDERMAN    0x080C37F4u /* his type's routine: jump table init 0x080C3819, update 0x080C388D, dying BN4_NAVI_DYING; his block
                                       * to 0x080C3AF0; his init runs his update once at its end (0x080C387C), BN6's actor init none */
#define BN4_THUNDERMAN_STATES 0x080C38A4u /* his state table: BN4's generic Navi 0 and 2-5, his own 1 (0x080C38CD: a call of BN4's generic
                                       * 1 alone), his decision 6 (0x080C38D5), MegaMan's 85-87 as his 7-9 */
#define BN4_THUNDERMAN_COMPANION 0x080C3D84u /* r0 its +0x2C, r1 its version: the first-kind 0x33 spawned (its +0x4C the Navi), called
                                       * by his init after the collision setup (0x080C3874) with r0 0x14 << 16 | its byte of
                                       * BN4_THUNDERMAN_DAMAGE by his version (+4), r1 his version */
#define BN4_THUNDERMAN_DAMAGE 0x080C3888u /* ... 4 bytes: 20, 40, 60, 80 by version */
#define BN4_THUNDERCLOUDS 0x080C3AF0u /* the first kind's 0x33 (his companion): jump table init 0x080C3B15, update 0x080C3B41, dying
                                       * BN4_DYING; its variables from +0x70; it spawns the third kind's 0x6A (0x080DAC24) */
#define BN4_THUNDERCLOUD  0x080DAA90u /* the third kind's 0x6A (the companion's), its block to 0x080DAC54 */
/* The engine's routines ThunderMan's code reaches that the families' pairs lack */
#define BN4_CAN_MOVE_KIN  0x0800AD54u /* BN4_CAN_MOVE's body (0x0800AD2A) with the flags' mask 0x1040 (its literal 0x0800B12C) */
#define BN4_PANEL_BREAK   0x08009BF0u /* r0, r1 a panel: broken (type 1) where no occupant's flag (0x0F0A0080) is set, else cracked
                                       * (type 3, 0 back), sound 0x95; BN6's twin 0x0800CA8C (BN6_PANEL_BREAK, audited) */
#define BN4_SIDE_COUNT_DOWN 0x08007990u /* BattleState +4 + its side less one (BN4_DYING's); BN6's twin 0x0800A10C, the same body */
#define BN4_FLAG_1187_TEST 0x080406A4u /* its event flag 0x1187 (0x0802509C on eToolkit +0x48's flags); paired by the symbol map with
                                       * BN6's battle_isPaused (0x0800A03C), audited as an interim */
#define BN4_PAUSED_BLINK  0x0801466Au /* an object blinked (+0 bit 2 by the clock) while [BattleState +0x3C]+6 has 0x20; called while
                                       * the battle is not running; BN6's place a nullsub (0x0801BE28), audited as an interim */
#define BN4_NUDGE_X       0x080D97F8u /* r0: its +0x36 moved by r0 toward its side's front (+0x16); a leaf of the third kind's 0x69 */

/* WindMan, the third BN4 Navi derived (tools/xnavirecipe_bn4.py, xnavirecipe_bn4_init.py; docs/multirom/bn4-navis.md, WindMan
 * under way): found by his type, each checked by the generator against his ROM */
#define BN4_WINDMAN       0x080C3234u /* his type's routine (BN4_T1_TYPES[BN4_NAVI_TYPE_WINDMAN]): jump table init 0x080C3259, update
                                       * 0x080C32DF, dying BN4_DYING (the families'); his block to 0x080C355C */
#define BN4_WINDMAN_STATES 0x080C32F4u /* his state table: BN4's generic Navi 0-2 and 5, his own 3 and 4 (0x080C3321, 0x080C333D: his
                                       * wind, +0x60, told to end, then the generic flinch and paralysis), his decision 6 (0x080C3359),
                                       * MegaMan's 63-66 as his 7-10 */
#define BN4_WINDMAN_WIND  0x60        /* his object's wind (the fourth kind's 0x41 his decision spawns where none is); +0x64 his second
                                       * companion; each companion's +0x60 the address of his +0x60 */
#define BN4_T1_COMPANION  0x5C        /* the first kind's type a Navi's companions are (WindMan's two, kinds 2 and 3; AquaMan's): */
#define BN4_COMPANION     0x080CCC9Cu /* ... its routine: init by its kind's sprite (BN4_COMPANION_KINDS), its owner (+0x4C) followed */
#define BN4_COMPANION_SPAWN 0x080CCF34u /* ... its spawner: r4 its kind, its owner r5, its side his */
#define BN4_COMPANION_KINDS 0x080CCC94u /* ... its kinds' sprites, two bytes a kind (list offset, number): 4:0x06, 4:0x0C, 4:0x28, 4:0x29 */
#define BN4_COMPANION_AURA 0x080CCDC8u /* ... its state 4's first step: drawn again through BN4's tile buffer 0x02037810 (its init takes it
                                       * where the battle state's +0x32 lacks bit 0 and its settings' +6 has 0x20) */
#define BN4_SET_FLAG      0x08012F82u /* r0 ORed into the collision data's +0x64 (BN6's twin 0x0801A152, LAYOUT): WindMan's init's 0x30 */
#define BN4_WIND_SPAWN    0x080E68F8u /* a side's wind spawned (the fourth kind's 0x41) where the side's slot holds none of level 1 on */
#define BN4_WIND_END      0x080E69BCu /* r0 a wind: told to end (+8 8, +0x60 0) */
#define BN4_WIND_SLOTS    0x88        /* BattleState +0x88 + 4 x side: a side's wind, its level 8 past it (0x080E69C6, 0x080E69E8) */
#define BN4_WIND_TYPE     0x080E6820u /* the wind's routine (BN4_T4_TYPES[0x41]): init 0x080E6840 (sound 0x121), its state 4 0x080E6858
                                       * runs its step unasked (BN6_WIND_HOLD's twin), which spawns its gust (bl BN4_GUST_SPAWN at 0x080E68C2) */
#define BN4_GUST_SPAWN    0x080D68ACu /* a wind's gust spawned (the third kind's 0x49, its routine 0x080D6724) */
#define BN4_GUST_INIT     0x080D6748u /* the gust's init: sprite_load of list 0xC's 0x2E with the r0 its +0 flags left (0x080D6750-0x080D675C),
                                       * the sprite state's +3 (BN6_GUST_INIT passes 0x80) */
#define BN4_STEP_BEGIN    0x0801393Eu /* an object's step onto a panel begun: collision flags 0x100 and 4 (0x0801398C, 0x08013992), and no
                                       * byte of the object's (BN6_STEP_BEGIN's twin, which sets its +0x0F) */

/* AquaMan, derived after FireMan (tools/xnavirecipe_bn4.py; docs/multirom/bn4-navis.md, AquaMan built): found by his type, each
 * checked by the generator against his ROM */
#define BN4_AQUAMAN       0x080B820Cu /* his type's routine (BN4_T1_TYPES[BN4_NAVI_TYPE_AQUAMAN]): jump table 0x080B8224 (init
                                       * 0x080B8231, update 0x080B8293, dying BN4_NAVI_DYING), then the after-step; his block to 0x080B8520 */
#define BN4_AQUAMAN_STATES 0x080B82A8u /* his state table: BN4's generic Navi 0-5, his decision 6 (0x080B82D5), MegaMan's 81-84 as his 7-10 */
#define BN4_AQUAMAN_TABLES 0x080B81F8u /* his tables below his routine, to BN4_AQUAMAN */
#define BN4_AQUAMAN_PANEL 0x06        /* his object's +6 (u16): his panel, which his init keeps there (0x080B8264) and his decision reads as
                                       * his last (0x080B833E) and writes anew (0x080B8344); BN6's spawn leaves a Navi's 0 */
#define BN4_AQUAMAN_COMPANION 0x74    /* his object's companion (the first kind's 0x5C, BN4_T1_COMPANION, kind 1) */
#define BN4_COMPANION_UPDATE 0x080CCD3Cu /* the 0x5C's update: its owner's place, flag 2 and sprite fields copied, then its mode (+9) */
#define BN4_COMPANION_MODE8 0x080CCE8Cu /* ... its mode 8: its kind's routine (+4), then the after-step (0x080143FC) unless time is stopped */
#define BN4_COMPANION_KIND1 0x080CCECCu /* ... kind 1's (AquaMan's): +0xC set and flag 2 cleared while its owner's +0x10 is not 0, then
                                         * +0xC cleared and +0x11 0xFF (its animation set up again by the after-step) once it is */
#define BN4_EFFECT_PATTERN 0x0801457Eu /* effects spawned on the panels around (r0, r1) by a pattern (r3, BN4_EFFECT_PATTERNS'), r2 the
                                        * effect (each a call of BN4_SPAWN_EFFECT with r4 its r2), r4 the side: BN6_EFFECT_PATTERN the same body */
#define BN4_EFFECT_PATTERNS 0x080125B0u /* ... its patterns' pointers (its literal at 0x080146A0) */

#endif

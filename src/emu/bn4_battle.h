/* Battle Network 4: Red Sun (USA)'s battle structures past bn4.h's: its
 * panels' fields its field init writes (docs/rom/bn4.md, The battle's
 * panels against BN6's) and a won battle's reward (Its rewards), apart from bn4.h,
 * which includes it. */
#ifndef CW_BN4_BATTLE_H
#define CW_BN4_BATTLE_H

/* A panel's (BN4_PANEL_SIZE), as BN4's field init writes them (0x08009120: the 40 panels cleared, then each one's three
 * u16 at 0x0800913A-0x0800913E; 0x080091BC: the field's types into +0 from its record's nibbles, 0x080091F4: the sides
 * into +1 from BN4_PANEL_SIDES): its type and side bn4.h's BN4_PANEL_TYPE and BN4_PANEL_ALLIANCE */
#define BN4_PANEL_STATE   0x0C        /* u16 0x258 at the field's start, as BN6's +0xE (BN6_PANEL_STATE, its field init 0x0800C562) */
#define BN4_PANEL_TIMER_A 0x0E        /* u16 0x3C0 at the field's start (BN4_PANEL_TIMER_A_IDLE), idle through a plain battle */
#define BN4_PANEL_TIMER_B 0x12        /* u16 0x80BE at the field's start (BN4_PANEL_TIMER_B_IDLE), idle through a plain battle; BN6
                                       * keeps one timer at +0x12, 0x708 idle (BN6_PANEL_TIMER, BN6_PANEL_TIMER_IDLE) */
#define BN4_PANEL_TIMER_A_IDLE 0x3C0
#define BN4_PANEL_TIMER_B_IDLE 0x80BE
#define BN4_PANEL_FLAGS_LAST 0x08     /* u32: a cracked panel's flags (BN4_PANEL_FLAGS) as of the last frame, 0 on any other: the */
#define BN4_PANEL_FIELD_STEP 0x08009240u /* ... field's step each frame (r7 the panel; its store at 0x0800927E, the flags from
                                          * 0x08009A86), BN6's +0x18 (BN6_PANEL_FLAGS_LAST) from its twin loop */
#define BN4_PANEL_SIDES   0x08009218u /* a panel's side at the field's start, a byte a panel (0x080091FA's literal): 0 on the border
                                       * columns, where BN6's per-column side reads 0xFF (0x0800C8C2's table) */

/* MegaMan's AI data (BN4_AI_DATA, a side's): three of his power attacks, each read by its dispatcher into BN4's table
 * (0x0800C9D2 +0xD, 0x0800CA60 +0xF, 0x0800C9E8 +0, the attack variables at +0x70 in r7), called in that order by his
 * attack state (0x080EEC1E, 0x080EEC3E, 0x080EEC5E) as BN6's by his (BN6_AI_POWER_A, _CHARGE, _C: +6, +7, +8); the charged
 * shot's setup (0x0800D836) writes +0xF and +0 from his loadout record (0x0800D8CE-0x0800D8D8), as BN6's (0x08013892) +7
 * and +8 (0x08013918-0x0801391E) */
#define BN4_AI_POWER_A    0x0D        /* (0x0800C9D2's; BN6_AI_POWER_A, +6) */
#define BN4_AI_POWER_CHARGE 0x0F      /* the charged shot's (0x0800CA60's; BN6_AI_POWER_CHARGE, +7) */
#define BN4_AI_POWER_C    0x00        /* (0x0800C9E8's; BN6_AI_POWER_C, +8) */

/* ... and its fields paired with BN6's by both games' code (docs/devtools/differential.md, MegaMan's AI data and his object) */
#define BN4_AI_POWER_CHIP 0x0E        /* u8 the chip charge's power attack (0xFF none: 0x0800BD76 for form 0, else the form's loadout at
                                       * 0x0800BD5E), the one the charge time BN4_AI_CHARGE_TIMES +0 is read for (0x0800BCFA); BN6's +5
                                       * (BN6_AI_POWER_CHIP), whose charge time BN6 reads for r7 5 (0x08012F6C) */
#define BN4_AI_KEYS       0x1C        /* u16 x 8 his keys, as the update 0x0800BA78 writes them (r4 his AI data): held +0x1C, pressed +0x1E,
                                       * let go +0x20, the last held +0x22, then the second set +0x24 to +0x2B; BN6's twin 0x08012DFC writes
                                       * the same at +0x22 to +0x31 (BN6_AI_HELD, store by store) */
#define BN4_AI_KEYS_SIZE  0x10        /* ... their bytes, both sets */
#define BN4_AI_MODES      0x4C        /* u32 his flags: test 0x0800C1D6, set 0x0800C1E8, clear 0x0800C1F2, each BN6's at +0x48
                                       * (0x08010300, 0x08010312, 0x0801031C: BN6_AI_MODES) instruction for instruction */
#define BN4_AI_HP_BUG     0x03        /* u8 the HP bug's frames counted (0x0800C164: HP above 1, its period NaviStats +0xE, one HP lost
                                       * through 0x0800AB92 at the period, then 0); BN6's +9 (0x08010230, BN6_AI_HP_BUG) */
#define BN4_AI_DAMAGE_TAKEN 0x06     /* u16 the damage he has taken, at most 0xFFFF: added by 0x0800C75C, read by 0x0800C776 (by side);
                                       * BN6's +0x20 (0x08010548, 0x0801055E: BN6_AI_DAMAGE_TAKEN), instruction for instruction */
#define BN4_AI_STRUCK_FRAMES 0x54     /* u32 the frames his collision flags hold 0xC00 (0x0800C518: 0x0800C500 counts, 0x0800C50A
                                       * clears, 0x0800C512 reads); BN6's +0x4C (0x080143FC: 0x08014432, 0x08014424, 0x0801442C) */
#define BN4_AI_ANGER      0x2E        /* u16 his anger's frames: a struck word's 0x200 sets flag 0x200000 and 600 here (0x0800C594, its
                                       * emotion 0x80 through 0x0800F4DE), counted down after (0x0800C5C6); BN6's +0x34 (0x08014350,
                                       * BN6_AI_ANGER), instruction for instruction */
#define BN4_AI_LOADOUT_PAD 0x0C       /* u8 the loadout's first byte, 0xFF in every form (0x080184F0's +0, 0x0800BD74 for form 0) and read
                                       * by no code of BN4's through an AI data pointer (written only at 0x0800BD5E and 0x0800BD74) */
#define BN4_AI_CHARGE_TIMES 0x34      /* u16 x 2 the charge times of BN4_AI_POWER_CHIP's and the charged shot's (0xFF none), cached as the
                                       * loadout is set (0x0800BCF4, from BN4_CHARGE_TIMES by his Charge) and read by the charge levels
                                       * (0x0800BB56, 0x0800BB6C); BN6 caches none, reading its table at each test (0x08012F62) */
#define BN4_AI_CONTROL    0x50        /* u32 his side's control block (BN4_CONTROL_BLOCKS): the AI data's clear (0x0800B7CE) points side 0's
                                       * at 0x02035A70 and side 1's at 0x02035AB0; his ready routine (0x080EED48) runs its state from it
                                       * (+0 set up, 0x0800EB86 / 0x0800EB8E; +1 its state, 0x0800E4F8) */
#define BN4_CONTROL_BLOCKS 0x02035A70u /* the sides' control blocks, 0x40 each (cleared by 0x0800E8FE); BN6's ready (0x080F1694) keeps none */

/* (what Walla's and Elmperor's specs met, each read against BN6's twin: docs/devtools/differential.md, The BN4 trace) */
#define BN4_PLAYER_STRUCK 0x08010960u /* MegaMan's struck state (BN4_PLAYER_STATES entry 3): sets his flags' 0x400 as it begins
                                       * (0x0801096C-0x08010970, object_setFlag 0x08012F82) and clears it as its count ends (0x080109DE);
                                       * BN6's twin 0x080174FE sets and clears 0x400400 (0x0801750A, 0x0801758A) */
#define BN4_HIT_EFFECT    0x08012F44u /* the effect an attack's hits make, run by it each frame (Walla's rings, 0x080D4BEA): its place
                                       * drawn (0x08014620) and the hit's spark spawned (0x080E2140); BN6's twin 0x0801A0D4 */
#define BN4_POISON_STEP   0x08013010u /* the poison panel's step under an object, its cmp r0,#4 (BN6_PANEL_STEP_POISON's twin):
                                       * BN4_COLL_POISON_TIMER less 1 a frame (0x0801301E), 6 again with +0x34 a point more (0x08013026-
                                       * 0x0801302E), 0 off poison (0x08013034) */
#define BN4_DIRECTION     0x0800AF90u /* the direction from a panel (r2 column, r3 row) to another (r0, r1), a leaf: 4 and 3 by the
                                       * column (right and left by the screen), else 2 and 1 by the row, 0 none (0x0800AF92-0x0800AFAE);
                                       * BN6's rule (0x0800E994) numbers right 4 and left 3 on side 0 and the other way on side 1 (r4,
                                       * the object's +0x16: 0x0800E9C4, 0x0800E9D0) and a move of two panels or a diagonal one 5 */
#define BN4_HP_LOW_STEP   0x0801472Cu /* the HP gauge's step, entry 0 of the HUD's table 0x080146CC (0x080146A8 runs them by its word's
                                       * bits each frame): HP at a quarter of the maximum or less counts +0x19 to 45, then sound 0x82
                                       * (0x0801475C); BN6's twin BN6_HP_LOW_STEP, its count +6, its sound 0x84 */
#define BN4_SPRITE_UPDATE 0x0800285Cu /* an object's sprite each fight frame, its work in IWRAM (0x03006377; BN6's twin 0x080026C4, its
                                       * 0x03006793): its frame's count down, at 0 the next frame's record (+0xAC, 20 bytes, its duration at
                                       * +16, +18's 0x80 its last) */

/* A virus's appear (docs/rom/bn4.md, A virus's appear and the engine's appear bits): the generic state 0 (BN4_STATE_GENERIC_0,
 * bn4.h) picks one of four by its +5 and the BattleSettings' +6 bit 0x20; a plain battle's viruses and MegaMan take
 * BN4_APPEAR, BN6_APPEAR's twin */
#define BN4_APPEAR        0x0800FE00u /* the plain appear, by +0xA through its table 0x0800FE14: 0x0800FE20 into the appear queue
                                       * (0x08007F1C) and hidden, waiting for BN4_APPEAR_BITS' bit 0 and the queue's head; 0x0800FE92
                                       * the 2 x 16 frame blink, then out of the queue (0x08007F46); 0x0800FED6 waiting for bit 1, then
                                       * CurAction 1. BN6_APPEAR's steps but for BN4's battle kind 0x44 ([[r10+0x18]+0xF]) and its sound */
#define BN4_APPEAR_BITS   0x080019C0u /* [[r10+0x18]+0x64] into r0: the battle state's appear bits (bit 0 the viruses may appear,
                                       * set by the opening at 0x080E1F0E after its fade; bit 1 all have); BN6_APPEAR_BITS' twin */
#define BN4_APPEAR_BITS_SET 0x080019A8u /* ... r0's bits set in it (BN6_APPEAR_BITS_SET's twin) */
/* The appear queue (docs/rom/bn4.md, Mettaur's appear queue): one queue, 8 bytes of objects' +0xE at [[r10+0x18]+0x58], which
 * BN4_APPEAR's objects and Mettaur's turns share; BN6's (BN6_QUEUE_*, bn6_families.h) are queues of 8 words of objects, r0
 * their number, its appear's 0: a moved family's call made through a shim that makes r0 0 (XvSoundCall) */
#define BN4_QUEUE_HEAD    0x08007F04u /* r0 1 where r5 heads the queue, else 0 */
#define BN4_QUEUE_ENTER   0x08007F1Cu /* r5 put at its end where it is not in it: r0 1 where it was put */
#define BN4_QUEUE_LEAVE   0x08007F46u /* r5 taken out, the rest moved up (0x08007F6A) */
#define BN4_COLL_REGION_SET 0x08012ED0u /* a collision's region changed by r5's (r1 what it is, r2 what it strikes, r3 the set-up's
                                       * arg): BN6_COLL_REGION_SET's twin but for its second word, stored at r4's +0x5C, r4 the
                                       * region's number (0x08012EEC: a write to the BIOS), so its +0x5C keeps the set-up's */

/* The sounds BN4's moved families play that are no BN6 song (xrecipe.song_sig): each a movs r0,#N before its bl to
 * BN4_PLAY_SOUND in the family's code; its song read by its number through BN4's song table
 * (XRomLayout.song_table), copied into BN6 at its slot of BN6's empty songs (tools/xrecipe_bn4.py's SOUND_SLOTS,
 * src/audio/xsong.c's XSOUNDS) on the player BN4's entry names, the copy's movs given the slot (docs/rom/bn4.md, the
 * families' sounds) */
#define BN4_SOUND_SPIKEY_C4   0x080D2E84u /* Spikey's 0xC4, player 9 (its fireball's code, the third kind's 0x30) */
#define BN4_SOUND_SPIKEY_BC   0x080D2F24u /* ... its 0xBC, player 9 */
#define BN4_SOUND_SHRIMPY_C9  0x080D2BEAu /* Shrimpy's 0xC9, player 8 (its bubble's code, the third kind's 0x2F) */
#define BN4_SOUND_SHRIMPY_9F  0x080D2C52u /* ... its 0x9F, player 28 */
#define BN4_SOUND_SHRIMPY_CA  0x080D2D24u /* ... its 0xCA, player 8 */
#define BN4_SOUND_WALLA_9A    0x080BEA88u /* Walla's 0x9A, player 8 */
#define BN4_SOUND_WALLA_B7    0x080BEAE8u /* ... its 0xB7, player 9 */
#define BN4_SOUND_WALLA_70    0x080D4C3Au /* ... its 0x70, player 9 */
#define BN4_SOUND_WALLA_A6    0x080D4CE2u /* ... its 0xA6, player 9 */
#define BN4_SOUND_ELMPEROR_FC 0x080C2A7Cu /* Elmperor's 0xFC, player 19 */
#define BN4_SOUND_ELMPEROR_9D 0x080DC6FEu /* ... its 0x9D, player 9 */
#define BN4_SOUND_CIRKILL_F9  0x080BD42Eu /* CirKill's 0xF9, player 29, two tracks */

/* A won battle's reward (docs/rom/bn4.md, Its rewards): BN6's chooser and pick by their bytes, an enemy's row by its id */
#define BN4_REWARD_CHOOSER 0x080F57DCu /* the chooser (BN6's 0x080AC150, bn6f sub_80AA8E0, by its bytes): the pick, then BattleState +0x36 */
#define BN4_REWARD_PICK   0x080F580Cu /* the pick (BN6_REWARD_PICK's twin): one of the enemies (0x080F5944, u8 ids where BN6's are u16), its
                                       * row (BN4_REWARD_ROW_OF), an entry by the busting level (BN4_BUSTING_TABLE) and MegaMan's HP, r0 */
#define BN4_REWARD_ROW_OF 0x080F5EC4u /* an enemy id's row: the id itself, but in a battle whose BattleState +0x60 is one of the 22 Navis'
                                       * of 0x0800840C (0x080083E8: not 20 or 21), with MegaMan's BattleNaviStats +0x36 past 469 or the
                                       * byte 0x0203DC81 or 0x0203DC82 not 0, 0x100 + its place there: a virus's always its id */
#define BN4_REWARD_ROWS   0x080F5F50u /* the reward rows: 20 u16 a row, 0x28 apart, BN6's format (BN6_DROP_ROWS: 0-9 MegaMan's HP over 3/8,
                                       * 10-19 at or below; bits 14-15 0 a chip, its id and code << 9, 1 zenny, 2 HP); 0x080F5F40's literal */
#define BN4_BUSTING_TABLE 0x0801881Du /* the busting levels' rows of 16 entries' offsets (BN6's 0x08020B9C), read at 0x080F59D6 */

/* What GutsMan's attack reaches of the engine (docs/rom/bn4.md, GutsMan's attack against BN6's engine): the camera's shake,
 * the battle state's players, the rocks' debris */
#define BN4_CAMERA_SHAKE  0x08025FE0u /* the shake each frame (BN6's 0x080301E8): two draws of GetRNG (BN4_RNG_PRIMARY's; BN6's of its */
#define BN4_SHAKE_DRAW_X  0x0802600Cu /* ... second word) masked by BN4_SHAKE_TABLE into the camera's x (+0x38, BN6's +0x3C), then */
#define BN4_SHAKE_DRAW_Y  0x08026032u /* ... its y (+0x3C, BN6's +0x40) */
#define BN4_SHAKE_TABLE   0x08026068u /* ... 8 bytes a strength (mask, centre), the same 32 bytes as BN6's 0x08030284 */
#define BN4_CAMERA_SHAKE_SET 0x0802608Cu /* its strength r0 and frames r1 into the camera's +0xE and +0xC (BN6's 0x080302A8, the same body) */
#define BN4_BATTLE_PLAYERS 0x44       /* BattleState +0x44: MegaMan's side's player object, then the other side's, a word each
                                       * (BN4_FIND_PLAYER's reads; BN6 keeps one a side 0x10 apart from +0xD0, BN6_BATTLE_PLAYERS) */
#define BN4_T4_DEBRIS     0x09        /* the fourth kind's 0x09, a rock's debris: its spawner BN4_SPAWN_DEBRIS (r6, r7 its speeds into */
#define BN4_SPAWN_DEBRIS  0x080E29F8u /* ... +0x40, +0x48; BN6's 0x080E23C0, the same bytes but the spawn's bl), its init */
#define BN4_DEBRIS_INIT   0x080E2934u /* ... setting up sprite list 4 0x0E (0x080E2942), BN6's twin 0x080E22FC its own list 4 5 */

/* A collision data's frame of hits, by the attack's element (0 none, 1 Fire, 2 Aqua, 3 Elec, 4 Wood), five u16 each, BN6's
 * twins by the code that writes and sums them (docs/rom/bn4.md, What a hit leaves the struck, by element): */
#define BN4_COLL_HIT_DAMAGES 0x2A     /* the hit pass adds each hit's damage, shifted by its multiplier, at +0x2A + 2 * its
                                       * element (0x08012C74-0x08012C82; BN6's +0x82, BN6_COLL_HIT_DAMAGES) */
#define BN4_COLL_ELEM_DAMAGES 0x3C    /* the frame's damage by element, the five summed at BN4_ELEM_DAMAGES_SUM (BN6's +0x94) */
#define BN4_ELEM_DAMAGES_SUM 0x08012E60u /* ldrh r0,[r4,#0x3c]; ldrh r1,[r4,#0x3e]; adds ... to +0x44 (BN6_ELEM_DAMAGES_SUM's twin) */

/* Its Program Advances (docs/rom/bn4.md, Its Program Advances; docs/multirom/bn4-chips.md): BN6's recipes' format, read for
 * Phase 4's grown list (BN6_PA_LIST's twin) */
#define BN4_PA_LIST       0x080212B0u /* the recipes' addresses, BN4_PA_COUNT of them, then 0 (BN6_PA_LIST's shape) */
#define BN4_PA_LIST_REF   0x0801F4E0u /* ... the one word that names it, loaded at 0x0801F3BE (r6) */
#define BN4_PA_COUNT      37          /* ... how many recipes: 30 Program Advances, LifeSrd, DarkNeo and HeatSprd by two or more */
#define BN4_PA_RECIPES    0x080211B0u /* the recipes: n, kind, the PA's id (u16), its chips (u16 id | code << 9) */
#define BN4_PA_KIND_LISTED 1          /* a recipe of n chips listed, each its own (BN6's kind 4); kind 0 one chip n times, codes in a row */

#endif

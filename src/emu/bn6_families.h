/* The battle engine's tables a virus family is found through, in BN6
 * Cybeast Gregar (USA), part of bn6.h's addresses (it includes this): what
 * src/emu/xvirus_grow.c grows and points elsewhere, and src/emu/xvirus.c
 * fills, to move another game's families into BN6's own battles (issue #82;
 * docs/multirom/families.md; docs/rom/virus-families.md, Virus families
 * moved into the battle engine, and The engine's room for them). */
#ifndef CW_BN6_FAMILIES_H
#define CW_BN6_FAMILIES_H

/* Virus families moved into the battle engine (src/emu/xvirus.c, issue #82;
 * docs/rom/virus-families.md, Transplanted virus families). The engine finds a
 * virus's code and records in eleven per-family tables by its AI index
 * (AIData +1, the third byte of its enemy id's triple, RomLayout.enemy_ids) */
#define BN6_FAMILY_TABLES     0x0810A378u /* the first of the eleven, BN6_FAMILY_STEP apart, BN6_FAMILIES words each: 0 the state tables
                                             (a Thumb pointer per CurAction), 1 struct1 (8 bytes: sprite list and slot, palette, element),
                                             2 struct2 (36: HP and damage per version), then hooks the engine calls by AI index: 3 each frame,
                                             4 as it spawns, 5 its spawn done, 6 paralysed, 7 frozen, 8 bubbled, 9 flinching, 10 dragged */
#define BN6_FAMILY_STEP       0x80        /* ... the bytes from one table to the next */
#define BN6_FAMILIES          32          /* ... their entries: AI indexes 0 (TestVirs) to 31 */
#define BN6_FAMILY_NULLSUB    0x0810A953u /* the hooks' entry where a family has none: mov pc, lr (Thumb) */
#define BN6_VIRUS_PRESENT     0x08016E64u /* a virus's after-state, run by the virus dispatcher after its states: BN4_VIRUS_PRESENT's twin
                                             (bn4.h) but for a pause test first (0x08016E84-0x08016E90: the BattleSettings' mode, 0x0802D234,
                                             not 1, then battle_isPaused 0x0800A03C), so no virus animates in the paused opening */
#define BN6_VIRUS_TYPE        0x0810A278u /* the first kind's type 0, every virus's (a moved family's too): its state by +8 (the virus init
                                             0x08016F56, the dispatcher 0x0810A29C, the dying 0x08016C4E), then BN6_VIRUS_PRESENT (its bl at
                                             0x0810A284), as each BN4 family's own type routine ends in BN4_VIRUS_PRESENT (bn4.h) */
#define BN6_SETTINGS_MODE_READ 0x0802D234u /* the BattleSettings' mode (BN6_SETTINGS_MODE) into r0: BN6_VIRUS_PRESENT's pause test's first
                                             call (its 1 runs the after-state paused or not) */
#define BN6_PAUSE_TEST        0x0800A03Cu /* the battle's pause byte tested ([eToolkit + BN6_TOOLKIT_PAUSE] + BN6_PAUSED): its flags ne while
                                             paused, BN6_VIRUS_PRESENT's pause test's second call */
#define BN6_SPRITE_UPDATE     0x080026C4u /* an object's sprite counted on a frame, its work in IWRAM (BN4_SPRITE_UPDATE's twin): BN6_VIRUS_PRESENT's
                                             last call (0x08016EBA) */
#define BN6_APPEAR            0x08016396u /* the plain appear (the generic state 0's, BN6_NAVI_SPAWN_STATE, where its AI data's +2
                                             is 0; else 0x080164A0, BN6_NAVI_PLATE's arrival), by +0xA through 0x080163A8: 0x080163B4
                                             into the appear queue (0x0800AA1A) and hidden, waiting for BN6_APPEAR_BITS' bit 0 and the
                                             queue's head; 0x0801641A the 2 x 16 frame blink, then out of the queue (0x0800AA40);
                                             0x08016460 waiting for bit 1, then CurAction 1 (BN4_APPEAR's steps, bn4_battle.h) */
#define BN6_VIRUS_INIT        0x08016F56u /* BN6_VIRUS_TYPE's +8 0 (its table 0x0810A290): a virus's init by its AI index, its +8
                                             made 4, the spawn-end hook (5) last; the type's present not run (a summon's shim runs
                                             it alone in the spawn frame: xvirus_shim_summon) */
#define BN6_QUEUE_HEAD        0x0800AA06u /* the appear queue r0 (8 words of objects, 0x20 bytes a queue from 0x02038170, by
                                             0x0800AAAE; BN6_APPEAR's 0, 0x080163D6): r0 1 where r5 heads it (BN4_QUEUE_HEAD's twin) */
#define BN6_QUEUE_ENTER       0x0800AA1Au /* ... r5 put at queue r0's end where it is not in it: r0 1 where it was put */
#define BN6_QUEUE_LEAVE       0x0800AA40u /* ... r5 taken out of queue r0, the rest moved up (0x0800AA64) */
#define BN6_COLL_REGION_SET   0x0801A082u /* r5's collision's region changed (r1 what it is, r2 what it strikes, r3 its +0xE): the
                                             set-up's tail (0x08019FB4 from 0x08019FDC), the time stop's bit in +0x30 (BN4_COLL_REGION_SET's twin) */
#define BN6_COLL_REGION_CLEAR 0x0801A074u /* r5's collision's +1 cleared, r0 its collision data back (BN4's 0x08012DC0's twin) */
#define BN6_SPAWN_ENEMY_2E    0x0800774Cu /* movs r0,#0xa; strh r0,[r5,#0x2e]: an enemy's +0x2E at its spawn (BN4's spawn gives the
                                             same), which the collision set-up copies into its +7 (BN6_COLL_OBJ_2E) */
#define BN6_APPEAR_BLINK_FIRST 0x080163FCu /* movs r0,#2 (0x2002), its first blink step's count stored to +0x20 as BN6_APPEAR's
                                             step 0 ends (0x080163FE); the blink (0x0801641A) counts it down and puts 2 back at
                                             0x08016424 each step. The BN4 differential's opening alignment makes it 3
                                             (tests/xdiff/legit.json, bn4, align; docs/devtools/differential.md, The BN4 trace) */
#define BN6_APPEAR_BITS       0x0800139Au /* [[r10+0x18]+0x5C] into r0: the battle state's appear bits (bit 0 set by the opening at
                                             0x080E1A0E after its fade, bit 1 once all have appeared); BN4_APPEAR_BITS' twin */
#define BN6_APPEAR_BITS_SET   0x08001382u /* ... r0's bits set in it */
#define BN6_BX_R0             0x08148865u /* a bx r0 the game itself calls into (0x0815E6A8; the tail of a routine, 0x08148860): where a
                                             BN4 family's pre-step goes on to BN6_VIRUS_STATE_STEP, r0 its address (tools/xrecipe_bn4.py) */
#define BN6_FAMILY_REF_STATES 0x0810A370u /* the literals that name the tables, one each, in their order: table 0's (the dispatcher's);
                                             1's and 2's RomLayout.enemy_traits and enemy_stats (the virus word of a triple: the
                                             viruses', the Navis' and the player's tables) */
#define BN6_FAMILY_REF_FRAME  0x0810A374u /* ... table 3's */
#define BN6_FAMILY_REF_SPAWN  0x0800F36Cu /* ... table 4's (each of 4-10 a triple's virus word) */
#define BN6_FAMILY_REF_SPAWNED 0x0800F388u /* ... table 5's */
#define BN6_FAMILY_REF_PARALYSIS 0x0800F3A4u /* ... table 6's */
#define BN6_FAMILY_REF_FREEZE 0x0800F3C0u /* ... table 7's */
#define BN6_FAMILY_REF_BUBBLE 0x0800F3DCu /* ... table 8's */
#define BN6_FAMILY_REF_FLINCH 0x0800F3F8u /* ... table 9's */
#define BN6_FAMILY_REF_DRAG   0x0800F414u /* ... table 10's */
#define BN6_FILLER_AI         1           /* the AI index BN6 gives enemy ids that no battle names (0xBB-0xCC: Mettaur's, their names blank to 0xC0) */
#define BN6_FILLER_ID         0xBB        /* the first of those ids: (version 0, a virus, BN6_FILLER_AI), its name blank, its reward row BN6's filler
                                             (100 and 60 zenny, else Cannon *): what each id past BN6's is given where the ids are grown */
#define BN6_HELD_CHIPS        0x080B05F0u /* per AI index (BN6_HELD_AIS words), its viruses' held chips: a list of u16 chips (0xFFFF none) 0x20 bytes
                                             a version; AI 32-34's TestVirs' */
#define BN6_HELD_CHIPS_REF    0x080B05E4u /* ... the virus word of the triple that names it (by the id triple's kind: the viruses', the Navis' and
                                             the player's tables; its reader 0x080B05C0, its one literal 0x080B05E0) */
#define BN6_HELD_AIS          35          /* the AI indexes BN6_HELD_CHIPS and the two tables of hooks beside it hold */
#define BN6_AI_HOOKS          0x08010E18u /* per AI index (BN6_HELD_AIS words), a routine a Navi's companion begins by (0x080114D5 each: mov pc, lr) */
#define BN6_AI_HOOKS_REF      0x08010E0Cu /* ... its triple's virus word (read by 0x08010DDA and 0x08010DF6, through the one literal 0x08010E08) */
#define BN6_AI_HOOKS2         0x08011068u /* per AI index (BN6_HELD_AIS words), the routine it ends by (0x080114D5 each) */
#define BN6_AI_HOOKS2_REF     0x0801105Cu /* ... its triple's virus word (read by 0x0801102A and 0x08011044, through 0x08011058) */
#define BN6_DELETE_COUNTS     0x02000070u /* a virus deleted, per AI index (32 bytes, to 255): the rare versions' check reads them */
#define BN6_FOE_DYING        0x08016620u /* the dying state (bn6f sub_80165F8) with the object's triple in r0 (version, actor type, AI index: its
                                           * NameID's, from 0x080182B4), the object in r5: ldrb r1,[r0,#1], before the counts below are called */
#define BN6_FOE_DYING_OP     0x7841u     /* ... that instruction */
#define BN6_DELETE_COUNT      0x0802D644u /* the count raised as a virus of version 0-3 is deleted: r0 its AI index (bn6f's dying state, 0x0801662E) */
#define BN6_DELETE_COUNT_READ 0x0802D652u /* ... read: r0 the AI index (the rare versions' check, 0x080ABFA6), the count back in r0 */
#define BN6_DELETE_RARE       0x0802D65Eu /* a version 4-5 deleted (0x08016636): r0 the AI index; with event flag 0x1D8 set and 0x196 + it clear
                                             (and it not 10 or 13) it is put at 0x0203EAEA (0x0800AB2E), the index of the chat
                                             of new virus data after the results (0x0802C2E8); no flag is set */
#define BN6_ICE_SIZES_REF     0x080EAF64u /* the ice block a frozen enemy is held in (BN6_STRUCK_FREEZE) takes its size from its AI index as it spawns
                                             (0x080EAF46: a byte per AI index, from a triple of tables by the id triple's kind): this the virus word, */
#define BN6_ICE_SIZES         0x080EAF70u /* ... the viruses' sizes, a byte each (0-2, larger blocks up), BN6_ICE_VIRUSES of them: AI 30 and 31 read on into the Navis' */
#define BN6_ICE_VIRUSES       30
#define BN6_ENEMY_IDS_REF     0x080182C0u /* the one word naming RomLayout.enemy_ids (its reader 0x080182B4: id x 3, no bound) */
#define BN6_ENEMY_IDS_COUNT   0x1C4       /* ... its ids: BN6's viruses to 0xBA, filler to 0xFF, the Navis 0x100-0x190, filler, the player's forms
                                             0x1A0-0x1C3; code from 0x08018810 */
#define BN6_DROP_ROWS_REF     0x080AC714u /* the one word naming BN6_DROP_ROWS (its reader 0x080AC708: id x 0x28, no bound) */
#define BN6_DROP_ROW_COUNT    0x191       /* ... its rows, ids 0x000-0x190; code from 0x080B05C0 */
#define BN6_NAME_OF           0x0800EC56u /* an enemy id's name: r0 the id, back r0 the text archive (RomLayout.enemy_names[0], [1] past 0xFF) and
                                             r1 its entry (the id's low byte); a leaf (mov pc, lr), its one caller the battle's name plates
                                             (0x0801E59A, the NameIDs 0x0800A998 reads as u16) */
#define BN6_PRELOAD_ID_LSL    0x080029CEu /* the battle's sprite preload (0x080029A8, over the BattleSettings' entities: each one's battle sprite,
                                             where it is compressed, decompressed into a cache of 12 below 0x02033000) reads an id as its low byte
                                             and, by lsls r2,r2,#0x1F here, */
#define BN6_PRELOAD_ID_LSR    0x080029D0u /* ... and lsrs r2,r2,#0x17 here, bit 0 of its high byte as its bit 8: 9 bits in all, where the spawn's
                                             own read (0x080073F4: lsls #0x1C, lsrs #0x14) takes the high byte's low 4 bits, 12 */

/* A family's records and what it draws: struct1's fields (BN6_FAMILY_TABLES
 * table 1, 8 bytes; read as a virus spawns, 0x08016F56) */
#define BN6_TRAITS_LIST       0           /* the sprite's list, as a byte offset into RomLayout.sprite_lists (4: list 1, the viruses') */
#define BN6_TRAITS_SPRITE     1           /* ... its number in the list */
#define BN6_TRAITS_PALETTE    2           /* the palette step: a version's palette is its version times this */
#define BN6_TRAITS_MODE       4           /* an index into BN6_TRAITS_MODES, read in battle mode 6 alone */
#define BN6_TRAITS_MODES      0x080170A4u /* ... its table: a byte each (0 or 1): in battle mode 6 a virus spawns with its +0x2C cleared where its
                                             byte is 0 (0x08016FB0-0x08016FBC; its one literal 0x080170A0, read nowhere else), */
#define BN6_TRAITS_MODE_COUNT 32          /* ... BN6's families' 32 */
#define BN6_TRAITS_ELEMENT    5           /* its element of the second wheel, ORed into BN6_T1_ELEMENT */
#define BN6_TRAITS_WEAKNESS   6           /* the second-wheel elements that strike it twice as hard */
#define BN6_TRAITS_SHADOW     7           /* a shadow drawn under it where not 0 */
#define BN6_TRAITS_SIZE       8           /* struct1's bytes */
#define BN6_STATS_SIZE        6           /* struct2 (table 2), per version: u16 element << 12 | HP, the version, flags, u16 element << 12 | damage */
#define BN6_SPRITES_VIRUSES   1           /* sprite list 1, the viruses' battle sprites (BN6_TRAITS_LIST 4), its pointer word at RomLayout.sprite_lists
                                             + 4 x 1, as each list's: */
#define BN6_SPRITES_VIRUSES_COUNT 32      /* ... its numbers (0x08031D24 on) */
#define BN6_SPRITES_NAVIS     2           /* sprite list 2, the Navis' battle sprites (struct1's list offset 8): */
#define BN6_SPRITES_NAVIS_COUNT 23        /* ... its numbers (0x08031DA4 on) */
#define BN6_SPRITE_PLACEHOLDER 0x084DC040u /* the placeholder sprite (a white dot) a list's unused numbers point at: list 1's 0x00 and 0x1E, list 7's
                                             numbers no map lists (src/layer/xnavi.c) */
#define BN6_SPRITES_LIST3     3           /* sprite list 3, the chips' attacks (MegaMan's sword, a gel, a crack's hole): */
#define BN6_SPRITES_LIST3_COUNT 105       /* ... its numbers (0x08031E00 on, list 4's after them) */
#define BN6_SPRITES_ATTACKS   4           /* sprite list 4, the battle's attacks and effects (their objects'): */
#define BN6_SPRITES_ATTACKS_COUNT 92      /* ... its numbers (0x08031FA4 on, list 5's after them) */
#define BN6_SPRITES_LIST5     5           /* sprite list 5 (MegaMan's buster's charge, a hit's spark): */
#define BN6_SPRITES_LIST5_COUNT 32        /* ... its numbers (0x08032114 on, list 6's after them) */
#define BN6_SPRITES_LIST5_THROUGH 0x80    /* ... read past them into list 6's table, the real world's people (5:0x20-0x7F: Lan's
                                             family, the town's folk, src/world/town_lines.c; sprite_load called from 0x0809FB58):
                                             a grown list 5 keeps BN6's words to 0x7F, measured (docs/rom/) */
#define BN6_EFFECTS           0x080E16D4u /* the effects a battle draws (the fourth kind of object's type 0, by the effect id at its +4), 4 bytes
                                             each: a sprite list's offset (as BN6_TRAITS_LIST), its number, the animation, a palette added */
#define BN6_EFFECTS_REF       0x080E1970u /* ... the one literal that names them (read at 0x080E18AA) */
#define BN6_EFFECT_COUNT      108         /* ... their ids, the effects' code after them */
/* The battle's objects by kind (their header's +2 & 0xF: 1, 3, 4) each run
 * the routine of their type (+1) from the kind's table each frame
 * (0x080031F0: the kinds' table 0x08003220, its one literal 0x0800321C),
 * code (push {r4-r7,lr}) after each */
#define BN6_T1_TYPES          0x08003C9Cu /* the first kind's routines (type 0 MegaMan and the enemies, by actor type; then their decoys,
                                             companions and flashes), a Thumb pointer per type, */
#define BN6_T1_TYPES_REF      0x08003224u /* ... its word in the kinds' table */
#define BN6_TYPES_T1_COUNT    95          /* ... its types */
#define BN6_T3_TYPES          0x08003EC4u /* the third kind's (attacks), */
#define BN6_T3_TYPES_REF      0x0800322Cu /* ... its word */
#define BN6_TYPES_T3_COUNT    213         /* ... its types */
#define BN6_T3_FREE           0xD1        /* a type whose entry is 0 (0xD2 too), which none of BN6's spawns names: free (CanGard's reticle took
                                             it until the second-level dispatch, XV_T3_DISPATCH 0xD2, ran BN5's types) */
#define BN6_T4_TYPES          0x080042C8u /* the fourth kind's (effects), */
#define BN6_T4_TYPES_REF      0x08003230u /* ... its word */
#define BN6_TYPES_T4_COUNT    146         /* ... its types */
/* The spawners (r0 the type, r1-r4 the object's four words, r5 back the
 * object or 0): their common part (0x08003278) takes a free slot of the
 * kind's 32 and the routine each ends with links the object into the run
 * order. The second-level dispatch (src/emu/xvirus_dispatch.c) calls them
 * for a moved family's code (BN5_SPAWN_* the same in BN5) */
#define BN6_SPAWN_1           0x08003320u /* the first kind's, run after the object running (0x08003400) */
#define BN6_SPAWN_1_LAST      0x0800333Cu /* ... at the list's end (0x08003428) */
#define BN6_SPAWN_3           0x08003358u /* the third kind's, after the object running */
#define BN6_SPAWN_3_LAST      0x08003374u /* ... at the end */
#define BN6_SPAWN_3_FIRST     0x08003390u /* ... at the start (0x08003440) */
#define BN6_SPAWN_4           0x080033ACu /* the fourth kind's, after the object running (0x080033C8 and 0x080033E4 its two others) */
#define BN6_OBJECT_FREE       0x08003458u /* object_freeMemory: r5 the object let go, its slot freed and its node out of the run order (as
                                             an object ends itself) */
#define BN6_DISPATCH_SIDE     0x0203FF00u /* the second-level dispatch's side tables, ours (docs/engine/emulation/memory.md, the RAM plan): a byte per
                                             object slot, its game's type the dispatch runs it as, the first kind's 32 here, the third's at
                                             +0x20, the fourth's at +0x40; the battle's setup clears them with the rest of 0x0203CDA8-
                                             0x0203FFFF (0x080071F2), and nothing of BN6's reads or writes 0x0203FB50 on */
#define BN6_DISPATCH_SIDE_STEP 0x20       /* ... from one kind's table to the next */
#define BN6_THUNDER           0x080CAC3Cu /* the third kind's 0x2A, a thunder: its routine (to 0x080CAF10, its type routine's next) */
#define BN6_THUNDER_ARG       0x080CACD6u /* ... its init's movs r3,#1 before its collision set up (0x08019FB4, BN6_COLL_SETUP_ARG),
                                             * BN4's 0 (bn4.h BN4_THUNDER_ARG): BN4's families run a copy with 0 (xvirus_bn4_copy.h) */
#define BN6_SPAWN_THUNDER     0x080CADBCu /* ... its spawner (r1 column, r2 row, r3 element; movs r0,#0x2A, BN6_SPAWN_3), called by
                                             * BN6 at 0x080EDB48 and 0x08108464 */

/* What a moved family's code asks of BN6's engine that BN6's own families
 * never do (issue #82's A2b; docs/rom/virus-families.md, The families' shims and
 * pre-steps) */
#define BN6_BY_NAME_RANGE     0x0800EBD4u /* object_getEnemyByNameRange: r0 a list it fills, r1 and r2 the NameIDs' bounds (each in), r3 the
                                             alliance; back r0 how many (BN5's twin 0x0800CE54, BN5_BY_NAME_RANGE) */
#define BN6_COUNT_BY_NAME     0x0810A960u /* the shared virus code's count of the enemies of a NameID range (r1, r2) whose word at r0 is not
                                             0: back r0 the count, r1 their sum (r1 left as BN6_BY_NAME_RANGE left it where there is none) */
#define BN6_VIRUS_NAMEID_CASES 0x0810A2A6u /* the virus dispatcher (0x0810A29C, each virus each frame) as it reads the virus's NameID (ldrh
                                             r0,[r5,#0x28]) for the cases it runs before the state (its own ids 0x49, 0xA3-0xA8, 0x61-0x66,
                                             0x43-0x48): the families' pre-step hook (xvirus_shim.h) */
#define BN6_VIRUS_STATE_STEP  0x0810A348u /* ... past those cases, the state's step (ldr r0,[r5,#0x58]: the AI index, its state table,
                                             0x0801B1C4), where a family's pre-step goes on (r5 the virus, nothing else live) */
#define BN6_VIRUS_STATE_CALL  0x0810A352u /* ... its call of the state runner (bl 0x0801B1C4), r0 the state table by the AI index
                                             (BN6_FAMILY_TABLES' first, 0x0810A350), r5 the virus: a summoned enemy's state 0
                                             taken from the summon's table there (xnavi_summoned.c) */
#define BN6_T1_SPAWN_KIND     0x05        /* u8 how it came: the battle actor setup writes its argument here and at its AI data's
                                             +2 (0x080076F0-0x080076F2): 0 one a battle begins with, 1 one spawned into a fight
                                             under way (BN6_SPAWN_ENEMY_1), which BN6's generic state 0 (0x08016380) reads at +2;
                                             2 a summon's, BN4's mark (BN4_SUMMON_MARK), written by the summon's shim alone */
#define BN6_AI_INDEX          1           /* an AI's data's (BN6_T1_AI_DATA) +1: its AI index, the third byte of its enemy id's triple */
#define BN6_CLEAR_FLAG2       0x0801A176u /* object_clearFlag2: r0 the bits cleared from its collision data's BN6_COLL_STRUCK (+0x40) */
#define BN6_SONGS_EMPTY_FIRST 0x26        /* BN6's empty song slots (RomLayout.song_table), each a shared header of no tracks: from, */
#define BN6_SONGS_EMPTY_LAST  0x62        /* ... to (other games' songs, src/audio/xsong.c; the families' sounds, XV_SONG_SLOTS from the last) */
/* An object's arm points: 0x08018810 finds its record by its struct1's list and number (0x0800F26C), in a table of records
 * by list offset (the words at 0x080188A0: MegaMan's, the viruses', the Navis'), a point's x and y signed bytes */
#define BN6_ARM_VIRUSES_REF   0x080188A4u /* the viruses' records' word (sprite list 1's numbers: 0x0801898C, the one word naming it) */
#define BN6_ARM_VIRUSES_COUNT 31          /* ... BN6's records, the Navis' table after them (0x080191C8) */
#define BN6_ARM_VIRUS_SIZE    0x44        /* ... a record: 34 points, BN5's 30 first (30-33 BN6's own: the fourth kind's 0x3C reads 30,
                                             the third's 0x95 31, 0x08010820 32, a freeze's ice block 33) */

#endif

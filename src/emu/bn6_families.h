/* The battle engine's tables a virus family is found through, in BN6
 * Cybeast Gregar (USA), part of bn6.h's addresses (it includes this): what
 * src/emu/xvirus_grow.c grows and points elsewhere, and src/emu/xvirus.c
 * fills, to move another game's families into BN6's own battles (issue #82;
 * docs/MULTIROM.md, Transplanted families; docs/ROM_DATA.md, Virus families
 * moved into the battle engine, and The engine's room for them). */
#ifndef CW_BN6_FAMILIES_H
#define CW_BN6_FAMILIES_H

/* Virus families moved into the battle engine (src/emu/xvirus.c, issue #82;
 * docs/ROM_DATA.md, Transplanted virus families). The engine finds a
 * virus's code and records in eleven per-family tables by its AI index
 * (AIData +1, the third byte of its enemy id's triple, RomLayout.enemy_ids) */
#define BN6_FAMILY_TABLES     0x0810A378u /* the first of the eleven, BN6_FAMILY_STEP apart, BN6_FAMILIES words each: 0 the state tables
                                             (a Thumb pointer per CurAction), 1 struct1 (8 bytes: sprite list and slot, palette, element),
                                             2 struct2 (36: HP and damage per version), then hooks the engine calls by AI index: 3 each frame,
                                             4 as it spawns, 5 its spawn done, 6 paralysed, 7 frozen, 8 bubbled, 9 flinching, 10 dragged */
#define BN6_FAMILY_STEP       0x80        /* ... the bytes from one table to the next */
#define BN6_FAMILIES          32          /* ... their entries: AI indexes 0 (TestVirs) to 31 */
#define BN6_FAMILY_NULLSUB    0x0810A953u /* the hooks' entry where a family has none: mov pc, lr (Thumb) */
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
#define BN6_SPRITES_ATTACKS   4           /* sprite list 4, the battle's attacks and effects (their objects'): */
#define BN6_SPRITES_ATTACKS_COUNT 92      /* ... its numbers (0x08031FA4 on, list 5's after them) */
#define BN6_SPRITES_LIST5     5           /* sprite list 5 (MegaMan's buster's charge, a hit's spark): */
#define BN6_SPRITES_LIST5_COUNT 32        /* ... its numbers (0x08032114 on) */
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
#define BN6_DISPATCH_SIDE     0x0203FF00u /* the second-level dispatch's side tables, ours (docs/EMULATION.md, the RAM plan): a byte per
                                             object slot, its game's type the dispatch runs it as, the first kind's 32 here, the third's at
                                             +0x20, the fourth's at +0x40; the battle's setup clears them with the rest of 0x0203CDA8-
                                             0x0203FFFF (0x080071F2), and nothing of BN6's reads or writes 0x0203FB50 on */
#define BN6_DISPATCH_SIDE_STEP 0x20       /* ... from one kind's table to the next */

#endif

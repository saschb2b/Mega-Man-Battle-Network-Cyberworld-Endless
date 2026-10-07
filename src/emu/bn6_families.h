/* The battle engine's tables a virus family is found through, in BN6
 * Cybeast Gregar (USA), part of bn6.h's addresses (it includes this): what
 * src/emu/xvirus.c grows and points elsewhere to move another game's
 * families into BN6's own battles (issue #82; docs/MULTIROM.md,
 * Transplanted families; docs/ROM_DATA.md, Virus families moved into the
 * battle engine). */
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
#define BN6_HELD_CHIPS        0x080B05F0u /* per AI index (35 words), its viruses' held chips: a list of u16 chips (0xFFFF none) 0x20 bytes
                                             a version; AI 32's TestVirs' */
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
#define BN6_ENEMY_NAMES_REF   0x0800ECECu /* the literals that name the viruses' names (RomLayout.enemy_names): this one, */
#define BN6_ENEMY_NAMES_REF2  0x0802B3F0u /* ... and this */

/* A family's records and what it draws: struct1's fields (BN6_FAMILY_TABLES
 * table 1, 8 bytes; read as a virus spawns, 0x08016F56) */
#define BN6_TRAITS_LIST       0           /* the sprite's list, as a byte offset into RomLayout.sprite_lists (4: list 1, the viruses') */
#define BN6_TRAITS_SPRITE     1           /* ... its number in the list */
#define BN6_TRAITS_PALETTE    2           /* the palette step: a version's palette is its version times this */
#define BN6_TRAITS_MODE       4           /* an index into the 32 bytes at 0x080170A4, read in battle mode 6 alone */
#define BN6_TRAITS_ELEMENT    5           /* its element of the second wheel, ORed into BN6_T1_ELEMENT */
#define BN6_TRAITS_WEAKNESS   6           /* the second-wheel elements that strike it twice as hard */
#define BN6_TRAITS_SHADOW     7           /* a shadow drawn under it where not 0 */
#define BN6_TRAITS_SIZE       8           /* struct1's bytes */
#define BN6_STATS_SIZE        6           /* struct2 (table 2), per version: u16 element << 12 | HP, the version, flags, u16 element << 12 | damage */
#define BN6_SPRITES_VIRUSES   1           /* sprite list 1, the viruses' battle sprites (BN6_TRAITS_LIST 4) */
#define BN6_SPRITE_PLACEHOLDER 0x084DC040u /* the placeholder sprite (a white dot) a list's unused numbers point at: list 1's 0x00 and 0x1E, list 7's
                                             numbers no map lists (src/layer/xnavi.c) */
#define BN6_SPRITES_ATTACKS   4           /* sprite list 4, the battle's attacks and effects (their objects'), its pointer word at
                                             RomLayout.sprite_lists + 4 x 4: */
#define BN6_SPRITES_ATTACKS_COUNT 92      /* ... its numbers (0x08031FA4 on, list 5's after them) */
#define BN6_EFFECTS           0x080E16D4u /* the effects a battle draws (the fourth kind of object's type 0, by the effect id at its +4), 4 bytes
                                             each: a sprite list's offset (as BN6_TRAITS_LIST), its number, the animation, a palette added */
#define BN6_EFFECTS_REF       0x080E1970u /* ... the one literal that names them (read at 0x080E18AA) */
#define BN6_EFFECT_COUNT      108         /* ... their ids, the effects' code after them */
#define BN6_T3_TYPES          0x08003EC4u /* the routines of the battle's third kind of object (attacks), a Thumb pointer per type, run each frame */
#define BN6_T3_TYPES_REF      0x0800322Cu /* ... its literal */
#define BN6_T3_FREE           0xD1        /* a type whose entry is 0 (0xD2 too), which none of BN6's spawns names: a moved family's attack's */

#endif

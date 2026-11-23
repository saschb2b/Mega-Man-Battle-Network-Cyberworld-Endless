/* MeddySoul's capsules on the Custom screen: BN5's Soul button for its Soul 6, and what Gregar kept of it (docs/multirom/souls.md,
 * Team ProtoMan's Souls, MeddySoul's capsules; docs/rom/souls.md, MeddySoul's capsules), what src/emu/soulcapsule.c stands
 * on: BN6's own screen draws a capsule as the chip its entry names (BN6_CAPSULE_DRAW, BN6_CAPSULE_INFO), BN6's chip table
 * holds three of BN5's five capsules at their ids (382 WhiCapsl, its art BN5's in the image: src/core/chips.c; 383 PrpCapsl,
 * 384 PnkCapsl) and the registry the other two (734 YelCapsl, 735 BlkCapsl, where BN6 has IceCube and Z Saver). */
#ifndef CW_BN5_CAPSULES_H
#define CW_BN5_CAPSULES_H

/* ---- BN5's (Team Colonel) ---- */

/* The Soul button's set-up (r4 the elements, r5 the screen, r7 the united Soul): MeddySoul's part */
#define BN5_SOUL_BUTTON       0x08023CF4u /* the set-up: 3 SearchSoul types 4 and 5, 6 MeddySoul types 6 and 7, 7 ColonelSoul 8 and 9 */
#define BN5_CAPSULES_SETUP    0x08023D1Cu /* cmp r7,#6: elements 8 and 9 made types 6 and 7, both grey (+7 1), no links set;
                                             their +8 the screen's BN5_SCREEN_CAPSULES and the halfword after */
#define BN5_CAPSULES_HEALTHY  0x08025E64u /* 16 chip entries the capsules are drawn from while MegaMan's HP (+0x24) is a quarter of
                                             his max (+0x26 >> 2) or more, weighted to YelCapsl, BlkCapsl and WhiCapsl, */
#define BN5_CAPSULES_HURT     0x08025E84u /* ... and 16 below a quarter, weighted to PnkCapsl */
#define BN5_SCREEN_CAPSULES   0x26        /* the Custom screen's two u16 at +0x26 and +0x28: the capsules' entries, drawn while +0x26
                                             is 0 by one draw of BN5's second RNG (0x080014D6: its halves, each & 0x1E an offset) */
#define BN5_CAPSULE_FIRST     380         /* the capsules' chip ids, 380-384: YelCapsl, BlkCapsl, WhiCapsl, PrpCapsl, PnkCapsl (their
                                             entries code 27, 0x377C-0x3780) */
#define BN5_CAPSULE_COUNT     5
#define BN5_ELEMENT_CAPSULE   6           /* an element's type: element 8's capsule (7 element 9's) */
#define BN5_ELEMENT_FLAGS     0x04        /* u8 a chip element's: bit 0 BN5's and BN6's own, bits 1-5 a capsule's (BN5_CAPSULE_BITS),
                                             copied at OK into the pending record's +0x44 + its place */
#define BN5_ELEMENT_CAPSULE_ON 0x05       /* u8 a chip element's: the element (8 or 9) of the capsule on it, 0 none */

/* The capsules lit, taken and given back */
#define BN5_CAPSULE_LIT       0x08024C78u /* the lit rule, the fifth of 0x08024A2A's: where element 8 is type 6, both lit (+7 0)
                                             while the last pick is a chip (type 0) with no capsule (+5 0) whose record shows its
                                             power (+9 bit 1), else grey; a chosen one (+7 2) left as it is */
#define BN5_A_BY_TYPE         0x080248D0u /* A's routines by element type, 10 words: 6 and 7 BN5_CAPSULE_A */
#define BN5_CAPSULE_A         0x08024A06u /* A on a capsule: lit, sound 0x81 and mode BN5_MODE_CAPSULE; grey, sound 0x69 */
#define BN5_MODE_CAPSULE      0x3C        /* the Custom screen's mode (its +1, the modes' table 0x08022DE4 by it): a capsule taken */
#define BN5_CAPSULE_MODE      0x08023736u /* its routine: its phase (+2) from BN5_CAPSULE_PHASES, then the screen's draws */
#define BN5_CAPSULE_PHASES    0x08023750u /* 7 words: BN5_CAPSULE_PHASE0, mode 0x24's four (UNITE's icon flying, 0x0802335C-0x080233C0),
                                             BN5_CAPSULE_PHASE5, BN5_CAPSULE_PHASE6 */
#define BN5_CAPSULE_PHASE0    0x0802376Cu /* the capsule's icon (BN5_CAPSULE_ICONS by id) to OBJ tiles 0x06017240, its palette to
                                             0x030036F0, +0x44 the picks x 16 + 0x18 (where it flies to) */
#define BN5_CAPSULE_ICONS     0x0874BA3Cu /* the capsules' icons, 0x80 bytes each by id - 380 (their records' +0x20), */
#define BN5_CAPSULE_ICON_PAL  0x0874BDBCu /* ... and their palette */
#define BN5_CAPSULE_LANDED    0x080237BAu /* ... in it, the flight landed (after its wait, 0x08005AAE): the capsule chosen and put on the pick */
#define BN5_CAPSULE_LIT_CALL  0x080237EAu /* ... and its call of the lit rules after (0x08024A2A) */
#define BN5_CAPSULE_PHASE5    0x080237B0u /* the capsule on the last pick: +7 2 (chosen), the pick's +5 the capsule's element, its +4
                                             ORed with BN5_CAPSULE_BITS by id, the lit rules again, sound 0x92 */
#define BN5_CAPSULE_BITS      0x08023820u /* 5 bytes by id - 380: 0x08, 0x04, 0x02, 0x20, 0x10 */
#define BN5_CAPSULE_PHASE6    0x08023828u /* back to mode 4 */
#define BN5_CAPSULE_UNDO      0x08024D7Cu /* B on the chips, the last pick given back: a capsule on it (+5) taken off (+5 0, +4 &
                                             ~0x3E) and lit again (its element's +7 0) */
#define BN5_CAPSULE_INFO      0x08024426u /* the chip's window for types 6 and 7: the capsule's name by BN5's chip names, its picture
                                             and palette by its record (+0x24, +0x28) */

/* The capsule in the fight */
#define BN5_CHIP_TAKEN        0x0800D04Cu /* the chip used: r4 the pending record's +0x3E + n << 8 | +0x44 + n (its flags) */
#define BN5_CHIP_FLAGS_DAMAGE 0x08010360u /* r0 the damage, r1 the flags: 2 -> 0x4000, 4 -> 0x2000, 8 -> 0x1000, 0x20 -> 0x0800 */
#define BN5_CAPSULE_HEAL      0x0800FFEEu /* in the chip's use (0x0800FF40): flag 0x10, (his max HP + 9) / 10 healed with it */
#define BN5_DAMAGE_DECODE     0x080165E4u /* a hit's damage word into its collision data (r7): & 0x7FF; 0x8000 doubled; 0x4000 the
                                             status +0x10 0x10, +0xE 0; 0x2000 0x20; 0x1000 0x30; 0x0800 its +0x12 0x18, +0x13 0x11 */
#define BN5_STATUS_TAKEN      0x08017334u /* a status taken (the collision data's +0x11): its high nibble a class of BN5_STATUSES, its
                                             low an entry (8 bytes: flags, frames, the field they go to) */
#define BN5_STATUSES          0x0801CEC0u /* ... the classes: 1 paralysis (+0x1E: 90 frames), 2 confusion (+0x20: 480), 3 blindness
                                             (+0x22: 480), 4 held (+0x24); BN6_STATUSES the same numbers, each field 2 lower */

/* ---- Gregar's: what BN6 kept of the capsules ---- */

#define BN6_SETUP_DUST        0x08027F10u /* (BN6_DUST_ELEMENTS) BN5's set-up's twin: DustCross's alone, no Soul 6 */
#define BN6_LIT_RULES         0x08028E32u /* the lit rules: four, BN5's first four's (DustCross's last), no capsule rule */
#define BN6_A_BY_TYPE         0x08028C9Cu /* A's routines by element type, BN5_A_BY_TYPE's: 6 and 7 BN6_A_CAPSULE */
#define BN6_A_CAPSULE         0x08028E30u /* mov pc,lr: A on a capsule takes nothing */
#define BN6_MODE_CAPSULE_STUB 0x0802753Eu /* mode 0x3C (the modes' table 0x08026AA4): straight back to mode 4, no phases */
#define BN6_CAPSULE_DRAW      0x080282E8u /* types 6 and 7 on the chips' screen (the draws by type, 0x08028370): drawn as a chip by
                                             their entry, its icon and code; nothing while chosen (BN5's 0x08024118) */
#define BN6_CAPSULE_INFO      0x080285FEu /* the chip's window for types 6 and 7 (0x080284CA): the name by BN6's chip names
                                             (0x08027D10), the picture and palette by BN6's record (0x08021AA4) of the entry's id */
#define BN6_CAPSULE_UNDO      0x080290D8u /* B on the chips: a capsule on the pick taken off and lit again, BN5_CAPSULE_UNDO's */
#define BN6_OK_FLAGS          0x0802915Au /* OK: a chip element's +4 into the pending record's +0x44 + n (0x08029214), BN5's */
#define BN6_CHIP_TAKEN        0x0800EDD0u /* BN5_CHIP_TAKEN's twin: r4 the same flags */
#define BN6_CHIP_FLAGS_DAMAGE 0x08012C34u /* BN5_CHIP_FLAGS_DAMAGE's: 2 -> 0x4000 and 4 -> 0x2000 alone (called at 0x08012884) */
#define BN6_CHIP_HEAL         0x080128C0u /* movs r2,#0: the chip's heal, BN6's own rule before it (forms 6 and 0x12, Aqua chips),
                                             where BN5's capsule heal stood, */
#define BN6_CHIP_HEAL_NEXT    0x080128C2u /* ... and the push of r2 after it */
#define BN6_UNITE_FLIGHT      0x080270D0u /* mode 0x24's phase 4 (UNITE's icon flying), its phase 8, */
#define BN6_UNITE_FLIGHT_8    0x080270EAu
#define BN6_UNITE_FLIGHT_C    0x08027118u /* ... 0xC */
#define BN6_UNITE_FLIGHT_10   0x08027134u /* ... and 0x10: BN5's 0x0802335C-0x080233C0 step for step, which BN5's mode 0x3C runs as
                                             its own phases 4-0x10 */
#define BN6_CUSTOM_DRAW_A     0x08029C08u /* the Custom screen's draws each of its modes runs after its phase (mode 0x38's, 0x0802741A; */
#define BN6_CUSTOM_DRAW_B     0x0802899Cu /* ... BN5's 0x08025828 and 0x08024798, the same steps on each game's screen) */
#define BN6_STATUS_TAKEN      0x0801A554u /* BN5_STATUS_TAKEN's twin, */
#define BN6_STATUSES          0x080209ECu /* ... its classes (BN6_COLL_PARALYSIS, BN6_COLL_CONFUSE, BN6_COLL_BLIND, BN6_COLL_IMMOBILE) */
#define BN6_DAMAGE_DECODE     0x08019F44u /* BN5_DAMAGE_DECODE's: 0x4000 the status 0x10 with +0xE 1; 0x2000 and 0x1000 its +0x12
                                             0xF8 and 0xF7 (0x0801A4D0); 0x0800 nothing */

#endif

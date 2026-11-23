/* MeddySoul's capsules on the Custom screen: BN5's Soul button for its Soul 6, and what Gregar kept of it (docs/multirom/souls.md,
 * Team ProtoMan's Souls, MeddySoul's capsules; docs/rom/souls.md, MeddySoul's capsules). Read from both ROMs and not run yet:
 * BN6's own screen draws a capsule as the chip its entry names (BN6_CAPSULE_DRAW, BN6_CAPSULE_INFO), and BN6's chip table holds
 * two of BN5's five capsules at their ids (383 PrpCapsl, 384 PnkCapsl: BN5's names, pictures and palettes, their icons' bytes
 * BN6's own), 382 WhiCapsl's name without its icon and picture, and IceCube and Z Saver where BN5 has 380 YelCapsl and
 * 381 BlkCapsl. */
#ifndef CW_BN5_CAPSULES_H
#define CW_BN5_CAPSULES_H

/* ---- BN5's (Team Colonel) ---- */

/* The Soul button's set-up (BN5's 0x08023CF4, r4 the elements, r5 the screen, r7 the united Soul): MeddySoul's part */
#define BN5_CAPSULES_SETUP    0x08023D1Cu /* cmp r7,#6: elements 8 and 9 made types 6 and 7, both grey (+7 1), no links set;
                                             their +8 the screen's BN5_SCREEN_CAPSULES and the halfword after */
#define BN5_CAPSULES_HEALTHY  0x08025E64u /* 16 chip entries the capsules are drawn from while MegaMan's HP (+0x24) is a quarter of
                                             his max (+0x26 >> 2) or more, weighted to YelCapsl, BlkCapsl and WhiCapsl, */
#define BN5_CAPSULES_HURT     0x08025E84u /* ... and 16 below a quarter, weighted to PnkCapsl */
#define BN5_SCREEN_CAPSULES   0x26        /* the Custom screen's two u16 at +0x26 and +0x28: the capsules' entries, drawn while +0x26
                                             is 0 by two of BN5's RNG draws (0x080014D6: the word's halves, each & 0x1E an offset) */
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
                                             where BN5's capsule heal stood */
#define BN6_DAMAGE_DECODE     0x08019F44u /* BN5_DAMAGE_DECODE's: 0x4000 the status 0x10 with +0xE 1; 0x2000 and 0x1000 its +0x12
                                             0xF8 and 0xF7 (0x0801A4D0); 0x0800 nothing */

#endif

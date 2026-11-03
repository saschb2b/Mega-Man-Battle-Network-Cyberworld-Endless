/* Battle Network 4: Red Sun (USA): what is mapped of it, read for research
 * alone so far (docs/rom/bn4.md; docs/multirom/bn4.md, the plan). BN4's
 * engine is BN5's and BN6's elder: its toolkit and chip records are laid
 * out as theirs, its battle records and enemy data are not. */
#ifndef CW_BN4_H
#define CW_BN4_H

#define BN4_TOOLKIT       0x02009E70u /* eToolkit, set up by `ldr r0,=eToolkit; mov r10,r0` (0x080060D8) as BN5's and BN6's are */
#define BN4_TOOLKIT_GAMESTATE 0x40    /* eToolkit +0x40: the game state (BN5's and BN6's at +0x3C) */
#define BN4_GAMESTATE_GROUP  0x04     /* the game state's +4: the map group (0x80 on the net) */
#define BN4_GAMESTATE_NUMBER 0x05     /* ... +5: the map's number in its group */
#define BN4_GAMESTATE_RECORD 0x2C     /* ... +0x2C: the battle record the encounter check chose, which the battle's setup reads (0x08006680) */
#define BN4_ENCOUNTER_CHECK 0x080F5370u /* the encounter check the map loop calls each frame (0x080053E6): on the net, its steps counted, a
                                       * record rolled (BN4_ROLL), stored at BN4_GAMESTATE_RECORD and returned (0 none); a record
                                       * returned starts its battle (0x080053F4). Patched to `ldr r0,=record; bx lr`, it starts that battle */
#define BN4_ROLL          0x080F5448u /* the roll: [[[BN4_BATTLE_TABLES + 4 on the net] + group * 4] + (number + its story offset) * 4], the
                                       * records whose BN4_RECORD_MASK meets r2's drawn by BN4_RECORD_WEIGHT (GetPositiveSignedRNG) */
#define BN4_STORY_OFFSET  0x080F54B8u /* the offset a net group's routine (one per group, by group - 0x80) adds to the map's number:
                                       * 0, or 0x10, 0x20 or 0x30 by story flags in the areas (ACDC Area's 0x080F5568) */
#define BN4_ENCOUNTER_RATES 0x080189E8u /* 16 bytes per net group (group - 0x80), one per map: its rate's class, 7 none */
#define BN4_BATTLE_TABLES 0x08017FB8u /* two pointers, the real world's map groups and the net's (group - 0x80, 21): per group a pointer
                                       * per map index (16 a story stage), per index a list of BN4_RECORD_SIZE records, 0xFF ending them */
#define BN4_RECORD_SIZE   12          /* a battle record: 8 bytes, then the pointer to its entities (BN5's and BN6's are 16 bytes) */
#define BN4_RECORD_WEIGHT 0x00        /* its weight in the roll (0x20 on ACDC Area's) */
#define BN4_RECORD_BACKDROP 0x04      /* the battle's background where the game state's +0xB is 0xFF (0x08007ADE) */
#define BN4_RECORD_MASK   0x06        /* tested against the roll's r2 (a progress byte, 0x17 of eToolkit +0x78's): 0x1F every one */
#define BN4_RECORD_ENTITIES 0x08      /* u32: its entity list, BN4_ENTITY_SIZE bytes each, 0xFF ending it */
#define BN4_ENTITY_SIZE   4           /* an entity: the enemy id (0 MegaMan), the panel's column and row, its kind */
#define BN4_ENTITY_KIND   0x03        /* ... its kind: 0 MegaMan, 1 an enemy, 5 an obstacle (id 1 a rock) */
#define BN4_ENEMY_STATS   0x08011888u /* a record per enemy id, BN4_ENEMY_STAT_SIZE bytes (0 MegaMan, 1-126 the viruses, 128 on the Navis) */
#define BN4_ENEMY_STAT_SIZE 12
#define BN4_ENEMY_COUNT   253         /* ids 0-252, as its enemy names archive (BN4_ENEMY_NAMES) holds them */
#define BN4_ENEMY_HP      0x00        /* u16: element << 12 | HP (Mettaur 40, VolGear Fire 80), as BN6's stats' first word */
#define BN4_ENEMY_KIND    0x03        /* 4 a virus, 8 a Navi */
#define BN4_ENEMY_FAMILY  0x04        /* the virus's family (1 Mettaur, 2 VolGear, 3 Billy) or the Navi's number (1 Roll, 2 GutsMan) */
#define BN4_ENEMY_VERSION 0x05        /* its version in its family: 0-5 (V1, EX, V2, EX, V3, EX) */
#define BN4_ENEMY_ATTACK  0x0A        /* u16: its attack's damage (Mettaur 10, Mettaur3 160) */
#define BN4_CHIP_RECORDS  0x080197ECu /* its chip records, 0x2C bytes each as BN6's (BN6_CHIP_RECORD_SIZE): codes at +0, power at +0x1A */
#define BN4_CHIP_COUNT    389         /* ids 0-388 */
#define BN4_GET_CHIP      0x080190D0u /* getChip: BN6's (0x08021AA4) found by its bytes, its literal BN4_CHIP_RECORDS */
#define BN4_CHIP_NAMES    0x087481D8u /* text archive: the chips' names, ids 0-255 (letters and digits spelt as BN6's) */
#define BN4_CHIP_NAMES_HI 0x08748C14u /* ... ids 256 on */
#define BN4_ENEMY_NAMES   0x0874A780u /* text archive: the enemies' names by id (a Navi's four versions in a row from 128) */
#define BN4_GET_RNG       0x08001B00u /* GetRNG: BN6's (0x0800151C) found by its calls and bytes */

#endif

/* BN6's chip tables as the engine grows and moves them, in BN6 Cybeast
 * Gregar (USA), part of bn6.h's addresses (it includes this): the words
 * and code that name the chip records, their names and descriptions, the
 * pack's routines and its extension in RAM, the pseudo-chips' test and the
 * battle deck's deal (issue #82; docs/MULTIROM.md, The chip registry;
 * docs/ROM_DATA.md, The chip registry; src/emu/xchips_install.c); and a
 * chip record's fields. */
#ifndef CW_BN6_CHIPS_H
#define CW_BN6_CHIPS_H

#define BN6_CHIP_TABLE_REF    0x08021AB0u /* getChip8021DA8's literal: the chip records (0x08021DA8, 411 of them), the one word that names them */
#define BN6_PACK_OFFSET_TAIL  0x08021C9Au /* getOffsetToQuantityOfChipCode's tail: mov r7,sl; ldr r7,[r7,#0x4c] (the pack) ... pop {r4,r7,pc} */
#define BN6_PACK_TOTAL_TAIL   0x08021BE4u /* GetTotalChipCount's: the same, then the chip's four counts summed, pop {r7,pc} */
#define BN6_PACK_SUM_COUNT    0x08021CD6u /* bn6f sub_8021CA8, the pack's totals (the save menu's count): a count read so, 7 instructions */
#define BN6_PACK_SUM_END      0x08021D04u /* ... the literal its loop over the ids ends at, 0x140 */
#define BN6_PACK_CLEAR_CALL   0x08021C70u /* zeroFill_e2002230 (a NEW GAME, the folder editor's exit before it writes the pack again): bl ZeroFillByWord, 0xF00 bytes */
#define BN6_PACK_LIST_END     0x08121D2Cu /* bn6f sub_811FE7C, the pack's list (the folder editor's): the literal its ids end at, 0x13A (to 314 inclusive) */
#define BN6_PSEUDO_TEST       0x080ED11Cu /* the support family's chip routine (bn6f sub_80EBD9C): cmp r2,r3 (its id, 0x170); blt; movs r1,#1: a pseudo-chip's freeze */
#define BN6_PSEUDO_POOL       0x080ED13Cu /* ... its literal 0x170 */
#define BN6_PSEUDO_TEST_NAVI  0x080ED6C4u /* the Navi chips' (bn6f sub_80EC350): the same */
#define BN6_PSEUDO_POOL_NAVI  0x080ED6D8u /* ... its literal 0x170 */
#define BN6_DECK_DEALT        0x0800A4D2u /* bn6f sub_800A3E4 (the battle's deck from the folder, the Regular chip first, the TagChips, shuffled) at its end: add sp,#0xc */
#define BN6_DECK_SIZE         30          /* BN6_BATTLE_DECK's entries */
#define BN6_BATTLE_REGULAR_FIRST (BN6_BATTLE_STATE + 0x17) /* 1 where the deck's first chip is the folder's Regular chip (bn6f BattleState Unk_17) */
#define BN6_BATTLE_TAGS_FIRST (BN6_BATTLE_STATE + 0x44) /* 1 where the folder's TagChips follow it (Unk_44) */

/* A chip record's fields (RomLayout.chip_data, 0x2C bytes an id, the
 * registry's grown copy past EMU_FREE as the core reads them; bn6f
 * ChipData; docs/ROM_DATA.md, BN6's own DarkChips and The chip registry),
 * as BN6's code reads them and the registry lays out BN5's */
#define BN6_CHIP_RECORD_SIZE  0x2C
#define BN6_CHIP_CODES        0x00        /* Codes: four, 0-25 A-Z, 26 *, 0xFF none (the pack counts a chip per code of these) */
#define BN6_CHIP_ATTACK_ELEMENT 0x04      /* AttackElement: 0 Null, 1 Fire, 2 Aqua, 3 Elec, 4 Wood */
#define BN6_CHIP_RARITY       0x05        /* Rarity: 0-4 */
#define BN6_CHIP_ELEMENT      0x06        /* ChipElement: its icon's element (0 Fire, 1 Aqua, 2 Elec, 3 Wood, 4 Plus, 5 Sword, 6 Cursor, 7 Obstacle, 8 Wind, 9 Break, 10 Null,
                                             11 a Program Advance; Gregar's records give DrilArm and AirHocky 9), which a hit ORs a second-wheel element by (0x080129E4) */
#define BN6_CHIP_LIBRARY_TYPE 0x07        /* LibraryType: 0 standard, 1 Mega, 2 Giga, 3 secret, 4 a Program Advance */
#define BN6_CHIP_MB           0x08        /* MB: its memory (a Regular chip's at most the Reg memory) */
#define BN6_CHIP_EFFECT_FLAGS 0x09        /* EffectFlags: BN6_CHIP_DARK_CLASS makes a DarkChip (the folder's three, MegaMan's NAVIGATOR line, the purple card) */
#define BN6_CHIP_FAMILY       0x0B        /* AttackFamily: its routine (0x080EBFA0, by family - 0x10), CurAction through object_setAttack2 */
#define BN6_CHIP_SUBFAMILY    0x0C        /* AttackSubFamily: a recovery chip's amount (BN6_RECOVERY_AMOUNTS), a sword's area */
#define BN6_CHIP_LOCK_ON      0x0F        /* LockOnEnable: 1 a chip that strikes an enemy outright */
#define BN6_CHIP_PARAMS       0x10        /* AttackParam1-4, a byte each, then LockoutFrames at +0x14 */
#define BN6_CHIP_LIBRARY_NUMBER 0x15      /* LibraryNum: its number in the PET's Library tab (a byte; 0 none) */
#define BN6_CHIP_LIBRARY_FLAGS 0x16       /* LibraryFlags: BN6_CHIP_UNLISTED keeps a chip out of the pack's list (bn6f sub_811FE7C) */
#define BN6_CHIP_LOCK_ON_TYPE 0x17        /* LockOnType: how it locks on (BN6's alone: BN5's byte here means otherwise) */
#define BN6_CHIP_GATE         0x1E        /* ChipGateUsageLimit: the BattleChip Gate's (Japan's physical reader) */
#define BN6_CHIP_IN_LIBRARY   0x40        /* BN6_CHIP_EFFECT_FLAGS: listed on the PET's Library screen, by its library number */
#define BN6_CHIP_SHOWS_POWER  0x02        /* BN6_CHIP_EFFECT_FLAGS: its power shown (the pack's sort by power, the Custom screen) */
#define BN6_CHIP_COMPUTED_POWER 0x80      /* BN6_CHIP_EFFECT_FLAGS: its power one BN6 computes (BN6_CHIP_ATTACK_POWER 1000 and up: BN6_CHIP_POWERS) */
#define BN6_CHIP_ALPHA_SORT   0x18        /* AlphabetSortPos, u16: the folder editor's ABC order */
#define BN6_CHIP_ATTACK_POWER 0x1A        /* AttackPower, u16: what the Custom screen and the hand show, and a chip deals */
#define BN6_CHIP_ID_SORT      0x1C        /* IDSortPos, u16: the editor's ID order, and with the code the key its swap tells two chips apart by (bn6f
                                             sub_811FCB8, sub_811FE7C); 0 on the five DarkChips, so it took any two for one chip and skipped its limits */
#define BN6_CHIP_DARK_ID      0x1F        /* DarkChipID: 0-4 BN6's five DarkChips, 0xFF any other chip */
#define BN6_CHIP_ICON_PTR     0x20        /* ChipIconPtr, then ChipImagePtr and ChipPalettePtr: the hand's icon, the card's picture */
#define BN6_CHIP_IMAGE_PTR    0x24
#define BN6_CHIP_PALETTE_PTR  0x28
#define BN6_CHIP_DARK_CLASS   0x20
#define BN6_CHIP_UNLISTED     0x20

/* How a chip runs (docs/ROM_DATA.md, The attack families and chip
 * routines grown): its record's family (BN6_CHIP_FAMILY) is the CurAction
 * its user takes (object_setAttack2), which RunAIAttack (0x0801B9E6) runs
 * from the attack families' table by CurAction - 0x10, unbounded (a byte:
 * up to 0xFF), as the lock-on path does (0x080EC276); family 0x15 (support)
 * runs a routine of the chip routines' table by its subfamily, and family
 * 0x1B (the Navi chips) one of its Navi part's from the Navi object it
 * spawns (0x080E2C6C, the fourth kind's type 0x10) */
#define BN6_ATTACK_FAMILIES   0x080EBFA0u /* a Thumb routine per family 0x10-0x5E (BN6_ATTACK_FAMILY_COUNT), code from 0x080EC0DC */
#define BN6_ATTACK_FAMILIES_REF 0x0801BB10u /* ... RunAIAttack's literal naming it, */
#define BN6_ATTACK_FAMILIES_REF2 0x080EC384u /* ... and the lock-on path's (0x080EC280): the only two words of the ROM that do */
#define BN6_ATTACK_FAMILY_FIRST 0x10      /* CurAction from here is a family's (below it a generic state of the object's) */
#define BN6_ATTACK_FAMILY_COUNT 79        /* ... BN6's: 0x10-0x5E */
#define BN6_EXIT_ATTACK       0x08011714u /* object_exitAttackState: the attack ended (AttackVars +0x1B, AIData +0x44's bits, CurAction 8) */
#define BN6_CHIP_ROUTINES     0x0802CCB4u /* the support family's routines by subfamily (0x00-0x29, BN6_CHIP_ROUTINE_COUNT), the Navi chips' after them: */
#define BN6_CHIP_ROUTINE_COUNT 0x2A
#define BN6_CHIP_POWERS       0x080109DCu /* the computed powers' routines (a chip's power of 1000 and up: the routine by power - 1000, r0 the */
#define BN6_CHIP_POWERS_COUNT 45          /* ... chip, r1 its side; bn6f 0x080109A4's table), 45 of them, code after */
#define BN6_CHIP_POWERS_REF   0x080109D4u /* ... the one word naming the table (0x080109C2: ldr r2, then by the power's index) */
#define BN6_NAVI_ROUTINES     0x0802CD5Cu /* ... from entry 0x2A, by the Navi chip's subfamily (BN6_NAVI_ROUTINE_COUNT) */
#define BN6_NAVI_ROUTINE_COUNT 29
#define BN6_CHIP_ROUTINES_REF 0x080ED138u /* the support family's literal naming BN6_CHIP_ROUTINES (0x080ED0EE: ldr r3, then by AttackVars +3) */
#define BN6_CHIP_ROUTINES_REF2 0x08017BB8u /* ... a second (0x08017B0C: a chip a Navi or virus uses, its family 0x15 run so) */
#define BN6_NAVI_ROUTINES_REF 0x080E2CF0u /* the Navi object's literal naming BN6_NAVI_ROUTINES (the fourth kind's type 0x10) */
#define BN6_NAVI_ROUTINES_REF2 0x08017BBCu /* ... a second, beside BN6_CHIP_ROUTINES_REF2, which no instruction loads (pointed at the copy all the same) */
#define BN6_CHIP_SUPPORT_FAMILY 0x15      /* the support chips' family */
#define BN6_CHIP_NAVI_FAMILY  0x1B        /* the Navi chips' family */

/* The pack (eToolkit +0x4C, BN6_TOOLKIT_PACK) and its extension */
#define BN6_PACK_IDS          320         /* the pack's ids (0x02002230 to 0x02003130; the key items' counts from 0x02003134) */
#define BN6_PACK_ENTRY        12          /* a pack entry's bytes (CHIP_PACK_ENTRY): a count per code of the chip's record's four first */
#define BN6_PACK_EXT          0x02010CE0u /* the pack's extension, ids BN6_PACK_IDS-511, 12 bytes each to 0x020115E0: EWRAM BN6 leaves alone (docs/EMULATION.md) */

#endif

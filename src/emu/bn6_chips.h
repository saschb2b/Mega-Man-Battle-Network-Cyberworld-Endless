/* BN6's chip tables as the engine grows and moves them, in BN6 Cybeast
 * Gregar (USA), part of bn6.h's addresses (it includes this): the words
 * and code that name the chip records, their names and descriptions, the
 * pack's routines and its extension in RAM, the pseudo-chips' test and the
 * battle deck's deal (issue #82; docs/MULTIROM.md, The chip registry;
 * docs/ROM_DATA.md, The chip registry; src/emu/xchips_install.c). */
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

/* The pack (eToolkit +0x4C, BN6_TOOLKIT_PACK) and its extension */
#define BN6_PACK_IDS          320         /* the pack's ids (0x02002230 to 0x02003130; the key items' counts from 0x02003134) */
#define BN6_PACK_EXT          0x02010CE0u /* the pack's extension, ids BN6_PACK_IDS-511, 12 bytes each to 0x020115E0: EWRAM BN6 leaves alone (docs/EMULATION.md) */

#endif

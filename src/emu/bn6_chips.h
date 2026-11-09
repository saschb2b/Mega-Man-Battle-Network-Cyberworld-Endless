/* BN6's chip tables as the engine grows and moves them, in BN6 Cybeast
 * Gregar (USA), part of bn6.h's addresses (it includes this): the words
 * and code that name the chip records, their names and descriptions, the
 * pack's routines and its extension in RAM, the pseudo-chips' test and the
 * battle deck's deal (issue #82; docs/multirom/chip-registry.md;
 * docs/rom/chips-code.md, The chip registry; src/emu/xchips_install.c); a chip
 * record's fields; and BN6's sorts of the chips, the folder editor's and the
 * Chip Trader's (issue #124: the Order Service's, src/world/order_sort.c). */
#ifndef CW_BN6_CHIPS_H
#define CW_BN6_CHIPS_H

#include "chip_entry.h"   /* a folder entry's form: its id's bits */

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
 * ChipData; docs/rom/chips-code.md, BN6's own DarkChips and The chip registry),
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
#define BN6_CHIP_LEFT_ALONE   0x01        /* BN6_CHIP_EFFECT_FLAGS: a chip no charge and no field's double takes (BN6's tests 0x08013236, 0x08012B2A,
                                             BN5's 0x08010916, 0x0801034E: the same bit in both) */
#define BN6_CHIP_COMPUTED_POWER 0x80      /* BN6_CHIP_EFFECT_FLAGS: its power one BN6 computes (BN6_CHIP_ATTACK_POWER 1000 and up: BN6_CHIP_POWERS) */
#define BN6_CHIP_NAVI_FLAG    0x04        /* BN6_CHIP_EFFECT_FLAGS: a Navi chip, which Navi+20 joins in the hand (bn6f sub_8029224) */
#define BN6_CHIP_ATK10        0xC0        /* the plus chips the hand's merge (bn6f sub_8029224, 0x08029224) adds to the chip before: Atk+10 and */
#define BN6_CHIP_ROCKCUBE     0x8F        /* RockCube: an Obstacle, the kind ColonelSoul unites with (tests/xdiff/soul_colonel_chip.json's 0x348F) */
#define BN6_CHIP_ATK30        0xC3        /* ... Atk+30 to one with BN6_CHIP_SHOWS_POWER, */
#define BN6_CHIP_NAVI20       0xC1        /* ... Navi+20 to one with BN6_CHIP_NAVI_FLAG */
#define BN6_CHIP_CANNON       0x01        /* Cannon: three in a row of codes (A, B, C) picked in order are a Program Advance, */
#define BN6_PA_ZCANNON1       0x140       /* ... this one (a record of OK's check 0x08029520: count 3, kind 0, this id, chip 1) */
#define BN6_PA_FIRST          0x140       /* the Program Advances' chip ids, the first and the last of the records OK's check walks (its tables'
                                             words at 0x080295B4 and 0x080295B8: 30 records, 3 or 4 chips each) */
#define BN6_PA_LAST           0x15D       /* ... the last (Gregar's records' ids, read from both tables) */
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
#define BN6_CHIP_NOTHING      0x185       /* the chip of nothing: OK's in a hand for a chip not owned (0x0800B090), its family 0x1C's spawner
                                             BN6_SPAWNER_NOTHING (0x08010488); BN5's 0x185 the same (family 0x1A, spawner 3 0x0800E190), its dud */
#define BN6_SPAWNER_NOTHING   3

/* How a chip runs (docs/rom/chips-code.md, The attack families and chip
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

#define BN6_TAKE_CHIPS        0x08021B93u /* TakeChips (chip, code, count): a settled Net Square trade's chip out of the Pack */

/* Where BN6's code takes a chip's id in nine bits (tools/chip_ids.py's census, docs/rom/chips-code.md, Ten-bit chip ids) */
#define BN6_ENTRY_SPLIT       0x08021AE0u /* a folder entry split: r1 its code (lsrs #9), r0 its id (ands with its literal's low half, 0x01FF) */
#define BN6_ENTRY_SPLIT_MASK  0x08021AECu /* ... that literal: 0x01FF, its high half the next routine's first instruction */
#define BN6_ENTRY_CHEAT       0x0802A55Cu /* the entry the Custom screen's two getters give for a chip the cheat test refuses: 0x3785, 389 in code 27 */
#define BN6_REWARD_SPLIT      0x0802C54Cu /* a battle's reward split (0xFFFF none): kind from bit 14, code from bit 9, id in 9 bits; stays nine bits */
#define BN6_LINK_CHIP_BOUND   0x0802E1B4u /* the link battle's real-time chip (battle flag 0x40): an id past 0x139 read as 0 */
#define BN6_EDITOR_LIST_OUT   0x081224A0u /* the folder editor's list given back (GiveChips): its items id << 7 | code (a halfword) */
#define BN6_EDITOR_LIST_IN    0x081224D4u /* ... the list into the folder (eToolkit +0x48): an entry made of each item, code << 9 | id */
#define BN6_PRESET_FOLDERS    0x080213ACu /* the preset folders GiveFolder copies, 0x3C bytes each: two here, */
#define BN6_PRESET_FOLDERS2   0x0802158Cu /* ... and four here (docs/rom/chips.md, Chip folders) */
#define BN6_CHIP_NAME_OF      0x08027D10u /* a chip's name: r0 its archive, r1 its entry, by id > 255 of two (the archives' words at 0x08027D2C, its literal at +0x18) */
#define BN6_CHIP_DESC_OF      0x08027D34u /* ... its description (0x08027D50, its literal at +0x18) */
#define BN6_PACK_MARK_STORE   0x08006E80u /* encryption_applyPack's strb r1,[r7,r0]: a chip's mark written, by id */
#define BN6_PACK_MARK_TEST    0x08006E96u /* encryption_testPack's cmp r1,r2: its key against its mark, equal where owned */
/* Ids past 511 where BN6's code takes a name or a description of two archives, marks a chip for the Library or splits a
 * reward (docs/rom/chips-code.md, The pack past 511) */
#define BN6_CHIP_NAMES_REF_RESULTS 0x0802C7C0u /* a literal naming the names' two archives' words, which its reader indexes by the id's high
                                             byte: the battle results' (0x0802C76C), */
#define BN6_CHIP_NAMES_REF_LINE 0x080476B0u /* ... a chip's name drawn as a line of tiles (0x0804763E), */
#define BN6_CHIP_NAMES_REF_LIST 0x08121F50u /* ... the folder editor's lists' (0x08121EEE), */
#define BN6_CHIP_NAMES_REF_PACK 0x08121FF0u /* ... its two more (0x08121FC0 and BN6_CHIP_NAMES_REF_PACK2's 0x08122028), whose third word is */
#define BN6_CHIP_NAMES_REF_PACK2 0x08122048u /* ... the archive of an empty entry's text, a RAM address (0x0201BA20), */
#define BN6_CHIP_NAMES_NONE   0x08121FBAu /* ... which each loads by ldr r0,[r0,#8] (0x6880), here and */
#define BN6_CHIP_NAMES_NONE2  0x08122022u /* ... here */
#define BN6_CHIP_NAMES_REF_LIBRARY 0x08128190u /* ... and the PET's Library's (0x0812807A, 0x08128134) */
#define BN6_CHIP_DESC_READ    0x0811A88Cu /* a description's archive picked of two by id > 255, five instructions (ldr; ldrh r1,[r5,#0xe]; cmp
                                             r1,#0xff; bgt; ldr), its first archive's literal at +0x130: a chip's card in the PET, */
#define BN6_CHIP_DESC_READ2   0x0811B280u /* ... and a second (its literal at +0x13C) */
#define BN6_CHAT_NAME_ARCHIVE 0x08041FEAu /* the chat box's buffer command taking its archive, ldr r0,[r0,r2]: r2 four times the command's kind
                                             (its +3's low four bits) plus the value's bits 8-11, into the table at 0x08042034, whose 1 and 2
                                             are the chip names' archives; r4 the command */
#define BN6_CHAT_CHIP_NAME    1           /* ... the kind that prints a chip's name */
#define BN6_GIVE_CHIPS_LIBRARY 0x08021B08u /* GiveChips' bl SetEventFlag, r0 BN6_FLAG_LIBRARY + the chip, */
#define BN6_GIVE_FOLDER_LIBRARY 0x08021B44u /* ... and bn6f sub_8021B2A's (a folder's chips given) */
#define BN6_REWARD_SPLIT_HP   0x0802C56Cu /* BN6_REWARD_SPLIT past a chip's and zenny's cases: cmp r1,#2, r0 the reward, r1 its kind (2 or 3); it
                                             returns by mov pc,lr with r0 the kind, r1 the id or amount, r2 a chip's code, nothing pushed */
#define BN6_FOLDERS           3           /* Folder1, Folder2 and the ExtraFolder at eToolkit +0x48, BN6_FOLDER_ENTRIES each, 0x3C apart */
#define BN6_FLAG_LIBRARY_IDS  512         /* BN6_FLAG_LIBRARY + chip id holds ids 0-511: 0x2020 on is another block */

/* The pack (eToolkit +0x4C, BN6_TOOLKIT_PACK) and its extension */
#define BN6_PACK_IDS          320         /* the pack's ids (0x02002230 to 0x02003130; the key items' counts from 0x02003134) */
#define BN6_PACK_ENTRY        12          /* a pack entry's bytes (CHIP_PACK_ENTRY): a count per code of the chip's record's four first */
#define BN6_PACK_EXT          0x02010CE0u /* the pack's extension, 192 entries of 12 bytes to 0x020115E0: EWRAM BN6 leaves alone (docs/engine/emulation/) */
#define BN6_PACK_EXT_IDS      512         /* ... as if ids BN6_PACK_IDS on followed, to here: the marks' and their keys' ids end here too */
#define BN6_PACK_EXT_FIRST    411         /* ... the first id a pack holds there: 320-410 are Program Advances and BN6's pseudo-chips, which no
                                             pack counts, their entries the wide ids' */
#define BN6_PACK_WIDE_FIRST   512         /* the ids past nine bits a pack counts: these, in the entries of ids BN6_PACK_IDS on, */
#define BN6_PACK_WIDE_END     (BN6_PACK_WIDE_FIRST + BN6_PACK_EXT_FIRST - BN6_PACK_IDS) /* ... to here, 603 (docs/rom/chips-code.md, The
                                             pack past 511); an id past them has no entry: counted 0, a chip given lost */

/* Chip `id`'s entry of the pack at `pack` (eToolkit +0x4C's address), 0 where it has none: as the engine's routine at
 * CHIPS_CODE answers BN6's own (xchips_install.c) */
static inline uint32_t bn6_pack_entry(uint32_t pack, unsigned id) {
	if (id < BN6_PACK_IDS) return pack + BN6_PACK_ENTRY * id;
	if (id >= BN6_PACK_EXT_FIRST && id < BN6_PACK_EXT_IDS) return BN6_PACK_EXT + BN6_PACK_ENTRY * (id - BN6_PACK_IDS);
	if (id >= BN6_PACK_WIDE_FIRST && id < BN6_PACK_WIDE_END) return BN6_PACK_EXT + BN6_PACK_ENTRY * (id - BN6_PACK_WIDE_FIRST);
	return 0;
}

/* What BN5's chips' moved code stands on in BN6's own (xchips_flash.c, xchips_code.c) */
#define BN6_FLASH_COUNT       0x080107DCu /* sub_80107D4 (the player's timers, a frame each): ldr r4,[r5,#0x58] past its time stop's test (0x0800A098) */
#define BN6_FLASH_COUNT_OP    0x6DAC      /* ... its ldr r4,[r5,#0x58] there, checked before the hook */
#define BN6_FLASH_TINT        0x0801B15Au /* an object's colours' chain (0x0801AF44): bl 0x080168C8, BN5's 0x08013738 (BN5 tints before it) */
#define BN6_FLASH_TINT_BL     0xFBB5F7FBu /* ... its two halves */
#define BN6_OBJ_SPRITE_BLOCK  0x02        /* an object's +2: its sprite state's block, the high four bits times 16 (sprite_setColorShader 0x08002ED0) */
#define BN6_SPRITE_SHADER     0x06        /* ... the colour shader there, u16 (strh r0,[r3,#6]) */
#define BN6_DARK_AFTERS       0x0800B7BCu /* BN6_DARK_AFTER's table by the DarkChip's id less 0x11E, a routine each: */
#define BN6_DARK_AFTER_HPBUG  0x0800B80Eu /* ... DarkPlus's (its fifth): the NaviStats' HP bug +4, at most 7 */
#define BN6_DARKINVS          0x121       /* BN6's DarkInvs (its DarkChipID 3) */
#define BN6_DARKPLUS          0x122       /* BN6's DarkPlus (its DarkChipID 4) */
#define BN6_CHIP_WRITER       0x080126E4u /* the chip writer, its first entry (r4 1: the chip's use logged; a virus's held chip, BN5's
                                             0x0800FE6A): AttackVars from the record of r0, its family returned */
#define BN6_SUPPORT_RUN       0x080ED0DCu /* the support family 0x15's routine: phase 0 the chip's routine, then +1 1; phase 1 the attack's end
                                             (BN5's 0x080EC400 ends it in phase 0's frame) */
#define BN6_PA_LIST           0x0802BCB0u /* the Program Advances' list the Custom screen walks (0x08029520): a recipe's address each, */
#define BN6_PA_COUNT          43          /* ... these, then 0 (0x0802BD5C) */
#define BN6_PA_LIST_REF       0x080295B4u /* ... its one word (loaded at 0x08029556; the link battles' list, 0x0802BC60, its own word 0x080295B8) */
#define BN6_PA_LIST_LINK_REF  0x080295B8u /* ... the link battles' list's word: 0x0802BC60, twenty recipes of its own, then BN6_PA_LIST's own
                                             (contiguous, ended by its 0); a partner's Program Advance checked against both */
#define BN6_PA_LETTERS        0x0802B99Cu /* the P.A. screen's literal 0x160: a part's code letter drawn below that id alone (0x0802B846) */
#define BN6_PA_LIBRARY_BOUND  0x08127AE4u /* the Library's P.A. tab's literal 0x1E: its builder's ids 320 to 320 + it (ble, 0x081279E4) */
#define BN6_NAVI_FOLDER       (BN6_NAVI_STATS + 0x2D) /* the folder in use (0-2), its 30 chips at eToolkit +0x48's + 0x3C times it (0x0800A422) */
#define BN6_NAVI_FOLDER2_REG  (BN6_NAVI_STATS + 0x2F) /* Folder2Reg: the second folder's Regular chip, its entry; 0xFF none (bn6f NaviStats) */
#define BN6_NAVI_FOLDER2_TAG  (BN6_NAVI_STATS + 0x58) /* Folder2Tag1 and Tag2: its TagChips, their entries; 0xFF none */
#define BN6_NAVI_FOLDER2_TAG2 (BN6_NAVI_STATS + 0x59)

/* BN6's blank chip records below its last id, 0x13F (docs/rom/chips.md, the
 * chip records and the chip registry): no chip of BN6's, where BN5's chips
 * are installed when its ROM is there. A rule of BN6's alone, so two games
 * agree on it whatever ROMs each has (the Net Square's trades and Arena). */
#define BN6_CHIP_BLANK(id) (((id) >= 203 && (id) <= 220) || ((id) >= 291 && (id) <= 300) || (id) == 314)

/* BN6's sorts (bn6f sub_8120018, Gregar 0x08121DF4; the keys bn6f
 * sub_811FE7C's): seven keys, ID, ABCDE, Code, Attack, Element, No., MB, each
 * up or down, its ties by ID up either way; a key newly picked goes the way
 * of its byte here XOR BN6_SORT_DOWN, picked again the other (bn6f
 * sub_804AE04, the Chip Trader's SORT window) */
#define BN6_SORT_MODES        0x030069A0u /* IWRAM data, a byte a key: 0, or BN6_SORT_DOWN (04 04 04 00 04 00 04 04: Attack and No. down) */
#define BN6_SORT_DOWN         0x04        /* ... a sort's way down */
#define BN6_SORT_ELEMENTS     0x08121D3Cu /* a chip's place in the Element sort, a byte by its ChipElement (bn6f byte_811FF60), BN6_SORT_ELEMENT_COUNT */
#define BN6_SORT_ELEMENT_COUNT 12
#define BN6_SORT_NO_POWER     5           /* the Attack sort's key for a chip whose power is not shown (bn6f sub_811FF80) */
#define BN6_SORT_POWER_ODD    1           /* ... and for one of BN6_CHIP_EFFECT_FLAGS' BN6_SORT_ODD_FLAG */
#define BN6_SORT_ODD_FLAG     0x10
#define BN6_SORT_COMPUTED     999         /* ... a shown power past this one BN6 computes, a Navi chip's: */
/* A Navi chip's power (BN6_NAVI_POWER_FIRST and up), by MegaMan's best
 * time against the Navi (bn6f sub_8010C76, Gregar's as Falzar's) */
#define BN6_NAVI_POWER_FIRST  1001
#define BN6_NAVI_TIMES        0x020018C0u /* u16 frames a Navi, by power - BN6_NAVI_POWER_FIRST */
#define BN6_NAVI_TIME_STEPS   0x08010CB8u /* BN6_NAVI_TIME_STEP_COUNT u32 times in BCD (0x1000 10 seconds): the first the time is not past, its step */
#define BN6_NAVI_TIME_STEP_COUNT 10
#define BN6_NAVI_POWERS       0x08020E54u /* u16 by step, BN6_NAVI_POWER_STEPS a Navi */
#define BN6_NAVI_POWER_STEPS  11          /* ... a step a time and one past the last */

#endif

/* xchips_bn4.c: BN4's chips' code as units for BN6's battle engine, written by tools/xchiprecipe_bn4.py from the two
 * ROMs (python3 tools/xchiprecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator
 * (or tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/bn4-chips.md):
 * addresses in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from
 * the player's ROM; nothing of either game's. The units are the generator's, whole (no decision open), in install order;
 * src/emu/xchips_code.c installs them, and the registry's XREG_MOVED rows of BN4's (src/core/xchips_registry_bn4.c) name
 * the chips that run them. */
#include "xchips_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"

/* ---- PanlSht: BN4's support 0x26 (080E7A59) as BN6's 0xa6; its chips 178 (559), 179 (560), 180 (561) ---- */
/* its fourth-kind 0x4f (080E79B8-080E7BF0), which its routine (080E7A58) spawns: moved */
/* its sprite 3:0x38 (080DCAD0): BN6's 3:0x33, the same bytes */
/* its sound 0xbf (080DCBEC): BN4's song copied into BN6's empty slot 0x4a, on BN4's player 9 (1 track; BN6's player 9 has 2) */
/* its sound 0x1c6 (080DCB58): BN4's song copied into BN6's empty slot 0x49, on BN4's player 17 (1 track; BN6's player 17 has 1) */

static const XvBlock panlsht_blocks[] = {
	{ 0x080E79B8, 0x080E7BF0, XC4_CODE + 0x0, 0x4F07EE8Bu },
	{ 0x080DCAA4, 0x080DCC5C, XC4_CODE + 0x240, 0x7D7EC92Fu },
};
static const XvData panlsht_data[] = {
	{ 0x080E79C6, 0x080E79C8, XV_RAW },
	{ 0x080E79C8, 0x080E79CC, XV_POOL },
	{ 0x080E79CC, 0x080E79D8, XV_TABLE },
	{ 0x080E79E6, 0x080E79E8, XV_RAW },
	{ 0x080E79E8, 0x080E79EC, XV_POOL },
	{ 0x080E79EC, 0x080E79FC, XV_TABLE },
	{ 0x080E7A0A, 0x080E7A0C, XV_RAW },
	{ 0x080E7A0C, 0x080E7A10, XV_POOL },
	{ 0x080E7A10, 0x080E7A18, XV_TABLE },
	{ 0x080E7A8A, 0x080E7A8C, XV_RAW },
	{ 0x080E7A8C, 0x080E7A90, XV_POOL },
	{ 0x080E7A90, 0x080E7A9C, XV_TABLE },
	{ 0x080E7B34, 0x080E7B38, XV_POOL },
	{ 0x080E7B38, 0x080E7B3C, XV_RAW },
	{ 0x080E7B96, 0x080E7B98, XV_RAW },
	{ 0x080E7B98, 0x080E7B9C, XV_POOL },
	{ 0x080E7B9C, 0x080E7BA8, XV_RAW },
	{ 0x080E7BDC, 0x080E7BE0, XV_POOL },
	{ 0x080E7BE0, 0x080E7BF0, XV_RAW },
	{ 0x080DCAB6, 0x080DCAB8, XV_RAW },
	{ 0x080DCAB8, 0x080DCABC, XV_POOL },
	{ 0x080DCABC, 0x080DCAC8, XV_TABLE },
	{ 0x080DCBA0, 0x080DCBA4, XV_POOL },
	{ 0x080DCBA4, 0x080DCBB0, XV_TABLE },
	{ 0x080DCC56, 0x080DCC58, XV_RAW },
	{ 0x080DCC58, 0x080DCC5C, XV_POOL },
};
static const XvPair panlsht_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009BAC, 0x0800C9F8 },   /* project:project:chips, DIFF (audited) */
	{ 0x08009E98, 0x0800CC72 },   /* search, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
};
static const XvPair panlsht_words[] = {
	{ 0x08008BD9, 0x0800B917 },   /* project:project:chips, DIFF */
	{ 0x08008BF9, 0x0800B94D },   /* project:project:chips, SAME */
	{ 0x08008C2B, 0x0800B9B1 },   /* project:project:chips, DIFF */
	{ 0x08008EB9, 0x0800BC89 },   /* project:project:chips, SAME */
	{ 0x08008EEF, 0x0800BD35 },   /* project:project:chips, DIFF */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm panlsht_imms[] = {
	{ 0x080DCACE, THUMB_MOVS, 2, 0, 0x38, 0x33 },   /* its object's sprite: list 3 0x38 in BN4, 0x33 in BN6 (the same bytes) */
	{ 0x080DCB24, THUMB_LDRB, 0, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080DCB54, THUMB_MOVS, 0, 0, 0xC7, 0x49 },   /* its sound 0x1c6 (a movs and an adds): BN4's copied into BN6's empty slot 0x49 */
	{ 0x080DCB56, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x1c6 (a movs and an adds): BN4's copied into BN6's empty slot 0x49 */
	{ 0x080DCB72, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DCBEA, THUMB_MOVS, 0, 0, 0xBF, 0x4A },   /* its sound 0xbf: BN4's copied into BN6's empty slot 0x4a, on its player 9 */
	{ 0x080DCC2E, THUMB_MOVS, 0, 0, 0x7B, 0xCA },   /* its object's type 0x7b: moved, the dispatch's 0xca */
	{ 0x080DCC3E, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7A5C, THUMB_MOVS, 0, 0, 0x4F, 0x91 },   /* its object's type 0x4f: moved, the dispatch's 0x91 */
	{ 0x080E7A6C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7AB8, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7B0E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7B74, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
};
static const XvType panlsht_types[] = {
	{ 3, 0xCA, XV_TYPE_MOVED, 0x7B },   /* BN4's third-kind 0x7b, moved, the dispatch's 0xca */
	{ 4, 0x91, XV_TYPE_MOVED, 0x4F },   /* BN4's fourth-kind 0x4f, moved, the dispatch's 0x91 */
};
static const XcSong panlsht_songs[] = { { 0xBF, 0x4A }, { 0x1C6, 0x49 } };

#define PANLSHT_CODE { .name = "PanlSht (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.blocks = panlsht_blocks, .nblocks = (int)(sizeof panlsht_blocks / sizeof *panlsht_blocks), \
	.data = panlsht_data, .ndata = (int)(sizeof panlsht_data / sizeof *panlsht_data), \
	.calls = panlsht_calls, .ncalls = (int)(sizeof panlsht_calls / sizeof *panlsht_calls), \
	.words = panlsht_words, .nwords = (int)(sizeof panlsht_words / sizeof *panlsht_words), \
	.imms = panlsht_imms, .nimms = (int)(sizeof panlsht_imms / sizeof *panlsht_imms), \
	.types = panlsht_types, .ntypes = (int)(sizeof panlsht_types / sizeof *panlsht_types) }
const XChipUnit xchips_bn4[] = {
	{ XCHIP_SUPPORT, 0x26, 0xA6, 0x080E7A59, XC4_HOMES + 0x0, PANLSHT_CODE, panlsht_songs, (int)(sizeof panlsht_songs / sizeof *panlsht_songs), 0 },
};
const int xchips_bn4_count = (int)(sizeof xchips_bn4 / sizeof *xchips_bn4);

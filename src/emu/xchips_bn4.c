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
/* its sound 0xbf (080DCBEC): BN4's song copied into the grown song table's slot 0x1db, on BN4's player 9 */
/* its sound 0x1c6 (080DCB58): BN4's song copied into the grown song table's slot 0x1da, on BN4's player 17 */

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
	{ 0x080DCB72, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
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
static const XvSoundCall panlsht_sound_calls[] = {
	{ 0x080DCBEC, 0x1DB },   /* its sound 0xbf: BN4's song at the grown table's slot 0x1db */
	{ 0x080DCB58, 0x1DA },   /* its sound 0x1c6: BN4's song at the grown table's slot 0x1da */
};
static const XcSong panlsht_songs[] = { { 0xBF, 0x1DB }, { 0x1C6, 0x1DA } };

#define PANLSHT_CODE { .name = "PanlSht (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x0), \
	.blocks = panlsht_blocks, .nblocks = (int)(sizeof panlsht_blocks / sizeof *panlsht_blocks), \
	.data = panlsht_data, .ndata = (int)(sizeof panlsht_data / sizeof *panlsht_data), \
	.calls = panlsht_calls, .ncalls = (int)(sizeof panlsht_calls / sizeof *panlsht_calls), \
	.words = panlsht_words, .nwords = (int)(sizeof panlsht_words / sizeof *panlsht_words), \
	.imms = panlsht_imms, .nimms = (int)(sizeof panlsht_imms / sizeof *panlsht_imms), \
	.types = panlsht_types, .ntypes = (int)(sizeof panlsht_types / sizeof *panlsht_types), \
	.sound_calls = panlsht_sound_calls, .nsound_calls = (int)(sizeof panlsht_sound_calls / sizeof *panlsht_sound_calls) }
/* ---- ShotStar: BN4's support 0x19 (080E5D79) as BN6's 0x99; its chips 210 (570) ---- */
/* its fourth-kind 0x34 (080E5C90-080E5E38), which its routine (080E5D78) spawns: moved */
/* its third-kind 0x56 (080D85AA-080D87B0), spawned through 080D879A: no BN6 type proved its twin, moved */
/* its fourth-kind 0x35 (080E5E38-080E5EF4): no BN6 type of its body, moved with him */
/* its table 080D8580's kind 1: sprite 3:0x31 in BN4, 0x70 in BN6 (copied) */
/* its effect 0x6: BN6's 0xcf, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0x70: BN4's song at slot 0x3a, the families' slot for it (FAMILY_SLOTS) */
/* its sound 0x146 (080D86CE): BN4's song copied into the grown song table's slot 0x1dc, on BN4's player 6 */

static const XvBlock shotstar_blocks[] = {
	{ 0x080E5C90, 0x080E5E38, XC4_CODE + 0x400, 0xD23F0E2Bu },
	{ 0x080D85AA, 0x080D87B0, XC4_CODE + 0x5B2, 0x52072B37u },
	{ 0x080E5E38, 0x080E5EF4, XC4_CODE + 0x7C0, 0x0A0D54D6u },
	{ 0x080D8580, 0x080D85AA, XC4_CODE + 0x880, 0x6C5C9366u },
};
static const XvData shotstar_data[] = {
	{ 0x080E5C9E, 0x080E5CA0, XV_RAW },
	{ 0x080E5CA0, 0x080E5CA4, XV_POOL },
	{ 0x080E5CA4, 0x080E5CB0, XV_TABLE },
	{ 0x080E5CBE, 0x080E5CC0, XV_RAW },
	{ 0x080E5CC0, 0x080E5CC4, XV_POOL },
	{ 0x080E5CC4, 0x080E5CD4, XV_TABLE },
	{ 0x080E5CE2, 0x080E5CE4, XV_RAW },
	{ 0x080E5CE4, 0x080E5CE8, XV_POOL },
	{ 0x080E5CE8, 0x080E5CF8, XV_TABLE },
	{ 0x080E5E0A, 0x080E5E0C, XV_RAW },
	{ 0x080E5E0C, 0x080E5E10, XV_POOL },
	{ 0x080E5E10, 0x080E5E20, XV_RAW },
	{ 0x080E5E20, 0x080E5E24, XV_POOL },
	{ 0x080E5E24, 0x080E5E38, XV_RAW },
	{ 0x080D85B8, 0x080D85BC, XV_POOL },
	{ 0x080D85BC, 0x080D85C8, XV_TABLE },
	{ 0x080D866E, 0x080D8670, XV_RAW },
	{ 0x080D8670, 0x080D867C, XV_POOL },
	{ 0x080D869A, 0x080D86AC, XV_RAW },
	{ 0x080D86AC, 0x080D86B0, XV_POOL },
	{ 0x080D86B0, 0x080D86BC, XV_TABLE },
	{ 0x080D8764, 0x080D8768, XV_POOL },
	{ 0x080D87AE, 0x080D87B0, XV_RAW },
	{ 0x080E5E46, 0x080E5E48, XV_RAW },
	{ 0x080E5E48, 0x080E5E4C, XV_POOL },
	{ 0x080E5E4C, 0x080E5E58, XV_TABLE },
	{ 0x080E5E94, 0x080E5E98, XV_POOL },
	{ 0x080D8580, 0x080D85AA, XV_RAW },
};
static const XvPair shotstar_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009AEC, 0x0800C938 },   /* project:gutsman, DIFF (audited) */
	{ 0x08009BAC, 0x0800C9F8 },   /* project:project:chips, DIFF (audited) */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800AB8A, 0x0800E2C2 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F7A, 0x0801A146 },   /* project:fireman, DIFF (audited) */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0802608C, 0x080302A8 },   /* project:research, SAME */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair shotstar_words[] = {
	{ 0x08008BD9, 0x0800B917 },   /* project:project:chips, DIFF */
	{ 0x08008BF9, 0x0800B94D },   /* project:project:chips, SAME */
	{ 0x08008C2B, 0x0800B9B1 },   /* project:project:chips, DIFF */
	{ 0x08008EB9, 0x0800BC89 },   /* project:project:chips, SAME */
	{ 0x08008EEF, 0x0800BD35 },   /* project:project:chips, DIFF */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm shotstar_imms[] = {
	{ 0x080D8702, THUMB_MOVS, 4, 0, 0x6, 0xCF },   /* its effect: 0x6 in BN4, 0xcf in BN6 (moved) */
	{ 0x080D8708, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's slot 0x3a, a BN4 family's too */
	{ 0x080D8780, THUMB_MOVS, 0, 0, 0x56, 0xC9 },   /* its object's type 0x56: moved, the dispatch's 0xc9 */
	{ 0x080D8790, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E5D3C, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E5D7C, THUMB_MOVS, 0, 0, 0x34, 0x92 },   /* its object's type 0x34: moved, the dispatch's 0x92 */
	{ 0x080E5D8C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E5EEC, THUMB_MOVS, 0, 0, 0x35, 0x93 },   /* its object's type 0x35: moved, the dispatch's 0x93 */
};
static const XvType shotstar_types[] = {
	{ 3, 0xC9, XV_TYPE_MOVED, 0x56 },   /* BN4's third-kind 0x56, moved, the dispatch's 0xc9 */
	{ 4, 0x92, XV_TYPE_MOVED, 0x34 },   /* BN4's fourth-kind 0x34, moved, the dispatch's 0x92 */
	{ 4, 0x93, XV_TYPE_MOVED, 0x35 },   /* BN4's fourth-kind 0x35, moved, the dispatch's 0x93 */
};
static const XvSprite shotstar_sprites[] = {
	{ 3, 0x31, 3, 0x70, XC4_ART + 0x0, 0x300 },   /* BN4's 3:0x31 copied */
	{ 5, 0x0A, 5, 0xEF, XC4_ART + 0x300, 0xD00 },   /* BN4's 5:0xa copied */
};
static const XvId shotstar_effects[] = { { 0x06, 0xCF } };
static const XvByte shotstar_bytes[] = {
	{ 0x080D8586, 0x31, 0x70 },   /* its table's kind 1: list 3 0x31 in BN4, 0x70 in BN6 (copied) */
};
static const XvSoundCall shotstar_sound_calls[] = {
	{ 0x080D86CE, 0x1DC },   /* its sound 0x146: BN4's song at the grown table's slot 0x1dc */
};
static const XcSong shotstar_songs[] = { { 0x70, 0x3A }, { 0x146, 0x1DC } };

#define SHOTSTAR_CODE { .name = "ShotStar (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x400), \
	.blocks = shotstar_blocks, .nblocks = (int)(sizeof shotstar_blocks / sizeof *shotstar_blocks), \
	.data = shotstar_data, .ndata = (int)(sizeof shotstar_data / sizeof *shotstar_data), \
	.calls = shotstar_calls, .ncalls = (int)(sizeof shotstar_calls / sizeof *shotstar_calls), \
	.words = shotstar_words, .nwords = (int)(sizeof shotstar_words / sizeof *shotstar_words), \
	.imms = shotstar_imms, .nimms = (int)(sizeof shotstar_imms / sizeof *shotstar_imms), \
	.types = shotstar_types, .ntypes = (int)(sizeof shotstar_types / sizeof *shotstar_types), \
	.sound_calls = shotstar_sound_calls, .nsound_calls = (int)(sizeof shotstar_sound_calls / sizeof *shotstar_sound_calls), \
	.sprites = shotstar_sprites, .nsprites = (int)(sizeof shotstar_sprites / sizeof *shotstar_sprites), \
	.effects = shotstar_effects, .neffects = (int)(sizeof shotstar_effects / sizeof *shotstar_effects), \
	.bytes = shotstar_bytes, .nbytes = (int)(sizeof shotstar_bytes / sizeof *shotstar_bytes) }
/* ---- Repair: BN4's misc 0xb (0800C721) as BN6's 0x8b; its chips 115 (557) ---- */
/* its routine (0800C720), a routine of BN4's by itself: copied */
/* its effect 0x7: BN6's 0xce, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0x9a: BN4's song at slot 0x39, the families' slot for it (FAMILY_SLOTS) */

static const XvBlock repair_blocks[] = {
	{ 0x0800C720, 0x0800C75C, XC4_CODE + 0x8B0, 0x61E08481u },
};
static const XvData repair_data[] = {
};
static const XvPair repair_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair repair_words[] = {
};
static const XvImm repair_imms[] = {
	{ 0x0800C728, THUMB_ADDS, 0, 0, 0x68, 0xA0 },   /* the battle state's obstacles by side: BN4 +0x68, BN6 +0xA0 (BN4_OBSTACLE_ADD 0x0800B230 and BN6_OBSTACLE_ADD 0x0800F614, the same routine but for that adds; three words a side, 0xC apart in both) */
	{ 0x0800C746, THUMB_MOVS, 4, 0, 0x7, 0xCE },   /* its effect: 0x7 in BN4, 0xce in BN6 (moved) */
	{ 0x0800C74C, THUMB_MOVS, 0, 0, 0x9A, 0x39 },   /* its sound 0x9a: BN4's copied into BN6's slot 0x39, a BN4 family's too */
};
static const XvType repair_types[] = {
};
static const XvSprite repair_sprites[] = {
	{ 3, 0x12, 4, 0x9F, XC4_ART + 0x1000, 0x1300 },   /* BN4's 3:0x12 copied */
};
static const XvId repair_effects[] = { { 0x07, 0xCE } };
static const XcSong repair_songs[] = { { 0x9A, 0x39 } };

#define REPAIR_CODE { .name = "Repair (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x800), \
	.blocks = repair_blocks, .nblocks = (int)(sizeof repair_blocks / sizeof *repair_blocks), \
	.data = repair_data, .ndata = (int)(sizeof repair_data / sizeof *repair_data), \
	.calls = repair_calls, .ncalls = (int)(sizeof repair_calls / sizeof *repair_calls), \
	.words = repair_words, .nwords = (int)(sizeof repair_words / sizeof *repair_words), \
	.imms = repair_imms, .nimms = (int)(sizeof repair_imms / sizeof *repair_imms), \
	.types = repair_types, .ntypes = (int)(sizeof repair_types / sizeof *repair_types), \
	.sprites = repair_sprites, .nsprites = (int)(sizeof repair_sprites / sizeof *repair_sprites), \
	.effects = repair_effects, .neffects = (int)(sizeof repair_effects / sizeof *repair_effects) }
/* ---- Counter: BN4's misc 0x0 (080CFE03) as BN6's 0x80; its chips 60 (567), 61 (568), 62 (569) ---- */
/* its routine (080CFE02) lies in the third-kind 0x17's code (080CFD48-080CFE5C), which it spawns: moved */
/* its sprite 5:0x10 (080CFD74): BN6's 5:0xee, copied */
/* its sound 0xbe (080CFD96): BN4's song copied into the grown song table's slot 0x1dd, on BN4's player 9 */

static const XvBlock counter_blocks[] = {
	{ 0x080CFD48, 0x080CFE5C, XC4_CODE + 0x8F0, 0x52937DA2u },
};
static const XvData counter_data[] = {
	{ 0x080CFD5A, 0x080CFD5C, XV_RAW },
	{ 0x080CFD5C, 0x080CFD60, XV_POOL },
	{ 0x080CFD60, 0x080CFD6C, XV_TABLE },
	{ 0x080CFDB8, 0x080CFDBC, XV_POOL },
	{ 0x080CFDE0, 0x080CFDE4, XV_RAW },
};
static const XvPair counter_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
};
static const XvPair counter_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm counter_imms[] = {
	{ 0x080CFD70, THUMB_MOVS, 2, 0, 0x10, 0xEE },   /* its object's sprite: list 5 0x10 in BN4, 0xee in BN6 (copied) */
	{ 0x080CFD9E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CFDE8, THUMB_MOVS, 0, 0, 0x17, 0xC8 },   /* its object's type 0x17: moved, the dispatch's 0xc8 */
	{ 0x080CFDF4, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CFE26, THUMB_LSLS, 0, 0, 0x2, 0x4 },   /* a side's players in the battle state: BN4 a word a side's first from +0x44, BN6 0x10 bytes a side from +0xD0 */
	{ 0x080CFE28, THUMB_ADDS, 0, 0, 0x44, 0xD0 },   /* a side's players in the battle state: BN4 +0x44, BN6 +0xD0 */
	{ 0x080CFE38, THUMB_LDRB, 1, 1, 0xB, 0xD },   /* a player's collision data's counter byte: BN4 +0xB (BN4_COLL_COUNTER), BN6 +0xD (BN6_COLL_COUNTER) */
};
static const XvNumber counter_numbers[] = {
	{ 0x080CFDB8, 0x04050601, 0x04050601 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4), as 0x0405FF01: BN6's own attacks pass the same (five of its pools; GutsMan's states, MegaMan's 0x080F052C and 0x080F0606) */
};
static const XvType counter_types[] = {
	{ 3, 0xC8, XV_TYPE_MOVED, 0x17 },   /* BN4's third-kind 0x17, moved, the dispatch's 0xc8 */
};
static const XvSprite counter_sprites[] = {
	{ 5, 0x10, 5, 0xEE, XC4_ART + 0x2300, 0xB00 },   /* BN4's 5:0x10 copied */
};
static const XvSoundCall counter_sound_calls[] = {
	{ 0x080CFD96, 0x1DD },   /* its sound 0xbe: BN4's song at the grown table's slot 0x1dd */
};
static const XcSong counter_songs[] = { { 0xBE, 0x1DD } };

#define COUNTER_CODE { .name = "Counter (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0xC00), \
	.blocks = counter_blocks, .nblocks = (int)(sizeof counter_blocks / sizeof *counter_blocks), \
	.data = counter_data, .ndata = (int)(sizeof counter_data / sizeof *counter_data), \
	.calls = counter_calls, .ncalls = (int)(sizeof counter_calls / sizeof *counter_calls), \
	.words = counter_words, .nwords = (int)(sizeof counter_words / sizeof *counter_words), \
	.imms = counter_imms, .nimms = (int)(sizeof counter_imms / sizeof *counter_imms), \
	.numbers = counter_numbers, .nnumbers = (int)(sizeof counter_numbers / sizeof *counter_numbers), \
	.types = counter_types, .ntypes = (int)(sizeof counter_types / sizeof *counter_types), \
	.sound_calls = counter_sound_calls, .nsound_calls = (int)(sizeof counter_sound_calls / sizeof *counter_sound_calls), \
	.sprites = counter_sprites, .nsprites = (int)(sizeof counter_sprites / sizeof *counter_sprites) }
const XChipUnit xchips_bn4[] = {
	{ XCHIP_SUPPORT, 0x26, 0xA6, 0x080E7A59, XC4_HOMES + 0x0, PANLSHT_CODE, panlsht_songs, (int)(sizeof panlsht_songs / sizeof *panlsht_songs), 0 },
	{ XCHIP_SUPPORT, 0x19, 0x99, 0x080E5D79, XC4_HOMES + 0x400, SHOTSTAR_CODE, shotstar_songs, (int)(sizeof shotstar_songs / sizeof *shotstar_songs), 0 },
	{ XCHIP_MISC, 0x0B, 0x8B, 0x0800C721, XC4_HOMES + 0x800, REPAIR_CODE, repair_songs, (int)(sizeof repair_songs / sizeof *repair_songs), 0 },
	{ XCHIP_MISC, 0x00, 0x80, 0x080CFE03, XC4_HOMES + 0xC00, COUNTER_CODE, counter_songs, (int)(sizeof counter_songs / sizeof *counter_songs), 0 },
};
const int xchips_bn4_count = (int)(sizeof xchips_bn4 / sizeof *xchips_bn4);

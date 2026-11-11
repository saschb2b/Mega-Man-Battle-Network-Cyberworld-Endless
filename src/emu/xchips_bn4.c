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
/* its sprite 3:0x38 (080DCAD0): BN6's 3:0x70, copied */
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
	{ 0x080DCACE, THUMB_MOVS, 2, 0, 0x38, 0x70 },   /* its object's sprite: list 3 0x38 in BN4, 0x70 in BN6 (copied) */
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
static const XvSprite panlsht_sprites[] = {
	{ 3, 0x38, 3, 0x70, XC4_ART + 0x0, 0x400 },   /* BN4's 3:0x38 copied */
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
	.sound_calls = panlsht_sound_calls, .nsound_calls = (int)(sizeof panlsht_sound_calls / sizeof *panlsht_sound_calls), \
	.sprites = panlsht_sprites, .nsprites = (int)(sizeof panlsht_sprites / sizeof *panlsht_sprites) }
/* ---- ShotStar: BN4's support 0x19 (080E5D79) as BN6's 0x99; its chips 210 (570) ---- */
/* its fourth-kind 0x34 (080E5C90-080E5E38), which its routine (080E5D78) spawns: moved */
/* its third-kind 0x56 (080D85AA-080D87B0), spawned through 080D879A: no BN6 type proved its twin, moved */
/* its fourth-kind 0x35 (080E5E38-080E5EF4): no BN6 type of its body, moved with him */
/* its table 080D8580's kind 1: sprite 3:0x31 in BN4, 0x71 in BN6 (copied) */
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
	{ 3, 0x31, 3, 0x71, XC4_ART + 0x400, 0x300 },   /* BN4's 3:0x31 copied */
	{ 5, 0x0A, 5, 0xEF, XC4_ART + 0x700, 0xD00 },   /* BN4's 5:0xa copied */
};
static const XvId shotstar_effects[] = { { 0x06, 0xCF } };
static const XvByte shotstar_bytes[] = {
	{ 0x080D8586, 0x31, 0x71 },   /* its table's kind 1: list 3 0x31 in BN4, 0x71 in BN6 (copied) */
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
	{ 3, 0x12, 4, 0x9F, XC4_ART + 0x1400, 0x1300 },   /* BN4's 3:0x12 copied */
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
	{ 5, 0x10, 5, 0xEE, XC4_ART + 0x2700, 0xB00 },   /* BN4's 5:0x10 copied */
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
/* ---- WhitWeb: BN4's misc 0x8 (080D4F21) as BN6's 0x88; its chips 72 (574), 73 (575), 74 (576) ---- */
/* its routine (080D4F20) lies in the third-kind 0x3d's code (080D4D30-080D4F40), which it spawns: moved */
/* its sprite 4:0x1c (080D4D74): BN6's 4:0x9e, copied */
/* its effect 0xf: BN6's 0xcd, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0x70: BN4's song at slot 0x3a, the families' slot for it (FAMILY_SLOTS) */
/* its sound 0xf7 (080D4DF0): BN4's song copied into the grown song table's slot 0x1de, on BN4's player 22 */

static const XvBlock whitweb_blocks[] = {
	{ 0x080D4D30, 0x080D4F40, XC4_CODE + 0xA10, 0x86F46E82u },
};
static const XvData whitweb_data[] = {
	{ 0x080D4D42, 0x080D4D44, XV_RAW },
	{ 0x080D4D44, 0x080D4D48, XV_POOL },
	{ 0x080D4D48, 0x080D4D54, XV_TABLE },
	{ 0x080D4E5E, 0x080D4E60, XV_RAW },
	{ 0x080D4E60, 0x080D4E68, XV_POOL },
	{ 0x080D4E68, 0x080D4E6C, XV_TABLE },
	{ 0x080D4E6C, 0x080D4E74, XV_RAW },
	{ 0x080D4F0C, 0x080D4F10, XV_POOL },
	{ 0x080D4F10, 0x080D4F20, XV_RAW },
	{ 0x080D4F3E, 0x080D4F40, XV_RAW },
};
static const XvPair whitweb_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x0800A10E, 0x0800D012 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x08010560, 0x080D0B94 },   /* project:spikey, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08013212, 0x0801A258 },   /* project:gutsman, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair whitweb_words[] = {
};
static const XvImm whitweb_imms[] = {
	{ 0x080D4D70, THUMB_MOVS, 2, 0, 0x1C, 0x9E },   /* its object's sprite: list 4 0x1c in BN4, 0x9e in BN6 (copied) */
	{ 0x080D4E18, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D4E48, THUMB_MOVS, 4, 0, 0xF, 0xCD },   /* its effect: 0xf in BN4, 0xcd in BN6 (moved) */
	{ 0x080D4E4E, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's slot 0x3a, a BN4 family's too */
	{ 0x080D4EA0, THUMB_MOVS, 0, 0, 0x3D, 0xC6 },   /* its object's type 0x3d: moved, the dispatch's 0xc6 */
	{ 0x080D4EB0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType whitweb_types[] = {
	{ 3, 0xC6, XV_TYPE_MOVED, 0x3D },   /* BN4's third-kind 0x3d, moved, the dispatch's 0xc6 */
};
static const XvSprite whitweb_sprites[] = {
	{ 4, 0x1C, 4, 0x9E, XC4_ART + 0x3200, 0x600 },   /* BN4's 4:0x1c copied */
	{ 5, 0x01, 5, 0xED, XC4_ART + 0x3800, 0x1800 },   /* BN4's 5:0x1 copied */
};
static const XvId whitweb_effects[] = { { 0x0F, 0xCD } };
static const XvSoundCall whitweb_sound_calls[] = {
	{ 0x080D4DF0, 0x1DE },   /* its sound 0xf7: BN4's song at the grown table's slot 0x1de */
};
static const XcSong whitweb_songs[] = { { 0x70, 0x3A }, { 0xF7, 0x1DE } };

#define WHITWEB_CODE { .name = "WhitWeb (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x1000), \
	.blocks = whitweb_blocks, .nblocks = (int)(sizeof whitweb_blocks / sizeof *whitweb_blocks), \
	.data = whitweb_data, .ndata = (int)(sizeof whitweb_data / sizeof *whitweb_data), \
	.calls = whitweb_calls, .ncalls = (int)(sizeof whitweb_calls / sizeof *whitweb_calls), \
	.words = whitweb_words, .nwords = (int)(sizeof whitweb_words / sizeof *whitweb_words), \
	.imms = whitweb_imms, .nimms = (int)(sizeof whitweb_imms / sizeof *whitweb_imms), \
	.types = whitweb_types, .ntypes = (int)(sizeof whitweb_types / sizeof *whitweb_types), \
	.sound_calls = whitweb_sound_calls, .nsound_calls = (int)(sizeof whitweb_sound_calls / sizeof *whitweb_sound_calls), \
	.sprites = whitweb_sprites, .nsprites = (int)(sizeof whitweb_sprites / sizeof *whitweb_sprites), \
	.effects = whitweb_effects, .neffects = (int)(sizeof whitweb_effects / sizeof *whitweb_effects) }
/* ---- Boomer: BN4's misc 0x2 (080D344F) as BN6's 0x82; its chips 75 (584), 76 (585), 77 (586) ---- */
/* its routine (080D344E) lies in the third-kind 0x32's code (080D3234-080D3468), which it spawns: moved */
/* its sprite 4:0x14 (080D3268): BN6's 4:0x9d, copied */
/* its sound 0xab (080D32F4): BN4's song copied into the grown song table's slot 0x1df, on BN4's player 28 */

static const XvBlock boomer_blocks[] = {
	{ 0x080D3234, 0x080D3468, XC4_CODE + 0xC20, 0x5EBC617Fu },
	{ 0x080D31EC, 0x080D3234, XC4_CODE + 0xE60, 0x9FE81DFDu },
};
static const XvData boomer_data[] = {
	{ 0x080D3246, 0x080D3248, XV_RAW },
	{ 0x080D3248, 0x080D324C, XV_POOL },
	{ 0x080D324C, 0x080D3258, XV_TABLE },
	{ 0x080D32FE, 0x080D3300, XV_RAW },
	{ 0x080D3300, 0x080D3304, XV_POOL },
	{ 0x080D3364, 0x080D3368, XV_POOL },
	{ 0x080D3368, 0x080D336C, XV_TABLE },
	{ 0x080D3406, 0x080D3408, XV_RAW },
	{ 0x080D3408, 0x080D340C, XV_POOL },
	{ 0x080D340C, 0x080D3418, XV_RAW },
	{ 0x080D3418, 0x080D341C, XV_POOL },
	{ 0x080D341C, 0x080D3428, XV_RAW },
	{ 0x080D3428, 0x080D3430, XV_POOL },
	{ 0x080D3464, 0x080D3468, XV_POOL },
	{ 0x080D31EC, 0x080D3234, XV_RAW },
};
static const XvPair boomer_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800AB8A, 0x0800E2C2 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x08010560, 0x080D0B94 },   /* project:spikey, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
};
static const XvPair boomer_words[] = {
};
static const XvImm boomer_imms[] = {
	{ 0x080D3264, THUMB_MOVS, 2, 0, 0x14, 0x9D },   /* its object's sprite: list 4 0x14 in BN4, 0x9d in BN6 (copied) */
	{ 0x080D3318, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D3434, THUMB_MOVS, 0, 0, 0x32, 0xC5 },   /* its object's type 0x32: moved, the dispatch's 0xc5 */
	{ 0x080D3444, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType boomer_types[] = {
	{ 3, 0xC5, XV_TYPE_MOVED, 0x32 },   /* BN4's third-kind 0x32, moved, the dispatch's 0xc5 */
};
static const XvSprite boomer_sprites[] = {
	{ 4, 0x14, 4, 0x9D, XC4_ART + 0x5000, 0xF00 },   /* BN4's 4:0x14 copied */
};
static const XvSoundCall boomer_sound_calls[] = {
	{ 0x080D32F4, 0x1DF },   /* its sound 0xab: BN4's song at the grown table's slot 0x1df */
};
static const XcSong boomer_songs[] = { { 0xAB, 0x1DF } };

#define BOOMER_CODE { .name = "Boomer (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x1C00), \
	.blocks = boomer_blocks, .nblocks = (int)(sizeof boomer_blocks / sizeof *boomer_blocks), \
	.data = boomer_data, .ndata = (int)(sizeof boomer_data / sizeof *boomer_data), \
	.calls = boomer_calls, .ncalls = (int)(sizeof boomer_calls / sizeof *boomer_calls), \
	.words = boomer_words, .nwords = (int)(sizeof boomer_words / sizeof *boomer_words), \
	.imms = boomer_imms, .nimms = (int)(sizeof boomer_imms / sizeof *boomer_imms), \
	.types = boomer_types, .ntypes = (int)(sizeof boomer_types / sizeof *boomer_types), \
	.sound_calls = boomer_sound_calls, .nsound_calls = (int)(sizeof boomer_sound_calls / sizeof *boomer_sound_calls), \
	.sprites = boomer_sprites, .nsprites = (int)(sizeof boomer_sprites / sizeof *boomer_sprites) }
/* ---- MokoRus: BN4's misc 0x1 (080D252B) as BN6's 0x81; its chips 130 (577), 131 (578), 132 (579) ---- */
/* its routine (080D252A) lies in the third-kind 0x2c's code (080D235C-080D2568), which it spawns: moved */
/* its sprite 1:0x11 (080D2388): BN6's 1:0x7f, copied */
/* its effect 0x1: BN6's 0xcc, moved (no effect of BN6's draws its sprite's bytes) */
/* its effect 0x15: BN6's 0xcb, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0xc7 (080D245E): BN4's song copied into the grown song table's slot 0x1e0, on BN4's player 9 */

static const XvBlock mokorus_blocks[] = {
	{ 0x080D235C, 0x080D2568, XC4_CODE + 0xEB0, 0xAAEAC10Du },
};
static const XvData mokorus_data[] = {
	{ 0x080D236E, 0x080D2370, XV_RAW },
	{ 0x080D2370, 0x080D2374, XV_POOL },
	{ 0x080D2374, 0x080D2380, XV_TABLE },
	{ 0x080D243E, 0x080D2440, XV_RAW },
	{ 0x080D2440, 0x080D2444, XV_POOL },
	{ 0x080D2444, 0x080D244C, XV_TABLE },
	{ 0x080D2500, 0x080D2504, XV_POOL },
	{ 0x080D2504, 0x080D250C, XV_RAW },
	{ 0x080D2564, 0x080D2568, XV_POOL },
};
static const XvPair mokorus_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08000AAA, 0x08000C72 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
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
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x08014620, 0x0801BDDE },   /* search, SAME */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair mokorus_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm mokorus_imms[] = {
	{ 0x080D2384, THUMB_MOVS, 2, 0, 0x11, 0x7F },   /* its object's sprite: list 1 0x11 in BN4, 0x7f in BN6 (copied) */
	{ 0x080D23E0, THUMB_MOVS, 3, 0, 0x21, 0x61 },   /* its hit's bits (the collision setup's r3): struck and pushed is bit 0x01 with a push bit in BN4 (BN4_STRUCK_PUSHED), bit 0x40 with one in BN6 (BN6_STRUCK_PUSHED) */
	{ 0x080D2412, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D242E, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080D24B0, THUMB_MOVS, 4, 0, 0x1, 0xCC },   /* its effect: 0x1 in BN4, 0xcc in BN6 (moved) */
	{ 0x080D24F0, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080D2510, THUMB_MOVS, 0, 0, 0x2C, 0xC4 },   /* its object's type 0x2c: moved, the dispatch's 0xc4 */
	{ 0x080D2520, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType mokorus_types[] = {
	{ 3, 0xC4, XV_TYPE_MOVED, 0x2C },   /* BN4's third-kind 0x2c, moved, the dispatch's 0xc4 */
};
static const XvSprite mokorus_sprites[] = {
	{ 1, 0x11, 1, 0x7F, XC4_ART + 0x5F00, 0x3900 },   /* BN4's 1:0x11 copied */
	{ 5, 0x02, 5, 0xEC, XC4_ART + 0x9800, 0x1300 },   /* BN4's 5:0x2 copied */
	{ 5, 0x0D, 5, 0xEB, XC4_ART + 0xAB00, 0x2300 },   /* BN4's 5:0xd copied */
};
static const XvId mokorus_effects[] = { { 0x01, 0xCC }, { 0x15, 0xCB } };
static const XvSoundCall mokorus_sound_calls[] = {
	{ 0x080D245E, 0x1E0 },   /* its sound 0xc7: BN4's song at the grown table's slot 0x1e0 */
};
static const XcSong mokorus_songs[] = { { 0xC7, 0x1E0 } };

#define MOKORUS_CODE { .name = "MokoRus (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x2000), \
	.blocks = mokorus_blocks, .nblocks = (int)(sizeof mokorus_blocks / sizeof *mokorus_blocks), \
	.data = mokorus_data, .ndata = (int)(sizeof mokorus_data / sizeof *mokorus_data), \
	.calls = mokorus_calls, .ncalls = (int)(sizeof mokorus_calls / sizeof *mokorus_calls), \
	.words = mokorus_words, .nwords = (int)(sizeof mokorus_words / sizeof *mokorus_words), \
	.imms = mokorus_imms, .nimms = (int)(sizeof mokorus_imms / sizeof *mokorus_imms), \
	.types = mokorus_types, .ntypes = (int)(sizeof mokorus_types / sizeof *mokorus_types), \
	.sound_calls = mokorus_sound_calls, .nsound_calls = (int)(sizeof mokorus_sound_calls / sizeof *mokorus_sound_calls), \
	.sprites = mokorus_sprites, .nsprites = (int)(sizeof mokorus_sprites / sizeof *mokorus_sprites), \
	.effects = mokorus_effects, .neffects = (int)(sizeof mokorus_effects / sizeof *mokorus_effects) }
/* ---- SidBmbo: BN4's misc 0x5 (080D5DDB) as BN6's 0x85; its chips 78 (571), 79 (572), 80 (573) ---- */
/* BN4's misc run waits 0x28 frames after its routine (080EC9AC): its entry the installer's wrapper */
/* its routine (080D5DDA) lies in the third-kind 0x44's code (080D5C50-080D5DF4), which it spawns: moved */
/* its sprite 1:0xc (080D5C7C): BN6's 1:0x7e, copied */
/* its effect 0x15: 0xcb, as MokoRus moved it */
/* its effect 0x15's sprite 5:0xd is listed by MokoRus (5:0xeb): installed with it */
/* its effect 0x15: BN6's 0xcb, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0x8e (080D5D36): BN4's song copied into the grown song table's slot 0x1e1, on BN4's player 9 */

static const XvBlock sidbmbo_blocks[] = {
	{ 0x080D5C50, 0x080D5DF4, XC4_CODE + 0x10C0, 0x957B533Eu },
};
static const XvData sidbmbo_data[] = {
	{ 0x080D5C62, 0x080D5C64, XV_RAW },
	{ 0x080D5C64, 0x080D5C68, XV_POOL },
	{ 0x080D5C68, 0x080D5C74, XV_TABLE },
	{ 0x080D5CEE, 0x080D5CF0, XV_RAW },
	{ 0x080D5CF0, 0x080D5CF4, XV_POOL },
	{ 0x080D5CF4, 0x080D5CFC, XV_TABLE },
	{ 0x080D5D1E, 0x080D5D20, XV_RAW },
	{ 0x080D5D20, 0x080D5D24, XV_POOL },
	{ 0x080D5D24, 0x080D5D2C, XV_TABLE },
	{ 0x080D5D82, 0x080D5D84, XV_RAW },
	{ 0x080D5D84, 0x080D5D88, XV_POOL },
	{ 0x080D5DF2, 0x080D5DF4, XV_RAW },
};
static const XvPair sidbmbo_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair sidbmbo_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm sidbmbo_imms[] = {
	{ 0x080D5C78, THUMB_MOVS, 2, 0, 0xC, 0x7E },   /* its object's sprite: list 1 0xc in BN4, 0x7e in BN6 (copied) */
	{ 0x080D5CE2, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080D5D60, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D5DAA, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080D5DBE, THUMB_MOVS, 0, 0, 0x44, 0xC3 },   /* its object's type 0x44: moved, the dispatch's 0xc3 */
	{ 0x080D5DD0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvNumber sidbmbo_numbers[] = {
	{ 0x080D5D84, 0x04050401, 0x04050401 },   /* a collision region's spawn word (Kilby's: its ldr r4 at 0x080BC946, then bl 0x080CD7E2, object_spawnCollisionRegion, whose twin BN6's 0x080C6BDA takes r4 the same), the form of 0x04050601 and 0x0405FF01; BN6's ROM holds the word too (0x080E4A4C) */
};
static const XvType sidbmbo_types[] = {
	{ 3, 0xC3, XV_TYPE_MOVED, 0x44 },   /* BN4's third-kind 0x44, moved, the dispatch's 0xc3 */
};
static const XvSprite sidbmbo_sprites[] = {
	{ 1, 0x0C, 1, 0x7E, XC4_ART + 0xCE00, 0x3300 },   /* BN4's 1:0xc copied */
};
static const XvSoundCall sidbmbo_sound_calls[] = {
	{ 0x080D5D36, 0x1E1 },   /* its sound 0x8e: BN4's song at the grown table's slot 0x1e1 */
};
static const XcSong sidbmbo_songs[] = { { 0x8E, 0x1E1 } };

#define SIDBMBO_CODE { .name = "SidBmbo (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x2400), \
	.blocks = sidbmbo_blocks, .nblocks = (int)(sizeof sidbmbo_blocks / sizeof *sidbmbo_blocks), \
	.data = sidbmbo_data, .ndata = (int)(sizeof sidbmbo_data / sizeof *sidbmbo_data), \
	.calls = sidbmbo_calls, .ncalls = (int)(sizeof sidbmbo_calls / sizeof *sidbmbo_calls), \
	.words = sidbmbo_words, .nwords = (int)(sizeof sidbmbo_words / sizeof *sidbmbo_words), \
	.imms = sidbmbo_imms, .nimms = (int)(sizeof sidbmbo_imms / sizeof *sidbmbo_imms), \
	.numbers = sidbmbo_numbers, .nnumbers = (int)(sizeof sidbmbo_numbers / sizeof *sidbmbo_numbers), \
	.types = sidbmbo_types, .ntypes = (int)(sizeof sidbmbo_types / sizeof *sidbmbo_types), \
	.sound_calls = sidbmbo_sound_calls, .nsound_calls = (int)(sizeof sidbmbo_sound_calls / sizeof *sidbmbo_sound_calls), \
	.sprites = sidbmbo_sprites, .nsprites = (int)(sizeof sidbmbo_sprites / sizeof *sidbmbo_sprites) }
const XChipUnit xchips_bn4[] = {
	{ XCHIP_SUPPORT, 0x26, 0xA6, 0x080E7A59, XC4_HOMES + 0x0, PANLSHT_CODE, panlsht_songs, (int)(sizeof panlsht_songs / sizeof *panlsht_songs), 0, 0 },
	{ XCHIP_SUPPORT, 0x19, 0x99, 0x080E5D79, XC4_HOMES + 0x400, SHOTSTAR_CODE, shotstar_songs, (int)(sizeof shotstar_songs / sizeof *shotstar_songs), 0, 0 },
	{ XCHIP_MISC, 0x0B, 0x8B, 0x0800C721, XC4_HOMES + 0x800, REPAIR_CODE, repair_songs, (int)(sizeof repair_songs / sizeof *repair_songs), 0, 0 },
	{ XCHIP_MISC, 0x00, 0x80, 0x080CFE03, XC4_HOMES + 0xC00, COUNTER_CODE, counter_songs, (int)(sizeof counter_songs / sizeof *counter_songs), 0, 0 },
	{ XCHIP_MISC, 0x08, 0x88, 0x080D4F21, XC4_HOMES + 0x1000, WHITWEB_CODE, whitweb_songs, (int)(sizeof whitweb_songs / sizeof *whitweb_songs), 0, 0 },
	{ XCHIP_MISC, 0x09, 0x89, 0x080D4F2B, XC4_HOMES + 0x1400, { .name = "WhitWeb (BN4's), its subfamily 0x9", .game = XROM_BN4_RED_SUN_US }, NULL, 0, 0, 5 },
	{ XCHIP_MISC, 0x0A, 0x8A, 0x080D4F35, XC4_HOMES + 0x1800, { .name = "WhitWeb (BN4's), its subfamily 0xa", .game = XROM_BN4_RED_SUN_US }, NULL, 0, 0, 5 },
	{ XCHIP_MISC, 0x02, 0x82, 0x080D344F, XC4_HOMES + 0x1C00, BOOMER_CODE, boomer_songs, (int)(sizeof boomer_songs / sizeof *boomer_songs), 0, 0 },
	{ XCHIP_MISC, 0x01, 0x81, 0x080D252B, XC4_HOMES + 0x2000, MOKORUS_CODE, mokorus_songs, (int)(sizeof mokorus_songs / sizeof *mokorus_songs), 0, 0 },
	{ XCHIP_MISC, 0x05, 0x85, 0x080D5DDB, XC4_HOMES + 0x2400, SIDBMBO_CODE, sidbmbo_songs, (int)(sizeof sidbmbo_songs / sizeof *sidbmbo_songs), 0, 0, 0x28 },
};
const int xchips_bn4_count = (int)(sizeof xchips_bn4 / sizeof *xchips_bn4);

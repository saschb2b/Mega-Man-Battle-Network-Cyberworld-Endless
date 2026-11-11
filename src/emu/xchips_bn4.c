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
/* ---- Spreads: BN4's attack 0x1e (080EC761) as BN6's 0xd0; its chips 19 (588), 20 (589), 21 (590), 22 (591), 23 (592), 24 (593) ---- */
/* MegaMan's state 0x1e (080EC760-080EC842), the family of its chips' records: moved */
/* its table 080CD524's kind 10: sprite 3:0x19 in BN4, 0x72 in BN6 (copied) */
/* its table 080CD524's kind 6: sprite 3:0xd in BN4, 0x73 in BN6 (copied) */
/* its table 080CD524's kind 14: sprite 3:0x1c in BN4, 0x74 in BN6 (copied) */
/* its table 080CD524's kind 50: sprite 3:0x19 in BN4, 0x72 in BN6 (copied) */
/* its table 080CD524's kind 12: sprite 3:0x1e in BN4, 0x75 in BN6 (copied) */
/* its effect 0x25 (its table 080CF3DC's): BN6's 0xca, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0x9f (its table 080CF3DC's): BN4's song at slot 0x3d, the families' slot for it (FAMILY_SLOTS) */
/* its effect 0x29 (its table 080CF3DC's): BN6's 0xc9, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0xbc (its table 080CF3DC's): BN4's song at slot 0x3f, the families' slot for it (FAMILY_SLOTS) */
/* its sound 0x8d (080EC7DE): BN4's song copied into the grown song table's slot 0x200, on BN4's player 17 */
/* its chip 18 has no row of the registry (the run's by name, or none) */
/* its chip 192 is a dark chip of BN4's (row 594): a battle it is used in costs 20 max HP */
/* its chip 192 (id 594) sits out: DrkSpred, a DarkChip of BN4's (the dark chips' session's: its kind 0xF of the shot's table and its price) */
/* its chip 321 (id 606) sits out: a Program Advance, unformed until BN4's recipes are listed */
/* its chip 322 (id 607) sits out: a Program Advance, unformed until BN4's recipes are listed */

static const XvBlock spreads_blocks[] = {
	{ 0x080EC760, 0x080EC842, XC4_CODE + 0x1270, 0x6A2FF2DDu },
	{ 0x080CD5F8, 0x080CD728, XC4_CODE + 0x1360, 0x0069BF7Du },
	{ 0x080CF420, 0x080CF5B8, XC4_CODE + 0x1490, 0x701B9ED2u },
	{ 0x080CD524, 0x080CD5F8, XC4_CODE + 0x1630, 0x40F51A6Du },
	{ 0x080CF3DC, 0x080CF420, XC4_CODE + 0x1710, 0x148921CFu },
};
static const XvData spreads_data[] = {
	{ 0x080EC772, 0x080EC774, XV_RAW },
	{ 0x080EC774, 0x080EC778, XV_POOL },
	{ 0x080EC778, 0x080EC780, XV_TABLE },
	{ 0x080EC808, 0x080EC814, XV_POOL },
	{ 0x080EC814, 0x080EC820, XV_RAW },
	{ 0x080CD606, 0x080CD608, XV_RAW },
	{ 0x080CD608, 0x080CD60C, XV_POOL },
	{ 0x080CD60C, 0x080CD618, XV_TABLE },
	{ 0x080CD708, 0x080CD720, XV_RAW },
	{ 0x080CD720, 0x080CD728, XV_POOL },
	{ 0x080CF42E, 0x080CF430, XV_RAW },
	{ 0x080CF430, 0x080CF434, XV_POOL },
	{ 0x080CF434, 0x080CF440, XV_TABLE },
	{ 0x080CF4C2, 0x080CF4D4, XV_RAW },
	{ 0x080CF4D4, 0x080CF4D8, XV_POOL },
	{ 0x080CF4D8, 0x080CF4E0, XV_TABLE },
	{ 0x080CF51A, 0x080CF51C, XV_RAW },
	{ 0x080CF51C, 0x080CF520, XV_POOL },
	{ 0x080CF520, 0x080CF528, XV_TABLE },
	{ 0x080CF5B4, 0x080CF5B8, XV_POOL },
	{ 0x080CD524, 0x080CD5F8, XV_RAW },
	{ 0x080CF3DC, 0x080CF420, XV_RAW },
};
static const XvPair spreads_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002CEA, 0x08002C7E },   /* search, SAME */
	{ 0x08002D4C, 0x08002CE0 },   /* search, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E20, 0x08002DB4 },   /* project:fireman, SAME */
	{ 0x08002E34, 0x08002DC8 },   /* search, SAME */
	{ 0x08002F22, 0x08002ED0 },   /* search, SAME */
	{ 0x08002F2E, 0x08002EDC },   /* search, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FD0, 0x08002F7E },   /* search, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800D7CA, 0x08013774 },   /* project:project:chips, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x0801288C, 0x08019F8C },   /* project:project:chips, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012ED0, 0x0801A082 },   /* project:melody, DIFF (audited) */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0801457E, 0x0801BD3C },   /* search, SAME */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair spreads_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm spreads_imms[] = {
	{ 0x080CD6E4, THUMB_MOVS, 0, 0, 0x2, 0xC2 },   /* its object's type 0x2: moved, the dispatch's 0xc2 */
	{ 0x080CF492, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CF546, THUMB_LDRB, 0, 7, 0x8, 0xA },   /* collision data +0x8 in BN4, +0xa in BN6 */
	{ 0x080CF548, THUMB_LDRB, 1, 7, 0x9, 0xB },   /* collision data +0x9 in BN4, +0xb in BN6 */
	{ 0x080CF58E, THUMB_LDRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CF59A, THUMB_MOVS, 0, 0, 0x12, 0xBE },   /* its object's type 0x12: moved, the dispatch's 0xbe */
	{ 0x080CF5A6, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EC764, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080EC788, THUMB_MOVS, 0, 0, 0xE, 0xB },   /* MegaMan's animation as he fires a spread chip: BN4's 0x0E, BN6's 0x0B of its own sprite of him (BN6's twin family 0x25 sets it at 0x080EDF14, through 0x0800F2B6, which stores it and 0xFF as BN4's state does) */
	{ 0x080EC796, THUMB_MOVS, 1, 0, 0x23, 0x29 },   /* the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 (BN6_NAVI_SIDE_NAVI) */
};
static const XvType spreads_types[] = {
	{ 3, 0xC2, XV_TYPE_MOVED, 0x02 },   /* BN4's third-kind 0x2, moved, the dispatch's 0xc2 */
	{ 3, 0xBE, XV_TYPE_MOVED, 0x12 },   /* BN4's third-kind 0x12, moved, the dispatch's 0xbe */
};
static const XvSprite spreads_sprites[] = {
	{ 3, 0x19, 3, 0x72, XC4_ART + 0x10100, 0x3600 },   /* BN4's 3:0x19 copied */
	{ 3, 0x0D, 3, 0x73, XC4_ART + 0x13700, 0x2400 },   /* BN4's 3:0xd copied */
	{ 3, 0x1C, 3, 0x74, XC4_ART + 0x15B00, 0x2700 },   /* BN4's 3:0x1c copied */
	{ 3, 0x1E, 3, 0x75, XC4_ART + 0x18200, 0x1900 },   /* BN4's 3:0x1e copied */
	{ 3, 0x1A, 4, 0x9C, XC4_ART + 0x19B00, 0x1100 },   /* BN4's 3:0x1a copied */
	{ 5, 0x11, 5, 0xEA, XC4_ART + 0x1AC00, 0x2F00 },   /* BN4's 5:0x11 copied */
};
static const XvId spreads_effects[] = { { 0x25, 0xCA }, { 0x29, 0xC9 } };
static const XvByte spreads_bytes[] = {
	{ 0x080CD54D, 0x19, 0x72 },   /* its table's kind 10: list 3 0x19 in BN4, 0x72 in BN6 (copied) */
	{ 0x080CD53D, 0x0D, 0x73 },   /* its table's kind 6: list 3 0xd in BN4, 0x73 in BN6 (copied) */
	{ 0x080CD55D, 0x1C, 0x74 },   /* its table's kind 14: list 3 0x1c in BN4, 0x74 in BN6 (copied) */
	{ 0x080CD5ED, 0x19, 0x72 },   /* its table's kind 50: list 3 0x19 in BN4, 0x72 in BN6 (copied) */
	{ 0x080CD555, 0x1E, 0x75 },   /* its table's kind 12: list 3 0x1e in BN4, 0x75 in BN6 (copied) */
	{ 0x080CF3EE, 0x25, 0xCA },   /* its table's kind 0x4's effect: 0x25 in BN4, 0xca in BN6 (moved) */
	{ 0x080CF3EF, 0x9F, 0x3D },   /* its table's kind 0x4's sound: BN4's 0x9f at BN6's slot 0x3d */
	{ 0x080CF3F2, 0x25, 0xCA },   /* its table's kind 0x5's effect: 0x25 in BN4, 0xca in BN6 (moved) */
	{ 0x080CF3F3, 0x9F, 0x3D },   /* its table's kind 0x5's sound: BN4's 0x9f at BN6's slot 0x3d */
	{ 0x080CF3F6, 0x25, 0xCA },   /* its table's kind 0x6's effect: 0x25 in BN4, 0xca in BN6 (moved) */
	{ 0x080CF3F7, 0x9F, 0x3D },   /* its table's kind 0x6's sound: BN4's 0x9f at BN6's slot 0x3d */
	{ 0x080CF3FA, 0x25, 0xCA },   /* its table's kind 0x7's effect: 0x25 in BN4, 0xca in BN6 (moved) */
	{ 0x080CF3FB, 0x9F, 0x3D },   /* its table's kind 0x7's sound: BN4's 0x9f at BN6's slot 0x3d */
	{ 0x080CF3FE, 0x29, 0xC9 },   /* its table's kind 0x8's effect: 0x29 in BN4, 0xc9 in BN6 (moved) */
	{ 0x080CF3FF, 0xBC, 0x3F },   /* its table's kind 0x8's sound: BN4's 0xbc at BN6's slot 0x3f */
	{ 0x080CF402, 0x29, 0xC9 },   /* its table's kind 0x9's effect: 0x29 in BN4, 0xc9 in BN6 (moved) */
	{ 0x080CF403, 0xBC, 0x3F },   /* its table's kind 0x9's sound: BN4's 0xbc at BN6's slot 0x3f */
	{ 0x080CF406, 0x29, 0xC9 },   /* its table's kind 0xa's effect: 0x29 in BN4, 0xc9 in BN6 (moved) */
	{ 0x080CF407, 0xBC, 0x3F },   /* its table's kind 0xa's sound: BN4's 0xbc at BN6's slot 0x3f */
	{ 0x080CF40A, 0x29, 0xC9 },   /* its table's kind 0xb's effect: 0x29 in BN4, 0xc9 in BN6 (moved) */
	{ 0x080CF40B, 0xBC, 0x3F },   /* its table's kind 0xb's sound: BN4's 0xbc at BN6's slot 0x3f */
};
static const XvSoundCall spreads_sound_calls[] = {
	{ 0x080EC7DE, 0x200 },   /* its sound 0x8d: BN4's song at the grown table's slot 0x200 */
};
static const XcSong spreads_songs[] = { { 0x9F, 0x3D }, { 0xBC, 0x3F }, { 0x8D, 0x200 } };

#define SPREADS_CODE { .name = "Spreads (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x2800), \
	.blocks = spreads_blocks, .nblocks = (int)(sizeof spreads_blocks / sizeof *spreads_blocks), \
	.data = spreads_data, .ndata = (int)(sizeof spreads_data / sizeof *spreads_data), \
	.calls = spreads_calls, .ncalls = (int)(sizeof spreads_calls / sizeof *spreads_calls), \
	.words = spreads_words, .nwords = (int)(sizeof spreads_words / sizeof *spreads_words), \
	.imms = spreads_imms, .nimms = (int)(sizeof spreads_imms / sizeof *spreads_imms), \
	.types = spreads_types, .ntypes = (int)(sizeof spreads_types / sizeof *spreads_types), \
	.sound_calls = spreads_sound_calls, .nsound_calls = (int)(sizeof spreads_sound_calls / sizeof *spreads_sound_calls), \
	.sprites = spreads_sprites, .nsprites = (int)(sizeof spreads_sprites / sizeof *spreads_sprites), \
	.effects = spreads_effects, .neffects = (int)(sizeof spreads_effects / sizeof *spreads_effects), \
	.bytes = spreads_bytes, .nbytes = (int)(sizeof spreads_bytes / sizeof *spreads_bytes) }
/* ---- FlmLine: BN4's attack 0x22 (080ECAAF) as BN6's 0xd1; its chips 12 (595), 13 (596), 14 (597) ---- */
/* MegaMan's state 0x22 (080ECAAE-080ECB74), the family of its chips' records: moved */
/* its third-kind 0x19 (080CFF80-080D014E), spawned through 080D00D8: no BN6 type proved its twin, moved */
/* its third-kind 0x2: Spreads's copy, the dispatch's 0xc2, shared (its spawner copied) */
/* its sprite 4:0xa (080CFFB2): BN6's 4:0x9b, copied */
/* its sound 0x9d: BN4's song at slot 0x36, the families' slot for it (FAMILY_SLOTS) */
/* its chip 329 (id 608) sits out: a Program Advance, unformed until BN4's recipes are listed */
/* its chip 330 (id 609) sits out: a Program Advance, unformed until BN4's recipes are listed */
/* its chip 331 (id 610) sits out: a Program Advance, unformed until BN4's recipes are listed */

static const XvBlock flmline_blocks[] = {
	{ 0x080ECAAE, 0x080ECB74, XC4_CODE + 0x1762, 0xACD55EB9u },
	{ 0x080CFF80, 0x080D014E, XC4_CODE + 0x1830, 0x12544EF6u },
	{ 0x080CD6E0, 0x080CD708, XC4_CODE + 0x1A00, 0x6FFE1431u },
};
static const XvData flmline_data[] = {
	{ 0x080ECAC0, 0x080ECAC4, XV_POOL },
	{ 0x080ECAC4, 0x080ECACC, XV_TABLE },
	{ 0x080CFF92, 0x080CFF94, XV_RAW },
	{ 0x080CFF94, 0x080CFF98, XV_POOL },
	{ 0x080CFF98, 0x080CFFA4, XV_TABLE },
	{ 0x080D002E, 0x080D0030, XV_RAW },
	{ 0x080D0030, 0x080D0034, XV_POOL },
	{ 0x080D0034, 0x080D0040, XV_TABLE },
	{ 0x080D013A, 0x080D014E, XV_RAW },
};
static const XvPair flmline_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08009ECC, 0x0800CCA6 },   /* project:gutsman, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800D7CA, 0x08013774 },   /* project:project:chips, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
};
static const XvPair flmline_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm flmline_imms[] = {
	{ 0x080CD6E4, THUMB_MOVS, 0, 0, 0x2, 0xC2 },   /* its object's type 0x2: moved, the dispatch's 0xc2 */
	{ 0x080CFFAE, THUMB_MOVS, 2, 0, 0xA, 0x9B },   /* its object's sprite: list 4 0xa in BN4, 0x9b in BN6 (copied) */
	{ 0x080CFFF8, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's slot 0x36, a BN4 family's too */
	{ 0x080D0014, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D00BC, THUMB_MOVS, 0, 0, 0x19, 0xBD },   /* its object's type 0x19: moved, the dispatch's 0xbd */
	{ 0x080D00CC, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080ECAB2, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080ECAD4, THUMB_MOVS, 0, 0, 0xD, 0xA },   /* MegaMan's animation as he readies FlmLine: BN4's 0x0D, BN6's 0x0A of its own sprite of him (one frame of 2 in each art) */
	{ 0x080ECAEA, THUMB_MOVS, 1, 0, 0x23, 0x29 },   /* the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 (BN6_NAVI_SIDE_NAVI) */
	{ 0x080ECB14, THUMB_MOVS, 0, 0, 0x11, 0xD },   /* MegaMan's animation as he fires FlmLine: BN4's 0x11, BN6's 0x0D of its own sprite of him (frames of 3 and 2 in each art) */
};
static const XvType flmline_types[] = {
	{ 3, 0xBD, XV_TYPE_MOVED, 0x19 },   /* BN4's third-kind 0x19, moved, the dispatch's 0xbd */
	{ 3, 0xC2, XV_TYPE_SHARED, 0x02 },   /* BN4's third-kind 0x2, Spreads's copy: the dispatch's 0xc2 */
};
static const XvSprite flmline_sprites[] = {
	{ 4, 0x0A, 4, 0x9B, XC4_ART + 0x1DB00, 0x1F00 },   /* BN4's 4:0xa copied */
};
static const XcSong flmline_songs[] = { { 0x9D, 0x36 } };

#define FLMLINE_CODE { .name = "FlmLine (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x2C00), \
	.blocks = flmline_blocks, .nblocks = (int)(sizeof flmline_blocks / sizeof *flmline_blocks), \
	.data = flmline_data, .ndata = (int)(sizeof flmline_data / sizeof *flmline_data), \
	.calls = flmline_calls, .ncalls = (int)(sizeof flmline_calls / sizeof *flmline_calls), \
	.words = flmline_words, .nwords = (int)(sizeof flmline_words / sizeof *flmline_words), \
	.imms = flmline_imms, .nimms = (int)(sizeof flmline_imms / sizeof *flmline_imms), \
	.types = flmline_types, .ntypes = (int)(sizeof flmline_types / sizeof *flmline_types), \
	.sprites = flmline_sprites, .nsprites = (int)(sizeof flmline_sprites / sizeof *flmline_sprites) }
/* ---- Thunder: BN4's attack 0x26 (080ECE15) as BN6's 0xd6; its chips 57 (647), 58 (648), 59 (649) ---- */
/* MegaMan's state 0x26 (080ECE14-080ECEAE), the family of its chips' records: moved */
/* its sprite 4:0x11 (080D2048): BN6's 4:0x9a, copied */
/* its sound 0xad (080D20B0): BN4's song copied into the grown song table's slot 0x201, on BN4's player 26 */

static const XvBlock thunder_blocks[] = {
	{ 0x080ECE14, 0x080ECEAE, XC4_CODE + 0x1A30, 0x9EFCA62Au },
	{ 0x080D2000, 0x080D2224, XC4_CODE + 0x1AD0, 0xC7652602u },
};
static const XvData thunder_data[] = {
	{ 0x080ECE26, 0x080ECE28, XV_RAW },
	{ 0x080ECE28, 0x080ECE2C, XV_POOL },
	{ 0x080ECE2C, 0x080ECE34, XV_TABLE },
	{ 0x080D2012, 0x080D2014, XV_RAW },
	{ 0x080D2014, 0x080D2018, XV_POOL },
	{ 0x080D2018, 0x080D2024, XV_TABLE },
	{ 0x080D210A, 0x080D210C, XV_RAW },
	{ 0x080D210C, 0x080D2110, XV_POOL },
	{ 0x080D2110, 0x080D2114, XV_TABLE },
	{ 0x080D2114, 0x080D2118, XV_POOL },
	{ 0x080D21CC, 0x080D21D4, XV_POOL },
	{ 0x080D221C, 0x080D2224, XV_POOL },
};
static const XvPair thunder_calls[] = {
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
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x08010560, 0x080D0B94 },   /* project:spikey, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08013212, 0x0801A258 },   /* project:gutsman, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
};
static const XvPair thunder_words[] = {
};
static const XvImm thunder_imms[] = {
	{ 0x080D2044, THUMB_MOVS, 2, 0, 0x11, 0x9A },   /* its object's sprite: list 4 0x11 in BN4, 0x9a in BN6 (copied) */
	{ 0x080D20DE, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D215A, THUMB_MOVS, 0, 0, 0x2A, 0xBB },   /* its object's type 0x2a: moved, the dispatch's 0xbb */
	{ 0x080D216A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080ECE18, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080ECE3C, THUMB_MOVS, 0, 0, 0x12, 0xE },   /* MegaMan's animation as he throws Thunder: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, 2 and 3 in each art) */
};
static const XvNumber thunder_numbers[] = {
	{ 0x080D221C, 0x04000000, 0x04000000 },   /* a panel's flag, the thunder's search for its target by its side (BN4 0x080D21E8 after its panel's flags, 0x08009A86; BN6's own thunder tests the same bit, 0x080CAE60) */
	{ 0x080D2220, 0x08000000, 0x08000000 },   /* ... the other side's (BN4 0x080D2208; BN6's 0x080CAE66) */
};
static const XvType thunder_types[] = {
	{ 3, 0xBB, XV_TYPE_MOVED, 0x2A },   /* BN4's third-kind 0x2a, moved, the dispatch's 0xbb */
};
static const XvSprite thunder_sprites[] = {
	{ 4, 0x11, 4, 0x9A, XC4_ART + 0x1FA00, 0xB00 },   /* BN4's 4:0x11 copied */
};
static const XvSoundCall thunder_sound_calls[] = {
	{ 0x080D20B0, 0x201 },   /* its sound 0xad: BN4's song at the grown table's slot 0x201 */
};
static const XcSong thunder_songs[] = { { 0xAD, 0x201 } };

#define THUNDER_CODE { .name = "Thunder (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x3000), \
	.blocks = thunder_blocks, .nblocks = (int)(sizeof thunder_blocks / sizeof *thunder_blocks), \
	.data = thunder_data, .ndata = (int)(sizeof thunder_data / sizeof *thunder_data), \
	.calls = thunder_calls, .ncalls = (int)(sizeof thunder_calls / sizeof *thunder_calls), \
	.words = thunder_words, .nwords = (int)(sizeof thunder_words / sizeof *thunder_words), \
	.imms = thunder_imms, .nimms = (int)(sizeof thunder_imms / sizeof *thunder_imms), \
	.numbers = thunder_numbers, .nnumbers = (int)(sizeof thunder_numbers / sizeof *thunder_numbers), \
	.types = thunder_types, .ntypes = (int)(sizeof thunder_types / sizeof *thunder_types), \
	.sound_calls = thunder_sound_calls, .nsound_calls = (int)(sizeof thunder_sound_calls / sizeof *thunder_sound_calls), \
	.sprites = thunder_sprites, .nsprites = (int)(sizeof thunder_sprites / sizeof *thunder_sprites) }
/* ---- TwnFng: BN4's attack 0x34 (080ED4C7) as BN6's 0xd3; its chips 69 (656), 70 (657), 71 (658) ---- */
/* MegaMan's state 0x34 (080ED4C6-080ED572), the family of its chips' records: moved */
/* its sprite 4:0x27 (080D79EC): BN6's 4:0x99, copied */
/* its effect 0x15: 0xcb, as MokoRus moved it */
/* its effect 0x15's sprite 5:0xd is listed by MokoRus (5:0xeb): installed with it */
/* its effect 0x15: BN6's 0xcb, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0xa6: BN4's song at slot 0x38, the families' slot for it (FAMILY_SLOTS) */
/* its chip 335 (id 614) sits out: a Program Advance, unformed until BN4's recipes are listed */
/* its chip 336 (id 615) sits out: a Program Advance, unformed until BN4's recipes are listed */
/* its chip 337 (id 616) sits out: a Program Advance, unformed until BN4's recipes are listed */

static const XvBlock twnfng_blocks[] = {
	{ 0x080ED4C6, 0x080ED572, XC4_CODE + 0x1D02, 0x5BDE5F2Cu },
	{ 0x080D79B8, 0x080D7B64, XC4_CODE + 0x1DB0, 0x709BC9C5u },
};
static const XvData twnfng_data[] = {
	{ 0x080ED4D8, 0x080ED4DC, XV_POOL },
	{ 0x080ED4DC, 0x080ED4E8, XV_TABLE },
	{ 0x080D79CA, 0x080D79CC, XV_RAW },
	{ 0x080D79CC, 0x080D79D0, XV_POOL },
	{ 0x080D79D0, 0x080D79DC, XV_TABLE },
	{ 0x080D7AA0, 0x080D7AA4, XV_POOL },
	{ 0x080D7AA4, 0x080D7AAC, XV_TABLE },
	{ 0x080D7B5A, 0x080D7B5C, XV_RAW },
	{ 0x080D7B5C, 0x080D7B64, XV_POOL },
};
static const XvPair twnfng_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012ED0, 0x0801A082 },   /* project:melody, DIFF (audited) */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair twnfng_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm twnfng_imms[] = {
	{ 0x080D79E8, THUMB_MOVS, 2, 0, 0x27, 0x99 },   /* its object's sprite: list 4 0x27 in BN4, 0x99 in BN6 (copied) */
	{ 0x080D7A54, THUMB_MOVS, 0, 0, 0xA6, 0x38 },   /* its sound 0xa6: BN4's copied into BN6's slot 0x38, a BN4 family's too */
	{ 0x080D7A8C, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080D7AB0, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D7B0C, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D7B3E, THUMB_MOVS, 0, 0, 0x51, 0xB9 },   /* its object's type 0x51: moved, the dispatch's 0xb9 */
	{ 0x080D7B50, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080ED4CA, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080ED4EA, THUMB_MOVS, 0, 0, 0x12, 0xE },   /* MegaMan's animation as he throws TwnFng: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, 2 and 3 in each art) */
};
static const XvType twnfng_types[] = {
	{ 3, 0xB9, XV_TYPE_MOVED, 0x51 },   /* BN4's third-kind 0x51, moved, the dispatch's 0xb9 */
};
static const XvSprite twnfng_sprites[] = {
	{ 4, 0x27, 4, 0x99, XC4_ART + 0x20500, 0x300 },   /* BN4's 4:0x27 copied */
};
static const XcSong twnfng_songs[] = { { 0xA6, 0x38 } };

#define TWNFNG_CODE { .name = "TwnFng (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x3400), \
	.blocks = twnfng_blocks, .nblocks = (int)(sizeof twnfng_blocks / sizeof *twnfng_blocks), \
	.data = twnfng_data, .ndata = (int)(sizeof twnfng_data / sizeof *twnfng_data), \
	.calls = twnfng_calls, .ncalls = (int)(sizeof twnfng_calls / sizeof *twnfng_calls), \
	.words = twnfng_words, .nwords = (int)(sizeof twnfng_words / sizeof *twnfng_words), \
	.imms = twnfng_imms, .nimms = (int)(sizeof twnfng_imms / sizeof *twnfng_imms), \
	.types = twnfng_types, .ntypes = (int)(sizeof twnfng_types / sizeof *twnfng_types), \
	.sprites = twnfng_sprites, .nsprites = (int)(sizeof twnfng_sprites / sizeof *twnfng_sprites) }
/* ---- Hole: BN4's support 0xb (080E45F7) as BN6's 0x8b; its chips 82 (527), 216 (528), 193 (562) ---- */
/* its fourth-kind 0x20 (080E4584-080E4618), which its routine (080E45F6) spawns: moved */
/* its fourth-kind 0x6a (080E9838-080E98E0), which its routine (080E98BC) spawns: moved */
/* its fourth-kind 0x1f (080E4354-080E4584): no BN6 type of its body, moved with him */
/* its table 080E423C's row 0x1: +0x0 0x8, none of BN6's, left as BN4's (+0 a row's panel type: BN4's plain 2, poison 4, grass 6 and ice 7 BN6's numbers too, its holy 9 BN6's 5 (BN4_PANEL_HOLY: BN6's 5's word), its Hole 0xB the engine's 15 (XPANEL_HOLE); its 5, 8 and 0xA (rows 5, 1 and 4) BN6 has none of, rows no moved chip's parameter names (docs/rom/bn4.md, The panel changer and the Hole panel)) */
/* its table 080E423C's row 0x4: +0x0 0xa, none of BN6's, left as BN4's (+0 a row's panel type: BN4's plain 2, poison 4, grass 6 and ice 7 BN6's numbers too, its holy 9 BN6's 5 (BN4_PANEL_HOLY: BN6's 5's word), its Hole 0xB the engine's 15 (XPANEL_HOLE); its 5, 8 and 0xA (rows 5, 1 and 4) BN6 has none of, rows no moved chip's parameter names (docs/rom/bn4.md, The panel changer and the Hole panel)) */
/* its table 080E423C's row 0x5: +0x0 0x5, none of BN6's, left as BN4's (+0 a row's panel type: BN4's plain 2, poison 4, grass 6 and ice 7 BN6's numbers too, its holy 9 BN6's 5 (BN4_PANEL_HOLY: BN6's 5's word), its Hole 0xB the engine's 15 (XPANEL_HOLE); its 5, 8 and 0xA (rows 5, 1 and 4) BN6 has none of, rows no moved chip's parameter names (docs/rom/bn4.md, The panel changer and the Hole panel)) */
/* its sound 0x105 (080E4394): BN4's song copied into the grown song table's slot 0x280, on BN4's player 15 */
/* its sound 0x106 (080E43E8): BN4's song copied into the grown song table's slot 0x281, on BN4's player 14 */
/* its sound 0x107 (080E447A, one of the call's: SOUND_ADDS): BN4's song copied into the grown song table's slot 0x282, on BN4's player 22 */
/* its sound 0x124 (080E447A, one of the call's: SOUND_ADDS): BN4's song copied into the grown song table's slot 0x29f, on BN4's player 18 */
/* its chip 82 is a dark chip of BN4's (row 527): a battle it is used in costs 20 max HP */
/* its chip 122 has no row of the registry (the run's by name, or none) */
/* its chip 139 has no row of the registry (the run's by name, or none) */
/* its chip 208 has no row of the registry (the run's by name, or none) */
/* its chip 216 is a dark chip of BN4's (row 528): a battle it is used in costs 20 max HP */
/* its chip 193 is a dark chip of BN4's (row 562): a battle it is used in costs 20 max HP */

static const XvBlock hole_blocks[] = {
	{ 0x080E4584, 0x080E4618, XC4_CODE + 0x1F60, 0x4B619B7Eu },
	{ 0x080E9838, 0x080E98E0, XC4_CODE + 0x2000, 0xD3ADC8B1u },
	{ 0x080E4354, 0x080E4584, XC4_CODE + 0x20B0, 0xCF968AD5u },
	{ 0x080E423C, 0x080E4354, XC4_CODE + 0x22E0, 0x77CB1EA6u },
};
static const XvData hole_data[] = {
	{ 0x080E4592, 0x080E4594, XV_RAW },
	{ 0x080E4594, 0x080E4598, XV_POOL },
	{ 0x080E4598, 0x080E45A4, XV_TABLE },
	{ 0x080E45B2, 0x080E45B4, XV_RAW },
	{ 0x080E45B4, 0x080E45B8, XV_POOL },
	{ 0x080E45B8, 0x080E45C8, XV_TABLE },
	{ 0x080E9846, 0x080E9848, XV_RAW },
	{ 0x080E9848, 0x080E984C, XV_POOL },
	{ 0x080E984C, 0x080E9858, XV_TABLE },
	{ 0x080E9866, 0x080E9868, XV_RAW },
	{ 0x080E9868, 0x080E986C, XV_POOL },
	{ 0x080E986C, 0x080E987C, XV_TABLE },
	{ 0x080E98DE, 0x080E98E0, XV_RAW },
	{ 0x080E4362, 0x080E4364, XV_RAW },
	{ 0x080E4364, 0x080E4368, XV_POOL },
	{ 0x080E4368, 0x080E4374, XV_TABLE },
	{ 0x080E43B0, 0x080E43B8, XV_POOL },
	{ 0x080E43B8, 0x080E43C8, XV_TABLE },
	{ 0x080E423C, 0x080E4354, XV_RAW },
};
static const XvPair hole_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009DC4, 0x0800CC0A },   /* project:families-2, DIFF (audited) */
	{ 0x08009E38, 0x0800CC52 },   /* project:project:chips, DIFF (audited) */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
};
static const XvPair hole_words[] = {
	{ 0x08008BD9, 0x0800B917 },   /* project:project:chips, DIFF */
	{ 0x08008BF9, 0x0800B94D },   /* project:project:chips, SAME */
	{ 0x08008C2B, 0x0800B9B1 },   /* project:project:chips, DIFF */
	{ 0x08008EB9, 0x0800BC89 },   /* project:project:chips, SAME */
	{ 0x08008EEF, 0x0800BD35 },   /* project:project:chips, DIFF */
};
static const XvImm hole_imms[] = {
	{ 0x080E441C, THUMB_LDRB, 3, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080E444C, THUMB_LDRB, 3, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080E456C, THUMB_MOVS, 0, 0, 0x1F, 0x94 },   /* its object's type 0x1f: moved, the dispatch's 0x94 */
	{ 0x080E45FA, THUMB_MOVS, 0, 0, 0x20, 0x95 },   /* its object's type 0x20: moved, the dispatch's 0x95 */
	{ 0x080E460A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E98C0, THUMB_MOVS, 0, 0, 0x6A, 0x96 },   /* its object's type 0x6a: moved, the dispatch's 0x96 */
	{ 0x080E98D0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType hole_types[] = {
	{ 4, 0x94, XV_TYPE_MOVED, 0x1F },   /* BN4's fourth-kind 0x1f, moved, the dispatch's 0x94 */
	{ 4, 0x95, XV_TYPE_MOVED, 0x20 },   /* BN4's fourth-kind 0x20, moved, the dispatch's 0x95 */
	{ 4, 0x96, XV_TYPE_MOVED, 0x6A },   /* BN4's fourth-kind 0x6a, moved, the dispatch's 0x96 */
};
static const XvByte hole_bytes[] = {
	{ 0x080E42B4, 0x09, 0x05 },   /* its table's row 0x6: +0x0 0x9 in BN4, 0x5 in BN6 */
	{ 0x080E42C8, 0x09, 0x05 },   /* its table's row 0x7: +0x0 0x9 in BN4, 0x5 in BN6 */
	{ 0x080E42F0, 0x0B, 0x0F },   /* its table's row 0x9: +0x0 0xb in BN4, 0xf in BN6 */
	{ 0x080E4304, 0x0B, 0x0F },   /* its table's row 0xa: +0x0 0xb in BN4, 0xf in BN6 */
	{ 0x080E432C, 0x0B, 0x0F },   /* its table's row 0xc: +0x0 0xb in BN4, 0xf in BN6 */
};
static const XvSoundCall hole_sound_calls[] = {
	{ 0x080E4394, 0x280 },   /* its sound 0x105: BN4's song at the grown table's slot 0x280 */
	{ 0x080E43E8, 0x281 },   /* its sound 0x106: BN4's song at the grown table's slot 0x281 */
	{ 0x080E447A, 0x17B, true },   /* its sounds 0x107, 0x124: BN4's numbers plus 0x17b, their slots of the grown table (the panel changer's sound as it sets its panels (0x080E4460-0x080E447A): none for its kinds 9 and 0xA (Hole's, DrkLine's: beq past the call), 0x124 for 8 and 0xD (poison: movs r0,#0x25; adds r0,#0xff at 0x080E4476), 0x107 for the rest (movs r0,#8; adds r0,#0xff at 0x080E446A), DrkStage's Holes 0xC) */
};
static const XcSong hole_songs[] = { { 0x105, 0x280 }, { 0x106, 0x281 }, { 0x107, 0x282 }, { 0x124, 0x29F } };

#define HOLE_CODE { .name = "Hole (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x3800), \
	.blocks = hole_blocks, .nblocks = (int)(sizeof hole_blocks / sizeof *hole_blocks), \
	.data = hole_data, .ndata = (int)(sizeof hole_data / sizeof *hole_data), \
	.calls = hole_calls, .ncalls = (int)(sizeof hole_calls / sizeof *hole_calls), \
	.words = hole_words, .nwords = (int)(sizeof hole_words / sizeof *hole_words), \
	.imms = hole_imms, .nimms = (int)(sizeof hole_imms / sizeof *hole_imms), \
	.types = hole_types, .ntypes = (int)(sizeof hole_types / sizeof *hole_types), \
	.sound_calls = hole_sound_calls, .nsound_calls = (int)(sizeof hole_sound_calls / sizeof *hole_sound_calls), \
	.bytes = hole_bytes, .nbytes = (int)(sizeof hole_bytes / sizeof *hole_bytes) }
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
	{ XCHIP_ATTACK, 0x1E, 0xD0, 0x080EC761, XC4_HOMES + 0x2800, SPREADS_CODE, spreads_songs, (int)(sizeof spreads_songs / sizeof *spreads_songs), 0, 0 },
	{ XCHIP_ATTACK, 0x22, 0xD1, 0x080ECAAF, XC4_HOMES + 0x2C00, FLMLINE_CODE, flmline_songs, (int)(sizeof flmline_songs / sizeof *flmline_songs), 0, 0 },
	{ XCHIP_ATTACK, 0x26, 0xD6, 0x080ECE15, XC4_HOMES + 0x3000, THUNDER_CODE, thunder_songs, (int)(sizeof thunder_songs / sizeof *thunder_songs), 0, 0 },
	{ XCHIP_ATTACK, 0x34, 0xD3, 0x080ED4C7, XC4_HOMES + 0x3400, TWNFNG_CODE, twnfng_songs, (int)(sizeof twnfng_songs / sizeof *twnfng_songs), 0, 0 },
	{ XCHIP_SUPPORT, 0x0B, 0x8B, 0x080E45F7, XC4_HOMES + 0x3800, HOLE_CODE, hole_songs, (int)(sizeof hole_songs / sizeof *hole_songs), 0, 0 },
	{ XCHIP_SUPPORT, 0x3B, 0xBB, 0x080E98BD, XC4_HOMES + 0x3C00, { .name = "Hole (BN4's), its subfamily 0x3b", .game = XROM_BN4_RED_SUN_US }, NULL, 0, 0, 15 },
};
const int xchips_bn4_count = (int)(sizeof xchips_bn4 / sizeof *xchips_bn4);

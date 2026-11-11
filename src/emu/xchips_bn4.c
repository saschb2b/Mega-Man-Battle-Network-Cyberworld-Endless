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
#include "xchips_bn4_art.h"

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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080DCC3E, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7A5C, THUMB_MOVS, 0, 0, 0x4F, 0x91 },   /* its object's type 0x4f: moved, the dispatch's 0x91 */
	{ 0x080E7A6C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7AB8, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7B0E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E7B74, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
};
static const XvType panlsht_types[] = {
	{ 3, 0x7B, XV_TYPE_MOVED4, 0x7B },   /* BN4's third-kind 0x7b, moved, the chips' page */
	{ 4, 0x91, XV_TYPE_MOVED, 0x4F },   /* BN4's fourth-kind 0x4f, moved, the dispatch's 0x91 */
};
static const XvSprite panlsht_sprites[] = {
	{ 3, 0x38, 3, 0x70, XC4_ROOM_ROW(0), 0x400 },   /* BN4's 3:0x38, the room's row 0 */
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
/* its table 080D8580's kind 6: sprite 3:0x31 in BN4, 0x71 in BN6 (copied) */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080D8790, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E5D3C, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E5D7C, THUMB_MOVS, 0, 0, 0x34, 0x92 },   /* its object's type 0x34: moved, the dispatch's 0x92 */
	{ 0x080E5D8C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E5EEC, THUMB_MOVS, 0, 0, 0x35, 0x93 },   /* its object's type 0x35: moved, the dispatch's 0x93 */
};
static const XvType shotstar_types[] = {
	{ 3, 0x56, XV_TYPE_MOVED4, 0x56 },   /* BN4's third-kind 0x56, moved, the chips' page */
	{ 4, 0x92, XV_TYPE_MOVED, 0x34 },   /* BN4's fourth-kind 0x34, moved, the dispatch's 0x92 */
	{ 4, 0x93, XV_TYPE_MOVED, 0x35 },   /* BN4's fourth-kind 0x35, moved, the dispatch's 0x93 */
};
static const XvSprite shotstar_sprites[] = {
	{ 3, 0x31, 3, 0x71, XC4_ROOM_ROW(1), 0x300 },   /* BN4's 3:0x31, the room's row 1 */
	{ 5, 0x0A, 5, 0xEF, XC4_ROOM_ROW(2), 0xD00 },   /* BN4's 5:0xa, the room's row 2 */
};
static const XvId shotstar_effects[] = { { 0x06, 0xCF } };
static const XvByte shotstar_bytes[] = {
	{ 0x080D8586, 0x31, 0x71 },   /* its table's kind 1: list 3 0x31 in BN4, 0x71 in BN6 (copied) */
	{ 0x080D85A4, 0x31, 0x71 },   /* its table's kind 6: list 3 0x31 in BN4, 0x71 in BN6 (copied) */
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
	{ 3, 0x12, 4, 0x9F, XC4_ROOM_ROW(3), 0x1300 },   /* BN4's 3:0x12, the room's row 3 */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080CFDF4, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CFE26, THUMB_LSLS, 0, 0, 0x2, 0x4 },   /* a side's players in the battle state: BN4 a word a side's first from +0x44, BN6 0x10 bytes a side from +0xD0 */
	{ 0x080CFE28, THUMB_ADDS, 0, 0, 0x44, 0xD0 },   /* a side's players in the battle state: BN4 +0x44, BN6 +0xD0 */
	{ 0x080CFE38, THUMB_LDRB, 1, 1, 0xB, 0xD },   /* a player's collision data's counter byte: BN4 +0xB (BN4_COLL_COUNTER), BN6 +0xD (BN6_COLL_COUNTER) */
};
static const XvNumber counter_numbers[] = {
	{ 0x080CFDB8, 0x04050601, 0x04050601 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4), as 0x0405FF01: BN6's own attacks pass the same (five of its pools; GutsMan's states, MegaMan's 0x080F052C and 0x080F0606) */
};
static const XvType counter_types[] = {
	{ 3, 0x17, XV_TYPE_MOVED4, 0x17 },   /* BN4's third-kind 0x17, moved, the chips' page */
};
static const XvSprite counter_sprites[] = {
	{ 5, 0x10, 5, 0xEE, XC4_ROOM_ROW(4), 0xB00 },   /* BN4's 5:0x10, the room's row 4 */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080D4EB0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType whitweb_types[] = {
	{ 3, 0x3D, XV_TYPE_MOVED4, 0x3D },   /* BN4's third-kind 0x3d, moved, the chips' page */
};
static const XvSprite whitweb_sprites[] = {
	{ 4, 0x1C, 4, 0x9E, XC4_ROOM_ROW(5), 0x600 },   /* BN4's 4:0x1c, the room's row 5 */
	{ 5, 0x01, 5, 0xED, XC4_ROOM_ROW(6), 0x1800 },   /* BN4's 5:0x1, the room's row 6 */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080D3444, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType boomer_types[] = {
	{ 3, 0x32, XV_TYPE_MOVED4, 0x32 },   /* BN4's third-kind 0x32, moved, the chips' page */
};
static const XvSprite boomer_sprites[] = {
	{ 4, 0x14, 4, 0x9D, XC4_ROOM_ROW(7), 0xF00 },   /* BN4's 4:0x14, the room's row 7 */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080D2520, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType mokorus_types[] = {
	{ 3, 0x2C, XV_TYPE_MOVED4, 0x2C },   /* BN4's third-kind 0x2c, moved, the chips' page */
};
static const XvSprite mokorus_sprites[] = {
	{ 1, 0x11, 1, 0x7F, XC4_ROOM_ROW(8), 0x3900 },   /* BN4's 1:0x11, the room's row 8 */
	{ 5, 0x02, 5, 0xEC, XC4_ROOM_ROW(9), 0x1300 },   /* BN4's 5:0x2, the room's row 9 */
	{ 5, 0x0D, 5, 0xEB, XC4_ROOM_ROW(10), 0x2300 },   /* BN4's 5:0xd, the room's row 10 */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080D5DD0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvNumber sidbmbo_numbers[] = {
	{ 0x080D5D84, 0x04050401, 0x04050401 },   /* a collision region's spawn word (Kilby's: its ldr r4 at 0x080BC946, then bl 0x080CD7E2, object_spawnCollisionRegion, whose twin BN6's 0x080C6BDA takes r4 the same), the form of 0x04050601 and 0x0405FF01; BN6's ROM holds the word too (0x080E4A4C) */
};
static const XvType sidbmbo_types[] = {
	{ 3, 0x44, XV_TYPE_MOVED4, 0x44 },   /* BN4's third-kind 0x44, moved, the chips' page */
};
static const XvSprite sidbmbo_sprites[] = {
	{ 1, 0x0C, 1, 0x7E, XC4_ROOM_ROW(11), 0x3300 },   /* BN4's 1:0xc, the room's row 11 */
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
/* its table 080CD524's kind 35: sprite 3:0x39 in BN4, 0x76 in BN6 (copied) */
/* its table 080CD524's kind 36: sprite 3:0x39 in BN4, 0x76 in BN6 (copied) */
/* its table 080CD524's kind 37: sprite 3:0x39 in BN4, 0x76 in BN6 (copied) */
/* its table 080CD524's kind 47: sprite 3:0x2 in BN4, 0x77 in BN6 (copied) */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080CF492, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CF546, THUMB_LDRB, 0, 7, 0x8, 0xA },   /* collision data +0x8 in BN4, +0xa in BN6 */
	{ 0x080CF548, THUMB_LDRB, 1, 7, 0x9, 0xB },   /* collision data +0x9 in BN4, +0xb in BN6 */
	{ 0x080CF58E, THUMB_LDRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CF5A6, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EC764, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080EC788, THUMB_MOVS, 0, 0, 0xE, 0xB },   /* MegaMan's animation as he fires a spread chip: BN4's 0x0E, BN6's 0x0B of its own sprite of him (BN6's twin family 0x25 sets it at 0x080EDF14, through 0x0800F2B6, which stores it and 0xFF as BN4's state does) */
	{ 0x080EC796, THUMB_MOVS, 1, 0, 0x23, 0x29 },   /* the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 (BN6_NAVI_SIDE_NAVI) */
};
static const XvType spreads_types[] = {
	{ 3, 0x02, XV_TYPE_MOVED4, 0x02 },   /* BN4's third-kind 0x2, moved, the chips' page */
	{ 3, 0x12, XV_TYPE_MOVED4, 0x12 },   /* BN4's third-kind 0x12, moved, the chips' page */
};
static const XvSprite spreads_sprites[] = {
	{ 3, 0x19, 3, 0x72, XC4_ROOM_ROW(12), 0x3600 },   /* BN4's 3:0x19, the room's row 12 */
	{ 3, 0x0D, 3, 0x73, XC4_ROOM_ROW(13), 0x2400 },   /* BN4's 3:0xd, the room's row 13 */
	{ 3, 0x1C, 3, 0x74, XC4_ROOM_ROW(14), 0x2700 },   /* BN4's 3:0x1c, the room's row 14 */
	{ 3, 0x1E, 3, 0x75, XC4_ROOM_ROW(15), 0x1900 },   /* BN4's 3:0x1e, the room's row 15 */
	{ 3, 0x39, 3, 0x76, XC4_ROOM_ROW(16), 0x1B00 },   /* BN4's 3:0x39, the room's row 16 */
	{ 3, 0x02, 3, 0x77, XC4_ROOM_ROW(17), 0xF00 },   /* BN4's 3:0x2, the room's row 17 */
	{ 3, 0x1A, 4, 0x9C, XC4_ROOM_ROW(18), 0x1100 },   /* BN4's 3:0x1a, the room's row 18 */
	{ 5, 0x11, 5, 0xEA, XC4_ROOM_ROW(19), 0x2F00 },   /* BN4's 5:0x11, the room's row 19 */
};
static const XvId spreads_effects[] = { { 0x25, 0xCA }, { 0x29, 0xC9 } };
static const XvByte spreads_bytes[] = {
	{ 0x080CD54D, 0x19, 0x72 },   /* its table's kind 10: list 3 0x19 in BN4, 0x72 in BN6 (copied) */
	{ 0x080CD53D, 0x0D, 0x73 },   /* its table's kind 6: list 3 0xd in BN4, 0x73 in BN6 (copied) */
	{ 0x080CD55D, 0x1C, 0x74 },   /* its table's kind 14: list 3 0x1c in BN4, 0x74 in BN6 (copied) */
	{ 0x080CD5ED, 0x19, 0x72 },   /* its table's kind 50: list 3 0x19 in BN4, 0x72 in BN6 (copied) */
	{ 0x080CD555, 0x1E, 0x75 },   /* its table's kind 12: list 3 0x1e in BN4, 0x75 in BN6 (copied) */
	{ 0x080CD5B1, 0x39, 0x76 },   /* its table's kind 35: list 3 0x39 in BN4, 0x76 in BN6 (copied) */
	{ 0x080CD5B5, 0x39, 0x76 },   /* its table's kind 36: list 3 0x39 in BN4, 0x76 in BN6 (copied) */
	{ 0x080CD5B9, 0x39, 0x76 },   /* its table's kind 37: list 3 0x39 in BN4, 0x76 in BN6 (copied) */
	{ 0x080CD5E1, 0x02, 0x77 },   /* its table's kind 47: list 3 0x2 in BN4, 0x77 in BN6 (copied) */
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
/* its third-kind 0x2: Spreads's copy, the dispatch's 0x2, shared (its spawner copied) */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080CFFAE, THUMB_MOVS, 2, 0, 0xA, 0x9B },   /* its object's sprite: list 4 0xa in BN4, 0x9b in BN6 (copied) */
	{ 0x080CFFF8, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's slot 0x36, a BN4 family's too */
	{ 0x080D0014, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D00CC, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080ECAB2, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080ECAD4, THUMB_MOVS, 0, 0, 0xD, 0xA },   /* MegaMan's animation as he readies FlmLine: BN4's 0x0D, BN6's 0x0A of its own sprite of him (one frame of 2 in each art) */
	{ 0x080ECAEA, THUMB_MOVS, 1, 0, 0x23, 0x29 },   /* the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 (BN6_NAVI_SIDE_NAVI) */
	{ 0x080ECB14, THUMB_MOVS, 0, 0, 0x11, 0xD },   /* MegaMan's animation as he fires FlmLine: BN4's 0x11, BN6's 0x0D of its own sprite of him (frames of 3 and 2 in each art) */
};
static const XvType flmline_types[] = {
	{ 3, 0x19, XV_TYPE_MOVED4, 0x19 },   /* BN4's third-kind 0x19, moved, the chips' page */
	{ 3, 0x02, XV_TYPE_SHARED4, 0x02 },   /* BN4's third-kind 0x2, Spreads's copy */
};
static const XvSprite flmline_sprites[] = {
	{ 4, 0x0A, 4, 0x9B, XC4_ROOM_ROW(20), 0x1F00 },   /* BN4's 4:0xa, the room's row 20 */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080D216A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080ECE18, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080ECE3C, THUMB_MOVS, 0, 0, 0x12, 0xE },   /* MegaMan's animation as he throws Thunder: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, 2 and 3 in each art) */
};
static const XvNumber thunder_numbers[] = {
	{ 0x080D221C, 0x04000000, 0x04000000 },   /* a panel's flag, the thunder's search for its target by its side (BN4 0x080D21E8 after its panel's flags, 0x08009A86; BN6's own thunder tests the same bit, 0x080CAE60) */
	{ 0x080D2220, 0x08000000, 0x08000000 },   /* ... the other side's (BN4 0x080D2208; BN6's 0x080CAE66) */
};
static const XvType thunder_types[] = {
	{ 3, 0x2A, XV_TYPE_MOVED4, 0x2A },   /* BN4's third-kind 0x2a, moved, the chips' page */
};
static const XvSprite thunder_sprites[] = {
	{ 4, 0x11, 4, 0x9A, XC4_ROOM_ROW(21), 0xB00 },   /* BN4's 4:0x11, the room's row 21 */
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
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
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
	{ 0x080D7B50, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080ED4CA, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080ED4EA, THUMB_MOVS, 0, 0, 0x12, 0xE },   /* MegaMan's animation as he throws TwnFng: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, 2 and 3 in each art) */
};
static const XvType twnfng_types[] = {
	{ 3, 0x51, XV_TYPE_MOVED4, 0x51 },   /* BN4's third-kind 0x51, moved, the chips' page */
};
static const XvSprite twnfng_sprites[] = {
	{ 4, 0x27, 4, 0x99, XC4_ROOM_ROW(22), 0x300 },   /* BN4's 4:0x27, the room's row 22 */
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
/* ---- MagBolt: BN4's attack 0x59 (080EE013) as BN6's 0xd5; its chips 35 (642), 36 (643), 37 (644) ---- */
/* MegaMan's state 0x59 (080EE012-080EE1A2), the family of its chips' records: moved */
/* its third-kind 0x2: Spreads's copy, the dispatch's 0x2, shared (its spawner copied) */
/* its sound 0xb9 (080EE0E0, 080EE100): BN4's song copied into the grown song table's slot 0x203, on BN4's player 16 */
/* its sound 0xf8 (080EE0DA): BN4's song copied into the grown song table's slot 0x202, on BN4's player 8 */
/* 08012DAA: its call to 0800AF90 (bn4_twins.SIDED): its left and right by the object's side, BN6's way */
/* its chip 341 (id 620) sits out: a Program Advance, unformed until BN4's recipes are listed */
/* its chip 342 (id 621) sits out: a Program Advance, unformed until BN4's recipes are listed */
/* its chip 343 (id 622) sits out: a Program Advance, unformed until BN4's recipes are listed */

static const XvBlock magbolt_blocks[] = {
	{ 0x080EE012, 0x080EE1A2, XC4_CODE + 0x1F62, 0x29ED9811u },
	{ 0x080DBCA0, 0x080DBD94, XC4_CODE + 0x2100, 0x1B22F5D9u },
	{ 0x0800AF90, 0x0800AFB0, XC4_CODE + 0x2200, 0x268EACF4u },
	{ 0x08012D9A, 0x08012DB2, XC4_CODE + 0x2222, 0xCE602857u },
	{ 0x080CD6E0, 0x080CD708, XC4_CODE + 0x2240, 0x6FFE1431u },
};
static const XvData magbolt_data[] = {
	{ 0x080EE024, 0x080EE028, XV_POOL },
	{ 0x080EE028, 0x080EE034, XV_TABLE },
	{ 0x080EE13E, 0x080EE140, XV_RAW },
	{ 0x080EE140, 0x080EE148, XV_POOL },
	{ 0x080EE148, 0x080EE16C, XV_RAW },
	{ 0x080DBCAE, 0x080DBCB0, XV_RAW },
	{ 0x080DBCB0, 0x080DBCB4, XV_POOL },
	{ 0x080DBCB4, 0x080DBCC0, XV_TABLE },
};
static const XvPair magbolt_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800D7CA, 0x08013774 },   /* project:project:chips, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x08012886, 0x08019F86 },   /* project:fireman, LAYOUT */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F7A, 0x0801A146 },   /* project:fireman, DIFF (audited) */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08013212, 0x0801A258 },   /* project:gutsman, LAYOUT */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
};
static const XvPair magbolt_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm magbolt_imms[] = {
	{ 0x08012D9E, THUMB_LDRB, 2, 4, 0x8, 0xA },   /* collision data +0x8 in BN4, +0xa in BN6 */
	{ 0x08012DA0, THUMB_LDRB, 3, 4, 0x9, 0xB },   /* collision data +0x9 in BN4, +0xb in BN6 */
	{ 0x08012DA4, THUMB_STRB, 0, 4, 0x8, 0xA },   /* collision data +0x8 in BN4, +0xa in BN6 */
	{ 0x08012DA8, THUMB_STRB, 1, 4, 0x9, 0xB },   /* collision data +0x9 in BN4, +0xb in BN6 */
	{ 0x08012DAE, THUMB_STRB, 0, 4, 0xA, 0xC },   /* collision data +0xa in BN4, +0xc in BN6 */
	{ 0x080DBCD2, THUMB_MOVS, 3, 0, 0x3, 0x43 },   /* its hit's bits (the collision setup's r3): with the pull's push bit, struck and pushed is bit 0x01 in BN4 (BN4_STRUCK_PUSHED), bit 0x40 in BN6 (BN6_STRUCK_PUSHED) */
	{ 0x080DBD12, THUMB_LDR, 0, 2, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DBD86, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EE016, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080EE044, THUMB_MOVS, 0, 0, 0xD, 0xA },   /* MegaMan's animation as he holds MagBolt out: BN4's 0x0D, BN6's 0x0A of its own sprite of him (one frame of 2 in each art) */
	{ 0x080EE052, THUMB_MOVS, 1, 0, 0x23, 0x29 },   /* the battle NaviStats' Navi, 0 MegaMan (his arm's animation by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 (BN6_NAVI_SIDE_NAVI) */
};
static const XvType magbolt_types[] = {
	{ 3, 0x75, XV_TYPE_MOVED4, 0x75 },   /* BN4's third-kind 0x75, moved, the chips' page */
	{ 3, 0x02, XV_TYPE_SHARED4, 0x02 },   /* BN4's third-kind 0x2, Spreads's copy */
};
static const uint32_t magbolt_sides[] = {
	0x08012DAA,   /* its call to 0800AF90 (bn4_twins.SIDED): its left and right by the object's side, BN6's way */
};
static const XvSoundCall magbolt_sound_calls[] = {
	{ 0x080EE0E0, 0x203 },   /* its sound 0xb9: BN4's song at the grown table's slot 0x203 */
	{ 0x080EE100, 0x203 },   /* its sound 0xb9: BN4's song at the grown table's slot 0x203 */
	{ 0x080EE0DA, 0x202 },   /* its sound 0xf8: BN4's song at the grown table's slot 0x202 */
};
static const XcSong magbolt_songs[] = { { 0xB9, 0x203 }, { 0xF8, 0x202 } };

#define MAGBOLT_CODE { .name = "MagBolt (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x3800), \
	.blocks = magbolt_blocks, .nblocks = (int)(sizeof magbolt_blocks / sizeof *magbolt_blocks), \
	.data = magbolt_data, .ndata = (int)(sizeof magbolt_data / sizeof *magbolt_data), \
	.calls = magbolt_calls, .ncalls = (int)(sizeof magbolt_calls / sizeof *magbolt_calls), \
	.words = magbolt_words, .nwords = (int)(sizeof magbolt_words / sizeof *magbolt_words), \
	.imms = magbolt_imms, .nimms = (int)(sizeof magbolt_imms / sizeof *magbolt_imms), \
	.types = magbolt_types, .ntypes = (int)(sizeof magbolt_types / sizeof *magbolt_types), \
	.sides = magbolt_sides, .nsides = (int)(sizeof magbolt_sides / sizeof *magbolt_sides), \
	.sound_calls = magbolt_sound_calls, .nsound_calls = (int)(sizeof magbolt_sound_calls / sizeof *magbolt_sound_calls) }
/* ---- PropBom: BN4's attack 0x62 (080EE533) as BN6's 0xd4; its chips 157 (671), 158 (672), 159 (673) ---- */
/* MegaMan's state 0x62 (080EE532-080EE5EA), the family of its chips' records: moved */
/* its sprite 4:0x2b (080DEBA0): BN6's 4:0x98, copied */
/* its effect 0x0: BN6's 0xc8, moved (no effect of BN6's draws its sprite's bytes) */
/* its effect 0x15: 0xcb, as MokoRus moved it */
/* its effect 0x15's sprite 5:0xd is listed by MokoRus (5:0xeb): installed with it */
/* its effect 0x15: BN6's 0xcb, moved (no effect of BN6's draws its sprite's bytes) */
/* its sound 0x8d (080EE586): BN4's song copied into the grown song table's slot 0x200, on BN4's player 17 */
/* its sound 0xc8 (080DEC6E): BN4's song copied into the grown song table's slot 0x205, on BN4's player 11 */
/* its sound 0x133 (080DEC08): BN4's song copied into the grown song table's slot 0x204, on BN4's player 28 */

static const XvBlock propbom_blocks[] = {
	{ 0x080EE532, 0x080EE5EA, XC4_CODE + 0x2272, 0x6CCC9E41u },
	{ 0x080DEB70, 0x080DED60, XC4_CODE + 0x2330, 0x5D37AB7Eu },
};
static const XvData propbom_data[] = {
	{ 0x080EE544, 0x080EE548, XV_POOL },
	{ 0x080EE548, 0x080EE554, XV_TABLE },
	{ 0x080EE5B6, 0x080EE5B8, XV_RAW },
	{ 0x080EE5B8, 0x080EE5BC, XV_POOL },
	{ 0x080EE5BC, 0x080EE5CC, XV_RAW },
	{ 0x080DEB88, 0x080DEB8C, XV_POOL },
	{ 0x080DEB8C, 0x080DEB98, XV_TABLE },
	{ 0x080DEC74, 0x080DEC80, XV_RAW },
	{ 0x080DEC8E, 0x080DEC90, XV_RAW },
	{ 0x080DEC90, 0x080DEC98, XV_POOL },
	{ 0x080DEC98, 0x080DECA4, XV_TABLE },
	{ 0x080DED2C, 0x080DED30, XV_POOL },
	{ 0x080DED5E, 0x080DED60, XV_RAW },
};
static const XvPair propbom_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800B230, 0x0800F614 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800B272, 0x0800F656 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800D7CA, 0x08013774 },   /* project:project:chips, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0801457E, 0x0801BD3C },   /* search, SAME */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair propbom_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm propbom_imms[] = {
	{ 0x080DEB9C, THUMB_MOVS, 2, 0, 0x2B, 0x98 },   /* its object's sprite: list 4 0x2b in BN4, 0x98 in BN6 (copied) */
	{ 0x080DEBF8, THUMB_STRH, 1, 0, 0x1E, 0x2E },   /* collision data +0x1e in BN4, +0x2e in BN6 */
	{ 0x080DEC2A, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DEC58, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080DEC66, THUMB_MOVS, 4, 0, 0x0, 0xC8 },   /* its effect: 0x0 in BN4, 0xc8 in BN6 (moved) */
	{ 0x080DED04, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DED16, THUMB_MOVS, 2, 0, 0x0, 0xC8 },   /* its effect over a pattern of panels: 0x0 in BN4, 0xc8 in BN6 (as its own spawn of it) */
	{ 0x080DED44, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EE536, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
	{ 0x080EE55C, THUMB_MOVS, 0, 0, 0x12, 0xE },   /* MegaMan's animation as he throws PropBom: BN4's 0x12, BN6's 0x0E of its own sprite of him (frames of 1, 2, 2 and 3 in each art) */
	{ 0x080EE58A, THUMB_MOVS, 1, 0, 0x23, 0x29 },   /* the battle NaviStats' Navi, 0 MegaMan (the bomb's height by it): BN4 +0x23 (BN4_NAVI_NAVI), BN6 +0x29 (BN6_NAVI_SIDE_NAVI) */
};
static const XvNumber propbom_numbers[] = {
	{ 0x080DED2C, 0x0405FF11, 0x0405FF11 },   /* PropBom's bomb's blast's hit word (BN4 0x080DED2C, r4 of the hit area's spawn 0x080CD7E2): its bits, its reach and its frames, bytes of a number as MagBolt's pull's */
};
static const XvType propbom_types[] = {
	{ 3, 0x8C, XV_TYPE_MOVED4, 0x8C },   /* BN4's third-kind 0x8c, moved, the chips' page */
};
static const XvSprite propbom_sprites[] = {
	{ 4, 0x2B, 4, 0x98, XC4_ROOM_ROW(23), 0x700 },   /* BN4's 4:0x2b, the room's row 23 */
	{ 5, 0x00, 5, 0xE9, XC4_ROOM_ROW(24), 0x1C00 },   /* BN4's 5:0x0, the room's row 24 */
};
static const XvId propbom_effects[] = { { 0x00, 0xC8 } };
static const XvSoundCall propbom_sound_calls[] = {
	{ 0x080EE586, 0x200 },   /* its sound 0x8d: BN4's song at the grown table's slot 0x200 */
	{ 0x080DEC6E, 0x205 },   /* its sound 0xc8: BN4's song at the grown table's slot 0x205 */
	{ 0x080DEC08, 0x204 },   /* its sound 0x133: BN4's song at the grown table's slot 0x204 */
};
static const XcSong propbom_songs[] = { { 0x8D, 0x200 }, { 0xC8, 0x205 }, { 0x133, 0x204 } };

#define PROPBOM_CODE { .name = "PropBom (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x3C00), \
	.blocks = propbom_blocks, .nblocks = (int)(sizeof propbom_blocks / sizeof *propbom_blocks), \
	.data = propbom_data, .ndata = (int)(sizeof propbom_data / sizeof *propbom_data), \
	.calls = propbom_calls, .ncalls = (int)(sizeof propbom_calls / sizeof *propbom_calls), \
	.words = propbom_words, .nwords = (int)(sizeof propbom_words / sizeof *propbom_words), \
	.imms = propbom_imms, .nimms = (int)(sizeof propbom_imms / sizeof *propbom_imms), \
	.numbers = propbom_numbers, .nnumbers = (int)(sizeof propbom_numbers / sizeof *propbom_numbers), \
	.types = propbom_types, .ntypes = (int)(sizeof propbom_types / sizeof *propbom_types), \
	.sound_calls = propbom_sound_calls, .nsound_calls = (int)(sizeof propbom_sound_calls / sizeof *propbom_sound_calls), \
	.sprites = propbom_sprites, .nsprites = (int)(sizeof propbom_sprites / sizeof *propbom_sprites), \
	.effects = propbom_effects, .neffects = (int)(sizeof propbom_effects / sizeof *propbom_effects) }
/* ---- ThunSoul: BN4's attack 0x47 (080EDC5B) as BN6's 0xd7; its chips  ---- */
/* MegaMan's state 0x47 (080EDC5A-080EDD2E), the family of its chips' records: moved */
/* ThunSoul's power attack 0x58 (0800D298), which returns the family: copied with it */
/* its third-kind 0x2: Spreads's copy, the dispatch's 0x2, shared (its spawner copied) */
/* its sprite 3:0x3a (080DAFF4): BN6's 3:0x78, copied */
/* its sprite 3:0x37 (080DAFF4, by 080DAFE6): BN6's 3:0x79, copied */
/* its sound 0x91 (its table 080DAFB0's kind 0x0): BN4's song at the grown table's slot 0x300, played through the unit's shim */
/* its chip 47 (id 583) sits out: SandRing, the generator line's (its records' kinds, the ring's rows 1-3 of table 080DAFB0 and its spec): the family is moved for ThunSoul's charged shot, whose ring is kind 0 (the attack's +0xC, which its power attack zeroes) */

static const XvBlock thunsoul_blocks[] = {
	{ 0x080EDC5A, 0x080EDD2E, XC4_CODE + 0x2522, 0xD47325F6u },
	{ 0x080DAFBC, 0x080DB0EC, XC4_CODE + 0x2600, 0x351F2B25u },
	{ 0x080DAFB0, 0x080DAFBC, XC4_CODE + 0x2730, 0xEF4D0545u },
	{ 0x0800D298, 0x0800D2B4, XC4_CODE + 0x2740, 0x92FC0667u },
	{ 0x080CD6E0, 0x080CD708, XC4_CODE + 0x2760, 0x6FFE1431u },
};
static const XvData thunsoul_data[] = {
	{ 0x080EDC6C, 0x080EDC70, XV_POOL },
	{ 0x080EDC70, 0x080EDC78, XV_TABLE },
	{ 0x080DAFCA, 0x080DAFCC, XV_RAW },
	{ 0x080DAFCC, 0x080DAFD0, XV_POOL },
	{ 0x080DAFD0, 0x080DAFDC, XV_TABLE },
	{ 0x080DB066, 0x080DB068, XV_RAW },
	{ 0x080DB068, 0x080DB06C, XV_POOL },
	{ 0x080DB0EA, 0x080DB0EC, XV_RAW },
	{ 0x080DAFB0, 0x080DAFBC, XV_RAW },
};
static const XvPair thunsoul_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009DC4, 0x0800CC0A },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800D7CA, 0x08013774 },   /* project:project:chips, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F7A, 0x0801A146 },   /* project:fireman, DIFF (audited) */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08013212, 0x0801A258 },   /* project:gutsman, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
};
static const XvPair thunsoul_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm thunsoul_imms[] = {
	{ 0x0800D2B0, THUMB_MOVS, 0, 0, 0x47, 0xD7 },   /* ThunSoul's power attack 0x58: the family it returns, BN4's 0x47, BN6's grown 0xd7 */
	{ 0x080DAFE6, THUMB_MOVS, 2, 0, 0x37, 0x79 },   /* its object's sprite on its other path: list 3 0x37 in BN4, 0x79 in BN6 (copied) */
	{ 0x080DAFEE, THUMB_MOVS, 2, 0, 0x3A, 0x78 },   /* its object's sprite: list 3 0x3a in BN4, 0x78 in BN6 (copied) */
	{ 0x080DB078, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DB0E0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EDC5E, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
};
static const XvType thunsoul_types[] = {
	{ 3, 0x6D, XV_TYPE_MOVED4, 0x6D },   /* BN4's third-kind 0x6d, moved, the chips' page */
	{ 3, 0x02, XV_TYPE_SHARED4, 0x02 },   /* BN4's third-kind 0x2, Spreads's copy */
};
static const XvSprite thunsoul_sprites[] = {
	{ 3, 0x3A, 3, 0x78, XC4_ROOM_ROW(25), 0x500 },   /* BN4's 3:0x3a, the room's row 25 */
	{ 3, 0x37, 3, 0x79, XC4_ROOM_ROW(26), 0xD00 },   /* BN4's 3:0x37, the room's row 26 */
};
static const XvSoundCall thunsoul_sound_calls[] = {
	{ 0x080DB05C, 0x300 },   /* its table 080DAFB0's kind 0x0's sound 0x91: BN4's song at the grown table's slot 0x300 */
};
static const XcSong thunsoul_songs[] = { { 0x91, 0x300 } };

#define THUNSOUL_CODE { .name = "ThunSoul (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x4000), \
	.blocks = thunsoul_blocks, .nblocks = (int)(sizeof thunsoul_blocks / sizeof *thunsoul_blocks), \
	.data = thunsoul_data, .ndata = (int)(sizeof thunsoul_data / sizeof *thunsoul_data), \
	.calls = thunsoul_calls, .ncalls = (int)(sizeof thunsoul_calls / sizeof *thunsoul_calls), \
	.words = thunsoul_words, .nwords = (int)(sizeof thunsoul_words / sizeof *thunsoul_words), \
	.imms = thunsoul_imms, .nimms = (int)(sizeof thunsoul_imms / sizeof *thunsoul_imms), \
	.types = thunsoul_types, .ntypes = (int)(sizeof thunsoul_types / sizeof *thunsoul_types), \
	.sound_calls = thunsoul_sound_calls, .nsound_calls = (int)(sizeof thunsoul_sound_calls / sizeof *thunsoul_sound_calls), \
	.sprites = thunsoul_sprites, .nsprites = (int)(sizeof thunsoul_sprites / sizeof *thunsoul_sprites) }
/* ---- SeekBom: BN4's attack 0x9 (080EB3FF) as BN6's 0xd2; its chips 160 (598), 161 (599), 162 (600) ---- */
/* MegaMan's state 0x9 (080EB3FE-080EB51E), the family of its chips' records: moved */
/* its subfamily 0x9's entry (080DEEF2) lies in the third-kind 0x8d's code (080DED60-080DEFEC): moved */
/* its fourth-kind 0x11 (080E3328-080E33E8): no BN6 type of its body, moved with him */
/* its third-kind 0x2: Spreads's copy, the dispatch's 0x2, shared (its spawner copied) */
/* its sprite 4:0x24 (080DED8C): BN6's 4:0x97, copied */
/* its effect 0x15: 0xcb, as MokoRus moved it */
/* its effect 0x15's sprite 5:0xd is listed by MokoRus (5:0xeb): installed with it */
/* its effect 0x15: BN6's 0xcb, moved (no effect of BN6's draws its sprite's bytes) */
/* its effect 0x0: 0xc8, as PropBom moved it */
/* its effect 0x0's sprite 5:0x0 is listed by PropBom (5:0xe9): installed with it */
/* its fourth-kind 0x11's effect, its +0x5: 0x0 from its spawns (2 in the unit's code): through a shim that gives r4 0xc8 for 0x0 */
/* its sound 0x70: BN4's song at slot 0x3a, the families' slot for it (FAMILY_SLOTS) */
/* its sound 0x90 (080EB454): BN4's song copied into the grown song table's slot 0x206, on BN4's player 8 */

static const XvBlock seekbom_blocks[] = {
	{ 0x080EB3FE, 0x080EB51E, XC4_CODE + 0x2792, 0x5D011E1Au },
	{ 0x080DED60, 0x080DEFEC, XC4_CODE + 0x28C0, 0xE55B2946u },
	{ 0x080E3328, 0x080E33E8, XC4_CODE + 0x2B50, 0xF6B3BF48u },
	{ 0x080CD6E0, 0x080CD708, XC4_CODE + 0x2C10, 0x6FFE1431u },
};
static const XvData seekbom_data[] = {
	{ 0x080EB410, 0x080EB414, XV_POOL },
	{ 0x080EB414, 0x080EB41C, XV_TABLE },
	{ 0x080EB4BA, 0x080EB4BC, XV_RAW },
	{ 0x080EB4BC, 0x080EB4C4, XV_POOL },
	{ 0x080EB4C4, 0x080EB4EC, XV_TABLE },
	{ 0x080EB4EC, 0x080EB500, XV_RAW },
	{ 0x080DED72, 0x080DED74, XV_RAW },
	{ 0x080DED74, 0x080DED78, XV_POOL },
	{ 0x080DED78, 0x080DED84, XV_TABLE },
	{ 0x080DEE30, 0x080DEE34, XV_POOL },
	{ 0x080DEE34, 0x080DEE3C, XV_TABLE },
	{ 0x080DEFDC, 0x080DEFE0, XV_POOL },
	{ 0x080DEFE0, 0x080DEFE8, XV_RAW },
	{ 0x080DEFE8, 0x080DEFEC, XV_POOL },
	{ 0x080E3336, 0x080E3338, XV_RAW },
	{ 0x080E3338, 0x080E3344, XV_TABLE },
	{ 0x080E3344, 0x080E3348, XV_POOL },
	{ 0x080E3364, 0x080E3368, XV_POOL },
	{ 0x080E3368, 0x080E336C, XV_RAW },
	{ 0x080E337A, 0x080E337C, XV_RAW },
	{ 0x080E337C, 0x080E3380, XV_POOL },
	{ 0x080E3380, 0x080E3384, XV_TABLE },
};
static const XvPair seekbom_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001956, 0x08001330 },   /* search, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009ECC, 0x0800CCA6 },   /* project:gutsman, SAME */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800BA66, 0x0800FDB6 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800C9FC, 0x08011714 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800D7CA, 0x08013774 },   /* project:project:chips, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x08014620, 0x0801BDDE },   /* search, SAME */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair seekbom_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
	{ 0x080CE2B9, 0x0810A953 },   /* its subfamily 0x2's entry, which no chip of the unit names: BN6's null routine */
	{ 0x080D2749, 0x0810A953 },   /* its subfamily 0x3's entry, which no chip of the unit names: BN6's null routine */
	{ 0x080D3A59, 0x0810A953 },   /* its subfamily 0x4's entry, which no chip of the unit names: BN6's null routine */
	{ 0x080D5B63, 0x0810A953 },   /* its subfamily 0x5's entry, which no chip of the unit names: BN6's null routine */
	{ 0x080D645F, 0x0810A953 },   /* its subfamily 0x6's entry, which no chip of the unit names: BN6's null routine */
	{ 0x080DC30D, 0x0810A953 },   /* its subfamily 0x7's entry, which no chip of the unit names: BN6's null routine */
	{ 0x080DCA7D, 0x0810A953 },   /* its subfamily 0x8's entry, which no chip of the unit names: BN6's null routine */
};
static const XvImm seekbom_imms[] = {
	{ 0x080DED88, THUMB_MOVS, 2, 0, 0x24, 0x97 },   /* its object's sprite: list 4 0x24 in BN4, 0x97 in BN6 (copied) */
	{ 0x080DEDFC, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DEE18, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080DEE80, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080DEEEC, THUMB_STRB, 1, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E33C0, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's slot 0x3a, a BN4 family's too */
	{ 0x080E33D8, THUMB_MOVS, 0, 0, 0x11, 0x94 },   /* its object's type 0x11: moved, the dispatch's 0x94 */
	{ 0x080EB402, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* MegaMan's AI data's +0x70 in BN4, +0xa0 in BN6: the attack's variables */
};
static const XvType seekbom_types[] = {
	{ 3, 0x8D, XV_TYPE_MOVED4, 0x8D },   /* BN4's third-kind 0x8d, moved, the chips' page */
	{ 4, 0x94, XV_TYPE_MOVED, 0x11 },   /* BN4's fourth-kind 0x11, moved, the dispatch's 0x94 */
	{ 3, 0x02, XV_TYPE_SHARED4, 0x02 },   /* BN4's third-kind 0x2, Spreads's copy */
};
static const XvSprite seekbom_sprites[] = {
	{ 4, 0x24, 4, 0x97, XC4_ROOM_ROW(27), 0x600 },   /* BN4's 4:0x24, the room's row 27 */
};
static const XvSoundCall seekbom_sound_calls[] = {
	{ 0x080EB454, 0x206 },   /* its sound 0x90: BN4's song at the grown table's slot 0x206 */
};
static const XvEffectCall seekbom_effect_calls[] = {
	{ 0x080E33B6, 1, { { 0x00, 0xC8 } } },   /* its fourth-kind 0x11's effect, its +0x5: 0x0 from its spawns (2 in the unit's code): 0x0 BN6's 0xc8 */
};
static const XcSong seekbom_songs[] = { { 0x70, 0x3A }, { 0x90, 0x206 } };

#define SEEKBOM_CODE { .name = "SeekBom (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x4400), \
	.blocks = seekbom_blocks, .nblocks = (int)(sizeof seekbom_blocks / sizeof *seekbom_blocks), \
	.data = seekbom_data, .ndata = (int)(sizeof seekbom_data / sizeof *seekbom_data), \
	.calls = seekbom_calls, .ncalls = (int)(sizeof seekbom_calls / sizeof *seekbom_calls), \
	.words = seekbom_words, .nwords = (int)(sizeof seekbom_words / sizeof *seekbom_words), \
	.imms = seekbom_imms, .nimms = (int)(sizeof seekbom_imms / sizeof *seekbom_imms), \
	.types = seekbom_types, .ntypes = (int)(sizeof seekbom_types / sizeof *seekbom_types), \
	.sound_calls = seekbom_sound_calls, .nsound_calls = (int)(sizeof seekbom_sound_calls / sizeof *seekbom_sound_calls), \
	.effect_calls = seekbom_effect_calls, .neffect_calls = (int)(sizeof seekbom_effect_calls / sizeof *seekbom_effect_calls), \
	.sprites = seekbom_sprites, .nsprites = (int)(sizeof seekbom_sprites / sizeof *seekbom_sprites) }
/* ---- Meteors: BN4's misc 0x11 (080EAE93) as BN6's 0x91; its chips 163 (674), 164 (675), 165 (676) ---- */
/* its routine (080EAE92) lies in the fourth-kind 0x81's code (080EADF4-080EAEA6), which it spawns: moved */
/* its third-kind 0x56: ShotStar's copy, the dispatch's 0x56, shared (its spawner copied) */

static const XvBlock meteors_blocks[] = {
	{ 0x080EADF4, 0x080EAEA6, XC4_CODE + 0x2C40, 0x8C47ED0Fu },
	{ 0x080D877C, 0x080D879A, XC4_CODE + 0x2D00, 0x50812991u },
};
static const XvData meteors_data[] = {
	{ 0x080EAE02, 0x080EAE04, XV_RAW },
	{ 0x080EAE04, 0x080EAE08, XV_POOL },
	{ 0x080EAE08, 0x080EAE14, XV_TABLE },
	{ 0x080EAE28, 0x080EAE2C, XV_POOL },
	{ 0x080EAE2C, 0x080EAE34, XV_TABLE },
};
static const XvPair meteors_calls[] = {
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800FC48, 0x08016168 },   /* project:project:chips, SAME (audited) */
};
static const XvPair meteors_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
};
static const XvImm meteors_imms[] = {
	{ 0x080D8790, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EAE54, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EAE70, THUMB_MOVS, 0, 0, 0x81, 0x95 },   /* its object's type 0x81: moved, the dispatch's 0x95 */
	{ 0x080EAE80, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType meteors_types[] = {
	{ 4, 0x95, XV_TYPE_MOVED, 0x81 },   /* BN4's fourth-kind 0x81, moved, the dispatch's 0x95 */
	{ 3, 0x56, XV_TYPE_SHARED4, 0x56 },   /* BN4's third-kind 0x56, ShotStar's copy */
};

#define METEORS_CODE { .name = "Meteors (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x4800), \
	.blocks = meteors_blocks, .nblocks = (int)(sizeof meteors_blocks / sizeof *meteors_blocks), \
	.data = meteors_data, .ndata = (int)(sizeof meteors_data / sizeof *meteors_data), \
	.calls = meteors_calls, .ncalls = (int)(sizeof meteors_calls / sizeof *meteors_calls), \
	.words = meteors_words, .nwords = (int)(sizeof meteors_words / sizeof *meteors_words), \
	.imms = meteors_imms, .nimms = (int)(sizeof meteors_imms / sizeof *meteors_imms), \
	.types = meteors_types, .ntypes = (int)(sizeof meteors_types / sizeof *meteors_types) }
/* ---- TimeBomb: BN4's support 0x11 (080E4CA7) as BN6's 0x91; its chips 97 (529) ---- */
/* its fourth-kind 0x27 (080E4C3C-080E4F5C), which its routine (080E4CA6) spawns: moved */
/* its fourth-kind 0x24 (080E4A0A-080E4AD4): no BN6 type of its body, moved with him */
/* its third-kind 0x4b is an actor (080D6CB8: the actors' runner with its table 080D6CCC, 7 states): its table in BN6's layout, 9 states, its own from 8 */
/* its sprite 3:0x23 (080D6C28): BN6's 3:0x7a, copied */
/* its effect 0x0: 0xc8, as PropBom moved it */
/* its effect 0x0's sprite 5:0x0 is listed by PropBom (5:0xe9): installed with it */
/* its effect 0x0 (its table 080E4A00's): BN6's 0xc8, moved (no effect of BN6's draws its sprite's bytes) */
/* its effect 0x0: 0xc8, as PropBom moved it */
/* its effect 0x0's sprite 5:0x0 is listed by PropBom (5:0xe9): installed with it */
/* its effect 0x0: BN6's 0xc8, moved (no effect of BN6's draws its sprite's bytes) */
/* its effect 0x2's sprite 5:0x2 is listed by MokoRus (5:0xec): installed with it */
/* its effect 0x2: BN6's 0xc7, moved (no effect of BN6's draws its sprite's bytes) */
/* its effect 0x15: 0xcb, as MokoRus moved it */
/* its effect 0x15's sprite 5:0xd is listed by MokoRus (5:0xeb): installed with it */
/* its effect 0x15: BN6's 0xcb, moved (no effect of BN6's draws its sprite's bytes) */
/* its effect 0x2's sprite 5:0x2, which MokoRus lists: listed by it too, the same number (5:0xec), as its effect's row needs its sprite among its own */
/* its sound 0x70: BN4's song at slot 0x3a, the families' slot for it (FAMILY_SLOTS) */
/* its sound 0xb0 (080D6C52): BN4's song copied into the grown song table's slot 0x1e2, on BN4's player 23 */
/* its sound 0xbc: BN4's song at slot 0x3f, the families' slot for it (FAMILY_SLOTS) */
/* its sound 0xe3 (080D6D92, 080D6DB6): BN4's song copied into the grown song table's slot 0x1e3, on BN4's player 8 */
/* its sound 0xe4 (080D6DDA): BN4's song copied into the grown song table's slot 0x1e4, on BN4's player 28 */
/* its chip 346 has no row of the registry (the run's by name, or none) */

static const XvBlock timebomb_blocks[] = {
	{ 0x080E4C3C, 0x080E4F5C, XC4_CODE + 0x2D20, 0x12C9A5CFu },
	{ 0x080D6BF8, 0x080D6ED4, XC4_CODE + 0x3040, 0xF7AE8E1Du },
	{ 0x080E4A0A, 0x080E4AD4, XC4_CODE + 0x3322, 0xB899F21Fu },
	{ 0x08012618, 0x08012630, XC4_CODE + 0x33F0, 0x4286738Eu },
	{ 0x080E4A00, 0x080E4A0A, XC4_CODE + 0x3410, 0xF8D9D3A5u },
	{ 0x08111888, 0x0811188E, XC4_CODE + 0x3420, 0x7B317508u },
};
static const XvData timebomb_data[] = {
	{ 0x080E4C4A, 0x080E4C4C, XV_RAW },
	{ 0x080E4C4C, 0x080E4C50, XV_POOL },
	{ 0x080E4C50, 0x080E4C5C, XV_TABLE },
	{ 0x080E4C6A, 0x080E4C6C, XV_RAW },
	{ 0x080E4C6C, 0x080E4C70, XV_POOL },
	{ 0x080E4C70, 0x080E4C80, XV_TABLE },
	{ 0x080E4D64, 0x080E4D74, XV_POOL },
	{ 0x080E4D74, 0x080E4D94, XV_RAW },
	{ 0x080E4DD6, 0x080E4DD8, XV_RAW },
	{ 0x080E4DD8, 0x080E4DE0, XV_POOL },
	{ 0x080E4DE0, 0x080E4DE4, XV_RAW },
	{ 0x080E4E60, 0x080E4E68, XV_POOL },
	{ 0x080D6C06, 0x080D6C08, XV_RAW },
	{ 0x080D6C08, 0x080D6C0C, XV_POOL },
	{ 0x080D6C0C, 0x080D6C18, XV_TABLE },
	{ 0x080D6C90, 0x080D6C98, XV_POOL },
	{ 0x080D6C98, 0x080D6CA0, XV_RAW },
	{ 0x080D6CC6, 0x080D6CC8, XV_RAW },
	{ 0x080D6CC8, 0x080D6CCC, XV_POOL },
	{ 0x080D6CCC, 0x080D6CE8, XV_TABLE },
	{ 0x080D6D50, 0x080D6D58, XV_POOL },
	{ 0x080D6D66, 0x080D6D68, XV_RAW },
	{ 0x080D6D68, 0x080D6D6C, XV_POOL },
	{ 0x080D6D6C, 0x080D6D7C, XV_TABLE },
	{ 0x080D6D9C, 0x080D6DA0, XV_POOL },
	{ 0x080D6DC0, 0x080D6DC4, XV_POOL },
	{ 0x080D6DE4, 0x080D6DE8, XV_POOL },
	{ 0x080D6E5E, 0x080D6E60, XV_RAW },
	{ 0x080D6E60, 0x080D6E64, XV_POOL },
	{ 0x080D6E64, 0x080D6E6C, XV_RAW },
	{ 0x080D6ED2, 0x080D6ED4, XV_RAW },
	{ 0x080E4A18, 0x080E4A1C, XV_POOL },
	{ 0x080E4A1C, 0x080E4A28, XV_TABLE },
	{ 0x080E4A90, 0x080E4A94, XV_POOL },
	{ 0x080E4AD0, 0x080E4AD4, XV_POOL },
	{ 0x08012618, 0x08012630, XV_RAW },
	{ 0x080E4A00, 0x080E4A0A, XV_RAW },
};
static const XvPair timebomb_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08000AAA, 0x08000C72 },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E44, 0x08002DD8 },   /* search, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002EFE, 0x08002EAC },   /* search, SAME */
	{ 0x08002FF8, 0x08002FA6 },   /* search, SAME */
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800B230, 0x0800F614 },   /* project:project:chips, DIFF (audited) */
	{ 0x0800B272, 0x0800F656 },   /* project:project:chips, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x0801393E, 0x0801AD12 },   /* project:project:chips, DIFF (audited) */
	{ 0x08013F3E, 0x0801B394 },   /* project:project:chips, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0802608C, 0x080302A8 },   /* project:research, SAME */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair timebomb_words[] = {
	{ 0x08008BD9, 0x0800B917 },   /* project:project:chips, DIFF */
	{ 0x08008BF9, 0x0800B94D },   /* project:project:chips, SAME */
	{ 0x08008C2B, 0x0800B9B1 },   /* project:project:chips, DIFF */
	{ 0x08008EB9, 0x0800BC89 },   /* project:project:chips, SAME */
	{ 0x08008EEF, 0x0800BD35 },   /* project:project:chips, DIFF */
	{ 0x0801013B, 0x080165B9 },   /* project:research, DIFF */
	{ 0x08010277, 0x080166AF },   /* project:project:chips, DIFF */
	{ 0x080104AD, 0x08016B03 },   /* project:research, DIFF */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
	{ 0x080112C5, 0x08017CC1 },   /* project:project:chips, SAME */
};
static const XvImm timebomb_imms[] = {
	{ 0x080D6C26, THUMB_MOVS, 2, 0, 0x23, 0x7A },   /* its object's sprite: list 3 0x23 in BN4, 0x7a in BN6 (copied) */
	{ 0x080D6D10, THUMB_MOVS, 4, 0, 0x15, 0xCB },   /* its effect: 0x15 in BN4, 0xcb in BN6 (moved) */
	{ 0x080D6D46, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080D6E06, THUMB_LDRB, 0, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080D6E18, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D6E3C, THUMB_MOVS, 0, 0, 0xBC, 0x3F },   /* its sound 0xbc: BN4's copied into BN6's slot 0x3f, a BN4 family's too */
	{ 0x080D6E7C, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's slot 0x3a, a BN4 family's too */
	{ 0x080D6E88, THUMB_MOVS, 4, 0, 0x0, 0xC8 },   /* its effect: 0x0 in BN4, 0xc8 in BN6 (moved) */
	{ 0x080E4AA0, THUMB_MOVS, 0, 0, 0x24, 0x96 },   /* its object's type 0x24: moved, the dispatch's 0x96 */
	{ 0x080E4CAA, THUMB_MOVS, 0, 0, 0x27, 0x97 },   /* its object's type 0x27: moved, the dispatch's 0x97 */
	{ 0x080E4CBA, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E4E56, THUMB_MOVS, 4, 0, 0x2, 0xC7 },   /* its effect: 0x2 in BN4, 0xc7 in BN6 (moved) */
};
static const XvNumber timebomb_numbers[] = {
	{ 0x080D6CC8, 0x080D6CCC, 0x088F3F80 },   /* its state table: the unit's in BN6's layout, at its home's XC4_HOME_STATES */
};
static const XvRef timebomb_states[] = {
	{ 0x080D6CE9, true, 0, false },   /* 0: its own */
	{ 0x080165B9, false, 0, false },   /* 1: BN6's */
	{ 0x080D6E6D, true, 0, false },   /* 2: its own */
	{ 0x080166AF, false, 0, false },   /* 3: BN6's */
	{ 0x08016B03, false, 0, false },   /* 4: BN6's */
	{ 0x08017CC1, false, 0, false },   /* 5: BN6's */
	{ 0x08016B37, false, 0, false },   /* 6: BN6's */
	{ 0x08016B73, false, 0, false },   /* 7: BN6's */
	{ 0x080D6D59, true, 0, false },   /* 8: its own */
};
static const XvType timebomb_types[] = {
	{ 3, 0x4B, XV_TYPE_MOVED4, 0x4B },   /* BN4's third-kind 0x4b, moved, the chips' page */
	{ 4, 0x96, XV_TYPE_MOVED, 0x24 },   /* BN4's fourth-kind 0x24, moved, the dispatch's 0x96 */
	{ 4, 0x97, XV_TYPE_MOVED, 0x27 },   /* BN4's fourth-kind 0x27, moved, the dispatch's 0x97 */
};
static const XvSprite timebomb_sprites[] = {
	{ 3, 0x23, 3, 0x7A, XC4_ROOM_ROW(28), 0xC00 },   /* BN4's 3:0x23, the room's row 28 */
	{ 5, 0x02, 5, 0xEC, XC4_ROOM_ROW(29), 0x1300 },   /* BN4's 5:0x2, the room's row 29 */
};
static const XvId timebomb_effects[] = { { 0x02, 0xC7 } };
static const XvByte timebomb_bytes[] = {
	{ 0x080E4A02, 0x00, 0xC8 },   /* its table's kind 0x1's effect: 0x0 in BN4, 0xc8 in BN6 (moved) */
};
static const XvSoundCall timebomb_sound_calls[] = {
	{ 0x080D6C52, 0x1E2 },   /* its sound 0xb0: BN4's song at the grown table's slot 0x1e2 */
	{ 0x080D6D92, 0x1E3 },   /* its sound 0xe3: BN4's song at the grown table's slot 0x1e3 */
	{ 0x080D6DB6, 0x1E3 },   /* its sound 0xe3: BN4's song at the grown table's slot 0x1e3 */
	{ 0x080D6DDA, 0x1E4 },   /* its sound 0xe4: BN4's song at the grown table's slot 0x1e4 */
};
static const XcSong timebomb_songs[] = { { 0x70, 0x3A }, { 0xB0, 0x1E2 }, { 0xBC, 0x3F }, { 0xE3, 0x1E3 }, { 0xE4, 0x1E4 } };

#define TIMEBOMB_CODE { .name = "TimeBomb (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x4C00), \
	.blocks = timebomb_blocks, .nblocks = (int)(sizeof timebomb_blocks / sizeof *timebomb_blocks), \
	.data = timebomb_data, .ndata = (int)(sizeof timebomb_data / sizeof *timebomb_data), \
	.calls = timebomb_calls, .ncalls = (int)(sizeof timebomb_calls / sizeof *timebomb_calls), \
	.words = timebomb_words, .nwords = (int)(sizeof timebomb_words / sizeof *timebomb_words), \
	.imms = timebomb_imms, .nimms = (int)(sizeof timebomb_imms / sizeof *timebomb_imms), \
	.numbers = timebomb_numbers, .nnumbers = (int)(sizeof timebomb_numbers / sizeof *timebomb_numbers), \
	.states = timebomb_states, .nstates = (int)(sizeof timebomb_states / sizeof *timebomb_states), \
	.types = timebomb_types, .ntypes = (int)(sizeof timebomb_types / sizeof *timebomb_types), \
	.sound_calls = timebomb_sound_calls, .nsound_calls = (int)(sizeof timebomb_sound_calls / sizeof *timebomb_sound_calls), \
	.sprites = timebomb_sprites, .nsprites = (int)(sizeof timebomb_sprites / sizeof *timebomb_sprites), \
	.effects = timebomb_effects, .neffects = (int)(sizeof timebomb_effects / sizeof *timebomb_effects), \
	.bytes = timebomb_bytes, .nbytes = (int)(sizeof timebomb_bytes / sizeof *timebomb_bytes) }
/* ---- Ligtnin: BN4's misc 0xc (080DF98D) as BN6's 0x8c; its chips 166 (677), 167 (678), 168 (679) ---- */
/* its routine (080DF98C) lies in the third-kind 0x93's code (080DF7F8-080DF9B0), which it spawns: moved */
/* its sprite 5:0x14 (080DF01C): BN6's 5:0xe8, copied */
/* its table 080DF674's kind 0: sprite 3:0x25 in BN4, 0x7b in BN6 (copied) */
/* its table 080DF674's kind 1: sprite 3:0xb in BN4, 0x7c in BN6 (copied) */
/* its effect 0x6b: BN6's 0xc6, moved (no effect of BN6's draws its sprite's bytes) */
/* its table 080DF674's kind 0: its sound 0x120, BN4's song copied into the grown song table's slot 0x240 */
/* its table 080DF674's kind 1: its sound 0x13e, BN4's song copied into the grown song table's slot 0x241 */
/* its sound 0x143 (080DF05E): BN4's song copied into the grown song table's slot 0x242, on BN4's player 23 */
/* its chip 181 (id 689) sits out: AquaUp, the same routine as Ligtnin's by its record's first parameter, its strike (the third kind's 0x92) moved with the unit but never run in a spec: it rises only from a cracked panel of the other side's with a foe on it, and the differential's BN4 battle sets no panel's type yet */
/* its chip 182 (id 690) sits out: AquaUp, the same routine as Ligtnin's by its record's first parameter, its strike (the third kind's 0x92) moved with the unit but never run in a spec: it rises only from a cracked panel of the other side's with a foe on it, and the differential's BN4 battle sets no panel's type yet */
/* its chip 183 (id 691) sits out: AquaUp, the same routine as Ligtnin's by its record's first parameter, its strike (the third kind's 0x92) moved with the unit but never run in a spec: it rises only from a cracked panel of the other side's with a foe on it, and the differential's BN4 battle sets no panel's type yet */
/* its chip 184 (id 692) sits out: GreenWd, the same routine as Ligtnin's by its record's first parameter, its strike (the third kind's 0x92) moved with the unit but never run in a spec: it rises only from a grass panel of the other side's with a foe on it, and the differential's BN4 battle sets no panel's type yet */
/* its chip 185 (id 693) sits out: GreenWd, the same routine as Ligtnin's by its record's first parameter, its strike (the third kind's 0x92) moved with the unit but never run in a spec: it rises only from a grass panel of the other side's with a foe on it, and the differential's BN4 battle sets no panel's type yet */
/* its chip 186 (id 694) sits out: GreenWd, the same routine as Ligtnin's by its record's first parameter, its strike (the third kind's 0x92) moved with the unit but never run in a spec: it rises only from a grass panel of the other side's with a foe on it, and the differential's BN4 battle sets no panel's type yet */

static const XvBlock ligtnin_blocks[] = {
	{ 0x080DF7F8, 0x080DF9B0, XC4_CODE + 0x3430, 0xCD3C14E6u },
	{ 0x080DEFEC, 0x080DF128, XC4_CODE + 0x35F0, 0xE3E47628u },
	{ 0x080DF68C, 0x080DF7F8, XC4_CODE + 0x3730, 0xD07CF041u },
	{ 0x080DF674, 0x080DF68C, XC4_CODE + 0x38A0, 0x38CC9DEFu },
};
static const XvData ligtnin_data[] = {
	{ 0x080DF806, 0x080DF808, XV_RAW },
	{ 0x080DF808, 0x080DF80C, XV_POOL },
	{ 0x080DF80C, 0x080DF818, XV_TABLE },
	{ 0x080DF894, 0x080DF898, XV_POOL },
	{ 0x080DF898, 0x080DF8A4, XV_TABLE },
	{ 0x080DF8F4, 0x080DF8F8, XV_POOL },
	{ 0x080DF8F8, 0x080DF908, XV_RAW },
	{ 0x080DF950, 0x080DF954, XV_POOL },
	{ 0x080DF954, 0x080DF964, XV_RAW },
	{ 0x080DF9A6, 0x080DF9A8, XV_RAW },
	{ 0x080DF9A8, 0x080DF9B0, XV_POOL },
	{ 0x080DEFFA, 0x080DEFFC, XV_RAW },
	{ 0x080DEFFC, 0x080DF000, XV_POOL },
	{ 0x080DF000, 0x080DF00C, XV_TABLE },
	{ 0x080DF0F4, 0x080DF0F8, XV_POOL },
	{ 0x080DF0F8, 0x080DF108, XV_RAW },
	{ 0x080DF126, 0x080DF128, XV_RAW },
	{ 0x080DF69E, 0x080DF6A0, XV_RAW },
	{ 0x080DF6A0, 0x080DF6A4, XV_POOL },
	{ 0x080DF6A4, 0x080DF6B0, XV_TABLE },
	{ 0x080DF742, 0x080DF744, XV_RAW },
	{ 0x080DF744, 0x080DF748, XV_POOL },
	{ 0x080DF748, 0x080DF754, XV_TABLE },
	{ 0x080DF7EE, 0x080DF7F0, XV_RAW },
	{ 0x080DF7F0, 0x080DF7F4, XV_POOL },
	{ 0x080DF7F4, 0x080DF7F8, XV_RAW },
	{ 0x080DF674, 0x080DF68C, XV_RAW },
};
static const XvPair ligtnin_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN4_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009BAC, 0x0800C9F8 },   /* project:project:chips, DIFF (audited) */
	{ 0x08009E98, 0x0800CC72 },   /* search, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F7A, 0x0801A146 },   /* project:fireman, DIFF (audited) */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x08014446, 0x0801BBF4 },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvPair ligtnin_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvImm ligtnin_imms[] = {
	{ 0x080DF01A, THUMB_MOVS, 2, 0, 0x14, 0xE8 },   /* its object's sprite: list 5 0x14 in BN4, 0xe8 in BN6 (copied) */
	{ 0x080DF0DA, THUMB_MOVS, 4, 0, 0x6B, 0xC6 },   /* its effect: 0x6b in BN4, 0xc6 in BN6 (moved) */
	{ 0x080DF11C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DF726, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DF7E4, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DF8AE, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x080DF8B0, THUMB_CMP, 1, 0, 0x3, 0x3 },   /* BN4's panel type 0x3, BN6's 0x3 */
	{ 0x080DF8B6, THUMB_LDRB, 2, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080DF8D8, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DF912, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x080DF914, THUMB_CMP, 1, 0, 0x6, 0x6 },   /* BN4's panel type 0x6, BN6's 0x6 */
	{ 0x080DF91A, THUMB_LDRB, 2, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080DF93C, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DF978, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DF99C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvNumber ligtnin_numbers[] = {
	{ 0x080DF9AC, 0x03800000, 0x03800000 },   /* a panel's flags it may not have (object_checkPanelParameters' r3, BN6's 0x0800CC86 the same body): 0x0B800000's bits but 0x08000000 (GutsMan's objects, 0x080D07C4) */
};
static const XvType ligtnin_types[] = {
	{ 3, 0x8E, XV_TYPE_MOVED4, 0x8E },   /* BN4's third-kind 0x8e, moved, the chips' page */
	{ 3, 0x92, XV_TYPE_MOVED4, 0x92 },   /* BN4's third-kind 0x92, moved, the chips' page */
	{ 3, 0x93, XV_TYPE_MOVED4, 0x93 },   /* BN4's third-kind 0x93, moved, the chips' page */
};
static const XvSprite ligtnin_sprites[] = {
	{ 5, 0x14, 5, 0xE8, XC4_ROOM_ROW(30), 0x700 },   /* BN4's 5:0x14, the room's row 30 */
	{ 3, 0x25, 3, 0x7B, XC4_ROOM_ROW(31), 0x1900 },   /* BN4's 3:0x25, the room's row 31 */
	{ 3, 0x0B, 3, 0x7C, XC4_ROOM_ROW(32), 0x1800 },   /* BN4's 3:0xb, the room's row 32 */
	{ 5, 0x05, 5, 0xE7, XC4_ROOM_ROW(33), 0x5500 },   /* BN4's 5:0x5, the room's row 33 */
};
static const XvId ligtnin_effects[] = { { 0x6B, 0xC6 } };
static const XvByte ligtnin_bytes[] = {
	{ 0x080DF675, 0x25, 0x7B },   /* its table's kind 0: list 3 0x25 in BN4, 0x7b in BN6 (copied) */
	{ 0x080DF681, 0x0B, 0x7C },   /* its table's kind 1: list 3 0xb in BN4, 0x7c in BN6 (copied) */
	{ 0x080DF67E, 0x20, 0x40 },   /* its table's kind 0: its sound 0x120 in BN4, the grown table's slot 0x240 in BN6 (byte 0) */
	{ 0x080DF67F, 0x01, 0x02 },   /* its table's kind 0: its sound 0x120 in BN4, the grown table's slot 0x240 in BN6 (byte 1) */
	{ 0x080DF68A, 0x3E, 0x41 },   /* its table's kind 1: its sound 0x13e in BN4, the grown table's slot 0x241 in BN6 (byte 0) */
	{ 0x080DF68B, 0x01, 0x02 },   /* its table's kind 1: its sound 0x13e in BN4, the grown table's slot 0x241 in BN6 (byte 1) */
};
static const XvSoundCall ligtnin_sound_calls[] = {
	{ 0x080DF05E, 0x242 },   /* its sound 0x143: BN4's song at the grown table's slot 0x242 */
};
static const XcSong ligtnin_songs[] = { { 0x120, 0x240 }, { 0x13E, 0x241 }, { 0x143, 0x242 } };

#define LIGTNIN_CODE { .name = "Ligtnin (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x5000), \
	.blocks = ligtnin_blocks, .nblocks = (int)(sizeof ligtnin_blocks / sizeof *ligtnin_blocks), \
	.data = ligtnin_data, .ndata = (int)(sizeof ligtnin_data / sizeof *ligtnin_data), \
	.calls = ligtnin_calls, .ncalls = (int)(sizeof ligtnin_calls / sizeof *ligtnin_calls), \
	.words = ligtnin_words, .nwords = (int)(sizeof ligtnin_words / sizeof *ligtnin_words), \
	.imms = ligtnin_imms, .nimms = (int)(sizeof ligtnin_imms / sizeof *ligtnin_imms), \
	.numbers = ligtnin_numbers, .nnumbers = (int)(sizeof ligtnin_numbers / sizeof *ligtnin_numbers), \
	.types = ligtnin_types, .ntypes = (int)(sizeof ligtnin_types / sizeof *ligtnin_types), \
	.sound_calls = ligtnin_sound_calls, .nsound_calls = (int)(sizeof ligtnin_sound_calls / sizeof *ligtnin_sound_calls), \
	.sprites = ligtnin_sprites, .nsprites = (int)(sizeof ligtnin_sprites / sizeof *ligtnin_sprites), \
	.effects = ligtnin_effects, .neffects = (int)(sizeof ligtnin_effects / sizeof *ligtnin_effects), \
	.bytes = ligtnin_bytes, .nbytes = (int)(sizeof ligtnin_bytes / sizeof *ligtnin_bytes) }
/* ---- Hole: BN4's support 0xb (080E45F7) as BN6's 0x8b; its chips 82 (527), 216 (528), 193 (562) ---- */
/* its fourth-kind 0x20 (080E4584-080E4618), which its routine (080E45F6) spawns: moved */
/* its fourth-kind 0x6a (080E9838-080E98E0), which its routine (080E98BC) spawns: moved */
/* its fourth-kind 0x1f (080E4354-080E4584): no BN6 type of its body, moved with him */
/* its table 080E423C's row 0x1: +0x0 0x8, none of BN6's, left as BN4's (+0 a row's panel type: BN4's plain 2, poison 4, grass 6 and ice 7 BN6's numbers too, its holy 9 BN6's 5 (BN4_PANEL_HOLY: BN6's 5's word), its Hole 0xB the engine's 16 (XPANEL_HOLE); its 5, 8 and 0xA (rows 5, 1 and 4) BN6 has none of, rows no moved chip's parameter names (docs/rom/bn4.md, The panel changer and the Hole panel)) */
/* its table 080E423C's row 0x4: +0x0 0xa, none of BN6's, left as BN4's (+0 a row's panel type: BN4's plain 2, poison 4, grass 6 and ice 7 BN6's numbers too, its holy 9 BN6's 5 (BN4_PANEL_HOLY: BN6's 5's word), its Hole 0xB the engine's 16 (XPANEL_HOLE); its 5, 8 and 0xA (rows 5, 1 and 4) BN6 has none of, rows no moved chip's parameter names (docs/rom/bn4.md, The panel changer and the Hole panel)) */
/* its table 080E423C's row 0x5: +0x0 0x5, none of BN6's, left as BN4's (+0 a row's panel type: BN4's plain 2, poison 4, grass 6 and ice 7 BN6's numbers too, its holy 9 BN6's 5 (BN4_PANEL_HOLY: BN6's 5's word), its Hole 0xB the engine's 16 (XPANEL_HOLE); its 5, 8 and 0xA (rows 5, 1 and 4) BN6 has none of, rows no moved chip's parameter names (docs/rom/bn4.md, The panel changer and the Hole panel)) */
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
	{ 0x080E4584, 0x080E4618, XC4_CODE + 0x38C0, 0x4B619B7Eu },
	{ 0x080E9838, 0x080E98E0, XC4_CODE + 0x3960, 0xD3ADC8B1u },
	{ 0x080E4354, 0x080E4584, XC4_CODE + 0x3A10, 0xCF968AD5u },
	{ 0x080E423C, 0x080E4354, XC4_CODE + 0x3C40, 0x77CB1EA6u },
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
	{ 0x080E456C, THUMB_MOVS, 0, 0, 0x1F, 0x98 },   /* its object's type 0x1f: moved, the dispatch's 0x98 */
	{ 0x080E45FA, THUMB_MOVS, 0, 0, 0x20, 0x99 },   /* its object's type 0x20: moved, the dispatch's 0x99 */
	{ 0x080E460A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E98C0, THUMB_MOVS, 0, 0, 0x6A, 0x9A },   /* its object's type 0x6a: moved, the dispatch's 0x9a */
	{ 0x080E98D0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvType hole_types[] = {
	{ 4, 0x98, XV_TYPE_MOVED, 0x1F },   /* BN4's fourth-kind 0x1f, moved, the dispatch's 0x98 */
	{ 4, 0x99, XV_TYPE_MOVED, 0x20 },   /* BN4's fourth-kind 0x20, moved, the dispatch's 0x99 */
	{ 4, 0x9A, XV_TYPE_MOVED, 0x6A },   /* BN4's fourth-kind 0x6a, moved, the dispatch's 0x9a */
};
static const XvByte hole_bytes[] = {
	{ 0x080E42B4, 0x09, 0x05 },   /* its table's row 0x6: +0x0 0x9 in BN4, 0x5 in BN6 */
	{ 0x080E42C8, 0x09, 0x05 },   /* its table's row 0x7: +0x0 0x9 in BN4, 0x5 in BN6 */
	{ 0x080E42F0, 0x0B, 0x10 },   /* its table's row 0x9: +0x0 0xb in BN4, 0x10 in BN6 */
	{ 0x080E4304, 0x0B, 0x10 },   /* its table's row 0xa: +0x0 0xb in BN4, 0x10 in BN6 */
	{ 0x080E432C, 0x0B, 0x10 },   /* its table's row 0xc: +0x0 0xb in BN4, 0x10 in BN6 */
};
static const XvSoundCall hole_sound_calls[] = {
	{ 0x080E4394, 0x280 },   /* its sound 0x105: BN4's song at the grown table's slot 0x280 */
	{ 0x080E43E8, 0x281 },   /* its sound 0x106: BN4's song at the grown table's slot 0x281 */
	{ 0x080E447A, 0x17B, true },   /* its sounds 0x107, 0x124: BN4's numbers plus 0x17b, their slots of the grown table (the panel changer's sound as it sets its panels (0x080E4460-0x080E447A): none for its kinds 9 and 0xA (Hole's, DrkLine's: beq past the call), 0x124 for 8 and 0xD (poison: movs r0,#0x25; adds r0,#0xff at 0x080E4476), 0x107 for the rest (movs r0,#8; adds r0,#0xff at 0x080E446A), DrkStage's Holes 0xC) */
};
static const XcSong hole_songs[] = { { 0x105, 0x280 }, { 0x106, 0x281 }, { 0x107, 0x282 }, { 0x124, 0x29F } };

#define HOLE_CODE { .name = "Hole (BN4's)", .game = XROM_BN4_RED_SUN_US, \
	.home = XC4_SHIMS_HOME(XC4_HOMES + 0x5400), \
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
	{ XCHIP_ATTACK, 0x59, 0xD5, 0x080EE013, XC4_HOMES + 0x3800, MAGBOLT_CODE, magbolt_songs, (int)(sizeof magbolt_songs / sizeof *magbolt_songs), 0, 0 },
	{ XCHIP_ATTACK, 0x62, 0xD4, 0x080EE533, XC4_HOMES + 0x3C00, PROPBOM_CODE, propbom_songs, (int)(sizeof propbom_songs / sizeof *propbom_songs), 0, 0 },
	{ XCHIP_ATTACK, 0x47, 0xD7, 0x080EDC5B, XC4_HOMES + 0x4000, THUNSOUL_CODE, thunsoul_songs, (int)(sizeof thunsoul_songs / sizeof *thunsoul_songs), 0, 0 },
	{ XCHIP_ATTACK, 0x09, 0xD2, 0x080EB3FF, XC4_HOMES + 0x4400, SEEKBOM_CODE, seekbom_songs, (int)(sizeof seekbom_songs / sizeof *seekbom_songs), 0, 0 },
	{ XCHIP_MISC, 0x11, 0x91, 0x080EAE93, XC4_HOMES + 0x4800, METEORS_CODE, NULL, 0, 0, 0 },
	{ XCHIP_SUPPORT, 0x11, 0x91, 0x080E4CA7, XC4_HOMES + 0x4C00, TIMEBOMB_CODE, timebomb_songs, (int)(sizeof timebomb_songs / sizeof *timebomb_songs), 0, 0 },
	{ XCHIP_MISC, 0x0C, 0x8C, 0x080DF98D, XC4_HOMES + 0x5000, LIGTNIN_CODE, ligtnin_songs, (int)(sizeof ligtnin_songs / sizeof *ligtnin_songs), 0, 0 },
	{ XCHIP_SUPPORT, 0x0B, 0x8B, 0x080E45F7, XC4_HOMES + 0x5400, HOLE_CODE, hole_songs, (int)(sizeof hole_songs / sizeof *hole_songs), 0, 0 },
	{ XCHIP_SUPPORT, 0x3B, 0xBB, 0x080E98BD, XC4_HOMES + 0x5800, { .name = "Hole (BN4's), its subfamily 0x3b", .game = XROM_BN4_RED_SUN_US }, NULL, 0, 0, 22 },
};
const int xchips_bn4_count = (int)(sizeof xchips_bn4 / sizeof *xchips_bn4);

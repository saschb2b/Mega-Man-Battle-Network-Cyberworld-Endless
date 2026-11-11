/* xnavi_bn4.c: BN4 Red Sun's Navis' recipes, written by tools/xnavirecipe_bn4.py from BN4's and BN6's ROMs
 * (python3 tools/xnavirecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/bn4-navis.md): addresses
 * in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and CRC32s. The Navis here are the generator's whole ones (no
 * decision open), each a Navi of BN6's Navi object at its seat's numbers (src/emu/xnavi_recipe.h XN_BN4_*), installed where
 * BN4's ROM is read, after BN4's families (their art's and the dispatch's numbers continue past theirs). */
#include "xnavi_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"
#include "xvirus_dispatch.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }

/* ---- BN4's GutsMan (its type 0xa there, its enemy ids 132-135), as Navi AI 49 with ids 0x384-0x387 ---- */

static const XvBlock gutsman_blocks[] = {
	{ 0x080B73C8, 0x080B7734, XN_BN4_CODE + 0x0, 0x4A4BF896u },
	{ 0x080CE6E0, 0x080CE840, XN_BN4_CODE + 0x370, 0x5AA41525u },
	{ 0x080D0A74, 0x080D0BBC, XN_BN4_CODE + 0x4D0, 0x97FC157Cu },
	{ 0x080D055C, 0x080D08BC, XN_BN4_CODE + 0x620, 0x6D3880CFu },
	{ 0x080D08BC, 0x080D0A74, XN_BN4_CODE + 0x980, 0xF86F0AAEu },
	{ 0x080B73AC, 0x080B73C8, XN_BN4_CODE + 0xB40, 0x59294829u },
	{ 0x080CE5B0, 0x080CE6E0, XN_BN4_CODE + 0xB60, 0x69329294u },
	{ 0x0800A244, 0x0800A2BE, XN_BN4_CODE + 0xC90, 0x6300CCC3u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0xD12, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x1122, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x1140, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x1150, 0xDCE85B3Du },
	{ 0x0800F734, 0x0800F780, XN_BN4_CODE + 0x1180, 0x0198FB6Eu },
	{ 0x0800FAD0, 0x0800FAF6, XN_BN4_CODE + 0x11D0, 0x04BFF932u },
	{ 0x080F0434, 0x080F0458, XN_BN4_CODE + 0x1200, 0x37712145u },
	{ 0x080F0458, 0x080F04A8, XN_BN4_CODE + 0x1230, 0x92AF7066u },
	{ 0x080F04A8, 0x080F04E6, XN_BN4_CODE + 0x1280, 0x1C2A869Cu },
	{ 0x080F04E6, 0x080F050A, XN_BN4_CODE + 0x12C2, 0x3B530142u },
	{ 0x080F050A, 0x080F052C, XN_BN4_CODE + 0x12F2, 0x657E64A9u },
	{ 0x080F052C, 0x080F0590, XN_BN4_CODE + 0x1320, 0xF1590379u },
	{ 0x080F0590, 0x080F05BA, XN_BN4_CODE + 0x1390, 0x14A942AFu },
	{ 0x080F05BA, 0x080F05E0, XN_BN4_CODE + 0x13C2, 0x982D80C6u },
	{ 0x080F05E0, 0x080F0606, XN_BN4_CODE + 0x13F0, 0x8FFB36E9u },
	{ 0x080F0606, 0x080F066C, XN_BN4_CODE + 0x1422, 0x66A475DDu },
	{ 0x080F066C, 0x080F069A, XN_BN4_CODE + 0x1490, 0x0961ED8Cu },
	{ 0x080F069A, 0x080F06C0, XN_BN4_CODE + 0x14C2, 0x9DBA2C53u },
	{ 0x080F06C0, 0x080F06F4, XN_BN4_CODE + 0x14F0, 0x56FBA242u },
	{ 0x080F06F4, 0x080F0752, XN_BN4_CODE + 0x1530, 0x6B8D505Eu },
	{ 0x080F0752, 0x080F077C, XN_BN4_CODE + 0x1592, 0x218D443Au },
	{ 0x080F077C, 0x080F0782, XN_BN4_CODE + 0x15C0, 0x4A7133B7u },
	{ 0x080F0782, 0x080F07D0, XN_BN4_CODE + 0x15D2, 0xC4311130u },
	{ 0x080F07D0, 0x080F080C, XN_BN4_CODE + 0x1620, 0x858734D8u },
	{ 0x080F080C, 0x080F086C, XN_BN4_CODE + 0x1660, 0xBE16B1F7u },
};
static const XvData gutsman_data[] = {
	{ 0x080B73C8, 0x080B73DA, XV_ZERO },
	{ 0x080B73DA, 0x080B73DC, XV_RAW },
	{ 0x080B73DC, 0x080B7484, XV_ZERO },
	{ 0x080B749A, 0x080B749C, XV_RAW },
	{ 0x080B749C, 0x080B74A0, XV_POOL },
	{ 0x080B74A0, 0x080B74AC, XV_TABLE },
	{ 0x080B74BA, 0x080B74BC, XV_RAW },
	{ 0x080B74BC, 0x080B74C0, XV_POOL },
	{ 0x080B74C0, 0x080B74D0, XV_TABLE },
	{ 0x080B75A2, 0x080B75A4, XV_RAW },
	{ 0x080B75A4, 0x080B75B4, XV_POOL },
	{ 0x080B75B4, 0x080B75BC, XV_RAW },
	{ 0x080B75EE, 0x080B75F0, XV_RAW },
	{ 0x080B75F0, 0x080B75F4, XV_POOL },
	{ 0x080B761C, 0x080B7624, XV_POOL },
	{ 0x080B7638, 0x080B7656, XV_RAW },
	{ 0x080B7658, 0x080B765C, XV_POOL },
	{ 0x080B766A, 0x080B766C, XV_RAW },
	{ 0x080B766C, 0x080B7670, XV_POOL },
	{ 0x080B7670, 0x080B7678, XV_TABLE },
	{ 0x080B76B8, 0x080B76C0, XV_POOL },
	{ 0x080B76CE, 0x080B76D0, XV_RAW },
	{ 0x080B76D0, 0x080B76D4, XV_POOL },
	{ 0x080B76D4, 0x080B76DC, XV_TABLE },
	{ 0x080B7712, 0x080B7714, XV_RAW },
	{ 0x080B7714, 0x080B7718, XV_POOL },
	{ 0x080B7720, 0x080B7734, XV_POOL },
	{ 0x080CE6F2, 0x080CE6F4, XV_RAW },
	{ 0x080CE6F4, 0x080CE6F8, XV_POOL },
	{ 0x080CE6F8, 0x080CE704, XV_TABLE },
	{ 0x080CE828, 0x080CE83C, XV_RAW },
	{ 0x080CE83C, 0x080CE840, XV_POOL },
	{ 0x080D0A82, 0x080D0A84, XV_RAW },
	{ 0x080D0A84, 0x080D0A88, XV_POOL },
	{ 0x080D0A88, 0x080D0A94, XV_TABLE },
	{ 0x080D0B24, 0x080D0B28, XV_POOL },
	{ 0x080D056A, 0x080D056C, XV_RAW },
	{ 0x080D056C, 0x080D0570, XV_POOL },
	{ 0x080D0570, 0x080D057C, XV_TABLE },
	{ 0x080D0640, 0x080D0644, XV_POOL },
	{ 0x080D0644, 0x080D0654, XV_RAW },
	{ 0x080D07B0, 0x080D07B4, XV_POOL },
	{ 0x080D07B4, 0x080D07C4, XV_RAW },
	{ 0x080D0888, 0x080D088C, XV_POOL },
	{ 0x080D088C, 0x080D0894, XV_RAW },
	{ 0x080D08B4, 0x080D08BC, XV_POOL },
	{ 0x080D08CA, 0x080D08CC, XV_RAW },
	{ 0x080D08CC, 0x080D08D0, XV_POOL },
	{ 0x080D08D0, 0x080D08DC, XV_TABLE },
	{ 0x080D0990, 0x080D0994, XV_POOL },
	{ 0x080D0994, 0x080D0998, XV_TABLE },
	{ 0x080D09E4, 0x080D09E8, XV_POOL },
	{ 0x080D0A1C, 0x080D0A20, XV_POOL },
	{ 0x080D0A20, 0x080D0A40, XV_RAW },
	{ 0x080D0A5E, 0x080D0A74, XV_RAW },
	{ 0x080B73AC, 0x080B73C8, XV_RAW },
	{ 0x080CE5B0, 0x080CE6E0, XV_RAW },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0800F76C, 0x0800F770, XV_POOL },
	{ 0x0800F770, 0x0800F780, XV_RAW },
	{ 0x080F0448, 0x080F044C, XV_POOL },
	{ 0x080F044C, 0x080F0458, XV_TABLE },
	{ 0x080F051E, 0x080F0520, XV_ZERO },
	{ 0x080F0520, 0x080F0524, XV_POOL },
	{ 0x080F0524, 0x080F052C, XV_TABLE },
	{ 0x080F058A, 0x080F058C, XV_ZERO },
	{ 0x080F058C, 0x080F0590, XV_POOL },
	{ 0x080F05CE, 0x080F05D0, XV_ZERO },
	{ 0x080F05D0, 0x080F05D4, XV_POOL },
	{ 0x080F05D4, 0x080F05E0, XV_TABLE },
	{ 0x080F0668, 0x080F066C, XV_POOL },
	{ 0x080F06AE, 0x080F06B0, XV_ZERO },
	{ 0x080F06B0, 0x080F06B4, XV_POOL },
	{ 0x080F06B4, 0x080F06C0, XV_TABLE },
	{ 0x080F07B8, 0x080F07C0, XV_POOL },
	{ 0x080F07C0, 0x080F07D0, XV_RAW },
	{ 0x080F07F4, 0x080F07FC, XV_POOL },
	{ 0x080F07FC, 0x080F080C, XV_RAW },
};
static const XvPair gutsman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08000AAA, 0x08000C72 },   /* project:research, SAME */
	{ 0x08001B00, 0x0800151C },   /* project:gutsman, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009AEC, 0x0800C938 },   /* project:gutsman, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009E98, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009ECC, 0x0800CCA6 },   /* project:gutsman, SAME */
	{ 0x08009ED8, 0x0800CCB2 },   /* map:calls+bytes, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800A1AE, 0x0800D086 },   /* project:research, SAME */
	{ 0x0800AB20, 0x0800E258 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800AB8A, 0x0800E2C2 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AE10, 0x0800E6E8 },   /* project:families-2, SAME */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x0800C456, 0x080103BC },   /* project:gutsman, DIFF (audited) */
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
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08013212, 0x0801A258 },   /* project:gutsman, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0802608C, 0x080302A8 },   /* project:research, SAME */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ 0x080E29F8, 0x080E23C0 },   /* map:bytes, SAME */
};
static const XvImm gutsman_imms[] = {
	{ 0x0800A27A, THUMB_LDRB, 1, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B7488, THUMB_MOVS, 6, 0, 0x64, 0x80 },   /* its AI data's +0x64 in BN4, +0x80 in BN6 (AI_BLOCKS) */
	{ 0x080B748C, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080B7512, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B753E, THUMB_LDRB, 1, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080B759A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080B75E6, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080B7682, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B76B0, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080B76E6, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B770A, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 set (BN4_SET_ACTION's r0) */
	{ 0x080CE74A, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE79A, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE7D6, THUMB_MOVS, 4, 0, 0x56, 0x99 },   /* its effect: 0x56 in BN4, 0x99 in BN6 (moved) */
	{ 0x080CE812, THUMB_MOVS, 0, 0, 0xB, 0xE6 },   /* its object's type 0xb: moved, the dispatch's 0xe6 */
	{ 0x080D0596, THUMB_MOVS, 0, 0, 0xA2, 0x32 },   /* its sound 0xa2: BN4's copied into BN6's empty slot 0x32, on its player 30 */
	{ 0x080D06D2, THUMB_LSLS, 0, 0, 0x2, 0x4 },   /* a side's player read from the battle state: BN4 +0x44 a word each, BN6 +0xD0 0x10 bytes each */
	{ 0x080D06D4, THUMB_ADDS, 0, 0, 0x44, 0xD0 },   /* a side's player read from the battle state: BN4 +0x44 a word each, BN6 +0xD0 0x10 bytes each */
	{ 0x080D06E6, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D072A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D079A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D085A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0874, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0898, THUMB_MOVS, 0, 0, 0x1C, 0xE7 },   /* its object's type 0x1c: moved, the dispatch's 0xe7 */
	{ 0x080D08A8, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D090A, THUMB_MOVS, 2, 0, 0xE, 0xCC },   /* its object's sprite: list 4 0xe in BN4, list 4 0xcc in BN6 (copied) */
	{ 0x080D0968, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D09D4, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0A14, THUMB_MOVS, 0, 0, 0xA3, 0x33 },   /* its sound 0xa3: BN4's copied into BN6's empty slot 0x33, on its player 9 */
	{ 0x080D0A44, THUMB_MOVS, 0, 0, 0x1D, 0xE8 },   /* its object's type 0x1d: moved, the dispatch's 0xe8 */
	{ 0x080D0A54, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0AA0, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080D0AA2, THUMB_MOVS, 2, 0, 0x4, 0xCB },   /* its object's sprite: list 3 0x4 in BN4, list 4 0xcb in BN6 (copied) */
	{ 0x080D0B3C, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D0B5C, THUMB_STRB, 0, 7, 0x8, 0xA },   /* collision data +0x8 in BN4, +0xa in BN6 */
	{ 0x080D0B5E, THUMB_STRB, 1, 7, 0x9, 0xB },   /* collision data +0x9 in BN4, +0xb in BN6 */
	{ 0x080D0B86, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0BA2, THUMB_MOVS, 0, 0, 0x1E, 0xE9 },   /* its object's type 0x1e: moved, the dispatch's 0xe9 */
	{ 0x080D0BB4, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080F0438, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F050E, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F05BE, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F064C, THUMB_MOVS, 0, 0, 0xA4, 0x63 },   /* its sound 0xa4: BN4's copied into BN6's empty slot 0x63, on its player 9 */
	{ 0x080F069E, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F0712, THUMB_MOVS, 0, 0, 0xA7, 0xE0 },   /* its sound 0xa7: BN4's copied into BN6's empty slot 0xe0, on its player 8 */
};
static const XvNumber gutsman_numbers[] = {
	{ 0x080D08B8, 0x03800000, 0x03800000 },   /* a panel's flags it may not have (object_checkPanelParameters' r3, BN6's 0x0800CC86 the same body): 0x0B800000's bits but 0x08000000 (GutsMan's objects, 0x080D07C4) */
	{ 0x080F058C, 0x04050601, 0x04050601 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4), as 0x0405FF01: BN6's own attacks pass the same (five of its pools; GutsMan's states, MegaMan's 0x080F052C and 0x080F0606) */
	{ 0x080F0668, 0x04050601, 0x04050601 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4), as 0x0405FF01: BN6's own attacks pass the same (five of its pools; GutsMan's states, MegaMan's 0x080F052C and 0x080F0606) */
};
static const XvPair gutsman_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvByte gutsman_bytes[] = {
	{ 0x080CE606, 0x15, 0xC8 },   /* its table's kind 5: list 4 0x15 in BN4, 0xc8 copied */
	{ 0x080D0A64, 0xA5, 0xD0 },   /* its table's kind 0: its sound 0xa5, BN4's copied into BN6's slot 0xd0 */
	{ 0x080D0A6A, 0xA6, 0x38 },   /* its table's kind 1: its sound 0xa6, BN4's copied into BN6's slot 0x38 */
	{ 0x080D0A70, 0xA6, 0x38 },   /* its table's kind 2: its sound 0xa6, BN4's copied into BN6's slot 0x38 */
};
static const XvType gutsman_types[] = {
	{ 3, 0xE6, XV_TYPE_MOVED, 0x0B },   /* BN4's third-kind 0xb, moved, the dispatch's 0xe6 */
	{ 3, 0xE7, XV_TYPE_MOVED, 0x1C },   /* BN4's third-kind 0x1c, moved, the dispatch's 0xe7 */
	{ 3, 0xE8, XV_TYPE_MOVED, 0x1D },   /* BN4's third-kind 0x1d, moved, the dispatch's 0xe8 */
	{ 3, 0xE9, XV_TYPE_MOVED, 0x1E },   /* BN4's third-kind 0x1e, moved, the dispatch's 0xe9 */
};
static const XvRef gutsman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080B7485), OWN(0x080F0435), OWN(0x080F050B), OWN(0x080F05BB), OWN(0x080F069B),
};
static const XvSprite gutsman_sprites[] = {
	{ 2, 0x02, 2, 0x41, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x04, 4, 0xCB, XN_BN4_ART + 0x0, 0x1000 },
	{ 4, 0x0E, 4, 0xCC, XN_BN4_ART + 0x1000, 0x800 },
};
static const XvId gutsman_effects[] = { { 0x15, 0x90 }, { 0x56, 0x99 } };
static const XvSong gutsman_songs[] = {
	{ 0xA2, 0x32, XN_BN4_ART + 0x1800, 0x22C0 },   /* its sound 0xa2, BN4's song copied */
	{ 0xA3, 0x33, XN_BN4_ART + 0x3AC0, 0x40 },   /* its sound 0xa3, BN4's song copied */
	{ 0xA4, 0x63, XN_BN4_ART + 0x3B00, 0x80 },   /* its sound 0xa4, BN4's song copied */
	{ 0xA5, 0xD0, XN_BN4_ART + 0x3B80, 0x40 },   /* its sound 0xa5, BN4's song copied */
	{ 0xA7, 0xE0, XN_BN4_ART + 0x3BC0, 0x40 },   /* its sound 0xa7, BN4's song copied */
};


static const XNavi bn4_gutsman = {
	.code = {
		.name = "GutsMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x0A,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 49,
		.first_id = XN_BN4_FIRST_ID + 0x4,
		.home = XN_BN4_HOMES + 0x800,
		.blocks = gutsman_blocks, .nblocks = (int)(sizeof gutsman_blocks / sizeof *gutsman_blocks),
		.data = gutsman_data, .ndata = (int)(sizeof gutsman_data / sizeof *gutsman_data),
		.calls = gutsman_calls, .ncalls = (int)(sizeof gutsman_calls / sizeof *gutsman_calls),
		.words = gutsman_words, .nwords = (int)(sizeof gutsman_words / sizeof *gutsman_words),
		.imms = gutsman_imms, .nimms = (int)(sizeof gutsman_imms / sizeof *gutsman_imms),
		.numbers = gutsman_numbers, .nnumbers = (int)(sizeof gutsman_numbers / sizeof *gutsman_numbers),
		.bytes = gutsman_bytes, .nbytes = (int)(sizeof gutsman_bytes / sizeof *gutsman_bytes),
		.types = gutsman_types, .ntypes = (int)(sizeof gutsman_types / sizeof *gutsman_types),
		.states = gutsman_states, .nstates = (int)(sizeof gutsman_states / sizeof *gutsman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08011EB8, .struct2 = 0x08011EB8,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0x84, 0x85, 0x86, 0x87 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x00 },
		.sprites = gutsman_sprites, .nsprites = (int)(sizeof gutsman_sprites / sizeof *gutsman_sprites),
		.effects = gutsman_effects, .neffects = (int)(sizeof gutsman_effects / sizeof *gutsman_effects),
		.songs = gutsman_songs, .nsongs = (int)(sizeof gutsman_songs / sizeof *gutsman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.coll_arg = 0x21,   /* (its init's collision setup's r3, BN6's actor init's 0x3: the hit bits its body gives, read alike by both games' pre-steps) */
	.palettes = 0x02000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
};
/* ---- BN4's ThunderMan (its type 0x32 there, its enemy ids 148-151), as Navi AI 53 with ids 0x394-0x397 ---- */

static const XvBlock thunderman_blocks[] = {
	{ 0x080C37F4, 0x080C3AF0, XN_BN4_CODE + 0x16C0, 0xA97DE01Fu },
	{ 0x080C3AF0, 0x080C3DA8, XN_BN4_CODE + 0x19C0, 0x2F11438Du },
	{ 0x080DAA90, 0x080DAC54, XN_BN4_CODE + 0x1C80, 0xDB9F1156u },
	{ 0x080DB284, 0x080DB3F8, XN_BN4_CODE + 0x1E50, 0x7250310Du },
	{ 0x080DB3F8, 0x080DB540, XN_BN4_CODE + 0x1FD0, 0xB6618B09u },
	{ 0x080DB540, 0x080DB6FC, XN_BN4_CODE + 0x2120, 0x802661DEu },
	{ 0x0800AD2A, 0x0800B130, XN_BN4_CODE + 0x22E2, 0x2BC6F75Du },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x26F2, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x2710, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x2720, 0xDCE85B3Du },
	{ 0x0801052C, 0x08010560, XN_BN4_CODE + 0x2750, 0x715C82C0u },
	{ 0x080D97F8, 0x080D9808, XN_BN4_CODE + 0x2790, 0x1F57393Cu },
	{ 0x080F20FC, 0x080F2120, XN_BN4_CODE + 0x27A0, 0x12F68C28u },
	{ 0x080F2120, 0x080F214C, XN_BN4_CODE + 0x27D0, 0xE0023745u },
	{ 0x080F214C, 0x080F218C, XN_BN4_CODE + 0x2800, 0xB8A3860Au },
	{ 0x080F218C, 0x080F21B0, XN_BN4_CODE + 0x2840, 0x34BD01BEu },
	{ 0x080F21B0, 0x080F21DC, XN_BN4_CODE + 0x2870, 0xBA609807u },
	{ 0x080F21DC, 0x080F2206, XN_BN4_CODE + 0x28A0, 0x03F68D75u },
	{ 0x080F2206, 0x080F2244, XN_BN4_CODE + 0x28D2, 0xF918512Eu },
	{ 0x080F2244, 0x080F2268, XN_BN4_CODE + 0x2910, 0x7D0735DEu },
	{ 0x080F2268, 0x080F22D8, XN_BN4_CODE + 0x2940, 0x78C7A916u },
	{ 0x080F22D8, 0x080F22FC, XN_BN4_CODE + 0x29B0, 0xACEFDA6Bu },
	{ 0x080F22FC, 0x080F2334, XN_BN4_CODE + 0x29E0, 0x64A4B97Bu },
	{ 0x080F2334, 0x080F235E, XN_BN4_CODE + 0x2A20, 0x7303363Cu },
	{ 0x080F235E, 0x080F2396, XN_BN4_CODE + 0x2A52, 0x937DF0BBu },
	{ 0x080F2396, 0x080F23DC, XN_BN4_CODE + 0x2A92, 0x676E8A3Bu },
	{ 0x080F23DC, 0x080F2406, XN_BN4_CODE + 0x2AE0, 0xF540705Eu },
	{ 0x080F2406, 0x080F242E, XN_BN4_CODE + 0x2B12, 0x4868B58Fu },
};
static const XvData thunderman_data[] = {
	{ 0x080C37F4, 0x080C3806, XV_ZERO },
	{ 0x080C3806, 0x080C3808, XV_RAW },
	{ 0x080C3808, 0x080C3882, XV_ZERO },
	{ 0x080C3882, 0x080C3884, XV_RAW },
	{ 0x080C3884, 0x080C3888, XV_ZERO },
	{ 0x080C3888, 0x080C388C, XV_RAW },
	{ 0x080C388C, 0x080C389E, XV_ZERO },
	{ 0x080C389E, 0x080C38A0, XV_RAW },
	{ 0x080C38A0, 0x080C38CC, XV_ZERO },
	{ 0x080C38CC, 0x080C38D4, XV_RAW },
	{ 0x080C38E6, 0x080C38E8, XV_RAW },
	{ 0x080C38E8, 0x080C38EC, XV_POOL },
	{ 0x080C38EC, 0x080C38F0, XV_TABLE },
	{ 0x080C3A14, 0x080C3A18, XV_POOL },
	{ 0x080C3A18, 0x080C3A30, XV_RAW },
	{ 0x080C3A30, 0x080C3A34, XV_POOL },
	{ 0x080C3A34, 0x080C3A40, XV_RAW },
	{ 0x080C3A40, 0x080C3A44, XV_POOL },
	{ 0x080C3AA2, 0x080C3AA4, XV_RAW },
	{ 0x080C3AA4, 0x080C3AA8, XV_POOL },
	{ 0x080C3AC4, 0x080C3ACC, XV_POOL },
	{ 0x080C3ACC, 0x080C3AD4, XV_RAW },
	{ 0x080C3AD4, 0x080C3AD8, XV_POOL },
	{ 0x080C3AD8, 0x080C3ADC, XV_RAW },
	{ 0x080C3ADC, 0x080C3AE0, XV_POOL },
	{ 0x080C3AE0, 0x080C3AF0, XV_RAW },
	{ 0x080C3B02, 0x080C3B04, XV_RAW },
	{ 0x080C3B04, 0x080C3B08, XV_POOL },
	{ 0x080C3B08, 0x080C3B14, XV_TABLE },
	{ 0x080C3B9C, 0x080C3BA0, XV_POOL },
	{ 0x080C3BA0, 0x080C3BAC, XV_TABLE },
	{ 0x080C3D34, 0x080C3D38, XV_POOL },
	{ 0x080C3D54, 0x080C3D58, XV_POOL },
	{ 0x080C3D58, 0x080C3D70, XV_RAW },
	{ 0x080C3D70, 0x080C3D74, XV_POOL },
	{ 0x080C3D74, 0x080C3D84, XV_RAW },
	{ 0x080C3DA6, 0x080C3DA8, XV_RAW },
	{ 0x080DAAA2, 0x080DAAA4, XV_RAW },
	{ 0x080DAAA4, 0x080DAAA8, XV_POOL },
	{ 0x080DAAA8, 0x080DAAB4, XV_TABLE },
	{ 0x080DAC08, 0x080DAC0C, XV_POOL },
	{ 0x080DAC0C, 0x080DAC1C, XV_RAW },
	{ 0x080DAC1C, 0x080DAC20, XV_POOL },
	{ 0x080DAC20, 0x080DAC24, XV_RAW },
	{ 0x080DB296, 0x080DB298, XV_RAW },
	{ 0x080DB298, 0x080DB29C, XV_POOL },
	{ 0x080DB29C, 0x080DB2A8, XV_TABLE },
	{ 0x080DB388, 0x080DB38C, XV_POOL },
	{ 0x080DB38C, 0x080DB394, XV_TABLE },
	{ 0x080DB3F4, 0x080DB3F8, XV_POOL },
	{ 0x080DB40A, 0x080DB40C, XV_RAW },
	{ 0x080DB40C, 0x080DB410, XV_POOL },
	{ 0x080DB410, 0x080DB41C, XV_TABLE },
	{ 0x080DB492, 0x080DB494, XV_RAW },
	{ 0x080DB494, 0x080DB498, XV_POOL },
	{ 0x080DB498, 0x080DB49C, XV_TABLE },
	{ 0x080DB520, 0x080DB524, XV_POOL },
	{ 0x080DB552, 0x080DB554, XV_RAW },
	{ 0x080DB554, 0x080DB558, XV_POOL },
	{ 0x080DB558, 0x080DB564, XV_TABLE },
	{ 0x080DB614, 0x080DB618, XV_POOL },
	{ 0x080DB618, 0x080DB620, XV_TABLE },
	{ 0x080DB6F8, 0x080DB6FC, XV_POOL },
	{ 0x0800AD7E, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B130, XV_POOL },
	{ 0x080F2110, 0x080F2114, XV_POOL },
	{ 0x080F2114, 0x080F2120, XV_TABLE },
	{ 0x080F21C4, 0x080F21C8, XV_POOL },
	{ 0x080F21C8, 0x080F21DC, XV_TABLE },
	{ 0x080F22D2, 0x080F22D4, XV_ZERO },
	{ 0x080F22D4, 0x080F22D8, XV_POOL },
	{ 0x080F2310, 0x080F2314, XV_POOL },
	{ 0x080F2314, 0x080F2334, XV_TABLE },
};
static const XvPair thunderman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032B8, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007990, 0x0800A10C },   /* project:project:thunderman, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007A8C, XV_FIGHT_FLAGS },   /* the battle's flags, bit 0 from the fight's second frame: his objects no state runs begin with his first own state, a frame into BN6's fight (FIGHT_GATED) */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009BF0, 0x0800CA8C },   /* project:project:thunderman, DIFF (audited) */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009ED8, 0x0800CCB2 },   /* map:calls+bytes, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080124C8, 0x080198CE },   /* map:calls, LAYOUT */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08013212, 0x0801A258 },   /* project:gutsman, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080144FE, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0801466A, 0x0801BE28 },   /* map:calls, DIFF (audited) */
	{ 0x080406A4, 0x0800A03C },   /* map:calls, DIFF (audited) */
	{ 0x08044AEC, 0x080011EC },   /* project:billy, DIFF (audited) */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm thunderman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800AD6A, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD6C, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080C3914, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080C3942, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080C395A, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C3984, THUMB_MOVS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C3A4E, THUMB_MOVS, 0, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C3ABC, THUMB_MOVS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C3D86, THUMB_MOVS, 0, 0, 0x33, 0xE0 },   /* its companion's type 0x33 (the first kind): moved, the dispatch's 0xe0 */
	{ 0x080DAAB8, THUMB_MOVS, 2, 0, 0x2D, 0xCD },   /* its object's sprite: list 4 0x2d in BN4, list 4 0xcd in BN6 (copied) */
	{ 0x080DAAF2, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DAC28, THUMB_MOVS, 0, 0, 0x6A, 0xEA },   /* its object's type 0x6a: moved, the dispatch's 0xea */
	{ 0x080DB2AC, THUMB_MOVS, 2, 0, 0x11, 0xCE },   /* its object's sprite: list 4 0x11 in BN4, list 4 0xce in BN6 (copied) */
	{ 0x080DB2F0, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DB342, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DB39C, THUMB_MOVS, 0, 0, 0x49, 0xD0 },   /* its sound 0x148 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DB39E, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x148 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DB3B0, THUMB_MOVS, 0, 0, 0xB8, 0x33 },   /* its sound 0xb8: BN4's copied into BN6's empty slot 0x33, on its player 17 */
	{ 0x080DB3D2, THUMB_MOVS, 0, 0, 0x6F, 0xEB },   /* its object's type 0x6f: moved, the dispatch's 0xeb */
	{ 0x080DB420, THUMB_MOVS, 2, 0, 0x2E, 0xCF },   /* its object's sprite: list 4 0x2e in BN4, list 4 0xcf in BN6 (copied) */
	{ 0x080DB4A8, THUMB_MOVS, 0, 0, 0xAD, 0x32 },   /* its sound 0xad: BN4's copied into BN6's empty slot 0x32, on its player 26 */
	{ 0x080DB528, THUMB_MOVS, 0, 0, 0x70, 0xEC },   /* its object's type 0x70: moved, the dispatch's 0xec */
	{ 0x080DB568, THUMB_MOVS, 2, 0, 0x14, 0x9B },   /* its object's sprite: list 5 0x14 in BN4, 0x9b in BN6 (copied) */
	{ 0x080DB59C, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DB5E2, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DB672, THUMB_MOVS, 0, 0, 0x44, 0x63 },   /* its sound 0x143 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DB674, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x143 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DB692, THUMB_MOVS, 4, 0, 0x47, 0x9B },   /* its effect: 0x47 in BN4, 0x9b in BN6 (moved) */
	{ 0x080DB6D2, THUMB_MOVS, 0, 0, 0x71, 0xED },   /* its object's type 0x71: moved, the dispatch's 0xed */
	{ 0x080F2100, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F21B4, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F2300, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
};
static const XvNumber thunderman_numbers[] = {
	{ 0x080C3D34, 0x03800000, 0x03800000 },   /* a panel's flags it may not have (object_checkPanelParameters' r3, BN6's 0x0800CC86 the same body): 0x0B800000's bits but 0x08000000 (GutsMan's objects, 0x080D07C4) */
	{ 0x080DB520, 0x04050301, 0x04050301 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4, as 0x04050601 and 0x0405FF01): BN6's own attacks pass the same (its pools at 0x080E4A44 and 0x08101400) */
};
static const XvPair thunderman_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvType thunderman_types[] = {
	{ 3, 0xEA, XV_TYPE_MOVED, 0x6A },   /* BN4's third-kind 0x6a, moved, the dispatch's 0xea */
	{ 3, 0xEB, XV_TYPE_MOVED, 0x6F },   /* BN4's third-kind 0x6f, moved, the dispatch's 0xeb */
	{ 3, 0xEC, XV_TYPE_MOVED, 0x70 },   /* BN4's third-kind 0x70, moved, the dispatch's 0xec */
	{ 3, 0xED, XV_TYPE_MOVED, 0x71 },   /* BN4's third-kind 0x71, moved, the dispatch's 0xed */
	{ 1, 0xE0, XV_TYPE_MOVED, 0x33 },   /* BN4's first-kind 0x33, moved, the dispatch's 0xe0 */
};
static const XvRef thunderman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C38D5), OWN(0x080F20FD), OWN(0x080F21B1), OWN(0x080F22FD),
};
static const XvSprite thunderman_sprites[] = {
	{ 2, 0x06, 2, 0x45, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 4, 0x2D, 4, 0xCD, XN_BN4_ART + 0x0, 0x2000 },
	{ 4, 0x11, 4, 0xCE, XN_BN4_ART + 0x2000, 0x1000 },
	{ 4, 0x2E, 4, 0xCF, XN_BN4_ART + 0x3000, 0x800 },
	{ 5, 0x14, 5, 0x9B, XN_BN4_ART + 0x3800, 0x700 },
	{ 5, 0x13, 5, 0x9C, XN_BN4_ART + 0x3F00, 0xC00 },
};
static const XvId thunderman_effects[] = { { 0x47, 0x9B } };
static const XvSong thunderman_songs[] = {
	{ 0xAD, 0x32, XN_BN4_ART + 0x4B00, 0xB80 },   /* its sound 0xad, BN4's song copied */
	{ 0xB8, 0x33, XN_BN4_ART + 0x5680, 0x840 },   /* its sound 0xb8, BN4's song copied */
	{ 0x143, 0x63, XN_BN4_ART + 0x5EC0, 0x840 },   /* its sound 0x143, BN4's song copied */
	{ 0x148, 0xD0, XN_BN4_ART + 0x6700, 0x840 },   /* its sound 0x148, BN4's song copied */
};
static const XnSpawn thunderman_spawns[] = {
	{ OWN(0x080C3D85), XN_R4_ANY, 0x080C3888, 0x14, 0x00, 0x00, 0x00 },   /* 080C3874: its first-kind 0x33 */
};
static const XnInit thunderman_init = { thunderman_spawns, 1, false, false, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */


static const XNavi bn4_thunderman = {
	.code = {
		.name = "ThunderMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x32,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 53,
		.first_id = XN_BN4_FIRST_ID + 0x14,
		.home = XN_BN4_HOMES + 0x2800,
		.blocks = thunderman_blocks, .nblocks = (int)(sizeof thunderman_blocks / sizeof *thunderman_blocks),
		.data = thunderman_data, .ndata = (int)(sizeof thunderman_data / sizeof *thunderman_data),
		.calls = thunderman_calls, .ncalls = (int)(sizeof thunderman_calls / sizeof *thunderman_calls),
		.words = thunderman_words, .nwords = (int)(sizeof thunderman_words / sizeof *thunderman_words),
		.imms = thunderman_imms, .nimms = (int)(sizeof thunderman_imms / sizeof *thunderman_imms),
		.numbers = thunderman_numbers, .nnumbers = (int)(sizeof thunderman_numbers / sizeof *thunderman_numbers),
		.types = thunderman_types, .ntypes = (int)(sizeof thunderman_types / sizeof *thunderman_types),
		.states = thunderman_states, .nstates = (int)(sizeof thunderman_states / sizeof *thunderman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08011F78, .struct2 = 0x08011F78,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0x94, 0x95, 0x96, 0x97 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x00 },
		.sprites = thunderman_sprites, .nsprites = (int)(sizeof thunderman_sprites / sizeof *thunderman_sprites),
		.effects = thunderman_effects, .neffects = (int)(sizeof thunderman_effects / sizeof *thunderman_effects),
		.songs = thunderman_songs, .nsongs = (int)(sizeof thunderman_songs / sizeof *thunderman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.palettes = 0x04000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &thunderman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
};
/* ---- BN4's WindMan (its type 0x30 there, its enemy ids 136-139), as Navi AI 50 with ids 0x388-0x38b ---- */
/* interim: 080CCDC8: the first kind's 0x5C's state 4, step 0 (WindMan's companions': its sprite drawn again through BN4's tile buffer 0x02037810, a copy of 0x0800087C's and 0x08003578's): its init takes state 4 only where the battle state's +0x32 has no bit 0 (0x08007A8C) and its settings' +6 has 0x20 (0x080CCD1E-0x080CCD2E), else 0 or 8; kept as BN4's bytes, unported (assumed unreached in a plain battle) */

static const XvBlock windman_blocks[] = {
	{ 0x080C3234, 0x080C355C, XN_BN4_CODE + 0x2B40, 0x806DFC0Fu },
	{ 0x080CCC9C, 0x080CCF5C, XN_BN4_CODE + 0x2E70, 0xBEC868D2u },
	{ 0x080DA430, 0x080DA6B4, XN_BN4_CODE + 0x3130, 0x45D32406u },
	{ 0x080D9FA0, 0x080DA220, XN_BN4_CODE + 0x33C0, 0x3A7A5B86u },
	{ 0x080D9C58, 0x080D9DDC, XN_BN4_CODE + 0x3640, 0x7E33F5B9u },
	{ 0x080CCC94, 0x080CCC9C, XN_BN4_CODE + 0x37D0, 0xD258FE18u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0x37E2, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x3BF2, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x3C10, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x3C20, 0xDCE85B3Du },
	{ 0x080F0AF8, 0x080F0B1C, XN_BN4_CODE + 0x3C50, 0x7E2518A1u },
	{ 0x080F0B1C, 0x080F0B6C, XN_BN4_CODE + 0x3C80, 0xA36A16DCu },
	{ 0x080F0B6C, 0x080F0BAA, XN_BN4_CODE + 0x3CD0, 0x2E5D6477u },
	{ 0x080F0BAA, 0x080F0BCE, XN_BN4_CODE + 0x3D12, 0xDD30CAB8u },
	{ 0x080F0BCE, 0x080F0BF4, XN_BN4_CODE + 0x3D42, 0xAC8D5279u },
	{ 0x080F0BF4, 0x080F0C1E, XN_BN4_CODE + 0x3D70, 0x3930D66Cu },
	{ 0x080F0C1E, 0x080F0C80, XN_BN4_CODE + 0x3DA2, 0x63E6371Fu },
	{ 0x080F0C80, 0x080F0C9E, XN_BN4_CODE + 0x3E10, 0xC504D80Cu },
	{ 0x080F0C9E, 0x080F0CC8, XN_BN4_CODE + 0x3E32, 0x97105168u },
	{ 0x080F0CC8, 0x080F0D0A, XN_BN4_CODE + 0x3E60, 0x0E84351Du },
	{ 0x080F0D0A, 0x080F0D56, XN_BN4_CODE + 0x3EB2, 0x8335BA75u },
	{ 0x080F0D56, 0x080F0D9E, XN_BN4_CODE + 0x3F02, 0xA353C10Cu },
	{ 0x080F0D9E, 0x080F0DC4, XN_BN4_CODE + 0x3F52, 0x36CD75B4u },
	{ 0x080F0DC4, 0x080F0DEC, XN_BN4_CODE + 0x3F80, 0x2D75A9CCu },
	{ 0x080F0DEC, 0x080F0E1A, XN_BN4_CODE + 0x3FB0, 0x04380CBFu },
	{ 0x080F0E1A, 0x080F0E28, XN_BN4_CODE + 0x3FE2, 0x17E9C5AAu },
	{ 0x080F0E28, 0x080F0E80, XN_BN4_CODE + 0x3FF0, 0xBDB40846u },
	{ 0x080F0E80, 0x080F0EA6, XN_BN4_CODE + 0x4050, 0x3A0A5E3Cu },
	{ 0x080F0EA6, 0x080F0F1C, XN_BN4_CODE + 0x4082, 0xF0515411u },
	{ 0x080F0F1C, 0x080F0F64, XN_BN4_CODE + 0x4100, 0x1EBAB1B9u },
};
static const XvData windman_data[] = {
	{ 0x080C3234, 0x080C3246, XV_ZERO },
	{ 0x080C3246, 0x080C3248, XV_RAW },
	{ 0x080C3248, 0x080C3320, XV_ZERO },
	{ 0x080C337C, 0x080C3380, XV_POOL },
	{ 0x080C3380, 0x080C3390, XV_TABLE },
	{ 0x080C340C, 0x080C3414, XV_POOL },
	{ 0x080C3414, 0x080C341C, XV_RAW },
	{ 0x080C347A, 0x080C347C, XV_RAW },
	{ 0x080C347C, 0x080C3488, XV_POOL },
	{ 0x080C3488, 0x080C3498, XV_RAW },
	{ 0x080C34DC, 0x080C34E8, XV_POOL },
	{ 0x080C34E8, 0x080C34F4, XV_RAW },
	{ 0x080C3538, 0x080C3540, XV_POOL },
	{ 0x080C3540, 0x080C3548, XV_RAW },
	{ 0x080C3548, 0x080C355C, XV_POOL },
	{ 0x080CCCAA, 0x080CCCAC, XV_RAW },
	{ 0x080CCCAC, 0x080CCCB0, XV_POOL },
	{ 0x080CCCB0, 0x080CCCBC, XV_TABLE },
	{ 0x080CCD38, 0x080CCD3C, XV_POOL },
	{ 0x080CCD88, 0x080CCD8C, XV_POOL },
	{ 0x080CCD8C, 0x080CCD98, XV_TABLE },
	{ 0x080CCDB8, 0x080CCDBC, XV_POOL },
	{ 0x080CCDBC, 0x080CCDC8, XV_TABLE },
	{ 0x080CCDC8, 0x080CCE40, XV_RAW },
	{ 0x080CCEB6, 0x080CCEB8, XV_RAW },
	{ 0x080CCEB8, 0x080CCEBC, XV_POOL },
	{ 0x080CCEBC, 0x080CCECC, XV_TABLE },
	{ 0x080CCF54, 0x080CCF5C, XV_RAW },
	{ 0x080DA442, 0x080DA444, XV_RAW },
	{ 0x080DA444, 0x080DA448, XV_POOL },
	{ 0x080DA448, 0x080DA454, XV_TABLE },
	{ 0x080DA50E, 0x080DA510, XV_RAW },
	{ 0x080DA510, 0x080DA514, XV_POOL },
	{ 0x080DA514, 0x080DA524, XV_TABLE },
	{ 0x080DA6B0, 0x080DA6B4, XV_POOL },
	{ 0x080D9FB8, 0x080D9FBC, XV_POOL },
	{ 0x080D9FBC, 0x080D9FC8, XV_TABLE },
	{ 0x080DA09E, 0x080DA0AA, XV_RAW },
	{ 0x080DA0B4, 0x080DA0BC, XV_POOL },
	{ 0x080DA0BC, 0x080DA0CC, XV_TABLE },
	{ 0x080DA21E, 0x080DA220, XV_RAW },
	{ 0x080D9C70, 0x080D9C74, XV_POOL },
	{ 0x080D9C74, 0x080D9C80, XV_TABLE },
	{ 0x080D9D64, 0x080D9D70, XV_RAW },
	{ 0x080D9D7A, 0x080D9D7C, XV_RAW },
	{ 0x080D9D7C, 0x080D9D84, XV_POOL },
	{ 0x080D9D84, 0x080D9D88, XV_TABLE },
	{ 0x080CCC94, 0x080CCC9C, XV_RAW },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x080F0B0C, 0x080F0B10, XV_POOL },
	{ 0x080F0B10, 0x080F0B1C, XV_TABLE },
	{ 0x080F0BE2, 0x080F0BE4, XV_ZERO },
	{ 0x080F0BE4, 0x080F0BE8, XV_POOL },
	{ 0x080F0BE8, 0x080F0BF4, XV_TABLE },
	{ 0x080F0C7A, 0x080F0C7C, XV_ZERO },
	{ 0x080F0C7C, 0x080F0C80, XV_POOL },
	{ 0x080F0CB2, 0x080F0CB4, XV_ZERO },
	{ 0x080F0CB4, 0x080F0CB8, XV_POOL },
	{ 0x080F0CB8, 0x080F0CC8, XV_TABLE },
	{ 0x080F0DD8, 0x080F0DDC, XV_POOL },
	{ 0x080F0DDC, 0x080F0DEC, XV_TABLE },
	{ 0x080F0EE0, 0x080F0EE8, XV_POOL },
	{ 0x080F0EE8, 0x080F0EF0, XV_POOL },
	{ 0x080F0EF0, 0x080F0F1C, XV_RAW },
	{ 0x080F0F36, 0x080F0F38, XV_ZERO },
	{ 0x080F0F38, 0x080F0F40, XV_POOL },
	{ 0x080F0F40, 0x080F0F48, XV_POOL },
	{ 0x080F0F48, 0x080F0F64, XV_RAW },
};
static const XvPair windman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08000AAA, 0x08000C72 },   /* project:research, SAME */
	{ 0x080019C0, 0x0800139A },   /* map:calls, LAYOUT */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002CEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08002D4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002DF8, 0x08002D8C },   /* map:bytes, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E20, 0x08002DB4 },   /* project:fireman, SAME */
	{ 0x08002E34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002F22, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002F2E, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002F54, 0x08002F02 },   /* project:fireman, SAME */
	{ 0x08002F90, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032B8, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x080078A8, 0x08009FF8 },   /* map:bytes, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007A8C, 0x0800A2F0 },   /* map:bytes, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009E98, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AE10, 0x0800E6E8 },   /* project:families-2, SAME */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x0800C456, 0x080103BC },   /* project:gutsman, DIFF (audited) */
	{ 0x0800F8AC, 0x08015DF0 },   /* map:bytes, SAME */
	{ 0x08010E88, 0x080174FE },   /* BN4's generic Navi state 3, its own state 3's tail: BN6's generic Navi state 3 (flinching), AIMED (audited) */
	{ 0x08010F08, 0x080175B8 },   /* BN4's generic Navi state 4, its own state 4's tail: BN6's generic Navi state 4 (paralysed), AIMED (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ 0x080E68F8, 0x080E666C },   /* project:windman, DIFF (audited) */
	{ 0x080E69BC, 0x080E6750 },   /* map:bytes, SAME */
};
static const XvImm windman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080C335C, THUMB_MOVS, 6, 0, 0x64, 0x80 },   /* its AI data's +0x64 in BN4, +0x80 in BN6 (AI_BLOCKS) */
	{ 0x080C3360, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C3404, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C3430, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C3472, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080C34AC, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C34D4, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080C3508, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C3530, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 set (BN4_SET_ACTION's r0) */
	{ 0x080CCD2E, THUMB_MOVS, 2, 0, 0x4, 0x0 },   /* its companion's mode 4 (its appear through BN4's tile buffer, unported) run as its mode 0 */
	{ 0x080CCF38, THUMB_MOVS, 0, 0, 0x5C, 0xE1 },   /* its companion's type 0x5c (the first kind): moved, the dispatch's 0xe1 */
	{ 0x080D9C84, THUMB_MOVS, 2, 0, 0x2A, 0xD2 },   /* its object's sprite: list 4 0x2a in BN4, list 4 0xd2 in BN6 (copied) */
	{ 0x080D9CC0, THUMB_MOVS, 0, 0, 0xB2, 0x32 },   /* its sound 0xb2: BN4's copied into BN6's empty slot 0x32, on its player 21 */
	{ 0x080D9D1A, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D9D48, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D9D56, THUMB_MOVS, 4, 0, 0x0, 0x9C },   /* its effect: 0x0 in BN4, 0x9c in BN6 (moved) */
	{ 0x080D9D5C, THUMB_MOVS, 0, 0, 0xC8, 0x33 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0x33, on its player 11 */
	{ 0x080D9DB6, THUMB_MOVS, 0, 0, 0x63, 0xEE },   /* its object's type 0x63: moved, the dispatch's 0xee */
	{ 0x080D9DC8, THUMB_STRB, 2, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D9FCC, THUMB_MOVS, 2, 0, 0x2B, 0xD3 },   /* its object's sprite: list 4 0x2b in BN4, list 4 0xd3 in BN6 (copied) */
	{ 0x080DA054, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DA082, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080DA090, THUMB_MOVS, 4, 0, 0x0, 0x9C },   /* its effect: 0x0 in BN4, 0x9c in BN6 (moved) */
	{ 0x080DA096, THUMB_MOVS, 0, 0, 0xC8, 0x33 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0x33, on its player 11 */
	{ 0x080DA0D4, THUMB_MOVS, 0, 0, 0x34, 0xD0 },   /* its sound 0x133 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DA0D6, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x133 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DA1E6, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080DA1FA, THUMB_MOVS, 0, 0, 0x65, 0xEF },   /* its object's type 0x65: moved, the dispatch's 0xef */
	{ 0x080DA20A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DA46A, THUMB_MOVS, 2, 0, 0x2F, 0xD4 },   /* its object's sprite: list 4 0x2f in BN4, list 4 0xd4 in BN6 (copied) */
	{ 0x080DA4A4, THUMB_MOVS, 0, 0, 0x27, 0x63 },   /* its sound 0x126 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DA4A6, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x126 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DA4E6, THUMB_LDR, 0, 2, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DA5E0, THUMB_LDRB, 0, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080DA694, THUMB_MOVS, 0, 0, 0x67, 0xF0 },   /* its object's type 0x67: moved, the dispatch's 0xf0 */
	{ 0x080DA6A0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080F0AFC, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F0BD2, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F0CA2, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F0DC8, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
};
static const XvPair windman_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvByte windman_bytes[] = {
	{ 0x080CCC99, 0x28, 0xD0 },   /* its table's kind 2: list 4 0x28 in BN4, 0xd0 copied */
	{ 0x080CCC9B, 0x29, 0xD1 },   /* its table's kind 3: list 4 0x29 in BN4, 0xd1 copied */
};
static const XvType windman_types[] = {
	{ 3, 0xEE, XV_TYPE_MOVED, 0x63 },   /* BN4's third-kind 0x63, moved, the dispatch's 0xee */
	{ 3, 0xEF, XV_TYPE_MOVED, 0x65 },   /* BN4's third-kind 0x65, moved, the dispatch's 0xef */
	{ 3, 0xF0, XV_TYPE_MOVED, 0x67 },   /* BN4's third-kind 0x67, moved, the dispatch's 0xf0 */
	{ 1, 0xE1, XV_TYPE_MOVED, 0x5C },   /* BN4's first-kind 0x5c, moved, the dispatch's 0xe1 */
};
static const XvRef windman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), OWN(0x080C3321), OWN(0x080C333D), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C3359), OWN(0x080F0AF9), OWN(0x080F0BCF), OWN(0x080F0C9F), OWN(0x080F0DC5),
};
static const XvSprite windman_sprites[] = {
	{ 2, 0x03, 2, 0x42, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 4, 0x28, 4, 0xD0, XN_BN4_ART + 0x0, 0x800 },
	{ 4, 0x29, 4, 0xD1, XN_BN4_ART + 0x800, 0x800 },
	{ 4, 0x2A, 4, 0xD2, XN_BN4_ART + 0x1000, 0x800 },
	{ 4, 0x2B, 4, 0xD3, XN_BN4_ART + 0x1800, 0x800 },
	{ 4, 0x2F, 4, 0xD4, XN_BN4_ART + 0x2000, 0x1000 },
	{ 5, 0x00, 5, 0x9D, XN_BN4_ART + 0x3000, 0x1C00 },
};
static const XvId windman_effects[] = { { 0x00, 0x9C }, { 0x15, 0x90 } };
static const XvSong windman_songs[] = {
	{ 0xB2, 0x32, XN_BN4_ART + 0x4C00, 0x1640 },   /* its sound 0xb2, BN4's song copied */
	{ 0xC8, 0x33, XN_BN4_ART + 0x6240, 0xB40 },   /* its sound 0xc8, BN4's song copied */
	{ 0x126, 0x63, XN_BN4_ART + 0x6D80, 0x1F00 },   /* its sound 0x126, BN4's song copied */
	{ 0x133, 0xD0, XN_BN4_ART + 0x8C80, 0x16C0 },   /* its sound 0x133, BN4's song copied */
};
static const XnSpawn windman_spawns[] = {
	{ OWN(0x080CCF35), 0x2, 0x00000000, 0x00, 0x60, 0x60, 0x00 },   /* 080C3292: its first-kind 0x5c */
	{ OWN(0x080CCF35), 0x3, 0x00000000, 0x00, 0x60, 0x60, 0x64 },   /* 080C32A2: its first-kind 0x5c */
};
static const XnInit windman_init = { windman_spawns, 2, true, false, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */


static const XNavi bn4_windman = {
	.code = {
		.name = "WindMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x30,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 50,
		.first_id = XN_BN4_FIRST_ID + 0x8,
		.home = XN_BN4_HOMES + 0x1000,
		.blocks = windman_blocks, .nblocks = (int)(sizeof windman_blocks / sizeof *windman_blocks),
		.data = windman_data, .ndata = (int)(sizeof windman_data / sizeof *windman_data),
		.calls = windman_calls, .ncalls = (int)(sizeof windman_calls / sizeof *windman_calls),
		.words = windman_words, .nwords = (int)(sizeof windman_words / sizeof *windman_words),
		.imms = windman_imms, .nimms = (int)(sizeof windman_imms / sizeof *windman_imms),
		.bytes = windman_bytes, .nbytes = (int)(sizeof windman_bytes / sizeof *windman_bytes),
		.types = windman_types, .ntypes = (int)(sizeof windman_types / sizeof *windman_types),
		.states = windman_states, .nstates = (int)(sizeof windman_states / sizeof *windman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08011EE8, .struct2 = 0x08011EE8,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0x88, 0x89, 0x8A, 0x8B },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x06 },
		.sprites = windman_sprites, .nsprites = (int)(sizeof windman_sprites / sizeof *windman_sprites),
		.effects = windman_effects, .neffects = (int)(sizeof windman_effects / sizeof *windman_effects),
		.songs = windman_songs, .nsongs = (int)(sizeof windman_songs / sizeof *windman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.palettes = 0x04000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &windman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
};
/* ---- BN4's FireMan (its type 0xc there, its enemy ids 144-147), as Navi AI 52 with ids 0x390-0x393 ---- */
/* interim: its sound 0x8f plays 1 of its 2 tracks: BN6's player 30 has 1 where BN4's has 2 (BN6_MPLAYS against BN4_MPLAYS), the rest let go as BN6's MPlayStart starts no more tracks than its player has */

static const XvBlock fireman_blocks[] = {
	{ 0x080B7DB4, 0x080B820C, XN_BN4_CODE + 0x4150, 0x6A76CB41u },
	{ 0x080CC3D8, 0x080CC624, XN_BN4_CODE + 0x45B0, 0xCFB85A08u },
	{ 0x080D0FFC, 0x080D10D0, XN_BN4_CODE + 0x4800, 0xD14BD3EDu },
	{ 0x080CFF80, 0x080D014E, XN_BN4_CODE + 0x48E0, 0x12544EF6u },
	{ 0x080D014E, 0x080D02F8, XN_BN4_CODE + 0x4AB2, 0xEED2A840u },
	{ 0x080CE2D8, 0x080CE4D0, XN_BN4_CODE + 0x4C60, 0x00B1870Eu },
	{ 0x080D0BBC, 0x080D0D64, XN_BN4_CODE + 0x4E60, 0x8906650Cu },
	{ 0x080CE4D0, 0x080CE6E0, XN_BN4_CODE + 0x5010, 0x842E9CE2u },
	{ 0x080B7D98, 0x080B7DB4, XN_BN4_CODE + 0x5220, 0xA9730ED9u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0x5242, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x5652, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x5670, 0xA9D6138Du },
	{ 0x0800C9FC, 0x0800CA28, XN_BN4_CODE + 0x5680, 0x3A5341C2u },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x56B0, 0xDCE85B3Du },
	{ 0x0800F734, 0x0800F780, XN_BN4_CODE + 0x56E0, 0x0198FB6Eu },
	{ 0x08014446, 0x0801447A, XN_BN4_CODE + 0x5732, 0x6AADAE25u },
	{ 0x080EDD2E, 0x080EDD54, XN_BN4_CODE + 0x5772, 0xE0FEDFDBu },
	{ 0x080EDD54, 0x080EDDA4, XN_BN4_CODE + 0x57A0, 0x044A19FDu },
	{ 0x080EDDA4, 0x080EDDEA, XN_BN4_CODE + 0x57F0, 0xB569EFE0u },
	{ 0x080EDDEA, 0x080EDE0E, XN_BN4_CODE + 0x5842, 0x8D466920u },
	{ 0x080F077C, 0x080F0782, XN_BN4_CODE + 0x5870, 0x4A7133B7u },
	{ 0x080F1810, 0x080F1834, XN_BN4_CODE + 0x5880, 0x8E543822u },
	{ 0x080F1834, 0x080F1848, XN_BN4_CODE + 0x58B0, 0xE4B8125Au },
	{ 0x080F1848, 0x080F18CA, XN_BN4_CODE + 0x58D0, 0xDD103622u },
	{ 0x080F18CA, 0x080F18F4, XN_BN4_CODE + 0x5962, 0x47D27B46u },
	{ 0x080F18F4, 0x080F1918, XN_BN4_CODE + 0x5990, 0x1E8EC382u },
	{ 0x080F1918, 0x080F1948, XN_BN4_CODE + 0x59C0, 0x61728EEAu },
	{ 0x080F1948, 0x080F19A8, XN_BN4_CODE + 0x59F0, 0x40750D73u },
	{ 0x080F19A8, 0x080F19D6, XN_BN4_CODE + 0x5A50, 0xE1F51CE0u },
	{ 0x080F19D6, 0x080F19FC, XN_BN4_CODE + 0x5A82, 0xA6CAAB71u },
	{ 0x080F19FC, 0x080F1A3A, XN_BN4_CODE + 0x5AB0, 0x7C0EA509u },
	{ 0x080F1A3A, 0x080F1A82, XN_BN4_CODE + 0x5AF2, 0xC0E4B18Bu },
	{ 0x080F1A82, 0x080F1AAC, XN_BN4_CODE + 0x5B42, 0x1725092Fu },
	{ 0x080F1AAC, 0x080F1AD0, XN_BN4_CODE + 0x5B70, 0xA812514Bu },
	{ 0x080F1AD0, 0x080F1AF6, XN_BN4_CODE + 0x5BA0, 0xF7DA1FF7u },
	{ 0x080F1AF6, 0x080F1B3E, XN_BN4_CODE + 0x5BD2, 0xC32FB049u },
	{ 0x080F1B3E, 0x080F1B68, XN_BN4_CODE + 0x5C22, 0x33C1CD7Du },
	{ 0x080F1B68, 0x080F1B70, XN_BN4_CODE + 0x5C50, 0xF4AF7FC1u },
	{ 0x080F1B70, 0x080F1C44, XN_BN4_CODE + 0x5C60, 0x182E2204u },
	{ 0x080F1C44, 0x080F1C50, XN_BN4_CODE + 0x5D40, 0x72B92A5Fu },
	{ 0x080F1C50, 0x080F1C60, XN_BN4_CODE + 0x5D50, 0x0AE0F513u },
	{ 0x080F1C60, 0x080F1CB2, XN_BN4_CODE + 0x5D60, 0xAD6236D8u },
	{ 0x080F1CB2, 0x080F1D4C, XN_BN4_CODE + 0x5DC2, 0x753C82C3u },
	{ 0x080F1D4C, 0x080F1D98, XN_BN4_CODE + 0x5E60, 0x7FE65FDEu },
	{ 0x080F1D98, 0x080F1DC4, XN_BN4_CODE + 0x5EB0, 0xB3CAF049u },
};
static const XvData fireman_data[] = {
	{ 0x080B7DB4, 0x080B7DC2, XV_ZERO },
	{ 0x080B7DC2, 0x080B7DC4, XV_RAW },
	{ 0x080B7DC4, 0x080B7E8C, XV_ZERO },
	{ 0x080B7EDE, 0x080B7EE0, XV_RAW },
	{ 0x080B7EE0, 0x080B7EE4, XV_POOL },
	{ 0x080B7EE4, 0x080B7EF8, XV_TABLE },
	{ 0x080B7FBA, 0x080B7FBC, XV_RAW },
	{ 0x080B7FBC, 0x080B7FC0, XV_POOL },
	{ 0x080B7FCE, 0x080B7FD0, XV_RAW },
	{ 0x080B7FD0, 0x080B7FD4, XV_POOL },
	{ 0x080B7FD4, 0x080B7FDC, XV_TABLE },
	{ 0x080B800C, 0x080B8010, XV_POOL },
	{ 0x080B8030, 0x080B8038, XV_POOL },
	{ 0x080B8046, 0x080B8048, XV_RAW },
	{ 0x080B8048, 0x080B804C, XV_POOL },
	{ 0x080B804C, 0x080B8054, XV_TABLE },
	{ 0x080B8084, 0x080B8088, XV_POOL },
	{ 0x080B80AE, 0x080B80B0, XV_RAW },
	{ 0x080B80B0, 0x080B80BC, XV_POOL },
	{ 0x080B80CA, 0x080B80CC, XV_RAW },
	{ 0x080B80CC, 0x080B80D0, XV_POOL },
	{ 0x080B80D0, 0x080B80D8, XV_TABLE },
	{ 0x080B8106, 0x080B8108, XV_RAW },
	{ 0x080B8108, 0x080B810C, XV_POOL },
	{ 0x080B8128, 0x080B812C, XV_POOL },
	{ 0x080B813A, 0x080B813C, XV_RAW },
	{ 0x080B813C, 0x080B8140, XV_POOL },
	{ 0x080B8140, 0x080B8148, XV_TABLE },
	{ 0x080B8178, 0x080B817C, XV_POOL },
	{ 0x080B819C, 0x080B81A0, XV_POOL },
	{ 0x080B81A8, 0x080B81AC, XV_POOL },
	{ 0x080B81AC, 0x080B81B0, XV_RAW },
	{ 0x080B81C2, 0x080B81C4, XV_RAW },
	{ 0x080B81C4, 0x080B81F8, XV_POOL },
	{ 0x080B81F8, 0x080B820C, XV_RAW },
	{ 0x080CC3E6, 0x080CC3E8, XV_RAW },
	{ 0x080CC3E8, 0x080CC3EC, XV_POOL },
	{ 0x080CC3EC, 0x080CC3F8, XV_TABLE },
	{ 0x080CC4C2, 0x080CC4C4, XV_RAW },
	{ 0x080CC4C4, 0x080CC4C8, XV_POOL },
	{ 0x080CC4C8, 0x080CC4D4, XV_TABLE },
	{ 0x080CC4FC, 0x080CC500, XV_POOL },
	{ 0x080CC500, 0x080CC50C, XV_POOL },
	{ 0x080CC50C, 0x080CC5B4, XV_RAW },
	{ 0x080CC614, 0x080CC61A, XV_RAW },
	{ 0x080D100A, 0x080D100C, XV_RAW },
	{ 0x080D100C, 0x080D1010, XV_POOL },
	{ 0x080D1010, 0x080D101C, XV_TABLE },
	{ 0x080D109C, 0x080D10A0, XV_POOL },
	{ 0x080D10A0, 0x080D10B0, XV_RAW },
	{ 0x080CFF92, 0x080CFF94, XV_RAW },
	{ 0x080CFF94, 0x080CFF98, XV_POOL },
	{ 0x080CFF98, 0x080CFFA4, XV_TABLE },
	{ 0x080D002E, 0x080D0030, XV_RAW },
	{ 0x080D0030, 0x080D0034, XV_POOL },
	{ 0x080D0034, 0x080D0040, XV_TABLE },
	{ 0x080D00D8, 0x080D014E, XV_RAW },
	{ 0x080D0160, 0x080D0164, XV_POOL },
	{ 0x080D0164, 0x080D0170, XV_TABLE },
	{ 0x080D022E, 0x080D0230, XV_RAW },
	{ 0x080D0230, 0x080D0234, XV_POOL },
	{ 0x080D026C, 0x080D0270, XV_POOL },
	{ 0x080D0270, 0x080D0278, XV_TABLE },
	{ 0x080CE2F6, 0x080CE2F8, XV_RAW },
	{ 0x080CE2F8, 0x080CE2FC, XV_POOL },
	{ 0x080CE2FC, 0x080CE308, XV_TABLE },
	{ 0x080CE440, 0x080CE444, XV_POOL },
	{ 0x080CE444, 0x080CE44C, XV_TABLE },
	{ 0x080D0BCE, 0x080D0BD0, XV_RAW },
	{ 0x080D0BD0, 0x080D0BD4, XV_POOL },
	{ 0x080D0BD4, 0x080D0BE0, XV_TABLE },
	{ 0x080D0C90, 0x080D0C94, XV_POOL },
	{ 0x080D0C94, 0x080D0C9C, XV_TABLE },
	{ 0x080D0D5E, 0x080D0D60, XV_RAW },
	{ 0x080D0D60, 0x080D0D64, XV_POOL },
	{ 0x080CE4E2, 0x080CE4E4, XV_RAW },
	{ 0x080CE4E4, 0x080CE4E8, XV_POOL },
	{ 0x080CE4E8, 0x080CE4F4, XV_TABLE },
	{ 0x080CE5AE, 0x080CE6E0, XV_RAW },
	{ 0x080B7D98, 0x080B7DB4, XV_RAW },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0800F76C, 0x0800F770, XV_POOL },
	{ 0x0800F770, 0x0800F780, XV_RAW },
	{ 0x080EDD42, 0x080EDD44, XV_ZERO },
	{ 0x080EDD44, 0x080EDD48, XV_POOL },
	{ 0x080EDD48, 0x080EDD54, XV_TABLE },
	{ 0x080F1824, 0x080F1828, XV_POOL },
	{ 0x080F1828, 0x080F1834, XV_TABLE },
	{ 0x080F1908, 0x080F190C, XV_POOL },
	{ 0x080F190C, 0x080F1918, XV_TABLE },
	{ 0x080F19A2, 0x080F19A4, XV_ZERO },
	{ 0x080F19A4, 0x080F19A8, XV_POOL },
	{ 0x080F19EA, 0x080F19EC, XV_ZERO },
	{ 0x080F19EC, 0x080F19F0, XV_POOL },
	{ 0x080F19F0, 0x080F19FC, XV_TABLE },
	{ 0x080F1AC0, 0x080F1AC4, XV_POOL },
	{ 0x080F1AC4, 0x080F1AD0, XV_TABLE },
	{ 0x080F1C1A, 0x080F1C1C, XV_ZERO },
	{ 0x080F1C1C, 0x080F1C24, XV_POOL },
	{ 0x080F1C24, 0x080F1C44, XV_RAW },
	{ 0x080F1D38, 0x080F1D3C, XV_POOL },
	{ 0x080F1D3C, 0x080F1D4C, XV_RAW },
	{ 0x080F1D84, 0x080F1D88, XV_POOL },
	{ 0x080F1D88, 0x080F1D98, XV_RAW },
	{ 0x080F1DAE, 0x080F1DB0, XV_ZERO },
	{ 0x080F1DB0, 0x080F1DB4, XV_POOL },
	{ 0x080F1DB4, 0x080F1DC4, XV_RAW },
};
static const XvPair fireman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001956, 0x08001330 },   /* map:bytes, SAME */
	{ 0x080019C0, 0x0800139A },   /* map:calls, LAYOUT */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002CEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08002D4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002DF8, 0x08002D8C },   /* map:bytes, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E20, 0x08002DB4 },   /* project:fireman, SAME */
	{ 0x08002E34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002F22, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002F2E, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002F54, 0x08002F02 },   /* project:fireman, SAME */
	{ 0x08002F90, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032B8, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x080078EC, 0x0800A03C },   /* project:fireman, DIFF (audited) */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007A8C, 0x0800A2F0 },   /* map:bytes, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009DC4, 0x0800CC0A },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009ECC, 0x0800CCA6 },   /* project:gutsman, SAME */
	{ 0x08009ED8, 0x0800CCB2 },   /* map:calls+bytes, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800A0D8, 0x0800CFDC },   /* project:research, SAME */
	{ 0x0800A10E, 0x0800D012 },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800A1AE, 0x0800D086 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x08010E88, 0x080174FE },   /* BN4's generic Navi state 3, its own state 3's tail: BN6's generic Navi state 3 (flinching), AIMED (audited) */
	{ 0x08010F08, 0x080175B8 },   /* BN4's generic Navi state 4, its own state 4's tail: BN6's generic Navi state 4 (paralysed), AIMED (audited) */
	{ 0x08010FAE, 0x080178B6 },   /* BN4's generic Navi state 5, its own state 5's tail: BN6's generic Navi state 5 (dragged), AIMED (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x08012886, 0x08019F86 },   /* project:fireman, LAYOUT */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F7A, 0x0801A146 },   /* project:fireman, DIFF (audited) */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm fireman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA06, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA1E, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B7ECC, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080B7ED0, THUMB_ADDS, 6, 0, 0x64, 0x80 },   /* its AI data's +0x64 in BN4, +0x80 in BN6 (AI_BLOCKS) */
	{ 0x080B7F40, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B8004, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B8028, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080B807C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B80A6, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 set (BN4_SET_ACTION's r0) */
	{ 0x080B80F8, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B8120, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 set (BN4_SET_ACTION's r0) */
	{ 0x080B8170, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B8194, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080CC464, THUMB_MOVS, 2, 0, 0x4, 0x0 },   /* its companion's mode 4 (its appear through BN4's tile buffer, unported) run as its mode 0 */
	{ 0x080CC5F8, THUMB_MOVS, 0, 0, 0x57, 0xE2 },   /* its companion's type 0x57 (the first kind): moved, the dispatch's 0xe2 */
	{ 0x080CE31C, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080CE31E, THUMB_MOVS, 2, 0, 0xE, 0xD5 },   /* its object's sprite: list 3 0xe in BN4, list 4 0xd5 in BN6 (copied) */
	{ 0x080CE358, THUMB_LDRB, 0, 5, 0xE, 0xF },   /* its palette: BN4 +0x0E, BN6 +0x0F (its +0x0E the element) */
	{ 0x080CE3D2, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE47E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE486, THUMB_LDRB, 7, 5, 0xE, 0xF },   /* its palette: BN4 +0x0E, BN6 +0x0F (its +0x0E the element) */
	{ 0x080CE4A0, THUMB_MOVS, 0, 0, 0x9, 0xF1 },   /* its object's type 0x9: moved, the dispatch's 0xf1 */
	{ 0x080CE4B0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE4BC, THUMB_STRB, 7, 0, 0xE, 0xF },   /* the object it spawned: palette: BN4 +0x0E, BN6 +0x0F (its +0x0E the element) */
	{ 0x080CE4F6, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080CE4F8, THUMB_MOVS, 2, 0, 0x11, 0xD6 },   /* its object's sprite: list 3 0x11 in BN4, list 4 0xd6 in BN6 (copied) */
	{ 0x080CE544, THUMB_MOVS, 0, 0, 0xA9, 0x63 },   /* its sound 0xa9: BN4's copied into BN6's empty slot 0x63, on its player 9 */
	{ 0x080CE570, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE592, THUMB_MOVS, 0, 0, 0xA, 0xF2 },   /* its object's type 0xa: moved, the dispatch's 0xf2 */
	{ 0x080CE5A2, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CFFAE, THUMB_MOVS, 2, 0, 0xA, 0xD8 },   /* its object's sprite: list 4 0xa in BN4, list 4 0xd8 in BN6 (copied) */
	{ 0x080CFFF8, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D0014, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D00BC, THUMB_MOVS, 0, 0, 0x19, 0xF3 },   /* its object's type 0x19: moved, the dispatch's 0xf3 */
	{ 0x080D00CC, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D01C2, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D021C, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D0248, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D02DE, THUMB_MOVS, 0, 0, 0x1A, 0xF4 },   /* its object's type 0x1a: moved, the dispatch's 0xf4 */
	{ 0x080D0BE2, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080D0BE4, THUMB_MOVS, 2, 0, 0x2, 0xD7 },   /* its object's sprite: list 3 0x2 in BN4, list 4 0xd7 in BN6 (copied) */
	{ 0x080D0C64, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D0C80, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0CE0, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0D2E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0D46, THUMB_MOVS, 0, 0, 0x1F, 0xF5 },   /* its object's type 0x1f: moved, the dispatch's 0xf5 */
	{ 0x080D0D58, THUMB_STRB, 1, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D1036, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D107E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D10B4, THUMB_MOVS, 0, 0, 0x21, 0xF6 },   /* its object's type 0x21: moved, the dispatch's 0xf6 */
	{ 0x080D10C4, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EDD32, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F1814, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F18AA, THUMB_MOVS, 0, 0, 0x8F, 0x32 },   /* its sound 0x8f: BN4's copied into BN6's empty slot 0x32, on its player 30 */
	{ 0x080F18F8, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F198E, THUMB_MOVS, 0, 0, 0xA8, 0x33 },   /* its sound 0xa8: BN4's copied into BN6's empty slot 0x33, on its player 9 */
	{ 0x080F19CC, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080F19DA, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F1AB0, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F1B02, THUMB_MOVS, 0, 0, 0xC0, 0xD0 },   /* its sound 0xc0: BN4's copied into BN6's empty slot 0xd0, on its player 27 */
};
static const XvPair fireman_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvType fireman_types[] = {
	{ 3, 0xF1, XV_TYPE_MOVED, 0x09 },   /* BN4's third-kind 0x9, moved, the dispatch's 0xf1 */
	{ 3, 0xF2, XV_TYPE_MOVED, 0x0A },   /* BN4's third-kind 0xa, moved, the dispatch's 0xf2 */
	{ 3, 0xF3, XV_TYPE_MOVED, 0x19 },   /* BN4's third-kind 0x19, moved, the dispatch's 0xf3 */
	{ 3, 0xF4, XV_TYPE_MOVED, 0x1A },   /* BN4's third-kind 0x1a, moved, the dispatch's 0xf4 */
	{ 3, 0xF5, XV_TYPE_MOVED, 0x1F },   /* BN4's third-kind 0x1f, moved, the dispatch's 0xf5 */
	{ 3, 0xF6, XV_TYPE_MOVED, 0x21 },   /* BN4's third-kind 0x21, moved, the dispatch's 0xf6 */
	{ 1, 0xE2, XV_TYPE_MOVED, 0x57 },   /* BN4's first-kind 0x57, moved, the dispatch's 0xe2 */
};
static const XvRef fireman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), OWN(0x080B7E8D), OWN(0x080B7EA1), OWN(0x080B7EB5), BN6(0x08017689), BN6(0x08017769), OWN(0x080B7EC9), OWN(0x080EDD2F), OWN(0x080F1811), OWN(0x080F18F5), OWN(0x080F19D7), OWN(0x080F1AAD),
};
static const XvSprite fireman_sprites[] = {
	{ 2, 0x05, 2, 0x44, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x0E, 4, 0xD5, XN_BN4_ART + 0x0, 0x1000 },
	{ 3, 0x11, 4, 0xD6, XN_BN4_ART + 0x1000, 0x1000 },
	{ 3, 0x02, 4, 0xD7, XN_BN4_ART + 0x2000, 0x1000 },
	{ 4, 0x0A, 4, 0xD8, XN_BN4_ART + 0x3000, 0x2000 },
	{ 4, 0x08, 4, 0xD9, XN_BN4_ART + 0x5000, 0x1800 },
};
static const XvId fireman_effects[] = { { 0x15, 0x90 } };
static const XvSong fireman_songs[] = {
	{ 0x8F, 0x32, XN_BN4_ART + 0x6800, 0x80 },   /* its sound 0x8f, BN4's song copied */
	{ 0xA8, 0x33, XN_BN4_ART + 0x6880, 0x40 },   /* its sound 0xa8, BN4's song copied */
	{ 0xA9, 0x63, XN_BN4_ART + 0x68C0, 0x40 },   /* its sound 0xa9, BN4's song copied */
	{ 0xC0, 0xD0, XN_BN4_ART + 0x6900, 0x880 },   /* its sound 0xc0, BN4's song copied */
};
static const XnSpawn fireman_spawns[] = {
	{ OWN(0x080CC5F5), 0xD910, 0x00000000, 0x00, 0x00, 0x00, 0x74 },   /* 080B7E16: its first-kind 0x57, BN4 r4 0x810 */
};
static const XnInit fireman_init = { fireman_spawns, 1, true, true, true, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */


static const XNavi bn4_fireman = {
	.code = {
		.name = "FireMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x0C,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 52,
		.first_id = XN_BN4_FIRST_ID + 0x10,
		.home = XN_BN4_HOMES + 0x2000,
		.blocks = fireman_blocks, .nblocks = (int)(sizeof fireman_blocks / sizeof *fireman_blocks),
		.data = fireman_data, .ndata = (int)(sizeof fireman_data / sizeof *fireman_data),
		.calls = fireman_calls, .ncalls = (int)(sizeof fireman_calls / sizeof *fireman_calls),
		.words = fireman_words, .nwords = (int)(sizeof fireman_words / sizeof *fireman_words),
		.imms = fireman_imms, .nimms = (int)(sizeof fireman_imms / sizeof *fireman_imms),
		.types = fireman_types, .ntypes = (int)(sizeof fireman_types / sizeof *fireman_types),
		.states = fireman_states, .nstates = (int)(sizeof fireman_states / sizeof *fireman_states),
		.hooks = { OWN(0x080B81B1), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },   /* (its post hook its update's call after its state runner) */
		.struct1 = 0x08011F48, .struct2 = 0x08011F48,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0x90, 0x91, 0x92, 0x93 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x00 },
		.sprites = fireman_sprites, .nsprites = (int)(sizeof fireman_sprites / sizeof *fireman_sprites),
		.effects = fireman_effects, .neffects = (int)(sizeof fireman_effects / sizeof *fireman_effects),
		.songs = fireman_songs, .nsongs = (int)(sizeof fireman_songs / sizeof *fireman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.coll_arg = 0x21,   /* (its init's collision setup's r3, BN6's actor init's 0x3: the hit bits its body gives, read alike by both games' pre-steps) */
	.palettes = 0x02000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &fireman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
};
/* ---- BN4's SearchMan (its type 0x2b there, its enemy ids 140-143), as Navi AI 51 with ids 0x38c-0x38f ---- */

static const XvBlock searchman_blocks[] = {
	{ 0x080C11F8, 0x080C151C, XN_BN4_CODE + 0x5EE0, 0xCC5717E1u },
	{ 0x080D87B0, 0x080D8878, XN_BN4_CODE + 0x6210, 0xEE768607u },
	{ 0x080D7B64, 0x080D7D94, XN_BN4_CODE + 0x62E0, 0x71045A30u },
	{ 0x080E5588, 0x080E5740, XN_BN4_CODE + 0x6510, 0x008C5168u },
	{ 0x080E589C, 0x080E5B50, XN_BN4_CODE + 0x66D0, 0xE26AEFB0u },
	{ 0x080D95CC, 0x080D9828, XN_BN4_CODE + 0x6990, 0x80642A2Du },
	{ 0x080E64FC, 0x080E65E0, XN_BN4_CODE + 0x6BF0, 0x26BA0CAAu },
	{ 0x0800A2E4, 0x0800A34E, XN_BN4_CODE + 0x6CE0, 0x2EEEEC12u },
	{ 0x0800AD2A, 0x0800B130, XN_BN4_CODE + 0x6D52, 0x2BC6F75Du },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x7162, 0xFBBA31FBu },
	{ 0x0800C0A6, 0x0800C0DA, XN_BN4_CODE + 0x7182, 0xC1233806u },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x71C0, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x71D0, 0xDCE85B3Du },
	{ 0x080DF332, 0x080DF36C, XN_BN4_CODE + 0x7202, 0x060C3CC2u },
	{ 0x080F0F64, 0x080F0F88, XN_BN4_CODE + 0x7240, 0xF15BEE4Bu },
	{ 0x080F0F88, 0x080F0FB2, XN_BN4_CODE + 0x7270, 0xAB725CCEu },
	{ 0x080F0FB2, 0x080F0FF0, XN_BN4_CODE + 0x72A2, 0xBC2BF68Cu },
	{ 0x080F0FF0, 0x080F1030, XN_BN4_CODE + 0x72E0, 0x9B39EAC0u },
	{ 0x080F1030, 0x080F1068, XN_BN4_CODE + 0x7320, 0x98B3FF6Bu },
	{ 0x080F1068, 0x080F1096, XN_BN4_CODE + 0x7360, 0x1BFFD103u },
	{ 0x080F1096, 0x080F10F0, XN_BN4_CODE + 0x7392, 0x95725CEDu },
	{ 0x080F10F0, 0x080F1110, XN_BN4_CODE + 0x73F0, 0xB0175748u },
	{ 0x080F1110, 0x080F1142, XN_BN4_CODE + 0x7410, 0xACE83B61u },
	{ 0x080F1142, 0x080F1178, XN_BN4_CODE + 0x7452, 0x4D9911A4u },
	{ 0x080F1178, 0x080F1244, XN_BN4_CODE + 0x7490, 0x4A890B06u },
	{ 0x080F1244, 0x080F124A, XN_BN4_CODE + 0x7560, 0xBE868227u },
	{ 0x080F124A, 0x080F1262, XN_BN4_CODE + 0x7572, 0x4083D653u },
	{ 0x080F1262, 0x080F1274, XN_BN4_CODE + 0x7592, 0x4ACE48F9u },
	{ 0x080F1274, 0x080F1290, XN_BN4_CODE + 0x75B0, 0x123F8351u },
	{ 0x080F1290, 0x080F12BC, XN_BN4_CODE + 0x75D0, 0x3140D1B0u },
	{ 0x080F12BC, 0x080F12E6, XN_BN4_CODE + 0x7600, 0x179E9893u },
	{ 0x080F12E6, 0x080F1314, XN_BN4_CODE + 0x7632, 0xE1FBF794u },
	{ 0x080F1314, 0x080F133E, XN_BN4_CODE + 0x7660, 0xD34E8B99u },
	{ 0x080F133E, 0x080F137C, XN_BN4_CODE + 0x7692, 0xF79D5787u },
	{ 0x080F137C, 0x080F13AE, XN_BN4_CODE + 0x76D0, 0x9694EB11u },
	{ 0x080F13AE, 0x080F13CC, XN_BN4_CODE + 0x7712, 0xD03724DCu },
	{ 0x080F13CC, 0x080F1416, XN_BN4_CODE + 0x7730, 0xA24EF4A5u },
	{ 0x080F1416, 0x080F1426, XN_BN4_CODE + 0x7782, 0x33B00DC5u },
	{ 0x080F1428, 0x080F1448, XN_BN4_CODE + 0x77A0, 0x91801A37u },
	{ 0x080F1448, 0x080F1480, XN_BN4_CODE + 0x77C0, 0x704BC5EFu },
	{ 0x080F1480, 0x080F14AE, XN_BN4_CODE + 0x7800, 0x1295256Du },
	{ 0x080F14AE, 0x080F1664, XN_BN4_CODE + 0x7832, 0xFE2DE1D7u },
	{ 0x080F1664, 0x080F1674, XN_BN4_CODE + 0x79F0, 0x1D710FF0u },
};
static const XvData searchman_data[] = {
	{ 0x080C11F8, 0x080C120A, XV_ZERO },
	{ 0x080C120A, 0x080C120C, XV_RAW },
	{ 0x080C120C, 0x080C12B8, XV_ZERO },
	{ 0x080C12B8, 0x080C12C0, XV_RAW },
	{ 0x080C12D2, 0x080C12D4, XV_RAW },
	{ 0x080C12D4, 0x080C12D8, XV_POOL },
	{ 0x080C12D8, 0x080C12DC, XV_TABLE },
	{ 0x080C13B8, 0x080C13BC, XV_POOL },
	{ 0x080C13EC, 0x080C13F0, XV_POOL },
	{ 0x080C13F0, 0x080C13F4, XV_RAW },
	{ 0x080C1410, 0x080C141C, XV_POOL },
	{ 0x080C141C, 0x080C1428, XV_RAW },
	{ 0x080C1428, 0x080C142C, XV_POOL },
	{ 0x080C142C, 0x080C1430, XV_RAW },
	{ 0x080C1478, 0x080C147C, XV_POOL },
	{ 0x080C147C, 0x080C148C, XV_RAW },
	{ 0x080C14FC, 0x080C1500, XV_POOL },
	{ 0x080C1500, 0x080C1518, XV_RAW },
	{ 0x080C1518, 0x080C151C, XV_POOL },
	{ 0x080D87BE, 0x080D87C0, XV_RAW },
	{ 0x080D87C0, 0x080D87C4, XV_POOL },
	{ 0x080D87C4, 0x080D87D0, XV_TABLE },
	{ 0x080D7B76, 0x080D7B78, XV_RAW },
	{ 0x080D7B78, 0x080D7B7C, XV_POOL },
	{ 0x080D7B7C, 0x080D7B88, XV_TABLE },
	{ 0x080D7C54, 0x080D7C58, XV_POOL },
	{ 0x080D7C58, 0x080D7C64, XV_TABLE },
	{ 0x080D7CE4, 0x080D7CE8, XV_POOL },
	{ 0x080D7D6C, 0x080D7D70, XV_POOL },
	{ 0x080E55A8, 0x080E55AC, XV_POOL },
	{ 0x080E55AC, 0x080E55B8, XV_TABLE },
	{ 0x080E562E, 0x080E5630, XV_RAW },
	{ 0x080E5630, 0x080E5634, XV_POOL },
	{ 0x080E5634, 0x080E5640, XV_TABLE },
	{ 0x080E5682, 0x080E5684, XV_RAW },
	{ 0x080E5684, 0x080E5688, XV_POOL },
	{ 0x080E5688, 0x080E56A0, XV_RAW },
	{ 0x080E56DC, 0x080E56E0, XV_POOL },
	{ 0x080E56E0, 0x080E56E4, XV_RAW },
	{ 0x080E589C, 0x080E58B4, XV_RAW },
	{ 0x080E58CC, 0x080E58D0, XV_POOL },
	{ 0x080E58D0, 0x080E58DC, XV_TABLE },
	{ 0x080E593C, 0x080E5944, XV_RAW },
	{ 0x080E5944, 0x080E5948, XV_POOL },
	{ 0x080E5948, 0x080E5960, XV_TABLE },
	{ 0x080E5A1C, 0x080E5A24, XV_POOL },
	{ 0x080E5A98, 0x080E5A9C, XV_POOL },
	{ 0x080E5A9C, 0x080E5AA0, XV_RAW },
	{ 0x080E5B44, 0x080E5B4C, XV_RAW },
	{ 0x080E5B4C, 0x080E5B50, XV_POOL },
	{ 0x080D95DE, 0x080D95E0, XV_RAW },
	{ 0x080D95E0, 0x080D95E4, XV_POOL },
	{ 0x080D95E4, 0x080D95F0, XV_TABLE },
	{ 0x080D96A6, 0x080D96AC, XV_RAW },
	{ 0x080D96AC, 0x080D96B0, XV_POOL },
	{ 0x080D96B0, 0x080D96C4, XV_TABLE },
	{ 0x080D9822, 0x080D9824, XV_RAW },
	{ 0x080D9824, 0x080D9828, XV_POOL },
	{ 0x080E650E, 0x080E6510, XV_RAW },
	{ 0x080E6510, 0x080E6514, XV_POOL },
	{ 0x080E6514, 0x080E6520, XV_TABLE },
	{ 0x080E65DA, 0x080E65E0, XV_RAW },
	{ 0x0800AD7E, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B130, XV_POOL },
	{ 0x080DF344, 0x080DF364, XV_ZERO },
	{ 0x080DF364, 0x080DF36C, XV_POOL },
	{ 0x080F0F78, 0x080F0F7C, XV_POOL },
	{ 0x080F0F7C, 0x080F0F88, XV_TABLE },
	{ 0x080F102A, 0x080F102C, XV_ZERO },
	{ 0x080F102C, 0x080F1030, XV_POOL },
	{ 0x080F104E, 0x080F1050, XV_ZERO },
	{ 0x080F1050, 0x080F1054, XV_POOL },
	{ 0x080F1054, 0x080F1068, XV_TABLE },
	{ 0x080F10EA, 0x080F10EC, XV_ZERO },
	{ 0x080F10EC, 0x080F10F0, XV_POOL },
	{ 0x080F10FE, 0x080F1100, XV_ZERO },
	{ 0x080F1100, 0x080F1104, XV_POOL },
	{ 0x080F1104, 0x080F1110, XV_TABLE },
	{ 0x080F1236, 0x080F1238, XV_ZERO },
	{ 0x080F1238, 0x080F123C, XV_POOL },
	{ 0x080F123C, 0x080F1244, XV_RAW },
	{ 0x080F12FA, 0x080F12FC, XV_ZERO },
	{ 0x080F12FC, 0x080F1300, XV_POOL },
	{ 0x080F1300, 0x080F1314, XV_TABLE },
	{ 0x080F1466, 0x080F1468, XV_ZERO },
	{ 0x080F1468, 0x080F146C, XV_POOL },
	{ 0x080F146C, 0x080F1480, XV_TABLE },
	{ 0x080F164E, 0x080F1650, XV_ZERO },
	{ 0x080F1650, 0x080F1654, XV_POOL },
	{ 0x080F1654, 0x080F1664, XV_RAW },
};
static const XvPair searchman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002CEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08002D4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08002E44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002F54, 0x08002F02 },   /* project:fireman, SAME */
	{ 0x08002F90, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x08003058, 0x08003006 },   /* map:bytes, SAME */
	{ 0x0800307C, 0x0800302A },   /* map:bytes, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007A8C, 0x0800A2F0 },   /* map:bytes, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009AEC, 0x0800C938 },   /* project:gutsman, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009E98, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x08009ED8, 0x0800CCB2 },   /* map:calls+bytes, SAME */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800A0A2, 0x0800CFA6 },   /* project:spikey, SAME */
	{ 0x0800A0D8, 0x0800CFDC },   /* project:research, SAME */
	{ 0x0800A10E, 0x0800D012 },   /* project:research, SAME */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800B0CA, 0x0800EB6C },   /* project:project:searchman, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x0800C126, 0x080101AE },   /* project:project:searchman, LAYOUT */
	{ 0x0800C13C, 0x080101C4 },   /* project:project:searchman, LAYOUT */
	{ 0x0800C456, 0x080103BC },   /* project:gutsman, DIFF (audited) */
	{ 0x0800D772, 0x080136CC },   /* search, SAME */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080124C8, 0x080198CE },   /* map:calls, LAYOUT */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0801457E, 0x0801BD3C },   /* map:bytes, SAME */
	{ 0x08014620, 0x0801BDDE },   /* map:bytes, SAME */
	{ 0x0801466A, 0x0801BE28 },   /* map:calls, DIFF (audited) */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ 0x080E65DA, 0x080E60E2 },   /* project:project:searchman, SAME */
};
static const XvImm searchman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800AD6A, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD6C, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C0BC, THUMB_LDR, 0, 4, 0x7C, 0x50 },   /* collision data +0x7c in BN4, +0x50 in BN6 */
	{ 0x0800C0C6, THUMB_STR, 0, 4, 0x7C, 0x50 },   /* collision data +0x7c in BN4, +0x50 in BN6 */
	{ 0x0800C0CA, THUMB_LDR, 0, 4, 0x7C, 0x50 },   /* collision data +0x7c in BN4, +0x50 in BN6 */
	{ 0x0800C0D6, THUMB_STR, 0, 4, 0x7C, 0x50 },   /* collision data +0x7c in BN4, +0x50 in BN6 */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080C1306, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 set (BN4_SET_ACTION's r0) */
	{ 0x080C1336, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080C1370, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080C13AC, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C13BE, THUMB_MOVS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C13CA, THUMB_MOVS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C13DE, THUMB_MOVS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C1408, THUMB_MOVS, 2, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080D7B8C, THUMB_MOVS, 2, 0, 0x24, 0xDB },   /* its object's sprite: list 4 0x24 in BN4, list 4 0xdb in BN6 (copied) */
	{ 0x080D7C1A, THUMB_MOVS, 0, 0, 0xC8, 0xD0 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0xd0, on its player 11 */
	{ 0x080D7D3E, THUMB_MOVS, 2, 0, 0x0, 0x9C },   /* its effect (r2 of 0801457E): 0x0 in BN4, 0x9c in BN6 (moved) */
	{ 0x080D7D52, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D7D60, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's empty slot 0x3a, on its player 9 */
	{ 0x080D7D74, THUMB_MOVS, 0, 0, 0x52, 0xF7 },   /* its object's type 0x52: moved, the dispatch's 0xf7 */
	{ 0x080D8820, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D8830, THUMB_MOVS, 4, 0, 0x8, 0x9D },   /* its effect: 0x8 in BN4, 0x9d in BN6 (moved; his shot's hit (the third kind's 0x57): r4 8 ORed with the RNG's bit 1 shifted to bit 9, BN4_SPAWN_EFFECT's flip, the id its low byte) */
	{ 0x080D8858, THUMB_MOVS, 0, 0, 0x57, 0xF8 },   /* its object's type 0x57: moved, the dispatch's 0xf8 */
	{ 0x080D95F4, THUMB_MOVS, 2, 0, 0x1E, 0xDC },   /* its object's sprite: list 4 0x1e in BN4, list 4 0xdc in BN6 (copied) */
	{ 0x080D9634, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D9686, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D970C, THUMB_MOVS, 0, 0, 0x5, 0xE0 },   /* its sound 0x104 (movs and adds #0xff) at the pool's 0xe0 */
	{ 0x080D970E, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x104 (movs and adds #0xff) at the pool's 0xe0 */
	{ 0x080D980A, THUMB_MOVS, 0, 0, 0x60, 0xF9 },   /* its object's type 0x60: moved, the dispatch's 0xf9 */
	{ 0x080DF33E, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080E55BC, THUMB_MOVS, 2, 0, 0x22, 0xDD },   /* its object's sprite: list 4 0x22 in BN4, list 4 0xdd in BN6 (copied) */
	{ 0x080E572A, THUMB_MOVS, 0, 0, 0x2E, 0xE0 },   /* its object's type 0x2e: moved, the dispatch's 0xe0 */
	{ 0x080E5A30, THUMB_MOVS, 0, 0, 0xC1, 0x63 },   /* its sound 0xc1: BN4's copied into BN6's empty slot 0x63, on its player 8 */
	{ 0x080E5B38, THUMB_MOVS, 0, 0, 0x31, 0xE1 },   /* its object's type 0x31: moved, the dispatch's 0xe1 */
	{ 0x080E652A, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080E652C, THUMB_MOVS, 2, 0, 0x2D, 0xDA },   /* its object's sprite: list 3 0x2d in BN4, list 4 0xda in BN6 (copied) */
	{ 0x080E65C6, THUMB_MOVS, 0, 0, 0x3E, 0xE2 },   /* its object's type 0x3e: moved, the dispatch's 0xe2 */
	{ 0x080F0F68, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F1034, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F1152, THUMB_MOVS, 0, 0, 0x90, 0x32 },   /* its sound 0x90: BN4's copied into BN6's empty slot 0x32, on its player 8 */
	{ 0x080F12EA, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F13DC, THUMB_MOVS, 0, 0, 0xBB, 0x33 },   /* its sound 0xbb: BN4's copied into BN6's empty slot 0x33, on its player 14 */
	{ 0x080F144C, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F166C, THUMB_LDRH, 1, 0, 0x1A, 0x26 },   /* collision data +0x1a in BN4, +0x26 in BN6 */
};
static const XvNumber searchman_numbers[] = {
	{ 0x080D7D6C, 0x0405FF09, 0x0405FF09 },   /* a collision region's spawn word (SearchMan's shot's, 0x080D7D56, r4 of 0x080CD7E2: BN6's 0x080C6BDA), as 0x0405FF01 */
	{ 0x080DF364, 0x0A800000, 0x0A800000 },   /* the flags its hits left it, tested (BN4_COLL_FROM_COLLISION): BN6's bits at FlagsFromCollision (+0x70), where the thunder's update tests the same mask, 0xFF800000, in both games */
	{ 0x080DF368, 0x05800000, 0x05800000 },   /* SearchMan's shot's test (0x080DF332): the hits word's bits of a side 0 object's foes (0x0A800000 a side 1 one's), as BN4_COLL_MASKS' and BN6_COLL_MASKS' alliance bits */
};
static const XvPair searchman_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
	{ 0x0801056F, 0x08016C9D },   /* project:project:searchman, SAME */
};
static const XvByte searchman_bytes[] = {
	{ 0x080E589D, 0x23, 0xDE },   /* its fourth-kind 0x31's kind 0: list 4 0x23 in BN4, 0xde copied */
	{ 0x080E58A1, 0x1F, 0xDF },   /* its fourth-kind 0x31's kind 1: list 4 0x1f in BN4, 0xdf copied */
	{ 0x080E58A5, 0x21, 0xE0 },   /* its fourth-kind 0x31's kind 2: list 4 0x21 in BN4, 0xe0 copied */
};
static const XvType searchman_types[] = {
	{ 3, 0xF7, XV_TYPE_MOVED, 0x52 },   /* BN4's third-kind 0x52, moved, the dispatch's 0xf7 */
	{ 3, 0xF8, XV_TYPE_MOVED, 0x57 },   /* BN4's third-kind 0x57, moved, the dispatch's 0xf8 */
	{ 3, 0xF9, XV_TYPE_MOVED, 0x60 },   /* BN4's third-kind 0x60, moved, the dispatch's 0xf9 */
	{ 4, 0xE0, XV_TYPE_MOVED, 0x2E },   /* BN4's fourth-kind 0x2e, moved, the dispatch's 0xe0 */
	{ 4, 0xE1, XV_TYPE_MOVED, 0x31 },   /* BN4's fourth-kind 0x31, moved, the dispatch's 0xe1 */
	{ 4, 0xE2, XV_TYPE_MOVED, 0x3E },   /* BN4's fourth-kind 0x3e, moved, the dispatch's 0xe2 */
};
static const XvRef searchman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C12C1), OWN(0x080F0F65), OWN(0x080F1031), OWN(0x080F12E7), OWN(0x080F1449),
};
static const XvSprite searchman_sprites[] = {
	{ 2, 0x04, 2, 0x43, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x2D, 4, 0xDA, XN_BN4_ART + 0x0, 0x800 },
	{ 4, 0x24, 4, 0xDB, XN_BN4_ART + 0x800, 0x800 },
	{ 4, 0x1E, 4, 0xDC, XN_BN4_ART + 0x1000, 0x1800 },
	{ 4, 0x22, 4, 0xDD, XN_BN4_ART + 0x2800, 0x800 },
	{ 4, 0x23, 4, 0xDE, XN_BN4_ART + 0x3000, 0x800 },
	{ 4, 0x1F, 4, 0xDF, XN_BN4_ART + 0x3800, 0x1800 },
	{ 4, 0x21, 4, 0xE0, XN_BN4_ART + 0x5000, 0x1000 },
	{ 5, 0x05, 5, 0x9E, XN_BN4_ART + 0x6000, 0x5500 },
	{ 5, 0x00, 5, 0x9D, XN_BN4_ART + 0xB500, 0x1C00 },
};
static const XvId searchman_effects[] = { { 0x08, 0x9D }, { 0x00, 0x9C } };
static const XvSong searchman_songs[] = {
	{ 0x90, 0x32, XN_BN4_ART + 0xD100, 0x40 },   /* its sound 0x90, BN4's song copied */
	{ 0xBB, 0x33, XN_BN4_ART + 0xD140, 0x900 },   /* its sound 0xbb, BN4's song copied */
	{ 0xC1, 0x63, XN_BN4_ART + 0xDA40, 0x80 },   /* its sound 0xc1, BN4's song copied */
	{ 0xC8, 0xD0, XN_BN4_ART + 0xDAC0, 0xB40 },   /* its sound 0xc8, BN4's song copied */
	{ 0x104, 0xE0, XN_BN4_ART + 0xE600, 0x14C0 },   /* its sound 0x104, BN4's song copied */
	{ 0x105, 0x104, XN_BN4_ART + 0xFAC0, 0x1480 },   /* its sound 0x105, BN4's song copied */
	{ 0x109, 0x117, XN_BN4_ART + 0x10F40, 0xF40 },   /* its sound 0x109, BN4's song copied */
	{ 0x128, 0x148, XN_BN4_ART + 0x11E80, 0x40 },   /* its sound 0x128, BN4's song copied */
	{ 0x12F, 0x150, XN_BN4_ART + 0x11EC0, 0x40 },   /* its sound 0x12f, BN4's song copied */
};
static const XvSoundCall searchman_sound_calls[] = {
	{ 0x080D966C, 0x104 },   /* its sound 0x105 (movs and adds #0xff) at the pool's 0x104, past a byte */
	{ 0x080E5678, 0x148 },   /* its sound 0x128 (movs and adds #0xff) at the pool's 0x148, past a byte */
	{ 0x080E56F8, 0x150 },   /* its sound 0x12f (movs and adds #0xff) at the pool's 0x150, past a byte */
	{ 0x080E5976, 0x148 },   /* its sound 0x128 (movs and adds #0xff) at the pool's 0x148, past a byte */
	{ 0x080E5998, 0x150 },   /* its sound 0x12f (movs and adds #0xff) at the pool's 0x150, past a byte */
	{ 0x080F10C4, 0x117 },   /* its sound 0x109 (movs and adds #0xff) at the pool's 0x117, past a byte */
	{ 0x080F14D2, 0x117 },   /* its sound 0x109 (movs and adds #0xff) at the pool's 0x117, past a byte */
};
static const XnInit searchman_init = { NULL, 0, false, false, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */


static const XNavi bn4_searchman = {
	.code = {
		.name = "SearchMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x2B,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 51,
		.first_id = XN_BN4_FIRST_ID + 0xC,
		.home = XN_BN4_HOMES + 0x1800,
		.blocks = searchman_blocks, .nblocks = (int)(sizeof searchman_blocks / sizeof *searchman_blocks),
		.data = searchman_data, .ndata = (int)(sizeof searchman_data / sizeof *searchman_data),
		.calls = searchman_calls, .ncalls = (int)(sizeof searchman_calls / sizeof *searchman_calls),
		.words = searchman_words, .nwords = (int)(sizeof searchman_words / sizeof *searchman_words),
		.imms = searchman_imms, .nimms = (int)(sizeof searchman_imms / sizeof *searchman_imms),
		.numbers = searchman_numbers, .nnumbers = (int)(sizeof searchman_numbers / sizeof *searchman_numbers),
		.bytes = searchman_bytes, .nbytes = (int)(sizeof searchman_bytes / sizeof *searchman_bytes),
		.types = searchman_types, .ntypes = (int)(sizeof searchman_types / sizeof *searchman_types),
		.states = searchman_states, .nstates = (int)(sizeof searchman_states / sizeof *searchman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08011F18, .struct2 = 0x08011F18,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0x8C, 0x8D, 0x8E, 0x8F },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x00 },
		.sprites = searchman_sprites, .nsprites = (int)(sizeof searchman_sprites / sizeof *searchman_sprites),
		.effects = searchman_effects, .neffects = (int)(sizeof searchman_effects / sizeof *searchman_effects),
		.songs = searchman_songs, .nsongs = (int)(sizeof searchman_songs / sizeof *searchman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
		.sound_calls = searchman_sound_calls, .nsound_calls = (int)(sizeof searchman_sound_calls / sizeof *searchman_sound_calls),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.palettes = 0x04000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &searchman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
	.hide_step = OWN(0x0800C0A7),   /* (its copy of BN4's hide step before each of its states, as BN4's Navi runner runs it: xnavirecipe_bn4_objects.hide_steps) */
};
/* ---- BN4's AquaMan (its type 0xd there, its enemy ids 168-171), as Navi AI 58 with ids 0x3a8-0x3ab ---- */
/* interim: 080CCDC8: the first kind's 0x5C's state 4, step 0 (WindMan's companions': its sprite drawn again through BN4's tile buffer 0x02037810, a copy of 0x0800087C's and 0x08003578's): its init takes state 4 only where the battle state's +0x32 has no bit 0 (0x08007A8C) and its settings' +6 has 0x20 (0x080CCD1E-0x080CCD2E), else 0 or 8; kept as BN4's bytes, unported (assumed unreached in a plain battle) */

static const XvBlock aquaman_blocks[] = {
	{ 0x080B820C, 0x080B8520, XN_BN4_CODE + 0x7A00, 0xBFB28341u },
	{ 0x080CCC9C, 0x080CCF5C, XN_BN4_CODE + 0x7D20, 0xBEC868D2u },
	{ 0x080D1944, 0x080D1C24, XN_BN4_CODE + 0x7FE0, 0x60EF8AA0u },
	{ 0x080D1448, 0x080D1678, XN_BN4_CODE + 0x82C0, 0xC707CE0Cu },
	{ 0x080D12BC, 0x080D1448, XN_BN4_CODE + 0x84F0, 0x96330CF8u },
	{ 0x080B81F8, 0x080B820C, XN_BN4_CODE + 0x8680, 0x16031ECDu },
	{ 0x080CCC94, 0x080CCC9C, XN_BN4_CODE + 0x86A0, 0xD258FE18u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0x86B2, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x8AC2, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x8AE0, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x8AF0, 0xDCE85B3Du },
	{ 0x0800F734, 0x0800F780, XN_BN4_CODE + 0x8B20, 0x0198FB6Eu },
	{ 0x080F077C, 0x080F0782, XN_BN4_CODE + 0x8B70, 0x4A7133B7u },
	{ 0x080F1D4C, 0x080F20FC, XN_BN4_CODE + 0x8B80, 0x21BAD96Bu },
	{ 0x080F4434, 0x080F4458, XN_BN4_CODE + 0x8F30, 0xD1CF0A2Cu },
	{ 0x080F4458, 0x080F44AC, XN_BN4_CODE + 0x8F60, 0xB1754E0Cu },
	{ 0x080F44AC, 0x080F44EA, XN_BN4_CODE + 0x8FC0, 0x010F79DAu },
	{ 0x080F44EA, 0x080F450E, XN_BN4_CODE + 0x9002, 0x100463F8u },
	{ 0x080F450E, 0x080F4534, XN_BN4_CODE + 0x9032, 0xC7D8F46Fu },
	{ 0x080F4534, 0x080F4564, XN_BN4_CODE + 0x9060, 0x3475FF94u },
	{ 0x080F4564, 0x080F45C2, XN_BN4_CODE + 0x9090, 0xADCC312Au },
	{ 0x080F45C2, 0x080F45EC, XN_BN4_CODE + 0x90F2, 0xDC486C48u },
	{ 0x080F45EC, 0x080F460C, XN_BN4_CODE + 0x9120, 0xE25DA5D6u },
	{ 0x080F460C, 0x080F4676, XN_BN4_CODE + 0x9140, 0x6A7D35B1u },
	{ 0x080F4676, 0x080F46A0, XN_BN4_CODE + 0x91B2, 0xAC6A70A8u },
	{ 0x080F46A0, 0x080F46C0, XN_BN4_CODE + 0x91E0, 0xCA0081BFu },
	{ 0x080F46C0, 0x080F4760, XN_BN4_CODE + 0x9200, 0x77546C4Eu },
	{ 0x080F4760, 0x080F478A, XN_BN4_CODE + 0x92A0, 0x11A1E57Fu },
	{ 0x080F478A, 0x080F484C, XN_BN4_CODE + 0x92D2, 0xA57326AAu },
	{ 0x080F484C, 0x080F48C8, XN_BN4_CODE + 0x93A0, 0xD8D80A54u },
	{ 0x080F48C8, 0x080F4960, XN_BN4_CODE + 0x9420, 0xB6692C5Bu },
	{ 0x080F4960, 0x080F4970, XN_BN4_CODE + 0x94C0, 0x9ED15F9Au },
	{ 0x080F4970, 0x080F4986, XN_BN4_CODE + 0x94D0, 0x924767E1u },
};
static const XvData aquaman_data[] = {
	{ 0x080B820C, 0x080B821E, XV_ZERO },
	{ 0x080B821E, 0x080B8220, XV_RAW },
	{ 0x080B8220, 0x080B82D4, XV_ZERO },
	{ 0x080B82EA, 0x080B82EC, XV_RAW },
	{ 0x080B82EC, 0x080B82F0, XV_POOL },
	{ 0x080B82F0, 0x080B82FC, XV_TABLE },
	{ 0x080B83A4, 0x080B83A8, XV_POOL },
	{ 0x080B83B6, 0x080B83B8, XV_RAW },
	{ 0x080B83B8, 0x080B83BC, XV_POOL },
	{ 0x080B83BC, 0x080B83C4, XV_TABLE },
	{ 0x080B83F4, 0x080B83F8, XV_POOL },
	{ 0x080B8426, 0x080B8428, XV_RAW },
	{ 0x080B8428, 0x080B842C, XV_POOL },
	{ 0x080B842C, 0x080B8434, XV_TABLE },
	{ 0x080B8464, 0x080B8468, XV_POOL },
	{ 0x080B849A, 0x080B849C, XV_RAW },
	{ 0x080B849C, 0x080B84A0, XV_POOL },
	{ 0x080B84A0, 0x080B84E0, XV_RAW },
	{ 0x080B84E8, 0x080B84EC, XV_POOL },
	{ 0x080B84EC, 0x080B84F0, XV_RAW },
	{ 0x080B84F0, 0x080B8520, XV_POOL },
	{ 0x080CCCAA, 0x080CCCAC, XV_RAW },
	{ 0x080CCCAC, 0x080CCCB0, XV_POOL },
	{ 0x080CCCB0, 0x080CCCBC, XV_TABLE },
	{ 0x080CCD38, 0x080CCD3C, XV_POOL },
	{ 0x080CCD88, 0x080CCD8C, XV_POOL },
	{ 0x080CCD8C, 0x080CCD98, XV_TABLE },
	{ 0x080CCDB8, 0x080CCDBC, XV_POOL },
	{ 0x080CCDBC, 0x080CCDC8, XV_TABLE },
	{ 0x080CCDC8, 0x080CCE40, XV_RAW },
	{ 0x080CCEB6, 0x080CCEB8, XV_RAW },
	{ 0x080CCEB8, 0x080CCEBC, XV_POOL },
	{ 0x080CCEBC, 0x080CCECC, XV_TABLE },
	{ 0x080CCF54, 0x080CCF5C, XV_RAW },
	{ 0x080D195C, 0x080D1960, XV_POOL },
	{ 0x080D1960, 0x080D196C, XV_TABLE },
	{ 0x080D1A3A, 0x080D1A3C, XV_RAW },
	{ 0x080D1A3C, 0x080D1A40, XV_POOL },
	{ 0x080D1A40, 0x080D1A58, XV_TABLE },
	{ 0x080D1B84, 0x080D1B88, XV_POOL },
	{ 0x080D1C20, 0x080D1C24, XV_POOL },
	{ 0x080D145A, 0x080D145C, XV_RAW },
	{ 0x080D145C, 0x080D1460, XV_POOL },
	{ 0x080D1460, 0x080D146C, XV_TABLE },
	{ 0x080D1504, 0x080D1508, XV_POOL },
	{ 0x080D1508, 0x080D1510, XV_TABLE },
	{ 0x080D15DC, 0x080D15E0, XV_POOL },
	{ 0x080D15E0, 0x080D15E4, XV_RAW },
	{ 0x080D1644, 0x080D1648, XV_POOL },
	{ 0x080D1648, 0x080D1658, XV_RAW },
	{ 0x080D1674, 0x080D1678, XV_POOL },
	{ 0x080D12CE, 0x080D12D0, XV_RAW },
	{ 0x080D12D0, 0x080D12D4, XV_POOL },
	{ 0x080D12D4, 0x080D12E0, XV_TABLE },
	{ 0x080D136A, 0x080D136C, XV_RAW },
	{ 0x080D136C, 0x080D1370, XV_POOL },
	{ 0x080D1370, 0x080D1378, XV_TABLE },
	{ 0x080D140A, 0x080D140C, XV_RAW },
	{ 0x080D140C, 0x080D1410, XV_POOL },
	{ 0x080D1410, 0x080D1414, XV_RAW },
	{ 0x080D1440, 0x080D1448, XV_POOL },
	{ 0x080B81F8, 0x080B820C, XV_RAW },
	{ 0x080CCC94, 0x080CCC9C, XV_RAW },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0800F76C, 0x0800F770, XV_POOL },
	{ 0x0800F770, 0x0800F780, XV_RAW },
	{ 0x080F1D84, 0x080F1D88, XV_POOL },
	{ 0x080F1D88, 0x080F1DB0, XV_RAW },
	{ 0x080F1DB0, 0x080F1DB4, XV_POOL },
	{ 0x080F1DB4, 0x080F1E0C, XV_RAW },
	{ 0x080F1E0C, 0x080F1E10, XV_POOL },
	{ 0x080F1E10, 0x080F1E24, XV_POOL },
	{ 0x080F1E24, 0x080F1EE4, XV_RAW },
	{ 0x080F1EE4, 0x080F1EE8, XV_POOL },
	{ 0x080F1EE8, 0x080F1EF0, XV_POOL },
	{ 0x080F1EF0, 0x080F1F48, XV_RAW },
	{ 0x080F1F48, 0x080F1F4C, XV_POOL },
	{ 0x080F1F4C, 0x080F1F5C, XV_RAW },
	{ 0x080F1F5C, 0x080F1F60, XV_POOL },
	{ 0x080F1F60, 0x080F1F68, XV_POOL },
	{ 0x080F1F68, 0x080F1FC8, XV_RAW },
	{ 0x080F1FC8, 0x080F1FD0, XV_POOL },
	{ 0x080F1FD0, 0x080F1FE0, XV_RAW },
	{ 0x080F1FE0, 0x080F1FE4, XV_POOL },
	{ 0x080F1FE4, 0x080F1FEC, XV_POOL },
	{ 0x080F1FEC, 0x080F204C, XV_RAW },
	{ 0x080F204C, 0x080F2050, XV_POOL },
	{ 0x080F2050, 0x080F2058, XV_POOL },
	{ 0x080F2058, 0x080F20C0, XV_RAW },
	{ 0x080F20C0, 0x080F20C4, XV_POOL },
	{ 0x080F20C4, 0x080F20D0, XV_RAW },
	{ 0x080F20D0, 0x080F20D4, XV_POOL },
	{ 0x080F20D4, 0x080F20D8, XV_RAW },
	{ 0x080F20D8, 0x080F20E0, XV_POOL },
	{ 0x080F20E0, 0x080F20E4, XV_RAW },
	{ 0x080F20E4, 0x080F20FC, XV_POOL },
	{ 0x080F4448, 0x080F444C, XV_POOL },
	{ 0x080F444C, 0x080F4458, XV_TABLE },
	{ 0x080F4522, 0x080F4524, XV_ZERO },
	{ 0x080F4524, 0x080F4528, XV_POOL },
	{ 0x080F4528, 0x080F4534, XV_TABLE },
	{ 0x080F4600, 0x080F4604, XV_POOL },
	{ 0x080F4604, 0x080F460C, XV_TABLE },
	{ 0x080F46B4, 0x080F46B8, XV_POOL },
	{ 0x080F46B8, 0x080F46C0, XV_TABLE },
	{ 0x080F4752, 0x080F4754, XV_ZERO },
	{ 0x080F4754, 0x080F4760, XV_POOL },
	{ 0x080F482A, 0x080F482C, XV_ZERO },
	{ 0x080F482C, 0x080F4834, XV_POOL },
	{ 0x080F4834, 0x080F484C, XV_RAW },
	{ 0x080F48B2, 0x080F48B4, XV_ZERO },
	{ 0x080F48B4, 0x080F48B8, XV_POOL },
	{ 0x080F48B8, 0x080F48C8, XV_RAW },
	{ 0x080F4948, 0x080F4950, XV_POOL },
	{ 0x080F4950, 0x080F4960, XV_RAW },
};
static const XvPair aquaman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001956, 0x08001330 },   /* map:bytes, SAME */
	{ 0x080019C0, 0x0800139A },   /* map:calls, LAYOUT */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002CEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08002D4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002DF8, 0x08002D8C },   /* map:bytes, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E20, 0x08002DB4 },   /* project:fireman, SAME */
	{ 0x08002E34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002F22, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002F2E, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002F54, 0x08002F02 },   /* project:fireman, SAME */
	{ 0x08002F90, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032B8, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x080078A8, 0x08009FF8 },   /* map:bytes, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007A8C, 0x0800A2F0 },   /* map:bytes, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009AEC, 0x0800C938 },   /* project:gutsman, DIFF (audited) */
	{ 0x08009E98, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009ECC, 0x0800CCA6 },   /* project:gutsman, SAME */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800A0D8, 0x0800CFDC },   /* project:research, SAME */
	{ 0x0800A1AE, 0x0800D086 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x0800C456, 0x080103BC },   /* project:gutsman, DIFF (audited) */
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
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0801457E, 0x0801BD3C },   /* map:bytes, SAME */
	{ 0x080406A4, 0x0800A03C },   /* map:calls, DIFF (audited) */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm aquaman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B82D8, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080B82DC, THUMB_ADDS, 6, 0, 0x64, 0x80 },   /* its AI data's +0x64 in BN4, +0x80 in BN6 (AI_BLOCKS) */
	{ 0x080B834A, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B839C, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080B83EC, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B8410, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080B845C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080B8492, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 set (BN4_SET_ACTION's r0) */
	{ 0x080CCD2E, THUMB_MOVS, 2, 0, 0x4, 0x0 },   /* its companion's mode 4 (its appear through BN4's tile buffer, unported) run as its mode 0 */
	{ 0x080CCF38, THUMB_MOVS, 0, 0, 0x5C, 0xE3 },   /* its companion's type 0x5c (the first kind): moved, the dispatch's 0xe3 */
	{ 0x080D12E4, THUMB_MOVS, 2, 0, 0xF, 0xE2 },   /* its object's sprite: list 4 0xf in BN4, list 4 0xe2 in BN6 (copied) */
	{ 0x080D132E, THUMB_MOVS, 0, 0, 0x9E, 0x33 },   /* its sound 0x9e: BN4's copied into BN6's empty slot 0x33, on its player 8 */
	{ 0x080D134A, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D13AA, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D13C8, THUMB_MOVS, 2, 0, 0x25, 0x95 },   /* its effect (r2 of 0801457E): 0x25 in BN4, 0x95 in BN6 (moved) */
	{ 0x080D13DA, THUMB_MOVS, 0, 0, 0x9F, 0x3D },   /* its sound 0x9f: BN4's copied into BN6's empty slot 0x3d, on its player 28 */
	{ 0x080D1424, THUMB_MOVS, 0, 0, 0x23, 0xFA },   /* its object's type 0x23: moved, the dispatch's 0xfa */
	{ 0x080D143A, THUMB_STRB, 1, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D146E, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080D1470, THUMB_MOVS, 2, 0, 0x2, 0xD7 },   /* its object's sprite: list 3 0x2 in BN4, list 4 0xd7 in BN6 (copied) */
	{ 0x080D14D8, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D14F4, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D155C, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D1582, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D159C, THUMB_MOVS, 4, 0, 0x25, 0x95 },   /* its effect: 0x25 in BN4, 0x95 in BN6 (moved) */
	{ 0x080D1618, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D162E, THUMB_MOVS, 2, 0, 0x25, 0x95 },   /* its effect (r2 of 0801457E): 0x25 in BN4, 0x95 in BN6 (moved) */
	{ 0x080D1638, THUMB_MOVS, 0, 0, 0x9F, 0x3D },   /* its sound 0x9f: BN4's copied into BN6's empty slot 0x3d, on its player 28 */
	{ 0x080D165C, THUMB_MOVS, 0, 0, 0x24, 0xFB },   /* its object's type 0x24: moved, the dispatch's 0xfb */
	{ 0x080D166E, THUMB_STRB, 1, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D1970, THUMB_MOVS, 2, 0, 0x10, 0xE3 },   /* its object's sprite: list 4 0x10 in BN4, list 4 0xe3 in BN6 (copied) */
	{ 0x080D1A10, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D1A1E, THUMB_MOVS, 4, 0, 0x0, 0x9C },   /* its effect: 0x0 in BN4, 0x9c in BN6 (moved) */
	{ 0x080D1A24, THUMB_MOVS, 0, 0, 0xC8, 0xE0 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0xe0, on its player 11 */
	{ 0x080D1A84, THUMB_MOVS, 0, 0, 0xA0, 0x63 },   /* its sound 0xa0: BN4's copied into BN6's empty slot 0x63, on its player 29 */
	{ 0x080D1AAA, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D1B22, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D1B3A, THUMB_MOVS, 0, 0, 0xA1, 0xD0 },   /* its sound 0xa1: BN4's copied into BN6's empty slot 0xd0, on its player 9 */
	{ 0x080D1B5A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D1BE2, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D1BF8, THUMB_MOVS, 0, 0, 0x27, 0xFC },   /* its object's type 0x27: moved, the dispatch's 0xfc */
	{ 0x080D1C08, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080F4438, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F4512, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F45A0, THUMB_ADDS, 4, 0, 0x23, 0x9E },   /* its effect (its side in r4's second byte): 0x23 in BN4, 0x9e in BN6 (moved) */
	{ 0x080F45F0, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F462A, THUMB_MOVS, 0, 0, 0x90, 0x32 },   /* its sound 0x90: BN4's copied into BN6's empty slot 0x32, on its player 8 */
	{ 0x080F46A4, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
};
static const XvNumber aquaman_numbers[] = {
	{ 0x080D1B84, 0x04050201, 0x04050201 },   /* a collision region's spawn word (AquaMan's third kind 0x27's, 0x080D1B60, its r4 to 0x080CD7E2): BN6's own attacks pass the same to its twin (0x080CABAC's to 0x080C6BDA) */
};
static const XvPair aquaman_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvByte aquaman_bytes[] = {
	{ 0x080CCC97, 0x0C, 0xE1 },   /* its table's kind 1: list 4 0xc in BN4, 0xe1 copied */
};
static const XvType aquaman_types[] = {
	{ 3, 0xFA, XV_TYPE_MOVED, 0x23 },   /* BN4's third-kind 0x23, moved, the dispatch's 0xfa */
	{ 3, 0xFB, XV_TYPE_MOVED, 0x24 },   /* BN4's third-kind 0x24, moved, the dispatch's 0xfb */
	{ 3, 0xFC, XV_TYPE_MOVED, 0x27 },   /* BN4's third-kind 0x27, moved, the dispatch's 0xfc */
	{ 1, 0xE3, XV_TYPE_MOVED, 0x5C },   /* BN4's first-kind 0x5c, moved, the dispatch's 0xe3 */
};
static const XvRef aquaman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080B82D5), OWN(0x080F4435), OWN(0x080F450F), OWN(0x080F45ED), OWN(0x080F46A1),
};
static const XvSprite aquaman_sprites[] = {
	{ 2, 0x0B, 2, 0x4A, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x02, 4, 0xD7, XN_BN4_ART + 0x4800, 0x1000 },
	{ 4, 0x0C, 4, 0xE1, XN_BN4_ART + 0x0, 0x800 },
	{ 4, 0x0F, 4, 0xE2, XN_BN4_ART + 0x800, 0x800 },
	{ 4, 0x10, 4, 0xE3, XN_BN4_ART + 0x1000, 0x3800 },
	{ 5, 0x00, 5, 0x9D, XN_BN4_ART + 0x5800, 0x1C00 },
};
static const XvId aquaman_effects[] = { { 0x00, 0x9C }, { 0x15, 0x90 }, { 0x25, 0x95 }, { 0x23, 0x9E } };
static const XvSong aquaman_songs[] = {
	{ 0x90, 0x32, XN_BN4_ART + 0x7400, 0x40 },   /* its sound 0x90, BN4's song copied */
	{ 0x9E, 0x33, XN_BN4_ART + 0x7440, 0x40 },   /* its sound 0x9e, BN4's song copied */
	{ 0xA0, 0x63, XN_BN4_ART + 0x7480, 0x900 },   /* its sound 0xa0, BN4's song copied */
	{ 0xA1, 0xD0, XN_BN4_ART + 0x7D80, 0x40 },   /* its sound 0xa1, BN4's song copied */
	{ 0xC8, 0xE0, XN_BN4_ART + 0x7DC0, 0xB40 },   /* its sound 0xc8, BN4's song copied */
};
static const XnSpawn aquaman_spawns[] = {
	{ OWN(0x080CCF35), 0x1, 0x00000000, 0x00, 0x00, 0x00, 0x74 },   /* 080B826A: its first-kind 0x5c */
};
static const XnInit aquaman_init = { aquaman_spawns, 1, true, false, false, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update, its panel at its +6) */


static const XNavi bn4_aquaman = {
	.code = {
		.name = "AquaMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x0D,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 58,
		.first_id = XN_BN4_FIRST_ID + 0x28,
		.home = XN_BN4_HOMES + 0x5000,
		.blocks = aquaman_blocks, .nblocks = (int)(sizeof aquaman_blocks / sizeof *aquaman_blocks),
		.data = aquaman_data, .ndata = (int)(sizeof aquaman_data / sizeof *aquaman_data),
		.calls = aquaman_calls, .ncalls = (int)(sizeof aquaman_calls / sizeof *aquaman_calls),
		.words = aquaman_words, .nwords = (int)(sizeof aquaman_words / sizeof *aquaman_words),
		.imms = aquaman_imms, .nimms = (int)(sizeof aquaman_imms / sizeof *aquaman_imms),
		.numbers = aquaman_numbers, .nnumbers = (int)(sizeof aquaman_numbers / sizeof *aquaman_numbers),
		.bytes = aquaman_bytes, .nbytes = (int)(sizeof aquaman_bytes / sizeof *aquaman_bytes),
		.types = aquaman_types, .ntypes = (int)(sizeof aquaman_types / sizeof *aquaman_types),
		.states = aquaman_states, .nstates = (int)(sizeof aquaman_states / sizeof *aquaman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08012068, .struct2 = 0x08012068,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0xA8, 0xA9, 0xAA, 0xAB },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x00 },
		.sprites = aquaman_sprites, .nsprites = (int)(sizeof aquaman_sprites / sizeof *aquaman_sprites),
		.effects = aquaman_effects, .neffects = (int)(sizeof aquaman_effects / sizeof *aquaman_effects),
		.songs = aquaman_songs, .nsongs = (int)(sizeof aquaman_songs / sizeof *aquaman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.coll_arg = 0x21,   /* (its init's collision setup's r3, BN6's actor init's 0x3: the hit bits its body gives, read alike by both games' pre-steps) */
	.palettes = 0x02000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &aquaman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
};
/* ---- BN4's Roll (its type 0x1d there, its enemy ids 128-131), as Navi AI 48 with ids 0x380-0x383 ---- */

static const XvBlock roll_blocks[] = {
	{ 0x080BC0E4, 0x080BC3FC, XN_BN4_CODE + 0x95F0, 0x6626EF64u },
	{ 0x080E3A98, 0x080E3AE6, XN_BN4_CODE + 0x9910, 0xFF36B97Eu },
	{ 0x080CE6E0, 0x080CE840, XN_BN4_CODE + 0x9960, 0x5AA41525u },
	{ 0x080CD5F8, 0x080CD728, XN_BN4_CODE + 0x9AC0, 0x0069BF7Du },
	{ 0x080CD524, 0x080CD5F8, XN_BN4_CODE + 0x9BF0, 0x40F51A6Du },
	{ 0x080CE5B0, 0x080CE6E0, XN_BN4_CODE + 0x9CD0, 0x69329294u },
	{ 0x0800A244, 0x0800A2BE, XN_BN4_CODE + 0x9E00, 0x6300CCC3u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0x9E82, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0xA292, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0xA2B0, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0xA2C0, 0xDCE85B3Du },
	{ 0x080EF87C, 0x080EF8B4, XN_BN4_CODE + 0xA2F0, 0x909D30CEu },
	{ 0x080EF8B4, 0x080EF90A, XN_BN4_CODE + 0xA330, 0xD447727Cu },
	{ 0x080EF90A, 0x080EF944, XN_BN4_CODE + 0xA392, 0x97D23364u },
	{ 0x080EF944, 0x080EF962, XN_BN4_CODE + 0xA3D0, 0x3F481C11u },
	{ 0x080EF962, 0x080EF990, XN_BN4_CODE + 0xA3F2, 0xC28BAFD0u },
	{ 0x080EF990, 0x080EFA14, XN_BN4_CODE + 0xA420, 0x976788CDu },
	{ 0x080EFA14, 0x080EFA3E, XN_BN4_CODE + 0xA4B0, 0x2375C002u },
	{ 0x080EFA3E, 0x080EFA7C, XN_BN4_CODE + 0xA4E2, 0xFCC783F9u },
	{ 0x080EFA7C, 0x080EFA9A, XN_BN4_CODE + 0xA520, 0xB4138418u },
	{ 0x080EFA9A, 0x080EFAC0, XN_BN4_CODE + 0xA542, 0x597DAEFFu },
	{ 0x080EFAC0, 0x080EFAF0, XN_BN4_CODE + 0xA570, 0xE180FD23u },
	{ 0x080EFAF0, 0x080EFB3E, XN_BN4_CODE + 0xA5A0, 0x7A26BE4Du },
	{ 0x080EFB3E, 0x080EFB60, XN_BN4_CODE + 0xA5F2, 0x246CC93Eu },
	{ 0x080EFB60, 0x080EFB90, XN_BN4_CODE + 0xA620, 0xDACF03A9u },
	{ 0x080EFB90, 0x080EFBD2, XN_BN4_CODE + 0xA650, 0xB6CCB635u },
	{ 0x080EFBD2, 0x080EFC10, XN_BN4_CODE + 0xA6A2, 0x78707E9Fu },
	{ 0x080EFC10, 0x080EFC42, XN_BN4_CODE + 0xA6E0, 0xB0FF32BEu },
	{ 0x080EFC42, 0x080EFC9E, XN_BN4_CODE + 0xA722, 0x5F56DDE2u },
	{ 0x080EFC9E, 0x080EFCE2, XN_BN4_CODE + 0xA782, 0x6F46462Au },
	{ 0x080EFCE2, 0x080EFD06, XN_BN4_CODE + 0xA7D2, 0x74AE3F84u },
	{ 0x080EFD06, 0x080EFD3C, XN_BN4_CODE + 0xA802, 0x485979B8u },
	{ 0x080EFD3C, 0x080EFD72, XN_BN4_CODE + 0xA840, 0xF963E758u },
	{ 0x080EFD72, 0x080EFDB0, XN_BN4_CODE + 0xA882, 0xBA320A81u },
	{ 0x080EFDB0, 0x080EFE00, XN_BN4_CODE + 0xA8C0, 0x58062622u },
	{ 0x080EFE00, 0x080EFE36, XN_BN4_CODE + 0xA910, 0xED4CA13Au },
	{ 0x080EFE36, 0x080EFE62, XN_BN4_CODE + 0xA952, 0x9B23EB05u },
	{ 0x080EFE62, 0x080EFEA4, XN_BN4_CODE + 0xA982, 0x3B90D376u },
	{ 0x080EFEA4, 0x080EFEC8, XN_BN4_CODE + 0xA9D0, 0x81B2DD96u },
	{ 0x080EFEC8, 0x080EFEEC, XN_BN4_CODE + 0xAA00, 0xBAE07B24u },
	{ 0x080EFEEC, 0x080EFF74, XN_BN4_CODE + 0xAA30, 0xB6435623u },
	{ 0x080EFF74, 0x080EFF98, XN_BN4_CODE + 0xAAC0, 0x6540E1D4u },
	{ 0x080EFF98, 0x080F01C0, XN_BN4_CODE + 0xAAF0, 0x1D5C7220u },
};
static const XvData roll_data[] = {
	{ 0x080BC0E4, 0x080BC0F6, XV_ZERO },
	{ 0x080BC0F6, 0x080BC0F8, XV_RAW },
	{ 0x080BC0F8, 0x080BC17E, XV_ZERO },
	{ 0x080BC17E, 0x080BC180, XV_RAW },
	{ 0x080BC180, 0x080BC1B4, XV_ZERO },
	{ 0x080BC1D6, 0x080BC1D8, XV_RAW },
	{ 0x080BC1D8, 0x080BC1DC, XV_POOL },
	{ 0x080BC1DC, 0x080BC1E8, XV_TABLE },
	{ 0x080BC1F6, 0x080BC1F8, XV_RAW },
	{ 0x080BC1F8, 0x080BC1FC, XV_POOL },
	{ 0x080BC1FC, 0x080BC204, XV_TABLE },
	{ 0x080BC282, 0x080BC284, XV_RAW },
	{ 0x080BC284, 0x080BC288, XV_POOL },
	{ 0x080BC288, 0x080BC298, XV_RAW },
	{ 0x080BC2C0, 0x080BC2C8, XV_POOL },
	{ 0x080BC2C8, 0x080BC2D0, XV_RAW },
	{ 0x080BC2F8, 0x080BC300, XV_POOL },
	{ 0x080BC300, 0x080BC308, XV_RAW },
	{ 0x080BC39E, 0x080BC3A0, XV_RAW },
	{ 0x080BC3A0, 0x080BC3A4, XV_POOL },
	{ 0x080BC3A4, 0x080BC3B4, XV_RAW },
	{ 0x080BC3D2, 0x080BC3D4, XV_RAW },
	{ 0x080BC3D4, 0x080BC3D8, XV_POOL },
	{ 0x080BC3D8, 0x080BC3E8, XV_POOL },
	{ 0x080BC3E8, 0x080BC3F8, XV_RAW },
	{ 0x080BC3F8, 0x080BC3FC, XV_POOL },
	{ 0x080CE6F2, 0x080CE6F4, XV_RAW },
	{ 0x080CE6F4, 0x080CE6F8, XV_POOL },
	{ 0x080CE6F8, 0x080CE704, XV_TABLE },
	{ 0x080CE828, 0x080CE83C, XV_RAW },
	{ 0x080CE83C, 0x080CE840, XV_POOL },
	{ 0x080CD606, 0x080CD608, XV_RAW },
	{ 0x080CD608, 0x080CD60C, XV_POOL },
	{ 0x080CD60C, 0x080CD618, XV_TABLE },
	{ 0x080CD708, 0x080CD720, XV_RAW },
	{ 0x080CD720, 0x080CD728, XV_POOL },
	{ 0x080CD524, 0x080CD5F8, XV_RAW },
	{ 0x080CE5B0, 0x080CE6E0, XV_RAW },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x080EF890, 0x080EF894, XV_POOL },
	{ 0x080EF894, 0x080EF8B4, XV_TABLE },
	{ 0x080EFA08, 0x080EFA10, XV_POOL },
	{ 0x080EFA10, 0x080EFA14, XV_RAW },
	{ 0x080EFAAE, 0x080EFAB0, XV_ZERO },
	{ 0x080EFAB0, 0x080EFAB4, XV_POOL },
	{ 0x080EFAB4, 0x080EFAC0, XV_TABLE },
	{ 0x080EFB74, 0x080EFB78, XV_POOL },
	{ 0x080EFB78, 0x080EFB90, XV_TABLE },
	{ 0x080EFD1A, 0x080EFD1C, XV_ZERO },
	{ 0x080EFD1C, 0x080EFD20, XV_POOL },
	{ 0x080EFD20, 0x080EFD3C, XV_TABLE },
	{ 0x080EFEDC, 0x080EFEE0, XV_POOL },
	{ 0x080EFEE0, 0x080EFEEC, XV_TABLE },
	{ 0x080EFF2C, 0x080EFF34, XV_ZERO },
	{ 0x080F0062, 0x080F0064, XV_ZERO },
	{ 0x080F0064, 0x080F0068, XV_POOL },
	{ 0x080F0068, 0x080F0078, XV_RAW },
	{ 0x080F00E0, 0x080F00E4, XV_POOL },
	{ 0x080F00E4, 0x080F00F4, XV_RAW },
	{ 0x080F00F4, 0x080F01B8, XV_ZERO },
	{ 0x080F01B8, 0x080F01C0, XV_POOL },
};
static const XvPair roll_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002CEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08002D4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E20, 0x08002DB4 },   /* project:fireman, SAME */
	{ 0x08002E34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002F22, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002F2E, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FD0, 0x08002F7E },   /* map:bytes, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08003344, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x0800A000, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x0800A0A2, 0x0800CFA6 },   /* project:spikey, SAME */
	{ 0x0800A0D8, 0x0800CFDC },   /* project:research, SAME */
	{ 0x0800A10E, 0x0800D012 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800AB8A, 0x0800E2C2 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AE10, 0x0800E6E8 },   /* project:families-2, SAME */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x0800C456, 0x080103BC },   /* project:gutsman, DIFF (audited) */
	{ 0x08010D4C, 0x080170C4 },   /* BN4's generic Navi state 2, its own state 2's tail: BN6's generic Navi state 2 (deleted), AIMED (audited) */
	{ 0x08010FA6, 0x08017888 },   /* BN4's generic Navi state 1, its own state 1's tail: BN6's generic Navi state 1 (on to its first state), AIMED (audited) */
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
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x08014620, 0x0801BDDE },   /* map:bytes, SAME */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ 0x080F00F4, XN_BN4_CODE + 0x94E8 },   /* BN4_SPAWN_ENEMY: the summon's shim (xnavi_init.c), BN6's BN6_SPAWN_ENEMY_1 by the id BN6 numbers it */
};
static const XvImm roll_imms[] = {
	{ 0x0800A27A, THUMB_LDRB, 1, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BC1C4, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080BC1C8, THUMB_ADDS, 6, 0, 0x64, 0x80 },   /* its AI data's +0x64 in BN4, +0x80 in BN6 (AI_BLOCKS) */
	{ 0x080BC210, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080BC228, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 set (BN4_SET_ACTION's r0) */
	{ 0x080BC24A, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 set (BN4_SET_ACTION's r0) */
	{ 0x080BC2B4, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080BC2EC, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080CD6E4, THUMB_MOVS, 0, 0, 0x2, 0xFD },   /* its object's type 0x2: moved, the dispatch's 0xfd */
	{ 0x080CE74A, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE79A, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE7D6, THUMB_MOVS, 4, 0, 0x56, 0x99 },   /* its effect: 0x56 in BN4, 0x99 in BN6 (moved) */
	{ 0x080CE812, THUMB_MOVS, 0, 0, 0xB, 0xFE },   /* its object's type 0xb: moved, the dispatch's 0xfe */
	{ 0x080E3ABE, THUMB_ADDS, 4, 0, 0x2E, 0x9A },   /* its effect: 0x2e in BN4, 0x9a in BN6 (moved) */
	{ 0x080E3ADC, THUMB_MOVS, 0, 0, 0x17, 0xE3 },   /* its object's type 0x17: moved, the dispatch's 0xe3 */
	{ 0x080EF880, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080EF99A, THUMB_MOVS, 0, 0, 0x32, 0x63 },   /* its sound 0x131 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080EF99C, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x131 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080EFA9E, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080EFB24, THUMB_MOVS, 0, 0, 0x33, 0xD0 },   /* its sound 0x132 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080EFB26, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x132 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080EFB64, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080EFC7E, THUMB_MOVS, 0, 0, 0x96, 0x32 },   /* its sound 0x96: BN4's copied into BN6's empty slot 0x32, on its player 22 */
	{ 0x080EFD0A, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080EFDD6, THUMB_MOVS, 0, 0, 0x96, 0x32 },   /* its sound 0x96: BN4's copied into BN6's empty slot 0x32, on its player 22 */
	{ 0x080EFECC, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
};
static const XvNumber roll_numbers[] = {
	{ 0x080EFA08, 0x04050001, 0x04050001 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4, Roll's MegaMan's state 44 at 0x080EF9EC, as 0x04050301): its bytes as 0x04050301's, its third byte 0 */
};
static const XvPair roll_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvByte roll_bytes[] = {
	{ 0x080CD540, 0x0C, 0x10 },   /* its table's kind 7: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CD541, 0x0F, 0xE4 },   /* its table's kind 7: list 3 0xf in BN4, list 4 0xe4 copied */
	{ 0x080CE615, 0x0C, 0x10 },   /* its table's kind 6: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE616, 0x10, 0xE5 },   /* its table's kind 6: list 3 0x10 in BN4, list 4 0xe5 copied */
	{ 0x080CE625, 0x0C, 0x10 },   /* its table's kind 7: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE626, 0x10, 0xE5 },   /* its table's kind 7: list 3 0x10 in BN4, list 4 0xe5 copied */
	{ 0x080CE635, 0x0C, 0x10 },   /* its table's kind 8: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE636, 0x10, 0xE5 },   /* its table's kind 8: list 3 0x10 in BN4, list 4 0xe5 copied */
	{ 0x080CE645, 0x0C, 0x10 },   /* its table's kind 9: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE646, 0x10, 0xE5 },   /* its table's kind 9: list 3 0x10 in BN4, list 4 0xe5 copied */
};
static const XvType roll_types[] = {
	{ 3, 0xFD, XV_TYPE_MOVED, 0x02 },   /* BN4's third-kind 0x2, moved, the dispatch's 0xfd */
	{ 3, 0xFE, XV_TYPE_MOVED, 0x0B },   /* BN4's third-kind 0xb, moved, the dispatch's 0xfe */
	{ 4, 0xE3, XV_TYPE_MOVED, 0x17 },   /* BN4's fourth-kind 0x17, moved, the dispatch's 0xe3 */
};
static const XvRef roll_states[] = {
	BN6(0x08016381), OWN(0x080BC1B5), OWN(0x080BC309), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080BC1C1), OWN(0x080EF87D), OWN(0x080EFA9B), OWN(0x080EFB61), OWN(0x080EFD07), OWN(0x080EFEC9),
};
static const XvSprite roll_sprites[] = {
	{ 2, 0x01, 2, 0x40, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x0F, 4, 0xE4, XN_BN4_ART + 0x0, 0x1000 },
	{ 3, 0x10, 4, 0xE5, XN_BN4_ART + 0x1000, 0x800 },
};
static const XvId roll_effects[] = { { 0x2E, 0x9A }, { 0x56, 0x99 } };
static const XvSong roll_songs[] = {
	{ 0x96, 0x32, XN_BN4_ART + 0x1800, 0x21C0, 10 },   /* its sound 0x96, BN4's song copied, on BN6's player 10 */
	{ 0xCE, 0x33, XN_BN4_ART + 0x39C0, 0x21C0 },   /* its sound 0xce, BN4's song copied */
	{ 0x131, 0x63, XN_BN4_ART + 0x5B80, 0x740, 10 },   /* its sound 0x131, BN4's song copied, on BN6's player 10 */
	{ 0x132, 0xD0, XN_BN4_ART + 0x62C0, 0xC80 },   /* its sound 0x132, BN4's song copied */
	{ 0x8B, 0xE0, XN_BN4_ART + 0x6F40, 0x40 },   /* its sound 0x8b, BN4's song copied */
	{ 0xC2, 0x104, XN_BN4_ART + 0x6F80, 0x40 },   /* its sound 0xc2, BN4's song copied */
	{ 0xC3, 0x117, XN_BN4_ART + 0x6FC0, 0x40 },   /* its sound 0xc3, BN4's song copied */
};
static const XnInit roll_init = { NULL, 0, false, false, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */
static const XnSummonId roll_summon_ids[] = {
	{ 0x01, 0x2E4 },   /* Mettaur's, moved (its recipe's ids) */
	{ 0x02, 0x2E5 },   /* Mettaur's, moved (its recipe's ids) */
	{ 0x03, 0x2E6 },   /* Mettaur's, moved (its recipe's ids) */
	{ 0x04, 0x2E7 },   /* Mettaur's, moved (its recipe's ids) */
	{ 0x0E, 0x2C1 },   /* Billy's, moved (its recipe's ids) */
	{ 0x0F, 0x2C2 },   /* Billy's, moved (its recipe's ids) */
	{ 0x10, 0x2C3 },   /* Billy's, moved (its recipe's ids) */
	{ 0x39, 0x2EC },   /* Melody's, moved (its recipe's ids) */
};
static const XnSummon roll_summon = { XN_BN4_CODE + 0x94E8, OWN(0x0800A245), { 0x00000010, 0x0F8800A0, 0x00000030, 0x0F880080 }, roll_summon_ids, (int)(sizeof roll_summon_ids / sizeof *roll_summon_ids), 0x33 };   /* (BN4_SPAWN_ENEMY's masks by side, 080F01A8; its sound 0xce) */


static const XNavi bn4_roll = {
	.code = {
		.name = "Roll (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x1D,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 48,
		.first_id = XN_BN4_FIRST_ID + 0x0,
		.home = XN_BN4_HOMES + 0x0,
		.blocks = roll_blocks, .nblocks = (int)(sizeof roll_blocks / sizeof *roll_blocks),
		.data = roll_data, .ndata = (int)(sizeof roll_data / sizeof *roll_data),
		.calls = roll_calls, .ncalls = (int)(sizeof roll_calls / sizeof *roll_calls),
		.words = roll_words, .nwords = (int)(sizeof roll_words / sizeof *roll_words),
		.imms = roll_imms, .nimms = (int)(sizeof roll_imms / sizeof *roll_imms),
		.numbers = roll_numbers, .nnumbers = (int)(sizeof roll_numbers / sizeof *roll_numbers),
		.bytes = roll_bytes, .nbytes = (int)(sizeof roll_bytes / sizeof *roll_bytes),
		.types = roll_types, .ntypes = (int)(sizeof roll_types / sizeof *roll_types),
		.states = roll_states, .nstates = (int)(sizeof roll_states / sizeof *roll_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08011E88, .struct2 = 0x08011E88,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0x80, 0x81, 0x82, 0x83 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x04 },
		.sprites = roll_sprites, .nsprites = (int)(sizeof roll_sprites / sizeof *roll_sprites),
		.effects = roll_effects, .neffects = (int)(sizeof roll_effects / sizeof *roll_effects),
		.songs = roll_songs, .nsongs = (int)(sizeof roll_songs / sizeof *roll_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = OWN(0x080BC341),   /* (its hook A its update's call before its pre-step) */
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.palettes = 0x04000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &roll_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
	.summon = &roll_summon,   /* (its summon: xnavi_init.c) */
};
/* ---- BN4's WoodMan (its type 0x46 there, its enemy ids 172-175), as Navi AI 59 with ids 0x3ac-0x3af ---- */
/* interim: its sound 0x8f plays 1 of its 2 tracks: BN6's player 30 has 1 where BN4's has 2 (BN6_MPLAYS against BN4_MPLAYS), the rest let go as BN6's MPlayStart starts no more tracks than its player has */

static const XvBlock woodman_blocks[] = {
	{ 0x080C8EC0, 0x080C91A8, XN_BN4_CODE + 0xAD20, 0xD063ED2Fu },
	{ 0x080CE6E0, 0x080CE840, XN_BN4_CODE + 0xB010, 0x5AA41525u },
	{ 0x080CDEB8, 0x080CE0A4, XN_BN4_CODE + 0xB170, 0x63D210C7u },
	{ 0x080DF9B0, 0x080DFA78, XN_BN4_CODE + 0xB360, 0xECE00DD1u },
	{ 0x080DFA78, 0x080DFD54, XN_BN4_CODE + 0xB430, 0x8020F70Eu },
	{ 0x080CE5B0, 0x080CE6E0, XN_BN4_CODE + 0xB710, 0x69329294u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0xB842, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0xBC52, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0xBC70, 0xA9D6138Du },
	{ 0x0800C9FC, 0x0800CA28, XN_BN4_CODE + 0xBC80, 0x3A5341C2u },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0xBCB0, 0xDCE85B3Du },
	{ 0x0800CA4A, 0x0800CA60, XN_BN4_CODE + 0xBCE2, 0xC3C68B85u },
	{ 0x0800F734, 0x0800F780, XN_BN4_CODE + 0xBD00, 0x0198FB6Eu },
	{ 0x080EE752, 0x080EE774, XN_BN4_CODE + 0xBD52, 0xFF58F881u },
	{ 0x080F4C00, 0x080F4C24, XN_BN4_CODE + 0xBD80, 0xB875F795u },
	{ 0x080F4C24, 0x080F4C9C, XN_BN4_CODE + 0xBDB0, 0xACCEA440u },
	{ 0x080F4C9C, 0x080F4D7C, XN_BN4_CODE + 0xBE30, 0x26D95F1Cu },
	{ 0x080F4D7C, 0x080F4DA0, XN_BN4_CODE + 0xBF10, 0x05BBE887u },
	{ 0x080F4DA0, 0x080F4DCC, XN_BN4_CODE + 0xBF40, 0x4B3019A4u },
	{ 0x080F4DCC, 0x080F4E4C, XN_BN4_CODE + 0xBF70, 0x5FA17F35u },
	{ 0x080F4E4C, 0x080F4F08, XN_BN4_CODE + 0xBFF0, 0x40C2375Au },
	{ 0x080F4F08, 0x080F4F22, XN_BN4_CODE + 0xC0B0, 0xAF249139u },
	{ 0x080F4F22, 0x080F4F50, XN_BN4_CODE + 0xC0D2, 0x2466B374u },
	{ 0x080F4F50, 0x080F5024, XN_BN4_CODE + 0xC100, 0xE86DB124u },
	{ 0x080F5024, 0x080F5044, XN_BN4_CODE + 0xC1E0, 0x7E56FBFBu },
	{ 0x080F5044, 0x080F507C, XN_BN4_CODE + 0xC200, 0x4A532348u },
	{ 0x080F507C, 0x080F509C, XN_BN4_CODE + 0xC240, 0x82DBF945u },
	{ 0x080F509C, 0x080F50F0, XN_BN4_CODE + 0xC260, 0x5A881F9Bu },
	{ 0x080F50F0, 0x080F5100, XN_BN4_CODE + 0xC2C0, 0x5ABD7887u },
	{ 0x080F5100, 0x080F5124, XN_BN4_CODE + 0xC2D0, 0xD56A24A4u },
};
static const XvData woodman_data[] = {
	{ 0x080C8EC0, 0x080C8ED2, XV_ZERO },
	{ 0x080C8ED2, 0x080C8ED4, XV_RAW },
	{ 0x080C8ED4, 0x080C8F80, XV_ZERO },
	{ 0x080C8F96, 0x080C8F98, XV_RAW },
	{ 0x080C8F98, 0x080C8F9C, XV_POOL },
	{ 0x080C8F9C, 0x080C8FA0, XV_TABLE },
	{ 0x080C8FAE, 0x080C8FB0, XV_RAW },
	{ 0x080C8FB0, 0x080C8FB4, XV_POOL },
	{ 0x080C8FB4, 0x080C8FC0, XV_TABLE },
	{ 0x080C902C, 0x080C9030, XV_POOL },
	{ 0x080C9030, 0x080C9034, XV_RAW },
	{ 0x080C9082, 0x080C9084, XV_RAW },
	{ 0x080C9084, 0x080C9088, XV_POOL },
	{ 0x080C9088, 0x080C908C, XV_RAW },
	{ 0x080C90D4, 0x080C90D8, XV_POOL },
	{ 0x080C90D8, 0x080C90DC, XV_RAW },
	{ 0x080C911A, 0x080C911C, XV_RAW },
	{ 0x080C911C, 0x080C9120, XV_POOL },
	{ 0x080C9164, 0x080C9168, XV_POOL },
	{ 0x080C9168, 0x080C9178, XV_RAW },
	{ 0x080C9190, 0x080C9194, XV_POOL },
	{ 0x080C9194, 0x080C9198, XV_RAW },
	{ 0x080C9198, 0x080C91A8, XV_POOL },
	{ 0x080CE6F2, 0x080CE6F4, XV_RAW },
	{ 0x080CE6F4, 0x080CE6F8, XV_POOL },
	{ 0x080CE6F8, 0x080CE704, XV_TABLE },
	{ 0x080CE828, 0x080CE83C, XV_RAW },
	{ 0x080CE83C, 0x080CE840, XV_POOL },
	{ 0x080CDECA, 0x080CDECC, XV_RAW },
	{ 0x080CDECC, 0x080CDED0, XV_POOL },
	{ 0x080CDED0, 0x080CDEDC, XV_TABLE },
	{ 0x080CDF4E, 0x080CDF50, XV_RAW },
	{ 0x080CDF50, 0x080CDF54, XV_POOL },
	{ 0x080CDF54, 0x080CDF5C, XV_TABLE },
	{ 0x080CDFA8, 0x080CDFAC, XV_POOL },
	{ 0x080CDFBA, 0x080CDFBC, XV_RAW },
	{ 0x080CDFBC, 0x080CDFC0, XV_POOL },
	{ 0x080CDFC0, 0x080CDFCC, XV_TABLE },
	{ 0x080CE074, 0x080CE0A4, XV_RAW },
	{ 0x080DF9C2, 0x080DF9C4, XV_RAW },
	{ 0x080DF9C4, 0x080DF9C8, XV_POOL },
	{ 0x080DF9C8, 0x080DF9D4, XV_TABLE },
	{ 0x080DFA8A, 0x080DFA8C, XV_RAW },
	{ 0x080DFA8C, 0x080DFA90, XV_POOL },
	{ 0x080DFA90, 0x080DFA9C, XV_TABLE },
	{ 0x080DFB32, 0x080DFB34, XV_RAW },
	{ 0x080DFB34, 0x080DFB38, XV_POOL },
	{ 0x080DFB38, 0x080DFB48, XV_RAW },
	{ 0x080DFC0C, 0x080DFC10, XV_POOL },
	{ 0x080DFC10, 0x080DFC18, XV_TABLE },
	{ 0x080DFC18, 0x080DFC1C, XV_POOL },
	{ 0x080DFD0A, 0x080DFD10, XV_RAW },
	{ 0x080DFD10, 0x080DFD14, XV_POOL },
	{ 0x080DFD48, 0x080DFD54, XV_POOL },
	{ 0x080CE5B0, 0x080CE6E0, XV_RAW },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0800F76C, 0x0800F770, XV_POOL },
	{ 0x0800F770, 0x0800F780, XV_RAW },
	{ 0x080F4C14, 0x080F4C18, XV_POOL },
	{ 0x080F4C18, 0x080F4C24, XV_TABLE },
	{ 0x080F4C94, 0x080F4C9C, XV_POOL },
	{ 0x080F4D72, 0x080F4D74, XV_ZERO },
	{ 0x080F4D74, 0x080F4D7C, XV_POOL },
	{ 0x080F4DB2, 0x080F4DB4, XV_ZERO },
	{ 0x080F4DB4, 0x080F4DB8, XV_POOL },
	{ 0x080F4DB8, 0x080F4DCC, XV_TABLE },
	{ 0x080F4E44, 0x080F4E4C, XV_POOL },
	{ 0x080F4EFE, 0x080F4F00, XV_ZERO },
	{ 0x080F4F00, 0x080F4F08, XV_POOL },
	{ 0x080F5010, 0x080F5014, XV_POOL },
	{ 0x080F5014, 0x080F5024, XV_RAW },
	{ 0x080F5036, 0x080F5038, XV_ZERO },
	{ 0x080F5038, 0x080F503C, XV_POOL },
	{ 0x080F503C, 0x080F5044, XV_TABLE },
	{ 0x080F508A, 0x080F508C, XV_ZERO },
	{ 0x080F508C, 0x080F5090, XV_POOL },
	{ 0x080F5090, 0x080F509C, XV_TABLE },
	{ 0x080F50E0, 0x080F50E4, XV_POOL },
	{ 0x080F50E4, 0x080F50E8, XV_RAW },
	{ 0x080F50E8, 0x080F50F0, XV_POOL },
};
static const XvPair woodman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08000AAA, 0x08000C72 },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800A10E, 0x0800D012 },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800AB20, 0x0800E258 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AE10, 0x0800E6E8 },   /* project:families-2, SAME */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
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
	{ 0x08012F7A, 0x0801A146 },   /* project:fireman, DIFF (audited) */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080144FE, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0802608C, 0x080302A8 },   /* project:research, SAME */
	{ 0x080406A4, 0x0800A03C },   /* map:calls, DIFF (audited) */
	{ 0x080B3878, 0x0800EC48 },   /* project:research, SAME */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm woodman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA06, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA1E, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x0800CA4E, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA56, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080C8F84, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C8F88, THUMB_ADDS, 6, 0, 0x64, 0x80 },   /* its AI data's +0x64 in BN4, +0x80 in BN6 (AI_BLOCKS) */
	{ 0x080C9020, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 set (BN4_SET_ACTION's r0) */
	{ 0x080C904A, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C9078, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080C90A2, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C90CC, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080C90F2, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C9112, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080CDEDE, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080CDEE0, THUMB_MOVS, 2, 0, 0xB, 0xE6 },   /* its object's sprite: list 3 0xb in BN4, list 4 0xe6 in BN6 (copied) */
	{ 0x080CDF34, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CDFF8, THUMB_MOVS, 0, 0, 0x3F, 0xD0 },   /* its sound 0x13e (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080CDFFA, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x13e (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080CE056, THUMB_MOVS, 0, 0, 0x7, 0xFF },   /* its object's type 0x7: moved, the dispatch's 0xff */
	{ 0x080CE066, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE74A, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE79A, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE7D6, THUMB_MOVS, 4, 0, 0x56, 0x99 },   /* its effect: 0x56 in BN4, 0x99 in BN6 (moved) */
	{ 0x080CE812, THUMB_MOVS, 0, 0, 0xB, 0xCC },   /* its object's type 0xb: moved, the dispatch's 0xcc */
	{ 0x080DFA3C, THUMB_MOVS, 0, 0, 0x94, 0xCD },   /* its object's type 0x94: moved, the dispatch's 0xcd */
	{ 0x080DFA50, THUMB_STRB, 1, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DFA6A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DFAA0, THUMB_MOVS, 2, 0, 0x20, 0xE8 },   /* its object's sprite: list 4 0x20 in BN4, list 4 0xe8 in BN6 (copied) */
	{ 0x080DFB76, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DFBF6, THUMB_MOVS, 4, 0, 0xF, 0x97 },   /* its effect: 0xf in BN4, 0x97 in BN6 (moved) */
	{ 0x080DFBFC, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's empty slot 0x3a, on its player 9 */
	{ 0x080DFC2C, THUMB_MOVS, 0, 0, 0xC7, 0x63 },   /* its sound 0xc7: BN4's copied into BN6's empty slot 0x63, on its player 9 */
	{ 0x080DFC52, THUMB_MOVS, 0, 0, 0x4C, 0xE0 },   /* its sound 0x14b (movs and adds #0xff) at the pool's 0xe0 */
	{ 0x080DFC54, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x14b (movs and adds #0xff) at the pool's 0xe0 */
	{ 0x080DFC88, THUMB_MOVS, 4, 0, 0x13, 0x92 },   /* its effect: 0x13 in BN4, 0x92 in BN6 (moved) */
	{ 0x080DFCA8, THUMB_MOVS, 0, 0, 0x95, 0xCE },   /* its object's type 0x95: moved, the dispatch's 0xce */
	{ 0x080DFCB8, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080EE756, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F4C04, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F4D42, THUMB_MOVS, 0, 0, 0xA2, 0x33 },   /* its sound 0xa2: BN4's copied into BN6's empty slot 0x33, on its player 30 */
	{ 0x080F4DA4, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F4EE0, THUMB_MOVS, 0, 0, 0xA2, 0x33 },   /* its sound 0xa2: BN4's copied into BN6's empty slot 0x33, on its player 30 */
	{ 0x080F5028, THUMB_ADDS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F50BA, THUMB_MOVS, 0, 0, 0x8F, 0x32 },   /* its sound 0x8f: BN4's copied into BN6's empty slot 0x32, on its player 30 */
};
static const XvNumber woodman_numbers[] = {
	{ 0x080CDFA8, 0x03840000, 0x03840000 },   /* a panel's occupants' bits WoodMan's third kind 0x07 may not stand on (0x080CDF82, its panel's flags by 0x08009A86, BN6's 0x0800C8F8): each game's panel flags OR in their occupants' alliance words' high halves (BN4's collision +0x58 at 0x0800A410, BN6's +0x30 at IWRAM 0x03007974), the same words of both games' collision masks' tables (BN4_COLL_MASKS, BN6_COLL_MASKS: 0x2A80, 0x5584 alike) */
	{ 0x080DFD4C, 0x0C000000, 0x0C000000 },   /* the hits word's bits WoodMan's third kind 0x95 tests (0x080DFB78, its collision's +0x54, BN6's +0x70): the attackers' alliance words ORed in by both games' hit passes (0x08012BE8, BN6's 0x081D7840), as 0x0A800000 */
};
static const XvPair woodman_words[] = {
	{ 0x080033D5, 0x08003459 },   /* project:research, SAME */
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvByte woodman_bytes[] = {
	{ 0x080CE6B6, 0x3D, 0xE7 },   /* its table's kind 16: list 4 0x3d in BN4, 0xe7 copied */
	{ 0x080CE6C6, 0x3D, 0xE7 },   /* its table's kind 17: list 4 0x3d in BN4, 0xe7 copied */
	{ 0x080CE6D6, 0x3D, 0xE7 },   /* its table's kind 18: list 4 0x3d in BN4, 0xe7 copied */
};
static const XvType woodman_types[] = {
	{ 3, 0xFF, XV_TYPE_MOVED, 0x07 },   /* BN4's third-kind 0x7, moved, the dispatch's 0xff */
	{ 3, 0xCC, XV_TYPE_MOVED, 0x0B },   /* BN4's third-kind 0xb, moved, the dispatch's 0xcc */
	{ 3, 0xCD, XV_TYPE_MOVED, 0x94 },   /* BN4's third-kind 0x94, moved, the dispatch's 0xcd */
	{ 3, 0xCE, XV_TYPE_MOVED, 0x95 },   /* BN4's third-kind 0x95, moved, the dispatch's 0xce */
};
static const XvRef woodman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C8F81), OWN(0x080EE753), OWN(0x080F4C01), OWN(0x080F4DA1), OWN(0x080F5025),
};
static const XvSprite woodman_sprites[] = {
	{ 2, 0x0C, 2, 0x4B, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x0B, 4, 0xE6, XN_BN4_ART + 0x0, 0x1800 },
	{ 4, 0x3D, 4, 0xE7, XN_BN4_ART + 0x1800, 0x800 },
	{ 4, 0x20, 4, 0xE8, XN_BN4_ART + 0x2000, 0x1000 },
};
static const XvId woodman_effects[] = { { 0x0F, 0x97 }, { 0x13, 0x92 }, { 0x56, 0x99 } };
static const XvSong woodman_songs[] = {
	{ 0x8F, 0x32, XN_BN4_ART + 0x3000, 0x80 },   /* its sound 0x8f, BN4's song copied */
	{ 0xA2, 0x33, XN_BN4_ART + 0x3080, 0x22C0 },   /* its sound 0xa2, BN4's song copied */
	{ 0xC7, 0x63, XN_BN4_ART + 0x5340, 0x40 },   /* its sound 0xc7, BN4's song copied */
	{ 0x13E, 0xD0, XN_BN4_ART + 0x5380, 0x40 },   /* its sound 0x13e, BN4's song copied */
	{ 0x14B, 0xE0, XN_BN4_ART + 0x53C0, 0xB40 },   /* its sound 0x14b, BN4's song copied */
};
static const XnInit woodman_init = { NULL, 0, false, true, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */
static const XnFar woodman_fars[] = {
	{ 0x080DFB8E, 0, 7, 0x28, 0x80 },   /* collision data +0x28 in BN4, +0x80 in BN6, past an ldrh's reach: its load and compare with 0 a shim's (its routine 080DFB58 pushes lr) */
};


static const XNavi bn4_woodman = {
	.code = {
		.name = "WoodMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x46,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 59,
		.first_id = XN_BN4_FIRST_ID + 0x2C,
		.home = XN_BN4_HOMES + 0x5800,
		.blocks = woodman_blocks, .nblocks = (int)(sizeof woodman_blocks / sizeof *woodman_blocks),
		.data = woodman_data, .ndata = (int)(sizeof woodman_data / sizeof *woodman_data),
		.calls = woodman_calls, .ncalls = (int)(sizeof woodman_calls / sizeof *woodman_calls),
		.words = woodman_words, .nwords = (int)(sizeof woodman_words / sizeof *woodman_words),
		.imms = woodman_imms, .nimms = (int)(sizeof woodman_imms / sizeof *woodman_imms),
		.numbers = woodman_numbers, .nnumbers = (int)(sizeof woodman_numbers / sizeof *woodman_numbers),
		.bytes = woodman_bytes, .nbytes = (int)(sizeof woodman_bytes / sizeof *woodman_bytes),
		.types = woodman_types, .ntypes = (int)(sizeof woodman_types / sizeof *woodman_types),
		.states = woodman_states, .nstates = (int)(sizeof woodman_states / sizeof *woodman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08012098, .struct2 = 0x08012098,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0xAC, 0xAD, 0xAE, 0xAF },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x00 },
		.sprites = woodman_sprites, .nsprites = (int)(sizeof woodman_sprites / sizeof *woodman_sprites),
		.effects = woodman_effects, .neffects = (int)(sizeof woodman_effects / sizeof *woodman_effects),
		.songs = woodman_songs, .nsongs = (int)(sizeof woodman_songs / sizeof *woodman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.palettes = 0x04000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &woodman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
	.fars = woodman_fars, .nfars = (int)(sizeof woodman_fars / sizeof *woodman_fars),   /* (its loads past an immediate's reach, through shims of its home) */
};
/* ---- BN4's JunkMan (its type 0x47 there, its enemy ids 164-167), as Navi AI 57 with ids 0x3a4-0x3a7 ---- */

static const XvBlock junkman_blocks[] = {
	{ 0x080C91A8, 0x080C95E8, XN_BN4_CODE + 0xC300, 0xAE4BAC9Fu },
	{ 0x080DFD54, 0x080DFF8C, XN_BN4_CODE + 0xC740, 0x20B94FDAu },
	{ 0x080E018C, 0x080E0318, XN_BN4_CODE + 0xC980, 0x377B5C3Fu },
	{ 0x080E05F8, 0x080E06AC, XN_BN4_CODE + 0xCB10, 0xA2BB237Au },
	{ 0x080DFF8C, 0x080E018C, XN_BN4_CODE + 0xCBD0, 0xC72A04F4u },
	{ 0x0800AD2A, 0x0800B130, XN_BN4_CODE + 0xCDD2, 0x2BC6F75Du },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0xD1E2, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0xD200, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0xD210, 0xDCE85B3Du },
	{ 0x080D97F8, 0x080D9808, XN_BN4_CODE + 0xD240, 0x1F57393Cu },
	{ 0x080DF332, 0x080DF36C, XN_BN4_CODE + 0xD252, 0x060C3CC2u },
	{ 0x080F401C, 0x080F4040, XN_BN4_CODE + 0xD290, 0x3399F950u },
	{ 0x080F4040, 0x080F406A, XN_BN4_CODE + 0xD2C0, 0xD7AAF444u },
	{ 0x080F406A, 0x080F40A8, XN_BN4_CODE + 0xD2F2, 0xD9832D73u },
	{ 0x080F40A8, 0x080F40CC, XN_BN4_CODE + 0xD330, 0x6E3C899Au },
	{ 0x080F40CC, 0x080F40F8, XN_BN4_CODE + 0xD360, 0x35AF033Fu },
	{ 0x080F40F8, 0x080F4122, XN_BN4_CODE + 0xD390, 0xF53A9248u },
	{ 0x080F4122, 0x080F4160, XN_BN4_CODE + 0xD3C2, 0xBCBA3418u },
	{ 0x080F4160, 0x080F4188, XN_BN4_CODE + 0xD400, 0x40EEE1EEu },
	{ 0x080F4188, 0x080F41DC, XN_BN4_CODE + 0xD430, 0x9011DE60u },
	{ 0x080F41DC, 0x080F4200, XN_BN4_CODE + 0xD490, 0xADFC0187u },
	{ 0x080F4200, 0x080F422C, XN_BN4_CODE + 0xD4C0, 0x89870123u },
	{ 0x080F422C, 0x080F4256, XN_BN4_CODE + 0xD4F0, 0x1BE9F5D8u },
	{ 0x080F4256, 0x080F428E, XN_BN4_CODE + 0xD522, 0xB859D99Cu },
	{ 0x080F428E, 0x080F42B8, XN_BN4_CODE + 0xD562, 0x0FBA541Fu },
	{ 0x080F42B8, 0x080F42E2, XN_BN4_CODE + 0xD590, 0x3D629CC9u },
	{ 0x080F42E2, 0x080F433C, XN_BN4_CODE + 0xD5C2, 0xCEBF243Fu },
};
static const XvData junkman_data[] = {
	{ 0x080C91A8, 0x080C91BA, XV_ZERO },
	{ 0x080C91BA, 0x080C91BC, XV_RAW },
	{ 0x080C91BC, 0x080C9242, XV_ZERO },
	{ 0x080C9242, 0x080C9244, XV_RAW },
	{ 0x080C9244, 0x080C9270, XV_ZERO },
	{ 0x080C92C0, 0x080C92C4, XV_POOL },
	{ 0x080C92C4, 0x080C92C8, XV_TABLE },
	{ 0x080C9346, 0x080C9348, XV_RAW },
	{ 0x080C9348, 0x080C934C, XV_POOL },
	{ 0x080C942A, 0x080C942C, XV_RAW },
	{ 0x080C942C, 0x080C9430, XV_POOL },
	{ 0x080C9430, 0x080C9434, XV_RAW },
	{ 0x080C9434, 0x080C9438, XV_POOL },
	{ 0x080C956A, 0x080C956C, XV_RAW },
	{ 0x080C956C, 0x080C9570, XV_POOL },
	{ 0x080C9570, 0x080C9574, XV_RAW },
	{ 0x080C9574, 0x080C9578, XV_POOL },
	{ 0x080C9578, 0x080C9588, XV_RAW },
	{ 0x080C95AC, 0x080C95B8, XV_POOL },
	{ 0x080C95B8, 0x080C95C4, XV_RAW },
	{ 0x080C95C4, 0x080C95C8, XV_POOL },
	{ 0x080C95C8, 0x080C95CC, XV_RAW },
	{ 0x080C95CC, 0x080C95D8, XV_POOL },
	{ 0x080C95D8, 0x080C95E8, XV_RAW },
	{ 0x080DFD62, 0x080DFD64, XV_RAW },
	{ 0x080DFD64, 0x080DFD68, XV_POOL },
	{ 0x080DFD68, 0x080DFD74, XV_TABLE },
	{ 0x080DFEAC, 0x080DFEB0, XV_POOL },
	{ 0x080DFEB0, 0x080DFEB8, XV_TABLE },
	{ 0x080DFF30, 0x080DFF34, XV_POOL },
	{ 0x080DFF34, 0x080DFF40, XV_RAW },
	{ 0x080DFF40, 0x080DFF44, XV_POOL },
	{ 0x080DFF54, 0x080DFF58, XV_POOL },
	{ 0x080DFF58, 0x080DFF60, XV_RAW },
	{ 0x080DFF8A, 0x080DFF8C, XV_RAW },
	{ 0x080E019A, 0x080E019C, XV_RAW },
	{ 0x080E019C, 0x080E01A0, XV_POOL },
	{ 0x080E01A0, 0x080E01AC, XV_TABLE },
	{ 0x080E026C, 0x080E0270, XV_POOL },
	{ 0x080E0270, 0x080E0278, XV_TABLE },
	{ 0x080E02E4, 0x080E02E8, XV_POOL },
	{ 0x080E0606, 0x080E0608, XV_RAW },
	{ 0x080E0608, 0x080E060C, XV_POOL },
	{ 0x080E060C, 0x080E0618, XV_TABLE },
	{ 0x080E063A, 0x080E063E, XV_RAW },
	{ 0x080DFF9A, 0x080DFF9C, XV_RAW },
	{ 0x080DFF9C, 0x080DFFA0, XV_POOL },
	{ 0x080DFFA0, 0x080DFFAC, XV_TABLE },
	{ 0x080E009A, 0x080E009C, XV_RAW },
	{ 0x080E009C, 0x080E00A0, XV_POOL },
	{ 0x080E00A0, 0x080E00AC, XV_TABLE },
	{ 0x080E00E2, 0x080E00E4, XV_RAW },
	{ 0x080E00E4, 0x080E00E8, XV_POOL },
	{ 0x080E016A, 0x080E016C, XV_RAW },
	{ 0x080E016C, 0x080E0170, XV_POOL },
	{ 0x0800AD7E, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B130, XV_POOL },
	{ 0x080DF344, 0x080DF364, XV_ZERO },
	{ 0x080DF364, 0x080DF36C, XV_POOL },
	{ 0x080F4030, 0x080F4034, XV_POOL },
	{ 0x080F4034, 0x080F4040, XV_TABLE },
	{ 0x080F40E0, 0x080F40E4, XV_POOL },
	{ 0x080F40E4, 0x080F40F8, XV_TABLE },
	{ 0x080F4214, 0x080F4218, XV_POOL },
	{ 0x080F4218, 0x080F422C, XV_TABLE },
};
static const XvPair junkman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002CEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08002D4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E20, 0x08002DB4 },   /* project:fireman, SAME */
	{ 0x08002E34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002F22, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002F2E, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007A8C, 0x0800A2F0 },   /* map:bytes, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x0800A030, 0x0800CED0 },   /* map:bytes, SAME */
	{ 0x0800A0D8, 0x0800CFDC },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800AB20, 0x0800E258 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800BFC0, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x08010FA6, 0x08017888 },   /* BN4's generic Navi state 1, its own state 1's tail: BN6's generic Navi state 1 (on to its first state), AIMED (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080124C8, 0x080198CE },   /* map:calls, LAYOUT */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012DC8, 0x0801A07C },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x0801466A, 0x0801BE28 },   /* map:calls, DIFF (audited) */
	{ 0x0802608C, 0x080302A8 },   /* project:research, SAME */
	{ 0x080406A4, 0x0800A03C },   /* map:calls, DIFF (audited) */
	{ 0x08044AEC, 0x080011EC },   /* project:billy, DIFF (audited) */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm junkman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800AD6A, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD6C, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800C9B4, THUMB_ADDS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x0800CA40, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080C9280, THUMB_MOVS, 0, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C92EE, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 set (BN4_SET_ACTION's r0) */
	{ 0x080C9306, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 set (BN4_SET_ACTION's r0) */
	{ 0x080C933A, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 set (BN4_SET_ACTION's r0) */
	{ 0x080C935C, THUMB_MOVS, 0, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C941A, THUMB_MOVS, 0, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080C958A, THUMB_MOVS, 1, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080DF33E, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DFD78, THUMB_MOVS, 2, 0, 0x3C, 0xE9 },   /* its object's sprite: list 4 0x3c in BN4, list 4 0xe9 in BN6 (copied) */
	{ 0x080DFDC0, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DFE18, THUMB_MOVS, 0, 0, 0xC8, 0xE0 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0xe0, on its player 11 */
	{ 0x080DFE84, THUMB_MOVS, 4, 0, 0x0, 0x9C },   /* its effect: 0x0 in BN4, 0x9c in BN6 (moved) */
	{ 0x080DFE8A, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's empty slot 0x3a, on its player 9 */
	{ 0x080DFE90, THUMB_MOVS, 0, 0, 0xC8, 0xE0 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0xe0, on its player 11 */
	{ 0x080DFEC4, THUMB_MOVS, 0, 0, 0xA0, 0x32 },   /* its sound 0xa0: BN4's copied into BN6's empty slot 0x32, on its player 29 */
	{ 0x080DFEF8, THUMB_MOVS, 0, 0, 0xBF, 0x63 },   /* its sound 0xbf: BN4's copied into BN6's empty slot 0x63, on its player 9 */
	{ 0x080DFF46, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DFF64, THUMB_MOVS, 0, 0, 0x96, 0xCF },   /* its object's type 0x96: moved, the dispatch's 0xcf */
	{ 0x080DFFB0, THUMB_MOVS, 2, 0, 0x3B, 0xEA },   /* its object's sprite: list 4 0x3b in BN4, list 4 0xea in BN6 (copied) */
	{ 0x080DFFFE, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080E0174, THUMB_MOVS, 0, 0, 0x97, 0xD0 },   /* its object's type 0x97: moved, the dispatch's 0xd0 */
	{ 0x080E01B0, THUMB_MOVS, 2, 0, 0x3A, 0xEB },   /* its object's sprite: list 4 0x3a in BN4, list 4 0xeb in BN6 (copied) */
	{ 0x080E02F8, THUMB_MOVS, 0, 0, 0x98, 0xD1 },   /* its object's type 0x98: moved, the dispatch's 0xd1 */
	{ 0x080E0692, THUMB_MOVS, 0, 0, 0x9B, 0xD2 },   /* its object's type 0x9b: moved, the dispatch's 0xd2 */
	{ 0x080F4020, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F40D0, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F41B8, THUMB_MOVS, 0, 0, 0xC0, 0xD0 },   /* its sound 0xc0: BN4's copied into BN6's empty slot 0xd0, on its player 27 */
	{ 0x080F4204, THUMB_MOVS, 7, 0, 0x70, 0xA0 },   /* its AI data's +0x70 in BN4, +0xa0 in BN6 (AI_BLOCKS) */
	{ 0x080F4312, THUMB_MOVS, 0, 0, 0xA2, 0x33 },   /* its sound 0xa2: BN4's copied into BN6's empty slot 0x33, on its player 30 */
};
static const XvNumber junkman_numbers[] = {
	{ 0x080DF364, 0x0A800000, 0x0A800000 },   /* the flags its hits left it, tested (BN4_COLL_FROM_COLLISION): BN6's bits at FlagsFromCollision (+0x70), where the thunder's update tests the same mask, 0xFF800000, in both games */
	{ 0x080DF368, 0x05800000, 0x05800000 },   /* SearchMan's shot's test (0x080DF332): the hits word's bits of a side 0 object's foes (0x0A800000 a side 1 one's), as BN4_COLL_MASKS' and BN6_COLL_MASKS' alliance bits */
	{ 0x080E02E4, 0x06050A01, 0x06050A01 },   /* a collision region's spawn word (JunkMan's third kind 0x98's, 0x080E02D8, its r4 to 0x080CD7E2): BN6's own attacks pass the same to its twin 0x080C6BDA (seven of its pools, 0x080BD3A0 the first) */
};
static const XvPair junkman_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvType junkman_types[] = {
	{ 3, 0xCF, XV_TYPE_MOVED, 0x96 },   /* BN4's third-kind 0x96, moved, the dispatch's 0xcf */
	{ 3, 0xD0, XV_TYPE_MOVED, 0x97 },   /* BN4's third-kind 0x97, moved, the dispatch's 0xd0 */
	{ 3, 0xD1, XV_TYPE_MOVED, 0x98 },   /* BN4's third-kind 0x98, moved, the dispatch's 0xd1 */
	{ 3, 0xD2, XV_TYPE_MOVED, 0x9B },   /* BN4's third-kind 0x9b, moved, the dispatch's 0xd2 */
};
static const XvRef junkman_states[] = {
	BN6(0x08016381), OWN(0x080C9271), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C92AF), OWN(0x080F401D), OWN(0x080F40CD), OWN(0x080F4201),
};
static const XvSprite junkman_sprites[] = {
	{ 2, 0x0A, 2, 0x49, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 4, 0x3C, 4, 0xE9, XN_BN4_ART + 0x0, 0x800 },
	{ 4, 0x3B, 4, 0xEA, XN_BN4_ART + 0x800, 0x800 },
	{ 4, 0x3A, 4, 0xEB, XN_BN4_ART + 0x1000, 0x1000 },
	{ 5, 0x00, 5, 0x9D, XN_BN4_ART + 0x2000, 0x1C00 },
};
static const XvId junkman_effects[] = { { 0x00, 0x9C } };
static const XvSong junkman_songs[] = {
	{ 0xA0, 0x32, XN_BN4_ART + 0x3C00, 0x900 },   /* its sound 0xa0, BN4's song copied */
	{ 0xA2, 0x33, XN_BN4_ART + 0x4500, 0x22C0 },   /* its sound 0xa2, BN4's song copied */
	{ 0xBF, 0x63, XN_BN4_ART + 0x67C0, 0x40 },   /* its sound 0xbf, BN4's song copied */
	{ 0xC0, 0xD0, XN_BN4_ART + 0x6800, 0x880 },   /* its sound 0xc0, BN4's song copied */
	{ 0xC8, 0xE0, XN_BN4_ART + 0x7080, 0xB40 },   /* its sound 0xc8, BN4's song copied */
	{ 0x14A, 0x104, XN_BN4_ART + 0x7BC0, 0x40 },   /* its sound 0x14a, BN4's song copied */
};
static const XvSoundCall junkman_sound_calls[] = {
	{ 0x080F42C8, 0x104 },   /* its sound 0x14a (movs and adds #0xff) at the pool's 0x104, past a byte */
};
static const XnInit junkman_init = { NULL, 0, false, false, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */
static const XnPreCall junkman_pre[] = {
	{ OWN(0x080C9499), 11 },
	{ OWN(0x080C94AF), XN_R0_ANY },
};


static const XNavi bn4_junkman = {
	.code = {
		.name = "JunkMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x47,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 57,
		.first_id = XN_BN4_FIRST_ID + 0x24,
		.home = XN_BN4_HOMES + 0x4800,
		.blocks = junkman_blocks, .nblocks = (int)(sizeof junkman_blocks / sizeof *junkman_blocks),
		.data = junkman_data, .ndata = (int)(sizeof junkman_data / sizeof *junkman_data),
		.calls = junkman_calls, .ncalls = (int)(sizeof junkman_calls / sizeof *junkman_calls),
		.words = junkman_words, .nwords = (int)(sizeof junkman_words / sizeof *junkman_words),
		.imms = junkman_imms, .nimms = (int)(sizeof junkman_imms / sizeof *junkman_imms),
		.numbers = junkman_numbers, .nnumbers = (int)(sizeof junkman_numbers / sizeof *junkman_numbers),
		.types = junkman_types, .ntypes = (int)(sizeof junkman_types / sizeof *junkman_types),
		.states = junkman_states, .nstates = (int)(sizeof junkman_states / sizeof *junkman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x08012038, .struct2 = 0x08012038,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0xA4, 0xA5, 0xA6, 0xA7 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = false, .palette = 0, .flags = 0x00 },
		.sprites = junkman_sprites, .nsprites = (int)(sizeof junkman_sprites / sizeof *junkman_sprites),
		.effects = junkman_effects, .neffects = (int)(sizeof junkman_effects / sizeof *junkman_effects),
		.songs = junkman_songs, .nsongs = (int)(sizeof junkman_songs / sizeof *junkman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
		.sound_calls = junkman_sound_calls, .nsound_calls = (int)(sizeof junkman_sound_calls / sizeof *junkman_sound_calls),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.palettes = 0x04000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &junkman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
	.pre = junkman_pre, .npre = 2,   /* (its update's calls between its pre-step and its runner, its hook A's shim) */
	.runner_r4 = true,   /* (its states run with r4 its +0 byte, as BN4's Navi runner: xnavirecipe_bn4_objects.RUNNER_R4) */
	.record_list = 1,   /* (its records' sprite list, BN4's preload's alone: its battle sprite its init's) */
};
/* ---- BN4's TopMan (its type 0x9 there, its enemy ids 176-179), as Navi AI 60 with ids 0x3b0-0x3b3 ---- */

static const XvBlock topman_blocks[] = {
	{ 0x080B68B0, 0x080B73C8, XN_BN4_CODE + 0xD620, 0x7BE7F845u },
	{ 0x080D02F8, 0x080D055C, XN_BN4_CODE + 0xE140, 0x7FDE117Au },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0xE3B2, 0x793106FEu },
	{ 0x080B552A, 0x080B5594, XN_BN4_CODE + 0xE7C2, 0x29597AFDu },
	{ 0x080B56A4, 0x080B56E4, XN_BN4_CODE + 0xE830, 0x67EEC233u },
};
static const XvData topman_data[] = {
	{ 0x080B68B0, 0x080B68C2, XV_ZERO },
	{ 0x080B68C2, 0x080B68C4, XV_RAW },
	{ 0x080B68C4, 0x080B6946, XV_ZERO },
	{ 0x080B6946, 0x080B6948, XV_RAW },
	{ 0x080B6948, 0x080B697C, XV_ZERO },
	{ 0x080B69A6, 0x080B69A8, XV_RAW },
	{ 0x080B69A8, 0x080B69AC, XV_POOL },
	{ 0x080B69AC, 0x080B69BC, XV_TABLE },
	{ 0x080B6A60, 0x080B6A64, XV_POOL },
	{ 0x080B6B9A, 0x080B6B9C, XV_RAW },
	{ 0x080B6B9C, 0x080B6BA4, XV_POOL },
	{ 0x080B6C0A, 0x080B6C0C, XV_RAW },
	{ 0x080B6C0C, 0x080B6C10, XV_POOL },
	{ 0x080B6C10, 0x080B6C20, XV_RAW },
	{ 0x080B6C20, 0x080B6C24, XV_POOL },
	{ 0x080B6C24, 0x080B6C2C, XV_RAW },
	{ 0x080B6C6E, 0x080B6C70, XV_RAW },
	{ 0x080B6C70, 0x080B6C7C, XV_POOL },
	{ 0x080B6C8A, 0x080B6C8C, XV_RAW },
	{ 0x080B6C8C, 0x080B6C90, XV_POOL },
	{ 0x080B6D0A, 0x080B6D0C, XV_RAW },
	{ 0x080B6D0C, 0x080B6D14, XV_POOL },
	{ 0x080B6D54, 0x080B6D58, XV_POOL },
	{ 0x080B6D58, 0x080B6D68, XV_TABLE },
	{ 0x080B6E20, 0x080B6E24, XV_POOL },
	{ 0x080B6E24, 0x080B6E30, XV_POOL },
	{ 0x080B6E30, 0x080B6E3C, XV_RAW },
	{ 0x080B6E5E, 0x080B6E60, XV_RAW },
	{ 0x080B6E60, 0x080B6E64, XV_POOL },
	{ 0x080B6E64, 0x080B6E84, XV_TABLE },
	{ 0x080B6ECA, 0x080B6ECC, XV_RAW },
	{ 0x080B6ECC, 0x080B6ED0, XV_POOL },
	{ 0x080B6F82, 0x080B6F84, XV_RAW },
	{ 0x080B6F84, 0x080B6F88, XV_POOL },
	{ 0x080B6F88, 0x080B6F8C, XV_RAW },
	{ 0x080B7008, 0x080B7010, XV_POOL },
	{ 0x080B7088, 0x080B708C, XV_POOL },
	{ 0x080B708C, 0x080B70A0, XV_TABLE },
	{ 0x080B71D4, 0x080B71DC, XV_POOL },
	{ 0x080B7206, 0x080B7208, XV_RAW },
	{ 0x080B7208, 0x080B720C, XV_POOL },
	{ 0x080B720C, 0x080B7234, XV_TABLE },
	{ 0x080B72BA, 0x080B72BC, XV_RAW },
	{ 0x080B72BC, 0x080B72C4, XV_POOL },
	{ 0x080B72C4, 0x080B72CC, XV_RAW },
	{ 0x080B730A, 0x080B730C, XV_RAW },
	{ 0x080B730C, 0x080B7310, XV_POOL },
	{ 0x080B739E, 0x080B73A0, XV_RAW },
	{ 0x080B73A0, 0x080B73AC, XV_POOL },
	{ 0x080B73AC, 0x080B73C8, XV_RAW },
	{ 0x080D030A, 0x080D030C, XV_RAW },
	{ 0x080D030C, 0x080D0310, XV_POOL },
	{ 0x080D0310, 0x080D031C, XV_TABLE },
	{ 0x080D03CE, 0x080D03D0, XV_RAW },
	{ 0x080D03D0, 0x080D03D4, XV_POOL },
	{ 0x080D03D4, 0x080D03D8, XV_RAW },
	{ 0x080D048E, 0x080D0490, XV_RAW },
	{ 0x080D0490, 0x080D0494, XV_POOL },
	{ 0x080D0494, 0x080D049C, XV_TABLE },
	{ 0x080D0520, 0x080D0528, XV_POOL },
	{ 0x080D0556, 0x080D0558, XV_RAW },
	{ 0x080D0558, 0x080D055C, XV_POOL },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x080B557E, 0x080B5580, XV_ZERO },
	{ 0x080B5580, 0x080B5584, XV_POOL },
	{ 0x080B5584, 0x080B5594, XV_RAW },
	{ 0x080B56D0, 0x080B56D4, XV_POOL },
	{ 0x080B56D4, 0x080B56E4, XV_RAW },
};
static const XvPair topman_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08000AAA, 0x08000C72 },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002EAC, 0x08002E52 },   /* map:bytes, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800A0A2, 0x0800CFA6 },   /* project:spikey, SAME */
	{ 0x0800A0D8, 0x0800CFDC },   /* project:research, SAME */
	{ 0x0800A10E, 0x0800D012 },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AE30, 0x0800E708 },   /* project:research, SAME */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x08010560, 0x080D0B94 },   /* project:spikey, SAME */
	{ 0x08010E88, 0x080174FE },   /* BN4's generic Navi state 3, its own state 3's tail: BN6's generic Navi state 3 (flinching), AIMED (audited) */
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
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080144FE, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0802608C, 0x080302A8 },   /* project:research, SAME */
	{ 0x080B3878, 0x0800EC48 },   /* project:research, SAME */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm topman_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x080B69C2, THUMB_LDRH, 1, 3, 0x1E, 0x2E },   /* collision data +0x1e in BN4, +0x2e in BN6 */
	{ 0x080B69C8, THUMB_STRH, 0, 3, 0x1E, 0x2E },   /* collision data +0x1e in BN4, +0x2e in BN6 */
	{ 0x080B69CC, THUMB_STRB, 0, 3, 0x5, 0x7 },   /* collision data +0x5 in BN4, +0x7 in BN6 */
	{ 0x080B69E0, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080B6A2C, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 written */
	{ 0x080B6A44, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080B6A4E, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 written */
	{ 0x080B6ABA, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080B6D26, THUMB_MOVS, 4, 0, 0x14, 0x93 },   /* its effect: 0x14 in BN4, 0x93 in BN6 (moved) */
	{ 0x080B6D3C, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B6D96, THUMB_MOVS, 4, 0, 0x14, 0x93 },   /* its effect: 0x14 in BN4, 0x93 in BN6 (moved) */
	{ 0x080B6DAA, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B6DCA, THUMB_MOVS, 4, 0, 0x13, 0x92 },   /* its effect: 0x13 in BN4, 0x92 in BN6 (moved) */
	{ 0x080B6E46, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B6EC0, THUMB_CMP, 1, 0, 0xB, 0xD },   /* its CurAction 11 compared */
	{ 0x080B6EFC, THUMB_CMP, 1, 0, 0x9, 0xB },   /* its CurAction 9 compared */
	{ 0x080B6F26, THUMB_MOVS, 0, 0, 0xAC, 0xD0 },   /* its sound 0xac: BN4's copied into BN6's empty slot 0xd0, on its player 27 */
	{ 0x080B6F72, THUMB_MOVS, 0, 0, 0xB5, 0xE0 },   /* its sound 0xb5: BN4's copied into BN6's empty slot 0xe0, on its player 19 */
	{ 0x080B702A, THUMB_CMP, 1, 0, 0xB, 0xD },   /* its CurAction 11 compared */
	{ 0x080B7052, THUMB_CMP, 1, 0, 0xB, 0xD },   /* its CurAction 11 compared */
	{ 0x080B7070, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B71EE, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B723E, THUMB_MOVS, 0, 0, 0xAC, 0xD0 },   /* its sound 0xac: BN4's copied into BN6's empty slot 0xd0, on its player 27 */
	{ 0x080B7284, THUMB_STRB, 0, 3, 0x5, 0x7 },   /* collision data +0x5 in BN4, +0x7 in BN6 */
	{ 0x080B728E, THUMB_STRH, 0, 3, 0x1E, 0x2E },   /* collision data +0x1e in BN4, +0x2e in BN6 */
	{ 0x080B7344, THUMB_MOVS, 4, 0, 0x6, 0x9F },   /* its effect: 0x6 in BN4, 0x9f in BN6 (moved) */
	{ 0x080B7352, THUMB_MOVS, 0, 0, 0xA2, 0x33 },   /* its sound 0xa2: BN4's copied into BN6's empty slot 0x33, on its player 30 */
	{ 0x080D0320, THUMB_MOVS, 2, 0, 0x12, 0x4C },   /* its object's sprite: his own battle sprite, list 2 0x12 in BN4, 0x4c in BN6 (XNAVI_SPRITE_BN4) */
	{ 0x080D03BE, THUMB_MOVS, 0, 0, 0xA0, 0x32 },   /* its sound 0xa0: BN4's copied into BN6's empty slot 0x32, on its player 29 */
	{ 0x080D0418, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D0478, THUMB_MOVS, 4, 0, 0xF, 0x97 },   /* its effect: 0xf in BN4, 0x97 in BN6 (moved) */
	{ 0x080D047E, THUMB_MOVS, 0, 0, 0x70, 0x3A },   /* its sound 0x70: BN4's copied into BN6's empty slot 0x3a, on its player 9 */
	{ 0x080D04AA, THUMB_MOVS, 0, 0, 0xAB, 0x63 },   /* its sound 0xab: BN4's copied into BN6's empty slot 0x63, on its player 28 */
	{ 0x080D04FE, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D052C, THUMB_MOVS, 0, 0, 0x1B, 0xD3 },   /* its object's type 0x1b: moved, the dispatch's 0xd3 */
	{ 0x080D053C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvNumber topman_numbers[] = {
	{ 0x080B700C, 0x03800000, 0x03800000 },   /* a panel's flags it may not have (object_checkPanelParameters' r3, BN6's 0x0800CC86 the same body): 0x0B800000's bits but 0x08000000 (GutsMan's objects, 0x080D07C4) */
};
static const XvType topman_types[] = {
	{ 3, 0xD3, XV_TYPE_MOVED, 0x1B },   /* BN4's third-kind 0x1b, moved, the dispatch's 0xd3 */
};
static const XvRef topman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), OWN(0x080B697D), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080B6995), OWN(0x080B6D33), OWN(0x080B6D47), OWN(0x080B6E51), OWN(0x080B707B), OWN(0x080B71F9),
};
static const XvSprite topman_sprites[] = {
	{ 2, 0x12, 2, 0x4C, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },   /* (LZ77 in BN4) */
	{ 5, 0x0A, 5, 0x9F, XN_BN4_ART + 0x0, 0xD00 },
};
static const XvId topman_effects[] = { { 0x06, 0x9F }, { 0x0F, 0x97 }, { 0x13, 0x92 }, { 0x14, 0x93 }, { 0x15, 0x90 } };
static const XvSong topman_songs[] = {
	{ 0xA0, 0x32, XN_BN4_ART + 0xD00, 0x900 },   /* its sound 0xa0, BN4's song copied */
	{ 0xA2, 0x33, XN_BN4_ART + 0x1600, 0x22C0 },   /* its sound 0xa2, BN4's song copied */
	{ 0xAB, 0x63, XN_BN4_ART + 0x38C0, 0x1080 },   /* its sound 0xab, BN4's song copied */
	{ 0xAC, 0xD0, XN_BN4_ART + 0x4940, 0xB40 },   /* its sound 0xac, BN4's song copied */
	{ 0xB5, 0xE0, XN_BN4_ART + 0x5480, 0xF40 },   /* its sound 0xb5, BN4's song copied */
	{ 0xB6, 0x104, XN_BN4_ART + 0x63C0, 0x1A80 },   /* its sound 0xb6, BN4's song copied */
};
static const XvSoundCall topman_sound_calls[] = {
	{ 0x080B7292, 0x104 },   /* its sound 0xb6 at the pool's 0x104, past a byte */
};
static const XnInit topman_init = { NULL, 0, false, true, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */


static const XNavi bn4_topman = {
	.code = {
		.name = "TopMan (BN4's)",
		.game = XROM_BN4_RED_SUN_US,
		.src_ai = 0x09,   /* (its first-kind type: BN4 has no AI index) */
		.ai = 60,
		.first_id = XN_BN4_FIRST_ID + 0x30,
		.home = XN_BN4_HOMES + 0x6000,
		.blocks = topman_blocks, .nblocks = (int)(sizeof topman_blocks / sizeof *topman_blocks),
		.data = topman_data, .ndata = (int)(sizeof topman_data / sizeof *topman_data),
		.calls = topman_calls, .ncalls = (int)(sizeof topman_calls / sizeof *topman_calls),
		.imms = topman_imms, .nimms = (int)(sizeof topman_imms / sizeof *topman_imms),
		.numbers = topman_numbers, .nnumbers = (int)(sizeof topman_numbers / sizeof *topman_numbers),
		.types = topman_types, .ntypes = (int)(sizeof topman_types / sizeof *topman_types),
		.states = topman_states, .nstates = (int)(sizeof topman_states / sizeof *topman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080120C8, .struct2 = 0x080120C8,   /* (its V1's enemy record: BN4 has no struct1 or struct2) */
		.names = { 0xB0, 0xB1, 0xB2, 0xB3 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 0, .flags = 0x00 },
		.sprites = topman_sprites, .nsprites = (int)(sizeof topman_sprites / sizeof *topman_sprites),
		.effects = topman_effects, .neffects = (int)(sizeof topman_effects / sizeof *topman_effects),
		.songs = topman_songs, .nsongs = (int)(sizeof topman_songs / sizeof *topman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
		.sound_calls = topman_sound_calls, .nsound_calls = (int)(sizeof topman_sound_calls / sizeof *topman_sound_calls),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.palettes = 0x03000000,   /* (its palette by version, BN4_NAVI_PALETTE's, a byte each from V1's: set at its spawn's end over BN6's actor init's 0, xnavi_init.c) */
	.init = &topman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
};
const XNavi *const xnavi_bn4[] = { &bn4_gutsman, &bn4_thunderman, &bn4_windman, &bn4_fireman, &bn4_searchman, &bn4_aquaman, &bn4_roll, &bn4_woodman, &bn4_junkman, &bn4_topman };
const int xnavi_bn4_count = (int)(sizeof xnavi_bn4 / sizeof *xnavi_bn4);

/* held from the install (tools/xnavirecipe_bn4.py HELD): none */
const XNavi *const xnavi_bn4_held[] = { NULL };
const int xnavi_bn4_held_count = 0;

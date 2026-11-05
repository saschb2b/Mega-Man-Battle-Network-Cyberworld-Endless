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
	{ 0x080CE812, THUMB_MOVS, 0, 0, 0xB, 0xE5 },   /* its object's type 0xb: moved, the dispatch's 0xe5 */
	{ 0x080D0596, THUMB_MOVS, 0, 0, 0xA2, 0x32 },   /* its sound 0xa2: BN4's copied into BN6's empty slot 0x32, on its player 30 */
	{ 0x080D06D2, THUMB_LSLS, 0, 0, 0x2, 0x4 },   /* a side's player read from the battle state: BN4 +0x44 a word each, BN6 +0xD0 0x10 bytes each */
	{ 0x080D06D4, THUMB_ADDS, 0, 0, 0x44, 0xD0 },   /* a side's player read from the battle state: BN4 +0x44 a word each, BN6 +0xD0 0x10 bytes each */
	{ 0x080D06E6, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D072A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D079A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D085A, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0874, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0898, THUMB_MOVS, 0, 0, 0x1C, 0xE6 },   /* its object's type 0x1c: moved, the dispatch's 0xe6 */
	{ 0x080D08A8, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D090A, THUMB_MOVS, 2, 0, 0xE, 0xCA },   /* its object's sprite: list 4 0xe in BN4, list 4 0xca in BN6 (copied) */
	{ 0x080D0968, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D09D4, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0A14, THUMB_MOVS, 0, 0, 0xA3, 0x33 },   /* its sound 0xa3: BN4's copied into BN6's empty slot 0x33, on its player 9 */
	{ 0x080D0A44, THUMB_MOVS, 0, 0, 0x1D, 0xE7 },   /* its object's type 0x1d: moved, the dispatch's 0xe7 */
	{ 0x080D0A54, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0AA0, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080D0AA2, THUMB_MOVS, 2, 0, 0x4, 0xC9 },   /* its object's sprite: list 3 0x4 in BN4, list 4 0xc9 in BN6 (copied) */
	{ 0x080D0B3C, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D0B5C, THUMB_STRB, 0, 7, 0x8, 0xA },   /* collision data +0x8 in BN4, +0xa in BN6 */
	{ 0x080D0B5E, THUMB_STRB, 1, 7, 0x9, 0xB },   /* collision data +0x9 in BN4, +0xb in BN6 */
	{ 0x080D0B86, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0BA2, THUMB_MOVS, 0, 0, 0x1E, 0xE8 },   /* its object's type 0x1e: moved, the dispatch's 0xe8 */
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
	{ 3, 0xE5, XV_TYPE_MOVED, 0x0B },   /* BN4's third-kind 0xb, moved, the dispatch's 0xe5 */
	{ 3, 0xE6, XV_TYPE_MOVED, 0x1C },   /* BN4's third-kind 0x1c, moved, the dispatch's 0xe6 */
	{ 3, 0xE7, XV_TYPE_MOVED, 0x1D },   /* BN4's third-kind 0x1d, moved, the dispatch's 0xe7 */
	{ 3, 0xE8, XV_TYPE_MOVED, 0x1E },   /* BN4's third-kind 0x1e, moved, the dispatch's 0xe8 */
};
static const XvRef gutsman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080B7485), OWN(0x080F0435), OWN(0x080F050B), OWN(0x080F05BB), OWN(0x080F069B),
};
static const XvSprite gutsman_sprites[] = {
	{ 2, 0x02, 2, 0x41, XN_BN4_ART + 0x1000, 0xF000 },
	{ 3, 0x04, 4, 0xC9, XN_BN4_ART + 0x0, 0x1000 },
	{ 4, 0x0E, 4, 0xCA, XN_BN4_ART + 0x10000, 0x800 },
};
static const XvId gutsman_effects[] = { { 0x15, 0x90 }, { 0x56, 0x99 } };
static const XvSong gutsman_songs[] = {
	{ 0xA2, 0x32, XN_BN4_ART + 0x10800, 0x22C0 },   /* its sound 0xa2, BN4's song copied */
	{ 0xA3, 0x33, XN_BN4_ART + 0x12AC0, 0x40 },   /* its sound 0xa3, BN4's song copied */
	{ 0xA4, 0x63, XN_BN4_ART + 0x12B00, 0x80 },   /* its sound 0xa4, BN4's song copied */
	{ 0xA5, 0xD0, XN_BN4_ART + 0x12B80, 0x40 },   /* its sound 0xa5, BN4's song copied */
	{ 0xA7, 0xE0, XN_BN4_ART + 0x12BC0, 0x40 },   /* its sound 0xa7, BN4's song copied */
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
};
/* ---- BN4's WindMan (its type 0x30 there, its enemy ids 136-139), as Navi AI 50 with ids 0x388-0x38b ---- */
/* interim: its palette: BN4_NAVI_PALETTE's 3, step 2, BN4's by +4 [2, 4, 2, 4]; BN6's actor init gives a Navi's V1-V3 palette 0 whatever its step (0x0800F334, its table 0x0800F354), so V1 draws BN6's palette 0 */
/* interim: 080CCDC8: the first kind's 0x5C's state 4, step 0 (its sprite drawn again through BN4's tile buffer 0x02037810, a copy of 0x0800087C's and 0x08003578's): its init takes state 4 only where the battle state's +0x32 has no bit 0 (0x08007A8C) and its settings' +6 has 0x20 (0x080CCD1E-0x080CCD2E), else 0 or 8; kept as BN4's bytes, unported (assumed unreached in a plain battle) */

static const XvBlock windman_blocks[] = {
	{ 0x080C3234, 0x080C355C, XN_BN4_CODE + 0x16C0, 0x806DFC0Fu },
	{ 0x080CCC9C, 0x080CCF5C, XN_BN4_CODE + 0x19F0, 0xBEC868D2u },
	{ 0x080DA430, 0x080DA6B4, XN_BN4_CODE + 0x1CB0, 0x45D32406u },
	{ 0x080D9FA0, 0x080DA220, XN_BN4_CODE + 0x1F40, 0x3A7A5B86u },
	{ 0x080D9C58, 0x080D9DDC, XN_BN4_CODE + 0x21C0, 0x7E33F5B9u },
	{ 0x080CCC94, 0x080CCC9C, XN_BN4_CODE + 0x2350, 0xD258FE18u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0x2362, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x2772, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x2790, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x27A0, 0xDCE85B3Du },
	{ 0x080F0AF8, 0x080F0B1C, XN_BN4_CODE + 0x27D0, 0x7E2518A1u },
	{ 0x080F0B1C, 0x080F0B6C, XN_BN4_CODE + 0x2800, 0xA36A16DCu },
	{ 0x080F0B6C, 0x080F0BAA, XN_BN4_CODE + 0x2850, 0x2E5D6477u },
	{ 0x080F0BAA, 0x080F0BCE, XN_BN4_CODE + 0x2892, 0xDD30CAB8u },
	{ 0x080F0BCE, 0x080F0BF4, XN_BN4_CODE + 0x28C2, 0xAC8D5279u },
	{ 0x080F0BF4, 0x080F0C1E, XN_BN4_CODE + 0x28F0, 0x3930D66Cu },
	{ 0x080F0C1E, 0x080F0C80, XN_BN4_CODE + 0x2922, 0x63E6371Fu },
	{ 0x080F0C80, 0x080F0C9E, XN_BN4_CODE + 0x2990, 0xC504D80Cu },
	{ 0x080F0C9E, 0x080F0CC8, XN_BN4_CODE + 0x29B2, 0x97105168u },
	{ 0x080F0CC8, 0x080F0D0A, XN_BN4_CODE + 0x29E0, 0x0E84351Du },
	{ 0x080F0D0A, 0x080F0D56, XN_BN4_CODE + 0x2A32, 0x8335BA75u },
	{ 0x080F0D56, 0x080F0D9E, XN_BN4_CODE + 0x2A82, 0xA353C10Cu },
	{ 0x080F0D9E, 0x080F0DC4, XN_BN4_CODE + 0x2AD2, 0x36CD75B4u },
	{ 0x080F0DC4, 0x080F0DEC, XN_BN4_CODE + 0x2B00, 0x2D75A9CCu },
	{ 0x080F0DEC, 0x080F0E1A, XN_BN4_CODE + 0x2B30, 0x04380CBFu },
	{ 0x080F0E1A, 0x080F0E28, XN_BN4_CODE + 0x2B62, 0x17E9C5AAu },
	{ 0x080F0E28, 0x080F0E80, XN_BN4_CODE + 0x2B70, 0xBDB40846u },
	{ 0x080F0E80, 0x080F0EA6, XN_BN4_CODE + 0x2BD0, 0x3A0A5E3Cu },
	{ 0x080F0EA6, 0x080F0F1C, XN_BN4_CODE + 0x2C02, 0xF0515411u },
	{ 0x080F0F1C, 0x080F0F64, XN_BN4_CODE + 0x2C80, 0x1EBAB1B9u },
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
	{ 0x08002E20, 0x08002DB4 },   /* project:windman, SAME */
	{ 0x08002E34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002F22, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002F2E, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002F54, 0x08002F02 },   /* project:windman, SAME */
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
	{ 0x08010E88, 0x080174FE },   /* project:windman, DIFF (audited) */
	{ 0x08010F08, 0x080175B8 },   /* project:windman, DIFF (audited) */
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
	{ 0x080CCF38, THUMB_MOVS, 0, 0, 0x5C, 0xE0 },   /* its companion's type 0x5c (the first kind): moved, the dispatch's 0xe0 */
	{ 0x080D9C84, THUMB_MOVS, 2, 0, 0x2A, 0xCD },   /* its object's sprite: list 4 0x2a in BN4, list 4 0xcd in BN6 (copied) */
	{ 0x080D9CC0, THUMB_MOVS, 0, 0, 0xB2, 0x32 },   /* its sound 0xb2: BN4's copied into BN6's empty slot 0x32, on its player 21 */
	{ 0x080D9D1A, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D9D48, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D9D56, THUMB_MOVS, 4, 0, 0x0, 0x9A },   /* its effect: 0x0 in BN4, 0x9a in BN6 (moved) */
	{ 0x080D9D5C, THUMB_MOVS, 0, 0, 0xC8, 0x33 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0x33, on its player 11 */
	{ 0x080D9DB6, THUMB_MOVS, 0, 0, 0x63, 0xE9 },   /* its object's type 0x63: moved, the dispatch's 0xe9 */
	{ 0x080D9DC8, THUMB_STRB, 2, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D9FCC, THUMB_MOVS, 2, 0, 0x2B, 0xCE },   /* its object's sprite: list 4 0x2b in BN4, list 4 0xce in BN6 (copied) */
	{ 0x080DA054, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DA082, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080DA090, THUMB_MOVS, 4, 0, 0x0, 0x9A },   /* its effect: 0x0 in BN4, 0x9a in BN6 (moved) */
	{ 0x080DA096, THUMB_MOVS, 0, 0, 0xC8, 0x33 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0x33, on its player 11 */
	{ 0x080DA0D4, THUMB_MOVS, 0, 0, 0x34, 0xD0 },   /* its sound 0x133 (a movs and an adds): BN4's copied into BN6's slot 0xd0 */
	{ 0x080DA0D6, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x133: the adds that took it past 0xFF, 0 (its slot by the movs alone) */
	{ 0x080DA1E6, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080DA1FA, THUMB_MOVS, 0, 0, 0x65, 0xEA },   /* its object's type 0x65: moved, the dispatch's 0xea */
	{ 0x080DA20A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DA46A, THUMB_MOVS, 2, 0, 0x2F, 0xCF },   /* its object's sprite: list 4 0x2f in BN4, list 4 0xcf in BN6 (copied) */
	{ 0x080DA4A4, THUMB_MOVS, 0, 0, 0x27, 0x63 },   /* its sound 0x126 (a movs and an adds): BN4's copied into BN6's slot 0x63 */
	{ 0x080DA4A6, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x126: the adds that took it past 0xFF, 0 (its slot by the movs alone) */
	{ 0x080DA4E6, THUMB_LDR, 0, 2, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DA5E0, THUMB_LDRB, 0, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080DA694, THUMB_MOVS, 0, 0, 0x67, 0xEB },   /* its object's type 0x67: moved, the dispatch's 0xeb */
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
	{ 0x080CCC99, 0x28, 0xCB },   /* its table's kind 2: list 4 0x28 in BN4, 0xcb copied */
	{ 0x080CCC9B, 0x29, 0xCC },   /* its table's kind 3: list 4 0x29 in BN4, 0xcc copied */
};
static const XvType windman_types[] = {
	{ 3, 0xE9, XV_TYPE_MOVED, 0x63 },   /* BN4's third-kind 0x63, moved, the dispatch's 0xe9 */
	{ 3, 0xEA, XV_TYPE_MOVED, 0x65 },   /* BN4's third-kind 0x65, moved, the dispatch's 0xea */
	{ 3, 0xEB, XV_TYPE_MOVED, 0x67 },   /* BN4's third-kind 0x67, moved, the dispatch's 0xeb */
	{ 1, 0xE0, XV_TYPE_MOVED, 0x5C },   /* BN4's first-kind 0x5c, moved, the dispatch's 0xe0 */
};
static const XvRef windman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), OWN(0x080C3321), OWN(0x080C333D), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C3359), OWN(0x080F0AF9), OWN(0x080F0BCF), OWN(0x080F0C9F), OWN(0x080F0DC5),
};
static const XvSprite windman_sprites[] = {
	{ 2, 0x03, 2, 0x42, XN_BN4_ART + 0x12C00, 0xA000 },
	{ 4, 0x28, 4, 0xCB, XN_BN4_ART + 0x1CC00, 0x800 },
	{ 4, 0x29, 4, 0xCC, XN_BN4_ART + 0x1D400, 0x800 },
	{ 4, 0x2A, 4, 0xCD, XN_BN4_ART + 0x1DC00, 0x800 },
	{ 4, 0x2B, 4, 0xCE, XN_BN4_ART + 0x1E400, 0x800 },
	{ 4, 0x2F, 4, 0xCF, XN_BN4_ART + 0x1EC00, 0x1000 },
	{ 5, 0x00, 5, 0x9B, XN_BN4_ART + 0x1FC00, 0x1C00 },
};
static const XvId windman_effects[] = { { 0x00, 0x9A }, { 0x15, 0x90 } };
static const XvSong windman_songs[] = {
	{ 0xB2, 0x32, XN_BN4_ART + 0x21800, 0x1640 },   /* its sound 0xb2, BN4's song copied */
	{ 0xC8, 0x33, XN_BN4_ART + 0x22E40, 0xB40 },   /* its sound 0xc8, BN4's song copied */
	{ 0x126, 0x63, XN_BN4_ART + 0x23980, 0x1F00 },   /* its sound 0x126, BN4's song copied */
	{ 0x133, 0xD0, XN_BN4_ART + 0x25880, 0x16C0 },   /* its sound 0x133, BN4's song copied */
};

static const XnCompanion windman_companions[] = {
	{ OWN(0x080CCF35), 2, 0x60, 0x60, 0x00 },   /* 080C3292: its first-kind 0x5c of kind 2 */
	{ OWN(0x080CCF35), 3, 0x60, 0x60, 0x64 },   /* 080C32A2: its first-kind 0x5c of kind 3 */
};

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
		.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 2, .flags = 0x06 },
		.sprites = windman_sprites, .nsprites = (int)(sizeof windman_sprites / sizeof *windman_sprites),
		.effects = windman_effects, .neffects = (int)(sizeof windman_effects / sizeof *windman_effects),
		.songs = windman_songs, .nsongs = (int)(sizeof windman_songs / sizeof *windman_songs),   /* (its sounds in the pool: xnavi_bn4_sounds.c) */
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = -1,   /* (its battle sprite its recipe's first: list 2 at its seat's number, XNAVI_SPRITE_BN4) */
	.companions = windman_companions, .ncompanions = (int)(sizeof windman_companions / sizeof *windman_companions),
};
const XNavi *const xnavi_bn4[] = { &bn4_gutsman };
const int xnavi_bn4_count = (int)(sizeof xnavi_bn4 / sizeof *xnavi_bn4);

/* held from the install (tools/xnavirecipe_bn4.py HELD): WindMan: with his companions set out (his companion start, xnavi_install.c) BN6's battle never reaches its fight (windman_v1: the run fails at the opening); without them it does */
const XNavi *const xnavi_bn4_held[] = { &bn4_windman };
const int xnavi_bn4_held_count = (int)(sizeof xnavi_bn4_held / sizeof *xnavi_bn4_held);

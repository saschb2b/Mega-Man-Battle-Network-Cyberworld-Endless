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
	{ 2, 0x02, 2, 0x41, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x04, 4, 0xC9, XN_BN4_ART + 0x0, 0x1000 },
	{ 4, 0x0E, 4, 0xCA, XN_BN4_ART + 0x1000, 0x800 },
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
};
/* ---- BN4's FireMan (its type 0xc there, its enemy ids 144-147), as Navi AI 52 with ids 0x390-0x393 ---- */
/* interim: its sound 0x8f plays 1 of its 2 tracks: BN6's player 30 has 1 where BN4's has 2 (BN6_MPLAYS against BN4_MPLAYS), the rest let go as BN6's MPlayStart starts no more tracks than its player has */

static const XvBlock fireman_blocks[] = {
	{ 0x080B7DB4, 0x080B820C, XN_BN4_CODE + 0x16C0, 0x6A76CB41u },
	{ 0x080CC3D8, 0x080CC624, XN_BN4_CODE + 0x1B20, 0xCFB85A08u },
	{ 0x080D0FFC, 0x080D10D0, XN_BN4_CODE + 0x1D70, 0xD14BD3EDu },
	{ 0x080CFF80, 0x080D014E, XN_BN4_CODE + 0x1E50, 0x12544EF6u },
	{ 0x080D014E, 0x080D02F8, XN_BN4_CODE + 0x2022, 0xEED2A840u },
	{ 0x080CE2D8, 0x080CE4D0, XN_BN4_CODE + 0x21D0, 0x00B1870Eu },
	{ 0x080D0BBC, 0x080D0D64, XN_BN4_CODE + 0x23D0, 0x8906650Cu },
	{ 0x080CE4D0, 0x080CE6E0, XN_BN4_CODE + 0x2580, 0x842E9CE2u },
	{ 0x080B7D98, 0x080B7DB4, XN_BN4_CODE + 0x2790, 0xA9730ED9u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0x27B2, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x2BC2, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x2BE0, 0xA9D6138Du },
	{ 0x0800C9FC, 0x0800CA28, XN_BN4_CODE + 0x2BF0, 0x3A5341C2u },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x2C20, 0xDCE85B3Du },
	{ 0x0800F734, 0x0800F780, XN_BN4_CODE + 0x2C50, 0x0198FB6Eu },
	{ 0x08014446, 0x0801447A, XN_BN4_CODE + 0x2CA2, 0x6AADAE25u },
	{ 0x080EDD2E, 0x080EDD54, XN_BN4_CODE + 0x2CE2, 0xE0FEDFDBu },
	{ 0x080EDD54, 0x080EDDA4, XN_BN4_CODE + 0x2D10, 0x044A19FDu },
	{ 0x080EDDA4, 0x080EDDEA, XN_BN4_CODE + 0x2D60, 0xB569EFE0u },
	{ 0x080EDDEA, 0x080EDE0E, XN_BN4_CODE + 0x2DB2, 0x8D466920u },
	{ 0x080F077C, 0x080F0782, XN_BN4_CODE + 0x2DE0, 0x4A7133B7u },
	{ 0x080F1810, 0x080F1834, XN_BN4_CODE + 0x2DF0, 0x8E543822u },
	{ 0x080F1834, 0x080F1848, XN_BN4_CODE + 0x2E20, 0xE4B8125Au },
	{ 0x080F1848, 0x080F18CA, XN_BN4_CODE + 0x2E40, 0xDD103622u },
	{ 0x080F18CA, 0x080F18F4, XN_BN4_CODE + 0x2ED2, 0x47D27B46u },
	{ 0x080F18F4, 0x080F1918, XN_BN4_CODE + 0x2F00, 0x1E8EC382u },
	{ 0x080F1918, 0x080F1948, XN_BN4_CODE + 0x2F30, 0x61728EEAu },
	{ 0x080F1948, 0x080F19A8, XN_BN4_CODE + 0x2F60, 0x40750D73u },
	{ 0x080F19A8, 0x080F19D6, XN_BN4_CODE + 0x2FC0, 0xE1F51CE0u },
	{ 0x080F19D6, 0x080F19FC, XN_BN4_CODE + 0x2FF2, 0xA6CAAB71u },
	{ 0x080F19FC, 0x080F1A3A, XN_BN4_CODE + 0x3020, 0x7C0EA509u },
	{ 0x080F1A3A, 0x080F1A82, XN_BN4_CODE + 0x3062, 0xC0E4B18Bu },
	{ 0x080F1A82, 0x080F1AAC, XN_BN4_CODE + 0x30B2, 0x1725092Fu },
	{ 0x080F1AAC, 0x080F1AD0, XN_BN4_CODE + 0x30E0, 0xA812514Bu },
	{ 0x080F1AD0, 0x080F1AF6, XN_BN4_CODE + 0x3110, 0xF7DA1FF7u },
	{ 0x080F1AF6, 0x080F1B3E, XN_BN4_CODE + 0x3142, 0xC32FB049u },
	{ 0x080F1B3E, 0x080F1B68, XN_BN4_CODE + 0x3192, 0x33C1CD7Du },
	{ 0x080F1B68, 0x080F1B70, XN_BN4_CODE + 0x31C0, 0xF4AF7FC1u },
	{ 0x080F1B70, 0x080F1C44, XN_BN4_CODE + 0x31D0, 0x182E2204u },
	{ 0x080F1C44, 0x080F1C50, XN_BN4_CODE + 0x32B0, 0x72B92A5Fu },
	{ 0x080F1C50, 0x080F1C60, XN_BN4_CODE + 0x32C0, 0x0AE0F513u },
	{ 0x080F1C60, 0x080F1CB2, XN_BN4_CODE + 0x32D0, 0xAD6236D8u },
	{ 0x080F1CB2, 0x080F1D4C, XN_BN4_CODE + 0x3332, 0x753C82C3u },
	{ 0x080F1D4C, 0x080F1D98, XN_BN4_CODE + 0x33D0, 0x7FE65FDEu },
	{ 0x080F1D98, 0x080F1DC4, XN_BN4_CODE + 0x3420, 0xB3CAF049u },
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
	{ 0x080CC5F8, THUMB_MOVS, 0, 0, 0x57, 0xE0 },   /* its companion's type 0x57 (the first kind): moved, the dispatch's 0xe0 */
	{ 0x080CE31C, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080CE31E, THUMB_MOVS, 2, 0, 0xE, 0xCB },   /* its object's sprite: list 3 0xe in BN4, list 4 0xcb in BN6 (copied) */
	{ 0x080CE3D2, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE47E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE4A0, THUMB_MOVS, 0, 0, 0x9, 0xE9 },   /* its object's type 0x9: moved, the dispatch's 0xe9 */
	{ 0x080CE4B0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE4F6, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080CE4F8, THUMB_MOVS, 2, 0, 0x11, 0xCC },   /* its object's sprite: list 3 0x11 in BN4, list 4 0xcc in BN6 (copied) */
	{ 0x080CE544, THUMB_MOVS, 0, 0, 0xA9, 0x63 },   /* its sound 0xa9: BN4's copied into BN6's empty slot 0x63, on its player 9 */
	{ 0x080CE570, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE592, THUMB_MOVS, 0, 0, 0xA, 0xEA },   /* its object's type 0xa: moved, the dispatch's 0xea */
	{ 0x080CE5A2, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CFFAE, THUMB_MOVS, 2, 0, 0xA, 0xCE },   /* its object's sprite: list 4 0xa in BN4, list 4 0xce in BN6 (copied) */
	{ 0x080CFFF8, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D0014, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D00BC, THUMB_MOVS, 0, 0, 0x19, 0xEB },   /* its object's type 0x19: moved, the dispatch's 0xeb */
	{ 0x080D00CC, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D01C2, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D021C, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D0248, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D02DE, THUMB_MOVS, 0, 0, 0x1A, 0xEC },   /* its object's type 0x1a: moved, the dispatch's 0xec */
	{ 0x080D0BE2, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080D0BE4, THUMB_MOVS, 2, 0, 0x2, 0xCD },   /* its object's sprite: list 3 0x2 in BN4, list 4 0xcd in BN6 (copied) */
	{ 0x080D0C64, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D0C80, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0CE0, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0D2E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0D46, THUMB_MOVS, 0, 0, 0x1F, 0xED },   /* its object's type 0x1f: moved, the dispatch's 0xed */
	{ 0x080D0D58, THUMB_STRB, 1, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D1036, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D107E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D10B4, THUMB_MOVS, 0, 0, 0x21, 0xEE },   /* its object's type 0x21: moved, the dispatch's 0xee */
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
	{ 3, 0xE9, XV_TYPE_MOVED, 0x09 },   /* BN4's third-kind 0x9, moved, the dispatch's 0xe9 */
	{ 3, 0xEA, XV_TYPE_MOVED, 0x0A },   /* BN4's third-kind 0xa, moved, the dispatch's 0xea */
	{ 3, 0xEB, XV_TYPE_MOVED, 0x19 },   /* BN4's third-kind 0x19, moved, the dispatch's 0xeb */
	{ 3, 0xEC, XV_TYPE_MOVED, 0x1A },   /* BN4's third-kind 0x1a, moved, the dispatch's 0xec */
	{ 3, 0xED, XV_TYPE_MOVED, 0x1F },   /* BN4's third-kind 0x1f, moved, the dispatch's 0xed */
	{ 3, 0xEE, XV_TYPE_MOVED, 0x21 },   /* BN4's third-kind 0x21, moved, the dispatch's 0xee */
	{ 1, 0xE0, XV_TYPE_MOVED, 0x57 },   /* BN4's third-kind 0x57, moved, the dispatch's 0xe0 */
};
static const XvRef fireman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), OWN(0x080B7E8D), OWN(0x080B7EA1), OWN(0x080B7EB5), BN6(0x08017689), BN6(0x08017769), OWN(0x080B7EC9), OWN(0x080EDD2F), OWN(0x080F1811), OWN(0x080F18F5), OWN(0x080F19D7), OWN(0x080F1AAD),
};
static const XvSprite fireman_sprites[] = {
	{ 2, 0x05, 2, 0x44, XN_BN4_ART + 0x15C00, 0xB000 },
	{ 3, 0x0E, 4, 0xCB, XN_BN4_ART + 0x12C00, 0x1000 },
	{ 3, 0x11, 4, 0xCC, XN_BN4_ART + 0x13C00, 0x1000 },
	{ 3, 0x02, 4, 0xCD, XN_BN4_ART + 0x14C00, 0x1000 },
	{ 4, 0x0A, 4, 0xCE, XN_BN4_ART + 0x20C00, 0x2000 },
	{ 4, 0x08, 4, 0xCF, XN_BN4_ART + 0x22C00, 0x1800 },
};
static const XvId fireman_effects[] = { { 0x15, 0x90 } };
static const XvSong fireman_songs[] = {
	{ 0x8F, 0x32, XN_BN4_ART + 0x24400, 0x80 },   /* its sound 0x8f, BN4's song copied */
	{ 0xA8, 0x33, XN_BN4_ART + 0x24480, 0x40 },   /* its sound 0xa8, BN4's song copied */
	{ 0xA9, 0x63, XN_BN4_ART + 0x244C0, 0x40 },   /* its sound 0xa9, BN4's song copied */
	{ 0xC0, 0xD0, XN_BN4_ART + 0x24500, 0x880 },   /* its sound 0xc0, BN4's song copied */
};


static const XnSpawned fireman_spawned = { OWN(0x080CC5F5), 0xCF10, 0x74, true, true };   /* (its init's tail at its spawn's end: its companion, the first kind's 0x57, BN4's word 0x810) */

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
	.spawned = &fireman_spawned,
};
const XNavi *const xnavi_bn4[] = { &bn4_gutsman, &bn4_fireman };
const int xnavi_bn4_count = (int)(sizeof xnavi_bn4 / sizeof *xnavi_bn4);

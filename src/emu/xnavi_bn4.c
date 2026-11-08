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
	{ 0x080DAAB8, THUMB_MOVS, 2, 0, 0x2D, 0xCB },   /* its object's sprite: list 4 0x2d in BN4, list 4 0xcb in BN6 (copied) */
	{ 0x080DAAF2, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DAC28, THUMB_MOVS, 0, 0, 0x6A, 0xE9 },   /* its object's type 0x6a: moved, the dispatch's 0xe9 */
	{ 0x080DB2AC, THUMB_MOVS, 2, 0, 0x11, 0xCC },   /* its object's sprite: list 4 0x11 in BN4, list 4 0xcc in BN6 (copied) */
	{ 0x080DB2F0, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DB342, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DB39C, THUMB_MOVS, 0, 0, 0x49, 0xD0 },   /* its sound 0x148 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DB39E, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x148 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DB3B0, THUMB_MOVS, 0, 0, 0xB8, 0x33 },   /* its sound 0xb8: BN4's copied into BN6's empty slot 0x33, on its player 17 */
	{ 0x080DB3D2, THUMB_MOVS, 0, 0, 0x6F, 0xEA },   /* its object's type 0x6f: moved, the dispatch's 0xea */
	{ 0x080DB420, THUMB_MOVS, 2, 0, 0x2E, 0xCD },   /* its object's sprite: list 4 0x2e in BN4, list 4 0xcd in BN6 (copied) */
	{ 0x080DB4A8, THUMB_MOVS, 0, 0, 0xAD, 0x32 },   /* its sound 0xad: BN4's copied into BN6's empty slot 0x32, on its player 26 */
	{ 0x080DB528, THUMB_MOVS, 0, 0, 0x70, 0xEB },   /* its object's type 0x70: moved, the dispatch's 0xeb */
	{ 0x080DB568, THUMB_MOVS, 2, 0, 0x14, 0x9B },   /* its object's sprite: list 5 0x14 in BN4, 0x9b in BN6 (copied) */
	{ 0x080DB59C, THUMB_STRB, 1, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DB5E2, THUMB_LDR, 0, 3, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DB672, THUMB_MOVS, 0, 0, 0x44, 0x63 },   /* its sound 0x143 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DB674, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x143 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DB692, THUMB_MOVS, 4, 0, 0x47, 0x9A },   /* its effect: 0x47 in BN4, 0x9a in BN6 (moved) */
	{ 0x080DB6D2, THUMB_MOVS, 0, 0, 0x71, 0xEC },   /* its object's type 0x71: moved, the dispatch's 0xec */
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
	{ 3, 0xE9, XV_TYPE_MOVED, 0x6A },   /* BN4's third-kind 0x6a, moved, the dispatch's 0xe9 */
	{ 3, 0xEA, XV_TYPE_MOVED, 0x6F },   /* BN4's third-kind 0x6f, moved, the dispatch's 0xea */
	{ 3, 0xEB, XV_TYPE_MOVED, 0x70 },   /* BN4's third-kind 0x70, moved, the dispatch's 0xeb */
	{ 3, 0xEC, XV_TYPE_MOVED, 0x71 },   /* BN4's third-kind 0x71, moved, the dispatch's 0xec */
	{ 1, 0xE0, XV_TYPE_MOVED, 0x33 },   /* BN4's first-kind 0x33, moved, the dispatch's 0xe0 */
};
static const XvRef thunderman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C38D5), OWN(0x080F20FD), OWN(0x080F21B1), OWN(0x080F22FD),
};
static const XvSprite thunderman_sprites[] = {
	{ 2, 0x06, 2, 0x45, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 4, 0x2D, 4, 0xCB, XN_BN4_ART + 0x3C00, 0x2000 },
	{ 4, 0x11, 4, 0xCC, XN_BN4_ART + 0x5C00, 0x1000 },
	{ 4, 0x2E, 4, 0xCD, XN_BN4_ART + 0x6C00, 0x800 },
	{ 5, 0x14, 5, 0x9B, XN_BN4_ART + 0x7400, 0x700 },
	{ 5, 0x13, 5, 0x9C, XN_BN4_ART + 0x7B00, 0xC00 },
};
static const XvId thunderman_effects[] = { { 0x47, 0x9A } };
static const XvSong thunderman_songs[] = {
	{ 0xAD, 0x32, XN_BN4_ART + 0x8700, 0xB80 },   /* its sound 0xad, BN4's song copied */
	{ 0xB8, 0x33, XN_BN4_ART + 0x9280, 0x840 },   /* its sound 0xb8, BN4's song copied */
	{ 0x143, 0x63, XN_BN4_ART + 0x9AC0, 0x840 },   /* its sound 0x143, BN4's song copied */
	{ 0x148, 0xD0, XN_BN4_ART + 0xA300, 0x840 },   /* its sound 0x148, BN4's song copied */
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
	{ 0x080D9C84, THUMB_MOVS, 2, 0, 0x2A, 0xD0 },   /* its object's sprite: list 4 0x2a in BN4, list 4 0xd0 in BN6 (copied) */
	{ 0x080D9CC0, THUMB_MOVS, 0, 0, 0xB2, 0x32 },   /* its sound 0xb2: BN4's copied into BN6's empty slot 0x32, on its player 21 */
	{ 0x080D9D1A, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D9D48, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D9D56, THUMB_MOVS, 4, 0, 0x0, 0x9B },   /* its effect: 0x0 in BN4, 0x9b in BN6 (moved) */
	{ 0x080D9D5C, THUMB_MOVS, 0, 0, 0xC8, 0x33 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0x33, on its player 11 */
	{ 0x080D9DB6, THUMB_MOVS, 0, 0, 0x63, 0xED },   /* its object's type 0x63: moved, the dispatch's 0xed */
	{ 0x080D9DC8, THUMB_STRB, 2, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D9FCC, THUMB_MOVS, 2, 0, 0x2B, 0xD1 },   /* its object's sprite: list 4 0x2b in BN4, list 4 0xd1 in BN6 (copied) */
	{ 0x080DA054, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DA082, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080DA090, THUMB_MOVS, 4, 0, 0x0, 0x9B },   /* its effect: 0x0 in BN4, 0x9b in BN6 (moved) */
	{ 0x080DA096, THUMB_MOVS, 0, 0, 0xC8, 0x33 },   /* its sound 0xc8: BN4's copied into BN6's empty slot 0x33, on its player 11 */
	{ 0x080DA0D4, THUMB_MOVS, 0, 0, 0x34, 0xD0 },   /* its sound 0x133 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DA0D6, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x133 (movs and adds #0xff) at the pool's 0xd0 */
	{ 0x080DA1E6, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080DA1FA, THUMB_MOVS, 0, 0, 0x65, 0xEE },   /* its object's type 0x65: moved, the dispatch's 0xee */
	{ 0x080DA20A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080DA46A, THUMB_MOVS, 2, 0, 0x2F, 0xD2 },   /* its object's sprite: list 4 0x2f in BN4, list 4 0xd2 in BN6 (copied) */
	{ 0x080DA4A4, THUMB_MOVS, 0, 0, 0x27, 0x63 },   /* its sound 0x126 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DA4A6, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x126 (movs and adds #0xff) at the pool's 0x63 */
	{ 0x080DA4E6, THUMB_LDR, 0, 2, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DA5E0, THUMB_LDRB, 0, 0, 0x1, 0x3 },   /* a panel's side: BN4 +0x1, BN6 +0x3 */
	{ 0x080DA694, THUMB_MOVS, 0, 0, 0x67, 0xEF },   /* its object's type 0x67: moved, the dispatch's 0xef */
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
	{ 0x080CCC99, 0x28, 0xCE },   /* its table's kind 2: list 4 0x28 in BN4, 0xce copied */
	{ 0x080CCC9B, 0x29, 0xCF },   /* its table's kind 3: list 4 0x29 in BN4, 0xcf copied */
};
static const XvType windman_types[] = {
	{ 3, 0xED, XV_TYPE_MOVED, 0x63 },   /* BN4's third-kind 0x63, moved, the dispatch's 0xed */
	{ 3, 0xEE, XV_TYPE_MOVED, 0x65 },   /* BN4's third-kind 0x65, moved, the dispatch's 0xee */
	{ 3, 0xEF, XV_TYPE_MOVED, 0x67 },   /* BN4's third-kind 0x67, moved, the dispatch's 0xef */
	{ 1, 0xE1, XV_TYPE_MOVED, 0x5C },   /* BN4's first-kind 0x5c, moved, the dispatch's 0xe1 */
};
static const XvRef windman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), OWN(0x080C3321), OWN(0x080C333D), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080C3359), OWN(0x080F0AF9), OWN(0x080F0BCF), OWN(0x080F0C9F), OWN(0x080F0DC5),
};
static const XvSprite windman_sprites[] = {
	{ 2, 0x03, 2, 0x42, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 4, 0x28, 4, 0xCE, XN_BN4_ART + 0xAB40, 0x800 },
	{ 4, 0x29, 4, 0xCF, XN_BN4_ART + 0xB340, 0x800 },
	{ 4, 0x2A, 4, 0xD0, XN_BN4_ART + 0xBB40, 0x800 },
	{ 4, 0x2B, 4, 0xD1, XN_BN4_ART + 0xC340, 0x800 },
	{ 4, 0x2F, 4, 0xD2, XN_BN4_ART + 0xCB40, 0x1000 },
	{ 5, 0x00, 5, 0x9D, XN_BN4_ART + 0xDB40, 0x1C00 },
};
static const XvId windman_effects[] = { { 0x00, 0x9B }, { 0x15, 0x90 } };
static const XvSong windman_songs[] = {
	{ 0xB2, 0x32, XN_BN4_ART + 0xF740, 0x1640 },   /* its sound 0xb2, BN4's song copied */
	{ 0xC8, 0x33, XN_BN4_ART + 0x10D80, 0xB40 },   /* its sound 0xc8, BN4's song copied */
	{ 0x126, 0x63, XN_BN4_ART + 0x118C0, 0x1F00 },   /* its sound 0x126, BN4's song copied */
	{ 0x133, 0xD0, XN_BN4_ART + 0x137C0, 0x16C0 },   /* its sound 0x133, BN4's song copied */
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
	{ 0x080CE31E, THUMB_MOVS, 2, 0, 0xE, 0xD3 },   /* its object's sprite: list 3 0xe in BN4, list 4 0xd3 in BN6 (copied) */
	{ 0x080CE358, THUMB_LDRB, 0, 5, 0xE, 0xF },   /* its palette: BN4 +0x0E, BN6 +0x0F (its +0x0E the element) */
	{ 0x080CE3D2, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE47E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE486, THUMB_LDRB, 7, 5, 0xE, 0xF },   /* its palette: BN4 +0x0E, BN6 +0x0F (its +0x0E the element) */
	{ 0x080CE4A0, THUMB_MOVS, 0, 0, 0x9, 0xF0 },   /* its object's type 0x9: moved, the dispatch's 0xf0 */
	{ 0x080CE4B0, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE4BC, THUMB_STRB, 7, 0, 0xE, 0xF },   /* the object it spawned: palette: BN4 +0x0E, BN6 +0x0F (its +0x0E the element) */
	{ 0x080CE4F6, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080CE4F8, THUMB_MOVS, 2, 0, 0x11, 0xD4 },   /* its object's sprite: list 3 0x11 in BN4, list 4 0xd4 in BN6 (copied) */
	{ 0x080CE544, THUMB_MOVS, 0, 0, 0xA9, 0x63 },   /* its sound 0xa9: BN4's copied into BN6's empty slot 0x63, on its player 9 */
	{ 0x080CE570, THUMB_LDR, 0, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE592, THUMB_MOVS, 0, 0, 0xA, 0xF1 },   /* its object's type 0xa: moved, the dispatch's 0xf1 */
	{ 0x080CE5A2, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CFFAE, THUMB_MOVS, 2, 0, 0xA, 0xD6 },   /* its object's sprite: list 4 0xa in BN4, list 4 0xd6 in BN6 (copied) */
	{ 0x080CFFF8, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D0014, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D00BC, THUMB_MOVS, 0, 0, 0x19, 0xF2 },   /* its object's type 0x19: moved, the dispatch's 0xf2 */
	{ 0x080D00CC, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D01C2, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D021C, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D0248, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D02DE, THUMB_MOVS, 0, 0, 0x1A, 0xF3 },   /* its object's type 0x1a: moved, the dispatch's 0xf3 */
	{ 0x080D0BE2, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its object's sprite's list: 3 in BN4, copied into 4 (LIST_MOVES) */
	{ 0x080D0BE4, THUMB_MOVS, 2, 0, 0x2, 0xD5 },   /* its object's sprite: list 3 0x2 in BN4, list 4 0xd5 in BN6 (copied) */
	{ 0x080D0C64, THUMB_LDR, 0, 1, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D0C80, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0CE0, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080D0D2E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D0D46, THUMB_MOVS, 0, 0, 0x1F, 0xF4 },   /* its object's type 0x1f: moved, the dispatch's 0xf4 */
	{ 0x080D0D58, THUMB_STRB, 1, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D1036, THUMB_MOVS, 0, 0, 0x9D, 0x36 },   /* its sound 0x9d: BN4's copied into BN6's empty slot 0x36, on its player 9 */
	{ 0x080D107E, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D10B4, THUMB_MOVS, 0, 0, 0x21, 0xF5 },   /* its object's type 0x21: moved, the dispatch's 0xf5 */
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
	{ 3, 0xF0, XV_TYPE_MOVED, 0x09 },   /* BN4's third-kind 0x9, moved, the dispatch's 0xf0 */
	{ 3, 0xF1, XV_TYPE_MOVED, 0x0A },   /* BN4's third-kind 0xa, moved, the dispatch's 0xf1 */
	{ 3, 0xF2, XV_TYPE_MOVED, 0x19 },   /* BN4's third-kind 0x19, moved, the dispatch's 0xf2 */
	{ 3, 0xF3, XV_TYPE_MOVED, 0x1A },   /* BN4's third-kind 0x1a, moved, the dispatch's 0xf3 */
	{ 3, 0xF4, XV_TYPE_MOVED, 0x1F },   /* BN4's third-kind 0x1f, moved, the dispatch's 0xf4 */
	{ 3, 0xF5, XV_TYPE_MOVED, 0x21 },   /* BN4's third-kind 0x21, moved, the dispatch's 0xf5 */
	{ 1, 0xE2, XV_TYPE_MOVED, 0x57 },   /* BN4's first-kind 0x57, moved, the dispatch's 0xe2 */
};
static const XvRef fireman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), OWN(0x080B7E8D), OWN(0x080B7EA1), OWN(0x080B7EB5), BN6(0x08017689), BN6(0x08017769), OWN(0x080B7EC9), OWN(0x080EDD2F), OWN(0x080F1811), OWN(0x080F18F5), OWN(0x080F19D7), OWN(0x080F1AAD),
};
static const XvSprite fireman_sprites[] = {
	{ 2, 0x05, 2, 0x44, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x0E, 4, 0xD3, XN_BN4_ART + 0x14E80, 0x1000 },
	{ 3, 0x11, 4, 0xD4, XN_BN4_ART + 0x15E80, 0x1000 },
	{ 3, 0x02, 4, 0xD5, XN_BN4_ART + 0x16E80, 0x1000 },
	{ 4, 0x0A, 4, 0xD6, XN_BN4_ART + 0x17E80, 0x2000 },
	{ 4, 0x08, 4, 0xD7, XN_BN4_ART + 0x19E80, 0x1800 },
};
static const XvId fireman_effects[] = { { 0x15, 0x90 } };
static const XvSong fireman_songs[] = {
	{ 0x8F, 0x32, XN_BN4_ART + 0x1B680, 0x80 },   /* its sound 0x8f, BN4's song copied */
	{ 0xA8, 0x33, XN_BN4_ART + 0x1B700, 0x40 },   /* its sound 0xa8, BN4's song copied */
	{ 0xA9, 0x63, XN_BN4_ART + 0x1B740, 0x40 },   /* its sound 0xa9, BN4's song copied */
	{ 0xC0, 0xD0, XN_BN4_ART + 0x1B780, 0x880 },   /* its sound 0xc0, BN4's song copied */
};
static const XnSpawn fireman_spawns[] = {
	{ OWN(0x080CC5F5), 0xD710, 0x00000000, 0x00, 0x00, 0x00, 0x74 },   /* 080B7E16: its first-kind 0x57, BN4 r4 0x810 */
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
	.init = &fireman_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
};
/* ---- BN4's Roll (its type 0x1d there, its enemy ids 128-131), as Navi AI 48 with ids 0x380-0x383 ---- */

static const XvBlock roll_blocks[] = {
	{ 0x080BC0E4, 0x080BC3FC, XN_BN4_CODE + 0x5FE0, 0x6626EF64u },
	{ 0x080E3A98, 0x080E3AE6, XN_BN4_CODE + 0x6300, 0xFF36B97Eu },
	{ 0x080CE6E0, 0x080CE840, XN_BN4_CODE + 0x6350, 0x5AA41525u },
	{ 0x080CD5F8, 0x080CD728, XN_BN4_CODE + 0x64B0, 0x0069BF7Du },
	{ 0x080CD524, 0x080CD5F8, XN_BN4_CODE + 0x65E0, 0x40F51A6Du },
	{ 0x080CE5B0, 0x080CE6E0, XN_BN4_CODE + 0x66C0, 0x69329294u },
	{ 0x0800A244, 0x0800A2BE, XN_BN4_CODE + 0x67F0, 0x6300CCC3u },
	{ 0x0800AD2A, 0x0800B12C, XN_BN4_CODE + 0x6872, 0x793106FEu },
	{ 0x0800BDAA, 0x0800BDBC, XN_BN4_CODE + 0x6C82, 0xFBBA31FBu },
	{ 0x0800C9AC, 0x0800C9BA, XN_BN4_CODE + 0x6CA0, 0xA9D6138Du },
	{ 0x0800CA28, 0x0800CA4A, XN_BN4_CODE + 0x6CB0, 0xDCE85B3Du },
	{ 0x080EF87C, 0x080EF8B4, XN_BN4_CODE + 0x6CE0, 0x909D30CEu },
	{ 0x080EF8B4, 0x080EF90A, XN_BN4_CODE + 0x6D20, 0xD447727Cu },
	{ 0x080EF90A, 0x080EF944, XN_BN4_CODE + 0x6D82, 0x97D23364u },
	{ 0x080EF944, 0x080EF962, XN_BN4_CODE + 0x6DC0, 0x3F481C11u },
	{ 0x080EF962, 0x080EF990, XN_BN4_CODE + 0x6DE2, 0xC28BAFD0u },
	{ 0x080EF990, 0x080EFA14, XN_BN4_CODE + 0x6E10, 0x976788CDu },
	{ 0x080EFA14, 0x080EFA3E, XN_BN4_CODE + 0x6EA0, 0x2375C002u },
	{ 0x080EFA3E, 0x080EFA7C, XN_BN4_CODE + 0x6ED2, 0xFCC783F9u },
	{ 0x080EFA7C, 0x080EFA9A, XN_BN4_CODE + 0x6F10, 0xB4138418u },
	{ 0x080EFA9A, 0x080EFAC0, XN_BN4_CODE + 0x6F32, 0x597DAEFFu },
	{ 0x080EFAC0, 0x080EFAF0, XN_BN4_CODE + 0x6F60, 0xE180FD23u },
	{ 0x080EFAF0, 0x080EFB3E, XN_BN4_CODE + 0x6F90, 0x7A26BE4Du },
	{ 0x080EFB3E, 0x080EFB60, XN_BN4_CODE + 0x6FE2, 0x246CC93Eu },
	{ 0x080EFB60, 0x080EFB90, XN_BN4_CODE + 0x7010, 0xDACF03A9u },
	{ 0x080EFB90, 0x080EFBD2, XN_BN4_CODE + 0x7040, 0xB6CCB635u },
	{ 0x080EFBD2, 0x080EFC10, XN_BN4_CODE + 0x7092, 0x78707E9Fu },
	{ 0x080EFC10, 0x080EFC42, XN_BN4_CODE + 0x70D0, 0xB0FF32BEu },
	{ 0x080EFC42, 0x080EFC9E, XN_BN4_CODE + 0x7112, 0x5F56DDE2u },
	{ 0x080EFC9E, 0x080EFCE2, XN_BN4_CODE + 0x7172, 0x6F46462Au },
	{ 0x080EFCE2, 0x080EFD06, XN_BN4_CODE + 0x71C2, 0x74AE3F84u },
	{ 0x080EFD06, 0x080EFD3C, XN_BN4_CODE + 0x71F2, 0x485979B8u },
	{ 0x080EFD3C, 0x080EFD72, XN_BN4_CODE + 0x7230, 0xF963E758u },
	{ 0x080EFD72, 0x080EFDB0, XN_BN4_CODE + 0x7272, 0xBA320A81u },
	{ 0x080EFDB0, 0x080EFE00, XN_BN4_CODE + 0x72B0, 0x58062622u },
	{ 0x080EFE00, 0x080EFE36, XN_BN4_CODE + 0x7300, 0xED4CA13Au },
	{ 0x080EFE36, 0x080EFE62, XN_BN4_CODE + 0x7342, 0x9B23EB05u },
	{ 0x080EFE62, 0x080EFEA4, XN_BN4_CODE + 0x7372, 0x3B90D376u },
	{ 0x080EFEA4, 0x080EFEC8, XN_BN4_CODE + 0x73C0, 0x81B2DD96u },
	{ 0x080EFEC8, 0x080EFEEC, XN_BN4_CODE + 0x73F0, 0xBAE07B24u },
	{ 0x080EFEEC, 0x080EFF74, XN_BN4_CODE + 0x7420, 0xB6435623u },
	{ 0x080EFF74, 0x080EFF98, XN_BN4_CODE + 0x74B0, 0x6540E1D4u },
	{ 0x080EFF98, 0x080F01C0, XN_BN4_CODE + 0x74E0, 0x1D5C7220u },
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
	{ 0x080F00F4, XN_BN4_CODE + 0x5EDC },   /* BN4_SPAWN_ENEMY: the summon's shim (xnavi_init.c), BN6's BN6_SPAWN_ENEMY_1 by the id BN6 numbers it */
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
	{ 0x080CD6E4, THUMB_MOVS, 0, 0, 0x2, 0xF6 },   /* its object's type 0x2: moved, the dispatch's 0xf6 */
	{ 0x080CE74A, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE79A, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE7D6, THUMB_MOVS, 4, 0, 0x56, 0x99 },   /* its effect: 0x56 in BN4, 0x99 in BN6 (moved) */
	{ 0x080CE812, THUMB_MOVS, 0, 0, 0xB, 0xF7 },   /* its object's type 0xb: moved, the dispatch's 0xf7 */
	{ 0x080E3ABE, THUMB_ADDS, 4, 0, 0x2E, 0x9C },   /* its effect: 0x2e in BN4, 0x9c in BN6 (moved) */
	{ 0x080E3ADC, THUMB_MOVS, 0, 0, 0x17, 0xE0 },   /* its object's type 0x17: moved, the dispatch's 0xe0 */
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
	{ 0x080CD541, 0x0F, 0xD8 },   /* its table's kind 7: list 3 0xf in BN4, list 4 0xd8 copied */
	{ 0x080CE615, 0x0C, 0x10 },   /* its table's kind 6: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE616, 0x10, 0xD9 },   /* its table's kind 6: list 3 0x10 in BN4, list 4 0xd9 copied */
	{ 0x080CE625, 0x0C, 0x10 },   /* its table's kind 7: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE626, 0x10, 0xD9 },   /* its table's kind 7: list 3 0x10 in BN4, list 4 0xd9 copied */
	{ 0x080CE635, 0x0C, 0x10 },   /* its table's kind 8: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE636, 0x10, 0xD9 },   /* its table's kind 8: list 3 0x10 in BN4, list 4 0xd9 copied */
	{ 0x080CE645, 0x0C, 0x10 },   /* its table's kind 9: its sprite's list 3 in BN4, copied into 4 */
	{ 0x080CE646, 0x10, 0xD9 },   /* its table's kind 9: list 3 0x10 in BN4, list 4 0xd9 copied */
};
static const XvType roll_types[] = {
	{ 3, 0xF6, XV_TYPE_MOVED, 0x02 },   /* BN4's third-kind 0x2, moved, the dispatch's 0xf6 */
	{ 3, 0xF7, XV_TYPE_MOVED, 0x0B },   /* BN4's third-kind 0xb, moved, the dispatch's 0xf7 */
	{ 4, 0xE0, XV_TYPE_MOVED, 0x17 },   /* BN4's fourth-kind 0x17, moved, the dispatch's 0xe0 */
};
static const XvRef roll_states[] = {
	BN6(0x08016381), OWN(0x080BC1B5), OWN(0x080BC309), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080BC1C1), OWN(0x080EF87D), OWN(0x080EFA9B), OWN(0x080EFB61), OWN(0x080EFD07), OWN(0x080EFEC9),
};
static const XvSprite roll_sprites[] = {
	{ 2, 0x01, 2, 0x40, XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE },
	{ 3, 0x0F, 4, 0xD8, XN_BN4_ART + 0x1C000, 0x1000 },
	{ 3, 0x10, 4, 0xD9, XN_BN4_ART + 0x1D000, 0x800 },
	{ 4, 0x17, 4, 0xDA, XN_BN4_ART + 0x1D800, 0x800 },
};
static const XvId roll_effects[] = { { 0x2E, 0x9C }, { 0x56, 0x99 } };
static const XvSong roll_songs[] = {
	{ 0x96, 0x32, XN_BN4_ART + 0x1E000, 0x21C0, 9 },   /* its sound 0x96, BN4's song copied, on BN6's player 9 */
	{ 0xCE, 0x33, XN_BN4_ART + 0x201C0, 0x21C0 },   /* its sound 0xce, BN4's song copied */
	{ 0x131, 0x63, XN_BN4_ART + 0x22380, 0x740, 9 },   /* its sound 0x131, BN4's song copied, on BN6's player 9 */
	{ 0x132, 0xD0, XN_BN4_ART + 0x22AC0, 0xC80 },   /* its sound 0x132, BN4's song copied */
};
static const XnInit roll_init = { NULL, 0, false, false, false, true };   /* (at its spawn's end, its +0x11, its sprite, its update) */
static const XnSummonId roll_summon_ids[] = {
	{ 0x01, 0x001 },   /* Mettaur (BN4's 40 HP), BN6's own Mettaur */
	{ 0x02, 0x002 },   /* Mettaur2, BN6's own Mettaur2 */
	{ 0x03, 0x003 },   /* Mettaur3, BN6's own Mettaur3 */
	{ 0x04, 0x004 },   /* MettaurSP, BN6's own MettaurSP */
	{ 0x0E, 0x2C1 },   /* Billy's, moved (its recipe's ids) */
	{ 0x0F, 0x2C2 },   /* Billy's, moved (its recipe's ids) */
	{ 0x10, 0x2C3 },   /* Billy's, moved (its recipe's ids) */
	{ 0x39, 0x000 },   /* its type 0x11 moved by no recipe and no native of BN6's by name: the shim spawns none (an interim) */
};
static const XnSummon roll_summon = { XN_BN4_CODE + 0x5EDC, OWN(0x0800A245), { 0x00000010, 0x0F8800A0, 0x00000030, 0x0F880080 }, roll_summon_ids, (int)(sizeof roll_summon_ids / sizeof *roll_summon_ids), 0x33 };   /* (BN4_SPAWN_ENEMY's masks by side, 080F01A8; its sound 0xce) */


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
	.init = &roll_init,   /* (its init's work past BN6's actor init: xnavi_init.c) */
	.summon = &roll_summon,   /* (its summon: xnavi_init.c) */
};
const XNavi *const xnavi_bn4[] = { &bn4_gutsman, &bn4_thunderman, &bn4_windman, &bn4_fireman, &bn4_roll };
const int xnavi_bn4_count = (int)(sizeof xnavi_bn4 / sizeof *xnavi_bn4);

/* held from the install (tools/xnavirecipe_bn4.py HELD): none */
const XNavi *const xnavi_bn4_held[] = { NULL };
const int xnavi_bn4_held_count = 0;

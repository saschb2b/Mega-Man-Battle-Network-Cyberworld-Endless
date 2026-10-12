/* xchips_bn5.c: BN5's chips' code as units for BN6's battle engine, written by tools/xchiprecipe.py from the two ROMs
 * (python3 tools/xchiprecipe.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn5_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/MULTIROM.md, The chips' code):
 * addresses in BN5 Team Colonel (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies
 * from the player's ROM; nothing of either game's. The units are the generator's, whole (no decision open), in install
 * order; src/emu/xchips_code.c installs them and src/core/xchips_moved.c names the chips that run them. */
#include "xchips_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xchips_code.h"

/* ---- CrakOut: BN5's attack 0x1b (080EC885) as BN6's 0x6b ---- */

static const XvBlock crakout_blocks[] = {
	{ 0x080C77EC, 0x080C7850, XC_CODE + 0x0, 0x931475F2u },
	{ 0x080C788C, 0x080C791C, XC_CODE + 0x70, 0x047F6F5Eu },
	{ 0x080EC884, 0x080EC8F4, XC_CODE + 0x100, 0xAA3CC7A6u },
	{ 0x080EC8FC, 0x080EC91A, XC_CODE + 0x170, 0x5D283077u },
};
static const XvData crakout_data[] = {
	{ 0x080C77FE, 0x080C7800, XV_RAW },
	{ 0x080C7800, 0x080C7804, XV_POOL },
	{ 0x080C7804, 0x080C7810, XV_TABLE },
	{ 0x080C78FA, 0x080C78FC, XV_RAW },
	{ 0x080C78FC, 0x080C7900, XV_POOL },
	{ 0x080C7900, 0x080C7904, XV_POOL },
	{ 0x080C7904, 0x080C7908, XV_POOL },
	{ 0x080C7908, 0x080C790C, XV_POOL },
	{ 0x080C790C, 0x080C791C, XV_RAW },
	{ 0x080EC892, 0x080EC894, XV_RAW },
	{ 0x080EC894, 0x080EC898, XV_POOL },
	{ 0x080EC898, 0x080EC8A0, XV_TABLE },
};
static const XvPair crakout_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080C7850, 0x080C8948 },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair crakout_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080C7851, 0x080C8949 },
};
static const XvImm crakout_imms[] = {
	{ 0x080C7814, THUMB_MOVS, 2, 0, 0xE, 0xFF },   /* its sprite: list 5 0xe in BN5, 0xff in BN6 (BN5's copied there) */
};
static const XvSprite crakout_sprites[] = {
	{ 5, 0x0E, 5, 0xFF, XC_ART + 0x0, 0x1000 },
};
static const XvType crakout_types[] = { { 3, 0x18, XV_TYPE_MOVED } };

#define CRAKOUT_CODE { .name = "CrakOut (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = crakout_blocks, .nblocks = (int)(sizeof crakout_blocks / sizeof *crakout_blocks), \
	.data = crakout_data, .ndata = (int)(sizeof crakout_data / sizeof *crakout_data), \
	.calls = crakout_calls, .ncalls = (int)(sizeof crakout_calls / sizeof *crakout_calls), \
	.words = crakout_words, .nwords = (int)(sizeof crakout_words / sizeof *crakout_words), \
	.imms = crakout_imms, .nimms = (int)(sizeof crakout_imms / sizeof *crakout_imms), \
	.sprites = crakout_sprites, .nsprites = (int)(sizeof crakout_sprites / sizeof *crakout_sprites), \
	.types = crakout_types, .ntypes = (int)(sizeof crakout_types / sizeof *crakout_types) }
/* ---- HotBody: BN5's attack 0x58 (080F0223) as BN6's 0xa8 ---- */

static const XvBlock hotbody_blocks[] = {
	{ 0x080DA478, 0x080DA5FC, XC_CODE + 0x190, 0x4CC534A3u },
	{ 0x080E6C70, 0x080E6E24, XC_CODE + 0x320, 0x73DEB67Cu },
	{ 0x080F0222, 0x080F0284, XC_CODE + 0x4E2, 0x7380EB94u },
};
static const XvData hotbody_data[] = {
	{ 0x080DA48A, 0x080DA48C, XV_RAW },
	{ 0x080DA48C, 0x080DA490, XV_POOL },
	{ 0x080DA490, 0x080DA49C, XV_TABLE },
	{ 0x080DA4E8, 0x080DA4EC, XV_POOL },
	{ 0x080DA4EC, 0x080DA4F8, XV_TABLE },
	{ 0x080DA5D2, 0x080DA5D4, XV_RAW },
	{ 0x080DA5D4, 0x080DA5DC, XV_POOL },
	{ 0x080DA5DC, 0x080DA5E4, XV_RAW },
	{ 0x080DA5E4, 0x080DA5F4, XV_RAW },
	{ 0x080DA5F4, 0x080DA5FC, XV_POOL },
	{ 0x080E6C7E, 0x080E6C80, XV_RAW },
	{ 0x080E6C80, 0x080E6C84, XV_POOL },
	{ 0x080E6C84, 0x080E6C90, XV_TABLE },
	{ 0x080E6DA4, 0x080E6DAC, XV_POOL },
	{ 0x080E6DAC, 0x080E6DB4, XV_RAW },
	{ 0x080E6DB4, 0x080E6DC4, XV_RAW },
	{ 0x080E6E1E, 0x080E6E20, XV_RAW },
	{ 0x080E6E20, 0x080E6E24, XV_POOL },
	{ 0x080F0230, 0x080F0234, XV_POOL },
	{ 0x080F0234, 0x080F023C, XV_TABLE },
};
static const XvPair hotbody_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08000800, 0x080008E0 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800085C, 0x0800093C },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
};
static const XvPair hotbody_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x080F0285, 0x080EFF23 },
};
static const XvImm hotbody_imms[] = {
	{ 0x080DA4CE, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvNumber hotbody_numbers[] = {
	{ 0x080DA5F4, 0x00001040, 0x000010FF },
	{ 0x080DA5F8, 0x04050300, 0x04050300 },
};
static const XvSprite hotbody_sprites[] = {
	{ 4, 0x40, 4, 0xFF, XC_ART + 0x1000, 0x2000 },
};
static const XvType hotbody_types[] = { { 3, 0x9D, XV_TYPE_MOVED }, { 4, 0x57, XV_TYPE_MOVED } };

#define HOTBODY_CODE { .name = "HotBody (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = hotbody_blocks, .nblocks = (int)(sizeof hotbody_blocks / sizeof *hotbody_blocks), \
	.data = hotbody_data, .ndata = (int)(sizeof hotbody_data / sizeof *hotbody_data), \
	.calls = hotbody_calls, .ncalls = (int)(sizeof hotbody_calls / sizeof *hotbody_calls), \
	.words = hotbody_words, .nwords = (int)(sizeof hotbody_words / sizeof *hotbody_words), \
	.imms = hotbody_imms, .nimms = (int)(sizeof hotbody_imms / sizeof *hotbody_imms), \
	.numbers = hotbody_numbers, .nnumbers = (int)(sizeof hotbody_numbers / sizeof *hotbody_numbers), \
	.sprites = hotbody_sprites, .nsprites = (int)(sizeof hotbody_sprites / sizeof *hotbody_sprites), \
	.types = hotbody_types, .ntypes = (int)(sizeof hotbody_types / sizeof *hotbody_types) }
/* ---- Jealousy: BN5's support 0x13 (080E4629) as BN6's 0x3d ---- */

static const XvBlock jealousy_blocks[] = {
	{ 0x080E4628, 0x080E464A, XC_CODE + 0x550, 0x26B6A490u },
};
static const XvPair jealousy_calls[] = {
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
};
static const XvType jealousy_types[] = { { 4, 0x36, 0x36 } };

#define JEALOUSY_CODE { .name = "Jealousy (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = jealousy_blocks, .nblocks = (int)(sizeof jealousy_blocks / sizeof *jealousy_blocks), \
	.calls = jealousy_calls, .ncalls = (int)(sizeof jealousy_calls / sizeof *jealousy_calls), \
	.types = jealousy_types, .ntypes = (int)(sizeof jealousy_types / sizeof *jealousy_types) }
/* ---- NrthWind: BN5's support 0x15 (080E493D) as BN6's 0x3f ---- */

static const XvBlock nrthwind_blocks[] = {
	{ 0x080D1314, 0x080D13A2, XC_CODE + 0x580, 0x209CD15Eu },
	{ 0x080D13B4, 0x080D1402, XC_CODE + 0x610, 0x0D5320C1u },
	{ 0x080E48CC, 0x080E49D8, XC_CODE + 0x660, 0xF94916BBu },
	{ 0x080E4A10, 0x080E4A26, XC_CODE + 0x770, 0x552B34D2u },
};
static const XvData nrthwind_data[] = {
	{ 0x080D1322, 0x080D1324, XV_RAW },
	{ 0x080D1324, 0x080D1328, XV_POOL },
	{ 0x080D1328, 0x080D1334, XV_TABLE },
	{ 0x080E48DA, 0x080E48DC, XV_RAW },
	{ 0x080E48DC, 0x080E48E0, XV_POOL },
	{ 0x080E48E0, 0x080E48EC, XV_TABLE },
	{ 0x080E48FA, 0x080E48FC, XV_RAW },
	{ 0x080E48FC, 0x080E4900, XV_POOL },
	{ 0x080E4900, 0x080E4910, XV_TABLE },
	{ 0x080E495E, 0x080E4960, XV_RAW },
	{ 0x080E496E, 0x080E4970, XV_RAW },
	{ 0x080E4970, 0x080E4974, XV_POOL },
	{ 0x080E4974, 0x080E4980, XV_TABLE },
	{ 0x080E49D0, 0x080E49D8, XV_POOL },
};
static const XvPair nrthwind_calls[] = {
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x08001490, 0x0800151C },   /* map:calls+bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080E49D8, 0x080E5C00 },   /* map:bytes, SAME */
};
static const XvPair nrthwind_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080D13A3, 0x080D1507 },
	{ 0x080E49D9, 0x080E5C01 },
	{ 0x080E4A09, 0x080E5C31 },
};
static const XvType nrthwind_types[] = { { 3, 0x5A, XV_TYPE_MOVED }, { 4, 0x39, XV_TYPE_MOVED }, { 4, 0x3A, XV_TYPE_MOVED } };

#define NRTHWIND_CODE { .name = "NrthWind (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = nrthwind_blocks, .nblocks = (int)(sizeof nrthwind_blocks / sizeof *nrthwind_blocks), \
	.data = nrthwind_data, .ndata = (int)(sizeof nrthwind_data / sizeof *nrthwind_data), \
	.calls = nrthwind_calls, .ncalls = (int)(sizeof nrthwind_calls / sizeof *nrthwind_calls), \
	.words = nrthwind_words, .nwords = (int)(sizeof nrthwind_words / sizeof *nrthwind_words), \
	.types = nrthwind_types, .ntypes = (int)(sizeof nrthwind_types / sizeof *nrthwind_types) }
const XChipUnit xchips_bn5[] = {
	{ XCHIP_ATTACK, 0x1B, 0x6B, 0x080EC885, XC_HOMES + 0x0, CRAKOUT_CODE },
	{ XCHIP_ATTACK, 0x58, 0xA8, 0x080F0223, XC_HOMES + 0x400, HOTBODY_CODE },
	{ XCHIP_SUPPORT, 0x13, 0x3D, 0x080E4629, XC_HOMES + 0x800, JEALOUSY_CODE },
	{ XCHIP_SUPPORT, 0x15, 0x3F, 0x080E493D, XC_HOMES + 0xC00, NRTHWIND_CODE },
};
const int xchips_bn5_count = (int)(sizeof xchips_bn5 / sizeof *xchips_bn5);

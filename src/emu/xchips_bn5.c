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

/* ---- MuzzleFlash: BN5's 080B9CAF, a helper (installed once, before the units) ---- */

static const XvBlock muzzleflash_blocks[] = {
	{ 0x080B9A5C, 0x080B9D0C, XC_HELPERS + 0x0, 0xA561E116u },
};
static const XvData muzzleflash_data[] = {
	{ 0x080B9A5C, 0x080B9B80, XV_RAW },
	{ 0x080B9B8E, 0x080B9B90, XV_RAW },
	{ 0x080B9B90, 0x080B9B94, XV_POOL },
	{ 0x080B9B94, 0x080B9BA0, XV_TABLE },
	{ 0x080B9CD6, 0x080B9D04, XV_RAW },
	{ 0x080B9D04, 0x080B9D0C, XV_POOL },
};
static const XvPair muzzleflash_calls[] = {
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08010D64, 0x080136CC },   /* map:calls+bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08015140, 0x08018810 },   /* map:calls, DIFF (audited) */
	{ 0x08002D40, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002C34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002C20, 0x08002DB4 },   /* near, SAME */
	{ 0x08002B4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002AEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08002DE2, 0x08002F7E },   /* map:bytes, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
};
static const XvPair muzzleflash_words[] = {
	{ 0x0800325D, 0x08003459 },
};
static const XvSprite muzzleflash_sprites[] = {
	{ 3, 0x00, 3, 0xF7, XC_ART + 0x17800, 0x5800 },
	{ 3, 0x03, 3, 0xF6, XC_ART + 0x1D000, 0x2000 },
	{ 3, 0x04, 3, 0xF5, XC_ART + 0x1F000, 0x2000 },
	{ 3, 0x1C, 3, 0xF4, XC_ART + 0x21000, 0x1800 },
	{ 3, 0x1E, 3, 0xF3, XC_ART + 0x22800, 0x1000 },
	{ 3, 0x38, 3, 0xF2, XC_ART + 0x23800, 0x800 },
};
static const XvByte muzzleflash_bytes[] = {
	{ 0x080B9A6C, 0x00, 0xF7 },   /* the flash 0x3: its sprite list 3 0x0 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080B9A8F, 0x00, 0xF7 },   /* the flash 0xa: its sprite list 3 0x0 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080B9B0C, 0x03, 0xF6 },   /* the flash 0x23: its sprite list 3 0x3 in BN5, 0xf6 in BN6 (BN5's copied there) */
	{ 0x080B9B34, 0x04, 0xF5 },   /* the flash 0x2b: its sprite list 3 0x4 in BN5, 0xf5 in BN6 (BN5's copied there) */
	{ 0x080B9B39, 0x1C, 0xF4 },   /* the flash 0x2c: its sprite list 3 0x1c in BN5, 0xf4 in BN6 (BN5's copied there) */
	{ 0x080B9B3E, 0x1C, 0xF4 },   /* the flash 0x2d: its sprite list 3 0x1c in BN5, 0xf4 in BN6 (BN5's copied there) */
	{ 0x080B9B43, 0x1C, 0xF4 },   /* the flash 0x2e: its sprite list 3 0x1c in BN5, 0xf4 in BN6 (BN5's copied there) */
	{ 0x080B9B48, 0x1E, 0xF3 },   /* the flash 0x2f: its sprite list 3 0x1e in BN5, 0xf3 in BN6 (BN5's copied there) */
	{ 0x080B9B57, 0x00, 0xF7 },   /* the flash 0x32: its sprite list 3 0x0 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080B9B5C, 0x03, 0xF6 },   /* the flash 0x33: its sprite list 3 0x3 in BN5, 0xf6 in BN6 (BN5's copied there) */
	{ 0x080B9B66, 0x38, 0xF2 },   /* the flash 0x35: its sprite list 3 0x38 in BN5, 0xf2 in BN6 (BN5's copied there) */
};
static const XvType muzzleflash_types[] = { { 1, 0x05, XV_TYPE_MOVED } };

#define MUZZLEFLASH_CODE { .name = "MuzzleFlash (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = muzzleflash_blocks, .nblocks = (int)(sizeof muzzleflash_blocks / sizeof *muzzleflash_blocks), \
	.data = muzzleflash_data, .ndata = (int)(sizeof muzzleflash_data / sizeof *muzzleflash_data), \
	.calls = muzzleflash_calls, .ncalls = (int)(sizeof muzzleflash_calls / sizeof *muzzleflash_calls), \
	.words = muzzleflash_words, .nwords = (int)(sizeof muzzleflash_words / sizeof *muzzleflash_words), \
	.sprites = muzzleflash_sprites, .nsprites = (int)(sizeof muzzleflash_sprites / sizeof *muzzleflash_sprites), \
	.types = muzzleflash_types, .ntypes = (int)(sizeof muzzleflash_types / sizeof *muzzleflash_types), \
	.bytes = muzzleflash_bytes, .nbytes = (int)(sizeof muzzleflash_bytes / sizeof *muzzleflash_bytes) }
/* ---- BusterArm: BN5's 080EBB97, a helper (installed once, before the units) ---- */

static const XvBlock busterarm_blocks[] = {
	{ 0x080EBB96, 0x080EBC90, XC_HELPERS + 0x2B2, 0x61AD6A76u },
};
static const XvData busterarm_data[] = {
	{ 0x080EBC8A, 0x080EBC8C, XV_RAW },
	{ 0x080EBC8C, 0x080EBC90, XV_POOL },
};
static const XvPair busterarm_calls[] = {
	{ 0x08010DEE, 0x08013774 },   /* map:calls+bytes, SAME */
	{ 0x0800E0B0, 0x0801032C },   /* map:bytes, SAME */
	{ 0x080B9CAE, 0x08E00E62 },   /* a helper, HELPER */
};

#define BUSTERARM_CODE { .name = "BusterArm (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = busterarm_blocks, .nblocks = (int)(sizeof busterarm_blocks / sizeof *busterarm_blocks), \
	.data = busterarm_data, .ndata = (int)(sizeof busterarm_data / sizeof *busterarm_data), \
	.calls = busterarm_calls, .ncalls = (int)(sizeof busterarm_calls / sizeof *busterarm_calls) }
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
/* ---- MetaGel: BN5's support 0x6 (080E2F85) as BN6's 0x30 ---- */

static const XvBlock metagel_blocks[] = {
	{ 0x080CE114, 0x080CE18C, XC_CODE + 0x790, 0x3D5FC9E7u },
	{ 0x080CE2FA, 0x080CE31C, XC_CODE + 0x812, 0x1A5C2F8Fu },
	{ 0x080E2E80, 0x080E2EE4, XC_CODE + 0x840, 0xD70534B7u },
	{ 0x080E2F0C, 0x080E2F5E, XC_CODE + 0x8B0, 0x2070918Au },
	{ 0x080E2F84, 0x080E2FA6, XC_CODE + 0x910, 0xF0583FFDu },
};
static const XvData metagel_data[] = {
	{ 0x080CE122, 0x080CE124, XV_RAW },
	{ 0x080CE124, 0x080CE128, XV_POOL },
	{ 0x080CE128, 0x080CE134, XV_TABLE },
	{ 0x080CE182, 0x080CE184, XV_RAW },
	{ 0x080CE184, 0x080CE18C, XV_POOL },
	{ 0x080E2E8E, 0x080E2E90, XV_RAW },
	{ 0x080E2E90, 0x080E2E94, XV_POOL },
	{ 0x080E2E94, 0x080E2EA0, XV_TABLE },
	{ 0x080E2EAE, 0x080E2EB0, XV_RAW },
	{ 0x080E2EB0, 0x080E2EB4, XV_POOL },
	{ 0x080E2EB4, 0x080E2EC4, XV_TABLE },
	{ 0x080E2ED2, 0x080E2ED4, XV_RAW },
	{ 0x080E2ED4, 0x080E2ED8, XV_POOL },
	{ 0x080E2ED8, 0x080E2EE4, XV_TABLE },
};
static const XvPair metagel_calls[] = {
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080CE18C, 0x080CE328 },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800BA2C, 0x0800D4D0 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
};
static const XvPair metagel_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080CE18D, 0x080CE329 },
	{ 0x080CE2F3, 0x080CE48F },
	{ 0x080E2EE5, 0x080E3F21 },
	{ 0x080E2F75, 0x080E3FB1 },
};
static const XvImm metagel_imms[] = {
	{ 0x080CE142, THUMB_MOVS, 2, 0, 0x26, 0xFF },   /* its sprite: list 3 0x26 in BN5, 0xff in BN6 (BN5's copied there) */
	{ 0x080CE170, THUMB_MOVS, 0, 0, 0x6, 0x52 },   /* its sound 0x105: BN5's copied to BN6's slot 0x52 */
	{ 0x080CE172, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x105: BN5's copied to BN6's slot 0x52 (its high part) */
};
static const XvSprite metagel_sprites[] = {
	{ 3, 0x26, 3, 0xFF, XC_ART + 0x3000, 0x1800 },
};
static const XvType metagel_types[] = { { 3, 0x45, XV_TYPE_MOVED }, { 4, 0x21, XV_TYPE_MOVED } };
static const XcSong metagel_songs[] = { { 0x105, 0x52 } };

#define METAGEL_CODE { .name = "MetaGel (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = metagel_blocks, .nblocks = (int)(sizeof metagel_blocks / sizeof *metagel_blocks), \
	.data = metagel_data, .ndata = (int)(sizeof metagel_data / sizeof *metagel_data), \
	.calls = metagel_calls, .ncalls = (int)(sizeof metagel_calls / sizeof *metagel_calls), \
	.words = metagel_words, .nwords = (int)(sizeof metagel_words / sizeof *metagel_words), \
	.imms = metagel_imms, .nimms = (int)(sizeof metagel_imms / sizeof *metagel_imms), \
	.sprites = metagel_sprites, .nsprites = (int)(sizeof metagel_sprites / sizeof *metagel_sprites), \
	.types = metagel_types, .ntypes = (int)(sizeof metagel_types / sizeof *metagel_types) }
/* ---- Blinder: BN5's support 0x8 (080E3229) as BN6's 0x32 ---- */

static const XvBlock blinder_blocks[] = {
	{ 0x080E1118, 0x080E125C, XC_CODE + 0x940, 0xD30DB034u },
	{ 0x080E3138, 0x080E3198, XC_CODE + 0xA90, 0x9C08FE6Bu },
	{ 0x080E31C0, 0x080E324A, XC_CODE + 0xAF0, 0x159366D2u },
};
static const XvData blinder_data[] = {
	{ 0x080E1128, 0x080E112C, XV_POOL },
	{ 0x080E112C, 0x080E1134, XV_TABLE },
	{ 0x080E11A2, 0x080E11A4, XV_RAW },
	{ 0x080E11A4, 0x080E11A8, XV_POOL },
	{ 0x080E11A8, 0x080E11B0, XV_RAW },
	{ 0x080E11B0, 0x080E11B8, XV_POOL },
	{ 0x080E1234, 0x080E1240, XV_POOL },
	{ 0x080E3146, 0x080E3148, XV_RAW },
	{ 0x080E3148, 0x080E314C, XV_POOL },
	{ 0x080E314C, 0x080E3158, XV_TABLE },
	{ 0x080E3166, 0x080E3168, XV_RAW },
	{ 0x080E3168, 0x080E316C, XV_POOL },
	{ 0x080E316C, 0x080E317C, XV_TABLE },
	{ 0x080E318A, 0x080E318C, XV_RAW },
	{ 0x080E318C, 0x080E3190, XV_POOL },
	{ 0x080E3190, 0x080E3198, XV_TABLE },
	{ 0x080E3212, 0x080E3214, XV_RAW },
	{ 0x080E3214, 0x080E321C, XV_POOL },
	{ 0x080E321C, 0x080E3224, XV_RAW },
	{ 0x080E3224, 0x080E3228, XV_POOL },
};
static const XvPair blinder_calls[] = {
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080022E8, 0x08002378 },   /* map:bytes, SAME */
	{ 0x0800230A, 0x0800239A },   /* map:bytes, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
};
static const XvPair blinder_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080E3199, 0x080E41D5 },
};
static const XvImm blinder_imms[] = {
	{ 0x080E31D2, THUMB_MOVS, 0, 0, 0xBC, 0x55 },   /* its sound 0xbc: BN5's copied to BN6's slot 0x55 */
};
static const XvNumber blinder_numbers[] = {
	{ 0x080E11B4, 0x03003B60, 0x03003B60 },
	{ 0x080E1238, 0x03003B60, 0x03003B60 },
	{ 0x080E123C, 0x03003750, 0x03003750 },
};
static const XvType blinder_types[] = { { 4, 0x0A, XV_TYPE_MOVED }, { 4, 0x23, XV_TYPE_MOVED } };
static const XcSong blinder_songs[] = { { 0xBC, 0x55 } };

#define BLINDER_CODE { .name = "Blinder (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = blinder_blocks, .nblocks = (int)(sizeof blinder_blocks / sizeof *blinder_blocks), \
	.data = blinder_data, .ndata = (int)(sizeof blinder_data / sizeof *blinder_data), \
	.calls = blinder_calls, .ncalls = (int)(sizeof blinder_calls / sizeof *blinder_calls), \
	.words = blinder_words, .nwords = (int)(sizeof blinder_words / sizeof *blinder_words), \
	.imms = blinder_imms, .nimms = (int)(sizeof blinder_imms / sizeof *blinder_imms), \
	.numbers = blinder_numbers, .nnumbers = (int)(sizeof blinder_numbers / sizeof *blinder_numbers), \
	.types = blinder_types, .ntypes = (int)(sizeof blinder_types / sizeof *blinder_types) }
/* ---- Skully: BN5's attack 0x55 (080EFD19) as BN6's 0xa5 ---- */

static const XvBlock skully_blocks[] = {
	{ 0x080D7378, 0x080D750C, XC_CODE + 0xB80, 0x629A4B6Au },
	{ 0x080EFD18, 0x080EFDB8, XC_CODE + 0xD20, 0x1AAC3C6Eu },
};
static const XvData skully_data[] = {
	{ 0x080D738A, 0x080D738C, XV_RAW },
	{ 0x080D738C, 0x080D7390, XV_POOL },
	{ 0x080D7390, 0x080D739C, XV_TABLE },
	{ 0x080D74B6, 0x080D74B8, XV_RAW },
	{ 0x080D74B8, 0x080D74BC, XV_POOL },
	{ 0x080D74BC, 0x080D74C4, XV_RAW },
	{ 0x080D7504, 0x080D750C, XV_POOL },
	{ 0x080EFD26, 0x080EFD28, XV_RAW },
	{ 0x080EFD28, 0x080EFD2C, XV_POOL },
	{ 0x080EFD2C, 0x080EFD34, XV_TABLE },
};
static const XvPair skully_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair skully_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm skully_imms[] = {
	{ 0x080D73E8, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080EFD4A, THUMB_MOVS, 0, 0, 0xB7, 0x61 },   /* its sound 0xb7: BN5's copied to BN6's slot 0x61 */
};
static const XvNumber skully_numbers[] = {
	{ 0x080D7504, 0x01001050, 0x010010FE },
	{ 0x080D7508, 0x04050300, 0x04050300 },
};
static const XvSprite skully_sprites[] = {
	{ 4, 0x50, 4, 0xFE, XC_ART + 0x4800, 0x800 },
};
static const XvType skully_types[] = { { 3, 0x88, XV_TYPE_MOVED } };
static const XcSong skully_songs[] = { { 0xB7, 0x61 } };

#define SKULLY_CODE { .name = "Skully (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = skully_blocks, .nblocks = (int)(sizeof skully_blocks / sizeof *skully_blocks), \
	.data = skully_data, .ndata = (int)(sizeof skully_data / sizeof *skully_data), \
	.calls = skully_calls, .ncalls = (int)(sizeof skully_calls / sizeof *skully_calls), \
	.words = skully_words, .nwords = (int)(sizeof skully_words / sizeof *skully_words), \
	.imms = skully_imms, .nimms = (int)(sizeof skully_imms / sizeof *skully_imms), \
	.numbers = skully_numbers, .nnumbers = (int)(sizeof skully_numbers / sizeof *skully_numbers), \
	.sprites = skully_sprites, .nsprites = (int)(sizeof skully_sprites / sizeof *skully_sprites), \
	.types = skully_types, .ntypes = (int)(sizeof skully_types / sizeof *skully_types) }
/* ---- DrilArm: BN5's attack 0x4f (080EF579) as BN6's 0x9f ---- */

static const XvBlock drilarm_blocks[] = {
	{ 0x080D41C8, 0x080D42D2, XC_CODE + 0xDC0, 0xFCA927A0u },
	{ 0x080EF578, 0x080EF652, XC_CODE + 0xED0, 0x717885B5u },
};
static const XvData drilarm_data[] = {
	{ 0x080D41D6, 0x080D41D8, XV_RAW },
	{ 0x080D41D8, 0x080D41DC, XV_POOL },
	{ 0x080D41DC, 0x080D41E8, XV_TABLE },
	{ 0x080EF586, 0x080EF588, XV_RAW },
	{ 0x080EF588, 0x080EF58C, XV_POOL },
	{ 0x080EF58C, 0x080EF598, XV_TABLE },
};
static const XvPair drilarm_calls[] = {
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x080D42E0, 0x080D4430 },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x080172CE, 0x0801A4D0 },   /* map:bytes, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080EA0BA, 0x08E00C00 },   /* a stub, STUB */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080B9CAE, 0x08E00E62 },   /* a helper, HELPER */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
};
static const XvPair drilarm_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080EF653, 0x080EE75D },
};
static const XvImm drilarm_imms[] = {
	{ 0x080D426C, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvType drilarm_types[] = { { 3, 0x71, XV_TYPE_MOVED } };

#define DRILARM_CODE { .name = "DrilArm (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = drilarm_blocks, .nblocks = (int)(sizeof drilarm_blocks / sizeof *drilarm_blocks), \
	.data = drilarm_data, .ndata = (int)(sizeof drilarm_data / sizeof *drilarm_data), \
	.calls = drilarm_calls, .ncalls = (int)(sizeof drilarm_calls / sizeof *drilarm_calls), \
	.words = drilarm_words, .nwords = (int)(sizeof drilarm_words / sizeof *drilarm_words), \
	.imms = drilarm_imms, .nimms = (int)(sizeof drilarm_imms / sizeof *drilarm_imms), \
	.types = drilarm_types, .ntypes = (int)(sizeof drilarm_types / sizeof *drilarm_types) }
/* ---- RainyDay: BN5's support 0x3d (080E91EF) as BN6's 0x67 ---- */

static const XvBlock rainyday_blocks[] = {
	{ 0x080D9890, 0x080D9AC0, XC_CODE + 0xFB0, 0xE8DC8FA0u },
	{ 0x080E9174, 0x080E9270, XC_CODE + 0x11E0, 0x20408A31u },
};
static const XvData rainyday_data[] = {
	{ 0x080D98A2, 0x080D98A4, XV_RAW },
	{ 0x080D98A4, 0x080D98A8, XV_POOL },
	{ 0x080D98A8, 0x080D98B4, XV_TABLE },
	{ 0x080D98E6, 0x080D98E8, XV_RAW },
	{ 0x080D98E8, 0x080D98EC, XV_POOL },
	{ 0x080D98EC, 0x080D98FC, XV_TABLE },
	{ 0x080D9AB8, 0x080D9AC0, XV_POOL },
	{ 0x080E9182, 0x080E9184, XV_RAW },
	{ 0x080E9184, 0x080E9188, XV_POOL },
	{ 0x080E9188, 0x080E9194, XV_TABLE },
	{ 0x080E91A2, 0x080E91A4, XV_RAW },
	{ 0x080E91A4, 0x080E91A8, XV_POOL },
	{ 0x080E91A8, 0x080E91B8, XV_TABLE },
	{ 0x080E9264, 0x080E9268, XV_POOL },
	{ 0x080E9268, 0x080E9270, XV_RAW },
};
static const XvPair rainyday_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08018926, 0x0801BE2A },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
};
static const XvPair rainyday_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
};
static const XvImm rainyday_imms[] = {
	{ 0x080D996E, THUMB_MOVS, 4, 0, 0x61, 0xFF },   /* its effect: 0x61 in BN5, 0xff in BN6 */
	{ 0x080D99BC, THUMB_MOVS, 4, 0, 0x29, 0x3B },   /* its effect: 0x29 in BN5, 0x3b in BN6 */
	{ 0x080D9A88, THUMB_CMP, 1, 0, 0xA, 0xE },   /* a panel's type compared with BN5's 0xa: BN6's 0xe */
};
static const XvNumber rainyday_numbers[] = {
	{ 0x080D9AB8, 0x01000C36, 0x01000CFE },
	{ 0x080D9ABC, 0x04050201, 0x04050201 },
};
static const XvSprite rainyday_sprites[] = {
	{ 3, 0x36, 3, 0xFE, XC_ART + 0x5000, 0x1000 },
	{ 3, 0x37, 3, 0xFD, XC_ART + 0x6000, 0x1000 },
};
static const XvId rainyday_effects[] = { { 0x61, 0xFF } };
static const XvType rainyday_types[] = { { 3, 0x97, XV_TYPE_MOVED }, { 4, 0x75, XV_TYPE_MOVED } };

#define RAINYDAY_CODE { .name = "RainyDay (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = rainyday_blocks, .nblocks = (int)(sizeof rainyday_blocks / sizeof *rainyday_blocks), \
	.data = rainyday_data, .ndata = (int)(sizeof rainyday_data / sizeof *rainyday_data), \
	.calls = rainyday_calls, .ncalls = (int)(sizeof rainyday_calls / sizeof *rainyday_calls), \
	.words = rainyday_words, .nwords = (int)(sizeof rainyday_words / sizeof *rainyday_words), \
	.imms = rainyday_imms, .nimms = (int)(sizeof rainyday_imms / sizeof *rainyday_imms), \
	.numbers = rainyday_numbers, .nnumbers = (int)(sizeof rainyday_numbers / sizeof *rainyday_numbers), \
	.sprites = rainyday_sprites, .nsprites = (int)(sizeof rainyday_sprites / sizeof *rainyday_sprites), \
	.effects = rainyday_effects, .neffects = (int)(sizeof rainyday_effects / sizeof *rainyday_effects), \
	.types = rainyday_types, .ntypes = (int)(sizeof rainyday_types / sizeof *rainyday_types) }
/* ---- WoodNos: BN5's attack 0x28 (080ED36F) as BN6's 0x78 ---- */

static const XvBlock woodnos_blocks[] = {
	{ 0x080DB0AC, 0x080DB340, XC_CODE + 0x12E0, 0xC9E8AF0Eu },
	{ 0x080ED36E, 0x080ED410, XC_CODE + 0x1582, 0x9566F586u },
};
static const XvData woodnos_data[] = {
	{ 0x080DB0BE, 0x080DB0C0, XV_RAW },
	{ 0x080DB0C0, 0x080DB0C4, XV_POOL },
	{ 0x080DB0C4, 0x080DB0D0, XV_TABLE },
	{ 0x080DB126, 0x080DB128, XV_RAW },
	{ 0x080DB128, 0x080DB12C, XV_POOL },
	{ 0x080DB12C, 0x080DB130, XV_TABLE },
	{ 0x080DB1E6, 0x080DB1E8, XV_RAW },
	{ 0x080DB1E8, 0x080DB1EC, XV_POOL },
	{ 0x080DB1EC, 0x080DB1F4, XV_RAW },
	{ 0x080DB200, 0x080DB20C, XV_POOL },
	{ 0x080DB21E, 0x080DB220, XV_RAW },
	{ 0x080DB220, 0x080DB224, XV_POOL },
	{ 0x080DB224, 0x080DB230, XV_TABLE },
	{ 0x080DB28C, 0x080DB290, XV_POOL },
	{ 0x080DB290, 0x080DB298, XV_TABLE },
	{ 0x080DB336, 0x080DB338, XV_RAW },
	{ 0x080DB338, 0x080DB340, XV_POOL },
	{ 0x080ED37C, 0x080ED380, XV_POOL },
	{ 0x080ED380, 0x080ED388, XV_TABLE },
};
static const XvPair woodnos_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08016C3A, 0x0801A146 },   /* project:chips, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08018926, 0x0801BE2A },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080EBB96, 0x08E00EC2 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair woodnos_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm woodnos_imms[] = {
	{ 0x080DB134, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvNumber woodnos_numbers[] = {
	{ 0x080DB200, 0x01000C54, 0x01000CFC },
	{ 0x080DB204, 0x04050100, 0x04050100 },
	{ 0x080DB338, 0x00001021, 0x000010FD },
	{ 0x080DB33C, 0x04050401, 0x04050401 },
};
static const XvSprite woodnos_sprites[] = {
	{ 3, 0x54, 3, 0xFC, XC_ART + 0x7000, 0x800 },
	{ 4, 0x21, 4, 0xFD, XC_ART + 0x7800, 0x800 },
};
static const XvType woodnos_types[] = { { 3, 0xA6, XV_TYPE_MOVED }, { 3, 0xA7, XV_TYPE_MOVED } };

#define WOODNOS_CODE { .name = "WoodNos (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = woodnos_blocks, .nblocks = (int)(sizeof woodnos_blocks / sizeof *woodnos_blocks), \
	.data = woodnos_data, .ndata = (int)(sizeof woodnos_data / sizeof *woodnos_data), \
	.calls = woodnos_calls, .ncalls = (int)(sizeof woodnos_calls / sizeof *woodnos_calls), \
	.words = woodnos_words, .nwords = (int)(sizeof woodnos_words / sizeof *woodnos_words), \
	.imms = woodnos_imms, .nimms = (int)(sizeof woodnos_imms / sizeof *woodnos_imms), \
	.numbers = woodnos_numbers, .nnumbers = (int)(sizeof woodnos_numbers / sizeof *woodnos_numbers), \
	.sprites = woodnos_sprites, .nsprites = (int)(sizeof woodnos_sprites / sizeof *woodnos_sprites), \
	.types = woodnos_types, .ntypes = (int)(sizeof woodnos_types / sizeof *woodnos_types) }
/* ---- CactBal: BN5's attack 0x48 (080EF1DD) as BN6's 0x98 ---- */

static const XvBlock cactbal_blocks[] = {
	{ 0x080D1B70, 0x080D1E68, XC_CODE + 0x1630, 0xC04F9A8Fu },
	{ 0x080EF1DC, 0x080EF270, XC_CODE + 0x1930, 0x7E59DE88u },
};
static const XvData cactbal_data[] = {
	{ 0x080D1B82, 0x080D1B84, XV_RAW },
	{ 0x080D1B84, 0x080D1B88, XV_POOL },
	{ 0x080D1B88, 0x080D1B94, XV_TABLE },
	{ 0x080D1C30, 0x080D1C34, XV_POOL },
	{ 0x080D1C34, 0x080D1C40, XV_TABLE },
	{ 0x080D1E58, 0x080D1E68, XV_POOL },
	{ 0x080EF1EA, 0x080EF1EC, XV_RAW },
	{ 0x080EF1EC, 0x080EF1F0, XV_POOL },
	{ 0x080EF1F0, 0x080EF1F8, XV_TABLE },
};
static const XvPair cactbal_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016C3A, 0x0801A146 },   /* project:chips, DIFF (audited) */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080EBB96, 0x08E00EC2 },   /* a helper, HELPER */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair cactbal_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm cactbal_imms[] = {
	{ 0x080D1BE0, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D1C44, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D1CE8, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D1D9C, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvNumber cactbal_numbers[] = {
	{ 0x080D1E58, 0x00041030, 0x000410FC },
	{ 0x080D1E5C, 0x04050100, 0x04050100 },
};
static const XvSprite cactbal_sprites[] = {
	{ 4, 0x30, 4, 0xFC, XC_ART + 0x8000, 0x1800 },
};
static const XvType cactbal_types[] = { { 3, 0x5E, XV_TYPE_MOVED } };

#define CACTBAL_CODE { .name = "CactBal (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = cactbal_blocks, .nblocks = (int)(sizeof cactbal_blocks / sizeof *cactbal_blocks), \
	.data = cactbal_data, .ndata = (int)(sizeof cactbal_data / sizeof *cactbal_data), \
	.calls = cactbal_calls, .ncalls = (int)(sizeof cactbal_calls / sizeof *cactbal_calls), \
	.words = cactbal_words, .nwords = (int)(sizeof cactbal_words / sizeof *cactbal_words), \
	.imms = cactbal_imms, .nimms = (int)(sizeof cactbal_imms / sizeof *cactbal_imms), \
	.numbers = cactbal_numbers, .nnumbers = (int)(sizeof cactbal_numbers / sizeof *cactbal_numbers), \
	.sprites = cactbal_sprites, .nsprites = (int)(sizeof cactbal_sprites / sizeof *cactbal_sprites), \
	.types = cactbal_types, .ntypes = (int)(sizeof cactbal_types / sizeof *cactbal_types) }
/* ---- SpShake: BN5's attack 0x51 (080EF733) as BN6's 0xa1 ---- */

static const XvBlock spshake_blocks[] = {
	{ 0x080D30E0, 0x080D32AC, XC_CODE + 0x19D0, 0x3354D1A3u },
	{ 0x080EF732, 0x080EF7CA, XC_CODE + 0x1BA2, 0x7779DD86u },
};
static const XvData spshake_data[] = {
	{ 0x080D30F2, 0x080D30F4, XV_RAW },
	{ 0x080D30F4, 0x080D30F8, XV_POOL },
	{ 0x080D30F8, 0x080D3104, XV_TABLE },
	{ 0x080D31B6, 0x080D31B8, XV_RAW },
	{ 0x080D31B8, 0x080D31BC, XV_POOL },
	{ 0x080D31BC, 0x080D31C8, XV_RAW },
	{ 0x080D323C, 0x080D3240, XV_POOL },
	{ 0x080D3240, 0x080D3244, XV_TABLE },
	{ 0x080D32A4, 0x080D32AC, XV_POOL },
	{ 0x080EF740, 0x080EF744, XV_POOL },
	{ 0x080EF744, 0x080EF74C, XV_TABLE },
};
static const XvPair spshake_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B380, 0x0800CCBE },   /* project:research, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080EBB96, 0x08E00EC2 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair spshake_words[] = {
	{ 0x08005CD0, 0x080065E0 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm spshake_imms[] = {
	{ 0x080D310C, THUMB_MOVS, 2, 0, 0x7, 0xFF },   /* its sprite: list 1 0x7 in BN5, 0xff in BN6 (BN5's copied there) */
	{ 0x080D320A, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite spshake_sprites[] = {
	{ 1, 0x07, 1, 0xFF, XC_ART + 0x9800, 0x1000 },
};
static const XvType spshake_types[] = { { 3, 0x68, XV_TYPE_MOVED } };

#define SPSHAKE_CODE { .name = "SpShake (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = spshake_blocks, .nblocks = (int)(sizeof spshake_blocks / sizeof *spshake_blocks), \
	.data = spshake_data, .ndata = (int)(sizeof spshake_data / sizeof *spshake_data), \
	.calls = spshake_calls, .ncalls = (int)(sizeof spshake_calls / sizeof *spshake_calls), \
	.words = spshake_words, .nwords = (int)(sizeof spshake_words / sizeof *spshake_words), \
	.imms = spshake_imms, .nimms = (int)(sizeof spshake_imms / sizeof *spshake_imms), \
	.sprites = spshake_sprites, .nsprites = (int)(sizeof spshake_sprites / sizeof *spshake_sprites), \
	.types = spshake_types, .ntypes = (int)(sizeof spshake_types / sizeof *spshake_types) }
/* ---- Pulsar: BN5's attack 0x52 (080EF7CB) as BN6's 0xa2 ---- */

static const XvBlock pulsar_blocks[] = {
	{ 0x080D33B8, 0x080D3570, XC_CODE + 0x1C40, 0xB5B961B7u },
	{ 0x080EF7CA, 0x080EF872, XC_CODE + 0x1E02, 0x043CE56Fu },
};
static const XvData pulsar_data[] = {
	{ 0x080D33CA, 0x080D33CC, XV_RAW },
	{ 0x080D33CC, 0x080D33D0, XV_POOL },
	{ 0x080D33D0, 0x080D33DC, XV_TABLE },
	{ 0x080D34C4, 0x080D34C8, XV_POOL },
	{ 0x080D34C8, 0x080D34D0, XV_TABLE },
	{ 0x080D3548, 0x080D354C, XV_POOL },
	{ 0x080D356C, 0x080D3570, XV_POOL },
	{ 0x080EF7D8, 0x080EF7DC, XV_POOL },
	{ 0x080EF7DC, 0x080EF7E4, XV_TABLE },
	{ 0x080EF83E, 0x080EF840, XV_RAW },
	{ 0x080EF840, 0x080EF844, XV_POOL },
};
static const XvPair pulsar_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B380, 0x0800CCBE },   /* project:research, DIFF (audited) */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080EBB96, 0x08E00EC2 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair pulsar_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm pulsar_imms[] = {
	{ 0x080D33E4, THUMB_MOVS, 2, 0, 0x3A, 0xFB },   /* its sprite: list 4 0x3a in BN5, 0xfb in BN6 (BN5's copied there) */
	{ 0x080D3476, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D3514, THUMB_ADDS, 4, 0, 0x56, 0xFE },   /* its effect: 0x56 in BN5, 0xfe in BN6 */
};
static const XvNumber pulsar_numbers[] = {
	{ 0x080D3548, 0x0405FF10, 0x0405FF10 },
	{ 0x080D356C, 0x03800000, 0x03800000 },
};
static const XvSprite pulsar_sprites[] = {
	{ 4, 0x3A, 4, 0xFB, XC_ART + 0xA800, 0x800 },
	{ 4, 0x3B, 4, 0xFA, XC_ART + 0xB000, 0x1000 },
};
static const XvId pulsar_effects[] = { { 0x56, 0xFE } };
static const XvType pulsar_types[] = { { 3, 0x6A, XV_TYPE_MOVED } };

#define PULSAR_CODE { .name = "Pulsar (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = pulsar_blocks, .nblocks = (int)(sizeof pulsar_blocks / sizeof *pulsar_blocks), \
	.data = pulsar_data, .ndata = (int)(sizeof pulsar_data / sizeof *pulsar_data), \
	.calls = pulsar_calls, .ncalls = (int)(sizeof pulsar_calls / sizeof *pulsar_calls), \
	.words = pulsar_words, .nwords = (int)(sizeof pulsar_words / sizeof *pulsar_words), \
	.imms = pulsar_imms, .nimms = (int)(sizeof pulsar_imms / sizeof *pulsar_imms), \
	.numbers = pulsar_numbers, .nnumbers = (int)(sizeof pulsar_numbers / sizeof *pulsar_numbers), \
	.sprites = pulsar_sprites, .nsprites = (int)(sizeof pulsar_sprites / sizeof *pulsar_sprites), \
	.effects = pulsar_effects, .neffects = (int)(sizeof pulsar_effects / sizeof *pulsar_effects), \
	.types = pulsar_types, .ntypes = (int)(sizeof pulsar_types / sizeof *pulsar_types) }
/* ---- MrkCan: BN5's attack 0x4e (080EF441) as BN6's 0x9e ---- */

static const XvBlock mrkcan_blocks[] = {
	{ 0x080E5580, 0x080E57B4, XC_CODE + 0x1EB0, 0xD17D0F24u },
	{ 0x080EF440, 0x080EF548, XC_CODE + 0x20F0, 0xCCB5A1A5u },
};
static const XvData mrkcan_data[] = {
	{ 0x080E5592, 0x080E5594, XV_RAW },
	{ 0x080E5594, 0x080E5598, XV_POOL },
	{ 0x080E5598, 0x080E55A4, XV_TABLE },
	{ 0x080E5602, 0x080E5604, XV_RAW },
	{ 0x080E5604, 0x080E5608, XV_POOL },
	{ 0x080E5608, 0x080E5610, XV_RAW },
	{ 0x080E561E, 0x080E5620, XV_RAW },
	{ 0x080E5620, 0x080E5624, XV_POOL },
	{ 0x080E5624, 0x080E562C, XV_TABLE },
	{ 0x080E5792, 0x080E5794, XV_RAW },
	{ 0x080E5794, 0x080E57B4, XV_POOL },
	{ 0x080EF44E, 0x080EF450, XV_RAW },
	{ 0x080EF450, 0x080EF454, XV_POOL },
	{ 0x080EF454, 0x080EF460, XV_TABLE },
	{ 0x080EF518, 0x080EF51C, XV_POOL },
};
static const XvPair mrkcan_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08018926, 0x0801BE2A },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080EF548, 0x080ED0A8 },   /* search, SAME */
	{ 0x080B9CAE, 0x08E00E62 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
};
static const XvPair mrkcan_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x08005CD0, 0x080065E0 },
	{ 0x08005D50, 0x08006660 },
};
static const XvImm mrkcan_imms[] = {
	{ 0x080E56FE, THUMB_MOVS, 0, 0, 0xBD, 0x4F },   /* its sound 0xbd: BN5's copied to BN6's slot 0x4f */
};
static const XvNumber mrkcan_numbers[] = {
	{ 0x080E5794, 0x00001024, 0x000010F9 },
	{ 0x080E57AC, 0x0405FF01, 0x0405FF01 },
	{ 0x080EF518, 0x04050701, 0x04050701 },
};
static const XvSprite mrkcan_sprites[] = {
	{ 4, 0x24, 4, 0xF9, XC_ART + 0xC000, 0x800 },
};
static const XvType mrkcan_types[] = { { 4, 0x44, XV_TYPE_MOVED } };
static const XcSong mrkcan_songs[] = { { 0xBD, 0x4F } };

#define MRKCAN_CODE { .name = "MrkCan (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = mrkcan_blocks, .nblocks = (int)(sizeof mrkcan_blocks / sizeof *mrkcan_blocks), \
	.data = mrkcan_data, .ndata = (int)(sizeof mrkcan_data / sizeof *mrkcan_data), \
	.calls = mrkcan_calls, .ncalls = (int)(sizeof mrkcan_calls / sizeof *mrkcan_calls), \
	.words = mrkcan_words, .nwords = (int)(sizeof mrkcan_words / sizeof *mrkcan_words), \
	.imms = mrkcan_imms, .nimms = (int)(sizeof mrkcan_imms / sizeof *mrkcan_imms), \
	.numbers = mrkcan_numbers, .nnumbers = (int)(sizeof mrkcan_numbers / sizeof *mrkcan_numbers), \
	.sprites = mrkcan_sprites, .nsprites = (int)(sizeof mrkcan_sprites / sizeof *mrkcan_sprites), \
	.types = mrkcan_types, .ntypes = (int)(sizeof mrkcan_types / sizeof *mrkcan_types) }
/* ---- SideBub: BN5's attack 0x1c (080EC91B) as BN6's 0x6c ---- */

static const XvBlock sidebub_blocks[] = {
	{ 0x080C5386, 0x080C53DE, XC_CODE + 0x2202, 0x4A1B50AFu },
	{ 0x080C5DF4, 0x080C6000, XC_CODE + 0x2260, 0xEDB80F1Au },
	{ 0x080D856C, 0x080D87D0, XC_CODE + 0x2470, 0x7524551Eu },
	{ 0x080EC91A, 0x080EC9E0, XC_CODE + 0x26E2, 0xE2038DC0u },
};
static const XvData sidebub_data[] = {
	{ 0x080C5DF4, 0x080C5EB4, XV_RAW },
	{ 0x080C5EC6, 0x080C5EC8, XV_RAW },
	{ 0x080C5EC8, 0x080C5ECC, XV_POOL },
	{ 0x080C5ECC, 0x080C5ED8, XV_TABLE },
	{ 0x080C5FFC, 0x080C6000, XV_POOL },
	{ 0x080D857E, 0x080D8580, XV_RAW },
	{ 0x080D8580, 0x080D8584, XV_POOL },
	{ 0x080D8584, 0x080D8590, XV_TABLE },
	{ 0x080D86F6, 0x080D86F8, XV_RAW },
	{ 0x080D86F8, 0x080D86FC, XV_POOL },
	{ 0x080D86FC, 0x080D8708, XV_TABLE },
	{ 0x080D8796, 0x080D8798, XV_RAW },
	{ 0x080D8798, 0x080D879C, XV_POOL },
	{ 0x080D879C, 0x080D87A2, XV_RAW },
	{ 0x080D87CA, 0x080D87CC, XV_RAW },
	{ 0x080D87CC, 0x080D87D0, XV_POOL },
	{ 0x080EC928, 0x080EC92C, XV_POOL },
	{ 0x080EC92C, 0x080EC934, XV_TABLE },
};
static const XvPair sidebub_calls[] = {
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB00, 0x0800E6E8 },   /* map:bytes, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800B380, 0x0800CCBE },   /* project:research, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08016B96, 0x0801A082 },   /* map:calls, DIFF (audited) */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080EBB96, 0x08E00EC2 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair sidebub_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm sidebub_imms[] = {
	{ 0x080C5F74, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D85AA, THUMB_MOVS, 2, 0, 0x41, 0xF8 },   /* its sprite: list 4 0x41 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080D869A, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite sidebub_sprites[] = {
	{ 4, 0x41, 4, 0xF8, XC_ART + 0xC800, 0x1000 },
	{ 3, 0x3E, 3, 0xFB, XC_ART + 0xD800, 0x800 },
	{ 3, 0x43, 3, 0xFA, XC_ART + 0xE000, 0x1000 },
	{ 4, 0x11, 4, 0xF7, XC_ART + 0xF000, 0x1000 },
	{ 4, 0x2D, 4, 0xF6, XC_ART + 0x10000, 0x800 },
};
static const XvByte sidebub_bytes[] = {
	{ 0x080C5DFA, 0x3E, 0xFB },   /* 080C5DF4's row 0: its sprite list 3 0x3e in BN5, 0xfb in BN6 (BN5's copied there) */
	{ 0x080C5E0A, 0x43, 0xFA },   /* 080C5DF4's row 1: its sprite list 3 0x43 in BN5, 0xfa in BN6 (BN5's copied there) */
	{ 0x080C5E1A, 0x11, 0xF7 },   /* 080C5DF4's row 2: its sprite list 4 0x11 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080C5E2A, 0x11, 0xF7 },   /* 080C5DF4's row 3: its sprite list 4 0x11 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080C5E3A, 0x11, 0xF7 },   /* 080C5DF4's row 4: its sprite list 4 0x11 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080C5E4A, 0x11, 0xF7 },   /* 080C5DF4's row 5: its sprite list 4 0x11 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080C5E5A, 0x2D, 0xF6 },   /* 080C5DF4's row 6: its sprite list 4 0x2d in BN5, 0xf6 in BN6 (BN5's copied there) */
	{ 0x080C5E6A, 0x41, 0xF8 },   /* 080C5DF4's row 7: its sprite list 4 0x41 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5E7A, 0x41, 0xF8 },   /* 080C5DF4's row 8: its sprite list 4 0x41 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5E8A, 0x41, 0xF8 },   /* 080C5DF4's row 9: its sprite list 4 0x41 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5E9A, 0x41, 0xF8 },   /* 080C5DF4's row 10: its sprite list 4 0x41 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5EAA, 0x11, 0xF7 },   /* 080C5DF4's row 11: its sprite list 4 0x11 in BN5, 0xf7 in BN6 (BN5's copied there) */
};
static const XvType sidebub_types[] = { { 3, 0x0B, XV_TYPE_MOVED }, { 3, 0x8F, XV_TYPE_MOVED } };

#define SIDEBUB_CODE { .name = "SideBub (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = sidebub_blocks, .nblocks = (int)(sizeof sidebub_blocks / sizeof *sidebub_blocks), \
	.data = sidebub_data, .ndata = (int)(sizeof sidebub_data / sizeof *sidebub_data), \
	.calls = sidebub_calls, .ncalls = (int)(sizeof sidebub_calls / sizeof *sidebub_calls), \
	.words = sidebub_words, .nwords = (int)(sizeof sidebub_words / sizeof *sidebub_words), \
	.imms = sidebub_imms, .nimms = (int)(sizeof sidebub_imms / sizeof *sidebub_imms), \
	.sprites = sidebub_sprites, .nsprites = (int)(sizeof sidebub_sprites / sizeof *sidebub_sprites), \
	.types = sidebub_types, .ntypes = (int)(sizeof sidebub_types / sizeof *sidebub_types), \
	.bytes = sidebub_bytes, .nbytes = (int)(sizeof sidebub_bytes / sizeof *sidebub_bytes) }
/* ---- AqWhirl: BN5's attack 0x47 (080EF119) as BN6's 0x97 ---- */

static const XvBlock aqwhirl_blocks[] = {
	{ 0x080C5386, 0x080C53DE, XC_CODE + 0x27B2, 0x4A1B50AFu },
	{ 0x080D17C8, 0x080D1B70, XC_CODE + 0x2810, 0xFEB7C3C6u },
	{ 0x080D9FCC, 0x080DA2C8, XC_CODE + 0x2BC0, 0x25761344u },
	{ 0x080DE9F8, 0x080DEDDC, XC_CODE + 0x2EC0, 0xF9535385u },
	{ 0x080EF118, 0x080EF1B8, XC_CODE + 0x32B0, 0x21F7A2E1u },
};
static const XvData aqwhirl_data[] = {
	{ 0x080D17DA, 0x080D17DC, XV_RAW },
	{ 0x080D17DC, 0x080D17E0, XV_POOL },
	{ 0x080D17E0, 0x080D17EC, XV_TABLE },
	{ 0x080D188C, 0x080D1890, XV_POOL },
	{ 0x080D1890, 0x080D18A4, XV_TABLE },
	{ 0x080D1B5A, 0x080D1B5C, XV_RAW },
	{ 0x080D1B5C, 0x080D1B60, XV_POOL },
	{ 0x080D1B60, 0x080D1B68, XV_RAW },
	{ 0x080D1B68, 0x080D1B70, XV_POOL },
	{ 0x080D9FE4, 0x080D9FE8, XV_POOL },
	{ 0x080D9FE8, 0x080D9FF4, XV_TABLE },
	{ 0x080DA0B0, 0x080DA0B4, XV_POOL },
	{ 0x080DA0B4, 0x080DA0C4, XV_TABLE },
	{ 0x080DA27C, 0x080DA28E, XV_RAW },
	{ 0x080DA2B0, 0x080DA2C8, XV_POOL },
	{ 0x080DEA0A, 0x080DEA0C, XV_RAW },
	{ 0x080DEA0C, 0x080DEA10, XV_POOL },
	{ 0x080DEA10, 0x080DEA1C, XV_TABLE },
	{ 0x080DEADC, 0x080DEAE0, XV_POOL },
	{ 0x080DEAE0, 0x080DEAF0, XV_TABLE },
	{ 0x080DECD8, 0x080DECDC, XV_POOL },
	{ 0x080DECDC, 0x080DECEC, XV_RAW },
	{ 0x080DED26, 0x080DED28, XV_RAW },
	{ 0x080DED28, 0x080DED2C, XV_POOL },
	{ 0x080DED2C, 0x080DED34, XV_RAW },
	{ 0x080DEDC6, 0x080DEDC8, XV_RAW },
	{ 0x080DEDC8, 0x080DEDDC, XV_POOL },
	{ 0x080EF126, 0x080EF128, XV_RAW },
	{ 0x080EF128, 0x080EF12C, XV_POOL },
	{ 0x080EF12C, 0x080EF134, XV_TABLE },
	{ 0x080EF19E, 0x080EF1A0, XV_RAW },
	{ 0x080EF1A0, 0x080EF1AC, XV_POOL },
	{ 0x080EF1AC, 0x080EF1B8, XV_TABLE },
};
static const XvPair aqwhirl_calls[] = {
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB00, 0x0800E6E8 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0802EFFE, 0x0802EF5C },   /* map:bytes, SAME */
	{ 0x0800D586, 0x0800F656 },   /* map:bytes, SAME */
	{ 0x0800C64C, 0x0800E24C },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x080EA0BA, 0x08E00C00 },   /* a stub, STUB */
	{ 0x0800D544, 0x0800F614 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800B374, 0x0800CCB2 },   /* map:bytes, SAME */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x0800B6BE, 0x0800D0BC },   /* map:bytes, SAME */
	{ 0x080095E4, 0x0800AE54 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
};
static const XvPair aqwhirl_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080EF1B9, 0x080EF09D },
};
static const XvImm aqwhirl_imms[] = {
	{ 0x080D183C, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D1852, THUMB_MOVS, 2, 0, 0xA, 0xE },   /* a panel set to BN5's type 0xa: BN6's 0xe */
	{ 0x080DA060, THUMB_MOVS, 0, 0, 0x70, 0x5F },   /* its sound 0x70: BN5's copied to BN6's slot 0x5f */
	{ 0x080DA06A, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DA264, THUMB_MOVS, 4, 0, 0x4F, 0x43 },   /* its effect: 0x4f in BN5, 0x43 in BN6 */
	{ 0x080DEA74, THUMB_LDR, 1, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DEB3C, THUMB_MOVS, 1, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080DEC40, THUMB_MOVS, 0, 0, 0xC, 0xE },   /* CurAction 12 written */
};
static const XvNumber aqwhirl_numbers[] = {
	{ 0x080D1B68, 0x01001008, 0x010010F4 },
	{ 0x080D1B6C, 0x04050300, 0x04050300 },
	{ 0x080DA2B0, 0x00000C29, 0x00000CF9 },
	{ 0x080DA2C0, 0x0405FF10, 0x0405FF10 },
	{ 0x080DEDC8, 0x0000104F, 0x000010F5 },
	{ 0x080DEDCC, 0x04050100, 0x04050100 },
};
static const XvSprite aqwhirl_sprites[] = {
	{ 3, 0x29, 3, 0xF9, XC_ART + 0x10800, 0x1000 },
	{ 4, 0x4F, 4, 0xF5, XC_ART + 0x11800, 0x1800 },
	{ 4, 0x08, 4, 0xF4, XC_ART + 0x13000, 0x1000 },
};
static const XvType aqwhirl_types[] = { { 3, 0x5D, XV_TYPE_MOVED }, { 3, 0x9B, XV_TYPE_MOVED }, { 3, 0xC0, XV_TYPE_MOVED } };
static const XcSong aqwhirl_songs[] = { { 0x70, 0x5F } };

#define AQWHIRL_CODE { .name = "AqWhirl (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = aqwhirl_blocks, .nblocks = (int)(sizeof aqwhirl_blocks / sizeof *aqwhirl_blocks), \
	.data = aqwhirl_data, .ndata = (int)(sizeof aqwhirl_data / sizeof *aqwhirl_data), \
	.calls = aqwhirl_calls, .ncalls = (int)(sizeof aqwhirl_calls / sizeof *aqwhirl_calls), \
	.words = aqwhirl_words, .nwords = (int)(sizeof aqwhirl_words / sizeof *aqwhirl_words), \
	.imms = aqwhirl_imms, .nimms = (int)(sizeof aqwhirl_imms / sizeof *aqwhirl_imms), \
	.numbers = aqwhirl_numbers, .nnumbers = (int)(sizeof aqwhirl_numbers / sizeof *aqwhirl_numbers), \
	.sprites = aqwhirl_sprites, .nsprites = (int)(sizeof aqwhirl_sprites / sizeof *aqwhirl_sprites), \
	.types = aqwhirl_types, .ntypes = (int)(sizeof aqwhirl_types / sizeof *aqwhirl_types) }
/* ---- ZSaver: BN5's attack 0x34 (080EE113) as BN6's 0x84 ---- */

static const XvBlock zsaver_blocks[] = {
	{ 0x08012530, 0x08012704, XC_CODE + 0x3350, 0x2030EB14u },
	{ 0x080D0F2C, 0x080D102C, XC_CODE + 0x3530, 0xA9630854u },
	{ 0x080D1052, 0x080D1068, XC_CODE + 0x3632, 0x7A72073Bu },
	{ 0x080E3618, 0x080E36F8, XC_CODE + 0x3650, 0xA30EBD26u },
	{ 0x080EC120, 0x080EC1AC, XC_CODE + 0x3730, 0x2FBBA38Cu },
	{ 0x080EE112, 0x080EE444, XC_CODE + 0x37C2, 0x223BD6DEu },
};
static const XvData zsaver_data[] = {
	{ 0x080125B0, 0x080125B8, XV_POOL },
	{ 0x080125B8, 0x080125C0, XV_RAW },
	{ 0x080125C0, 0x080125CC, XV_RAW },
	{ 0x080126C0, 0x080126C8, XV_POOL },
	{ 0x080126C8, 0x080126D0, XV_RAW },
	{ 0x080126D0, 0x080126FC, XV_RAW },
	{ 0x080126FC, 0x08012704, XV_POOL },
	{ 0x080D0F3A, 0x080D0F3C, XV_RAW },
	{ 0x080D0F3C, 0x080D0F40, XV_POOL },
	{ 0x080D0F40, 0x080D0F4C, XV_TABLE },
	{ 0x080D0FD8, 0x080D0FDC, XV_POOL },
	{ 0x080D0FDC, 0x080D0FE4, XV_RAW },
	{ 0x080D1028, 0x080D102C, XV_POOL },
	{ 0x080E3626, 0x080E3628, XV_RAW },
	{ 0x080E3628, 0x080E362C, XV_POOL },
	{ 0x080E362C, 0x080E3638, XV_TABLE },
	{ 0x080EE120, 0x080EE124, XV_POOL },
	{ 0x080EE124, 0x080EE138, XV_TABLE },
	{ 0x080EE1E8, 0x080EE1F0, XV_RAW },
	{ 0x080EE1F0, 0x080EE1F8, XV_POOL },
	{ 0x080EE3B8, 0x080EE3C0, XV_POOL },
	{ 0x080EE3C0, 0x080EE3CC, XV_RAW },
	{ 0x080EE3CC, 0x080EE3D8, XV_POOL },
	{ 0x080EE3D8, 0x080EE3DC, XV_RAW },
};
static const XvPair zsaver_calls[] = {
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08010DEE, 0x08013774 },   /* map:calls+bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0800DD78, 0x08010018 },   /* map:calls+bytes, SAME */
	{ 0x0801DF0C, 0x08021AA4 },   /* map:bytes, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x080D102C, 0x080D1168 },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002CBE, 0x08002E52 },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800CA7C, 0x0800E5FC },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800D1C6, 0x0800F2AA },   /* map:bytes, SAME */
	{ 0x0800DA6A, 0x0800FCE4 },   /* map:bytes, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0801A514, 0x0801DBD4 },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080101EA, 0x0801299C },   /* map:bytes, SAME */
	{ 0x080101A4, 0x08012956 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080B9CD6, 0x080BA6C8 },   /* search, by place, SAME */
	{ 0x0800F114, 0x08011450 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080B9CAE, 0x08E00E62 },   /* a helper, HELPER */
	{ 0x080B9CE0, 0x080BA6D2 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair zsaver_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm zsaver_imms[] = {
	{ 0x0801261C, THUMB_CMP, 1, 0, 0x8, 0xD },   /* a panel's type compared with BN5's 0x8: BN6's 0xd */
	{ 0x080D0F62, THUMB_MOVS, 2, 0, 0x14, 0xF8 },   /* its sprite: list 3 0x14 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080D0FF4, THUMB_LDR, 0, 2, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite zsaver_sprites[] = {
	{ 3, 0x14, 3, 0xF8, XC_ART + 0x14000, 0x2800 },
};
static const XvId zsaver_effects[] = { { 0x39, 0xFD }, { 0x3A, 0xFC }, { 0x3B, 0xFB } };
static const XvByte zsaver_bytes[] = {
	{ 0x080EE3D8, 0x39, 0xFD },   /* 080EE3D8's row 0: its effect 0x39 in BN5, 0xfd in BN6 */
	{ 0x080EE3D9, 0x3A, 0xFC },   /* 080EE3D8's row 1: its effect 0x3a in BN5, 0xfc in BN6 */
	{ 0x080EE3DA, 0x3B, 0xFB },   /* 080EE3D8's row 2: its effect 0x3b in BN5, 0xfb in BN6 */
};
static const XvType zsaver_types[] = { { 3, 0x58, XV_TYPE_MOVED }, { 4, 0x28, XV_TYPE_MOVED } };

#define ZSAVER_CODE { .name = "ZSaver (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = zsaver_blocks, .nblocks = (int)(sizeof zsaver_blocks / sizeof *zsaver_blocks), \
	.data = zsaver_data, .ndata = (int)(sizeof zsaver_data / sizeof *zsaver_data), \
	.calls = zsaver_calls, .ncalls = (int)(sizeof zsaver_calls / sizeof *zsaver_calls), \
	.words = zsaver_words, .nwords = (int)(sizeof zsaver_words / sizeof *zsaver_words), \
	.imms = zsaver_imms, .nimms = (int)(sizeof zsaver_imms / sizeof *zsaver_imms), \
	.sprites = zsaver_sprites, .nsprites = (int)(sizeof zsaver_sprites / sizeof *zsaver_sprites), \
	.effects = zsaver_effects, .neffects = (int)(sizeof zsaver_effects / sizeof *zsaver_effects), \
	.types = zsaver_types, .ntypes = (int)(sizeof zsaver_types / sizeof *zsaver_types), \
	.bytes = zsaver_bytes, .nbytes = (int)(sizeof zsaver_bytes / sizeof *zsaver_bytes) }
/* ---- Katana: BN5's attack 0x54 (080EFA73) as BN6's 0xa4 ---- */

static const XvBlock katana_blocks[] = {
	{ 0x08012530, 0x08012704, XC_CODE + 0x3B00, 0x2030EB14u },
	{ 0x080E3618, 0x080E36F8, XC_CODE + 0x3CE0, 0xA30EBD26u },
	{ 0x080EC120, 0x080EC1AC, XC_CODE + 0x3DC0, 0x2FBBA38Cu },
	{ 0x080EFA72, 0x080EFD08, XC_CODE + 0x3E52, 0xBFFFFFC8u },
};
static const XvData katana_data[] = {
	{ 0x080125B0, 0x080125B8, XV_POOL },
	{ 0x080125B8, 0x080125C0, XV_RAW },
	{ 0x080125C0, 0x080125CC, XV_RAW },
	{ 0x080126C0, 0x080126C8, XV_POOL },
	{ 0x080126C8, 0x080126D0, XV_RAW },
	{ 0x080126D0, 0x080126FC, XV_RAW },
	{ 0x080126FC, 0x08012704, XV_POOL },
	{ 0x080E3626, 0x080E3628, XV_RAW },
	{ 0x080E3628, 0x080E362C, XV_POOL },
	{ 0x080E362C, 0x080E3638, XV_TABLE },
	{ 0x080EFA80, 0x080EFA84, XV_POOL },
	{ 0x080EFA84, 0x080EFA98, XV_TABLE },
	{ 0x080EFB42, 0x080EFB4C, XV_RAW },
	{ 0x080EFB4C, 0x080EFB54, XV_POOL },
	{ 0x080EFCA2, 0x080EFCA4, XV_RAW },
	{ 0x080EFCA4, 0x080EFCAC, XV_POOL },
	{ 0x080EFCAC, 0x080EFCBC, XV_RAW },
	{ 0x080EFCBC, 0x080EFCC8, XV_POOL },
	{ 0x080EFCC8, 0x080EFCCC, XV_RAW },
};
static const XvPair katana_calls[] = {
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08010DEE, 0x08013774 },   /* map:calls+bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0800DD78, 0x08010018 },   /* map:calls+bytes, SAME */
	{ 0x0801DF0C, 0x08021AA4 },   /* map:bytes, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002CBE, 0x08002E52 },   /* map:bytes, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800CA7C, 0x0800E5FC },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x0800D1C6, 0x0800F2AA },   /* map:bytes, SAME */
	{ 0x0800DA6A, 0x0800FCE4 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0801A514, 0x0801DBD4 },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800F114, 0x08011450 },   /* map:bytes, SAME */
	{ 0x080B9CD6, 0x080BA6C8 },   /* search, by place, SAME */
	{ 0x080B9CAE, 0x08E00E62 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x080B9CE0, 0x080BA6D2 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
};
static const XvPair katana_words[] = {
	{ 0x080EFD09, 0x080EF67B },
};
static const XvImm katana_imms[] = {
	{ 0x0801261C, THUMB_CMP, 1, 0, 0x8, 0xD },   /* a panel's type compared with BN5's 0x8: BN6's 0xd */
};
static const XvType katana_types[] = { { 4, 0x28, XV_TYPE_MOVED } };

#define KATANA_CODE { .name = "Katana (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = katana_blocks, .nblocks = (int)(sizeof katana_blocks / sizeof *katana_blocks), \
	.data = katana_data, .ndata = (int)(sizeof katana_data / sizeof *katana_data), \
	.calls = katana_calls, .ncalls = (int)(sizeof katana_calls / sizeof *katana_calls), \
	.words = katana_words, .nwords = (int)(sizeof katana_words / sizeof *katana_words), \
	.imms = katana_imms, .nimms = (int)(sizeof katana_imms / sizeof *katana_imms), \
	.types = katana_types, .ntypes = (int)(sizeof katana_types / sizeof *katana_types) }
/* ---- ElcReel: BN5's attack 0x23 (080ECF35) as BN6's 0x73 ---- */

static const XvBlock elcreel_blocks[] = {
	{ 0x080DACA4, 0x080DAEA4, XC_CODE + 0x40F0, 0xAD7987AAu },
	{ 0x080ECF34, 0x080ECFE6, XC_CODE + 0x42F0, 0x13F258A9u },
};
static const XvData elcreel_data[] = {
	{ 0x080DACB6, 0x080DACB8, XV_RAW },
	{ 0x080DACB8, 0x080DACBC, XV_POOL },
	{ 0x080DACBC, 0x080DACC8, XV_TABLE },
	{ 0x080DAD10, 0x080DAD14, XV_POOL },
	{ 0x080DAD14, 0x080DAD20, XV_TABLE },
	{ 0x080DAE62, 0x080DAE64, XV_RAW },
	{ 0x080DAE64, 0x080DAE6C, XV_POOL },
	{ 0x080DAE6C, 0x080DAE74, XV_RAW },
	{ 0x080DAE74, 0x080DAE80, XV_RAW },
	{ 0x080DAE92, 0x080DAE94, XV_RAW },
	{ 0x080DAE94, 0x080DAEA4, XV_POOL },
	{ 0x080ECF42, 0x080ECF44, XV_RAW },
	{ 0x080ECF44, 0x080ECF48, XV_POOL },
	{ 0x080ECF48, 0x080ECF50, XV_TABLE },
};
static const XvPair elcreel_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08016C3A, 0x0801A146 },   /* project:chips, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080B9CAE, 0x08E00E62 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair elcreel_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm elcreel_imms[] = {
	{ 0x080DAD24, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DAD98, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080ECF78, THUMB_MOVS, 0, 0, 0xBA, 0x60 },   /* its sound 0xba: BN5's copied to BN6's slot 0x60 */
};
static const XvNumber elcreel_numbers[] = {
	{ 0x080DAE94, 0x0002103E, 0x000210F3 },
	{ 0x080DAE98, 0x0A050100, 0x0A050100 },
	{ 0x080DAEA0, 0x0A050301, 0x0A050301 },
};
static const XvSprite elcreel_sprites[] = {
	{ 4, 0x3E, 4, 0xF3, XC_ART + 0x16800, 0x1000 },
};
static const XvId elcreel_effects[] = { { 0x64, 0xFA }, { 0x65, 0xF9 } };
static const XvByte elcreel_bytes[] = {
	{ 0x080DAE76, 0x64, 0xFA },   /* 080DAE74's row 0: its effect 0x64 in BN5, 0xfa in BN6 */
	{ 0x080DAE79, 0x65, 0xF9 },   /* 080DAE74's row 1: its effect 0x65 in BN5, 0xf9 in BN6 */
	{ 0x080DAE7C, 0x64, 0xFA },   /* 080DAE74's row 2: its effect 0x64 in BN5, 0xfa in BN6 */
	{ 0x080DAE7F, 0x65, 0xF9 },   /* 080DAE74's row 3: its effect 0x65 in BN5, 0xf9 in BN6 */
};
static const XvType elcreel_types[] = { { 3, 0xA4, XV_TYPE_MOVED } };
static const XcSong elcreel_songs[] = { { 0xBA, 0x60 } };

#define ELCREEL_CODE { .name = "ElcReel (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = elcreel_blocks, .nblocks = (int)(sizeof elcreel_blocks / sizeof *elcreel_blocks), \
	.data = elcreel_data, .ndata = (int)(sizeof elcreel_data / sizeof *elcreel_data), \
	.calls = elcreel_calls, .ncalls = (int)(sizeof elcreel_calls / sizeof *elcreel_calls), \
	.words = elcreel_words, .nwords = (int)(sizeof elcreel_words / sizeof *elcreel_words), \
	.imms = elcreel_imms, .nimms = (int)(sizeof elcreel_imms / sizeof *elcreel_imms), \
	.numbers = elcreel_numbers, .nnumbers = (int)(sizeof elcreel_numbers / sizeof *elcreel_numbers), \
	.sprites = elcreel_sprites, .nsprites = (int)(sizeof elcreel_sprites / sizeof *elcreel_sprites), \
	.effects = elcreel_effects, .neffects = (int)(sizeof elcreel_effects / sizeof *elcreel_effects), \
	.types = elcreel_types, .ntypes = (int)(sizeof elcreel_types / sizeof *elcreel_types), \
	.bytes = elcreel_bytes, .nbytes = (int)(sizeof elcreel_bytes / sizeof *elcreel_bytes) }
const XChipUnit xchips_bn5[] = {
	{ XCHIP_HELPER, 0x00, 0x00, 0x080B9CAF, XC_HOMES + 0x0, MUZZLEFLASH_CODE, NULL, 0 },
	{ XCHIP_HELPER, 0x00, 0x00, 0x080EBB97, XC_HOMES + 0x400, BUSTERARM_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x1B, 0x6B, 0x080EC885, XC_HOMES + 0x800, CRAKOUT_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x58, 0xA8, 0x080F0223, XC_HOMES + 0xC00, HOTBODY_CODE, NULL, 0 },
	{ XCHIP_SUPPORT, 0x13, 0x3D, 0x080E4629, XC_HOMES + 0x1000, JEALOUSY_CODE, NULL, 0 },
	{ XCHIP_SUPPORT, 0x15, 0x3F, 0x080E493D, XC_HOMES + 0x1400, NRTHWIND_CODE, NULL, 0 },
	{ XCHIP_SUPPORT, 0x06, 0x30, 0x080E2F85, XC_HOMES + 0x1800, METAGEL_CODE, metagel_songs, (int)(sizeof metagel_songs / sizeof *metagel_songs) },
	{ XCHIP_SUPPORT, 0x08, 0x32, 0x080E3229, XC_HOMES + 0x1C00, BLINDER_CODE, blinder_songs, (int)(sizeof blinder_songs / sizeof *blinder_songs) },
	{ XCHIP_ATTACK, 0x55, 0xA5, 0x080EFD19, XC_HOMES + 0x2000, SKULLY_CODE, skully_songs, (int)(sizeof skully_songs / sizeof *skully_songs) },
	{ XCHIP_ATTACK, 0x4F, 0x9F, 0x080EF579, XC_HOMES + 0x2400, DRILARM_CODE, NULL, 0 },
	{ XCHIP_SUPPORT, 0x3D, 0x67, 0x080E91EF, XC_HOMES + 0x2800, RAINYDAY_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x28, 0x78, 0x080ED36F, XC_HOMES + 0x2C00, WOODNOS_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x48, 0x98, 0x080EF1DD, XC_HOMES + 0x3000, CACTBAL_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x51, 0xA1, 0x080EF733, XC_HOMES + 0x3400, SPSHAKE_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x52, 0xA2, 0x080EF7CB, XC_HOMES + 0x3800, PULSAR_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x4E, 0x9E, 0x080EF441, XC_HOMES + 0x3C00, MRKCAN_CODE, mrkcan_songs, (int)(sizeof mrkcan_songs / sizeof *mrkcan_songs) },
	{ XCHIP_ATTACK, 0x1C, 0x6C, 0x080EC91B, XC_HOMES + 0x4000, SIDEBUB_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x47, 0x97, 0x080EF119, XC_HOMES + 0x4400, AQWHIRL_CODE, aqwhirl_songs, (int)(sizeof aqwhirl_songs / sizeof *aqwhirl_songs) },
	{ XCHIP_ATTACK, 0x34, 0x84, 0x080EE113, XC_HOMES + 0x4800, ZSAVER_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x54, 0xA4, 0x080EFA73, XC_HOMES + 0x4C00, KATANA_CODE, NULL, 0 },
	{ XCHIP_ATTACK, 0x23, 0x73, 0x080ECF35, XC_HOMES + 0x5000, ELCREEL_CODE, elcreel_songs, (int)(sizeof elcreel_songs / sizeof *elcreel_songs) },
};
const int xchips_bn5_count = (int)(sizeof xchips_bn5 / sizeof *xchips_bn5);

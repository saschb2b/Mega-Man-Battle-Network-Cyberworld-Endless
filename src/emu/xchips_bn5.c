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
#include "xnavi_recipe.h"

/* ---- BusterArm: BN5's 080EBB97, a helper (installed once, before the units) ---- */

static const XvBlock busterarm_blocks[] = {
	{ 0x080EBB96, 0x080EBC90, XC_HELPERS + 0x2, 0x61AD6A76u },
};
static const XvData busterarm_data[] = {
	{ 0x080EBC8A, 0x080EBC8C, XV_RAW },
	{ 0x080EBC8C, 0x080EBC90, XV_POOL },
};
static const XvPair busterarm_calls[] = {
	{ 0x08010DEE, 0x08013774 },   /* map:calls+bytes, SAME */
	{ 0x0800E0B0, 0x0801032C },   /* map:bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
};
static const XvType busterarm_types[] = { { 1, 0x05, XV_TYPE_SHARED } };

#define BUSTERARM_CODE { .name = "BusterArm (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = busterarm_blocks, .nblocks = (int)(sizeof busterarm_blocks / sizeof *busterarm_blocks), \
	.data = busterarm_data, .ndata = (int)(sizeof busterarm_data / sizeof *busterarm_data), \
	.calls = busterarm_calls, .ncalls = (int)(sizeof busterarm_calls / sizeof *busterarm_calls), \
	.types = busterarm_types, .ntypes = (int)(sizeof busterarm_types / sizeof *busterarm_types) }
/* ---- Shake: BN5's 080D3287, a helper (installed once, before the units) ---- */

static const XvBlock shake_blocks[] = {
	{ 0x080D30E0, 0x080D32AC, XC_HELPERS + 0x100, 0x3354D1A3u },
};
static const XvData shake_data[] = {
	{ 0x080D30F2, 0x080D30F4, XV_RAW },
	{ 0x080D30F4, 0x080D30F8, XV_POOL },
	{ 0x080D30F8, 0x080D3104, XV_TABLE },
	{ 0x080D31B6, 0x080D31B8, XV_RAW },
	{ 0x080D31B8, 0x080D31BC, XV_POOL },
	{ 0x080D31BC, 0x080D31C8, XV_RAW },
	{ 0x080D323C, 0x080D3240, XV_POOL },
	{ 0x080D3240, 0x080D3244, XV_TABLE },
	{ 0x080D32A4, 0x080D32AC, XV_POOL },
};
static const XvPair shake_calls[] = {
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
};
static const XvPair shake_words[] = {
	{ 0x08005CD0, 0x080065E0 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm shake_imms[] = {
	{ 0x080D310C, THUMB_MOVS, 2, 0, 0x7, 0xFF },   /* its sprite: list 1 0x7 in BN5, 0xff in BN6 (BN5's copied there) */
	{ 0x080D320A, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite shake_sprites[] = {
	{ 1, 0x07, 1, 0xFF, XC_ART + 0x0, 0x1000 },
};
static const XvType shake_types[] = { { 3, 0x68, XV_TYPE_MOVED } };

#define SHAKE_CODE { .name = "Shake (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = shake_blocks, .nblocks = (int)(sizeof shake_blocks / sizeof *shake_blocks), \
	.data = shake_data, .ndata = (int)(sizeof shake_data / sizeof *shake_data), \
	.calls = shake_calls, .ncalls = (int)(sizeof shake_calls / sizeof *shake_calls), \
	.words = shake_words, .nwords = (int)(sizeof shake_words / sizeof *shake_words), \
	.imms = shake_imms, .nimms = (int)(sizeof shake_imms / sizeof *shake_imms), \
	.sprites = shake_sprites, .nsprites = (int)(sizeof shake_sprites / sizeof *shake_sprites), \
	.types = shake_types, .ntypes = (int)(sizeof shake_types / sizeof *shake_types) }
/* ---- PowerCustSwrd: BN5's 0800EA19, a helper (installed once, before the units) ---- */

static const XvBlock powercustswrd_blocks[] = {
	{ 0x0800EA18, 0x0800EC48, XC_HELPERS + 0x2D0, 0xC1592C94u },
};
static const XvData powercustswrd_data[] = {
	{ 0x0800EA70, 0x0800EB90, XV_RAW },
	{ 0x0800EB90, 0x0800EB94, XV_POOL },
	{ 0x0800EB94, 0x0800EC44, XV_RAW },
	{ 0x0800EC44, 0x0800EC48, XV_POOL },
};
static const XvPair powercustswrd_calls[] = {
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0802D4E6, 0x0802E070 },   /* map:bytes, SAME */
	{ 0x0801A8D0, 0x0801DFE4 },   /* map:calls+bytes, SAME */
};

#define POWERCUSTSWRD_CODE { .name = "PowerCustSwrd (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = powercustswrd_blocks, .nblocks = (int)(sizeof powercustswrd_blocks / sizeof *powercustswrd_blocks), \
	.data = powercustswrd_data, .ndata = (int)(sizeof powercustswrd_data / sizeof *powercustswrd_data), \
	.calls = powercustswrd_calls, .ncalls = (int)(sizeof powercustswrd_calls / sizeof *powercustswrd_calls) }
/* ---- PowerCusVolt: BN5's 0800EB07, a helper (installed once, before the units) ---- */

static const XvBlock powercusvolt_blocks[] = {
	{ 0x0800EB06, 0x0800EB50, XC_HELPERS + 0x502, 0x1210F73Au },
};
static const XvPair powercusvolt_calls[] = {
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0802D4E6, 0x0802E070 },   /* map:bytes, SAME */
	{ 0x0801A8D0, 0x0801DFE4 },   /* map:calls+bytes, SAME */
};

#define POWERCUSVOLT_CODE { .name = "PowerCusVolt (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = powercusvolt_blocks, .nblocks = (int)(sizeof powercusvolt_blocks / sizeof *powercusvolt_blocks), \
	.calls = powercusvolt_calls, .ncalls = (int)(sizeof powercusvolt_calls / sizeof *powercusvolt_calls) }
/* ---- PowerSP: BN5's 0800E8D7, a helper (installed once, before the units) ---- */

static const XvBlock powersp_blocks[] = {
	{ 0x0800E8D6, 0x0800EC38, XC_HELPERS + 0x552, 0xB626432Bu },
	{ 0x0801D27A, 0x0801D39A, XC_HELPERS + 0x8C2, 0xE261AFDBu },
};
static const XvData powersp_data[] = {
	{ 0x0800E92A, 0x0800E92C, XV_RAW },
	{ 0x0800E92C, 0x0800E934, XV_POOL },
	{ 0x0800E934, 0x0800E95C, XV_RAW },
	{ 0x0800E95C, 0x0800E9E8, XV_RAW },
	{ 0x0800E9E8, 0x0800E9EC, XV_POOL },
	{ 0x0800E9EC, 0x0800EB90, XV_RAW },
	{ 0x0800EB90, 0x0800EB94, XV_POOL },
	{ 0x0800EB94, 0x0800EC28, XV_RAW },
	{ 0x0800EC28, 0x0800EC38, XV_POOL },
	{ 0x0801D27A, 0x0801D39A, XV_RAW },
};
static const XvPair powersp_calls[] = {
	{ 0x0801DF0C, 0x08021AA4 },   /* map:bytes, SAME */
	{ 0x08000CA4, 0x08000D84 },   /* map:bytes, SAME */
	{ 0x080090E8, 0x08E00C08 },   /* a stub, STUB */
};
static const XvPair powersp_words[] = {
	{ 0x0203E560, 0x0203E560 },
};
static const XvNumber powersp_numbers[] = {
	{ 0x0800EC2C, 0x000003E9, 0x00000469 },
};

#define POWERSP_CODE { .name = "PowerSP (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = powersp_blocks, .nblocks = (int)(sizeof powersp_blocks / sizeof *powersp_blocks), \
	.data = powersp_data, .ndata = (int)(sizeof powersp_data / sizeof *powersp_data), \
	.calls = powersp_calls, .ncalls = (int)(sizeof powersp_calls / sizeof *powersp_calls), \
	.words = powersp_words, .nwords = (int)(sizeof powersp_words / sizeof *powersp_words), \
	.numbers = powersp_numbers, .nnumbers = (int)(sizeof powersp_numbers / sizeof *powersp_numbers) }
/* ---- PowerDS: BN5's 0800E9A9, a helper (installed once, before the units) ---- */

static const XvBlock powerds_blocks[] = {
	{ 0x0800E9A8, 0x0800EC44, XC_HELPERS + 0x9F0, 0xF5055B73u },
	{ 0x0801D27A, 0x0801D39A, XC_HELPERS + 0xC92, 0xE261AFDBu },
};
static const XvData powerds_data[] = {
	{ 0x0800E9E6, 0x0800E9E8, XV_RAW },
	{ 0x0800E9E8, 0x0800E9EC, XV_POOL },
	{ 0x0800E9EC, 0x0800EB90, XV_RAW },
	{ 0x0800EB90, 0x0800EB94, XV_POOL },
	{ 0x0800EB94, 0x0800EC28, XV_RAW },
	{ 0x0800EC28, 0x0800EC2C, XV_POOL },
	{ 0x0800EC2C, 0x0800EC34, XV_RAW },
	{ 0x0800EC34, 0x0800EC38, XV_POOL },
	{ 0x0800EC38, 0x0800EC3C, XV_RAW },
	{ 0x0800EC3C, 0x0800EC44, XV_POOL },
	{ 0x0801D27A, 0x0801D39A, XV_RAW },
};
static const XvPair powerds_calls[] = {
	{ 0x0801DF0C, 0x08021AA4 },   /* map:bytes, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0800931C, 0x0800AB3A },   /* map:calls, DIFF (audited) */
};
static const XvNumber powerds_numbers[] = {
	{ 0x0800EC3C, 0x000003FF, 0x0000047F },
};

#define POWERDS_CODE { .name = "PowerDS (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = powerds_blocks, .nblocks = (int)(sizeof powerds_blocks / sizeof *powerds_blocks), \
	.data = powerds_data, .ndata = (int)(sizeof powerds_data / sizeof *powerds_data), \
	.calls = powerds_calls, .ncalls = (int)(sizeof powerds_calls / sizeof *powerds_calls), \
	.numbers = powerds_numbers, .nnumbers = (int)(sizeof powerds_numbers / sizeof *powerds_numbers) }
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
	{ 0x080C7900, 0x080C790C, XV_POOL },
	{ 0x080C790C, 0x080C7914, XV_RAW },
	{ 0x080C7914, 0x080C791C, XV_RAW },
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
	{ 5, 0x0E, 5, 0xFF, XC_ART + 0x1000, 0x1000 },
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
	{ 4, 0x40, 4, 0xFF, XC_ART + 0x2000, 0x2000 },
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
	{ 0x080CE170, THUMB_MOVS, 0, 0, 0x6, 0x59 },   /* its sound 0x105: BN5's copied to BN6's slot 0x59 */
	{ 0x080CE172, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x105: BN5's copied to BN6's slot 0x59 (its high part) */
};
static const XvSprite metagel_sprites[] = {
	{ 3, 0x26, 3, 0xFF, XC_ART + 0x4000, 0x1800 },
};
static const XvType metagel_types[] = { { 3, 0x45, XV_TYPE_MOVED }, { 4, 0x21, XV_TYPE_MOVED } };
static const XcSong metagel_songs[] = { { 0x105, 0x59 } };

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
	{ 4, 0x50, 4, 0xFE, XC_ART + 0x5800, 0x800 },
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
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
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
static const XvType drilarm_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0x71, XV_TYPE_MOVED } };

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
	{ 3, 0x36, 3, 0xFE, XC_ART + 0x6000, 0x1000 },
	{ 3, 0x37, 3, 0xFD, XC_ART + 0x7000, 0x1000 },
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
	{ 0x08016C3A, 0x0801A146 },   /* project:families, DIFF (audited) */
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
	{ 0x080EBB96, 0x08E00C22 },   /* a helper, HELPER */
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
	{ 0x080DB338, 0x00001021, 0x0000106C },
	{ 0x080DB33C, 0x04050401, 0x04050401 },
};
static const XvSprite woodnos_sprites[] = {
	{ 3, 0x54, 3, 0xFC, XC_ART + 0x8000, 0x800 },
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
	{ 0x08016C3A, 0x0801A146 },   /* project:families, DIFF (audited) */
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
	{ 0x080EBB96, 0x08E00C22 },   /* a helper, HELPER */
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
	{ 0x080D1E58, 0x00041030, 0x000410FD },
	{ 0x080D1E5C, 0x04050100, 0x04050100 },
};
static const XvSprite cactbal_sprites[] = {
	{ 4, 0x30, 4, 0xFD, XC_ART + 0x8800, 0x1800 },
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
	{ 0x080EF732, 0x080EF7CA, XC_CODE + 0x19D2, 0x7779DD86u },
};
static const XvData spshake_data[] = {
	{ 0x080EF740, 0x080EF744, XV_POOL },
	{ 0x080EF744, 0x080EF74C, XV_TABLE },
};
static const XvPair spshake_calls[] = {
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080EBB96, 0x08E00C22 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080D3286, 0x08E00EC6 },   /* a helper, HELPER */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};

#define SPSHAKE_CODE { .name = "SpShake (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = spshake_blocks, .nblocks = (int)(sizeof spshake_blocks / sizeof *spshake_blocks), \
	.data = spshake_data, .ndata = (int)(sizeof spshake_data / sizeof *spshake_data), \
	.calls = spshake_calls, .ncalls = (int)(sizeof spshake_calls / sizeof *spshake_calls) }
/* ---- Pulsar: BN5's attack 0x52 (080EF7CB) as BN6's 0xa2 ---- */

static const XvBlock pulsar_blocks[] = {
	{ 0x080D33B8, 0x080D3570, XC_CODE + 0x1A70, 0xB5B961B7u },
	{ 0x080EF7CA, 0x080EF872, XC_CODE + 0x1C32, 0x043CE56Fu },
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
	{ 0x080EBB96, 0x08E00C22 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair pulsar_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm pulsar_imms[] = {
	{ 0x080D33E4, THUMB_MOVS, 2, 0, 0x3A, 0xFC },   /* its sprite: list 4 0x3a in BN5, 0xfc in BN6 (BN5's copied there) */
	{ 0x080D3476, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D3514, THUMB_ADDS, 4, 0, 0x56, 0xFE },   /* its effect: 0x56 in BN5, 0xfe in BN6 */
};
static const XvNumber pulsar_numbers[] = {
	{ 0x080D3548, 0x0405FF10, 0x0405FF10 },
	{ 0x080D356C, 0x03800000, 0x03800000 },
};
static const XvSprite pulsar_sprites[] = {
	{ 4, 0x3A, 4, 0xFC, XC_ART + 0xA000, 0x800 },
	{ 4, 0x3B, 4, 0xFB, XC_ART + 0xA800, 0x1000 },
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
	{ 0x080E5580, 0x080E57B4, XC_CODE + 0x1CE0, 0xD17D0F24u },
	{ 0x080EF440, 0x080EF578, XC_CODE + 0x1F20, 0x35D98A37u },
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
	{ 0x080EF55C, 0x080EF560, XV_POOL },
	{ 0x080EF560, 0x080EF578, XV_RAW },
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
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
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
	{ 0x080E5794, 0x00001024, 0x000010FA },
	{ 0x080E57AC, 0x0405FF01, 0x0405FF01 },
	{ 0x080EF518, 0x04050701, 0x04050701 },
};
static const XvSprite mrkcan_sprites[] = {
	{ 4, 0x24, 4, 0xFA, XC_ART + 0xB800, 0x800 },
};
static const XvType mrkcan_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 4, 0x44, XV_TYPE_MOVED } };
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
	{ 0x08016B96, 0x08016BBC, XC_CODE + 0x2062, 0xA53A3BEFu },
	{ 0x080C5386, 0x080C53DE, XC_CODE + 0x2092, 0x4A1B50AFu },
	{ 0x080C5DF4, 0x080C6000, XC_CODE + 0x20F0, 0xEDB80F1Au },
	{ 0x080D856C, 0x080D87D0, XC_CODE + 0x2300, 0x7524551Eu },
	{ 0x080EC91A, 0x080EC9E0, XC_CODE + 0x2572, 0xE2038DC0u },
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
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
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
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080EBB96, 0x08E00C22 },   /* a helper, HELPER */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair sidebub_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm sidebub_imms[] = {
	{ 0x080C5F74, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D85AA, THUMB_MOVS, 2, 0, 0x41, 0xF9 },   /* its sprite: list 4 0x41 in BN5, 0xf9 in BN6 (BN5's copied there) */
	{ 0x080D869A, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite sidebub_sprites[] = {
	{ 4, 0x41, 4, 0xF9, XC_ART + 0xC000, 0x1000 },
	{ 3, 0x3E, 3, 0xFB, XC_ART + 0xD000, 0x800 },
	{ 3, 0x43, 3, 0xFA, XC_ART + 0xD800, 0x1000 },
	{ 4, 0x11, 4, 0xF8, XC_ART + 0xE800, 0x1000 },
	{ 4, 0x2D, 4, 0xF7, XC_ART + 0xF800, 0x800 },
};
static const XvByte sidebub_bytes[] = {
	{ 0x080C5DFA, 0x3E, 0xFB },   /* 080C5DF4's row 0: its sprite list 3 0x3e in BN5, 0xfb in BN6 (BN5's copied there) */
	{ 0x080C5E0A, 0x43, 0xFA },   /* 080C5DF4's row 1: its sprite list 3 0x43 in BN5, 0xfa in BN6 (BN5's copied there) */
	{ 0x080C5E1A, 0x11, 0xF8 },   /* 080C5DF4's row 2: its sprite list 4 0x11 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5E2A, 0x11, 0xF8 },   /* 080C5DF4's row 3: its sprite list 4 0x11 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5E3A, 0x11, 0xF8 },   /* 080C5DF4's row 4: its sprite list 4 0x11 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5E4A, 0x11, 0xF8 },   /* 080C5DF4's row 5: its sprite list 4 0x11 in BN5, 0xf8 in BN6 (BN5's copied there) */
	{ 0x080C5E5A, 0x2D, 0xF7 },   /* 080C5DF4's row 6: its sprite list 4 0x2d in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080C5E6A, 0x41, 0xF9 },   /* 080C5DF4's row 7: its sprite list 4 0x41 in BN5, 0xf9 in BN6 (BN5's copied there) */
	{ 0x080C5E7A, 0x41, 0xF9 },   /* 080C5DF4's row 8: its sprite list 4 0x41 in BN5, 0xf9 in BN6 (BN5's copied there) */
	{ 0x080C5E8A, 0x41, 0xF9 },   /* 080C5DF4's row 9: its sprite list 4 0x41 in BN5, 0xf9 in BN6 (BN5's copied there) */
	{ 0x080C5E9A, 0x41, 0xF9 },   /* 080C5DF4's row 10: its sprite list 4 0x41 in BN5, 0xf9 in BN6 (BN5's copied there) */
	{ 0x080C5EAA, 0x11, 0xF8 },   /* 080C5DF4's row 11: its sprite list 4 0x11 in BN5, 0xf8 in BN6 (BN5's copied there) */
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
	{ 0x080C5386, 0x080C53DE, XC_CODE + 0x2642, 0x4A1B50AFu },
	{ 0x080D17C8, 0x080D1B70, XC_CODE + 0x26A0, 0xFEB7C3C6u },
	{ 0x080D9FCC, 0x080DA2C8, XC_CODE + 0x2A50, 0x25761344u },
	{ 0x080DE9F8, 0x080DEDDC, XC_CODE + 0x2D50, 0xF9535385u },
	{ 0x080EF118, 0x080EF1B8, XC_CODE + 0x3140, 0x21F7A2E1u },
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
	{ 0x080D1B68, 0x01001008, 0x010010F5 },
	{ 0x080D1B6C, 0x04050300, 0x04050300 },
	{ 0x080DA2B0, 0x00000C29, 0x00000CF9 },
	{ 0x080DA2C0, 0x0405FF10, 0x0405FF10 },
	{ 0x080DEDC8, 0x0000104F, 0x000010F6 },
	{ 0x080DEDCC, 0x04050100, 0x04050100 },
};
static const XvSprite aqwhirl_sprites[] = {
	{ 3, 0x29, 3, 0xF9, XC_ART + 0x10000, 0x1000 },
	{ 4, 0x4F, 4, 0xF6, XC_ART + 0x11000, 0x1800 },
	{ 4, 0x08, 4, 0xF5, XC_ART + 0x12800, 0x1000 },
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
	{ 0x08012530, 0x08012704, XC_CODE + 0x31E0, 0x2030EB14u },
	{ 0x080D0F2C, 0x080D102C, XC_CODE + 0x33C0, 0xA9630854u },
	{ 0x080D1052, 0x080D1068, XC_CODE + 0x34C2, 0x7A72073Bu },
	{ 0x080E3618, 0x080E36F8, XC_CODE + 0x34E0, 0xA30EBD26u },
	{ 0x080EC120, 0x080EC1AC, XC_CODE + 0x35C0, 0x2FBBA38Cu },
	{ 0x080EE112, 0x080EE444, XC_CODE + 0x3652, 0x223BD6DEu },
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
	{ 0x080B9CD6, XN_OBJECTS + 0x27A },   /* the moved objects, SAME */
	{ 0x0800F114, 0x08011450 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x080B9CE0, XN_OBJECTS + 0x284 },   /* the moved objects, SAME */
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
	{ 3, 0x14, 3, 0xF8, XC_ART + 0x13800, 0x2800 },
};
static const XvId zsaver_effects[] = { { 0x39, 0xFD }, { 0x3A, 0xFC }, { 0x3B, 0xFB } };
static const XvByte zsaver_bytes[] = {
	{ 0x080EE3D8, 0x39, 0xFD },   /* 080EE3D8's row 0: its effect 0x39 in BN5, 0xfd in BN6 */
	{ 0x080EE3D9, 0x3A, 0xFC },   /* 080EE3D8's row 1: its effect 0x3a in BN5, 0xfc in BN6 */
	{ 0x080EE3DA, 0x3B, 0xFB },   /* 080EE3D8's row 2: its effect 0x3b in BN5, 0xfb in BN6 */
};
static const XvType zsaver_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0x58, XV_TYPE_MOVED }, { 4, 0x28, XV_TYPE_MOVED } };

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
	{ 0x08012530, 0x08012704, XC_CODE + 0x3990, 0x2030EB14u },
	{ 0x080E3618, 0x080E36F8, XC_CODE + 0x3B70, 0xA30EBD26u },
	{ 0x080EC120, 0x080EC1AC, XC_CODE + 0x3C50, 0x2FBBA38Cu },
	{ 0x080EFA72, 0x080EFD08, XC_CODE + 0x3CE2, 0xBFFFFFC8u },
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
	{ 0x080B9CD6, XN_OBJECTS + 0x27A },   /* the moved objects, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x080B9CE0, XN_OBJECTS + 0x284 },   /* the moved objects, SAME */
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
static const XvType katana_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 4, 0x28, XV_TYPE_MOVED } };

#define KATANA_CODE { .name = "Katana (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = katana_blocks, .nblocks = (int)(sizeof katana_blocks / sizeof *katana_blocks), \
	.data = katana_data, .ndata = (int)(sizeof katana_data / sizeof *katana_data), \
	.calls = katana_calls, .ncalls = (int)(sizeof katana_calls / sizeof *katana_calls), \
	.words = katana_words, .nwords = (int)(sizeof katana_words / sizeof *katana_words), \
	.imms = katana_imms, .nimms = (int)(sizeof katana_imms / sizeof *katana_imms), \
	.types = katana_types, .ntypes = (int)(sizeof katana_types / sizeof *katana_types) }
/* ---- ElcReel: BN5's attack 0x23 (080ECF35) as BN6's 0x73 ---- */

static const XvBlock elcreel_blocks[] = {
	{ 0x080DACA4, 0x080DAEA4, XC_CODE + 0x3F80, 0xAD7987AAu },
	{ 0x080ECF34, 0x080ECFE6, XC_CODE + 0x4180, 0x13F258A9u },
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
	{ 0x08016C3A, 0x0801A146 },   /* project:families, DIFF (audited) */
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
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
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
	{ 0x080DAE94, 0x0002103E, 0x000210F4 },
	{ 0x080DAE98, 0x0A050100, 0x0A050100 },
	{ 0x080DAEA0, 0x0A050301, 0x0A050301 },
};
static const XvSprite elcreel_sprites[] = {
	{ 4, 0x3E, 4, 0xF4, XC_ART + 0x16000, 0x1000 },
};
static const XvId elcreel_effects[] = { { 0x64, 0xFA }, { 0x65, 0xF9 } };
static const XvByte elcreel_bytes[] = {
	{ 0x080DAE76, 0x64, 0xFA },   /* 080DAE74's row 0: its effect 0x64 in BN5, 0xfa in BN6 */
	{ 0x080DAE79, 0x65, 0xF9 },   /* 080DAE74's row 1: its effect 0x65 in BN5, 0xf9 in BN6 */
	{ 0x080DAE7C, 0x64, 0xFA },   /* 080DAE74's row 2: its effect 0x64 in BN5, 0xfa in BN6 */
	{ 0x080DAE7F, 0x65, 0xF9 },   /* 080DAE74's row 3: its effect 0x65 in BN5, 0xf9 in BN6 */
};
static const XvType elcreel_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0xA4, XV_TYPE_MOVED } };
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
/* ---- BoyBomb: BN5's support 0x3a (080E816F) as BN6's 0x64 ---- */

static const XvBlock boybomb_blocks[] = {
	{ 0x080CCEDC, 0x080CCEFC, XC_CODE + 0x4240, 0x60DBE9DAu },
	{ 0x080CCF0E, 0x080CCFD0, XC_CODE + 0x4262, 0xD30E7093u },
	{ 0x080CD1C8, 0x080CD200, XC_CODE + 0x4330, 0x36DE3168u },
	{ 0x080E80F0, 0x080E8190, XC_CODE + 0x4370, 0xEB27C392u },
};
static const XvData boybomb_data[] = {
	{ 0x080CCEEA, 0x080CCEEC, XV_RAW },
	{ 0x080CCEEC, 0x080CCEF0, XV_POOL },
	{ 0x080CCEF0, 0x080CCEFC, XV_TABLE },
	{ 0x080E80FE, 0x080E8100, XV_RAW },
	{ 0x080E8100, 0x080E8104, XV_POOL },
	{ 0x080E8104, 0x080E8110, XV_TABLE },
	{ 0x080E811E, 0x080E8120, XV_RAW },
	{ 0x080E8120, 0x080E8124, XV_POOL },
	{ 0x080E8124, 0x080E8134, XV_TABLE },
};
static const XvPair boybomb_calls[] = {
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08002AE6, 0x08002C7A },   /* map:calls+bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800D544, 0x0800F614 },   /* map:bytes, SAME */
	{ 0x080E8190, 0x080E91A4 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
};
static const XvPair boybomb_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080CCEFD, 0x080CD3D5 },
	{ 0x080CCFD1, 0x080CD4A9 },
};
static const XvImm boybomb_imms[] = {
	{ 0x080CCF16, THUMB_MOVS, 2, 0, 0x36, 0xF3 },   /* its sprite: list 4 0x36 in BN5, 0xf3 in BN6 (BN5's copied there) */
};
static const XvSprite boybomb_sprites[] = {
	{ 4, 0x36, 4, 0xF3, XC_ART + 0x17000, 0x800 },
};
static const XvType boybomb_types[] = { { 3, 0x3E, XV_TYPE_MOVED }, { 4, 0x67, XV_TYPE_MOVED } };

#define BOYBOMB_CODE { .name = "BoyBomb (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = boybomb_blocks, .nblocks = (int)(sizeof boybomb_blocks / sizeof *boybomb_blocks), \
	.data = boybomb_data, .ndata = (int)(sizeof boybomb_data / sizeof *boybomb_data), \
	.calls = boybomb_calls, .ncalls = (int)(sizeof boybomb_calls / sizeof *boybomb_calls), \
	.words = boybomb_words, .nwords = (int)(sizeof boybomb_words / sizeof *boybomb_words), \
	.imms = boybomb_imms, .nimms = (int)(sizeof boybomb_imms / sizeof *boybomb_imms), \
	.sprites = boybomb_sprites, .nsprites = (int)(sizeof boybomb_sprites / sizeof *boybomb_sprites), \
	.types = boybomb_types, .ntypes = (int)(sizeof boybomb_types / sizeof *boybomb_types) }
/* ---- Guard: BN5's attack 0x1f (080ECBCF) as BN6's 0x6f ---- */

static const XvBlock guard_blocks[] = {
	{ 0x08016B96, 0x08016BBC, XC_CODE + 0x4412, 0xA53A3BEFu },
	{ 0x080C4800, 0x080C4AD8, XC_CODE + 0x4440, 0x0F349360u },
	{ 0x080C7448, 0x080C762C, XC_CODE + 0x4720, 0x3C691250u },
	{ 0x080CA2D8, 0x080CA480, XC_CODE + 0x4910, 0x83E20487u },
	{ 0x080DB340, 0x080DB4A0, XC_CODE + 0x4AC0, 0x2787CEBCu },
	{ 0x080ECBCE, 0x080ECCEC, XC_CODE + 0x4C22, 0xDFB8E09Cu },
	{ 0x080ECCF8, 0x080ECCFC, XC_CODE + 0x4D40, 0xFDC9DE79u },
};
static const XvData guard_data[] = {
	{ 0x080C4800, 0x080C4974, XV_RAW },
	{ 0x080C4986, 0x080C4988, XV_RAW },
	{ 0x080C4988, 0x080C498C, XV_POOL },
	{ 0x080C498C, 0x080C4998, XV_TABLE },
	{ 0x080C4AD2, 0x080C4AD4, XV_RAW },
	{ 0x080C4AD4, 0x080C4AD8, XV_POOL },
	{ 0x080C7448, 0x080C7484, XV_RAW },
	{ 0x080C7496, 0x080C7498, XV_RAW },
	{ 0x080C7498, 0x080C749C, XV_POOL },
	{ 0x080C749C, 0x080C74A8, XV_TABLE },
	{ 0x080C754E, 0x080C7550, XV_RAW },
	{ 0x080C7550, 0x080C7554, XV_POOL },
	{ 0x080C7594, 0x080C7598, XV_POOL },
	{ 0x080C7598, 0x080C75A0, XV_TABLE },
	{ 0x080CA2D8, 0x080CA314, XV_RAW },
	{ 0x080CA32C, 0x080CA330, XV_POOL },
	{ 0x080CA330, 0x080CA33C, XV_TABLE },
	{ 0x080CA3DA, 0x080CA3DC, XV_RAW },
	{ 0x080CA3DC, 0x080CA3E0, XV_POOL },
	{ 0x080CA3E0, 0x080CA3E8, XV_TABLE },
	{ 0x080CA478, 0x080CA480, XV_POOL },
	{ 0x080DB352, 0x080DB354, XV_RAW },
	{ 0x080DB354, 0x080DB358, XV_POOL },
	{ 0x080DB358, 0x080DB364, XV_TABLE },
	{ 0x080DB3EC, 0x080DB3F0, XV_POOL },
	{ 0x080DB3F0, 0x080DB3FC, XV_TABLE },
	{ 0x080DB496, 0x080DB498, XV_RAW },
	{ 0x080DB498, 0x080DB4A0, XV_POOL },
	{ 0x080ECBDC, 0x080ECBE0, XV_POOL },
	{ 0x080ECBE0, 0x080ECBE4, XV_TABLE },
	{ 0x080ECCE0, 0x080ECCE4, XV_RAW },
	{ 0x080ECCE4, 0x080ECCEC, XV_POOL },
	{ 0x080ECCF8, 0x080ECCFC, XV_RAW },
};
static const XvPair guard_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x080172CE, 0x0801A4D0 },   /* map:bytes, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800AFF0, 0x0800C938 },   /* map:calls, DIFF (audited) */
	{ 0x0800B144, 0x0800CA8C },   /* map:calls, DIFF (audited) */
	{ 0x0800B374, 0x0800CCB2 },   /* map:bytes, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x080C762C, 0x080C856C },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B380, 0x0800CCBE },   /* project:research, DIFF (audited) */
	{ 0x080EA08C, 0x08E00C00 },   /* a stub, STUB */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800CDEC, 0x0800EB6C },   /* map:bytes, SAME */
	{ 0x0800E0B0, 0x0801032C },   /* map:bytes, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x08015140, 0x08018810 },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0802D4A8, 0x0802E032 },   /* map:bytes, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair guard_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm guard_imms[] = {
	{ 0x080C4A2E, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C753C, THUMB_MOVS, 0, 0, 0xA6, 0x51 },   /* its sound 0xa6: BN5's copied to BN6's slot 0x51 */
	{ 0x080C7576, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DB400, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080ECC4C, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvNumber guard_numbers[] = {
	{ 0x080DB498, 0x01010405, 0x010104FE },
};
static const XvSprite guard_sprites[] = {
	{ 4, 0x04, 4, 0xF2, XC_ART + 0x17800, 0x1800 },
	{ 1, 0x05, 1, 0xFE, XC_ART + 0x19000, 0x1800 },
	{ 3, 0x00, 3, 0xF7, XC_ART + 0x1A800, 0x5800 },
	{ 2, 0x01, 2, 0xFF, XC_ART + 0x20000, 0x9800 },
};
static const XvByte guard_bytes[] = {
	{ 0x080C7448, 0x04, 0xF2 },   /* 080C7448's row 0: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C744C, 0x04, 0xF2 },   /* 080C7448's row 1: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7450, 0x04, 0xF2 },   /* 080C7448's row 2: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7454, 0x04, 0xF2 },   /* 080C7448's row 3: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7458, 0x04, 0xF2 },   /* 080C7448's row 4: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C745C, 0x04, 0xF2 },   /* 080C7448's row 5: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7460, 0x04, 0xF2 },   /* 080C7448's row 6: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7464, 0x04, 0xF2 },   /* 080C7448's row 7: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7468, 0x04, 0xF2 },   /* 080C7448's row 8: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C746C, 0x04, 0xF2 },   /* 080C7448's row 9: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7470, 0x04, 0xF2 },   /* 080C7448's row 10: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7474, 0x04, 0xF2 },   /* 080C7448's row 11: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7478, 0x04, 0xF2 },   /* 080C7448's row 12: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C747C, 0x04, 0xF2 },   /* 080C7448's row 13: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C7480, 0x04, 0xF2 },   /* 080C7448's row 14: its sprite list 4 0x4 in BN5, 0xf2 in BN6 (BN5's copied there) */
	{ 0x080C481E, 0x00, 0xF7 },   /* 080C4800's row 2: its sprite list 3 0x0 in BN5, 0xf7 in BN6 (BN5's copied there) */
	{ 0x080CA2EB, 0x01, 0xFF },   /* 080CA2D8's row 3: its sprite list 2 0x1 in BN5, 0xff in BN6 (BN5's copied there) */
	{ 0x080CA2F1, 0x05, 0xFE },   /* 080CA2D8's row 4: its sprite list 1 0x5 in BN5, 0xfe in BN6 (BN5's copied there) */
	{ 0x080CA2F7, 0x05, 0xFE },   /* 080CA2D8's row 5: its sprite list 1 0x5 in BN5, 0xfe in BN6 (BN5's copied there) */
	{ 0x080CA2FD, 0x05, 0xFE },   /* 080CA2D8's row 6: its sprite list 1 0x5 in BN5, 0xfe in BN6 (BN5's copied there) */
	{ 0x080C48F8, 0x0A, 0x0E },   /* 080C4800's row 20: a panel set to BN5's type 0xa: BN6's 0xe */
	{ 0x080C4940, 0x08, 0x0D },   /* 080C4800's row 26: a panel set to BN5's type 0x8: BN6's 0xd */
};
static const XvType guard_types[] = { { 3, 0x00, XV_TYPE_MOVED }, { 3, 0x16, XV_TYPE_MOVED }, { 3, 0x2B, XV_TYPE_MOVED }, { 3, 0xA8, XV_TYPE_MOVED } };
static const XcSong guard_songs[] = { { 0xA6, 0x51 } };

#define GUARD_CODE { .name = "Guard (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = guard_blocks, .nblocks = (int)(sizeof guard_blocks / sizeof *guard_blocks), \
	.data = guard_data, .ndata = (int)(sizeof guard_data / sizeof *guard_data), \
	.calls = guard_calls, .ncalls = (int)(sizeof guard_calls / sizeof *guard_calls), \
	.words = guard_words, .nwords = (int)(sizeof guard_words / sizeof *guard_words), \
	.imms = guard_imms, .nimms = (int)(sizeof guard_imms / sizeof *guard_imms), \
	.numbers = guard_numbers, .nnumbers = (int)(sizeof guard_numbers / sizeof *guard_numbers), \
	.sprites = guard_sprites, .nsprites = (int)(sizeof guard_sprites / sizeof *guard_sprites), \
	.types = guard_types, .ntypes = (int)(sizeof guard_types / sizeof *guard_types), \
	.bytes = guard_bytes, .nbytes = (int)(sizeof guard_bytes / sizeof *guard_bytes) }
/* ---- MoonBld: BN5's attack 0x53 (080EF873) as BN6's 0xa3 ---- */

static const XvBlock moonbld_blocks[] = {
	{ 0x08012530, 0x08012704, XC_CODE + 0x4D50, 0x2030EB14u },
	{ 0x080D32AC, 0x080D332C, XC_CODE + 0x4F30, 0x16780E8Fu },
	{ 0x080D3340, 0x080D33B8, XC_CODE + 0x4FB0, 0x11680FB5u },
	{ 0x080E3618, 0x080E36F8, XC_CODE + 0x5030, 0xA30EBD26u },
	{ 0x080EF872, 0x080EFA72, XC_CODE + 0x5112, 0x53AE5D7Cu },
};
static const XvData moonbld_data[] = {
	{ 0x080125B0, 0x080125B8, XV_POOL },
	{ 0x080125B8, 0x080125C0, XV_RAW },
	{ 0x080125C0, 0x080125CC, XV_RAW },
	{ 0x080126C0, 0x080126C8, XV_POOL },
	{ 0x080126C8, 0x080126D0, XV_RAW },
	{ 0x080126D0, 0x080126FC, XV_RAW },
	{ 0x080126FC, 0x08012704, XV_POOL },
	{ 0x080D32BE, 0x080D32C0, XV_RAW },
	{ 0x080D32C0, 0x080D32C4, XV_POOL },
	{ 0x080D32C4, 0x080D32D0, XV_TABLE },
	{ 0x080D3382, 0x080D3384, XV_RAW },
	{ 0x080D3384, 0x080D3388, XV_POOL },
	{ 0x080D3388, 0x080D3398, XV_RAW },
	{ 0x080D33AE, 0x080D33B0, XV_RAW },
	{ 0x080D33B0, 0x080D33B8, XV_POOL },
	{ 0x080E3626, 0x080E3628, XV_RAW },
	{ 0x080E3628, 0x080E362C, XV_POOL },
	{ 0x080E362C, 0x080E3638, XV_TABLE },
	{ 0x080EF880, 0x080EF884, XV_POOL },
	{ 0x080EF884, 0x080EF898, XV_TABLE },
	{ 0x080EF948, 0x080EF950, XV_RAW },
	{ 0x080EF950, 0x080EF958, XV_POOL },
	{ 0x080EFA14, 0x080EFA1C, XV_POOL },
	{ 0x080EFA1C, 0x080EFA20, XV_RAW },
};
static const XvPair moonbld_calls[] = {
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
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x080D332C, 0x080D7538 },   /* map:bytes, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002CBE, 0x08002E52 },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800CA7C, 0x0800E5FC },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800D1C6, 0x0800F2AA },   /* map:bytes, SAME */
	{ 0x0800DA6A, 0x0800FCE4 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0801A514, 0x0801DBD4 },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair moonbld_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm moonbld_imms[] = {
	{ 0x0801261C, THUMB_CMP, 1, 0, 0x8, 0xD },   /* a panel's type compared with BN5's 0x8: BN6's 0xd */
	{ 0x080D32D6, THUMB_MOVS, 2, 0, 0x38, 0xF1 },   /* its sprite: list 4 0x38 in BN5, 0xf1 in BN6 (BN5's copied there) */
};
static const XvNumber moonbld_numbers[] = {
	{ 0x080D33B0, 0x0705FF01, 0x0705FF01 },
	{ 0x080D33B4, 0x01180003, 0x00180003 },
};
static const XvSprite moonbld_sprites[] = {
	{ 4, 0x38, 4, 0xF1, XC_ART + 0x29800, 0x2000 },
};
static const XvType moonbld_types[] = { { 3, 0x69, XV_TYPE_MOVED }, { 4, 0x28, XV_TYPE_MOVED } };

#define MOONBLD_CODE { .name = "MoonBld (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = moonbld_blocks, .nblocks = (int)(sizeof moonbld_blocks / sizeof *moonbld_blocks), \
	.data = moonbld_data, .ndata = (int)(sizeof moonbld_data / sizeof *moonbld_data), \
	.calls = moonbld_calls, .ncalls = (int)(sizeof moonbld_calls / sizeof *moonbld_calls), \
	.words = moonbld_words, .nwords = (int)(sizeof moonbld_words / sizeof *moonbld_words), \
	.imms = moonbld_imms, .nimms = (int)(sizeof moonbld_imms / sizeof *moonbld_imms), \
	.numbers = moonbld_numbers, .nnumbers = (int)(sizeof moonbld_numbers / sizeof *moonbld_numbers), \
	.sprites = moonbld_sprites, .nsprites = (int)(sizeof moonbld_sprites / sizeof *moonbld_sprites), \
	.types = moonbld_types, .ntypes = (int)(sizeof moonbld_types / sizeof *moonbld_types) }
/* ---- CusVolt: BN5's attack 0x27 (080ED28B) as BN6's 0x77 ---- */

static const XvBlock cusvolt_blocks[] = {
	{ 0x080DD4B8, 0x080DD5B4, XC_CODE + 0x5320, 0x62FEA406u },
	{ 0x080ED28A, 0x080ED36E, XC_CODE + 0x5422, 0x80BF1385u },
};
static const XvData cusvolt_data[] = {
	{ 0x080DD4CA, 0x080DD4CC, XV_RAW },
	{ 0x080DD4CC, 0x080DD4D0, XV_POOL },
	{ 0x080DD4D0, 0x080DD4DC, XV_TABLE },
	{ 0x080DD5AC, 0x080DD5B4, XV_POOL },
	{ 0x080ED298, 0x080ED29C, XV_POOL },
	{ 0x080ED29C, 0x080ED2A4, XV_TABLE },
	{ 0x080ED332, 0x080ED338, XV_RAW },
	{ 0x080ED338, 0x080ED33C, XV_POOL },
};
static const XvPair cusvolt_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800B392, 0x0800CCD4 },   /* map:bytes, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08015140, 0x08018810 },   /* map:calls, DIFF (audited) */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08008CA0, 0x0800A2F0 },   /* map:bytes, SAME */
	{ 0x0802B3E0, 0x0802D234 },   /* map:calls+bytes, SAME */
	{ 0x0802B3F2, 0x0802D246 },   /* map:calls+bytes, SAME */
	{ 0x0801A8BC, 0x0801DFD0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair cusvolt_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm cusvolt_imms[] = {
	{ 0x080DD50A, THUMB_LDR, 1, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DD588, THUMB_MOVS, 1, 0, 0x1A, 0x1B },   /* MegaMan's point 0x1a: CusVolt's volt's place (0x080DD58E, at his centre, 18 up, 40 pixels ahead): BN5's 0x1A (0, 18) on every row of his; BN6's 0x1A another (28, 25), no BN6 code asking for it, its 0x1B the centre (2, 18), as every point of his BN6 put 2 pixels further right (0x00 (2, 0) against (0, 0)) */
	{ 0x080ED2F2, THUMB_MOVS, 0, 0, 0xC6, 0x50 },   /* its sound 0xc6: BN5's copied to BN6's slot 0x50 */
};
static const XvNumber cusvolt_numbers[] = {
	{ 0x080DD5AC, 0x00000C40, 0x00000CF6 },
	{ 0x080DD5B0, 0x0A050300, 0x0A050300 },
};
static const XvSprite cusvolt_sprites[] = {
	{ 3, 0x40, 3, 0xF6, XC_ART + 0x2B800, 0x1000 },
};
static const XvType cusvolt_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0xB8, XV_TYPE_MOVED } };
static const XcSong cusvolt_songs[] = { { 0xC6, 0x50 } };

#define CUSVOLT_CODE { .name = "CusVolt (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = cusvolt_blocks, .nblocks = (int)(sizeof cusvolt_blocks / sizeof *cusvolt_blocks), \
	.data = cusvolt_data, .ndata = (int)(sizeof cusvolt_data / sizeof *cusvolt_data), \
	.calls = cusvolt_calls, .ncalls = (int)(sizeof cusvolt_calls / sizeof *cusvolt_calls), \
	.words = cusvolt_words, .nwords = (int)(sizeof cusvolt_words / sizeof *cusvolt_words), \
	.imms = cusvolt_imms, .nimms = (int)(sizeof cusvolt_imms / sizeof *cusvolt_imms), \
	.numbers = cusvolt_numbers, .nnumbers = (int)(sizeof cusvolt_numbers / sizeof *cusvolt_numbers), \
	.sprites = cusvolt_sprites, .nsprites = (int)(sizeof cusvolt_sprites / sizeof *cusvolt_sprites), \
	.types = cusvolt_types, .ntypes = (int)(sizeof cusvolt_types / sizeof *cusvolt_types) }
/* ---- CustSwrd: BN5's attack 0x13 (080EBDC7) as BN6's 0x63 ---- */

static const XvBlock custswrd_blocks[] = {
	{ 0x08012530, 0x08012704, XC_CODE + 0x5510, 0x2030EB14u },
	{ 0x080E3618, 0x080E36F8, XC_CODE + 0x56F0, 0xA30EBD26u },
	{ 0x080EBDC6, 0x080EBEAC, XC_CODE + 0x57D2, 0xB3DC1986u },
	{ 0x080EBEC2, 0x080EC0F4, XC_CODE + 0x58C2, 0xB3695FD3u },
	{ 0x080EC110, 0x080EC234, XC_CODE + 0x5B00, 0xD6930760u },
};
static const XvData custswrd_data[] = {
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
	{ 0x080EBDD4, 0x080EBDD8, XV_POOL },
	{ 0x080EBDD8, 0x080EBDEC, XV_TABLE },
	{ 0x080EBE9C, 0x080EBEA4, XV_RAW },
	{ 0x080EBEA4, 0x080EBEAC, XV_POOL },
	{ 0x080EC04E, 0x080EC050, XV_RAW },
	{ 0x080EC050, 0x080EC05C, XV_POOL },
	{ 0x080EC05C, 0x080EC074, XV_POOL },
	{ 0x080EC074, 0x080EC0A8, XV_RAW },
	{ 0x080EC0A8, 0x080EC0DC, XV_RAW },
	{ 0x080EC0DC, 0x080EC0F4, XV_RAW },
	{ 0x080EC110, 0x080EC120, XV_RAW },
};
static const XvPair custswrd_calls[] = {
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
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x0800D1C6, 0x0800F2AA },   /* map:bytes, SAME */
	{ 0x0800DA6A, 0x0800FCE4 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0801A514, 0x0801DBD4 },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800F114, 0x08011450 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x080EA0BA, 0x08E00C00 },   /* a stub, STUB */
	{ 0x080B9CE0, XN_OBJECTS + 0x284 },   /* the moved objects, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair custswrd_words[] = {
	{ 0x080EBEAD, 0x080ECB8D },
};
static const XvImm custswrd_imms[] = {
	{ 0x0801261C, THUMB_CMP, 1, 0, 0x8, 0xD },   /* a panel's type compared with BN5's 0x8: BN6's 0xd */
};
static const XvNumber custswrd_numbers[] = {
	{ 0x080EC064, 0x0705FF01, 0x0705FF01 },
};
static const XvSprite custswrd_sprites[] = {
	{ 3, 0x15, 3, 0xF5, XC_ART + 0x2C800, 0x1800 },
};
static const XvId custswrd_effects[] = { { 0x28, 0xF8 } };
static const XvByte custswrd_bytes[] = {
	{ 0x080EC117, 0x28, 0xF8 },   /* 080EC110's row 7: its effect 0x28 in BN5, 0xf8 in BN6 */
};
static const XvType custswrd_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 4, 0x28, XV_TYPE_MOVED } };

#define CUSTSWRD_CODE { .name = "CustSwrd (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = custswrd_blocks, .nblocks = (int)(sizeof custswrd_blocks / sizeof *custswrd_blocks), \
	.data = custswrd_data, .ndata = (int)(sizeof custswrd_data / sizeof *custswrd_data), \
	.calls = custswrd_calls, .ncalls = (int)(sizeof custswrd_calls / sizeof *custswrd_calls), \
	.words = custswrd_words, .nwords = (int)(sizeof custswrd_words / sizeof *custswrd_words), \
	.imms = custswrd_imms, .nimms = (int)(sizeof custswrd_imms / sizeof *custswrd_imms), \
	.numbers = custswrd_numbers, .nnumbers = (int)(sizeof custswrd_numbers / sizeof *custswrd_numbers), \
	.sprites = custswrd_sprites, .nsprites = (int)(sizeof custswrd_sprites / sizeof *custswrd_sprites), \
	.effects = custswrd_effects, .neffects = (int)(sizeof custswrd_effects / sizeof *custswrd_effects), \
	.types = custswrd_types, .ntypes = (int)(sizeof custswrd_types / sizeof *custswrd_types), \
	.bytes = custswrd_bytes, .nbytes = (int)(sizeof custswrd_bytes / sizeof *custswrd_bytes) }
/* ---- Anti: BN5's support 0xc (080E380B) as BN6's 0x36 ---- */

static const XvBlock anti_blocks[] = {
	{ 0x080093A2, 0x080093C6, XC_CODE + 0x5C32, 0xE16DEFCEu },
	{ 0x080CF678, 0x080CF804, XC_CODE + 0x5C60, 0x197E34B7u },
	{ 0x080E32EC, 0x080E3340, XC_CODE + 0x5DF0, 0x5A064D24u },
	{ 0x080E378C, 0x080E3872, XC_CODE + 0x5E50, 0x3737D999u },
	{ 0x080E3878, 0x080E39FC, XC_CODE + 0x5F40, 0x65D8045Fu },
};
static const XvData anti_data[] = {
	{ 0x080CF686, 0x080CF688, XV_RAW },
	{ 0x080CF688, 0x080CF68C, XV_POOL },
	{ 0x080CF68C, 0x080CF698, XV_TABLE },
	{ 0x080CF720, 0x080CF724, XV_POOL },
	{ 0x080CF724, 0x080CF72C, XV_TABLE },
	{ 0x080CF7C2, 0x080CF7C4, XV_RAW },
	{ 0x080CF7C4, 0x080CF7C8, XV_POOL },
	{ 0x080CF7C8, 0x080CF7D8, XV_RAW },
	{ 0x080CF800, 0x080CF804, XV_POOL },
	{ 0x080E3338, 0x080E3340, XV_POOL },
	{ 0x080E379A, 0x080E379C, XV_RAW },
	{ 0x080E379C, 0x080E37A0, XV_POOL },
	{ 0x080E37A0, 0x080E37AC, XV_TABLE },
	{ 0x080E37BA, 0x080E37BC, XV_RAW },
	{ 0x080E37BC, 0x080E37C0, XV_POOL },
	{ 0x080E37C0, 0x080E37D0, XV_TABLE },
	{ 0x080E383C, 0x080E3840, XV_POOL },
	{ 0x080E3840, 0x080E3860, XV_TABLE },
	{ 0x080E3886, 0x080E3888, XV_RAW },
	{ 0x080E3888, 0x080E388C, XV_POOL },
	{ 0x080E388C, 0x080E3898, XV_TABLE },
	{ 0x080E38A6, 0x080E38A8, XV_RAW },
	{ 0x080E38A8, 0x080E38AC, XV_POOL },
	{ 0x080E38AC, 0x080E38BC, XV_TABLE },
	{ 0x080E39A2, 0x080E39A4, XV_RAW },
	{ 0x080E39A4, 0x080E39B8, XV_POOL },
	{ 0x080E39B8, 0x080E39C0, XV_RAW },
	{ 0x080E39C0, 0x080E39E8, XV_RAW },
	{ 0x080E39E8, 0x080E39F2, XV_RAW },
	{ 0x080E39F2, 0x080E39FC, XV_RAW },
};
static const XvPair anti_calls[] = {
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08029A16, 0x0802CEA6 },   /* map:bytes, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016162, 0x080198CE },   /* map:calls, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800A50A, 0x0800BF16 },   /* map:bytes, SAME */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x080F625A, 0x0800D3FE },   /* map:bytes, SAME */
	{ 0x080299FA, 0x0802CE8A },   /* map:bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
};
static const XvPair anti_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A211, 0x0800BBA9 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x0801631C, 0x08019C34 },
	{ 0x080E3873, 0x080E48DF },
};
static const XvImm anti_imms[] = {
	{ 0x080093B8, THUMB_ADDS, 4, 0, 0x2B, 0xF7 },   /* its effect: 0x2b in BN5, 0xf7 in BN6 */
	{ 0x080CF6CC, THUMB_ADDS, 0, 0, 0x7C, 0x84 },   /* collision data +0x7c in BN5, +0x84 in BN6: the Anti chips' trap (the third kind's 0x4D): the struck's damage by element, a halfword each from Fire on, BN5's +0x7C + 2 x its kind (0x080CF6C8-0x080CF6D2), BN6's from +0x84 (its own trap's 0x080CF844, from Fire) */
	{ 0x080E3920, THUMB_LDR, 1, 0, 0x18, 0x14 },   /* a panel's +0x18 in BN5, +0x14 in BN6 (legit.json, layouts, panel) */
};
static const XvSprite anti_sprites[] = {
	{ 5, 0x05, 5, 0xFE, XC_ART + 0x2E000, 0x5800 },
};
static const XvId anti_effects[] = { { 0x2B, 0xF7 } };
static const XvByte anti_bytes[] = {
	{ 0x080E39EE, 0x2E, 0x52 },   /* 080E39E8's row 3: its sound 0x12e, BN5's copied to BN6's slot 0x52 */
	{ 0x080E39EF, 0x01, 0x00 },   /* 080E39E8's row 3: its sound 0x12e, BN5's copied to BN6's slot 0x52 (its high byte) */
};
static const XvType anti_types[] = { { 3, 0x4D, XV_TYPE_MOVED }, { 4, 0x24, 0x24 }, { 4, 0x2A, XV_TYPE_MOVED }, { 4, 0x2B, XV_TYPE_MOVED } };
static const XcSong anti_songs[] = { { 0x12E, 0x52 } };

#define ANTI_CODE { .name = "Anti (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = anti_blocks, .nblocks = (int)(sizeof anti_blocks / sizeof *anti_blocks), \
	.data = anti_data, .ndata = (int)(sizeof anti_data / sizeof *anti_data), \
	.calls = anti_calls, .ncalls = (int)(sizeof anti_calls / sizeof *anti_calls), \
	.words = anti_words, .nwords = (int)(sizeof anti_words / sizeof *anti_words), \
	.imms = anti_imms, .nimms = (int)(sizeof anti_imms / sizeof *anti_imms), \
	.sprites = anti_sprites, .nsprites = (int)(sizeof anti_sprites / sizeof *anti_sprites), \
	.effects = anti_effects, .neffects = (int)(sizeof anti_effects / sizeof *anti_effects), \
	.types = anti_types, .ntypes = (int)(sizeof anti_types / sizeof *anti_types), \
	.bytes = anti_bytes, .nbytes = (int)(sizeof anti_bytes / sizeof *anti_bytes) }
/* ---- Reaction49: BN5's attack 0x49 (080EF271) as BN6's 0x99 ---- */

static const XvBlock reaction49_blocks[] = {
	{ 0x080BA804, 0x080BA978, XC_CODE + 0x60D0, 0x0061F28Du },
	{ 0x080EF270, 0x080EF2CC, XC_CODE + 0x6250, 0x5F8B19D4u },
};
static const XvData reaction49_data[] = {
	{ 0x080BA816, 0x080BA818, XV_RAW },
	{ 0x080BA818, 0x080BA81C, XV_POOL },
	{ 0x080BA81C, 0x080BA828, XV_TABLE },
	{ 0x080BA8B6, 0x080BA8B8, XV_RAW },
	{ 0x080BA8B8, 0x080BA8BC, XV_POOL },
	{ 0x080BA8BC, 0x080BA8C8, XV_TABLE },
	{ 0x080BA974, 0x080BA978, XV_POOL },
	{ 0x080EF2BE, 0x080EF2C0, XV_RAW },
	{ 0x080EF2C0, 0x080EF2CC, XV_POOL },
};
static const XvPair reaction49_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800D1B8, 0x0800F29C },   /* map:bytes, SAME */
	{ 0x08010D64, 0x080136CC },   /* map:calls+bytes, SAME */
	{ 0x0800DA32, 0x0800FC9E },   /* map:calls, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x0800DE7A, 0x080100D6 },   /* near, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800ED06, 0x08010DF6 },   /* map:bytes, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x0800EEB6, 0x08011044 },   /* map:bytes, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x0800E0A0, 0x0801031C },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800DD4E, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x0801742A, 0x0801A66C },   /* project:chips, LAYOUT */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair reaction49_words[] = {
	{ 0x08000000, 0x08000000 },
	{ 0x0800325D, 0x08003459 },
};
static const XvImm reaction49_imms[] = {
	{ 0x080EF2A2, THUMB_MOVS, 0, 0, 0x8E, 0x4C },   /* its sound 0x8e: BN5's copied to BN6's slot 0x4c */
};
static const XvType reaction49_types[] = { { 1, 0x0A, XV_TYPE_MOVED } };
static const XcSong reaction49_songs[] = { { 0x8E, 0x4C } };

#define REACTION49_CODE { .name = "Reaction49 (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = reaction49_blocks, .nblocks = (int)(sizeof reaction49_blocks / sizeof *reaction49_blocks), \
	.data = reaction49_data, .ndata = (int)(sizeof reaction49_data / sizeof *reaction49_data), \
	.calls = reaction49_calls, .ncalls = (int)(sizeof reaction49_calls / sizeof *reaction49_calls), \
	.words = reaction49_words, .nwords = (int)(sizeof reaction49_words / sizeof *reaction49_words), \
	.imms = reaction49_imms, .nimms = (int)(sizeof reaction49_imms / sizeof *reaction49_imms), \
	.types = reaction49_types, .ntypes = (int)(sizeof reaction49_types / sizeof *reaction49_types) }
/* ---- Slasher: BN5's attack 0x29 (080ED411) as BN6's 0x79 ---- */

static const XvBlock slasher_blocks[] = {
	{ 0x080EC120, 0x080EC1AC, XC_CODE + 0x62B0, 0x2FBBA38Cu },
	{ 0x080ED410, 0x080ED58A, XC_CODE + 0x6340, 0x7982E3D7u },
};
static const XvData slasher_data[] = {
	{ 0x080ED448, 0x080ED454, XV_POOL },
	{ 0x080ED454, 0x080ED464, XV_TABLE },
	{ 0x080ED4B2, 0x080ED4B4, XV_RAW },
	{ 0x080ED4B4, 0x080ED4B8, XV_POOL },
	{ 0x080ED55A, 0x080ED55C, XV_RAW },
	{ 0x080ED55C, 0x080ED560, XV_POOL },
};
static const XvPair slasher_calls[] = {
	{ 0x08010DEE, 0x08013774 },   /* map:calls+bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800CD92, 0x0800EB08 },   /* map:calls, LAYOUT */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800FEBC, 0x0801273E },   /* map:bytes, SAME */
	{ 0x0800CD84, 0x0800EAFA },   /* map:calls, LAYOUT */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x080091EA, 0x0800A9EC },   /* map:bytes, SAME */
	{ 0x0801B3AC, 0x0801EB18 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
};
static const XvImm slasher_imms[] = {
	{ 0x080ED43C, THUMB_MOVS, 0, 0, 0x49, 0x99 },   /* setAttack0 with BN5's family 0x49: its unit's BN6 number 0x99 */
};
static const XvNumber slasher_numbers[] = {
	{ 0x080ED55C, 0x0705FF04, 0x0705FF04 },
};
static const XvType slasher_types[] = { { 1, 0x05, XV_TYPE_SHARED } };

#define SLASHER_CODE { .name = "Slasher (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = slasher_blocks, .nblocks = (int)(sizeof slasher_blocks / sizeof *slasher_blocks), \
	.data = slasher_data, .ndata = (int)(sizeof slasher_data / sizeof *slasher_data), \
	.calls = slasher_calls, .ncalls = (int)(sizeof slasher_calls / sizeof *slasher_calls), \
	.imms = slasher_imms, .nimms = (int)(sizeof slasher_imms / sizeof *slasher_imms), \
	.numbers = slasher_numbers, .nnumbers = (int)(sizeof slasher_numbers / sizeof *slasher_numbers), \
	.types = slasher_types, .ntypes = (int)(sizeof slasher_types / sizeof *slasher_types) }
/* ---- ElemRage: BN5's attack 0x56 (080EFDB9) as BN6's 0xa6 ---- */

static const XvBlock elemrage_blocks[] = {
	{ 0x080D9AC0, 0x080D9D02, XC_CODE + 0x64C0, 0x08A9CF90u },
	{ 0x080E2A34, 0x080E2CEA, XC_CODE + 0x6710, 0xD1828CADu },
	{ 0x080E2DCE, 0x080E2DEA, XC_CODE + 0x69D2, 0x1C4F15C8u },
	{ 0x080EFDB8, 0x080EFEE4, XC_CODE + 0x69F0, 0x7D3AEAF6u },
};
static const XvData elemrage_data[] = {
	{ 0x080D9ADE, 0x080D9AE0, XV_RAW },
	{ 0x080D9AE0, 0x080D9AE4, XV_POOL },
	{ 0x080D9AE4, 0x080D9AF0, XV_TABLE },
	{ 0x080D9C2A, 0x080D9C2C, XV_RAW },
	{ 0x080D9C2C, 0x080D9C30, XV_POOL },
	{ 0x080D9C30, 0x080D9C38, XV_TABLE },
	{ 0x080E2A34, 0x080E2BC4, XV_RAW },
	{ 0x080E2BD2, 0x080E2BD4, XV_RAW },
	{ 0x080E2BD4, 0x080E2BD8, XV_POOL },
	{ 0x080E2BD8, 0x080E2BE4, XV_TABLE },
	{ 0x080E2C18, 0x080E2C20, XV_POOL },
	{ 0x080E2C20, 0x080E2C30, XV_TABLE },
	{ 0x080EFDC6, 0x080EFDC8, XV_RAW },
	{ 0x080EFDC8, 0x080EFDCC, XV_POOL },
	{ 0x080EFDCC, 0x080EFDD4, XV_TABLE },
	{ 0x080EFEC6, 0x080EFEC8, XV_RAW },
	{ 0x080EFEC8, 0x080EFECC, XV_POOL },
	{ 0x080EFECC, 0x080EFEE4, XV_RAW },
};
static const XvPair elemrage_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C3A, 0x0801A146 },   /* project:families, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x0801662E, 0x08019F86 },   /* map:bytes, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0800B314, 0x0800CC52 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800D188, 0x0800F26C },   /* map:bytes, SAME */
	{ 0x08015164, 0x08E00C10 },   /* a stub, STUB */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
};
static const XvPair elemrage_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080E2CEB, 0x080E3D27 },
	{ 0x080E2CFD, 0x080E3D39 },
	{ 0x080E2D35, 0x080E3D71 },
	{ 0x080E2D69, 0x080E3DA5 },
	{ 0x080E2D9F, 0x080E3DDB },
	{ 0x080EFEE5, 0x080EF5B5 },
};
static const XvImm elemrage_imms[] = {
	{ 0x080D9B06, THUMB_MOVS, 2, 0, 0xE, 0xF4 },   /* its sprite: list 3 0xe in BN5, 0xf4 in BN6 (BN5's copied there) */
	{ 0x080D9BBC, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite elemrage_sprites[] = {
	{ 3, 0x0E, 3, 0xF4, XC_ART + 0x33800, 0x800 },
};
static const XvByte elemrage_bytes[] = {
	{ 0x080E2A48, 0x08, 0x0D },   /* 080E2A34's row 1: a panel set to BN5's type 0x8: BN6's 0xd */
	{ 0x080E2A98, 0x0A, 0x0E },   /* 080E2A34's row 5: a panel set to BN5's type 0xa: BN6's 0xe */
	{ 0x080E2AAC, 0x09, 0x05 },   /* 080E2A34's row 6: a panel set to BN5's type 0x9: BN6's 0x5 */
	{ 0x080E2AC0, 0x09, 0x05 },   /* 080E2A34's row 7: a panel set to BN5's type 0x9: BN6's 0x5 */
	{ 0x080EFECC, 0x08, 0x0D },   /* ElemRage's element by MegaMan's panel (0x080EFEA2: its type against each row's till 0xFF): row 0, BN5's type 0x8 BN6's 0xd */
	{ 0x080EFED0, 0x0A, 0x0E },   /* ElemRage's element by MegaMan's panel (0x080EFEA2: its type against each row's till 0xFF): row 1, BN5's type 0xa BN6's 0xe */
	{ 0x080EFEDC, 0x05, 0xFE },   /* ElemRage's element by MegaMan's panel (0x080EFEA2: its type against each row's till 0xFF): row 4, BN5's type 0x5 (the magnet panel), which BN6 lacks: none */
};
static const XvType elemrage_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0x98, XV_TYPE_MOVED }, { 4, 0x1F, XV_TYPE_MOVED } };

#define ELEMRAGE_CODE { .name = "ElemRage (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = elemrage_blocks, .nblocks = (int)(sizeof elemrage_blocks / sizeof *elemrage_blocks), \
	.data = elemrage_data, .ndata = (int)(sizeof elemrage_data / sizeof *elemrage_data), \
	.calls = elemrage_calls, .ncalls = (int)(sizeof elemrage_calls / sizeof *elemrage_calls), \
	.words = elemrage_words, .nwords = (int)(sizeof elemrage_words / sizeof *elemrage_words), \
	.imms = elemrage_imms, .nimms = (int)(sizeof elemrage_imms / sizeof *elemrage_imms), \
	.sprites = elemrage_sprites, .nsprites = (int)(sizeof elemrage_sprites / sizeof *elemrage_sprites), \
	.types = elemrage_types, .ntypes = (int)(sizeof elemrage_types / sizeof *elemrage_types), \
	.bytes = elemrage_bytes, .nbytes = (int)(sizeof elemrage_bytes / sizeof *elemrage_bytes) }
/* ---- Bombs: BN5's attack 0x12 (080EBC91) as BN6's 0x62 ---- */

static const XvBlock bombs_blocks[] = {
	{ 0x080C928C, 0x080C9518, XC_CODE + 0x6B20, 0xFF0FF08Au },
	{ 0x080CB6B0, 0x080CB8E0, XC_CODE + 0x6DB0, 0xFFA9A750u },
	{ 0x080CDA78, 0x080CDCBC, XC_CODE + 0x6FE0, 0x4B4968FEu },
	{ 0x080CDCD2, 0x080CDDDE, XC_CODE + 0x7232, 0xBD548CF1u },
	{ 0x080CECEC, 0x080CEFCC, XC_CODE + 0x7340, 0x2A4F2B31u },
	{ 0x080CFEF4, 0x080D0190, XC_CODE + 0x7620, 0x11D067B4u },
	{ 0x080EBC90, 0x080EBDA8, XC_CODE + 0x78C0, 0xDC94F556u },
};
static const XvData bombs_data[] = {
	{ 0x080C929E, 0x080C92A0, XV_RAW },
	{ 0x080C92A0, 0x080C92A4, XV_POOL },
	{ 0x080C92A4, 0x080C92B0, XV_TABLE },
	{ 0x080C931A, 0x080C931C, XV_RAW },
	{ 0x080C931C, 0x080C9320, XV_POOL },
	{ 0x080C9320, 0x080C9328, XV_TABLE },
	{ 0x080C940E, 0x080C9410, XV_RAW },
	{ 0x080C9410, 0x080C9414, XV_POOL },
	{ 0x080C9414, 0x080C9418, XV_RAW },
	{ 0x080C94A8, 0x080C94B0, XV_POOL },
	{ 0x080C94B0, 0x080C94D0, XV_RAW },
	{ 0x080C94D0, 0x080C94D4, XV_RAW },
	{ 0x080C950E, 0x080C9510, XV_RAW },
	{ 0x080C9510, 0x080C9518, XV_POOL },
	{ 0x080CB6C2, 0x080CB6C4, XV_RAW },
	{ 0x080CB6C4, 0x080CB6C8, XV_POOL },
	{ 0x080CB6C8, 0x080CB6D4, XV_TABLE },
	{ 0x080CB780, 0x080CB78C, XV_RAW },
	{ 0x080CB796, 0x080CB798, XV_RAW },
	{ 0x080CB798, 0x080CB79C, XV_POOL },
	{ 0x080CB79C, 0x080CB7A4, XV_TABLE },
	{ 0x080CB882, 0x080CB884, XV_RAW },
	{ 0x080CB884, 0x080CB888, XV_POOL },
	{ 0x080CB888, 0x080CB88C, XV_RAW },
	{ 0x080CB8DC, 0x080CB8E0, XV_POOL },
	{ 0x080CDA78, 0x080CDA7A, XV_RAW },
	{ 0x080CDA8C, 0x080CDA90, XV_POOL },
	{ 0x080CDA90, 0x080CDA9C, XV_TABLE },
	{ 0x080CDB38, 0x080CDB44, XV_POOL },
	{ 0x080CDB80, 0x080CDB84, XV_POOL },
	{ 0x080CDB84, 0x080CDB94, XV_TABLE },
	{ 0x080CDC5A, 0x080CDC5C, XV_RAW },
	{ 0x080CDC5C, 0x080CDC64, XV_POOL },
	{ 0x080CDC98, 0x080CDC9C, XV_RAW },
	{ 0x080CDD12, 0x080CDD14, XV_RAW },
	{ 0x080CDDA6, 0x080CDDA8, XV_RAW },
	{ 0x080CDDA8, 0x080CDDB0, XV_POOL },
	{ 0x080CDDB0, 0x080CDDC0, XV_RAW },
	{ 0x080CECFE, 0x080CED00, XV_RAW },
	{ 0x080CED00, 0x080CED04, XV_POOL },
	{ 0x080CED04, 0x080CED10, XV_TABLE },
	{ 0x080CED44, 0x080CED48, XV_POOL },
	{ 0x080CED48, 0x080CED4C, XV_RAW },
	{ 0x080CED92, 0x080CED94, XV_RAW },
	{ 0x080CED94, 0x080CED98, XV_POOL },
	{ 0x080CED98, 0x080CEDA0, XV_TABLE },
	{ 0x080CEE42, 0x080CEE44, XV_RAW },
	{ 0x080CEE44, 0x080CEE5C, XV_RAW },
	{ 0x080CEEA4, 0x080CEEB0, XV_POOL },
	{ 0x080CEEB0, 0x080CEEB8, XV_RAW },
	{ 0x080CEEB8, 0x080CEEBC, XV_RAW },
	{ 0x080CEEBC, 0x080CEEC0, XV_RAW },
	{ 0x080CEFB4, 0x080CEFCC, XV_POOL },
	{ 0x080CFF02, 0x080CFF04, XV_RAW },
	{ 0x080CFF04, 0x080CFF08, XV_POOL },
	{ 0x080CFF08, 0x080CFF14, XV_TABLE },
	{ 0x080CFFA6, 0x080CFFA8, XV_RAW },
	{ 0x080CFFA8, 0x080CFFAC, XV_POOL },
	{ 0x080CFFAC, 0x080CFFC0, XV_TABLE },
	{ 0x080D00C8, 0x080D00CC, XV_POOL },
	{ 0x080D00CC, 0x080D00D0, XV_RAW },
	{ 0x080D0144, 0x080D0148, XV_POOL },
	{ 0x080D0148, 0x080D014C, XV_RAW },
	{ 0x080D0182, 0x080D0184, XV_RAW },
	{ 0x080D0184, 0x080D0190, XV_POOL },
	{ 0x080EBC9E, 0x080EBCA0, XV_RAW },
	{ 0x080EBCA0, 0x080EBCA4, XV_POOL },
	{ 0x080EBCA4, 0x080EBCAC, XV_TABLE },
	{ 0x080EBD42, 0x080EBD44, XV_RAW },
	{ 0x080EBD44, 0x080EBD4C, XV_POOL },
	{ 0x080EBD4C, 0x080EBD88, XV_TABLE },
	{ 0x080EBD88, 0x080EBDA8, XV_RAW },
};
static const XvPair bombs_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x080EA0BA, 0x08E00C00 },   /* a stub, STUB */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08018838, 0x0801BD3C },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800AFF0, 0x0800C938 },   /* map:calls, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B144, 0x0800CA8C },   /* map:calls, DIFF (audited) */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08001118, 0x0800117C },   /* map:bytes, SAME */
	{ 0x0800113C, 0x080011A0 },   /* map:bytes, SAME */
	{ 0x080011AA, 0x0800120E },   /* map:bytes, SAME */
	{ 0x0800C64C, 0x0800E24C },   /* map:bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x0800C6C2, 0x0800E2C2 },   /* map:calls+bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x080EA08C, 0x08E00C00 },   /* a stub, STUB */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
};
static const XvPair bombs_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x0801629C, 0x08019B78 },
	{ 0x0801631C, 0x08019C34 },
	{ 0x080C57C5, 0x08E00C05 },
	{ 0x080CA48B, 0x08E00C05 },
	{ 0x080CDCBD, 0x080CB535 },
	{ 0x080CE7EF, 0x08E00C05 },
	{ 0x080D5199, 0x08E00C05 },
	{ 0x080D5A0F, 0x08E00C05 },
	{ 0x080EBDA9, 0x080ECA99 },
};
static const XvImm bombs_imms[] = {
	{ 0x080C92B4, THUMB_MOVS, 2, 0, 0x2, 0xF3 },   /* its sprite: list 3 0x2 in BN5, 0xf3 in BN6 (BN5's copied there) */
	{ 0x080CB6D8, THUMB_MOVS, 2, 0, 0x2, 0xF3 },   /* its sprite: list 3 0x2 in BN5, 0xf3 in BN6 (BN5's copied there) */
	{ 0x080CB756, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CB82E, THUMB_MOVS, 0, 0, 0x70, 0x5F },   /* its sound 0x70: BN5's copied to BN6's slot 0x5f */
	{ 0x080CDB66, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CEE66, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CFF22, THUMB_MOVS, 2, 0, 0x2, 0xF3 },   /* its sprite: list 3 0x2 in BN5, 0xf3 in BN6 (BN5's copied there) */
	{ 0x080D0088, THUMB_MOVS, 2, 0, 0x2, 0xFD },   /* its sprite: list 1 0x2 in BN5, 0xfd in BN6 (BN5's copied there) */
	{ 0x080D00FE, THUMB_MOVS, 0, 0, 0xC0, 0x62 },   /* its sound 0xc0: BN5's copied to BN6's slot 0x62 */
};
static const XvNumber bombs_numbers[] = {
	{ 0x080C9514, 0x0A05FF04, 0x0A05FF04 },
	{ 0x080CDC60, 0x04050201, 0x04050201 },
	{ 0x080CDDAC, 0x04050201, 0x04050201 },
	{ 0x080CEFB4, 0x01090C02, 0x01090CF3 },
	{ 0x080CEFB8, 0x04050000, 0x04050000 },
};
static const XvSprite bombs_sprites[] = {
	{ 1, 0x02, 1, 0xFD, XC_ART + 0x34000, 0x2000 },
	{ 5, 0x18, 5, 0xFD, XC_ART + 0x36000, 0x1000 },
	{ 3, 0x13, 3, 0xF2, XC_ART + 0x37000, 0x1000 },
};
static const XvId bombs_effects[] = { { 0x50, 0xF6 }, { 0x51, 0xF5 }, { 0x52, 0xF4 }, { 0x53, 0xF3 }, { 0x55, 0xF2 } };
static const XvByte bombs_bytes[] = {
	{ 0x080CDA79, 0x02, 0xF3 },   /* 080CDA78's row 0: its sprite list 3 0x2 in BN5, 0xf3 in BN6 (BN5's copied there) */
	{ 0x080C94D0, 0x50, 0xF6 },   /* 080C94D0's row 0: its effect 0x50 in BN5, 0xf6 in BN6 */
	{ 0x080C94D1, 0x51, 0xF5 },   /* 080C94D0's row 1: its effect 0x51 in BN5, 0xf5 in BN6 */
	{ 0x080C94D2, 0x52, 0xF4 },   /* 080C94D0's row 2: its effect 0x52 in BN5, 0xf4 in BN6 */
	{ 0x080C94D3, 0x53, 0xF3 },   /* 080C94D0's row 3: its effect 0x53 in BN5, 0xf3 in BN6 */
	{ 0x080CEEBC, 0x29, 0x3B },   /* 080CEEBC's row 0: its effect 0x29 in BN5, 0x3b in BN6 */
	{ 0x080CEEBD, 0x29, 0x3B },   /* 080CEEBC's row 1: its effect 0x29 in BN5, 0x3b in BN6 */
	{ 0x080CEEBE, 0x54, 0x3C },   /* 080CEEBC's row 2: its effect 0x54 in BN5, 0x3c in BN6 */
	{ 0x080CEEBF, 0x55, 0xF2 },   /* 080CEEBC's row 3: its effect 0x55 in BN5, 0xf2 in BN6 */
	{ 0x080CEEB9, 0x0A, 0x0E },   /* 080CEEB8's row 1: a panel set to BN5's type 0xa: BN6's 0xe */
	{ 0x080CEEBB, 0x08, 0x0D },   /* 080CEEB8's row 3: a panel set to BN5's type 0x8: BN6's 0xd */
	{ 0x080CEEB2, 0xE1, 0x5B },   /* 080CEEB0's row 1: its sound 0xe1, BN5's copied to BN6's slot 0x5b */
	{ 0x080C9414, 0x70, 0x5F },   /* 080C9414's row 0: its sound 0x70, BN5's copied to BN6's slot 0x5f */
};
static const XvType bombs_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0x24, XV_TYPE_MOVED }, { 3, 0x35, XV_TYPE_MOVED }, { 3, 0x42, XV_TYPE_MOVED }, { 3, 0x43, XV_TYPE_MOVED }, { 3, 0x4A, XV_TYPE_MOVED }, { 3, 0x51, XV_TYPE_MOVED } };
static const XcSong bombs_songs[] = { { 0x70, 0x5F }, { 0xC0, 0x62 }, { 0xE1, 0x5B }, { 0x70, 0x5F } };

#define BOMBS_CODE { .name = "Bombs (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = bombs_blocks, .nblocks = (int)(sizeof bombs_blocks / sizeof *bombs_blocks), \
	.data = bombs_data, .ndata = (int)(sizeof bombs_data / sizeof *bombs_data), \
	.calls = bombs_calls, .ncalls = (int)(sizeof bombs_calls / sizeof *bombs_calls), \
	.words = bombs_words, .nwords = (int)(sizeof bombs_words / sizeof *bombs_words), \
	.imms = bombs_imms, .nimms = (int)(sizeof bombs_imms / sizeof *bombs_imms), \
	.numbers = bombs_numbers, .nnumbers = (int)(sizeof bombs_numbers / sizeof *bombs_numbers), \
	.sprites = bombs_sprites, .nsprites = (int)(sizeof bombs_sprites / sizeof *bombs_sprites), \
	.effects = bombs_effects, .neffects = (int)(sizeof bombs_effects / sizeof *bombs_effects), \
	.types = bombs_types, .ntypes = (int)(sizeof bombs_types / sizeof *bombs_types), \
	.bytes = bombs_bytes, .nbytes = (int)(sizeof bombs_bytes / sizeof *bombs_bytes) }
/* ---- Misc: BN5's attack 0x1a (080EC7DF) as BN6's 0x6a ---- */

static const XvBlock misc_blocks[] = {
	{ 0x080127E0, 0x080127FA, XC_CODE + 0x79E0, 0x46E9AEDFu },
	{ 0x080C5386, 0x080C53DE, XC_CODE + 0x7A02, 0x4A1B50AFu },
	{ 0x080D0C0C, 0x080D0D20, XC_CODE + 0x7A60, 0xCF20A9B0u },
	{ 0x080D0E2C, 0x080D0E4A, XC_CODE + 0x7B80, 0x293B728Bu },
	{ 0x080D1404, 0x080D14C6, XC_CODE + 0x7BA0, 0x1D76E83Au },
	{ 0x080D1508, 0x080D151E, XC_CODE + 0x7C70, 0xA68513DEu },
	{ 0x080D8014, 0x080D8430, XC_CODE + 0x7C90, 0xADA72B75u },
	{ 0x080D87D0, 0x080D8A44, XC_CODE + 0x80B0, 0xFEF2C880u },
	{ 0x080D8BD8, 0x080D8CD8, XC_CODE + 0x8330, 0x86E15D59u },
	{ 0x080DAEA4, 0x080DB0AC, XC_CODE + 0x8430, 0xEBC4ABA5u },
	{ 0x080DC6F4, 0x080DC968, XC_CODE + 0x8640, 0xB6CD8D83u },
	{ 0x080E5970, 0x080E5A52, XC_CODE + 0x88C0, 0x6CBB2A8Fu },
	{ 0x080E5A9E, 0x080E5B28, XC_CODE + 0x89B2, 0x2E153CAAu },
	{ 0x080E7DC8, 0x080E80EE, XC_CODE + 0x8A40, 0x1BF22DEBu },
	{ 0x080EC7DE, 0x080EC884, XC_CODE + 0x8D72, 0x92C75B5Du },
};
static const XvData misc_data[] = {
	{ 0x080D0C0C, 0x080D0C30, XV_RAW },
	{ 0x080D0C3E, 0x080D0C40, XV_RAW },
	{ 0x080D0C40, 0x080D0C44, XV_POOL },
	{ 0x080D0C44, 0x080D0C50, XV_TABLE },
	{ 0x080D0D12, 0x080D0D14, XV_RAW },
	{ 0x080D0D14, 0x080D0D20, XV_POOL },
	{ 0x080D1416, 0x080D1418, XV_RAW },
	{ 0x080D1418, 0x080D141C, XV_POOL },
	{ 0x080D141C, 0x080D1428, XV_TABLE },
	{ 0x080D147E, 0x080D1480, XV_RAW },
	{ 0x080D1480, 0x080D1484, XV_POOL },
	{ 0x080D1484, 0x080D148C, XV_TABLE },
	{ 0x080D8026, 0x080D8028, XV_RAW },
	{ 0x080D8028, 0x080D802C, XV_POOL },
	{ 0x080D802C, 0x080D8038, XV_TABLE },
	{ 0x080D80EA, 0x080D80EC, XV_RAW },
	{ 0x080D80EC, 0x080D80F0, XV_POOL },
	{ 0x080D80F0, 0x080D80FC, XV_TABLE },
	{ 0x080D81EE, 0x080D81F0, XV_RAW },
	{ 0x080D81F0, 0x080D81F4, XV_POOL },
	{ 0x080D81F4, 0x080D8200, XV_TABLE },
	{ 0x080D8364, 0x080D8368, XV_POOL },
	{ 0x080D8368, 0x080D8374, XV_RAW },
	{ 0x080D83F6, 0x080D83F8, XV_RAW },
	{ 0x080D83F8, 0x080D83FC, XV_POOL },
	{ 0x080D83FC, 0x080D8418, XV_RAW },
	{ 0x080D8418, 0x080D8430, XV_POOL },
	{ 0x080D87E2, 0x080D87E4, XV_RAW },
	{ 0x080D87E4, 0x080D87E8, XV_POOL },
	{ 0x080D87E8, 0x080D87F4, XV_TABLE },
	{ 0x080D88A6, 0x080D88A8, XV_RAW },
	{ 0x080D88A8, 0x080D88AC, XV_POOL },
	{ 0x080D88AC, 0x080D88BC, XV_TABLE },
	{ 0x080D8A28, 0x080D8A2C, XV_POOL },
	{ 0x080D8A2C, 0x080D8A34, XV_RAW },
	{ 0x080D8A34, 0x080D8A44, XV_POOL },
	{ 0x080D8BEA, 0x080D8BEC, XV_RAW },
	{ 0x080D8BEC, 0x080D8BF0, XV_POOL },
	{ 0x080D8BF0, 0x080D8BFC, XV_TABLE },
	{ 0x080D8CC4, 0x080D8CC8, XV_POOL },
	{ 0x080D8CC8, 0x080D8CD0, XV_RAW },
	{ 0x080D8CD0, 0x080D8CD8, XV_POOL },
	{ 0x080DAEB6, 0x080DAEB8, XV_RAW },
	{ 0x080DAEB8, 0x080DAEBC, XV_POOL },
	{ 0x080DAEBC, 0x080DAEC8, XV_TABLE },
	{ 0x080DAF6E, 0x080DAF70, XV_RAW },
	{ 0x080DAF70, 0x080DAF74, XV_POOL },
	{ 0x080DAF74, 0x080DAF80, XV_TABLE },
	{ 0x080DB082, 0x080DB084, XV_RAW },
	{ 0x080DB084, 0x080DB088, XV_POOL },
	{ 0x080DB088, 0x080DB0A0, XV_RAW },
	{ 0x080DB0A0, 0x080DB0AC, XV_POOL },
	{ 0x080DC706, 0x080DC708, XV_RAW },
	{ 0x080DC708, 0x080DC70C, XV_POOL },
	{ 0x080DC70C, 0x080DC718, XV_TABLE },
	{ 0x080DC788, 0x080DC78C, XV_POOL },
	{ 0x080DC78C, 0x080DC794, XV_TABLE },
	{ 0x080DC93E, 0x080DC940, XV_RAW },
	{ 0x080DC940, 0x080DC948, XV_POOL },
	{ 0x080DC948, 0x080DC950, XV_RAW },
	{ 0x080DC950, 0x080DC954, XV_RAW },
	{ 0x080DC954, 0x080DC968, XV_POOL },
	{ 0x080E597E, 0x080E5980, XV_RAW },
	{ 0x080E5980, 0x080E5984, XV_POOL },
	{ 0x080E5984, 0x080E5990, XV_TABLE },
	{ 0x080E5B10, 0x080E5B18, XV_POOL },
	{ 0x080E5B18, 0x080E5B20, XV_RAW },
	{ 0x080E5B20, 0x080E5B28, XV_RAW },
	{ 0x080E7DD6, 0x080E7DD8, XV_RAW },
	{ 0x080E7DD8, 0x080E7DDC, XV_POOL },
	{ 0x080E7DDC, 0x080E7DE8, XV_TABLE },
	{ 0x080E7E46, 0x080E7E48, XV_RAW },
	{ 0x080E7E48, 0x080E7E4C, XV_POOL },
	{ 0x080E7E4C, 0x080E7E50, XV_RAW },
	{ 0x080E7EEC, 0x080E7EF0, XV_POOL },
	{ 0x080E7EF0, 0x080E7EF8, XV_RAW },
	{ 0x080E7F06, 0x080E7F08, XV_RAW },
	{ 0x080E7F08, 0x080E7F0C, XV_POOL },
	{ 0x080E7F0C, 0x080E7F18, XV_TABLE },
	{ 0x080E8032, 0x080E8034, XV_RAW },
	{ 0x080E8034, 0x080E803C, XV_POOL },
	{ 0x080E803C, 0x080E8044, XV_RAW },
	{ 0x080E8044, 0x080E8048, XV_RAW },
	{ 0x080E8056, 0x080E8058, XV_RAW },
	{ 0x080E8058, 0x080E805C, XV_POOL },
	{ 0x080E805C, 0x080E8068, XV_TABLE },
	{ 0x080E80BE, 0x080E80C0, XV_RAW },
	{ 0x080E80C0, 0x080E80C4, XV_POOL },
	{ 0x080E80C4, 0x080E80C8, XV_RAW },
	{ 0x080EC812, 0x080EC814, XV_RAW },
	{ 0x080EC814, 0x080EC818, XV_POOL },
	{ 0x080EC818, 0x080EC884, XV_TABLE },
};
static const XvPair misc_calls[] = {
	{ 0x08010D18, 0x08013682 },   /* map:calls, DIFF (audited) */
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C3A, 0x0801A146 },   /* project:families, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x080172CE, 0x0801A4D0 },   /* map:bytes, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080E450E, 0x080E5736 },   /* map:bytes, SAME */
	{ 0x080EA08C, 0x08E00C00 },   /* a stub, STUB */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08002D10, 0x08002EAC },   /* map:bytes, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0802EFFE, 0x0802EF5C },   /* map:bytes, SAME */
	{ 0x0800D586, 0x0800F656 },   /* map:bytes, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x0800C768, 0x0800E2FC },   /* project:chips, DIFF (audited) */
	{ 0x0800CD84, 0x0800EAFA },   /* map:calls, LAYOUT */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x08012704, 0x08015B54 },   /* map:calls, DIFF (audited) */
	{ 0x080127CE, 0x08015BEC },   /* map:calls, DIFF (audited) */
	{ 0x0800D544, 0x0800F614 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800B5A8, 0x0800CFA6 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x08002E0A, 0x08002FA6 },   /* map:bytes, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x080EA0BA, 0x08E00C00 },   /* a stub, STUB */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x080E5A52, 0x080E6DCA },   /* map:bytes, SAME */
	{ 0x0800931C, 0x0800AB3A },   /* map:calls, DIFF (audited) */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080D3286, 0x08E00EC6 },   /* a helper, HELPER */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair misc_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080096C5, 0x08E00C05 },
	{ 0x0800E17D, 0x08E00C05 },
	{ 0x0800E191, 0x08E00C05 },
	{ 0x0800E239, 0x08E00C05 },
	{ 0x0800E265, 0x08E00C05 },
	{ 0x0800E5AB, 0x08E00C05 },
	{ 0x080124C9, 0x08E00C05 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x0802D62F, 0x08E00C05 },
	{ 0x0802D669, 0x08E00C05 },
	{ 0x0802D679, 0x08E00C05 },
	{ 0x080CA485, 0x08E00C05 },
	{ 0x080CB29B, 0x08E00C05 },
	{ 0x080CECB1, 0x08E00C05 },
	{ 0x080CFA79, 0x08E00C05 },
	{ 0x080D0D21, 0x080D0CF9 },
	{ 0x080D14C7, 0x080D1621 },
	{ 0x080D151F, 0x08E00C05 },
	{ 0x080DC03D, 0x08E00C05 },
	{ 0x080E6CF5, 0x08E00C05 },
	{ 0x080E86EF, 0x08E00C05 },
	{ 0x080E9E8B, 0x08E00C05 },
	{ 0x080EA961, 0x08E00C05 },
};
static const XvImm misc_imms[] = {
	{ 0x080D80AA, THUMB_MOVS, 0, 0, 0x70, 0x5F },   /* its sound 0x70: BN5's copied to BN6's slot 0x5f */
	{ 0x080D837A, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D8862, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D892A, THUMB_MOVS, 0, 0, 0xBA, 0x60 },   /* its sound 0xba: BN5's copied to BN6's slot 0x60 */
	{ 0x080D8C30, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DAF44, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DC762, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DC7A6, THUMB_MOVS, 0, 0, 0x6, 0x59 },   /* its sound 0x105: BN5's copied to BN6's slot 0x59 */
	{ 0x080DC7A8, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x105: BN5's copied to BN6's slot 0x59 (its high part) */
};
static const XvNumber misc_numbers[] = {
	{ 0x080D8418, 0x00000C4E, 0x00000CF0 },
	{ 0x080D8A38, 0x01000417, 0x010004FC },
	{ 0x080D8A3C, 0x0A050300, 0x0A050300 },
	{ 0x080D8CD0, 0x00001011, 0x000010F8 },
	{ 0x080D8CD4, 0x0A050300, 0x0A050300 },
	{ 0x080DB0A0, 0x00000C3A, 0x00000CF1 },
	{ 0x080DB0A4, 0x04050300, 0x04050300 },
	{ 0x080DC958, 0x01041030, 0x010410FD },
	{ 0x080DC95C, 0x0A050100, 0x0A050100 },
};
static const XvSprite misc_sprites[] = {
	{ 3, 0x3A, 3, 0xF1, XC_ART + 0x38000, 0x1000 },
	{ 3, 0x4E, 3, 0xF0, XC_ART + 0x39000, 0x800 },
	{ 3, 0x31, 3, 0xEF, XC_ART + 0x39800, 0x800 },
	{ 1, 0x17, 1, 0xFC, XC_ART + 0x3A000, 0x1000 },
};
static const XvByte misc_bytes[] = {
	{ 0x080D0C0C, 0x31, 0xEF },   /* 080D0C0C's row 0: its sprite list 3 0x31 in BN5, 0xef in BN6 (BN5's copied there) */
	{ 0x080D0C12, 0x31, 0xEF },   /* 080D0C0C's row 1: its sprite list 3 0x31 in BN5, 0xef in BN6 (BN5's copied there) */
	{ 0x080D0C18, 0x31, 0xEF },   /* 080D0C0C's row 2: its sprite list 3 0x31 in BN5, 0xef in BN6 (BN5's copied there) */
	{ 0x080D0C1E, 0x31, 0xEF },   /* 080D0C0C's row 3: its sprite list 3 0x31 in BN5, 0xef in BN6 (BN5's copied there) */
	{ 0x080D0C24, 0x31, 0xEF },   /* 080D0C0C's row 4: its sprite list 3 0x31 in BN5, 0xef in BN6 (BN5's copied there) */
	{ 0x080D0C2A, 0x31, 0xEF },   /* 080D0C0C's row 5: its sprite list 3 0x31 in BN5, 0xef in BN6 (BN5's copied there) */
};
static const XvType misc_types[] = { { 3, 0x56, XV_TYPE_MOVED }, { 3, 0x5B, XV_TYPE_MOVED }, { 3, 0x8D, XV_TYPE_MOVED }, { 3, 0x90, XV_TYPE_MOVED }, { 3, 0x92, XV_TYPE_MOVED }, { 3, 0xA5, XV_TYPE_MOVED }, { 3, 0xB1, XV_TYPE_MOVED }, { 4, 0x46, XV_TYPE_MOVED }, { 4, 0x64, XV_TYPE_MOVED }, { 4, 0x65, XV_TYPE_MOVED }, { 4, 0x66, XV_TYPE_MOVED } };
static const XcSong misc_songs[] = { { 0x70, 0x5F }, { 0xBA, 0x60 }, { 0x105, 0x59 } };

#define MISC_CODE { .name = "Misc (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = misc_blocks, .nblocks = (int)(sizeof misc_blocks / sizeof *misc_blocks), \
	.data = misc_data, .ndata = (int)(sizeof misc_data / sizeof *misc_data), \
	.calls = misc_calls, .ncalls = (int)(sizeof misc_calls / sizeof *misc_calls), \
	.words = misc_words, .nwords = (int)(sizeof misc_words / sizeof *misc_words), \
	.imms = misc_imms, .nimms = (int)(sizeof misc_imms / sizeof *misc_imms), \
	.numbers = misc_numbers, .nnumbers = (int)(sizeof misc_numbers / sizeof *misc_numbers), \
	.sprites = misc_sprites, .nsprites = (int)(sizeof misc_sprites / sizeof *misc_sprites), \
	.types = misc_types, .ntypes = (int)(sizeof misc_types / sizeof *misc_types), \
	.bytes = misc_bytes, .nbytes = (int)(sizeof misc_bytes / sizeof *misc_bytes) }
/* ---- Vulcan: BN5's attack 0x19 (080EC6B9) as BN6's 0x69 ---- */

static const XvBlock vulcan_blocks[] = {
	{ 0x08016634, 0x0801663C, XC_CODE + 0x8E20, 0xB384251Bu },
	{ 0x08016B96, 0x08016BBC, XC_CODE + 0x8E32, 0xA53A3BEFu },
	{ 0x080C6C08, 0x080C6E28, XC_CODE + 0x8E60, 0xFFCD61DDu },
	{ 0x080EC6B8, 0x080EC7DE, XC_CODE + 0x9080, 0x755A6368u },
};
static const XvData vulcan_data[] = {
	{ 0x080C6C08, 0x080C6C80, XV_RAW },
	{ 0x080C6C8E, 0x080C6C90, XV_RAW },
	{ 0x080C6C90, 0x080C6C94, XV_POOL },
	{ 0x080C6C94, 0x080C6CA0, XV_TABLE },
	{ 0x080C6D24, 0x080C6D38, XV_RAW },
	{ 0x080C6D38, 0x080C6D3C, XV_POOL },
	{ 0x080C6D3C, 0x080C6D44, XV_TABLE },
	{ 0x080C6D7E, 0x080C6D80, XV_RAW },
	{ 0x080C6D80, 0x080C6D84, XV_POOL },
	{ 0x080C6D84, 0x080C6D8C, XV_TABLE },
	{ 0x080C6E24, 0x080C6E28, XV_POOL },
	{ 0x080EC6C6, 0x080EC6C8, XV_RAW },
	{ 0x080EC6C8, 0x080EC6CC, XV_POOL },
	{ 0x080EC6CC, 0x080EC6D8, XV_TABLE },
	{ 0x080EC794, 0x080EC79C, XV_POOL },
	{ 0x080EC79C, 0x080EC7A2, XV_RAW },
};
static const XvPair vulcan_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08018838, 0x0801BD3C },   /* map:bytes, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair vulcan_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080EC7A9, 0x080ED335 },
};
static const XvImm vulcan_imms[] = {
	{ 0x080C6CF4, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite vulcan_sprites[] = {
	{ 5, 0x0F, 5, 0xFC, XC_ART + 0x3B000, 0x1800 },
	{ 5, 0x04, 5, 0xFB, XC_ART + 0x3C800, 0x1000 },
};
static const XvId vulcan_effects[] = { { 0x3E, 0xF1 }, { 0x38, 0xF0 }, { 0x3D, 0xEF }, { 0x58, 0xEE } };
static const XvByte vulcan_bytes[] = {
	{ 0x080C6C5E, 0x3E, 0xF1 },   /* 080C6C08's row 14: its effect 0x3e in BN5, 0xf1 in BN6 */
	{ 0x080C6C64, 0x38, 0xF0 },   /* 080C6C08's row 15: its effect 0x38 in BN5, 0xf0 in BN6 */
	{ 0x080C6C6A, 0x3D, 0xEF },   /* 080C6C08's row 16: its effect 0x3d in BN5, 0xef in BN6 */
	{ 0x080C6C70, 0x58, 0xEE },   /* 080C6C08's row 17: its effect 0x58 in BN5, 0xee in BN6 */
	{ 0x080C6C76, 0x58, 0xEE },   /* 080C6C08's row 18: its effect 0x58 in BN5, 0xee in BN6 */
	{ 0x080C6C7C, 0x58, 0xEE },   /* 080C6C08's row 19: its effect 0x58 in BN5, 0xee in BN6 */
};
static const XvType vulcan_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0x12, XV_TYPE_MOVED } };

#define VULCAN_CODE { .name = "Vulcan (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = vulcan_blocks, .nblocks = (int)(sizeof vulcan_blocks / sizeof *vulcan_blocks), \
	.data = vulcan_data, .ndata = (int)(sizeof vulcan_data / sizeof *vulcan_data), \
	.calls = vulcan_calls, .ncalls = (int)(sizeof vulcan_calls / sizeof *vulcan_calls), \
	.words = vulcan_words, .nwords = (int)(sizeof vulcan_words / sizeof *vulcan_words), \
	.imms = vulcan_imms, .nimms = (int)(sizeof vulcan_imms / sizeof *vulcan_imms), \
	.sprites = vulcan_sprites, .nsprites = (int)(sizeof vulcan_sprites / sizeof *vulcan_sprites), \
	.effects = vulcan_effects, .neffects = (int)(sizeof vulcan_effects / sizeof *vulcan_effects), \
	.types = vulcan_types, .ntypes = (int)(sizeof vulcan_types / sizeof *vulcan_types), \
	.bytes = vulcan_bytes, .nbytes = (int)(sizeof vulcan_bytes / sizeof *vulcan_bytes) }
/* ---- Spreads: BN5's attack 0x25 (080ED093) as BN6's 0x75 ---- */

static const XvBlock spreads_blocks[] = {
	{ 0x080CC878, 0x080CC89C, XC_CODE + 0x91B0, 0xBED8190Eu },
	{ 0x080CC8B0, 0x080CCA04, XC_CODE + 0x91E0, 0xA5A768A0u },
	{ 0x080CCA62, 0x080CCACC, XC_CODE + 0x9342, 0x52845C15u },
	{ 0x080E3618, 0x080E36F8, XC_CODE + 0x93B0, 0xA30EBD26u },
	{ 0x080ED092, 0x080ED180, XC_CODE + 0x9492, 0x79BE7235u },
};
static const XvData spreads_data[] = {
	{ 0x080CC88A, 0x080CC88C, XV_RAW },
	{ 0x080CC88C, 0x080CC890, XV_POOL },
	{ 0x080CC890, 0x080CC89C, XV_TABLE },
	{ 0x080CC964, 0x080CC968, XV_POOL },
	{ 0x080CC968, 0x080CC990, XV_RAW },
	{ 0x080CC9FA, 0x080CC9FC, XV_RAW },
	{ 0x080CC9FC, 0x080CCA00, XV_POOL },
	{ 0x080CCA00, 0x080CCA04, XV_TABLE },
	{ 0x080CCAC2, 0x080CCAC4, XV_RAW },
	{ 0x080CCAC4, 0x080CCACC, XV_POOL },
	{ 0x080E3626, 0x080E3628, XV_RAW },
	{ 0x080E3628, 0x080E362C, XV_POOL },
	{ 0x080E362C, 0x080E3638, XV_TABLE },
	{ 0x080ED0A0, 0x080ED0A4, XV_POOL },
	{ 0x080ED0A4, 0x080ED0B0, XV_TABLE },
	{ 0x080ED0EC, 0x080ED0F0, XV_POOL },
	{ 0x080ED0F0, 0x080ED0F4, XV_RAW },
	{ 0x080ED146, 0x080ED148, XV_RAW },
	{ 0x080ED148, 0x080ED14C, XV_POOL },
	{ 0x080ED14C, 0x080ED150, XV_RAW },
};
static const XvPair spreads_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080172CE, 0x0801A4D0 },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002CBE, 0x08002E52 },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800DB3C, 0x0800FDB6 },   /* map:calls+bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
};
static const XvPair spreads_words[] = {
	{ 0x080CC89D, 0x080CCD31 },
	{ 0x080CCA05, 0x080CCE99 },
};
static const XvImm spreads_imms[] = {
	{ 0x080CC8B4, THUMB_MOVS, 2, 0, 0x1D, 0xF0 },   /* its sprite: list 4 0x1d in BN5, 0xf0 in BN6 (BN5's copied there) */
	{ 0x080CC9A4, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvSprite spreads_sprites[] = {
	{ 4, 0x1D, 4, 0xF0, XC_ART + 0x3D800, 0x1000 },
};
static const XvType spreads_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 3, 0x3B, XV_TYPE_MOVED }, { 4, 0x28, XV_TYPE_MOVED } };

#define SPREADS_CODE { .name = "Spreads (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = spreads_blocks, .nblocks = (int)(sizeof spreads_blocks / sizeof *spreads_blocks), \
	.data = spreads_data, .ndata = (int)(sizeof spreads_data / sizeof *spreads_data), \
	.calls = spreads_calls, .ncalls = (int)(sizeof spreads_calls / sizeof *spreads_calls), \
	.words = spreads_words, .nwords = (int)(sizeof spreads_words / sizeof *spreads_words), \
	.imms = spreads_imms, .nimms = (int)(sizeof spreads_imms / sizeof *spreads_imms), \
	.sprites = spreads_sprites, .nsprites = (int)(sizeof spreads_sprites / sizeof *spreads_sprites), \
	.types = spreads_types, .ntypes = (int)(sizeof spreads_types / sizeof *spreads_types) }
/* ---- DarkCirc: BN5's support 0x2a (080E75DD) as BN6's 0x54 ---- */

static const XvBlock darkcirc_blocks[] = {
	{ 0x080D750C, 0x080D77FA, XC_CODE + 0x9580, 0x5A1AAF5Fu },
	{ 0x080D7880, 0x080D78AC, XC_CODE + 0x9870, 0x744C637Bu },
	{ 0x080E7544, 0x080E75FE, XC_CODE + 0x98A0, 0x5E71E8A7u },
};
static const XvData darkcirc_data[] = {
	{ 0x080D751E, 0x080D7520, XV_RAW },
	{ 0x080D7520, 0x080D7524, XV_POOL },
	{ 0x080D7524, 0x080D7530, XV_TABLE },
	{ 0x080D759E, 0x080D75A0, XV_RAW },
	{ 0x080D75A0, 0x080D75A4, XV_POOL },
	{ 0x080D75A4, 0x080D75AC, XV_TABLE },
	{ 0x080D7638, 0x080D763C, XV_POOL },
	{ 0x080D763C, 0x080D7644, XV_TABLE },
	{ 0x080D7718, 0x080D771C, XV_POOL },
	{ 0x080D771C, 0x080D772C, XV_RAW },
	{ 0x080D7782, 0x080D7784, XV_RAW },
	{ 0x080D7784, 0x080D7788, XV_POOL },
	{ 0x080D779A, 0x080D779C, XV_RAW },
	{ 0x080D779C, 0x080D77A0, XV_POOL },
	{ 0x080D77A0, 0x080D77AC, XV_TABLE },
	{ 0x080E7552, 0x080E7554, XV_RAW },
	{ 0x080E7554, 0x080E7558, XV_POOL },
	{ 0x080E7558, 0x080E7564, XV_TABLE },
	{ 0x080E7572, 0x080E7574, XV_RAW },
	{ 0x080E7574, 0x080E7578, XV_POOL },
	{ 0x080E7578, 0x080E7588, XV_TABLE },
	{ 0x080E75D4, 0x080E75D8, XV_POOL },
	{ 0x080E75D8, 0x080E75DC, XV_RAW },
};
static const XvPair darkcirc_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800BB4C, 0x0800D5F0 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080D77FA, 0x080D80CA },   /* search, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
};
static const XvPair darkcirc_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x080D77FB, 0x080D80CB },
};
static const XvImm darkcirc_imms[] = {
	{ 0x080D7534, THUMB_MOVS, 2, 0, 0x22, 0xEF },   /* its sprite: list 4 0x22 in BN5, 0xef in BN6 (BN5's copied there) */
	{ 0x080D75EC, THUMB_MOVS, 0, 0, 0xF, 0x4E },   /* its sound 0x10e: BN5's copied to BN6's slot 0x4e */
	{ 0x080D75EE, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x10e: BN5's copied to BN6's slot 0x4e (its high part) */
	{ 0x080D77B0, THUMB_MOVS, 2, 0, 0x22, 0xEF },   /* its sprite: list 4 0x22 in BN5, 0xef in BN6 (BN5's copied there) */
	{ 0x080D77EA, THUMB_MOVS, 0, 0, 0xBD, 0x4F },   /* its sound 0xbd: BN5's copied to BN6's slot 0x4f */
};
static const XvSprite darkcirc_sprites[] = {
	{ 4, 0x22, 4, 0xEF, XC_ART + 0x3E800, 0x800 },
};
static const XvType darkcirc_types[] = { { 3, 0x89, XV_TYPE_MOVED }, { 3, 0x8A, XV_TYPE_MOVED }, { 4, 0x5E, XV_TYPE_MOVED } };
static const XcSong darkcirc_songs[] = { { 0xBD, 0x4F }, { 0x10E, 0x4E } };

#define DARKCIRC_CODE { .name = "DarkCirc (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = darkcirc_blocks, .nblocks = (int)(sizeof darkcirc_blocks / sizeof *darkcirc_blocks), \
	.data = darkcirc_data, .ndata = (int)(sizeof darkcirc_data / sizeof *darkcirc_data), \
	.calls = darkcirc_calls, .ncalls = (int)(sizeof darkcirc_calls / sizeof *darkcirc_calls), \
	.words = darkcirc_words, .nwords = (int)(sizeof darkcirc_words / sizeof *darkcirc_words), \
	.imms = darkcirc_imms, .nimms = (int)(sizeof darkcirc_imms / sizeof *darkcirc_imms), \
	.sprites = darkcirc_sprites, .nsprites = (int)(sizeof darkcirc_sprites / sizeof *darkcirc_sprites), \
	.types = darkcirc_types, .ntypes = (int)(sizeof darkcirc_types / sizeof *darkcirc_types) }
/* ---- DrkSonic: BN5's support 0x1b (080E5F31) as BN6's 0x45 ---- */

static const XvBlock drksonic_blocks[] = {
	{ 0x080D5240, 0x080D53A0, XC_CODE + 0x9960, 0xF78416AFu },
	{ 0x080D53B0, 0x080D53F0, XC_CODE + 0x9AC0, 0x75186ED8u },
	{ 0x080D540A, 0x080D5634, XC_CODE + 0x9B02, 0x358997F9u },
	{ 0x080E5E88, 0x080E5F52, XC_CODE + 0x9D30, 0x831E5B08u },
};
static const XvData drksonic_data[] = {
	{ 0x080D5240, 0x080D5254, XV_RAW },
	{ 0x080D5262, 0x080D5264, XV_RAW },
	{ 0x080D5264, 0x080D5268, XV_POOL },
	{ 0x080D5268, 0x080D5274, XV_TABLE },
	{ 0x080D52FE, 0x080D5300, XV_RAW },
	{ 0x080D5300, 0x080D530C, XV_POOL },
	{ 0x080D530C, 0x080D5314, XV_RAW },
	{ 0x080D533A, 0x080D533C, XV_RAW },
	{ 0x080D533C, 0x080D5340, XV_POOL },
	{ 0x080D5340, 0x080D5360, XV_TABLE },
	{ 0x080D5394, 0x080D5398, XV_POOL },
	{ 0x080D5398, 0x080D53A0, XV_TABLE },
	{ 0x080D53DA, 0x080D53DC, XV_RAW },
	{ 0x080D53DC, 0x080D53E0, XV_POOL },
	{ 0x080D53E0, 0x080D53F0, XV_TABLE },
	{ 0x080D5456, 0x080D5458, XV_RAW },
	{ 0x080D5458, 0x080D5460, XV_POOL },
	{ 0x080D5460, 0x080D5474, XV_TABLE },
	{ 0x080D5474, 0x080D547E, XV_RAW },
	{ 0x080D5492, 0x080D5494, XV_RAW },
	{ 0x080D5494, 0x080D5498, XV_POOL },
	{ 0x080D5498, 0x080D54A0, XV_RAW },
	{ 0x080D555E, 0x080D5560, XV_RAW },
	{ 0x080D5560, 0x080D5564, XV_POOL },
	{ 0x080D5564, 0x080D556C, XV_RAW },
	{ 0x080D558E, 0x080D5590, XV_RAW },
	{ 0x080D5590, 0x080D5594, XV_POOL },
	{ 0x080D55AE, 0x080D55B0, XV_RAW },
	{ 0x080D55B0, 0x080D55B4, XV_POOL },
	{ 0x080D5626, 0x080D5628, XV_RAW },
	{ 0x080D5628, 0x080D5634, XV_POOL },
	{ 0x080E5E96, 0x080E5E98, XV_RAW },
	{ 0x080E5E98, 0x080E5E9C, XV_POOL },
	{ 0x080E5E9C, 0x080E5EA8, XV_TABLE },
	{ 0x080E5EB6, 0x080E5EB8, XV_RAW },
	{ 0x080E5EB8, 0x080E5EBC, XV_POOL },
	{ 0x080E5EBC, 0x080E5ECC, XV_TABLE },
	{ 0x080E5F1C, 0x080E5F20, XV_POOL },
	{ 0x080E5F20, 0x080E5F30, XV_RAW },
};
static const XvPair drksonic_calls[] = {
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x0801797C, 0x0801ADFA },   /* map:bytes, SAME */
	{ 0x0800D5A2, 0x0800F672 },   /* map:bytes, SAME */
	{ 0x08017FCC, 0x0801B394 },   /* map:calls, DIFF (audited) */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016C70, 0x0801A180 },   /* map:bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0802EFFE, 0x0802EF5C },   /* map:bytes, SAME */
	{ 0x0800D586, 0x0800F656 },   /* map:bytes, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800CD84, 0x0800EAFA },   /* map:calls, LAYOUT */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800D544, 0x0800F614 },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
};
static const XvPair drksonic_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080133C5, 0x080165B9 },
	{ 0x080134A1, 0x080166AF },
	{ 0x08013825, 0x08016B03 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x08014709, 0x08017CC1 },
	{ 0x080D53A1, 0x080D5A49 },
	{ 0x080D53F1, 0x080D5A99 },
};
static const XvImm drksonic_imms[] = {
	{ 0x080D5282, THUMB_MOVS, 2, 0, 0x8, 0xFB },   /* its sprite: list 1 0x8 in BN5, 0xfb in BN6 (BN5's copied there) */
	{ 0x080D54DC, THUMB_MOVS, 0, 0, 0x70, 0x5F },   /* its sound 0x70: BN5's copied to BN6's slot 0x5f */
};
static const XvNumber drksonic_numbers[] = {
	{ 0x080D562C, 0x0605FF01, 0x0605FF01 },
};
static const XvSprite drksonic_sprites[] = {
	{ 1, 0x08, 1, 0xFB, XC_ART + 0x3F000, 0x2000 },
};
static const XvType drksonic_types[] = { { 3, 0x78, XV_TYPE_MOVED }, { 4, 0x4A, XV_TYPE_MOVED } };
static const XcSong drksonic_songs[] = { { 0x70, 0x5F } };

#define DRKSONIC_CODE { .name = "DrkSonic (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = drksonic_blocks, .nblocks = (int)(sizeof drksonic_blocks / sizeof *drksonic_blocks), \
	.data = drksonic_data, .ndata = (int)(sizeof drksonic_data / sizeof *drksonic_data), \
	.calls = drksonic_calls, .ncalls = (int)(sizeof drksonic_calls / sizeof *drksonic_calls), \
	.words = drksonic_words, .nwords = (int)(sizeof drksonic_words / sizeof *drksonic_words), \
	.imms = drksonic_imms, .nimms = (int)(sizeof drksonic_imms / sizeof *drksonic_imms), \
	.numbers = drksonic_numbers, .nnumbers = (int)(sizeof drksonic_numbers / sizeof *drksonic_numbers), \
	.sprites = drksonic_sprites, .nsprites = (int)(sizeof drksonic_sprites / sizeof *drksonic_sprites), \
	.types = drksonic_types, .ntypes = (int)(sizeof drksonic_types / sizeof *drksonic_types) }
/* ---- NumbrMan: BN5's navi 0x0 (080BB629) as BN6's 0x1d ---- */

static const XvBlock numbrman_blocks[] = {
	{ 0x080BB434, 0x080BB64E, XC_CODE + 0x9E00, 0x1FA0548Cu },
	{ 0x080C4214, 0x080C43CE, XC_CODE + 0xA020, 0x5C6519A0u },
	{ 0x080C5346, 0x080C5386, XC_CODE + 0xA1E2, 0xCBC3BA1Eu },
};
static const XvData numbrman_data[] = {
	{ 0x080BB446, 0x080BB448, XV_RAW },
	{ 0x080BB448, 0x080BB44C, XV_POOL },
	{ 0x080BB44C, 0x080BB458, XV_TABLE },
	{ 0x080BB4BA, 0x080BB4BC, XV_RAW },
	{ 0x080BB4BC, 0x080BB4C0, XV_POOL },
	{ 0x080BB4C0, 0x080BB4D0, XV_TABLE },
	{ 0x080BB542, 0x080BB544, XV_RAW },
	{ 0x080BB544, 0x080BB548, XV_POOL },
	{ 0x080BB548, 0x080BB550, XV_TABLE },
	{ 0x080BB5B6, 0x080BB5B8, XV_RAW },
	{ 0x080BB5B8, 0x080BB5C0, XV_POOL },
	{ 0x080BB5C0, 0x080BB5D0, XV_RAW },
	{ 0x080C4214, 0x080C4216, XV_RAW },
	{ 0x080C4224, 0x080C4228, XV_POOL },
	{ 0x080C4228, 0x080C4234, XV_TABLE },
	{ 0x080C4280, 0x080C4284, XV_POOL },
	{ 0x080C42D0, 0x080C42D4, XV_POOL },
	{ 0x080C42D4, 0x080C42DC, XV_TABLE },
	{ 0x080C432C, 0x080C4330, XV_POOL },
	{ 0x080C4330, 0x080C4340, XV_TABLE },
};
static const XvPair numbrman_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800DA82, 0x0800FCFC },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002BF8, 0x08002D8C },   /* map:bytes, SAME */
	{ 0x08002D40, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002C34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002C20, 0x08002DB4 },   /* near, SAME */
	{ 0x08002DA2, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002D66, 0x08002F02 },   /* near, SAME */
	{ 0x08002B4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002AEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08001336, 0x0800139A },   /* map:bytes, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
};
static const XvPair numbrman_words[] = {
	{ 0x0800325D, 0x08003459 },
};
static const XvImm numbrman_imms[] = {
	{ 0x080BB45C, THUMB_MOVS, 2, 0, 0x9, 0x22 },   /* BN5's Navi 9's battle sprite: list 2 0x9 in BN5, 0x22 in BN6 (the Navis') */
};
static const XvByte numbrman_bytes[] = {
	{ 0x080C4215, 0x06, 0xAC },   /* 080C4214's row 0: its sprite list 4 0x6 in BN5, 0xac in BN6 (BN5's copied there) */
};
static const XvType numbrman_types[] = { { 1, 0x0F, XV_TYPE_MOVED }, { 1, 0x5C, XV_TYPE_MOVED }, { 3, 0x05, XV_TYPE_SHARED } };

#define NUMBRMAN_CODE { .name = "NumbrMan (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = numbrman_blocks, .nblocks = (int)(sizeof numbrman_blocks / sizeof *numbrman_blocks), \
	.data = numbrman_data, .ndata = (int)(sizeof numbrman_data / sizeof *numbrman_data), \
	.calls = numbrman_calls, .ncalls = (int)(sizeof numbrman_calls / sizeof *numbrman_calls), \
	.words = numbrman_words, .nwords = (int)(sizeof numbrman_words / sizeof *numbrman_words), \
	.imms = numbrman_imms, .nimms = (int)(sizeof numbrman_imms / sizeof *numbrman_imms), \
	.types = numbrman_types, .ntypes = (int)(sizeof numbrman_types / sizeof *numbrman_types), \
	.bytes = numbrman_bytes, .nbytes = (int)(sizeof numbrman_bytes / sizeof *numbrman_bytes) }
/* ---- KnightMan: BN5's navi 0x5 (080B9F4D) as BN6's 0x22 ---- */

static const XvBlock knightman_blocks[] = {
	{ 0x080B9D10, 0x080B9F80, XC_CODE + 0xA230, 0x2E1E0154u },
	{ 0x080C7772, 0x080C77AC, XC_CODE + 0xA4A2, 0x473CBAC8u },
};
static const XvData knightman_data[] = {
	{ 0x080B9D22, 0x080B9D24, XV_RAW },
	{ 0x080B9D24, 0x080B9D28, XV_POOL },
	{ 0x080B9D28, 0x080B9D34, XV_TABLE },
	{ 0x080B9DA6, 0x080B9DA8, XV_RAW },
	{ 0x080B9DA8, 0x080B9DAC, XV_POOL },
	{ 0x080B9DAC, 0x080B9DBC, XV_TABLE },
	{ 0x080B9E66, 0x080B9E68, XV_RAW },
	{ 0x080B9E68, 0x080B9E6C, XV_POOL },
	{ 0x080B9E6C, 0x080B9E74, XV_TABLE },
	{ 0x080B9EF8, 0x080B9EFC, XV_POOL },
	{ 0x080B9EFC, 0x080B9F04, XV_TABLE },
	{ 0x080B9F72, 0x080B9F74, XV_RAW },
	{ 0x080B9F74, 0x080B9F80, XV_POOL },
};
static const XvPair knightman_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800ED06, 0x08010DF6 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x0800EEB6, 0x08011044 },   /* map:bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080C3856, 0x080C5D6A },   /* map:bytes, SAME */
	{ 0x080C3840, 0x080C5D54 },   /* map:bytes, SAME */
	{ 0x080C386C, 0x080C5D80 },   /* map:bytes, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
};
static const XvPair knightman_words[] = {
	{ 0x0800325D, 0x08003459 },
};
static const XvImm knightman_imms[] = {
	{ 0x080B9D38, THUMB_MOVS, 2, 0, 0xB, 0x24 },   /* BN5's Navi 11's battle sprite: list 2 0xb in BN5, 0x24 in BN6 (the Navis') */
	{ 0x080B9D82, THUMB_MOVS, 1, 0, 0xB, 0x24 },   /* a Navi's companion begun by its AI (the triple 0x0800ED1C's second table, BN6's twin 0x08010DF6 reading BN6_NAVI_COMPANION_REF's, grown by the Navis: KnightMan's overlay, its kind BN6's 23): BN5's AI index 0xb, BN6's 0x24 */
	{ 0x080B9DF6, THUMB_MOVS, 1, 0, 0xB, 0x24 },   /* a Navi's companion ended by its AI (the triple 0x0800EECC's second table, BN6's twin 0x08011044 reading BN6_NAVI_COMPANION_END_REF's, grown by the Navis): BN5's AI index 0xb, BN6's 0x24 */
	{ 0x080B9E1E, THUMB_MOVS, 0, 0, 0xC0, 0x62 },   /* its sound 0xc0: BN5's copied to BN6's slot 0x62 */
	{ 0x080B9F3E, THUMB_MOVS, 1, 0, 0xB, 0x24 },   /* a Navi's companion ended by its AI (the triple 0x0800EECC's second table, BN6's twin 0x08011044 reading BN6_NAVI_COMPANION_END_REF's, grown by the Navis): BN5's AI index 0xb, BN6's 0x24 */
};
static const XvType knightman_types[] = { { 1, 0x06, XV_TYPE_MOVED }, { 3, 0x17, XV_TYPE_SHARED } };
static const XcSong knightman_songs[] = { { 0xC0, 0x62 } };

#define KNIGHTMAN_CODE { .name = "KnightMan (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = knightman_blocks, .nblocks = (int)(sizeof knightman_blocks / sizeof *knightman_blocks), \
	.data = knightman_data, .ndata = (int)(sizeof knightman_data / sizeof *knightman_data), \
	.calls = knightman_calls, .ncalls = (int)(sizeof knightman_calls / sizeof *knightman_calls), \
	.words = knightman_words, .nwords = (int)(sizeof knightman_words / sizeof *knightman_words), \
	.imms = knightman_imms, .nimms = (int)(sizeof knightman_imms / sizeof *knightman_imms), \
	.types = knightman_types, .ntypes = (int)(sizeof knightman_types / sizeof *knightman_types) }
/* ---- Colonel: BN5's navi 0xa (080BA5F1) as BN6's 0x27 ---- */

static const XvBlock colonel_blocks[] = {
	{ 0x080BA368, 0x080BA6A8, XC_CODE + 0xA4E0, 0xF4D02C86u },
	{ 0x080E1118, 0x080E125C, XC_CODE + 0xA820, 0xD30DB034u },
};
static const XvData colonel_data[] = {
	{ 0x080BA37A, 0x080BA37C, XV_RAW },
	{ 0x080BA37C, 0x080BA380, XV_POOL },
	{ 0x080BA380, 0x080BA38C, XV_TABLE },
	{ 0x080BA3F2, 0x080BA3F4, XV_RAW },
	{ 0x080BA3F4, 0x080BA3F8, XV_POOL },
	{ 0x080BA3F8, 0x080BA408, XV_TABLE },
	{ 0x080BA4AC, 0x080BA4B0, XV_POOL },
	{ 0x080BA4B0, 0x080BA4B8, XV_TABLE },
	{ 0x080BA57A, 0x080BA57C, XV_RAW },
	{ 0x080BA57C, 0x080BA584, XV_POOL },
	{ 0x080BA584, 0x080BA590, XV_RAW },
	{ 0x080BA590, 0x080BA594, XV_RAW },
	{ 0x080BA694, 0x080BA698, XV_POOL },
	{ 0x080BA698, 0x080BA6A0, XV_RAW },
	{ 0x080BA6A0, 0x080BA6A8, XV_POOL },
	{ 0x080E1128, 0x080E112C, XV_POOL },
	{ 0x080E112C, 0x080E1134, XV_TABLE },
	{ 0x080E11A2, 0x080E11A4, XV_RAW },
	{ 0x080E11A4, 0x080E11A8, XV_POOL },
	{ 0x080E11A8, 0x080E11B0, XV_RAW },
	{ 0x080E11B0, 0x080E11B8, XV_POOL },
	{ 0x080E1234, 0x080E1240, XV_POOL },
};
static const XvPair colonel_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800DA82, 0x0800FCFC },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800ED06, 0x08010DF6 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x0800EEB6, 0x08011044 },   /* map:bytes, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080022E8, 0x08002378 },   /* map:bytes, SAME */
	{ 0x0800230A, 0x0800239A },   /* map:bytes, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
};
static const XvPair colonel_words[] = {
	{ 0x0800325D, 0x08003459 },
};
static const XvImm colonel_imms[] = {
	{ 0x080BA390, THUMB_MOVS, 2, 0, 0x7, 0x20 },   /* BN5's Navi 7's battle sprite: list 2 0x7 in BN5, 0x20 in BN6 (the Navis') */
	{ 0x080BA526, THUMB_ADDS, 0, 0, 0x48, 0xA8 },   /* its effects 0x48-0x4a in BN5, 0xa8-0xaa in BN6: Colonel's soldiers' bursts: 0x48 + its +5 (0x080BA524-0x080BA528), list 4's 0x1F, its animations 0-2 */
};
static const XvNumber colonel_numbers[] = {
	{ 0x080E11B4, 0x03003B60, 0x03003B60 },
	{ 0x080E1238, 0x03003B60, 0x03003B60 },
	{ 0x080E123C, 0x03003750, 0x03003750 },
};
static const XvType colonel_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 1, 0x08, XV_TYPE_MOVED }, { 4, 0x0A, XV_TYPE_MOVED } };

#define COLONEL_CODE { .name = "Colonel (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = colonel_blocks, .nblocks = (int)(sizeof colonel_blocks / sizeof *colonel_blocks), \
	.data = colonel_data, .ndata = (int)(sizeof colonel_data / sizeof *colonel_data), \
	.calls = colonel_calls, .ncalls = (int)(sizeof colonel_calls / sizeof *colonel_calls), \
	.words = colonel_words, .nwords = (int)(sizeof colonel_words / sizeof *colonel_words), \
	.imms = colonel_imms, .nimms = (int)(sizeof colonel_imms / sizeof *colonel_imms), \
	.numbers = colonel_numbers, .nnumbers = (int)(sizeof colonel_numbers / sizeof *colonel_numbers), \
	.types = colonel_types, .ntypes = (int)(sizeof colonel_types / sizeof *colonel_types) }
/* ---- ShadowMan: BN5's navi 0xb (080BA343) as BN6's 0x28 ---- */

static const XvBlock shadowman_blocks[] = {
	{ 0x080B9F80, 0x080BA368, XC_CODE + 0xA970, 0x927DF3A0u },
	{ 0x080D15D8, 0x080D16E2, XC_CODE + 0xAD60, 0xCC11F0DEu },
	{ 0x080D1794, 0x080D17C2, XC_CODE + 0xAE70, 0xDE422133u },
};
static const XvData shadowman_data[] = {
	{ 0x080B9F9E, 0x080B9FA0, XV_RAW },
	{ 0x080B9FA0, 0x080B9FA4, XV_POOL },
	{ 0x080B9FA4, 0x080B9FB0, XV_TABLE },
	{ 0x080BA000, 0x080BA004, XV_POOL },
	{ 0x080BA004, 0x080BA014, XV_TABLE },
	{ 0x080BA07A, 0x080BA07C, XV_RAW },
	{ 0x080BA07C, 0x080BA080, XV_POOL },
	{ 0x080BA080, 0x080BA090, XV_TABLE },
	{ 0x080BA23A, 0x080BA23C, XV_RAW },
	{ 0x080BA23C, 0x080BA240, XV_POOL },
	{ 0x080BA240, 0x080BA250, XV_RAW },
	{ 0x080D15E6, 0x080D15E8, XV_RAW },
	{ 0x080D15E8, 0x080D15EC, XV_POOL },
	{ 0x080D15EC, 0x080D15F8, XV_TABLE },
	{ 0x080D16AE, 0x080D16B0, XV_RAW },
	{ 0x080D16B0, 0x080D16B4, XV_POOL },
	{ 0x080D16B4, 0x080D16C4, XV_TABLE },
};
static const XvPair shadowman_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800DA82, 0x0800FCFC },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B6BE, 0x0800D0BC },   /* map:bytes, SAME */
	{ 0x0800C6CE, 0x0800E2CE },   /* map:bytes, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA08C, 0x08E00C00 },   /* a stub, STUB */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
};
static const XvPair shadowman_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x080D16E3, 0x080D182D },
	{ 0x080D174D, 0x080D1897 },
	{ 0x080D176B, 0x080D18B5 },
};
static const XvImm shadowman_imms[] = {
	{ 0x080B9FB4, THUMB_MOVS, 2, 0, 0x8, 0x21 },   /* BN5's Navi 8's battle sprite: list 2 0x8 in BN5, 0x21 in BN6 (the Navis') */
	{ 0x080BA0FA, THUMB_MOVS, 0, 0, 0xB7, 0x61 },   /* its sound 0xb7: BN5's copied to BN6's slot 0x61 */
	{ 0x080D167E, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvType shadowman_types[] = { { 1, 0x07, XV_TYPE_MOVED }, { 3, 0x5C, XV_TYPE_MOVED } };
static const XcSong shadowman_songs[] = { { 0xB7, 0x61 } };

#define SHADOWMAN_CODE { .name = "ShadowMan (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = shadowman_blocks, .nblocks = (int)(sizeof shadowman_blocks / sizeof *shadowman_blocks), \
	.data = shadowman_data, .ndata = (int)(sizeof shadowman_data / sizeof *shadowman_data), \
	.calls = shadowman_calls, .ncalls = (int)(sizeof shadowman_calls / sizeof *shadowman_calls), \
	.words = shadowman_words, .nwords = (int)(sizeof shadowman_words / sizeof *shadowman_words), \
	.imms = shadowman_imms, .nimms = (int)(sizeof shadowman_imms / sizeof *shadowman_imms), \
	.types = shadowman_types, .ntypes = (int)(sizeof shadowman_types / sizeof *shadowman_types) }
/* ---- TomahawkMan: BN5's navi 0xc (080BC199) as BN6's 0x29 ---- */

static const XvBlock tomahawkman_blocks[] = {
	{ 0x080BBFB8, 0x080BC1C2, XC_CODE + 0xAEA0, 0x08BBE9BCu },
};
static const XvData tomahawkman_data[] = {
	{ 0x080BBFCA, 0x080BBFCC, XV_RAW },
	{ 0x080BBFCC, 0x080BBFD0, XV_POOL },
	{ 0x080BBFD0, 0x080BBFDC, XV_TABLE },
	{ 0x080BC030, 0x080BC034, XV_POOL },
	{ 0x080BC034, 0x080BC048, XV_TABLE },
	{ 0x080BC164, 0x080BC168, XV_POOL },
};
static const XvPair tomahawkman_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800DA82, 0x0800FCFC },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
};
static const XvPair tomahawkman_words[] = {
	{ 0x0800325D, 0x08003459 },
};
static const XvImm tomahawkman_imms[] = {
	{ 0x080BBFE8, THUMB_MOVS, 2, 0, 0xA, 0x23 },   /* BN5's Navi 10's battle sprite: list 2 0xa in BN5, 0x23 in BN6 (the Navis') */
	{ 0x080BC144, THUMB_MOVS, 0, 0, 0x45, 0xA6 },   /* its effects 0x45-0x45 in BN5, 0xa6-0xa6 in BN6: TomahawkMan's swing's: 0x45 (0x080BC144-0x080BC146, added to the side's high bytes) */
};
static const XvNumber tomahawkman_numbers[] = {
	{ 0x080BC164, 0x0700FF11, 0x0700FF11 },
};
static const XvType tomahawkman_types[] = { { 1, 0x13, XV_TYPE_MOVED } };

#define TOMAHAWKMAN_CODE { .name = "TomahawkMan (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = tomahawkman_blocks, .nblocks = (int)(sizeof tomahawkman_blocks / sizeof *tomahawkman_blocks), \
	.data = tomahawkman_data, .ndata = (int)(sizeof tomahawkman_data / sizeof *tomahawkman_data), \
	.calls = tomahawkman_calls, .ncalls = (int)(sizeof tomahawkman_calls / sizeof *tomahawkman_calls), \
	.words = tomahawkman_words, .nwords = (int)(sizeof tomahawkman_words / sizeof *tomahawkman_words), \
	.imms = tomahawkman_imms, .nimms = (int)(sizeof tomahawkman_imms / sizeof *tomahawkman_imms), \
	.numbers = tomahawkman_numbers, .nnumbers = (int)(sizeof tomahawkman_numbers / sizeof *tomahawkman_numbers), \
	.types = tomahawkman_types, .ntypes = (int)(sizeof tomahawkman_types / sizeof *tomahawkman_types) }
/* ---- ToadMan: BN5's navi 0xd (080BC9EF) as BN6's 0x2a ---- */

static const XvBlock toadman_blocks[] = {
	{ 0x080BC830, 0x080BCAC4, XC_CODE + 0xB0B0, 0x24D516FEu },
};
static const XvData toadman_data[] = {
	{ 0x080BC842, 0x080BC844, XV_RAW },
	{ 0x080BC844, 0x080BC848, XV_POOL },
	{ 0x080BC848, 0x080BC854, XV_TABLE },
	{ 0x080BC8A8, 0x080BC8AC, XV_POOL },
	{ 0x080BC8AC, 0x080BC8BC, XV_TABLE },
	{ 0x080BC92A, 0x080BC92C, XV_RAW },
	{ 0x080BC92C, 0x080BC930, XV_POOL },
	{ 0x080BC930, 0x080BC93C, XV_TABLE },
	{ 0x080BCABE, 0x080BCAC0, XV_RAW },
	{ 0x080BCAC0, 0x080BCAC4, XV_POOL },
};
static const XvPair toadman_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800DA82, 0x0800FCFC },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080D688E, XN_CODE + 0x14206 },   /* the Navis' copy, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
};
static const XvPair toadman_words[] = {
	{ 0x0800325D, 0x08003459 },
};
static const XvImm toadman_imms[] = {
	{ 0x080BC858, THUMB_MOVS, 2, 0, 0xC, 0x25 },   /* BN5's Navi 12's battle sprite: list 2 0xc in BN5, 0x25 in BN6 (the Navis') */
	{ 0x080BC986, THUMB_MOVS, 0, 0, 0x18, 0x5E },   /* its sound 0x117: BN5's copied to BN6's slot 0x5e */
	{ 0x080BC988, THUMB_ADDS, 0, 0, 0xFF, 0x0 },   /* its sound 0x117: BN5's copied to BN6's slot 0x5e (its high part) */
};
static const XvType toadman_types[] = { { 1, 0x17, XV_TYPE_MOVED } };
static const XcSong toadman_songs[] = { { 0x117, 0x5E } };

#define TOADMAN_CODE { .name = "ToadMan (BN5's)", .game = XROM_BN5_COLONEL_US, \
	.blocks = toadman_blocks, .nblocks = (int)(sizeof toadman_blocks / sizeof *toadman_blocks), \
	.data = toadman_data, .ndata = (int)(sizeof toadman_data / sizeof *toadman_data), \
	.calls = toadman_calls, .ncalls = (int)(sizeof toadman_calls / sizeof *toadman_calls), \
	.words = toadman_words, .nwords = (int)(sizeof toadman_words / sizeof *toadman_words), \
	.imms = toadman_imms, .nimms = (int)(sizeof toadman_imms / sizeof *toadman_imms), \
	.types = toadman_types, .ntypes = (int)(sizeof toadman_types / sizeof *toadman_types) }
const XChipUnit xchips_bn5[] = {
	{ XCHIP_HELPER, 0x00, 0x00, 0x080EBB97, XC_HOMES + 0x0, BUSTERARM_CODE, NULL, 0, 0x00 },
	{ XCHIP_HELPER, 0x00, 0x00, 0x080D3287, XC_HOMES + 0x400, SHAKE_CODE, NULL, 0, 0x00 },
	{ XCHIP_HELPER, 0x00, 0x00, 0x0800EA19, XC_HOMES + 0x800, POWERCUSTSWRD_CODE, NULL, 0, 0x00 },
	{ XCHIP_HELPER, 0x00, 0x00, 0x0800EB07, XC_HOMES + 0xC00, POWERCUSVOLT_CODE, NULL, 0, 0x00 },
	{ XCHIP_HELPER, 0x00, 0x00, 0x0800E8D7, XC_HOMES + 0x1000, POWERSP_CODE, NULL, 0, 0x00 },
	{ XCHIP_HELPER, 0x00, 0x00, 0x0800E9A9, XC_HOMES + 0x1400, POWERDS_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x1B, 0x6B, 0x080EC885, XC_HOMES + 0x1800, CRAKOUT_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x58, 0xA8, 0x080F0223, XC_HOMES + 0x1C00, HOTBODY_CODE, NULL, 0, 0x00 },
	{ XCHIP_SUPPORT, 0x13, 0x3D, 0x080E4629, XC_HOMES + 0x2000, JEALOUSY_CODE, NULL, 0, 0x00 },
	{ XCHIP_SUPPORT, 0x15, 0x3F, 0x080E493D, XC_HOMES + 0x2400, NRTHWIND_CODE, NULL, 0, 0x00 },
	{ XCHIP_SUPPORT, 0x06, 0x30, 0x080E2F85, XC_HOMES + 0x2800, METAGEL_CODE, metagel_songs, (int)(sizeof metagel_songs / sizeof *metagel_songs), 0x00 },
	{ XCHIP_SUPPORT, 0x08, 0x32, 0x080E3229, XC_HOMES + 0x2C00, BLINDER_CODE, blinder_songs, (int)(sizeof blinder_songs / sizeof *blinder_songs), 0x00 },
	{ XCHIP_ATTACK, 0x55, 0xA5, 0x080EFD19, XC_HOMES + 0x3000, SKULLY_CODE, skully_songs, (int)(sizeof skully_songs / sizeof *skully_songs), 0x00 },
	{ XCHIP_ATTACK, 0x4F, 0x9F, 0x080EF579, XC_HOMES + 0x3400, DRILARM_CODE, NULL, 0, 0x00 },
	{ XCHIP_SUPPORT, 0x3D, 0x67, 0x080E91EF, XC_HOMES + 0x3800, RAINYDAY_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x28, 0x78, 0x080ED36F, XC_HOMES + 0x3C00, WOODNOS_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x48, 0x98, 0x080EF1DD, XC_HOMES + 0x4000, CACTBAL_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x51, 0xA1, 0x080EF733, XC_HOMES + 0x4400, SPSHAKE_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x52, 0xA2, 0x080EF7CB, XC_HOMES + 0x4800, PULSAR_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x4E, 0x9E, 0x080EF441, XC_HOMES + 0x4C00, MRKCAN_CODE, mrkcan_songs, (int)(sizeof mrkcan_songs / sizeof *mrkcan_songs), 0x00 },
	{ XCHIP_ATTACK, 0x1C, 0x6C, 0x080EC91B, XC_HOMES + 0x5000, SIDEBUB_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x47, 0x97, 0x080EF119, XC_HOMES + 0x5400, AQWHIRL_CODE, aqwhirl_songs, (int)(sizeof aqwhirl_songs / sizeof *aqwhirl_songs), 0x00 },
	{ XCHIP_ATTACK, 0x34, 0x84, 0x080EE113, XC_HOMES + 0x5800, ZSAVER_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x54, 0xA4, 0x080EFA73, XC_HOMES + 0x5C00, KATANA_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x23, 0x73, 0x080ECF35, XC_HOMES + 0x6000, ELCREEL_CODE, elcreel_songs, (int)(sizeof elcreel_songs / sizeof *elcreel_songs), 0x00 },
	{ XCHIP_SUPPORT, 0x3A, 0x64, 0x080E816F, XC_HOMES + 0x6400, BOYBOMB_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x1F, 0x6F, 0x080ECBCF, XC_HOMES + 0x6800, GUARD_CODE, guard_songs, (int)(sizeof guard_songs / sizeof *guard_songs), 0x00 },
	{ XCHIP_ATTACK, 0x53, 0xA3, 0x080EF873, XC_HOMES + 0x6C00, MOONBLD_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x27, 0x77, 0x080ED28B, XC_HOMES + 0x7000, CUSVOLT_CODE, cusvolt_songs, (int)(sizeof cusvolt_songs / sizeof *cusvolt_songs), 0x00 },
	{ XCHIP_ATTACK, 0x13, 0x63, 0x080EBDC7, XC_HOMES + 0x7400, CUSTSWRD_CODE, NULL, 0, 0x00 },
	{ XCHIP_SUPPORT, 0x0C, 0x36, 0x080E380B, XC_HOMES + 0x7800, ANTI_CODE, anti_songs, (int)(sizeof anti_songs / sizeof *anti_songs), 0x00 },
	{ XCHIP_ATTACK, 0x49, 0x99, 0x080EF271, XC_HOMES + 0x7C00, REACTION49_CODE, reaction49_songs, (int)(sizeof reaction49_songs / sizeof *reaction49_songs), 0x00 },
	{ XCHIP_ATTACK, 0x29, 0x79, 0x080ED411, XC_HOMES + 0x8000, SLASHER_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x56, 0xA6, 0x080EFDB9, XC_HOMES + 0x8400, ELEMRAGE_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x12, 0x62, 0x080EBC91, XC_HOMES + 0x8800, BOMBS_CODE, bombs_songs, (int)(sizeof bombs_songs / sizeof *bombs_songs), 0x00 },
	{ XCHIP_ATTACK, 0x1A, 0x6A, 0x080EC7DF, XC_HOMES + 0x8C00, MISC_CODE, misc_songs, (int)(sizeof misc_songs / sizeof *misc_songs), 0x00 },
	{ XCHIP_ATTACK, 0x19, 0x69, 0x080EC6B9, XC_HOMES + 0x9000, VULCAN_CODE, NULL, 0, 0x00 },
	{ XCHIP_ATTACK, 0x25, 0x75, 0x080ED093, XC_HOMES + 0x9400, SPREADS_CODE, NULL, 0, 0x00 },
	{ XCHIP_SUPPORT, 0x2A, 0x54, 0x080E75DD, XC_HOMES + 0x9800, DARKCIRC_CODE, darkcirc_songs, (int)(sizeof darkcirc_songs / sizeof *darkcirc_songs), 0x00 },
	{ XCHIP_SUPPORT, 0x1B, 0x45, 0x080E5F31, XC_HOMES + 0x9C00, DRKSONIC_CODE, drksonic_songs, (int)(sizeof drksonic_songs / sizeof *drksonic_songs), 0x00 },
	{ XCHIP_NAVI, 0x00, 0x1D, 0x080BB629, XC_HOMES + 0xA000, NUMBRMAN_CODE, NULL, 0, 0x04 },
	{ XCHIP_NAVI, 0x05, 0x22, 0x080B9F4D, XC_HOMES + 0xA400, KNIGHTMAN_CODE, knightman_songs, (int)(sizeof knightman_songs / sizeof *knightman_songs), 0x10 },
	{ XCHIP_NAVI, 0x0A, 0x27, 0x080BA5F1, XC_HOMES + 0xA800, COLONEL_CODE, NULL, 0, 0x01 },
	{ XCHIP_NAVI, 0x0B, 0x28, 0x080BA343, XC_HOMES + 0xAC00, SHADOWMAN_CODE, shadowman_songs, (int)(sizeof shadowman_songs / sizeof *shadowman_songs), 0x02 },
	{ XCHIP_NAVI, 0x0C, 0x29, 0x080BC199, XC_HOMES + 0xB000, TOMAHAWKMAN_CODE, NULL, 0, 0x08 },
	{ XCHIP_NAVI, 0x0D, 0x2A, 0x080BC9EF, XC_HOMES + 0xB400, TOADMAN_CODE, toadman_songs, (int)(sizeof toadman_songs / sizeof *toadman_songs), 0x20 },
};
const int xchips_bn5_count = (int)(sizeof xchips_bn5 / sizeof *xchips_bn5);

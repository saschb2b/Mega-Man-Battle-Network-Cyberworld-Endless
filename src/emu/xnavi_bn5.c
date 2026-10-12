/* xnavi_bn5.c: the Navis' recipes, written by tools/xnavirecipe.py from the two ROMs (python3 tools/xnavirecipe.py;
 * --check says whether this is what it writes). Do not edit: change the generator (or tools/bn5_navi_twins.py, its pairs
 * and audits) and write it again. Numbers alone (docs/MULTIROM.md, Transplanted Navis): addresses in BN5 Team Colonel
 * (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from the player's ROM;
 * nothing of either game's. The control (BN6's own HeatMan, derived by the same walk) and BN5's Navis proven against
 * BN5's own (tools/xdiff.py), each at its fixed numbers (src/emu/xnavi_recipe.h). */
#include "xnavi_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xvirus_dispatch.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }

/* ---- BN5's engine objects moved once for every track (tools/bn5_objects.py), each module whole: its image object (the first kind's type 5; its kinds' sprites as BN6 lists them where a kind is fixed, BN5's bytes where none installed spawns it) and its sea's marker (the fourth kind's 0x3E) ---- */

static const XvBlock objects_blocks[] = {
	{ 0x080B9A5C, 0x080B9D10, XN_OBJECTS + 0x0, 0xBA3CBC8Cu },
	{ 0x080E4C4C, 0x080E4D3C, XN_OBJECTS + 0x300, 0xAF8AA9C6u },
};
static const XvData objects_data[] = {
	{ 0x080B9A5C, 0x080B9B80, XV_RAW },
	{ 0x080B9B8E, 0x080B9B90, XV_RAW },
	{ 0x080B9B90, 0x080B9B94, XV_POOL },
	{ 0x080B9B94, 0x080B9BA0, XV_TABLE },
	{ 0x080B9D04, 0x080B9D10, XV_POOL },
	{ 0x080E4C5E, 0x080E4C60, XV_RAW },
	{ 0x080E4C60, 0x080E4C64, XV_POOL },
	{ 0x080E4C64, 0x080E4C70, XV_TABLE },
};
static const XvPair objects_calls[] = {
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
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002E6A, 0x08003006 },   /* map:bytes, SAME */
	{ 0x08002E8E, 0x0800302A },   /* map:bytes, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x0800CDEC, 0x0800EB6C },   /* map:bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
};
static const XvPair objects_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138F9, 0x08016C9D },
};
static const XvImm objects_imms[] = {
	{ 0x080E4D12, THUMB_MOVS, 4, 0, 0x5D, 0xA1 },   /* its effect: 0x5d in BN5, 0xa1 in BN6 */
};
static const XvByte objects_bytes[] = {
	{ 0x080B9A6B, 0x0C, 0x10 },   /* its kind 0x3: BN5's sprite 3:0x0, copied at 4:0xb5 (navis) */
	{ 0x080B9A6C, 0x00, 0xB5 },   /* its kind 0x3: BN5's sprite 3:0x0, copied at 4:0xb5 (navis) */
	{ 0x080B9A9D, 0x0C, 0x14 },   /* its kind 0xd: BN5's sprite 3:0x6, copied at 5:0xa5 (navis) */
	{ 0x080B9A9E, 0x06, 0xA5 },   /* its kind 0xd: BN5's sprite 3:0x6, copied at 5:0xa5 (navis) */
	{ 0x080B9AB2, 0x2A, 0x59 },   /* its kind 0x11: BN5's sprite 3:0x2a, BN6's 3:0x59 the same bytes */
	{ 0x080B9B0B, 0x0C, 0x14 },   /* its kind 0x23: BN5's sprite 3:0x3, copied at 5:0xa6 (navis) */
	{ 0x080B9B0C, 0x03, 0xA6 },   /* its kind 0x23: BN5's sprite 3:0x3, copied at 5:0xa6 (navis) */
	{ 0x080B9B48, 0x1E, 0x58 },   /* its kind 0x2f: BN5's sprite 3:0x1e, BN6's 3:0x58 the same bytes */
	{ 0x080B9B4D, 0x21, 0x6C },   /* its kind 0x30: BN5's sprite 4:0x21, copied at 4:0x6c (families) */
	{ 0x080B9B70, 0x2A, 0x59 },   /* its kind 0x37: BN5's sprite 3:0x2a, BN6's 3:0x59 the same bytes */
};
static const XvSprite objects_sprites[] = {
	{ 3, 0x00, 4, 0xB5, XN_SHARED_ART + 0x1C800, 0x5800 },
	{ 3, 0x06, 5, 0xA5, XN_SHARED_ART + 0x22000, 0x800 },
	{ 3, 0x03, 5, 0xA6, XN_SHARED_ART + 0x22800, 0x2000 },
	{ 4, 0x21, 4, 0x6C, XN_SHARED_ART + 0x24800, 0x800 },
	{ 5, 0x1E, 5, 0xA1, XN_SHARED_ART + 0x3800, 0x1000 },
	{ 5, 0x1F, 5, 0xA4, XN_SHARED_ART + 0x25000, 0x800 },
};
static const XvId objects_effects[] = { { 0x5D, 0xA1 }, { 0x63, 0xAC } };
static const XvType objects_types[] = { { 1, 0x05, XV_TYPE_MOVED }, { 4, 0x3E, XV_TYPE_MOVED } };

const XVirusFamily xnavi_objects_bn5 = {
	.name = "BN5's moved objects",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = -1,
	.ai = -1,
	.home = XN_HOMES + 0x7800,
	.blocks = objects_blocks, .nblocks = (int)(sizeof objects_blocks / sizeof *objects_blocks),
	.data = objects_data, .ndata = (int)(sizeof objects_data / sizeof *objects_data),
	.calls = objects_calls, .ncalls = (int)(sizeof objects_calls / sizeof *objects_calls),
	.words = objects_words, .nwords = (int)(sizeof objects_words / sizeof *objects_words),
	.imms = objects_imms, .nimms = (int)(sizeof objects_imms / sizeof *objects_imms),
	.bytes = objects_bytes, .nbytes = (int)(sizeof objects_bytes / sizeof *objects_bytes),
	.sprites = objects_sprites, .nsprites = (int)(sizeof objects_sprites / sizeof *objects_sprites),
	.effects = objects_effects, .neffects = (int)(sizeof objects_effects / sizeof *objects_effects),
	.types = objects_types, .ntypes = (int)(sizeof objects_types / sizeof *objects_types),
};

/* ---- HeatMan (BN6's own, the control: AI 1 there, its ids 0x101, 0x102, 0x103, 0x104, 0x105, 0x106), as Navi AI 25 with ids 0x1c4-0x1c9 ---- */

static const XvBlock heatman_blocks[] = {
	{ 0x080F3E3C, 0x080F4C08, XN_CODE + 0x1A400, 0xCE00CB34u },
};
static const XvData heatman_data[] = {
	{ 0x080F3E3C, 0x080F3E70, XV_ZERO },
	{ 0x080F3E86, 0x080F3E88, XV_RAW },
	{ 0x080F3E88, 0x080F3E8C, XV_POOL },
	{ 0x080F3E8C, 0x080F3E94, XV_TABLE },
	{ 0x080F4004, 0x080F4008, XV_POOL },
	{ 0x080F4008, 0x080F400C, XV_RAW },
	{ 0x080F4026, 0x080F4028, XV_RAW },
	{ 0x080F4028, 0x080F402C, XV_POOL },
	{ 0x080F402C, 0x080F4030, XV_RAW },
	{ 0x080F404A, 0x080F404C, XV_RAW },
	{ 0x080F404C, 0x080F4050, XV_POOL },
	{ 0x080F4050, 0x080F4054, XV_RAW },
	{ 0x080F406E, 0x080F4070, XV_RAW },
	{ 0x080F4070, 0x080F4074, XV_POOL },
	{ 0x080F4074, 0x080F4078, XV_RAW },
	{ 0x080F4082, 0x080F4084, XV_RAW },
	{ 0x080F4084, 0x080F4088, XV_POOL },
	{ 0x080F4088, 0x080F408C, XV_RAW },
	{ 0x080F40A4, 0x080F40A8, XV_POOL },
	{ 0x080F40A8, 0x080F40B0, XV_RAW },
	{ 0x080F40C8, 0x080F40CC, XV_POOL },
	{ 0x080F40CC, 0x080F40D4, XV_RAW },
	{ 0x080F40DE, 0x080F40E0, XV_RAW },
	{ 0x080F40E0, 0x080F40E4, XV_POOL },
	{ 0x080F40E4, 0x080F40E8, XV_RAW },
	{ 0x080F4132, 0x080F4134, XV_RAW },
	{ 0x080F4134, 0x080F4140, XV_POOL },
	{ 0x080F4140, 0x080F4148, XV_RAW },
	{ 0x080F4148, 0x080F4150, XV_RAW },
	{ 0x080F4150, 0x080F4158, XV_RAW },
	{ 0x080F4158, 0x080F4164, XV_POOL },
	{ 0x080F4164, 0x080F416C, XV_RAW },
	{ 0x080F416C, 0x080F4174, XV_RAW },
	{ 0x080F4174, 0x080F417C, XV_RAW },
	{ 0x080F41A2, 0x080F41A4, XV_RAW },
	{ 0x080F41A4, 0x080F41B0, XV_POOL },
	{ 0x080F41B0, 0x080F41B8, XV_RAW },
	{ 0x080F41B8, 0x080F41C0, XV_RAW },
	{ 0x080F41C0, 0x080F41C8, XV_RAW },
	{ 0x080F41C8, 0x080F41D4, XV_POOL },
	{ 0x080F41D4, 0x080F41DC, XV_RAW },
	{ 0x080F41DC, 0x080F41E4, XV_RAW },
	{ 0x080F41E4, 0x080F41EC, XV_RAW },
	{ 0x080F41EC, 0x080F41FC, XV_POOL },
	{ 0x080F420A, 0x080F420C, XV_RAW },
	{ 0x080F420C, 0x080F4210, XV_POOL },
	{ 0x080F4210, 0x080F421C, XV_TABLE },
	{ 0x080F4266, 0x080F4268, XV_RAW },
	{ 0x080F4268, 0x080F426C, XV_POOL },
	{ 0x080F426C, 0x080F427C, XV_RAW },
	{ 0x080F42F0, 0x080F42F4, XV_POOL },
	{ 0x080F42F4, 0x080F430C, XV_TABLE },
	{ 0x080F4354, 0x080F4358, XV_POOL },
	{ 0x080F4358, 0x080F4368, XV_RAW },
	{ 0x080F441C, 0x080F4420, XV_POOL },
	{ 0x080F4420, 0x080F4424, XV_RAW },
	{ 0x080F4424, 0x080F4428, XV_POOL },
	{ 0x080F4428, 0x080F442C, XV_RAW },
	{ 0x080F445A, 0x080F445C, XV_RAW },
	{ 0x080F445C, 0x080F4460, XV_POOL },
	{ 0x080F4460, 0x080F4484, XV_TABLE },
	{ 0x080F45DE, 0x080F45E2, XV_RAW },
	{ 0x080F475E, 0x080F4760, XV_RAW },
	{ 0x080F4760, 0x080F4764, XV_POOL },
	{ 0x080F4764, 0x080F4788, XV_TABLE },
	{ 0x080F47FE, 0x080F4800, XV_RAW },
	{ 0x080F4800, 0x080F4804, XV_POOL },
	{ 0x080F4804, 0x080F4814, XV_RAW },
	{ 0x080F4AA2, 0x080F4AA4, XV_RAW },
	{ 0x080F4AA4, 0x080F4AA8, XV_POOL },
	{ 0x080F4AA8, 0x080F4AAC, XV_RAW },
	{ 0x080F4B46, 0x080F4B48, XV_RAW },
	{ 0x080F4B48, 0x080F4B4C, XV_POOL },
	{ 0x080F4B4C, 0x080F4B54, XV_RAW },
};
static const XvPair heatman_calls[] = {
	{ 0x0800FE36, 0x0800FE36 },   /* BN6's own, SAME */
	{ 0x08011680, 0x08011680 },   /* BN6's own, SAME */
	{ 0x08001532, 0x08001532 },   /* BN6's own, SAME */
	{ 0x080103BC, 0x080103BC },   /* BN6's own, SAME */
	{ 0x08011690, 0x08011690 },   /* BN6's own, SAME */
	{ 0x080C5D42, 0x080C5D42 },   /* BN6's own, SAME */
	{ 0x0801A15C, 0x0801A15C },   /* BN6's own, SAME */
	{ 0x0801DC7C, 0x0801DC7C },   /* BN6's own, SAME */
	{ 0x0801A082, 0x0801A082 },   /* BN6's own, SAME */
	{ 0x0801A07C, 0x0801A07C },   /* BN6's own, SAME */
	{ 0x0800FE28, 0x0800FE28 },   /* BN6's own, SAME */
	{ 0x0801A166, 0x0801A166 },   /* BN6's own, SAME */
	{ 0x0800FE12, 0x0800FE12 },   /* BN6's own, SAME */
	{ 0x0800E5E2, 0x0800E5E2 },   /* BN6's own, SAME */
	{ 0x0801BB1C, 0x0801BB1C },   /* BN6's own, SAME */
	{ 0x0801A152, 0x0801A152 },   /* BN6's own, SAME */
	{ 0x0801BB46, 0x0801BB46 },   /* BN6's own, SAME */
	{ 0x0800E29C, 0x0800E29C },   /* BN6's own, SAME */
	{ 0x0801A04C, 0x0801A04C },   /* BN6's own, SAME */
	{ 0x08011714, 0x08011714 },   /* BN6's own, SAME */
	{ 0x0800E9DC, 0x0800E9DC },   /* BN6's own, SAME */
	{ 0x0800CC72, 0x0800CC72 },   /* BN6's own, SAME */
	{ 0x0801A074, 0x0801A074 },   /* BN6's own, SAME */
	{ 0x0810AAC4, 0x0810AAC4 },   /* BN6's own, SAME */
	{ 0x080005CC, 0x080005CC },   /* BN6's own, SAME */
	{ 0x0800CCA6, 0x0800CCA6 },   /* BN6's own, SAME */
	{ 0x080302A8, 0x080302A8 },   /* BN6's own, SAME */
	{ 0x080CA7E4, 0x080CA7E4 },   /* BN6's own, SAME */
	{ 0x0800CFA6, 0x0800CFA6 },   /* BN6's own, SAME */
	{ 0x0800D012, 0x0800D012 },   /* BN6's own, SAME */
	{ 0x0800CC86, 0x0800CC86 },   /* BN6's own, SAME */
	{ 0x0800E2CA, 0x0800E2CA },   /* BN6's own, SAME */
	{ 0x0800C8F8, 0x0800C8F8 },   /* BN6's own, SAME */
	{ 0x080CA650, 0x080CA650 },   /* BN6's own, SAME */
	{ 0x080CA4BC, 0x080CA4BC },   /* BN6's own, SAME */
	{ 0x0800CBD8, 0x0800CBD8 },   /* BN6's own, SAME */
	{ 0x0800CCD4, 0x0800CCD4 },   /* BN6's own, SAME */
};
static const XvRef heatman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F3E71), OWN(0x080F41FD), OWN(0x080F42E3), OWN(0x080F444D), OWN(0x080F4751),
};

const XNavi xnavi_control = {
	.code = {
		.name = "HeatMan (BN6's own, the control)",
		.game = XV_GAME_BN6,
		.src_ai = 1,
		.ai = 25,
		.first_id = XN_FIRST_ID + 0x0,
		.home = XN_HOMES + 0x3800,
		.blocks = heatman_blocks, .nblocks = (int)(sizeof heatman_blocks / sizeof *heatman_blocks),
		.data = heatman_data, .ndata = (int)(sizeof heatman_data / sizeof *heatman_data),
		.calls = heatman_calls, .ncalls = (int)(sizeof heatman_calls / sizeof *heatman_calls),
		.states = heatman_states, .nstates = (int)(sizeof heatman_states / sizeof *heatman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), OWN(0x080F3FB9), OWN(0x080F3FB9), OWN(0x080F3FB9), OWN(0x080F3FAB), OWN(0x080F3FB9) },
		.struct1 = 0x080F3D50, .struct2 = 0x080F3D58,
		.names = { 0x101, 0x102, 0x103, 0x104, 0x105, 0x106 },
		.held = 0x080F3D7C,
		.ice = 2,
	},
	.versions = 6,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x08010F6B), .companion_end = BN6(0x080111CB), .hide = BN6(0x080C5D43),
	.sprite_of = -1,
};
/* ---- ToadMan (BN5's: AI 12 there, its ids 0x143, 0x144, 0x145, 0x146), as Navi AI 37 with ids 0x1d8-0x1db ---- */

static const XvBlock toadman_blocks[] = {
	{ 0x080FC398, 0x080FD188, XN_CODE + 0x12C00, 0x9BC4C557u },
	{ 0x080C5386, 0x080C53DE, XN_CODE + 0x13A02, 0x4A1B50AFu },
	{ 0x080D5A40, 0x080D5F9C, XN_CODE + 0x13B00, 0x634A5FAFu },
	{ 0x080D6788, 0x080D68C0, XN_CODE + 0x14100, 0x1432C507u },
};
static const XvData toadman_data[] = {
	{ 0x080FC398, 0x080FC3C8, XV_ZERO },
	{ 0x080FC3C8, 0x080FC3D2, XV_RAW },
	{ 0x080FC3D2, 0x080FC3DC, XV_RAW },
	{ 0x080FC3DC, 0x080FC3E6, XV_RAW },
	{ 0x080FC3E6, 0x080FC3EA, XV_RAW },
	{ 0x080FC3EA, 0x080FC3EE, XV_RAW },
	{ 0x080FC3EE, 0x080FC3F2, XV_RAW },
	{ 0x080FC3F2, 0x080FC3F6, XV_RAW },
	{ 0x080FC3F6, 0x080FC3FA, XV_RAW },
	{ 0x080FC3FA, 0x080FC3FE, XV_RAW },
	{ 0x080FC3FE, 0x080FC402, XV_RAW },
	{ 0x080FC402, 0x080FC406, XV_RAW },
	{ 0x080FC406, 0x080FC418, XV_RAW },
	{ 0x080FC42E, 0x080FC430, XV_RAW },
	{ 0x080FC430, 0x080FC434, XV_POOL },
	{ 0x080FC440, 0x080FC444, XV_POOL },
	{ 0x080FC444, 0x080FC454, XV_TABLE },
	{ 0x080FC532, 0x080FC534, XV_RAW },
	{ 0x080FC534, 0x080FC548, XV_POOL },
	{ 0x080FC794, 0x080FC79C, XV_POOL },
	{ 0x080FC83A, 0x080FC83C, XV_RAW },
	{ 0x080FC83C, 0x080FC840, XV_POOL },
	{ 0x080FC8BC, 0x080FC8C0, XV_POOL },
	{ 0x080FC8C0, 0x080FC8D0, XV_RAW },
	{ 0x080FC904, 0x080FC938, XV_POOL },
	{ 0x080FC946, 0x080FC948, XV_RAW },
	{ 0x080FC948, 0x080FC94C, XV_POOL },
	{ 0x080FC94C, 0x080FC958, XV_TABLE },
	{ 0x080FC9C2, 0x080FC9C4, XV_RAW },
	{ 0x080FC9C4, 0x080FC9C8, XV_POOL },
	{ 0x080FCA06, 0x080FCA08, XV_RAW },
	{ 0x080FCA08, 0x080FCA0C, XV_POOL },
	{ 0x080FCA48, 0x080FCA4C, XV_POOL },
	{ 0x080FCA4C, 0x080FCA5C, XV_TABLE },
	{ 0x080FCB2C, 0x080FCB30, XV_POOL },
	{ 0x080FCB30, 0x080FCB38, XV_TABLE },
	{ 0x080FCBC8, 0x080FCBCC, XV_POOL },
	{ 0x080FCBCC, 0x080FCBF0, XV_TABLE },
	{ 0x080FCCF2, 0x080FCCF4, XV_RAW },
	{ 0x080FCCF4, 0x080FCCF8, XV_POOL },
	{ 0x080FCD4C, 0x080FCD50, XV_POOL },
	{ 0x080FCE0E, 0x080FCE10, XV_RAW },
	{ 0x080FCE10, 0x080FCE14, XV_POOL },
	{ 0x080FCE14, 0x080FCE1C, XV_TABLE },
	{ 0x080FCEE4, 0x080FCEE8, XV_POOL },
	{ 0x080FCEE8, 0x080FCEF8, XV_RAW },
	{ 0x080FD038, 0x080FD03C, XV_POOL },
	{ 0x080FD03C, 0x080FD04C, XV_RAW },
	{ 0x080FD0EE, 0x080FD0F0, XV_RAW },
	{ 0x080FD0F0, 0x080FD0F4, XV_POOL },
	{ 0x080FD0F4, 0x080FD104, XV_RAW },
	{ 0x080FD17E, 0x080FD180, XV_RAW },
	{ 0x080FD180, 0x080FD188, XV_POOL },
	{ 0x080D5A52, 0x080D5A54, XV_RAW },
	{ 0x080D5A54, 0x080D5A58, XV_POOL },
	{ 0x080D5A58, 0x080D5A64, XV_TABLE },
	{ 0x080D5AB4, 0x080D5AB8, XV_POOL },
	{ 0x080D5AB8, 0x080D5AC0, XV_TABLE },
	{ 0x080D5BAA, 0x080D5BAC, XV_RAW },
	{ 0x080D5BAC, 0x080D5BB0, XV_POOL },
	{ 0x080D5BB0, 0x080D5BC0, XV_RAW },
	{ 0x080D5BC0, 0x080D5BD0, XV_POOL },
	{ 0x080D5BE2, 0x080D5BE4, XV_RAW },
	{ 0x080D5BE4, 0x080D5BE8, XV_POOL },
	{ 0x080D5BE8, 0x080D5BF4, XV_TABLE },
	{ 0x080D5C22, 0x080D5C24, XV_RAW },
	{ 0x080D5C24, 0x080D5C2C, XV_POOL },
	{ 0x080D5C86, 0x080D5C88, XV_RAW },
	{ 0x080D5C88, 0x080D5C90, XV_POOL },
	{ 0x080D5C90, 0x080D5CA0, XV_TABLE },
	{ 0x080D5F7A, 0x080D5F7C, XV_RAW },
	{ 0x080D5F7C, 0x080D5F84, XV_POOL },
	{ 0x080D5F84, 0x080D5F8C, XV_RAW },
	{ 0x080D5F8C, 0x080D5F94, XV_RAW },
	{ 0x080D5F94, 0x080D5F9C, XV_POOL },
	{ 0x080D679A, 0x080D679C, XV_RAW },
	{ 0x080D679C, 0x080D67A0, XV_POOL },
	{ 0x080D67A0, 0x080D67AC, XV_TABLE },
	{ 0x080D6840, 0x080D6844, XV_POOL },
	{ 0x080D6844, 0x080D684C, XV_TABLE },
	{ 0x080D68B6, 0x080D68B8, XV_RAW },
	{ 0x080D68B8, 0x080D68C0, XV_POOL },
};
static const XvPair toadman_calls[] = {
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x08002E6A, 0x08003006 },   /* map:bytes, SAME */
	{ 0x08002E8E, 0x0800302A },   /* map:bytes, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB00, 0x0800E6E8 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x08016C3A, 0x0801A146 },   /* project:navis, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08018720, 0x0801BC24 },   /* map:bytes, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08002DBE, 0x08002F5A },   /* near, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800BA08, 0x0800D4AC },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800E0A0, 0x0801031C },   /* map:bytes, SAME */
	{ 0x0800E096, 0x08010312 },   /* map:bytes, SAME */
	{ 0x08002CBE, 0x08002E52 },   /* map:bytes, SAME */
	{ 0x0800D2CC, 0x0800F3E8 },   /* map:calls+bytes, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0801707C, 0x0801A284 },   /* project:navis, LAYOUT */
	{ 0x0800DF42, 0x080101C4 },   /* map:calls, LAYOUT */
	{ 0x08010528, 0x08012EA8 },   /* map:bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x08009326, 0x0800AB46 },   /* map:calls, DIFF (audited) */
	{ 0x0800DD4E, 0x0800FFEE },   /* map:bytes, SAME */
	{ 0x0800D2B0, 0x0800F394 },   /* map:calls+bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800D20C, 0x0800F2F0 },   /* map:bytes, SAME */
	{ 0x0800B6DE, 0x0800D0DC },   /* map:bytes, SAME */
	{ 0x0800C6C2, 0x0800E2C2 },   /* map:calls+bytes, SAME */
};
static const XvPair toadman_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm toadman_imms[] = {
	{ 0x080D5C64, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D5D10, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080D5D6E, THUMB_MOVS, 2, 0, 0xA, 0xE },   /* a panel set to BN5's type 0xa: BN6's 0xe */
	{ 0x080D5E22, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080D5E80, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080D6808, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080FC460, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FC574, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FC5C6, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FC5EC, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FC61A, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FC690, THUMB_CMP, 0, 0, 0xB, 0xD },   /* CurAction 11 compared */
	{ 0x080FC78A, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080FC830, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080FC8E8, THUMB_CMP, 0, 0, 0xA, 0xE },   /* a panel's type compared with BN5's 0xa: BN6's 0xe */
	{ 0x080FC8F8, THUMB_CMP, 0, 0, 0xA, 0xE },   /* a panel's type compared with BN5's 0xa: BN6's 0xe */
	{ 0x080FC97E, THUMB_MOVS, 4, 0, 0x5D, 0xA1 },   /* its effect: 0x5d in BN5, 0xa1 in BN6 */
	{ 0x080FCC2E, THUMB_MOVS, 4, 0, 0x5D, 0xA1 },   /* its effect: 0x5d in BN5, 0xa1 in BN6 */
	{ 0x080FCCC0, THUMB_MOVS, 4, 0, 0x5D, 0xA1 },   /* its effect: 0x5d in BN5, 0xa1 in BN6 */
	{ 0x080FCDC8, THUMB_MOVS, 4, 0, 0x5D, 0xA1 },   /* its effect: 0x5d in BN5, 0xa1 in BN6 */
};
static const XvNumber toadman_numbers[] = {
	{ 0x080D5BC0, 0x01001043, 0x010010A0 },
	{ 0x080D5C24, 0x01041046, 0x010410A1 },
	{ 0x080D68B8, 0x00001044, 0x000010A2 },
	{ 0x080D68BC, 0x0A050000, 0x0A050000 },
	{ 0x080FCD4C, 0x04050201, 0x04050201 },
};
static const XvRef toadman_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080170C5), OWN(0x080FC6F9), OWN(0x080FC79D), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080FC419), OWN(0x080FCA3B), OWN(0x080FCB1F), OWN(0x080FCBBB), OWN(0x080FCE01), OWN(0x080FC939),
};
static const XvSprite toadman_sprites[] = {
	{ 4, 0x43, 4, 0xA0, XN_SHARED_ART + 0x0, 0x800 },
	{ 4, 0x46, 4, 0xA1, XN_SHARED_ART + 0x800, 0x1000 },
	{ 4, 0x44, 4, 0xA2, XN_SHARED_ART + 0x1800, 0x800 },
	{ 5, 0x0D, 5, 0xA0, XN_SHARED_ART + 0x2000, 0x1800 },
	{ 5, 0x1E, 5, 0xA1, XN_SHARED_ART + 0x3800, 0x1000 },
};
static const XvId toadman_effects[] = { { 0x14, 0xA0 }, { 0x5D, 0xA1 } };
static const XvType toadman_types[] = { { 3, 0x7B, XV_TYPE_MOVED }, { 3, 0x7C, XV_TYPE_MOVED }, { 3, 0x81, XV_TYPE_MOVED } };
static const XnSound toadman_sounds[] = {
	{ 0x080D5C52, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080FCB98, 0x080FCB9A, 0x117 },   /* BN6's song 0x117 another */
	{ 0x080FCC64, 0x00000000, 0x0E1 },   /* BN6's song 0xe1 another */
};

static const XNavi bn5_toadman = {
	.code = {
		.name = "ToadMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 12,
		.ai = 37,
		.first_id = XN_FIRST_ID + 0x14,
		.home = XN_HOMES + 0x2800,
		.blocks = toadman_blocks, .nblocks = (int)(sizeof toadman_blocks / sizeof *toadman_blocks),
		.data = toadman_data, .ndata = (int)(sizeof toadman_data / sizeof *toadman_data),
		.calls = toadman_calls, .ncalls = (int)(sizeof toadman_calls / sizeof *toadman_calls),
		.words = toadman_words, .nwords = (int)(sizeof toadman_words / sizeof *toadman_words),
		.imms = toadman_imms, .nimms = (int)(sizeof toadman_imms / sizeof *toadman_imms),
		.numbers = toadman_numbers, .nnumbers = (int)(sizeof toadman_numbers / sizeof *toadman_numbers),
		.states = toadman_states, .nstates = (int)(sizeof toadman_states / sizeof *toadman_states),
		.hooks = { OWN(0x080FC669), BN6(0x080F3BE9), OWN(0x080FC6CD), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080FC2AC, .struct2 = 0x080FC2B4,
		.names = { 0x143, 0x144, 0x145, 0x146, 0x147, 0x148 },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x0C, .element = 0, .weakness = 0, .shadow = true },
		.sprites = toadman_sprites, .nsprites = (int)(sizeof toadman_sprites / sizeof *toadman_sprites),
		.effects = toadman_effects, .neffects = (int)(sizeof toadman_effects / sizeof *toadman_effects),
		.types = toadman_types, .ntypes = (int)(sizeof toadman_types / sizeof *toadman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801AA49), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 12,
	.sounds = toadman_sounds, .nsounds = (int)(sizeof toadman_sounds / sizeof *toadman_sounds),
};
/* ---- KnightMan (BN5's: AI 11 there, its ids 0x13d, 0x13e, 0x13f, 0x140), as Navi AI 36 with ids 0x1d4-0x1d7 ---- */

static const XvBlock knightman_blocks[] = {
	{ 0x080FB764, 0x080FC2AC, XN_CODE + 0xF000, 0x3F21B1FBu },
	{ 0x0800EE72, 0x0800EE82, XN_CODE + 0xFC02, 0x9C0993B9u },
	{ 0x0800F000, 0x0800F00E, XN_CODE + 0xFD00, 0x3DDA94D8u },
	{ 0x080C6FD4, 0x080C7324, XN_CODE + 0xFE00, 0xD0724D66u },
	{ 0x080C7334, 0x080C7446, XN_CODE + 0x10200, 0xA682A0E9u },
	{ 0x080C7650, 0x080C7794, XN_CODE + 0x10400, 0x8F85A796u },
	{ 0x080C77AC, 0x080C77EC, XN_CODE + 0x10600, 0xA8FCBCCBu },
	{ 0x080C80A0, 0x080C827C, XN_CODE + 0x10700, 0x8E650F14u },
	{ 0x080E1014, 0x080E1116, XN_CODE + 0x10900, 0xF92C66C4u },
};
static const XvData knightman_data[] = {
	{ 0x080FB764, 0x080FB790, XV_ZERO },
	{ 0x080FB790, 0x080FB79A, XV_RAW },
	{ 0x080FB79A, 0x080FB7A4, XV_RAW },
	{ 0x080FB7A4, 0x080FB7AE, XV_RAW },
	{ 0x080FB7AE, 0x080FB7B2, XV_RAW },
	{ 0x080FB7B2, 0x080FB7B6, XV_RAW },
	{ 0x080FB7B6, 0x080FB7BA, XV_RAW },
	{ 0x080FB7BA, 0x080FB7BE, XV_RAW },
	{ 0x080FB7BE, 0x080FB7C2, XV_RAW },
	{ 0x080FB7C2, 0x080FB7C8, XV_RAW },
	{ 0x080FB7DE, 0x080FB7E0, XV_RAW },
	{ 0x080FB7E0, 0x080FB7E4, XV_POOL },
	{ 0x080FB7F0, 0x080FB7F4, XV_POOL },
	{ 0x080FB7F4, 0x080FB808, XV_TABLE },
	{ 0x080FBAF4, 0x080FBAF8, XV_POOL },
	{ 0x080FBAF8, 0x080FBB08, XV_RAW },
	{ 0x080FBC12, 0x080FBC14, XV_RAW },
	{ 0x080FBC14, 0x080FBC4C, XV_POOL },
	{ 0x080FBC5A, 0x080FBC5C, XV_RAW },
	{ 0x080FBC5C, 0x080FBC60, XV_POOL },
	{ 0x080FBC60, 0x080FBC70, XV_TABLE },
	{ 0x080FBD3A, 0x080FBD3C, XV_RAW },
	{ 0x080FBD3C, 0x080FBD44, XV_POOL },
	{ 0x080FBE3A, 0x080FBE3C, XV_RAW },
	{ 0x080FBE3C, 0x080FBE48, XV_POOL },
	{ 0x080FBE8E, 0x080FBE90, XV_RAW },
	{ 0x080FBE90, 0x080FBE94, XV_POOL },
	{ 0x080FBE94, 0x080FBEA4, XV_TABLE },
	{ 0x080FBFA8, 0x080FBFAC, XV_POOL },
	{ 0x080FBFAC, 0x080FBFC0, XV_TABLE },
	{ 0x080FC0EA, 0x080FC0EC, XV_RAW },
	{ 0x080FC0EC, 0x080FC0F0, XV_POOL },
	{ 0x080FC0F0, 0x080FC0FC, XV_TABLE },
	{ 0x080FC212, 0x080FC214, XV_RAW },
	{ 0x080FC214, 0x080FC218, XV_POOL },
	{ 0x080FC218, 0x080FC228, XV_RAW },
	{ 0x080FC2AA, 0x080FC2AC, XV_RAW },
	{ 0x080C6FE6, 0x080C6FE8, XV_RAW },
	{ 0x080C6FE8, 0x080C6FEC, XV_POOL },
	{ 0x080C6FEC, 0x080C6FF8, XV_TABLE },
	{ 0x080C706A, 0x080C706C, XV_RAW },
	{ 0x080C706C, 0x080C7070, XV_POOL },
	{ 0x080C7070, 0x080C7078, XV_TABLE },
	{ 0x080C70D6, 0x080C70D8, XV_RAW },
	{ 0x080C70D8, 0x080C70DC, XV_POOL },
	{ 0x080C70DC, 0x080C70E4, XV_TABLE },
	{ 0x080C72F8, 0x080C7300, XV_POOL },
	{ 0x080C7300, 0x080C7310, XV_RAW },
	{ 0x080C7310, 0x080C7320, XV_RAW },
	{ 0x080C7320, 0x080C7324, XV_POOL },
	{ 0x080C7346, 0x080C7348, XV_RAW },
	{ 0x080C7348, 0x080C734C, XV_POOL },
	{ 0x080C734C, 0x080C7358, XV_TABLE },
	{ 0x080C7668, 0x080C766C, XV_POOL },
	{ 0x080C766C, 0x080C7678, XV_TABLE },
	{ 0x080C77E2, 0x080C77E4, XV_RAW },
	{ 0x080C77E4, 0x080C77EC, XV_POOL },
	{ 0x080C80AE, 0x080C80B0, XV_RAW },
	{ 0x080C80B0, 0x080C80B4, XV_POOL },
	{ 0x080C80B4, 0x080C80C0, XV_TABLE },
	{ 0x080C819E, 0x080C81A0, XV_RAW },
	{ 0x080C81A0, 0x080C81A4, XV_POOL },
	{ 0x080C81A4, 0x080C81A8, XV_TABLE },
	{ 0x080C8200, 0x080C8204, XV_POOL },
	{ 0x080C8238, 0x080C823C, XV_POOL },
	{ 0x080C823C, 0x080C825C, XV_RAW },
	{ 0x080E1022, 0x080E1024, XV_RAW },
	{ 0x080E1024, 0x080E1028, XV_POOL },
	{ 0x080E1028, 0x080E1034, XV_TABLE },
	{ 0x080E1078, 0x080E107C, XV_RAW },
	{ 0x080E1090, 0x080E1094, XV_POOL },
	{ 0x080E1094, 0x080E109C, XV_TABLE },
	{ 0x080E10C2, 0x080E10C4, XV_RAW },
	{ 0x080E10C4, 0x080E10C8, XV_POOL },
};
static const XvPair knightman_calls[] = {
	{ 0x080C3808, 0x080C5D18 },   /* project:navis, DIFF (audited) */
	{ 0x080C382C, 0x080C5D38 },   /* project:navis, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x080187CC, 0x0801BCD0 },   /* map:bytes, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x08001490, 0x0800151C },   /* map:calls+bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002C80, 0x08002E14 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800D250, 0x0800F334 },   /* map:bytes, SAME */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800BA08, 0x0800D4AC },   /* map:bytes, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x080C386C, 0x080C5D80 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0800E0EC, 0x08010368 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08011318, 0x08013CC4 },   /* map:calls, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x080C3856, 0x080C5D6A },   /* map:bytes, SAME */
	{ 0x080C3840, 0x080C5D54 },   /* map:bytes, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800B050, 0x0800C998 },   /* map:calls, DIFF (audited) */
};
static const XvPair knightman_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x08005CD0, 0x080065E0 },
	{ 0x08005D50, 0x08006660 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm knightman_imms[] = {
	{ 0x0800EE74, THUMB_MOVS, 4, 0, 0x1, 0x17 },   /* its overlay's kind: 1 in BN5, 23 in BN6 (its own, past BN6's) */
	{ 0x080C6FFC, THUMB_MOVS, 2, 0, 0x47, 0xA5 },   /* its sprite: list 4 0x47 in BN5, list 4 0xa5 in BN6 */
	{ 0x080C7086, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
	{ 0x080C735C, THUMB_MOVS, 2, 0, 0x47, 0xA5 },   /* its sprite: list 4 0x47 in BN5, list 4 0xa5 in BN6 */
	{ 0x080C739A, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
	{ 0x080C73D4, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C767C, THUMB_MOVS, 2, 0, 0x47, 0xA5 },   /* its sprite: list 4 0x47 in BN5, list 4 0xa5 in BN6 */
	{ 0x080C7684, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its sprite: list 3 0x5 in BN5, list 4 0xa3 in BN6 (its list's offset) */
	{ 0x080C7686, THUMB_MOVS, 2, 0, 0x5, 0xA3 },   /* its sprite: list 3 0x5 in BN5, list 4 0xa3 in BN6 */
	{ 0x080C770E, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C80EE, THUMB_MOVS, 2, 0, 0xE, 0xA4 },   /* its sprite: list 4 0xe in BN5, list 4 0xa4 in BN6 */
	{ 0x080C816A, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C81E4, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080E1042, THUMB_MOVS, 2, 0, 0xE, 0xA4 },   /* its sprite: list 4 0xe in BN5, list 4 0xa4 in BN6 */
	{ 0x080FB834, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080FB948, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FB982, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FB9C4, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FBA28, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080FBB36, THUMB_MOVS, 0, 0, 0x74, 0xA2 },   /* the chip it writes for its attack: BN5's 0x74, BN6's 0xa2 (PanlGrab) */
};
static const XvNumber knightman_numbers[] = {
	{ 0x080FBE44, 0x0600FF01, 0x0600FF01 },
};
static const XvRef knightman_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080FB7C9), OWN(0x080FBC4D), OWN(0x080FBE81), OWN(0x080FBF9B), OWN(0x080FC0DD),
};
static const XvSprite knightman_sprites[] = {
	{ 3, 0x05, 4, 0xA3, XN_SHARED_ART + 0x4800, 0x800 },
	{ 4, 0x0E, 4, 0xA4, XN_SHARED_ART + 0x5000, 0x800 },
	{ 4, 0x47, 4, 0xA5, XN_SHARED_ART + 0x5800, 0x1800 },
	{ 5, 0x02, 5, 0xA2, XN_SHARED_ART + 0x7000, 0x1800 },
	{ 5, 0x0D, 5, 0xA0, XN_SHARED_ART + 0x2000, 0x1800 },
};
static const XvId knightman_effects[] = { { 0x02, 0xA2 }, { 0x14, 0xA0 } };
static const XvType knightman_types[] = { { 3, 0x14, XV_TYPE_MOVED }, { 3, 0x15, XV_TYPE_MOVED }, { 3, 0x17, XV_TYPE_MOVED }, { 3, 0x1D, XV_TYPE_MOVED }, { 4, 0x09, XV_TYPE_MOVED } };
static const XnSound knightman_sounds[] = {
	{ 0x080FBE08, 0x00000000, 0x0C0 },   /* BN6's song 0xc0 another */
};
static const XnOverlay knightman_overlay = { 0x080C3626, XN_ART + 0x1000, 0x6BBF3ED3u, 0x14, 23, 4, 0xA5 };   /* BN5's kind 1, its sprite 4:0x47 */

static const XNavi bn5_knightman = {
	.code = {
		.name = "KnightMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 11,
		.ai = 36,
		.first_id = XN_FIRST_ID + 0x10,
		.home = XN_HOMES + 0x2000,
		.blocks = knightman_blocks, .nblocks = (int)(sizeof knightman_blocks / sizeof *knightman_blocks),
		.data = knightman_data, .ndata = (int)(sizeof knightman_data / sizeof *knightman_data),
		.calls = knightman_calls, .ncalls = (int)(sizeof knightman_calls / sizeof *knightman_calls),
		.words = knightman_words, .nwords = (int)(sizeof knightman_words / sizeof *knightman_words),
		.imms = knightman_imms, .nimms = (int)(sizeof knightman_imms / sizeof *knightman_imms),
		.numbers = knightman_numbers, .nnumbers = (int)(sizeof knightman_numbers / sizeof *knightman_numbers),
		.states = knightman_states, .nstates = (int)(sizeof knightman_states / sizeof *knightman_states),
		.hooks = { OWN(0x080FBA9B), BN6(0x080F3BE9), OWN(0x080FBAB5), OWN(0x080FBB09), OWN(0x080FBB09), OWN(0x080FBB09), OWN(0x080FBB09), OWN(0x080FBB09) },
		.struct1 = 0x080FB678, .struct2 = 0x080FB680,
		.names = { 0x13D, 0x13E, 0x13F, 0x140, 0x141, 0x142 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x0B, .element = 0, .weakness = 0, .shadow = true },
		.sprites = knightman_sprites, .nsprites = (int)(sizeof knightman_sprites / sizeof *knightman_sprites),
		.effects = knightman_effects, .neffects = (int)(sizeof knightman_effects / sizeof *knightman_effects),
		.types = knightman_types, .ntypes = (int)(sizeof knightman_types / sizeof *knightman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = OWN(0x0800EE73), .companion_end = OWN(0x0800F001), .hide = BN6(0x080C5D43),
	.sprite_of = 11,
	.overlay = &knightman_overlay,
	.sounds = knightman_sounds, .nsounds = (int)(sizeof knightman_sounds / sizeof *knightman_sounds),
};
/* ---- ShadowMan (BN5's: AI 8 there, its ids 0x12b, 0x12c, 0x12d, 0x12e), as Navi AI 33 with ids 0x1c8-0x1cb ---- */

static const XvBlock shadowman_blocks[] = {
	{ 0x080F9228, 0x080F9D80, XN_CODE + 0x3C00, 0xA464FF1Bu },
	{ 0x080B946C, 0x080B9A5C, XN_CODE + 0x4800, 0x616BFFC5u },
	{ 0x080C544C, 0x080C5592, XN_CODE + 0x4E00, 0xAF9CBA1Eu },
};
static const XvData shadowman_data[] = {
	{ 0x080F9228, 0x080F9254, XV_ZERO },
	{ 0x080F9254, 0x080F925E, XV_RAW },
	{ 0x080F925E, 0x080F9268, XV_RAW },
	{ 0x080F9268, 0x080F9272, XV_RAW },
	{ 0x080F9272, 0x080F9276, XV_RAW },
	{ 0x080F9276, 0x080F927A, XV_RAW },
	{ 0x080F927A, 0x080F927E, XV_RAW },
	{ 0x080F927E, 0x080F9282, XV_RAW },
	{ 0x080F9282, 0x080F9286, XV_RAW },
	{ 0x080F9286, 0x080F928C, XV_RAW },
	{ 0x080F92B2, 0x080F92B4, XV_RAW },
	{ 0x080F92B4, 0x080F92B8, XV_POOL },
	{ 0x080F92B8, 0x080F92C8, XV_TABLE },
	{ 0x080F94BE, 0x080F94C0, XV_RAW },
	{ 0x080F94C0, 0x080F950C, XV_POOL },
	{ 0x080F951A, 0x080F951C, XV_RAW },
	{ 0x080F951C, 0x080F9520, XV_POOL },
	{ 0x080F9520, 0x080F952C, XV_TABLE },
	{ 0x080F9618, 0x080F961C, XV_POOL },
	{ 0x080F961C, 0x080F9628, XV_TABLE },
	{ 0x080F96E6, 0x080F96E8, XV_RAW },
	{ 0x080F96E8, 0x080F96EC, XV_POOL },
	{ 0x080F96EC, 0x080F9700, XV_TABLE },
	{ 0x080F98A4, 0x080F98A8, XV_POOL },
	{ 0x080F98A8, 0x080F98C4, XV_TABLE },
	{ 0x080F9BC4, 0x080F9BC8, XV_POOL },
	{ 0x080F9BC8, 0x080F9BD8, XV_RAW },
	{ 0x080F9C70, 0x080F9C74, XV_POOL },
	{ 0x080F9C74, 0x080F9C84, XV_RAW },
	{ 0x080F9D4C, 0x080F9D50, XV_POOL },
	{ 0x080F9D50, 0x080F9D58, XV_RAW },
	{ 0x080F9D74, 0x080F9D80, XV_POOL },
	{ 0x080B9484, 0x080B9488, XV_POOL },
	{ 0x080B9488, 0x080B9494, XV_TABLE },
	{ 0x080B950E, 0x080B9510, XV_RAW },
	{ 0x080B9510, 0x080B9514, XV_POOL },
	{ 0x080B9576, 0x080B9578, XV_RAW },
	{ 0x080B9578, 0x080B957C, XV_POOL },
	{ 0x080B957C, 0x080B9598, XV_TABLE },
	{ 0x080B967A, 0x080B967C, XV_RAW },
	{ 0x080B967C, 0x080B9680, XV_POOL },
	{ 0x080B9680, 0x080B9688, XV_TABLE },
	{ 0x080B9756, 0x080B9758, XV_RAW },
	{ 0x080B9758, 0x080B975C, XV_POOL },
	{ 0x080B975C, 0x080B9770, XV_TABLE },
	{ 0x080B98E0, 0x080B98E4, XV_POOL },
	{ 0x080B98E4, 0x080B98F0, XV_TABLE },
	{ 0x080B9A02, 0x080B9A04, XV_RAW },
	{ 0x080B9A04, 0x080B9A08, XV_POOL },
	{ 0x080B9A08, 0x080B9A18, XV_RAW },
	{ 0x080B9A50, 0x080B9A5C, XV_POOL },
	{ 0x080C545E, 0x080C5460, XV_RAW },
	{ 0x080C5460, 0x080C5464, XV_POOL },
	{ 0x080C5464, 0x080C5470, XV_TABLE },
};
static const XvPair shadowman_calls[] = {
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800D250, 0x0800F334 },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x0800CDEC, 0x0800EB6C },   /* map:bytes, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x080C53DE, 0x080115CE },   /* map:bytes, SAME */
	{ 0x08006BC2, 0x080077B4 },   /* map:bytes, SAME */
	{ 0x0802EFFE, 0x0802EF5C },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08016162, 0x080198CE },   /* map:calls, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x080142D0, 0x08017888 },   /* navis, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08013010, 0x08016218 },   /* map:calls, SAME */
	{ 0x080DF2A2, 0x080DF18A },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800CEE6, 0x0800EC66 },   /* map:bytes, SAME */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
};
static const XvPair shadowman_words[] = {
	{ 0x08012A21, 0x08015C49 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm shadowman_imms[] = {
	{ 0x080B9498, THUMB_MOVS, 2, 0, 0x8, 0x21 },   /* its sprite: list 2 0x8 in BN5, list 2 0x21 in BN6 */
	{ 0x080B9880, THUMB_ADDS, 4, 0, 0x18, 0xA4 },   /* its effect: 0x18 in BN5, 0xa4 in BN6 */
	{ 0x080B9884, THUMB_ADDS, 4, 0, 0x17, 0xA3 },   /* its effect: 0x17 in BN5, 0xa3 in BN6 */
	{ 0x080C5474, THUMB_MOVS, 2, 0, 0x42, 0xA6 },   /* its sprite: list 4 0x42 in BN5, list 4 0xa6 in BN6 */
	{ 0x080C5538, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080F9356, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F9392, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F93CE, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F94B6, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F973A, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
	{ 0x080F99BE, THUMB_ADDS, 4, 0, 0x18, 0xA4 },   /* its effect: 0x18 in BN5, 0xa4 in BN6 */
};
static const XvNumber shadowman_numbers[] = {
	{ 0x080B9A50, 0x0704FF01, 0x0704FF01 },
	{ 0x080B9A54, 0x0704FF02, 0x0704FF02 },
	{ 0x080F9D74, 0x0704FF01, 0x0704FF01 },
};
static const XvRef shadowman_states[] = {
	BN6(0x08016381), OWN(0x080F9419), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F928D), OWN(0x080F950D), OWN(0x080F960B), OWN(0x080F96D9), OWN(0x080F9897),
};
static const XvSprite shadowman_sprites[] = {
	{ 4, 0x42, 4, 0xA6, XN_SHARED_ART + 0x8800, 0x1800 },
	{ 5, 0x02, 5, 0xA2, XN_SHARED_ART + 0x7000, 0x1800 },
	{ 3, 0x14, 4, 0xA7, XN_SHARED_ART + 0xA000, 0x2800 },
};
static const XvId shadowman_effects[] = { { 0x02, 0xA2 }, { 0x17, 0xA3 }, { 0x18, 0xA4 } };
static const XvType shadowman_types[] = { { 1, 0x04, XV_TYPE_MOVED }, { 3, 0x07, XV_TYPE_MOVED } };
static const XnSound shadowman_sounds[] = {
	{ 0x080F97B8, 0x00000000, 0x0B7 },   /* BN6's song 0xb7 another */
};
static const XvType shadowman_needs[] = { { 1, 0x05, XV_TYPE_MOVED } };   /* the moved objects' (xnavi_objects.h) */

static const XNavi bn5_shadowman = {
	.code = {
		.name = "ShadowMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 8,
		.ai = 33,
		.first_id = XN_FIRST_ID + 0x4,
		.home = XN_HOMES + 0x800,
		.blocks = shadowman_blocks, .nblocks = (int)(sizeof shadowman_blocks / sizeof *shadowman_blocks),
		.data = shadowman_data, .ndata = (int)(sizeof shadowman_data / sizeof *shadowman_data),
		.calls = shadowman_calls, .ncalls = (int)(sizeof shadowman_calls / sizeof *shadowman_calls),
		.words = shadowman_words, .nwords = (int)(sizeof shadowman_words / sizeof *shadowman_words),
		.imms = shadowman_imms, .nimms = (int)(sizeof shadowman_imms / sizeof *shadowman_imms),
		.numbers = shadowman_numbers, .nnumbers = (int)(sizeof shadowman_numbers / sizeof *shadowman_numbers),
		.states = shadowman_states, .nstates = (int)(sizeof shadowman_states / sizeof *shadowman_states),
		.hooks = { OWN(0x080F9425), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080F913C, .struct2 = 0x080F9144,
		.names = { 0x12B, 0x12C, 0x12D, 0x12E, 0x12F, 0x130 },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x08, .element = 0, .weakness = 0, .shadow = true },
		.sprites = shadowman_sprites, .nsprites = (int)(sizeof shadowman_sprites / sizeof *shadowman_sprites),
		.effects = shadowman_effects, .neffects = (int)(sizeof shadowman_effects / sizeof *shadowman_effects),
		.types = shadowman_types, .ntypes = (int)(sizeof shadowman_types / sizeof *shadowman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 8,
	.sounds = shadowman_sounds, .nsounds = (int)(sizeof shadowman_sounds / sizeof *shadowman_sounds),
	.needs = shadowman_needs, .nneeds = (int)(sizeof shadowman_needs / sizeof *shadowman_needs),
};
/* ---- NumberMan (BN5's: AI 9 there, its ids 0x131, 0x132, 0x133, 0x134), as Navi AI 34 with ids 0x1cc-0x1cf ---- */

static const XvBlock numberman_blocks[] = {
	{ 0x080F9E6C, 0x080FAA88, XN_CODE + 0x7800, 0x16B437C4u },
	{ 0x0800A2F0, 0x0800A326, XN_CODE + 0x8500, 0x0337D8E7u },
	{ 0x0800EE66, 0x0800EE72, XN_CODE + 0x8602, 0x2FB638AFu },
	{ 0x0800EFF2, 0x0800F000, XN_CODE + 0x8702, 0xE7D02240u },
	{ 0x080C33A8, 0x080C34E0, XN_CODE + 0x8800, 0x217F5537u },
	{ 0x080C5014, 0x080C544C, XN_CODE + 0x8A00, 0x76FE03D1u },
	{ 0x080D78B4, 0x080D7A72, XN_CODE + 0x8F00, 0xB98CC4DCu },
	{ 0x080D7A80, 0x080D8014, XN_CODE + 0x9100, 0x4FEF51BEu },
	{ 0x080D8CD8, 0x080D8F1C, XN_CODE + 0x9700, 0xA2627512u },
	{ 0x080EABAC, 0x080EACB4, XN_CODE + 0x9A00, 0x5D2B4ED6u },
};
static const XvData numberman_data[] = {
	{ 0x080F9E6C, 0x080F9E98, XV_ZERO },
	{ 0x080F9EAE, 0x080F9EB0, XV_RAW },
	{ 0x080F9EB0, 0x080F9EB4, XV_POOL },
	{ 0x080F9EC8, 0x080F9ECC, XV_POOL },
	{ 0x080F9ECC, 0x080F9EE4, XV_TABLE },
	{ 0x080F9F30, 0x080F9F34, XV_POOL },
	{ 0x080F9FFE, 0x080FA000, XV_RAW },
	{ 0x080FA000, 0x080FA004, XV_POOL },
	{ 0x080FA004, 0x080FA008, XV_RAW },
	{ 0x080FA008, 0x080FA00C, XV_POOL },
	{ 0x080FA00C, 0x080FA010, XV_RAW },
	{ 0x080FA030, 0x080FA034, XV_POOL },
	{ 0x080FA0F2, 0x080FA0F4, XV_RAW },
	{ 0x080FA0F4, 0x080FA0F8, XV_POOL },
	{ 0x080FA0F8, 0x080FA0FC, XV_RAW },
	{ 0x080FA106, 0x080FA108, XV_RAW },
	{ 0x080FA108, 0x080FA10C, XV_POOL },
	{ 0x080FA10C, 0x080FA110, XV_RAW },
	{ 0x080FA11A, 0x080FA11C, XV_RAW },
	{ 0x080FA11C, 0x080FA120, XV_POOL },
	{ 0x080FA120, 0x080FA124, XV_RAW },
	{ 0x080FA146, 0x080FA148, XV_RAW },
	{ 0x080FA148, 0x080FA154, XV_POOL },
	{ 0x080FA154, 0x080FA15E, XV_RAW },
	{ 0x080FA15E, 0x080FA168, XV_RAW },
	{ 0x080FA168, 0x080FA174, XV_RAW },
	{ 0x080FA174, 0x080FA178, XV_POOL },
	{ 0x080FA178, 0x080FA17C, XV_RAW },
	{ 0x080FA17C, 0x080FA198, XV_POOL },
	{ 0x080FA1A6, 0x080FA1A8, XV_RAW },
	{ 0x080FA1A8, 0x080FA1AC, XV_POOL },
	{ 0x080FA1AC, 0x080FA1B8, XV_TABLE },
	{ 0x080FA202, 0x080FA204, XV_RAW },
	{ 0x080FA204, 0x080FA208, XV_POOL },
	{ 0x080FA208, 0x080FA218, XV_RAW },
	{ 0x080FA28C, 0x080FA290, XV_POOL },
	{ 0x080FA290, 0x080FA2B0, XV_TABLE },
	{ 0x080FA30A, 0x080FA30C, XV_RAW },
	{ 0x080FA30C, 0x080FA310, XV_POOL },
	{ 0x080FA310, 0x080FA320, XV_RAW },
	{ 0x080FA426, 0x080FA428, XV_RAW },
	{ 0x080FA428, 0x080FA42C, XV_POOL },
	{ 0x080FA48E, 0x080FA490, XV_RAW },
	{ 0x080FA490, 0x080FA494, XV_POOL },
	{ 0x080FA4CC, 0x080FA4D0, XV_POOL },
	{ 0x080FA4D0, 0x080FA4D4, XV_RAW },
	{ 0x080FA504, 0x080FA508, XV_POOL },
	{ 0x080FA508, 0x080FA518, XV_TABLE },
	{ 0x080FA540, 0x080FA544, XV_POOL },
	{ 0x080FA5E2, 0x080FA5E4, XV_RAW },
	{ 0x080FA5E4, 0x080FA5E8, XV_POOL },
	{ 0x080FA5E8, 0x080FA604, XV_TABLE },
	{ 0x080FA660, 0x080FA664, XV_POOL },
	{ 0x080FA664, 0x080FA674, XV_RAW },
	{ 0x080FA760, 0x080FA764, XV_POOL },
	{ 0x080FA764, 0x080FA774, XV_RAW },
	{ 0x080FA774, 0x080FA77C, XV_POOL },
	{ 0x080FA858, 0x080FA85C, XV_POOL },
	{ 0x080FA85C, 0x080FA864, XV_RAW },
	{ 0x080FAA38, 0x080FAA3C, XV_POOL },
	{ 0x080FAA3C, 0x080FAA40, XV_RAW },
	{ 0x080FAA40, 0x080FAA44, XV_POOL },
	{ 0x080FAA44, 0x080FAA54, XV_RAW },
	{ 0x080FAA86, 0x080FAA88, XV_RAW },
	{ 0x080C33A8, 0x080C33AC, XV_RAW },
	{ 0x080C33BA, 0x080C33BC, XV_RAW },
	{ 0x080C33BC, 0x080C33C0, XV_POOL },
	{ 0x080C33C0, 0x080C33CC, XV_TABLE },
	{ 0x080C34A0, 0x080C34A4, XV_RAW },
	{ 0x080C34DC, 0x080C34E0, XV_POOL },
	{ 0x080C5038, 0x080C503C, XV_POOL },
	{ 0x080C503C, 0x080C5048, XV_TABLE },
	{ 0x080C514A, 0x080C514C, XV_RAW },
	{ 0x080C514C, 0x080C5154, XV_POOL },
	{ 0x080C5154, 0x080C5164, XV_TABLE },
	{ 0x080C5164, 0x080C516C, XV_RAW },
	{ 0x080C53DE, 0x080C543C, XV_RAW },
	{ 0x080C543C, 0x080C544C, XV_POOL },
	{ 0x080D78B4, 0x080D78BC, XV_RAW },
	{ 0x080D78CE, 0x080D78D0, XV_RAW },
	{ 0x080D78D0, 0x080D78D4, XV_POOL },
	{ 0x080D78D4, 0x080D78E0, XV_TABLE },
	{ 0x080D796C, 0x080D7970, XV_POOL },
	{ 0x080D7970, 0x080D797C, XV_TABLE },
	{ 0x080D7B32, 0x080D7B34, XV_RAW },
	{ 0x080D7B34, 0x080D7B38, XV_POOL },
	{ 0x080D7B38, 0x080D7B40, XV_RAW },
	{ 0x080D7B40, 0x080D7B44, XV_POOL },
	{ 0x080D7B44, 0x080D7B4C, XV_RAW },
	{ 0x080D7BBA, 0x080D7BBC, XV_RAW },
	{ 0x080D7BBC, 0x080D7BC0, XV_POOL },
	{ 0x080D7BC0, 0x080D7BD0, XV_RAW },
	{ 0x080D7C50, 0x080D7C54, XV_POOL },
	{ 0x080D7C54, 0x080D7C64, XV_RAW },
	{ 0x080D7C64, 0x080D7C68, XV_POOL },
	{ 0x080D7C68, 0x080D7C70, XV_RAW },
	{ 0x080D7C9C, 0x080D7CA0, XV_POOL },
	{ 0x080D7CA0, 0x080D7CA4, XV_RAW },
	{ 0x080D7CA4, 0x080D7CA8, XV_POOL },
	{ 0x080D7CA8, 0x080D7CFC, XV_RAW },
	{ 0x080D7D5C, 0x080D7D60, XV_POOL },
	{ 0x080D7D60, 0x080D7DB4, XV_RAW },
	{ 0x080D7DB4, 0x080D7DB8, XV_POOL },
	{ 0x080D7DB8, 0x080D7DC8, XV_RAW },
	{ 0x080D7DC8, 0x080D7DCC, XV_POOL },
	{ 0x080D7DDE, 0x080D7DE0, XV_RAW },
	{ 0x080D7DE0, 0x080D7DE4, XV_POOL },
	{ 0x080D7DE4, 0x080D7DF0, XV_TABLE },
	{ 0x080D7E64, 0x080D7E68, XV_POOL },
	{ 0x080D7E68, 0x080D7E74, XV_TABLE },
	{ 0x080D7EA2, 0x080D7EA4, XV_RAW },
	{ 0x080D7EA4, 0x080D7EA8, XV_POOL },
	{ 0x080D7EA8, 0x080D7EB2, XV_RAW },
	{ 0x080D7EF4, 0x080D7EF8, XV_POOL },
	{ 0x080D7EF8, 0x080D7F0C, XV_RAW },
	{ 0x080D7F6A, 0x080D7F6C, XV_RAW },
	{ 0x080D7F6C, 0x080D7F70, XV_POOL },
	{ 0x080D7F70, 0x080D7F78, XV_RAW },
	{ 0x080D7F78, 0x080D7F80, XV_POOL },
	{ 0x080D8002, 0x080D8004, XV_RAW },
	{ 0x080D8004, 0x080D8008, XV_POOL },
	{ 0x080D8008, 0x080D8010, XV_RAW },
	{ 0x080D8010, 0x080D8014, XV_POOL },
	{ 0x080D8CEA, 0x080D8CEC, XV_RAW },
	{ 0x080D8CEC, 0x080D8CF0, XV_POOL },
	{ 0x080D8CF0, 0x080D8CFC, XV_TABLE },
	{ 0x080D8D94, 0x080D8D98, XV_POOL },
	{ 0x080D8D98, 0x080D8DB0, XV_RAW },
	{ 0x080D8DB0, 0x080D8DB4, XV_POOL },
	{ 0x080D8DB4, 0x080D8DC4, XV_RAW },
	{ 0x080D8E34, 0x080D8E38, XV_POOL },
	{ 0x080D8E38, 0x080D8E44, XV_TABLE },
	{ 0x080D8F14, 0x080D8F1C, XV_POOL },
	{ 0x080EABBA, 0x080EABBC, XV_RAW },
	{ 0x080EABBC, 0x080EABC0, XV_POOL },
	{ 0x080EABC0, 0x080EABCC, XV_TABLE },
	{ 0x080EABDA, 0x080EABDC, XV_RAW },
	{ 0x080EABDC, 0x080EABE0, XV_POOL },
	{ 0x080EABE0, 0x080EABF0, XV_TABLE },
	{ 0x080EABFE, 0x080EAC00, XV_RAW },
	{ 0x080EAC00, 0x080EAC04, XV_POOL },
	{ 0x080EAC04, 0x080EAC0C, XV_TABLE },
};
static const XvPair numberman_calls[] = {
	{ 0x08009F2E, 0x0800B892 },   /* map:bytes, SAME */
	{ 0x08005A28, 0x08006270 },   /* map:bytes, SAME */
	{ 0x08005AAE, 0x080062F8 },   /* map:calls+bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002BF8, 0x08002D8C },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08002D40, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002C34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002C20, 0x08002DB4 },   /* near, SAME */
	{ 0x08002DA2, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002D66, 0x08002F02 },   /* near, SAME */
	{ 0x08002B4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002AEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08001224, 0x08001288 },   /* map:bytes, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ 0x08018838, 0x0801BD3C },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x080E34E6, 0x080E4582 },   /* map:bytes, SAME */
	{ 0x0800A50A, 0x0800BF16 },   /* map:bytes, SAME */
	{ 0x080E08D8, 0x080E1B1C },   /* search, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x08000B20, 0x08000C00 },   /* map:bytes, SAME */
	{ 0x08002E0A, 0x08002FA6 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
};
static const XvPair numberman_words[] = {
	{ 0x0405FF0F, 0x0405FF0F },
	{ 0x0800325D, 0x08003459 },
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm numberman_imms[] = {
	{ 0x080C504A, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its sprite: list 3 0xa in BN5, list 4 0xa8 in BN6 (its list's offset) */
	{ 0x080C504C, THUMB_MOVS, 2, 0, 0xA, 0xA8 },   /* its sprite: list 3 0xa in BN5, list 4 0xa8 in BN6 */
	{ 0x080C50EC, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C5114, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080C51FA, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
	{ 0x080D78F6, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its sprite: list 3 0x22 in BN5, list 4 0xa9 in BN6 (its list's offset) */
	{ 0x080D78F8, THUMB_MOVS, 2, 0, 0x22, 0xA9 },   /* its sprite: list 3 0x22 in BN5, list 4 0xa9 in BN6 */
	{ 0x080D7E0C, THUMB_MOVS, 2, 0, 0x25, 0xAA },   /* its sprite: list 4 0x25 in BN5, list 4 0xaa in BN6 */
	{ 0x080D8D04, THUMB_MOVS, 2, 0, 0x5, 0xAB },   /* its sprite: list 4 0x5 in BN5, list 4 0xab in BN6 */
	{ 0x080D8DEC, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D8E10, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080F9F28, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080F9F86, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F9FCA, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F9FEC, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FA05C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
};
static const XvByte numberman_bytes[] = {
	{ 0x080C33A9, 0x06, 0xAC },   /* 080C33E0: its sprite table's entry 0 */
	{ 0x080C33AB, 0x01, 0xAD },   /* 080C33E0: its sprite table's entry 1 */
};
static const XvNumber numberman_numbers[] = {
	{ 0x080D7F78, 0x04050001, 0x04050001 },
};
static const XvRef numberman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F9E99), OWN(0x080FA199), OWN(0x080FA27F), OWN(0x080FA4F7), OWN(0x080FA5D5),
};
static const XvSprite numberman_sprites[] = {
	{ 3, 0x0A, 4, 0xA8, XN_SHARED_ART + 0xC800, 0x1800 },
	{ 3, 0x22, 4, 0xA9, XN_SHARED_ART + 0xE000, 0x800 },
	{ 4, 0x25, 4, 0xAA, XN_SHARED_ART + 0xE800, 0x1000 },
	{ 4, 0x05, 4, 0xAB, XN_SHARED_ART + 0xF800, 0x1000 },
	{ 5, 0x02, 5, 0xA2, XN_SHARED_ART + 0x7000, 0x1800 },
	{ 5, 0x0D, 5, 0xA0, XN_SHARED_ART + 0x2000, 0x1800 },
	{ 4, 0x06, 4, 0xAC, XN_SHARED_ART + 0x10800, 0x800 },
	{ 4, 0x01, 4, 0xAD, XN_SHARED_ART + 0x11000, 0x800 },
};
static const XvId numberman_effects[] = { { 0x02, 0xA2 }, { 0x14, 0xA0 } };
static const XvType numberman_types[] = { { 1, 0x54, XV_TYPE_MOVED }, { 3, 0x05, XV_TYPE_MOVED }, { 3, 0x8B, XV_TYPE_MOVED }, { 3, 0x8C, XV_TYPE_MOVED }, { 3, 0x93, XV_TYPE_MOVED }, { 4, 0x8E, XV_TYPE_MOVED } };
static const XnSound numberman_sounds[] = {
	{ 0x080C5130, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080C5252, 0x080C5254, 0x115 },   /* BN6's song 0x115 another */
	{ 0x080D7EBE, 0x080D7EC0, 0x105 },   /* BN6's song 0x105 another */
	{ 0x080D8E24, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
};

static const XNavi bn5_numberman = {
	.code = {
		.name = "NumberMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 9,
		.ai = 34,
		.first_id = XN_FIRST_ID + 0x8,
		.home = XN_HOMES + 0x1000,
		.blocks = numberman_blocks, .nblocks = (int)(sizeof numberman_blocks / sizeof *numberman_blocks),
		.data = numberman_data, .ndata = (int)(sizeof numberman_data / sizeof *numberman_data),
		.calls = numberman_calls, .ncalls = (int)(sizeof numberman_calls / sizeof *numberman_calls),
		.words = numberman_words, .nwords = (int)(sizeof numberman_words / sizeof *numberman_words),
		.imms = numberman_imms, .nimms = (int)(sizeof numberman_imms / sizeof *numberman_imms),
		.numbers = numberman_numbers, .nnumbers = (int)(sizeof numberman_numbers / sizeof *numberman_numbers),
		.bytes = numberman_bytes, .nbytes = (int)(sizeof numberman_bytes / sizeof *numberman_bytes),
		.states = numberman_states, .nstates = (int)(sizeof numberman_states / sizeof *numberman_states),
		.hooks = { OWN(0x080FA0A5), BN6(0x080F3BE9), OWN(0x080FA093), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080F9D80, .struct2 = 0x080F9D88,
		.names = { 0x131, 0x132, 0x133, 0x134, 0x135, 0x136 },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x09, .element = 0, .weakness = 0, .shadow = true },
		.sprites = numberman_sprites, .nsprites = (int)(sizeof numberman_sprites / sizeof *numberman_sprites),
		.effects = numberman_effects, .neffects = (int)(sizeof numberman_effects / sizeof *numberman_effects),
		.types = numberman_types, .ntypes = (int)(sizeof numberman_types / sizeof *numberman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = OWN(0x0800EE67), .companion_end = OWN(0x0800EFF3), .hide = BN6(0x080114D5),
	.sprite_of = 9,
	.sounds = numberman_sounds, .nsounds = (int)(sizeof numberman_sounds / sizeof *numberman_sounds),
};
/* ---- TomahawkMan (BN5's: AI 10 there, its ids 0x137, 0x138, 0x139, 0x13a), as Navi AI 35 with ids 0x1d0-0x1d3 ---- */

static const XvBlock tomahawkman_blocks[] = {
	{ 0x080FAB74, 0x080FB678, XN_CODE + 0xB400, 0x7F8D1048u },
	{ 0x080C5386, 0x080C53DE, XN_CODE + 0xC002, 0x4A1B50AFu },
	{ 0x080C83F0, 0x080C928C, XN_CODE + 0xC100, 0x37A3F3B8u },
};
static const XvData tomahawkman_data[] = {
	{ 0x080FAB74, 0x080FABA0, XV_ZERO },
	{ 0x080FABB6, 0x080FABB8, XV_RAW },
	{ 0x080FABB8, 0x080FABBC, XV_POOL },
	{ 0x080FABD0, 0x080FABD4, XV_POOL },
	{ 0x080FABD4, 0x080FABE8, XV_TABLE },
	{ 0x080FAD28, 0x080FAD2C, XV_POOL },
	{ 0x080FAD2C, 0x080FAD30, XV_RAW },
	{ 0x080FAD76, 0x080FAD78, XV_RAW },
	{ 0x080FAD78, 0x080FAD7C, XV_POOL },
	{ 0x080FAD7C, 0x080FAD80, XV_RAW },
	{ 0x080FADB2, 0x080FADB4, XV_RAW },
	{ 0x080FADB4, 0x080FADB8, XV_POOL },
	{ 0x080FADB8, 0x080FADBC, XV_RAW },
	{ 0x080FADFA, 0x080FADFC, XV_RAW },
	{ 0x080FADFC, 0x080FAE00, XV_POOL },
	{ 0x080FAE00, 0x080FAE04, XV_RAW },
	{ 0x080FAE1E, 0x080FAE20, XV_RAW },
	{ 0x080FAE20, 0x080FAE24, XV_POOL },
	{ 0x080FAE24, 0x080FAE28, XV_RAW },
	{ 0x080FAE4A, 0x080FAE4C, XV_RAW },
	{ 0x080FAE4C, 0x080FAE58, XV_POOL },
	{ 0x080FAE58, 0x080FAE62, XV_RAW },
	{ 0x080FAE62, 0x080FAE6C, XV_RAW },
	{ 0x080FAE6C, 0x080FAE78, XV_RAW },
	{ 0x080FAE78, 0x080FAE7C, XV_POOL },
	{ 0x080FAE7C, 0x080FAE80, XV_RAW },
	{ 0x080FAEC6, 0x080FAEC8, XV_RAW },
	{ 0x080FAEC8, 0x080FAECC, XV_POOL },
	{ 0x080FAECC, 0x080FAEDC, XV_RAW },
	{ 0x080FAEDC, 0x080FAEF4, XV_POOL },
	{ 0x080FAF02, 0x080FAF04, XV_RAW },
	{ 0x080FAF04, 0x080FAF08, XV_POOL },
	{ 0x080FAF08, 0x080FAF14, XV_TABLE },
	{ 0x080FAF60, 0x080FAF64, XV_POOL },
	{ 0x080FAF64, 0x080FAF74, XV_RAW },
	{ 0x080FAFE8, 0x080FAFEC, XV_POOL },
	{ 0x080FAFEC, 0x080FB008, XV_TABLE },
	{ 0x080FB054, 0x080FB058, XV_POOL },
	{ 0x080FB058, 0x080FB068, XV_RAW },
	{ 0x080FB0E2, 0x080FB0E4, XV_RAW },
	{ 0x080FB0E4, 0x080FB0E8, XV_POOL },
	{ 0x080FB168, 0x080FB16C, XV_POOL },
	{ 0x080FB16C, 0x080FB184, XV_TABLE },
	{ 0x080FB1CE, 0x080FB1D0, XV_RAW },
	{ 0x080FB1D0, 0x080FB1D4, XV_POOL },
	{ 0x080FB1D4, 0x080FB1E4, XV_RAW },
	{ 0x080FB304, 0x080FB308, XV_POOL },
	{ 0x080FB33A, 0x080FB33C, XV_RAW },
	{ 0x080FB33C, 0x080FB340, XV_POOL },
	{ 0x080FB340, 0x080FB360, XV_TABLE },
	{ 0x080FB5DE, 0x080FB5E0, XV_RAW },
	{ 0x080FB666, 0x080FB668, XV_RAW },
	{ 0x080FB668, 0x080FB670, XV_POOL },
	{ 0x080FB670, 0x080FB674, XV_RAW },
	{ 0x080FB674, 0x080FB678, XV_POOL },
	{ 0x080C83F0, 0x080C83F8, XV_RAW },
	{ 0x080C83F8, 0x080C8404, XV_RAW },
	{ 0x080C8416, 0x080C8418, XV_RAW },
	{ 0x080C8418, 0x080C841C, XV_POOL },
	{ 0x080C841C, 0x080C842C, XV_TABLE },
	{ 0x080C8506, 0x080C8508, XV_RAW },
	{ 0x080C8508, 0x080C850C, XV_POOL },
	{ 0x080C850C, 0x080C851C, XV_TABLE },
	{ 0x080C859A, 0x080C859C, XV_RAW },
	{ 0x080C859C, 0x080C85A0, XV_POOL },
	{ 0x080C85A0, 0x080C85A8, XV_RAW },
	{ 0x080C85A8, 0x080C85AC, XV_POOL },
	{ 0x080C85AC, 0x080C85CC, XV_RAW },
	{ 0x080C8620, 0x080C8624, XV_POOL },
	{ 0x080C8624, 0x080C862C, XV_RAW },
	{ 0x080C86A6, 0x080C86A8, XV_RAW },
	{ 0x080C86A8, 0x080C86AC, XV_POOL },
	{ 0x080C87D2, 0x080C87D4, XV_RAW },
	{ 0x080C87D4, 0x080C87D8, XV_POOL },
	{ 0x080C87D8, 0x080C8804, XV_RAW },
	{ 0x080C8804, 0x080C8808, XV_POOL },
	{ 0x080C8808, 0x080C8810, XV_RAW },
	{ 0x080C8810, 0x080C8824, XV_POOL },
	{ 0x080C8824, 0x080C8834, XV_RAW },
	{ 0x080C8846, 0x080C8848, XV_RAW },
	{ 0x080C8848, 0x080C884C, XV_POOL },
	{ 0x080C884C, 0x080C8858, XV_TABLE },
	{ 0x080C88FA, 0x080C88FC, XV_RAW },
	{ 0x080C88FC, 0x080C8900, XV_POOL },
	{ 0x080C8900, 0x080C8904, XV_TABLE },
	{ 0x080C8B20, 0x080C8B28, XV_POOL },
	{ 0x080C8B3A, 0x080C8B3C, XV_RAW },
	{ 0x080C8B3C, 0x080C8B40, XV_POOL },
	{ 0x080C8B40, 0x080C8B4C, XV_TABLE },
	{ 0x080C8BF6, 0x080C8BF8, XV_RAW },
	{ 0x080C8BF8, 0x080C8BFC, XV_POOL },
	{ 0x080C8BFC, 0x080C8C0C, XV_TABLE },
	{ 0x080C8E50, 0x080C8E54, XV_POOL },
	{ 0x080C8E54, 0x080C8E58, XV_RAW },
	{ 0x080C8EEC, 0x080C8EF8, XV_POOL },
	{ 0x080C8EF8, 0x080C8EFC, XV_RAW },
	{ 0x080C8EFC, 0x080C8F04, XV_POOL },
	{ 0x080C8F16, 0x080C8F18, XV_RAW },
	{ 0x080C8F18, 0x080C8F1C, XV_POOL },
	{ 0x080C8F1C, 0x080C8F28, XV_TABLE },
	{ 0x080C8FE2, 0x080C8FE4, XV_RAW },
	{ 0x080C8FE4, 0x080C8FE8, XV_POOL },
	{ 0x080C8FE8, 0x080C8FF4, XV_TABLE },
	{ 0x080C90E2, 0x080C90E4, XV_RAW },
	{ 0x080C90E4, 0x080C90E8, XV_POOL },
	{ 0x080C90FA, 0x080C90FC, XV_RAW },
	{ 0x080C90FC, 0x080C9100, XV_POOL },
	{ 0x080C9100, 0x080C910C, XV_TABLE },
	{ 0x080C91CA, 0x080C91CC, XV_RAW },
	{ 0x080C91CC, 0x080C91D0, XV_POOL },
	{ 0x080C91D0, 0x080C91D4, XV_RAW },
	{ 0x080C91D4, 0x080C91D8, XV_POOL },
	{ 0x080C91D8, 0x080C91DC, XV_RAW },
	{ 0x080C91DC, 0x080C91E0, XV_POOL },
	{ 0x080C91E0, 0x080C91E4, XV_RAW },
	{ 0x080C9250, 0x080C9254, XV_POOL },
	{ 0x080C9254, 0x080C9274, XV_RAW },
	{ 0x080C9274, 0x080C9278, XV_POOL },
	{ 0x080C9278, 0x080C927C, XV_RAW },
	{ 0x080C927C, 0x080C9280, XV_POOL },
	{ 0x080C9280, 0x080C928C, XV_RAW },
};
static const XvPair tomahawkman_calls[] = {
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08002D4C, 0x08002EE8 },   /* map:bytes, SAME */
	{ 0x0800CD92, 0x0800EB08 },   /* map:calls, LAYOUT */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x0800C758, 0x0800E2EC },   /* map:bytes, SAME */
	{ 0x0800CD84, 0x0800EAFA },   /* map:calls, LAYOUT */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB20, 0x0800E708 },   /* map:bytes, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x080E5E64, 0x080E711C },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800AFF0, 0x0800C938 },   /* map:calls, DIFF (audited) */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B688, 0x0800D086 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x0800B572, 0x0800CF70 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
};
static const XvImm tomahawkman_imms[] = {
	{ 0x080C8860, THUMB_MOVS, 2, 0, 0x29, 0x2B },   /* its sprite: list 4 0x29 in BN5, list 4 0x2b in BN6 */
	{ 0x080C8908, THUMB_LDR, 0, 4, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C8B5C, THUMB_MOVS, 2, 0, 0xA, 0x23 },   /* its sprite: list 2 0xa in BN5, list 2 0x23 in BN6 */
	{ 0x080C8D32, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C8ECE, THUMB_MOVS, 0, 0, 0x46, 0xA7 },   /* its effect: 0x46 in BN5, 0xa7 in BN6 */
	{ 0x080C8F50, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its sprite: list 3 0x31 in BN5, list 4 0xae in BN6 (its list's offset) */
	{ 0x080C8F52, THUMB_MOVS, 2, 0, 0x31, 0xAE },   /* its sprite: list 3 0x31 in BN5, list 4 0xae in BN6 */
	{ 0x080C90A2, THUMB_MOVS, 4, 0, 0x5, 0xA5 },   /* its effect: 0x5 in BN5, 0xa5 in BN6 */
	{ 0x080C9126, THUMB_MOVS, 2, 0, 0x27, 0xAF },   /* its sprite: list 4 0x27 in BN5, list 4 0xaf in BN6 */
	{ 0x080FAC10, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080FAD6E, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FADAA, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FADF2, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FB2E4, THUMB_MOVS, 0, 0, 0x45, 0xA6 },   /* its effect: 0x45 in BN5, 0xa6 in BN6 */
};
static const XvNumber tomahawkman_numbers[] = {
	{ 0x080C8EEC, 0x04000001, 0x04000001 },
	{ 0x080FB304, 0x0700FF11, 0x0700FF11 },
};
static const XvRef tomahawkman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080FABA1), OWN(0x080FAEF5), OWN(0x080FAFDB), OWN(0x080FB15B), OWN(0x080FB32D),
};
static const XvSprite tomahawkman_sprites[] = {
	{ 3, 0x31, 4, 0xAE, XN_SHARED_ART + 0x11800, 0x800 },
	{ 4, 0x27, 4, 0xAF, XN_SHARED_ART + 0x12000, 0x1000 },
	{ 5, 0x0A, 5, 0xA3, XN_SHARED_ART + 0x13000, 0x1000 },
	{ 3, 0x15, 4, 0xB0, XN_SHARED_ART + 0x14000, 0x1800 },
	{ 4, 0x42, 4, 0xA6, XN_SHARED_ART + 0x8800, 0x1800 },
};
static const XvId tomahawkman_effects[] = { { 0x05, 0xA5 }, { 0x45, 0xA6 }, { 0x46, 0xA7 } };
static const XvType tomahawkman_types[] = { { 3, 0x1F, XV_TYPE_MOVED }, { 3, 0x20, XV_TYPE_MOVED }, { 3, 0x21, XV_TYPE_MOVED }, { 3, 0x22, XV_TYPE_MOVED }, { 3, 0x23, XV_TYPE_MOVED } };
static const XnSound tomahawkman_sounds[] = {
	{ 0x080C8CC2, 0x080C8CC4, 0x10C },   /* BN6's song 0x10c another */
	{ 0x080C8DE2, 0x080C8DE4, 0x10D },   /* BN6's song 0x10d another */
	{ 0x080C9096, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
};

static const XNavi bn5_tomahawkman = {
	.code = {
		.name = "TomahawkMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 10,
		.ai = 35,
		.first_id = XN_FIRST_ID + 0xC,
		.home = XN_HOMES + 0x1800,
		.blocks = tomahawkman_blocks, .nblocks = (int)(sizeof tomahawkman_blocks / sizeof *tomahawkman_blocks),
		.data = tomahawkman_data, .ndata = (int)(sizeof tomahawkman_data / sizeof *tomahawkman_data),
		.calls = tomahawkman_calls, .ncalls = (int)(sizeof tomahawkman_calls / sizeof *tomahawkman_calls),
		.imms = tomahawkman_imms, .nimms = (int)(sizeof tomahawkman_imms / sizeof *tomahawkman_imms),
		.numbers = tomahawkman_numbers, .nnumbers = (int)(sizeof tomahawkman_numbers / sizeof *tomahawkman_numbers),
		.states = tomahawkman_states, .nstates = (int)(sizeof tomahawkman_states / sizeof *tomahawkman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), OWN(0x080FAD09), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080FAA88, .struct2 = 0x080FAA90,
		.names = { 0x137, 0x138, 0x139, 0x13A, 0x13B, 0x13C },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x0A, .element = 0, .weakness = 0, .shadow = true },
		.sprites = tomahawkman_sprites, .nsprites = (int)(sizeof tomahawkman_sprites / sizeof *tomahawkman_sprites),
		.effects = tomahawkman_effects, .neffects = (int)(sizeof tomahawkman_effects / sizeof *tomahawkman_effects),
		.types = tomahawkman_types, .ntypes = (int)(sizeof tomahawkman_types / sizeof *tomahawkman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 10,
	.sounds = tomahawkman_sounds, .nsounds = (int)(sizeof tomahawkman_sounds / sizeof *tomahawkman_sounds),
};
/* ---- Colonel (BN5's: AI 7 there, its ids 0x125, 0x126, 0x127, 0x128), as Navi AI 32 with ids 0x1c4-0x1c7 ---- */

static const XvBlock colonel_blocks[] = {
	{ 0x080F80E4, 0x080F913C, XN_CODE + 0x0, 0x5EBCF2FEu },
	{ 0x0800EE56, 0x0800EE66, XN_CODE + 0x1102, 0xA8E8B692u },
	{ 0x0800EFE4, 0x0800EFF2, XN_CODE + 0x1200, 0xFA32C0D8u },
	{ 0x08029C5A, 0x08029D00, XN_CODE + 0x1302, 0x8CAA13CAu },
	{ 0x080C4BCC, 0x080C4DB8, XN_CODE + 0x1400, 0x13082317u },
	{ 0x080CACE0, 0x080CAD40, XN_CODE + 0x1600, 0x0356CF2Cu },
	{ 0x080CCACC, 0x080CCCF0, XN_CODE + 0x1700, 0x8C2CC975u },
	{ 0x080D1068, 0x080D1314, XN_CODE + 0x1A00, 0xE4DE2942u },
	{ 0x080E47F0, 0x080E48CC, XN_CODE + 0x1D00, 0xB264E36Au },
};
static const XvData colonel_data[] = {
	{ 0x080F80E4, 0x080F8114, XV_ZERO },
	{ 0x080F8114, 0x080F811E, XV_RAW },
	{ 0x080F811E, 0x080F8128, XV_RAW },
	{ 0x080F8128, 0x080F8132, XV_RAW },
	{ 0x080F8132, 0x080F813C, XV_RAW },
	{ 0x080F813C, 0x080F8146, XV_RAW },
	{ 0x080F8146, 0x080F8150, XV_RAW },
	{ 0x080F8150, 0x080F8154, XV_RAW },
	{ 0x080F8154, 0x080F8158, XV_RAW },
	{ 0x080F8158, 0x080F815C, XV_RAW },
	{ 0x080F815C, 0x080F8160, XV_RAW },
	{ 0x080F8160, 0x080F8164, XV_RAW },
	{ 0x080F8164, 0x080F8168, XV_RAW },
	{ 0x080F817E, 0x080F8180, XV_RAW },
	{ 0x080F8180, 0x080F8184, XV_POOL },
	{ 0x080F8190, 0x080F8194, XV_POOL },
	{ 0x080F8194, 0x080F81B0, XV_TABLE },
	{ 0x080F830E, 0x080F8310, XV_RAW },
	{ 0x080F8310, 0x080F8330, XV_POOL },
	{ 0x080F838E, 0x080F8390, XV_RAW },
	{ 0x080F8390, 0x080F839C, XV_POOL },
	{ 0x080F8436, 0x080F8438, XV_RAW },
	{ 0x080F8438, 0x080F8450, XV_POOL },
	{ 0x080F84A6, 0x080F84A8, XV_RAW },
	{ 0x080F84A8, 0x080F84B4, XV_POOL },
	{ 0x080F84F8, 0x080F8500, XV_POOL },
	{ 0x080F8538, 0x080F8540, XV_POOL },
	{ 0x080F8558, 0x080F855C, XV_POOL },
	{ 0x080F8618, 0x080F861C, XV_POOL },
	{ 0x080F861C, 0x080F862C, XV_RAW },
	{ 0x080F88F4, 0x080F88F8, XV_POOL },
	{ 0x080F88F8, 0x080F8908, XV_RAW },
	{ 0x080F8908, 0x080F890C, XV_POOL },
	{ 0x080F89BC, 0x080F89C4, XV_POOL },
	{ 0x080F89D2, 0x080F89D4, XV_RAW },
	{ 0x080F89D4, 0x080F89D8, XV_POOL },
	{ 0x080F89D8, 0x080F89E8, XV_TABLE },
	{ 0x080F8AAA, 0x080F8AAC, XV_RAW },
	{ 0x080F8AAC, 0x080F8AB0, XV_POOL },
	{ 0x080F8AB0, 0x080F8ABC, XV_TABLE },
	{ 0x080F8B12, 0x080F8B14, XV_RAW },
	{ 0x080F8B14, 0x080F8B18, XV_POOL },
	{ 0x080F8B18, 0x080F8B1C, XV_RAW },
	{ 0x080F8BD4, 0x080F8BDC, XV_POOL },
	{ 0x080F8BDC, 0x080F8BE8, XV_RAW },
	{ 0x080F8BE8, 0x080F8BEC, XV_RAW },
	{ 0x080F8C26, 0x080F8C28, XV_RAW },
	{ 0x080F8C28, 0x080F8C2C, XV_POOL },
	{ 0x080F8C2C, 0x080F8C3C, XV_TABLE },
	{ 0x080F8D06, 0x080F8D0C, XV_RAW },
	{ 0x080F8D46, 0x080F8D48, XV_RAW },
	{ 0x080F8D48, 0x080F8D4C, XV_POOL },
	{ 0x080F8D4C, 0x080F8D7C, XV_TABLE },
	{ 0x080F8EE6, 0x080F8EE8, XV_RAW },
	{ 0x080F8EE8, 0x080F8EF0, XV_POOL },
	{ 0x080F909E, 0x080F90A0, XV_RAW },
	{ 0x080F90A0, 0x080F90A4, XV_POOL },
	{ 0x080F90B2, 0x080F90B4, XV_RAW },
	{ 0x080F90B4, 0x080F90B8, XV_POOL },
	{ 0x080F90B8, 0x080F90C0, XV_TABLE },
	{ 0x080F9138, 0x080F913C, XV_POOL },
	{ 0x08029CB6, 0x08029CFC, XV_RAW },
	{ 0x08029CFC, 0x08029D00, XV_POOL },
	{ 0x080C4BE4, 0x080C4BE8, XV_POOL },
	{ 0x080C4BE8, 0x080C4BF4, XV_TABLE },
	{ 0x080C4C8C, 0x080C4C90, XV_POOL },
	{ 0x080C4C90, 0x080C4CA0, XV_TABLE },
	{ 0x080C4DB0, 0x080C4DB8, XV_POOL },
	{ 0x080CACEE, 0x080CAD06, XV_RAW },
	{ 0x080CAD18, 0x080CAD3C, XV_RAW },
	{ 0x080CAD3C, 0x080CAD40, XV_POOL },
	{ 0x080CCACC, 0x080CCAD4, XV_RAW },
	{ 0x080CCAE6, 0x080CCAE8, XV_RAW },
	{ 0x080CCAE8, 0x080CCAEC, XV_POOL },
	{ 0x080CCAEC, 0x080CCAF8, XV_TABLE },
	{ 0x080CCBA4, 0x080CCBA8, XV_POOL },
	{ 0x080CCBA8, 0x080CCBB0, XV_TABLE },
	{ 0x080CCC84, 0x080CCC88, XV_POOL },
	{ 0x080CCCEC, 0x080CCCF0, XV_POOL },
	{ 0x080D1068, 0x080D1088, XV_RAW },
	{ 0x080D1096, 0x080D1098, XV_RAW },
	{ 0x080D1098, 0x080D109C, XV_POOL },
	{ 0x080D109C, 0x080D10A8, XV_TABLE },
	{ 0x080D1144, 0x080D114C, XV_POOL },
	{ 0x080D1176, 0x080D1178, XV_RAW },
	{ 0x080D1178, 0x080D117C, XV_POOL },
	{ 0x080D117C, 0x080D1198, XV_TABLE },
	{ 0x080D1308, 0x080D1314, XV_POOL },
	{ 0x080E47FE, 0x080E4800, XV_RAW },
	{ 0x080E4800, 0x080E480C, XV_TABLE },
	{ 0x080E480C, 0x080E4810, XV_POOL },
	{ 0x080E4872, 0x080E4874, XV_RAW },
	{ 0x080E48A4, 0x080E48A8, XV_POOL },
};
static const XvPair colonel_calls[] = {
	{ 0x080C3808, 0x080C5D18 },   /* project:navis, DIFF (audited) */
	{ 0x080C382C, 0x080C5D38 },   /* project:navis, SAME */
	{ 0x0800C6CE, 0x0800E2CE },   /* map:bytes, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080187CC, 0x0801BCD0 },   /* map:bytes, SAME */
	{ 0x0800F198, 0x08011504 },   /* map:bytes, SAME */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x0800131E, XN_STUB_SIDES },   /* a stub: what BN6's engine lacks */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800AFF0, 0x0800C938 },   /* map:calls, DIFF (audited) */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08017A10, 0x0801AD9E },   /* map:calls, DIFF (audited) */
	{ 0x0800D5A2, 0x0800F672 },   /* map:bytes, SAME */
	{ 0x08017FCC, 0x0801B394 },   /* map:calls, DIFF (audited) */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0802EFFE, 0x0802EF5C },   /* map:bytes, SAME */
	{ 0x0800D586, 0x0800F656 },   /* map:bytes, SAME */
	{ 0x08016C70, 0x0801A180 },   /* map:bytes, SAME */
	{ 0x080188DA, 0x0801BDDE },   /* map:bytes, SAME */
	{ 0x0800D544, 0x0800F614 },   /* map:bytes, SAME */
	{ 0x08001490, 0x0800151C },   /* map:calls+bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800BA08, 0x0800D4AC },   /* map:bytes, SAME */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800E0EC, 0x08010368 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800B5A8, 0x0800CFA6 },   /* map:bytes, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x080C3836, 0x080C5F26 },   /* search, by place, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08018900, 0x0801BE04 },   /* map:bytes, SAME */
	{ 0x0800E096, 0x08010312 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B392, 0x0800CCD4 },   /* map:bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x080B9CEE, XN_OBJECTS + 0x292 },   /* the moved objects, SAME */
	{ 0x0800CA7C, 0x0800E5FC },   /* map:bytes, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
};
static const XvPair colonel_words[] = {
	{ 0x02034000, 0x0203FF60 },
	{ 0x080133C5, 0x080165B9 },
	{ 0x080134A1, 0x080166AF },
	{ 0x08013825, 0x08016B03 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x0801486F, 0x08017E27 },
	{ 0x0801629C, 0x08019B78 },
};
static const XvImm colonel_imms[] = {
	{ 0x0800EE58, THUMB_MOVS, 4, 0, 0x3, 0x13 },   /* its overlay's kind: 3 in BN5, 19 in BN6 (its own, past BN6's) */
	{ 0x080C4CF2, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CCB6A, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CCC10, THUMB_ADDS, 4, 0, 0x47, 0x60 },   /* its effect: 0x47 in BN5, 0x60 in BN6 */
	{ 0x080D10C2, THUMB_MOVS, 2, 0, 0x2, 0xB2 },   /* its sprite: list 4 0x2 in BN5, list 4 0xb2 in BN6 */
	{ 0x080D1284, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080D12BA, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
	{ 0x080E481E, THUMB_MOVS, 2, 0, 0x9, 0xB3 },   /* its sprite: list 4 0x9 in BN5, list 4 0xb3 in BN6 */
	{ 0x080E48B0, THUMB_MOVS, 4, 0, 0x1, 0xAB },   /* its effect: 0x1 in BN5, 0xab in BN6 */
	{ 0x080F81E4, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F8230, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080F8260, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080F828A, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080F8306, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F837C, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F83F6, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F842E, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F849E, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F84F0, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F8B8C, THUMB_ADDS, 0, 0, 0x48, 0xA8 },   /* its effects 0x48-0x4a in BN5, 0xa8 on in BN6 (a variable added) */
};
static const XvNumber colonel_numbers[] = {
	{ 0x080C4DB0, 0x0019101E, 0x001910B1 },
	{ 0x080C4DB4, 0x04050000, 0x04050000 },
	{ 0x080CCC84, 0x0405FF04, 0x0405FF04 },
	{ 0x080F90A0, 0x0705FF01, 0x0705FF01 },
};
static const XvRef colonel_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F8169), OWN(0x080F89C5), OWN(0x080F8A97), OWN(0x080F8C19), OWN(0x080F8D39), OWN(0x080F90A5),
};
static const XvSprite colonel_sprites[] = {
	{ 4, 0x1E, 4, 0xB1, XN_SHARED_ART + 0x15800, 0x3800 },
	{ 4, 0x02, 4, 0xB2, XN_SHARED_ART + 0x19000, 0x1800 },
	{ 4, 0x09, 4, 0xB3, XN_SHARED_ART + 0x1A800, 0x800 },
	{ 4, 0x1F, 4, 0xB4, XN_SHARED_ART + 0x1B000, 0x1800 },
	{ 5, 0x02, 5, 0xA2, XN_SHARED_ART + 0x7000, 0x1800 },
	{ 5, 0x0D, 5, 0xA0, XN_SHARED_ART + 0x2000, 0x1800 },
};
static const XvId colonel_effects[] = { { 0x48, 0xA8 }, { 0x49, 0xA9 }, { 0x4A, 0xAA }, { 0x01, 0xAB }, { 0x02, 0xA2 }, { 0x14, 0xA0 } };
static const XvType colonel_types[] = { { 3, 0x02, XV_TYPE_MOVED }, { 3, 0x3C, XV_TYPE_MOVED }, { 3, 0x59, XV_TYPE_MOVED }, { 4, 0x38, XV_TYPE_MOVED } };
static const XnSound colonel_sounds[] = {
	{ 0x080D123C, 0x00000000, 0x0C0 },   /* BN6's song 0xc0 another */
	{ 0x080F8DCA, 0x00000000, 0x0CF },   /* BN6's song 0xcf another */
};
static const XnOverlay colonel_overlay = { 0x080C365C, XN_ART + 0x0, 0x598AABE1u, 0x29, 19, 4, 0xB1 };   /* BN5's kind 3, its sprite 4:0x1e */
static const XvType colonel_needs[] = { { 1, 0x05, XV_TYPE_MOVED } };   /* the moved objects' (xnavi_objects.h) */

static const XNavi bn5_colonel = {
	.code = {
		.name = "Colonel (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 7,
		.ai = 32,
		.first_id = XN_FIRST_ID + 0x0,
		.home = XN_HOMES + 0x0,
		.blocks = colonel_blocks, .nblocks = (int)(sizeof colonel_blocks / sizeof *colonel_blocks),
		.data = colonel_data, .ndata = (int)(sizeof colonel_data / sizeof *colonel_data),
		.calls = colonel_calls, .ncalls = (int)(sizeof colonel_calls / sizeof *colonel_calls),
		.words = colonel_words, .nwords = (int)(sizeof colonel_words / sizeof *colonel_words),
		.imms = colonel_imms, .nimms = (int)(sizeof colonel_imms / sizeof *colonel_imms),
		.numbers = colonel_numbers, .nnumbers = (int)(sizeof colonel_numbers / sizeof *colonel_numbers),
		.states = colonel_states, .nstates = (int)(sizeof colonel_states / sizeof *colonel_states),
		.hooks = { OWN(0x080F897D), BN6(0x080F3BE9), OWN(0x080F8501), OWN(0x080F8993), OWN(0x080F8993), OWN(0x080F8993), OWN(0x080F8993), OWN(0x080F8993) },
		.struct1 = 0x080F7FF8, .struct2 = 0x080F8000,
		.names = { 0x125, 0x126, 0x127, 0x128, 0x129, 0x12A },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x08, .element = 0, .weakness = 0, .shadow = true },
		.sprites = colonel_sprites, .nsprites = (int)(sizeof colonel_sprites / sizeof *colonel_sprites),
		.effects = colonel_effects, .neffects = (int)(sizeof colonel_effects / sizeof *colonel_effects),
		.types = colonel_types, .ntypes = (int)(sizeof colonel_types / sizeof *colonel_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = OWN(0x0800EE57), .companion_end = OWN(0x0800EFE5), .hide = BN6(0x080C5D43),
	.sprite_of = 7,
	.overlay = &colonel_overlay,
	.shown = { 0x000, 0x000, 0x125, 0x000, 0x000, 0x000 },
	.sounds = colonel_sounds, .nsounds = (int)(sizeof colonel_sounds / sizeof *colonel_sounds),
	.needs = colonel_needs, .nneeds = (int)(sizeof colonel_needs / sizeof *colonel_needs),
};
/* ---- the Dark Soul (BN5's: AI 22 there, its ids 0x18d), as Navi AI 38 with ids 0x1dc-0x1dc ---- */

static const XvBlock thedarksoul_blocks[] = {
	{ 0x081042D8, 0x0810440C, XN_CODE + 0x16800, 0xF3096313u },
	{ 0x08012C30, 0x08012CB6, XN_CODE + 0x16A00, 0xCB0294BCu },
	{ 0x08022AF0, 0x08022C58, XN_CODE + 0x16B00, 0x499C338Du },
	{ 0x08029B40, 0x08029D8E, XN_CODE + 0x16D00, 0xA6657004u },
	{ 0x08029DA4, 0x0802A1FE, XN_CODE + 0x17000, 0xDBCA229Cu },
	{ 0x0802A20C, 0x0802A560, XN_CODE + 0x17500, 0x7703DDE5u },
	{ 0x0802A56C, 0x0802AE70, XN_CODE + 0x17900, 0x501187C5u },
	{ 0x0802BA18, 0x0802BD7A, XN_CODE + 0x18300, 0xC8D2D109u },
	{ 0x0802BD96, 0x0802BE1E, XN_CODE + 0x18702, 0x313B55A7u },
	{ 0x0802BE9E, 0x0802C1D0, XN_CODE + 0x18802, 0xAC444115u },
	{ 0x080EBB96, 0x080EBC90, XN_CODE + 0x18C02, 0x61AD6A76u },
	{ 0x080EC4A4, 0x080EC56C, XN_CODE + 0x18D00, 0xD1E75E8Du },
};
static const XvData thedarksoul_data[] = {
	{ 0x081042D8, 0x081042F8, XV_ZERO },
	{ 0x081043A6, 0x081043A8, XV_RAW },
	{ 0x081043A8, 0x081043B0, XV_POOL },
	{ 0x0810440A, 0x0810440C, XV_RAW },
	{ 0x08012C42, 0x08012C44, XV_RAW },
	{ 0x08012C44, 0x08012C48, XV_POOL },
	{ 0x08022AF8, 0x08022B54, XV_POOL },
	{ 0x08022B54, 0x08022B5D, XV_RAW },
	{ 0x08022B5D, 0x08022B60, XV_RAW },
	{ 0x08022B60, 0x08022B67, XV_RAW },
	{ 0x08022B67, 0x08022B6E, XV_RAW },
	{ 0x08022B6E, 0x08022B71, XV_RAW },
	{ 0x08022B71, 0x08022B7C, XV_RAW },
	{ 0x08022B7C, 0x08022B7F, XV_RAW },
	{ 0x08022B7F, 0x08022B86, XV_RAW },
	{ 0x08022B86, 0x08022B8B, XV_RAW },
	{ 0x08022B8B, 0x08022B92, XV_RAW },
	{ 0x08022B92, 0x08022B9F, XV_RAW },
	{ 0x08022B9F, 0x08022BB2, XV_RAW },
	{ 0x08022BB2, 0x08022BBB, XV_RAW },
	{ 0x08022BBB, 0x08022BC8, XV_RAW },
	{ 0x08022BC8, 0x08022BD9, XV_RAW },
	{ 0x08022BD9, 0x08022BE4, XV_RAW },
	{ 0x08022BE4, 0x08022BEF, XV_RAW },
	{ 0x08022BEF, 0x08022BF4, XV_RAW },
	{ 0x08022BF4, 0x08022BFF, XV_RAW },
	{ 0x08022BFF, 0x08022C10, XV_RAW },
	{ 0x08022C10, 0x08022C23, XV_RAW },
	{ 0x08022C23, 0x08022C34, XV_RAW },
	{ 0x08022C34, 0x08022C54, XV_RAW },
	{ 0x08022C54, 0x08022C58, XV_POOL },
	{ 0x08029B68, 0x08029B6C, XV_POOL },
	{ 0x08029B6C, 0x08029BF0, XV_TABLE },
	{ 0x08029C5A, 0x08029CB6, XV_RAW },
	{ 0x08029CF6, 0x08029CF8, XV_RAW },
	{ 0x08029CF8, 0x08029CFC, XV_POOL },
	{ 0x08029CFC, 0x08029D00, XV_RAW },
	{ 0x08029D0E, 0x08029D10, XV_RAW },
	{ 0x08029D10, 0x08029D14, XV_POOL },
	{ 0x08029D14, 0x08029D1C, XV_TABLE },
	{ 0x08029DB2, 0x08029DB4, XV_RAW },
	{ 0x08029DB4, 0x08029DB8, XV_POOL },
	{ 0x08029DB8, 0x08029DC0, XV_TABLE },
	{ 0x08029E80, 0x08029E84, XV_POOL },
	{ 0x08029E84, 0x08029E8C, XV_TABLE },
	{ 0x08029EC6, 0x08029EC8, XV_RAW },
	{ 0x08029EC8, 0x08029ECC, XV_POOL },
	{ 0x08029EDA, 0x08029EDC, XV_RAW },
	{ 0x08029EDC, 0x08029EE0, XV_POOL },
	{ 0x08029EE0, 0x08029EE8, XV_TABLE },
	{ 0x08029FBA, 0x08029FBC, XV_RAW },
	{ 0x08029FCA, 0x08029FCC, XV_RAW },
	{ 0x08029FCC, 0x08029FD0, XV_POOL },
	{ 0x08029FD0, 0x08029FD8, XV_TABLE },
	{ 0x0802A046, 0x0802A048, XV_RAW },
	{ 0x0802A048, 0x0802A04C, XV_POOL },
	{ 0x0802A04C, 0x0802A054, XV_TABLE },
	{ 0x0802A112, 0x0802A114, XV_RAW },
	{ 0x0802A114, 0x0802A118, XV_POOL },
	{ 0x0802A118, 0x0802A120, XV_RAW },
	{ 0x0802A12E, 0x0802A130, XV_RAW },
	{ 0x0802A130, 0x0802A134, XV_POOL },
	{ 0x0802A134, 0x0802A13C, XV_TABLE },
	{ 0x0802A2A2, 0x0802A2A4, XV_RAW },
	{ 0x0802A2BE, 0x0802A2C0, XV_RAW },
	{ 0x0802A2C0, 0x0802A2C4, XV_POOL },
	{ 0x0802A2C4, 0x0802A2CC, XV_TABLE },
	{ 0x0802A312, 0x0802A314, XV_RAW },
	{ 0x0802A322, 0x0802A324, XV_RAW },
	{ 0x0802A324, 0x0802A328, XV_POOL },
	{ 0x0802A328, 0x0802A334, XV_TABLE },
	{ 0x0802A3C8, 0x0802A3CC, XV_POOL },
	{ 0x0802A3CC, 0x0802A3D8, XV_RAW },
	{ 0x0802A3E6, 0x0802A3E8, XV_RAW },
	{ 0x0802A3E8, 0x0802A3EC, XV_POOL },
	{ 0x0802A3EC, 0x0802A3F4, XV_TABLE },
	{ 0x0802A414, 0x0802A418, XV_RAW },
	{ 0x0802A446, 0x0802A448, XV_RAW },
	{ 0x0802A448, 0x0802A44C, XV_POOL },
	{ 0x0802A44C, 0x0802A45C, XV_RAW },
	{ 0x0802A46A, 0x0802A46C, XV_RAW },
	{ 0x0802A46C, 0x0802A470, XV_POOL },
	{ 0x0802A470, 0x0802A478, XV_TABLE },
	{ 0x0802A48A, 0x0802A48C, XV_RAW },
	{ 0x0802A49A, 0x0802A49C, XV_RAW },
	{ 0x0802A49C, 0x0802A4A0, XV_POOL },
	{ 0x0802A4A0, 0x0802A4A8, XV_TABLE },
	{ 0x0802A5B2, 0x0802A5B4, XV_RAW },
	{ 0x0802A5B4, 0x0802A5B8, XV_POOL },
	{ 0x0802A5B8, 0x0802A5C0, XV_TABLE },
	{ 0x0802A608, 0x0802A60C, XV_POOL },
	{ 0x0802A60C, 0x0802A61C, XV_RAW },
	{ 0x0802A62A, 0x0802A62C, XV_RAW },
	{ 0x0802A62C, 0x0802A630, XV_POOL },
	{ 0x0802A630, 0x0802A638, XV_TABLE },
	{ 0x0802A6EE, 0x0802A6F0, XV_RAW },
	{ 0x0802A6F0, 0x0802A6F4, XV_POOL },
	{ 0x0802A744, 0x0802A748, XV_POOL },
	{ 0x0802A748, 0x0802A758, XV_RAW },
	{ 0x0802A78E, 0x0802A790, XV_RAW },
	{ 0x0802A790, 0x0802A794, XV_POOL },
	{ 0x0802A794, 0x0802A7A0, XV_RAW },
	{ 0x0802A89E, 0x0802A8A0, XV_RAW },
	{ 0x0802A8F2, 0x0802A8F4, XV_RAW },
	{ 0x0802A902, 0x0802A904, XV_RAW },
	{ 0x0802A904, 0x0802A908, XV_POOL },
	{ 0x0802A908, 0x0802A910, XV_TABLE },
	{ 0x0802A9B6, 0x0802A9B8, XV_RAW },
	{ 0x0802AA36, 0x0802AA38, XV_RAW },
	{ 0x0802AA76, 0x0802AA78, XV_RAW },
	{ 0x0802AADE, 0x0802AAE0, XV_RAW },
	{ 0x0802AAE0, 0x0802AAE4, XV_POOL },
	{ 0x0802AAE4, 0x0802AAEC, XV_TABLE },
	{ 0x0802ABDE, 0x0802ABE0, XV_RAW },
	{ 0x0802ABE0, 0x0802ABE4, XV_POOL },
	{ 0x0802ABE4, 0x0802ABEC, XV_RAW },
	{ 0x0802ABFA, 0x0802ABFC, XV_RAW },
	{ 0x0802ABFC, 0x0802AC00, XV_POOL },
	{ 0x0802AC00, 0x0802AC08, XV_TABLE },
	{ 0x0802AC80, 0x0802AC84, XV_POOL },
	{ 0x0802ACB2, 0x0802ACB4, XV_RAW },
	{ 0x0802AD02, 0x0802AD04, XV_RAW },
	{ 0x0802AD94, 0x0802AD98, XV_POOL },
	{ 0x0802AD98, 0x0802ADA0, XV_RAW },
	{ 0x0802ADA0, 0x0802ADA4, XV_POOL },
	{ 0x0802ADB2, 0x0802ADB4, XV_RAW },
	{ 0x0802ADB4, 0x0802ADB8, XV_POOL },
	{ 0x0802ADB8, 0x0802ADC0, XV_TABLE },
	{ 0x0802AE66, 0x0802AE68, XV_RAW },
	{ 0x0802AE68, 0x0802AE70, XV_POOL },
	{ 0x0802BA2C, 0x0802BA30, XV_POOL },
	{ 0x0802BA30, 0x0802BA38, XV_TABLE },
	{ 0x0802BA46, 0x0802BA48, XV_RAW },
	{ 0x0802BA48, 0x0802BA4C, XV_POOL },
	{ 0x0802BA4C, 0x0802BA68, XV_TABLE },
	{ 0x0802BB1E, 0x0802BB20, XV_RAW },
	{ 0x0802BB20, 0x0802BB24, XV_POOL },
	{ 0x0802BB46, 0x0802BB48, XV_RAW },
	{ 0x0802BB48, 0x0802BB4C, XV_POOL },
	{ 0x0802BB4C, 0x0802BB54, XV_TABLE },
	{ 0x0802BBBE, 0x0802BBC0, XV_RAW },
	{ 0x0802BBC0, 0x0802BBC4, XV_POOL },
	{ 0x0802BBC4, 0x0802BBD0, XV_TABLE },
	{ 0x0802BD08, 0x0802BD0C, XV_POOL },
	{ 0x0802BEEA, 0x0802BEEE, XV_RAW },
	{ 0x0802BEFE, 0x0802BF02, XV_RAW },
	{ 0x0802BF1A, 0x0802BF20, XV_RAW },
	{ 0x0802BF52, 0x0802C016, XV_RAW },
	{ 0x0802C03E, 0x0802C098, XV_RAW },
	{ 0x0802C0D8, 0x0802C0E0, XV_RAW },
	{ 0x0802C10A, 0x0802C1B4, XV_RAW },
	{ 0x0802C1B4, 0x0802C1C8, XV_POOL },
	{ 0x0802C1C8, 0x0802C1CC, XV_RAW },
	{ 0x0802C1CC, 0x0802C1D0, XV_POOL },
	{ 0x080EBC8A, 0x080EBC8C, XV_RAW },
	{ 0x080EBC8C, 0x080EBC90, XV_POOL },
	{ 0x080EC4B2, 0x080EC4B4, XV_RAW },
	{ 0x080EC4B4, 0x080EC4B8, XV_POOL },
	{ 0x080EC4B8, 0x080EC4C4, XV_TABLE },
	{ 0x080EC4D8, 0x080EC4DC, XV_RAW },
};
static const XvPair thedarksoul_calls[] = {
	{ 0x0800C6C2, 0x0800E2C2 },   /* map:calls+bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0801DF0C, XN_STUB_CHIPDATA },   /* a stub: what BN6's engine lacks */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800E0EC, 0x08010368 },   /* map:bytes, SAME */
	{ 0x08012A20, 0x08015C48 },   /* map:bytes, SAME */
	{ 0x0800E0B6, 0x08010332 },   /* map:calls, DIFF (audited) */
	{ 0x0800F280, 0x080116F6 },   /* map:bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0801311E, 0x080162AA },   /* near, SAME */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x08012A98, 0x08015CC0 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08012C1E, 0x08015E46 },   /* map:bytes, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0800EA18, 0x08010B78 },   /* map:bytes, SAME */
	{ 0x0802D4E6, 0x0802E070 },   /* map:bytes, SAME */
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x080299E8, 0x0802CE78 },   /* map:bytes, SAME */
	{ 0x0800B3E4, 0x0800CDB4 },   /* map:bytes, SAME */
	{ 0x08010DEE, 0x08013774 },   /* map:calls+bytes, SAME */
	{ 0x0800DC0C, 0x0800FE52 },   /* project:navis, DIFF (audited) */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x08008F3A, 0x0800A772 },   /* map:bytes, SAME */
	{ 0x081BC994, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08012CB6, 0x08015EC6 },   /* search, SAME */
	{ 0x080091EA, 0x0800A9EC },   /* map:bytes, SAME */
	{ 0x0801A410, 0x0801DACC },   /* project:navis, SAME */
	{ 0x0800F350, XN_STUB_BUSTER },   /* a stub: what BN6's engine lacks */
	{ 0x0800F204, 0x08011684 },   /* project:navis, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x08012D40, 0x08015F50 },   /* near, SAME */
	{ 0x08010528, 0x08012EA8 },   /* map:bytes, SAME */
	{ 0x0800F31A, 0x080117A4 },   /* map:bytes, SAME */
	{ 0x0800CA98, 0x0800E618 },   /* map:bytes, SAME */
	{ 0x08000DE4, 0x08000EB6 },   /* map:bytes, SAME */
	{ 0x0800E6F0, 0x080109A4 },   /* map:bytes, SAME */
	{ 0x0800F208, 0x08011688 },   /* project:navis, DIFF (audited) */
	{ 0x0800E0B0, 0x0801032C },   /* map:bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800D1D2, 0x0800F2B6 },   /* map:bytes, SAME */
	{ 0x0800F114, 0x08011450 },   /* map:bytes, SAME */
	{ 0x080B9CD6, XN_OBJECTS + 0x27A },   /* the moved objects, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800D928, 0x0800FAAC },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x080E0DF2, 0x080E20FC },   /* map:calls+bytes, SAME */
	{ 0x080173B0, 0x0801A5E2 },   /* map:bytes, SAME */
	{ 0x0801A576, 0x0801DC36 },   /* map:bytes, SAME */
	{ 0x08029940, 0x0802CDD0 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08006BC2, 0x080077B4 },   /* map:bytes, SAME */
};
static const XvPair thedarksoul_words[] = {
	{ 0x02034C20, 0x0203FBE0 },
	{ 0x08000000, 0x08000000 },
};
static const XvImm thedarksoul_imms[] = {
	{ 0x08029F02, THUMB_CMP, 1, 0, 0x8, 0xD },   /* a panel's type compared with BN5's 0x8: BN6's 0xd */
	{ 0x08029F88, THUMB_CMP, 1, 0, 0x8, 0xD },   /* a panel's type compared with BN5's 0x8: BN6's 0xd */
	{ 0x0802A1C8, THUMB_CMP, 1, 0, 0x8, 0xD },   /* a panel's type compared with BN5's 0x8: BN6's 0xd */
	{ 0x0802A7E6, THUMB_CMP, 1, 0, 0x9, 0x5 },   /* a panel's type compared with BN5's 0x9: BN6's 0x5 */
	{ 0x0802A984, THUMB_CMP, 1, 0, 0x9, 0x5 },   /* a panel's type compared with BN5's 0x9: BN6's 0x5 */
	{ 0x080EBC3C, THUMB_CMP, 0, 0, 0x16, 0x26 },   /* the buster's arm: the Dark Soul's: BN5's AI index 0x16, BN6's 0x26 */
	{ 0x0810430E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvRef thedarksoul_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x081042F9), OWN(0x081043B1),
};
static const XvType thedarksoul_needs[] = { { 1, 0x05, XV_TYPE_MOVED } };   /* the moved objects' (xnavi_objects.h) */
static const XnFamily thedarksoul_families[] = { { 0x16, 0x66, OWN(0x080EC4A5) } };   /* BN5's attack families it runs, moved with it (XN_FAMILY_OF) */

static const XNavi bn5_thedarksoul = {
	.code = {
		.name = "the Dark Soul (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 22,
		.ai = 38,
		.first_id = XN_FIRST_ID + 0x18,
		.home = XN_HOMES + 0x3000,
		.blocks = thedarksoul_blocks, .nblocks = (int)(sizeof thedarksoul_blocks / sizeof *thedarksoul_blocks),
		.data = thedarksoul_data, .ndata = (int)(sizeof thedarksoul_data / sizeof *thedarksoul_data),
		.calls = thedarksoul_calls, .ncalls = (int)(sizeof thedarksoul_calls / sizeof *thedarksoul_calls),
		.words = thedarksoul_words, .nwords = (int)(sizeof thedarksoul_words / sizeof *thedarksoul_words),
		.imms = thedarksoul_imms, .nimms = (int)(sizeof thedarksoul_imms / sizeof *thedarksoul_imms),
		.states = thedarksoul_states, .nstates = (int)(sizeof thedarksoul_states / sizeof *thedarksoul_states),
		.hooks = { OWN(0x0810431D), BN6(0x080F3BE9), OWN(0x08104359), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x081042A8, .struct2 = 0x081042B0,
		.names = { 0x18D, 0x000, 0x000, 0x000, 0x000, 0x000 },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x16, .element = 0, .weakness = 0, .shadow = true },
	},
	.versions = 1,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 22,
	.needs = thedarksoul_needs, .nneeds = (int)(sizeof thedarksoul_needs / sizeof *thedarksoul_needs),
	.families = thedarksoul_families, .nfamilies = (int)(sizeof thedarksoul_families / sizeof *thedarksoul_families),
};
const XNavi *const xnavi_bn5[] = { &bn5_toadman, &bn5_knightman, &bn5_shadowman, &bn5_numberman, &bn5_tomahawkman, &bn5_colonel, &bn5_thedarksoul };
const int xnavi_bn5_count = (int)(sizeof xnavi_bn5 / sizeof *xnavi_bn5);

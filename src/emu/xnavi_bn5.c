/* xnavi_bn5.c: the Navis' recipes, written by tools/xnavirecipe.py from the two ROMs (python3 tools/xnavirecipe.py;
 * --check says whether this is what it writes). Do not edit: change the generator (or tools/bn5_navi_twins.py, its pairs
 * and audits) and write it again. Numbers alone (docs/multirom/navis.md): addresses in BN5 Team Colonel
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
	{ 0x080B9A71, 0x02, 0xF3 },   /* its kind 0x4: BN5's sprite 3:0x2, copied at 3:0xf3 (chips) */
	{ 0x080B9A8E, 0x0C, 0x10 },   /* its kind 0xa: BN5's sprite 3:0x0, copied at 4:0xb5 (chips) */
	{ 0x080B9A8F, 0x00, 0xB5 },   /* its kind 0xa: BN5's sprite 3:0x0, copied at 4:0xb5 (chips) */
	{ 0x080B9A94, 0x1D, 0xEE },   /* its kind 0xb: BN5's sprite 3:0x1d, copied at 3:0xee (chips) */
	{ 0x080B9A9D, 0x0C, 0x14 },   /* its kind 0xd: BN5's sprite 3:0x6, copied at 5:0xa5 (navis) */
	{ 0x080B9A9E, 0x06, 0xA5 },   /* its kind 0xd: BN5's sprite 3:0x6, copied at 5:0xa5 (navis) */
	{ 0x080B9AB2, 0x2A, 0x59 },   /* its kind 0x11: BN5's sprite 3:0x2a, BN6's 3:0x59 the same bytes */
	{ 0x080B9ADF, 0x02, 0xF3 },   /* its kind 0x1a: BN5's sprite 3:0x2, copied at 3:0xf3 (chips) */
	{ 0x080B9AF8, 0x1D, 0xEE },   /* its kind 0x1f: BN5's sprite 3:0x1d, copied at 3:0xee (chips) */
	{ 0x080B9B0B, 0x0C, 0x14 },   /* its kind 0x23: BN5's sprite 3:0x3, copied at 5:0xa6 (navis) */
	{ 0x080B9B0C, 0x03, 0xA6 },   /* its kind 0x23: BN5's sprite 3:0x3, copied at 5:0xa6 (navis) */
	{ 0x080B9B25, 0x02, 0xF3 },   /* its kind 0x28: BN5's sprite 3:0x2, copied at 3:0xf3 (chips) */
	{ 0x080B9B2F, 0x02, 0xF3 },   /* its kind 0x2a: BN5's sprite 3:0x2, copied at 3:0xf3 (chips) */
	{ 0x080B9B34, 0x04, 0xED },   /* its kind 0x2b: BN5's sprite 3:0x4, copied at 3:0xed (chips) */
	{ 0x080B9B39, 0x1C, 0xEC },   /* its kind 0x2c: BN5's sprite 3:0x1c, copied at 3:0xec (chips) */
	{ 0x080B9B3E, 0x1C, 0xEC },   /* its kind 0x2d: BN5's sprite 3:0x1c, copied at 3:0xec (chips) */
	{ 0x080B9B43, 0x1C, 0xEC },   /* its kind 0x2e: BN5's sprite 3:0x1c, copied at 3:0xec (chips) */
	{ 0x080B9B48, 0x1E, 0x58 },   /* its kind 0x2f: BN5's sprite 3:0x1e, BN6's 3:0x58 the same bytes */
	{ 0x080B9B4D, 0x21, 0x6C },   /* its kind 0x30: BN5's sprite 4:0x21, copied at 4:0x6c (families) */
	{ 0x080B9B51, 0x0C, 0x00 },   /* its kind 0x31: BN5's sprite 3:0x1f, the Souls' at 0:0x4e (their install copies and lists it) */
	{ 0x080B9B52, 0x1F, 0x4E },   /* its kind 0x31: BN5's sprite 3:0x1f, the Souls' at 0:0x4e (their install copies and lists it) */
	{ 0x080B9B56, 0x0C, 0x10 },   /* its kind 0x32: BN5's sprite 3:0x0, copied at 4:0xb5 (chips) */
	{ 0x080B9B57, 0x00, 0xB5 },   /* its kind 0x32: BN5's sprite 3:0x0, copied at 4:0xb5 (chips) */
	{ 0x080B9B5B, 0x0C, 0x14 },   /* its kind 0x33: BN5's sprite 3:0x3, copied at 5:0xa6 (chips) */
	{ 0x080B9B5C, 0x03, 0xA6 },   /* its kind 0x33: BN5's sprite 3:0x3, copied at 5:0xa6 (chips) */
	{ 0x080B9B61, 0x33, 0xEB },   /* its kind 0x34: BN5's sprite 3:0x33, copied at 3:0xeb (chips) */
	{ 0x080B9B66, 0x38, 0xEA },   /* its kind 0x35: BN5's sprite 3:0x38, copied at 3:0xea (chips) */
	{ 0x080B9B6B, 0x39, 0xE9 },   /* its kind 0x36: BN5's sprite 3:0x39, copied at 3:0xe9 (chips) */
	{ 0x080B9B70, 0x2A, 0x59 },   /* its kind 0x37: BN5's sprite 3:0x2a, BN6's 3:0x59 the same bytes */
};
static const XvSprite objects_sprites[] = {
	{ 3, 0x00, 4, 0xB5, XN_SHARED_ART + 0x20800, 0x5800 },
	{ 3, 0x02, 3, 0xF3, XN_SHARED_ART + 0x26000, 0x2000 },
	{ 3, 0x1D, 3, 0xEE, XN_SHARED_ART + 0x28000, 0x800 },
	{ 3, 0x06, 5, 0xA5, XN_SHARED_ART + 0x28800, 0x800 },
	{ 3, 0x03, 5, 0xA6, XN_SHARED_ART + 0x29000, 0x2000 },
	{ 3, 0x04, 3, 0xED, XN_SHARED_ART + 0x2B000, 0x2000 },
	{ 3, 0x1C, 3, 0xEC, XN_SHARED_ART + 0x2D000, 0x1800 },
	{ 4, 0x21, 4, 0x6C, XN_SHARED_ART + 0x2E800, 0x800 },
	{ 3, 0x33, 3, 0xEB, XN_SHARED_ART + 0x2F000, 0x800 },
	{ 3, 0x38, 3, 0xEA, XN_SHARED_ART + 0x2F800, 0x800 },
	{ 3, 0x39, 3, 0xE9, XN_SHARED_ART + 0x30000, 0x800 },
	{ 5, 0x1E, 5, 0xA1, XN_SHARED_ART + 0x3800, 0x1000 },
	{ 5, 0x1F, 5, 0xA4, XN_SHARED_ART + 0x30800, 0x800 },
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
	{ 0x0800EE66, 0x0800EE72, XN_CODE + 0x8502, 0x2FB638AFu },
	{ 0x0800EFF2, 0x0800F000, XN_CODE + 0x8602, 0xE7D02240u },
	{ 0x080C33A8, 0x080C34E0, XN_CODE + 0x8700, 0x217F5537u },
	{ 0x080C5014, 0x080C544C, XN_CODE + 0x8900, 0x76FE03D1u },
	{ 0x080D78B4, 0x080D7A72, XN_CODE + 0x8E00, 0xB98CC4DCu },
	{ 0x080D7A80, 0x080D8014, XN_CODE + 0x9000, 0x4FEF51BEu },
	{ 0x080D8CD8, 0x080D8F1C, XN_CODE + 0x9600, 0xA2627512u },
	{ 0x080EABAC, 0x080EACB4, XN_CODE + 0x9900, 0x5D2B4ED6u },
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
	{ 0x0800A2F1, 0x0800BC89 },
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
	{ 0x0802B4B0, 0x0802B804, XN_CODE + 0x18300, 0x220B5F7Du },
	{ 0x0802BA18, 0x0802BD7A, XN_CODE + 0x18700, 0xC8D2D109u },
	{ 0x0802BD96, 0x0802BE1E, XN_CODE + 0x18B02, 0x313B55A7u },
	{ 0x0802BE52, 0x0802C1D0, XN_CODE + 0x18C02, 0x4233E6EEu },
	{ 0x080EBB96, 0x080EBC90, XN_CODE + 0x19002, 0x61AD6A76u },
	{ 0x080EC4A4, 0x080EC56C, XN_CODE + 0x19100, 0xD1E75E8Du },
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
	{ 0x0802B4C4, 0x0802B4C8, XV_POOL },
	{ 0x0802B4C8, 0x0802B4D0, XV_TABLE },
	{ 0x0802B4DE, 0x0802B4E0, XV_RAW },
	{ 0x0802B4E0, 0x0802B4E4, XV_POOL },
	{ 0x0802B4E4, 0x0802B500, XV_TABLE },
	{ 0x0802B5B6, 0x0802B5B8, XV_RAW },
	{ 0x0802B5B8, 0x0802B5BC, XV_POOL },
	{ 0x0802B5DE, 0x0802B5E0, XV_RAW },
	{ 0x0802B5E0, 0x0802B5E4, XV_POOL },
	{ 0x0802B5E4, 0x0802B5EC, XV_TABLE },
	{ 0x0802B654, 0x0802B658, XV_POOL },
	{ 0x0802B658, 0x0802B664, XV_TABLE },
	{ 0x0802B794, 0x0802B798, XV_POOL },
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
	{ 0x0802BF52, 0x0802C016, XV_RAW },
	{ 0x0802C03E, 0x0802C098, XV_RAW },
	{ 0x0802C0D8, 0x0802C0DA, XV_RAW },
	{ 0x0802C10A, 0x0802C1B0, XV_RAW },
	{ 0x0802C1B0, 0x0802C1C8, XV_POOL },
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
	{ 0x081BC994, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
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
/* ---- BlizzardMan (BN5's: AI 14 there, its ids 0x14f, 0x150, 0x151, 0x152), as Navi AI 40 with ids 0x1e4-0x1e7 ---- */

static const XvBlock blizzardman_blocks[] = {
	{ 0x080FDED0, 0x080FEE98, XN_MORE + 0x3C00, 0xF48B9603u },
	{ 0x08016B96, 0x08016BBC, XN_MORE + 0x4C02, 0xA53A3BEFu },
	{ 0x080C5386, 0x080C53DE, XN_MORE + 0x4D02, 0x4A1B50AFu },
	{ 0x080C9784, 0x080C9B54, XN_MORE + 0x4E00, 0x4C49ECE0u },
	{ 0x080C9D18, 0x080CA048, XN_MORE + 0x5200, 0x97F57057u },
	{ 0x080CA838, 0x080CA91C, XN_MORE + 0x5600, 0xE4F67358u },
};
static const XvData blizzardman_data[] = {
	{ 0x080FDED0, 0x080FDEFC, XV_ZERO },
	{ 0x080FDF12, 0x080FDF14, XV_RAW },
	{ 0x080FDF14, 0x080FDF18, XV_POOL },
	{ 0x080FDF18, 0x080FDF1C, XV_TABLE },
	{ 0x080FE05C, 0x080FE060, XV_POOL },
	{ 0x080FE060, 0x080FE064, XV_RAW },
	{ 0x080FE06E, 0x080FE070, XV_RAW },
	{ 0x080FE070, 0x080FE074, XV_POOL },
	{ 0x080FE074, 0x080FE078, XV_RAW },
	{ 0x080FE082, 0x080FE084, XV_RAW },
	{ 0x080FE084, 0x080FE088, XV_POOL },
	{ 0x080FE088, 0x080FE08C, XV_RAW },
	{ 0x080FE0AE, 0x080FE0B0, XV_RAW },
	{ 0x080FE0B0, 0x080FE0BC, XV_POOL },
	{ 0x080FE0BC, 0x080FE0C6, XV_RAW },
	{ 0x080FE0C6, 0x080FE0D0, XV_RAW },
	{ 0x080FE0D0, 0x080FE0DC, XV_RAW },
	{ 0x080FE0DC, 0x080FE0E0, XV_POOL },
	{ 0x080FE0E0, 0x080FE0E4, XV_RAW },
	{ 0x080FE0E4, 0x080FE0FC, XV_POOL },
	{ 0x080FE10A, 0x080FE10C, XV_RAW },
	{ 0x080FE10C, 0x080FE110, XV_POOL },
	{ 0x080FE110, 0x080FE11C, XV_TABLE },
	{ 0x080FE168, 0x080FE16C, XV_POOL },
	{ 0x080FE16C, 0x080FE17C, XV_RAW },
	{ 0x080FE1F8, 0x080FE1FC, XV_POOL },
	{ 0x080FE1FC, 0x080FE218, XV_TABLE },
	{ 0x080FE286, 0x080FE288, XV_RAW },
	{ 0x080FE288, 0x080FE28C, XV_POOL },
	{ 0x080FE28C, 0x080FE29C, XV_RAW },
	{ 0x080FE29C, 0x080FE2A0, XV_POOL },
	{ 0x080FE2A0, 0x080FE2A4, XV_RAW },
	{ 0x080FE2F0, 0x080FE2F4, XV_POOL },
	{ 0x080FE2F4, 0x080FE2F8, XV_RAW },
	{ 0x080FE36A, 0x080FE36C, XV_RAW },
	{ 0x080FE36C, 0x080FE370, XV_POOL },
	{ 0x080FE3AC, 0x080FE3B0, XV_POOL },
	{ 0x080FE3B0, 0x080FE3C0, XV_RAW },
	{ 0x080FE3F6, 0x080FE3F8, XV_RAW },
	{ 0x080FE3F8, 0x080FE3FC, XV_POOL },
	{ 0x080FE3FC, 0x080FE414, XV_TABLE },
	{ 0x080FE462, 0x080FE464, XV_RAW },
	{ 0x080FE464, 0x080FE468, XV_POOL },
	{ 0x080FE468, 0x080FE478, XV_RAW },
	{ 0x080FE55E, 0x080FE560, XV_RAW },
	{ 0x080FE560, 0x080FE564, XV_POOL },
	{ 0x080FE564, 0x080FE58C, XV_TABLE },
	{ 0x080FE5E2, 0x080FE5E4, XV_RAW },
	{ 0x080FE5E4, 0x080FE5E8, XV_POOL },
	{ 0x080FE5E8, 0x080FE5F8, XV_RAW },
	{ 0x080FE6D8, 0x080FE6DC, XV_POOL },
	{ 0x080FE6DC, 0x080FE6EC, XV_RAW },
	{ 0x080FE6EC, 0x080FE6F4, XV_POOL },
	{ 0x080FE78C, 0x080FE790, XV_POOL },
	{ 0x080FE790, 0x080FE794, XV_RAW },
	{ 0x080FE794, 0x080FE798, XV_POOL },
	{ 0x080FE806, 0x080FE808, XV_RAW },
	{ 0x080FE808, 0x080FE80C, XV_POOL },
	{ 0x080FE80C, 0x080FE810, XV_RAW },
	{ 0x080FE864, 0x080FE868, XV_POOL },
	{ 0x080FE868, 0x080FE86C, XV_RAW },
	{ 0x080FE9B4, 0x080FE9B8, XV_POOL },
	{ 0x080FE9B8, 0x080FE9BC, XV_RAW },
	{ 0x080FE9BC, 0x080FE9C0, XV_POOL },
	{ 0x080FE9C0, 0x080FE9CC, XV_RAW },
	{ 0x080FEA6C, 0x080FEA70, XV_POOL },
	{ 0x080FEA70, 0x080FEA7C, XV_RAW },
	{ 0x080FEAA6, 0x080FEAA8, XV_RAW },
	{ 0x080FEAA8, 0x080FEAAC, XV_POOL },
	{ 0x080FEAAC, 0x080FEAB0, XV_RAW },
	{ 0x080FEB42, 0x080FEB44, XV_RAW },
	{ 0x080FEB44, 0x080FEB48, XV_POOL },
	{ 0x080FEB48, 0x080FEB54, XV_RAW },
	{ 0x080FEC52, 0x080FEC54, XV_RAW },
	{ 0x080FEC54, 0x080FEC58, XV_POOL },
	{ 0x080FEC58, 0x080FEC5C, XV_RAW },
	{ 0x080FED02, 0x080FED04, XV_RAW },
	{ 0x080FED04, 0x080FED08, XV_POOL },
	{ 0x080FED08, 0x080FED0C, XV_RAW },
	{ 0x080FED0C, 0x080FED10, XV_POOL },
	{ 0x080FED10, 0x080FED1C, XV_RAW },
	{ 0x080FEDF0, 0x080FEDF4, XV_POOL },
	{ 0x080FEDF4, 0x080FEE00, XV_RAW },
	{ 0x080FEE00, 0x080FEE04, XV_POOL },
	{ 0x080FEE8A, 0x080FEE8C, XV_RAW },
	{ 0x080FEE8C, 0x080FEE90, XV_POOL },
	{ 0x080FEE90, 0x080FEE94, XV_RAW },
	{ 0x080FEE94, 0x080FEE98, XV_POOL },
	{ 0x080C9796, 0x080C9798, XV_RAW },
	{ 0x080C9798, 0x080C979C, XV_POOL },
	{ 0x080C979C, 0x080C97AC, XV_TABLE },
	{ 0x080C9828, 0x080C982C, XV_POOL },
	{ 0x080C982C, 0x080C9834, XV_RAW },
	{ 0x080C9884, 0x080C9888, XV_POOL },
	{ 0x080C9888, 0x080C988C, XV_TABLE },
	{ 0x080C988C, 0x080C9890, XV_POOL },
	{ 0x080C98FE, 0x080C9900, XV_RAW },
	{ 0x080C9900, 0x080C9904, XV_POOL },
	{ 0x080C9904, 0x080C9914, XV_RAW },
	{ 0x080C9914, 0x080C9918, XV_POOL },
	{ 0x080C9918, 0x080C9928, XV_RAW },
	{ 0x080C9994, 0x080C99B0, XV_RAW },
	{ 0x080C99B0, 0x080C99B4, XV_POOL },
	{ 0x080C99B4, 0x080C99BC, XV_RAW },
	{ 0x080C99BC, 0x080C99C0, XV_POOL },
	{ 0x080C99D8, 0x080C99DC, XV_POOL },
	{ 0x080C99DC, 0x080C99E8, XV_TABLE },
	{ 0x080C9A5A, 0x080C9A5C, XV_RAW },
	{ 0x080C9A5C, 0x080C9A60, XV_POOL },
	{ 0x080C9A60, 0x080C9A68, XV_TABLE },
	{ 0x080C9AF8, 0x080C9AFC, XV_POOL },
	{ 0x080C9AFC, 0x080C9B04, XV_RAW },
	{ 0x080C9B50, 0x080C9B54, XV_POOL },
	{ 0x080C9D2A, 0x080C9D2C, XV_RAW },
	{ 0x080C9D2C, 0x080C9D30, XV_POOL },
	{ 0x080C9D30, 0x080C9D3C, XV_TABLE },
	{ 0x080C9DB4, 0x080C9DB8, XV_POOL },
	{ 0x080C9DB8, 0x080C9DC0, XV_RAW },
	{ 0x080C9E20, 0x080C9E24, XV_POOL },
	{ 0x080C9E24, 0x080C9E38, XV_TABLE },
	{ 0x080C9E38, 0x080C9E3C, XV_POOL },
	{ 0x080C9E6E, 0x080C9E70, XV_RAW },
	{ 0x080C9E70, 0x080C9E74, XV_POOL },
	{ 0x080C9E74, 0x080C9E78, XV_RAW },
	{ 0x080C9EFA, 0x080C9EFC, XV_RAW },
	{ 0x080C9EFC, 0x080C9F00, XV_POOL },
	{ 0x080C9F34, 0x080C9F38, XV_POOL },
	{ 0x080C9FFE, 0x080CA000, XV_RAW },
	{ 0x080CA000, 0x080CA004, XV_POOL },
	{ 0x080CA004, 0x080CA030, XV_RAW },
	{ 0x080CA030, 0x080CA034, XV_POOL },
	{ 0x080CA034, 0x080CA03C, XV_RAW },
	{ 0x080CA03C, 0x080CA048, XV_POOL },
	{ 0x080CA84A, 0x080CA84C, XV_RAW },
	{ 0x080CA84C, 0x080CA850, XV_POOL },
	{ 0x080CA850, 0x080CA85C, XV_TABLE },
};
static const XvPair blizzardman_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800D1C6, 0x0800F2AA },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800B722, 0x0800D120 },   /* map:bytes, SAME */
	{ 0x0800B688, 0x0800D086 },   /* map:bytes, SAME */
};
static const XvImm blizzardman_imms[] = {
	{ 0x080C97B4, THUMB_MOVS, 2, 0, 0xE, 0x28 },   /* its sprite: list 2 0xe in BN5, list 2 0x28 in BN6 */
	{ 0x080C9852, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C9A02, THUMB_MOVS, 2, 0, 0xE, 0x28 },   /* its sprite: list 2 0xe in BN5, list 2 0x28 in BN6 */
	{ 0x080C9D40, THUMB_MOVS, 2, 0, 0xE, 0x28 },   /* its sprite: list 2 0xe in BN5, list 2 0x28 in BN6 */
	{ 0x080C9DE4, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C9ED6, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C9F16, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CA8B8, THUMB_MOVS, 1, 0, 0xA, 0xC },   /* its parent's CurAction 10 compared: BN6's 12 */
	{ 0x080FDF60, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FDFA4, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FDFF2, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FE00A, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080FE6F8, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
};
static const XvNumber blizzardman_numbers[] = {
	{ 0x080C9B50, 0x04050201, 0x04050201 },
	{ 0x080FE6EC, 0x0405FF01, 0x0405FF01 },
	{ 0x080FE794, 0x0C800000, 0x0C800000 },
};
static const XvRef blizzardman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080FDEFD), OWN(0x080FE0FD), OWN(0x080FE1EB), OWN(0x080FE3E9), OWN(0x080FE551),
};
static const XvType blizzardman_types[] = { { 3, 0x26, XV_TYPE_MOVED }, { 3, 0x27, XV_TYPE_MOVED }, { 3, 0x29, XV_TYPE_MOVED }, { 3, 0x2F, XV_TYPE_MOVED } };
static const XnSound blizzardman_sounds[] = {
	{ 0x080FE6B4, 0x00000000, 0x0E3 },   /* BN6's song 0xe3 another */
	{ 0x080FE7F4, 0x00000000, 0x0E3 },   /* BN6's song 0xe3 another */
	{ 0x080FE824, 0x00000000, 0x0C0 },   /* BN6's song 0xc0 another */
};

static const XNavi bn5_blizzardman = {
	.code = {
		.name = "BlizzardMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 14,
		.ai = 40,
		.first_id = XN_FIRST_ID + 0x20,
		.home = XN_HOMES + 0x4800,
		.blocks = blizzardman_blocks, .nblocks = (int)(sizeof blizzardman_blocks / sizeof *blizzardman_blocks),
		.data = blizzardman_data, .ndata = (int)(sizeof blizzardman_data / sizeof *blizzardman_data),
		.calls = blizzardman_calls, .ncalls = (int)(sizeof blizzardman_calls / sizeof *blizzardman_calls),
		.imms = blizzardman_imms, .nimms = (int)(sizeof blizzardman_imms / sizeof *blizzardman_imms),
		.numbers = blizzardman_numbers, .nnumbers = (int)(sizeof blizzardman_numbers / sizeof *blizzardman_numbers),
		.states = blizzardman_states, .nstates = (int)(sizeof blizzardman_states / sizeof *blizzardman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), OWN(0x080FE013), OWN(0x080FE017), OWN(0x080FE017), OWN(0x080FE017), OWN(0x080FE017), OWN(0x080FE017) },
		.struct1 = 0x080FDDE4, .struct2 = 0x080FDDEC,
		.names = { 0x14F, 0x150, 0x151, 0x152, 0x153, 0x154 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x0E, .element = 0, .weakness = 0, .shadow = true },
		.types = blizzardman_types, .ntypes = (int)(sizeof blizzardman_types / sizeof *blizzardman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801AAC1), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 14,
	.sounds = blizzardman_sounds, .nsounds = (int)(sizeof blizzardman_sounds / sizeof *blizzardman_sounds),
};
/* ---- ShadeMan (BN5's: AI 13 there, its ids 0x149, 0x14a, 0x14b, 0x14c), as Navi AI 39 with ids 0x1e0-0x1e3 ---- */

static const XvBlock shademan_blocks[] = {
	{ 0x080FD274, 0x080FDDE4, XN_MORE + 0x0, 0xB3FBF0FAu },
	{ 0x080BA978, 0x080BAB66, XN_MORE + 0xC00, 0x0917E926u },
	{ 0x080C5386, 0x080C53DE, XN_MORE + 0xE02, 0x4A1B50AFu },
	{ 0x080C66A8, 0x080C67C8, XN_MORE + 0xF00, 0x38C50E28u },
	{ 0x080C693C, 0x080C6B2C, XN_MORE + 0x1100, 0xD3ECE979u },
	{ 0x080E2548, 0x080E25B0, XN_MORE + 0x1300, 0x51B8DE54u },
	{ 0x08103928, 0x08103A14, XN_MORE + 0x1400, 0xE012CD00u },
	{ 0x08103D62, 0x08103E1A, XN_MORE + 0x1502, 0xF254B657u },
	{ 0x08103FC8, 0x0810412C, XN_MORE + 0x1600, 0x6A2F3071u },
	{ 0x08104220, 0x0810429C, XN_MORE + 0x1800, 0xDDF7D805u },
};
static const XvData shademan_data[] = {
	{ 0x080FD274, 0x080FD2A0, XV_ZERO },
	{ 0x080FD2A0, 0x080FD2AA, XV_RAW },
	{ 0x080FD2AA, 0x080FD2B4, XV_RAW },
	{ 0x080FD2B4, 0x080FD2BE, XV_RAW },
	{ 0x080FD2BE, 0x080FD2C8, XV_RAW },
	{ 0x080FD2C8, 0x080FD2D4, XV_RAW },
	{ 0x080FD2D4, 0x080FD2DC, XV_RAW },
	{ 0x080FD2DC, 0x080FD2E0, XV_RAW },
	{ 0x080FD2EE, 0x080FD2F0, XV_RAW },
	{ 0x080FD2F0, 0x080FD2F4, XV_POOL },
	{ 0x080FD2F4, 0x080FD308, XV_TABLE },
	{ 0x080FD550, 0x080FD554, XV_POOL },
	{ 0x080FD554, 0x080FD558, XV_RAW },
	{ 0x080FD5D2, 0x080FD5D4, XV_RAW },
	{ 0x080FD5D4, 0x080FD62C, XV_POOL },
	{ 0x080FD63A, 0x080FD63C, XV_RAW },
	{ 0x080FD63C, 0x080FD640, XV_POOL },
	{ 0x080FD640, 0x080FD658, XV_TABLE },
	{ 0x080FD748, 0x080FD74C, XV_POOL },
	{ 0x080FD74C, 0x080FD788, XV_TABLE },
	{ 0x080FD7EA, 0x080FD7EC, XV_RAW },
	{ 0x080FD7EC, 0x080FD7F4, XV_POOL },
	{ 0x080FD7F4, 0x080FD804, XV_RAW },
	{ 0x080FD82C, 0x080FD830, XV_RAW },
	{ 0x080FD992, 0x080FD994, XV_RAW },
	{ 0x080FD994, 0x080FD998, XV_POOL },
	{ 0x080FDAA2, 0x080FDAA4, XV_RAW },
	{ 0x080FDAA4, 0x080FDAA8, XV_POOL },
	{ 0x080FDBDA, 0x080FDBDC, XV_RAW },
	{ 0x080FDBDC, 0x080FDBE4, XV_POOL },
	{ 0x080FDBE4, 0x080FDC24, XV_RAW },
	{ 0x080FDC24, 0x080FDC2C, XV_RAW },
	{ 0x080FDC54, 0x080FDC58, XV_POOL },
	{ 0x080FDC58, 0x080FDC64, XV_RAW },
	{ 0x080FDDA8, 0x080FDDAC, XV_POOL },
	{ 0x080FDDAC, 0x080FDDBC, XV_RAW },
	{ 0x080FDDD2, 0x080FDDD4, XV_RAW },
	{ 0x080FDDD4, 0x080FDDE4, XV_POOL },
	{ 0x080BA98A, 0x080BA98C, XV_RAW },
	{ 0x080BA98C, 0x080BA990, XV_POOL },
	{ 0x080BA990, 0x080BA99C, XV_TABLE },
	{ 0x080BAA24, 0x080BAA28, XV_POOL },
	{ 0x080BAA28, 0x080BAA2C, XV_TABLE },
	{ 0x080BAA4C, 0x080BAA50, XV_POOL },
	{ 0x080BAA50, 0x080BAA58, XV_TABLE },
	{ 0x080C66BA, 0x080C66BC, XV_RAW },
	{ 0x080C66BC, 0x080C66C0, XV_POOL },
	{ 0x080C66C0, 0x080C66CC, XV_TABLE },
	{ 0x080C694E, 0x080C6950, XV_RAW },
	{ 0x080C6950, 0x080C6954, XV_POOL },
	{ 0x080C6954, 0x080C6960, XV_TABLE },
	{ 0x080C6A4A, 0x080C6A4C, XV_RAW },
	{ 0x080C6A4C, 0x080C6A50, XV_POOL },
	{ 0x080C6A50, 0x080C6A58, XV_TABLE },
	{ 0x080C6B28, 0x080C6B2C, XV_POOL },
	{ 0x080E2556, 0x080E2558, XV_RAW },
	{ 0x080E2558, 0x080E255C, XV_POOL },
	{ 0x080E255C, 0x080E2568, XV_TABLE },
	{ 0x0810393A, 0x0810393C, XV_RAW },
	{ 0x0810393C, 0x08103940, XV_POOL },
	{ 0x08103940, 0x0810394C, XV_TABLE },
	{ 0x08103996, 0x08103998, XV_RAW },
	{ 0x08103998, 0x0810399C, XV_POOL },
	{ 0x0810399C, 0x081039AC, XV_RAW },
	{ 0x08103A10, 0x08103A14, XV_POOL },
	{ 0x08103D74, 0x08103D78, XV_POOL },
	{ 0x08103D78, 0x08103D84, XV_TABLE },
	{ 0x0810408C, 0x08104090, XV_POOL },
	{ 0x08104090, 0x081040A0, XV_RAW },
	{ 0x08104116, 0x08104118, XV_RAW },
	{ 0x08104118, 0x0810411C, XV_POOL },
	{ 0x0810411C, 0x0810412C, XV_RAW },
	{ 0x0810423C, 0x08104298, XV_RAW },
	{ 0x08104298, 0x0810429C, XV_POOL },
};
static const XvPair shademan_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800DA90, 0x0800FD0A },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x080E14AA, 0x080E268E },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08010DEE, 0x08013774 },   /* map:calls+bytes, SAME */
	{ 0x0800C6D8, 0x0800E2D8 },   /* map:calls, DIFF (audited) */
	{ 0x0800C758, 0x0800E2EC },   /* map:bytes, SAME */
	{ 0x080E1534, 0x080E2718 },   /* map:calls, DIFF (audited) */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x0802B3E0, 0x0802D234 },   /* map:calls+bytes, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800B380, 0x0800CCBE },   /* project:research, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB00, 0x0800E6E8 },   /* map:bytes, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800AFC2, 0x0800C90A },   /* map:bytes, DIFF (audited) */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x080E450E, 0x080E5736 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800D20C, 0x0800F2F0 },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800A550, 0x0800BF5C },   /* map:calls+bytes, SAME */
	{ 0x0800A50A, 0x0800BF16 },   /* map:bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x0800B9A8, 0x0800D3A6 },   /* map:bytes, SAME */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800B5A8, 0x0800CFA6 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800C6C2, 0x0800E2C2 },   /* map:calls+bytes, SAME */
	{ 0x08012C1E, 0x08015E46 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
};
static const XvPair shademan_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x08009FB3, 0x0800B917 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm shademan_imms[] = {
	{ 0x080BA9A2, THUMB_MOVS, 2, 0, 0xD, 0x27 },   /* its sprite: list 2 0xd in BN5, list 2 0x27 in BN6 */
	{ 0x080C66D0, THUMB_MOVS, 2, 0, 0xD, 0x27 },   /* its sprite: list 2 0xd in BN5, list 2 0x27 in BN6 */
	{ 0x080C6776, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C6964, THUMB_MOVS, 2, 0, 0xD, 0x27 },   /* its sprite: list 2 0xd in BN5, list 2 0x27 in BN6 */
	{ 0x080C69F6, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080FD330, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD36C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD3C4, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD3FC, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD42A, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD488, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD4AE, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD4E2, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD548, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FD57C, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FDB62, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
};
static const XvNumber shademan_numbers[] = {
	{ 0x080C6B28, 0x0A800000, 0x0A800000 },
	{ 0x08104298, 0x0705FF04, 0x0705FF04 },
};
static const XvRef shademan_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080FD2E1), OWN(0x08103929), OWN(0x080FD73B), OWN(0x080FD62D), OWN(0x08103D63),
};
static const XvSprite shademan_sprites[] = {
	{ 5, 0x02, 5, 0xA2, XN_SHARED_ART + 0x7000, 0x1800 },
};
static const XvId shademan_effects[] = { { 0x02, 0xA2 } };
static const XvType shademan_types[] = { { 1, 0x0B, XV_TYPE_MOVED }, { 3, 0x0E, XV_TYPE_MOVED }, { 3, 0x10, XV_TYPE_MOVED }, { 4, 0x1A, XV_TYPE_MOVED } };
static const XnSound shademan_sounds[] = {
	{ 0x080C6A3A, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
};

static const XNavi bn5_shademan = {
	.code = {
		.name = "ShadeMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 13,
		.ai = 39,
		.first_id = XN_FIRST_ID + 0x1C,
		.home = XN_HOMES + 0x4000,
		.blocks = shademan_blocks, .nblocks = (int)(sizeof shademan_blocks / sizeof *shademan_blocks),
		.data = shademan_data, .ndata = (int)(sizeof shademan_data / sizeof *shademan_data),
		.calls = shademan_calls, .ncalls = (int)(sizeof shademan_calls / sizeof *shademan_calls),
		.words = shademan_words, .nwords = (int)(sizeof shademan_words / sizeof *shademan_words),
		.imms = shademan_imms, .nimms = (int)(sizeof shademan_imms / sizeof *shademan_imms),
		.numbers = shademan_numbers, .nnumbers = (int)(sizeof shademan_numbers / sizeof *shademan_numbers),
		.states = shademan_states, .nstates = (int)(sizeof shademan_states / sizeof *shademan_states),
		.hooks = { OWN(0x080FD585), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080FD188, .struct2 = 0x080FD190,
		.names = { 0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x15, .element = 0, .weakness = 0, .shadow = true },
		.sprites = shademan_sprites, .nsprites = (int)(sizeof shademan_sprites / sizeof *shademan_sprites),
		.effects = shademan_effects, .neffects = (int)(sizeof shademan_effects / sizeof *shademan_effects),
		.types = shademan_types, .ntypes = (int)(sizeof shademan_types / sizeof *shademan_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801AAC1), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 13,
	.sounds = shademan_sounds, .nsounds = (int)(sizeof shademan_sounds / sizeof *shademan_sounds),
};
/* ---- CosmoMan (BN5's: AI 16 there, its ids 0x15b, 0x15c, 0x15d, 0x15e), as Navi AI 42 with ids 0x1ec-0x1ef ---- */

static const XvBlock cosmoman_blocks[] = {
	{ 0x080FF810, 0x08100358, XN_MORE + 0xB400, 0x0B218E2Eu },
	{ 0x080C5386, 0x080C53DE, XN_MORE + 0xC002, 0x4A1B50AFu },
	{ 0x080D3640, 0x080D39A8, XN_MORE + 0xC100, 0x47E68561u },
	{ 0x080D3CDC, 0x080D41C8, XN_MORE + 0xC500, 0x0D39129Bu },
	{ 0x080D4908, 0x080D4C48, XN_MORE + 0xCA00, 0x015A1E79u },
	{ 0x080D5634, 0x080D56FE, XN_MORE + 0xCE00, 0x03D6771Fu },
	{ 0x080D6EB0, 0x080D7270, XN_MORE + 0xCF00, 0x67956633u },
	{ 0x080E57B4, 0x080E596E, XN_MORE + 0xD300, 0x6B23036Au },
};
static const XvData cosmoman_data[] = {
	{ 0x080FF810, 0x080FF840, XV_ZERO },
	{ 0x080FF856, 0x080FF858, XV_RAW },
	{ 0x080FF858, 0x080FF85C, XV_POOL },
	{ 0x080FF85C, 0x080FF86C, XV_TABLE },
	{ 0x080FF970, 0x080FF974, XV_POOL },
	{ 0x080FF974, 0x080FF978, XV_RAW },
	{ 0x080FF978, 0x080FF97C, XV_POOL },
	{ 0x080FF97C, 0x080FF980, XV_RAW },
	{ 0x080FF9B6, 0x080FF9B8, XV_RAW },
	{ 0x080FF9B8, 0x080FF9BC, XV_POOL },
	{ 0x080FF9BC, 0x080FF9C0, XV_RAW },
	{ 0x080FFA42, 0x080FFA44, XV_RAW },
	{ 0x080FFA44, 0x080FFA48, XV_POOL },
	{ 0x080FFA48, 0x080FFA50, XV_RAW },
	{ 0x080FFA50, 0x080FFA54, XV_POOL },
	{ 0x080FFA98, 0x080FFA9C, XV_POOL },
	{ 0x080FFA9C, 0x080FFAA0, XV_RAW },
	{ 0x080FFAAA, 0x080FFAAC, XV_RAW },
	{ 0x080FFAAC, 0x080FFAB0, XV_POOL },
	{ 0x080FFAB0, 0x080FFAB4, XV_RAW },
	{ 0x080FFABE, 0x080FFAC0, XV_RAW },
	{ 0x080FFAC0, 0x080FFAC4, XV_POOL },
	{ 0x080FFAC4, 0x080FFAC8, XV_RAW },
	{ 0x080FFAD2, 0x080FFAD4, XV_RAW },
	{ 0x080FFAD4, 0x080FFAD8, XV_POOL },
	{ 0x080FFAD8, 0x080FFADC, XV_RAW },
	{ 0x080FFAE6, 0x080FFAE8, XV_RAW },
	{ 0x080FFAE8, 0x080FFAEC, XV_POOL },
	{ 0x080FFAEC, 0x080FFAF0, XV_RAW },
	{ 0x080FFB14, 0x080FFB24, XV_POOL },
	{ 0x080FFB24, 0x080FFB2E, XV_RAW },
	{ 0x080FFB2E, 0x080FFB38, XV_RAW },
	{ 0x080FFB38, 0x080FFB42, XV_RAW },
	{ 0x080FFB42, 0x080FFB4C, XV_RAW },
	{ 0x080FFB4C, 0x080FFB50, XV_POOL },
	{ 0x080FFB50, 0x080FFB54, XV_RAW },
	{ 0x080FFB54, 0x080FFB64, XV_POOL },
	{ 0x080FFB72, 0x080FFB74, XV_RAW },
	{ 0x080FFB74, 0x080FFB78, XV_POOL },
	{ 0x080FFB78, 0x080FFB84, XV_TABLE },
	{ 0x080FFBCC, 0x080FFBD0, XV_POOL },
	{ 0x080FFBD0, 0x080FFBE0, XV_RAW },
	{ 0x080FFC54, 0x080FFC58, XV_POOL },
	{ 0x080FFC58, 0x080FFC68, XV_TABLE },
	{ 0x080FFD36, 0x080FFD38, XV_RAW },
	{ 0x080FFD38, 0x080FFD3C, XV_POOL },
	{ 0x080FFD3C, 0x080FFD50, XV_TABLE },
	{ 0x080FFD98, 0x080FFD9C, XV_POOL },
	{ 0x080FFD9C, 0x080FFDAC, XV_RAW },
	{ 0x080FFEB4, 0x080FFEBC, XV_POOL },
	{ 0x080FFEBC, 0x080FFEC4, XV_RAW },
	{ 0x080FFEC4, 0x080FFEC8, XV_POOL },
	{ 0x080FFEC8, 0x080FFEF8, XV_RAW },
	{ 0x080FFF2A, 0x080FFF2C, XV_RAW },
	{ 0x080FFF2C, 0x080FFF30, XV_POOL },
	{ 0x080FFF30, 0x080FFF3C, XV_TABLE },
	{ 0x080FFF66, 0x080FFF68, XV_RAW },
	{ 0x080FFF68, 0x080FFF6C, XV_POOL },
	{ 0x080FFF6C, 0x080FFF74, XV_RAW },
	{ 0x080FFFD8, 0x080FFFDC, XV_POOL },
	{ 0x0810000E, 0x08100010, XV_RAW },
	{ 0x08100010, 0x08100014, XV_POOL },
	{ 0x08100014, 0x08100030, XV_TABLE },
	{ 0x08100078, 0x0810007C, XV_POOL },
	{ 0x0810007C, 0x0810008C, XV_RAW },
	{ 0x081001CA, 0x081001CC, XV_RAW },
	{ 0x081001CC, 0x081001D0, XV_POOL },
	{ 0x081001D0, 0x081001D8, XV_RAW },
	{ 0x0810028A, 0x0810028C, XV_RAW },
	{ 0x0810028C, 0x08100290, XV_POOL },
	{ 0x08100290, 0x08100294, XV_RAW },
	{ 0x081002AE, 0x081002B0, XV_RAW },
	{ 0x081002B0, 0x081002B4, XV_POOL },
	{ 0x081002B4, 0x081002B8, XV_RAW },
	{ 0x081002C6, 0x081002C8, XV_RAW },
	{ 0x081002C8, 0x081002CC, XV_POOL },
	{ 0x081002CC, 0x081002E0, XV_RAW },
	{ 0x081002EE, 0x081002F0, XV_RAW },
	{ 0x081002F0, 0x081002F4, XV_POOL },
	{ 0x081002F4, 0x08100308, XV_RAW },
	{ 0x08100356, 0x08100358, XV_RAW },
	{ 0x080D3640, 0x080D3648, XV_RAW },
	{ 0x080D365A, 0x080D365C, XV_RAW },
	{ 0x080D365C, 0x080D3660, XV_POOL },
	{ 0x080D3660, 0x080D366C, XV_TABLE },
	{ 0x080D3768, 0x080D376C, XV_POOL },
	{ 0x080D376C, 0x080D3774, XV_TABLE },
	{ 0x080D3774, 0x080D3778, XV_POOL },
	{ 0x080D380C, 0x080D3810, XV_POOL },
	{ 0x080D3810, 0x080D3820, XV_RAW },
	{ 0x080D3854, 0x080D3870, XV_RAW },
	{ 0x080D3870, 0x080D3874, XV_POOL },
	{ 0x080D3874, 0x080D387C, XV_RAW },
	{ 0x080D3894, 0x080D3898, XV_POOL },
	{ 0x080D38AA, 0x080D38AC, XV_RAW },
	{ 0x080D38AC, 0x080D38B0, XV_POOL },
	{ 0x080D38B0, 0x080D38BC, XV_TABLE },
	{ 0x080D3958, 0x080D395C, XV_POOL },
	{ 0x080D3CDC, 0x080D3CE4, XV_RAW },
	{ 0x080D3CF6, 0x080D3CF8, XV_RAW },
	{ 0x080D3CF8, 0x080D3CFC, XV_POOL },
	{ 0x080D3CFC, 0x080D3D08, XV_TABLE },
	{ 0x080D3DA4, 0x080D3DA8, XV_POOL },
	{ 0x080D3DA8, 0x080D3DC0, XV_TABLE },
	{ 0x080D3EFC, 0x080D3F00, XV_POOL },
	{ 0x080D3F00, 0x080D3F04, XV_RAW },
	{ 0x080D3F04, 0x080D3F08, XV_POOL },
	{ 0x080D3F08, 0x080D3F18, XV_RAW },
	{ 0x080D3F18, 0x080D3F1C, XV_POOL },
	{ 0x080D3F1C, 0x080D3F2C, XV_RAW },
	{ 0x080D3F2C, 0x080D3F30, XV_POOL },
	{ 0x080D3F30, 0x080D3F38, XV_RAW },
	{ 0x080D3FAC, 0x080D3FB0, XV_POOL },
	{ 0x080D3FB0, 0x080D3FD0, XV_RAW },
	{ 0x080D3FD0, 0x080D3FD4, XV_POOL },
	{ 0x080D3FD4, 0x080D3FDC, XV_RAW },
	{ 0x080D4120, 0x080D4124, XV_POOL },
	{ 0x080D4124, 0x080D4150, XV_RAW },
	{ 0x080D4188, 0x080D418C, XV_POOL },
	{ 0x080D418C, 0x080D41AC, XV_RAW },
	{ 0x080D41AC, 0x080D41C8, XV_POOL },
	{ 0x080D4908, 0x080D4914, XV_RAW },
	{ 0x080D4938, 0x080D493C, XV_POOL },
	{ 0x080D493C, 0x080D4948, XV_TABLE },
	{ 0x080D49FE, 0x080D4A00, XV_RAW },
	{ 0x080D4A00, 0x080D4A04, XV_POOL },
	{ 0x080D4A04, 0x080D4A2C, XV_RAW },
	{ 0x080D4A2C, 0x080D4A30, XV_POOL },
	{ 0x080D4A30, 0x080D4A38, XV_RAW },
	{ 0x080D4A76, 0x080D4A88, XV_RAW },
	{ 0x080D4A92, 0x080D4A94, XV_RAW },
	{ 0x080D4A94, 0x080D4A98, XV_POOL },
	{ 0x080D4A98, 0x080D4AA4, XV_TABLE },
	{ 0x080D4AA4, 0x080D4AA8, XV_POOL },
	{ 0x080D4C2C, 0x080D4C30, XV_POOL },
	{ 0x080D4C30, 0x080D4C38, XV_RAW },
	{ 0x080D4C38, 0x080D4C3C, XV_POOL },
	{ 0x080D4C3C, 0x080D4C44, XV_RAW },
	{ 0x080D4C44, 0x080D4C48, XV_POOL },
	{ 0x080D5642, 0x080D5644, XV_RAW },
	{ 0x080D5644, 0x080D5648, XV_POOL },
	{ 0x080D5648, 0x080D5654, XV_TABLE },
	{ 0x080D56CE, 0x080D56D0, XV_RAW },
	{ 0x080D56D0, 0x080D56D4, XV_POOL },
	{ 0x080D56D4, 0x080D56DC, XV_RAW },
	{ 0x080D56DC, 0x080D56E0, XV_POOL },
	{ 0x080D6EC2, 0x080D6EC4, XV_RAW },
	{ 0x080D6EC4, 0x080D6EC8, XV_POOL },
	{ 0x080D6EC8, 0x080D6ED4, XV_TABLE },
	{ 0x080D6F92, 0x080D6F94, XV_RAW },
	{ 0x080D6F94, 0x080D6F98, XV_POOL },
	{ 0x080D6F98, 0x080D6FA0, XV_TABLE },
	{ 0x080D7070, 0x080D7074, XV_POOL },
	{ 0x080D7074, 0x080D7080, XV_RAW },
	{ 0x080D7080, 0x080D7084, XV_POOL },
	{ 0x080D70C6, 0x080D70C8, XV_RAW },
	{ 0x080D70C8, 0x080D70D0, XV_POOL },
	{ 0x080D70E2, 0x080D70E4, XV_RAW },
	{ 0x080D70E4, 0x080D70E8, XV_POOL },
	{ 0x080D70E8, 0x080D70F4, XV_TABLE },
	{ 0x080D71A2, 0x080D71A4, XV_RAW },
	{ 0x080D71A4, 0x080D71A8, XV_POOL },
	{ 0x080D71A8, 0x080D71B4, XV_TABLE },
	{ 0x080D7236, 0x080D7238, XV_RAW },
	{ 0x080D7238, 0x080D723C, XV_POOL },
	{ 0x080D723C, 0x080D7244, XV_RAW },
	{ 0x080E57C6, 0x080E57C8, XV_RAW },
	{ 0x080E57C8, 0x080E57CC, XV_POOL },
	{ 0x080E57CC, 0x080E57D8, XV_TABLE },
	{ 0x080E5844, 0x080E5848, XV_POOL },
	{ 0x080E5848, 0x080E5854, XV_TABLE },
	{ 0x080E58FE, 0x080E5900, XV_RAW },
	{ 0x080E5900, 0x080E5904, XV_POOL },
	{ 0x080E5904, 0x080E5910, XV_RAW },
};
static const XvPair cosmoman_calls[] = {
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x0800CB20, 0x0800E708 },   /* map:bytes, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800AFF0, 0x0800C938 },   /* map:calls, DIFF (audited) */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
};
static const XvPair cosmoman_words[] = {
	{ 0x08005CD0, 0x080065E0 },
	{ 0x08005D50, 0x08006660 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm cosmoman_imms[] = {
	{ 0x080D3674, THUMB_MOVS, 2, 0, 0x10, 0x2A },   /* its sprite: list 2 0x10 in BN5, list 2 0x2a in BN6 */
	{ 0x080D3704, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D390A, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D3D10, THUMB_MOVS, 2, 0, 0x10, 0x2A },   /* its sprite: list 2 0x10 in BN5, list 2 0x2a in BN6 */
	{ 0x080D3EAC, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D3F3C, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D4950, THUMB_MOVS, 2, 0, 0x10, 0x2A },   /* its sprite: list 2 0x10 in BN5, list 2 0x2a in BN6 */
	{ 0x080D4A4C, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D5696, THUMB_LDR, 0, 3, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D6EFE, THUMB_MOVS, 2, 0, 0x10, 0x2A },   /* its sprite: list 2 0x10 in BN5, list 2 0x2a in BN6 */
	{ 0x080D70FC, THUMB_MOVS, 2, 0, 0x10, 0x2A },   /* its sprite: list 2 0x10 in BN5, list 2 0x2a in BN6 */
	{ 0x080D721A, THUMB_LDR, 0, 3, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080E57E4, THUMB_MOVS, 2, 0, 0x10, 0x2A },   /* its sprite: list 2 0x10 in BN5, list 2 0x2a in BN6 */
	{ 0x080FF8AC, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FF8F6, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FF916, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080FF966, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FFA24, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber cosmoman_numbers[] = {
	{ 0x080D3774, 0x0C800000, 0x0C800000 },
	{ 0x080D3958, 0x0C800000, 0x0C800000 },
	{ 0x080D7080, 0x04050701, 0x04050701 },
};
static const XvRef cosmoman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080FF841), OWN(0x080FFB65), OWN(0x080FFC47), OWN(0x080FFD29), OWN(0x080FFF1D), OWN(0x08100001),
};
static const XvType cosmoman_types[] = { { 3, 0x6C, XV_TYPE_MOVED }, { 3, 0x6D, XV_TYPE_MOVED }, { 3, 0x70, XV_TYPE_MOVED }, { 3, 0x74, XV_TYPE_MOVED }, { 3, 0x79, XV_TYPE_MOVED }, { 3, 0x85, XV_TYPE_MOVED }, { 3, 0x86, XV_TYPE_MOVED }, { 4, 0x45, XV_TYPE_MOVED } };
static const XnSound cosmoman_sounds[] = {
	{ 0x080D3740, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080D7064, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
};

static const XNavi bn5_cosmoman = {
	.code = {
		.name = "CosmoMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 16,
		.ai = 42,
		.first_id = XN_FIRST_ID + 0x28,
		.home = XN_HOMES + 0x5800,
		.blocks = cosmoman_blocks, .nblocks = (int)(sizeof cosmoman_blocks / sizeof *cosmoman_blocks),
		.data = cosmoman_data, .ndata = (int)(sizeof cosmoman_data / sizeof *cosmoman_data),
		.calls = cosmoman_calls, .ncalls = (int)(sizeof cosmoman_calls / sizeof *cosmoman_calls),
		.words = cosmoman_words, .nwords = (int)(sizeof cosmoman_words / sizeof *cosmoman_words),
		.imms = cosmoman_imms, .nimms = (int)(sizeof cosmoman_imms / sizeof *cosmoman_imms),
		.numbers = cosmoman_numbers, .nnumbers = (int)(sizeof cosmoman_numbers / sizeof *cosmoman_numbers),
		.states = cosmoman_states, .nstates = (int)(sizeof cosmoman_states / sizeof *cosmoman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), OWN(0x080FFA6B), OWN(0x080FFA73), OWN(0x080FFA73), OWN(0x080FFA73), OWN(0x080FFA73), OWN(0x080FFA73) },
		.struct1 = 0x080FF724, .struct2 = 0x080FF72C,
		.names = { 0x15B, 0x15C, 0x15D, 0x15E, 0x15F, 0x160 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x05, .element = 0, .weakness = 0, .shadow = false },
		.types = cosmoman_types, .ntypes = (int)(sizeof cosmoman_types / sizeof *cosmoman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801AAC1), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 16,
	.sounds = cosmoman_sounds, .nsounds = (int)(sizeof cosmoman_sounds / sizeof *cosmoman_sounds),
};
/* ---- CloudMan (BN5's: AI 15 there, its ids 0x155, 0x156, 0x157, 0x158), as Navi AI 41 with ids 0x1e8-0x1eb ---- */

static const XvBlock cloudman_blocks[] = {
	{ 0x080FEF84, 0x080FF724, XN_MORE + 0x7800, 0xDCFDC69Du },
	{ 0x080C5386, 0x080C53DE, XN_MORE + 0x8002, 0x4A1B50AFu },
	{ 0x080C5DF4, 0x080C6000, XN_MORE + 0x8100, 0xEDB80F1Au },
	{ 0x080CB8E0, 0x080CBFBC, XN_MORE + 0x84E0, 0xF638673Cu },
	{ 0x080CCCF0, 0x080CCEDA, XN_MORE + 0x8C00, 0xF172E552u },
	{ 0x080CD484, 0x080CDA78, XN_MORE + 0x8E00, 0xEABEC922u },
	{ 0x080CDDE0, 0x080CE114, XN_MORE + 0x9400, 0x62E1A456u },
	{ 0x080D1E68, 0x080D2010, XN_MORE + 0x9800, 0x3590FE54u },
	{ 0x080FF724, 0x080FF794, XN_MORE + 0x9A00, 0x8C751699u },
};
static const XvData cloudman_data[] = {
	{ 0x080FEF84, 0x080FEFAC, XV_ZERO },
	{ 0x080FEFC2, 0x080FEFC4, XV_RAW },
	{ 0x080FEFC4, 0x080FEFC8, XV_POOL },
	{ 0x080FEFC8, 0x080FEFD0, XV_TABLE },
	{ 0x080FF034, 0x080FF038, XV_POOL },
	{ 0x080FF038, 0x080FF03C, XV_RAW },
	{ 0x080FF0F0, 0x080FF0F4, XV_POOL },
	{ 0x080FF0F4, 0x080FF100, XV_RAW },
	{ 0x080FF124, 0x080FF128, XV_POOL },
	{ 0x080FF128, 0x080FF12C, XV_RAW },
	{ 0x080FF136, 0x080FF138, XV_RAW },
	{ 0x080FF138, 0x080FF13C, XV_POOL },
	{ 0x080FF13C, 0x080FF140, XV_RAW },
	{ 0x080FF14A, 0x080FF14C, XV_RAW },
	{ 0x080FF14C, 0x080FF150, XV_POOL },
	{ 0x080FF150, 0x080FF154, XV_RAW },
	{ 0x080FF178, 0x080FF184, XV_POOL },
	{ 0x080FF184, 0x080FF18E, XV_RAW },
	{ 0x080FF18E, 0x080FF198, XV_RAW },
	{ 0x080FF198, 0x080FF1A4, XV_RAW },
	{ 0x080FF1A4, 0x080FF1A8, XV_POOL },
	{ 0x080FF1A8, 0x080FF1AC, XV_RAW },
	{ 0x080FF1AC, 0x080FF1B8, XV_POOL },
	{ 0x080FF1C6, 0x080FF1C8, XV_RAW },
	{ 0x080FF1C8, 0x080FF1CC, XV_POOL },
	{ 0x080FF1CC, 0x080FF1D8, XV_TABLE },
	{ 0x080FF240, 0x080FF244, XV_POOL },
	{ 0x080FF244, 0x080FF254, XV_RAW },
	{ 0x080FF254, 0x080FF258, XV_POOL },
	{ 0x080FF258, 0x080FF268, XV_RAW },
	{ 0x080FF2F8, 0x080FF2FC, XV_POOL },
	{ 0x080FF2FC, 0x080FF30C, XV_RAW },
	{ 0x080FF342, 0x080FF344, XV_RAW },
	{ 0x080FF344, 0x080FF348, XV_POOL },
	{ 0x080FF348, 0x080FF350, XV_TABLE },
	{ 0x080FF3B2, 0x080FF3B4, XV_RAW },
	{ 0x080FF3B4, 0x080FF3B8, XV_POOL },
	{ 0x080FF3B8, 0x080FF3DC, XV_TABLE },
	{ 0x080FF4B8, 0x080FF4BC, XV_POOL },
	{ 0x080FF4BC, 0x080FF4C4, XV_RAW },
	{ 0x080FF60E, 0x080FF610, XV_RAW },
	{ 0x080FF610, 0x080FF614, XV_POOL },
	{ 0x080FF614, 0x080FF618, XV_RAW },
	{ 0x080FF618, 0x080FF61C, XV_POOL },
	{ 0x080FF61C, 0x080FF628, XV_RAW },
	{ 0x080FF6AE, 0x080FF6B0, XV_RAW },
	{ 0x080FF6B0, 0x080FF6B4, XV_POOL },
	{ 0x080FF6B4, 0x080FF6C4, XV_RAW },
	{ 0x080FF6C4, 0x080FF6C8, XV_POOL },
	{ 0x080FF6C8, 0x080FF6D8, XV_RAW },
	{ 0x080FF706, 0x080FF708, XV_RAW },
	{ 0x080FF708, 0x080FF70C, XV_POOL },
	{ 0x080FF70C, 0x080FF710, XV_RAW },
	{ 0x080FF710, 0x080FF714, XV_POOL },
	{ 0x080FF714, 0x080FF724, XV_RAW },
	{ 0x080C5DF4, 0x080C5EB4, XV_RAW },
	{ 0x080C5EC6, 0x080C5EC8, XV_RAW },
	{ 0x080C5EC8, 0x080C5ECC, XV_POOL },
	{ 0x080C5ECC, 0x080C5ED8, XV_TABLE },
	{ 0x080C5FE8, 0x080C5FFC, XV_RAW },
	{ 0x080C5FFC, 0x080C6000, XV_POOL },
	{ 0x080CB8F2, 0x080CB8F4, XV_RAW },
	{ 0x080CB8F4, 0x080CB8F8, XV_POOL },
	{ 0x080CB8F8, 0x080CB904, XV_TABLE },
	{ 0x080CB94E, 0x080CB950, XV_RAW },
	{ 0x080CB950, 0x080CB954, XV_POOL },
	{ 0x080CB954, 0x080CB95C, XV_TABLE },
	{ 0x080CB9CA, 0x080CB9CC, XV_RAW },
	{ 0x080CB9CC, 0x080CB9D4, XV_POOL },
	{ 0x080CB9D4, 0x080CB9DC, XV_RAW },
	{ 0x080CBAEE, 0x080CBAF0, XV_RAW },
	{ 0x080CBAF0, 0x080CBAF4, XV_POOL },
	{ 0x080CBAF4, 0x080CBB04, XV_RAW },
	{ 0x080CBB04, 0x080CBB08, XV_POOL },
	{ 0x080CBB08, 0x080CBB10, XV_RAW },
	{ 0x080CBB10, 0x080CBB14, XV_POOL },
	{ 0x080CBB14, 0x080CBB1C, XV_RAW },
	{ 0x080CBB52, 0x080CBB54, XV_RAW },
	{ 0x080CBB54, 0x080CBB58, XV_POOL },
	{ 0x080CBB58, 0x080CBB68, XV_RAW },
	{ 0x080CBB7A, 0x080CBB7C, XV_RAW },
	{ 0x080CBB7C, 0x080CBB80, XV_POOL },
	{ 0x080CBB80, 0x080CBB90, XV_TABLE },
	{ 0x080CBC18, 0x080CBC1C, XV_POOL },
	{ 0x080CBC1C, 0x080CBC24, XV_RAW },
	{ 0x080CBC88, 0x080CBC8C, XV_POOL },
	{ 0x080CBC8C, 0x080CBCA4, XV_TABLE },
	{ 0x080CBD72, 0x080CBD74, XV_RAW },
	{ 0x080CBD74, 0x080CBD78, XV_POOL },
	{ 0x080CBD78, 0x080CBD7C, XV_RAW },
	{ 0x080CBE18, 0x080CBE1C, XV_POOL },
	{ 0x080CBE1C, 0x080CBE24, XV_RAW },
	{ 0x080CBE40, 0x080CBE44, XV_POOL },
	{ 0x080CBE44, 0x080CBE4C, XV_TABLE },
	{ 0x080CBFB0, 0x080CBFB4, XV_POOL },
	{ 0x080CBFB4, 0x080CBFB8, XV_RAW },
	{ 0x080CBFB8, 0x080CBFBC, XV_POOL },
	{ 0x080CCD02, 0x080CCD04, XV_RAW },
	{ 0x080CCD04, 0x080CCD08, XV_POOL },
	{ 0x080CCD08, 0x080CCD14, XV_TABLE },
	{ 0x080CCDB6, 0x080CCDB8, XV_RAW },
	{ 0x080CCDB8, 0x080CCDBC, XV_POOL },
	{ 0x080CCDBC, 0x080CCDCC, XV_TABLE },
	{ 0x080CCDF6, 0x080CCDF8, XV_RAW },
	{ 0x080CCDF8, 0x080CCDFC, XV_POOL },
	{ 0x080CCDFC, 0x080CCE04, XV_RAW },
	{ 0x080CD484, 0x080CD494, XV_RAW },
	{ 0x080CD4A6, 0x080CD4A8, XV_RAW },
	{ 0x080CD4A8, 0x080CD4AC, XV_POOL },
	{ 0x080CD4AC, 0x080CD4B8, XV_TABLE },
	{ 0x080CD558, 0x080CD55C, XV_POOL },
	{ 0x080CD55C, 0x080CD568, XV_TABLE },
	{ 0x080CD69A, 0x080CD69C, XV_RAW },
	{ 0x080CD69C, 0x080CD6A0, XV_POOL },
	{ 0x080CD7C6, 0x080CD7C8, XV_RAW },
	{ 0x080CD7C8, 0x080CD7CC, XV_POOL },
	{ 0x080CD7CC, 0x080CD7DC, XV_RAW },
	{ 0x080CD7DC, 0x080CD7E0, XV_POOL },
	{ 0x080CD7E0, 0x080CD7E8, XV_RAW },
	{ 0x080CD86C, 0x080CD870, XV_POOL },
	{ 0x080CD870, 0x080CD880, XV_RAW },
	{ 0x080CD880, 0x080CD884, XV_POOL },
	{ 0x080CD884, 0x080CD88C, XV_RAW },
	{ 0x080CD88C, 0x080CD894, XV_POOL },
	{ 0x080CD894, 0x080CD89C, XV_RAW },
	{ 0x080CD8AE, 0x080CD8B0, XV_RAW },
	{ 0x080CD8B0, 0x080CD8B4, XV_POOL },
	{ 0x080CD8B4, 0x080CD8C0, XV_TABLE },
	{ 0x080CD9A2, 0x080CD9A4, XV_RAW },
	{ 0x080CD9A4, 0x080CD9A8, XV_POOL },
	{ 0x080CD9A8, 0x080CD9B4, XV_TABLE },
	{ 0x080CDA06, 0x080CDA08, XV_RAW },
	{ 0x080CDA08, 0x080CDA0C, XV_POOL },
	{ 0x080CDA0C, 0x080CDA14, XV_TABLE },
	{ 0x080CDA74, 0x080CDA78, XV_POOL },
	{ 0x080CDDE0, 0x080CDDE4, XV_RAW },
	{ 0x080CDDF6, 0x080CDDF8, XV_RAW },
	{ 0x080CDDF8, 0x080CDDFC, XV_POOL },
	{ 0x080CDDFC, 0x080CDE08, XV_TABLE },
	{ 0x080CDE74, 0x080CDE78, XV_POOL },
	{ 0x080CDE78, 0x080CDE7C, XV_TABLE },
	{ 0x080CE044, 0x080CE048, XV_POOL },
	{ 0x080CE048, 0x080CE050, XV_RAW },
	{ 0x080CE0FE, 0x080CE10C, XV_RAW },
	{ 0x080CE10C, 0x080CE114, XV_POOL },
	{ 0x080D1E68, 0x080D1E6C, XV_RAW },
	{ 0x080D1E7E, 0x080D1E80, XV_RAW },
	{ 0x080D1E80, 0x080D1E84, XV_POOL },
	{ 0x080D1E84, 0x080D1E90, XV_TABLE },
	{ 0x080D1EDA, 0x080D1EDC, XV_RAW },
	{ 0x080D1EDC, 0x080D1EE0, XV_POOL },
	{ 0x080D1EE0, 0x080D1EE4, XV_TABLE },
	{ 0x080D1F2C, 0x080D1F30, XV_POOL },
	{ 0x080D1F30, 0x080D1F38, XV_RAW },
	{ 0x080D1F9E, 0x080D1FAA, XV_RAW },
	{ 0x080D2006, 0x080D2008, XV_RAW },
	{ 0x080D2008, 0x080D2010, XV_POOL },
	{ 0x080FF724, 0x080FF794, XV_RAW },
};
static const XvPair cloudman_calls[] = {
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
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800C758, 0x0800E2EC },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x080CBFBC, XN_STUB_CLOUDS },   /* a stub: what BN6's engine lacks */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800CB20, 0x0800E708 },   /* map:bytes, SAME */
	{ 0x08000B92, 0x08000C72 },   /* map:calls+bytes, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x080E5084, 0x080E666C },   /* map:bytes, SAME */
	{ 0x080E5168, 0x080E6750 },   /* map:bytes, SAME */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
};
static const XvPair cloudman_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm cloudman_imms[] = {
	{ 0x080C5F74, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CB930, THUMB_MOVS, 1, 0, 0x8, 0xA },   /* its parent's CurAction 8 compared: BN6's 10 */
	{ 0x080CBBA4, THUMB_MOVS, 2, 0, 0xF, 0x29 },   /* its sprite: list 2 0xf in BN5, list 2 0x29 in BN6 */
	{ 0x080CBC4A, THUMB_MOVS, 1, 0, 0x9, 0xB },   /* its parent's CurAction 9 compared: BN6's 11 */
	{ 0x080CBCD8, THUMB_MOVS, 1, 0, 0x8, 0xA },   /* its parent's CurAction 8 compared: BN6's 10 */
	{ 0x080CBE94, THUMB_MOVS, 1, 0, 0x9, 0xB },   /* its parent's CurAction 9 compared: BN6's 11 */
	{ 0x080CCD20, THUMB_MOVS, 2, 0, 0xF, 0x29 },   /* its sprite: list 2 0xf in BN5, list 2 0x29 in BN6 */
	{ 0x080CCE68, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CD4C4, THUMB_MOVS, 2, 0, 0xF, 0x29 },   /* its sprite: list 2 0xf in BN5, list 2 0x29 in BN6 */
	{ 0x080CD538, THUMB_MOVS, 1, 0, 0x9, 0xB },   /* its parent's CurAction 9 compared: BN6's 11 */
	{ 0x080CD584, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CD5AC, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CD8CC, THUMB_MOVS, 2, 0, 0xF, 0x29 },   /* its sprite: list 2 0xf in BN5, list 2 0x29 in BN6 */
	{ 0x080D1EEA, THUMB_MOVS, 1, 0, 0x8, 0xA },   /* its parent's CurAction 8 compared: BN6's 10 */
	{ 0x080D1EF0, THUMB_MOVS, 1, 0, 0x9, 0xB },   /* its parent's CurAction 9 compared: BN6's 11 */
	{ 0x080FF02C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080FF078, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080FF0AE, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
};
static const XvByte cloudman_bytes[] = {
	{ 0x080C5DF9, 0x0C, 0x10 },   /* 080C5DF4's row 0: its sprite list 3 0x3e in BN5, list 4 0xb6 in BN6 (its list's offset) */
	{ 0x080C5DFA, 0x3E, 0xB6 },   /* 080C5DF4's row 0: its sprite list 3 0x3e in BN5, list 4 0xb6 in BN6 */
	{ 0x080C5E09, 0x0C, 0x10 },   /* 080C5DF4's row 1: its sprite list 3 0x43 in BN5, list 4 0xb7 in BN6 (its list's offset) */
	{ 0x080C5E0A, 0x43, 0xB7 },   /* 080C5DF4's row 1: its sprite list 3 0x43 in BN5, list 4 0xb7 in BN6 */
	{ 0x080C5E1A, 0x11, 0xB8 },   /* 080C5DF4's row 2: its sprite list 4 0x11 in BN5, list 4 0xb8 in BN6 */
	{ 0x080C5E2A, 0x11, 0xB8 },   /* 080C5DF4's row 3: its sprite list 4 0x11 in BN5, list 4 0xb8 in BN6 */
	{ 0x080C5E3A, 0x11, 0xB8 },   /* 080C5DF4's row 4: its sprite list 4 0x11 in BN5, list 4 0xb8 in BN6 */
	{ 0x080C5E4A, 0x11, 0xB8 },   /* 080C5DF4's row 5: its sprite list 4 0x11 in BN5, list 4 0xb8 in BN6 */
	{ 0x080C5E5A, 0x2D, 0xB9 },   /* 080C5DF4's row 6: its sprite list 4 0x2d in BN5, list 4 0xb9 in BN6 */
	{ 0x080C5E6A, 0x41, 0xBA },   /* 080C5DF4's row 7: its sprite list 4 0x41 in BN5, list 4 0xba in BN6 */
	{ 0x080C5E7A, 0x41, 0xBA },   /* 080C5DF4's row 8: its sprite list 4 0x41 in BN5, list 4 0xba in BN6 */
	{ 0x080C5E8A, 0x41, 0xBA },   /* 080C5DF4's row 9: its sprite list 4 0x41 in BN5, list 4 0xba in BN6 */
	{ 0x080C5E9A, 0x41, 0xBA },   /* 080C5DF4's row 10: its sprite list 4 0x41 in BN5, list 4 0xba in BN6 */
	{ 0x080C5EAA, 0x11, 0xB8 },   /* 080C5DF4's row 11: its sprite list 4 0x11 in BN5, list 4 0xb8 in BN6 */
};
static const XvRef cloudman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080FEFAD), OWN(0x080FF1B9), OWN(0x080FF335), OWN(0x080FF3A5),
};
static const XvSprite cloudman_sprites[] = {
	{ 3, 0x3E, 4, 0xB6, XN_SHARED_ART + 0x1C800, 0x800 },
	{ 3, 0x43, 4, 0xB7, XN_SHARED_ART + 0x1D000, 0x1000 },
	{ 4, 0x11, 4, 0xB8, XN_SHARED_ART + 0x1E000, 0x1000 },
	{ 4, 0x2D, 4, 0xB9, XN_SHARED_ART + 0x1F000, 0x800 },
	{ 4, 0x41, 4, 0xBA, XN_SHARED_ART + 0x1F800, 0x1000 },
};
static const XvType cloudman_types[] = { { 3, 0x0B, XV_TYPE_MOVED }, { 3, 0x36, XV_TYPE_MOVED }, { 3, 0x37, XV_TYPE_MOVED }, { 3, 0x3D, XV_TYPE_MOVED }, { 3, 0x40, XV_TYPE_MOVED }, { 3, 0x41, XV_TYPE_MOVED }, { 3, 0x44, XV_TYPE_MOVED }, { 3, 0x5F, XV_TYPE_MOVED } };
static const XnSound cloudman_sounds[] = {
	{ 0x080CCE5A, 0x00000000, 0x0BA },   /* BN6's song 0xba another */
};

static const XNavi bn5_cloudman = {
	.code = {
		.name = "CloudMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 15,
		.ai = 41,
		.first_id = XN_FIRST_ID + 0x24,
		.home = XN_HOMES + 0x5000,
		.blocks = cloudman_blocks, .nblocks = (int)(sizeof cloudman_blocks / sizeof *cloudman_blocks),
		.data = cloudman_data, .ndata = (int)(sizeof cloudman_data / sizeof *cloudman_data),
		.calls = cloudman_calls, .ncalls = (int)(sizeof cloudman_calls / sizeof *cloudman_calls),
		.words = cloudman_words, .nwords = (int)(sizeof cloudman_words / sizeof *cloudman_words),
		.imms = cloudman_imms, .nimms = (int)(sizeof cloudman_imms / sizeof *cloudman_imms),
		.bytes = cloudman_bytes, .nbytes = (int)(sizeof cloudman_bytes / sizeof *cloudman_bytes),
		.states = cloudman_states, .nstates = (int)(sizeof cloudman_states / sizeof *cloudman_states),
		.hooks = { OWN(0x080FF101), BN6(0x080F3BE9), OWN(0x080FF0BD), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080FEE98, .struct2 = 0x080FEEA0,
		.names = { 0x155, 0x156, 0x157, 0x158, 0x159, 0x15A },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x0F, .element = 0, .weakness = 0, .shadow = true },
		.sprites = cloudman_sprites, .nsprites = (int)(sizeof cloudman_sprites / sizeof *cloudman_sprites),
		.types = cloudman_types, .ntypes = (int)(sizeof cloudman_types / sizeof *cloudman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801AAC1), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 15,
	.sounds = cloudman_sounds, .nsounds = (int)(sizeof cloudman_sounds / sizeof *cloudman_sounds),
};
/* ---- NapalmMan (BN5's: AI 4 there, its ids 0x113, 0x114, 0x115, 0x116), as Navi AI 29 with ids 0x1fc-0x1ff ---- */

static const XvBlock napalmman_blocks[] = {
	{ 0x080F5D6C, 0x080F6400, XN_TEAM_CODE + 0x7800, 0x6FD5813Au },
	{ 0x0800EE82, 0x0800EE92, XN_TEAM_CODE + 0x7F02, 0xFFDB7278u },
	{ 0x0800F00E, 0x0800F01C, XN_TEAM_CODE + 0x8002, 0x5E2EBED8u },
	{ 0x080C4800, 0x080C4AD8, XN_TEAM_CODE + 0x8100, 0x0F349360u },
	{ 0x080C5386, 0x080C53DE, XN_TEAM_CODE + 0x8402, 0x4A1B50AFu },
	{ 0x080CB2B8, 0x080CB594, XN_TEAM_CODE + 0x8500, 0x1FD89B04u },
	{ 0x080CC08C, 0x080CC878, XN_TEAM_CODE + 0x8800, 0xE35C73D2u },
	{ 0x080E1C24, 0x080E1D42, XN_TEAM_CODE + 0x9000, 0x3A491EE2u },
	{ 0x08104D86, 0x08104DA8, XN_TEAM_CODE + 0x9202, 0x21492011u },
};
static const XvData napalmman_data[] = {
	{ 0x080F5D6C, 0x080F5D94, XV_ZERO },
	{ 0x080F5D94, 0x080F5D9E, XV_RAW },
	{ 0x080F5D9E, 0x080F5DA8, XV_RAW },
	{ 0x080F5DA8, 0x080F5DB2, XV_RAW },
	{ 0x080F5DB2, 0x080F5DBA, XV_RAW },
	{ 0x080F5DBA, 0x080F5DBE, XV_RAW },
	{ 0x080F5DBE, 0x080F5DC2, XV_RAW },
	{ 0x080F5DC2, 0x080F5DC8, XV_RAW },
	{ 0x080F5DDE, 0x080F5DE0, XV_RAW },
	{ 0x080F5DE0, 0x080F5DE4, XV_POOL },
	{ 0x080F5DF8, 0x080F5DFC, XV_POOL },
	{ 0x080F5DFC, 0x080F5E0C, XV_TABLE },
	{ 0x080F603E, 0x080F6040, XV_RAW },
	{ 0x080F6040, 0x080F6044, XV_POOL },
	{ 0x080F6044, 0x080F604C, XV_RAW },
	{ 0x080F604C, 0x080F6050, XV_POOL },
	{ 0x080F6050, 0x080F6060, XV_RAW },
	{ 0x080F6060, 0x080F608C, XV_POOL },
	{ 0x080F609A, 0x080F609C, XV_RAW },
	{ 0x080F609C, 0x080F60A0, XV_POOL },
	{ 0x080F60A0, 0x080F60B0, XV_TABLE },
	{ 0x080F6230, 0x080F6234, XV_POOL },
	{ 0x080F62B4, 0x080F62B8, XV_POOL },
	{ 0x080F62C6, 0x080F62C8, XV_RAW },
	{ 0x080F62C8, 0x080F62CC, XV_POOL },
	{ 0x080F62CC, 0x080F62DC, XV_TABLE },
	{ 0x080F6350, 0x080F6354, XV_POOL },
	{ 0x080F63EA, 0x080F63EC, XV_RAW },
	{ 0x080F63EC, 0x080F63FC, XV_RAW },
	{ 0x080F63FC, 0x080F6400, XV_POOL },
	{ 0x080C4800, 0x080C4974, XV_RAW },
	{ 0x080C4986, 0x080C4988, XV_RAW },
	{ 0x080C4988, 0x080C498C, XV_POOL },
	{ 0x080C498C, 0x080C4998, XV_TABLE },
	{ 0x080C4AD2, 0x080C4AD4, XV_RAW },
	{ 0x080C4AD4, 0x080C4AD8, XV_POOL },
	{ 0x080CB2CA, 0x080CB2CC, XV_RAW },
	{ 0x080CB2CC, 0x080CB2D0, XV_POOL },
	{ 0x080CB2D0, 0x080CB2DC, XV_TABLE },
	{ 0x080CB3B6, 0x080CB3B8, XV_RAW },
	{ 0x080CB3B8, 0x080CB3BC, XV_POOL },
	{ 0x080CB3BC, 0x080CB3D4, XV_TABLE },
	{ 0x080CB58E, 0x080CB590, XV_RAW },
	{ 0x080CB590, 0x080CB594, XV_POOL },
	{ 0x080CC09E, 0x080CC0A0, XV_RAW },
	{ 0x080CC0A0, 0x080CC0A4, XV_POOL },
	{ 0x080CC0A4, 0x080CC0B0, XV_TABLE },
	{ 0x080CC17E, 0x080CC180, XV_RAW },
	{ 0x080CC180, 0x080CC184, XV_POOL },
	{ 0x080CC184, 0x080CC18C, XV_TABLE },
	{ 0x080CC27E, 0x080CC280, XV_RAW },
	{ 0x080CC2F4, 0x080CC2FC, XV_POOL },
	{ 0x080CC30E, 0x080CC310, XV_RAW },
	{ 0x080CC310, 0x080CC314, XV_POOL },
	{ 0x080CC314, 0x080CC320, XV_TABLE },
	{ 0x080CC39C, 0x080CC3A0, XV_POOL },
	{ 0x080CC3A0, 0x080CC3AC, XV_TABLE },
	{ 0x080CC4D6, 0x080CC4D8, XV_RAW },
	{ 0x080CC4D8, 0x080CC4DC, XV_POOL },
	{ 0x080CC4DC, 0x080CC4EC, XV_TABLE },
	{ 0x080CC4EC, 0x080CC4F0, XV_POOL },
	{ 0x080CC4F0, 0x080CC500, XV_TABLE },
	{ 0x080CC554, 0x080CC558, XV_POOL },
	{ 0x080CC558, 0x080CC55A, XV_RAW },
	{ 0x080CC576, 0x080CC578, XV_RAW },
	{ 0x080CC58A, 0x080CC58C, XV_RAW },
	{ 0x080CC58C, 0x080CC590, XV_POOL },
	{ 0x080CC590, 0x080CC59C, XV_TABLE },
	{ 0x080CC65E, 0x080CC660, XV_RAW },
	{ 0x080CC660, 0x080CC664, XV_POOL },
	{ 0x080CC664, 0x080CC66C, XV_TABLE },
	{ 0x080CC79C, 0x080CC7A0, XV_POOL },
	{ 0x080CC802, 0x080CC816, XV_RAW },
	{ 0x080CC86A, 0x080CC86C, XV_RAW },
	{ 0x080CC86C, 0x080CC878, XV_POOL },
	{ 0x080E1C32, 0x080E1C34, XV_RAW },
	{ 0x080E1C34, 0x080E1C38, XV_POOL },
	{ 0x080E1C38, 0x080E1C44, XV_TABLE },
	{ 0x080E1C58, 0x080E1C5C, XV_POOL },
	{ 0x080E1C5C, 0x080E1C60, XV_TABLE },
	{ 0x080E1D12, 0x080E1D14, XV_RAW },
	{ 0x080E1D14, 0x080E1D18, XV_POOL },
	{ 0x080E1D18, 0x080E1D28, XV_RAW },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair napalmman_calls[] = {
	{ 0x080C3808, 0x080C5D18 },   /* project:navis, DIFF (audited) */
	{ 0x080C382C, 0x080C5D38 },   /* project:navis, SAME */
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
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x0800B66C, 0x0800D06A },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x0800B368, 0x0800CCA6 },   /* map:calls+bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800B392, 0x0800CCD4 },   /* map:bytes, SAME */
	{ 0x08018838, 0x0801BD3C },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08000BC6, 0x08000CA6 },   /* project:navis, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800B64A, 0x0800D048 },   /* map:bytes, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800B688, 0x0800D086 },   /* map:bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08012A6C, 0x08015C94 },   /* map:bytes, SAME */
	{ 0x08012FBC, 0x080161C4 },   /* near, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B544, 0x0800CF14 },   /* map:bytes, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B4D0, 0x0800CEA0 },   /* map:bytes, SAME */
	{ 0x0800C6C2, 0x0800E2C2 },   /* map:calls+bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B7F2, 0x0800D1F0 },   /* map:bytes, SAME */
};
static const XvPair napalmman_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x0801629C, 0x08019B78 },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm napalmman_imms[] = {
	{ 0x0800EE84, THUMB_MOVS, 4, 0, 0x4, 0x21 },   /* its overlay's kind: 4 in BN5, 33 in BN6 (its own, past BN6's) */
	{ 0x080C4A2E, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CB302, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x1a in BN5, list 2 0x80 in BN6 (its list's offset) */
	{ 0x080CB304, THUMB_MOVS, 2, 0, 0x1A, 0x80 },   /* its sprite: list 4 0x1a in BN5, list 2 0x80 in BN6 */
	{ 0x080CB394, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CB4A6, THUMB_MOVS, 4, 0, 0x4C, 0xAE },   /* its effect: 0x4c in BN5, 0xae in BN6 */
	{ 0x080CC0B2, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x1b in BN5, list 2 0x81 in BN6 (its list's offset) */
	{ 0x080CC0B4, THUMB_MOVS, 2, 0, 0x1B, 0x81 },   /* its sprite: list 4 0x1b in BN5, list 2 0x81 in BN6 */
	{ 0x080CC13C, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CC158, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080CC166, THUMB_MOVS, 4, 0, 0x8, 0xAF },   /* its effect: 0x8 in BN5, 0xaf in BN6 */
	{ 0x080CC1D8, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080CC1F6, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080CC212, THUMB_MOVS, 4, 0, 0x4B, 0xAD },   /* its effect: 0x4b in BN5, 0xad in BN6 */
	{ 0x080CC2A2, THUMB_MOVS, 2, 0, 0x4B, 0xAD },   /* its effect: 0x4b in BN5, 0xad in BN6 */
	{ 0x080CC326, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x1c in BN5, list 2 0x82 in BN6 (its list's offset) */
	{ 0x080CC328, THUMB_MOVS, 2, 0, 0x1C, 0x82 },   /* its sprite: list 4 0x1c in BN5, list 2 0x82 in BN6 */
	{ 0x080CC59E, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x1b in BN5, list 2 0x81 in BN6 (its list's offset) */
	{ 0x080CC5A0, THUMB_MOVS, 2, 0, 0x1B, 0x81 },   /* its sprite: list 4 0x1b in BN5, list 2 0x81 in BN6 */
	{ 0x080CC62E, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080CC64E, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080CC6DA, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080CC6F8, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080CC7B2, THUMB_MOVS, 4, 0, 0x4B, 0xAD },   /* its effect: 0x4b in BN5, 0xad in BN6 */
	{ 0x080CC85C, THUMB_MOVS, 4, 0, 0x4B, 0xAD },   /* its effect: 0x4b in BN5, 0xad in BN6 */
	{ 0x080F5E74, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F5F16, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F5F76, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvByte napalmman_bytes[] = {
	{ 0x080C481D, 0x0C, 0x10 },   /* 080C4800's row 2: its sprite list 3 0x0 in BN5, list 4 0xb5 in BN6 (its list's offset) */
	{ 0x080C481E, 0x00, 0xB5 },   /* 080C4800's row 2: its sprite list 3 0x0 in BN5, list 4 0xb5 in BN6 */
	{ 0x080C48F8, 0x0A, 0x0E },   /* 080C4800's row 20: a panel set to BN5's type 0xa: BN6's 0xe */
	{ 0x080C4940, 0x08, 0x0D },   /* 080C4800's row 26: a panel set to BN5's type 0x8: BN6's 0xd */
};
static const XvNumber napalmman_numbers[] = {
	{ 0x080CB590, 0x0C800000, 0x0C800000 },
	{ 0x080CC2F8, 0x0405FF0A, 0x0405FF0A },
	{ 0x080CC79C, 0x0405FF10, 0x0405FF10 },
};
static const XvRef napalmman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F5DC9), OWN(0x08104D87), OWN(0x080F608D), OWN(0x080F62B9),
};
static const XvSprite napalmman_sprites[] = {
	{ 4, 0x1A, 2, 0x80, XN_SHARED_ART + 0x31000, 0x1800 },
	{ 4, 0x1B, 2, 0x81, XN_SHARED_ART + 0x32800, 0x800 },
	{ 4, 0x1C, 2, 0x82, XN_SHARED_ART + 0x33000, 0x800 },
	{ 3, 0x00, 4, 0xB5, XN_SHARED_ART + 0x20800, 0x5800 },
	{ 5, 0x0D, 5, 0xA0, XN_SHARED_ART + 0x2000, 0x1800 },
	{ 4, 0x19, 2, 0x83, XN_SHARED_ART + 0x33800, 0x1800 },
	{ 5, 0x05, 5, 0xA7, XN_TEAM_SHARED + 0x0, 0x5800 },
	{ 4, 0x18, 2, 0x84, XN_TEAM_SHARED + 0x5800, 0x2000 },
};
static const XvId napalmman_effects[] = { { 0x14, 0xA0 }, { 0x4B, 0xAD }, { 0x4C, 0xAE }, { 0x08, 0xAF } };
static const XvType napalmman_types[] = { { 3, 0x00, XV_TYPE_MOVED }, { 3, 0x33, XV_TYPE_MOVED }, { 3, 0x38, XV_TYPE_MOVED }, { 3, 0x39, XV_TYPE_MOVED }, { 3, 0x3A, XV_TYPE_MOVED }, { 4, 0x12, XV_TYPE_MOVED } };
static const XnSound napalmman_sounds[] = {
	{ 0x080CB3EC, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080CB490, 0x00000000, 0x0BD },   /* BN6's song 0xbd another */
	{ 0x080CC218, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080CC2C0, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080CC36E, 0x080CC370, 0x10E },   /* BN6's song 0x10e another */
	{ 0x080CC424, 0x080CC426, 0x10E },   /* BN6's song 0x10e another */
	{ 0x080CC462, 0x00000000, 0x0BD },   /* BN6's song 0xbd another */
};
static const XnOverlay napalmman_overlay = { 0x080C3685, XN_TEAM_ART + 0xC00, 0x6A95E770u, 0x17, 33, 2, 0x84 };   /* BN5's kind 4, its sprite 4:0x18 */

static const XNavi bn5_napalmman = {
	.code = {
		.name = "NapalmMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 4,
		.ai = 29,
		.first_id = XN_FIRST_ID + 0x38,
		.home = XN_TEAM_HOMES + 0x1800,
		.blocks = napalmman_blocks, .nblocks = (int)(sizeof napalmman_blocks / sizeof *napalmman_blocks),
		.data = napalmman_data, .ndata = (int)(sizeof napalmman_data / sizeof *napalmman_data),
		.calls = napalmman_calls, .ncalls = (int)(sizeof napalmman_calls / sizeof *napalmman_calls),
		.words = napalmman_words, .nwords = (int)(sizeof napalmman_words / sizeof *napalmman_words),
		.imms = napalmman_imms, .nimms = (int)(sizeof napalmman_imms / sizeof *napalmman_imms),
		.numbers = napalmman_numbers, .nnumbers = (int)(sizeof napalmman_numbers / sizeof *napalmman_numbers),
		.bytes = napalmman_bytes, .nbytes = (int)(sizeof napalmman_bytes / sizeof *napalmman_bytes),
		.states = napalmman_states, .nstates = (int)(sizeof napalmman_states / sizeof *napalmman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080F5C80, .struct2 = 0x080F5C88,
		.names = { 0x113, 0x114, 0x115, 0x116, 0x117, 0x000 },
		.ice = 2,
		.traits = { .laid = true, .mode = 0x04, .element = 0, .weakness = 0, .shadow = true },
		.sprites = napalmman_sprites, .nsprites = (int)(sizeof napalmman_sprites / sizeof *napalmman_sprites),
		.effects = napalmman_effects, .neffects = (int)(sizeof napalmman_effects / sizeof *napalmman_effects),
		.types = napalmman_types, .ntypes = (int)(sizeof napalmman_types / sizeof *napalmman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = OWN(0x0800EE83), .companion_end = OWN(0x0800F00F), .hide = BN6(0x080C5D43),
	.sprite_of = 4,
	.overlay = &napalmman_overlay,
	.sounds = napalmman_sounds, .nsounds = (int)(sizeof napalmman_sounds / sizeof *napalmman_sounds),
};
/* ---- GyroMan (BN5's: AI 2 there, its ids 0x107, 0x108, 0x109, 0x10a), as Navi AI 27 with ids 0x1f4-0x1f7 ---- */

static const XvBlock gyroman_blocks[] = {
	{ 0x080F4044, 0x080F4DD4, XN_TEAM_CODE + 0x2800, 0x527CBA87u },
	{ 0x0800EE46, 0x0800EE56, XN_TEAM_CODE + 0x3602, 0x38DA1124u },
	{ 0x0800EFD6, 0x0800EFE4, XN_TEAM_CODE + 0x3702, 0xF0F7C9C1u },
	{ 0x080C791C, 0x080C7BDA, XN_TEAM_CODE + 0x3800, 0x1874B5DAu },
	{ 0x080C7D54, 0x080C809E, XN_TEAM_CODE + 0x3B00, 0x8D109E2Bu },
	{ 0x080C827C, 0x080C8404, XN_TEAM_CODE + 0x3F00, 0xC7AD3691u },
	{ 0x080F4DD4, 0x080F4E4C, XN_TEAM_CODE + 0x4100, 0x5BA7DC32u },
};
static const XvData gyroman_data[] = {
	{ 0x080F4044, 0x080F4074, XV_ZERO },
	{ 0x080F408A, 0x080F408C, XV_RAW },
	{ 0x080F408C, 0x080F4090, XV_POOL },
	{ 0x080F40A4, 0x080F40A8, XV_POOL },
	{ 0x080F40A8, 0x080F40B8, XV_TABLE },
	{ 0x080F4226, 0x080F4228, XV_RAW },
	{ 0x080F4228, 0x080F422C, XV_POOL },
	{ 0x080F422C, 0x080F4234, XV_RAW },
	{ 0x080F4284, 0x080F4288, XV_POOL },
	{ 0x080F4288, 0x080F4290, XV_RAW },
	{ 0x080F429A, 0x080F429C, XV_RAW },
	{ 0x080F429C, 0x080F42A0, XV_POOL },
	{ 0x080F42A0, 0x080F42A4, XV_RAW },
	{ 0x080F42BE, 0x080F42C0, XV_RAW },
	{ 0x080F42C0, 0x080F42C4, XV_POOL },
	{ 0x080F42C4, 0x080F42C8, XV_RAW },
	{ 0x080F42D2, 0x080F42D4, XV_RAW },
	{ 0x080F42D4, 0x080F42D8, XV_POOL },
	{ 0x080F42D8, 0x080F42DC, XV_RAW },
	{ 0x080F42FE, 0x080F4300, XV_RAW },
	{ 0x080F4300, 0x080F4310, XV_POOL },
	{ 0x080F4310, 0x080F431A, XV_RAW },
	{ 0x080F431A, 0x080F4324, XV_RAW },
	{ 0x080F4324, 0x080F432E, XV_RAW },
	{ 0x080F432E, 0x080F4338, XV_RAW },
	{ 0x080F4338, 0x080F433C, XV_POOL },
	{ 0x080F433C, 0x080F4340, XV_RAW },
	{ 0x080F4340, 0x080F4354, XV_POOL },
	{ 0x080F4362, 0x080F4364, XV_RAW },
	{ 0x080F4364, 0x080F4368, XV_POOL },
	{ 0x080F4368, 0x080F4374, XV_TABLE },
	{ 0x080F43BC, 0x080F43C0, XV_POOL },
	{ 0x080F43C0, 0x080F43D0, XV_RAW },
	{ 0x080F4444, 0x080F4448, XV_POOL },
	{ 0x080F4448, 0x080F4468, XV_TABLE },
	{ 0x080F44B8, 0x080F44BC, XV_POOL },
	{ 0x080F44BC, 0x080F44CC, XV_RAW },
	{ 0x080F45E2, 0x080F45E4, XV_RAW },
	{ 0x080F45E4, 0x080F45E8, XV_POOL },
	{ 0x080F45E8, 0x080F4604, XV_TABLE },
	{ 0x080F4666, 0x080F4668, XV_RAW },
	{ 0x080F4668, 0x080F466C, XV_POOL },
	{ 0x080F466C, 0x080F467C, XV_RAW },
	{ 0x080F4752, 0x080F4754, XV_RAW },
	{ 0x080F4754, 0x080F4758, XV_POOL },
	{ 0x080F4758, 0x080F475C, XV_RAW },
	{ 0x080F4786, 0x080F4788, XV_RAW },
	{ 0x080F4788, 0x080F478C, XV_POOL },
	{ 0x080F478C, 0x080F4794, XV_RAW },
	{ 0x080F47A2, 0x080F47A4, XV_RAW },
	{ 0x080F47A4, 0x080F47A8, XV_POOL },
	{ 0x080F47A8, 0x080F47CC, XV_TABLE },
	{ 0x080F483E, 0x080F4840, XV_RAW },
	{ 0x080F4840, 0x080F4844, XV_POOL },
	{ 0x080F4844, 0x080F4854, XV_RAW },
	{ 0x080F4854, 0x080F4858, XV_POOL },
	{ 0x080F4858, 0x080F485C, XV_RAW },
	{ 0x080F48F2, 0x080F48F4, XV_RAW },
	{ 0x080F48F4, 0x080F48F8, XV_POOL },
	{ 0x080F48F8, 0x080F4908, XV_RAW },
	{ 0x080F49A4, 0x080F49A8, XV_POOL },
	{ 0x080F49A8, 0x080F49B8, XV_RAW },
	{ 0x080F49B8, 0x080F49BC, XV_POOL },
	{ 0x080F49BC, 0x080F49C4, XV_RAW },
	{ 0x080F49C4, 0x080F49C8, XV_POOL },
	{ 0x080F4A42, 0x080F4A44, XV_RAW },
	{ 0x080F4A44, 0x080F4A48, XV_POOL },
	{ 0x080F4A48, 0x080F4A50, XV_RAW },
	{ 0x080F4A5E, 0x080F4A60, XV_RAW },
	{ 0x080F4A60, 0x080F4A64, XV_POOL },
	{ 0x080F4A64, 0x080F4A74, XV_TABLE },
	{ 0x080F4AD4, 0x080F4ADC, XV_POOL },
	{ 0x080F4ADC, 0x080F4AE0, XV_RAW },
	{ 0x080F4CA0, 0x080F4CA4, XV_POOL },
	{ 0x080F4CA4, 0x080F4CA8, XV_RAW },
	{ 0x080F4CFA, 0x080F4CFC, XV_RAW },
	{ 0x080F4CFC, 0x080F4D00, XV_POOL },
	{ 0x080F4DC6, 0x080F4DC8, XV_RAW },
	{ 0x080F4DC8, 0x080F4DCC, XV_POOL },
	{ 0x080F4DCC, 0x080F4DD4, XV_RAW },
	{ 0x080C792E, 0x080C7930, XV_RAW },
	{ 0x080C7930, 0x080C7934, XV_POOL },
	{ 0x080C7934, 0x080C7940, XV_TABLE },
	{ 0x080C79DE, 0x080C79E0, XV_RAW },
	{ 0x080C79E0, 0x080C79E4, XV_POOL },
	{ 0x080C79E4, 0x080C79E8, XV_TABLE },
	{ 0x080C7A6E, 0x080C7A70, XV_RAW },
	{ 0x080C7A70, 0x080C7A74, XV_POOL },
	{ 0x080C7A74, 0x080C7A84, XV_RAW },
	{ 0x080C7B62, 0x080C7B64, XV_RAW },
	{ 0x080C7B64, 0x080C7B68, XV_POOL },
	{ 0x080C7B68, 0x080C7B78, XV_RAW },
	{ 0x080C7B78, 0x080C7B7C, XV_POOL },
	{ 0x080C7B7C, 0x080C7B88, XV_RAW },
	{ 0x080C7B88, 0x080C7B8C, XV_POOL },
	{ 0x080C7B8C, 0x080C7B9C, XV_RAW },
	{ 0x080C7D66, 0x080C7D68, XV_RAW },
	{ 0x080C7D68, 0x080C7D6C, XV_POOL },
	{ 0x080C7D6C, 0x080C7D78, XV_TABLE },
	{ 0x080C7DF0, 0x080C7DF4, XV_POOL },
	{ 0x080C7DF4, 0x080C7E00, XV_RAW },
	{ 0x080C7E00, 0x080C7E04, XV_POOL },
	{ 0x080C7E04, 0x080C7E10, XV_RAW },
	{ 0x080C7E4A, 0x080C7E4C, XV_RAW },
	{ 0x080C7E4C, 0x080C7E50, XV_POOL },
	{ 0x080C7E50, 0x080C7E5C, XV_TABLE },
	{ 0x080C7EAC, 0x080C7EB0, XV_POOL },
	{ 0x080C7EB0, 0x080C7EB4, XV_RAW },
	{ 0x080C7F86, 0x080C7F88, XV_RAW },
	{ 0x080C7F9A, 0x080C7F9C, XV_RAW },
	{ 0x080C7F9C, 0x080C7FA0, XV_POOL },
	{ 0x080C7FA0, 0x080C7FAC, XV_TABLE },
	{ 0x080C8294, 0x080C8298, XV_POOL },
	{ 0x080C8298, 0x080C82A4, XV_TABLE },
	{ 0x080C8326, 0x080C8328, XV_RAW },
	{ 0x080C8328, 0x080C832C, XV_POOL },
	{ 0x080C832C, 0x080C8330, XV_TABLE },
	{ 0x080C83DA, 0x080C83DC, XV_RAW },
	{ 0x080C83DC, 0x080C83E4, XV_POOL },
	{ 0x080C83E4, 0x080C8404, XV_RAW },
	{ 0x080F4DD4, 0x080F4E4C, XV_RAW },
};
static const XvPair gyroman_calls[] = {
	{ 0x080C3808, 0x080C5D18 },   /* project:navis, DIFF (audited) */
	{ 0x080C382C, 0x080C5D38 },   /* project:navis, SAME */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB20, 0x0800E708 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x08012B58, 0x08015D80 },   /* map:bytes, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B392, 0x0800CCD4 },   /* map:bytes, SAME */
	{ 0x0800F1EE, 0x0801155A },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x080C3836, 0x080C5F26 },   /* search, by place, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800B5A8, 0x0800CFA6 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B688, 0x0800D086 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
};
static const XvPair gyroman_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm gyroman_imms[] = {
	{ 0x0800EE48, THUMB_MOVS, 4, 0, 0x0, 0x1F },   /* its overlay's kind: 0 in BN5, 31 in BN6 (its own, past BN6's) */
	{ 0x080C794A, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0xb in BN5, list 2 0x85 in BN6 (its list's offset) */
	{ 0x080C794C, THUMB_MOVS, 2, 0, 0xB, 0x85 },   /* its sprite: list 4 0xb in BN5, list 2 0x85 in BN6 */
	{ 0x080C79B8, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C7D7A, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0xd in BN5, list 2 0x87 in BN6 (its list's offset) */
	{ 0x080C7D7C, THUMB_MOVS, 2, 0, 0xD, 0x87 },   /* its sprite: list 4 0xd in BN5, list 2 0x87 in BN6 */
	{ 0x080C7F36, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C7FD6, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0xf in BN5, list 2 0x88 in BN6 (its list's offset) */
	{ 0x080C7FD8, THUMB_MOVS, 2, 0, 0xF, 0x88 },   /* its sprite: list 4 0xf in BN5, list 2 0x88 in BN6 */
	{ 0x080C8050, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C82A6, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0xc in BN5, list 2 0x86 in BN6 (its list's offset) */
	{ 0x080C82A8, THUMB_MOVS, 2, 0, 0xC, 0x86 },   /* its sprite: list 4 0xc in BN5, list 2 0x86 in BN6 */
	{ 0x080F40E4, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080F414C, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F4180, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F41A8, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F4214, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 (setAttack0): BN6's two status states before the family's own */
};
static const XvRef gyroman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F4075), OWN(0x080F4355), OWN(0x080F4437), OWN(0x080F45D5), OWN(0x080F4795), OWN(0x080F4A51),
};
static const XvSprite gyroman_sprites[] = {
	{ 4, 0x0B, 2, 0x85, XN_SHARED_ART + 0x35000, 0x800 },
	{ 4, 0x0C, 2, 0x86, XN_TEAM_MORE + 0x0, 0x2000 },
	{ 4, 0x0D, 2, 0x87, XN_SHARED_ART + 0x35800, 0x800 },
	{ 4, 0x0F, 2, 0x88, XN_TEAM_MORE + 0x2000, 0x800 },
	{ 4, 0x0A, 2, 0x89, XN_TEAM_MORE + 0x2800, 0x6000 },
};
static const XvType gyroman_types[] = { { 3, 0x19, XV_TYPE_MOVED }, { 3, 0x1B, XV_TYPE_MOVED }, { 3, 0x1C, XV_TYPE_MOVED }, { 3, 0x1E, XV_TYPE_MOVED } };
static const XnSound gyroman_sounds[] = {
	{ 0x080C7F22, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080C82FC, 0x080C82FE, 0x103 },   /* BN6's song 0x103 another */
	{ 0x080F4732, 0x080F4734, 0x103 },   /* BN6's song 0x103 another */
	{ 0x080F48B2, 0x080F48B4, 0x14C },   /* BN6's song 0x14c another */
	{ 0x080F48E4, 0x080F48E6, 0x104 },   /* BN6's song 0x104 another */
	{ 0x080F4D02, 0x080F4D04, 0x105 },   /* BN6's song 0x105 another */
};
static const XnOverlay gyroman_overlay = { 0x080C3610, XN_TEAM_ART + 0x400, 0x59F5F3FFu, 0x16, 31, 2, 0x89 };   /* BN5's kind 0, its sprite 4:0xa */

static const XNavi bn5_gyroman = {
	.code = {
		.name = "GyroMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 2,
		.ai = 27,
		.first_id = XN_FIRST_ID + 0x30,
		.home = XN_TEAM_HOMES + 0x800,
		.blocks = gyroman_blocks, .nblocks = (int)(sizeof gyroman_blocks / sizeof *gyroman_blocks),
		.data = gyroman_data, .ndata = (int)(sizeof gyroman_data / sizeof *gyroman_data),
		.calls = gyroman_calls, .ncalls = (int)(sizeof gyroman_calls / sizeof *gyroman_calls),
		.words = gyroman_words, .nwords = (int)(sizeof gyroman_words / sizeof *gyroman_words),
		.imms = gyroman_imms, .nimms = (int)(sizeof gyroman_imms / sizeof *gyroman_imms),
		.states = gyroman_states, .nstates = (int)(sizeof gyroman_states / sizeof *gyroman_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), OWN(0x080F424B), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), OWN(0x080F424F), BN6(0x080F3BE9) },
		.struct1 = 0x080F3F58, .struct2 = 0x080F3F60,
		.names = { 0x107, 0x108, 0x109, 0x10A, 0x10B, 0x10C },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x02, .element = 0, .weakness = 0, .shadow = true },
		.sprites = gyroman_sprites, .nsprites = (int)(sizeof gyroman_sprites / sizeof *gyroman_sprites),
		.types = gyroman_types, .ntypes = (int)(sizeof gyroman_types / sizeof *gyroman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = OWN(0x0800EE47), .companion_end = OWN(0x0800EFD7), .hide = BN6(0x080C5D43),
	.sprite_of = 2,
	.overlay = &gyroman_overlay,
	.sounds = gyroman_sounds, .nsounds = (int)(sizeof gyroman_sounds / sizeof *gyroman_sounds),
};
/* ---- SearchMan (BN5's: AI 3 there, its ids 0x10d, 0x10e, 0x10f, 0x110), as Navi AI 28 with ids 0x1f8-0x1fb ---- */

static const XvBlock searchman_blocks[] = {
	{ 0x080F4EC0, 0x080F5C80, XN_TEAM_CODE + 0x5000, 0xFB3B0CD9u },
	{ 0x080C5386, 0x080C53DE, XN_TEAM_CODE + 0x5E02, 0x4A1B50AFu },
	{ 0x080DB6BC, 0x080DBAF4, XN_TEAM_CODE + 0x5F00, 0xBC0F635Eu },
	{ 0x080DC0F8, 0x080DC2B0, XN_TEAM_CODE + 0x6400, 0xE5BD127Du },
	{ 0x080E6F84, 0x080E71F4, XN_TEAM_CODE + 0x6600, 0x9285FC05u },
	{ 0x080E7764, 0x080E7AA8, XN_TEAM_CODE + 0x6900, 0x8113F9ACu },
};
static const XvData searchman_data[] = {
	{ 0x080F4EC0, 0x080F4EF0, XV_ZERO },
	{ 0x080F4F06, 0x080F4F08, XV_RAW },
	{ 0x080F4F08, 0x080F4F0C, XV_POOL },
	{ 0x080F4F20, 0x080F4F24, XV_POOL },
	{ 0x080F4F24, 0x080F4F38, XV_TABLE },
	{ 0x080F4F94, 0x080F4F98, XV_POOL },
	{ 0x080F5050, 0x080F5054, XV_POOL },
	{ 0x080F50BA, 0x080F50BC, XV_RAW },
	{ 0x080F50BC, 0x080F50C0, XV_POOL },
	{ 0x080F515C, 0x080F5160, XV_POOL },
	{ 0x080F5160, 0x080F5164, XV_RAW },
	{ 0x080F51DE, 0x080F51E0, XV_RAW },
	{ 0x080F51E0, 0x080F51E4, XV_POOL },
	{ 0x080F51E4, 0x080F51E8, XV_RAW },
	{ 0x080F51F2, 0x080F51F4, XV_RAW },
	{ 0x080F51F4, 0x080F51F8, XV_POOL },
	{ 0x080F51F8, 0x080F51FC, XV_RAW },
	{ 0x080F5206, 0x080F5208, XV_RAW },
	{ 0x080F5208, 0x080F520C, XV_POOL },
	{ 0x080F520C, 0x080F5210, XV_RAW },
	{ 0x080F521A, 0x080F521C, XV_RAW },
	{ 0x080F521C, 0x080F5220, XV_POOL },
	{ 0x080F5220, 0x080F5224, XV_RAW },
	{ 0x080F5246, 0x080F5248, XV_RAW },
	{ 0x080F5248, 0x080F5254, XV_POOL },
	{ 0x080F5254, 0x080F525E, XV_RAW },
	{ 0x080F525E, 0x080F5268, XV_RAW },
	{ 0x080F5268, 0x080F5274, XV_RAW },
	{ 0x080F5274, 0x080F5278, XV_POOL },
	{ 0x080F5278, 0x080F527C, XV_RAW },
	{ 0x080F527C, 0x080F5298, XV_POOL },
	{ 0x080F52A6, 0x080F52A8, XV_RAW },
	{ 0x080F52A8, 0x080F52AC, XV_POOL },
	{ 0x080F52AC, 0x080F52B8, XV_TABLE },
	{ 0x080F5300, 0x080F5304, XV_POOL },
	{ 0x080F5304, 0x080F5314, XV_RAW },
	{ 0x080F5388, 0x080F538C, XV_POOL },
	{ 0x080F538C, 0x080F5398, XV_TABLE },
	{ 0x080F5418, 0x080F541C, XV_POOL },
	{ 0x080F541C, 0x080F5440, XV_TABLE },
	{ 0x080F5496, 0x080F5498, XV_RAW },
	{ 0x080F5498, 0x080F549C, XV_POOL },
	{ 0x080F549C, 0x080F54AC, XV_RAW },
	{ 0x080F55F8, 0x080F55FC, XV_POOL },
	{ 0x080F55FC, 0x080F561C, XV_TABLE },
	{ 0x080F565C, 0x080F5660, XV_POOL },
	{ 0x080F5660, 0x080F5670, XV_RAW },
	{ 0x080F577A, 0x080F577C, XV_RAW },
	{ 0x080F577C, 0x080F5780, XV_POOL },
	{ 0x080F57FE, 0x080F5800, XV_RAW },
	{ 0x080F5800, 0x080F5804, XV_POOL },
	{ 0x080F5804, 0x080F5824, XV_TABLE },
	{ 0x080F586C, 0x080F5870, XV_POOL },
	{ 0x080F5870, 0x080F5880, XV_RAW },
	{ 0x080F5A34, 0x080F5A38, XV_POOL },
	{ 0x080F5A38, 0x080F5A40, XV_RAW },
	{ 0x080F5A40, 0x080F5A44, XV_POOL },
	{ 0x080F5A44, 0x080F5A48, XV_RAW },
	{ 0x080F5B0A, 0x080F5B0C, XV_RAW },
	{ 0x080F5B0C, 0x080F5B10, XV_POOL },
	{ 0x080F5BB4, 0x080F5BB8, XV_POOL },
	{ 0x080F5C16, 0x080F5C18, XV_RAW },
	{ 0x080F5C18, 0x080F5C1C, XV_POOL },
	{ 0x080F5C1C, 0x080F5C20, XV_RAW },
	{ 0x080F5C7A, 0x080F5C7C, XV_RAW },
	{ 0x080F5C7C, 0x080F5C80, XV_POOL },
	{ 0x080DB6D4, 0x080DB6D8, XV_POOL },
	{ 0x080DB6D8, 0x080DB6E4, XV_TABLE },
	{ 0x080DB768, 0x080DB76C, XV_POOL },
	{ 0x080DB76C, 0x080DB77C, XV_RAW },
	{ 0x080DB77C, 0x080DB780, XV_POOL },
	{ 0x080DB780, 0x080DB788, XV_RAW },
	{ 0x080DB7EE, 0x080DB7F0, XV_RAW },
	{ 0x080DB7F0, 0x080DB7F4, XV_POOL },
	{ 0x080DB7F4, 0x080DB804, XV_TABLE },
	{ 0x080DB91C, 0x080DB920, XV_POOL },
	{ 0x080DB920, 0x080DB928, XV_RAW },
	{ 0x080DB99C, 0x080DB9A0, XV_POOL },
	{ 0x080DB9A0, 0x080DB9AC, XV_RAW },
	{ 0x080DB9AC, 0x080DB9B0, XV_POOL },
	{ 0x080DBA16, 0x080DBA18, XV_RAW },
	{ 0x080DBA18, 0x080DBA1C, XV_POOL },
	{ 0x080DBA1C, 0x080DBA2C, XV_RAW },
	{ 0x080DBA44, 0x080DBA48, XV_POOL },
	{ 0x080DBA56, 0x080DBA58, XV_RAW },
	{ 0x080DBA58, 0x080DBA5C, XV_POOL },
	{ 0x080DBA5C, 0x080DBA68, XV_TABLE },
	{ 0x080DC116, 0x080DC118, XV_RAW },
	{ 0x080DC118, 0x080DC11C, XV_POOL },
	{ 0x080DC11C, 0x080DC128, XV_TABLE },
	{ 0x080DC1B2, 0x080DC1B4, XV_RAW },
	{ 0x080DC1B4, 0x080DC1B8, XV_POOL },
	{ 0x080DC1B8, 0x080DC1BC, XV_RAW },
	{ 0x080DC1BC, 0x080DC1C0, XV_POOL },
	{ 0x080DC1C0, 0x080DC1C8, XV_RAW },
	{ 0x080DC208, 0x080DC20C, XV_POOL },
	{ 0x080DC20C, 0x080DC214, XV_TABLE },
	{ 0x080DC2A8, 0x080DC2B0, XV_POOL },
	{ 0x080E6F96, 0x080E6F98, XV_RAW },
	{ 0x080E6F98, 0x080E6F9C, XV_POOL },
	{ 0x080E6F9C, 0x080E6FA8, XV_TABLE },
	{ 0x080E703E, 0x080E7040, XV_RAW },
	{ 0x080E7052, 0x080E7054, XV_RAW },
	{ 0x080E7054, 0x080E7058, XV_POOL },
	{ 0x080E7058, 0x080E7064, XV_TABLE },
	{ 0x080E70F8, 0x080E70FC, XV_POOL },
	{ 0x080E70FC, 0x080E7108, XV_TABLE },
	{ 0x080E714E, 0x080E7150, XV_RAW },
	{ 0x080E7150, 0x080E7154, XV_POOL },
	{ 0x080E7154, 0x080E716C, XV_RAW },
	{ 0x080E7782, 0x080E7784, XV_RAW },
	{ 0x080E7784, 0x080E7788, XV_POOL },
	{ 0x080E7788, 0x080E7794, XV_TABLE },
	{ 0x080E780E, 0x080E7810, XV_RAW },
	{ 0x080E7810, 0x080E7814, XV_POOL },
	{ 0x080E7814, 0x080E7824, XV_TABLE },
	{ 0x080E796C, 0x080E7970, XV_POOL },
	{ 0x080E798E, 0x080E7990, XV_RAW },
	{ 0x080E7990, 0x080E7994, XV_POOL },
	{ 0x080E7994, 0x080E79A0, XV_TABLE },
	{ 0x080E7A42, 0x080E7A44, XV_RAW },
	{ 0x080E7A44, 0x080E7A4C, XV_POOL },
	{ 0x080E7AA2, 0x080E7AA4, XV_RAW },
	{ 0x080E7AA4, 0x080E7AA8, XV_POOL },
};
static const XvPair searchman_calls[] = {
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB20, 0x0800E708 },   /* map:bytes, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800AFF0, 0x0800C938 },   /* map:calls, DIFF (audited) */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x08013D1C, 0x080170C4 },   /* navis, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
};
static const XvPair searchman_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm searchman_imms[] = {
	{ 0x080DB6EE, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x33 in BN5, list 2 0x8c in BN6 (its list's offset) */
	{ 0x080DB6F0, THUMB_MOVS, 2, 0, 0x33, 0x8C },   /* its sprite: list 4 0x33 in BN5, list 2 0x8c in BN6 */
	{ 0x080DB7A6, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DB7CA, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080DB90C, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080DBAB2, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DC132, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x4e in BN5, list 2 0x8f in BN6 (its list's offset) */
	{ 0x080DC134, THUMB_MOVS, 2, 0, 0x4E, 0x8F },   /* its sprite: list 4 0x4e in BN5, list 2 0x8f in BN6 */
	{ 0x080DC1DC, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080E6FC0, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x23 in BN5, list 2 0x8b in BN6 (its list's offset) */
	{ 0x080E6FC2, THUMB_MOVS, 2, 0, 0x23, 0x8B },   /* its sprite: list 4 0x23 in BN5, list 2 0x8b in BN6 */
	{ 0x080E7014, THUMB_MOVS, 1, 0, 0xA, 0xC },   /* its parent's CurAction 10 compared: BN6's 12 */
	{ 0x080E7094, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x22 in BN5, list 2 0x8a in BN6 (its list's offset) */
	{ 0x080E7096, THUMB_MOVS, 2, 0, 0x22, 0x8A },   /* its sprite: list 4 0x22 in BN5, list 2 0x8a in BN6 */
	{ 0x080E70E0, THUMB_MOVS, 1, 0, 0xB, 0xD },   /* its parent's CurAction 11 compared: BN6's 13 */
	{ 0x080E77AE, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x4c in BN5, list 2 0x8d in BN6 (its list's offset) */
	{ 0x080E77B0, THUMB_MOVS, 2, 0, 0x4C, 0x8D },   /* its sprite: list 4 0x4c in BN5, list 2 0x8d in BN6 */
	{ 0x080E79BC, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x4d in BN5, list 2 0x8e in BN6 (its list's offset) */
	{ 0x080E79BE, THUMB_MOVS, 2, 0, 0x4D, 0x8E },   /* its sprite: list 4 0x4d in BN5, list 2 0x8e in BN6 */
	{ 0x080F4F66, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F4F84, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F501C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080F50A4, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F5154, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F518C, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F51BC, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber searchman_numbers[] = {
	{ 0x080DB9AC, 0x04050701, 0x04050701 },
};
static const XvRef searchman_states[] = {
	BN6(0x08016381), BN6(0x08017889), OWN(0x080F5115), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F4EF1), OWN(0x080F5299), OWN(0x080F537B), OWN(0x080F540B), OWN(0x080F55EB), OWN(0x080F57F1),
};
static const XvSprite searchman_sprites[] = {
	{ 4, 0x22, 2, 0x8A, XN_TEAM_MORE + 0x8800, 0x800 },
	{ 4, 0x23, 2, 0x8B, XN_TEAM_MORE + 0x9000, 0x800 },
	{ 4, 0x33, 2, 0x8C, XN_TEAM_MORE + 0x9800, 0x800 },
	{ 4, 0x4C, 2, 0x8D, XN_TEAM_MORE + 0xA000, 0x1800 },
	{ 4, 0x4D, 2, 0x8E, XN_TEAM_MORE + 0xB800, 0x1800 },
	{ 4, 0x4E, 2, 0x8F, XN_TEAM_MORE + 0xD000, 0x1000 },
	{ 5, 0x0D, 5, 0xA0, XN_SHARED_ART + 0x2000, 0x1800 },
};
static const XvId searchman_effects[] = { { 0x14, 0xA0 } };
static const XvType searchman_types[] = { { 3, 0xAA, XV_TYPE_MOVED }, { 3, 0xAB, XV_TYPE_MOVED }, { 3, 0xAF, XV_TYPE_MOVED }, { 4, 0x5A, XV_TYPE_MOVED }, { 4, 0x5B, XV_TYPE_MOVED }, { 4, 0x61, XV_TYPE_MOVED }, { 4, 0x62, XV_TYPE_MOVED } };
static const XnSound searchman_sounds[] = {
	{ 0x080DB7DE, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080DB92A, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080E7130, 0x080E7132, 0x10E },   /* BN6's song 0x10e another */
	{ 0x080E71BC, 0x00000000, 0x0BD },   /* BN6's song 0xbd another */
	{ 0x080F5740, 0x00000000, 0x0BD },   /* BN6's song 0xbd another */
	{ 0x080F5932, 0x080F5934, 0x10E },   /* BN6's song 0x10e another */
};

static const XNavi bn5_searchman = {
	.code = {
		.name = "SearchMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 3,
		.ai = 28,
		.first_id = XN_FIRST_ID + 0x34,
		.home = XN_TEAM_HOMES + 0x1000,
		.blocks = searchman_blocks, .nblocks = (int)(sizeof searchman_blocks / sizeof *searchman_blocks),
		.data = searchman_data, .ndata = (int)(sizeof searchman_data / sizeof *searchman_data),
		.calls = searchman_calls, .ncalls = (int)(sizeof searchman_calls / sizeof *searchman_calls),
		.words = searchman_words, .nwords = (int)(sizeof searchman_words / sizeof *searchman_words),
		.imms = searchman_imms, .nimms = (int)(sizeof searchman_imms / sizeof *searchman_imms),
		.numbers = searchman_numbers, .nnumbers = (int)(sizeof searchman_numbers / sizeof *searchman_numbers),
		.states = searchman_states, .nstates = (int)(sizeof searchman_states / sizeof *searchman_states),
		.hooks = { OWN(0x080F50E3), BN6(0x080F3BE9), OWN(0x080F50D7), OWN(0x080F50FD), OWN(0x080F50FD), OWN(0x080F50FD), OWN(0x080F50FD), OWN(0x080F50FD) },
		.struct1 = 0x080F4DD4, .struct2 = 0x080F4DDC,
		.names = { 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112 },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x03, .element = 0, .weakness = 0, .shadow = false },
		.sprites = searchman_sprites, .nsprites = (int)(sizeof searchman_sprites / sizeof *searchman_sprites),
		.effects = searchman_effects, .neffects = (int)(sizeof searchman_effects / sizeof *searchman_effects),
		.types = searchman_types, .ntypes = (int)(sizeof searchman_types / sizeof *searchman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 3,
	.sounds = searchman_sounds, .nsounds = (int)(sizeof searchman_sounds / sizeof *searchman_sounds),
};
/* ---- Meddy (BN5's: AI 6 there, its ids 0x11f, 0x120, 0x121, 0x122), as Navi AI 31 with ids 0x3e8-0x3eb ---- */

static const XvBlock meddy_blocks[] = {
	{ 0x080F73E8, 0x080F7FF8, XN_TEAM_CODE + 0xC800, 0x2277A18Au },
	{ 0x080C5386, 0x080C53DE, XN_TEAM_CODE + 0xD502, 0x4A1B50AFu },
	{ 0x080C57E4, 0x080C5D1A, XN_TEAM_CODE + 0xD600, 0xD0362A11u },
	{ 0x080C6000, 0x080C665C, XN_TEAM_CODE + 0xDC00, 0x0AF177EBu },
	{ 0x080E125C, 0x080E1354, XN_TEAM_CODE + 0xE300, 0x32363378u },
	{ 0x080E179C, 0x080E197C, XN_TEAM_CODE + 0xE400, 0xB3893E35u },
	{ 0x080E4B10, 0x080E4BBE, XN_TEAM_CODE + 0xE600, 0xEF7BA781u },
};
static const XvData meddy_data[] = {
	{ 0x080F73E8, 0x080F7414, XV_ZERO },
	{ 0x080F742A, 0x080F742C, XV_RAW },
	{ 0x080F742C, 0x080F7430, XV_POOL },
	{ 0x080F7444, 0x080F7448, XV_POOL },
	{ 0x080F7448, 0x080F7454, XV_TABLE },
	{ 0x080F7592, 0x080F7594, XV_RAW },
	{ 0x080F7594, 0x080F7598, XV_POOL },
	{ 0x080F7598, 0x080F759C, XV_RAW },
	{ 0x080F75A6, 0x080F75A8, XV_RAW },
	{ 0x080F75A8, 0x080F75AC, XV_POOL },
	{ 0x080F75AC, 0x080F75B0, XV_RAW },
	{ 0x080F75D2, 0x080F75D4, XV_RAW },
	{ 0x080F75D4, 0x080F75DC, XV_POOL },
	{ 0x080F75DC, 0x080F75E6, XV_RAW },
	{ 0x080F75E6, 0x080F75F0, XV_RAW },
	{ 0x080F75F0, 0x080F75F4, XV_POOL },
	{ 0x080F75F4, 0x080F75F8, XV_RAW },
	{ 0x080F75F8, 0x080F760C, XV_POOL },
	{ 0x080F761A, 0x080F761C, XV_RAW },
	{ 0x080F761C, 0x080F7620, XV_POOL },
	{ 0x080F7620, 0x080F762C, XV_TABLE },
	{ 0x080F7674, 0x080F7678, XV_POOL },
	{ 0x080F7678, 0x080F7688, XV_RAW },
	{ 0x080F76FC, 0x080F7700, XV_POOL },
	{ 0x080F7700, 0x080F770C, XV_TABLE },
	{ 0x080F7792, 0x080F7794, XV_RAW },
	{ 0x080F7794, 0x080F7798, XV_POOL },
	{ 0x080F7798, 0x080F77B4, XV_TABLE },
	{ 0x080F78A4, 0x080F78A8, XV_POOL },
	{ 0x080F795A, 0x080F795C, XV_RAW },
	{ 0x080F795C, 0x080F7960, XV_POOL },
	{ 0x080F7960, 0x080F7978, XV_TABLE },
	{ 0x080F79C2, 0x080F79C4, XV_RAW },
	{ 0x080F79C4, 0x080F79C8, XV_POOL },
	{ 0x080F79C8, 0x080F79D8, XV_RAW },
	{ 0x080F7ABC, 0x080F7AC0, XV_POOL },
	{ 0x080F7B6C, 0x080F7B70, XV_POOL },
	{ 0x080F7B70, 0x080F7B74, XV_RAW },
	{ 0x080F7BD0, 0x080F7BD4, XV_POOL },
	{ 0x080F7BD4, 0x080F7BD8, XV_RAW },
	{ 0x080F7C8C, 0x080F7C90, XV_POOL },
	{ 0x080F7C90, 0x080F7C9C, XV_RAW },
	{ 0x080F7C9C, 0x080F7CA0, XV_POOL },
	{ 0x080F7CA0, 0x080F7CA8, XV_RAW },
	{ 0x080F7CA8, 0x080F7CAC, XV_POOL },
	{ 0x080F7CAC, 0x080F7CBC, XV_RAW },
	{ 0x080F7CBC, 0x080F7CC0, XV_POOL },
	{ 0x080F7CC0, 0x080F7CC4, XV_RAW },
	{ 0x080F7CC4, 0x080F7CC8, XV_POOL },
	{ 0x080F7CC8, 0x080F7CD8, XV_RAW },
	{ 0x080F7D26, 0x080F7D28, XV_RAW },
	{ 0x080F7D28, 0x080F7D2C, XV_POOL },
	{ 0x080F7E1C, 0x080F7E20, XV_POOL },
	{ 0x080F7E20, 0x080F7E30, XV_RAW },
	{ 0x080F7EA6, 0x080F7EA8, XV_RAW },
	{ 0x080F7EA8, 0x080F7EAC, XV_POOL },
	{ 0x080F7EAC, 0x080F7EBC, XV_RAW },
	{ 0x080F7EBC, 0x080F7EC0, XV_POOL },
	{ 0x080F7EC0, 0x080F7EC4, XV_RAW },
	{ 0x080F7F46, 0x080F7F48, XV_RAW },
	{ 0x080F7F48, 0x080F7F4C, XV_POOL },
	{ 0x080F7F4C, 0x080F7F5C, XV_RAW },
	{ 0x080F7FDE, 0x080F7FE0, XV_RAW },
	{ 0x080F7FE0, 0x080F7FE4, XV_POOL },
	{ 0x080F7FE4, 0x080F7FF4, XV_RAW },
	{ 0x080F7FF4, 0x080F7FF8, XV_POOL },
	{ 0x080C57F6, 0x080C57F8, XV_RAW },
	{ 0x080C57F8, 0x080C57FC, XV_POOL },
	{ 0x080C57FC, 0x080C580C, XV_TABLE },
	{ 0x080C58A4, 0x080C58A8, XV_POOL },
	{ 0x080C58A8, 0x080C58B0, XV_RAW },
	{ 0x080C593C, 0x080C5940, XV_POOL },
	{ 0x080C5940, 0x080C5950, XV_TABLE },
	{ 0x080C59D4, 0x080C59D8, XV_POOL },
	{ 0x080C59D8, 0x080C59E8, XV_TABLE },
	{ 0x080C5B54, 0x080C5B58, XV_POOL },
	{ 0x080C5B58, 0x080C5B60, XV_RAW },
	{ 0x080C5B60, 0x080C5B64, XV_POOL },
	{ 0x080C5B64, 0x080C5B6C, XV_RAW },
	{ 0x080C5C90, 0x080C5C98, XV_POOL },
	{ 0x080C5C98, 0x080C5CA8, XV_RAW },
	{ 0x080C5CEE, 0x080C5CF0, XV_RAW },
	{ 0x080C5CF0, 0x080C5CF4, XV_POOL },
	{ 0x080C5CF4, 0x080C5CFC, XV_RAW },
	{ 0x080C6024, 0x080C6028, XV_POOL },
	{ 0x080C6028, 0x080C6034, XV_TABLE },
	{ 0x080C60CA, 0x080C60CC, XV_RAW },
	{ 0x080C60CC, 0x080C60D0, XV_POOL },
	{ 0x080C60D0, 0x080C60D8, XV_RAW },
	{ 0x080C60D8, 0x080C60DC, XV_POOL },
	{ 0x080C60DC, 0x080C60F4, XV_RAW },
	{ 0x080C60F4, 0x080C60F8, XV_POOL },
	{ 0x080C60F8, 0x080C6110, XV_RAW },
	{ 0x080C617A, 0x080C617C, XV_RAW },
	{ 0x080C617C, 0x080C6180, XV_POOL },
	{ 0x080C6180, 0x080C618C, XV_TABLE },
	{ 0x080C618C, 0x080C6190, XV_POOL },
	{ 0x080C6190, 0x080C6198, XV_RAW },
	{ 0x080C6294, 0x080C6298, XV_POOL },
	{ 0x080C6298, 0x080C62A0, XV_RAW },
	{ 0x080C62E0, 0x080C62E4, XV_POOL },
	{ 0x080C62E4, 0x080C62EC, XV_RAW },
	{ 0x080C636E, 0x080C6370, XV_RAW },
	{ 0x080C6370, 0x080C6374, XV_POOL },
	{ 0x080C6374, 0x080C637C, XV_RAW },
	{ 0x080C637C, 0x080C6380, XV_POOL },
	{ 0x080C6380, 0x080C638C, XV_RAW },
	{ 0x080C638C, 0x080C6390, XV_POOL },
	{ 0x080C6390, 0x080C63A8, XV_RAW },
	{ 0x080C63A8, 0x080C63AC, XV_POOL },
	{ 0x080C63AC, 0x080C63B4, XV_RAW },
	{ 0x080C6420, 0x080C6424, XV_POOL },
	{ 0x080C6424, 0x080C6428, XV_RAW },
	{ 0x080C6428, 0x080C6434, XV_POOL },
	{ 0x080C6446, 0x080C6448, XV_RAW },
	{ 0x080C6448, 0x080C644C, XV_POOL },
	{ 0x080C644C, 0x080C6458, XV_TABLE },
	{ 0x080C6566, 0x080C6568, XV_RAW },
	{ 0x080C6568, 0x080C656C, XV_POOL },
	{ 0x080C656C, 0x080C6578, XV_TABLE },
	{ 0x080C6578, 0x080C657C, XV_POOL },
	{ 0x080C657C, 0x080C6588, XV_RAW },
	{ 0x080C6588, 0x080C658C, XV_POOL },
	{ 0x080C658C, 0x080C6590, XV_RAW },
	{ 0x080C6608, 0x080C660C, XV_POOL },
	{ 0x080C660C, 0x080C661C, XV_RAW },
	{ 0x080E126A, 0x080E126C, XV_RAW },
	{ 0x080E126C, 0x080E1270, XV_POOL },
	{ 0x080E1270, 0x080E127C, XV_TABLE },
	{ 0x080E128A, 0x080E128C, XV_RAW },
	{ 0x080E128C, 0x080E1290, XV_POOL },
	{ 0x080E1290, 0x080E12A0, XV_TABLE },
	{ 0x080E12AE, 0x080E12B0, XV_RAW },
	{ 0x080E12B0, 0x080E12B4, XV_POOL },
	{ 0x080E12B4, 0x080E12BC, XV_TABLE },
	{ 0x080E12E6, 0x080E12E8, XV_RAW },
	{ 0x080E12E8, 0x080E12EC, XV_POOL },
	{ 0x080E12EC, 0x080E12F0, XV_RAW },
	{ 0x080E17AA, 0x080E17AC, XV_RAW },
	{ 0x080E17AC, 0x080E17B0, XV_POOL },
	{ 0x080E17B0, 0x080E17BC, XV_TABLE },
	{ 0x080E17D6, 0x080E17D8, XV_RAW },
	{ 0x080E17D8, 0x080E17DC, XV_POOL },
	{ 0x080E17DC, 0x080E17E4, XV_TABLE },
	{ 0x080E1802, 0x080E1804, XV_RAW },
	{ 0x080E1804, 0x080E1808, XV_POOL },
	{ 0x080E1808, 0x080E180C, XV_RAW },
	{ 0x080E186E, 0x080E1870, XV_RAW },
	{ 0x080E1870, 0x080E1874, XV_POOL },
	{ 0x080E1874, 0x080E1880, XV_TABLE },
	{ 0x080E18EC, 0x080E18F0, XV_POOL },
	{ 0x080E18F0, 0x080E1900, XV_RAW },
	{ 0x080E1970, 0x080E197C, XV_POOL },
	{ 0x080E4B22, 0x080E4B24, XV_RAW },
	{ 0x080E4B24, 0x080E4B28, XV_POOL },
	{ 0x080E4B28, 0x080E4B34, XV_TABLE },
};
static const XvPair meddy_calls[] = {
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CB20, 0x0800E708 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08018838, 0x0801BD3C },   /* map:bytes, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08006BC2, 0x080077B4 },   /* map:bytes, SAME */
	{ 0x0802EFFE, 0x0802EF5C },   /* map:bytes, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x080EA0BA, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x080C665C, 0x080D1014 },   /* map:bytes, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800C6D8, 0x0800E2D8 },   /* map:calls, DIFF (audited) */
	{ 0x0800C758, 0x0800E2EC },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08001224, 0x08001288 },   /* map:bytes, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x0800DD5E, 0x0800FFFE },   /* map:calls+bytes, SAME */
	{ 0x0800E2C2, 0x0801056A },   /* map:calls, DIFF (audited) */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0802C114, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0802C10A, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0802B804, XN_STUB_DS },   /* a stub: what BN6's engine lacks */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800CEE6, 0x0800EC66 },   /* map:bytes, SAME */
	{ 0x0800A50A, 0x0800BF16 },   /* map:bytes, SAME */
	{ 0x0800B544, 0x0800CF14 },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B5A8, 0x0800CFA6 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
};
static const XvPair meddy_words[] = {
	{ 0x08009FB3, 0x0800B917 },
	{ 0x08009FE9, 0x0800B94D },
	{ 0x0800A01B, 0x0800B9B1 },
	{ 0x0800A2F1, 0x0800BC89 },
	{ 0x0800A327, 0x0800BD35 },
};
static const XvImm meddy_imms[] = {
	{ 0x080C581E, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x15 in BN5, list 2 0x92 in BN6 (its list's offset) */
	{ 0x080C5820, THUMB_MOVS, 2, 0, 0x15, 0x92 },   /* its sprite: list 4 0x15 in BN5, list 2 0x92 in BN6 */
	{ 0x080C58D0, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C5974, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C59FC, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C5A3E, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C5B3E, THUMB_MOVS, 2, 0, 0x44, 0xB0 },   /* its effect: 0x44 in BN5, 0xb0 in BN6 */
	{ 0x080C603E, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x13 in BN5, list 2 0x91 in BN6 (its list's offset) */
	{ 0x080C6040, THUMB_MOVS, 2, 0, 0x13, 0x91 },   /* its sprite: list 4 0x13 in BN5, list 2 0x91 in BN6 */
	{ 0x080C6136, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080C645E, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x10 in BN5, list 2 0x90 in BN6 (its list's offset) */
	{ 0x080C6460, THUMB_MOVS, 2, 0, 0x10, 0x90 },   /* its sprite: list 4 0x10 in BN5, list 2 0x90 in BN6 */
	{ 0x080C64DE, THUMB_MOVS, 1, 0, 0xA, 0xC },   /* its parent's CurAction 10 compared: BN6's 12 */
	{ 0x080C64E6, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080E1886, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x10 in BN5, list 2 0x90 in BN6 (its list's offset) */
	{ 0x080E1888, THUMB_MOVS, 2, 0, 0x10, 0x90 },   /* its sprite: list 4 0x10 in BN5, list 2 0x90 in BN6 */
	{ 0x080E4B3A, THUMB_MOVS, 1, 0, 0x10, 0x8 },   /* its sprite: list 4 0x10 in BN5, list 2 0x90 in BN6 (its list's offset) */
	{ 0x080E4B3C, THUMB_MOVS, 2, 0, 0x10, 0x90 },   /* its sprite: list 4 0x10 in BN5, list 2 0x90 in BN6 */
	{ 0x080F7480, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x080F74F8, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F751E, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x080F7552, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber meddy_numbers[] = {
	{ 0x080C5C90, 0x03880080, 0x03880080 },
};
static const XvRef meddy_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x080F7415), OWN(0x080F760D), OWN(0x080F76EF), OWN(0x080F7785), OWN(0x080F794D),
};
static const XvSprite meddy_sprites[] = {
	{ 4, 0x10, 2, 0x90, XN_TEAM_MORE + 0xE000, 0x800 },
	{ 4, 0x13, 2, 0x91, XN_TEAM_MORE + 0xE800, 0x800 },
	{ 4, 0x15, 2, 0x92, XN_SHARED_FOUR + 0x0, 0x1000 },
	{ 5, 0x19, 5, 0xA8, XN_SHARED_FOUR + 0x1000, 0x1000 },
};
static const XvId meddy_effects[] = { { 0x44, 0xB0 } };
static const XvType meddy_types[] = { { 3, 0x09, XV_TYPE_MOVED }, { 3, 0x0C, XV_TYPE_MOVED }, { 3, 0x0D, XV_TYPE_MOVED }, { 4, 0x0B, XV_TYPE_MOVED }, { 4, 0x0E, XV_TYPE_MOVED }, { 4, 0x0F, XV_TYPE_MOVED }, { 4, 0x3C, XV_TYPE_MOVED } };
static const XnSound meddy_sounds[] = {
	{ 0x080C59AC, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080C615E, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080C6240, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080C62F2, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
	{ 0x080C6534, 0x00000000, 0x0C6 },   /* BN6's song 0xc6 another */
	{ 0x080C65A8, 0x00000000, 0x0BA },   /* BN6's song 0xba another */
	{ 0x080E17EE, 0x00000000, 0x0E0 },   /* BN6's song 0xe0 another */
};

static const XNavi bn5_meddy = {
	.code = {
		.name = "Meddy (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 6,
		.ai = 31,
		.first_id = XN_MORE_IDS + 0x4,
		.home = XN_TEAM_HOMES + 0x2800,
		.blocks = meddy_blocks, .nblocks = (int)(sizeof meddy_blocks / sizeof *meddy_blocks),
		.data = meddy_data, .ndata = (int)(sizeof meddy_data / sizeof *meddy_data),
		.calls = meddy_calls, .ncalls = (int)(sizeof meddy_calls / sizeof *meddy_calls),
		.words = meddy_words, .nwords = (int)(sizeof meddy_words / sizeof *meddy_words),
		.imms = meddy_imms, .nimms = (int)(sizeof meddy_imms / sizeof *meddy_imms),
		.numbers = meddy_numbers, .nnumbers = (int)(sizeof meddy_numbers / sizeof *meddy_numbers),
		.states = meddy_states, .nstates = (int)(sizeof meddy_states / sizeof *meddy_states),
		.hooks = { BN6(0x080F3BE9), BN6(0x080F3BE9), OWN(0x080F7575), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9), BN6(0x080F3BE9) },
		.struct1 = 0x080F72FC, .struct2 = 0x080F7304,
		.names = { 0x11F, 0x120, 0x121, 0x122, 0x123, 0x124 },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x06, .element = 0, .weakness = 0, .shadow = true },
		.sprites = meddy_sprites, .nsprites = (int)(sizeof meddy_sprites / sizeof *meddy_sprites),
		.effects = meddy_effects, .neffects = (int)(sizeof meddy_effects / sizeof *meddy_effects),
		.types = meddy_types, .ntypes = (int)(sizeof meddy_types / sizeof *meddy_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801A9B9), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 6,
	.sounds = meddy_sounds, .nsounds = (int)(sizeof meddy_sounds / sizeof *meddy_sounds),
};
/* ---- GridMan (BN5's: AI 18 there, its ids 0x167, 0x168, 0x169, 0x16a), as Navi AI 44 with ids 0x3f0-0x3f3 ---- */

static const XvBlock gridman_blocks[] = {
	{ 0x08100EC0, 0x08101DB4, XN_NEBULA_CODE + 0x3000, 0xDAD129B2u },
	{ 0x08016B96, 0x08016BBC, XN_NEBULA_CODE + 0x3F02, 0xA53A3BEFu },
	{ 0x080BCAC4, 0x080BD1D0, XN_NEBULA_CODE + 0x4000, 0xDC63A647u },
	{ 0x080C5386, 0x080C53DE, XN_NEBULA_CODE + 0x4802, 0x4A1B50AFu },
	{ 0x080D8F1C, 0x080D976C, XN_NEBULA_CODE + 0x4900, 0x2D99FC1Fu },
	{ 0x080DF714, 0x080DF81C, XN_NEBULA_CODE + 0x5200, 0xF62378E1u },
	{ 0x080DF900, 0x080DFA00, XN_NEBULA_CODE + 0x5400, 0x21E9D2AEu },
	{ 0x080E6E24, 0x080E6EDC, XN_NEBULA_CODE + 0x5500, 0xE6B7F0C0u },
};
static const XvData gridman_data[] = {
	{ 0x08100EC0, 0x08100EF0, XV_ZERO },
	{ 0x08100F06, 0x08100F08, XV_RAW },
	{ 0x08100F08, 0x08100F0C, XV_POOL },
	{ 0x08100F0C, 0x08100F14, XV_TABLE },
	{ 0x08101004, 0x0810100C, XV_POOL },
	{ 0x0810100C, 0x08101010, XV_RAW },
	{ 0x08101090, 0x08101094, XV_POOL },
	{ 0x08101094, 0x08101098, XV_RAW },
	{ 0x08101098, 0x0810109C, XV_POOL },
	{ 0x0810111A, 0x0810111C, XV_RAW },
	{ 0x0810111C, 0x08101120, XV_POOL },
	{ 0x08101120, 0x08101128, XV_RAW },
	{ 0x0810119C, 0x081011A0, XV_POOL },
	{ 0x081011A0, 0x081011A4, XV_RAW },
	{ 0x081011AE, 0x081011B0, XV_RAW },
	{ 0x081011B0, 0x081011B4, XV_POOL },
	{ 0x081011B4, 0x081011B8, XV_RAW },
	{ 0x081011C2, 0x081011C4, XV_RAW },
	{ 0x081011C4, 0x081011C8, XV_POOL },
	{ 0x081011C8, 0x081011CC, XV_RAW },
	{ 0x081011D6, 0x081011D8, XV_RAW },
	{ 0x081011D8, 0x081011DC, XV_POOL },
	{ 0x081011DC, 0x081011E0, XV_RAW },
	{ 0x081011EA, 0x081011EC, XV_RAW },
	{ 0x081011EC, 0x081011F0, XV_POOL },
	{ 0x081011F0, 0x081011F4, XV_RAW },
	{ 0x08101216, 0x08101218, XV_RAW },
	{ 0x08101218, 0x08101228, XV_POOL },
	{ 0x08101228, 0x08101230, XV_RAW },
	{ 0x08101230, 0x08101238, XV_RAW },
	{ 0x08101238, 0x08101240, XV_RAW },
	{ 0x08101240, 0x08101248, XV_RAW },
	{ 0x08101248, 0x0810124C, XV_POOL },
	{ 0x0810124C, 0x08101250, XV_RAW },
	{ 0x0810129A, 0x0810129C, XV_RAW },
	{ 0x0810129C, 0x081012B0, XV_POOL },
	{ 0x081012BE, 0x081012C0, XV_RAW },
	{ 0x081012C0, 0x081012C4, XV_POOL },
	{ 0x081012C4, 0x081012D0, XV_TABLE },
	{ 0x0810131A, 0x0810131C, XV_RAW },
	{ 0x0810131C, 0x08101320, XV_POOL },
	{ 0x08101320, 0x08101330, XV_RAW },
	{ 0x081013A4, 0x081013A8, XV_POOL },
	{ 0x081013A8, 0x081013B8, XV_TABLE },
	{ 0x081014C4, 0x081014C8, XV_POOL },
	{ 0x081014C8, 0x081014D8, XV_RAW },
	{ 0x081014D8, 0x081014E0, XV_POOL },
	{ 0x08101558, 0x0810155C, XV_POOL },
	{ 0x0810155C, 0x08101574, XV_TABLE },
	{ 0x081015B4, 0x081015B8, XV_POOL },
	{ 0x081015B8, 0x081015C8, XV_RAW },
	{ 0x081016B6, 0x081016B8, XV_RAW },
	{ 0x081016B8, 0x081016BC, XV_POOL },
	{ 0x081016BC, 0x081016D4, XV_TABLE },
	{ 0x08101714, 0x08101718, XV_POOL },
	{ 0x08101718, 0x08101728, XV_RAW },
	{ 0x0810181E, 0x08101820, XV_RAW },
	{ 0x08101820, 0x08101824, XV_POOL },
	{ 0x08101824, 0x08101844, XV_TABLE },
	{ 0x0810188C, 0x08101890, XV_POOL },
	{ 0x08101890, 0x081018A0, XV_RAW },
	{ 0x08101A72, 0x08101A74, XV_RAW },
	{ 0x08101A74, 0x08101A7C, XV_POOL },
	{ 0x08101A7C, 0x08101A84, XV_RAW },
	{ 0x08101A84, 0x08101A88, XV_POOL },
	{ 0x08101ADC, 0x08101AE0, XV_POOL },
	{ 0x08101AE0, 0x08101AE8, XV_RAW },
	{ 0x08101AE8, 0x08101AEC, XV_POOL },
	{ 0x08101AEC, 0x08101AF4, XV_RAW },
	{ 0x08101C08, 0x08101C0C, XV_POOL },
	{ 0x08101C0C, 0x08101C14, XV_RAW },
	{ 0x08101C14, 0x08101C18, XV_POOL },
	{ 0x08101C18, 0x08101C1C, XV_RAW },
	{ 0x08101C3A, 0x08101C3C, XV_RAW },
	{ 0x08101C3C, 0x08101C40, XV_POOL },
	{ 0x08101C40, 0x08101C44, XV_RAW },
	{ 0x08101CBE, 0x08101CC0, XV_RAW },
	{ 0x08101CC0, 0x08101CC4, XV_POOL },
	{ 0x08101CC4, 0x08101D34, XV_RAW },
	{ 0x08101D34, 0x08101D38, XV_POOL },
	{ 0x08101D38, 0x08101D48, XV_RAW },
	{ 0x08101D48, 0x08101D4C, XV_POOL },
	{ 0x08101D4C, 0x08101D5C, XV_RAW },
	{ 0x08101DB2, 0x08101DB4, XV_RAW },
	{ 0x080BCAD6, 0x080BCAD8, XV_RAW },
	{ 0x080BCAD8, 0x080BCADC, XV_POOL },
	{ 0x080BCADC, 0x080BCAEC, XV_TABLE },
	{ 0x080BCB6E, 0x080BCB70, XV_RAW },
	{ 0x080BCB70, 0x080BCB74, XV_POOL },
	{ 0x080BCB74, 0x080BCB7C, XV_RAW },
	{ 0x080BCC00, 0x080BCC04, XV_POOL },
	{ 0x080BCC04, 0x080BCC18, XV_TABLE },
	{ 0x080BCC18, 0x080BCC1C, XV_POOL },
	{ 0x080BCC54, 0x080BCC58, XV_POOL },
	{ 0x080BCC58, 0x080BCC70, XV_TABLE },
	{ 0x080BCCFC, 0x080BCD00, XV_POOL },
	{ 0x080BCD00, 0x080BCD04, XV_RAW },
	{ 0x080BCD80, 0x080BCD84, XV_POOL },
	{ 0x080BCD84, 0x080BCD94, XV_TABLE },
	{ 0x080BCE78, 0x080BCE7C, XV_POOL },
	{ 0x080BCE7C, 0x080BCE8C, XV_RAW },
	{ 0x080BCE8C, 0x080BCE90, XV_POOL },
	{ 0x080BCFB4, 0x080BCFB8, XV_POOL },
	{ 0x080BCFB8, 0x080BCFC0, XV_RAW },
	{ 0x080BCFFA, 0x080BCFFC, XV_RAW },
	{ 0x080BCFFC, 0x080BD000, XV_POOL },
	{ 0x080BD000, 0x080BD008, XV_TABLE },
	{ 0x080BD0AC, 0x080BD100, XV_RAW },
	{ 0x080BD100, 0x080BD104, XV_POOL },
	{ 0x080BD104, 0x080BD114, XV_RAW },
	{ 0x080BD190, 0x080BD194, XV_POOL },
	{ 0x080BD194, 0x080BD1C0, XV_RAW },
	{ 0x080BD1C0, 0x080BD1C4, XV_POOL },
	{ 0x080BD1C4, 0x080BD1CC, XV_RAW },
	{ 0x080BD1CC, 0x080BD1D0, XV_POOL },
	{ 0x080D8F2E, 0x080D8F30, XV_RAW },
	{ 0x080D8F30, 0x080D8F34, XV_POOL },
	{ 0x080D8F34, 0x080D8F40, XV_TABLE },
	{ 0x080D8FE4, 0x080D8FE8, XV_POOL },
	{ 0x080D8FE8, 0x080D8FF4, XV_TABLE },
	{ 0x080D9192, 0x080D9194, XV_RAW },
	{ 0x080D9194, 0x080D9198, XV_POOL },
	{ 0x080D9198, 0x080D91B8, XV_RAW },
	{ 0x080D92F4, 0x080D92F8, XV_POOL },
	{ 0x080D92F8, 0x080D9300, XV_RAW },
	{ 0x080D9300, 0x080D9304, XV_POOL },
	{ 0x080D9304, 0x080D9314, XV_RAW },
	{ 0x080D9314, 0x080D9318, XV_POOL },
	{ 0x080D9318, 0x080D9328, XV_RAW },
	{ 0x080D9328, 0x080D932C, XV_POOL },
	{ 0x080D932C, 0x080D9334, XV_RAW },
	{ 0x080D9334, 0x080D9340, XV_POOL },
	{ 0x080D9352, 0x080D9354, XV_RAW },
	{ 0x080D9354, 0x080D9358, XV_POOL },
	{ 0x080D9358, 0x080D9364, XV_TABLE },
	{ 0x080D93E6, 0x080D93E8, XV_RAW },
	{ 0x080D93E8, 0x080D93EC, XV_POOL },
	{ 0x080D93EC, 0x080D93FC, XV_TABLE },
	{ 0x080D9458, 0x080D945C, XV_POOL },
	{ 0x080D95A0, 0x080D95A4, XV_POOL },
	{ 0x080D95A4, 0x080D95AC, XV_RAW },
	{ 0x080D95AC, 0x080D95B4, XV_POOL },
	{ 0x080D9656, 0x080D9658, XV_RAW },
	{ 0x080D9658, 0x080D965C, XV_POOL },
	{ 0x080D965C, 0x080D9664, XV_RAW },
	{ 0x080D969A, 0x080D969C, XV_RAW },
	{ 0x080D969C, 0x080D96A0, XV_POOL },
	{ 0x080D96A0, 0x080D96B0, XV_RAW },
	{ 0x080D96E2, 0x080D96E4, XV_RAW },
	{ 0x080D96E4, 0x080D96E8, XV_POOL },
	{ 0x080D96E8, 0x080D96F0, XV_RAW },
	{ 0x080D974A, 0x080D974C, XV_RAW },
	{ 0x080D974C, 0x080D9750, XV_POOL },
	{ 0x080D9750, 0x080D9758, XV_RAW },
	{ 0x080D9758, 0x080D976C, XV_POOL },
	{ 0x080DF726, 0x080DF728, XV_RAW },
	{ 0x080DF728, 0x080DF72C, XV_POOL },
	{ 0x080DF72C, 0x080DF738, XV_TABLE },
	{ 0x080DF814, 0x080DF81C, XV_POOL },
	{ 0x080DF912, 0x080DF914, XV_RAW },
	{ 0x080DF914, 0x080DF918, XV_POOL },
	{ 0x080DF918, 0x080DF924, XV_TABLE },
	{ 0x080E6E36, 0x080E6E38, XV_RAW },
	{ 0x080E6E38, 0x080E6E3C, XV_POOL },
	{ 0x080E6E3C, 0x080E6E48, XV_TABLE },
};
static const XvPair gridman_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800C758, 0x0800E2EC },   /* map:bytes, SAME */
	{ 0x080138AE, 0x08016C4E },   /* map:calls, DIFF (audited) */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800C6C0, 0x0800E2C0 },   /* map:calls+bytes, SAME */
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ 0x080EA08C, XN_STUB_ZERO },   /* a stub: what BN6's engine lacks */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800B334, 0x0800CC72 },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x080C4EAE, 0x080C6C16 },   /* map:calls+bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800DBBC, 0x0800FE36 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F210, 0x08011690 },   /* map:calls+bytes, DIFF (audited) */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x0800DBAE, 0x0800FE28 },   /* map:bytes, SAME */
	{ 0x0800DB98, 0x0800FE12 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800CB20, 0x0800E708 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08018838, 0x0801BD3C },   /* map:bytes, SAME */
};
static const XvPair gridman_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm gridman_imms[] = {
	{ 0x080BCAF8, THUMB_MOVS, 2, 0, 0x12, 0x32 },   /* its sprite: list 2 0x12 in BN5, list 2 0x32 in BN6 */
	{ 0x080BCBA4, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080BCBBA, THUMB_MOVS, 1, 0, 0xB, 0xD },   /* its parent's CurAction 11 compared: BN6's 13 */
	{ 0x080BCDCC, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080BCE70, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
	{ 0x080BD05E, THUMB_MOVS, 1, 0, 0xB, 0xD },   /* its parent's CurAction 11 compared: BN6's 13 */
	{ 0x080D8F4C, THUMB_MOVS, 2, 0, 0x12, 0x32 },   /* its sprite: list 2 0x12 in BN5, list 2 0x32 in BN6 */
	{ 0x080D936C, THUMB_MOVS, 2, 0, 0x12, 0x32 },   /* its sprite: list 2 0x12 in BN5, list 2 0x32 in BN6 */
	{ 0x080D951C, THUMB_MOVS, 0, 0, 0x5F, 0xB1 },   /* its effect: 0x5f in BN5, 0xb1 in BN6 */
	{ 0x080D9594, THUMB_MOVS, 4, 0, 0x14, 0xA0 },   /* its effect: 0x14 in BN5, 0xa0 in BN6 */
	{ 0x080DF7F8, THUMB_MOVS, 1, 0, 0x8, 0xA },   /* its parent's CurAction 8 compared: BN6's 10 */
	{ 0x080DF9C8, THUMB_MOVS, 1, 0, 0xB, 0xD },   /* its parent's CurAction 11 compared: BN6's 13 */
	{ 0x080E6E62, THUMB_MOVS, 2, 0, 0x12, 0x32 },   /* its sprite: list 2 0x12 in BN5, list 2 0x32 in BN6 */
	{ 0x080E6EAC, THUMB_MOVS, 1, 0, 0xB, 0xD },   /* its parent's CurAction 11 compared: BN6's 13 */
	{ 0x08100F58, THUMB_MOVS, 0, 0, 0xB, 0xD },   /* CurAction 11 (setAttack0): BN6's two status states before the family's own */
	{ 0x08100F92, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x08100FDE, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x08100FFC, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack4): BN6's two status states before the family's own */
	{ 0x08101076, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x081013F6, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x081014BC, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
	{ 0x08101944, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x08101DAA, THUMB_MOVS, 4, 0, 0x2, 0xA2 },   /* its effect: 0x2 in BN5, 0xa2 in BN6 */
};
static const XvNumber gridman_numbers[] = {
	{ 0x080BCC18, 0x0C800000, 0x0C800000 },
	{ 0x080BCE8C, 0x0C800000, 0x0C800000 },
	{ 0x080D95AC, 0x0A050001, 0x0A050001 },
	{ 0x081014DC, 0x0C800000, 0x0C800000 },
	{ 0x08101A84, 0x0C800000, 0x0C800000 },
};
static const XvRef gridman_states[] = {
	BN6(0x08016381), BN6(0x08017889), BN6(0x080170C5), BN6(0x080174FF), BN6(0x080175B9), BN6(0x080178B7), BN6(0x08017689), BN6(0x08017769), OWN(0x08100EF1), OWN(0x081012B1), OWN(0x08101397), OWN(0x0810154B), OWN(0x081016A9), OWN(0x08101811),
};
static const XvSprite gridman_sprites[] = {
	{ 5, 0x02, 5, 0xA2, XN_SHARED_ART + 0x7000, 0x1800 },
	{ 5, 0x0D, 5, 0xA0, XN_SHARED_ART + 0x2000, 0x1800 },
	{ 2, 0x12, 2, 0x32, FREE_NEBULA_SPRITES + 0x0, 0x8800 },
};
static const XvId gridman_effects[] = { { 0x02, 0xA2 }, { 0x14, 0xA0 }, { 0x5F, 0xB1 } };
static const XvType gridman_types[] = { { 1, 0x18, XV_TYPE_MOVED }, { 3, 0x94, XV_TYPE_MOVED }, { 3, 0x95, XV_TYPE_MOVED }, { 3, 0xC6, XV_TYPE_MOVED }, { 3, 0xC8, XV_TYPE_MOVED }, { 4, 0x58, XV_TYPE_MOVED } };
static const XnSound gridman_sounds[] = {
	{ 0x080BCDFE, 0x00000000, 0x0E3 },   /* BN6's song 0xe3 another */
	{ 0x0810144E, 0x00000000, 0x0E3 },   /* BN6's song 0xe3 another */
	{ 0x08101A5A, 0x00000000, 0x0E4 },   /* BN6's song 0xe4 another */
	{ 0x08101AD0, 0x00000000, 0x070 },   /* BN6's song 0x70 another */
};

static const XNavi bn5_gridman = {
	.code = {
		.name = "GridMan (BN5's)",
		.game = XROM_BN5_COLONEL_US,
		.src_ai = 18,
		.ai = 44,
		.first_id = XN_MORE_IDS + 0xC,
		.home = XN_NEBULA_HOMES + 0x800,
		.blocks = gridman_blocks, .nblocks = (int)(sizeof gridman_blocks / sizeof *gridman_blocks),
		.data = gridman_data, .ndata = (int)(sizeof gridman_data / sizeof *gridman_data),
		.calls = gridman_calls, .ncalls = (int)(sizeof gridman_calls / sizeof *gridman_calls),
		.words = gridman_words, .nwords = (int)(sizeof gridman_words / sizeof *gridman_words),
		.imms = gridman_imms, .nimms = (int)(sizeof gridman_imms / sizeof *gridman_imms),
		.numbers = gridman_numbers, .nnumbers = (int)(sizeof gridman_numbers / sizeof *gridman_numbers),
		.states = gridman_states, .nstates = (int)(sizeof gridman_states / sizeof *gridman_states),
		.hooks = { OWN(0x08101129), BN6(0x080F3BE9), OWN(0x0810109D), OWN(0x08101151), OWN(0x08101151), OWN(0x08101151), OWN(0x08101151), OWN(0x08101151) },
		.struct1 = 0x08100DD4, .struct2 = 0x08100DDC,
		.names = { 0x167, 0x168, 0x169, 0x16A, 0x16B, 0x16C },
		.ice = 1,
		.traits = { .laid = true, .mode = 0x12, .element = 0, .weakness = 0, .shadow = false },
		.sprites = gridman_sprites, .nsprites = (int)(sizeof gridman_sprites / sizeof *gridman_sprites),
		.effects = gridman_effects, .neffects = (int)(sizeof gridman_effects / sizeof *gridman_effects),
		.types = gridman_types, .ntypes = (int)(sizeof gridman_types / sizeof *gridman_types),
	},
	.versions = 4,
	.prestep = BN6(0x0801AAC1), .hook_a = BN6(0x080F3BE9),
	.companion = BN6(0x080114D5), .companion_end = BN6(0x080114D5), .hide = BN6(0x080114D5),
	.sprite_of = 18,
	.sounds = gridman_sounds, .nsounds = (int)(sizeof gridman_sounds / sizeof *gridman_sounds),
};
const XNavi *const xnavi_bn5[] = { &bn5_toadman, &bn5_knightman, &bn5_shadowman, &bn5_numberman, &bn5_tomahawkman, &bn5_colonel, &bn5_thedarksoul, &bn5_blizzardman, &bn5_shademan, &bn5_cosmoman, &bn5_cloudman, &bn5_napalmman, &bn5_gyroman, &bn5_searchman, &bn5_meddy, &bn5_gridman };
const int xnavi_bn5_count = (int)(sizeof xnavi_bn5 / sizeof *xnavi_bn5);

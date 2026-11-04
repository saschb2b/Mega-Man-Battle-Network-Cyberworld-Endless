/* xvirus_bn4.c: BN4 Red Sun's virus families' recipes, written by tools/xrecipe_bn4.py from BN4's and BN6's ROMs
 * (python3 tools/xrecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/families.md): addresses
 * in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and CRC32s. The families here are the generator's whole ones
 * (no decision open; its interims named over each), installed only where BN4's ROM is read for research (xrom_research:
 * --dev bn4families); what stays open is in its report (.build/xrecipe/bn4_report.md). First, BN6's own types copied for
 * them with a number BN4's (tools/xrecipe_bn4_copy.py; xvirus_bn4_copy.h installs them before the families). */
#include "xvirus_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }
#define ENTERED(a) { (a), true, 0, true }

/* ---- BN6's third-kind 0x2a copied for BN4's families (bn4_twins.COPIED6; docs/rom/bn4.md, The thunder's copy): run as the dispatch's 0x2a, its own number (no BN5 track names it) ---- */
static const XvBlock bn6copy_blocks[] = {
	{ 0x080CAC3C, 0x080CAF10, XV_CODE + 0x14000, 0xE83405D1u },
};
static const XvData bn6copy_data[] = {
	{ 0x080CAC4E, 0x080CAC50, XV_RAW },
	{ 0x080CAC50, 0x080CAC54, XV_POOL },
	{ 0x080CAC54, 0x080CAC60, XV_TABLE },
	{ 0x080CAD6E, 0x080CAD70, XV_RAW },
	{ 0x080CAD70, 0x080CAD74, XV_POOL },
	{ 0x080CAD74, 0x080CAD78, XV_TABLE },
	{ 0x080CAD78, 0x080CAD7C, XV_POOL },
	{ 0x080CAE4A, 0x080CAE4C, XV_RAW },
	{ 0x080CAE4C, 0x080CAE5C, XV_POOL },
	{ 0x080CAECC, 0x080CAED4, XV_POOL },
	{ 0x080CAED4, 0x080CAF10, XV_RAW },
};
static const XvPair bn6copy_calls[] = {
	{ 0x080005CC, 0x080005CC },   /* BN6's own */
	{ 0x080026A4, 0x080026A4 },   /* BN6's own */
	{ 0x080026E4, 0x080026E4 },   /* BN6's own */
	{ 0x08002D80, 0x08002D80 },   /* BN6's own */
	{ 0x08002DA4, 0x08002DA4 },   /* BN6's own */
	{ 0x08002DEA, 0x08002DEA },   /* BN6's own */
	{ 0x08002F5C, 0x08002F5C },   /* BN6's own */
	{ 0x08002F90, 0x08002F90 },   /* BN6's own */
	{ 0x08003358, XV_SPAWN_3 },   /* BN6's spawner: the dispatch's */
	{ 0x08003458, 0x08003458 },   /* BN6's own */
	{ 0x0800A18E, 0x0800A18E },   /* BN6's own */
	{ 0x0800C8F8, 0x0800C8F8 },   /* BN6's own */
	{ 0x0800CC66, 0x0800CC66 },   /* BN6's own */
	{ 0x0800CCBE, 0x0800CCBE },   /* BN6's own */
	{ 0x0800E29C, 0x0800E29C },   /* BN6's own */
	{ 0x0800E2AC, 0x0800E2AC },   /* BN6's own */
	{ 0x0800E2CA, 0x0800E2CA },   /* BN6's own */
	{ 0x0800E456, 0x0800E456 },   /* BN6's own */
	{ 0x08016C8A, 0x08016C8A },   /* BN6's own */
	{ 0x08019892, 0x08019892 },   /* BN6's own */
	{ 0x08019FB4, 0x08019FB4 },   /* BN6's own */
	{ 0x0801A00E, 0x0801A00E },   /* BN6's own */
	{ 0x0801A018, 0x0801A018 },   /* BN6's own */
	{ 0x0801A04C, 0x0801A04C },   /* BN6's own */
	{ 0x0801A074, 0x0801A074 },   /* BN6's own */
	{ 0x0801A0D4, 0x0801A0D4 },   /* BN6's own */
	{ 0x0801A140, 0x0801A140 },   /* BN6's own */
	{ 0x0801A258, 0x0801A258 },   /* BN6's own */
	{ 0x0801A4D0, 0x0801A4D0 },   /* BN6's own */
	{ 0x0801BBAC, 0x0801BBAC },   /* BN6's own */
};
static const XvImm bn6copy_imms[] = {
	{ 0x080CACD6, THUMB_MOVS, 3, 0, 0x1, 0x0 },   /* its collision set up (0x08019FB4) with r3 0, as BN4's init (0x080D208C) passes, where BN6's passes 1 */
};
static const XvNumber bn6copy_numbers[] = {
	{ 0x080CAECC, 0x04000000, 0x04000000 },   /* a mask of BN6's own code, kept */
	{ 0x080CAED0, 0x08000000, 0x08000000 },   /* a mask of BN6's own code, kept */
};
static const XvType bn6copy_types[] = {
	{ 3, 0x2A, XV_TYPE_MOVED, 0x2A },   /* BN6's third-kind 0x2a, copied, the dispatch's 0x2a */
};
const XVirusFamily xvirus_bn4_bn6copy = {
	.name = "BN6's third-kind 0x2a for BN4's families",
	.game = XV_GAME_BN6,
	.blocks = bn6copy_blocks, .nblocks = (int)(sizeof bn6copy_blocks / sizeof *bn6copy_blocks),
	.data = bn6copy_data, .ndata = (int)(sizeof bn6copy_data / sizeof *bn6copy_data),
	.calls = bn6copy_calls, .ncalls = (int)(sizeof bn6copy_calls / sizeof *bn6copy_calls),
	.imms = bn6copy_imms, .nimms = (int)(sizeof bn6copy_imms / sizeof *bn6copy_imms),
	.numbers = bn6copy_numbers, .nnumbers = (int)(sizeof bn6copy_numbers / sizeof *bn6copy_numbers),
	.types = bn6copy_types, .ntypes = (int)(sizeof bn6copy_types / sizeof *bn6copy_types),
};
const uint32_t xvirus_bn4_bn6copy_veneers = XV_CODE + 0x142D4;
const int xvirus_bn4_bn6copy_nveneers = 30;

/* ---- BN4's Billy (its type 0x13 there, its enemy ids 13-18), as AI 64 with ids 0x2c0-0x2c5 ---- */

static const XvBlock billy_blocks[] = {
	{ 0x080B94E4, 0x080B999C, XV_CODE + 0x14440, 0xBBE06B34u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x14902, 0x793106FEu },
	{ 0x08010736, 0x0801079C, XV_CODE + 0x14D12, 0x7631C49Cu },
	{ 0x080B52E4, 0x080B52EC, XV_CODE + 0x14D80, 0xF6607FB0u },
	{ 0x080B54D0, 0x080B54F4, XV_CODE + 0x14D90, 0xB6A58CE9u },
};
static const XvData billy_data[] = {
	{ 0x080B94E4, 0x080B94F6, XV_ZERO },
	{ 0x080B94F6, 0x080B94F8, XV_RAW },
	{ 0x080B94F8, 0x080B9562, XV_ZERO },
	{ 0x080B9562, 0x080B9564, XV_RAW },
	{ 0x080B9564, 0x080B957A, XV_ZERO },
	{ 0x080B957A, 0x080B957C, XV_RAW },
	{ 0x080B957C, 0x080B95A8, XV_ZERO },
	{ 0x080B95CA, 0x080B95CC, XV_RAW },
	{ 0x080B95CC, 0x080B95D0, XV_POOL },
	{ 0x080B95D0, 0x080B95DC, XV_TABLE },
	{ 0x080B95EA, 0x080B95EC, XV_RAW },
	{ 0x080B95EC, 0x080B95F0, XV_POOL },
	{ 0x080B95F0, 0x080B95F8, XV_TABLE },
	{ 0x080B96B2, 0x080B96B4, XV_RAW },
	{ 0x080B96B4, 0x080B96B8, XV_POOL },
	{ 0x080B96B8, 0x080B96C4, XV_RAW },
	{ 0x080B96C4, 0x080B96C8, XV_POOL },
	{ 0x080B96C8, 0x080B96D8, XV_RAW },
	{ 0x080B96D8, 0x080B96DC, XV_POOL },
	{ 0x080B9796, 0x080B9798, XV_RAW },
	{ 0x080B9798, 0x080B979C, XV_POOL },
	{ 0x080B97BE, 0x080B97C0, XV_RAW },
	{ 0x080B97C0, 0x080B97C4, XV_POOL },
	{ 0x080B97C4, 0x080B97D4, XV_TABLE },
	{ 0x080B9884, 0x080B9888, XV_POOL },
	{ 0x080B9888, 0x080B988E, XV_RAW },
	{ 0x080B98B4, 0x080B98B8, XV_POOL },
	{ 0x080B98B8, 0x080B98C4, XV_TABLE },
	{ 0x080B9950, 0x080B9954, XV_POOL },
	{ 0x080B9954, 0x080B995C, XV_RAW },
	{ 0x080B995C, 0x080B9960, XV_POOL },
	{ 0x080B9960, 0x080B9968, XV_RAW },
	{ 0x080B9968, 0x080B996C, XV_POOL },
	{ 0x080B996C, 0x080B9984, XV_RAW },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0801077A, 0x08010798, XV_ZERO },
	{ 0x08010798, 0x0801079C, XV_POOL },
};
static const XvPair billy_calls[] = {
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007BE4, 0x0800A704 },   /* project:research, SAME */
	{ 0x08007C20, 0x0800A754 },   /* project:research, SAME */
	{ 0x080090B8, 0x0800BF16 },   /* project:research, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0800F83C, 0x08015D80 },   /* project:research, SAME */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08012FB0, 0x0801A180 },   /* project:after-state, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080190D0, 0x08021AA4 },   /* project:billy, SAME */
	{ 0x08044AEC, 0x080011EC },   /* project:billy, DIFF (audited) */
	{ 0x080B3878, 0x0800EC48 },   /* project:research, SAME */
	{ 0x080B6568, 0x0800EBD4 },   /* project:billy, DIFF (audited) */
	{ 0x080BE634, 0x0810A952 },   /* project:billy, DIFF (audited) */
	{ 0x080D2156, XV_CODE + 0x14180 },   /* BN6's 080CADBC as copied for BN4's families: its third-kind 0x2a run as the dispatch's 0x2a (bn4_twins.COPIED6; project:billy, DIFF, audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ 0x080E514E, 0x0810A952 },   /* project:billy, DIFF (audited) */
};
static const XvImm billy_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x080B9600, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B9650, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080B967A, THUMB_CMP, 2, 0, 0x9, 0xB },   /* another of its family's (its NameID range's) CurAction 9 compared */
	{ 0x080B968E, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080B9730, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B97A6, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B97FE, THUMB_MOVS, 4, 0, 0x16, 0x91 },   /* its effect: 0x16 in BN4, 0x91 in BN6 (moved) */
	{ 0x080B9816, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B984A, THUMB_MOVS, 4, 0, 0x15, 0x90 },   /* its effect: 0x15 in BN4, 0x90 in BN6 (moved) */
	{ 0x080B989C, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B9992, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
};
static const XvRange billy_ranges[] = {
	{ 0x080B966C, 0x2C0, 0x2C5 },   /* BN4's ids 0xd-0x12, its own */
};
static const XvRef billy_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080B95A9), OWN(0x080B979D), OWN(0x080B97B1), OWN(0x080B98A7),
};
static const XvSprite billy_sprites[] = {
	{ 1, 0x03, 1, 0xC0, XV_ART + 0x60000, 0x3000 },
	{ 5, 0x0D, 5, 0x98, XV_ART + 0x63000, 0x2300 },
};
static const XvId billy_effects[] = { { 0x15, 0x90 }, { 0x16, 0x91 } };

static const XVirusFamily bn4_billy = {
	.name = "Billy (BN4's)",
	.game = XROM_BN4_RED_SUN_US,
	.src_ai = 0x13,   /* (its first-kind type: BN4 has no AI index) */
	.ai = 64,
	.first_id = XV_FIRST_ID + 0xC0,
	.home = XV_HOMES + 0x10000,
	.blocks = billy_blocks, .nblocks = (int)(sizeof billy_blocks / sizeof *billy_blocks),
	.data = billy_data, .ndata = (int)(sizeof billy_data / sizeof *billy_data),
	.calls = billy_calls, .ncalls = (int)(sizeof billy_calls / sizeof *billy_calls),
	.imms = billy_imms, .nimms = (int)(sizeof billy_imms / sizeof *billy_imms),
	.ranges = billy_ranges, .nranges = (int)(sizeof billy_ranges / sizeof *billy_ranges),
	.states = billy_states, .nstates = (int)(sizeof billy_states / sizeof *billy_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08011924, .struct2 = 0x08011924,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 2, .flags = 0x08 },
	.sprites = billy_sprites, .nsprites = (int)(sizeof billy_sprites / sizeof *billy_sprites),
	.effects = billy_effects, .neffects = (int)(sizeof billy_effects / sizeof *billy_effects),
	.present = 0x08010736,   /* its after-state, copied: run after its state 0 and its hook 5 while BN6's battle is paused */
};
/* ---- BN4's Spikey (its type 0x17 there, its enemy ids 25-30), as AI 65 with ids 0x2c6-0x2cb ---- */
/* interim: its sounds 0xbc and 0xc4 (080D2F26, 080D2E86) kept as BN6's same numbers, other songs: BN4's sounds have no slots among BN6's empty songs yet (src/audio/xsong.c's allocations), where a copy of each would go */

static const XvBlock spikey_blocks[] = {
	{ 0x080BA404, 0x080BA7F4, XV_CODE + 0x14DC0, 0x7BA719BAu },
	{ 0x080D2DE8, 0x080D2FDC, XV_CODE + 0x151B0, 0x6EDE0007u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x153B2, 0x793106FEu },
	{ 0x08010736, 0x0801079C, XV_CODE + 0x157C2, 0x7631C49Cu },
	{ 0x080B552A, 0x080B5594, XV_CODE + 0x15832, 0x29597AFDu },
	{ 0x080B56A4, 0x080B56E4, XV_CODE + 0x158A0, 0x67EEC233u },
};
static const XvData spikey_data[] = {
	{ 0x080BA404, 0x080BA416, XV_ZERO },
	{ 0x080BA416, 0x080BA418, XV_RAW },
	{ 0x080BA418, 0x080BA48A, XV_ZERO },
	{ 0x080BA4A0, 0x080BA4CC, XV_ZERO },
	{ 0x080BA4DE, 0x080BA4E0, XV_RAW },
	{ 0x080BA4E0, 0x080BA4E4, XV_POOL },
	{ 0x080BA4E4, 0x080BA4E8, XV_TABLE },
	{ 0x080BA4F6, 0x080BA4F8, XV_RAW },
	{ 0x080BA4F8, 0x080BA4FC, XV_POOL },
	{ 0x080BA4FC, 0x080BA500, XV_TABLE },
	{ 0x080BA5B6, 0x080BA5B8, XV_RAW },
	{ 0x080BA5B8, 0x080BA5BC, XV_POOL },
	{ 0x080BA61C, 0x080BA624, XV_POOL },
	{ 0x080BA646, 0x080BA648, XV_RAW },
	{ 0x080BA648, 0x080BA64C, XV_POOL },
	{ 0x080BA64C, 0x080BA65C, XV_TABLE },
	{ 0x080BA718, 0x080BA71C, XV_POOL },
	{ 0x080BA71C, 0x080BA728, XV_POOL },
	{ 0x080BA728, 0x080BA73A, XV_RAW },
	{ 0x080BA75C, 0x080BA760, XV_POOL },
	{ 0x080BA760, 0x080BA770, XV_TABLE },
	{ 0x080BA7E8, 0x080BA7EC, XV_POOL },
	{ 0x080BA7EC, 0x080BA7F4, XV_RAW },
	{ 0x080D2DFA, 0x080D2DFC, XV_RAW },
	{ 0x080D2DFC, 0x080D2E00, XV_POOL },
	{ 0x080D2E00, 0x080D2E0C, XV_TABLE },
	{ 0x080D2E90, 0x080D2E94, XV_POOL },
	{ 0x080D2E94, 0x080D2EAC, XV_RAW },
	{ 0x080D2EAC, 0x080D2EB0, XV_POOL },
	{ 0x080D2EB0, 0x080D2EB6, XV_RAW },
	{ 0x080D2F34, 0x080D2F38, XV_POOL },
	{ 0x080D2F38, 0x080D2F3C, XV_TABLE },
	{ 0x080D2F3C, 0x080D2F40, XV_POOL },
	{ 0x080D2F40, 0x080D2F4C, XV_POOL },
	{ 0x080D2F4C, 0x080D2F5C, XV_RAW },
	{ 0x080D2FD4, 0x080D2FDC, XV_POOL },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0801077A, 0x08010798, XV_ZERO },
	{ 0x08010798, 0x0801079C, XV_POOL },
	{ 0x080B557E, 0x080B5580, XV_ZERO },
	{ 0x080B5580, 0x080B5584, XV_POOL },
	{ 0x080B5584, 0x080B5594, XV_RAW },
	{ 0x080B56D0, 0x080B56D4, XV_POOL },
	{ 0x080B56D4, 0x080B56E4, XV_RAW },
};
static const XvPair spikey_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x08009FC4, 0x0800CE64 },   /* project:research, SAME */
	{ 0x0800A0A2, 0x0800CFA6 },   /* project:spikey, SAME */
	{ 0x0800A10E, 0x0800D012 },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
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
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08012FB0, 0x0801A180 },   /* project:after-state, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080B3878, 0x0800EC48 },   /* project:research, SAME */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm spikey_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x080BA53A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080BA5A4, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080BA62E, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA68A, THUMB_MOVS, 4, 0, 0x14, 0x93 },   /* its effect: 0x14 in BN4, 0x93 in BN6 (moved) */
	{ 0x080BA69E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BA6BE, THUMB_MOVS, 4, 0, 0x13, 0x92 },   /* its effect: 0x13 in BN4, 0x92 in BN6 (moved) */
	{ 0x080BA744, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA7DA, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080D2E18, THUMB_MOVS, 2, 0, 0x13, 0xC0 },   /* its object's sprite: list 4 0x13 in BN4, list 4 0xc0 in BN6 (copied) */
	{ 0x080D2ECA, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D2EFC, THUMB_MOVS, 4, 0, 0x2A, 0x94 },   /* its effect: 0x2a in BN4, 0x94 in BN6 (moved) */
	{ 0x080D2F74, THUMB_MOVS, 0, 0, 0x30, 0xE0 },   /* its object's type 0x30: moved, the dispatch's 0xe0 */
	{ 0x080D2F84, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D2FA6, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D2FCC, THUMB_MOVS, 4, 0, 0x2A, 0x94 },   /* its effect: 0x2a in BN4, 0x94 in BN6 (moved) */
};
static const XvNumber spikey_numbers[] = {
	{ 0x080D2FD4, 0x0405FF01, 0x0405FF01 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4: its hit's kinds), which BN6's own attacks pass the same (eight of its pools) */
	{ 0x080D2FD8, 0x0A800000, 0x0A800000 },   /* the flags its hits left it, tested (BN4_COLL_FROM_COLLISION): BN6's bits at FlagsFromCollision (+0x70), where the thunder's update tests the same mask, 0xFF800000, in both games */
};
static const XvType spikey_types[] = {
	{ 3, 0xE0, XV_TYPE_MOVED, 0x30 },   /* BN4's third-kind 0x30, moved, the dispatch's 0xe0 */
};
static const XvRef spikey_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080BA4CD), OWN(0x080BA625), OWN(0x080BA639), OWN(0x080BA74F),
};
static const XvSprite spikey_sprites[] = {
	{ 1, 0x05, 1, 0xC1, XV_ART + 0x65300, 0x3000 },   /* (LZ77 in BN4) */
	{ 4, 0x13, 4, 0xC0, XV_ART + 0x68300, 0x800 },
	{ 5, 0x11, 5, 0x99, XV_ART + 0x68B00, 0x2F00 },
};
static const XvId spikey_effects[] = { { 0x13, 0x92 }, { 0x14, 0x93 }, { 0x2A, 0x94 } };

static const XVirusFamily bn4_spikey = {
	.name = "Spikey (BN4's)",
	.game = XROM_BN4_RED_SUN_US,
	.src_ai = 0x17,   /* (its first-kind type: BN4 has no AI index) */
	.ai = 65,
	.first_id = XV_FIRST_ID + 0xC6,
	.home = XV_HOMES + 0x10800,
	.blocks = spikey_blocks, .nblocks = (int)(sizeof spikey_blocks / sizeof *spikey_blocks),
	.data = spikey_data, .ndata = (int)(sizeof spikey_data / sizeof *spikey_data),
	.calls = spikey_calls, .ncalls = (int)(sizeof spikey_calls / sizeof *spikey_calls),
	.imms = spikey_imms, .nimms = (int)(sizeof spikey_imms / sizeof *spikey_imms),
	.numbers = spikey_numbers, .nnumbers = (int)(sizeof spikey_numbers / sizeof *spikey_numbers),
	.types = spikey_types, .ntypes = (int)(sizeof spikey_types / sizeof *spikey_types),
	.states = spikey_states, .nstates = (int)(sizeof spikey_states / sizeof *spikey_states),
	.hooks = { ENTERED(0x080BA48B), BN6(BN6_FAMILY_NULLSUB), OWN(0x080BA5BD), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	/* hook 3: Spikey's update past its pre-step, state and present (BN6's dispatcher runs each): its animation 2 back to 0 at its last frame */
	.struct1 = 0x080119B4, .struct2 = 0x080119B4,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = false, .palette = 1, .flags = 0x00 },
	.sprites = spikey_sprites, .nsprites = (int)(sizeof spikey_sprites / sizeof *spikey_sprites),
	.effects = spikey_effects, .neffects = (int)(sizeof spikey_effects / sizeof *spikey_effects),
	.present = 0x08010736,   /* its after-state, copied: run after its state 0 and its hook 5 while BN6's battle is paused */
};
/* ---- BN4's Shrimpy (its type 0x16 there, its enemy ids 31-36), as AI 66 with ids 0x2cc-0x2d1 ---- */
/* interim: its sounds 0x9f and 0xc9 and 0xca (080D2C54, 080D2BEC, 080D2D26) kept as BN6's same numbers, other songs: BN4's sounds have no slots among BN6's empty songs yet (src/audio/xsong.c's allocations), where a copy of each would go */

static const XvBlock shrimpy_blocks[] = {
	{ 0x080BA0AC, 0x080BA404, XV_CODE + 0x158E0, 0x9599E33Fu },
	{ 0x080D2B28, 0x080D2DE8, XV_CODE + 0x15C40, 0xAAC60F82u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x15F02, 0x793106FEu },
	{ 0x08010736, 0x0801079C, XV_CODE + 0x16312, 0x7631C49Cu },
};
static const XvData shrimpy_data[] = {
	{ 0x080BA0AC, 0x080BA0BE, XV_ZERO },
	{ 0x080BA0BE, 0x080BA0C0, XV_RAW },
	{ 0x080BA0C0, 0x080BA11A, XV_ZERO },
	{ 0x080BA12E, 0x080BA13E, XV_ZERO },
	{ 0x080BA154, 0x080BA180, XV_ZERO },
	{ 0x080BA192, 0x080BA194, XV_RAW },
	{ 0x080BA194, 0x080BA198, XV_POOL },
	{ 0x080BA198, 0x080BA19C, XV_TABLE },
	{ 0x080BA1AA, 0x080BA1AC, XV_RAW },
	{ 0x080BA1AC, 0x080BA1B0, XV_POOL },
	{ 0x080BA1B0, 0x080BA1B4, XV_TABLE },
	{ 0x080BA1E2, 0x080BA1E4, XV_RAW },
	{ 0x080BA1E4, 0x080BA1E8, XV_POOL },
	{ 0x080BA1E8, 0x080BA21E, XV_RAW },
	{ 0x080BA288, 0x080BA294, XV_POOL },
	{ 0x080BA2C6, 0x080BA2C8, XV_RAW },
	{ 0x080BA2C8, 0x080BA2CC, XV_POOL },
	{ 0x080BA2CC, 0x080BA2D4, XV_TABLE },
	{ 0x080BA322, 0x080BA324, XV_RAW },
	{ 0x080BA324, 0x080BA328, XV_POOL },
	{ 0x080BA328, 0x080BA340, XV_RAW },
	{ 0x080BA340, 0x080BA344, XV_POOL },
	{ 0x080BA344, 0x080BA34A, XV_RAW },
	{ 0x080BA39E, 0x080BA3A0, XV_RAW },
	{ 0x080BA3A0, 0x080BA3A4, XV_POOL },
	{ 0x080BA3A4, 0x080BA3AC, XV_TABLE },
	{ 0x080BA3E4, 0x080BA3E8, XV_POOL },
	{ 0x080BA3E8, 0x080BA3EE, XV_RAW },
	{ 0x080BA402, 0x080BA404, XV_RAW },
	{ 0x080D2B3A, 0x080D2B3C, XV_RAW },
	{ 0x080D2B3C, 0x080D2B40, XV_POOL },
	{ 0x080D2B40, 0x080D2B4C, XV_TABLE },
	{ 0x080D2BF6, 0x080D2BF8, XV_RAW },
	{ 0x080D2BF8, 0x080D2BFC, XV_POOL },
	{ 0x080D2BFC, 0x080D2C14, XV_RAW },
	{ 0x080D2C14, 0x080D2C18, XV_POOL },
	{ 0x080D2C18, 0x080D2C1E, XV_RAW },
	{ 0x080D2C9C, 0x080D2CA0, XV_POOL },
	{ 0x080D2CA0, 0x080D2CA8, XV_TABLE },
	{ 0x080D2CA8, 0x080D2CAC, XV_POOL },
	{ 0x080D2CAC, 0x080D2CB8, XV_POOL },
	{ 0x080D2CB8, 0x080D2CC8, XV_RAW },
	{ 0x080D2D36, 0x080D2D38, XV_RAW },
	{ 0x080D2D38, 0x080D2D3C, XV_POOL },
	{ 0x080D2D3C, 0x080D2D54, XV_RAW },
	{ 0x080D2D54, 0x080D2D58, XV_POOL },
	{ 0x080D2DBE, 0x080D2DC0, XV_RAW },
	{ 0x080D2DC0, 0x080D2DC4, XV_POOL },
	{ 0x080D2DE4, 0x080D2DE8, XV_POOL },
	{ 0x0800AD54, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0801077A, 0x08010798, XV_ZERO },
	{ 0x08010798, 0x0801079C, XV_POOL },
};
static const XvPair shrimpy_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
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
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08012FB0, 0x0801A180 },   /* project:after-state, LAYOUT */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080BA12E, 0x0810A952 },   /* its update: BN6's dispatcher runs its state and present, its own step is hook 3 */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm shrimpy_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x080BA24C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BA25A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080BA29A, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080BA2AE, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA318, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BA386, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA3F8, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080D2B6E, THUMB_MOVS, 2, 0, 0xB, 0xC1 },   /* its object's sprite: list 4 0xb in BN4, list 4 0xc1 in BN6 (copied) */
	{ 0x080D2C32, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D2C6A, THUMB_MOVS, 4, 0, 0x25, 0x95 },   /* its effect: 0x25 in BN4, 0x95 in BN6 (moved) */
	{ 0x080D2D5C, THUMB_MOVS, 0, 0, 0x2F, 0xE1 },   /* its object's type 0x2f: moved, the dispatch's 0xe1 */
	{ 0x080D2D6C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D2D90, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D2DB6, THUMB_MOVS, 4, 0, 0x25, 0x95 },   /* its effect: 0x25 in BN4, 0x95 in BN6 (moved) */
};
static const XvNumber shrimpy_numbers[] = {
	{ 0x080D2DC0, 0x0405FF01, 0x0405FF01 },   /* a collision region's spawn word (object_spawnCollisionRegion's r4: its hit's kinds), which BN6's own attacks pass the same (eight of its pools) */
	{ 0x080D2DE4, 0x0A800000, 0x0A800000 },   /* the flags its hits left it, tested (BN4_COLL_FROM_COLLISION): BN6's bits at FlagsFromCollision (+0x70), where the thunder's update tests the same mask, 0xFF800000, in both games */
};
static const XvType shrimpy_types[] = {
	{ 3, 0xE1, XV_TYPE_MOVED, 0x2F },   /* BN4's third-kind 0x2f, moved, the dispatch's 0xe1 */
};
static const XvRef shrimpy_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080BA181), OWN(0x080BA2A5), OWN(0x080BA2B9), OWN(0x080BA391),
};
static const XvSprite shrimpy_sprites[] = {
	{ 1, 0x06, 1, 0xC2, XV_ART + 0x6BA00, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x0B, 4, 0xC1, XV_ART + 0x6DA00, 0x1000 },
	{ 3, 0x1A, 4, 0xC2, XV_ART + 0x6EA00, 0x1100 },
};
static const XvId shrimpy_effects[] = { { 0x25, 0x95 } };

static const XVirusFamily bn4_shrimpy = {
	.name = "Shrimpy (BN4's)",
	.game = XROM_BN4_RED_SUN_US,
	.src_ai = 0x16,   /* (its first-kind type: BN4 has no AI index) */
	.ai = 66,
	.first_id = XV_FIRST_ID + 0xCC,
	.home = XV_HOMES + 0x11000,
	.blocks = shrimpy_blocks, .nblocks = (int)(sizeof shrimpy_blocks / sizeof *shrimpy_blocks),
	.data = shrimpy_data, .ndata = (int)(sizeof shrimpy_data / sizeof *shrimpy_data),
	.calls = shrimpy_calls, .ncalls = (int)(sizeof shrimpy_calls / sizeof *shrimpy_calls),
	.imms = shrimpy_imms, .nimms = (int)(sizeof shrimpy_imms / sizeof *shrimpy_imms),
	.numbers = shrimpy_numbers, .nnumbers = (int)(sizeof shrimpy_numbers / sizeof *shrimpy_numbers),
	.types = shrimpy_types, .ntypes = (int)(sizeof shrimpy_types / sizeof *shrimpy_types),
	.states = shrimpy_states, .nstates = (int)(sizeof shrimpy_states / sizeof *shrimpy_states),
	.hooks = { ENTERED(0x080BA13F), BN6(BN6_FAMILY_NULLSUB), ENTERED(0x080BA11B), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	/* hook 3: Shrimpy's update past its pre-step, state and present (BN6's dispatcher runs each): its animation 1 back to 0 at its last frame */
	/* hook 5: Shrimpy's init past BN6's virus init's work (its sprite, its collision and its float, struct2's flag 4): its first way, up or down, at random (+7), then +8 4, as BN6's init sets it after, and its update's call (a nullsub: its tail is hook 3) */
	.struct1 = 0x080119FC, .struct2 = 0x080119FC,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 1, .flags = 0x04 },
	.sprites = shrimpy_sprites, .nsprites = (int)(sizeof shrimpy_sprites / sizeof *shrimpy_sprites),
	.effects = shrimpy_effects, .neffects = (int)(sizeof shrimpy_effects / sizeof *shrimpy_effects),
	.present = 0x08010736,   /* its after-state, copied: run after its state 0 and its hook 5 while BN6's battle is paused */
};
/* ---- BN4's Walla (its type 0x25 there, its enemy ids 103-108), as AI 67 with ids 0x2d2-0x2d7 ---- */
/* interim: its sounds 0x70 and 0x9a and 0xa6 and 0xb7 (080D4C3C, 080BEA8A, 080D4CE4, 080BEAEA) kept as BN6's same numbers, other songs: BN4's sounds have no slots among BN6's empty songs yet (src/audio/xsong.c's allocations), where a copy of each would go */

static const XvBlock walla_blocks[] = {
	{ 0x080BE888, 0x080BEB68, XV_CODE + 0x16380, 0xD185211Eu },
	{ 0x080D4B04, 0x080D4D30, XV_CODE + 0x16660, 0xDA54AE1Cu },
	{ 0x08010736, 0x0801079C, XV_CODE + 0x16892, 0x7631C49Cu },
	{ 0x080B52E4, 0x080B52EC, XV_CODE + 0x16900, 0xF6607FB0u },
	{ 0x080B54D0, 0x080B54F4, XV_CODE + 0x16910, 0xB6A58CE9u },
};
static const XvData walla_data[] = {
	{ 0x080BE888, 0x080BE89A, XV_ZERO },
	{ 0x080BE89A, 0x080BE89C, XV_RAW },
	{ 0x080BE89C, 0x080BE934, XV_ZERO },
	{ 0x080BE946, 0x080BE948, XV_RAW },
	{ 0x080BE948, 0x080BE94C, XV_POOL },
	{ 0x080BE94C, 0x080BE958, XV_TABLE },
	{ 0x080BE966, 0x080BE968, XV_RAW },
	{ 0x080BE968, 0x080BE96C, XV_POOL },
	{ 0x080BE96C, 0x080BE970, XV_TABLE },
	{ 0x080BE998, 0x080BE99C, XV_POOL },
	{ 0x080BEAAE, 0x080BEAB0, XV_RAW },
	{ 0x080BEAB0, 0x080BEAB8, XV_POOL },
	{ 0x080BEAC6, 0x080BEAC8, XV_RAW },
	{ 0x080BEAC8, 0x080BEACC, XV_POOL },
	{ 0x080BEACC, 0x080BEAD4, XV_TABLE },
	{ 0x080BEB3A, 0x080BEB3C, XV_RAW },
	{ 0x080BEB3C, 0x080BEB40, XV_POOL },
	{ 0x080BEB40, 0x080BEB46, XV_RAW },
	{ 0x080D4B16, 0x080D4B18, XV_RAW },
	{ 0x080D4B18, 0x080D4B1C, XV_POOL },
	{ 0x080D4B1C, 0x080D4B28, XV_TABLE },
	{ 0x080D4BC4, 0x080D4BCC, XV_POOL },
	{ 0x080D4BCC, 0x080D4BDC, XV_RAW },
	{ 0x080D4BDC, 0x080D4BE4, XV_POOL },
	{ 0x080D4C4A, 0x080D4C4C, XV_RAW },
	{ 0x080D4C4C, 0x080D4C50, XV_POOL },
	{ 0x080D4C50, 0x080D4C5C, XV_TABLE },
	{ 0x080D4CBA, 0x080D4CBC, XV_RAW },
	{ 0x080D4CBC, 0x080D4CC0, XV_POOL },
	{ 0x080D4CC0, 0x080D4CD8, XV_RAW },
	{ 0x080D4D2C, 0x080D4D30, XV_POOL },
	{ 0x0801077A, 0x08010798, XV_ZERO },
	{ 0x08010798, 0x0801079C, XV_POOL },
};
static const XvPair walla_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007BE4, 0x0800A704 },   /* project:research, SAME */
	{ 0x08007C20, 0x0800A754 },   /* project:research, SAME */
	{ 0x080090B8, 0x0800BF16 },   /* project:research, SAME */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:spikey, DIFF (audited) */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ABCA, 0x0810A952 },   /* project:families-2, DIFF (audited) */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08012FB0, 0x0801A180 },   /* project:after-state, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080164B4, 0x0810A952 },   /* project:families-2, DIFF (audited) */
	{ 0x080190D0, 0x08021AA4 },   /* project:billy, SAME */
	{ 0x080BE634, 0x0810A952 },   /* project:billy, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ 0x080E407A, 0x0810A952 },   /* project:families-2, DIFF (audited) */
};
static const XvImm walla_imms[] = {
	{ 0x080BE98E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BE9E4, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BEA82, THUMB_MOVS, 4, 0, 0x7, 0x96 },   /* its effect: 0x7 in BN4, 0x96 in BN6 (moved) */
	{ 0x080BEB5E, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080D4B2C, THUMB_MOVS, 2, 0, 0x27, 0xC3 },   /* its object's sprite: list 4 0x27 in BN4, list 4 0xc3 in BN6 (copied) */
	{ 0x080D4C02, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D4C34, THUMB_MOVS, 4, 0, 0xF, 0x97 },   /* its effect: 0xf in BN4, 0x97 in BN6 (moved) */
	{ 0x080D4D0A, THUMB_MOVS, 0, 0, 0x3C, 0xE2 },   /* its object's type 0x3c: moved, the dispatch's 0xe2 */
	{ 0x080D4D1A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvNumber walla_numbers[] = {
	{ 0x080D4D2C, 0x0A800000, 0x0A800000 },   /* the flags its hits left it, tested (BN4_COLL_FROM_COLLISION): BN6's bits at FlagsFromCollision (+0x70), where the thunder's update tests the same mask, 0xFF800000, in both games */
};
static const XvPair walla_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvType walla_types[] = {
	{ 3, 0xE2, XV_TYPE_MOVED, 0x3C },   /* BN4's third-kind 0x3c, moved, the dispatch's 0xe2 */
};
static const XvRef walla_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080BE935), OWN(0x080BEAB9),
};
static const XvSprite walla_sprites[] = {
	{ 1, 0x12, 1, 0xC3, XV_ART + 0x6FB00, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x27, 4, 0xC3, XV_ART + 0x71B00, 0x800 },
	{ 3, 0x12, 4, 0xC4, XV_ART + 0x72300, 0x1300 },
	{ 5, 0x01, 5, 0x9A, XV_ART + 0x73600, 0x1800 },
};
static const XvId walla_effects[] = { { 0x07, 0x96 }, { 0x0F, 0x97 } };

static const XVirusFamily bn4_walla = {
	.name = "Walla (BN4's)",
	.game = XROM_BN4_RED_SUN_US,
	.src_ai = 0x25,   /* (its first-kind type: BN4 has no AI index) */
	.ai = 67,
	.first_id = XV_FIRST_ID + 0xD2,
	.home = XV_HOMES + 0x11800,
	.blocks = walla_blocks, .nblocks = (int)(sizeof walla_blocks / sizeof *walla_blocks),
	.data = walla_data, .ndata = (int)(sizeof walla_data / sizeof *walla_data),
	.calls = walla_calls, .ncalls = (int)(sizeof walla_calls / sizeof *walla_calls),
	.words = walla_words, .nwords = (int)(sizeof walla_words / sizeof *walla_words),
	.imms = walla_imms, .nimms = (int)(sizeof walla_imms / sizeof *walla_imms),
	.numbers = walla_numbers, .nnumbers = (int)(sizeof walla_numbers / sizeof *walla_numbers),
	.types = walla_types, .ntypes = (int)(sizeof walla_types / sizeof *walla_types),
	.states = walla_states, .nstates = (int)(sizeof walla_states / sizeof *walla_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08011D5C, .struct2 = 0x08011D5C,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 1, .flags = 0x00 },
	.sprites = walla_sprites, .nsprites = (int)(sizeof walla_sprites / sizeof *walla_sprites),
	.effects = walla_effects, .neffects = (int)(sizeof walla_effects / sizeof *walla_effects),
	.present = 0x08010736,   /* its after-state, copied: run after its state 0 and its hook 5 while BN6's battle is paused */
};
/* ---- BN4's Elmperor (its type 0x2e there, its enemy ids 109-114), as AI 68 with ids 0x2d8-0x2dd ---- */
/* interim: its sounds 0x9d and 0xfc (080DC700, 080C2A7E) kept as BN6's same numbers, other songs: BN4's sounds have no slots among BN6's empty songs yet (src/audio/xsong.c's allocations), where a copy of each would go */
/* interim: its panel types 8 and 0xA (its attack's, 0x080DC780: 6, 7, 8, 4, 0xA, set by BN6's object_setPanelType) and the panel flags 0x1000 and 0x200 it reads its element by (0x080C2948: 0x400, 0x800, 0x1000, 0x100, 0x200) kept as BN4's numbers: 8 is BN6's volcano (an eruption every 140 frames) by its number and its word (0x11010), which is BN5's lava's too, and which of them BN4's 8 is stays unread; 0xA is BN4's holding panel, none in BN6 (bn4_twins.PANEL4TO6), where BN6's 0xA is a conveyor and its flag 0x200 the conveyors' (0x10210): the attack's fifth panel pushes in BN6, and Elmperor standing on a conveyor reads that element. 6, 7 and 4 (grass, ice, poison) and their flags 0x400, 0x800 and 0x100 are BN6's own numbers and words (BN4_PANEL_FLAGS_OF against BN6_PANEL_FLAGS_OF), decided */
/* interim: a panel its attack changes keeps its new type: BN4 gives it 0xBE frames (+0x12) to change back, which BN6's panels count for its conveyors alone */
/* interim: its aura (the fourth kind's 0x4C, its sprite drawn again in its element's colours through BN4's tile buffer 0x02037810) not drawn: its spawn answered 0 (bn4_twins.AUDITS); its HP's return goes on */

static const XvBlock elmperor_blocks[] = {
	{ 0x080C2790, 0x080C2A9C, XV_CODE + 0x16940, 0x1AD67A9Bu },
	{ 0x080DC678, 0x080DC7AC, XV_CODE + 0x16C50, 0x10B8694Cu },
	{ 0x08010736, 0x0801079C, XV_CODE + 0x16D92, 0x7631C49Cu },
};
static const XvData elmperor_data[] = {
	{ 0x080C2790, 0x080C27A2, XV_ZERO },
	{ 0x080C27A2, 0x080C27A4, XV_RAW },
	{ 0x080C27A4, 0x080C280E, XV_ZERO },
	{ 0x080C2824, 0x080C2828, XV_POOL },
	{ 0x080C2828, 0x080C2850, XV_ZERO },
	{ 0x080C2892, 0x080C2894, XV_RAW },
	{ 0x080C2894, 0x080C2898, XV_POOL },
	{ 0x080C2898, 0x080C289E, XV_RAW },
	{ 0x080C28B0, 0x080C28B4, XV_POOL },
	{ 0x080C28B4, 0x080C28B8, XV_TABLE },
	{ 0x080C28C6, 0x080C28C8, XV_RAW },
	{ 0x080C28C8, 0x080C28CC, XV_POOL },
	{ 0x080C28CC, 0x080C28D4, XV_TABLE },
	{ 0x080C2942, 0x080C2944, XV_RAW },
	{ 0x080C2944, 0x080C2948, XV_POOL },
	{ 0x080C2948, 0x080C2952, XV_RAW },
	{ 0x080C298A, 0x080C298C, XV_RAW },
	{ 0x080C298C, 0x080C2990, XV_POOL },
	{ 0x080C2990, 0x080C2998, XV_TABLE },
	{ 0x080C2A24, 0x080C2A2C, XV_POOL },
	{ 0x080C2A2C, 0x080C2A38, XV_RAW },
	{ 0x080C2A46, 0x080C2A48, XV_RAW },
	{ 0x080C2A48, 0x080C2A4C, XV_POOL },
	{ 0x080C2A4C, 0x080C2A50, XV_TABLE },
	{ 0x080C2A90, 0x080C2A94, XV_POOL },
	{ 0x080C2A94, 0x080C2A9C, XV_RAW },
	{ 0x080DC68A, 0x080DC68C, XV_RAW },
	{ 0x080DC68C, 0x080DC690, XV_POOL },
	{ 0x080DC690, 0x080DC69C, XV_TABLE },
	{ 0x080DC744, 0x080DC748, XV_POOL },
	{ 0x080DC748, 0x080DC74C, XV_TABLE },
	{ 0x080DC77A, 0x080DC77C, XV_RAW },
	{ 0x080DC77C, 0x080DC780, XV_POOL },
	{ 0x080DC780, 0x080DC78A, XV_RAW },
	{ 0x080DC7A8, 0x080DC7AC, XV_POOL },
	{ 0x0801077A, 0x08010798, XV_ZERO },
	{ 0x08010798, 0x0801079C, XV_POOL },
};
static const XvPair elmperor_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009DA0, 0x0800CBD8 },   /* project:families-2, DIFF (audited) */
	{ 0x08009DC4, 0x0800CC0A },   /* project:families-2, DIFF (audited) */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800ABBA, 0x0800E2EC },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x0801014C, 0x080165C2 },   /* project:families-2, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08012FB0, 0x0801A180 },   /* project:after-state, LAYOUT */
	{ 0x08013C48, 0x08148864 },   /* its pre-step's way on: a bx r0 of BN6's, r0 BN6's state step, AIMED (audited) */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080B6568, 0x0800EBD4 },   /* project:billy, DIFF (audited) */
	{ 0x080E76EA, 0x080011EC },   /* project:families-2, DIFF (audited) */
	{ 0x080E770C, 0x0810A952 },   /* project:families-2, DIFF (audited) */
};
static const XvImm elmperor_imms[] = {
	{ 0x080C2872, THUMB_STRB, 0, 3, 0x6, 0x8 },   /* collision data +0x6 in BN4, +0x8 in BN6 */
	{ 0x080C28DC, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080C28FE, THUMB_CMP, 1, 0, 0x8, 0xA },   /* another of its family's (its NameID range's) CurAction 8 compared */
	{ 0x080C2908, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080C2916, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080C2960, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080C2972, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080C2A1A, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080C2A86, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080DC6AA, THUMB_MOVS, 2, 0, 0x38, 0xC5 },   /* its object's sprite: list 4 0x38 in BN4, list 4 0xc5 in BN6 (copied) */
	{ 0x080DC71E, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DC78E, THUMB_MOVS, 0, 0, 0x79, 0xE3 },   /* its object's type 0x79: moved, the dispatch's 0xe3 */
	{ 0x080DC79E, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
};
static const XvRange elmperor_ranges[] = {
	{ 0x080C28F0, 0x2D8, 0x2DD },   /* BN4's ids 0x6d-0x72, its own */
};
static const XvNumber elmperor_numbers[] = {
	{ 0x080C2824, 0x080C2828, 0x0810A349 },   /* its pre-step's way on: BN6's state step, not its state table */
};
static const XvPair elmperor_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvType elmperor_types[] = {
	{ 3, 0xE3, XV_TYPE_MOVED, 0x79 },   /* BN4's third-kind 0x79, moved, the dispatch's 0xe3 */
};
static const XvRef elmperor_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x080C2851), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080C289F), OWN(0x080C2953), OWN(0x080C297D), OWN(0x080C2A39),
};
static const XvSprite elmperor_sprites[] = {
	{ 1, 0x13, 1, 0xC4, XV_ART + 0x74E00, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x38, 4, 0xC5, XV_ART + 0x76E00, 0x1800 },
};

static const XVirusFamily bn4_elmperor = {
	.name = "Elmperor (BN4's)",
	.game = XROM_BN4_RED_SUN_US,
	.src_ai = 0x2E,   /* (its first-kind type: BN4 has no AI index) */
	.ai = 68,
	.first_id = XV_FIRST_ID + 0xD8,
	.home = XV_HOMES + 0x12000,
	.blocks = elmperor_blocks, .nblocks = (int)(sizeof elmperor_blocks / sizeof *elmperor_blocks),
	.data = elmperor_data, .ndata = (int)(sizeof elmperor_data / sizeof *elmperor_data),
	.calls = elmperor_calls, .ncalls = (int)(sizeof elmperor_calls / sizeof *elmperor_calls),
	.words = elmperor_words, .nwords = (int)(sizeof elmperor_words / sizeof *elmperor_words),
	.imms = elmperor_imms, .nimms = (int)(sizeof elmperor_imms / sizeof *elmperor_imms),
	.ranges = elmperor_ranges, .nranges = (int)(sizeof elmperor_ranges / sizeof *elmperor_ranges),
	.numbers = elmperor_numbers, .nnumbers = (int)(sizeof elmperor_numbers / sizeof *elmperor_numbers),
	.types = elmperor_types, .ntypes = (int)(sizeof elmperor_types / sizeof *elmperor_types),
	.states = elmperor_states, .nstates = (int)(sizeof elmperor_states / sizeof *elmperor_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08011DA4, .struct2 = 0x08011DA4,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 1, .flags = 0x04 },
	.sprites = elmperor_sprites, .nsprites = (int)(sizeof elmperor_sprites / sizeof *elmperor_sprites),
	.prestep = 0x080C280E,   /* its update's own work on, run as its pre-step (bn4_twins.PRESTEP) */
	.present = 0x08010736,   /* its after-state, copied: run after its state 0 and its hook 5 while BN6's battle is paused */
};
/* ---- BN4's CirKill (its type 0x20 there, its enemy ids 91-96), as AI 69 with ids 0x2de-0x2e3 ---- */
/* interim: its sounds 0xf9 (080BD430) kept as BN6's same numbers, other songs: BN4's sounds have no slots among BN6's empty songs yet (src/audio/xsong.c's allocations), where a copy of each would go */

static const XvBlock cirkill_blocks[] = {
	{ 0x080BD094, 0x080BD4B0, XV_CODE + 0x16E00, 0xB6638170u },
	{ 0x080CE6E0, 0x080CE840, XV_CODE + 0x17220, 0x5AA41525u },
	{ 0x080CE5B0, 0x080CE6E0, XV_CODE + 0x17380, 0x69329294u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x174B2, 0x793106FEu },
	{ 0x08010736, 0x0801079C, XV_CODE + 0x178C2, 0x7631C49Cu },
};
static const XvData cirkill_data[] = {
	{ 0x080BD094, 0x080BD0A6, XV_ZERO },
	{ 0x080BD0A6, 0x080BD0A8, XV_RAW },
	{ 0x080BD0A8, 0x080BD112, XV_ZERO },
	{ 0x080BD136, 0x080BD138, XV_RAW },
	{ 0x080BD138, 0x080BD13C, XV_POOL },
	{ 0x080BD13C, 0x080BD15C, XV_ZERO },
	{ 0x080BD172, 0x080BD174, XV_RAW },
	{ 0x080BD174, 0x080BD178, XV_POOL },
	{ 0x080BD178, 0x080BD17C, XV_TABLE },
	{ 0x080BD18A, 0x080BD18C, XV_RAW },
	{ 0x080BD18C, 0x080BD190, XV_POOL },
	{ 0x080BD190, 0x080BD194, XV_TABLE },
	{ 0x080BD20E, 0x080BD210, XV_RAW },
	{ 0x080BD210, 0x080BD214, XV_POOL },
	{ 0x080BD214, 0x080BD224, XV_RAW },
	{ 0x080BD224, 0x080BD230, XV_POOL },
	{ 0x080BD242, 0x080BD244, XV_RAW },
	{ 0x080BD244, 0x080BD248, XV_POOL },
	{ 0x080BD248, 0x080BD250, XV_TABLE },
	{ 0x080BD2B0, 0x080BD2B4, XV_POOL },
	{ 0x080BD2B4, 0x080BD2CC, XV_RAW },
	{ 0x080BD2CC, 0x080BD2D0, XV_POOL },
	{ 0x080BD2D0, 0x080BD2E8, XV_RAW },
	{ 0x080BD2E8, 0x080BD2EC, XV_POOL },
	{ 0x080BD2EC, 0x080BD2F2, XV_RAW },
	{ 0x080BD37E, 0x080BD380, XV_RAW },
	{ 0x080BD380, 0x080BD384, XV_POOL },
	{ 0x080BD392, 0x080BD394, XV_RAW },
	{ 0x080BD394, 0x080BD398, XV_POOL },
	{ 0x080BD398, 0x080BD3A4, XV_TABLE },
	{ 0x080BD43E, 0x080BD440, XV_RAW },
	{ 0x080BD440, 0x080BD448, XV_POOL },
	{ 0x080BD448, 0x080BD44E, XV_RAW },
	{ 0x080BD4AA, 0x080BD4AC, XV_RAW },
	{ 0x080BD4AC, 0x080BD4B0, XV_ZERO },
	{ 0x080CE6F2, 0x080CE6F4, XV_RAW },
	{ 0x080CE6F4, 0x080CE6F8, XV_POOL },
	{ 0x080CE6F8, 0x080CE704, XV_TABLE },
	{ 0x080CE828, 0x080CE83C, XV_RAW },
	{ 0x080CE83C, 0x080CE840, XV_POOL },
	{ 0x080CE5B0, 0x080CE6E0, XV_RAW },
	{ 0x0800AD54, 0x0800AF90, XV_ZERO },
	{ 0x0800AFBC, 0x0800B128, XV_ZERO },
	{ 0x0800B128, 0x0800B12C, XV_POOL },
	{ 0x0801077A, 0x08010798, XV_ZERO },
	{ 0x08010798, 0x0801079C, XV_POOL },
};
static const XvPair cirkill_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800285C, 0x080026C4 },   /* project:after-state, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:thunder, DIFF (audited) */
	{ 0x0800A192, 0x0800D06A },   /* project:research, SAME */
	{ 0x0800AB3E, 0x0800E276 },   /* project:research, SAME */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AE10, 0x0800E6E8 },   /* project:families-2, SAME */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:thunder, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DB2, 0x0801A066 },   /* project:families-2, LAYOUT */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:thunder, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08012FB0, 0x0801A180 },   /* project:after-state, LAYOUT */
	{ 0x08013C48, 0x08148864 },   /* its pre-step's way on: a bx r0 of BN6's, r0 BN6's state step, AIMED (audited) */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080B6568, 0x0800EBD4 },   /* project:billy, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm cirkill_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa, none in BN6: compared with no type of BN6's */
	{ 0x0800AFB8, THUMB_STRB, 0, 1, 0xA, 0xC },   /* collision data +0xa in BN4, +0xc in BN6 */
	{ 0x080BD1FE, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BD2A6, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BD374, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BD422, THUMB_MOVS, 4, 0, 0x46, 0x98 },   /* its effect: 0x46 in BN4, 0x98 in BN6 (moved) */
	{ 0x080CE74A, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE79A, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE7D6, THUMB_MOVS, 4, 0, 0x56, 0x99 },   /* its effect: 0x56 in BN4, 0x99 in BN6 (moved) */
	{ 0x080CE812, THUMB_MOVS, 0, 0, 0xB, 0xE4 },   /* its object's type 0xb: moved, the dispatch's 0xe4 */
};
static const XvRange cirkill_ranges[] = {
	{ 0x080BD480, 0x2DE, 0x2E3 },   /* BN4's ids 0x5b-0x60, its own */
};
static const XvNumber cirkill_numbers[] = {
	{ 0x080BD138, 0x080BD13C, 0x0810A349 },   /* its pre-step's way on: BN6's state step, not its state table */
	{ 0x080BD228, 0x0B800000, 0x0B800000 },   /* a panel's flags it may not have (object_checkPanelParameters, BN6's 0x0800CC86 the same body): the support and alliance bits, BN6's own code naming the same mask (0x0800E6D4) */
	{ 0x080BD22C, 0x04080080, 0x04080080 },   /* a panel's flags (its panel's, 0x08009A86, BN6's 0x0800C8F8) CirKill waits on: 0x04000000 the enemy's alliance, 0x00080000 and 0x80 as bn6f's PanelData flags have them (an occupant's, a panel struck) */
};
static const XvPair cirkill_words[] = {
	{ 0x08010561, 0x080D0B95 },   /* project:spikey, SAME */
};
static const XvByte cirkill_bytes[] = {
	{ 0x080CE656, 0x0D, 0xC6 },   /* its table's kind 10: list 4 0xd in BN4, 0xc6 copied */
	{ 0x080CE666, 0x0D, 0xC6 },   /* its table's kind 11: list 4 0xd in BN4, 0xc6 copied */
	{ 0x080CE676, 0x0D, 0xC6 },   /* its table's kind 12: list 4 0xd in BN4, 0xc6 copied */
};
static const XvType cirkill_types[] = {
	{ 3, 0xE4, XV_TYPE_MOVED, 0x0B },   /* BN4's third-kind 0xb, moved, the dispatch's 0xe4 */
};
static const XvRef cirkill_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080BD15D), OWN(0x080BD231),
};
static const XvSprite cirkill_sprites[] = {
	{ 1, 0x10, 1, 0xC5, XV_ART + 0x78600, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x0D, 4, 0xC6, XV_ART + 0x7A600, 0x800 },
	{ 4, 0x25, 4, 0xC7, XV_ART + 0x7AE00, 0x400 },
	{ 4, 0x15, 4, 0xC8, XV_ART + 0x7B200, 0x800 },
};
static const XvId cirkill_effects[] = { { 0x46, 0x98 }, { 0x56, 0x99 } };

static const XVirusFamily bn4_cirkill = {
	.name = "CirKill (BN4's)",
	.game = XROM_BN4_RED_SUN_US,
	.src_ai = 0x20,   /* (its first-kind type: BN4 has no AI index) */
	.ai = 69,
	.first_id = XV_FIRST_ID + 0xDE,
	.home = XV_HOMES + 0x12800,
	.blocks = cirkill_blocks, .nblocks = (int)(sizeof cirkill_blocks / sizeof *cirkill_blocks),
	.data = cirkill_data, .ndata = (int)(sizeof cirkill_data / sizeof *cirkill_data),
	.calls = cirkill_calls, .ncalls = (int)(sizeof cirkill_calls / sizeof *cirkill_calls),
	.words = cirkill_words, .nwords = (int)(sizeof cirkill_words / sizeof *cirkill_words),
	.imms = cirkill_imms, .nimms = (int)(sizeof cirkill_imms / sizeof *cirkill_imms),
	.ranges = cirkill_ranges, .nranges = (int)(sizeof cirkill_ranges / sizeof *cirkill_ranges),
	.numbers = cirkill_numbers, .nnumbers = (int)(sizeof cirkill_numbers / sizeof *cirkill_numbers),
	.bytes = cirkill_bytes, .nbytes = (int)(sizeof cirkill_bytes / sizeof *cirkill_bytes),
	.types = cirkill_types, .ntypes = (int)(sizeof cirkill_types / sizeof *cirkill_types),
	.states = cirkill_states, .nstates = (int)(sizeof cirkill_states / sizeof *cirkill_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08011CCC, .struct2 = 0x08011CCC,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 1, .flags = 0x08 },
	.sprites = cirkill_sprites, .nsprites = (int)(sizeof cirkill_sprites / sizeof *cirkill_sprites),
	.effects = cirkill_effects, .neffects = (int)(sizeof cirkill_effects / sizeof *cirkill_effects),
	.prestep = 0x080BD112,   /* its update's own work on, run as its pre-step (bn4_twins.PRESTEP) */
	.present = 0x08010736,   /* its after-state, copied: run after its state 0 and its hook 5 while BN6's battle is paused */
};
const XVirusFamily *const xvirus_bn4[] = { &bn4_billy, &bn4_spikey, &bn4_shrimpy, &bn4_walla, &bn4_elmperor, &bn4_cirkill };
const int xvirus_bn4_count = (int)(sizeof xvirus_bn4 / sizeof *xvirus_bn4);

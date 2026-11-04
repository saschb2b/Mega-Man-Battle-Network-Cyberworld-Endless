/* xvirus_bn4.c: BN4 Red Sun's virus families' recipes, written by tools/xrecipe_bn4.py from BN4's and BN6's ROMs
 * (python3 tools/xrecipe_bn4.py; --check says whether this is what it writes). Do not edit: change the generator (or
 * tools/bn4_twins.py, its pairs and their audits) and write it again. Numbers alone (docs/multirom/families.md): addresses
 * in BN4 Red Sun (USA) and BN6 Cybeast Gregar (USA), counts and CRC32s. The families here are the generator's whole ones
 * (no decision open; its interims named over each), installed only where BN4's ROM is read for research (xrom_research:
 * --dev bn4families); what stays open is in its report (.build/xrecipe/bn4_report.md). */
#include "xvirus_bn4.h"

#include "bn4.h"
#include "bn6.h"
#include "rom.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }

/* ---- BN4's Billy (its type 0x13 there, its enemy ids 13-18), as AI 64 with ids 0x2c0-0x2c5 ---- */

static const XvBlock billy_blocks[] = {
	{ 0x080B94E4, 0x080B999C, XV_CODE + 0x14000, 0xBBE06B34u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x14602, 0x793106FEu },
	{ 0x080B52E4, 0x080B52EC, XV_CODE + 0x14C00, 0xF6607FB0u },
	{ 0x080B54D0, 0x080B54F4, XV_CODE + 0x14E00, 0xB6A58CE9u },
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
};
static const XvPair billy_calls[] = {
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
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
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080190D0, 0x08021AA4 },   /* project:billy, SAME */
	{ 0x08044AEC, 0x080011EC },   /* project:billy, DIFF (audited) */
	{ 0x080B3878, 0x0800EC48 },   /* project:research, SAME */
	{ 0x080B6568, 0x0800EBD4 },   /* project:billy, DIFF (audited) */
	{ 0x080BE634, 0x0810A952 },   /* project:billy, DIFF (audited) */
	{ 0x080D2156, 0x080CADBC },   /* project:billy, DIFF (audited) */
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
	{ 0x080B97FE, THUMB_MOVS, 4, 0, 0x16, 0x98 },   /* its effect: 0x16 in BN4, 0x98 in BN6 */
	{ 0x080B9816, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B984A, THUMB_MOVS, 4, 0, 0x15, 0x99 },   /* its effect: 0x15 in BN4, 0x99 in BN6 */
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
	{ 5, 0x0D, 5, 0x98, XV_ART + 0x63000, 0x2400 },
};
static const XvId billy_effects[] = { { 0x16, 0x98 }, { 0x15, 0x99 } };

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
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 2, .stats_flags = 0x08 },
	.sprites = billy_sprites, .nsprites = (int)(sizeof billy_sprites / sizeof *billy_sprites),
	.effects = billy_effects, .neffects = (int)(sizeof billy_effects / sizeof *billy_effects),
};
const XVirusFamily *const xvirus_bn4[] = { &bn4_billy };
const int xvirus_bn4_count = (int)(sizeof xvirus_bn4 / sizeof *xvirus_bn4);

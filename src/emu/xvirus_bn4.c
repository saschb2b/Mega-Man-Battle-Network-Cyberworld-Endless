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
/* interim: 0800AD42: its panel type 0xa compared, which no BN6 type is known to be: compared with 0xFF, no type of BN6's (the case never holds; bn4_twins.PANEL4TO6 to give it) */
/* interim: its effects 0x15 and 0x16 (080B9800, 080B984C) kept as BN6's same ids: BN4's effect table is not mapped yet, so their twins are unchecked (effect_twin) */

static const XvBlock billy_blocks[] = {
	{ 0x080B94E4, 0x080B999C, XV_CODE + 0x14000, 0xBBE06B34u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x144C2, 0x793106FEu },
	{ 0x080B52E4, 0x080B52EC, XV_CODE + 0x148D0, 0xF6607FB0u },
	{ 0x080B54D0, 0x080B54F4, XV_CODE + 0x148E0, 0xB6A58CE9u },
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
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa */
	{ 0x080B9600, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B9650, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080B967A, THUMB_CMP, 2, 0, 0x9, 0xB },   /* another of its family's (its NameID range's) CurAction 9 compared */
	{ 0x080B968E, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080B9730, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080B97A6, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080B9816, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
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
};

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
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 2 },
	.sprites = billy_sprites, .nsprites = (int)(sizeof billy_sprites / sizeof *billy_sprites),
};
/* ---- BN4's Walla (its type 0x25 there, its enemy ids 103-108), as AI 67 with ids 0x2d2-0x2d7 ---- */
/* interim: its effects 0x7 and 0xf (080BEA84, 080D4C36) kept as BN6's same ids: BN4's effect table is not mapped yet, so their twins are unchecked (effect_twin) */

static const XvBlock walla_blocks[] = {
	{ 0x080BE888, 0x080BEB68, XV_CODE + 0x15880, 0xD185211Eu },
	{ 0x080D4B04, 0x080D4D30, XV_CODE + 0x15B60, 0xDA54AE1Cu },
	{ 0x080B52E4, 0x080B52EC, XV_CODE + 0x15D90, 0xF6607FB0u },
	{ 0x080B54D0, 0x080B54F4, XV_CODE + 0x15DA0, 0xB6A58CE9u },
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
};
static const XvPair walla_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08007948, 0x0800A098 },   /* project:research, SAME */
	{ 0x08007A12, 0x0800A18E },   /* project:research, SAME */
	{ 0x08007BE4, 0x0800A704 },   /* project:research, SAME */
	{ 0x08007C20, 0x0800A754 },   /* project:research, SAME */
	{ 0x080090B8, 0x0800BF16 },   /* project:research, SAME */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x0800A17A, 0x0800D048 },   /* project:families-2, DIFF (audited) */
	{ 0x0800AB64, 0x0800E29C },   /* project:research, SAME */
	{ 0x0800AB74, 0x0800E2AC },   /* project:research, SAME */
	{ 0x0800AB88, 0x0800E2C0 },   /* project:research, SAME */
	{ 0x0800ABCA, 0x0810A952 },   /* project:families-2, DIFF (audited) */
	{ 0x0800ACA6, 0x0800E456 },   /* project:research, DIFF (audited) */
	{ 0x0800AFBC, 0x0800E9DC },   /* project:billy, DIFF (audited) */
	{ 0x08012496, 0x08019892 },   /* project:research, DIFF (audited) */
	{ 0x080129AC, 0x08019FB4 },   /* project:research, DIFF (audited) */
	{ 0x080129FC, 0x0801A00E },   /* project:families-2, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:families-2, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:families-2, DIFF (audited) */
	{ 0x080164B4, 0x0810A952 },   /* project:families-2, DIFF (audited) */
	{ 0x080190D0, 0x08021AA4 },   /* project:billy, SAME */
	{ 0x080BE634, 0x0810A952 },   /* project:billy, DIFF (audited) */
	{ 0x080CDCE4, 0x0801156A },   /* project:families-2, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ 0x080E407A, 0x0810A952 },   /* project:families-2, DIFF (audited) */
	{ BN4_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
};
static const XvImm walla_imms[] = {
	{ 0x080BE98E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BE9E4, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BEB5E, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080D4B2C, THUMB_MOVS, 2, 0, 0x27, 0xC0 },   /* its object's sprite, list 4's 0x27 in BN4, 0xc0 copied */
	{ 0x080D4C02, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D4D0A, THUMB_MOVS, 0, 0, 0x3C, 0xD0 },   /* its object 3:0x3c, moved, run as the dispatch's 0xd0 */
	{ 0x080D4D1A, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the element of the object it spawned: BN4 +0x17, BN6 +0x0E */
};
static const XvRef walla_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080BE935), OWN(0x080BEAB9),
};
static const XvPair walla_words[] = {
	{ 0x08010561, 0x08016C8B },   /* project:families-2, DIFF */
};
static const XvNumber walla_numbers[] = {
	{ 0x080D4D2C, 0x0A800000, 0x0A800000 },   /* a collision mask's bits (the alliance and support bits BN5's KEPT reads in BN6 the same, e.g. BN6's 0x080C7386), tested against what struck it (BN4's collision +0x54, BN6's +0x70) */
};
static const XvType walla_types[] = {
	{ 3, 0x3C, XV_TYPE_MOVED, 0xD0 },   /* Walla's ring (its spawner 0x080D4D06): BN6's 0x3C (0x080CCF69) another object, its init a panel's strike with effect 0x22, no motion; BN4's flies at a speed of its own (+0x40, +0x44) */
};
static const XvSprite walla_sprites[] = {
	{ 1, 0x12, 1, 0xC3, XV_ART + 0x68000, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x27, 4, 0xC0, XV_ART + 0x6A000, 0x1000 },
};

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
	.states = walla_states, .nstates = (int)(sizeof walla_states / sizeof *walla_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08011D5C, .struct2 = 0x08011D5C,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 1 },
	.sprites = walla_sprites, .nsprites = (int)(sizeof walla_sprites / sizeof *walla_sprites),
	.types = walla_types, .ntypes = (int)(sizeof walla_types / sizeof *walla_types),
	.numbers = walla_numbers, .nnumbers = (int)(sizeof walla_numbers / sizeof *walla_numbers),
};
/* ---- BN4's Elmperor (its type 0x2e there, its enemy ids 109-114), as AI 68 with ids 0x2d8-0x2dd ---- */
/* interim: its panel types (its attack's, 0x080DC780: 6, 7, 8, 4, 0xA, set by BN6's object_setPanelType) and the panel flags it reads its element by (0x080C2948: 0x400, 0x800, 0x1000, 0x100, 0x200) kept as BN4's numbers: BN4's panel types are not mapped yet (the same interim as Billy's panel type 0xA; BN6's flag 0x200 is its conveyors') */
/* interim: a panel its attack changes keeps its new type: BN4 gives it 0xBE frames (+0x12) to change back, which BN6's panels count for its conveyors alone */
/* interim: its aura (the fourth kind's 0x4C, its sprite drawn again in its element's colours through BN4's tile buffer 0x02037810) not drawn: its spawn answered 0 (bn4_twins.AUDITS); its HP's return goes on */

static const XvBlock elmperor_blocks[] = {
	{ 0x080C2790, 0x080C2A9C, XV_CODE + 0x15DD0, 0x1AD67A9Bu },
	{ 0x080DC678, 0x080DC7AC, XV_CODE + 0x160E0, 0x10B8694Cu },
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
};
static const XvPair elmperor_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
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
	{ 0x080129FC, 0x0801A00E },   /* project:families-2, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:families-2, LAYOUT */
	{ 0x08013C48, 0x08148864 },   /* its pre-step's way on: a bx r0 of BN6's, r0 BN6's state step, AIMED (audited) */
	{ 0x080143FC, 0x0801BBAC },   /* project:families-2, DIFF (audited) */
	{ 0x080B6568, 0x0800EBD4 },   /* project:billy, DIFF (audited) */
	{ 0x080E76EA, 0x080011EC },   /* project:families-2, DIFF (audited) */
	{ 0x080E770C, 0x0810A952 },   /* project:families-2, DIFF (audited) */
	{ BN4_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
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
	{ 0x080DC6AA, THUMB_MOVS, 2, 0, 0x38, 0xC1 },   /* its object's sprite, list 4's 0x38 in BN4, 0xc1 copied */
	{ 0x080DC71E, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080DC78E, THUMB_MOVS, 0, 0, 0x79, 0xD1 },   /* its object 3:0x79, moved, run as the dispatch's 0xd1 */
	{ 0x080DC79E, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the element of the object it spawned: BN4 +0x17, BN6 +0x0E */
};
static const XvRange elmperor_ranges[] = {
	{ 0x080C28F0, 0x2D8, 0x2DD },   /* BN4's ids 0x6d-0x72, its own */
};
static const XvRef elmperor_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x080C2851), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080C289F), OWN(0x080C2953), OWN(0x080C297D), OWN(0x080C2A39),
};
static const XvPair elmperor_words[] = {
	{ 0x08010561, 0x08016C8B },   /* project:families-2, DIFF */
};
static const XvNumber elmperor_numbers[] = {
	{ 0x080C2824, 0x080C2828, 0x0810A349 },   /* its pre-step's way on: BN6's state step, not its state table */
};
static const XvType elmperor_types[] = {
	{ 3, 0x79, XV_TYPE_MOVED, 0xD1 },   /* Elmperor's attack (its spawner 0x080DC78A), the panel it strikes changed to its element's type: BN6's 0x79 (0x080D5CB1) another object, nearest none */
};
static const XvSprite elmperor_sprites[] = {
	{ 1, 0x13, 1, 0xC4, XV_ART + 0x6B000, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x38, 4, 0xC1, XV_ART + 0x6D000, 0x2000 },
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
	.states = elmperor_states, .nstates = (int)(sizeof elmperor_states / sizeof *elmperor_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08011DA4, .struct2 = 0x08011DA4,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 1 },
	.sprites = elmperor_sprites, .nsprites = (int)(sizeof elmperor_sprites / sizeof *elmperor_sprites),
	.types = elmperor_types, .ntypes = (int)(sizeof elmperor_types / sizeof *elmperor_types),
	.numbers = elmperor_numbers, .nnumbers = (int)(sizeof elmperor_numbers / sizeof *elmperor_numbers),
	.prestep = 0x080C280E,   /* its update's own work on, run as its pre-step (bn4_twins.PRESTEP) */
};
/* ---- BN4's CirKill (its type 0x20 there, its enemy ids 91-96), as AI 69 with ids 0x2de-0x2e3 ---- */
/* interim: 0800AD42: its panel type 0xa compared, which no BN6 type is known to be: compared with 0xFF, no type of BN6's (the case never holds; bn4_twins.PANEL4TO6 to give it) */
/* interim: its effects 0x46 and 0x56 (080BD424, 080CE7D8) kept as BN6's same ids: BN4's effect table is not mapped yet, so their twins are unchecked (effect_twin) */

static const XvBlock cirkill_blocks[] = {
	{ 0x080BD094, 0x080BD4B0, XV_CODE + 0x16220, 0xB6638170u },
	{ 0x080CE6E0, 0x080CE840, XV_CODE + 0x16640, 0x5AA41525u },
	{ 0x080CE5B0, 0x080CE6E0, XV_CODE + 0x167A0, 0x69329294u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x168D2, 0x793106FEu },
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
};
static const XvPair cirkill_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
	{ 0x08009A86, 0x0800C8F8 },   /* project:research, LAYOUT */
	{ 0x08009ABC, 0x0800C90A },   /* project:billy, DIFF (audited) */
	{ 0x08009E8C, 0x0800CC66 },   /* project:research, SAME */
	{ 0x08009EAC, 0x0800CC86 },   /* project:research, SAME */
	{ 0x08009EE4, 0x0800CCBE },   /* project:families-2, DIFF (audited) */
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
	{ 0x080129FC, 0x0801A00E },   /* project:families-2, DIFF (audited) */
	{ 0x08012D62, 0x0801A018 },   /* project:research, DIFF (audited) */
	{ 0x08012D9A, 0x0801A04C },   /* project:research, DIFF (audited) */
	{ 0x08012DB2, 0x0801A066 },   /* project:families-2, LAYOUT */
	{ 0x08012DC0, 0x0801A074 },   /* project:research, SAME */
	{ 0x08012F44, 0x0801A0D4 },   /* project:research, DIFF (audited) */
	{ 0x08012F74, 0x0801A140 },   /* project:families-2, LAYOUT */
	{ 0x08012F82, 0x0801A152 },   /* project:research, LAYOUT */
	{ 0x08012F8C, 0x0801A15C },   /* project:research, LAYOUT */
	{ 0x08012F96, 0x0801A166 },   /* project:research, LAYOUT */
	{ 0x08013C48, 0x08148864 },   /* its pre-step's way on: a bx r0 of BN6's, r0 BN6's state step, AIMED (audited) */
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:families-2, DIFF (audited) */
	{ 0x080B6568, 0x0800EBD4 },   /* project:billy, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
	{ BN4_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
};
static const XvImm cirkill_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa */
	{ 0x0800AFB8, THUMB_STRB, 0, 1, 0xA, 0xC },   /* collision data +0xa in BN4, +0xc in BN6 */
	{ 0x080BD1FE, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BD2A6, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BD374, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080CE74A, THUMB_STRB, 0, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
	{ 0x080CE79A, THUMB_LDR, 1, 0, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080CE812, THUMB_MOVS, 0, 0, 0xB, 0xD2 },   /* its object 3:0xb, moved, run as the dispatch's 0xd2 */
};
static const XvRange cirkill_ranges[] = {
	{ 0x080BD480, 0x2DE, 0x2E3 },   /* BN4's ids 0x5b-0x60, its own */
};
static const XvRef cirkill_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x080BD15D), OWN(0x080BD231),
};
static const XvPair cirkill_words[] = {
	{ 0x08010561, 0x08016C8B },   /* project:families-2, DIFF */
};
static const XvNumber cirkill_numbers[] = {
	{ 0x080BD138, 0x080BD13C, 0x0810A349 },   /* its pre-step's way on: BN6's state step, not its state table */
	{ 0x080BD228, 0x0B800000, 0x0B800000 },   /* a panel's flags it may not have (object_checkPanelParameters, BN6's 0x0800CC86 SAME): the support and alliance bits, BN6's own code naming the same mask (0x0800E6D4) */
	{ 0x080BD22C, 0x04080080, 0x04080080 },   /* a panel's flags (its panel's, 0x08009A86, BN6's 0x0800C8F8) CirKill waits on: 0x04000000 the enemy's alliance, 0x00080000 and 0x80 as BN6's panel flags have them (an occupant's, a panel struck) */
};
static const XvByte cirkill_bytes[] = {
	{ 0x080CE656, 0x0D, 0xC2 },   /* its table's kind 10: list 4's 0xd in BN4, 0xc2 copied */
	{ 0x080CE666, 0x0D, 0xC2 },   /* its table's kind 11: list 4's 0xd in BN4, 0xc2 copied */
	{ 0x080CE676, 0x0D, 0xC2 },   /* its table's kind 12: list 4's 0xd in BN4, 0xc2 copied */
};
static const XvType cirkill_types[] = {
	{ 3, 0x0B, XV_TYPE_MOVED, 0xD2 },   /* CirKill's attack (its spawner 0x080CE80E): BN6's 0x0B (0x080C7919) another object; BN5's 0x0B, moved by BN5's track, a kin of it (0.89 by body), not the same */
};
static const XvSprite cirkill_sprites[] = {
	{ 1, 0x10, 1, 0xC5, XV_ART + 0x6F000, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x0D, 4, 0xC2, XV_ART + 0x71000, 0x1000 },
};

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
	.states = cirkill_states, .nstates = (int)(sizeof cirkill_states / sizeof *cirkill_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08011CCC, .struct2 = 0x08011CCC,   /* (its first version's enemy record: BN4 has no struct1 or struct2) */
	.names = { 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 1 },
	.sprites = cirkill_sprites, .nsprites = (int)(sizeof cirkill_sprites / sizeof *cirkill_sprites),
	.types = cirkill_types, .ntypes = (int)(sizeof cirkill_types / sizeof *cirkill_types),
	.numbers = cirkill_numbers, .nnumbers = (int)(sizeof cirkill_numbers / sizeof *cirkill_numbers),
	.bytes = cirkill_bytes, .nbytes = (int)(sizeof cirkill_bytes / sizeof *cirkill_bytes),
	.prestep = 0x080BD112,   /* its update's own work on, run as its pre-step (bn4_twins.PRESTEP) */
};
const XVirusFamily *const xvirus_bn4[] = { &bn4_billy, &bn4_walla, &bn4_elmperor, &bn4_cirkill };
const int xvirus_bn4_count = (int)(sizeof xvirus_bn4 / sizeof *xvirus_bn4);

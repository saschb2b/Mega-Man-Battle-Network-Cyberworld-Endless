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
#define ENTERED(a) { (a), true, 0, true }

/* ---- BN4's Billy (its type 0x13 there, its enemy ids 13-18), as AI 64 with ids 0x2c0-0x2c5 ---- */
/* interim: 0800AD42: its panel type 0xa compared, which no BN6 type is known to be: compared with 0xFF, no type of BN6's (the case never holds; bn4_twins.PANEL4TO6 to give it) */
/* interim: its effects 0x15 and 0x16 (080B984C, 080B9800) kept as BN6's same ids: BN4's effect table is not mapped yet, so their twins are unchecked (effect_twin) */

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
	.traits = { .laid = true, .mode = 0x00, .element = 0, .weakness = 0, .shadow = true, .palette = 2, .flags = 0x08 },
	.sprites = billy_sprites, .nsprites = (int)(sizeof billy_sprites / sizeof *billy_sprites),
};
/* ---- BN4's Spikey (its type 0x17 there, its enemy ids 25-30), as AI 65 with ids 0x2c6-0x2cb ---- */
/* interim: 0800AD42: its panel type 0xa compared, which no BN6 type is known to be: compared with 0xFF, no type of BN6's (the case never holds; bn4_twins.PANEL4TO6 to give it) */
/* interim: its effects 0x13 and 0x14 and 0x2a (080BA6C0, 080BA68C, 080D2FCE, 080D2EFE) kept as BN6's same ids: BN4's effect table is not mapped yet, so their twins are unchecked (effect_twin) */
/* interim: its sounds 0xbc and 0xc4 (080D2F26, 080D2E86) kept as BN6's same numbers, other songs: BN4's sounds have no slots among BN6's empty songs yet (src/audio/xsong.c's allocations), where a copy of each would go */

static const XvBlock spikey_blocks[] = {
	{ 0x080BA404, 0x080BA7F4, XV_CODE + 0x15000, 0x7BA719BAu },
	{ 0x080D2DE8, 0x080D2FDC, XV_CODE + 0x15400, 0x6EDE0007u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x15602, 0x793106FEu },
	{ 0x080B552A, 0x080B5594, XV_CODE + 0x15C02, 0x29597AFDu },
	{ 0x080B56A4, 0x080B56E4, XV_CODE + 0x15E00, 0x67EEC233u },
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
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x08002FE2, 0x08002F90 },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
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
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080B3878, 0x0800EC48 },   /* project:research, SAME */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm spikey_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa */
	{ 0x080BA53A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080BA5A4, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080BA62E, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA69E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BA744, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA7DA, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080D2E18, THUMB_MOVS, 2, 0, 0x13, 0xC0 },   /* its object's sprite: list 4 0x13 in BN4, list 4 0xc0 in BN6 (copied) */
	{ 0x080D2ECA, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D2F74, THUMB_MOVS, 0, 0, 0x30, 0xE0 },   /* its object's type 0x30: moved, the dispatch's 0xe0 */
	{ 0x080D2F84, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D2FA6, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
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
	{ 1, 0x05, 1, 0xC1, XV_ART + 0x63000, 0x3000 },   /* (LZ77 in BN4) */
	{ 4, 0x13, 4, 0xC0, XV_ART + 0x66000, 0x800 },
};

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
};
/* ---- BN4's Shrimpy (its type 0x16 there, its enemy ids 31-36), as AI 66 with ids 0x2cc-0x2d1 ---- */
/* interim: 0800AD42: its panel type 0xa compared, which no BN6 type is known to be: compared with 0xFF, no type of BN6's (the case never holds; bn4_twins.PANEL4TO6 to give it) */
/* interim: its effects 0x25 (080D2DB8, 080D2C6C) kept as BN6's same ids: BN4's effect table is not mapped yet, so their twins are unchecked (effect_twin) */
/* interim: its sounds 0x9f and 0xc9 and 0xca (080D2C54, 080D2BEC, 080D2D26) kept as BN6's same numbers, other songs: BN4's sounds have no slots among BN6's empty songs yet (src/audio/xsong.c's allocations), where a copy of each would go */

static const XvBlock shrimpy_blocks[] = {
	{ 0x080BA0AC, 0x080BA404, XV_CODE + 0x16000, 0x9599E33Fu },
	{ 0x080D2B28, 0x080D2DE8, XV_CODE + 0x16400, 0xAAC60F82u },
	{ 0x0800AD2A, 0x0800B12C, XV_CODE + 0x16802, 0x793106FEu },
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
};
static const XvPair shrimpy_calls[] = {
	{ 0x08000560, 0x080005CC },   /* project:research, SAME */
	{ 0x08001B16, 0x08001532 },   /* project:research, SAME */
	{ 0x0800283C, 0x080026A4 },   /* project:research, SAME */
	{ 0x0800287C, 0x080026E4 },   /* project:research, SAME */
	{ 0x08002DEC, 0x08002D80 },   /* project:research, SAME */
	{ 0x08002E10, 0x08002DA4 },   /* project:research, SAME */
	{ 0x08002E56, 0x08002DEA },   /* project:research, SAME */
	{ 0x08002E96, 0x08002E3C },   /* project:research, SAME */
	{ 0x08002FAE, 0x08002F5C },   /* project:research, SAME */
	{ 0x080032F0, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080033D4, 0x08003458 },   /* project:research, SAME */
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
	{ 0x080143A8, 0x0801BB1C },   /* project:billy, DIFF (audited) */
	{ 0x080143CA, 0x0801BB46 },   /* project:research, LAYOUT */
	{ 0x080143FC, 0x0801BBAC },   /* project:thunder, DIFF (audited) */
	{ 0x080BA12E, 0x0810A952 },   /* its update: BN6's dispatcher runs its state and present, its own step is hook 3 */
	{ 0x080CD7E2, 0x080C6BDA },   /* project:research, DIFF (audited) */
	{ 0x080E18EE, 0x080E1932 },   /* project:research, SAME */
};
static const XvImm shrimpy_imms[] = {
	{ 0x0800AD40, THUMB_LDRB, 1, 0, 0x0, 0x2 },   /* a panel's type: BN4 +0, BN6 +2 */
	{ 0x0800AD42, THUMB_CMP, 1, 0, 0xA, 0xFF },   /* BN4's panel type 0xa */
	{ 0x080BA24C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BA25A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 written */
	{ 0x080BA29A, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 written */
	{ 0x080BA2AE, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA318, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 written */
	{ 0x080BA386, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080BA3F8, THUMB_MOVS, 0, 0, 0x6, 0x8 },   /* CurAction 6 written */
	{ 0x080D2B6E, THUMB_MOVS, 2, 0, 0xB, 0xC1 },   /* its object's sprite: list 4 0xb in BN4, list 4 0xc1 in BN6 (copied) */
	{ 0x080D2C32, THUMB_LDR, 0, 7, 0x54, 0x70 },   /* collision data +0x54 in BN4, +0x70 in BN6 */
	{ 0x080D2D5C, THUMB_MOVS, 0, 0, 0x2F, 0xE1 },   /* its object's type 0x2f: moved, the dispatch's 0xe1 */
	{ 0x080D2D6C, THUMB_STRB, 3, 0, 0x17, 0xE },   /* the object it spawned: element: BN4 +0x17, BN6 +0x0E */
	{ 0x080D2D90, THUMB_LDRB, 2, 5, 0x17, 0xE },   /* its element: BN4 +0x17, BN6 +0x0E */
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
	{ 1, 0x06, 1, 0xC2, XV_ART + 0x66800, 0x2000 },   /* (LZ77 in BN4) */
	{ 4, 0x0B, 4, 0xC1, XV_ART + 0x68800, 0x1000 },
};

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
};
const XVirusFamily *const xvirus_bn4[] = { &bn4_billy, &bn4_spikey, &bn4_shrimpy };
const int xvirus_bn4_count = (int)(sizeof xvirus_bn4 / sizeof *xvirus_bn4);

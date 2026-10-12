/* xvirus_bn5.c: BN5's virus families' recipes, written by tools/xrecipe.py from the two ROMs (python3 tools/xrecipe.py;
 * --check says whether this is what it writes). Do not edit: change the generator (or tools/bn5_twins.py, its pairs and
 * their audits) and write it again. Numbers alone (docs/MULTIROM.md, Transplanted families): addresses in BN5 Team
 * Colonel (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from the player's
 * ROM; nothing of either game's. The families here are the generator's, whole (no decision open), in install order
 * (each its AI index, ids, home, code and art places by its slot): the ones installed wherever BN5's ROM is read
 * (src/emu/xvirus.c), and tests/test_core.c holds the generated CanGard and Handi to the recipes made by hand
 * (src/emu/xvirus_families.c), field for field. */
#include "xvirus_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"
#include "xnavi_recipe.h"

#define OWN(a) { (a), true, 0 }
#define BN6(a) { (a), false, 0 }
#define CLONE(k, a) { (a), true, (k) }

/* ---- BN5's CanGard (AI 14 there, its enemy ids 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54), as AI 32 with ids 0x200-0x205 ---- */

static const XvBlock cangard_blocks[] = {
	{ 0x08108E14, 0x08109140, XV_CODE + 0x0, 0x018F4F22u },
	{ 0x080C9518, 0x080C9784, XV_CODE + 0x400, 0xD7ABC4C3u },
};
static const XvData cangard_data[] = {
	{ 0x08108E22, 0x08108E24, XV_RAW },
	{ 0x08108E24, 0x08108E28, XV_POOL },
	{ 0x08108E28, 0x08108E34, XV_TABLE },
	{ 0x08108EB2, 0x08108EB4, XV_RAW },
	{ 0x08108EB4, 0x08108EB8, XV_POOL },
	{ 0x08108EB8, 0x08108ED0, XV_TABLE },
	{ 0x08109000, 0x08109028, XV_ZERO },
	{ 0x0810909A, 0x0810909C, XV_RAW },
	{ 0x0810909C, 0x081090A0, XV_POOL },
	{ 0x081090A0, 0x081090AC, XV_TABLE },
	{ 0x0810911C, 0x08109128, XV_POOL },
	{ 0x08109128, 0x0810912E, XV_RAW },
	{ 0x0810912E, 0x08109134, XV_RAW },
	{ 0x08109134, 0x0810913C, XV_RAW },
	{ 0x0810913C, 0x08109140, XV_POOL },
	{ 0x080C952A, 0x080C952C, XV_RAW },
	{ 0x080C952C, 0x080C9530, XV_POOL },
	{ 0x080C9530, 0x080C953C, XV_TABLE },
	{ 0x080C9604, 0x080C960C, XV_POOL },
	{ 0x080C960C, 0x080C9624, XV_RAW },
	{ 0x080C9624, 0x080C962A, XV_RAW },
	{ 0x080C9670, 0x080C9674, XV_POOL },
	{ 0x080C9674, 0x080C9680, XV_TABLE },
};
static const XvPair cangard_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x080133CE, 0x080165C2 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
};
static const XvImm cangard_imms[] = {
	{ 0x080C955A, THUMB_MOVS, 2, 0, 0x24, 0x5C },   /* its sprite: list 4 0x24 in BN5, 0x5c in BN6 */
	{ 0x080C9636, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x08108FB6, THUMB_ADDS, 4, 0, 0x57, 0x6C },   /* its effect: 0x57 in BN5, 0x6c in BN6 */
	{ 0x081090C6, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x08109110, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvRef cangard_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x08109055), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08109087), OWN(0x08108E15), OWN(0x08108EA5), OWN(0x08108FD9),
};
static const XvSprite cangard_sprites[] = {
	{ 1, 0x0E, 1, 0x1E, XV_ART + 0x0, 0x3000 },   /* (LZ77 in BN5) */
	{ 4, 0x24, 4, 0x5C, XV_ART + 0x3000, 0x800 },
	{ 4, 0x3D, 4, 0x5D, XV_ART + 0x3800, 0xC00 },
};
static const XvId cangard_effects[] = { { 0x57, 0x6C } };
static const XvType cangard_types[] = { { 3, 0x25, XV_TYPE_MOVED } };

static const XVirusFamily bn5_cangard = {
	.name = "CanGard (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 14,
	.ai = 32,
	.first_id = XV_FIRST_ID + 0x0,
	.home = XV_HOMES + 0x0,
	.blocks = cangard_blocks, .nblocks = (int)(sizeof cangard_blocks / sizeof *cangard_blocks),
	.data = cangard_data, .ndata = (int)(sizeof cangard_data / sizeof *cangard_data),
	.calls = cangard_calls, .ncalls = (int)(sizeof cangard_calls / sizeof *cangard_calls),
	.imms = cangard_imms, .nimms = (int)(sizeof cangard_imms / sizeof *cangard_imms),
	.states = cangard_states, .nstates = (int)(sizeof cangard_states / sizeof *cangard_states),
	.hooks = { OWN(0x08109029), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), OWN(0x08109071), OWN(0x08109071), OWN(0x08109071), BN6(BN6_FAMILY_NULLSUB), OWN(0x08109071) },
	.struct1 = 0x08108D28, .struct2 = 0x08108D30,
	.names = { 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x0E, .element = 0, .weakness = 0, .shadow = true },
	.sprites = cangard_sprites, .nsprites = (int)(sizeof cangard_sprites / sizeof *cangard_sprites),
	.effects = cangard_effects, .neffects = (int)(sizeof cangard_effects / sizeof *cangard_effects),
	.types = cangard_types, .ntypes = (int)(sizeof cangard_types / sizeof *cangard_types),
};
/* ---- BN5's Handi (AI 4 there, its enemy ids 0x13, 0x14, 0x15, 0x16, 0x17, 0x18), as AI 33 with ids 0x206-0x20b ---- */

static const XvBlock handi_blocks[] = {
	{ 0x081061D4, 0x081063E0, XV_CODE + 0x800, 0x4D6C3897u },
};
static const XvData handi_data[] = {
	{ 0x081061E2, 0x081061E4, XV_RAW },
	{ 0x081061E4, 0x081061E8, XV_POOL },
	{ 0x081061E8, 0x08106200, XV_TABLE },
	{ 0x08106288, 0x081062AC, XV_ZERO },
	{ 0x081062C0, 0x081062C4, XV_POOL },
	{ 0x081062C4, 0x081062CC, XV_TABLE },
	{ 0x08106304, 0x08106308, XV_POOL },
	{ 0x08106308, 0x08106310, XV_RAW },
	{ 0x08106384, 0x0810638C, XV_POOL },
	{ 0x0810638C, 0x08106392, XV_RAW },
	{ 0x08106392, 0x08106398, XV_RAW },
	{ 0x081063C6, 0x081063C8, XV_RAW },
	{ 0x081063C8, 0x081063CC, XV_POOL },
	{ 0x081063CC, 0x081063DC, XV_RAW },
	{ 0x081063DC, 0x081063E0, XV_POOL },
};
static const XvPair handi_calls[] = {
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08104E60, 0x0810AD80 },   /* project:handi, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080CF37E, 0x080CF4EC },   /* project:handi, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08012DCC, 0x08015FDC },   /* project:handi, SAME */
};
static const XvPair handi_words[] = {
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
};
static const XvImm handi_imms[] = {
	{ 0x081062E8, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x08106362, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvRange handi_ranges[] = {
	{ 0x081063A0, 0x206, 0x20B },   /* NameIDs 0x13-0x18 in BN5, 0x206-0x20b in BN6 */
};
static const XvRef handi_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x081062AD), BN6(0x0810AE95), OWN(0x081061D5),
};
static const XvSprite handi_sprites[] = {
	{ 1, 0x04, 1, 0x00, XV_ART + 0x4800, 0x3000 },   /* (LZ77 in BN5) */
};

static const XVirusFamily bn5_handi = {
	.name = "Handi (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 4,
	.ai = 33,
	.first_id = XV_FIRST_ID + 0x6,
	.home = XV_HOMES + 0x800,
	.blocks = handi_blocks, .nblocks = (int)(sizeof handi_blocks / sizeof *handi_blocks),
	.data = handi_data, .ndata = (int)(sizeof handi_data / sizeof *handi_data),
	.calls = handi_calls, .ncalls = (int)(sizeof handi_calls / sizeof *handi_calls),
	.words = handi_words, .nwords = (int)(sizeof handi_words / sizeof *handi_words),
	.imms = handi_imms, .nimms = (int)(sizeof handi_imms / sizeof *handi_imms),
	.ranges = handi_ranges, .nranges = (int)(sizeof handi_ranges / sizeof *handi_ranges),
	.states = handi_states, .nstates = (int)(sizeof handi_states / sizeof *handi_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x081060E8, .struct2 = 0x081060F0,
	.names = { 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x0B, .element = 0, .weakness = 0, .shadow = true },
	.sprites = handi_sprites, .nsprites = (int)(sizeof handi_sprites / sizeof *handi_sprites),
};
/* ---- BN5's Powie (AI 2 there, its enemy ids 0x7, 0x8, 0x9, 0xa, 0xb, 0xc), as AI 34 with ids 0x20c-0x211 ---- */

static const XvBlock powie_blocks[] = {
	{ 0x08105790, 0x08105CFC, XV_CODE + 0xC00, 0xAA5A4CE0u },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0x1202, 0xA53A3BEFu },
	{ 0x08104B96, 0x08104BBC, XV_CODE + 0x1402, 0x8F659214u },
};
static const XvData powie_data[] = {
	{ 0x0810579E, 0x081057A0, XV_RAW },
	{ 0x081057A0, 0x081057A4, XV_POOL },
	{ 0x081057A4, 0x081057B4, XV_TABLE },
	{ 0x081058E4, 0x081058E8, XV_POOL },
	{ 0x081058E8, 0x081058FC, XV_TABLE },
	{ 0x08105AFA, 0x08105AFC, XV_RAW },
	{ 0x08105AFC, 0x08105B04, XV_POOL },
	{ 0x08105B04, 0x08105B2C, XV_ZERO },
	{ 0x08105B6A, 0x08105B6C, XV_RAW },
	{ 0x08105B6C, 0x08105B70, XV_POOL },
	{ 0x08105B70, 0x08105B78, XV_TABLE },
	{ 0x08105C06, 0x08105C08, XV_RAW },
	{ 0x08105C08, 0x08105C14, XV_POOL },
	{ 0x08105C14, 0x08105C1A, XV_RAW },
	{ 0x08105C1A, 0x08105C20, XV_RAW },
	{ 0x08105C20, 0x08105C26, XV_RAW },
	{ 0x08105C52, 0x08105C54, XV_RAW },
	{ 0x08105C54, 0x08105C5C, XV_POOL },
	{ 0x08105C5C, 0x08105C74, XV_RAW },
	{ 0x08105C74, 0x08105C7A, XV_RAW },
	{ 0x08105CAA, 0x08105CAC, XV_RAW },
	{ 0x08105CAC, 0x08105CB0, XV_POOL },
	{ 0x08105CD8, 0x08105CDC, XV_POOL },
	{ 0x08105CDC, 0x08105CEC, XV_RAW },
	{ 0x08105CEC, 0x08105CFC, XV_POOL },
};
static const XvPair powie_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08018838, 0x0801BD3C },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800AFF0, 0x0800C938 },   /* map:calls, DIFF (audited) */
	{ 0x080133C4, 0x080165B8 },   /* project:research, DIFF (audited) */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
	{ 0x08012A6C, 0x08015C94 },   /* map:bytes, SAME */
};
static const XvPair powie_words[] = {
	{ 0x08012A6D, 0x08015C95 },
};
static const XvImm powie_imms[] = {
	{ 0x08105920, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810596A, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x08105A42, THUMB_MOVS, 0, 0, 0xC0, 0x62 },   /* its sound 0xc0: BN5's copied into BN6's empty slot 0x62 */
	{ 0x08105A80, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x08105AA6, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x08105B48, THUMB_LDR, 0, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x08105BB8, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x08105BFA, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x08105C46, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber powie_numbers[] = {
	{ 0x08105B00, 0x0405FF00, 0x0405FF00 },
	{ 0x08105CEC, 0x08000000, 0x08000000 },
};
static const XvRange powie_ranges[] = {
	{ 0x08105B8A, 0x20C, 0x211 },   /* NameIDs 0x7-0xc in BN5, 0x20c-0x211 in BN6 */
};
static const XvSong powie_songs[] = {
	{ 0xC0, 0x62, XV_ART + 0xAC00, 0xF00 },   /* its sound 0xc0, BN5's song copied */
};
static const XvRef powie_states[] = {
	BN6(0x08016381), OWN(0x08105B2D), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08105B57), BN6(0x0810AE95), OWN(0x08105791), OWN(0x081058D7),
};
static const XvSprite powie_sprites[] = {
	{ 1, 0x02, 1, 0x20, XV_ART + 0x7800, 0x2000 },
	{ 5, 0x0D, 5, 0x20, XV_ART + 0x9800, 0x1400 },
};
static const XvId powie_effects[] = { { 0x14, 0x6D }, { 0x15, 0x6E } };
static const uint16_t powie_held[XV_VERSIONS * 16] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34D3, 0x34D3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34D3, 0x34D3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34D3, 0x34D3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34D3, 0x34D3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

static const XVirusFamily bn5_powie = {
	.name = "Powie (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 2,
	.ai = 34,
	.first_id = XV_FIRST_ID + 0xC,
	.home = XV_HOMES + 0x1000,
	.blocks = powie_blocks, .nblocks = (int)(sizeof powie_blocks / sizeof *powie_blocks),
	.data = powie_data, .ndata = (int)(sizeof powie_data / sizeof *powie_data),
	.calls = powie_calls, .ncalls = (int)(sizeof powie_calls / sizeof *powie_calls),
	.words = powie_words, .nwords = (int)(sizeof powie_words / sizeof *powie_words),
	.imms = powie_imms, .nimms = (int)(sizeof powie_imms / sizeof *powie_imms),
	.numbers = powie_numbers, .nnumbers = (int)(sizeof powie_numbers / sizeof *powie_numbers),
	.ranges = powie_ranges, .nranges = (int)(sizeof powie_ranges / sizeof *powie_ranges),
	.songs = powie_songs, .nsongs = (int)(sizeof powie_songs / sizeof *powie_songs),
	.states = powie_states, .nstates = (int)(sizeof powie_states / sizeof *powie_states),
	.hooks = { OWN(0x08105B45), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x081056A4, .struct2 = 0x081056AC,
	.names = { 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C },
	.held_chips = powie_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x02, .element = 0, .weakness = 0, .shadow = true },
	.sprites = powie_sprites, .nsprites = (int)(sizeof powie_sprites / sizeof *powie_sprites),
	.effects = powie_effects, .neffects = (int)(sizeof powie_effects / sizeof *powie_effects),
};
/* ---- BN5's Bugtank (AI 3 there, its enemy ids 0xd, 0xe, 0xf, 0x10, 0x11, 0x12), as AI 35 with ids 0x212-0x217 ---- */

static const XvBlock bugtank_blocks[] = {
	{ 0x08105DE8, 0x081060E8, XV_CODE + 0x1600, 0x7B208735u },
	{ 0x080C928C, 0x080C92F2, XV_CODE + 0x1A00, 0x7533EA15u },
	{ 0x080C9420, 0x080C943C, XV_CODE + 0x1C00, 0xD58CA8F2u },
};
static const XvData bugtank_data[] = {
	{ 0x08105E98, 0x08105E9C, XV_POOL },
	{ 0x08105E9C, 0x08105EA4, XV_RAW },
	{ 0x08105EA4, 0x08105ECC, XV_ZERO },
	{ 0x08105EE0, 0x08105EE4, XV_POOL },
	{ 0x08105EE4, 0x08105EF4, XV_TABLE },
	{ 0x08105F92, 0x08105F94, XV_RAW },
	{ 0x08105F94, 0x08105F98, XV_POOL },
	{ 0x08105F98, 0x08105FB0, XV_RAW },
	{ 0x08105FFC, 0x08106004, XV_POOL },
	{ 0x08106004, 0x0810600A, XV_RAW },
	{ 0x0810600A, 0x08106010, XV_RAW },
	{ 0x0810602E, 0x08106030, XV_RAW },
	{ 0x08106030, 0x08106034, XV_POOL },
	{ 0x08106034, 0x0810604C, XV_RAW },
	{ 0x08106090, 0x08106094, XV_POOL },
	{ 0x081060D0, 0x081060D4, XV_POOL },
	{ 0x081060D4, 0x081060E4, XV_RAW },
	{ 0x081060E4, 0x081060E8, XV_POOL },
	{ 0x080C929E, 0x080C92A0, XV_RAW },
	{ 0x080C92A0, 0x080C92A4, XV_POOL },
	{ 0x080C92A4, 0x080C92B0, XV_TABLE },
};
static const XvPair bugtank_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x080C92F2, 0x080CA102 },   /* map:bytes, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
};
static const XvPair bugtank_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080C92F3, 0x080CA103 },
};
static const XvImm bugtank_imms[] = {
	{ 0x080C92B2, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its sprite's list: 3 in BN5, 4 in BN6 (its own not grown) */
	{ 0x080C92B4, THUMB_MOVS, 2, 0, 0x2, 0x5E },   /* its sprite: list 3 0x2 in BN5, 0x5e in BN6 */
	{ 0x08105F12, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x08105F4E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x08105F72, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x08105FEC, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810601E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810605E, THUMB_CMP, 4, 0, 0x6F, 0x9E },   /* a held chip compared: BN5's chip 111, BN6's 158 (the chip registry) */
	{ 0x08106062, THUMB_CMP, 4, 0, 0x6E, 0x9D },   /* a held chip compared: BN5's chip 110, BN6's 157 (the chip registry) */
};
static const XvRef bugtank_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08105ECD), BN6(0x0810AE95), BN6(0x0810AB35), OWN(0x08105DE9),
};
static const XvSprite bugtank_sprites[] = {
	{ 1, 0x03, 1, 0x21, XV_ART + 0xC000, 0x3000 },   /* (LZ77 in BN5) */
	{ 3, 0x02, 4, 0x5E, XV_ART + 0xF000, 0x2000 },
};
static const XvType bugtank_types[] = { { 3, 0x24, XV_TYPE_MOVED } };
static const uint16_t bugtank_held[XV_VERSIONS * 16] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0x34A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0x34A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x349D, 0x349D, 0x349E, 0x349E, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x349D, 0x349D, 0x349E, 0x349E, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x349E, 0x349E, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

static const XVirusFamily bn5_bugtank = {
	.name = "Bugtank (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 3,
	.ai = 35,
	.first_id = XV_FIRST_ID + 0x12,
	.home = XV_HOMES + 0x1800,
	.blocks = bugtank_blocks, .nblocks = (int)(sizeof bugtank_blocks / sizeof *bugtank_blocks),
	.data = bugtank_data, .ndata = (int)(sizeof bugtank_data / sizeof *bugtank_data),
	.calls = bugtank_calls, .ncalls = (int)(sizeof bugtank_calls / sizeof *bugtank_calls),
	.words = bugtank_words, .nwords = (int)(sizeof bugtank_words / sizeof *bugtank_words),
	.imms = bugtank_imms, .nimms = (int)(sizeof bugtank_imms / sizeof *bugtank_imms),
	.states = bugtank_states, .nstates = (int)(sizeof bugtank_states / sizeof *bugtank_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08105CFC, .struct2 = 0x08105D04,
	.names = { 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12 },
	.held_chips = bugtank_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x03, .element = 0, .weakness = 0, .shadow = true },
	.sprites = bugtank_sprites, .nsprites = (int)(sizeof bugtank_sprites / sizeof *bugtank_sprites),
	.types = bugtank_types, .ntypes = (int)(sizeof bugtank_types / sizeof *bugtank_types),
};
/* ---- BN5's Dominerd (AI 5 there, its enemy ids 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e), as AI 36 with ids 0x218-0x21d ---- */

static const XvBlock dominerd_blocks[] = {
	{ 0x081064CC, 0x08106A08, XV_CODE + 0x1E00, 0xFFA4BD54u },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0x2402, 0xA53A3BEFu },
	{ 0x08104B96, 0x08104BBC, XV_CODE + 0x2602, 0x8F659214u },
};
static const XvData dominerd_data[] = {
	{ 0x081064DA, 0x081064DC, XV_RAW },
	{ 0x081064DC, 0x081064E0, XV_POOL },
	{ 0x081064E0, 0x081064F4, XV_TABLE },
	{ 0x081066E4, 0x08106710, XV_ZERO },
	{ 0x08106772, 0x08106774, XV_RAW },
	{ 0x08106774, 0x08106778, XV_POOL },
	{ 0x08106778, 0x08106788, XV_TABLE },
	{ 0x0810682C, 0x0810683C, XV_POOL },
	{ 0x0810683C, 0x08106854, XV_RAW },
	{ 0x08106854, 0x0810685A, XV_RAW },
	{ 0x0810685A, 0x08106860, XV_RAW },
	{ 0x08106860, 0x08106866, XV_RAW },
	{ 0x08106916, 0x08106918, XV_RAW },
	{ 0x08106918, 0x08106924, XV_POOL },
	{ 0x08106924, 0x0810692A, XV_RAW },
	{ 0x0810692A, 0x08106930, XV_RAW },
	{ 0x08106930, 0x08106936, XV_RAW },
	{ 0x081069E8, 0x081069EC, XV_POOL },
	{ 0x081069EC, 0x081069FC, XV_RAW },
	{ 0x081069FC, 0x08106A08, XV_POOL },
};
static const XvPair dominerd_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x0801A576, 0x0801DC36 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x08013824, 0x08016B02 },   /* project:research, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x0801A4C4, 0x0801DB84 },   /* map:bytes, SAME */
	{ 0x0800CE54, 0x0800EBD4 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800C6CE, 0x0800E2CE },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08012A6C, 0x08015C94 },   /* map:bytes, SAME */
};
static const XvPair dominerd_words[] = {
	{ 0x08104BFD, 0x0810AE95 },
};
static const XvImm dominerd_imms[] = {
	{ 0x0810652A, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x08106566, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810662C, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810672A, THUMB_LDR, 0, 4, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x081067E6, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x08106824, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x08106894, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x081068F8, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber dominerd_numbers[] = {
	{ 0x081069FC, 0x08000000, 0x08000000 },
};
static const XvRange dominerd_ranges[] = {
	{ 0x081068C4, 0x218, 0x21D },   /* NameIDs 0x19-0x1e in BN5, 0x218-0x21d in BN6 */
	{ 0x08106940, 0x000, 0x217 },   /* NameIDs 0x0-0x18 in BN5, 0x0-0x217 in BN6 */
	{ 0x08106950, 0x21E, 0x3FF },   /* NameIDs 0x1f-0x190 in BN5, 0x21e-0x3ff in BN6 */
	{ 0x081067F8, 0x218, 0x21D },   /* NameIDs 0x19-0x1e in BN5, 0x218-0x21d in BN6 */
};
static const XvClone dominerd_clones[] = {
	{ 0x08106710, 0x08106726, XV_CODE + 0x2800, 0x08013824, 0x08016B36, { 0 } },   /* its paralysis wrapper for BN6's state 6 */
	{ 0x08106710, 0x08106726, XV_CODE + 0x2A00, 0x08013824, 0x08016B72, { 0 } },   /* its paralysis wrapper for BN6's state 7 */
};
static const XvRef dominerd_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), OWN(0x08106711), BN6(0x08016CE9), CLONE(1, 0x08106711), CLONE(2, 0x08106711), OWN(0x0810675F), BN6(0x0810AE95), BN6(0x0810AB35), OWN(0x081064CD), OWN(0x08106699),
};
static const XvSprite dominerd_sprites[] = {
	{ 1, 0x05, 1, 0x22, XV_ART + 0x11000, 0x2000 },
};

static const XVirusFamily bn5_dominerd = {
	.name = "Dominerd (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 5,
	.ai = 36,
	.first_id = XV_FIRST_ID + 0x18,
	.home = XV_HOMES + 0x2000,
	.blocks = dominerd_blocks, .nblocks = (int)(sizeof dominerd_blocks / sizeof *dominerd_blocks),
	.data = dominerd_data, .ndata = (int)(sizeof dominerd_data / sizeof *dominerd_data),
	.calls = dominerd_calls, .ncalls = (int)(sizeof dominerd_calls / sizeof *dominerd_calls),
	.words = dominerd_words, .nwords = (int)(sizeof dominerd_words / sizeof *dominerd_words),
	.imms = dominerd_imms, .nimms = (int)(sizeof dominerd_imms / sizeof *dominerd_imms),
	.numbers = dominerd_numbers, .nnumbers = (int)(sizeof dominerd_numbers / sizeof *dominerd_numbers),
	.ranges = dominerd_ranges, .nranges = (int)(sizeof dominerd_ranges / sizeof *dominerd_ranges),
	.clones = dominerd_clones, .nclones = (int)(sizeof dominerd_clones / sizeof *dominerd_clones),
	.states = dominerd_states, .nstates = (int)(sizeof dominerd_states / sizeof *dominerd_states),
	.hooks = { OWN(0x08106727), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x081063E0, .struct2 = 0x081063E8,
	.names = { 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E },
	.prestep = 0x08104588,   /* BN5's dispatcher's case for its ids, ported (bn5_twins.PRESTEP) */
	.ice = 0,
	.traits = { .laid = true, .mode = 0x0B, .element = 0, .weakness = 0, .shadow = true },
	.sprites = dominerd_sprites, .nsprites = (int)(sizeof dominerd_sprites / sizeof *dominerd_sprites),
};
/* ---- BN5's MetFire (AI 6 there, its enemy ids 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24), as AI 37 with ids 0x21e-0x223 ---- */

static const XvBlock metfire_blocks[] = {
	{ 0x08106AF4, 0x08106DD4, XV_CODE + 0x2C00, 0x336DDF02u },
	{ 0x080D0C0C, 0x080D0D20, XV_CODE + 0x3000, 0xCF20A9B0u },
	{ 0x080D0E2C, 0x080D0E4A, XV_CODE + 0x3200, 0x293B728Bu },
	{ 0x080EA08C, 0x080EA0BA, XV_CODE + 0x3400, 0x23FC3BE1u },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0x3602, 0x21492011u },
};
static const XvData metfire_data[] = {
	{ 0x08106B02, 0x08106B04, XV_RAW },
	{ 0x08106B04, 0x08106B08, XV_POOL },
	{ 0x08106B08, 0x08106B18, XV_TABLE },
	{ 0x08106BC0, 0x08106BC4, XV_POOL },
	{ 0x08106BC4, 0x08106BD4, XV_RAW },
	{ 0x08106BEE, 0x08106BF0, XV_RAW },
	{ 0x08106BF0, 0x08106C18, XV_ZERO },
	{ 0x08106C2C, 0x08106C30, XV_POOL },
	{ 0x08106C30, 0x08106C34, XV_TABLE },
	{ 0x08106CB2, 0x08106CB4, XV_RAW },
	{ 0x08106CB4, 0x08106CC0, XV_POOL },
	{ 0x08106CC0, 0x08106CC6, XV_RAW },
	{ 0x08106CC6, 0x08106CCC, XV_RAW },
	{ 0x08106CCC, 0x08106CD2, XV_RAW },
	{ 0x08106D02, 0x08106D04, XV_RAW },
	{ 0x08106D04, 0x08106D08, XV_POOL },
	{ 0x08106DB4, 0x08106DBC, XV_POOL },
	{ 0x08106DBC, 0x08106DCC, XV_RAW },
	{ 0x08106DCC, 0x08106DD4, XV_POOL },
	{ 0x080D0C0C, 0x080D0C30, XV_RAW },
	{ 0x080D0C3E, 0x080D0C40, XV_RAW },
	{ 0x080D0C40, 0x080D0C44, XV_POOL },
	{ 0x080D0C44, 0x080D0C50, XV_TABLE },
	{ 0x080D0D12, 0x080D0D14, XV_RAW },
	{ 0x080D0D14, 0x080D0D20, XV_POOL },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair metfire_calls[] = {
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
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080090E8, 0x0800A8F8 },   /* map:bytes, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08012C1E, 0x08015E46 },   /* map:bytes, SAME */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08012A20, 0x08015C48 },   /* map:bytes, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
};
static const XvPair metfire_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080D0D21, 0x080D0CF9 },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm metfire_imms[] = {
	{ 0x080D0C66, THUMB_MOVS, 1, 0, 0xC, 0x10 },   /* its table 080D0C0C's sprites' list: 3 in BN5, 4 in BN6 (its own not grown) */
	{ 0x08106C74, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x08106C8C, THUMB_MOVS, 0, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x08106C90, THUMB_MOVS, 0, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x08106CA4, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvByte metfire_bytes[] = {
	{ 0x080D0C0C, 0x31, 0x5F },   /* a sprite of its table 080D0C0C: list 3 0x31 in BN5, 0x5f in BN6 */
};
static const XvRange metfire_ranges[] = {
	{ 0x08106C46, 0x21E, 0x223 },   /* NameIDs 0x1f-0x24 in BN5, 0x21e-0x223 in BN6 */
};
static const XvRef metfire_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08106C19), BN6(0x0810AE95), OWN(0x08104D87), OWN(0x08106AF5),
};
static const XvSprite metfire_sprites[] = {
	{ 1, 0x06, 1, 0x23, XV_ART + 0x13000, 0x3000 },   /* (LZ77 in BN5) */
	{ 3, 0x31, 4, 0x5F, XV_ART + 0x16000, 0x800 },
};
static const XvType metfire_types[] = { { 3, 0x56, XV_TYPE_MOVED } };
static const uint16_t metfire_held[XV_VERSIONS * 16] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x348F, 0x348F, 0x348F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x348F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0x348F, 0x348F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0x34A3, 0x348F, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

static const XVirusFamily bn5_metfire = {
	.name = "MetFire (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 6,
	.ai = 37,
	.first_id = XV_FIRST_ID + 0x1E,
	.home = XV_HOMES + 0x2800,
	.blocks = metfire_blocks, .nblocks = (int)(sizeof metfire_blocks / sizeof *metfire_blocks),
	.data = metfire_data, .ndata = (int)(sizeof metfire_data / sizeof *metfire_data),
	.calls = metfire_calls, .ncalls = (int)(sizeof metfire_calls / sizeof *metfire_calls),
	.words = metfire_words, .nwords = (int)(sizeof metfire_words / sizeof *metfire_words),
	.imms = metfire_imms, .nimms = (int)(sizeof metfire_imms / sizeof *metfire_imms),
	.bytes = metfire_bytes, .nbytes = (int)(sizeof metfire_bytes / sizeof *metfire_bytes),
	.ranges = metfire_ranges, .nranges = (int)(sizeof metfire_ranges / sizeof *metfire_ranges),
	.states = metfire_states, .nstates = (int)(sizeof metfire_states / sizeof *metfire_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08106A08, .struct2 = 0x08106A10,
	.names = { 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24 },
	.held_chips = metfire_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x06, .element = 0, .weakness = 0, .shadow = true },
	.sprites = metfire_sprites, .nsprites = (int)(sizeof metfire_sprites / sizeof *metfire_sprites),
	.types = metfire_types, .ntypes = (int)(sizeof metfire_types / sizeof *metfire_types),
};
/* ---- BN5's Shakey (AI 7 there, its enemy ids 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a), as AI 38 with ids 0x224-0x229 ---- */

static const XvBlock shakey_blocks[] = {
	{ 0x08106EC0, 0x081072BC, XV_CODE + 0x3800, 0xE01DACDBu },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0x3C02, 0xA53A3BEFu },
	{ 0x08104BBC, 0x08104BFC, XV_CODE + 0x3E00, 0x838CAF0Du },
	{ 0x081072BC, 0x08107334, XV_CODE + 0x4000, 0x9B7C0F9Fu },
};
static const XvData shakey_data[] = {
	{ 0x08106ECE, 0x08106ED0, XV_RAW },
	{ 0x08106ED0, 0x08106ED4, XV_POOL },
	{ 0x08106ED4, 0x08106EEC, XV_TABLE },
	{ 0x081071B8, 0x081071BC, XV_POOL },
	{ 0x081071BC, 0x081071C4, XV_RAW },
	{ 0x081071C4, 0x081071D4, XV_POOL },
	{ 0x081071D4, 0x081071F8, XV_ZERO },
	{ 0x0810722E, 0x08107230, XV_RAW },
	{ 0x08107230, 0x08107234, XV_POOL },
	{ 0x08107234, 0x0810723C, XV_TABLE },
	{ 0x08107292, 0x08107294, XV_RAW },
	{ 0x08107294, 0x08107298, XV_POOL },
	{ 0x08107298, 0x081072B0, XV_RAW },
	{ 0x081072B0, 0x081072B4, XV_POOL },
	{ 0x081072B4, 0x081072BC, XV_RAW },
	{ 0x08104BF4, 0x08104BF8, XV_RAW },
	{ 0x08104BF8, 0x08104BFC, XV_POOL },
	{ 0x081072BC, 0x08107334, XV_RAW },
};
static const XvPair shakey_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x0800C6CE, 0x0800E2CE },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800B380, 0x0800CCBE },   /* project:research, DIFF (audited) */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
};
static const XvPair shakey_words[] = {
	{ 0x08005CD0, 0x080065E0 },
	{ 0x08005D50, 0x08006660 },
};
static const XvImm shakey_imms[] = {
	{ 0x08104BE2, THUMB_LDR, 0, 3, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810702C, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810707E, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x08107242, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x08107286, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber shakey_numbers[] = {
	{ 0x08104BF8, 0x08000000, 0x08000000 },
	{ 0x081071D0, 0x03800000, 0x03800000 },
};
static const XvRef shakey_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810721B), BN6(0x0810AE95), OWN(0x08106EC1),
};
static const XvSprite shakey_sprites[] = {
	{ 1, 0x07, 1, 0x24, XV_ART + 0x16800, 0x1000 },
};

static const XVirusFamily bn5_shakey = {
	.name = "Shakey (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 7,
	.ai = 38,
	.first_id = XV_FIRST_ID + 0x24,
	.home = XV_HOMES + 0x3000,
	.blocks = shakey_blocks, .nblocks = (int)(sizeof shakey_blocks / sizeof *shakey_blocks),
	.data = shakey_data, .ndata = (int)(sizeof shakey_data / sizeof *shakey_data),
	.calls = shakey_calls, .ncalls = (int)(sizeof shakey_calls / sizeof *shakey_calls),
	.words = shakey_words, .nwords = (int)(sizeof shakey_words / sizeof *shakey_words),
	.imms = shakey_imms, .nimms = (int)(sizeof shakey_imms / sizeof *shakey_imms),
	.numbers = shakey_numbers, .nnumbers = (int)(sizeof shakey_numbers / sizeof *shakey_numbers),
	.states = shakey_states, .nstates = (int)(sizeof shakey_states / sizeof *shakey_states),
	.hooks = { OWN(0x08107207), BN6(BN6_FAMILY_NULLSUB), OWN(0x081071F9), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08106DD4, .struct2 = 0x08106DDC,
	.names = { 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x07, .element = 0, .weakness = 0, .shadow = true },
	.sprites = shakey_sprites, .nsprites = (int)(sizeof shakey_sprites / sizeof *shakey_sprites),
};
/* ---- BN5's NinJoy (AI 9 there, its enemy ids 0x31, 0x32, 0x33, 0x34, 0x35, 0x36), as AI 39 with ids 0x22a-0x22f ---- */

static const XvBlock ninjoy_blocks[] = {
	{ 0x081078DC, 0x08107D44, XV_CODE + 0x4200, 0x0F7B27BFu },
	{ 0x08104B96, 0x08104BBC, XV_CODE + 0x4802, 0x8F659214u },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0x4A02, 0x21492011u },
};
static const XvData ninjoy_data[] = {
	{ 0x081078EA, 0x081078EC, XV_RAW },
	{ 0x081078EC, 0x081078F0, XV_POOL },
	{ 0x081078F0, 0x08107908, XV_TABLE },
	{ 0x08107AB8, 0x08107AC0, XV_POOL },
	{ 0x08107BB2, 0x08107BB4, XV_RAW },
	{ 0x08107BB4, 0x08107BB8, XV_POOL },
	{ 0x08107BB8, 0x08107BC8, XV_RAW },
	{ 0x08107BC8, 0x08107BD0, XV_POOL },
	{ 0x08107BD0, 0x08107BE0, XV_RAW },
	{ 0x08107BE0, 0x08107C04, XV_ZERO },
	{ 0x08107C34, 0x08107C38, XV_POOL },
	{ 0x08107C38, 0x08107C3C, XV_TABLE },
	{ 0x08107CAA, 0x08107CAC, XV_RAW },
	{ 0x08107CAC, 0x08107CB8, XV_POOL },
	{ 0x08107CB8, 0x08107CBE, XV_RAW },
	{ 0x08107CBE, 0x08107CC4, XV_RAW },
	{ 0x08107CC4, 0x08107CCA, XV_RAW },
	{ 0x08107D34, 0x08107D44, XV_POOL },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair ninjoy_calls[] = {
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800B5A8, 0x0800CFA6 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x080133CE, 0x080165C2 },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
};
static const XvPair ninjoy_words[] = {
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm ninjoy_imms[] = {
	{ 0x08107962, THUMB_MOVS, 4, 0, 0x4E, 0x6F },   /* its effect: 0x4e in BN5, 0x6f in BN6 */
	{ 0x081079E4, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x08107A50, THUMB_ADDS, 4, 0, 0x4F, 0x43 },   /* its effect: 0x4f in BN5, 0x43 in BN6 */
	{ 0x08107AF0, THUMB_MOVS, 4, 0, 0x4E, 0x6F },   /* its effect: 0x4e in BN5, 0x6f in BN6 */
	{ 0x08107B32, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x08107C98, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber ninjoy_numbers[] = {
	{ 0x08107AB8, 0x0705FF01, 0x0705FF01 },
};
static const XvRef ninjoy_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x08107C05), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08107C21), OWN(0x081078DD), OWN(0x08104D87),
};
static const XvSprite ninjoy_sprites[] = {
	{ 1, 0x09, 1, 0x25, XV_ART + 0x17800, 0x2000 },   /* (LZ77 in BN5) */
	{ 4, 0x37, 4, 0x60, XV_ART + 0x19800, 0xC00 },
};
static const XvId ninjoy_effects[] = { { 0x4E, 0x6F } };

static const XVirusFamily bn5_ninjoy = {
	.name = "NinJoy (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 9,
	.ai = 39,
	.first_id = XV_FIRST_ID + 0x2A,
	.home = XV_HOMES + 0x3800,
	.blocks = ninjoy_blocks, .nblocks = (int)(sizeof ninjoy_blocks / sizeof *ninjoy_blocks),
	.data = ninjoy_data, .ndata = (int)(sizeof ninjoy_data / sizeof *ninjoy_data),
	.calls = ninjoy_calls, .ncalls = (int)(sizeof ninjoy_calls / sizeof *ninjoy_calls),
	.words = ninjoy_words, .nwords = (int)(sizeof ninjoy_words / sizeof *ninjoy_words),
	.imms = ninjoy_imms, .nimms = (int)(sizeof ninjoy_imms / sizeof *ninjoy_imms),
	.numbers = ninjoy_numbers, .nnumbers = (int)(sizeof ninjoy_numbers / sizeof *ninjoy_numbers),
	.states = ninjoy_states, .nstates = (int)(sizeof ninjoy_states / sizeof *ninjoy_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x081077F0, .struct2 = 0x081077F8,
	.names = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x09, .element = 0, .weakness = 0, .shadow = true },
	.sprites = ninjoy_sprites, .nsprites = (int)(sizeof ninjoy_sprites / sizeof *ninjoy_sprites),
	.effects = ninjoy_effects, .neffects = (int)(sizeof ninjoy_effects / sizeof *ninjoy_effects),
};
/* ---- BN5's Lark (AI 11 there, its enemy ids 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42), as AI 40 with ids 0x230-0x235 ---- */

static const XvBlock lark_blocks[] = {
	{ 0x08107F8C, 0x08108254, XV_CODE + 0x4C00, 0xC9817107u },
	{ 0x08108254, 0x081082C4, XV_CODE + 0x5000, 0x06CDB91Eu },
};
static const XvData lark_data[] = {
	{ 0x08107F9A, 0x08107F9C, XV_RAW },
	{ 0x08107F9C, 0x08107FA0, XV_POOL },
	{ 0x08107FA0, 0x08107FAC, XV_TABLE },
	{ 0x0810800A, 0x0810800C, XV_RAW },
	{ 0x0810800C, 0x08108034, XV_ZERO },
	{ 0x08108048, 0x0810804C, XV_POOL },
	{ 0x0810804C, 0x08108054, XV_TABLE },
	{ 0x081080CA, 0x081080CC, XV_RAW },
	{ 0x081080CC, 0x081080D4, XV_POOL },
	{ 0x081080D4, 0x081080DA, XV_RAW },
	{ 0x08108198, 0x0810819C, XV_POOL },
	{ 0x0810819C, 0x081081A4, XV_RAW },
	{ 0x081081A4, 0x081081A8, XV_POOL },
	{ 0x081081A8, 0x081081C0, XV_RAW },
	{ 0x081081E6, 0x081081E8, XV_RAW },
	{ 0x081081E8, 0x081081F0, XV_POOL },
	{ 0x081081F0, 0x081081F6, XV_RAW },
	{ 0x081081F6, 0x081081FC, XV_RAW },
	{ 0x08108236, 0x08108238, XV_RAW },
	{ 0x08108238, 0x08108244, XV_POOL },
	{ 0x08108244, 0x08108254, XV_RAW },
	{ 0x08108254, 0x081082C4, XV_RAW },
};
static const XvPair lark_calls[] = {
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080CCA62, 0x080CCEF6 },   /* map:bytes, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
};
static const XvPair lark_words[] = {
	{ 0x08104BFD, 0x0810AE95 },
};
static const XvImm lark_imms[] = {
	{ 0x0810818C, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x081081DE, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
};
static const XvRange lark_ranges[] = {
	{ 0x0810808C, 0x230, 0x235 },   /* NameIDs 0x3d-0x42 in BN5, 0x230-0x235 in BN6 */
};
static const XvCmp lark_cmps[] = {
	{ 0x08108214, 4, 0x81, 0x1C8 },   /* a held chip compared: BN5's chip 129, BN6's 456 (the chip registry), past an immediate's 255 */
	{ 0x08108218, 4, 0x8E, 0x1CC },   /* a held chip compared: BN5's chip 142, BN6's 460 (the chip registry), past an immediate's 255 */
};
static const XvRef lark_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08108035), BN6(0x0810AE95), BN6(0x0810AB35), OWN(0x08107F8D),
};
static const XvSprite lark_sprites[] = {
	{ 1, 0x0B, 1, 0x26, XV_ART + 0x1A800, 0x3000 },   /* (LZ77 in BN5) */
};
static const uint16_t lark_held[XV_VERSIONS * 16] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C8, 0x35C8, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C2, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34D3, 0x34D3, 0x35CC, 0x35CC, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34D3, 0x34D3, 0x35C2, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C8, 0x34D3, 0x35CC, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

static const XVirusFamily bn5_lark = {
	.name = "Lark (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 11,
	.ai = 40,
	.first_id = XV_FIRST_ID + 0x30,
	.home = XV_HOMES + 0x4000,
	.blocks = lark_blocks, .nblocks = (int)(sizeof lark_blocks / sizeof *lark_blocks),
	.data = lark_data, .ndata = (int)(sizeof lark_data / sizeof *lark_data),
	.calls = lark_calls, .ncalls = (int)(sizeof lark_calls / sizeof *lark_calls),
	.words = lark_words, .nwords = (int)(sizeof lark_words / sizeof *lark_words),
	.imms = lark_imms, .nimms = (int)(sizeof lark_imms / sizeof *lark_imms),
	.ranges = lark_ranges, .nranges = (int)(sizeof lark_ranges / sizeof *lark_ranges),
	.cmps = lark_cmps, .ncmps = (int)(sizeof lark_cmps / sizeof *lark_cmps),
	.states = lark_states, .nstates = (int)(sizeof lark_states / sizeof *lark_states),
	.hooks = { OWN(0x081080DB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08107EA0, .struct2 = 0x08107EA8,
	.names = { 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42 },
	.held_chips = lark_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x0B, .element = 0, .weakness = 0, .shadow = true },
	.sprites = lark_sprites, .nsprites = (int)(sizeof lark_sprites / sizeof *lark_sprites),
};
/* ---- BN5's BomBoy (AI 12 there, its enemy ids 0x43, 0x44, 0x45, 0x46, 0x47, 0x48), as AI 41 with ids 0x236-0x23b ---- */

static const XvBlock bomboy_blocks[] = {
	{ 0x08108340, 0x081086EC, XV_CODE + 0x5200, 0x923D786Eu },
	{ 0x080CCEDC, 0x080CCEFC, XV_CODE + 0x5600, 0x60DBE9DAu },
	{ 0x080CCF0E, 0x080CCFD0, XV_CODE + 0x5802, 0xD30E7093u },
	{ 0x080CD1C8, 0x080CD200, XV_CODE + 0x5A00, 0x36DE3168u },
	{ 0x080CD282, 0x080CD3B4, XV_CODE + 0x5C02, 0xEF65060Bu },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0x5E02, 0x21492011u },
};
static const XvData bomboy_data[] = {
	{ 0x0810834E, 0x08108350, XV_RAW },
	{ 0x08108350, 0x08108354, XV_POOL },
	{ 0x08108354, 0x0810835C, XV_TABLE },
	{ 0x0810839E, 0x081083A0, XV_RAW },
	{ 0x081083A0, 0x081083A4, XV_POOL },
	{ 0x081083A4, 0x081083AC, XV_TABLE },
	{ 0x081084F4, 0x081084F8, XV_POOL },
	{ 0x081084F8, 0x08108508, XV_RAW },
	{ 0x08108508, 0x0810850C, XV_POOL },
	{ 0x0810850C, 0x08108538, XV_ZERO },
	{ 0x0810858E, 0x08108590, XV_RAW },
	{ 0x08108590, 0x08108594, XV_POOL },
	{ 0x08108594, 0x081085A0, XV_TABLE },
	{ 0x081085D8, 0x081085DC, XV_POOL },
	{ 0x081085DC, 0x081085E2, XV_RAW },
	{ 0x0810862E, 0x08108630, XV_RAW },
	{ 0x08108630, 0x08108634, XV_POOL },
	{ 0x08108634, 0x0810864C, XV_RAW },
	{ 0x08108682, 0x08108684, XV_RAW },
	{ 0x08108684, 0x08108688, XV_POOL },
	{ 0x08108688, 0x0810868E, XV_RAW },
	{ 0x081086D2, 0x081086D4, XV_RAW },
	{ 0x081086D4, 0x081086D8, XV_POOL },
	{ 0x081086D8, 0x081086E8, XV_RAW },
	{ 0x081086E8, 0x081086EC, XV_POOL },
	{ 0x080CCEEA, 0x080CCEEC, XV_RAW },
	{ 0x080CCEEC, 0x080CCEF0, XV_POOL },
	{ 0x080CCEF0, 0x080CCEFC, XV_TABLE },
	{ 0x080CD2D2, 0x080CD3A0, XV_ZERO },
	{ 0x080CD3A0, 0x080CD3A4, XV_POOL },
	{ 0x080CD3A4, 0x080CD3AC, XV_ZERO },
	{ 0x080CD3AC, 0x080CD3B4, XV_POOL },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair bomboy_calls[] = {
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
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0801A5FE, 0x0801DCCC },   /* map:bytes, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x080CD2D2, 0x080CD7C4 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080CD200, 0x080CD6F2 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08013944, 0x08016CE8 },   /* project:research, SAME */
	{ 0x080133CE, 0x080165C2 },   /* project:cangard, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800B688, 0x0800D086 },   /* map:bytes, SAME */
};
static const XvPair bomboy_words[] = {
	{ 0x080CCEFD, 0x080CD3D5 },
	{ 0x080CCFD1, 0x080CD4A9 },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm bomboy_imms[] = {
	{ 0x080CCF16, THUMB_MOVS, 2, 0, 0x36, 0x61 },   /* its sprite: list 4 0x36 in BN5, 0x61 in BN6 */
	{ 0x081085CC, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x081085FC, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x08108622, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x08108662, THUMB_MOVS, 0, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x08108666, THUMB_MOVS, 0, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810867A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber bomboy_numbers[] = {
	{ 0x080CD3B0, 0x06800000, 0x06800000 },
};
static const XvRef bomboy_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x08108563), BN6(0x080166AF), BN6(0x08016B03), OWN(0x08108539), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810857B), BN6(0x0810AE95), OWN(0x08104D87), OWN(0x08108341), OWN(0x08108391),
};
static const XvSprite bomboy_sprites[] = {
	{ 1, 0x0C, 1, 0x27, XV_ART + 0x1D800, 0x2000 },   /* (LZ77 in BN5) */
	{ 4, 0x36, 4, 0x61, XV_ART + 0x1F800, 0x800 },
};
static const XvType bomboy_types[] = { { 3, 0x3E, XV_TYPE_MOVED } };

static const XVirusFamily bn5_bomboy = {
	.name = "BomBoy (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 12,
	.ai = 41,
	.first_id = XV_FIRST_ID + 0x36,
	.home = XV_HOMES + 0x4800,
	.blocks = bomboy_blocks, .nblocks = (int)(sizeof bomboy_blocks / sizeof *bomboy_blocks),
	.data = bomboy_data, .ndata = (int)(sizeof bomboy_data / sizeof *bomboy_data),
	.calls = bomboy_calls, .ncalls = (int)(sizeof bomboy_calls / sizeof *bomboy_calls),
	.words = bomboy_words, .nwords = (int)(sizeof bomboy_words / sizeof *bomboy_words),
	.imms = bomboy_imms, .nimms = (int)(sizeof bomboy_imms / sizeof *bomboy_imms),
	.numbers = bomboy_numbers, .nnumbers = (int)(sizeof bomboy_numbers / sizeof *bomboy_numbers),
	.states = bomboy_states, .nstates = (int)(sizeof bomboy_states / sizeof *bomboy_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08108254, .struct2 = 0x0810825C,
	.names = { 0x43, 0x44, 0x45, 0x46, 0x47, 0x48 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x0C, .element = 0, .weakness = 0, .shadow = true },
	.sprites = bomboy_sprites, .nsprites = (int)(sizeof bomboy_sprites / sizeof *bomboy_sprites),
	.types = bomboy_types, .ntypes = (int)(sizeof bomboy_types / sizeof *bomboy_types),
};
/* ---- BN5's Zomon (AI 13 there, its enemy ids 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e), as AI 42 with ids 0x23c-0x241 ---- */

static const XvBlock zomon_blocks[] = {
	{ 0x081087D8, 0x08108D28, XV_CODE + 0x6000, 0x45D9DACCu },
	{ 0x08104C1C, 0x08104C38, XV_CODE + 0x6600, 0x2A5560B0u },
};
static const XvData zomon_data[] = {
	{ 0x081087E6, 0x081087E8, XV_RAW },
	{ 0x081087E8, 0x081087EC, XV_POOL },
	{ 0x081087EC, 0x081087FC, XV_TABLE },
	{ 0x081088EE, 0x081088F0, XV_RAW },
	{ 0x081088F0, 0x081088F8, XV_POOL },
	{ 0x081088F8, 0x08108900, XV_RAW },
	{ 0x08108900, 0x08108904, XV_RAW },
	{ 0x08108904, 0x08108930, XV_ZERO },
	{ 0x08108974, 0x08108978, XV_POOL },
	{ 0x08108978, 0x0810898C, XV_TABLE },
	{ 0x081089A8, 0x081089AC, XV_POOL },
	{ 0x081089AC, 0x081089B8, XV_RAW },
	{ 0x08108AA2, 0x08108AA4, XV_RAW },
	{ 0x08108AA4, 0x08108AA8, XV_POOL },
	{ 0x08108AA8, 0x08108AC0, XV_RAW },
	{ 0x08108BB2, 0x08108BB4, XV_RAW },
	{ 0x08108BB4, 0x08108BC0, XV_POOL },
	{ 0x08108BC0, 0x08108BD8, XV_RAW },
	{ 0x08108BD8, 0x08108BDE, XV_RAW },
	{ 0x08108BDE, 0x08108BE4, XV_RAW },
	{ 0x08108C28, 0x08108C2C, XV_POOL },
	{ 0x08108C2C, 0x08108C32, XV_RAW },
	{ 0x08108CB2, 0x08108CB4, XV_RAW },
	{ 0x08108CB4, 0x08108CB8, XV_POOL },
	{ 0x08108CB8, 0x08108CC4, XV_RAW },
	{ 0x08108CF6, 0x08108CF8, XV_RAW },
	{ 0x08108CF8, 0x08108CFC, XV_POOL },
	{ 0x08108CFC, 0x08108D0C, XV_RAW },
	{ 0x08108D0C, 0x08108D10, XV_POOL },
	{ 0x08108D10, 0x08108D20, XV_RAW },
	{ 0x08108D20, 0x08108D28, XV_POOL },
};
static const XvPair zomon_calls[] = {
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0801A576, 0x0801DC36 },   /* map:bytes, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800B6BE, 0x0800D0BC },   /* map:bytes, SAME */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
	{ 0x0800CE54, 0x0800EBD4 },   /* map:bytes, SAME */
};
static const XvImm zomon_imms[] = {
	{ 0x08108932, THUMB_MOVS, 0, 0, 0x75, 0xA3 },   /* a chip given its object (+0x2A): BN5's chip 117, BN6's 163 (the chip registry) */
	{ 0x08108A96, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x08108B18, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x08108B50, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x08108B9E, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x08108BA6, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x08108C1C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvByte zomon_bytes[] = {
	{ 0x08108900, 0x17, 0x70 },   /* an effect id of its table 08108900: 0x17 in BN5, 0x70 in BN6 */
	{ 0x08108901, 0x16, 0x71 },   /* an effect id of its table 08108900: 0x16 in BN5, 0x71 in BN6 */
};
static const XvRange zomon_ranges[] = {
	{ 0x08108C94, 0x23C, 0x241 },   /* NameIDs 0x49-0x4e in BN5, 0x23c-0x241 in BN6 */
};
static const XvRef zomon_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08108961), OWN(0x08104C1D), BN6(0x0810AB35), OWN(0x081087D9), BN6(0x0810AE95),
};
static const XvSprite zomon_sprites[] = {
	{ 1, 0x0D, 1, 0x28, XV_ART + 0x20000, 0x4000 },   /* (LZ77 in BN5) */
	{ 3, 0x14, 4, 0x62, XV_ART + 0x24000, 0x2800 },
};
static const XvId zomon_effects[] = { { 0x17, 0x70 }, { 0x16, 0x71 } };
static const uint16_t zomon_held[XV_VERSIONS * 16] = {
	0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3,
	0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3,
	0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3,
	0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3,
	0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3,
	0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3, 0x34A3,
};

static const XVirusFamily bn5_zomon = {
	.name = "Zomon (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 13,
	.ai = 42,
	.first_id = XV_FIRST_ID + 0x3C,
	.home = XV_HOMES + 0x5000,
	.blocks = zomon_blocks, .nblocks = (int)(sizeof zomon_blocks / sizeof *zomon_blocks),
	.data = zomon_data, .ndata = (int)(sizeof zomon_data / sizeof *zomon_data),
	.calls = zomon_calls, .ncalls = (int)(sizeof zomon_calls / sizeof *zomon_calls),
	.imms = zomon_imms, .nimms = (int)(sizeof zomon_imms / sizeof *zomon_imms),
	.bytes = zomon_bytes, .nbytes = (int)(sizeof zomon_bytes / sizeof *zomon_bytes),
	.ranges = zomon_ranges, .nranges = (int)(sizeof zomon_ranges / sizeof *zomon_ranges),
	.states = zomon_states, .nstates = (int)(sizeof zomon_states / sizeof *zomon_states),
	.hooks = { OWN(0x08108941), BN6(BN6_FAMILY_NULLSUB), OWN(0x08108931), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x081086EC, .struct2 = 0x081086F4,
	.names = { 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E },
	.held_chips = zomon_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x0B, .element = 0, .weakness = 0, .shadow = true },
	.sprites = zomon_sprites, .nsprites = (int)(sizeof zomon_sprites / sizeof *zomon_sprites),
	.effects = zomon_effects, .neffects = (int)(sizeof zomon_effects / sizeof *zomon_effects),
};
/* ---- BN5's Drixol (AI 16 there, its enemy ids 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60), as AI 43 with ids 0x242-0x247 ---- */

static const XvBlock drixol_blocks[] = {
	{ 0x0810971C, 0x08109BF8, XV_CODE + 0x6800, 0xF894CE81u },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0x6E02, 0xA53A3BEFu },
	{ 0x080E95DC, 0x080E96A2, XV_CODE + 0x7000, 0xF0B85ED1u },
	{ 0x08104BBC, 0x08104BFC, XV_CODE + 0x7200, 0x838CAF0Du },
	{ 0x08104C1C, 0x08104C38, XV_CODE + 0x7400, 0x2A5560B0u },
};
static const XvData drixol_data[] = {
	{ 0x0810972A, 0x0810972C, XV_RAW },
	{ 0x0810972C, 0x08109730, XV_POOL },
	{ 0x08109730, 0x08109748, XV_TABLE },
	{ 0x08109820, 0x08109824, XV_POOL },
	{ 0x08109824, 0x0810982C, XV_RAW },
	{ 0x08109858, 0x08109860, XV_POOL },
	{ 0x08109860, 0x08109868, XV_RAW },
	{ 0x08109928, 0x0810992C, XV_POOL },
	{ 0x0810992C, 0x08109934, XV_RAW },
	{ 0x08109A5A, 0x08109A5C, XV_RAW },
	{ 0x08109A5C, 0x08109A60, XV_POOL },
	{ 0x08109A60, 0x08109A88, XV_ZERO },
	{ 0x08109AA6, 0x08109AA8, XV_RAW },
	{ 0x08109AA8, 0x08109AAC, XV_POOL },
	{ 0x08109AAC, 0x08109AB8, XV_TABLE },
	{ 0x08109B3C, 0x08109B40, XV_POOL },
	{ 0x08109B40, 0x08109B58, XV_RAW },
	{ 0x08109B58, 0x08109B5C, XV_POOL },
	{ 0x08109B5C, 0x08109B74, XV_RAW },
	{ 0x08109BC8, 0x08109BCC, XV_POOL },
	{ 0x08109BCC, 0x08109BD2, XV_RAW },
	{ 0x08109BE0, 0x08109BE4, XV_POOL },
	{ 0x08109BE4, 0x08109BF4, XV_RAW },
	{ 0x08109BF4, 0x08109BF8, XV_POOL },
	{ 0x080E95EA, 0x080E95EC, XV_RAW },
	{ 0x080E95EC, 0x080E95F0, XV_POOL },
	{ 0x080E95F0, 0x080E95FC, XV_TABLE },
	{ 0x08104BF4, 0x08104BF8, XV_RAW },
	{ 0x08104BF8, 0x08104BFC, XV_POOL },
};
static const XvPair drixol_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800CDEC, 0x0800EB6C },   /* map:bytes, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x0800CE54, 0x0800EBD4 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
};
static const XvPair drixol_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x08104BFD, 0x0810AE95 },
};
static const XvImm drixol_imms[] = {
	{ 0x080E9600, THUMB_MOVS, 2, 0, 0x3F, 0x63 },   /* its sprite: list 4 0x3f in BN5, 0x63 in BN6 */
	{ 0x08104BE2, THUMB_LDR, 0, 3, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x081099D2, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x08109A0C, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x08109AF4, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x08109B28, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x08109BBC, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber drixol_numbers[] = {
	{ 0x08104BF8, 0x08000000, 0x08000000 },
	{ 0x08109858, 0x06050A01, 0x06050A01 },
};
static const XvRange drixol_ranges[] = {
	{ 0x08109944, 0x242, 0x247 },   /* NameIDs 0x5b-0x60 in BN5, 0x242-0x247 in BN6 */
};
static const XvRef drixol_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08109A93), OWN(0x08104C1D), BN6(0x0810AB35), OWN(0x0810971D),
};
static const XvSprite drixol_sprites[] = {
	{ 1, 0x10, 1, 0x29, XV_ART + 0x26800, 0x3000 },   /* (LZ77 in BN5) */
	{ 4, 0x3F, 4, 0x63, XV_ART + 0x29800, 0x1000 },
};
static const XvType drixol_types[] = { { 4, 0x78, XV_TYPE_MOVED } };

static const XVirusFamily bn5_drixol = {
	.name = "Drixol (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 16,
	.ai = 43,
	.first_id = XV_FIRST_ID + 0x42,
	.home = XV_HOMES + 0x5800,
	.blocks = drixol_blocks, .nblocks = (int)(sizeof drixol_blocks / sizeof *drixol_blocks),
	.data = drixol_data, .ndata = (int)(sizeof drixol_data / sizeof *drixol_data),
	.calls = drixol_calls, .ncalls = (int)(sizeof drixol_calls / sizeof *drixol_calls),
	.words = drixol_words, .nwords = (int)(sizeof drixol_words / sizeof *drixol_words),
	.imms = drixol_imms, .nimms = (int)(sizeof drixol_imms / sizeof *drixol_imms),
	.numbers = drixol_numbers, .nnumbers = (int)(sizeof drixol_numbers / sizeof *drixol_numbers),
	.ranges = drixol_ranges, .nranges = (int)(sizeof drixol_ranges / sizeof *drixol_ranges),
	.states = drixol_states, .nstates = (int)(sizeof drixol_states / sizeof *drixol_states),
	.hooks = { OWN(0x08109A89), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08109630, .struct2 = 0x08109638,
	.names = { 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x10, .element = 0, .weakness = 0, .shadow = true },
	.sprites = drixol_sprites, .nsprites = (int)(sizeof drixol_sprites / sizeof *drixol_sprites),
	.types = drixol_types, .ntypes = (int)(sizeof drixol_types / sizeof *drixol_types),
};
/* ---- BN5's Batty (AI 17 there, its enemy ids 0x61, 0x62, 0x63, 0x64, 0x65, 0x66), as AI 44 with ids 0x248-0x24d ---- */

static const XvBlock batty_blocks[] = {
	{ 0x08109CE4, 0x0810A088, XV_CODE + 0x7600, 0x6C83B155u },
	{ 0x080D33B8, 0x080D3570, XV_CODE + 0x7A00, 0xB5B961B7u },
};
static const XvData batty_data[] = {
	{ 0x08109CF2, 0x08109CF4, XV_RAW },
	{ 0x08109CF4, 0x08109CF8, XV_POOL },
	{ 0x08109CF8, 0x08109D04, XV_TABLE },
	{ 0x08109DA6, 0x08109DA8, XV_RAW },
	{ 0x08109DA8, 0x08109DAC, XV_POOL },
	{ 0x08109DAC, 0x08109DBC, XV_TABLE },
	{ 0x08109E6C, 0x08109E70, XV_POOL },
	{ 0x08109E70, 0x08109E94, XV_ZERO },
	{ 0x08109EB8, 0x08109EBC, XV_POOL },
	{ 0x08109EBC, 0x08109EC4, XV_RAW },
	{ 0x08109EEA, 0x08109EEC, XV_RAW },
	{ 0x08109EEC, 0x08109EF0, XV_POOL },
	{ 0x08109EF0, 0x08109EFC, XV_TABLE },
	{ 0x08109F96, 0x08109F98, XV_RAW },
	{ 0x08109F98, 0x08109FA8, XV_POOL },
	{ 0x08109FA8, 0x08109FC0, XV_RAW },
	{ 0x08109FC0, 0x08109FC6, XV_RAW },
	{ 0x08109FC6, 0x08109FCC, XV_RAW },
	{ 0x08109FCC, 0x08109FD2, XV_RAW },
	{ 0x08109FFE, 0x0810A000, XV_RAW },
	{ 0x0810A000, 0x0810A004, XV_POOL },
	{ 0x0810A004, 0x0810A00A, XV_RAW },
	{ 0x0810A02A, 0x0810A02C, XV_RAW },
	{ 0x0810A02C, 0x0810A030, XV_POOL },
	{ 0x0810A030, 0x0810A048, XV_RAW },
	{ 0x0810A06C, 0x0810A070, XV_POOL },
	{ 0x0810A070, 0x0810A080, XV_RAW },
	{ 0x0810A080, 0x0810A088, XV_POOL },
	{ 0x080D33CA, 0x080D33CC, XV_RAW },
	{ 0x080D33CC, 0x080D33D0, XV_POOL },
	{ 0x080D33D0, 0x080D33DC, XV_TABLE },
	{ 0x080D34C4, 0x080D34C8, XV_POOL },
	{ 0x080D34C8, 0x080D34D0, XV_TABLE },
	{ 0x080D3548, 0x080D354C, XV_POOL },
	{ 0x080D356C, 0x080D3570, XV_POOL },
};
static const XvPair batty_calls[] = {
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
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x080131A0, 0x08016380 },   /* project:research, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0801A5FE, 0x0801DCCC },   /* map:bytes, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800B614, 0x0800D012 },   /* map:bytes, SAME */
};
static const XvPair batty_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x08104BFD, 0x0810AE95 },
};
static const XvImm batty_imms[] = {
	{ 0x080D33E4, THUMB_MOVS, 2, 0, 0x3A, 0x64 },   /* its sprite: list 4 0x3a in BN5, 0x64 in BN6 */
	{ 0x080D3476, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D3514, THUMB_ADDS, 4, 0, 0x56, 0x72 },   /* its effect: 0x56 in BN5, 0x72 in BN6 */
	{ 0x08109F7C, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x08109FF6, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber batty_numbers[] = {
	{ 0x080D3548, 0x0405FF10, 0x0405FF10 },
	{ 0x080D356C, 0x03800000, 0x03800000 },
};
static const XvRef batty_states[] = {
	OWN(0x08109EC5), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x08109ED7), OWN(0x08109CE5), OWN(0x08109D99),
};
static const XvSprite batty_sprites[] = {
	{ 1, 0x11, 1, 0x2A, XV_ART + 0x2A800, 0x2000 },   /* (LZ77 in BN5) */
	{ 4, 0x3A, 4, 0x64, XV_ART + 0x2C800, 0x800 },
	{ 4, 0x3B, 4, 0x65, XV_ART + 0x2D000, 0x1000 },
};
static const XvId batty_effects[] = { { 0x56, 0x72 } };
static const XvType batty_types[] = { { 3, 0x6A, XV_TYPE_MOVED } };

static const XVirusFamily bn5_batty = {
	.name = "Batty (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 17,
	.ai = 44,
	.first_id = XV_FIRST_ID + 0x48,
	.home = XV_HOMES + 0x6000,
	.blocks = batty_blocks, .nblocks = (int)(sizeof batty_blocks / sizeof *batty_blocks),
	.data = batty_data, .ndata = (int)(sizeof batty_data / sizeof *batty_data),
	.calls = batty_calls, .ncalls = (int)(sizeof batty_calls / sizeof *batty_calls),
	.words = batty_words, .nwords = (int)(sizeof batty_words / sizeof *batty_words),
	.imms = batty_imms, .nimms = (int)(sizeof batty_imms / sizeof *batty_imms),
	.numbers = batty_numbers, .nnumbers = (int)(sizeof batty_numbers / sizeof *batty_numbers),
	.states = batty_states, .nstates = (int)(sizeof batty_states / sizeof *batty_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), OWN(0x08109E95), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x08109BF8, .struct2 = 0x08109C00,
	.names = { 0x61, 0x62, 0x63, 0x64, 0x65, 0x66 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x11, .element = 0, .weakness = 0, .shadow = true },
	.sprites = batty_sprites, .nsprites = (int)(sizeof batty_sprites / sizeof *batty_sprites),
	.effects = batty_effects, .neffects = (int)(sizeof batty_effects / sizeof *batty_effects),
	.types = batty_types, .ntypes = (int)(sizeof batty_types / sizeof *batty_types),
};
/* ---- BN5's Skarab (AI 18 there, its enemy ids 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c), as AI 45 with ids 0x24e-0x253 ---- */

static const XvBlock skarab_blocks[] = {
	{ 0x0810A174, 0x0810A4C4, XV_CODE + 0x7C00, 0x6322155Au },
	{ 0x080DEDDC, 0x080DF0D8, XV_CODE + 0x8000, 0x64989091u },
};
static const XvData skarab_data[] = {
	{ 0x0810A1BA, 0x0810A1BC, XV_RAW },
	{ 0x0810A1BC, 0x0810A1C0, XV_POOL },
	{ 0x0810A1C0, 0x0810A1D0, XV_TABLE },
	{ 0x0810A23C, 0x0810A240, XV_POOL },
	{ 0x0810A240, 0x0810A244, XV_TABLE },
	{ 0x0810A2AA, 0x0810A2AC, XV_RAW },
	{ 0x0810A2AC, 0x0810A2B0, XV_POOL },
	{ 0x0810A2B0, 0x0810A2DC, XV_ZERO },
	{ 0x0810A34A, 0x0810A34C, XV_RAW },
	{ 0x0810A34C, 0x0810A350, XV_POOL },
	{ 0x0810A350, 0x0810A358, XV_TABLE },
	{ 0x0810A402, 0x0810A404, XV_RAW },
	{ 0x0810A404, 0x0810A420, XV_POOL },
	{ 0x0810A420, 0x0810A426, XV_RAW },
	{ 0x0810A426, 0x0810A432, XV_RAW },
	{ 0x0810A432, 0x0810A438, XV_RAW },
	{ 0x0810A438, 0x0810A43E, XV_RAW },
	{ 0x0810A43E, 0x0810A444, XV_RAW },
	{ 0x0810A444, 0x0810A44A, XV_RAW },
	{ 0x0810A44A, 0x0810A450, XV_RAW },
	{ 0x0810A480, 0x0810A484, XV_POOL },
	{ 0x0810A4AC, 0x0810A4B0, XV_POOL },
	{ 0x0810A4B0, 0x0810A4C0, XV_RAW },
	{ 0x0810A4C0, 0x0810A4C4, XV_POOL },
	{ 0x080DEDEE, 0x080DEDF0, XV_RAW },
	{ 0x080DEDF0, 0x080DEDF4, XV_POOL },
	{ 0x080DEDF4, 0x080DEE00, XV_TABLE },
	{ 0x080DEED4, 0x080DEED8, XV_POOL },
	{ 0x080DEED8, 0x080DEEE0, XV_TABLE },
	{ 0x080DEEE0, 0x080DEEE4, XV_POOL },
	{ 0x080DF046, 0x080DF0B6, XV_ZERO },
	{ 0x080DF0D0, 0x080DF0D8, XV_POOL },
};
static const XvPair skarab_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x080DF046, 0x080CAE5C },   /* search, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080133CE, 0x080165C2 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08012C1E, 0x08015E46 },   /* map:bytes, SAME */
};
static const XvPair skarab_words[] = {
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm skarab_imms[] = {
	{ 0x080DEE18, THUMB_MOVS, 2, 0, 0x50, 0x66 },   /* its sprite: list 4 0x50 in BN5, 0x66 in BN6 */
	{ 0x080DEEA0, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DEEC4, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810A296, THUMB_MOVS, 0, 0, 0xB7, 0x61 },   /* its sound 0xb7: BN5's copied into BN6's empty slot 0x61 */
	{ 0x0810A2EC, THUMB_CMP, 0, 0, 0x9, 0xB },   /* CurAction 9 compared */
	{ 0x0810A35E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810A372, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810A3B8, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810A3E2, THUMB_MOVS, 0, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810A3E6, THUMB_MOVS, 0, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810A3FA, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvSong skarab_songs[] = {
	{ 0xB7, 0x61, XV_ART + 0x31800, 0x100 },   /* its sound 0xb7, BN5's song copied */
};
static const XvRef skarab_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x0810A325), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810A337), OWN(0x0810A175), OWN(0x0810A18B), OWN(0x0810A1D1), OWN(0x0810A223),
};
static const XvSprite skarab_sprites[] = {
	{ 1, 0x12, 1, 0x2B, XV_ART + 0x2E000, 0x3000 },   /* (LZ77 in BN5) */
	{ 4, 0x50, 4, 0x66, XV_ART + 0x31000, 0x800 },
};
static const XvType skarab_types[] = { { 3, 0xC1, XV_TYPE_MOVED } };
static const uint16_t skarab_held[XV_VERSIONS * 16] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C8, 0x35C8, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C8, 0x34A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0x34A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0x34A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x34A3, 0x34A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

static const XVirusFamily bn5_skarab = {
	.name = "Skarab (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 18,
	.ai = 45,
	.first_id = XV_FIRST_ID + 0x4E,
	.home = XV_HOMES + 0x6800,
	.blocks = skarab_blocks, .nblocks = (int)(sizeof skarab_blocks / sizeof *skarab_blocks),
	.data = skarab_data, .ndata = (int)(sizeof skarab_data / sizeof *skarab_data),
	.calls = skarab_calls, .ncalls = (int)(sizeof skarab_calls / sizeof *skarab_calls),
	.words = skarab_words, .nwords = (int)(sizeof skarab_words / sizeof *skarab_words),
	.imms = skarab_imms, .nimms = (int)(sizeof skarab_imms / sizeof *skarab_imms),
	.songs = skarab_songs, .nsongs = (int)(sizeof skarab_songs / sizeof *skarab_songs),
	.states = skarab_states, .nstates = (int)(sizeof skarab_states / sizeof *skarab_states),
	.hooks = { OWN(0x0810A2E9), BN6(BN6_FAMILY_NULLSUB), OWN(0x0810A2DD), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810A088, .struct2 = 0x0810A090,
	.names = { 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C },
	.held_chips = skarab_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x12, .element = 0, .weakness = 0, .shadow = true },
	.sprites = skarab_sprites, .nsprites = (int)(sizeof skarab_sprites / sizeof *skarab_sprites),
	.types = skarab_types, .ntypes = (int)(sizeof skarab_types / sizeof *skarab_types),
};
/* ---- BN5's Draggin (AI 20 there, its enemy ids 0x73, 0x74, 0x75, 0x76, 0x77, 0x78), as AI 46 with ids 0x254-0x259 ---- */

static const XvBlock draggin_blocks[] = {
	{ 0x0810ACE0, 0x0810B080, XV_CODE + 0x8400, 0xB5C313FEu },
	{ 0x080D7270, 0x080D7378, XV_CODE + 0x8800, 0x94910498u },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0x8A02, 0x21492011u },
};
static const XvData draggin_data[] = {
	{ 0x0810ACEE, 0x0810ACF0, XV_RAW },
	{ 0x0810ACF0, 0x0810ACF4, XV_POOL },
	{ 0x0810ACF4, 0x0810AD04, XV_TABLE },
	{ 0x0810AD84, 0x0810AD8C, XV_POOL },
	{ 0x0810AD8C, 0x0810AD9C, XV_RAW },
	{ 0x0810AECE, 0x0810AED0, XV_RAW },
	{ 0x0810AED0, 0x0810AED8, XV_POOL },
	{ 0x0810AED8, 0x0810AF00, XV_ZERO },
	{ 0x0810AF14, 0x0810AF18, XV_POOL },
	{ 0x0810AF18, 0x0810AF24, XV_TABLE },
	{ 0x0810AF74, 0x0810AF78, XV_POOL },
	{ 0x0810AF78, 0x0810AF7E, XV_RAW },
	{ 0x0810AFA4, 0x0810AFA8, XV_POOL },
	{ 0x0810AFA8, 0x0810AFAE, XV_RAW },
	{ 0x0810B012, 0x0810B014, XV_RAW },
	{ 0x0810B014, 0x0810B018, XV_POOL },
	{ 0x0810B064, 0x0810B068, XV_POOL },
	{ 0x0810B068, 0x0810B078, XV_RAW },
	{ 0x0810B078, 0x0810B080, XV_POOL },
	{ 0x080D7282, 0x080D7284, XV_RAW },
	{ 0x080D7284, 0x080D7288, XV_POOL },
	{ 0x080D7288, 0x080D7294, XV_TABLE },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair draggin_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08012A6C, 0x08015C94 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800C6C2, 0x0800E2C2 },   /* map:calls+bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
	{ 0x08012FBC, 0x080161C4 },   /* near, SAME */
	{ 0x0800B544, 0x0800CF14 },   /* map:bytes, SAME */
};
static const XvPair draggin_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x0801629C, 0x08019B78 },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm draggin_imms[] = {
	{ 0x080D72A0, THUMB_MOVS, 2, 0, 0x40, 0x67 },   /* its sprite: list 4 0x40 in BN5, 0x67 in BN6 */
	{ 0x080D731A, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810AF98, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810AFBA, THUMB_MOVS, 0, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810AFBE, THUMB_MOVS, 0, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810AFCA, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810AFE4, THUMB_CMP, 4, 0, 0x6D, 0x9C },   /* a held chip compared: BN5's chip 109, BN6's 156 (the chip registry) */
};
static const XvRange draggin_ranges[] = {
	{ 0x0810AF54, 0x254, 0x259 },   /* NameIDs 0x73-0x78 in BN5, 0x254-0x259 in BN6 */
};
static const XvRef draggin_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810AF01), BN6(0x0810AE95), OWN(0x08104D87), OWN(0x0810ACE1),
};
static const XvSprite draggin_sprites[] = {
	{ 1, 0x14, 1, 0x2C, XV_ART + 0x32000, 0x2000 },   /* (LZ77 in BN5) */
	{ 4, 0x40, 4, 0x67, XV_ART + 0x34000, 0x2000 },
};
static const XvType draggin_types[] = { { 3, 0x87, XV_TYPE_MOVED } };
static const uint16_t draggin_held[XV_VERSIONS * 16] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x349C, 0x349C, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x349C, 0x349C, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35CB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35CB, 0x35CB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35CB, 0x35CB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

static const XVirusFamily bn5_draggin = {
	.name = "Draggin (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 20,
	.ai = 46,
	.first_id = XV_FIRST_ID + 0x54,
	.home = XV_HOMES + 0x7000,
	.blocks = draggin_blocks, .nblocks = (int)(sizeof draggin_blocks / sizeof *draggin_blocks),
	.data = draggin_data, .ndata = (int)(sizeof draggin_data / sizeof *draggin_data),
	.calls = draggin_calls, .ncalls = (int)(sizeof draggin_calls / sizeof *draggin_calls),
	.words = draggin_words, .nwords = (int)(sizeof draggin_words / sizeof *draggin_words),
	.imms = draggin_imms, .nimms = (int)(sizeof draggin_imms / sizeof *draggin_imms),
	.ranges = draggin_ranges, .nranges = (int)(sizeof draggin_ranges / sizeof *draggin_ranges),
	.states = draggin_states, .nstates = (int)(sizeof draggin_states / sizeof *draggin_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810ABF4, .struct2 = 0x0810ABFC,
	.names = { 0x73, 0x74, 0x75, 0x76, 0x77, 0x78 },
	.held_chips = draggin_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x14, .element = 0, .weakness = 0, .shadow = true },
	.sprites = draggin_sprites, .nsprites = (int)(sizeof draggin_sprites / sizeof *draggin_sprites),
	.types = draggin_types, .ntypes = (int)(sizeof draggin_types / sizeof *draggin_types),
};
/* ---- BN5's Whirly (AI 21 there, its enemy ids 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e), as AI 47 with ids 0x25a-0x25f ---- */

static const XvBlock whirly_blocks[] = {
	{ 0x0810B16C, 0x0810B824, XV_CODE + 0x8C00, 0x86808BABu },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0x9402, 0xA53A3BEFu },
	{ 0x08104BBC, 0x08104BFC, XV_CODE + 0x9600, 0x838CAF0Du },
};
static const XvData whirly_data[] = {
	{ 0x0810B17A, 0x0810B17C, XV_RAW },
	{ 0x0810B17C, 0x0810B180, XV_POOL },
	{ 0x0810B180, 0x0810B194, XV_TABLE },
	{ 0x0810B2EC, 0x0810B2F0, XV_POOL },
	{ 0x0810B2F0, 0x0810B2FC, XV_TABLE },
	{ 0x0810B53C, 0x0810B53E, XV_RAW },
	{ 0x0810B582, 0x0810B584, XV_RAW },
	{ 0x0810B584, 0x0810B588, XV_POOL },
	{ 0x0810B588, 0x0810B58C, XV_RAW },
	{ 0x0810B58C, 0x0810B590, XV_POOL },
	{ 0x0810B590, 0x0810B5A0, XV_RAW },
	{ 0x0810B5A0, 0x0810B5A4, XV_POOL },
	{ 0x0810B5A4, 0x0810B5B4, XV_RAW },
	{ 0x0810B5B4, 0x0810B5B8, XV_POOL },
	{ 0x0810B5B8, 0x0810B5C0, XV_RAW },
	{ 0x0810B5C0, 0x0810B5E8, XV_ZERO },
	{ 0x0810B630, 0x0810B634, XV_POOL },
	{ 0x0810B634, 0x0810B638, XV_TABLE },
	{ 0x0810B684, 0x0810B688, XV_POOL },
	{ 0x0810B688, 0x0810B694, XV_RAW },
	{ 0x0810B77A, 0x0810B77C, XV_RAW },
	{ 0x0810B77C, 0x0810B780, XV_POOL },
	{ 0x0810B780, 0x0810B788, XV_RAW },
	{ 0x0810B788, 0x0810B78C, XV_POOL },
	{ 0x0810B78C, 0x0810B792, XV_RAW },
	{ 0x0810B7C4, 0x0810B7CC, XV_POOL },
	{ 0x0810B7CC, 0x0810B7D2, XV_RAW },
	{ 0x0810B7D2, 0x0810B7D8, XV_RAW },
	{ 0x0810B808, 0x0810B810, XV_POOL },
	{ 0x0810B810, 0x0810B820, XV_RAW },
	{ 0x0810B820, 0x0810B824, XV_POOL },
	{ 0x08104BF4, 0x08104BF8, XV_RAW },
	{ 0x08104BF8, 0x08104BFC, XV_POOL },
};
static const XvPair whirly_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08104B80, 0x0810A9D4 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800B688, 0x0800D086 },   /* map:bytes, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800CE54, 0x0800EBD4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x0800D250, 0x0800F334 },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x080133C4, 0x080165B8 },   /* project:research, DIFF (audited) */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800D010, 0x0800ED90 },   /* map:calls, DIFF (audited) */
	{ 0x08008EC6, 0x0800A704 },   /* map:bytes, SAME */
	{ 0x0800FE6A, 0x080126E4 },   /* project:research, DIFF (audited) */
	{ 0x0800D23E, 0x0800F322 },   /* map:bytes, SAME */
};
static const XvImm whirly_imms[] = {
	{ 0x08104BE2, THUMB_LDR, 0, 3, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810B22A, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810B260, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810B47E, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810B498, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810B76E, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810B7BC, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber whirly_numbers[] = {
	{ 0x08104BF8, 0x08000000, 0x08000000 },
};
static const XvByte whirly_bytes[] = {
	{ 0x0810B589, 0x15, 0x2D },   /* a sprite of its table 0810B588: list 1 0x15 in BN5, 0x2d in BN6 */
	{ 0x0810B58B, 0x08, 0x68 },   /* a sprite of its table 0810B588: list 4 0x8 in BN5, 0x68 in BN6 */
};
static const XvRange whirly_ranges[] = {
	{ 0x0810B51A, 0x25A, 0x25F },   /* NameIDs 0x79-0x7e in BN5, 0x25a-0x25f in BN6 */
};
static const XvRef whirly_states[] = {
	BN6(0x08016381), OWN(0x0810B5E9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810B61D), BN6(0x0810AE95), BN6(0x0810AB35), OWN(0x0810B16D),
};
static const XvSprite whirly_sprites[] = {
	{ 1, 0x15, 1, 0x2D, XV_ART + 0x36000, 0x2000 },   /* (LZ77 in BN5) */
	{ 4, 0x08, 4, 0x68, XV_ART + 0x38000, 0x1000 },
};
static const uint16_t whirly_held[XV_VERSIONS * 16] = {
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C2, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C2, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C2, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C2, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
	0x35C2, 0x35C2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
};

static const XVirusFamily bn5_whirly = {
	.name = "Whirly (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 21,
	.ai = 47,
	.first_id = XV_FIRST_ID + 0x5A,
	.home = XV_HOMES + 0x7800,
	.blocks = whirly_blocks, .nblocks = (int)(sizeof whirly_blocks / sizeof *whirly_blocks),
	.data = whirly_data, .ndata = (int)(sizeof whirly_data / sizeof *whirly_data),
	.calls = whirly_calls, .ncalls = (int)(sizeof whirly_calls / sizeof *whirly_calls),
	.imms = whirly_imms, .nimms = (int)(sizeof whirly_imms / sizeof *whirly_imms),
	.numbers = whirly_numbers, .nnumbers = (int)(sizeof whirly_numbers / sizeof *whirly_numbers),
	.bytes = whirly_bytes, .nbytes = (int)(sizeof whirly_bytes / sizeof *whirly_bytes),
	.ranges = whirly_ranges, .nranges = (int)(sizeof whirly_ranges / sizeof *whirly_ranges),
	.states = whirly_states, .nstates = (int)(sizeof whirly_states / sizeof *whirly_states),
	.hooks = { OWN(0x0810B695), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810B080, .struct2 = 0x0810B088,
	.names = { 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E },
	.held_chips = whirly_held,
	.ice = 0,
	.traits = { .laid = true, .mode = 0x15, .element = 0, .weakness = 0, .shadow = true },
	.sprites = whirly_sprites, .nsprites = (int)(sizeof whirly_sprites / sizeof *whirly_sprites),
};
/* ---- BN5's Marina (AI 22 there, its enemy ids 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84), as AI 48 with ids 0x260-0x265 ---- */

static const XvBlock marina_blocks[] = {
	{ 0x0810B910, 0x0810BC5C, XV_CODE + 0x9800, 0xE2AF121Cu },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0x9C02, 0xA53A3BEFu },
	{ 0x080C5386, 0x080C53DE, XV_CODE + 0x9E02, 0x4A1B50AFu },
	{ 0x080C5DF4, 0x080C6000, XV_CODE + 0xA000, 0xEDB80F1Au },
	{ 0x080D856C, 0x080D87D0, XV_CODE + 0xA400, 0x7524551Eu },
	{ 0x08104C0C, 0x08104C1C, XV_CODE + 0xA800, 0x4F85166Bu },
};
static const XvData marina_data[] = {
	{ 0x0810B91E, 0x0810B920, XV_RAW },
	{ 0x0810B920, 0x0810B924, XV_POOL },
	{ 0x0810B924, 0x0810B92C, XV_TABLE },
	{ 0x0810B99E, 0x0810B9A0, XV_RAW },
	{ 0x0810B9A0, 0x0810B9A4, XV_POOL },
	{ 0x0810B9A4, 0x0810B9CC, XV_ZERO },
	{ 0x0810BA44, 0x0810BA48, XV_POOL },
	{ 0x0810BA48, 0x0810BA58, XV_TABLE },
	{ 0x0810BB32, 0x0810BB34, XV_RAW },
	{ 0x0810BB34, 0x0810BB38, XV_POOL },
	{ 0x0810BB38, 0x0810BB3E, XV_RAW },
	{ 0x0810BBA4, 0x0810BBB0, XV_POOL },
	{ 0x0810BBB0, 0x0810BBB6, XV_RAW },
	{ 0x0810BBB6, 0x0810BBBC, XV_RAW },
	{ 0x0810BBBC, 0x0810BBC2, XV_RAW },
	{ 0x0810BC3A, 0x0810BC3C, XV_RAW },
	{ 0x0810BC3C, 0x0810BC40, XV_POOL },
	{ 0x0810BC40, 0x0810BC50, XV_RAW },
	{ 0x0810BC50, 0x0810BC5C, XV_POOL },
	{ 0x080C5DF4, 0x080C5EB4, XV_RAW },
	{ 0x080C5EC6, 0x080C5EC8, XV_RAW },
	{ 0x080C5EC8, 0x080C5ECC, XV_POOL },
	{ 0x080C5ECC, 0x080C5ED8, XV_TABLE },
	{ 0x080C5FE8, 0x080C5FFC, XV_ZERO },
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
};
static const XvPair marina_calls[] = {
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
	{ 0x0800F2A6, 0x0801171C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800E096, 0x08010312 },   /* map:bytes, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
};
static const XvPair marina_words[] = {
	{ 0x080138EB, 0x08016C8B },
};
static const XvImm marina_imms[] = {
	{ 0x080C5F74, THUMB_LDR, 1, 0, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080D85AA, THUMB_MOVS, 2, 0, 0x41, 0x69 },   /* its sprite: list 4 0x41 in BN5, 0x69 in BN6 */
	{ 0x080D869A, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810BA28, THUMB_MOVS, 4, 0, 0x5D, 0x73 },   /* its effect: 0x5d in BN5, 0x73 in BN6 */
	{ 0x0810BA80, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810BAE8, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810BB1A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810BB44, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810BB94, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber marina_numbers[] = {
	{ 0x0810B9A0, 0x03800000, 0x03800000 },
};
static const XvByte marina_bytes[] = {
	{ 0x080C5E6A, 0x41, 0x69 },   /* a sprite of its table 080C5DF4: list 4 0x41 in BN5, 0x69 in BN6 */
	{ 0x080C5E7A, 0x41, 0x69 },   /* a sprite of its table 080C5DF4: list 4 0x41 in BN5, 0x69 in BN6 */
	{ 0x080C5E8A, 0x41, 0x69 },   /* a sprite of its table 080C5DF4: list 4 0x41 in BN5, 0x69 in BN6 */
	{ 0x080C5E9A, 0x41, 0x69 },   /* a sprite of its table 080C5DF4: list 4 0x41 in BN5, 0x69 in BN6 */
};
static const XvRange marina_ranges[] = {
	{ 0x0810B958, 0x260, 0x265 },   /* NameIDs 0x7f-0x84 in BN5, 0x260-0x265 in BN6 */
};
static const XvRef marina_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810BA31), OWN(0x08104C0D), BN6(0x0810AB35), OWN(0x0810B911),
};
static const XvSprite marina_sprites[] = {
	{ 1, 0x16, 1, 0x2E, XV_ART + 0x39000, 0x1000 },   /* (LZ77 in BN5) */
	{ 4, 0x41, 4, 0x69, XV_ART + 0x3A000, 0x1000 },
	{ 5, 0x1E, 5, 0x21, XV_ART + 0x3B000, 0xC00 },
};
static const XvId marina_effects[] = { { 0x5D, 0x73 } };
static const XvType marina_types[] = { { 3, 0x0B, XV_TYPE_MOVED }, { 3, 0x8F, XV_TYPE_MOVED } };

static const XVirusFamily bn5_marina = {
	.name = "Marina (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 22,
	.ai = 48,
	.first_id = XV_FIRST_ID + 0x60,
	.home = XV_HOMES + 0x8000,
	.blocks = marina_blocks, .nblocks = (int)(sizeof marina_blocks / sizeof *marina_blocks),
	.data = marina_data, .ndata = (int)(sizeof marina_data / sizeof *marina_data),
	.calls = marina_calls, .ncalls = (int)(sizeof marina_calls / sizeof *marina_calls),
	.words = marina_words, .nwords = (int)(sizeof marina_words / sizeof *marina_words),
	.imms = marina_imms, .nimms = (int)(sizeof marina_imms / sizeof *marina_imms),
	.numbers = marina_numbers, .nnumbers = (int)(sizeof marina_numbers / sizeof *marina_numbers),
	.bytes = marina_bytes, .nbytes = (int)(sizeof marina_bytes / sizeof *marina_bytes),
	.ranges = marina_ranges, .nranges = (int)(sizeof marina_ranges / sizeof *marina_ranges),
	.states = marina_states, .nstates = (int)(sizeof marina_states / sizeof *marina_states),
	.hooks = { OWN(0x0810B9D7), BN6(BN6_FAMILY_NULLSUB), OWN(0x0810B9CD), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810B824, .struct2 = 0x0810B82C,
	.names = { 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x16, .element = 0, .weakness = 0, .shadow = true },
	.sprites = marina_sprites, .nsprites = (int)(sizeof marina_sprites / sizeof *marina_sprites),
	.effects = marina_effects, .neffects = (int)(sizeof marina_effects / sizeof *marina_effects),
	.types = marina_types, .ntypes = (int)(sizeof marina_types / sizeof *marina_types),
};
/* ---- BN5's Flashy (AI 23 there, its enemy ids 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a), as AI 49 with ids 0x266-0x26b ---- */

static const XvBlock flashy_blocks[] = {
	{ 0x0810BD48, 0x0810C1C4, XV_CODE + 0xAA00, 0x8EAEFDA1u },
	{ 0x080D3B50, 0x080D3CDA, XV_CODE + 0xB000, 0xE027226Cu },
	{ 0x08104B4C, 0x08104B80, XV_CODE + 0xB200, 0xD74B7D5Eu },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0xB402, 0x21492011u },
};
static const XvData flashy_data[] = {
	{ 0x0810BD56, 0x0810BD58, XV_RAW },
	{ 0x0810BD58, 0x0810BD5C, XV_POOL },
	{ 0x0810BD5C, 0x0810BD78, XV_TABLE },
	{ 0x0810C016, 0x0810C018, XV_RAW },
	{ 0x0810C018, 0x0810C020, XV_POOL },
	{ 0x0810C020, 0x0810C030, XV_RAW },
	{ 0x0810C030, 0x0810C034, XV_POOL },
	{ 0x0810C034, 0x0810C044, XV_RAW },
	{ 0x0810C044, 0x0810C04C, XV_POOL },
	{ 0x0810C04C, 0x0810C074, XV_ZERO },
	{ 0x0810C088, 0x0810C08C, XV_POOL },
	{ 0x0810C08C, 0x0810C094, XV_TABLE },
	{ 0x0810C0BA, 0x0810C0BC, XV_RAW },
	{ 0x0810C0BC, 0x0810C0C4, XV_POOL },
	{ 0x0810C0C4, 0x0810C0DC, XV_POOL },
	{ 0x0810C0DC, 0x0810C0E2, XV_RAW },
	{ 0x0810C0E2, 0x0810C0F4, XV_RAW },
	{ 0x0810C0F4, 0x0810C100, XV_RAW },
	{ 0x0810C19C, 0x0810C1A8, XV_POOL },
	{ 0x0810C1A8, 0x0810C1AE, XV_RAW },
	{ 0x0810C1AE, 0x0810C1B4, XV_RAW },
	{ 0x0810C1B4, 0x0810C1BC, XV_RAW },
	{ 0x0810C1BC, 0x0810C1C4, XV_POOL },
	{ 0x080D3B62, 0x080D3B64, XV_RAW },
	{ 0x080D3B64, 0x080D3B68, XV_POOL },
	{ 0x080D3B68, 0x080D3B74, XV_TABLE },
	{ 0x080D3C54, 0x080D3C58, XV_POOL },
	{ 0x080D3C58, 0x080D3C60, XV_TABLE },
	{ 0x08104B7A, 0x08104B7C, XV_RAW },
	{ 0x08104B7C, 0x08104B80, XV_POOL },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair flashy_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800D250, 0x0800F334 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x08012C1E, 0x08015E46 },   /* map:bytes, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08012DA4, 0x08015FB4 },   /* project:research, SAME */
	{ 0x0800B494, 0x0800CE64 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
};
static const XvPair flashy_words[] = {
	{ 0x08012A21, 0x08015C49 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x0801629C, 0x08019B78 },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm flashy_imms[] = {
	{ 0x080D3B80, THUMB_MOVS, 2, 0, 0x3E, 0x6A },   /* its sprite: list 4 0x3e in BN5, 0x6a in BN6 */
	{ 0x080D3C0E, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810BDBC, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810BDE4, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810BF04, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810BF1E, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810C000, THUMB_MOVS, 0, 0, 0xBA, 0x60 },   /* its sound 0xba: BN5's copied into BN6's empty slot 0x60 */
	{ 0x0810C09E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810C11C, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810C142, THUMB_MOVS, 0, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810C146, THUMB_MOVS, 0, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810C152, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810C190, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
};
static const XvRange flashy_ranges[] = {
	{ 0x0810C160, 0x266, 0x26B },   /* NameIDs 0x85-0x8a in BN5, 0x266-0x26b in BN6 */
};
static const XvSong flashy_songs[] = {
	{ 0xBA, 0x60, XV_ART + 0x3E000, 0x100 },   /* its sound 0xba, BN5's song copied */
};
static const XvRef flashy_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810C075), BN6(0x0810AE95), OWN(0x08104D87), OWN(0x0810BD49),
};
static const XvSprite flashy_sprites[] = {
	{ 1, 0x17, 1, 0x2F, XV_ART + 0x3C000, 0x1000 },
	{ 4, 0x3E, 4, 0x6A, XV_ART + 0x3D000, 0x1000 },
};
static const XvType flashy_types[] = { { 3, 0x6F, XV_TYPE_MOVED } };

static const XVirusFamily bn5_flashy = {
	.name = "Flashy (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 23,
	.ai = 49,
	.first_id = XV_FIRST_ID + 0x66,
	.home = XV_HOMES + 0x8800,
	.blocks = flashy_blocks, .nblocks = (int)(sizeof flashy_blocks / sizeof *flashy_blocks),
	.data = flashy_data, .ndata = (int)(sizeof flashy_data / sizeof *flashy_data),
	.calls = flashy_calls, .ncalls = (int)(sizeof flashy_calls / sizeof *flashy_calls),
	.words = flashy_words, .nwords = (int)(sizeof flashy_words / sizeof *flashy_words),
	.imms = flashy_imms, .nimms = (int)(sizeof flashy_imms / sizeof *flashy_imms),
	.ranges = flashy_ranges, .nranges = (int)(sizeof flashy_ranges / sizeof *flashy_ranges),
	.songs = flashy_songs, .nsongs = (int)(sizeof flashy_songs / sizeof *flashy_songs),
	.states = flashy_states, .nstates = (int)(sizeof flashy_states / sizeof *flashy_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810BC5C, .struct2 = 0x0810BC64,
	.names = { 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x17, .element = 0, .weakness = 0, .shadow = true },
	.sprites = flashy_sprites, .nsprites = (int)(sizeof flashy_sprites / sizeof *flashy_sprites),
	.types = flashy_types, .ntypes = (int)(sizeof flashy_types / sizeof *flashy_types),
};
/* ---- BN5's Eleogre (AI 24 there, its enemy ids 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90), as AI 50 with ids 0x26c-0x271 ---- */

static const XvBlock eleogre_blocks[] = {
	{ 0x0810C2B0, 0x0810C780, XV_CODE + 0xB600, 0x01260ACAu },
	{ 0x080D3CBC, 0x080D3CDA, XV_CODE + 0xBC00, 0x7A9ADD7Bu },
	{ 0x08104B96, 0x08104BBC, XV_CODE + 0xBE02, 0x8F659214u },
};
static const XvData eleogre_data[] = {
	{ 0x0810C2BE, 0x0810C2C0, XV_RAW },
	{ 0x0810C2C0, 0x0810C2C4, XV_POOL },
	{ 0x0810C2C4, 0x0810C2D8, XV_TABLE },
	{ 0x0810C468, 0x0810C46C, XV_POOL },
	{ 0x0810C46C, 0x0810C47C, XV_RAW },
	{ 0x0810C47C, 0x0810C480, XV_POOL },
	{ 0x0810C48E, 0x0810C490, XV_RAW },
	{ 0x0810C490, 0x0810C494, XV_POOL },
	{ 0x0810C494, 0x0810C4A4, XV_TABLE },
	{ 0x0810C598, 0x0810C5C0, XV_ZERO },
	{ 0x0810C5D4, 0x0810C5D8, XV_POOL },
	{ 0x0810C5D8, 0x0810C5E8, XV_TABLE },
	{ 0x0810C654, 0x0810C65C, XV_POOL },
	{ 0x0810C65C, 0x0810C662, XV_RAW },
	{ 0x0810C662, 0x0810C668, XV_RAW },
	{ 0x0810C6BE, 0x0810C6C0, XV_RAW },
	{ 0x0810C6C0, 0x0810C6C8, XV_POOL },
	{ 0x0810C6C8, 0x0810C6CE, XV_RAW },
	{ 0x0810C6CE, 0x0810C6D4, XV_RAW },
	{ 0x0810C766, 0x0810C768, XV_RAW },
	{ 0x0810C768, 0x0810C76C, XV_POOL },
	{ 0x0810C76C, 0x0810C77C, XV_RAW },
	{ 0x0810C77C, 0x0810C780, XV_POOL },
};
static const XvPair eleogre_calls[] = {
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08104C38, 0x0810AB34 },   /* search, SAME */
	{ 0x08104B1C, 0x0810A960 },   /* project:handi, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08012C1E, 0x08015E46 },   /* map:bytes, SAME */
};
static const XvPair eleogre_words[] = {
	{ 0x08104BFD, 0x0810AE95 },
};
static const XvImm eleogre_imms[] = {
	{ 0x0810C368, THUMB_MOVS, 0, 0, 0xBA, 0x60 },   /* its sound 0xba: BN5's copied into BN6's empty slot 0x60 */
	{ 0x0810C510, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810C54A, THUMB_MOVS, 4, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810C5EE, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810C63E, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810C6AA, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810C6F2, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvRange eleogre_ranges[] = {
	{ 0x0810C670, 0x26C, 0x271 },   /* NameIDs 0x8b-0x90 in BN5, 0x26c-0x271 in BN6 */
};
static const XvRef eleogre_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810C5C1), BN6(0x0810AE95), OWN(0x0810C2B1), BN6(0x0810AB35),
};
static const XvSprite eleogre_sprites[] = {
	{ 1, 0x18, 1, 0x30, XV_ART + 0x3E800, 0x2000 },   /* (LZ77 in BN5) */
};
static const XvType eleogre_types[] = { { 3, 0x6F, XV_TYPE_SHARED } };

static const XVirusFamily bn5_eleogre = {
	.name = "Eleogre (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 24,
	.ai = 50,
	.first_id = XV_FIRST_ID + 0x6C,
	.home = XV_HOMES + 0x9000,
	.blocks = eleogre_blocks, .nblocks = (int)(sizeof eleogre_blocks / sizeof *eleogre_blocks),
	.data = eleogre_data, .ndata = (int)(sizeof eleogre_data / sizeof *eleogre_data),
	.calls = eleogre_calls, .ncalls = (int)(sizeof eleogre_calls / sizeof *eleogre_calls),
	.words = eleogre_words, .nwords = (int)(sizeof eleogre_words / sizeof *eleogre_words),
	.imms = eleogre_imms, .nimms = (int)(sizeof eleogre_imms / sizeof *eleogre_imms),
	.ranges = eleogre_ranges, .nranges = (int)(sizeof eleogre_ranges / sizeof *eleogre_ranges),
	.states = eleogre_states, .nstates = (int)(sizeof eleogre_states / sizeof *eleogre_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810C1C4, .struct2 = 0x0810C1CC,
	.names = { 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x18, .element = 0, .weakness = 0, .shadow = true },
	.sprites = eleogre_sprites, .nsprites = (int)(sizeof eleogre_sprites / sizeof *eleogre_sprites),
	.types = eleogre_types, .ntypes = (int)(sizeof eleogre_types / sizeof *eleogre_types),
};
/* ---- BN5's Cactikil (AI 25 there, its enemy ids 0x91, 0x92, 0x93, 0x94, 0x95, 0x96), as AI 51 with ids 0x272-0x277 ---- */

static const XvBlock cactikil_blocks[] = {
	{ 0x0810C86C, 0x0810CC8C, XV_CODE + 0xC000, 0x940C91A4u },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0xC602, 0xA53A3BEFu },
	{ 0x080BB9C4, 0x080BBB88, XV_CODE + 0xC800, 0xF1AE1059u },
	{ 0x080C5386, 0x080C53DE, XV_CODE + 0xCA02, 0x4A1B50AFu },
	{ 0x08104BBC, 0x08104BFC, XV_CODE + 0xCC00, 0x838CAF0Du },
};
static const XvData cactikil_data[] = {
	{ 0x0810C87A, 0x0810C87C, XV_RAW },
	{ 0x0810C87C, 0x0810C880, XV_POOL },
	{ 0x0810C880, 0x0810C898, XV_TABLE },
	{ 0x0810CAC0, 0x0810CAD0, XV_POOL },
	{ 0x0810CAD0, 0x0810CAF0, XV_ZERO },
	{ 0x0810CB66, 0x0810CB68, XV_RAW },
	{ 0x0810CB68, 0x0810CB6C, XV_POOL },
	{ 0x0810CB6C, 0x0810CB74, XV_RAW },
	{ 0x0810CBB8, 0x0810CBBC, XV_POOL },
	{ 0x0810CBBC, 0x0810CBC8, XV_TABLE },
	{ 0x0810CC00, 0x0810CC04, XV_POOL },
	{ 0x0810CC04, 0x0810CC10, XV_RAW },
	{ 0x0810CC4E, 0x0810CC50, XV_RAW },
	{ 0x0810CC50, 0x0810CC54, XV_POOL },
	{ 0x0810CC54, 0x0810CC6C, XV_RAW },
	{ 0x0810CC6C, 0x0810CC70, XV_POOL },
	{ 0x0810CC70, 0x0810CC88, XV_RAW },
	{ 0x0810CC88, 0x0810CC8C, XV_POOL },
	{ 0x080BB9E6, 0x080BB9E8, XV_RAW },
	{ 0x080BB9E8, 0x080BB9EC, XV_POOL },
	{ 0x080BB9EC, 0x080BB9F8, XV_TABLE },
	{ 0x080BBAC2, 0x080BBAC4, XV_RAW },
	{ 0x080BBAC4, 0x080BBAC8, XV_POOL },
	{ 0x080BBAC8, 0x080BBACC, XV_TABLE },
	{ 0x080BBB78, 0x080BBB7C, XV_POOL },
	{ 0x080BBB7C, 0x080BBB84, XV_RAW },
	{ 0x080BBB84, 0x080BBB88, XV_POOL },
	{ 0x08104BF4, 0x08104BF8, XV_RAW },
	{ 0x08104BF8, 0x08104BFC, XV_POOL },
};
static const XvPair cactikil_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x08013B04, 0x08016E64 },   /* map:calls, DIFF (audited) */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080E1BFE, 0x080E2DAA },   /* map:bytes, SAME */
	{ 0x08002BF8, 0x08002D8C },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08002D40, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002C34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002C20, 0x08002DB4 },   /* near, SAME */
	{ 0x08002DA2, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002D66, 0x08002F02 },   /* near, SAME */
	{ 0x08002B4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002AEA, 0x08002C7E },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08104B80, 0x0810A9D4 },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x0800D250, 0x0800F334 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x080133CE, 0x080165C2 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
};
static const XvPair cactikil_words[] = {
	{ 0x080138AF, 0x08016C4F },
};
static const XvImm cactikil_imms[] = {
	{ 0x080BB9FC, THUMB_MOVS, 2, 0, 0x19, 0x31 },   /* its sprite: list 1 0x19 in BN5, 0x31 in BN6 */
	{ 0x08104BE2, THUMB_LDR, 0, 3, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810CA24, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810CAF6, THUMB_MOVS, 2, 0, 0x30, 0x6B },   /* its sprite: list 4 0x30 in BN5, 0x6b in BN6 */
	{ 0x0810CC3E, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber cactikil_numbers[] = {
	{ 0x08104BF8, 0x08000000, 0x08000000 },
	{ 0x0810CAC4, 0x03800000, 0x03800000 },
};
static const XvRef cactikil_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x0810CB75), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810CBA5), OWN(0x0810C86D),
};
static const XvSprite cactikil_sprites[] = {
	{ 1, 0x19, 1, 0x31, XV_ART + 0x40800, 0x1000 },   /* (LZ77 in BN5) */
	{ 4, 0x30, 4, 0x6B, XV_ART + 0x41800, 0x1800 },
};
static const XvType cactikil_types[] = { { 1, 0x11, XV_TYPE_MOVED } };

static const XVirusFamily bn5_cactikil = {
	.name = "Cactikil (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 25,
	.ai = 51,
	.first_id = XV_FIRST_ID + 0x72,
	.home = XV_HOMES + 0x9800,
	.blocks = cactikil_blocks, .nblocks = (int)(sizeof cactikil_blocks / sizeof *cactikil_blocks),
	.data = cactikil_data, .ndata = (int)(sizeof cactikil_data / sizeof *cactikil_data),
	.calls = cactikil_calls, .ncalls = (int)(sizeof cactikil_calls / sizeof *cactikil_calls),
	.words = cactikil_words, .nwords = (int)(sizeof cactikil_words / sizeof *cactikil_words),
	.imms = cactikil_imms, .nimms = (int)(sizeof cactikil_imms / sizeof *cactikil_imms),
	.numbers = cactikil_numbers, .nnumbers = (int)(sizeof cactikil_numbers / sizeof *cactikil_numbers),
	.states = cactikil_states, .nstates = (int)(sizeof cactikil_states / sizeof *cactikil_states),
	.hooks = { OWN(0x0810CB95), BN6(BN6_FAMILY_NULLSUB), OWN(0x0810CAF1), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810C780, .struct2 = 0x0810C788,
	.names = { 0x91, 0x92, 0x93, 0x94, 0x95, 0x96 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x19, .element = 0, .weakness = 0, .shadow = true },
	.sprites = cactikil_sprites, .nsprites = (int)(sizeof cactikil_sprites / sizeof *cactikil_sprites),
	.types = cactikil_types, .ntypes = (int)(sizeof cactikil_types / sizeof *cactikil_types),
};
/* ---- BN5's WuNote (AI 26 there, its enemy ids 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c), as AI 52 with ids 0x278-0x27d ---- */

static const XvBlock wunote_blocks[] = {
	{ 0x0810CD78, 0x0810D290, XV_CODE + 0xCE00, 0x49C8083Bu },
	{ 0x080BC6EC, 0x080BC82E, XV_CODE + 0xD400, 0x5569E6C2u },
	{ 0x080C5386, 0x080C53DE, XV_CODE + 0xD602, 0x4A1B50AFu },
	{ 0x080D6C90, 0x080D6EB0, XV_CODE + 0xD800, 0xAD752C27u },
	{ 0x08104B4C, 0x08104B80, XV_CODE + 0xDC00, 0xD74B7D5Eu },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0xDE02, 0x21492011u },
};
static const XvData wunote_data[] = {
	{ 0x0810CD86, 0x0810CD88, XV_RAW },
	{ 0x0810CD88, 0x0810CD8C, XV_POOL },
	{ 0x0810CD8C, 0x0810CD94, XV_TABLE },
	{ 0x0810CE7E, 0x0810CE80, XV_RAW },
	{ 0x0810CE80, 0x0810CE84, XV_POOL },
	{ 0x0810CE84, 0x0810CE94, XV_TABLE },
	{ 0x0810D0A4, 0x0810D0AC, XV_POOL },
	{ 0x0810D0AC, 0x0810D0BC, XV_RAW },
	{ 0x0810D0BC, 0x0810D0C4, XV_POOL },
	{ 0x0810D0C4, 0x0810D0F0, XV_ZERO },
	{ 0x0810D124, 0x0810D128, XV_POOL },
	{ 0x0810D128, 0x0810D140, XV_TABLE },
	{ 0x0810D1A6, 0x0810D1A8, XV_RAW },
	{ 0x0810D1A8, 0x0810D1AC, XV_POOL },
	{ 0x0810D1AC, 0x0810D1B2, XV_RAW },
	{ 0x0810D1CA, 0x0810D1CC, XV_RAW },
	{ 0x0810D1CC, 0x0810D1D0, XV_POOL },
	{ 0x0810D1D0, 0x0810D1DC, XV_RAW },
	{ 0x0810D278, 0x0810D27C, XV_POOL },
	{ 0x0810D27C, 0x0810D28C, XV_RAW },
	{ 0x0810D28C, 0x0810D290, XV_POOL },
	{ 0x080BC6FE, 0x080BC700, XV_RAW },
	{ 0x080BC700, 0x080BC704, XV_POOL },
	{ 0x080BC704, 0x080BC710, XV_TABLE },
	{ 0x080BC7B4, 0x080BC7B8, XV_POOL },
	{ 0x080BC7B8, 0x080BC7BC, XV_TABLE },
	{ 0x080D6CA2, 0x080D6CA4, XV_RAW },
	{ 0x080D6CA4, 0x080D6CA8, XV_POOL },
	{ 0x080D6CA8, 0x080D6CB4, XV_TABLE },
	{ 0x080D6DD4, 0x080D6DD8, XV_POOL },
	{ 0x080D6DD8, 0x080D6DE0, XV_TABLE },
	{ 0x080D6EAC, 0x080D6EB0, XV_POOL },
	{ 0x08104B7A, 0x08104B7C, XV_RAW },
	{ 0x08104B7C, 0x08104B80, XV_POOL },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair wunote_calls[] = {
	{ 0x08013B04, 0x08016E64 },   /* map:calls, DIFF (audited) */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x08002BF8, 0x08002D8C },   /* map:bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x08002D40, 0x08002EDC },   /* map:bytes, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x08002C34, 0x08002DC8 },   /* map:bytes, SAME */
	{ 0x08002C20, 0x08002DB4 },   /* near, SAME */
	{ 0x08002DA2, 0x08002F3E },   /* map:bytes, SAME */
	{ 0x08002D66, 0x08002F02 },   /* near, SAME */
	{ 0x08002B4C, 0x08002CE0 },   /* map:bytes, SAME */
	{ 0x08002AEA, 0x08002C7E },   /* map:bytes, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x0800CE1E, 0x0800EB9E },   /* map:calls, LAYOUT */
	{ 0x0800C7CC, 0x0800E3BE },   /* map:calls, DIFF (audited) */
	{ 0x08002C44, 0x08002DD8 },   /* map:bytes, SAME */
	{ 0x08002C1C, 0x08002DB0 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080187A2, 0x0801BCA6 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x080138EA, 0x08016C8A },   /* project:cangard, DIFF (audited) */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800B6BE, 0x0800D0BC },   /* map:bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800D250, 0x0800F334 },   /* map:bytes, SAME */
	{ 0x080B9CAE, XN_OBJECTS + 0x252 },   /* the moved objects, SAME */
	{ 0x080B9CFA, 0x080BA6EC },   /* search, by place, SAME */
	{ 0x080B9CD6, 0x080BA6C8 },   /* search, by place, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x080133CE, 0x080165C2 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800B544, 0x0800CF14 },   /* map:bytes, SAME */
};
static const XvPair wunote_words[] = {
	{ 0x080138AF, 0x08016C4F },
	{ 0x0801629C, 0x08019B78 },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm wunote_imms[] = {
	{ 0x080BC714, THUMB_MOVS, 2, 0, 0x1A, 0x32 },   /* its sprite: list 1 0x1a in BN5, 0x32 in BN6 */
	{ 0x080D6CE6, THUMB_MOVS, 2, 0, 0x1A, 0x32 },   /* its sprite: list 1 0x1a in BN5, 0x32 in BN6 */
	{ 0x080D6D8A, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810D146, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810D162, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810D174, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810D19A, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810D1BE, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810D1F2, THUMB_MOVS, 0, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810D1F6, THUMB_MOVS, 0, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810D206, THUMB_MOVS, 0, 0, 0xA, 0xC },   /* CurAction 10 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber wunote_numbers[] = {
	{ 0x080D6EAC, 0x0A800000, 0x0A800000 },
	{ 0x0810D0A4, 0x04050401, 0x04050401 },
};
static const XvRef wunote_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x0810D0F1), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810D111), BN6(0x0810AE95), OWN(0x0810CD79), OWN(0x0810CE71), OWN(0x08104D87),
};
static const XvSprite wunote_sprites[] = {
	{ 1, 0x1A, 1, 0x32, XV_ART + 0x43000, 0x2000 },   /* (LZ77 in BN5) */
};
static const XvType wunote_types[] = { { 1, 0x05, XV_TYPE_SHARED }, { 1, 0x16, XV_TYPE_MOVED }, { 3, 0x84, XV_TYPE_MOVED } };

static const XVirusFamily bn5_wunote = {
	.name = "WuNote (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 26,
	.ai = 52,
	.first_id = XV_FIRST_ID + 0x78,
	.home = XV_HOMES + 0xA000,
	.blocks = wunote_blocks, .nblocks = (int)(sizeof wunote_blocks / sizeof *wunote_blocks),
	.data = wunote_data, .ndata = (int)(sizeof wunote_data / sizeof *wunote_data),
	.calls = wunote_calls, .ncalls = (int)(sizeof wunote_calls / sizeof *wunote_calls),
	.words = wunote_words, .nwords = (int)(sizeof wunote_words / sizeof *wunote_words),
	.imms = wunote_imms, .nimms = (int)(sizeof wunote_imms / sizeof *wunote_imms),
	.numbers = wunote_numbers, .nnumbers = (int)(sizeof wunote_numbers / sizeof *wunote_numbers),
	.states = wunote_states, .nstates = (int)(sizeof wunote_states / sizeof *wunote_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810CC8C, .struct2 = 0x0810CC94,
	.names = { 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x1A, .element = 0, .weakness = 0, .shadow = false },
	.sprites = wunote_sprites, .nsprites = (int)(sizeof wunote_sprites / sizeof *wunote_sprites),
	.types = wunote_types, .ntypes = (int)(sizeof wunote_types / sizeof *wunote_types),
};
/* ---- BN5's Appley (AI 27 there, its enemy ids 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2), as AI 53 with ids 0x27e-0x283 ---- */

static const XvBlock appley_blocks[] = {
	{ 0x0810D37C, 0x0810D874, XV_CODE + 0xE000, 0x08929E7Du },
	{ 0x080C10F8, 0x080C12C0, XV_CODE + 0xE600, 0xD6114BDEu },
	{ 0x080DBAF4, 0x080DBC38, XV_CODE + 0xE800, 0x7F26FB77u },
	{ 0x080DBC4C, 0x080DBC6A, XV_CODE + 0xEA00, 0xF03FE711u },
	{ 0x080EADC0, 0x080EAE26, XV_CODE + 0xEC00, 0x491FE2BCu },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0xEE02, 0x21492011u },
};
static const XvData appley_data[] = {
	{ 0x0810D38A, 0x0810D38C, XV_RAW },
	{ 0x0810D38C, 0x0810D390, XV_POOL },
	{ 0x0810D390, 0x0810D3A0, XV_TABLE },
	{ 0x0810D444, 0x0810D46C, XV_ZERO },
	{ 0x0810D4AE, 0x0810D4B0, XV_RAW },
	{ 0x0810D4B0, 0x0810D4B8, XV_POOL },
	{ 0x0810D4B8, 0x0810D4C4, XV_TABLE },
	{ 0x0810D566, 0x0810D568, XV_RAW },
	{ 0x0810D568, 0x0810D56C, XV_POOL },
	{ 0x0810D56C, 0x0810D578, XV_TABLE },
	{ 0x0810D5F0, 0x0810D600, XV_POOL },
	{ 0x0810D600, 0x0810D606, XV_RAW },
	{ 0x0810D606, 0x0810D60C, XV_RAW },
	{ 0x0810D60C, 0x0810D612, XV_RAW },
	{ 0x0810D612, 0x0810D61E, XV_RAW },
	{ 0x0810D6D6, 0x0810D6D8, XV_RAW },
	{ 0x0810D6D8, 0x0810D6DC, XV_POOL },
	{ 0x0810D858, 0x0810D85C, XV_POOL },
	{ 0x0810D85C, 0x0810D86C, XV_RAW },
	{ 0x0810D86C, 0x0810D874, XV_POOL },
	{ 0x080C110A, 0x080C110C, XV_RAW },
	{ 0x080C110C, 0x080C1110, XV_POOL },
	{ 0x080C1110, 0x080C111C, XV_TABLE },
	{ 0x080C1188, 0x080C118C, XV_POOL },
	{ 0x080C118C, 0x080C1198, XV_TABLE },
	{ 0x080C12B8, 0x080C12C0, XV_POOL },
	{ 0x080DBB06, 0x080DBB08, XV_RAW },
	{ 0x080DBB08, 0x080DBB0C, XV_POOL },
	{ 0x080DBB0C, 0x080DBB18, XV_TABLE },
	{ 0x080DBC30, 0x080DBC34, XV_POOL },
	{ 0x080DBC34, 0x080DBC38, XV_TABLE },
	{ 0x080EADCE, 0x080EADD0, XV_RAW },
	{ 0x080EADD0, 0x080EADD4, XV_POOL },
	{ 0x080EADD4, 0x080EADE0, XV_TABLE },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair appley_calls[] = {
	{ 0x080186F0, 0x0801BBF4 },   /* map:bytes, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002CA8, 0x08002E3C },   /* map:bytes, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002630, 0x080026C4 },   /* map:calls+bytes, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08002D34, 0x08002ED0 },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080012CC, 0x08001330 },   /* map:bytes, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0800C758, 0x0800E2EC },   /* map:bytes, SAME */
	{ BN5_SPAWN_1, XV_SPAWN_1 },   /* the dispatch's spawner */
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08017054, 0x0801A258 },   /* map:bytes, SAME */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800B2A6, 0x0800CC0A },   /* map:calls, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ BN5_SPAWN_4, XV_SPAWN_4 },   /* the dispatch's spawner */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08002C56, 0x08002DEA },   /* project:cangard, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08013B5C, 0x08016EC4 },   /* map:calls+bytes, SAME */
	{ 0x080133CE, 0x080165C2 },   /* project:cangard, SAME */
	{ 0x080173B0, 0x0801A5E2 },   /* map:bytes, SAME */
	{ 0x0800DF42, 0x080101C4 },   /* map:calls, LAYOUT */
	{ 0x0801A576, 0x0801DC36 },   /* map:bytes, SAME */
	{ 0x080174F0, 0x0801A7F4 },   /* map:bytes, SAME */
	{ 0x08008AFC, 0x0800A11C },   /* map:bytes, SAME */
	{ 0x0802EFFE, 0x0802EF5C },   /* map:bytes, SAME */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080095D4, 0x0800AE44 },   /* map:bytes, SAME */
	{ 0x08029940, 0x0802CDD0 },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x08006BC2, 0x080077B4 },   /* map:bytes, SAME */
	{ 0x0800EE92, 0x08011020 },   /* map:calls+bytes, SAME */
	{ 0x0800A550, 0x0800BF5C },   /* map:calls+bytes, SAME */
	{ 0x0800A50A, 0x0800BF16 },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800C6C2, 0x0800E2C2 },   /* map:calls+bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800CE54, 0x0800EBD4 },   /* map:bytes, SAME */
};
static const XvPair appley_words[] = {
	{ 0x0800325D, 0x08003459 },
	{ 0x08009FB3, 0x0800B917 },
	{ 0x0800A327, 0x0800BD35 },
	{ 0x080138EB, 0x08016C8B },
	{ 0x080162C0, 0x08019B9C },
	{ 0x080DBC39, 0x080DF409 },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm appley_imms[] = {
	{ 0x080C1120, THUMB_MOVS, 2, 0, 0x1B, 0x33 },   /* its sprite: list 1 0x1b in BN5, 0x33 in BN6 */
	{ 0x080C11FE, THUMB_MOVS, 4, 0, 0x26, 0x74 },   /* its effect: 0x26 in BN5, 0x74 in BN6 */
	{ 0x080DBB20, THUMB_MOVS, 2, 0, 0x39, 0x6D },   /* its sprite: list 4 0x39 in BN5, 0x6d in BN6 */
	{ 0x080DBBC0, THUMB_LDR, 0, 1, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x080DBBEC, THUMB_MOVS, 4, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810D5AC, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810D5E4, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810D628, THUMB_MOVS, 0, 0, 0x14, 0x6D },   /* its effect: 0x14 in BN5, 0x6d in BN6 */
	{ 0x0810D62C, THUMB_MOVS, 0, 0, 0x15, 0x6E },   /* its effect: 0x15 in BN5, 0x6e in BN6 */
	{ 0x0810D644, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber appley_numbers[] = {
	{ 0x0810D870, 0x00000190, 0x000003FF },
};
static const XvRef appley_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x0810D46D), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810D553), BN6(0x0810AE95), OWN(0x08104D87), OWN(0x0810D37D),
};
static const XvSprite appley_sprites[] = {
	{ 1, 0x1B, 1, 0x33, XV_ART + 0x45000, 0x2000 },   /* (LZ77 in BN5) */
	{ 4, 0x39, 4, 0x6D, XV_ART + 0x47000, 0x800 },
	{ 4, 0x17, 4, 0x6E, XV_ART + 0x47800, 0x800 },
};
static const XvId appley_effects[] = { { 0x26, 0x74 } };
static const XvType appley_types[] = { { 1, 0x2F, XV_TYPE_MOVED }, { 3, 0xAC, XV_TYPE_MOVED }, { 4, 0x90, XV_TYPE_MOVED } };

static const XVirusFamily bn5_appley = {
	.name = "Appley (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 27,
	.ai = 53,
	.first_id = XV_FIRST_ID + 0x7E,
	.home = XV_HOMES + 0xA800,
	.blocks = appley_blocks, .nblocks = (int)(sizeof appley_blocks / sizeof *appley_blocks),
	.data = appley_data, .ndata = (int)(sizeof appley_data / sizeof *appley_data),
	.calls = appley_calls, .ncalls = (int)(sizeof appley_calls / sizeof *appley_calls),
	.words = appley_words, .nwords = (int)(sizeof appley_words / sizeof *appley_words),
	.imms = appley_imms, .nimms = (int)(sizeof appley_imms / sizeof *appley_imms),
	.numbers = appley_numbers, .nnumbers = (int)(sizeof appley_numbers / sizeof *appley_numbers),
	.states = appley_states, .nstates = (int)(sizeof appley_states / sizeof *appley_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810D290, .struct2 = 0x0810D298,
	.names = { 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2 },
	.prestep = 0x0810455C,   /* BN5's dispatcher's case for its ids, ported (bn5_twins.PRESTEP) */
	.ice = 0,
	.traits = { .laid = true, .mode = 0x1B, .element = 0, .weakness = 0, .shadow = true },
	.sprites = appley_sprites, .nsprites = (int)(sizeof appley_sprites / sizeof *appley_sprites),
	.effects = appley_effects, .neffects = (int)(sizeof appley_effects / sizeof *appley_effects),
	.types = appley_types, .ntypes = (int)(sizeof appley_types / sizeof *appley_types),
};
/* ---- BN5's BigBrute (AI 28 there, its enemy ids 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8), as AI 54 with ids 0x284-0x289 ---- */

static const XvBlock bigbrute_blocks[] = {
	{ 0x0810D960, 0x0810DD70, XV_CODE + 0xF000, 0x1C16C032u },
	{ 0x080D2264, 0x080D236C, XV_CODE + 0xF600, 0x77B05DFDu },
	{ 0x08104D86, 0x08104DA8, XV_CODE + 0xF802, 0x21492011u },
};
static const XvData bigbrute_data[] = {
	{ 0x0810D96E, 0x0810D970, XV_RAW },
	{ 0x0810D970, 0x0810D974, XV_POOL },
	{ 0x0810D974, 0x0810D98C, XV_TABLE },
	{ 0x0810DA3E, 0x0810DA40, XV_RAW },
	{ 0x0810DA40, 0x0810DA44, XV_POOL },
	{ 0x0810DA5C, 0x0810DA84, XV_ZERO },
	{ 0x0810DA98, 0x0810DA9C, XV_POOL },
	{ 0x0810DA9C, 0x0810DAA4, XV_TABLE },
	{ 0x0810DB0A, 0x0810DB0C, XV_RAW },
	{ 0x0810DB0C, 0x0810DB18, XV_POOL },
	{ 0x0810DB18, 0x0810DB24, XV_RAW },
	{ 0x0810DB24, 0x0810DB2A, XV_RAW },
	{ 0x0810DB2A, 0x0810DB30, XV_RAW },
	{ 0x0810DB92, 0x0810DB94, XV_RAW },
	{ 0x0810DB94, 0x0810DB98, XV_POOL },
	{ 0x0810DB98, 0x0810DB9E, XV_RAW },
	{ 0x0810DD4E, 0x0810DD50, XV_RAW },
	{ 0x0810DD50, 0x0810DD54, XV_POOL },
	{ 0x0810DD54, 0x0810DD64, XV_RAW },
	{ 0x0810DD64, 0x0810DD70, XV_POOL },
	{ 0x080D2276, 0x080D2278, XV_RAW },
	{ 0x080D2278, 0x080D227C, XV_POOL },
	{ 0x080D227C, 0x080D2288, XV_TABLE },
	{ 0x08104D94, 0x08104D98, XV_POOL },
	{ 0x08104D98, 0x08104DA8, XV_TABLE },
};
static const XvPair bigbrute_calls[] = {
	{ 0x080186A8, 0x0801BBAC },   /* project:cangard, SAME */
	{ 0x0800B328, 0x0800CC66 },   /* project:cangard, SAME */
	{ 0x08002650, 0x080026E4 },   /* project:cangard, SAME */
	{ 0x08002DF4, 0x08002F90 },   /* project:cangard, SAME */
	{ 0x08002C10, 0x08002DA4 },   /* project:cangard, SAME */
	{ 0x08002610, 0x080026A4 },   /* project:cangard, SAME */
	{ 0x08002BEC, 0x08002D80 },   /* project:cangard, SAME */
	{ 0x0800C88E, 0x0800E456 },   /* project:cangard, SAME */
	{ 0x08002DC0, 0x08002F5C },   /* project:cangard, SAME */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016126, 0x08019892 },   /* project:cangard, DIFF (audited) */
	{ 0x0800325C, 0x08003458 },   /* project:cangard, SAME */
	{ 0x08016760, 0x08019FB4 },   /* project:cangard, DIFF (audited) */
	{ 0x08016C34, 0x0801A140 },   /* project:cangard, SAME */
	{ 0x08016B1A, 0x0801A018 },   /* project:cangard, DIFF (audited) */
	{ 0x080167B0, 0x0801A00E },   /* project:cangard, DIFF (audited) */
	{ 0x08016C04, 0x0801A0D4 },   /* project:cangard, DIFF (audited) */
	{ 0x08008B6E, 0x0800A18E },   /* project:cangard, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ BN5_SPAWN_3, XV_SPAWN_3 },   /* the dispatch's spawner */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x0800B348, 0x0800CC86 },   /* map:bytes, SAME */
	{ 0x0800B6BE, 0x0800D0BC },   /* map:bytes, SAME */
};
static const XvPair bigbrute_words[] = {
	{ 0x080138EB, 0x08016C8B },
	{ 0x080162C0, 0x08019B9C },
	{ 0x08104BFD, 0x0810AE95 },
	{ 0x08104DA9, 0x0810ACC9 },
	{ 0x08104DEB, 0x0810AD0B },
	{ 0x08104E61, 0x0810AD81 },
};
static const XvImm bigbrute_imms[] = {
	{ 0x080D2294, THUMB_MOVS, 2, 0, 0x3, 0x2 },   /* its sprite: list 4 0x3 in BN5, 0x2 in BN6 */
	{ 0x080D230E, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810DAE6, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810DB40, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810DB5C, THUMB_MOVS, 0, 0, 0x12, 0x75 },   /* its effect: 0x12 in BN5, 0x75 in BN6 */
	{ 0x0810DB60, THUMB_MOVS, 0, 0, 0x13, 0x76 },   /* its effect: 0x13 in BN5, 0x76 in BN6 */
	{ 0x0810DB8A, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvRef bigbrute_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810DA85), BN6(0x0810AE95), OWN(0x08104D87), OWN(0x0810D961),
};
static const XvSprite bigbrute_sprites[] = {
	{ 1, 0x1C, 1, 0x34, XV_ART + 0x48000, 0x7000 },   /* (LZ77 in BN5) */
};
static const XvId bigbrute_effects[] = { { 0x12, 0x75 }, { 0x13, 0x76 } };
static const XvType bigbrute_types[] = { { 3, 0x61, XV_TYPE_MOVED } };

static const XVirusFamily bn5_bigbrute = {
	.name = "BigBrute (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 28,
	.ai = 54,
	.first_id = XV_FIRST_ID + 0x84,
	.home = XV_HOMES + 0xB000,
	.blocks = bigbrute_blocks, .nblocks = (int)(sizeof bigbrute_blocks / sizeof *bigbrute_blocks),
	.data = bigbrute_data, .ndata = (int)(sizeof bigbrute_data / sizeof *bigbrute_data),
	.calls = bigbrute_calls, .ncalls = (int)(sizeof bigbrute_calls / sizeof *bigbrute_calls),
	.words = bigbrute_words, .nwords = (int)(sizeof bigbrute_words / sizeof *bigbrute_words),
	.imms = bigbrute_imms, .nimms = (int)(sizeof bigbrute_imms / sizeof *bigbrute_imms),
	.states = bigbrute_states, .nstates = (int)(sizeof bigbrute_states / sizeof *bigbrute_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810D874, .struct2 = 0x0810D87C,
	.names = { 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x1C, .element = 0, .weakness = 0, .shadow = true },
	.sprites = bigbrute_sprites, .nsprites = (int)(sizeof bigbrute_sprites / sizeof *bigbrute_sprites),
	.effects = bigbrute_effects, .neffects = (int)(sizeof bigbrute_effects / sizeof *bigbrute_effects),
	.types = bigbrute_types, .ntypes = (int)(sizeof bigbrute_types / sizeof *bigbrute_types),
};
/* ---- BN5's TinHawk (AI 29 there, its enemy ids 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae), as AI 55 with ids 0x28a-0x28f ---- */

static const XvBlock tinhawk_blocks[] = {
	{ 0x0810DE5C, 0x0810E710, XV_CODE + 0xFA00, 0x00142C9Au },
	{ 0x08016B96, 0x08016BBC, XV_CODE + 0x10402, 0xA53A3BEFu },
	{ 0x08104B96, 0x08104BFC, XV_CODE + 0x10602, 0xEFA709B2u },
	{ 0x08104C0C, 0x08104C1C, XV_CODE + 0x10800, 0x4F85166Bu },
};
static const XvData tinhawk_data[] = {
	{ 0x0810DE6A, 0x0810DE6C, XV_RAW },
	{ 0x0810DE6C, 0x0810DE70, XV_POOL },
	{ 0x0810DE70, 0x0810DE80, XV_TABLE },
	{ 0x0810DF7C, 0x0810DF80, XV_POOL },
	{ 0x0810DF80, 0x0810DFA0, XV_TABLE },
	{ 0x0810DFEC, 0x0810DFF0, XV_POOL },
	{ 0x0810DFF0, 0x0810E000, XV_TABLE },
	{ 0x0810E1DE, 0x0810E1E0, XV_RAW },
	{ 0x0810E1E0, 0x0810E1E4, XV_POOL },
	{ 0x0810E34A, 0x0810E34C, XV_RAW },
	{ 0x0810E34C, 0x0810E354, XV_POOL },
	{ 0x0810E354, 0x0810E37C, XV_ZERO },
	{ 0x0810E3E8, 0x0810E3EC, XV_POOL },
	{ 0x0810E3EC, 0x0810E3F8, XV_TABLE },
	{ 0x0810E414, 0x0810E418, XV_POOL },
	{ 0x0810E418, 0x0810E430, XV_RAW },
	{ 0x0810E488, 0x0810E48C, XV_POOL },
	{ 0x0810E48C, 0x0810E492, XV_RAW },
	{ 0x0810E4DE, 0x0810E4E0, XV_RAW },
	{ 0x0810E4E0, 0x0810E4F4, XV_POOL },
	{ 0x0810E4F4, 0x0810E50C, XV_RAW },
	{ 0x0810E50C, 0x0810E512, XV_RAW },
	{ 0x0810E512, 0x0810E518, XV_RAW },
	{ 0x0810E518, 0x0810E51E, XV_RAW },
	{ 0x0810E51E, 0x0810E524, XV_RAW },
	{ 0x0810E6F0, 0x0810E6F4, XV_POOL },
	{ 0x0810E6F4, 0x0810E704, XV_RAW },
	{ 0x0810E704, 0x0810E710, XV_POOL },
	{ 0x08104BF4, 0x08104BF8, XV_RAW },
	{ 0x08104BF8, 0x08104BFC, XV_POOL },
};
static const XvPair tinhawk_calls[] = {
	{ 0x08016BBC, 0x0801A0BA },   /* search, SAME */
	{ 0x080165E4, 0x08019F44 },   /* project:families, DIFF (audited) */
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0800F2A6, 0x0801171C },   /* map:calls, DIFF (audited) */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x0801A5FE, 0x0801DCCC },   /* map:bytes, SAME */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x0800C6CA, 0x0800E2CA },   /* project:cangard, SAME */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x0800AFB0, 0x0800C8F8 },   /* map:calls, LAYOUT */
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0800C6AC, 0x0800E2AC },   /* project:cangard, SAME */
	{ 0x0800C658, 0x0800E258 },   /* map:bytes, SAME */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x08008A1C, 0x0800A03C },   /* project:cangard, LAYOUT */
	{ 0x08008A78, 0x0800A098 },   /* project:cangard, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x08012C1E, 0x08015E46 },   /* map:bytes, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x0800B6BE, 0x0800D0BC },   /* map:bytes, SAME */
};
static const XvImm tinhawk_imms[] = {
	{ 0x08104BE2, THUMB_LDR, 0, 3, 0x68, 0x70 },   /* collision data +0x68 in BN5, +0x70 in BN6 */
	{ 0x0810DED4, THUMB_MOVS, 4, 0, 0x13, 0x76 },   /* its effect: 0x13 in BN5, 0x76 in BN6 */
	{ 0x0810DEEE, THUMB_MOVS, 4, 0, 0x12, 0x75 },   /* its effect: 0x12 in BN5, 0x75 in BN6 */
	{ 0x0810E16E, THUMB_ADDS, 4, 0, 0x62, 0x77 },   /* its effect: 0x62 in BN5, 0x77 in BN6 */
	{ 0x0810E258, THUMB_MOVS, 4, 0, 0x13, 0x76 },   /* its effect: 0x13 in BN5, 0x76 in BN6 */
	{ 0x0810E272, THUMB_MOVS, 4, 0, 0x12, 0x75 },   /* its effect: 0x12 in BN5, 0x75 in BN6 */
	{ 0x0810E2CA, THUMB_MOVS, 4, 0, 0x13, 0x76 },   /* its effect: 0x13 in BN5, 0x76 in BN6 */
	{ 0x0810E2E4, THUMB_MOVS, 4, 0, 0x12, 0x75 },   /* its effect: 0x12 in BN5, 0x75 in BN6 */
	{ 0x0810E3C8, THUMB_CMP, 0, 0, 0x9, 0xB },   /* CurAction 9 compared */
	{ 0x0810E480, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810E4D2, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
};
static const XvNumber tinhawk_numbers[] = {
	{ 0x08104BF8, 0x08000000, 0x08000000 },
	{ 0x0810E1E0, 0x06050A01, 0x06050A01 },
	{ 0x0810E350, 0x03800000, 0x03800000 },
};
static const XvRef tinhawk_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810E3D5), BN6(0x0810AE95), OWN(0x0810DE5D), OWN(0x0810DF6F),
};
static const XvSprite tinhawk_sprites[] = {
	{ 1, 0x1D, 1, 0x35, XV_ART + 0x4F000, 0x5000 },   /* (LZ77 in BN5) */
	{ 5, 0x0E, 5, 0x22, XV_ART + 0x54000, 0xC00 },
};
static const XvId tinhawk_effects[] = { { 0x62, 0x77 } };

static const XVirusFamily bn5_tinhawk = {
	.name = "TinHawk (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 29,
	.ai = 55,
	.first_id = XV_FIRST_ID + 0x8A,
	.home = XV_HOMES + 0xB800,
	.blocks = tinhawk_blocks, .nblocks = (int)(sizeof tinhawk_blocks / sizeof *tinhawk_blocks),
	.data = tinhawk_data, .ndata = (int)(sizeof tinhawk_data / sizeof *tinhawk_data),
	.calls = tinhawk_calls, .ncalls = (int)(sizeof tinhawk_calls / sizeof *tinhawk_calls),
	.imms = tinhawk_imms, .nimms = (int)(sizeof tinhawk_imms / sizeof *tinhawk_imms),
	.numbers = tinhawk_numbers, .nnumbers = (int)(sizeof tinhawk_numbers / sizeof *tinhawk_numbers),
	.states = tinhawk_states, .nstates = (int)(sizeof tinhawk_states / sizeof *tinhawk_states),
	.hooks = { OWN(0x0810E3A9), BN6(BN6_FAMILY_NULLSUB), OWN(0x0810E37D), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810DD70, .struct2 = 0x0810DD78,
	.names = { 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x1D, .element = 0, .weakness = 0, .shadow = true },
	.sprites = tinhawk_sprites, .nsprites = (int)(sizeof tinhawk_sprites / sizeof *tinhawk_sprites),
	.effects = tinhawk_effects, .neffects = (int)(sizeof tinhawk_effects / sizeof *tinhawk_effects),
};
/* ---- BN5's Bladia (AI 30 there, its enemy ids 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4), as AI 56 with ids 0x290-0x295 ---- */

static const XvBlock bladia_blocks[] = {
	{ 0x0810E7FC, 0x0810EC34, XV_CODE + 0x10A00, 0x7048D0EBu },
	{ 0x08104B4C, 0x08104B80, XV_CODE + 0x11000, 0xD74B7D5Eu },
	{ 0x08104B96, 0x08104BBC, XV_CODE + 0x11202, 0x8F659214u },
	{ 0x08104C0C, 0x08104C1C, XV_CODE + 0x11400, 0x4F85166Bu },
};
static const XvData bladia_data[] = {
	{ 0x0810E87E, 0x0810E880, XV_RAW },
	{ 0x0810E880, 0x0810E884, XV_POOL },
	{ 0x0810E884, 0x0810E890, XV_TABLE },
	{ 0x0810E972, 0x0810E974, XV_RAW },
	{ 0x0810E974, 0x0810E978, XV_POOL },
	{ 0x0810E986, 0x0810E988, XV_RAW },
	{ 0x0810E988, 0x0810E98C, XV_POOL },
	{ 0x0810E98C, 0x0810E99C, XV_TABLE },
	{ 0x0810EA7C, 0x0810EA80, XV_POOL },
	{ 0x0810EA80, 0x0810EAB0, XV_ZERO },
	{ 0x0810EAC4, 0x0810EAC8, XV_POOL },
	{ 0x0810EAC8, 0x0810EAD4, XV_TABLE },
	{ 0x0810EAF6, 0x0810EAF8, XV_RAW },
	{ 0x0810EAF8, 0x0810EAFC, XV_POOL },
	{ 0x0810EAFC, 0x0810EB02, XV_RAW },
	{ 0x0810EB3C, 0x0810EB4C, XV_POOL },
	{ 0x0810EB4C, 0x0810EB52, XV_RAW },
	{ 0x0810EB52, 0x0810EB58, XV_RAW },
	{ 0x0810EB58, 0x0810EB5E, XV_RAW },
	{ 0x0810EB5E, 0x0810EB6A, XV_RAW },
	{ 0x0810EB90, 0x0810EB94, XV_POOL },
	{ 0x0810EB94, 0x0810EB9A, XV_RAW },
	{ 0x0810EC1A, 0x0810EC1C, XV_RAW },
	{ 0x0810EC1C, 0x0810EC20, XV_POOL },
	{ 0x0810EC20, 0x0810EC30, XV_RAW },
	{ 0x0810EC30, 0x0810EC34, XV_POOL },
	{ 0x08104B7A, 0x08104B7C, XV_RAW },
	{ 0x08104B7C, 0x08104B80, XV_POOL },
};
static const XvPair bladia_calls[] = {
	{ 0x0800B290, 0x0800CBD8 },   /* map:bytes, SAME */
	{ 0x0802B416, 0x0802D26A },   /* map:calls+bytes, SAME */
	{ 0x0800D1E2, 0x0800F2C6 },   /* map:bytes, SAME */
	{ 0x0800F2A6, 0x0801171C },   /* map:calls, DIFF (audited) */
	{ 0x08016C56, 0x0801A166 },   /* project:cangard, SAME */
	{ 0x08016C42, 0x0801A152 },   /* project:cangard, SAME */
	{ 0x08016C4C, 0x0801A15C },   /* project:cangard, SAME */
	{ 0x0800F29E, 0x08011714 },   /* project:cangard, DIFF (audited) */
	{ 0x0800CCD2, 0x0800E9DC },   /* project:cangard, DIFF (audited) */
	{ 0x08000598, 0x080005CC },   /* project:cangard, SAME */
	{ 0x08030E1C, 0x080302A8 },   /* map:calls, SAME */
	{ 0x080C94D4, 0x080CA2D0 },   /* map:bytes, SAME */
	{ 0x0800C676, 0x0800E276 },   /* project:cangard, SAME */
	{ 0x080DFFD2, 0x080E1932 },   /* project:cangard, SAME */
	{ 0x080C4E72, 0x080C6BDA },   /* map:bytes, SAME */
	{ 0x0800CA62, 0x0800E5E2 },   /* map:bytes, SAME */
	{ 0x08018654, 0x0801BB1C },   /* map:calls, DIFF (audited) */
	{ 0x08016B88, 0x0801A074 },   /* project:cangard, SAME */
	{ 0x0801A666, 0x0801DD34 },   /* map:bytes, SAME */
	{ 0x08016B90, 0x0801A07C },   /* project:cangard, SAME */
	{ 0x0801A5BC, 0x0801DC7C },   /* map:calls, DIFF (audited) */
	{ 0x08018676, 0x0801BB46 },   /* map:calls, LAYOUT */
	{ 0x0800C69C, 0x0800E29C },   /* project:cangard, SAME */
	{ 0x08016B60, 0x0801A04C },   /* project:cangard, SAME */
	{ 0x0800E140, 0x080103BC },   /* project:cangard, SAME */
	{ 0x0800F200, 0x08011680 },   /* project:cangard, DIFF (audited) */
	{ 0x0810567C, 0x0810AA2A },   /* project:handi, SAME */
	{ 0x0800B5DE, 0x0800CFDC },   /* map:bytes, SAME */
	{ 0x080014A6, 0x08001532 },   /* project:handi, SAME */
};
static const XvPair bladia_words[] = {
	{ 0x0801629C, 0x08019B78 },
};
static const XvImm bladia_imms[] = {
	{ 0x0810E952, THUMB_MOVS, 4, 0, 0x60, 0x78 },   /* its effect: 0x60 in BN5, 0x78 in BN6 */
	{ 0x0810E9E8, THUMB_MOVS, 4, 0, 0x12, 0x75 },   /* its effect: 0x12 in BN5, 0x75 in BN6 */
	{ 0x0810EA24, THUMB_MOVS, 4, 0, 0x13, 0x76 },   /* its effect: 0x13 in BN5, 0x76 in BN6 */
	{ 0x0810EAEA, THUMB_MOVS, 0, 0, 0x7, 0x9 },   /* CurAction 7 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810EB30, THUMB_MOVS, 0, 0, 0x9, 0xB },   /* CurAction 9 (setAttack0): BN6's two status states before the family's own */
	{ 0x0810EB84, THUMB_MOVS, 0, 0, 0x8, 0xA },   /* CurAction 8 (setAttack0): BN6's two status states before the family's own */
};
static const XvRef bladia_states[] = {
	BN6(0x08016381), BN6(0x080165B9), BN6(0x080165C3), BN6(0x080166AF), BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73), OWN(0x0810EAB1), OWN(0x0810E7FD), OWN(0x0810E979), OWN(0x0810E871), BN6(0x0810AB35), BN6(0x0810AE95),
};
static const XvSprite bladia_sprites[] = {
	{ 1, 0x1E, 1, 0x36, XV_ART + 0x55000, 0x4000 },   /* (LZ77 in BN5) */
	{ 4, 0x48, 4, 0x6F, XV_ART + 0x59000, 0x800 },
};
static const XvId bladia_effects[] = { { 0x60, 0x78 } };

static const XVirusFamily bn5_bladia = {
	.name = "Bladia (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 30,
	.ai = 56,
	.first_id = XV_FIRST_ID + 0x90,
	.home = XV_HOMES + 0xC000,
	.blocks = bladia_blocks, .nblocks = (int)(sizeof bladia_blocks / sizeof *bladia_blocks),
	.data = bladia_data, .ndata = (int)(sizeof bladia_data / sizeof *bladia_data),
	.calls = bladia_calls, .ncalls = (int)(sizeof bladia_calls / sizeof *bladia_calls),
	.words = bladia_words, .nwords = (int)(sizeof bladia_words / sizeof *bladia_words),
	.imms = bladia_imms, .nimms = (int)(sizeof bladia_imms / sizeof *bladia_imms),
	.states = bladia_states, .nstates = (int)(sizeof bladia_states / sizeof *bladia_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x0810E710, .struct2 = 0x0810E718,
	.names = { 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x1E, .element = 0, .weakness = 0, .shadow = true },
	.sprites = bladia_sprites, .nsprites = (int)(sizeof bladia_sprites / sizeof *bladia_sprites),
	.effects = bladia_effects, .neffects = (int)(sizeof bladia_effects / sizeof *bladia_effects),
};
const XVirusFamily *const xvirus_bn5[] = { &bn5_cangard, &bn5_handi, &bn5_powie, &bn5_bugtank, &bn5_dominerd, &bn5_metfire, &bn5_shakey, &bn5_ninjoy, &bn5_lark, &bn5_bomboy, &bn5_zomon, &bn5_drixol, &bn5_batty, &bn5_skarab, &bn5_draggin, &bn5_whirly, &bn5_marina, &bn5_flashy, &bn5_eleogre, &bn5_cactikil, &bn5_wunote, &bn5_appley, &bn5_bigbrute, &bn5_tinhawk, &bn5_bladia };
const int xvirus_bn5_count = (int)(sizeof xvirus_bn5 / sizeof *xvirus_bn5);

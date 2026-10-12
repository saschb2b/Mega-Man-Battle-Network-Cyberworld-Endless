/* xvirus_bn5.c: BN5's virus families' recipes, written by tools/xrecipe.py from the two ROMs (python3 tools/xrecipe.py;
 * --check says whether this is what it writes). Do not edit: change the generator (or tools/bn5_twins.py, its pairs and
 * their audits) and write it again. Numbers alone (docs/MULTIROM.md, Transplanted families): addresses in BN5 Team
 * Colonel (USA) and BN6 Cybeast Gregar (USA), counts and the CRC32s of the ranges the install copies from the player's
 * ROM; nothing of either game's. The families here are the generator's, whole (no decision open), in install order
 * (each its AI index, ids, home, code and art places by its slot); src/emu/xvirus_families.c's xvirus_bn5 lists the ones
 * installed, and tests/test_core.c holds the generated CanGard and Handi to the recipes installed, field for field. */
#include "xvirus_bn5.h"

#include "bn5.h"
#include "bn6.h"
#include "rom.h"

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
	.states = handi_states, .nstates = (int)(sizeof handi_states / sizeof *handi_states),
	.hooks = { BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB) },
	.struct1 = 0x081060E8, .struct2 = 0x081060F0,
	.names = { 0x13, 0x14, 0x15, 0x16, 0x17, 0x18 },
	.ice = 0,
	.traits = { .laid = true, .mode = 0x0B, .element = 0, .weakness = 0, .shadow = true },
	.sprites = handi_sprites, .nsprites = (int)(sizeof handi_sprites / sizeof *handi_sprites),
};
const XVirusFamily *const xvirus_bn5_generated[] = { &bn5_cangard, &bn5_handi };
const int xvirus_bn5_generated_count = (int)(sizeof xvirus_bn5_generated / sizeof *xvirus_bn5_generated);

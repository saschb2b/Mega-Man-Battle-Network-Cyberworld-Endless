/* xvirus_families.h: the recipes, numbers alone (docs/ROM_DATA.md,
 * Transplanted virus families, says how each was found and how to check
 * it). The code and records they name are copied from the player's ROMs as
 * the core starts. */
#include "xvirus_families.h"

#include "bn6.h"
#include "rom.h"

#define OWN(a)  { (a), true }    /* a routine of the family's own: its address in its game */
#define BN6(a)  { (a), false }   /* one of BN6's */
#define SAME(a) { (a), (a) }     /* a call of BN6's own code to the same routine, through a veneer */

/* ---- the control: BN6's own Mettaur (AI 1) moved, as AI 32 with ids from 0x200 ----
 * Its code is 0x0810AFEC-0x0810B5EF, between its held chips and the next
 * family's records: a recursive walk from its state table's own entries
 * reached 1276 bytes of code, 17 literal words and 5 jump tables, and the
 * rest are its tables of numbers, its state table and alignment. No word
 * of BN6's outside its tables and held chips names any of it, and no BL
 * outside it calls into it (whole-ROM scans). Its 64 BLs call 32 routines
 * outside it. */

static const XvBlock control_blocks[] = {
	{ 0x0810AFEC, 0x0810B5F0, XV_CODE, 0xD9CF832Cu },
};

static const XvData control_data[] = {
	{ 0x0810B024, 0x0810B028, XV_POOL }, { 0x0810B028, 0x0810B038, XV_TABLE },
	{ 0x0810B08C, 0x0810B090, XV_POOL }, { 0x0810B090, 0x0810B0A0, XV_RAW },
	{ 0x0810B110, 0x0810B114, XV_POOL }, { 0x0810B114, 0x0810B11C, XV_TABLE },
	{ 0x0810B1B8, 0x0810B1BC, XV_POOL }, { 0x0810B1BC, 0x0810B1C8, XV_TABLE },
	{ 0x0810B224, 0x0810B258, XV_ZERO },   /* its state table, moved to its home */
	{ 0x0810B258, 0x0810B270, XV_RAW }, { 0x0810B270, 0x0810B276, XV_RAW }, { 0x0810B276, 0x0810B27C, XV_RAW },
	{ 0x0810B31A, 0x0810B31C, XV_RAW },
	{ 0x0810B31C, 0x0810B320, XV_POOL }, { 0x0810B320, 0x0810B334, XV_TABLE },
	{ 0x0810B3AA, 0x0810B3AC, XV_RAW }, { 0x0810B3AC, 0x0810B3B0, XV_POOL },
	{ 0x0810B3F8, 0x0810B3FC, XV_POOL }, { 0x0810B3FC, 0x0810B404, XV_TABLE },
	{ 0x0810B4DE, 0x0810B4E0, XV_RAW }, { 0x0810B4E0, 0x0810B4E4, XV_POOL },
	{ 0x0810B530, 0x0810B534, XV_POOL },
	{ 0x0810B5BA, 0x0810B5BC, XV_RAW }, { 0x0810B5BC, 0x0810B5C0, XV_POOL }, { 0x0810B5C0, 0x0810B5D0, XV_RAW },
	{ 0x0810B5D0, 0x0810B5D4, XV_POOL }, { 0x0810B5D4, 0x0810B5D8, XV_RAW }, { 0x0810B5D8, 0x0810B5F0, XV_POOL },
};

static const XvPair control_calls[] = {
	SAME(0x08001532), SAME(0x0800A704), SAME(0x0800AA06), SAME(0x0800AA1A),
	SAME(0x0800AA40), SAME(0x0800CC86), SAME(0x0800E29C), SAME(0x0800E2CA),
	SAME(0x0800E5E2), SAME(0x0800E9DC), SAME(0x0800EBD4), SAME(0x0800ED90),
	SAME(0x0800F322), SAME(0x080103F8), SAME(0x08011680), SAME(0x08011714),
	SAME(0x080126E4), SAME(0x080165B8), SAME(0x080165C2), SAME(0x08016B02),
	SAME(0x08016B36), SAME(0x08016B72), SAME(0x08016CE8), SAME(0x0801A04C),
	SAME(0x0801A152), SAME(0x0801A15C), SAME(0x0801A166), SAME(0x0801BB1C),
	SAME(0x0801BB46), SAME(0x080C8554), SAME(0x080E1932), SAME(0x0810AA2A),
};

/* its state table as BN6's at 0x0810B224 has it: BN6's spawn (0) and
 * flinch (3), its own wrappers of the others, its own states from 8 */
static const XvRef control_states[] = {
	BN6(0x08016381), OWN(0x0810B27D), OWN(0x0810B29B), BN6(0x080166AF),
	OWN(0x0810B2AF), OWN(0x0810B2F5), OWN(0x0810B2BB), OWN(0x0810B2C7),
	OWN(0x0810B307), OWN(0x0810AFED), OWN(0x0810B017), OWN(0x0810B103), OWN(0x0810B1AB),
};

const XVirusFamily xvirus_control = {
	.name = "Mettaur (BN6's own, the control)",
	.game = XV_GAME_BN6,
	.src_ai = 1,
	.ai = 32,
	.first_id = XV_FIRST_ID,
	.home = XV_HOMES,
	.blocks = control_blocks, .nblocks = (int)(sizeof control_blocks / sizeof *control_blocks),
	.data = control_data, .ndata = (int)(sizeof control_data / sizeof *control_data),
	.calls = control_calls, .ncalls = (int)(sizeof control_calls / sizeof *control_calls),
	.states = control_states, .nstates = (int)(sizeof control_states / sizeof *control_states),
	.hooks = {
		BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB),
		BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB),
	},
	.struct1 = 0x0810AF00, .struct2 = 0x0810AF08,
	.names = { 1, 2, 3, 4, 5, 6 },
	.held = 0x0810AF2C,   /* (its own lists, where BN6_HELD_CHIPS has them for AI 1) */
	.ice = XV_ICE_OWN,    /* (AI 1's in BN6_ICE_SIZES: 0) */
};

/* ---- BN5's CanGard (AI 14 there, its enemy ids 0x4F-0x54), as AI 32 with ids 0x200-0x205 ----
 * Two blocks of BN5's: CanGard's own code from its first state on, with
 * its jump tables, its state table in BN5's layout (dead in the copy), its
 * per-version tables of numbers and a mask, to the next family's records
 * (AI 15's struct1, 0x08109140); and the attack it fires, BN5's type 0x25,
 * a reticle that walks to MegaMan's panel and strikes there, with the
 * spawn and release CanGard calls. A BL scan of both finds 4 calls within
 * them (one from CanGard to the reticle's spawner, two to its release) and
 * 39 to 39 routines outside, each paired with BN6's own (docs/ROM_DATA.md,
 * Virus families moved into the battle engine: BN5's CanGard). */

static const XvBlock cangard_blocks[] = {
	{ 0x08108E14, 0x08109140, XV_CODE, 0x018F4F22u },
	{ 0x080C9518, 0x080C9784, XV_CODE + 0x400, 0xD7ABC4C3u },
};

static const XvData cangard_data[] = {
	/* (CanGard's) */
	{ 0x08108E22, 0x08108E24, XV_RAW }, { 0x08108E24, 0x08108E28, XV_POOL }, { 0x08108E28, 0x08108E34, XV_TABLE },
	{ 0x08108EB2, 0x08108EB4, XV_RAW }, { 0x08108EB4, 0x08108EB8, XV_POOL }, { 0x08108EB8, 0x08108ED0, XV_TABLE },
	{ 0x08109000, 0x08109028, XV_ZERO },   /* its state table, BN5's layout: BN6's in its home */
	{ 0x0810909A, 0x0810909C, XV_RAW }, { 0x0810909C, 0x081090A0, XV_POOL }, { 0x081090A0, 0x081090AC, XV_TABLE },
	{ 0x0810911C, 0x08109128, XV_POOL },
	{ 0x08109128, 0x0810912E, XV_RAW }, { 0x0810912E, 0x08109134, XV_RAW }, { 0x08109134, 0x0810913A, XV_RAW }, { 0x0810913A, 0x0810913C, XV_RAW },
	{ 0x0810913C, 0x08109140, XV_POOL },   /* (a mask, 0xA000: blind and confused, the same bits in BN6) */
	/* (the reticle's) */
	{ 0x080C952A, 0x080C952C, XV_RAW }, { 0x080C952C, 0x080C9530, XV_POOL }, { 0x080C9530, 0x080C953C, XV_TABLE },
	{ 0x080C9604, 0x080C960C, XV_POOL }, { 0x080C960C, 0x080C9624, XV_RAW }, { 0x080C9624, 0x080C962A, XV_RAW },
	{ 0x080C9670, 0x080C9674, XV_POOL }, { 0x080C9674, 0x080C9680, XV_TABLE },
};

/* BN5's routines it calls and BN6's in their place (each pair's bodies
 * compared, docs/ROM_DATA.md) */
static const XvPair cangard_calls[] = {
	/* CanGard's: an object's flags set, cleared and read; its attack begun and ended; its animation's frame; its counter
	 * time; a panel's coordinates; an effect spawned; the battle paused, time stopped, over; MegaMan found; the dying state */
	{ 0x08016C42, 0x0801A152 }, { 0x08016C4C, 0x0801A15C }, { 0x08016C56, 0x0801A166 }, { 0x0800F29E, 0x08011714 },
	{ 0x0800F200, 0x08011680 }, { 0x08002C56, 0x08002DEA }, { 0x0800CCD2, 0x0800E9DC }, { 0x0800C676, 0x0800E276 },
	{ 0x080DFFD2, 0x080E1932 }, { 0x08008A1C, 0x0800A03C }, { 0x08008A78, 0x0800A098 }, { 0x0800E140, 0x080103BC },
	{ 0x080133CE, 0x080165C2 }, { 0x08008B6E, 0x0800A18E },
	/* the reticle's: an attack spawned, its sprite run, an object's end, its panel's check; a sprite's loading, shadow,
	 * animation, palette and flip; facing; coordinates and panels; memory freed; its collision data made, set up, given
	 * its hit's effect, its region cleared and set, its panels and its presence updated, its hit placed, its strike's
	 * effect; a sound */
	{ 0x08003178, 0x08003358 }, { 0x080186A8, 0x0801BBAC }, { 0x080138EA, 0x08016C8A }, { 0x0800B328, 0x0800CC66 },
	{ 0x08002650, 0x080026E4 }, { 0x08002DF4, 0x08002F90 }, { 0x08002C10, 0x08002DA4 }, { 0x08002610, 0x080026A4 },
	{ 0x08002BEC, 0x08002D80 }, { 0x0800C88E, 0x0800E456 }, { 0x08002DC0, 0x08002F5C }, { 0x0800C6CA, 0x0800E2CA },
	{ 0x0800C69C, 0x0800E29C }, { 0x0800C6AC, 0x0800E2AC }, { 0x0800325C, 0x08003458 }, { 0x08016126, 0x08019892 },
	{ 0x08016760, 0x08019FB4 }, { 0x08016C34, 0x0801A140 }, { 0x08016B88, 0x0801A074 }, { 0x08016B90, 0x0801A07C },
	{ 0x08016B60, 0x0801A04C }, { 0x08016B1A, 0x0801A018 }, { 0x080167B0, 0x0801A00E }, { 0x08016C04, 0x0801A0D4 },
	{ 0x08000598, 0x080005CC },
};

/* What differs in BN6, each checked against the instruction it sits in */
static const XvImm cangard_imms[] = {
	{ 0x081090C6, THUMB_MOVS, 0, 0, 7, 9 },        /* its guard's CurAction: BN5's 7, BN6's two status states before it */
	{ 0x08109110, THUMB_MOVS, 0, 0, 8, 10 },       /* its shot's: 8, 10 */
	{ 0x08108FB6, THUMB_ADDS, 4, 0, 0x57, 0x6C },  /* its muzzle's blast: BN5's effect 0x57, BN6's 0x6C (cangard_effects) */
	{ 0x080C975C, THUMB_MOVS, 0, 0, 0x25, 0xD1 },  /* the reticle's type: BN5's 0x25, BN6's empty 0xD1 (cangard_types) */
	{ 0x080C955A, THUMB_MOVS, 2, 0, 0x24, 0x5C },  /* its sprite: list 4's 0x24 in BN5, 0x5C in BN6 (cangard_sprites) */
	{ 0x080C9636, THUMB_LDR, 0, 7, 0x68, 0x70 },   /* the flags a hit leaves in its collision data: BN5's +0x68, BN6's +0x70 */
};

/* its states BN6's way: BN6's generic 0-7 but its own dying (2), which
 * lets the reticle go first; its own from 8 (BN5's from 6) */
static const XvRef cangard_states[] = {
	BN6(0x08016381), BN6(0x080165B9), OWN(0x08109055), BN6(0x080166AF),
	BN6(0x08016B03), BN6(0x08016CE9), BN6(0x08016B37), BN6(0x08016B73),
	OWN(0x08109087), OWN(0x08108E15), OWN(0x08108EA5), OWN(0x08108FD9),
};

static const XvSprite cangard_sprites[] = {
	{ 1, 0x0E, 1, 0x1E, XV_ART, 0x3000 },            /* CanGard (LZ77 in BN5), at list 1's placeholder 0x1E (the grown list's) */
	{ 4, 0x24, 4, 0x5C, XV_ART + 0x3000, 0x800 },    /* the reticle, past list 4's 92 in its grown copy */
	{ 4, 0x3D, 4, 0x5D, XV_ART + 0x3800, 0xC00 },    /* the muzzle's blast */
};
static const XvId cangard_effects[] = { { 0x57, 0x6C } };
static const XvId cangard_types[] = { { 0x25, BN6_T3_FREE } };

const XVirusFamily xvirus_cangard = {
	.name = "CanGard (BN5's)",
	.game = XROM_BN5_COLONEL_US,
	.src_ai = 14,
	.ai = 32,
	.first_id = XV_FIRST_ID,
	.home = XV_HOMES,
	.blocks = cangard_blocks, .nblocks = (int)(sizeof cangard_blocks / sizeof *cangard_blocks),
	.data = cangard_data, .ndata = (int)(sizeof cangard_data / sizeof *cangard_data),
	.calls = cangard_calls, .ncalls = (int)(sizeof cangard_calls / sizeof *cangard_calls),
	.imms = cangard_imms, .nimms = (int)(sizeof cangard_imms / sizeof *cangard_imms),
	.states = cangard_states, .nstates = (int)(sizeof cangard_states / sizeof *cangard_states),
	/* each frame: the reticle let go where MegaMan left its row; paralysed, frozen, bubbled and dragged: the reticle let
	 * go (BN5's tables 6 and 8; frozen and bubbled BN6's alone, given the same as BN6's AI 17 gives one hook all four) */
	.hooks = {
		OWN(0x08109029), BN6(BN6_FAMILY_NULLSUB), BN6(BN6_FAMILY_NULLSUB), OWN(0x08109071),
		OWN(0x08109071), OWN(0x08109071), BN6(BN6_FAMILY_NULLSUB), OWN(0x08109071),
	},
	.struct1 = 0x08108D28, .struct2 = 0x08108D30,
	.names = { 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54 },
	.held = 0,   /* (none: BN5's lists for it are empty) */
	.ice = 0,    /* (BN5 freezes none: the smallest block, as BN6's small viruses') */
	.traits = { .laid = true, .mode = 0x0E, .element = 0, .weakness = 0, .shadow = true },
	.sprites = cangard_sprites, .nsprites = (int)(sizeof cangard_sprites / sizeof *cangard_sprites),
	.effects = cangard_effects, .neffects = (int)(sizeof cangard_effects / sizeof *cangard_effects),
	.types = cangard_types, .ntypes = (int)(sizeof cangard_types / sizeof *cangard_types),
};

const XVirusFamily *const xvirus_bn5[] = { &xvirus_cangard };
const int xvirus_bn5_count = (int)(sizeof xvirus_bn5 / sizeof *xvirus_bn5);

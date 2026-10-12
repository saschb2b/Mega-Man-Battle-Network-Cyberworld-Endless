/* What a Soul does to the chips MegaMan uses in BN6's engine (soulchips.h). */
#include "soulchips.h"

#include <stdint.h>
#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "bn6_chips.h"
#include "bn6_forms.h"
#include "bn6_panels.h"
#include "bytes.h"
#include "chips.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "soul_room.h"
#include "soulcode.h"
#include "soulkinds.h"
#include "soullife.h"
#include "soulmood.h"
#include "soulpower.h"
#include "thumb_reloc.h"

#define BUS 0x08000000u
#define THUMB_MOVS_R0 0x2000u
#define THUMB_BX_LR 0x4770u
#define SOUL_SHADOW 8
#define SOUL_TOMAHAWK 10
#define SOUL_KNIGHT 11
#define SOUL_TOAD 12
/* BN5_CHIP_KIND's: the kinds BN5's chip charge takes by the Soul (0x08010902) */
#define KIND_AQUA 1
#define KIND_WOOD 3
#define KIND_SWORD 6
#define KIND_BREAK 11
#define CHARGE_DOUBLE 0xFF   /* BN5's charged chip's bonus, its damage once more (0x080103C8: 0xFF, 0x0800FF84) */
#define MODIFIED_DOUBLE 0x8000u   /* a chip's damage doubled by its modifier (BN5's 0x0801045C, BN6's 0x08012DE4: the same bit) */
#define KNIGHT_GUARD 0x32    /* KnightSoul's frames unharmed after a chip from his front column (0x080103C0) */
#define SHADOW_NONE 0        /* the shadow type of ShadowSoul (0x0800E0CE: 0, every other form's 4) */
#define GRASS_CODE_SIZE 28   /* the Thumb TomahawkSoul's grass jumps to (grass_code), then */
#define ANSWER_NO (GRASS_CODE_SIZE)        /* ... movs r0,#0; bx lr and */
#define ANSWER_YES (GRASS_CODE_SIZE + 4)   /* ... movs r0,#1; bx lr: an answer with its flags set, as BN6's callers test them */
#define THUMB_SIZE (GRASS_CODE_SIZE + 8)

static int breaks;
static bool done;
static uint32_t sword_family;   /* the moved sword family's CurAction (BN5's 0x13 as the Souls' own) */

static int side_of(uint32_t obj) { return hook_read8(obj + BN6_T1_ALLIANCE) & 1; }

static int soul_on(int side) { return done && soul_united(side) ? hook_read8(soul_field(side, BN6_SOUL_ID)) : 0; }

static uint8_t flags_of(int chip) {
	return chip > 0 && chip < CHIPS_IDS ? hook_read8(chips_table_bus() + (uint32_t)chip * BN6_CHIP_RECORD_SIZE + BN6_CHIP_EFFECT_FLAGS) : 0;
}

/* (BN5's test of an attack chip in its chip's modifiers: power shown, not left alone, 0x0801034A-0x08010352) */
static bool attack_chip(int chip) {
	uint8_t f = flags_of(chip);
	return (f & BN6_CHIP_SHOWS_POWER) && !(f & BN6_CHIP_LEFT_ALONE);
}

static uint32_t panel_at(uint32_t x, uint32_t y) { return BN6_FIELD_PANELS + (y * BN6_FIELD_COLUMNS + x) * BN6_PANEL_SIZE; }

/* ---- the chip charge ---- */

/* (the chip charge's test, r0 the chip, r5 MegaMan, its first instruction) a Soul's by BN5's rule: a chip of its kind, not
 * left alone, no DarkChip; none for a Soul of no kind's charge. Its callers test the flags its last movs set (0x080130AC),
 * so the answer is one of two movs and a bx lr of ours */
static HookAct charge_test(HookRegs *r, void *user) {
	(void)user;
	int soul = soul_on(side_of(r->r[5])), chip = (int)r->r[0];
	if (!soul) return HOOK_CONTINUE;
	int kind = soul == SOUL_SHADOW ? KIND_SWORD : soul == SOUL_KNIGHT ? KIND_BREAK : soul == SOUL_TOAD ? KIND_AQUA : -1;
	bool ok = kind >= 0 && chip > 0 && chip < CHIPS_IDS && !(flags_of(chip) & BN6_CHIP_LEFT_ALONE) && !soulkinds_dark(chip) &&
		soulkinds_of(chip) == kind && !(breaks & SOULCHIPS_BREAK_CHARGE);
	r->r[12] = (SOULS_THUMB + (ok ? ANSWER_YES : ANSWER_NO)) | 1;
	return HOOK_JUMP;
}

/* (a charged chip's bonus by the form, r0 whether it was charged, its first instruction) KnightSoul's and ToadSoul's double,
 * none for the rest */
static HookAct charge_bonus(HookRegs *r, void *user) {
	(void)user;
	int soul = soul_on(side_of(r->r[5]));
	if (!soul) return HOOK_CONTINUE;
	r->r[0] = r->r[0] && (soul == SOUL_KNIGHT || soul == SOUL_TOAD) && !(breaks & SOULCHIPS_BREAK_DOUBLE) ? CHARGE_DOUBLE : 0;
	return HOOK_RETURN;
}

/* (the release of a charged A, r1 the A power attack, r5 MegaMan) a Soul's chip charge: the chip used charged, BN5's way
 * (its empty power attacks name the charge time alone, 0x0800D9E0-0x0800D9EE), where BN6 would run the power attack */
static HookAct release(HookRegs *r, void *user) {
	if (!soul_on(side_of(r->r[5])) || !soulpower_chip_charge((int)r->r[1])) return HOOK_CONTINUE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* ---- what the chip used gets ---- */

/* (the chip's damage modified, r0 it, r1 what its modifier did, r7 its attack's variables, at the strh that keeps it)
 * TomahawkSoul's Wood attack chip on grass: doubled, and on to the Thumb that makes the panel plain (BN5's case 4,
 * 0x0800FFCA) and plays the modifier's sound */
static HookAct grass(HookRegs *r, void *user) {
	int chip = hook_read16(r->r[7] + BN6_ATTACK_CHIP);
	if (soul_on(side_of(r->r[5])) != SOUL_TOMAHAWK || r->r[1] || (breaks & SOULCHIPS_BREAK_GRASS)) return HOOK_CONTINUE;
	uint32_t x = hook_read8(r->r[5] + BN6_T1_PANEL_X), y = hook_read8(r->r[5] + BN6_T1_PANEL_Y);
	if (soulkinds_of(chip) != KIND_WOOD || !attack_chip(chip) || hook_read8(panel_at(x, y) + BN6_PANEL_TYPE) != BN6_PANEL_GRASS)
		return HOOK_CONTINUE;
	r->r[0] |= MODIFIED_DOUBLE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* (the chip used, its last step: r7 its attack's variables, r5 MegaMan) KnightSoul's: an attack chip from his front column
 * (the panel before him the enemy's) leaves him 50 frames unharmed, as BN5's 0x0800CD84 does it (BN6's twin 0x0800EAFA) */
static HookAct front(HookRegs *r, void *user) {
	(void)user;
	uint32_t me = r->r[5];
	int side = side_of(me), chip = hook_read16(r->r[7] + BN6_ATTACK_CHIP);
	soulmood_chip_used(me, chip);   /* (BN5's DarkChip use, 0x08010028: in BN5's chip use before KnightSoul's step) */
	uint32_t dud;
	if (soulmood_chip_refused(me, r->r[7], chip, &dud)) {   /* (BN5's dark gate, 0x080100DE: the dud uses no step of KnightSoul's) */
		r->r[4] = dud;
		return HOOK_CONTINUE;
	}
	if (soul_on(side) != SOUL_KNIGHT || !attack_chip(chip) || (breaks & SOULCHIPS_BREAK_FRONT)) return HOOK_CONTINUE;
	/* (his facing: BN5's 0x0800C6CA, BN6's 0x0800E2CA, +1 or -1 by his side and flip) */
	int dx = (side ^ hook_read8(me + BN6_T1_FLIP)) ? -1 : 1;
	uint32_t x = (uint32_t)(hook_read8(me + BN6_T1_PANEL_X) + dx), y = hook_read8(me + BN6_T1_PANEL_Y);
	if (x >= BN6_FIELD_COLUMNS || hook_read8(panel_at(x, y) + BN6_PANEL_ALLIANCE) == (uint8_t)side) return HOOK_CONTINUE;
	uint32_t coll = hook_read32(me + BN6_T1_COLLISION);
	hook_write16(coll + BN6_COLL_INVULNERABLE, KNIGHT_GUARD);
	hook_write32(coll + BN6_COLL_FLAGS, hook_read32(coll + BN6_COLL_FLAGS) | BN6_OBJ_INVULNERABLE);
	return HOOK_CONTINUE;
}

/* (the chip's family read as the chip use returns it, r4 its record, r7 its attack's variables) ShadowSoul's charged
 * sword: BN5's sword family, moved, which takes his paths (BN5 0x080EB762 on: a slash before the enemy) */
static HookAct sword(HookRegs *r, void *user) {
	int chip = hook_read16(r->r[7] + BN6_ATTACK_CHIP);
	if (soul_on(side_of(r->r[5])) != SOUL_SHADOW || !hook_read8(r->r[7] + BN6_ATTACK_CHARGED) || soulkinds_of(chip) != KIND_SWORD ||
		hook_read8(r->r[4] + BN6_CHIP_FAMILY) != BN6_FAMILY_SWORD || (breaks & SOULCHIPS_BREAK_SWORD))
		return HOOK_CONTINUE;
	r->r[0] = sword_family;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* (the shadow type, past its battle mode's test: r5 the object) ShadowSoul's side's MegaMan without one (BN5 0x0800E0B6),
 * where his NaviStats name no Navi of his side */
static HookAct shadow(HookRegs *r, void *user) {
	int side = side_of(r->r[5]);
	if (soul_on(side) != SOUL_SHADOW || hook_read8(BN6_BATTLE_NAVI + (uint32_t)side * BN6_BATTLE_NAVI_SIZE + BN6_NAVI_SIDE_NAVI) ||
		(breaks & SOULCHIPS_BREAK_SHADOW))
		return HOOK_CONTINUE;
	r->r[0] = SHADOW_NONE;
	r->r[12] = (uint32_t)(uintptr_t)user | 1;
	return HOOK_JUMP;
}

/* ---- the install ---- */

/* the Thumb TomahawkSoul's grass jumps to: the hooked strh, the panel under him plain (BN6_SET_PANEL_TYPE, r0 column, r1 row,
 * r2 the type), on to the modifier's sound (BN6_CHIP_MODIFIED_SOUND) */
static bool grass_code(uint32_t at, uint8_t out[GRASS_CODE_SIZE]) {
	static const uint16_t code[] = {
		0x8138,           /* strh r0,[r7,#8] */
		0x7CA8,           /* ldrb r0,[r5,#0x12] */
		0x7CE9,           /* ldrb r1,[r5,#0x13] */
		0x2202,           /* movs r2,#2 (BN6_PANEL_PLAIN) */
		0x4B02,           /* ldr r3,[pc,#8]: the panel's routine */
		0, 0,             /* bl the bx below */
		0x4B02,           /* ldr r3,[pc,#8]: the way on */
		0x4718,           /* bx r3 */
		0x46C0,           /* nop */
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i) put16(out + 2 * i, code[i]);
	put32(out + 20, BN6_SET_PANEL_TYPE | 1);
	put32(out + 24, BN6_CHIP_MODIFIED_SOUND | 1);
	return BN6_PANEL_PLAIN == 2 && thumb_bl_write(out, 10, at, at + 16);
}

static bool expected(void) {
	static const struct { uint32_t at; uint16_t op; } code[] = {
		{ BN6_CHARGE_TEST, 0xB5D0 }, { BN6_CHARGE_BONUS, 0xB510 }, { BN6_CHIP_MODIFIED, 0x8138 }, { BN6_CHIP_MODIFIED_SOUND, 0x2087 },
		{ BN6_CHIP_USED_END, 0x8AB8 }, { BN6_CHIP_FAMILY_READ, 0x7AE0 }, { BN6_SHADOW_TYPE_SIDE, 0x2129 }, { BN6_SHADOW_TYPE_DONE, 0xBD10 },
		{ BN6_SET_PANEL_TYPE, 0xB570 }, { BN6_CHARGED_RELEASE, 0x290D }, { BN6_CHARGED_CHIP_USE, 0x2001 },
	};
	for (size_t i = 0; i < sizeof code / sizeof *code; ++i)
		if (rom_u16(code[i].at - BUS) != code[i].op) {
			fprintf(stderr, "soulchips: %08x is not the instruction expected\n", (unsigned)code[i].at);
			return false;
		}
	return true;
}


bool soulchips_install(int brk) {
	if (done) return true;
	if (!R.data || !emu_ready() || !expected() || !soulkinds_make()) return false;
	breaks = brk;
	sword_family = (uint32_t)soulcode_family(BN6_FAMILY_SWORD);
	uint8_t stub[THUMB_SIZE];
	if ((int)sword_family < 0 || !grass_code(SOULS_THUMB, stub)) return false;
	put16(stub + ANSWER_NO, THUMB_MOVS_R0 | 0);
	put16(stub + ANSWER_NO + 2, THUMB_BX_LR);
	put16(stub + ANSWER_YES, THUMB_MOVS_R0 | 1);
	put16(stub + ANSWER_YES + 2, THUMB_BX_LR);
	claims_begin(SOULCHIPS_CLAIMS);
	emu_write(SOULS_THUMB, stub, sizeof stub);
	bool ok = emu_hook(BN6_CHARGE_TEST, charge_test, NULL) && emu_hook(BN6_CHARGE_BONUS, charge_bonus, NULL) &&
		emu_hook(BN6_CHARGED_RELEASE, release, (void *)(uintptr_t)BN6_CHARGED_CHIP_USE) &&
		emu_hook(BN6_CHIP_MODIFIED, grass, (void *)(uintptr_t)SOULS_THUMB) && emu_hook(BN6_CHIP_USED_END, front, NULL) &&
		emu_hook(BN6_CHIP_FAMILY_READ, sword, (void *)(uintptr_t)BN6_CHIP_FAMILY_NEXT) &&
		emu_hook(BN6_SHADOW_TYPE_SIDE, shadow, (void *)(uintptr_t)BN6_SHADOW_TYPE_DONE);
	claims_end();
	done = ok;
	printf("soulchips: the Souls' chips%s%s\n", ok ? ": the chip charge, its double, TomahawkSoul's grass, KnightSoul's front column, "
		"ShadowSoul's sword and shadow" : " NOT placed whole", brk ? " (soulbreak: parts of it broken, the differential's own test)" : "");
	return ok;
}

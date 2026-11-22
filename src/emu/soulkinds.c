/* BN6's chips as BN5's UNITE reads them (soulkinds.h). */
#include "soulkinds.h"

#include <stdint.h>
#include <stdio.h>

#include "bn4.h"
#include "bn5.h"
#include "bn6.h"
#include "chips.h"
#include "data.h"
#include "rom.h"
#include "xchips.h"
#include "xchips_registry.h"

#define CHIPS CHIPS_IDS   /* the chip ids a folder holds: BN6's (1-313) and the registry's of BN5's (chips.h), to 511 */
#define BN5_CHIP_DARK 0x20   /* a BN5 chip record's +9 (BN6_CHIP_EFFECT_FLAGS'): a DarkChip (its UNITE check 0x08024B76) */

#define BUS 0x08000000u
#define BN4_KIND_CURSOR4 8   /* BN4_CHIP_KIND's 8, panel-cracking, where BN5's 8 is Cursor */
#define BN4_CHIPS_NAMED 389  /* (BN4_CHIP_COUNT) */

static uint8_t kind[CHIPS], dark[CHIPS], kind4[CHIPS];
static char names6[CHIPS][XCHIP_NAME];   /* (BN6's chips' names, soulkinds_make's: BN4's namesakes paired with them later, make4) */
static bool made, made4;

int soulkinds_bn5(int bn6_kind) {
	/* (BN6's 0-10 by bn6f's CHIP_ELEM_*: Fire, Aqua, Elec, Wood, Plus, Sword, Cursor, Obstacle, Wind, Break, none;
	 * BN5's 0-12, BN5_CHIP_KIND: Fire, Aqua, Elec, Wood, Recovery, Plus, Sword, Invisible, Cursor, Obstacle, Wind,
	 * Break, none; read from both games' records of the chips both have, docs/rom/) */
	static const uint8_t as5[] = { 0, 1, 2, 3, 5, 6, 8, 9, 10, 11, SOULKIND_NONE };
	return bn6_kind >= 0 && bn6_kind < (int)sizeof as5 ? as5[bn6_kind] : SOULKIND_NONE;
}

bool soulkinds_make(void) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (made) return true;
	if (!R.data || !x->data) return false;
	static char names5[BN5_CHIPS][XCHIP_NAME];
	static uint16_t to5[CHIPS], from5[BN5_CHIPS];
	for (int id = 1; id < CHIPS; ++id) {
		ChipInfo ci;
		chip_info(id, &ci);
		snprintf(names6[id], sizeof names6[id], "%s", ci.name);
	}
	for (int i = 1; i < BN5_CHIPS; ++i)
		xrom_text(XROM_BN5_COLONEL_US, i < 256 ? BN5_CHIP_NAMES_LOW : BN5_CHIP_NAMES_HIGH, i < 256 ? i : i - 256, names5[i], sizeof names5[i]);
	int paired = xchips_pair((const char (*)[XCHIP_NAME])names6, CHIPS, (const char (*)[XCHIP_NAME])names5, BN5_CHIPS, to5, from5);
	for (int id = 0; id < CHIPS; ++id) {
		/* (a chip's record as the game reads it, chips.h: the registry's of BN5's go by their BN5 namesake, themselves) */
		const uint8_t *rec6 = chips_record(id);
		uint32_t rec5 = BN5_CHIP_RECORDS + (uint32_t)to5[id] * BN6_CHIP_RECORD_SIZE;
		kind[id] = (uint8_t)(to5[id] ? x->data[rec5 + BN5_CHIP_KIND] : rec6 ? soulkinds_bn5(rec6[BN6_CHIP_ELEMENT]) : SOULKIND_NONE);
		dark[id] = (uint8_t)(to5[id] ? (x->data[rec5 + BN6_CHIP_EFFECT_FLAGS] & BN5_CHIP_DARK) != 0 :
			rec6 && (rec6[BN6_CHIP_EFFECT_FLAGS] & BN6_CHIP_DARK_CLASS) != 0);
	}
	made = true;
	printf("soulkinds: %d of BN6's chips go by their BN5 namesake's kind, the rest by their own\n", paired);
	return true;
}

/* BN4's kinds, once BN4's ROM is read (its read may come after BN5's kinds are made): a BN4 chip's own (its registry row), a
 * chip's BN4 namesake's, else BN5's with no Cursor, BN4's 8 being panel-cracking */
static bool make4(void) {
	const XRom *x4 = &XR[XROM_BN4_RED_SUN_US];
	if (made4) return true;
	if (!made || !x4->data) return false;
	static char names4[BN4_CHIPS_NAMED][XCHIP_NAME];
	static uint16_t to4[CHIPS], from4[BN4_CHIPS_NAMED];
	for (int i = 1; i < BN4_CHIPS_NAMED; ++i)
		xrom_text(XROM_BN4_RED_SUN_US, i < 256 ? BN4_CHIP_NAMES : BN4_CHIP_NAMES_HI, i < 256 ? i : i - 256, names4[i], sizeof names4[i]);
	int paired = xchips_pair((const char (*)[XCHIP_NAME])names6, CHIPS, (const char (*)[XCHIP_NAME])names4, BN4_CHIPS_NAMED, to4, from4), own = 0;
	for (int id = 0; id < CHIPS; ++id) {
		/* (a BN4 chip of the registry by its own id, else a namesake's) */
		const XChipReg4 *r4 = xreg4_at(id);
		int bn4 = r4 ? r4->bn4 : to4[id];
		own += r4 != NULL;
		kind4[id] = (uint8_t)(bn4 ? x4->data[BN4_CHIP_RECORDS - BUS + (uint32_t)bn4 * BN6_CHIP_RECORD_SIZE + BN4_CHIP_KIND] :
			kind[id] == BN4_KIND_CURSOR4 ? SOULKIND_NONE : kind[id]);
	}
	made4 = true;
	printf("soulkinds: BN4's kinds for BN4's Souls: %d of BN4's own chips, %d of BN6's by a BN4 namesake\n", own, paired);
	return true;
}

int soulkinds_bn4_of(int chip) { return make4() && chip > 0 && chip < CHIPS ? kind4[chip] : SOULKIND_NONE; }

int soulkinds_of(int chip) { return made && chip > 0 && chip < CHIPS ? kind[chip] : SOULKIND_NONE; }

bool soulkinds_dark(int chip) { return made && chip > 0 && chip < CHIPS && dark[chip]; }

int soulkinds_lights(const SoulTables *t, int of, bool is_dark, unsigned held, uint32_t used, int *chaos) {
	*chaos = 0;
	for (int s = 1; s <= SOULKINDS_BN5; ++s) {
		if (t->kinds5[s - 1] != of) continue;
		int k = t->bits5[s - 1];
		/* (the other version's Soul, or one the run has not won: BN4's of the kind, below) */
		if (k < 0 || !(held >> k & 1)) break;
		if (used & (1u << s) << (is_dark ? 16 : 0)) return 0;
		*chaos = is_dark;
		return s;
	}
	for (int k = 0; k < t->n4; ++k) {
		if (t->kinds4[k] != (t->by4 ? t->kind4 : of)) continue;
		int s = t->first4 + k;
		if (is_dark || !(t->moved4 >> k & 1) || !(held >> (t->held4 + k) & 1) || (used & 1u << s)) return 0;
		return s;
	}
	return 0;
}

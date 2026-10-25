/* BN6's chips as BN5's UNITE reads them (soulkinds.h). */
#include "soulkinds.h"

#include <stdint.h>
#include <stdio.h>

#include "bn5.h"
#include "bn6.h"
#include "chips.h"
#include "data.h"
#include "rom.h"
#include "xchips.h"

#define CHIPS CHIPS_IDS   /* the chip ids a folder holds: BN6's (1-313) and the registry's of BN5's (chips.h), to 511 */
#define BN5_CHIP_DARK 0x20   /* a BN5 chip record's +9 (BN6_CHIP_EFFECT_FLAGS'): a DarkChip (its UNITE check 0x08024B76) */

static uint8_t kind[CHIPS], dark[CHIPS];
static bool made;

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
	static char names6[CHIPS][XCHIP_NAME], names5[BN5_CHIPS][XCHIP_NAME];
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

int soulkinds_of(int chip) { return made && chip > 0 && chip < CHIPS ? kind[chip] : SOULKIND_NONE; }

bool soulkinds_dark(int chip) { return made && chip > 0 && chip < CHIPS && dark[chip]; }

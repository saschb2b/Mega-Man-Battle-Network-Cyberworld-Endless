/* BN4 Red Sun's Navis as a run meets them (xguardian_bn4.h). */
#include "xguardian_bn4.h"

#include <stddef.h>
#include <stdint.h>

#include "bn4.h"
#include "bytes.h"
#include "rom.h"

#define BUS 0x08000000u

/* The proved: BN4's V1 enemy id (his seat (id - BN4_NAVI_FIRST_ID) / 4), his plate on BN4's bracket (BN4_ENTRANT_ID, named
 * BN4_PLATE_*, tied to his Navi from BN4's data: docs/multirom/bn4-tournament.md, Each plate's Navi), his overworld sprite's
 * number in BN4's list 6 (his mugshot's the same in its list 8, BN4_OW_*: docs/rom/bn4.md, Sprite lists), the BN6 Navi whose
 * chip stands in for his own until his plays (his element's, else his kind's: docs/game/bosses/bn4-navis.md) and his look on
 * a card. A Navi joins with his line once his spec PASSes (docs/multirom/bn4-navis.md), every mutation of it caught. */
static const struct {
	uint8_t v1, plate, ow, kin;
	Guardian look;
} proved[] = {
	{ 0x84, BN4_PLATE_GUTSMAN, BN4_OW_GUTSMAN, 9, { "GutsMan", "The Pounding Fist", GUARDIAN_NO_MUGSHOT, -1, 232, 152, 56 } },   /* tests/xdiff/gutsman_v1.json */
	{ 0x88, BN4_PLATE_WINDMAN, BN4_OW_WINDMAN, 8, { "WindMan", "Master of the Wind", GUARDIAN_NO_MUGSHOT, -1, 120, 200, 168 } },   /* tests/xdiff/windman_v1.json */
	{ 0x8C, BN4_PLATE_SEARCHMAN, BN4_OW_SEARCHMAN, 4, { "SearchMan", "The Unseen Marksman", GUARDIAN_NO_MUGSHOT, -1, 136, 168, 80 } },   /* tests/xdiff/searchman_v1.json */
	{ 0x90, BN4_PLATE_FIREMAN, BN4_OW_FIREMAN, 1, { "FireMan", "The Blazing Furnace", GUARDIAN_NO_MUGSHOT, -1, 232, 80, 48 } },   /* tests/xdiff/fireman_v1.json */
	{ 0x94, BN4_PLATE_THUNDERMAN, BN4_OW_THUNDERMAN, 2, { "ThunderMan", "The Thundering Cloud", GUARDIAN_NO_MUGSHOT, -1, 240, 200, 48 } },   /* tests/xdiff/thunderman_v1.json */
	{ 0xA8, BN4_PLATE_AQUAMAN, BN4_OW_AQUAMAN, 6, { "AquaMan", "The Rushing Tide", GUARDIAN_NO_MUGSHOT, -1, 72, 152, 240 } },   /* tests/xdiff/aquaman_v1.json */
	{ 0x80, BN4_PLATE_ROLL, BN4_OW_ROLL, 14, { "Roll", "The Summoning Petal", GUARDIAN_NO_MUGSHOT, -1, 248, 120, 176 } },   /* tests/xdiff/roll_v1.json */
	{ 0xAC, BN4_PLATE_WOODMAN, BN4_OW_WOODMAN, 7, { "WoodMan", "The Towering Timber", GUARDIAN_NO_MUGSHOT, -1, 112, 192, 64 } },   /* tests/xdiff/woodman_v1.json */
	{ 0xA4, BN4_PLATE_JUNKMAN, BN4_OW_JUNKMAN, 10, { "JunkMan", "The Scrapheap Storm", GUARDIAN_NO_MUGSHOT, -1, 184, 136, 80 } },   /* tests/xdiff/junkman_v1.json */
	{ 0xB0, BN4_PLATE_TOPMAN, BN4_OW_TOPMAN, 5, { "TopMan", "The Whirling Top", GUARDIAN_NO_MUGSHOT, -1, 224, 72, 64 } },   /* tests/xdiff/topman_v1.json */
	{ 0xC0, BN4_PLATE_SPARKMAN, BN4_OW_SPARKMAN, 2, { "SparkMan", "The Crackling Spark", GUARDIAN_NO_MUGSHOT, -1, 248, 216, 64 } },   /* tests/xdiff/sparkman_v1.json */
	{ 0xB8, BN4_PLATE_BURNERMAN, BN4_OW_BURNERMAN, 1, { "BurnerMan", "The Roaring Burner", GUARDIAN_NO_MUGSHOT, -1, 240, 112, 40 } },   /* tests/xdiff/burnerman_v1.json */
};
#define NPROVED ((int)(sizeof proved / sizeof *proved))

/* (his line of the proved, -1 none) */
static int line_of(int navi) {
	for (int k = 0; guardian_bn4(navi) && k < NPROVED; ++k)
		if (GUARDIAN_BN4_FIRST + (proved[k].v1 - BN4_NAVI_FIRST_ID) / 4 == navi) return k;
	return -1;
}

int xguardian_bn4_count(void) { return NPROVED; }

int xguardian_bn4_navi(int k) { return k >= 0 && k < NPROVED ? GUARDIAN_BN4_FIRST + (proved[k].v1 - BN4_NAVI_FIRST_ID) / 4 : 0; }

int xguardian_bn4_v1(int navi) {
	int k = line_of(navi);
	return k >= 0 ? proved[k].v1 : -1;
}

const Guardian *xguardian_bn4_look(int navi) {
	int k = line_of(navi);
	return k >= 0 ? &proved[k].look : NULL;
}

int xguardian_bn4_plate(int navi) {
	int k = line_of(navi);
	return k >= 0 ? proved[k].plate : -1;
}

int xguardian_bn4_ow(int navi) {
	int k = line_of(navi);
	return k >= 0 ? proved[k].ow : 0;
}

int xguardian_bn4_kin(int navi) {
	int k = line_of(navi);
	return k >= 0 ? proved[k].kin : 0;
}

int xguardian_bn4_hp(int navi, int version) {
	int k = line_of(navi);
	const uint8_t *d = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t at = BN4_ENEMY_STATS - BUS + (uint32_t)(k >= 0 ? proved[k].v1 + version : 0) * BN4_ENEMY_STAT_SIZE + BN4_ENEMY_HP;
	if (k < 0 || version < 0 || version > 3 || !d || at + 2 > ROM_SIZE) return -1;
	return get16(d + at) & 0x0FFF;
}

int xguardian_bn4_element(int navi) {
	int k = line_of(navi);
	const uint8_t *d = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t at = BN4_ENEMY_STATS - BUS + (uint32_t)(k >= 0 ? proved[k].v1 : 0) * BN4_ENEMY_STAT_SIZE + BN4_ENEMY_HP;
	if (k < 0 || !d || at + 2 > ROM_SIZE) return 0;
	/* (BN4's element numbers BN6's: 1 Fire, 2 Aqua, 3 Elec, 4 Wood) */
	int e = get16(d + at) >> 12;
	return e <= 4 ? e : 0;
}

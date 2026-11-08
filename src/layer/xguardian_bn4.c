/* BN4 Red Sun's Navis as a run meets them (xguardian_bn4.h). */
#include "xguardian_bn4.h"

#include <stddef.h>
#include <stdint.h>

#include "bn4.h"
#include "bytes.h"
#include "rom.h"

#define BUS 0x08000000u

/* The proved: BN4's V1 enemy id (his seat (id - BN4_NAVI_FIRST_ID) / 4), his plate on BN4's bracket (BN4_ENTRANT_ID, named
 * BN4_PLATE_*, tied to his Navi from BN4's data: docs/multirom/bn4-tournament.md, Each plate's Navi) and his look on a card. A Navi joins with his line once his spec PASSes
 * (docs/multirom/bn4-navis.md), every mutation of it caught. */
static const struct {
	uint8_t v1, plate;
	Guardian look;
} proved[] = {
	{ 0x84, BN4_PLATE_GUTSMAN, { "GutsMan", "The Pounding Fist", GUARDIAN_NO_MUGSHOT, -1, 232, 152, 56 } },   /* tests/xdiff/gutsman_v1.json */
	{ 0x88, BN4_PLATE_WINDMAN, { "WindMan", "Master of the Wind", GUARDIAN_NO_MUGSHOT, -1, 120, 200, 168 } },   /* tests/xdiff/windman_v1.json */
	{ 0x8C, BN4_PLATE_SEARCHMAN, { "SearchMan", "The Unseen Marksman", GUARDIAN_NO_MUGSHOT, -1, 136, 168, 80 } },   /* tests/xdiff/searchman_v1.json */
	{ 0x90, BN4_PLATE_FIREMAN, { "FireMan", "The Blazing Furnace", GUARDIAN_NO_MUGSHOT, -1, 232, 80, 48 } },   /* tests/xdiff/fireman_v1.json */
	{ 0x94, BN4_PLATE_THUNDERMAN, { "ThunderMan", "The Thundering Cloud", GUARDIAN_NO_MUGSHOT, -1, 240, 200, 48 } },   /* tests/xdiff/thunderman_v1.json */
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

int xguardian_bn4_hp(int navi, int version) {
	int k = line_of(navi);
	const uint8_t *d = XR[XROM_BN4_RED_SUN_US].data;
	uint32_t at = BN4_ENEMY_STATS - BUS + (uint32_t)(k >= 0 ? proved[k].v1 + version : 0) * BN4_ENEMY_STAT_SIZE + BN4_ENEMY_HP;
	if (k < 0 || version < 0 || version > 3 || !d || at + 2 > ROM_SIZE) return -1;
	return get16(d + at) & 0x0FFF;
}

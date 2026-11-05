/* BN4 Red Sun's Navis as a run meets them (xguardian_bn4.h). */
#include "xguardian_bn4.h"

#include <stddef.h>
#include <stdint.h>

#include "bn4.h"
#include "bytes.h"
#include "data.h"
#include "rom.h"
#include "run.h"
#include "xnavi_install.h"

#define BUS 0x08000000u

/* The proved: BN4's V1 enemy id (his seat (id - BN4_NAVI_FIRST_ID) / 4), his plate on BN4's bracket (BN4_ENTRANT_ID, named
 * by sight: docs/multirom/bn4-tournament.md) and his look on a card. A Navi joins with his line once his spec PASSes
 * (docs/multirom/bn4-navis.md), every mutation of it caught. */
static const struct {
	uint8_t v1, plate;
	Guardian look;
} proved[] = {
	{ 0x84, 0x00, { "GutsMan", "The Pounding Fist", GUARDIAN_NO_MUGSHOT, -1, 232, 152, 56 } },   /* tests/xdiff/gutsman_v1.json */
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

bool xguardian_bn4_fightable(int navi) {
	int k = line_of(navi);
	for (int v = 0; k >= 0 && v < 4; ++v)
		if (xnavi_bn4_id(proved[k].v1 + v) < 0) return false;
	return k >= 0;
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

Encounter xguardian_bn4_encounter(int navi, int version) {
	Encounter e = { 0 };
	e.biome = run.biome;
	e.from_pick = -1;
	for (int i = 0; i < MAX_FOES; ++i) e.foes[i].id = -1;
	int k = line_of(navi), id = k >= 0 && version >= 0 && version <= 3 ? xnavi_bn4_id(proved[k].v1 + version) : -1;
	if (id < 0) return e;
	const uint8_t *t = enemy_triple(id);
	e.boss = true;
	e.held = true;
	e.nfoes = 1;
	/* (where BN6's guardians stand, make_boss's panel, as BN5's: xguardian_encounter) */
	e.foes[0] = (Foe){ FOE_NAVI, t ? t[2] : 0, version, 4, 1, id };
	return e;
}

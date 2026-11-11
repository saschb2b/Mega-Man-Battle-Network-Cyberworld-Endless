/* Another game's Navis as a territory's guardians (xguardian.h). */
#include "xguardian.h"

#include <stdio.h>
#include <string.h>

#include "bn4.h"
#include "bn5.h"
#include "bn6.h"
#include "chips.h"
#include "data.h"
#include "guardians.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "save.h"
#include "souls.h"
#include "xchips.h"
#include "xchips_registry.h"
#include "xnavi.h"
#include "xnavi_install.h"
#include "xrecords.h"

#define XROM XROM_BN5_COLONEL_US

/* (his BN6 enemy id at `version`, 0 V1 .. 3 SP; -1 none) */
static int xguardian_id(int navi, int version) {
	const uint8_t *d = XR[XROM].data;
	int bn5 = d && guardian_bn5_ai(navi) >= 0 ? xrecords_navi_id(d, guardian_bn5_ai(navi), version) : 0;
	return bn5 ? xnavi_bn6_id(bn5) : -1;
}

bool xguardian_fightable(int navi) {
	if (guardian_bn4(navi)) return xguardian_bn4_fightable(navi);
	/* (V1 to SP: the story's versions and the net's, every one a run takes) */
	for (int v = 0; v < 4; ++v)
		if (xguardian_id(navi, v) < 0) return false;
	return true;
}

/* (a moved Navi's battle: his BN6 id `id` at `version`; none where -1) */
static Encounter moved_encounter(int id, int version) {
	Encounter e = { 0 };
	e.biome = run.biome;
	e.from_pick = -1;
	for (int i = 0; i < MAX_FOES; ++i) e.foes[i].id = -1;
	if (id < 0) return e;
	const uint8_t *t = enemy_triple(id);
	e.boss = true;
	e.held = true;
	e.nfoes = 1;
	/* (where BN6's guardians stand, make_boss's panel; his moved id, the
	 * engine's family his AI index there) */
	e.foes[0] = (Foe){ FOE_NAVI, t ? t[2] : 0, version, 4, 1, id };
	return e;
}

Encounter xguardian_encounter(int navi, int version) {
	return guardian_bn4(navi) ? xguardian_bn4_encounter(navi, version) : moved_encounter(xguardian_id(navi, version), version);
}

bool xguardian_bn4_fightable(int navi) {
	int v1 = xguardian_bn4_v1(navi);
	for (int v = 0; v1 >= 0 && v < 4; ++v)
		if (xnavi_bn4_id(v1 + v) < 0) return false;
	return v1 >= 0;
}

Encounter xguardian_bn4_encounter(int navi, int version) {
	int v1 = xguardian_bn4_v1(navi);
	return moved_encounter(v1 >= 0 && version >= 0 && version <= 3 ? xnavi_bn4_id(v1 + version) : -1, version);
}

int xguardian_named(int xrom, int id) {
	if (xrom == XROM_BN4_RED_SUN_US) return id >= BN4_NAVI_FIRST_ID ? GUARDIAN_BN4_FIRST + (id - BN4_NAVI_FIRST_ID) / 4 : 0;
	return xrom == XROM && id ? guardian_of_older(id) : 0;
}

int xguardian_hp(int navi, int version) {
	if (guardian_bn4(navi)) return xguardian_bn4_hp(navi, version);
	return guardian_bn5_ai(navi) >= 0 ? xrecords_navi_hp(XROM, guardian_bn5_ai(navi), version, NULL) : -1;
}

int xguardian_version(int navi, int depth) {
	int hp[4];
	for (int v = 0; v < 4; ++v) hp[v] = xguardian_hp(navi, v);
	/* (threat 4, docs/game/meta/: as BN6's guardians come at EX from act 2) */
	return pacing_xguardian_version(hp, pacing_act(depth), pacing_loop(depth), run.threat >= 4);
}

int xguardian_hp_fought(int navi, int version, int depth) {
	int hp = xguardian_hp(navi, version);
	return hp < 0 ? hp : pacing_xguardian_hp(hp, pacing_act(depth), pacing_loop(depth));
}

int guardian_element(int navi) {
	int e = 0;
	if (guardian_older(navi)) xrecords_navi_hp(XROM, guardian_older_ai(navi), 0, &e);
	else if (guardian_bn4(navi)) e = xguardian_bn4_element(navi);
	else e = navi > 0 ? enemy_element(enemy_id(1, navi, 0)) : 0;
	return e > 0 ? e : 0;
}

int guardian_weakness(int navi) {
	static const int beats[ELEM_WOOD + 1] = { 0, ELEM_AQUA, ELEM_ELEC, ELEM_WOOD, ELEM_FIRE };
	if (!guardian_moved(navi)) return navi > 0 ? enemy_weakness(enemy_id(1, navi, 0)) : -1;
	int e = guardian_element(navi);
	return e > 0 && e <= ELEM_WOOD ? beats[e] : 0;
}

/* (the chip kind his Soul unites with in his game, BN5_CHIP_KIND; -1 none) */
static int xguardian_kind(int navi) { return guardian_older(navi) ? xrecords_soul_kind(XROM, guardian_older_ai(navi)) : -1; }

/* BN5's V1 Navi chip of each of Team Colonel's six, by AI index from
 * Colonel (BN5's chip ids: its SP the next, its DS the one after; the
 * registry's rows of these names, xchips_registry.c) */
static const uint16_t navi_chips[6] = { 245, 248, 242, 251, 254, 257 };

/* BN6's id of his Navi chip `v` (0 V1, 1 SP) where the registry has it and
 * it plays in BN6's battles here; 0 none */
static int navi_chip_of(int navi, int v) {
	const XChipReg *r = guardian_older(navi) ? xreg_of_bn5(navi_chips[guardian_older_ai(navi) - BN5_NAVI_COLONEL] + v) : NULL;
	return r && !chips_sits_out(r->bn6) && chips_record(r->bn6) ? r->bn6 : 0;
}

/* BN4's Navi `navi`'s own chip `v` (0 V1, 1 SP), BN6's id, where the registry has it and it plays in BN6's battles here: BN4's
 * Navi chips three a seat from Roll's (BN4_NAVI_CHIPS: V1, SP, DS); 0 none */
static int bn4_chip_of(int navi, int v) {
	int seat = navi - GUARDIAN_BN4_FIRST;
	const XChipReg4 *r = guardian_bn4(navi) ? xreg4_of(BN4_NAVI_CHIPS + 3 * seat + v) : NULL;
	return r && !chips_sits_out(r->bn6) && chips_record(r->bn6) ? r->bn6 : 0;
}

/* (his chip, the version his act gives: his own game's V1 in the first three acts, his SP from the fourth (where BN6's
 * guardians give EX: BN5 and BN4 have none between), his V1 where his SP sits out; BN4's where his own sits out, his BN6 kin's
 * at the version he is fought at, V2 and V3 as EX: xguardian_bn4_kin) */
static int chip_at(int navi, int depth) {
	bool late = pacing_act(depth) >= 3 || pacing_loop(depth) > 0;
	if (guardian_bn4(navi)) {
		int id = late ? bn4_chip_of(navi, 1) : 0;
		if (!id) id = bn4_chip_of(navi, 0);
		int v = xguardian_version(navi, depth), kin = xguardian_bn4_kin(navi);
		return id ? id : kin ? navi_chip(kin, v <= 0 ? 0 : v >= 3 ? 2 : 1) : 0;
	}
	int id = late ? navi_chip_of(navi, 1) : 0;
	return id ? id : navi_chip_of(navi, 0);
}

int xguardian_chip(int navi, int depth, int *code) {
	*code = 26;
	int id = chip_at(navi, depth);
	if (id <= 0) return 0;
	/* (its letter where the folder holds it, else its *, else its first: a
	 * guardian's Navi chip comes so, guardian_objs.c) */
	ChipInfo ci;
	chip_info(id, &ci);
	char c = ci.ncodes ? ci.codes[0] : '*';
	bool held = c == '*';
	for (int k = 0; k < 3 && run.codes[k]; ++k) held |= c == 'A' + run.codes[k] - 1;
	if (!held && memchr(ci.codes, '*', (size_t)ci.ncodes)) c = '*';
	*code = c == '*' ? 26 : c - 'A';
	return id;
}

bool xguardian_navi_chip(int chip) {
	for (int k = 0; chip > 0 && k < 6; ++k)
		for (int v = 0; v < 3; ++v) {
			const XChipReg *r = xreg_of_bn5(navi_chips[k] + v);
			if (r && r->bn6 == chip) return true;
		}
	/* (BN4's of the Navis a run meets: BN4_NAVI_CHIPS' three a seat) */
	for (int k = 0; chip > 0 && k < xguardian_bn4_count(); ++k)
		for (int v = 0; v < 3; ++v) {
			const XChipReg4 *r = xreg4_of(BN4_NAVI_CHIPS + 3 * (xguardian_bn4_navi(k) - GUARDIAN_BN4_FIRST) + v);
			if (r && r->bn6 == chip) return true;
		}
	return false;
}

const char *xguardian_soul_words(int navi) {
	int kind = xguardian_kind(navi);
	if (kind < 0 || kind >= 12) return NULL;
	/* (held already, won from him in an earlier act or cycle: the layer's
	 * words are made with the run's Souls as its checkpoint keeps them, so
	 * a CONTINUE before his battle says it as the first time did) */
	if (soul_held(navi)) return guardian_soul_words(navi, kind, true, "", false);
	/* (one of the run's chips of his kind, as BN5's UNITE reads them) */
	uint16_t ids[64];
	ChipInfo ci = { .name = "" };
	if (xchips_kind(kind, ids, 64) > 0) chip_info(ids[0], &ci);
	return guardian_soul_words(navi, kind, false, ci.name, !profile.soul_form_taught);
}

int xguardian_slot(int navi) {
	if (guardian_bn4(navi)) return xguardian_bn4_ow(navi) ? xnavi_guardian(XROM_BN4_RED_SUN_US, xguardian_bn4_ow(navi)) : -1;
	int sprite = guardian_older_sprite(navi);
	return sprite ? xnavi_guardian(XROM, sprite) : -1;
}

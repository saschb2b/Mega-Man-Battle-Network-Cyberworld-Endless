/* Another game's Navis as a territory's guardians (xguardian.h). */
#include "xguardian.h"

#include <string.h>

#include "data.h"
#include "guardians.h"
#include "guest.h"
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "xnavi.h"

#define XROM XROM_BN5_COLONEL_US

int xguardian_hp(int navi, int version) {
	return guardian_older(navi) ? guest_navi_hp(XROM, guardian_older_ai(navi), version, NULL) : -1;
}

int xguardian_version(int navi, int depth) {
	int hp[4];
	for (int v = 0; v < 4; ++v) hp[v] = xguardian_hp(navi, v);
	/* (threat 4, docs/META.md: as BN6's guardians come at EX from act 2) */
	return pacing_xguardian_version(hp, pacing_act(depth), pacing_loop(depth), run.threat >= 4);
}

int xguardian_hp_fought(int navi, int version, int depth) {
	int hp = xguardian_hp(navi, version);
	return hp < 0 ? hp : pacing_xguardian_hp(hp, pacing_act(depth), pacing_loop(depth));
}

int guardian_element(int navi) {
	int e = 0;
	if (guardian_older(navi)) guest_navi_hp(XROM, guardian_older_ai(navi), 0, &e);
	else e = navi > 0 ? enemy_element(enemy_id(1, navi, 0)) : 0;
	return e > 0 ? e : 0;
}

int guardian_weakness(int navi) {
	static const int beats[ELEM_WOOD + 1] = { 0, ELEM_AQUA, ELEM_ELEC, ELEM_WOOD, ELEM_FIRE };
	if (!guardian_older(navi)) return navi > 0 ? enemy_weakness(enemy_id(1, navi, 0)) : -1;
	int e = guardian_element(navi);
	return e > 0 && e <= ELEM_WOOD ? beats[e] : 0;
}

/* (the chip kind his Soul unites with in his game, BN5_CHIP_KIND; -1 none) */
static int xguardian_kind(int navi) { return guardian_older(navi) ? guest_soul_kind(XROM, guardian_older_ai(navi)) : -1; }

int xguardian_chip(int navi, uint32_t seed, int *code) {
	*code = 26;
	uint16_t ids[64];
	int n = guest_kind_chips(xguardian_kind(navi), ids, 64), common = 0;
	if (!n) return 0;
	/* (the rarest, BN6's rarity 4, only where there is nothing else: a
	 * Guardian or an Anubis is a deep find, not an act's key) */
	for (int i = 0; i < n; ++i) common += R.data[R.layout->chip_data + ids[i] * 0x2Cu + 5] < 4;
	int pick = (int)(seed % (uint32_t)(common ? common : n)), id = 0;
	for (int i = 0, k = 0; i < n && !id; ++i)
		if (!common || R.data[R.layout->chip_data + ids[i] * 0x2Cu + 5] < 4) {
			if (k++ == pick) id = ids[i];
		}
	/* (its letter where the folder holds it, else its *, else its first: a
	 * guardian's Navi chip comes so, guardian_objs.c) */
	ChipInfo ci;
	chip_info(id, &ci);
	char c = loot_fit_code(id, ci.ncodes ? ci.codes[0] : '*', true);
	bool held = c == '*';
	for (int k = 0; k < 3 && run.codes[k]; ++k) held |= c == 'A' + run.codes[k] - 1;
	if (!held && memchr(ci.codes, '*', (size_t)ci.ncodes)) c = '*';
	*code = c == '*' ? 26 : c - 'A';
	return id;
}

int xguardian_slot(int navi) {
	int sprite = guardian_older_sprite(navi);
	return sprite ? xnavi_guardian(XROM, sprite) : -1;
}

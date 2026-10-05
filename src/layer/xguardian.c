/* Another game's Navis as a territory's guardians (xguardian.h). */
#include "xguardian.h"

#include <stdio.h>
#include <string.h>

#include "bn5.h"
#include "data.h"
#include "guardians.h"
#include "guest.h"
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "save.h"
#include "souls.h"
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

/* BN5's DarkChip of chip kind `kind` (0-11, its id less BN5_DARK_FIRST), -1
 * none: the one that unites with a Soul of that kind */
static int dark_of_kind(int kind) {
	const uint8_t *d = XR[XROM].data;
	for (int k = 0; d && k < GUEST_DARK_KINDS; ++k)
		if (d[BN5_CHIP_RECORDS + 0x2Cu * (uint32_t)(BN5_DARK_FIRST + k) + BN5_CHIP_KIND] == kind) return k;
	return -1;
}

const char *xguardian_soul_words(int navi) {
	static char s[720];
	/* (the kinds by BN5_CHIP_KIND, as MegaMan says them) */
	static const char *const kinds[12] = { "Fire", "Aqua", "Elec", "Wood", "Recovery", "Plus", "Sword", "Invisible", "Cursor", "obstacle",
		"Wind", "Break" };
	const char *name = guardian(navi)->name;
	int kind = xguardian_kind(navi), dark = kind >= 0 ? dark_of_kind(kind) : -1;
	if (kind < 0 || kind >= 12) return NULL;
	/* (held already, won from him in an earlier act or cycle: the layer's
	 * words are made with the run's Souls as its checkpoint keeps them, so
	 * a CONTINUE before his battle says it as the first time did) */
	if (soul_held(navi)) {
		snprintf(s, sizeof s, "@M %s's Soul data... We already have his Soul!", name);
		return s;
	}
	/* (one of BN6's chips of his kind, as the Guardian Data's chip may be) */
	uint16_t ids[64];
	ChipInfo ci = { .name = "" };
	if (guest_kind_chips(kind, ids, 64) > 0) chip_info(ids[0], &ci);
	int k = snprintf(s, sizeof s, "MegaMan got:\n%s's\nSoul!!", name);
	if (profile.soul_taught)
		snprintf(s + k, sizeof s - (size_t)k, "|@M His Soul,Lan!|@M Our %s chips unite us in the older Net's battles.|@M Pick one,then UNITE on the Custom screen!",
			kinds[kind]);
	else {
		k += snprintf(s + k, sizeof s - (size_t)k, "|@M %s's Soul is in me,Lan!|@M It only wakes in the older Net's battles.|@M There,pick one of our %s "
			"chips%s%s.|@M Then UNITE on the Custom screen!|@M I'll fight with his Soul for a few turns.|@M Once a battle,and only while I'm calm.", name,
			kinds[kind], ci.name[0] ? ",like " : "", ci.name);
		if (dark >= 0 && k < (int)sizeof s)
			snprintf(s + k, sizeof s - (size_t)k, "|@M The DarkChip %s unites us too...|@M Darker. That's Chaos Unison.|@M Out here in our Net,his Soul sleeps.",
				guest_dark_name(dark));
	}
	return s;
}

int xguardian_slot(int navi) {
	int sprite = guardian_older_sprite(navi);
	return sprite ? xnavi_guardian(XROM, sprite) : -1;
}

/* Chip records (0x2C bytes, bn6f chip_data_struct): codes at 0, rarity 0-4
 * at 5, library type at 7 (0 standard, 1 Mega, 2 Giga, 3 secret, 4 Program
 * Advance), library number at 0x15 (0: not in the library) and library
 * flags at 0x16: bit 0 on the other version's Navi chips and Otenko, 0x30
 * on unused arm and dark chips, 0x10 alone on the Beast chips; its picture
 * at 0x24 and 0x28, one blank for both on the chips cut from this US
 * version. The Gigas (301-310) end the library; past Falzar (313) come
 * pseudo-chips. The registry's chips of BN5's (xchips_registry.h) hold
 * library number and flags 0 (out of BN6's Library screen) and come by
 * their registry row instead. */
#include "chip_pool.h"

#include <string.h>

#include "chips.h"
#include "data.h"
#include "game.h"
#include "rom.h"

#define MAX_CHIPS 420
#define LAST_CHIP 313

static short pool[2][CHIP_TIERS][CHIPS_IDS];
static int count[2][CHIP_TIERS];
static bool loaded, loaded_bn5;

/* (a record's tier by its rarity and Library type) */
static int tier_of(const uint8_t *c) {
	int rarity = c[5];
	switch (c[7]) {
	case 0: return rarity >= 3 ? 3 : rarity;
	case 1: return rarity >= 4 ? 4 : 3;
	case 2: return 4;
	default: return -1;
	}
}

bool chip_pool_older(int id) {
	const XChipReg *r = xreg_at(id);
	return r && xreg_own(r) && r->how != XREG_DARK && r->how != XREG_PA && r->type <= 2;
}

/* (one of BN5's own: by its registry row, where it plays in BN6's battles) */
static int older_tier(int id) {
	const uint8_t *c = chip_pool_older(id) && !chips_sits_out(id) && chips_from_bn5() ? chips_record(id) : NULL;
	if (!c || c[0] > 26 || !memcmp(c + 0x24, c + 0x28, 4)) return -1;
	ChipInfo ci;
	chip_info(id, &ci);
	return ci.name[0] ? tier_of(c) : -1;
}

int chip_pool_tier(int id) {
	if (chips_bn5(id)) return older_tier(id);
	const uint8_t *c = id > 0 && id <= LAST_CHIP ? chips_record(id) : NULL;
	if (!c || c[0] > 26 || !c[0x15]) return -1;   /* no code, or not in the library */
	int flags = c[0x16];
	if (flags & 0x01 || flags & 0x20 || flags == 0x10) return -1;
	/* (no picture of its own: a chip cut from the US version, which passes
	 * every rule above but HackJack's three, Megas whose Navi the US game
	 * no longer has: used, MegaMan vanished for good and was deleted, taken
	 * from an official vault, issue #15) */
	if (!memcmp(c + 0x24, c + 0x28, 4)) return -1;
	ChipInfo ci;
	chip_info(id, &ci);
	if (!ci.name[0]) return -1;             /* blank records */
	return tier_of(c);
}

/* BN6's chips by tier; and BN5's catalogue, where its ROM is read: every
 * chip of the registry's that plays, a chip both games name by BN6's id
 * (one chip per name) and a DarkChip of neither (they come from flames) */
static void load(void) {
	loaded = true;
	loaded_bn5 = chips_from_bn5();
	memset(count, 0, sizeof count);
	for (int id = 1; id < MAX_CHIPS; ++id) {
		int t = chips_bn5(id) ? -1 : chip_pool_tier(id);
		if (t >= 0) pool[CHIP_NET_BN6][t][count[CHIP_NET_BN6][t]++] = (short)id;
	}
	for (int i = 0; loaded_bn5 && i < xreg_count; ++i) {
		const XChipReg *r = &xreg_rows[i];
		int t = r->how == XREG_DARK || r->how == XREG_PA ? -1 : chip_pool_tier(r->bn6);
		bool again = false;
		for (int k = 0; t >= 0 && k < count[CHIP_NET_BN5][t] && !again; ++k) again = pool[CHIP_NET_BN5][t][k] == r->bn6;
		if (t >= 0 && !again) pool[CHIP_NET_BN5][t][count[CHIP_NET_BN5][t]++] = (short)r->bn6;
	}
}

/* (made again where BN5's ROM came or went since: the ROMs screen) */
static void ready(void) {
	if (!loaded || loaded_bn5 != chips_from_bn5()) load();
}

int chip_pool_class(int id) {
	if (chips_bn5(id) || chip_pool_tier(id) < 0) return -1;
	int t = chips_record(id)[7];
	return t <= 2 ? t : -1;
}

int chip_pool_class_count(int cls) {
	static int n[3] = { -1, -1, -1 };
	if (cls < 0 || cls > 2 || !R.data) return 0;
	if (n[cls] < 0) {
		n[cls] = 0;
		for (int id = 1; id <= LAST_CHIP; ++id) n[cls] += chip_pool_class(id) == cls;
	}
	return n[cls];
}

int chip_pool_older_count(void) {
	int n = 0;
	for (int id = 1; id < CHIPS_IDS; ++id) n += chips_bn5(id) && older_tier(id) >= 0;
	return n;
}

int chip_pool_pick(int tier) {
	if (!R.data) return -1;
	ready();
	/* the nearest tier that has chips */
	for (int d = 0; d < CHIP_TIERS; ++d)
		for (int s = -1; s <= 1; s += 2) {
			int t = tier + s * d;
			if (t >= 0 && t < CHIP_TIERS && count[CHIP_NET_BN6][t]) return pool[CHIP_NET_BN6][t][rng_range(0, count[CHIP_NET_BN6][t] - 1)];
		}
	return -1;
}

int chip_pool_pick_net(int tier, int net) {
	if (net == CHIP_NET_BN6) return chip_pool_pick(tier);
	if (!R.data || net != CHIP_NET_BN5 || tier < 0 || tier >= CHIP_TIERS) return -1;
	ready();
	int n = count[CHIP_NET_BN5][tier];
	return n ? pool[CHIP_NET_BN5][tier][rng_range(0, n - 1)] : -1;
}

int chip_pool_pick_if(int tier, int net, bool (*want)(int id)) {
	if (!R.data || net < CHIP_NET_BN6 || net > CHIP_NET_BN5 || tier < 0 || tier >= CHIP_TIERS) return -1;
	ready();
	int n = 0;
	for (int k = 0; k < count[net][tier]; ++k) n += want(pool[net][tier][k]);
	if (!n) return -1;
	int pick = rng_range(0, n - 1);
	for (int k = 0; k < count[net][tier]; ++k)
		if (want(pool[net][tier][k]) && pick-- == 0) return pool[net][tier][k];
	return -1;
}

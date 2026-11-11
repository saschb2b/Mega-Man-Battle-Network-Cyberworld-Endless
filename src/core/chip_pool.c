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

#include "bn4.h"
#include "bn6_chips.h"
#include "chips.h"
#include "data.h"
#include "game.h"
#include "rom.h"
#include "xchips.h"
#include "xchips_registry.h"

#define MAX_CHIPS 420
#define LAST_CHIP 313

/* A net's chips in the order they were listed, each with its tier: a pick
 * is the k-th of its tier in that order */
static short ids[CHIP_NETS][CHIPS_IDS];
static int8_t tiers[CHIP_NETS][CHIPS_IDS];
static int listed[CHIP_NETS], count[CHIP_NETS][CHIP_TIERS];
static bool loaded, loaded_bn5, loaded_bn4;

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
	if (r) return xreg_own(r) && r->how != XREG_DARK && r->how != XREG_PA && r->type <= 2;
	const XChipReg4 *r4 = xreg4_at(id);
	return r4 && r4->how != XREG_PA && r4->type <= 2 && r4->mb != XREG4_DARKCHIP_MB;
}

/* (one of BN5's own or BN4's: by its registry row, where it plays in BN6's
 * battles and its game's ROM is read, `read`; never one the pack has no
 * entry for yet, an id from BN6_PACK_WIDE_END: given, it would be lost) */
static int older_tier(int id, bool read) {
	const uint8_t *c = chip_pool_older(id) && !chips_sits_out(id) && read ? chips_record(id) : NULL;
	if (!c || c[0] > 26 || !memcmp(c + 0x24, c + 0x28, 4) || !bn6_pack_entry(BN6_PACK_EXT, (unsigned)id)) return -1;
	ChipInfo ci;
	chip_info(id, &ci);
	return ci.name[0] ? tier_of(c) : -1;
}

int chip_pool_tier(int id) {
	if (chips_bn5(id)) return older_tier(id, chips_from_bn5());
	if (chips_bn4(id)) return older_tier(id, chips_from_bn4());
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

/* (chip `id` of tier `t` listed in net `net`, once) */
static void add(int net, int id, int t) {
	for (int k = 0; k < listed[net]; ++k)
		if (ids[net][k] == id) return;
	if (t < 0 || listed[net] >= CHIPS_IDS) return;
	ids[net][listed[net]] = (short)id;
	tiers[net][listed[net]++] = (int8_t)t;
	++count[net][t];
}

/* (net `net`'s k-th chip of `tier`) */
static int nth(int net, int tier, int k) {
	for (int i = 0; i < listed[net]; ++i)
		if (tiers[net][i] == tier && k-- == 0) return ids[net][i];
	return -1;
}

/* BN6's chips by tier; BN5's catalogue, where its ROM is read: every chip
 * of the registry's that plays, a chip both games name by BN6's id (one
 * chip per name) and a DarkChip of neither (they come from flames); BN4's,
 * where its ROM is read: the run's chip of each of BN4's (xchips_bn4_paid:
 * BN6's of its name, the registry's of BN5's, its own row's where it
 * plays), no DarkChip nor dark chip of its own, no Program Advance; and
 * the whole, BN5's and then BN4's others */
static void load(void) {
	loaded = true;
	loaded_bn5 = chips_from_bn5();
	loaded_bn4 = chips_from_bn4();
	memset(count, 0, sizeof count);
	memset(listed, 0, sizeof listed);
	for (int id = 1; id < MAX_CHIPS; ++id) add(CHIP_NET_BN6, id, chips_bn5(id) ? -1 : chip_pool_tier(id));
	for (int i = 0; loaded_bn5 && i < xreg_count; ++i) {
		const XChipReg *r = &xreg_rows[i];
		add(CHIP_NET_BN5, r->bn6, r->how == XREG_DARK || r->how == XREG_PA ? -1 : chip_pool_tier(r->bn6));
	}
	for (int x = 1; loaded_bn4 && x < BN4_CHIP_COUNT; ++x) {
		int id = xchips_bn4_paid(x);
		/* (never a dark chip of its own, xreg4_dark: one costs 20 max HP a battle it is used in, and a find tells no
		 * price; a flame does. The owner's call 10 of docs/multirom/bn4-chips.md) */
		add(CHIP_NET_BN4, id, id > 0 && !xreg4_dark(id) ? chip_pool_tier(id) : -1);
	}
	for (int net = CHIP_NET_BN5; net <= CHIP_NET_BN4; ++net)
		for (int i = 0; i < listed[net]; ++i) add(CHIP_NET_OLDER, ids[net][i], tiers[net][i]);
}

/* (made again where BN5's ROM or BN4's came or went since: the ROMs screen) */
static void ready(void) {
	if (!loaded || loaded_bn5 != chips_from_bn5() || loaded_bn4 != chips_from_bn4()) load();
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

int chip_pool_pick(int tier) {
	if (!R.data) return -1;
	ready();
	/* the nearest tier that has chips */
	for (int d = 0; d < CHIP_TIERS; ++d)
		for (int s = -1; s <= 1; s += 2) {
			int t = tier + s * d;
			if (t >= 0 && t < CHIP_TIERS && count[CHIP_NET_BN6][t]) return nth(CHIP_NET_BN6, t, rng_range(0, count[CHIP_NET_BN6][t] - 1));
		}
	return -1;
}

int chip_pool_pick_net(int tier, int net) {
	if (net == CHIP_NET_BN6) return chip_pool_pick(tier);
	if (!R.data || net < 0 || net >= CHIP_NETS || tier < 0 || tier >= CHIP_TIERS) return -1;
	ready();
	int n = count[net][tier];
	return n ? nth(net, tier, rng_range(0, n - 1)) : -1;
}

int chip_pool_pick_if(int tier, int net, bool (*want)(int id)) {
	if (!R.data || net < 0 || net >= CHIP_NETS || tier < 0 || tier >= CHIP_TIERS) return -1;
	ready();
	int n = 0;
	for (int k = 0; k < listed[net]; ++k) n += tiers[net][k] == tier && want(ids[net][k]);
	if (!n) return -1;
	int pick = rng_range(0, n - 1);
	for (int k = 0; k < listed[net]; ++k)
		if (tiers[net][k] == tier && want(ids[net][k]) && pick-- == 0) return ids[net][k];
	return -1;
}

int chip_pool_count(int net, int tier) {
	if (!R.data || net < 0 || net >= CHIP_NETS || tier >= CHIP_TIERS) return 0;
	ready();
	return tier < 0 ? listed[net] : count[net][tier];
}

int chip_pool_list(int net, short *out, int most) {
	int n = chip_pool_count(net, -1);
	if (n > most) n = most;
	memcpy(out, ids[net < 0 || net >= CHIP_NETS ? 0 : net], (size_t)n * sizeof *out);
	return n;
}

int chip_pool_net_of(int xrom) { return xrom == XROM_BN5_COLONEL_US ? CHIP_NET_BN5 : xrom == XROM_BN4_RED_SUN_US ? CHIP_NET_BN4 : -1; }

bool chip_pool_net_read(int net) {
	return net == CHIP_NET_BN6 ? R.data != NULL : ((net == CHIP_NET_BN5 || net == CHIP_NET_OLDER) && chips_from_bn5()) ||
		((net == CHIP_NET_BN4 || net == CHIP_NET_OLDER) && chips_from_bn4());
}

/* The finds and the dealers' chips leaned to the run's Souls (loot_souls.h). */
#include "loot_souls.h"

#include "bn5.h"
#include "chip_pool.h"
#include "chips.h"
#include "data.h"
#include "economy.h"
#include "game.h"
#include "loot.h"
#include "rom.h"
#include "soulkinds.h"
#include "xrecords.h"

#define SOULS_TEAM 6   /* Team Colonel's: BN5's Souls 7-12 (souls.h) */

static unsigned kinds;   /* the held Souls' kinds, a bit each (BN5_CHIP_KIND) */
static int want;         /* the kind a pick takes */
static bool stocking;    /* ... and none the folder is full of (a dealer's) */

int loot_soul_kind_of(int soul) {
	return soul >= 0 && soul < SOULS_TEAM ? xrecords_soul_kind(XROM_BN5_COLONEL_US, BN5_NAVI_COLONEL + soul) : -1;
}

bool loot_soul_chip(int id, int soul) {
	int kind = loot_soul_kind_of(soul);
	return kind >= 0 && soulkinds_make() && soulkinds_of(id) == kind && !soulkinds_dark(id);
}

void loot_soul_souls(unsigned souls) {
	kinds = 0;
	if (!souls || !soulkinds_make()) return;
	for (int k = 0; k < SOULS_TEAM; ++k) {
		int kind = souls >> k & 1 ? loot_soul_kind_of(k) : -1;
		if (kind >= 0 && kind < SOULKIND_NONE) kinds |= 1u << kind;
	}
}

int loot_soul_kind(void) {
	if (!kinds) return -1;
	/* (the folder's chips of each kind, as the layer was made; a DarkChip
	 * lights a Chaos Unison, not the Soul) */
	int count[SOULKIND_NONE] = { 0 };
	for (int id = 1; id < CHIPS_IDS; ++id) {
		int k = loot_folder_copies(id) ? soulkinds_of(id) : SOULKIND_NONE;
		if (k < SOULKIND_NONE && !soulkinds_dark(id)) count[k] += loot_folder_copies(id);
	}
	int least = -1, tied = 0;
	for (int k = 0; k < SOULKIND_NONE; ++k) {
		if (!(kinds >> k & 1)) continue;
		if (least < 0 || count[k] < count[least]) { least = k; tied = 1; }
		else if (count[k] == count[least]) ++tied;
	}
	int pick = tied > 1 ? rng_range(0, tied - 1) : 0;
	for (int k = 0; k < SOULKIND_NONE; ++k)
		if (kinds >> k & 1 && count[k] == count[least] && pick-- == 0) return k;
	return least;
}

static bool of_kind(int id) { return soulkinds_of(id) == want && !soulkinds_dark(id) && !(stocking && loot_folder_full(id)); }

/* A chip of the kind of `tier` from the nets the layer's chips come from:
 * the older net's its share of the time (loot_pool_pick), the other where
 * one has none; -1 none */
static int kind_pick(int tier) {
	int first = loot_older_share > 0 && rng_range(0, 99) < loot_older_share ? CHIP_NET_BN5 : CHIP_NET_BN6;
	int id = chip_pool_pick_if(tier, first, of_kind);
	if (id < 0 && loot_older_share > 0) id = chip_pool_pick_if(tier, first == CHIP_NET_BN5 ? CHIP_NET_BN6 : CHIP_NET_BN5, of_kind);
	return id;
}

static char code_of(int id) {
	ChipInfo ci;
	chip_info(id, &ci);
	return ci.ncodes ? ci.codes[rng_range(0, ci.ncodes - 1)] : '*';
}

bool loot_soul_find(int *id, char *code) {
	if (!kinds || rng_range(0, 99) >= LOOT_SOUL_LEAN) return false;
	int tier = chip_pool_tier(*id);
	want = loot_soul_kind();
	stocking = false;
	int pick = tier >= 0 && want >= 0 ? kind_pick(tier) : -1;
	if (pick <= 0) return false;
	*id = pick;
	*code = code_of(pick);
	return true;
}

int loot_soul_stock(int tier, char *code) {
	want = loot_soul_kind();
	if (want < 0) return -1;
	stocking = true;
	int pick = -1;
	for (int t = tier < CHIP_TIERS ? tier : CHIP_TIERS - 1; t >= 0 && pick <= 0; --t) pick = kind_pick(t);
	for (int t = tier + 1; t <= ECON_GOOD_TIER && pick <= 0; ++t) pick = kind_pick(t);
	stocking = false;
	if (pick <= 0) return -1;
	*code = code_of(pick);
	return pick;
}

/* The trade screen looks the current map up in two tables, both keyed by
 * map group << 8 | number (docs/rom/): the prizes (a chip list and
 * weights per rarity) and the trader's kind (the script of the trader
 * archive whose lines it shows). A map in neither, as every generated
 * layer is, got no prize list at all (garbage names and pictures, and a
 * long roll over whatever memory it read) and lines picked by its map
 * number. Their second entries, Sky Town's (0x501), a map the run never
 * visits, become the layer's; the first stay AsterLand's own, whose Chip
 * Trader stands at home (docs/world/home/asterland-and-academy.md). */
#include "trader.h"

#include <stdio.h>
#include <string.h>

#include "debug.h"

#include "bn6.h"
#include "bytes.h"
#include "chip_pool.h"
#include "chips.h"
#include "emu.h"
#include "game.h"
#include "rom.h"
#include "loot.h"
#include "run.h"

/* The original pools (entries of BN6_TRADER_POOLS, by their map key and
 * the rarities of their chips): 0x104 70 chips of rarity 1-3, 0x501 130 of
 * 1-4, 0x000 108 of 1-3, 0x400 163 mostly 3-4, and 0x9301, Undernet 2's
 * BugFrag trader, 60 of 3-5. */
enum { POOL_COMMON = 0, POOL_RARE = 1, POOL_WIDE = 2, POOL_STRONG = 3, POOL_BUGFRAG = 4 };
#define LAYER_ENTRY 1   /* the entry of both tables a layer's trader takes */

static int pool_for(TraderKind kind, int depth) {
	if (kind == TRADER_BUGFRAG) return POOL_BUGFRAG;
	if (kind == TRADER_SPECIAL || depth > CYCLE_LAYERS) return POOL_STRONG;
	int p = (depth - 1) % CYCLE_LAYERS;
	return p < 4 ? POOL_COMMON : p < 9 ? POOL_WIDE : p < 14 ? POOL_RARE : POOL_STRONG;
}

/* The prizes of the pool whose entry is at ROM offset `from` with the
 * older net's catalogue beside them, half of each rarity's, at
 * FREE_TRADER_LIST; false where none of it was written. Every prize
 * keeps its record's codes: BN6's own pick (bn6f sub_804BF18, Gregar
 * 0x0804CCDC, its weights 0xC0 and 0x40 at 0x0804BE2C) gives a code the
 * pack holds none of three times in four, which spreads a run's codes
 * as BN6 spreads them (docs/game/economy/: the folder's codes, which the
 * traders gave until issue #154, made every run's finds alike) */
static bool both_nets(uint32_t from);

void trader_install(int group, int number, TraderKind kind, int depth) {
	uint32_t key = (uint32_t)(group << 8 | number);
	uint32_t pools = BN6_TRADER_POOLS & 0x1FFFFFF, from = pools + (uint32_t)pool_for(kind, depth) * 12;
	uint8_t pool[12];
	put32(pool, key);
	put32(pool + 4, rom_u32(from + 4));   /* chip list and weights, from the ROM file */
	put32(pool + 8, rom_u32(from + 8));
	/* (on the older net's areas and in the deep, half its prizes the older
	 * net's: docs/multirom/one-net.md) */
	if (loot_older_share > 0 && both_nets(from)) put32(pool + 4, FREE_TRADER_LIST);
	emu_write(BN6_TRADER_POOLS + 12u * LAYER_ENTRY, pool, sizeof pool);
	uint8_t kinds[8] = { 0 };
	put32(kinds, key);
	kinds[4] = (uint8_t)kind;
	emu_write(BN6_TRADER_KINDS + 8u * LAYER_ENTRY, kinds, sizeof kinds);
	/* a chip new to the Library: the game's three in four from those it
	 * has (a whole story's) left a run's few, and a trader gave BlastMan B
	 * three times running; the ones it has are still its fallback */
	static const uint8_t modes[2] = { 0, 64 };
	emu_write(BN6_TRADER_MODES, modes, sizeof modes);
}

void trader_home(void) {
	/* (AsterLand's own pool, its first entry, as BN6 has it, its codes
	 * BN6's pick's as a layer's are) */
	static const uint8_t modes[2] = { 0, 64 };
	emu_write(BN6_TRADER_MODES, modes, sizeof modes);
}

#define REC 6   /* a list's record: the chip (u16), four codes ending 0xFF */
#define LIST_MAX ((int)((FREE_TRADER_LIST_END - FREE_TRADER_LIST) / REC) - 1)

/* Chip `id`'s record in a list, its record's codes */
static void record_of(int id, const uint8_t codes[4], uint8_t rec[REC]) {
	put16(rec, (uint16_t)id);
	memcpy(rec + 2, codes, 4);
}

/* The pool at `from`'s list: its records (ROM offset, from the file) to
 * `*end` */
static uint32_t list_of(uint32_t from, uint32_t *end) {
	uint32_t list = rom_u32(from + 4);
	*end = 0;
	if (!rom_is_ptr(list)) return 0;
	uint32_t start = rom_off(list), e = start;
	while (e + REC <= ROM_SIZE && rom_u16(e)) e += REC;
	*end = e;
	return start;
}

/* The older net's catalogue of rarity `rarity` (chip_pool.h: the chips of
 * the registry that play, no DarkChip; a chip both nets have by name in
 * both halves, as each net has it), `want` of them at random, their
 * records into `out`; how many */
static int older_of(int rarity, int want, uint8_t *out) {
	static short ids[XREG_IDS];
	int n = 0;
	for (int i = 0; i < xreg_count; ++i) {
		int id = xreg_rows[i].bn6;
		const uint8_t *c = chips_record(id);
		if (xreg_rows[i].how == XREG_DARK || xreg_rows[i].how == XREG_PA || !c || c[5] != rarity || chip_pool_tier(id) < 0) continue;
		bool again = false;
		for (int k = 0; k < n && !again; ++k) again = ids[k] == id;
		if (!again) ids[n++] = (short)id;
	}
	int m = 0;
	for (; m < want && m < n; ++m) {
		int j = rng_range(m, n - 1);
		short t = ids[m]; ids[m] = ids[j]; ids[j] = t;
		const uint8_t *c = chips_record(ids[m]);
		record_of(ids[m], c, out + REC * m);
	}
	return m;
}

static bool both_nets(uint32_t from) {
	uint32_t end;
	uint32_t start = list_of(from, &end);
	if (!start || !chips_from_bn5()) return false;
	static uint8_t list[(LIST_MAX + 1) * REC];
	int n = 0, by[5] = { 0 };
	/* (BN6's pool's records first) */
	for (uint32_t r = start; r < end && n < LIST_MAX / 2; r += REC) {
		int id = rom_u16(r) & 0x1FF;
		record_of(id, R.data + r + 2, list + REC * n++);
		const uint8_t *rec = chips_record(id);
		if (rec && rec[5] < 5) ++by[rec[5]];
	}
	int six = n;
	for (int rarity = 0; rarity < 5; ++rarity) n += older_of(rarity, by[rarity], list + REC * n);
	put16(list + REC * n, 0);
	emu_write(FREE_TRADER_LIST, list, (size_t)(REC * n + 2));
	if (emu_debug_on()) fprintf(stderr, "trader: %d of BN6's chips and %d of the older net's\n", six, n - six);
	return true;
}


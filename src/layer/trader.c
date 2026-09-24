/* The trade screen looks the current map up in two tables, both keyed by
 * map group << 8 | number (docs/ROM_DATA.md): the prizes (a chip list and
 * weights per rarity) and the trader's kind (the script of the trader
 * archive whose lines it shows). A map in neither, as every generated
 * layer is, got no prize list at all (garbage names and pictures, and a
 * long roll over whatever memory it read) and lines picked by its map
 * number. Their first entries, for maps the run never visits, become the
 * layer's. */
#include "trader.h"

#include "bn6.h"
#include "bytes.h"
#include "emu.h"
#include "rom.h"
#include "run.h"

/* The original pools (entries of BN6_TRADER_POOLS, by their map key and
 * the rarities of their chips): 0x104 70 chips of rarity 1-3, 0x501 130 of
 * 1-4, 0x000 108 of 1-3, 0x400 163 mostly 3-4, and 0x9301, Undernet 2's
 * BugFrag trader, 60 of 3-5. */
enum { POOL_COMMON = 0, POOL_RARE = 1, POOL_WIDE = 2, POOL_STRONG = 3, POOL_BUGFRAG = 4 };

static int pool_for(TraderKind kind, int depth) {
	if (kind == TRADER_BUGFRAG) return POOL_BUGFRAG;
	if (kind == TRADER_SPECIAL || depth > CYCLE_LAYERS) return POOL_STRONG;
	int p = (depth - 1) % CYCLE_LAYERS;
	return p < 4 ? POOL_COMMON : p < 9 ? POOL_WIDE : p < 14 ? POOL_RARE : POOL_STRONG;
}

void trader_install(int group, int number, TraderKind kind, int depth) {
	uint32_t key = (uint32_t)(group << 8 | number);
	uint32_t pools = BN6_TRADER_POOLS & 0x1FFFFFF, from = pools + (uint32_t)pool_for(kind, depth) * 12;
	uint8_t pool[12];
	put32(pool, key);
	put32(pool + 4, rom_u32(from + 4));   /* chip list and weights, from the ROM file */
	put32(pool + 8, rom_u32(from + 8));
	emu_write(BN6_TRADER_POOLS, pool, sizeof pool);
	uint8_t kinds[8] = { 0 };
	put32(kinds, key);
	kinds[4] = (uint8_t)kind;
	emu_write(BN6_TRADER_KINDS, kinds, sizeof kinds);
}

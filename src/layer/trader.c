/* The trade screen looks the current map up in two tables, both keyed by
 * map group << 8 | number (docs/ROM_DATA.md): the prizes (a chip list and
 * weights per rarity) and the trader's kind (the script of the trader
 * archive whose lines it shows). A map in neither, as every generated
 * layer is, got no prize list at all (garbage names and pictures, and a
 * long roll over whatever memory it read) and lines picked by its map
 * number. Their second entries, Sky Town's (0x501), a map the run never
 * visits, become the layer's; the first stay AsterLand's own, whose Chip
 * Trader stands at home (docs/HOME.md, piece 10). */
#include "trader.h"

#include <stdio.h>
#include <string.h>

#include "debug.h"

#include "bn6.h"
#include "bytes.h"
#include "emu.h"
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

/* the prizes of the pool whose entry is at ROM offset `from` in the
 * folder's codes */
static void fit_codes(uint32_t from);

void trader_install(int group, int number, TraderKind kind, int depth) {
	uint32_t key = (uint32_t)(group << 8 | number);
	uint32_t pools = BN6_TRADER_POOLS & 0x1FFFFFF, from = pools + (uint32_t)pool_for(kind, depth) * 12;
	uint8_t pool[12];
	put32(pool, key);
	put32(pool + 4, rom_u32(from + 4));   /* chip list and weights, from the ROM file */
	put32(pool + 8, rom_u32(from + 8));
	emu_write(BN6_TRADER_POOLS + 12u * LAYER_ENTRY, pool, sizeof pool);
	fit_codes(from);
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
	/* (AsterLand's own pool, its first entry, as BN6 has it, its prizes in
	 * the folder's codes as a layer's are) */
	fit_codes((BN6_TRADER_POOLS & 0x1FFFFFF) + 12u * POOL_COMMON);
	static const uint8_t modes[2] = { 0, 64 };
	emu_write(BN6_TRADER_MODES, modes, sizeof modes);
}

static void fit_codes(uint32_t from) {
	/* its prizes in the folder's codes, or a * where the chip has one and
	 * none of them: a trade turns three chips the folder cannot play into
	 * one it can (BN6's own pick, sub_804BF18, takes a code the pack lacks
	 * three times in four; a Blade run traded three Recov10 for TrplShot V).
	 * And only the chips that come in them, where a dozen of the pool do (a
	 * playtester's two trades gave SumnBlk2 H and GunDelS2 E beside a
	 * folder of S and *: neither chip comes in them). The list's records
	 * (u16 chip, four codes ending 0xFF, a 0 chip ending the list) are
	 * rewritten in the core's copy from the file's own, the fitting ones
	 * first and the list ended after them, so each run's folder sets them. */
	uint32_t list = rom_u32(from + 4);
	if (rom_is_ptr(list)) {
		uint32_t start = rom_off(list), end = start;
		int fit = 0;
		for (; end + 6 <= ROM_SIZE && rom_u16(end); end += 6)
			if (loot_folder_code(rom_u16(end) & 0x1FF, true)) ++fit;
		bool only_fit = fit >= 12;
		uint32_t w = start;
		for (uint32_t r = start; r < end; r += 6) {
			char c = loot_folder_code(rom_u16(r) & 0x1FF, true);
			if (only_fit && !c) continue;
			uint8_t rec[6];
			memcpy(rec, R.data + r, sizeof rec);
			if (c) { rec[2] = (uint8_t)(c == '*' ? 26 : c - 'A'); rec[3] = rec[4] = rec[5] = 0xFF; }
			emu_write(0x08000000u + w, rec, sizeof rec);
			w += 6;
		}
		if (w < end) { static const uint8_t none[2] = { 0, 0 }; emu_write(0x08000000u + w, none, sizeof none); }
		if (emu_debug_on()) fprintf(stderr, "trader: %d of %d chips in the folder's codes%s\n", fit, (int)((end - start) / 6), only_fit ? ", only they" : "");
	}
}

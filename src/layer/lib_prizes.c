/* A dark layer's prizes and finds (layer_objs.h; docs/LIBERATION.md): the
 * par's prizes, BN5's tiers (two under par a chip a Server's win would
 * pay, at par one a tier lower, over par zenny: lib_par_zenny), the chips
 * and zenny its board's item panels hold and the chips its BonusPanels
 * give (a chip of the layer's own band each, the act's zenny, BN5's
 * amounts), each chip in the folder's code where it has one; each from
 * rolls of its own, the layer's seed's. */
#include "layer_objs.h"

#include <stdint.h>
#include <string.h>

#include "data.h"
#include "liberation_board.h"
#include "loot.h"
#include "game.h"
#include "run.h"
#include "scripts.h"
#include "text.h"

_Static_assert(LIB_LOOT == sizeof ((LayerObjs *)0)->lib_loot / sizeof (int), "a board's loot, a script each");
_Static_assert(LIB_BONUS_CHIPS == sizeof ((LayerObjs *)0)->lib_bonus / sizeof (int), "a board's BonusPanels' chips, a script each");

/* A chip rolled `depth` deep at `bonus` from rolls `salt`, its name and
 * code (0-25, 26 '*') */
static int roll(int depth, int bonus, uint32_t salt, ChipInfo *ci, int *code) {
	rng_seed(run.layer_seed ^ (salt + 1u) * 0x9E3779B9u);
	char c = '*';
	int chip = roll_chip(depth, bonus, &c);
	chip_info(chip, ci);
	char fit = loot_folder_code(chip, true);
	if (fit) c = fit;
	else if (strchr(ci->codes, '*')) c = '*';
	*code = c == '*' ? 26 : c - 'A';
	return chip;
}

/* (BN5's better chip and its lesser one: a Server's prize, a tier lower) */
int lib_prize_chip(int tier, int depth, int *code) {
	static const int tiers[2][2] = { { 2, 4 }, { 1, 2 } };
	ChipInfo ci;
	tier = tier ? 1 : 0;
	return roll(depth + tiers[tier][0], tiers[tier][1], 0x180u + (uint32_t)tier, &ci, code);
}

void lib_prizes_install(LayerObjs *out, TextArchive *text) {
	ChipInfo ci;
	int code;
	/* (given straight after the clear's tally, BN5's own words, as BN5's
	 * "You got a battle chip for:" and "You got:" follow its tally) */
	for (int k = 0; k < 2; ++k) {
		int chip = lib_prize_chip(k, run.depth, &code);
		chip_info(chip, &ci);
		out->lib_prize[k] = ta_lib_prize(text, chip, ci.name, code);
	}
	out->lib_prize[2] = ta_lib_prize_zenny(text, lib_par_zenny(run.depth));
	for (int k = 0; k < LIB_LOOT; ++k) {
		int chip = roll(run.depth, 1, 0x190u + (uint32_t)k, &ci, &code);
		out->lib_loot[k] = ta_lib_find_chip(text, chip, ci.name, code);
	}
	out->lib_zenny = ta_lib_find_zenny(text, lib_zenny((run.depth - 1) % CYCLE_LAYERS / 3));
	/* (the BonusPanels' four, the item panels' band: BN5's are a mission's
	 * standard chips, as its item panels' are) */
	for (int k = 0; k < LIB_BONUS_CHIPS; ++k) {
		int chip = roll(run.depth, 1, 0x1A0u + (uint32_t)k, &ci, &code);
		out->lib_bonus[k] = ta_lib_find_chip(text, chip, ci.name, code);
	}
}

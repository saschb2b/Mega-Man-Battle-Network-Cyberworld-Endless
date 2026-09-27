/* Layer shops (bn6f shop data; addresses in bn6.h and docs/ROM_DATA.md).
 *
 * Each shop has a 16-byte descriptor (currency, keeper's text, offset into
 * the shop data, entries); the shop data itself lives in EWRAM, copied from
 * the ROM at a new game. The shop screen lists only entries that match the
 * ROM's, so a layer writes its stock to both. The initial shops (up to the
 * Chip Order list) hold the programs and SubChips a layer can offer. */
#include "shop.h"

#include "bn6.h"
#include "emu.h"
#include "game.h"
#include "data.h"
#include "loot.h"
#include "navicust.h"
#include "pacing.h"
#include "rom.h"

#define ORDER_SHOP 18   /* the Chip Order list: one entry per chip */

static uint32_t desc(int shop) { return BN6_SHOP_DESCS + (uint32_t)shop * 16; }

/* An entry of the initial shops as the player's ROM has it (the core's
 * copy carries the layers' stock), so the picks and their prices never
 * depend on the layers built before. */
static void read_item(uint32_t a, ShopItem *it) {
	uint32_t o = a - 0x08000000u;
	it->kind = R.data[o];
	it->stock = R.data[o + 1];
	it->id = rom_u16(o + 2);
	it->code = R.data[o + 4];
	it->price = rom_u16(o + 6);
}

bool shop_install(int shop, const ShopItem *items, int n) {
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SHOP_DATA);
	if (data < 0x02000000u || data >= 0x02040000u) return false;
	uint32_t at = data + emu_read32(desc(shop) + 8);
	int slots = (int)emu_read32(desc(shop) + 12);
	for (int i = 0; i < slots; ++i, at += 8) {
		uint8_t e[8] = { 0 };
		if (i < n) {
			e[0] = items[i].kind; e[1] = items[i].stock;
			e[2] = (uint8_t)items[i].id; e[3] = (uint8_t)(items[i].id >> 8);
			e[4] = items[i].code;
			e[6] = (uint8_t)items[i].price; e[7] = (uint8_t)(items[i].price >> 8);
		}
		emu_write(at, e, 8);
		emu_write(BN6_SHOP_INIT + (at - data), e, 8);
	}
	return true;
}

/* A random entry of `kind` (items: id at least min_id) from the game's
 * initial shops. */
static bool pick(int kind, int min_id, ShopItem *out) {
	uint32_t end = BN6_SHOP_INIT + emu_read32(desc(ORDER_SHOP) + 8);
	int count = 0;
	ShopItem it;
	for (uint32_t a = BN6_SHOP_INIT; a < end; a += 8) {
		read_item(a, &it);
		if (it.kind == kind && it.id >= min_id) ++count;
	}
	if (!count) return false;
	int k = rng_range(0, count - 1);
	for (uint32_t a = BN6_SHOP_INIT; a < end; a += 8) {
		read_item(a, &it);
		if (it.kind == kind && it.id >= min_id && k-- == 0) {
			*out = it;
			return true;
		}
	}
	return false;
}

/* The game's own entry for item `id` of `kind` in its initial shops. */
static bool find_item(int kind, int id, ShopItem *out) {
	uint32_t end = BN6_SHOP_INIT + emu_read32(desc(ORDER_SHOP) + 8);
	for (uint32_t a = BN6_SHOP_INIT; a < end; a += 8) {
		read_item(a, out);
		if (out->kind == kind && out->id == id) return true;
	}
	return false;
}

static bool listed(const ShopItem *items, int n, const ShopItem *it) {
	for (int i = 0; i < n; ++i)
		if (items[i].kind == it->kind && items[i].id == it->id && items[i].code == it->code) return true;
	return false;
}

/* The first of a Net Dealer's list: of the element that answers the act,
 * the hardest hitting of the first eight found (the cheapest was IceSeed, 10
 * damage against a Fire guardian), but a hit of at most a third of the
 * act's guardian (AquaSwrd's 160 doubled took 320 of BlastMan's 400 in the
 * first hand: he never had a turn; with four found, half of act 1's Elec
 * answers were over it); a Standard chip, as he brings two (a folder takes
 * one DiveMan); rolled on until one is found, as he names the element.
 * `id` and `code` the roll it starts from, the answer returned. */
/* The most an answer may hit for: a third of the act's guardian (an
 * element's answer hits for double) */
static int answer_most(int depth, int counter) {
	int lo, hi;
	pacing_guardian_band(pacing_act(depth), &lo, &hi);
	return counter > 0 ? lo / 6 : lo / 3;
}

static int answer(int depth, int counter, int id, char *code) {
	int most = answer_most(depth, counter);
	int best = -1, best_power = 0, found = 0;
	char best_code = *code, c = *code;
	for (int tries = 0; tries < 400 && (best < 0 || (tries < 80 && found < 8)); ++tries) {
		ChipInfo ci;
		chip_info(id, &ci);
		/* (-1: a guardian of no element, answered by the hardest hit) */
		/* (and one that reaches: CircusMan kept to his back column,
		 * and two AquaSwrd never touched him) */
		if ((ci.element == counter || counter < 0) && ci.power > 0 && chip_direct(id) && !chip_sword(id) && chip_standard(id)) {
			++found;
			/* the hardest under the most, else the lightest over it */
			bool under = ci.power <= most, best_under = best >= 0 && best_power <= most;
			bool better = best < 0 || (under && !best_under) ||
				(under && best_under && (ci.power > best_power || (ci.power == best_power && chip_price(id) < chip_price(best)))) ||
				(!under && !best_under && ci.power < best_power);
			if (better) { best = id; best_power = ci.power; best_code = c; }
		}
		id = roll_chip(depth + 2, tries < 90 ? tries / 30 : 3, &c);
	}
	if (best < 0) return -1;
	*code = best_code;
	return best;
}

int shop_dealer_answer(int depth, int counter, char *code) {
	*code = '*';
	int id = roll_chip(depth + 2, 0, code);
	return answer(depth, counter, id, code);
}

int shop_dealer_stock(int depth, int counter, int viruses, ShopItem out[SHOP_MAX_ITEMS]) {
	int n = 0;
	for (int i = 0; i < 4; ++i) {
		char code = '*';
		ShopItem it = { 2, 1, 0, 0, 0 };
		it.id = (uint16_t)roll_chip(depth + 2, 0, &code);
		/* the first: the act's answer, two of it (one in 30 chips missed a
		 * whole guardian fight), one where it hits harder than the most (act
		 * 1's Elec answers: Thunder the only one under it, ElcPuls1 two
		 * times 200 of DiveMan's 500) */
		if (i == 0 && counter != 0) {
			int a = answer(depth, counter, it.id, &code);
			if (a >= 0) {
				ChipInfo ci;
				chip_info(a, &ci);
				it.id = (uint16_t)a;
				it.stock = ci.power > answer_most(depth, counter) ? 1 : 2;
			}
		}
		/* the second, where the first is the hardest hit for a guardian of
		 * no element: a chip of the element the act's viruses can't stand,
		 * which he names (a playtester heard "Fire" beside a list of none) */
		if (i == 1 && counter < 0 && viruses > 0) {
			int a = answer(depth, viruses, it.id, &code);
			if (a >= 0) it.id = (uint16_t)a;
		}
		it.code = (uint8_t)(code == '*' ? 26 : code - 'A');
		it.price = (uint16_t)(chip_price(it.id) / 100);
		/* (the answer at a price a run has by then: 400 zenny in act 1, 300
		 * more an act) */
		int cap = 4 + 3 * (pacing_act(depth) + 7 * pacing_loop(depth));
		if (i == 0 && counter != 0 && it.price > cap) it.price = (uint16_t)cap;
		if (!listed(out, n, &it)) out[n++] = it;
	}
	/* one HPMemory, dearer act by act (800 zenny in the first: about what
	 * a layer's battles and Mystery Data bring) */
	ShopItem hp = { 1, 1, 0x70, 0xFF, 0 };
	hp.price = (uint16_t)(8 + 4 * (pacing_act(depth) + 7 * pacing_loop(depth)));
	out[n++] = hp;
	/* SubChips: always a MiniEnrg (a heal to carry), and one of FullEnrg,
	 * SneakRun or Untrap (LocEnemy's 7000 zenny and an Unlocker, with no
	 * purple Mystery Data about, are no use in a run) */
	static const uint16_t subs[] = { SUB_FULL_ENERGY, SUB_SNEAK_RUN, SUB_UNTRAP };
	ShopItem mini;
	if (find_item(1, SUB_MINI_ENERGY, &mini)) out[n++] = mini;
	ShopItem other;
	if (find_item(1, subs[rng_range(0, (int)(sizeof subs / sizeof *subs) - 1)], &other) && !listed(out, n, &other)) out[n++] = other;
	return n;
}

bool shop_pick_program(ShopItem *out) { return pick(3, 0, out); }

/* The gift's programs: those that change how a first act plays at once
 * (the names' index is the program; a playtester offered MegFldr1 on
 * layer 1, room for a Mega chip beside a starting folder, took it only to
 * see the PET) */
static const struct { uint8_t program; const char *about; } gifts[] = {
	{ 1, "SUPERARMOR: NO FLINCHING WHEN YOU'RE HIT!" },
	{ 2, "CUSTOM1: ONE MORE CHIP IN THE CUSTOM SCREEN!" },
	{ 35, "ATTACK+1: A STRONGER BUSTER!" },
	{ 37, "CHARGE+1: A QUICKER CHARGE SHOT!" },
};

const char *shop_pick_gift_program(ShopItem *out) {
	int k = rng_range(0, (int)(sizeof gifts / sizeof *gifts) - 1);
	/* in one of its colours as the ROM's program records have them */
	int color = navicust_color(gifts[k].program);
	if (!color) { pick(3, 0, out); return NULL; }
	*out = (ShopItem){ 3, 1, (uint16_t)(gifts[k].program * 4), (uint8_t)color, 0 };
	return gifts[k].about;
}

#define PROGRAM_HP_200 0xAC   /* its id in the game's shops (4200 zenny there) */

int shop_program_stock(int depth, ShopItem out[SHOP_MAX_ITEMS]) {
	int n = 0;
	for (int i = 0; i < 10 && n < 4; ++i) {
		ShopItem it;
		/* (the NaviCust's pool and its tiers, docs/NAVICUST.md: HP+400 at
		 * 2300 zenny in act 2 beside the dealer's 20-HP HPMemory at 1200;
		 * SneakRun judged the game's battles, not the engine's) */
		if (!pick(3, 0, &it) || listed(out, n, &it) || !navicust_offerable(it.id / 4, depth)) continue;
		it.stock = 1;
		/* (a quarter of the game's price, which is its endgame's: a run
		 * brings 100 to 1000 zenny a battle or Mystery Data, and the
		 * programs sat at 2500 to 7100; 200 more an act) */
		it.price = (uint16_t)(it.price / 4 + 2 + 2 * (pacing_act(depth) + 7 * pacing_loop(depth)));
		/* (HP+200 at that price tripled a first act's 100 HP for 1200
		 * zenny, where an HPMemory's 20 cost 800) */
		if (it.id == PROGRAM_HP_200) it.price = (uint16_t)(24 + 4 * (pacing_act(depth) + 7 * pacing_loop(depth)));
		out[n++] = it;
	}
	return n;
}

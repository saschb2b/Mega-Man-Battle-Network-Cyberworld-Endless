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
#include "save.h"
#include "navicust.h"
#include "net.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"

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

bool shop_install(int shop, const ShopItem *items, int n, bool kept) {
	uint32_t data = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_SHOP_DATA);
	if (data < BN6_EWRAM || data >= BN6_EWRAM_END) return false;
	uint32_t at = data + emu_read32(desc(shop) + 8);
	int slots = (int)emu_read32(desc(shop) + 12);
	uint8_t was[16][8] = { { 0 } };
	for (int i = 0; kept && i < slots && i < 16; ++i)
		for (int b = 0; b < 8; ++b) was[i][b] = emu_read8(at + 8u * (uint32_t)i + (uint32_t)b);
	for (int i = 0; i < slots; ++i, at += 8) {
		uint8_t e[8] = { 0 };
		if (i < n) {
			e[0] = items[i].kind; e[1] = items[i].stock;
			e[2] = (uint8_t)items[i].id; e[3] = (uint8_t)(items[i].id >> 8);
			e[4] = items[i].code;
			e[6] = (uint8_t)items[i].price; e[7] = (uint8_t)(items[i].price >> 8);
		}
		/* (the ROM copy as a fresh list has it: the screen checks the list
		 * against its ids and codes, never its stock) */
		emu_write(BN6_SHOP_INIT + (at - data), e, 8);
		for (int k = 0; kept && i < n && k < 16; ++k)
			if (was[k][0] == e[0] && was[k][2] == e[2] && was[k][3] == e[3] && was[k][4] == e[4]) {
				e[1] = was[k][1];
				was[k][0] = 0;   /* (each once) */
				break;
			}
		emu_write(at, e, 8);
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
	/* (a chip once, whatever its code: ElcPuls3 A at 700 zenny, the pick,
	 * stood beside ElcPuls3 S at 2000 on a playtester's list) */
	for (int i = 0; i < n; ++i)
		if (items[i].kind == it->kind && items[i].id == it->id && (it->kind == 2 || items[i].code == it->code)) return true;
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

/* The Undernet's skull doors the act holds from layer `depth` on (issue
 * #47): one WWW-ID opens them all, as BN6's HeelNavi sells one in the
 * Undernet. */
static int skull_doors_ahead(int depth) {
	int doors = 0;
	for (int d = depth; d <= depth + 2 && (d == depth || layer_in_act(d) > 0); ++d) {
		int b = biome_for_depth(d);
		doors += (layer_pieces(d, b, LAYER_NORMAL) & PIECE_CUBE) && layer_cube_kind(d, b, LAYER_NORMAL) == BLOCK_SKULL;
	}
	return doors;
}

/* The keys for the set pieces the act holds from here on, at about a
 * layer's zenny each (BN6's 4000 for an Unlocker and 3000 for RushFood
 * would keep them out of a run's reach, keys priced past their use):
 *   an Unlocker for each purple data (issue #41);
 *   RushFood for each Rush gap, as many more as the longest needs held
 *   (Rush comes for as many as its panels, and eats one: issue #14);
 *   a WWW-ID where a skull door lies ahead (one opens them all, at about
 *   two layers' zenny: issue #47).
 * The stock's new count. */
static int add_keys(ShopItem *out, int n, int depth) {
	int act = pacing_act(depth) + 7 * pacing_loop(depth);
	ShopItem key;
	int locks = layer_pieces_ahead(depth, PIECE_PURPLE);
	if (locks && n < SHOP_MAX_ITEMS && find_item(1, SUB_UNLOCKER, &key)) {
		key.stock = (uint8_t)locks;
		key.price = (uint16_t)(6 + 3 * act);
		out[n++] = key;
	}
	int gaps = 0, longest = 0;
	for (int d = depth; d <= depth + 2 && (d == depth || layer_in_act(d) > 0); ++d) {
		int b = biome_for_depth(d);
		if (!(layer_pieces(d, b, LAYER_NORMAL) & PIECE_RUSH)) continue;
		++gaps;
		if (layer_rush_len(d, b) > longest) longest = layer_rush_len(d, b);
	}
	if (gaps && n < SHOP_MAX_ITEMS) {
		ShopItem food = { 1, (uint8_t)(longest + gaps - 1), ITEM_RUSH_FOOD, 0xFF, (uint16_t)(3 + act) };
		out[n++] = food;
	}
	if (skull_doors_ahead(depth) && n < SHOP_MAX_ITEMS) {
		ShopItem id = { 1, 1, ITEM_WWW_ID, 0xFF, (uint16_t)(10 + 3 * act) };
		out[n++] = id;
	}
	return n;
}

/* Whether chip `id` answers `counter` (-1: a guardian of no element,
 * answered by the hardest hit): a Standard chip of that element that
 * strikes outright, and reaches, not only beside MegaMan (CircusMan kept
 * to his back column, and two AquaSwrd never touched him; EraseMan holds
 * still at the back), but a sword where swords are the answer (TenguMan
 * hovers right in front between his dashes, and a Sword's 80 took 160 of
 * his HP); nor TankCan's line, which fires after a wind-up (CircusMan
 * hopped out of the row during it, and 200 went off on nothing). */
static bool answers(int id, const ChipInfo *ci, int counter) {
	if (counter >= 0 && chip_hits_with(id) != counter) return false;
	if (chip_melee(id) && counter != ELEM_SWORD) return false;
	return ci->power > 0 && chip_direct(id) && chip_standard(id) && chip_family(id) != CHIP_FAMILY_TANKCAN;
}

static int answer(int depth, int counter, int id, char *code) {
	int most = answer_most(depth, counter);
	int best = -1, best_power = 0, found = 0;
	bool best_fits = false;
	char best_code = *code, c = *code;
	for (int tries = 0; tries < 400 && (best < 0 || (tries < 80 && (found < 8 || (!best_fits && found < 16)))); ++tries) {
		ChipInfo ci;
		chip_info(id, &ci);
		if (answers(id, &ci, counter)) {
			/* (not one the folder holds as many of as it may: a playtester's
			 * pick, two ElcPuls1 S beside the three his folder had, was no
			 * use at all) */
			if (loot_folder_full(id) && tries < 390) { id = roll_chip(depth + 2, tries < 90 ? tries / 30 : 3, &c); continue; }
			++found;
			/* the hardest under the most, else the lightest over it; one
			 * that comes in the folder's codes or * counted a quarter
			 * harder (a playtester's pick, WideSht Q beside a folder of S
			 * and *, paired with nothing in two fights; preferred outright,
			 * act 1's picks fell from 120 to Cannon's 40: loot_folder_code) */
			bool under = ci.power <= most, best_under = best >= 0 && best_power <= most;
			bool fits = loot_folder_code(id, true) != 0;
			int eff = ci.power * (fits ? 5 : 4), best_eff = best_power * (best_fits ? 5 : 4);
			bool better = best < 0 || (under && !best_under) ||
				(under && best_under && (eff > best_eff || (eff == best_eff && chip_price(id) < chip_price(best)))) ||
				(!under && !best_under && (ci.power < best_power || (ci.power == best_power && fits && !best_fits)));
			if (better) { best = id; best_power = ci.power; best_code = c; best_fits = fits; }
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

/* The keys, then the SubChips: always a MiniEnrg (a heal to carry), and
 * one of FullEnrg, SneakRun or Untrap (LocEnemy's 7000 zenny and an
 * Unlocker, with no purple Mystery Data about, are no use in a run). The
 * keys first: the list holds eight, and an Unlocker took the last place,
 * a WWW-ID for the act's skull doors left off; a lock whose key is never
 * sold is dead weight. */
static int add_subs(ShopItem *out, int n, int depth) {
	static const uint16_t subs[] = { SUB_FULL_ENERGY, SUB_SNEAK_RUN, SUB_UNTRAP };
	int pick = rng_range(0, (int)(sizeof subs / sizeof *subs) - 1);
	n = add_keys(out, n, depth);
	ShopItem mini, other;
	if (n < SHOP_MAX_ITEMS && find_item(1, SUB_MINI_ENERGY, &mini)) out[n++] = mini;
	if (n < SHOP_MAX_ITEMS && find_item(1, subs[pick], &other) && !listed(out, n, &other)) out[n++] = other;
	return n;
}

int shop_dealer_stock(int depth, int counter, int viruses, ShopItem out[SHOP_MAX_ITEMS]) {
	int n = 0;
	for (int i = 0; i < 4; ++i) {
		char code = '*';
		ShopItem it = { 2, 1, 0, 0, 0 };
		it.id = (uint16_t)roll_chip(depth + 2, 0, &code);
		for (int t = 0; t < 8 && loot_folder_full(it.id); ++t) it.id = (uint16_t)roll_chip(depth + 2, 0, &code);
		/* the first: the act's answer, two of it (one in 30 chips missed a
		 * whole guardian fight), one where it hits harder than the most (act
		 * 1's Elec answers: Thunder the only one under it, ElcPuls1 two
		 * times 200 of DiveMan's 500) */
		if (i == 0 && counter != 0) {
			/* (none of the element found, which a few layers' rolls of
			 * Cursor chips came to: the hardest hit, and the dealer says so) */
			int c = counter, a = answer(depth, c, it.id, &code);
			if (a < 0 && c > 0) a = answer(depth, c = -1, it.id, &code);
			if (a >= 0) {
				ChipInfo ci;
				chip_info(a, &ci);
				it.id = (uint16_t)a;
				it.stock = ci.power > answer_most(depth, c) ? 1 : 2;
			}
		}
		/* the second, where the first is the hardest hit for a guardian of
		 * no element: a chip of the element the act's viruses can't stand,
		 * which he names (a playtester heard "Fire" beside a list of none) */
		if (i == 1 && counter < 0 && viruses > 0) {
			int a = answer(depth, viruses, it.id, &code);
			if (a >= 0) it.id = (uint16_t)a;
		}
		/* (in the folder's codes where the chip comes in them: a playtester's
		 * rewards, each in its own code, left him hands of five codes) */
		code = loot_fit_code(it.id, code, true);
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
	n = add_subs(out, n, depth);
	/* (threat 3, docs/META.md: half again for everything) */
	if (run.threat >= 3)
		for (int i = 0; i < n; ++i) out[i].price = (uint16_t)(out[i].price + out[i].price / 2);
	return n;
}

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

bool shop_program_found(int program) {
	return program > 0 && program < 64 && (profile.programs_found[program / 8] >> (program % 8) & 1);
}

/* Program `id`'s price at `depth`, from `base`, the game's shop price (0:
 * the first the game's shops ask for it): a quarter of it, which is its
 * endgame's (a run brings 100 to 1000 zenny a battle or Mystery Data, and
 * the programs sat at 2500 to 7100), 200 more an act; one the game's
 * shops don't sell, by its tier as they price theirs */
static uint16_t program_price(int id, int base, int depth) {
	int step = pacing_act(depth) + 7 * pacing_loop(depth);
	uint32_t end = BN6_SHOP_INIT + emu_read32(desc(ORDER_SHOP) + 8);
	ShopItem it;
	for (uint32_t a = BN6_SHOP_INIT; a < end && !base; a += 8) {
		read_item(a, &it);
		if (it.kind == 3 && it.id / 4 == id / 4) base = it.price;
	}
	if (!base) {
		static const uint8_t by_tier[8] = { 25, 25, 40, 40, 55, 55, 60, 70 };   /* hundreds */
		int t = navicust_tier(id / 4);
		base = by_tier[t < 0 ? 0 : t > 7 ? 7 : t];
	}
	/* (HP+200 at that price tripled a first act's 100 HP for 1200 zenny,
	 * where an HPMemory's 20 cost 800) */
	if (id == PROGRAM_HP_200) return (uint16_t)(24 + 4 * step);
	return (uint16_t)(base / 4 + 2 + 2 * step);
}

int shop_program_stock(int depth, ShopItem out[SHOP_MAX_ITEMS]) {
	int n = 0;
	/* first, two at most of the programs MegaMan has had in earlier runs
	 * that the act may offer (docs/NAVICUST.md, 7: the draft finds, the
	 * vendor keeps), but those he has now (run.programs, kept with the
	 * run as the layer is made) */
	int found[64], nfound = 0;
	for (int p = 1; p < 64; ++p) {
		bool has = false;
		for (int i = 0; i < (int)sizeof run.programs && run.programs[i]; ++i) has |= run.programs[i] / 4 == p;
		if (!has && shop_program_found(p) && navicust_offerable(p, depth)) found[nfound++] = p;
	}
	for (int i = nfound - 1; i > 0; --i) { int j = rng_range(0, i), t = found[i]; found[i] = found[j]; found[j] = t; }
	for (int i = 0; i < nfound && n < 2; ++i) {
		int color = navicust_color(found[i]);
		if (!color) continue;
		ShopItem it = { 3, 1, (uint16_t)(found[i] * 4), (uint8_t)color, 0 };
		it.price = program_price(it.id, 0, depth);
		out[n++] = it;
	}
	/* then those of the game's shops the act may offer, each once, in a
	 * random order, to four (ten random draws of their twelve had brought
	 * two or three in the first two acts, and none one time in sixty) */
	uint32_t end = BN6_SHOP_INIT + emu_read32(desc(ORDER_SHOP) + 8);
	ShopItem offer[32];
	int noffer = 0;
	for (uint32_t a = BN6_SHOP_INIT; a < end && noffer < (int)(sizeof offer / sizeof *offer); a += 8) {
		ShopItem it;
		read_item(a, &it);
		/* (the NaviCust's pool and its tiers, docs/NAVICUST.md: HP+400 at
		 * 2300 zenny in act 2 beside the dealer's 20-HP HPMemory at 1200;
		 * SneakRun judged the game's battles, not the engine's) */
		if (it.kind != 3 || !navicust_offerable(it.id / 4, depth)) continue;
		bool again = false;   /* (a program once, whatever its colour) */
		for (int k = 0; k < n; ++k) again |= out[k].id / 4 == it.id / 4;
		for (int k = 0; k < noffer; ++k) again |= offer[k].id / 4 == it.id / 4;
		if (!again) offer[noffer++] = it;
	}
	for (int i = noffer - 1; i > 0; --i) { int j = rng_range(0, i); ShopItem t = offer[i]; offer[i] = offer[j]; offer[j] = t; }
	for (int i = 0; i < noffer && n < 4; ++i) {
		offer[i].stock = 1;
		offer[i].price = program_price(offer[i].id, offer[i].price, depth);
		out[n++] = offer[i];
	}
	return n;
}

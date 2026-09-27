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
#include "loot.h"
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

static bool listed(const ShopItem *items, int n, const ShopItem *it) {
	for (int i = 0; i < n; ++i)
		if (items[i].kind == it->kind && items[i].id == it->id && items[i].code == it->code) return true;
	return false;
}

int shop_dealer_stock(int depth, ShopItem out[SHOP_MAX_ITEMS]) {
	int n = 0;
	for (int i = 0; i < 4; ++i) {
		char code = '*';
		ShopItem it = { 2, 1, 0, 0, 0 };
		it.id = (uint16_t)roll_chip(depth + 2, 0, &code);
		it.code = (uint8_t)(code == '*' ? 26 : code - 'A');
		it.price = (uint16_t)(chip_price(it.id) / 100);
		if (!listed(out, n, &it)) out[n++] = it;
	}
	/* one HPMemory, dearer act by act (1200 zenny in the first) */
	ShopItem hp = { 1, 1, 0x70, 0xFF, 0 };
	hp.price = (uint16_t)(12 + 6 * (pacing_act(depth) + 7 * pacing_loop(depth)));
	out[n++] = hp;
	for (int i = 0; i < 2; ++i) {
		ShopItem it;
		if (pick(1, 0x80, &it) && !listed(out, n, &it)) out[n++] = it;
	}
	return n;
}

bool shop_pick_program(ShopItem *out) { return pick(3, 0, out); }

/* The gift's programs: of the game's shop programs, those that help a
 * first act at once (ids per the shop data, the names' index is id / 4) */
static const struct { uint16_t id; const char *about; } gifts[] = {
	{ 0x04, "SUPERARMOR: NO FLINCHING WHEN YOU'RE HIT!" },
	{ 0x08, "CUSTOM1: ONE MORE CHIP IN THE CUSTOM SCREEN!" },
	{ 0x10, "MEGFLDR1: ROOM FOR ONE MORE MEGA CHIP!" },
	{ 0x8C, "ATTACK+1: A STRONGER BUSTER!" },
};

const char *shop_pick_gift_program(ShopItem *out) {
	int k = rng_range(0, (int)(sizeof gifts / sizeof *gifts) - 1);
	/* its color as the game's shop data has it */
	uint32_t end = BN6_SHOP_INIT + emu_read32(desc(ORDER_SHOP) + 8);
	for (uint32_t a = BN6_SHOP_INIT; a < end; a += 8) {
		ShopItem it;
		read_item(a, &it);
		if (it.kind == 3 && it.id == gifts[k].id) { *out = it; return gifts[k].about; }
	}
	pick(3, 0, out);
	return NULL;
}

int shop_program_stock(int depth, ShopItem out[SHOP_MAX_ITEMS]) {
	int n = 0;
	for (int i = 0; i < 6 && n < 4; ++i) {
		ShopItem it;
		if (!pick(3, 0, &it) || listed(out, n, &it)) continue;
		it.stock = 1;
		it.price = (uint16_t)(it.price + depth);
		out[n++] = it;
	}
	return n;
}

/* Layer shops on the game's own shop screen: a shop's stock is rewritten in
 * the game's shop data, and its keeper opens it with ts_start_shop. */
#ifndef CW_SHOP_H
#define CW_SHOP_H

#include <stdbool.h>
#include <stdint.h>

/* One stock entry in the game's format: kind 1 item (0x70 HPMemory, 0x80+
 * SubChips), 2 chip (code index, '*' is 26), 3 NaviCust program (code is
 * its color); stock 0xFF never runs out; price in hundreds of zenny. */
typedef struct {
	uint8_t kind, stock;
	uint16_t id;
	uint8_t code;
	uint16_t price;
} ShopItem;

#define SHOP_DEALER   0   /* shops the layers take over */
#define SHOP_PROGRAMS 3   /* (its keeper's face is the technician navi's) */
#define SHOP_MAX_ITEMS 8

/* SubChips (item ids, per the initial shops' prices: MiniEnrg 100 zenny,
 * FullEnrg 1000, SneakRun 500, Untrap 800, LocEnemy 7000, Unlocker 4000) */
#define SUB_MINI_ENERGY 0x80
#define SUB_FULL_ENERGY 0x81
#define SUB_SNEAK_RUN   0x82
#define SUB_UNTRAP      0x83

/* Writes the stock of shop `shop`; false before the game has set up its data. */
bool shop_install(int shop, const ShopItem *items, int n);

/* A layer's stock at `depth` (the rng decides the picks). */
int shop_dealer_stock(int depth, int counter, ShopItem out[SHOP_MAX_ITEMS]);
int shop_program_stock(int depth, ShopItem out[SHOP_MAX_ITEMS]);
/* One NaviCust program (id, color in `code`) from the game's shops. */
bool shop_pick_program(ShopItem *out);
/* One of the programs the start gift offers, and what it does (in Mr.
 * Prog's capitals; NULL when it had to take any program). */
const char *shop_pick_gift_program(ShopItem *out);

#endif

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
#define SHOP_PROGRAMS 3   /* (its keeper's face made the vendor's: shop_install) */
#define SHOP_MAX_ITEMS 8

/* SubChips (item ids, per the initial shops' prices: MiniEnrg 100 zenny,
 * FullEnrg 1000, SneakRun 500, Untrap 800, LocEnemy 7000, Unlocker 4000) */
#define SUB_MINI_ENERGY 0x80
#define SUB_FULL_ENERGY 0x81
#define SUB_SNEAK_RUN   0x82
#define SUB_UNTRAP      0x83
#define SUB_UNLOCKER    0x85   /* opens a purple Mystery Data (issue #41) */
#define ITEM_RUSH_FOOD  0x2C   /* calls Rush across a gap: as many held as its panels, one eaten (issue #14) */
#define ITEM_WWW_ID     0x44   /* opens the Undernet's skull doors, every one of a run (issue #47) */

/* Writes the stock of shop `shop`; false before the game has set up its
 * data. `kept`: over a saved state's list in RAM, an entry it holds keeps
 * its stock there (what was bought stays bought). */
bool shop_install(int shop, const ShopItem *items, int n, bool kept);

/* A layer's stock at `depth` (the rng decides the picks): first two of
 * the hardest hitting chip of element `counter` (ELEM_*), or of any element
 * for -1 (a guardian with none), 0 for no such chip; with -1, a chip of
 * element `viruses` (ELEM_*, the act's viruses' weakness) second. */
int shop_dealer_stock(int depth, int counter, int viruses, ShopItem out[SHOP_MAX_ITEMS]);
/* The chip a Net Dealer lists first at `depth` for the element `counter`
 * (-1: a guardian of none, the hardest hitter), as a layer's roll picks it
 * (the pacing report samples it); -1 for none found. */
int shop_dealer_answer(int depth, int counter, char *code);
int shop_program_stock(int depth, ShopItem out[SHOP_MAX_ITEMS]);
/* Whether MegaMan has had `program` in any run (profile.programs_found). */
bool shop_program_found(int program);
/* One of the programs the start gift offers, and what it does (in Mr.
 * Prog's capitals; NULL when it had to take any program). */
const char *shop_pick_gift_program(ShopItem *out);

#endif

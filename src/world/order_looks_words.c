/* order_looks_words.h. The clerk, as BN6's shopkeepers talk: short,
 * friendly, the price named (docs/voice/rules.md). */
#include "order_looks_words.h"

#include <stdio.h>

#include "looks.h"
#include "looks_routes.h"

const char *const order_looks_services[2] = { "Order a chip", "MegaMan's colors" };

const char *order_looks_ask(int p) {
	static char ask[64];
	snprintf(ask, sizeof ask, "%s colors,\n%d Zenny. OK?", looks_palette_name(p), LOOKS_SHOP_PRICE);
	return ask;
}

const char *const order_looks_broke = "Sorry,Lan. You're short on Zenny.";
const char *const order_looks_sold = "Those are yours now,Lan!";
const char *const order_looks_thanks = "Here you go! He'll look great!";

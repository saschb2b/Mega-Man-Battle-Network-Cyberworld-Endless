/* order_looks.h. The clerk's script, built from its ends up (a script
 * jumps only to those before it): goodbye, his words, the order, each
 * palette's sale, the colors' list, then his greeting and its choice. */
#include "order_looks.h"

#include "looks.h"
#include "looks_got_words.h"
#include "looks_routes.h"
#include "order_looks_words.h"
#include "script_kit.h"
#include "scripts.h"
#include "shop.h"

/* palette `p`'s sale: already sold this visit, else his question, Yes
 * taking the zenny (short: his word) and setting its flag */
static int sale(TextArchive *t, int face, int p, int bye, int broke, int sold) {
	int flag = LOOKS_SOLD_FLAG + p - LOOK_CENTRAL;
	int paid = ta_script(t);
	ta_flag_set(t, flag);
	bool first = true;
	ta_pages(t, order_looks_thanks, face, &first);
	ta_page(t, FACE_NONE, looks_got_box(p), false);
	ta_end(t);
	int i = ta_script(t);
	ta_if_flag(t, flag, sold);
	ta_ask(t, face, order_looks_ask(p), bye);
	ta_take_zenny(t, LOOKS_SHOP_PRICE, paid, broke);
	ta_end(t);
	return i;
}

int order_looks_script(TextArchive *t, int shop, int face, const char *greeting, const char *closed, const int *offer, int n) {
	if (n > LOOKS_SHOP_COUNT) n = LOOKS_SHOP_COUNT;
	int bye = ta_closing(t);
	int shut = ta_say(t, face, closed);
	int broke = ta_say(t, face, order_looks_broke);
	int sold = ta_say(t, face, order_looks_sold);
	/* the order: one a visit, as before */
	int order = ta_script(t);
	ta_if_flag(t, HOME_ORDER_FLAG, shut);
	ta_shop_open(t, shop);
	ta_end(t);
	int buy[LOOKS_SHOP_COUNT];
	for (int k = 0; k < n; ++k) buy[k] = sale(t, face, offer[k], bye, broke, sold);
	/* the colors he has, BN6's options (B: none) */
	const char *names[LOOKS_SHOP_COUNT];
	for (int k = 0; k < n; ++k) names[k] = looks_palette_name(offer[k]);
	int colours = ta_script(t);
	ta_mugshot(t, face);
	ta_clear(t);
	ta_options(t, names, n);
	ta_choose(t, buy, n, bye);
	/* his greeting, then the order or colors */
	int i = ta_script(t);
	bool first = true;
	ta_pages(t, greeting, face, &first);
	ta_mugshot(t, face);
	ta_clear(t);
	ta_options(t, order_looks_services, 2);
	int to[2] = { order, colours };
	ta_choose(t, to, 2, bye);
	return i;
}

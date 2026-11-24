/* AsterLand's clerk sells MegaMan's home palettes beside the Order Service
 * (docs/world/home/home-shop.md; docs/game/looks-earning.md, Earning), in BN6's own
 * chat box: its options, its Yes and No, its zenny taken. */
#ifndef CW_ORDER_LOOKS_H
#define CW_ORDER_LOOKS_H

#include "text.h"

/* Set as the clerk takes a palette's zenny, + its place among the home's
 * (looks_routes.h: Central Town, Seaside, Sky); the director grants it
 * (looks_due.c). BN6's jack-in clears them with its green Mystery Data's
 * flags: a visit's sale is granted before the next. */
#define LOOKS_SOLD_FLAG 0x14DA   /* 0x14DA-0x14DC */

/* The clerk's script where the profile lacks a home palette: his
 * greeting, then a choice of an order (BN6's Order Service, `shop`; one a
 * visit, `closed` after it) or colors, the `n` palettes of `offer`, each
 * for LOOKS_SHOP_PRICE zenny. Its script's number in `t`. */
int order_looks_script(TextArchive *t, int shop, int face, const char *greeting, const char *closed, const int *offer, int n);

#endif

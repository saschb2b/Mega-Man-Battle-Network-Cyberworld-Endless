/* AsterLand's Order Service sorted as BN6's folder editor and Chip Trader
 * sort (order_sort.c; issue #124, docs/world/home/home-shop.md). */
#ifndef CW_ORDER_SORT_H
#define CW_ORDER_SORT_H

#include <stdint.h>

/* The pack's reader, for the sort by No.: up to `most` of the pack's chips
 * (chip | code << 9) and their counts, how many. The director gives its
 * own as its hooks are installed (director_pack_now, events_install), so
 * the real world includes nothing of the director (docs/code/layers.md);
 * none given, MegaMan holds no copies */
typedef int OrderSortPack(uint16_t *entry, uint8_t *count, int most);
void order_sort_pack(OrderSortPack *pack);

/* The hook that has the clerk's box say the sort, once, with the game's
 * (game_hooks.c). */
void order_sort_install(void);
/* His words for it, where AsterLand's are written (aster_land_install). */
void order_sort_words(void);
/* Each frame in AsterLand: on his list START sorts it by the next key,
 * SELECT the other way; as it opens, it takes the session's sort again. */
void order_sort_frame(void);

#endif

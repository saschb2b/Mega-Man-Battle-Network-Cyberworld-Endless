/* AsterLand's Order Service sorted as BN6's folder editor and Chip Trader
 * sort (order_sort.c; issue #124, docs/HOME.md piece 6). */
#ifndef CW_ORDER_SORT_H
#define CW_ORDER_SORT_H

/* The hook that has the clerk's box say the sort, once, with the game's
 * (game_hooks.c). */
void order_sort_install(void);
/* His words for it, where AsterLand's are written (aster_land_install). */
void order_sort_words(void);
/* Each frame in AsterLand: on his list START sorts it by the next key,
 * SELECT the other way; as it opens, it takes the session's sort again. */
void order_sort_frame(void);

#endif

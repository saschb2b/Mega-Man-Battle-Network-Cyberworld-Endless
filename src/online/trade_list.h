/* The spares list's rows (trade_list.c, issue #145; docs/ONLINE.md, Trade
 * Central): what Trade Central lists as MegaMan's to trade, a chip in a
 * code with the copies past those his folders hold, filtered by a board
 * offer's want and sorted by one of BN6's keys. Numbers only: the game's
 * records give each row its keys (trade_central.c), the screen draws them
 * (trade_screen.c), and the tests make rows of their own. */
#ifndef CW_TRADE_LIST_H
#define CW_TRADE_LIST_H

#include <stdbool.h>
#include <stdint.h>

#include "online_trade.h"

#define TRADE_LIST_MOST 320   /* rows at most: the spares MegaMan can hold in BN6's ids and the pack's extension */
#define TRADE_COUNT_MOST 99   /* a row's count as shown */

/* BN6's keys for the list, its folder editor's first three: ID (its
 * record's IDSortPos), ABCDE (AlphabetSortPos) and Code (A to Z, then *) */
enum { TRADE_SORT_ID, TRADE_SORT_ABCDE, TRADE_SORT_CODE, TRADE_SORT_KEYS };

/* What a chip picked from the list is for: an offer to a Navi, one back
 * to his, an answer to a board offer, a post on the board */
enum { TRADE_FOR_OFFER, TRADE_FOR_ANSWER, TRADE_FOR_TAKE, TRADE_FOR_POST };

typedef struct {
	TradeChip chip;
	uint8_t count;       /* the spares: copies past the folders' */
	uint16_t id_order;   /* its record's IDSortPos */
	uint16_t abc_order;  /* ... and AlphabetSortPos */
} TradeRow;

/* The spares of a chip in one code: `have` in the Pack (the folders'
 * among them), `in_folders` in MegaMan's three folders; 0 none, at most
 * TRADE_COUNT_MOST */
int trade_list_spare(int have, int in_folders);

/* The rows of `all` (n) that meet `want` (-1: every row) by `meets`,
 * those with a spare alone, into `out` in their order: how many */
int trade_list_filter(const TradeRow *all, int n, int want, bool (*meets)(int want, TradeChip chip), TradeRow *out);

/* `rows` sorted by `key` (TRADE_SORT_*), up, ties by ID then code */
void trade_list_sort(TradeRow *rows, int n, int key);

/* The key after `key`, round to ID after Code */
int trade_list_next_key(int key);

#endif

/* Trade Central's two screens drawn (trade_screen_draw.c, issue #145): PET
 * screens of the project's own in the NetPass's colours and parts
 * (netpass_draw.h, tools/site_art.py), 240 x 160 over the game's picture.
 * A chip's card is read from the player's ROM as it is drawn (its picture
 * and palette, icon, element, name, code, power and description; BN5's
 * chips through the chip registry), never kept in a file. Drawing only: it
 * reads the view it is given and changes nothing. */
#ifndef CW_TRADE_SCREEN_DRAW_H
#define CW_TRADE_SCREEN_DRAW_H

#include "online_trade.h"
#include "trade_list.h"

#define TRADE_ROWS 6   /* the list's rows shown at once */

/* The chip trade's stages: asked, Deal! said and waiting, the swap, done */
enum { TRADE_ASK, TRADE_WAIT, TRADE_SWAP, TRADE_SWAPPED };
#define TRADE_SWAP_FRAMES 40

typedef struct {
	int t;                      /* frames since it opened */
	const char *partner;        /* the other player's name ("" a post) */
	/* the spares list */
	const TradeRow *rows;
	int n, at, top;             /* rows, the cursor's, the first shown */
	int sort, purpose, want;    /* TRADE_SORT_*, TRADE_FOR_*, the want (-1 none) */
	bool sorting;               /* START's SORT pop-up open, as the folder editor's */
	int sort_at;                /* ... its cursor, a TRADE_SORT_* */
	TradeChip theirs;           /* the chip offered to MegaMan (an answer), id 0 none */
	/* the chip trade */
	TradeChip mine, get;
	int cursor;                 /* 0 No deal, 1 Deal! */
	int stage, swap_t;          /* TRADE_ASK.., frames into the swap */
} TradeScreenView;

void trade_screen_draw_list(int x, int y, const TradeScreenView *v);
void trade_screen_draw_deal(int x, int y, const TradeScreenView *v);

#endif

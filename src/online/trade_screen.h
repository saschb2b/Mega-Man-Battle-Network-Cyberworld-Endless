/* Trade Central's two screens (issue #145; docs/online/trade-central.md, Trade
 * Central), PET screens of the project's own over BN6's picture, as the
 * NetPass is (netpass_screen.h): the game held under them, no key reaching
 * the core while one is up, its closing A or B kept from BN6 until let go.
 *
 * The spares list: a chip card (the picture, code, element, damage and
 * description read from the player's ROM, BN5's chips through the chip
 * registry) beside MegaMan's spares, sorted by ID, ABCDE or Code (START
 * steps through them; the session keeps its sort), the cursor's row lit.
 * Every chip picked for Trade Central is picked here: an offer to a Navi,
 * one back to his, an answer to a board offer (its want's spares alone),
 * a post.
 *
 * The chip trade: both chips' cards, You and Them, the arrows between.
 * No deal and Deal!, the cursor on No deal; B is No deal. After Deal! the
 * cards hold while the other player decides and the server commits; then
 * they swap places with BN6's jingle, and A or B closes it. Called off
 * meanwhile (B, or the other player, or the server), it closes.
 *
 * trade_central.c opens them and takes what was chosen (trade_screen_event);
 * online_chat.c runs their frames, online_scene.c draws them. */
#ifndef CW_TRADE_SCREEN_H
#define CW_TRADE_SCREEN_H

#include <stdbool.h>

#include "online_trade.h"
#include "trade_list.h"

/* What was chosen on a screen, once: a chip picked, the list left, Deal!
 * (the screen stays, waiting), No deal or called off (it closed) */
enum { TRADE_EVENT_NONE, TRADE_EVENT_PICKED, TRADE_EVENT_BACK, TRADE_EVENT_DEAL, TRADE_EVENT_DECLINE };

/* The spares list opened: `rows` (n, copied, sorted by the session's
 * sort), for TRADE_FOR_* `purpose`, with `partner` (NULL a post),
 * `want` the board offer's (-1 none), `theirs` the chip he offers (id 0
 * none) */
void trade_screen_list(const TradeRow *rows, int n, int purpose, const char *partner, int want, TradeChip theirs);
/* The chip trade opened: what MegaMan gives and gets, with `partner` */
void trade_screen_deal(TradeChip mine, TradeChip theirs, const char *partner);
/* The server's word on the trade shown: done (the cards swap), or off
 * (the screen closes) */
void trade_screen_swap(void);
void trade_screen_off(void);

/* What was chosen since the last call (TRADE_EVENT_*), the chip picked in `chip` */
int trade_screen_event(TradeChip *chip);
/* A screen up; the chip trade's */
bool trade_screen_shown(void);
bool trade_screen_dealing(void);

/* The frame's course while one is up (keys, the swap): true while it holds
 * the game, its keys to the core none */
bool trade_screen_update(void);
/* ... drawn over the game's picture (P.core_x, P.core_y) while it is up */
void trade_screen_draw(void);

#endif

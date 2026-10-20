/* Trade Central in the Net Square (trade_central.c): its clerk's chats,
 * the chip picked on the spares list and the pair on the chip trade's
 * screen (trade_screen.h), MegaMan's word as the trade goes. */
#ifndef CW_TRADE_CENTRAL_H
#define CW_TRADE_CENTRAL_H

#include "online_types.h"
#include "trade_list.h"

/* As MegaMan arrives in the Square. */
void trade_central_reset(void);
/* The last player this game traded with there, NULL none. */
const char *trade_central_last(void);
/* After each frame there, with the snapshot shown (online_chat.c's). */
void trade_central_frame(const OnlineSnapshot *s);
/* MegaMan's spares in the running game as the spares list's rows (at most
 * TRADE_LIST_MOST), their sort keys from their records: how many */
int trade_central_spares(TradeRow *rows);

#endif

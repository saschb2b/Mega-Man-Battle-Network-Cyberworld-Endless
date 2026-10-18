/* Trade Central in the Net Square (trade_central.c): its clerk's chats,
 * the chip picked on BN6's order screen, MegaMan's word as the trade
 * goes. */
#ifndef CW_TRADE_CENTRAL_H
#define CW_TRADE_CENTRAL_H

#include "online_types.h"

/* As MegaMan arrives in the Square. */
void trade_central_reset(void);
/* After each frame there, with the snapshot shown (online_chat.c's). */
void trade_central_frame(const OnlineSnapshot *s);

#endif

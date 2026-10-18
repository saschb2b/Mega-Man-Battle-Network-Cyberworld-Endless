/* The selected WebSocket protocol, never inferred from a server message. */
#ifndef CW_ONLINE_CLIENT_NEGOTIATION_H
#define CW_ONLINE_CLIENT_NEGOTIATION_H

#include <stdbool.h>

typedef struct { bool offer_trades, trades; } OnlineNegotiation;
const char *online_negotiation_offer(const OnlineNegotiation *selection);
bool online_negotiation_accept(OnlineNegotiation *selection, const char *protocol);

#endif

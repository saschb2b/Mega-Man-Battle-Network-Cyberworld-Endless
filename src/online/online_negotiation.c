#include "online_negotiation.h"

#include <string.h>

#include "negotiation.h"

const char *online_negotiation_offer(const OnlineNegotiation *selection) {
	return selection->offer_trades ? ONLINE_TRADE_PROTOCOLS : ONLINE_SUBPROTOCOL;
}

bool online_negotiation_accept(OnlineNegotiation *selection, const char *protocol) {
	selection->trades = false;
	if (!protocol) return false;
	if (!strcmp(protocol, ONLINE_SUBPROTOCOL)) return true;
	if (!selection->offer_trades || strcmp(protocol, ONLINE_TRADE_SUBPROTOCOL)) return false;
	selection->trades = true;
	return true;
}

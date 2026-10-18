/* Copied worker-to-frame events. Caller owns synchronization and connection. */
#ifndef CW_ONLINE_TRADE_INBOX_H
#define CW_ONLINE_TRADE_INBOX_H

#include "online_trade_wire.h"

#define ONLINE_TRADE_INBOX_SIZE 16

typedef struct {
	bool opened, enabled;
	unsigned read, write;
	OnlineTradeMessage events[ONLINE_TRADE_INBOX_SIZE];
} OnlineTradeInbox;

/* CAPS must be first and occur once. Overflow refuses the connection;
 * never overwrite an unread decision with a newer board snapshot. */
bool online_trade_inbox_push(OnlineTradeInbox *inbox, const uint8_t *data, size_t size);
bool online_trade_inbox_pop(OnlineTradeInbox *inbox, OnlineTradeMessage *out);
/* Bounded outgoing command kinds. Encode fields with online_trade_wire_*;
 * this admission check does not certify inventory, consent or persistence. */
bool online_trade_inbox_command(const OnlineTradeInbox *inbox, const uint8_t *data, size_t size);

#endif

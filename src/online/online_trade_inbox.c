#include "online_trade_inbox.h"

bool online_trade_inbox_push(OnlineTradeInbox *inbox, const uint8_t *data, size_t size) {
	OnlineTradeMessage event;
	if (inbox->write - inbox->read >= ONLINE_TRADE_INBOX_SIZE || !online_trade_wire_read(&event, data, size)) return false;
	bool caps = event.kind == ONLINE_TRADE_CAPS;
	if (caps == inbox->opened) return false;
	if (caps) { inbox->opened = true; inbox->enabled = event.value.caps.enabled; }
	inbox->events[inbox->write++ % ONLINE_TRADE_INBOX_SIZE] = event;
	return true;
}

bool online_trade_inbox_pop(OnlineTradeInbox *inbox, OnlineTradeMessage *out) {
	if (!out || inbox->read == inbox->write) return false;
	*out = inbox->events[inbox->read++ % ONLINE_TRADE_INBOX_SIZE];
	return true;
}

bool online_trade_inbox_command(const OnlineTradeInbox *inbox, const uint8_t *data, size_t size) {
	if (!inbox->opened || !data || !size) return false;
	switch (data[0]) {
	case ONLINE_TRADE_BOARD_REQUEST: return size == 1;
	case ONLINE_TRADE_CANCEL:
	case ONLINE_TRADE_STATUS:
	case ONLINE_TRADE_ACK: return size == 17;
	case ONLINE_TRADE_READY: return inbox->enabled && size == 54;
	case ONLINE_TRADE_OFFER: return inbox->enabled && size == 5;
	case ONLINE_TRADE_REQUEST: return inbox->enabled && size == 21;
	case ONLINE_TRADE_PREPARE: return inbox->enabled && size == 49;
	default: return false;
	}
}

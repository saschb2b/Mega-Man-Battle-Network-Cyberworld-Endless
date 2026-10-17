/* Bounded trade extension messages. Parsing never changes a checkpoint. */
#ifndef CW_ONLINE_TRADE_WIRE_H
#define CW_ONLINE_TRADE_WIRE_H

#include "online_trade_terms.h"

typedef struct {
	uint8_t run_id[16], checkpoint[32], rules;
	uint32_t revision;
} OnlineTradeReady;

typedef struct {
	OnlineTradeTerms terms;
	uint8_t state, prepared, applied;
} OnlineTradeDecision;

typedef struct {
	uint8_t identity[16], rules;
	char name[17];
	uint16_t have, want;
} OnlineTradeListing;

typedef struct {
	uint8_t kind;
	union {
		struct { uint8_t server[32]; bool enabled; } caps;
		struct { size_t count; OnlineTradeListing items[ONLINE_TRADE_LISTINGS_MAX]; } board;
		OnlineTradeDecision decision;
	} value;
} OnlineTradeMessage;

/* Failure leaves the caller's output untouched. State/authority checks belong
 * to the session and journal; this layer checks only the wire contract. */
bool online_trade_wire_read(OnlineTradeMessage *out, const uint8_t *bytes, size_t size);
bool online_trade_wire_ready(uint8_t out[54], const OnlineTradeReady *ready);
bool online_trade_wire_offer(uint8_t out[5], uint16_t have, uint16_t want);
bool online_trade_wire_request(uint8_t out[21], const uint8_t target[16], uint16_t give, uint16_t receive);
bool online_trade_wire_prepare(uint8_t out[49], const uint8_t transaction[16], const uint8_t digest[32]);
bool online_trade_wire_transaction(uint8_t out[17], uint8_t kind, const uint8_t transaction[16]);

#endif

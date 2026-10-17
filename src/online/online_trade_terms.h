#ifndef CW_ONLINE_TRADE_TERMS_H
#define CW_ONLINE_TRADE_TERMS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "trade.h"

typedef struct {
	uint8_t identity[16], run_id[16];
	uint32_t revision;
	uint8_t checkpoint[32];
	uint16_t chip;
	uint8_t rules;
} OnlineTradeSide;

typedef struct {
	uint8_t server[32], transaction[16];
	OnlineTradeSide side[2];
} OnlineTradeTerms;

bool online_trade_terms_valid(const OnlineTradeTerms *terms);
bool online_trade_terms_encode(const OnlineTradeTerms *terms, uint8_t out[ONLINE_TRADE_TERMS_BYTES]);
bool online_trade_terms_decode(OnlineTradeTerms *terms, const uint8_t *bytes, size_t n);
bool online_trade_terms_digest(const OnlineTradeTerms *terms, uint8_t out[32]);

#endif

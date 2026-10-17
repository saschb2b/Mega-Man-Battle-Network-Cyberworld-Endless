/* Portable run identity and settled receipts. A receipt describes the
 * checkpoint settled by that trade, not every later offline checkpoint. */
#ifndef CW_ONLINE_TRADE_RECORD_H
#define CW_ONLINE_TRADE_RECORD_H

#include "online_trade_terms.h"

#define ONLINE_TRADE_RECORD_INITIAL 60
#define ONLINE_TRADE_RECORD_LIMIT (4u << 20)

bool online_trade_record_new(const uint8_t run_id[16], uint8_t out[ONLINE_TRADE_RECORD_INITIAL]);
bool online_trade_record_read(const uint8_t *data, size_t n, uint8_t run_id[16], uint32_t *revision);
bool online_trade_record_has(const uint8_t *data, size_t n, const OnlineTradeTerms *terms);
/* A new malloc'd record; prior is unchanged. Only after durable server
 * COMMITTED may a checkpoint carrying this receipt become playable. */
uint8_t *online_trade_record_append(const uint8_t *prior, size_t n, const OnlineTradeTerms *terms,
	unsigned side, const uint8_t post_content[32], size_t *written);

#endif

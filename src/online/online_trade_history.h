/* Device-local high-water marks survive import, Undo and receipt loss.
 * This is not protection against an owner editing their offline files. */
#ifndef CW_ONLINE_TRADE_HISTORY_H
#define CW_ONLINE_TRADE_HISTORY_H

#include "online_trade_record.h"

typedef enum {
	TRADE_HISTORY_OK, TRADE_HISTORY_MISSING, TRADE_HISTORY_STALE,
	TRADE_HISTORY_BROKEN, TRADE_HISTORY_IO, TRADE_HISTORY_UNSUPPORTED
} OnlineTradeHistoryResult;

/* First installation only, with OS randomness supplied by the caller.
 * Existing history never changes namespace. This does not create a run. */
bool online_trade_history_create(const char *root, const uint8_t namespace_id[16]);
OnlineTradeHistoryResult online_trade_history_namespace(const char *root, uint8_t out[16]);
/* Read-only preflight. Unknown run IDs are permitted; known IDs must
 * retain their seed and extend the saved receipt prefix. */
OnlineTradeHistoryResult online_trade_history_check(const char *root, uint32_t seed, const uint8_t *record, size_t n);
/* Durable monotonic update; must succeed before releasing an APPLIED
 * journal or making a newly imported receipt-bearing checkpoint playable. */
OnlineTradeHistoryResult online_trade_history_remember(const char *root, uint32_t seed, const uint8_t *record, size_t n);
/* One-time legacy identity migration. The seed mapping is durable before
 * initial is returned. Known traded/missing receipts never get a new ID. */
OnlineTradeHistoryResult online_trade_history_legacy(const char *root, uint32_t seed,
	uint8_t initial[ONLINE_TRADE_RECORD_INITIAL]);

#endif

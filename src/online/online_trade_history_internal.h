/* Storage components already owning trade.lock use these. Never call
 * the public lock-owning history API while retaining that lock. */
#ifndef CW_ONLINE_TRADE_HISTORY_INTERNAL_H
#define CW_ONLINE_TRADE_HISTORY_INTERNAL_H

#include "online_trade_history.h"

OnlineTradeHistoryResult online_trade_history_namespace_locked(const char *root, uint8_t out[16]);
OnlineTradeHistoryResult online_trade_history_check_locked(const char *root, uint32_t seed, const uint8_t *record, size_t n);
OnlineTradeHistoryResult online_trade_history_remember_locked(const char *root, uint32_t seed, const uint8_t *record, size_t n);
OnlineTradeHistoryResult online_trade_history_legacy_locked(const char *root, uint32_t seed,
	uint8_t initial[ONLINE_TRADE_RECORD_INITIAL]);

#endif

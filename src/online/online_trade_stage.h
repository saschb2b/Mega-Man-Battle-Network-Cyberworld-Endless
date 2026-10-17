/* Owned checkpoint bytes; no emulator, filesystem or network mutation. */
#ifndef CW_ONLINE_TRADE_STAGE_H
#define CW_ONLINE_TRADE_STAGE_H

#include <stddef.h>
#include <stdint.h>

#include "online_trade_capture.h"
#include "online_trade_terms.h"

/* First proven scope: pristine Gregar, mGBA 0.10.5 raw state with SRAM/RTC,
 * ordinary native codes. Patched running-project states, BN5, Souls and
 * All * refuse until their separate original/project proof is complete.
 * The prepared inventory must still exactly match the input's loadout.
 * Returns malloc-owned bytes, or NULL with *written=0. Source is immutable.
 * The journal must independently bind run ID/revision and full checkpoint
 * digest to terms before this replacement may be promised or installed. */
uint8_t *online_trade_stage(const OnlineTradePrepared *prepared, const OnlineTradeTerms *terms, unsigned side,
	const uint8_t *source, size_t size, size_t *written);

#endif

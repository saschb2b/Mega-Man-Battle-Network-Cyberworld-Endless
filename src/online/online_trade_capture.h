/* An owned inventory prepared between frames; no live RAM pointers. */
#ifndef CW_ONLINE_TRADE_CAPTURE_H
#define CW_ONLINE_TRADE_CAPTURE_H

#include "online_trade_inventory.h"

typedef struct {
	OnlineLoadout loadout;
	OnlineTradeChip chips[ONLINE_PACK_IDS];
	uint16_t equipped[ONLINE_TRADE_EQUIPPED_MAX];
	uint8_t folder_count, active_folder;
	bool ready, checkpoint, bn5, all_star;
} OnlineTradePrepared;

/* Only at a settled map-side checkpoint, never in the folder editor
 * (which temporarily removes equipped copies from Pack). The caller owns
 * checkpoint/souls/helper truth. False clears out: stale offers cannot
 * survive a failed refresh. This neither writes RAM nor saves anything. */
bool online_trade_capture(OnlineTradePrepared *out, bool checkpoint, unsigned souls, bool all_star);

/* Borrow a const view of the prepared object, which must outlive the
 * view. Capturing another object or changing the emulator cannot affect
 * this one. A not-ready object produces an invalid inventory view. */
OnlineTradeInventory online_trade_inventory_view(const OnlineTradePrepared *prepared);

#endif

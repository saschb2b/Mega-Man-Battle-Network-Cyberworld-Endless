/* Local trade eligibility, over immutable checkpoint copies. No network,
 * emulator, save writes or chip conversion happens here. */
#ifndef CW_ONLINE_TRADE_INVENTORY_H
#define CW_ONLINE_TRADE_INVENTORY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "online_snapshot.h"

#define ONLINE_TRADE_CODES 4
#define ONLINE_TRADE_COUNT_MAX 99
#define ONLINE_TRADE_EQUIPPED_MAX (3 * BN6_FOLDER_ENTRIES)

/* The caller derives these numbers from the local chip registry and ROM
 * accessors, never from a peer. A canonical chip has canonical_id == its
 * index. available means a real, playable Pack chip; no pseudo-chip,
 * placeholder, unimplemented record or Program Advance. codes are the
 * unmodified record's four codes (0-26, or 255 for an empty slot). */
typedef struct {
	uint16_t canonical_id;
	uint8_t codes[ONLINE_TRADE_CODES];
	bool available, needs_bn5;
} OnlineTradeChip;

/* equipped is the complete list of all folder entries counted in pack,
 * including loadout->folder. A caller must verify that list before using
 * this API: OnlineLoadout alone only snapshots the first folder. Empty
 * entries may be 0 or 0xffff. The view and its backing memory stay local
 * and immutable through offer/confirmation. */
typedef struct {
	const OnlineLoadout *loadout;
	const OnlineTradeChip *chips; /* exactly ONLINE_PACK_IDS entries */
	const uint16_t *equipped;
	size_t equipped_count;
	bool checkpoint, bn5, all_star;
} OnlineTradeInventory;

typedef enum {
	ONLINE_TRADE_VALID,
	ONLINE_TRADE_INVALID_INVENTORY,
	ONLINE_TRADE_NO_CHECKPOINT,
	ONLINE_TRADE_HELPER_MISMATCH,
	ONLINE_TRADE_SAME_CHIP,
	ONLINE_TRADE_GIVE_UNAVAILABLE,
	ONLINE_TRADE_NO_SPARE,
	ONLINE_TRADE_RECEIVE_UNAVAILABLE,
	ONLINE_TRADE_PACK_FULL
} OnlineTradeCheck;

/* Pack quantities include equipped copies outside BN6's editor. This
 * subtracts every matching equipped copy; malformed or unavailable data
 * offers zero. A count is for this exact canonical chip/code pair. */
unsigned online_trade_spares(const OnlineTradeInventory *inventory, uint16_t entry);

/* One copy out, one copy in. All * runs trade with other All * runs only;
 * the received code is never changed. A valid result is eligibility, not
 * authorization to modify a save or evidence of a server commit. */
OnlineTradeCheck online_trade_check(const OnlineTradeInventory *inventory, uint16_t give, uint16_t receive, bool peer_all_star);

#endif

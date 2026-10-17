#include "online_trade_inventory.h"

#define TRADE_ID_MASK (ONLINE_PACK_IDS - 1u)
#define TRADE_CODE_STAR 26u

static bool inventory_valid(const OnlineTradeInventory *inventory) {
	return inventory && inventory->loadout && inventory->chips && inventory->equipped &&
		inventory->equipped_count >= BN6_FOLDER_ENTRIES && inventory->equipped_count <= ONLINE_TRADE_EQUIPPED_MAX;
}

/* An unsupported code never falls back to slot zero as BN6's own pack
 * accessor can: accepting that fallback would turn a forged offer into
 * a different chip code. All * explicitly owns slot zero. */
static int entry_slot(const OnlineTradeInventory *inventory, uint16_t entry) {
	unsigned id = entry & TRADE_ID_MASK, code = entry >> 9;
	const OnlineTradeChip *chip = &inventory->chips[id];
	if (!id || code > TRADE_CODE_STAR || chip->canonical_id != id || !chip->available ||
		(chip->needs_bn5 && !inventory->bn5)) return -1;
	if (inventory->all_star) return code == TRADE_CODE_STAR ? 0 : -1;
	int slot = -1;
	for (unsigned i = 0; i < ONLINE_TRADE_CODES; ++i) {
		if (chip->codes[i] != code) continue;
		if (slot >= 0) return -1; /* An ambiguous local record is not safe. */
		slot = (int)i;
	}
	return slot;
}

static unsigned spares_at(const OnlineTradeInventory *inventory, uint16_t entry, unsigned slot) {
	unsigned count = inventory->loadout->pack[entry & TRADE_ID_MASK][slot];
	if (count > ONLINE_TRADE_COUNT_MAX) return 0;
	unsigned held = 0;
	for (size_t i = 0; i < inventory->equipped_count; ++i) held += inventory->equipped[i] == entry;
	return count > held ? count - held : 0;
}

unsigned online_trade_spares(const OnlineTradeInventory *inventory, uint16_t entry) {
	if (!inventory_valid(inventory) || !inventory->checkpoint) return 0;
	int slot = entry_slot(inventory, entry);
	return slot < 0 ? 0 : spares_at(inventory, entry, (unsigned)slot);
}

OnlineTradeCheck online_trade_check(const OnlineTradeInventory *inventory, uint16_t give, uint16_t receive, bool peer_all_star) {
	if (!inventory_valid(inventory)) return ONLINE_TRADE_INVALID_INVENTORY;
	if (!inventory->checkpoint) return ONLINE_TRADE_NO_CHECKPOINT;
	if (inventory->all_star != peer_all_star) return ONLINE_TRADE_HELPER_MISMATCH;
	if (give == receive) return ONLINE_TRADE_SAME_CHIP;
	int give_slot = entry_slot(inventory, give), receive_slot = entry_slot(inventory, receive);
	if (give_slot < 0) return ONLINE_TRADE_GIVE_UNAVAILABLE;
	if (!spares_at(inventory, give, (unsigned)give_slot)) return ONLINE_TRADE_NO_SPARE;
	if (receive_slot < 0) return ONLINE_TRADE_RECEIVE_UNAVAILABLE;
	unsigned count = inventory->loadout->pack[receive & TRADE_ID_MASK][receive_slot];
	if (count >= ONLINE_TRADE_COUNT_MAX) return ONLINE_TRADE_PACK_FULL;
	return ONLINE_TRADE_VALID;
}

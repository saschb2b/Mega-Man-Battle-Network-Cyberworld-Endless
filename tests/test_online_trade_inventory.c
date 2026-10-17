/* Made-up chip metadata and checkpoints: no ROM or emulator needed. */
#include "test_online_trade_inventory.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "online_trade_inventory.h"

static OnlineLoadout loadout;
static OnlineTradeChip chips[ONLINE_PACK_IDS];
static uint16_t equipped[ONLINE_TRADE_EQUIPPED_MAX];

static uint16_t entry(unsigned id, unsigned code) { return (uint16_t)(id | code << 9); }

static OnlineTradeInventory fixture(void) {
	memset(&loadout, 0, sizeof loadout);
	memset(chips, 0, sizeof chips);
	memset(equipped, 0xff, sizeof equipped);
	chips[1] = (OnlineTradeChip){ .canonical_id = 1, .codes = { 0, 1, 2, 26 }, .available = true };
	chips[2] = (OnlineTradeChip){ .canonical_id = 2, .codes = { 3, 4, 255, 255 }, .available = true };
	chips[400] = (OnlineTradeChip){ .canonical_id = 400, .codes = { 5, 26, 255, 255 }, .available = true, .needs_bn5 = true };
	loadout.pack[1][0] = 5;
	equipped[0] = equipped[1] = equipped[BN6_FOLDER_ENTRIES] = entry(1, 0);
	return (OnlineTradeInventory){ .loadout = &loadout, .chips = chips, .equipped = equipped,
		.equipped_count = ONLINE_TRADE_EQUIPPED_MAX, .checkpoint = true };
}

static void spare_copies(void) {
	OnlineTradeInventory inventory = fixture();
	assert(online_trade_spares(&inventory, entry(1, 0)) == 2);
	assert(online_trade_check(&inventory, entry(1, 0), entry(2, 3), false) == ONLINE_TRADE_VALID);
	loadout.pack[1][0] = 3;
	assert(!online_trade_spares(&inventory, entry(1, 0)));
	assert(online_trade_check(&inventory, entry(1, 0), entry(2, 3), false) == ONLINE_TRADE_NO_SPARE);
	loadout.pack[1][0] = 1; /* Corrupt count smaller than equipped: no underflow. */
	assert(!online_trade_spares(&inventory, entry(1, 0)));
	loadout.pack[1][0] = 100;
	assert(!online_trade_spares(&inventory, entry(1, 0)));
	loadout.pack[1][1] = 1;
	assert(online_trade_spares(&inventory, entry(1, 1)) == 1); /* Same name, different code. */
}

static void recipient_rules(void) {
	OnlineTradeInventory inventory = fixture();
	assert(online_trade_check(&inventory, entry(1, 0), entry(400, 5), false) == ONLINE_TRADE_RECEIVE_UNAVAILABLE);
	inventory.bn5 = true;
	assert(online_trade_check(&inventory, entry(1, 0), entry(400, 5), false) == ONLINE_TRADE_VALID);
	chips[400].available = false; /* ROM present, but this record cannot play. */
	assert(online_trade_check(&inventory, entry(1, 0), entry(400, 5), false) == ONLINE_TRADE_RECEIVE_UNAVAILABLE);
	loadout.pack[2][0] = 99;
	assert(online_trade_check(&inventory, entry(1, 0), entry(2, 3), false) == ONLINE_TRADE_PACK_FULL);
	loadout.pack[2][0] = 98;
	assert(online_trade_check(&inventory, entry(1, 0), entry(2, 3), false) == ONLINE_TRADE_VALID);
	assert(online_trade_check(&inventory, entry(1, 0), entry(1, 1), false) == ONLINE_TRADE_VALID);
	assert(online_trade_check(&inventory, entry(1, 0), entry(1, 0), false) == ONLINE_TRADE_SAME_CHIP);
}

static void canonical_codes(void) {
	OnlineTradeInventory inventory = fixture();
	assert(online_trade_check(&inventory, entry(1, 0), entry(2, 26), false) == ONLINE_TRADE_RECEIVE_UNAVAILABLE);
	assert(online_trade_check(&inventory, entry(1, 27), entry(2, 3), false) == ONLINE_TRADE_GIVE_UNAVAILABLE);
	chips[1].canonical_id = 2; /* Alias IDs never become separate inventory. */
	assert(!online_trade_spares(&inventory, entry(1, 0)));
	chips[1].canonical_id = 1;
	chips[1].codes[1] = 0;
	assert(!online_trade_spares(&inventory, entry(1, 0)));
	assert(!online_trade_spares(&inventory, 0));
	assert(!online_trade_spares(&inventory, UINT16_MAX));
}

static void helpers(void) {
	OnlineTradeInventory inventory = fixture();
	assert(online_trade_check(&inventory, entry(1, 0), entry(2, 3), true) == ONLINE_TRADE_HELPER_MISMATCH);
	inventory.all_star = true;
	for (size_t i = 0; i < inventory.equipped_count; ++i)
		if (equipped[i] != UINT16_MAX) equipped[i] = entry(1, 26);
	assert(online_trade_spares(&inventory, entry(1, 26)) == 2);
	assert(!online_trade_spares(&inventory, entry(1, 0)));
	assert(online_trade_check(&inventory, entry(1, 26), entry(2, 26), true) == ONLINE_TRADE_VALID);
	assert(online_trade_check(&inventory, entry(1, 26), entry(2, 3), true) == ONLINE_TRADE_RECEIVE_UNAVAILABLE);
	assert(online_trade_check(&inventory, entry(1, 26), entry(2, 26), false) == ONLINE_TRADE_HELPER_MISMATCH);
}

static void invalid_views(void) {
	OnlineTradeInventory inventory = fixture();
	assert(online_trade_check(NULL, 1, 2, false) == ONLINE_TRADE_INVALID_INVENTORY);
	inventory.checkpoint = false;
	assert(online_trade_check(&inventory, 1, 2, false) == ONLINE_TRADE_NO_CHECKPOINT);
	assert(!online_trade_spares(&inventory, 1));
	inventory.checkpoint = true;
	inventory.equipped_count = ONLINE_TRADE_EQUIPPED_MAX + 1;
	assert(online_trade_check(&inventory, 1, 2, false) == ONLINE_TRADE_INVALID_INVENTORY);
	inventory.equipped_count = 0;
	assert(!online_trade_spares(&inventory, 1));
	inventory.equipped_count = BN6_FOLDER_ENTRIES;
	inventory.chips = NULL;
	assert(!online_trade_spares(&inventory, 1));
}

static void immutable_and_bounded(void) {
	OnlineTradeInventory inventory = fixture();
	OnlineLoadout before = loadout;
	OnlineTradeChip before_chips[ONLINE_PACK_IDS];
	uint16_t before_equipped[ONLINE_TRADE_EQUIPPED_MAX];
	memcpy(before_chips, chips, sizeof chips);
	memcpy(before_equipped, equipped, sizeof equipped);
	for (unsigned value = 0; value <= UINT16_MAX; ++value) {
		assert(online_trade_spares(&inventory, (uint16_t)value) <= ONLINE_TRADE_COUNT_MAX);
		(void)online_trade_check(&inventory, entry(1, 0), (uint16_t)value, false);
	}
	assert(!memcmp(&before, &loadout, sizeof loadout));
	assert(!memcmp(before_chips, chips, sizeof chips));
	assert(!memcmp(before_equipped, equipped, sizeof equipped));
}

void test_online_trade_inventory(void) {
	spare_copies(); recipient_rules(); canonical_codes(); helpers(); invalid_views(); immutable_and_bounded();
	puts("Trade inventory: equipped copies, recipient rules, codes, helpers and immutable bounds passed");
}

#ifdef CW_TRADE_INVENTORY_TEST_MAIN
int main(void) {
	test_online_trade_inventory();
	return 0;
}
#endif

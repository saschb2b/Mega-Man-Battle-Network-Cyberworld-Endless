/* Synthetic RAM/registry doubles check failure closure and owned copies.
 * Native folder accounting is proved separately by tools/online_trade_proof.py. */
#include "online_trade_capture.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "chip_pool.h"
#include "chips.h"
#include "emu.h"

#define TEST_MODE (BN6_EWRAM + 0x30000u)
#define TEST_CONTEXT (BN6_EWRAM + 0x30100u)
#define TEST_FOLDERS (BN6_EWRAM + 0x30200u)
#define TEST_PACK (BN6_EWRAM + 0x31000u)

static uint8_t ram[BN6_EWRAM_END - BN6_EWRAM];
static uint8_t records[ONLINE_PACK_IDS][BN6_CHIP_RECORD_SIZE];
static bool with_bn5;

uint8_t emu_read8(uint32_t address) {
	assert(address >= BN6_EWRAM && address < BN6_EWRAM_END);
	return ram[address - BN6_EWRAM];
}
uint16_t emu_read16(uint32_t address) { return (uint16_t)(emu_read8(address) | emu_read8(address + 1) << 8); }
uint32_t emu_read32(uint32_t address) { return (uint32_t)emu_read16(address) | (uint32_t)emu_read16(address + 2) << 16; }
static void put8(uint32_t address, uint8_t value) { ram[address - BN6_EWRAM] = value; }
static void put16(uint32_t address, uint16_t value) { put8(address, (uint8_t)value); put8(address + 1, (uint8_t)(value >> 8)); }
static void put32(uint32_t address, uint32_t value) { put16(address, (uint16_t)value); put16(address + 2, (uint16_t)(value >> 16)); }
const uint8_t *chips_record(int id) { return id > 0 && id < ONLINE_PACK_IDS ? records[id] : NULL; }
bool chips_bn5(int id) { return id == 400; }
bool chips_from_bn5(void) { return with_bn5; }
int chip_pool_tier(int id) { return id == 1 || id == 2 || (id == 400 && with_bn5) ? 0 : -1; }

static void fixture(void) {
	memset(ram, 0, sizeof ram);
	memset(records, 255, sizeof records);
	records[1][0] = 0; records[1][1] = 1; records[2][0] = 2; records[400][0] = 3;
	put32(BN6_TOOLKIT, TEST_MODE); put8(TEST_MODE, BN6_MODE_GAME);
	put8(BN6_GAMESTATE, BN6_MODE_GAME);
	put32(BN6_TOOLKIT + BN6_TOOLKIT_STEPS, TEST_CONTEXT);
	put8(TEST_CONTEXT + BN6_FOLDER_COUNT_FIELD, BN6_FOLDERS_MAX);
	put32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, TEST_FOLDERS);
	put32(BN6_TOOLKIT + BN6_TOOLKIT_PACK, TEST_PACK);
	for (unsigned i = 0; i < ONLINE_TRADE_EQUIPPED_MAX; ++i) put16(TEST_FOLDERS + 2 * i, 1);
	put8(TEST_PACK + BN6_PACK_ENTRY, 99);
	with_bn5 = false;
}

static void copy_and_helpers(void) {
	fixture();
	static OnlineTradePrepared prepared, before;
	static uint8_t before_ram[sizeof ram];
	memcpy(before_ram, ram, sizeof ram);
	assert(online_trade_capture(&prepared, true, 3, false));
	assert(!memcmp(before_ram, ram, sizeof ram));
	assert(prepared.folder_count == 3 && prepared.loadout.souls == 3);
	OnlineTradeInventory view = online_trade_inventory_view(&prepared);
	assert(online_trade_spares(&view, 1) == 9);
	assert(!prepared.chips[400].available && prepared.chips[400].needs_bn5);
	before = prepared;
	put8(TEST_PACK + BN6_PACK_ENTRY, 1);
	put16(TEST_FOLDERS, 2);
	records[1][0] = 7;
	assert(!memcmp(&before, &prepared, sizeof prepared));
	assert(online_trade_spares(&view, 1) == 9);
	fixture(); with_bn5 = true;
	for (unsigned i = 0; i < ONLINE_TRADE_EQUIPPED_MAX; ++i) put16(TEST_FOLDERS + 2 * i, (uint16_t)(1 | 26 << 9));
	assert(online_trade_capture(&prepared, true, 0, true));
	view = online_trade_inventory_view(&prepared);
	assert(prepared.bn5 && prepared.chips[400].available);
	assert(online_trade_spares(&view, (uint16_t)(1 | 26 << 9)) == 9);
	assert(!online_trade_spares(&view, 1));
}

static void rejects_bad_state(void) {
	static OnlineTradePrepared prepared, empty;
	fixture();
	assert(!online_trade_capture(NULL, true, 0, false));
	assert(!online_trade_capture(&prepared, false, 0, false));
	assert(!memcmp(&prepared, &empty, sizeof empty));
	for (unsigned count = 0; count <= 4; ++count) {
		fixture(); put8(TEST_CONTEXT + BN6_FOLDER_COUNT_FIELD, (uint8_t)count);
		assert(online_trade_capture(&prepared, true, 0, false) == (count >= 1 && count <= 3));
	}
	fixture(); put8(BN6_NAVI_FOLDER, 3);
	assert(!online_trade_capture(&prepared, true, 0, false));
	fixture(); put8(TEST_MODE, BN6_MODE_SUBMENU);
	assert(!online_trade_capture(&prepared, true, 0, false));
	fixture(); put8(BN6_GAMESTATE, 8); /* Battle submode. */
	assert(!online_trade_capture(&prepared, true, 0, false));
	fixture(); put16(TEST_FOLDERS, (uint16_t)(1 | 27 << 9));
	assert(!online_trade_capture(&prepared, true, 0, false));
	fixture(); put16(TEST_FOLDERS, (uint16_t)(1 | 3 << 9));
	assert(!online_trade_capture(&prepared, true, 0, false));
	fixture();
	assert(!online_trade_capture(&prepared, true, 0, true)); /* Unnormalised helper folder. */
	const uint32_t pointers[] = {0, BN6_EWRAM_END, TEST_FOLDERS + 1, BN6_EWRAM_END - 2 * BN6_FOLDER_ENTRIES};
	for (unsigned i = 0; i < sizeof pointers / sizeof *pointers; ++i) {
		fixture(); put32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS, pointers[i]);
		assert(!online_trade_capture(&prepared, true, 0, false));
		assert(!memcmp(&prepared, &empty, sizeof empty));
	}
	OnlineTradeInventory view = online_trade_inventory_view(&prepared);
	assert(online_trade_check(&view, 1, 2, false) == ONLINE_TRADE_INVALID_INVENTORY);
	assert(!online_trade_inventory_view(NULL).loadout);
}

int main(void) {
	copy_and_helpers(); rejects_bad_state();
	puts("Trade capture: immutable copies, helper metadata and invalid-state refusal passed");
	return 0;
}

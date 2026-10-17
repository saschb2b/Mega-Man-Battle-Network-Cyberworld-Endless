#include "online_trade_capture.h"

#include <string.h>

#include "chip_pool.h"
#include "chips.h"
#include "emu.h"

static bool ram_range(uint32_t at, unsigned bytes) {
	return at >= BN6_EWRAM && at <= BN6_EWRAM_END - bytes;
}

static bool folder_state(OnlineTradePrepared *out, uint32_t *folders) {
	uint32_t mode = emu_read32(BN6_TOOLKIT);
	uint32_t context = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_STEPS);
	if (!ram_range(mode, 1) || emu_read8(mode) != BN6_MODE_GAME ||
		emu_read8(BN6_GAMESTATE) != BN6_MODE_GAME || !ram_range(context, BN6_FOLDER_COUNT_FIELD + sizeof(uint8_t))) return false;
	out->folder_count = emu_read8(context + BN6_FOLDER_COUNT_FIELD);
	out->active_folder = emu_read8(BN6_NAVI_FOLDER);
	if (!out->folder_count || out->folder_count > BN6_FOLDERS_MAX || out->active_folder >= out->folder_count) return false;
	*folders = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	return !(*folders & 1u) && ram_range(*folders, out->folder_count * BN6_FOLDER_ENTRIES * 2);
}

static void copy_metadata(OnlineTradePrepared *out) {
	for (unsigned id = 0; id < ONLINE_PACK_IDS; ++id) {
		OnlineTradeChip *chip = &out->chips[id];
		const uint8_t *record = chips_record((int)id);
		chip->canonical_id = (uint16_t)id;
		memset(chip->codes, 255, sizeof chip->codes);
		if (record) memcpy(chip->codes, record + BN6_CHIP_CODES, sizeof chip->codes);
		chip->needs_bn5 = chips_bn5((int)id);
		chip->available = record && chip_pool_tier((int)id) >= 0;
	}
}

static bool copy_folders(OnlineTradePrepared *out, uint32_t folders) {
	unsigned count = out->folder_count * BN6_FOLDER_ENTRIES;
	for (unsigned i = 0; i < count; ++i) {
		uint16_t entry = emu_read16(folders + i * 2);
		out->equipped[i] = entry;
		if (!entry || entry == UINT16_MAX) continue;
		unsigned id = entry & (ONLINE_PACK_IDS - 1u), code = entry >> 9;
		if (!id || code > 26) return false;
		if (out->all_star) { if (code != 26) return false; continue; }
		unsigned matches = 0;
		for (unsigned slot = 0; slot < ONLINE_TRADE_CODES; ++slot) matches += out->chips[id].codes[slot] == code;
		if (matches != 1) return false; /* Never allow the native fallback to slot zero. */
	}
	return true;
}

bool online_trade_capture(OnlineTradePrepared *out, bool checkpoint, unsigned souls, bool all_star) {
	if (!out) return false;
	memset(out, 0, sizeof *out);
	uint32_t folders;
	if (!checkpoint || !folder_state(out, &folders) || !online_snapshot_read(&out->loadout, souls)) {
		memset(out, 0, sizeof *out);
		return false;
	}
	copy_metadata(out);
	out->all_star = all_star;
	if (!copy_folders(out, folders)) { memset(out, 0, sizeof *out); return false; }
	out->checkpoint = true;
	out->bn5 = chips_from_bn5();
	out->ready = true;
	return true;
}

OnlineTradeInventory online_trade_inventory_view(const OnlineTradePrepared *prepared) {
	if (!prepared || !prepared->ready) return (OnlineTradeInventory){0};
	return (OnlineTradeInventory){ .loadout = &prepared->loadout, .chips = prepared->chips,
		.equipped = prepared->equipped, .equipped_count = prepared->folder_count * BN6_FOLDER_ENTRIES,
		.checkpoint = prepared->checkpoint, .bn5 = prepared->bn5, .all_star = prepared->all_star };
}

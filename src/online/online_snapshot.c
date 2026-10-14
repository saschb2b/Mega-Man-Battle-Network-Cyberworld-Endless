#include "online_snapshot.h"

#include <string.h>

#include "emu.h"

static void read_bytes(uint8_t *out, uint32_t address, unsigned count) {
	for (unsigned i = 0; i < count; ++i) out[i] = emu_read8(address + i);
}

static bool snapshot_flag(unsigned flag) {
	return (emu_read8(BN6_EVENT_FLAGS + flag / 8) & (0x80u >> (flag & 7))) != 0;
}

bool online_snapshot_read(OnlineLoadout *out, unsigned souls) {
	uint32_t folder = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	uint32_t pack = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if (!out || (folder & 1u) || folder < BN6_EWRAM || folder > BN6_EWRAM_END - 2 * BN6_FOLDER_ENTRIES ||
		pack < BN6_EWRAM || pack > BN6_EWRAM_END - BN6_PACK_IDS * BN6_PACK_ENTRY) return false;
	memset(out, 0, sizeof *out);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) out->folder[i] = emu_read16(folder + 2 * i);
	for (unsigned i = 0; i < ONLINE_PACK_IDS; ++i) {
		uint32_t at = i < BN6_PACK_IDS ? pack + i * BN6_PACK_ENTRY : BN6_PACK_EXT + (i - BN6_PACK_IDS) * BN6_PACK_ENTRY;
		read_bytes(out->pack[i], at, BN6_PACK_ENTRY);
	}
	read_bytes(out->board, BN6_NAVICUST_PLACED, ONLINE_BOARD_BYTES);
	read_bytes(out->grid, BN6_NAVICUST_GRID, ONLINE_BOARD_CELLS);
	out->hp = emu_read16(BN6_NAVI_HP);
	out->max_hp = emu_read16(BN6_NAVI_MAX_HP);
	out->base_hp = emu_read16(BN6_NAVI_BASE_MAX_HP);
	out->attack = emu_read8(BN6_NAVI_ATTACK);
	out->speed = emu_read8(BN6_NAVI_SPEED);
	out->charge = emu_read8(BN6_NAVI_CHARGE);
	out->regular_mb = emu_read8(BN6_NAVI_REG);
	out->mega_limit = emu_read8(BN6_NAVI_MEGA_LEVEL);
	out->giga_limit = emu_read8(BN6_NAVI_GIGA_LEVEL);
	out->regular = emu_read8(BN6_NAVI_FOLDER1_REG);
	out->tag[0] = emu_read8(BN6_NAVI_FOLDER1_TAG);
	out->tag[1] = emu_read8(BN6_NAVI_FOLDER1_TAG2);
	for (unsigned i = 0; i < 5; ++i)
		if (snapshot_flag(BN6_FLAG_HEAT_CROSS + i)) out->crosses |= (uint8_t)(1u << i);
	out->souls = (uint8_t)souls;
	out->beast = snapshot_flag(BN6_FLAG_BEAST_OUT);
	return true;
}

#include "online_snapshot.h"

#include <string.h>

#include "emu.h"

static uint8_t main_read8(void *context, uint32_t address) {
	(void)context;
	return emu_read8(address);
}

static void read_bytes(const OnlineMemory *m, uint8_t *out, uint32_t address, unsigned count) {
	for (unsigned i = 0; i < count; ++i) out[i] = m->read8(m->context, address + i);
}

static uint16_t read16(const OnlineMemory *m, uint32_t address) {
	return (uint16_t)(m->read8(m->context, address) | m->read8(m->context, address + 1) << 8);
}

static uint32_t read32(const OnlineMemory *m, uint32_t address) {
	return read16(m, address) | (uint32_t)read16(m, address + 2) << 16;
}

static bool snapshot_flag(const OnlineMemory *m, unsigned flag) {
	return (m->read8(m->context, BN6_EVENT_FLAGS + flag / 8) & (0x80u >> (flag & 7))) != 0;
}

bool online_snapshot_read(OnlineLoadout *out, unsigned souls) {
	const OnlineMemory m = { main_read8, NULL };
	return online_snapshot_read_from(&m, out, souls);
}

bool online_snapshot_read_from(const OnlineMemory *m, OnlineLoadout *out, unsigned souls) {
	uint32_t folder = read32(m, BN6_TOOLKIT + BN6_TOOLKIT_CHIPS);
	uint32_t pack = read32(m, BN6_TOOLKIT + BN6_TOOLKIT_PACK);
	if (!out || (folder & 1u) || folder < BN6_EWRAM || folder > BN6_EWRAM_END - 2 * BN6_FOLDER_ENTRIES ||
		pack < BN6_EWRAM || pack > BN6_EWRAM_END - BN6_PACK_IDS * BN6_PACK_ENTRY) return false;
	memset(out, 0, sizeof *out);
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i) out->folder[i] = read16(m, folder + 2 * i);
	for (unsigned i = 0; i < ONLINE_PACK_IDS; ++i) {
		uint32_t at = i < BN6_PACK_IDS ? pack + i * BN6_PACK_ENTRY : BN6_PACK_EXT + (i - BN6_PACK_IDS) * BN6_PACK_ENTRY;
		read_bytes(m, out->pack[i], at, BN6_PACK_ENTRY);
	}
	read_bytes(m, out->board, BN6_NAVICUST_PLACED, ONLINE_BOARD_BYTES);
	read_bytes(m, out->grid, BN6_NAVICUST_GRID, ONLINE_BOARD_CELLS);
	out->hp = read16(m, BN6_NAVI_HP);
	out->max_hp = read16(m, BN6_NAVI_MAX_HP);
	out->base_hp = read16(m, BN6_NAVI_BASE_MAX_HP);
	out->attack = m->read8(m->context, BN6_NAVI_ATTACK);
	out->speed = m->read8(m->context, BN6_NAVI_SPEED);
	out->charge = m->read8(m->context, BN6_NAVI_CHARGE);
	out->regular_mb = m->read8(m->context, BN6_NAVI_REG);
	out->mega_limit = m->read8(m->context, BN6_NAVI_MEGA_LEVEL);
	out->giga_limit = m->read8(m->context, BN6_NAVI_GIGA_LEVEL);
	out->regular = m->read8(m->context, BN6_NAVI_FOLDER1_REG);
	out->tag[0] = m->read8(m->context, BN6_NAVI_FOLDER1_TAG);
	out->tag[1] = m->read8(m->context, BN6_NAVI_FOLDER1_TAG2);
	for (unsigned i = 0; i < 5; ++i)
		if (snapshot_flag(m, BN6_FLAG_HEAT_CROSS + i)) out->crosses |= (uint8_t)(1u << i);
	out->souls = (uint8_t)souls;
	out->beast = snapshot_flag(m, BN6_FLAG_BEAST_OUT);
	return true;
}

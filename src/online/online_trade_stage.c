#include "online_trade_stage.h"

#include <stdlib.h>
#include <string.h>

#include <mgba/internal/gba/serialize.h>

#include "bytes.h"

#define STAGE_CORE_BYTES 397312u
#define STAGE_FILE_BYTES 430144u
#define STAGE_MAGIC 0x01000007u
#define STAGE_ROM_CRC 0x79452182u
#define STAGE_ROM_ID 0x45355242u
#define STAGE_EXT_RECORD 16u
#define STAGE_SRAM_BYTES 32768u
#define STAGE_RTC_BYTES 16u

_Static_assert(sizeof(struct GBASerializedState) == STAGE_CORE_BYTES, "trade state layout needs a new proof");
_Static_assert(offsetof(struct GBASerializedState, wram) == 135168, "trade RAM location needs a new proof");

typedef struct {
	const uint8_t *bytes;
	uint32_t pack, folders, marks;
} State;

static const uint8_t *ram(const State *state, uint32_t address, size_t size) {
	if (size > BN6_EWRAM_END - BN6_EWRAM || address < BN6_EWRAM || address > BN6_EWRAM_END - size) return NULL;
	return state->bytes + offsetof(struct GBASerializedState, wram) + address - BN6_EWRAM;
}

static uint32_t pointer(const State *state, unsigned field) {
	const uint8_t *bytes = ram(state, BN6_TOOLKIT + field, sizeof(uint32_t));
	return bytes ? get32(bytes) : 0;
}

static bool same_bytes(const State *state, uint32_t address, const void *expected, size_t size) {
	const uint8_t *bytes = ram(state, address, size);
	return bytes && !memcmp(bytes, expected, size);
}

static bool envelope(const uint8_t *source, size_t size) {
	if (!source || size != STAGE_FILE_BYTES || get32(source) != STAGE_MAGIC ||
		get32(source + offsetof(struct GBASerializedState, romCrc32)) != STAGE_ROM_CRC ||
		get32(source + offsetof(struct GBASerializedState, id)) != STAGE_ROM_ID) return false;
	/* Pinned raw mGBA envelope: SRAM and RTC records, a terminator, then
	 * their data. Preserve all of it; reject other layouts before copying. */
	const uint32_t expected[] = {
		2, STAGE_SRAM_BYTES, STAGE_CORE_BYTES + 3 * STAGE_EXT_RECORD, 0,
		4, STAGE_RTC_BYTES, STAGE_CORE_BYTES + 3 * STAGE_EXT_RECORD + STAGE_SRAM_BYTES, 0,
		0, 0, 0, 0
	};
	for (size_t i = 0; i < sizeof expected / sizeof *expected; ++i)
		if (get32(source + STAGE_CORE_BYTES + i * sizeof(uint32_t)) != expected[i]) return false;
	return true;
}

static bool folders_match(const State *state, const OnlineTradePrepared *prepared) {
	uint32_t main = pointer(state, 0), context = pointer(state, BN6_TOOLKIT_STEPS);
	const uint8_t *mode = ram(state, main, 1), *count = ram(state, context, BN6_FOLDER_COUNT_FIELD + sizeof(uint8_t));
	if (!mode || *mode != BN6_MODE_GAME || *ram(state, BN6_GAMESTATE, 1) != BN6_MODE_GAME || !count ||
		!prepared->folder_count || prepared->folder_count > BN6_FOLDERS_MAX ||
		count[BN6_FOLDER_COUNT_FIELD] != prepared->folder_count ||
		*ram(state, BN6_NAVI_FOLDER, 1) != prepared->active_folder || prepared->active_folder >= prepared->folder_count ||
		(state->folders & 1u)) return false;
	unsigned entries = prepared->folder_count * BN6_FOLDER_ENTRIES;
	const uint8_t *folder = ram(state, state->folders, entries * sizeof(uint16_t));
	if (!folder) return false;
	for (unsigned i = 0; i < entries; ++i)
		if (get16(folder + i * 2) != prepared->equipped[i]) return false;
	for (unsigned i = 0; i < BN6_FOLDER_ENTRIES; ++i)
		if (get16(folder + i * 2) != prepared->loadout.folder[i]) return false;
	return true;
}

static bool pack_matches(const State *state, const OnlineTradePrepared *prepared) {
	if (!ram(state, state->pack, BN6_PACK_IDS * BN6_PACK_ENTRY) || !ram(state, state->marks, ONLINE_PACK_IDS)) return false;
	for (unsigned id = 0; id < ONLINE_PACK_IDS; ++id) {
		uint32_t at = id < BN6_PACK_IDS ? state->pack + id * BN6_PACK_ENTRY : BN6_PACK_EXT + (id - BN6_PACK_IDS) * BN6_PACK_ENTRY;
		if (!same_bytes(state, at, prepared->loadout.pack[id], BN6_PACK_ENTRY)) return false;
	}
	return true;
}

static bool stats_match(const State *state, const OnlineLoadout *loadout) {
	static const struct { size_t member; uint32_t address; size_t size; } fields[] = {
#define FIELD(member, address) { offsetof(OnlineLoadout, member), address, sizeof(((OnlineLoadout *)0)->member) }
		FIELD(board, BN6_NAVICUST_PLACED), FIELD(grid, BN6_NAVICUST_GRID),
		FIELD(attack, BN6_NAVI_ATTACK), FIELD(speed, BN6_NAVI_SPEED), FIELD(charge, BN6_NAVI_CHARGE),
		FIELD(regular_mb, BN6_NAVI_REG), FIELD(mega_limit, BN6_NAVI_MEGA_LEVEL), FIELD(giga_limit, BN6_NAVI_GIGA_LEVEL),
		FIELD(regular, BN6_NAVI_FOLDER1_REG)
#undef FIELD
	};
	for (size_t i = 0; i < sizeof fields / sizeof *fields; ++i)
		if (!same_bytes(state, fields[i].address, (const uint8_t *)loadout + fields[i].member, fields[i].size)) return false;
	if (get16(ram(state, BN6_NAVI_HP, 2)) != loadout->hp || get16(ram(state, BN6_NAVI_MAX_HP, 2)) != loadout->max_hp ||
		get16(ram(state, BN6_NAVI_BASE_MAX_HP, 2)) != loadout->base_hp ||
		*ram(state, BN6_NAVI_FOLDER1_TAG, 1) != loadout->tag[0] || *ram(state, BN6_NAVI_FOLDER1_TAG2, 1) != loadout->tag[1]) return false;
	unsigned crosses = 0;
	for (unsigned i = 0; i < 5; ++i) {
		unsigned flag = BN6_FLAG_HEAT_CROSS + i;
		if (*ram(state, BN6_EVENT_FLAGS + flag / 8, 1) & (0x80u >> (flag & 7))) crosses |= 1u << i;
	}
	bool beast = (*ram(state, BN6_EVENT_FLAGS + BN6_FLAG_BEAST_OUT / 8, 1) & (0x80u >> (BN6_FLAG_BEAST_OUT & 7))) != 0;
	return crosses == loadout->crosses && beast == loadout->beast;
}

static bool prepare(State *state, const OnlineTradePrepared *prepared, const uint8_t *source, size_t size) {
	if (!prepared || !prepared->ready || !prepared->checkpoint || prepared->bn5 || prepared->all_star ||
		prepared->loadout.souls || !envelope(source, size)) return false;
	state->bytes = source;
	state->pack = pointer(state, BN6_TOOLKIT_PACK);
	state->folders = pointer(state, BN6_TOOLKIT_CHIPS);
	state->marks = pointer(state, BN6_TOOLKIT_CHIP_MARKS);
	return folders_match(state, prepared) && pack_matches(state, prepared) && stats_match(state, &prepared->loadout);
}

static unsigned code_slot(const OnlineTradePrepared *prepared, uint16_t entry) {
	const OnlineTradeChip *chip = &prepared->chips[entry & (ONLINE_PACK_IDS - 1)];
	for (unsigned i = 0; i < ONLINE_TRADE_CODES; ++i) if (chip->codes[i] == (entry >> 9)) return i;
	return ONLINE_TRADE_CODES;
}

static size_t location(const State *state, uint32_t address) {
	return (size_t)(ram(state, address, 1) - state->bytes);
}

uint8_t *online_trade_stage(const OnlineTradePrepared *prepared, const OnlineTradeTerms *terms, unsigned side,
	const uint8_t *source, size_t size, size_t *written) {
	if (written) *written = 0;
	if (!written || side > 1 || !online_trade_terms_valid(terms) || terms->side[0].rules || terms->side[1].rules) return NULL;
	State state;
	if (!prepare(&state, prepared, source, size)) return NULL;
	uint16_t give = terms->side[side].chip, receive = terms->side[1 - side].chip;
	unsigned give_id = give & (ONLINE_PACK_IDS - 1), receive_id = receive & (ONLINE_PACK_IDS - 1);
	if (give_id >= BN6_PACK_IDS || receive_id >= BN6_PACK_IDS) return NULL;
	OnlineTradeInventory view = online_trade_inventory_view(prepared);
	if (online_trade_check(&view, give, receive, false) != ONLINE_TRADE_VALID) return NULL;
	unsigned give_slot = code_slot(prepared, give), receive_slot = code_slot(prepared, receive);
	if (give_slot >= ONLINE_TRADE_CODES || receive_slot >= ONLINE_TRADE_CODES) return NULL;
	uint8_t *out = malloc(size);
	if (!out) return NULL;
	memcpy(out, source, size);
	--out[location(&state, state.pack + give_id * BN6_PACK_ENTRY + give_slot)];
	++out[location(&state, state.pack + receive_id * BN6_PACK_ENTRY + receive_slot)];
	out[location(&state, state.marks + receive_id)] = *ram(&state, BN6_CHIP_KEYS + receive_id, 1) ^ BN6_CHIP_KEY_XOR;
	unsigned flag = BN6_FLAG_LIBRARY + receive_id;
	out[location(&state, BN6_EVENT_FLAGS + flag / 8)] |= (uint8_t)(0x80u >> (flag & 7));
	uint16_t order = get16(ram(&state, BN6_CHIP_ORDER_COUNTER, 2));
	put16(out + location(&state, state.pack + receive_id * BN6_PACK_ENTRY + BN6_PACK_ORDER + 2 * receive_slot), order);
	put16(out + location(&state, BN6_CHIP_ORDER_COUNTER), (uint16_t)(order - 1u));
	*written = size;
	return out;
}

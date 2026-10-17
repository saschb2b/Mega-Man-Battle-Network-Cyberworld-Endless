/* Synthetic refusal/ownership checks, not proof of original chip semantics. */
#include "online_trade_stage.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mgba/internal/gba/serialize.h>

#include "bytes.h"

#define TEST_MODE (BN6_EWRAM + 0x30000u)
#define TEST_CONTEXT (BN6_EWRAM + 0x30100u)
#define TEST_FOLDERS (BN6_EWRAM + 0x30200u)
#define TEST_MARKS (BN6_EWRAM + 0x30400u)
#define TEST_PACK (BN6_EWRAM + 0x31000u)
#define TEST_BYTES 430144u

static uint8_t source[TEST_BYTES], before[TEST_BYTES];
static OnlineTradePrepared prepared;
static OnlineTradeTerms terms;

static uint8_t *ram(uint32_t address) { return source + offsetof(struct GBASerializedState, wram) + address - BN6_EWRAM; }

static void fixture(void) {
	memset(source, 0, sizeof source); memset(&prepared, 0, sizeof prepared); memset(&terms, 0, sizeof terms);
	put32(source, 0x01000007); put32(source + offsetof(struct GBASerializedState, romCrc32), 0x79452182);
	put32(source + offsetof(struct GBASerializedState, id), 0x45355242);
	const uint32_t ext[] = {2, 32768, 397360, 0, 4, 16, 430128, 0, 0, 0, 0, 0};
	for (size_t i = 0; i < sizeof ext / sizeof *ext; ++i) put32(source + sizeof(struct GBASerializedState) + 4 * i, ext[i]);
	put32(ram(BN6_TOOLKIT), TEST_MODE); *ram(TEST_MODE) = BN6_MODE_GAME; *ram(BN6_GAMESTATE) = BN6_MODE_GAME;
	put32(ram(BN6_TOOLKIT + BN6_TOOLKIT_STEPS), TEST_CONTEXT); *ram(TEST_CONTEXT + BN6_FOLDER_COUNT_FIELD) = 1;
	put32(ram(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), TEST_FOLDERS); put16(ram(TEST_FOLDERS), 1);
	put32(ram(BN6_TOOLKIT + BN6_TOOLKIT_PACK), TEST_PACK); *ram(TEST_PACK + BN6_PACK_ENTRY) = 2;
	put32(ram(BN6_TOOLKIT + BN6_TOOLKIT_CHIP_MARKS), TEST_MARKS);
	prepared.ready = prepared.checkpoint = true; prepared.folder_count = 1;
	prepared.equipped[0] = prepared.loadout.folder[0] = 1; prepared.loadout.pack[1][0] = 2;
	for (unsigned i = 1; i <= 2; ++i) {
		prepared.chips[i] = (OnlineTradeChip){.canonical_id=(uint16_t)i, .codes={0,255,255,255}, .available=true};
		terms.side[i - 1].identity[0] = (uint8_t)i; terms.side[i - 1].run_id[0] = (uint8_t)(i + 2);
		terms.side[i - 1].checkpoint[0] = 5; terms.side[i - 1].chip = (uint16_t)i;
	}
	terms.server[0] = terms.transaction[0] = 1;
}

static void refused(void) {
	memcpy(before, source, sizeof before);
	OnlineTradePrepared held = prepared;
	size_t written = 7;
	assert(!online_trade_stage(&prepared, &terms, 0, source, sizeof source, &written));
	assert(!written && !memcmp(source, before, sizeof source) && !memcmp(&prepared, &held, sizeof prepared));
}

static void owned_success(void) {
	fixture(); memcpy(before, source, sizeof before);
	size_t written = 0;
	uint8_t *staged = online_trade_stage(&prepared, &terms, 0, source, sizeof source, &written);
	assert(staged && staged != source && written == sizeof source && !memcmp(source, before, sizeof source));
	free(staged);
	/* The other participant index selects that participant's same exact give. */
	OnlineTradeSide swap = terms.side[0]; terms.side[0] = terms.side[1]; terms.side[1] = swap;
	staged = online_trade_stage(&prepared, &terms, 1, source, sizeof source, &written);
	assert(staged && written == sizeof source); free(staged);
}

static void inventory_refusals(void) {
	fixture(); prepared.loadout.pack[1][0]++; refused(); /* stale Pack */
	fixture(); prepared.equipped[0] = 2; refused();
	fixture(); prepared.loadout.hp = 1; refused();
	fixture(); prepared.loadout.board[10] = 1; refused();
	fixture(); prepared.loadout.crosses = 1; refused();
	fixture(); *ram(TEST_PACK + BN6_PACK_ENTRY) = prepared.loadout.pack[1][0] = 1; refused(); /* only equipped */
	fixture(); terms.side[0].chip |= 2 << 9; refused(); /* unavailable exact code */
	fixture(); prepared.chips[2].available = false; refused();
	fixture(); prepared.chips[2].needs_bn5 = true; refused();
	fixture(); prepared.chips[1].codes[1] = 0; refused(); /* ambiguous code */
	fixture(); prepared.chips[2].canonical_id = 1; refused();
	fixture(); *ram(TEST_PACK + 2 * BN6_PACK_ENTRY) = prepared.loadout.pack[2][0] = 99; refused();
	fixture(); terms.side[1].chip = terms.side[0].chip; refused();
	fixture(); terms.side[0].rules = ONLINE_TRADE_RULE_ALL_STAR; refused();
	fixture(); prepared.bn5 = true; refused();
	fixture(); prepared.all_star = true; refused();
	fixture(); prepared.loadout.souls = 1; refused();
	fixture(); prepared.ready = false; refused();
	fixture(); prepared.checkpoint = false; refused();
}

static void envelope_refusals(void) {
	const uint32_t fields[] = {0, BN6_TOOLKIT_STEPS, BN6_TOOLKIT_CHIPS, BN6_TOOLKIT_PACK, BN6_TOOLKIT_CHIP_MARKS};
	for (size_t i = 0; i < sizeof fields / sizeof *fields; ++i) {
		fixture(); put32(ram(BN6_TOOLKIT + fields[i]), BN6_EWRAM_END); refused();
		fixture(); put32(ram(BN6_TOOLKIT + fields[i]), 0); refused();
	}
	fixture(); put32(ram(BN6_TOOLKIT + BN6_TOOLKIT_CHIPS), TEST_FOLDERS + 1); refused();
	fixture(); *ram(TEST_CONTEXT + BN6_FOLDER_COUNT_FIELD) = 4; refused();
	fixture(); *ram(BN6_NAVI_FOLDER) = 1; refused();
	fixture(); *ram(TEST_MODE) = BN6_MODE_SUBMENU; refused();
	fixture(); *ram(BN6_GAMESTATE) = 8; refused();
	fixture(); source[0] ^= 1; refused();
	fixture(); source[offsetof(struct GBASerializedState, romCrc32)] ^= 1; refused();
	fixture(); source[offsetof(struct GBASerializedState, id)] ^= 1; refused();
	fixture(); source[sizeof(struct GBASerializedState)] ^= 1; refused();
	fixture(); source[sizeof(struct GBASerializedState) + 8] ^= 1; refused();
	fixture(); size_t written = 9;
	assert(!online_trade_stage(&prepared, &terms, 0, source, sizeof source - 1, &written) && !written);
	assert(!online_trade_stage(&prepared, &terms, 2, source, sizeof source, &written) && !written);
	assert(!online_trade_stage(NULL, &terms, 0, source, sizeof source, &written) && !written);
	assert(!online_trade_stage(&prepared, NULL, 0, source, sizeof source, &written) && !written);
	assert(!online_trade_stage(&prepared, &terms, 0, NULL, sizeof source, &written) && !written);
	assert(!online_trade_stage(&prepared, &terms, 0, source, sizeof source, NULL));
}

int main(void) {
	owned_success(); inventory_refusals(); envelope_refusals();
	puts("online trade stage refusal tests passed");
	return 0;
}

/* Mystery Data table ownership (issue #122), with made-up ROM tables:
 * every original map must be empty before a jack-in can reroll its flags,
 * including maps no generated layer has visited. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "claims.h"
#include "emu.h"
#include "mapslot.h"

#define ROM_BASE 0x08000000u
#define RAM_BASE 0x02000000u
#define FIRST_MAPS (ROM_BASE + 0x1000)
#define HOME_MAPS (ROM_BASE + 0x1100)
#define AFTER_END (ROM_BASE + 0x1200)
#define ORIGINAL (ROM_BASE + 0x2000)

static uint8_t rom[EMU_ROM_SIZE], ram[0x40000];
static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static uint8_t *memory(uint32_t addr, size_t len) {
	if (addr >= ROM_BASE && addr - ROM_BASE < sizeof rom) {
		assert(len <= sizeof rom - (addr - ROM_BASE));
		return rom + (addr - ROM_BASE);
	}
	assert(addr >= RAM_BASE && addr - RAM_BASE < sizeof ram);
	assert(len <= sizeof ram - (addr - RAM_BASE));
	return ram + (addr - RAM_BASE);
}

uint8_t emu_read8(uint32_t addr) { return *memory(addr, 1); }
uint16_t emu_read16(uint32_t addr) {
	return (uint16_t)(emu_read8(addr) | (uint16_t)emu_read8(addr + 1) << 8);
}
uint32_t emu_read32(uint32_t addr) {
	return (uint32_t)emu_read16(addr) | (uint32_t)emu_read16(addr + 2) << 16;
}
void emu_write8(uint32_t addr, uint8_t v) {
	claims_note(addr, 1);
	*memory(addr, 1) = v;
}
void emu_write32(uint32_t addr, uint32_t v) {
	claims_note(addr, 4);
	put32(memory(addr, 4), v);
}
void emu_write(uint32_t addr, const void *data, size_t len) {
	claims_note(addr, (uint32_t)len);
	memcpy(memory(addr, len), data, len);
}

static void fixture(void) {
	/* Group 0 is a hole, not the end. Neither its pointer nor the one
	 * following group 1 may be followed. */
	static const uint32_t groups[] = { 0, 0, 0x80, FIRST_MAPS, 0, 0, 0x88, HOME_MAPS, 1, 0, 0x89, AFTER_END };
	for (size_t i = 0; i < sizeof groups / sizeof groups[0]; ++i)
		emu_write32(BN6_MYSTERY_DATA + (uint32_t)i * 4, groups[i]);
	for (int i = 0; i < 3; ++i) {
		emu_write32(FIRST_MAPS + (uint32_t)i * 4, ORIGINAL);
		emu_write32(HOME_MAPS + (uint32_t)i * 4, ORIGINAL);
	}
	/* Each map table ends at its first null, with unrelated data after. */
	emu_write32(FIRST_MAPS + 16, ORIGINAL);
	emu_write32(HOME_MAPS + 16, ORIGINAL);
	emu_write32(AFTER_END, ORIGINAL);
	emu_write8(ORIGINAL, MYSTERY_GREEN);
	/* The only other tables mapslot_install needs to succeed. */
	emu_write32(BN6_NPC_LISTS, ROM_BASE + 0x3000);
	emu_write32(BN6_NPC_LISTS + 8 * 4, ROM_BASE + 0x3100);
}

static void check_tables(uint32_t active) {
	static const uint32_t maps[] = { FIRST_MAPS, HOME_MAPS };
	for (size_t g = 0; g < sizeof maps / sizeof maps[0]; ++g) {
		for (int i = 0; i < 3; ++i) {
			uint32_t slot = maps[g] + (uint32_t)i * 4;
			if (slot != active)
				CHECK(emu_read32(slot) == FREE_NO_MYSTERY, "unused map %08x is empty, even if never visited", slot);
		}
		CHECK(emu_read32(maps[g] + 12) == 0, "map table %08x still ends at its null", maps[g]);
		CHECK(emu_read32(maps[g] + 16) == ORIGINAL, "data after map table %08x is untouched", maps[g]);
	}
	CHECK(emu_read32(AFTER_END) == ORIGINAL, "the group table stops at group 1");
	CHECK(emu_read32(BN6_MYSTERY_DATA) == 0 && emu_read32(BN6_MYSTERY_DATA + 16) == 0,
		"group 0 holes are preserved");
	for (int i = 0; i < 12; ++i)
		CHECK(emu_read8(FREE_NO_MYSTERY + (uint32_t)i) == 0, "shared empty list byte %d", i);
}

static uint32_t install_layer(int group, int number) {
	static const MysteryData md[] = {
		{ .x = 80, .y = 120, .type = MYSTERY_GREEN, .content = { 1, 2, 3, 4, 5, 6, 7, 8 } },
		{ .x = 160, .y = 240, .type = MYSTERY_BLUE, .content = { 8, 7, 6, 5, 4, 3, 2, 1 } },
	};
	mapslot_reset();
	CHECK(mapslot_install(group, number, NULL, md, 2), "layer %02x/%d installs", group, number);
	uint32_t slot = (group == 0x80 ? FIRST_MAPS : HOME_MAPS) + (uint32_t)number * 4;
	uint32_t list = emu_read32(slot);
	for (int i = 0; i < 2; ++i) {
		uint32_t entry = list + (uint32_t)i * 12;
		CHECK(emu_read8(entry) == md[i].type && emu_read16(entry + 2) == MAPSLOT_MD_FLAG + i,
			"active Mystery Data %d keeps its type and flag", i);
		uint32_t place = emu_read32(entry + 4), content = emu_read32(entry + 8);
		CHECK(emu_read16(place + 2) == md[i].x && emu_read16(place + 4) == md[i].y,
			"active Mystery Data %d keeps its placement", i);
		CHECK(memcmp(memory(content, 8), md[i].content, 8) == 0, "active Mystery Data %d keeps its content", i);
	}
	CHECK(emu_read32(list + 24) == 0, "generated list still terminates");
	check_tables(slot);
	return list;
}

int main(void) {
	fixture();
	/* A home install with no Mystery Data changes only its own map. */
	mapslot_hp(true);
	CHECK(mapslot_install(0x88, 0, NULL, NULL, 0), "home installs before a layer");
	mapslot_hp(false);
	CHECK(emu_read32(HOME_MAPS) == FREE_NO_MYSTERY && emu_read32(FIRST_MAPS) == ORIGINAL &&
		emu_read32(HOME_MAPS + 4) == ORIGINAL, "home initially leaves other maps alone");
	uint32_t first = install_layer(0x80, 1);
	uint8_t saved[36];
	memcpy(saved, memory(first, sizeof saved), sizeof saved);
	mapslot_hp(true);
	CHECK(mapslot_install(0x88, 0, NULL, NULL, 0), "home installs beside the pending layer");
	mapslot_hp(false);
	CHECK(emu_read32(FIRST_MAPS + 4) == first && memcmp(memory(first, sizeof saved), saved, sizeof saved) == 0,
		"home keeps the pending layer's list intact");
	check_tables(FIRST_MAPS + 4);
	/* Replacing the layer empties the old map even as its arena is reused. */
	(void)install_layer(0x88, 2);
	(void)install_layer(0x80, 0);
	CHECK(claims_who(FREE_NO_MYSTERY) && claims_who(FIRST_MAPS) && claims_who(HOME_MAPS),
		"Mystery Data lists and map slots are claimed");
	CHECK(claims_overlaps() == 0, "home and replacement layers keep the same Mystery Data owner");
	printf("mapslot: %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
	return failures ? 1 : 0;
}

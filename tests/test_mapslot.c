/* The maps' Mystery Data lists as mapslot.c leaves them, in a made-up
 * ROM's tables (issue #122): a layer's install points every map BN6's
 * jack-in roll reaches at the empty list but its own, maps no layer took
 * too, and leaves the table's holes, its end, each group's null and what
 * follows them alone; home's installs, with none, point only their own
 * map at it, and keep the layer's list. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "emu.h"
#include "mapslot.h"

#define ROM_BASE 0x08000000u
#define RAM_BASE 0x02000000u
#define NET_MAPS (ROM_BASE + 0x1000)    /* group 0x80's three maps' lists, a null, a word past it */
#define HOME_MAPS (ROM_BASE + 0x1100)   /* group 0x88's, the same */
#define PAST_END (ROM_BASE + 0x1200)    /* a group's maps after the table's end */
#define ORIGINAL (ROM_BASE + 0x2000)    /* one green Mystery Data, BN6's own */

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
uint16_t emu_read16(uint32_t addr) { return get16(memory(addr, 2)); }
uint32_t emu_read32(uint32_t addr) { return get32(memory(addr, 4)); }
void emu_write8(uint32_t addr, uint8_t v) { *memory(addr, 1) = v; }
void emu_write32(uint32_t addr, uint32_t v) { put32(memory(addr, 4), v); }
void emu_write(uint32_t addr, const void *data, size_t len) { memcpy(memory(addr, len), data, len); }

static void fixture(void) {
	/* (a hole first and between, as BN6's table has them; a group past
	 * the end's 1) */
	static const uint32_t groups[] = { 0, 0, 0x80, NET_MAPS, 0, 0, 0x88, HOME_MAPS, 1, 0, 0x89, PAST_END };
	for (size_t i = 0; i < sizeof groups / sizeof groups[0]; ++i)
		emu_write32(BN6_MYSTERY_DATA + (uint32_t)i * 4, groups[i]);
	for (int i = 0; i < 3; ++i) {
		emu_write32(NET_MAPS + (uint32_t)i * 4, ORIGINAL);
		emu_write32(HOME_MAPS + (uint32_t)i * 4, ORIGINAL);
	}
	emu_write32(NET_MAPS + 16, ORIGINAL);
	emu_write32(HOME_MAPS + 16, ORIGINAL);
	emu_write32(PAST_END, ORIGINAL);
	emu_write8(ORIGINAL, MYSTERY_GREEN);
	/* (the groups' NPC lists, which an install needs) */
	emu_write32(BN6_NPC_LISTS, ROM_BASE + 0x3000);
	emu_write32(BN6_NPC_LISTS + 8 * 4, ROM_BASE + 0x3100);
}

/* Every map but `own`'s at the empty list, and the rest as BN6 has it */
static void check_tables(uint32_t own) {
	static const uint32_t groups[] = { NET_MAPS, HOME_MAPS };
	for (size_t g = 0; g < sizeof groups / sizeof groups[0]; ++g) {
		for (int i = 0; i < 3; ++i) {
			uint32_t slot = groups[g] + (uint32_t)i * 4;
			if (slot != own) CHECK(emu_read32(slot) == FREE_NO_MYSTERY, "map %08x at the empty list, taken by a layer or not", slot);
		}
		CHECK(emu_read32(groups[g] + 12) == 0, "group %08x's maps still end at their null", groups[g]);
		CHECK(emu_read32(groups[g] + 16) == ORIGINAL, "the word past group %08x's null untouched", groups[g]);
	}
	CHECK(emu_read32(PAST_END) == ORIGINAL, "the group past the table's end untouched");
	CHECK(emu_read32(BN6_MYSTERY_DATA) == 0 && emu_read32(BN6_MYSTERY_DATA + 16) == 0, "the table's holes kept");
	for (int i = 0; i < 12; ++i) CHECK(emu_read8(FREE_NO_MYSTERY + (uint32_t)i) == 0, "the empty list's byte %d", i);
}

/* A layer of two Mystery Data on group:number; its list */
static uint32_t install_layer(int group, int number) {
	static const MysteryData md[] = {
		{ .x = 80, .y = 120, .type = MYSTERY_GREEN, .content = { 1, 2, 3, 4, 5, 6, 7, 8 } },
		{ .x = 160, .y = 240, .type = MYSTERY_BLUE, .content = { 8, 7, 6, 5, 4, 3, 2, 1 } },
	};
	mapslot_reset();
	CHECK(mapslot_install(group, number, NULL, md, 2), "a layer on %02x:%d", group, number);
	uint32_t slot = (group == 0x80 ? NET_MAPS : HOME_MAPS) + (uint32_t)number * 4;
	uint32_t list = emu_read32(slot);
	for (int i = 0; i < 2; ++i) {
		uint32_t entry = list + (uint32_t)i * 12;
		CHECK(emu_read8(entry) == md[i].type && emu_read16(entry + 2) == MAPSLOT_MD_FLAG + i, "the layer's Mystery Data %d, its type and flag", i);
		uint32_t place = emu_read32(entry + 4), content = emu_read32(entry + 8);
		CHECK(emu_read16(place + 2) == md[i].x && emu_read16(place + 4) == md[i].y, "the layer's Mystery Data %d, its place", i);
		CHECK(memcmp(memory(content, 8), md[i].content, 8) == 0, "the layer's Mystery Data %d, its content", i);
	}
	CHECK(emu_read32(list + 24) == 0, "the layer's list ends");
	check_tables(slot);
	return list;
}

int main(void) {
	fixture();
	/* home before any layer: its own map alone */
	mapslot_hp(true);
	CHECK(mapslot_install(0x88, 0, NULL, NULL, 0), "home before a layer");
	mapslot_hp(false);
	CHECK(emu_read32(HOME_MAPS) == FREE_NO_MYSTERY && emu_read32(NET_MAPS) == ORIGINAL && emu_read32(HOME_MAPS + 4) == ORIGINAL,
		"home before a layer: its own map at the empty list, the rest left");
	/* a layer: every other map BN6's roll reaches emptied */
	uint32_t first = install_layer(0x80, 1);
	uint8_t kept[36];
	memcpy(kept, memory(first, sizeof kept), sizeof kept);
	/* home after it: the layer's list kept for the jack-in */
	mapslot_hp(true);
	CHECK(mapslot_install(0x88, 0, NULL, NULL, 0), "home after a layer");
	mapslot_hp(false);
	CHECK(emu_read32(NET_MAPS + 4) == first && !memcmp(memory(first, sizeof kept), kept, sizeof kept), "home after a layer: the layer's list kept");
	check_tables(NET_MAPS + 4);
	/* the next layers: the last one's map emptied, its half written over */
	install_layer(0x88, 2);
	install_layer(0x80, 0);
	if (failures) return 1;
	printf("mapslot: every map but the layer's at the empty list, BN6's own too; the table's holes, end and nulls kept\n");
	return 0;
}

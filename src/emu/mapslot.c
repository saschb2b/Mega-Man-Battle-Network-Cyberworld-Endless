/* Taking over a game map for a layer (tables per bn6f; see docs/ROM_DATA.md).
 *
 * NPCs: NPCList_maps80 -> per-map list of NPC script pointers ending 0xFF.
 * Map scripts: (on enter, continuous) per-map lists; ms_end (0x00) runs none.
 * Objects: each group's SpawnMapObjectsForMap loads its per-map table with a
 * PC-relative literal, read from the routine itself.
 * Mystery Data: (group, per-map list) pairs; a map's list holds 12-byte
 * entries (type, flag, placements, contents) ending with 0; the game's pick
 * per flag (placement, content) lives at 0x02004348 + 2 * (flag - 0x1400). */
#include "mapslot.h"

#include <string.h>

#include "bn6.h"
#include "emu.h"

#define SCRATCH     (EMU_FREE + 0x3000)  /* bump allocator for layer data */
#define SCRATCH_END (EMU_FREE + 0x10000)

static uint32_t next = SCRATCH;

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

void mapslot_reset(void) { next = SCRATCH; }

uint32_t mapslot_alloc(const void *bytes, int len) {
	uint32_t at = next;
	if (at + (uint32_t)len > SCRATCH_END) return 0;
	emu_write(at, bytes, (size_t)len);
	next = (at + (uint32_t)len + 3) & ~3u;
	return at;
}

/* The per-map table a group's object spawner reads: its first
 * "ldr r1, [pc, #imm]" literal. */
static uint32_t object_table(int group) {
	uint32_t fn = emu_read32(BN6_OBJ_SPAWNERS + (uint32_t)(group - 0x80) * 4) & ~1u;
	if (fn < 0x08000000u) return 0;
	for (uint32_t pc = fn; pc < fn + 32; pc += 2) {
		uint16_t op = emu_read16(pc);
		if ((op & 0xFF00) == 0x4900) return emu_read32(((pc + 4) & ~3u) + (uint32_t)(op & 0xFF) * 4);
	}
	return 0;
}

/* The map's list in the Mystery Data table. */
static uint32_t mystery_slot(int group, int number) {
	for (uint32_t a = BN6_MYSTERY_DATA; emu_read32(a) != 1; a += 8)
		if (emu_read32(a) == (uint32_t)group) return emu_read32(a + 4) + (uint32_t)number * 4;
	return 0;
}

bool mapslot_install(int group, int number, const NpcList *npcs, const MysteryData *md, int nmd) {
	uint32_t g = (uint32_t)(group - 0x80);
	/* NPCs */
	uint8_t list[33 * 4];
	int n = npcs ? npcs->n : 0;
	for (int i = 0; i < n; ++i) put32(list + i * 4, npcs->script[i]);
	put32(list + n * 4, 0xFF);
	uint32_t npc_at = mapslot_alloc(list, (n + 1) * 4);
	uint32_t npc_lists = emu_read32(BN6_NPC_LISTS + g * 4);
	if (!npc_at || npc_lists < 0x08000000u) return false;
	emu_write32(npc_lists + (uint32_t)number * 4, npc_at);
	/* map scripts: none */
	static const uint8_t ms_end[4] = { 0 };
	uint32_t end_at = mapslot_alloc(ms_end, 4);
	for (int k = 0; k < 2; ++k) {
		uint32_t scripts = emu_read32(BN6_MAP_SCRIPTS + g * 8 + (uint32_t)k * 4);
		if (scripts >= 0x08000000u) emu_write32(scripts + (uint32_t)number * 4, end_at);
	}
	/* objects: none */
	uint32_t objs = object_table(group);
	static const uint8_t no_objects[4] = { 0xFF };
	if (objs >= 0x08000000u) emu_write32(objs + (uint32_t)number * 4, mapslot_alloc(no_objects, 4));
	/* Mystery Data: one placement and one content each, picked and not taken */
	uint8_t entries[33 * 12];
	if (nmd > 32) nmd = 32;
	for (int i = 0; i < nmd; ++i) {
		uint8_t place[16] = { 1, 0x20 };
		place[2] = (uint8_t)md[i].x; place[3] = (uint8_t)(md[i].x >> 8);
		place[4] = (uint8_t)md[i].y; place[5] = (uint8_t)(md[i].y >> 8);
		uint8_t content[16];
		memcpy(content, md[i].content, 8);
		memset(content + 8, 0, 8);
		uint32_t pa = mapslot_alloc(place, 16), ca = mapslot_alloc(content, 16);
		uint16_t flag = (uint16_t)(MAPSLOT_MD_FLAG + i);
		uint8_t *e = entries + i * 12;
		e[0] = 5; e[1] = 0; e[2] = (uint8_t)flag; e[3] = (uint8_t)(flag >> 8);   /* green */
		put32(e + 4, pa);
		put32(e + 8, ca);
		emu_write8(BN6_MYSTERY_PICKS + (uint32_t)i * 2, 0);
		emu_write8(BN6_MYSTERY_PICKS + (uint32_t)i * 2 + 1, 0);
		/* not taken yet */
		uint32_t fb = BN6_EVENT_FLAGS + flag / 8u;
		emu_write8(fb, (uint8_t)(emu_read8(fb) & ~(0x80u >> (flag & 7))));
	}
	memset(entries + nmd * 12, 0, 12);
	uint32_t md_at = mapslot_alloc(entries, (nmd + 1) * 12);
	uint32_t slot = mystery_slot(group, number);
	if (slot) emu_write32(slot, md_at);
	return true;
}

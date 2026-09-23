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
#include "flags.h"

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

/* The per-map sprite load lists a group's loader passes to uncompSprite:
 * the literal of "lsl r1,r1,#2; ldr r0,[pc,#n]; ldr r0,[r0,r1]". */
static uint32_t sprite_table(int group) {
	uint32_t fn = emu_read32(BN6_ENTER_GROUP + (uint32_t)(group - 0x80) * 4) & ~1u;
	if (fn < 0x08000000u) return 0;
	for (uint32_t pc = fn; pc < fn + 160; pc += 2) {
		uint16_t op = emu_read16(pc + 2);
		if (emu_read16(pc) == 0x0089 && (op & 0xFF00) == 0x4800 && emu_read16(pc + 4) == 0x5840)
			return emu_read32(((pc + 2 + 4) & ~3u) + (uint32_t)(op & 0xFF) * 4);
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
	/* sprites to decompress for the map: the layer's (the original's objects
	 * are gone, and their sprites would fill the buffer) */
	uint32_t sprites = sprite_table(group);
	if (sprites >= 0x08000000u && npcs) {
		uint8_t list[2 * 8 + 2];
		int n = 0;
		for (int i = 0; i < npcs->nsprites && i < 8; ++i) {
			list[n++] = npcs->sprite_cat[i];
			list[n++] = npcs->sprite_idx[i];
		}
		list[n++] = 0xFF;
		list[n++] = 0xFF;
		uint32_t at = mapslot_alloc(list, n);
		if (at) emu_write32(sprites + (uint32_t)number * 4, at);
	}
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
		e[0] = (uint8_t)md[i].type; e[1] = 0; e[2] = (uint8_t)flag; e[3] = (uint8_t)(flag >> 8);
		put32(e + 4, pa);
		put32(e + 8, ca);
		emu_write8(BN6_MYSTERY_PICKS + (uint32_t)i * 2, 0);
		emu_write8(BN6_MYSTERY_PICKS + (uint32_t)i * 2 + 1, 0);
		/* not taken yet */
		flag_clear(flag);
	}
	memset(entries + nmd * 12, 0, 12);
	uint32_t md_at = mapslot_alloc(entries, (nmd + 1) * 12);
	uint32_t slot = mystery_slot(group, number);
	if (slot) emu_write32(slot, md_at);
	return true;
}

bool mapslot_music(int group, int number, int song) {
	/* the song per map of the group, then a list holding only that group */
	uint8_t songs[16];
	memset(songs, 0x63, sizeof songs);          /* 0x63: no song */
	if (number < 0 || number >= 16) return false;
	songs[number] = (uint8_t)song;
	uint32_t at = mapslot_alloc(songs, sizeof songs);
	uint8_t list[16] = { (uint8_t)group };
	put32(list + 4, at);
	list[8] = 0xFF;
	uint32_t list_at = mapslot_alloc(list, sizeof list);
	if (!at || !list_at) return false;
	for (int i = 0; i < BN6_MAP_MUSIC_LISTS; ++i) emu_write32(BN6_MAP_MUSIC + (uint32_t)i * 4, list_at);
	return true;
}

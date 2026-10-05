/* Taking over a game map for a layer or the town (tables per bn6f; see
 * docs/ROM_DATA.md). Real-world groups (0x00-0x06) have the same tables at
 * other addresses (RW_*).
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
#include "bytes.h"
#include "emu.h"
#include "flags.h"
#include "lz.h"

/* Layer data comes from a bump allocator over two halves, one per layer in
 * turn: the map being left keeps running its NPC scripts while the next
 * layer is built during its warp. */
#define SCRATCH      (EMU_FREE + 0x3000)
#define SCRATCH_HALF 0x6800
#define WARP_LIST    (EMU_FREE + 0x2F00)  /* the layer map's warps: entry 1 is the exit */
#define NO_MYSTERY   (EMU_FREE + 0x0180)  /* an empty Mystery Data list, for the maps the layers left */
#define TOWN_ARENA   (EMU_FREE + 0x140000) /* the town's data, apart from the layers' */
#define TOWN_SIZE    0x8000
#define HP_ARENA     (EMU_FREE + 0x148000) /* Lan's HP's (docs/HOME.md) */
#define HP_SIZE      0x8000

#define RW_GROUPS      7
#define RW_NPC_LISTS   0x08034638u /* NPCList_maps00 */
#define RW_MAP_SCRIPTS 0x080345E4u /* RealWorldMapScriptPointers: (on enter, continuous) per group */
#define RW_OBJ_SPAWNERS 0x08034654u /* RealWorldSpawnMapObjectJumptable */
#define RW_ENTER_GROUP 0x08030904u /* EnterMap_RealWorldMapGroupJumptable */
#define RW_CHECK_TABLES 0x0803461Cu /* per real-world group, per map: the 16 checks' text scripts (bn6f off_803461C) */
#define MAP_TEXT_ARCHIVES 0x08040794u /* per map, its LZ77 text archive (bn6f mapPtrs80407C0: real world, internet) */
#define JACK_IN_RECORDS 0x08099A00u /* 20-byte jack-in destinations (bn6f byte_80984C8) */
#define JACK_IN_RECORD  42           /* the one the town rewrites (a comp the run never visits) */

static int half;
static uint32_t arena;   /* the space allocation is from: 0 the layers' halves */
static uint32_t next = SCRATCH, end = SCRATCH + SCRATCH_HALF;
static uint32_t layer_next, layer_end;

void mapslot_reset(void) {
	half ^= 1;
	next = SCRATCH + (uint32_t)half * SCRATCH_HALF;
	end = next + SCRATCH_HALF;
}

static void use_arena(uint32_t base, uint32_t size) {
	if (base == arena) return;
	if (!arena) { layer_next = next; layer_end = end; }
	arena = base;
	if (base) { next = base; end = base + size; }
	else { next = layer_next; end = layer_end; }
}

void mapslot_town(bool on) { use_arena(on ? TOWN_ARENA : 0, TOWN_SIZE); }
void mapslot_hp(bool on) { use_arena(on ? HP_ARENA : 0, HP_SIZE / 2); }
void mapslot_house(bool on) { use_arena(on ? HP_ARENA + HP_SIZE / 2 : 0, HP_SIZE / 2); }

uint32_t mapslot_alloc(const void *bytes, int len) {
	uint32_t at = next;
	if (at + (uint32_t)len > end) return 0;
	emu_write(at, bytes, (size_t)len);
	next = (at + (uint32_t)len + 3) & ~3u;
	return at;
}

static bool real_world(int group) { return group >= 0 && group < RW_GROUPS; }
static uint32_t group_entry(uint32_t rw_table, uint32_t net_table, int group) {
	return real_world(group) ? rw_table + (uint32_t)group * 4 : net_table + (uint32_t)(group - 0x80) * 4;
}

/* The per-map table a group's object spawner reads: its first
 * "ldr r1, [pc, #imm]" literal. */
static uint32_t object_table(int group) {
	uint32_t fn = emu_read32(group_entry(RW_OBJ_SPAWNERS, BN6_OBJ_SPAWNERS, group)) & ~1u;
	if (fn < 0x08000000u) return 0;
	for (uint32_t pc = fn; pc < fn + 32; pc += 2) {
		uint16_t op = emu_read16(pc);
		if ((op & 0xFF00) == 0x4900) return emu_read32(((pc + 4) & ~3u) + (uint32_t)(op & 0xFF) * 4);
	}
	return 0;
}

uint32_t mapslot_objects(int group, int number) {
	uint32_t objs = object_table(group);
	return objs >= 0x08000000u ? emu_read32(objs + (uint32_t)number * 4) : 0;
}

/* The per-map sprite load lists a group's loader passes to uncompSprite:
 * the literal of "lsl r1,r1,#2; ldr r0,[pc,#n]; ldr r0,[r0,r1]". */
static uint32_t sprite_table(int group) {
	uint32_t fn = emu_read32(group_entry(RW_ENTER_GROUP, BN6_ENTER_GROUP, group)) & ~1u;
	if (fn < 0x08000000u) return 0;
	for (uint32_t pc = fn; pc < fn + 160; pc += 2) {
		uint16_t op = emu_read16(pc + 2);
		if (emu_read16(pc) == 0x0089 && (op & 0xFF00) == 0x4800 && emu_read16(pc + 4) == 0x5840)
			return emu_read32(((pc + 2 + 4) & ~3u) + (uint32_t)(op & 0xFF) * 4);
	}
	return 0;
}

/* The per-map warp lists a group's loader hands the game: the literal of its
 * first "ldr r0, [pc]". */
static uint32_t warp_table(int group) {
	uint32_t fn = emu_read32(group_entry(RW_ENTER_GROUP, BN6_ENTER_GROUP, group)) & ~1u;
	if (fn < 0x08000000u) return 0;
	for (uint32_t pc = fn; pc < fn + 16; pc += 2) {
		uint16_t op = emu_read16(pc);
		if ((op & 0xF800) == 0x4800) return emu_read32(((pc + 4) & ~3u) + (uint32_t)(op & 0xFF) * 4);
	}
	return 0;
}

/* The per-map jack-in tables a real-world group's loader stores in
 * GameState+0x64: the literal loaded before its "str r0, [r5, #0x64]". */
static uint32_t jack_in_table(int group) {
	uint32_t fn = emu_read32(group_entry(RW_ENTER_GROUP, BN6_ENTER_GROUP, group)) & ~1u;
	if (fn < 0x08000000u || !real_world(group)) return 0;
	uint32_t lit = 0;
	for (uint32_t pc = fn; pc < fn + 200; pc += 2) {
		uint16_t op = emu_read16(pc);
		if ((op & 0xFF00) == 0x4800) lit = emu_read32(((pc + 4) & ~3u) + (uint32_t)(op & 0xFF) * 4);
		if (op == 0x6668) return lit;   /* str r0, [r5, #0x64] */
	}
	return 0;
}

bool mapslot_jack_in(int group, int number, int to_group, int to_number, int x, int y, int facing) {
	/* trigger 0x40 (index 0) -> the record the town takes over */
	uint8_t table[24];
	memset(table, 0xFF, sizeof table);
	table[0] = JACK_IN_RECORD;
	uint32_t at = mapslot_alloc(table, sizeof table), tables = jack_in_table(group);
	if (!at || tables < 0x08000000u) return false;
	emu_write32(tables + (uint32_t)number * 4, at);
	/* WarpData (group, number, departure, facing, x, y, z), then the "Jack in!" line: 0 */
	uint8_t rec[20] = { (uint8_t)to_group, (uint8_t)to_number, 0, (uint8_t)facing };
	put32(rec + 4, (uint32_t)x << 16);
	put32(rec + 8, (uint32_t)y << 16);
	emu_write(JACK_IN_RECORDS + JACK_IN_RECORD * 20, rec, sizeof rec);
	return true;
}

uint32_t mapslot_own_warps(int group, int number, int x, int y, int facing) {
	uint8_t list[16 * 16];
	for (int i = 0; i < 16; ++i) {
		uint8_t *e = list + i * 16;
		memset(e, 0, 16);
		e[0] = (uint8_t)group; e[1] = (uint8_t)number; e[3] = (uint8_t)facing;
		put32(e + 4, (uint32_t)x << 16);
		put32(e + 8, (uint32_t)y << 16);
	}
	uint32_t at = mapslot_alloc(list, sizeof list), warps = warp_table(group);
	if (!at || warps < 0x08000000u) return 0;
	emu_write32(warps + (uint32_t)number * 4, at);
	return at;
}


void mapslot_copy_warp(uint32_t list, int entry, uint32_t from, int from_entry) {
	if (!list || !from || entry < 1 || entry > 16 || from_entry < 1 || from_entry > 16) return;
	uint8_t e[16];
	for (int i = 0; i < 16; ++i) e[i] = emu_read8(from + 16u * (uint32_t)(from_entry - 1) + (uint32_t)i);
	emu_write(list + 16u * (uint32_t)(entry - 1), e, sizeof e);
}

uint32_t mapslot_warps(int group, int number) {
	uint32_t warps = warp_table(group);
	return warps >= 0x08000000u ? emu_read32(warps + (uint32_t)number * 4) : 0;
}

void mapslot_warp(uint32_t list, int entry, int group, int number, int x, int y, int facing) {
	if (!list || entry < 1 || entry > 16) return;
	/* WarpData: group, number, departure (8: jack out, fade, jack in), facing, x, y, z (16.16) */
	uint8_t e[16] = { (uint8_t)group, (uint8_t)number, 0x08, (uint8_t)facing };
	put32(e + 4, (uint32_t)x << 16);
	put32(e + 8, (uint32_t)y << 16);
	emu_write(list + 16u * (uint32_t)(entry - 1), e, sizeof e);
}

bool mapslot_jack_record(int index, int *group, int *number, int *x, int *y) {
	uint32_t at = JACK_IN_RECORDS + (uint32_t)index * 20;
	if (index < 0 || index >= JACK_IN_RECORD) return false;
	*group = emu_read8(at);
	*number = emu_read8(at + 1);
	*x = (int32_t)emu_read32(at + 4) >> 16;
	*y = (int32_t)emu_read32(at + 8) >> 16;
	return true;
}

bool mapslot_checks(int group, int number, const uint8_t script[16], const uint8_t *archive, int len) {
	if (!real_world(group) || len <= 0 || len > 0x1600 - 4) return false;
	/* the map's archive decompresses to 0x02033400; the checks read it
	 * from +4, after a word holding its size */
	static uint8_t raw[0x1600], lz[0x1600 + 0x1600 / 8 + 16];
	raw[0] = 0;
	raw[1] = (uint8_t)(len + 4);
	raw[2] = (uint8_t)((len + 4) >> 8);
	raw[3] = 0;
	memcpy(raw + 4, archive, (size_t)len);
	size_t n = lz_literal(raw, (size_t)len + 4, lz);
	uint32_t at = mapslot_alloc(lz, (int)n), table = mapslot_alloc(script, 16);
	uint32_t archives = emu_read32(emu_read32(MAP_TEXT_ARCHIVES) + (uint32_t)group * 4);
	uint32_t checks = emu_read32(RW_CHECK_TABLES + (uint32_t)group * 4);
	if (!at || !table || archives < 0x08000000u || checks < 0x08000000u) return false;
	emu_write32(archives + (uint32_t)number * 4, at);
	emu_write32(checks + (uint32_t)number * 4, table);
	/* (flags 0x16C0 + n turn check n off; the story sets some) */
	for (int i = 0; i < 16; ++i) flag_clear(0x16C0 + i);
	return true;
}

void mapslot_exit_to(int group, int number, int x, int y, int facing) {
	/* WarpData: group, number, departure (8: jack out, fade, jack in), facing, x, y, z (16.16) */
	uint8_t e[16] = { (uint8_t)group, (uint8_t)number, 0x08, (uint8_t)facing };
	put32(e + 4, (uint32_t)x << 16);
	put32(e + 8, (uint32_t)y << 16);
	emu_write(WARP_LIST, e, sizeof e);
}

void mapslot_teleport(int entry, int group, int number, int x, int y, int facing) {
	if (entry < 2 || entry > 15) return;
	uint8_t e[16] = { (uint8_t)group, (uint8_t)number, 0x0C, (uint8_t)facing };
	put32(e + 4, (uint32_t)x << 16);
	put32(e + 8, (uint32_t)y << 16);
	emu_write(WARP_LIST + 16u * (uint32_t)(entry - 1), e, sizeof e);
}

/* The map's list in the Mystery Data table. */
static uint32_t mystery_slot(int group, int number) {
	for (uint32_t a = BN6_MYSTERY_DATA; emu_read32(a) != 1; a += 8)
		if (emu_read32(a) == (uint32_t)group) return emu_read32(a + 4) + (uint32_t)number * 4;
	return 0;
}

/* `slot` given the layer's list at `md_at`, and every other map a layer
 * took an empty one. BN6 rolls every net map's green Mystery Data again at
 * each jack-in (bn6f sub_8033FDC calls sub_809F9DC, docs/ROM_DATA.md), and
 * a map a layer left kept its list in the half a later layer has written
 * over: a jack-in from home read one and crashed the game. (An empty
 * list, not none: the roll ends a group's maps at a null.) */
static void mystery_none(uint32_t slot) {
	static const uint8_t none[12] = { 0 };
	emu_write(NO_MYSTERY, none, sizeof none);
	emu_write32(slot, NO_MYSTERY);
}

static void mystery_only(uint32_t slot, uint32_t md_at) {
	static uint32_t slots[64];   /* (more than the maps the areas take) */
	static int n;
	static const uint8_t none[12] = { 0 };
	emu_write(NO_MYSTERY, none, sizeof none);
	bool known = false;
	for (int i = 0; i < n; ++i) {
		known |= slots[i] == slot;
		if (slots[i] != slot) emu_write32(slots[i], NO_MYSTERY);
	}
	if (!known && n < (int)(sizeof slots / sizeof slots[0])) slots[n++] = slot;
	emu_write32(slot, md_at);
}

/* The map's Mystery Data (mapslot_install's, a net map's) */
static bool install_mystery(int group, int number, const MysteryData *md, int nmd) {
	/* (none for a map of the run's own, as Lan's HP: the layers' lists
	 * left as they are) */
	if (!md) {
		uint32_t own = mystery_slot(group, number);
		if (own) mystery_none(own);
		return true;
	}
	/* Mystery Data: one placement and one content each, picked and not taken.
	 * Each is written eight times over: the picks are kept per flag
	 * (0x1400 + n), and the game's own Mystery Data of other maps share
	 * those flags; a new game steps their picks (bn6f sub_809FAF4), and a
	 * pick past our one record read zeros (a Mystery Data at the world's
	 * origin, another that said it was locked and printed stray text). */
	uint8_t entries[33 * 12];
	if (nmd > 32) nmd = 32;
	enum { COPIES = 8 };
	for (int i = 0; i < nmd; ++i) {
		uint8_t place[(COPIES + 1) * 8], content[(COPIES + 1) * 8];
		memset(place, 0, sizeof place);
		memset(content, 0, sizeof content);
		for (int c = 0; c < COPIES; ++c) {
			uint8_t *p = place + c * 8;
			p[0] = 1; p[1] = 0x20;
			p[2] = (uint8_t)md[i].x; p[3] = (uint8_t)(md[i].x >> 8);
			p[4] = (uint8_t)md[i].y; p[5] = (uint8_t)(md[i].y >> 8);
			p[6] = (uint8_t)md[i].z; p[7] = (uint8_t)(md[i].z >> 8);
			memcpy(content + c * 8, md[i].content, 8);
		}
		uint32_t pa = mapslot_alloc(place, (int)sizeof place), ca = mapslot_alloc(content, (int)sizeof content);
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
	if (slot) mystery_only(slot, md_at);
	return true;
}

bool mapslot_install(int group, int number, const NpcList *npcs, const MysteryData *md, int nmd) {
	bool rw = real_world(group);
	uint32_t g = rw ? (uint32_t)group : (uint32_t)(group - 0x80);
	/* sprites to decompress for the map: the layer's (the original's objects
	 * are gone, and their sprites would fill the buffer) */
	uint32_t sprites = sprite_table(group);
	if (sprites >= 0x08000000u && npcs) {
		uint8_t list[2 * MAPSLOT_SPRITES + 2];
		int n = 0;
		for (int i = 0; i < npcs->nsprites && i < MAPSLOT_SPRITES; ++i) {
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
	uint32_t npc_lists = emu_read32((rw ? RW_NPC_LISTS : BN6_NPC_LISTS) + g * 4);
	if (!npc_at || npc_lists < 0x08000000u) return false;
	emu_write32(npc_lists + (uint32_t)number * 4, npc_at);
	/* map scripts: none */
	static const uint8_t ms_end[4] = { 0 };
	uint32_t end_at = mapslot_alloc(ms_end, 4);
	for (int k = 0; k < 2; ++k) {
		uint32_t scripts = emu_read32((rw ? RW_MAP_SCRIPTS : BN6_MAP_SCRIPTS) + g * 8 + (uint32_t)k * 4);
		if (scripts >= 0x08000000u) emu_write32(scripts + (uint32_t)number * 4, end_at);
	}
	/* warps: the fixed list whose entry 1 the exit pad takes */
	uint32_t warps = warp_table(group);
	if (warps >= 0x08000000u) emu_write32(warps + (uint32_t)number * 4, WARP_LIST);
	/* objects: the town's (20-byte spawn records), else none */
	uint32_t objs = object_table(group);
	static const uint8_t no_objects[4] = { 0xFF };
	uint32_t obj_at = npcs && npcs->objects ? npcs->objects : mapslot_alloc(no_objects, 4);
	if (objs >= 0x08000000u) emu_write32(objs + (uint32_t)number * 4, obj_at);
	if (rw) return true;   /* no Mystery Data in the real world */
	return install_mystery(group, number, md, nmd);
}

/* the maps whose songs the lists hold: the town, the layer, and home's
 * (Lan's HP, his house and his room) */
enum { SONGS_MAPS = 2 + MAPSLOT_HOME_MAPS };
static struct { int group, number, song; } songs_of[SONGS_MAPS] = { { -1 }, { -1 }, { -1 }, { -1 }, { -1 } };

static bool music_list(void);

bool mapslot_music(int group, int number, int song) {
	if (number < 0 || number >= 16) return false;
	/* one entry per kind of map, real world and internet */
	int slot = real_world(group) ? 0 : 1;
	songs_of[slot].group = group;
	songs_of[slot].number = number;
	songs_of[slot].song = song;
	return music_list();
}

bool mapslot_music_home(int k, int group, int number, int song) {
	if (number < 0 || number >= 16 || k < 0 || k >= MAPSLOT_HOME_MAPS) return false;
	songs_of[2 + k].group = group;
	songs_of[2 + k].number = number;
	songs_of[2 + k].song = song;
	return music_list();
}

/* per map of each group its song, then a list of (group, songs): a group
 * once, with every map of it the lists hold (Lan's HP shares 0x88 with the
 * homepages' layers) */
static bool music_list(void) {
	uint8_t list[(SONGS_MAPS + 1) * 8];
	int n = 0;
	for (int i = 0; i < SONGS_MAPS; ++i) {
		bool before = false;
		for (int j = 0; j < i; ++j) before |= songs_of[j].group == songs_of[i].group;
		if (songs_of[i].group < 0 || before) continue;
		uint8_t songs[16];
		memset(songs, 0x63, sizeof songs);          /* 0x63: no song */
		for (int j = i; j < SONGS_MAPS; ++j)
			if (songs_of[j].group == songs_of[i].group) songs[songs_of[j].number] = (uint8_t)songs_of[j].song;
		uint32_t at = mapslot_alloc(songs, sizeof songs);
		if (!at) return false;
		memset(list + n * 8, 0, 8);
		list[n * 8] = (uint8_t)songs_of[i].group;
		put32(list + n * 8 + 4, at);
		++n;
	}
	memset(list + n * 8, 0, 8);
	list[n * 8] = 0xFF;
	uint32_t list_at = mapslot_alloc(list, (n + 1) * 8);
	if (!list_at) return false;
	for (int i = 0; i < BN6_MAP_MUSIC_LISTS; ++i) emu_write32(BN6_MAP_MUSIC + (uint32_t)i * 4, list_at);
	return true;
}

void mapslot_music_forget_town(void) { songs_of[0].group = -1; }

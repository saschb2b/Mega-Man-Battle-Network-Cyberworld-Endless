/* The random battles the game rolls on its internet maps (bn6f sub_80AA5F4):
 * four tables, the default and three that event flags 0x67F-0x681 choose
 * during story scenes (they differ only in Central and Seaside), each
 * holding for the real world and the internet a list per map group, per
 * map, of 16-byte BattleSettings ending with 0xFF. An area takes every
 * battle of its maps (RomLayout.net_area), listed once, weighted by how many
 * tables list it; not the rare battles (byte 7 = 1), which the game holds
 * back until a family has been deleted 16 or 32 times. */
#include "formations.h"

#include <stdlib.h>
#include <string.h>

#include "rom.h"
#include "run.h"

#define MAX_FORMATIONS 160

static Formation list[BIOME_COUNT][MAX_FORMATIONS];
static uint32_t origin[BIOME_COUNT][MAX_FORMATIONS];
static int count[BIOME_COUNT];
static bool loaded[BIOME_COUNT];

static void add(int biome, uint32_t rec) {
	for (int i = 0; i < count[biome]; ++i)
		if (origin[biome][i] == rec) { if (list[biome][i].weight < 255) ++list[biome][i].weight; return; }
	if (count[biome] >= MAX_FORMATIONS || R.data[rec + 7] == 1) return;
	uint32_t ents = rom_u32(rec + 12);
	if (!rom_is_ptr(ents)) return;
	Formation *f = &list[biome][count[biome]];
	memset(f, 0, sizeof *f);
	f->battlefield = R.data[rec];
	f->weight = 1;
	for (uint32_t a = ents & 0x1FFFFFF; R.data[a] != 0xF0 && f->n < FORMATION_MAX_ENTS; a += 4) {
		if (R.data[a] != 0x11) continue;   /* 0x00 is MegaMan */
		uint16_t id = rom_u16(a + 2);
		f->ent[f->n].panel = R.data[a + 1];
		f->ent[f->n++].id = id;
		if (R.data[R.layout->enemy_ids + id * 3 + 1] == 1) f->navi = true;
	}
	if (!f->n) return;
	origin[biome][count[biome]++] = rec;
}

static void load(int biome) {
	loaded[biome] = true;
	const __typeof__(R.layout->net_area[0]) *a = &R.layout->net_area[biome];
	if (!R.layout->encounters || !a->nmaps) return;
	for (int stage = 0; stage < 4; ++stage) {
		uint32_t internet = rom_u32(R.layout->encounters + (uint32_t)stage * 8 + 4);
		if (!rom_is_ptr(internet)) continue;
		uint32_t group = rom_u32((internet & 0x1FFFFFF) + (uint32_t)(a->battles - 0x80) * 4);
		if (!rom_is_ptr(group)) continue;
		for (int m = a->first; m < a->first + a->nmaps; ++m) {
			uint32_t recs = rom_u32((group & 0x1FFFFFF) + (uint32_t)m * 4);
			if (!rom_is_ptr(recs)) continue;
			for (uint32_t r = recs & 0x1FFFFFF; R.data[r] != 0xFF && r + 16 <= ROM_SIZE; r += 16) add(biome, r);
		}
	}
}

int formations_of(int biome, const Formation **out) {
	if (biome < 0 || biome >= BIOME_COUNT || !R.data) return 0;
	if (!loaded[biome]) load(biome);
	*out = list[biome];
	return count[biome];
}

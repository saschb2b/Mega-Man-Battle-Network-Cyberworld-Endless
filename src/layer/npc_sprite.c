/* npc_sprite.h: a sprite is a 4-byte header, then its animation table,
 * each animation a run of 20-byte frames (xnavi.c reads the same). */
#include "npc_sprite.h"

#include <stdint.h>

#include "emu.h"
#include "rom.h"

/* A sprite's animation table, after its 4-byte header: offsets from the
 * table, the first over four their count (xnavi.c reads the same); 0
 * where the list points at a compressed one or outside the ROM */
static uint32_t sprite_table(int category, int index) {
	if (!R.layout || category < 0 || category > 9 || index < 0 || index > 255) return 0;
	uint32_t list = emu_read32(0x08000000u + R.layout->sprite_lists + (uint32_t)category * 4);
	uint32_t ptr = emu_read32(list + (uint32_t)index * 4);
	return ptr & 0x80000000u || ptr < 0x08000000u || ptr >= 0x0A000000u ? 0 : ptr + 4;
}

int npc_sprite_anims(int category, int index) {
	uint32_t tab = sprite_table(category, index);
	uint32_t n = tab ? emu_read32(tab) / 4 : 0;
	return n && n <= 256 ? (int)n : -1;
}

bool npc_sprite_has(int category, int index, int anim) {
	int n = npc_sprite_anims(category, index);
	return n < 0 || (anim >= 0 && anim < n);
}

bool npc_sprite_shows(int category, int index, int anim) {
	int n = npc_sprite_anims(category, index);
	if (n < 0) return true;
	if (anim < 0 || anim >= n) return false;
	/* (its first frame: +0 its tiles, a length first, +8 its
	 * sub-animation, whose first entry's first byte names the object list,
	 * +12 its object table of offsets to the lists, 5 bytes an object,
	 * 0xFF 0xFF ending one. BN4's blank facings are one object of one
	 * tile, all of it colour 0) */
	uint32_t tab = sprite_table(category, index), f = tab + emu_read32(tab + 4u * (uint32_t)anim);
	uint32_t sub = tab + emu_read32(f + 8), objs = tab + emu_read32(f + 12);
	uint32_t list = emu_read8(sub + emu_read32(sub));
	uint32_t first = objs + emu_read32(objs + 4 * list);
	if (emu_read8(first) == 0xFF && emu_read8(first + 1) == 0xFF) return false;
	uint32_t tiles = tab + emu_read32(f), len = emu_read32(tiles);
	for (uint32_t i = 0; i < len && i < 0x2000; i += 4)
		if (emu_read32(tiles + 4 + i)) return true;
	return false;
}

int npc_sprite_stand(int category, int index, int anim) {
	static const uint8_t toward[] = { 3, 5, 4, 2 };
	/* (a Navi's alone: an object's first frame may be blank by design, the
	 * dark warp's flame drawn three frames in five) */
	if (category != 6 || npc_sprite_shows(category, index, anim)) return anim;
	for (unsigned k = 0; k < sizeof toward; ++k)
		if (npc_sprite_shows(category, index, toward[k])) return toward[k];
	return anim;
}

bool npc_sprite_turns(int category, int index) {
	if (category == 0 || category == 7) return false;
	for (int a = 1; a < 8; a += 2)
		if (!npc_sprite_shows(category, index, a)) return false;
	return true;
}


/* xdiff_sprite.h: a sprite by its bytes, read where the core has it. */
#include "xdiff_sprite.h"

#include <stdlib.h>

#include "hook.h"
#include "rom.h"
#include "xnavi.h"
#include "xvirus_recipe.h"

#define BUS      0x08000000u
#define READ_MAX 0x20000u   /* a sprite in the core's free space read to here at most (the largest battle sprite is under it) */

uint32_t xd_sprite_crc(int game, uint32_t list_offset, uint32_t number) {
	const uint8_t *rom = game == 6 ? R.data : XR[XROM_BN5_COLONEL_US].data;
	const XRomLayout *x = game == 6 ? NULL : XR[XROM_BN5_COLONEL_US].layout;
	uint32_t lists = game == 6 ? (R.layout ? R.layout->sprite_lists : 0) : (x ? x->sprite_lists : 0);
	if (!rom || !lists || list_offset > 0xFC || number > 0xFF) return 0;
	uint32_t base = hook_read32(BUS + lists + list_offset), p = base >= BUS ? hook_read32(base + 4u * number) : 0, a = p & 0x7FFFFFFFu;
	if (a < BUS) return 0;
	/* (compressed: in its game's ROM, as the player's file has it; else in the ROM, or a copy in the core's free space) */
	if (p & 0x80000000u) {
		if (a - BUS >= ROM_SIZE) return 0;
		size_t n = 0;
		uint8_t *d = lz77_decompress(rom + (a - BUS), ROM_SIZE - (a - BUS), &n);
		uint32_t len = d && n > 8 ? xnavi_sprite_len(d + 4, (uint32_t)n - 4) : 0, crc = len ? xvirus_crc32(d + 4, len) : 0;
		free(d);
		return crc;
	}
	if (a - BUS < ROM_SIZE) {
		uint32_t len = xnavi_sprite_len(rom + (a - BUS), ROM_SIZE - (a - BUS));
		return len ? xvirus_crc32(rom + (a - BUS), len) : 0;
	}
	static uint8_t buf[READ_MAX];
	for (uint32_t i = 0; i < READ_MAX; i += 4) {
		uint32_t w = hook_read32(a + i);
		buf[i] = (uint8_t)w, buf[i + 1] = (uint8_t)(w >> 8), buf[i + 2] = (uint8_t)(w >> 16), buf[i + 3] = (uint8_t)(w >> 24);
	}
	uint32_t len = xnavi_sprite_len(buf, READ_MAX);
	return len ? xvirus_crc32(buf, len) : 0;
}

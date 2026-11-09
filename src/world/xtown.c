/* Another game's town in BN6's real world (docs/world/bn5-town.md): its
 * map is copied whole as Seaside Town's is (town.c's style), in the map of
 * BN6's it takes over; here its tile set and colours, which BN6 loads from
 * that map's descriptor, are copied from the other game's ROM into a range
 * of their own, read at run time from the player's ROM. */
#include "xtown.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "debug.h"
#include "emu.h"

#define XTOWN_AT FREE_XTOWN
#define XTOWN_SIZE (FREE_XTOWN_END - FREE_XTOWN)
#define UI_FIRST 208   /* the interface's colours, 208-255, BN6's while the game runs (docs/world/overworld.md) */
#define PAL_HEAD 4     /* a palette's size word, then its colours */

/* the map whose descriptor a copy changed, and what it held */
static uint32_t held_desc, held_ts, held_pal;

bool xtown_gfx(uint32_t desc, int rom, int group, int number) {
	uint32_t at = 0x08000000u + desc;
	if (held_desc && (!rom || held_desc != desc)) {
		emu_write32(0x08000000u + held_desc, held_ts);
		emu_write32(0x08000000u + held_desc + 4, held_pal);
		held_desc = 0;
	}
	if (!rom) return true;
	held_desc = desc;
	held_ts = emu_read32(at);
	held_pal = emu_read32(at + 4);
	uint8_t *ts, *pal;
	size_t nts, npal;
	if (!area_src_gfx(rom, group, number, &ts, &nts, &pal, &npal)) return false;
	uint32_t pal_at = XTOWN_AT + (((uint32_t)nts + 3) & ~3u);
	bool ok = npal >= PAL_HEAD + 512 && pal_at + npal <= XTOWN_AT + XTOWN_SIZE;
	if (ok) {
		/* (the chat box, the PET's marks and the map's name in BN6's own) */
		for (int i = UI_FIRST * 2; i < 512; ++i) pal[PAL_HEAD + i] = emu_read8(held_pal + PAL_HEAD + (uint32_t)i);
		emu_write(XTOWN_AT, ts, nts);
		emu_write(pal_at, pal, npal);
		emu_write32(at, XTOWN_AT);
		emu_write32(at + 4, pal_at);
	}
	if (emu_debug_on()) fprintf(stderr, "xtown: %u bytes of tiles, %u of colours%s\n", (unsigned)nts, (unsigned)npal, ok ? "" : ": no room");
	free(ts);
	free(pal);
	return ok;
}

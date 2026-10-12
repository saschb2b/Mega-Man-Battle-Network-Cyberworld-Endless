/* BN5's Navis' battle sprites in BN6's list 2 (xnavi_sprites.h). */
#include "xnavi_sprites.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bn6.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"
#include "xnavi.h"
#include "xvirus_grow.h"

#define BUS 0x08000000u
#define BN5_LIST_NAVIS 2   /* BN5's list of its Navis' battle sprites, by AI index (its struct1's, as BN6's) */
#define NAVIS (XNAVI_SPRITE_LAST_AI - XNAVI_SPRITE_FIRST_AI + 1)
_Static_assert(FREE_NAVI_SPRITES + NAVIS * XNAVI_SPRITE_ROOM <= FREE_NAVI_SPRITES_END, "BN5's Navis' sprites past their room (free_space.h)");

int xnavi_sprite_number(int n) {
	return n >= XNAVI_SPRITE_FIRST_AI && n <= XNAVI_SPRITE_LAST_AI ? XNAVI_SPRITE_FIRST + (n - XNAVI_SPRITE_FIRST_AI) : -1;
}

/* the list-2 entry the engine reads for BN6's number k (the families' room's grown list 2, through the list table's word) */
static uint32_t entry_at(int k) {
	uint32_t list = emu_read32(BUS + R.layout->sprite_lists + 4u * BN6_SPRITES_NAVIS);
	return list + 4u * (uint32_t)k;
}

bool xnavi_sprite_install(int n) {
	int k = xnavi_sprite_number(n);
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	if (k < 0 || !R.data || !R.layout || !emu_ready() || !x->data || !x->layout) return false;
	uint32_t at = FREE_NAVI_SPRITES + (uint32_t)(n - XNAVI_SPRITE_FIRST_AI) * XNAVI_SPRITE_ROOM;
	/* (list 2 grown by the families' room: BN6's own ends at BN6_SPRITES_NAVIS_COUNT) */
	if (!xg_grow()) return false;
	uint32_t entry = entry_at(k), was = emu_read32(entry);
	if (was == at) return true;
	if (was != BN6_SPRITE_PLACEHOLDER) {
		fprintf(stderr, "xnavi_sprites: BN6's list-2 number %#x holds %08x, not the placeholder: BN5's Navi %d not listed\n", k, (unsigned)was, n);
		return false;
	}
	uint32_t len;
	uint8_t *d = xnavi_sprite_bytes(x->data, x->layout->sprite_lists, BN5_LIST_NAVIS, n, &len);
	bool fits = d && len <= XNAVI_SPRITE_ROOM;
	for (uint32_t i = 0; fits && i < len; i += 4) fits = emu_read32(at + i) == 0xFFFFFFFFu;
	if (!fits) {
		fprintf(stderr, "xnavi_sprites: BN5's Navi %d's battle sprite cannot be read, or its room is not free\n", n);
		free(d);
		return false;
	}
	claims_begin(XNAVI_SPRITE_CLAIMS);
	emu_write(at, d, len);
	claims_end();
	/* (the list's entry stays the families' room's: claims.h) */
	claims_begin(XV_CLAIMS);
	emu_write32(entry, at);
	claims_end();
	free(d);
	return true;
}

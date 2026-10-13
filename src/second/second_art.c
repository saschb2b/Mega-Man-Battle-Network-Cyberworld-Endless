/* second_art.h. A chip's record (RomLayout.chip_data): its icon's tiles at
 * BN6_CHIP_ICON_PTR in the icons' palette, its element and its rank; the
 * element icons in their own palette (docs/ROM_DATA.md, the second
 * screen's chip pictures). */
#include "second_art.h"

#include <stdbool.h>
#include <stddef.h>

#include "bn6.h"
#include "bytes.h"
#include "chips.h"
#include "gfx.h"
#include "rom.h"

#define ELEMENTS 11      /* Fire .. Null: BN6_CHIP_ELEMENT's values */
#define ICON_BYTES 128   /* 2 x 2 tiles of 4 bits a pixel */
#define ART_TILES 42     /* a card's picture: 7 x 6 tiles, uncompressed, in its record's palette */

/* the chip's record as the core's copy holds it (src/core/chips.h), NULL none */
static const uint8_t *record(int chip) { return chip > 0 ? chips_record(chip) : NULL; }

int second_chip_rank(int chip) {
	const uint8_t *r = record(chip);
	if (!r) return RANK_STANDARD;
	if (r[BN6_CHIP_DARK_ID] != 0xFF || (r[BN6_CHIP_EFFECT_FLAGS] & BN6_CHIP_DARK_CLASS)) return RANK_DARK;
	int t = r[BN6_CHIP_LIBRARY_TYPE];
	return t == 1 ? RANK_MEGA : t == 2 ? RANK_GIGA : RANK_STANDARD;
}

int second_chip_element(int chip) {
	const uint8_t *r = record(chip);
	return r && r[BN6_CHIP_ELEMENT] < ELEMENTS ? r[BN6_CHIP_ELEMENT] : -1;
}

void second_chip_icon(int chip, int x, int y) {
	const uint8_t *r = record(chip);
	if (!r) return;
	/* (its icon where the core's copy holds it: past the player's ROM for
	 * one of BN5's, which gfx.c's tiles read through chips_offset_bytes) */
	uint32_t at = get32(r + BN6_CHIP_ICON_PTR);
	if (!chips_bytes(at, ICON_BYTES)) return;
	rom_tiles(at - 0x08000000u, R.layout->chip_icon_pal, x, y, 2, 2, 0);
}

void second_chip_art(int chip, int x, int y, int scale) {
	const uint8_t *r = record(chip);
	if (!r) return;
	uint32_t art = get32(r + BN6_CHIP_IMAGE_PTR), pal = get32(r + BN6_CHIP_PALETTE_PTR);
	if (!chips_bytes(art, ART_TILES * 32) || !chips_bytes(pal, 32)) return;
	rom_tiles_scaled(art - 0x08000000u, pal - 0x08000000u, x, y, 7, 6, scale);
}

void second_element_icon(int element, int x, int y) {
	if (element < 0 || element >= ELEMENTS || !R.layout->element_icons) return;
	rom_tiles(R.layout->element_icons + (uint32_t)element * ICON_BYTES, R.layout->element_icon_pal, x, y, 2, 2, 0);
}

/* second_art.h. A chip's record (RomLayout.chip_data): its icon's tiles at
 * BN6_CHIP_ICON_PTR in the icons' palette, its element and its rank; the
 * element icons in their own palette (docs/ROM_DATA.md, the second
 * screen's chip pictures). */
#include "second_art.h"

#include <stdbool.h>
#include <stddef.h>

#include "bn6.h"
#include "bytes.h"
#include "gfx.h"
#include "rom.h"

#define ELEMENTS 11      /* Fire .. Null: BN6_CHIP_ELEMENT's values */
#define ICON_BYTES 128   /* 2 x 2 tiles of 4 bits a pixel */
#define ART_TILES 42     /* a card's picture: 7 x 6 tiles, uncompressed, in its record's palette */

/* the chip's record, NULL past the ROM */
static const uint8_t *record(int chip) {
	if (!R.layout) return NULL;
	size_t at = R.layout->chip_data + (size_t)chip * BN6_CHIP_RECORD_SIZE;
	return chip > 0 && chip < 512 && R.data && at + BN6_CHIP_RECORD_SIZE <= ROM_SIZE ? R.data + at : NULL;
}

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
	uint32_t at = get32(r + BN6_CHIP_ICON_PTR);
	if (at < 0x08000000u || at - 0x08000000u + ICON_BYTES > ROM_SIZE) return;
	rom_tiles(at - 0x08000000u, R.layout->chip_icon_pal, x, y, 2, 2, 0);
}

void second_chip_art(int chip, int x, int y, int scale) {
	const uint8_t *r = record(chip);
	if (!r) return;
	uint32_t art = get32(r + BN6_CHIP_IMAGE_PTR), pal = get32(r + BN6_CHIP_PALETTE_PTR);
	if (art < 0x08000000u || pal < 0x08000000u || art - 0x08000000u + ART_TILES * 32 > ROM_SIZE || pal - 0x08000000u + 32 > ROM_SIZE) return;
	rom_tiles_scaled(art - 0x08000000u, pal - 0x08000000u, x, y, 7, 6, scale);
}

void second_element_icon(int element, int x, int y) {
	if (element < 0 || element >= ELEMENTS || !R.layout->element_icons) return;
	rom_tiles(R.layout->element_icons + (uint32_t)element * ICON_BYTES, R.layout->element_icon_pal, x, y, 2, 2, 0);
}

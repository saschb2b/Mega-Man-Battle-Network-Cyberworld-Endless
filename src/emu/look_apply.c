/* look_apply.h. STUB: battle palette 0's armour and suit ramps, each
 * shade the colour picked at BN6's own shade's brightness. */
#include "look_apply.h"

#include "bn6.h"
#include "claims.h"
#include "emu.h"
#include "rom.h"

static uint16_t shade(uint16_t pick, uint16_t was) {
	int v = ((was & 31) + (was >> 5 & 31) + (was >> 10 & 31)) / 3 + 6;
	int r = (pick & 31) * v / 26, g = (pick >> 5 & 31) * v / 26, b = (pick >> 10 & 31) * v / 26;
	return (uint16_t)((r > 31 ? 31 : r) | (g > 31 ? 31 : g) << 5 | (b > 31 ? 31 : b) << 10);
}

void look_palette_of(const Look *look, int which, uint16_t out16[16]) {
	(void)which;
	uint32_t at = BN6_LOOK_BATTLE_PALS - 0x08000000u;
	for (int i = 0; i < 16; ++i) out16[i] = R.data ? (uint16_t)(R.data[at + 2 * i] | R.data[at + 2 * i + 1] << 8) : 0;
	static const int first[2] = { BN6_LOOK_ARMOUR_FIRST, BN6_LOOK_SUIT_FIRST }, last[2] = { BN6_LOOK_ARMOUR_LAST, BN6_LOOK_SUIT_LAST };
	for (int c = 0; c < 2; ++c) {
		if (look->palette[c] == LOOK_NONE) continue;
		uint16_t pick = looks_colour(look->palette[c], look->colour[c]);
		for (int i = first[c]; i <= last[c]; ++i) out16[i] = shade(pick, out16[i]);
	}
}

void look_apply(const Look *look) {
	uint16_t pal[16];
	look_palette_of(look, LOOK_ART_BATTLE, pal);
	claims_begin("MegaMan's look (stub)");
	emu_write(BN6_LOOK_BATTLE_PALS, pal, sizeof pal);
	claims_end();
}

/* Chip records (0x2C bytes, bn6f chip_data_struct): codes at 0, rarity 0-4
 * at 5, library type at 7 (0 standard, 1 Mega, 2 Giga, 3 secret, 4 Program
 * Advance), library number at 0x15 (0: not in the library) and library
 * flags at 0x16: bit 0 on the other version's Navi chips and Otenko, 0x30
 * on unused arm and dark chips, 0x10 alone on the Beast chips. The Gigas
 * (301-310) end the library; past Falzar (313) come pseudo-chips. */
#include "chip_pool.h"

#include "data.h"
#include "game.h"
#include "rom.h"

#define MAX_CHIPS 420
#define LAST_CHIP 313

static short pool[CHIP_TIERS][MAX_CHIPS];
static int count[CHIP_TIERS];
static bool loaded;

int chip_pool_tier(int id) {
	if (!R.data || id <= 0 || id > LAST_CHIP) return -1;
	const uint8_t *c = R.data + R.layout->chip_data + (uint32_t)id * 0x2C;
	if (c[0] > 26 || !c[0x15]) return -1;   /* no code, or not in the library */
	int flags = c[0x16];
	if (flags & 0x01 || flags & 0x20 || flags == 0x10) return -1;
	ChipInfo ci;
	chip_info(id, &ci);
	if (!ci.name[0]) return -1;             /* blank records */
	int rarity = c[5];
	switch (c[7]) {
	case 0: return rarity >= 3 ? 3 : rarity;
	case 1: return rarity >= 4 ? 4 : 3;
	case 2: return 4;
	default: return -1;
	}
}

static void load(void) {
	loaded = true;
	for (int id = 1; id < MAX_CHIPS; ++id) {
		int t = chip_pool_tier(id);
		if (t >= 0) pool[t][count[t]++] = (short)id;
	}
}

int chip_pool_pick(int tier) {
	if (!R.data) return -1;
	if (!loaded) load();
	/* the nearest tier that has chips */
	for (int d = 0; d < CHIP_TIERS; ++d)
		for (int s = -1; s <= 1; s += 2) {
			int t = tier + s * d;
			if (t >= 0 && t < CHIP_TIERS && count[t]) return pool[t][rng_range(0, count[t] - 1)];
		}
	return -1;
}

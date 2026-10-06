/* What a guest battle's results screen gives, as the run gets it
 * (guest_reward.h). Its game's reward words are BN6's encoding (bn5.h,
 * BN5_REWARD): its screen shows two at most, the busting level's from the
 * enemies' reward rows and the find of a green Mystery Data still on the
 * field (BN5_REWARD_FIND), and gives each as it ends; BN6 gets them through
 * its own GiveChips, GiveZenny and GiveBugfrags (director_guest.c). */
#include "guest_reward.h"

#include <stdio.h>

#include "bn5.h"
#include "data.h"
#include "debug.h"
#include "rom.h"
#include "xchips.h"

/* The zenny a reward's chip pays where BN6 has none of its name: the
 * results screen shows what the run gets (docs/MULTIROM.md, Guest
 * battles) */
#define REWARD_ZENNY 200

static uint16_t zenny_word(int zenny) { return (uint16_t)(BN5_REWARD_ZENNY << 14 | (zenny > 0x3FFF ? 0x3FFF : zenny)); }

/* (its ROM's own: the copy holds the last battle's) */
static uint16_t rom16(uint32_t a) {
	const uint8_t *d = XR[XROM_BN5_COLONEL_US].data + (a - 0x08000000u);
	return (uint16_t)(d[0] | d[1] << 8);
}

/* BN6's chip `id`'s four codes as its game's records hold theirs (0-25 A-Z,
 * 26 *, 0xFF none) */
static void bn6_codes(int id, uint8_t out[4]) {
	ChipInfo ci;
	chip_info(id, &ci);
	for (int i = 0; i < 4; ++i) out[i] = i >= ci.ncodes ? 0xFF : ci.codes[i] == '*' ? 26 : (uint8_t)(ci.codes[i] - 'A');
}

/* Chip entry v of row i of an enemy's rewards in one of the folder's codes
 * where both BN5's chip and BN6's of its name have it, on every other row
 * (xchips_fit, issue #63), so BN5's own results screen shows it as the run
 * gets it. */
static uint16_t row_coded(uint16_t v, uint32_t i, const uint8_t codes[3]) {
	uint32_t x = v & 0x1FF;
	uint8_t other[4], bn6[4];
	for (uint32_t k = 0; k < 4; ++k) other[k] = XR[XROM_BN5_COLONEL_US].data[BN5_CHIP_RECORDS + 0x2Cu * x + k];
	bn6_codes(guest_chip_bn6((int)x), bn6);
	return (uint16_t)(x | (uint32_t)xchips_fit(other, bn6, codes, v >> 9 & 0x1F, (int)i) << 9);
}

static bool is_chip(uint16_t v) { return v && v != 0xFFFF && !(v >> 14) && (v & 0x1FF) < BN5_CHIPS; }

/* Entry v of a reward table as the run gets it where it is a chip (row i
 * of an enemy's, -1 a Mystery Data's): zenny where BN6 has none of its
 * name, its game's own encoding (as BN6's rewards are); in * with All *
 * (its results screen shows the chip as the run gets it); else an enemy's
 * half the time in the folder's codes. Zenny, HP and BugFrags as they are. */
static uint16_t entry_fit(uint16_t v, int i, const GuestRowsFit *how) {
	if (!is_chip(v)) return v;
	if (!guest_chip_bn6(v & 0x1FF)) return zenny_word(REWARD_ZENNY);
	return how->star ? chip_entry_star(v) : i >= 0 ? row_coded(v, (uint32_t)i, how->codes) : v;
}

void guest_rows_fit(const int *ids, int n, const GuestRowsFit *how) {
	for (int k = 0; k < n; ++k) {
		/* (its enemies read a byte at a time, guest_record_foes: a list can
		 * start at an odd address, and a halfword read there takes the even
		 * one below; CanGard's MrkCan1 S showed on a playtester's results
		 * screen, came back as zenny, session 66; scaled as the battle is) */
		int id = ids[k];
		if (emu_debug_on()) fprintf(stderr, "guest: the reward rows of enemy %#x fitted\n", id);
		if (id <= 0 || id >= 0x200) continue;
		for (uint32_t i = 0; i < 20; ++i) {
			uint32_t a = BN5_REWARD_ROWS + (uint32_t)id * 0x28 + 2 * i;
			uint16_t v = rom16(a);
			/* (a guardian's chips as his zenny, as BN6's guardians' battles
			 * pay where their row holds their chip) */
			bool boss = id == how->boss_id && how->boss_zenny > 0 && is_chip(v);
			guest_write16(a, boss ? zenny_word(how->boss_zenny) : entry_fit(v, (int)i, how));
		}
	}
}

void guest_finds_fit(const GuestRowsFit *how) {
	int changed = 0;
	for (uint32_t i = 0; i < BN5_FIND_ROW_COUNT * 8; ++i) {
		uint16_t v = rom16(BN5_FIND_ROWS + 2 * i), w = entry_fit(v, -1, how);
		changed += w != v;
		guest_write16(BN5_FIND_ROWS + 2 * i, w);
	}
	if (emu_debug_on()) fprintf(stderr, "guest: the Mystery Data's finds fitted, %d of %d written otherwise\n", changed, BN5_FIND_ROW_COUNT * 8);
}

GuestReward guest_reward_of(uint16_t word) {
	GuestReward r = { .from = -1 };
	int kind = word >> 14, n = word & 0x3FFF, x = word & 0x1FF, code = word >> 9 & 0x1F;
	if (!word || word == 0xFFFF) return r;
	if (kind == BN5_REWARD_ZENNY) r.zenny = n;
	else if (kind == BN5_REWARD_HP) r.heal = n;
	else if (kind == BN5_REWARD_BUGFRAGS) r.bugfrags = n;
	else if (x < BN5_CHIPS) {
		r.chip = guest_chip_bn6(x);
		if (!r.chip) { r.zenny = REWARD_ZENNY; return r; }
		/* (in a code BN6's chip of its name has: the same letter, else its
		 * *, else its first; MegaMan says so the first time,
		 * director_guest.c) */
		uint8_t bn6[4];
		bn6_codes(r.chip, bn6);
		r.code = xchips_code(bn6, code);
		if (r.code != code) r.from = code;
	}
	return r;
}

/* BN6's Virus Battlers in a run: their series, stakes, pairs and prizes
 * (battler.h), read where BN6 keeps them (src/emu/bn6_battler.h). */
#include "battler.h"

#include <ctype.h>
#include <stdio.h>

#include "battler_words.h"
#include "bn6.h"
#include "data.h"
#include "debug.h"
#include "flags.h"
#include "rom.h"
#include "save.h"

/* Each Battler's first record in list 0 and his rounds (BN6_VB_FIRST_RECORD's
 * five series in a row), and his repeat prize in zenny (ts_check_give_zenny
 * in his archive): the stake to enter, and what a win adds to the pot */
static const struct { uint8_t first, rounds; uint16_t zenny; } series[BN6_VB_BATTLERS] = {
	{ 0x1A, 3, 500 }, { 0x1D, 5, 700 }, { 0x22, 5, 1500 }, { 0x27, 10, 2000 }, { 0x31, 10, 3000 },
};

static bool known(int k) { return k >= 1 && k <= BN6_VB_BATTLERS; }

int battler_rounds(int k) { return known(k) ? series[k - 1].rounds : 0; }

int battler_stake(int k) { return known(k) ? series[k - 1].zenny : 0; }

uint32_t battler_record(int k, int round) {
	if (!known(k)) return 0;
	if (round < 0) round = 0;
	if (round >= series[k - 1].rounds) round = series[k - 1].rounds - 1;
	return BN6_BATTLE_LIST0 + (uint32_t)(series[k - 1].first + round) * 16u;
}

/* (a record's +12: its entity list, four bytes an entry, 0x11 an enemy and
 * its id at +2, 0xF0 its end; the record's +3 its battle mode) */
bool battler_pair(int k, int round, char *out, size_t n) {
	if (n) *out = 0;
	uint32_t at = battler_record(k, round);
	if (!at || !R.data) return false;
	uint32_t rec = at - 0x08000000u;
	if (R.data[rec + 3] != BN6_MODE_VIRUS_BATTLER) return false;
	uint32_t list = rom_u32(rec + 12) - 0x08000000u;
	char names[2][24] = { "", "" };
	int found = 0;
	for (int e = 0; e < 8 && list + 4 * (uint32_t)e + 4 <= ROM_SIZE && found < 2; ++e) {
		uint32_t p = list + 4 * (uint32_t)e;
		if (R.data[p] == 0xF0) break;
		if (R.data[p] == 0x11) enemy_name(rom_u16(p + 2), names[found++], sizeof names[0]);
	}
	if (found < 2) return false;
	snprintf(out, n, "%s AND %s", names[0], names[1]);
	for (char *c = out; *c; ++c) *c = (char)toupper((unsigned char)*c);
	return true;
}

int battler_sprite(int k) {
	static const uint8_t sprite[BN6_VB_BATTLERS] = { BN6_VB_SPRITE_1, BN6_VB_SPRITE_2, BN6_VB_SPRITE_3, BN6_VB_SPRITE_4, BN6_VB_SPRITE_5 };
	return known(k) ? sprite[k - 1] : BN6_VB_SPRITE_1;
}

void battler_prize(int k, BattlerPrize *out) {
	*out = (BattlerPrize){ true, 0, 26, "" };
	switch (k) {
	case 1: out->id = BN6_VB_PRIZE_CHIP_1; break;
	case 2: *out = (BattlerPrize){ false, BN6_VB_PRIZE_PROGRAM_2, BN6_VB_PRIZE_COLOR_2, "" }; break;
	case 3: out->id = BN6_VB_PRIZE_CHIP_3; break;
	case 4: *out = (BattlerPrize){ false, BN6_VB_PRIZE_PROGRAM_4, BN6_VB_PRIZE_COLOR_4, "" }; break;
	case 5: out->id = BN6_VB_PRIZE_CHIP_5; out->code = BN6_VB_PRIZE_CODE_5; break;
	default: out->chip = false; return;
	}
	if (out->chip) {
		ChipInfo ci;
		chip_info(out->id, &ci);
		snprintf(out->name, sizeof out->name, "%s", ci.name);
	} else program_name(out->id >> 2, out->name, sizeof out->name);
}

int battler_offer(TextArchive *t, int flag, int k) {
	char pair[48];
	if (!battler_pair(k, 0, pair, sizeof pair)) snprintf(pair, sizeof pair, "MY VIRUSES");
	BattlerOffer b = { flag, battler_stake(k), battler_rounds(k), pair, (profile.btlr_card & BTLR_CARD_HELD) != 0, battler_sprite(k) };
	flag_clear(BATTLER_ON_FLAG);
	flag_clear(BATTLER_DONE_FLAG);
	flag_clear(BATTLER_CARD_FLAG);
	if (emu_debug_on()) fprintf(stderr, "battler: %d, %d rounds, stake %d, first pair %s\n", k, b.rounds, b.stake, pair);
	return ta_battler(t, &b);
}

/* The BtlrCard across runs (btlr_card.h): the profile's, given into each
 * run's game and read back from it. */
#include "btlr_card.h"

#include "battler.h"
#include "bn6.h"
#include "emu.h"
#include "flags.h"
#include "save.h"

/* The VIRUS LIST's families (BN6_CARD_FLAGS: AI 1-29 but 10, Trumpy, and
 * 13, Kettle, whose Rares BN6 registers nothing of), and Mettaur's and
 * Gunner's, the card's own */
#define AI_MOST 29
#define AI_METTAUR 1
#define AI_GUNNER 23
static bool on_card(int ai) { return ai >= 1 && ai <= AI_MOST && ai != 10 && ai != 13; }

bool card_held(void) { return (profile.btlr_card & BTLR_CARD_HELD) != 0; }

int card_families(void) {
	int n = 0;
	for (int ai = 1; ai <= AI_MOST; ++ai) n += on_card(ai) && (profile.btlr_data >> ai & 1);
	return n;
}

static void key_item_set(int id, int count) {
	uint32_t items = emu_read32(BN6_TOOLKIT + BN6_TOOLKIT_KEY_ITEMS);
	if (items >= BN6_EWRAM && items < BN6_EWRAM_END) emu_write8(items + (uint32_t)id, (uint8_t)count);
}

void card_to_game(void) {
	bool held = card_held();
	if (held) flag_set(BN6_FLAG_BTLRCARD);
	else flag_clear(BN6_FLAG_BTLRCARD);
	for (int ai = 1; ai <= AI_MOST; ++ai) {
		if (!on_card(ai)) continue;
		if (held && profile.btlr_data >> ai & 1) flag_set(BN6_FLAG_VIRUS_DATA + ai);
		else flag_clear(BN6_FLAG_VIRUS_DATA + ai);
	}
	key_item_set(BN6_ITEM_BTLRCARD, held ? 1 : 0);
}

void card_from_game(void) {
	bool changed = false;
	if (!card_held() && flag_get(BATTLER_CARD_FLAG)) {
		profile.btlr_card |= BTLR_CARD_HELD;
		profile.btlr_data |= 1u << AI_METTAUR | 1u << AI_GUNNER;
		changed = true;
	}
	if (card_held())
		for (int ai = 1; ai <= AI_MOST; ++ai)
			if (on_card(ai) && !(profile.btlr_data >> ai & 1) && flag_get(BN6_FLAG_VIRUS_DATA + ai)) {
				profile.btlr_data |= 1u << ai;
				changed = true;
			}
	if (changed) profile_save();
}

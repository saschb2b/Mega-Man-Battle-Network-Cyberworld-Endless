/* A Virus Battler's series on a layer (docs/LEVEL_DESIGN.md, The Virus
 * Battler): BN6's own rounds fought in BN6's battle mode 6, its VIRUS LIST
 * and its fight, the stake held as a pot that each win adds to; after a
 * win his word and the next pair, the pot taken or the next round; a loss
 * or a draw, the pot his. */
#include "director_battler.h"

#include <stdio.h>

#include "battler.h"
#include "battler_words.h"
#include "bn6.h"
#include "btlr_card.h"
#include "debug.h"
#include "director_state.h"
#include "emu.h"
#include "encounter.h"
#include "flags.h"
#include "run.h"
#include "talk.h"

static struct {
	int k;          /* the layer's Battler, 1-5, 0 none */
	int round;      /* the round under way or just fought, 0-based */
	int result;     /* the last round's BN6_RESULT_*, 0 none waiting to be said */
	bool known;     /* ... of the layer `seed` made, as MegaMan came onto it */
	uint32_t seed;
} B;

void battler_forget(void) { B.known = false; }

/* The layer's Battler, once MegaMan is on its map (a new one, or the
 * same one continued): none waiting yet; and the profile's card in the
 * game, whose state a CONTINUE has just put back */
static void battler_layer(void) {
	B.k = B.round = B.result = 0;
	D.battler_series = false;
	B.known = true;
	B.seed = run.layer_seed;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_BATTLER) B.k = layer.obj[i].param;
	card_to_game();
	if (emu_debug_on() && B.k)
		fprintf(stderr, "battler %d on the layer, card %d with %d families, first prizes %d%d%d%d%d\n", B.k, card_held(), card_families(),
			flag_get(BN6_FLAG_VB_FIRST_PRIZE), flag_get(BN6_FLAG_VB_FIRST_PRIZE + 1), flag_get(BN6_FLAG_VB_FIRST_PRIZE + 2),
			flag_get(BN6_FLAG_VB_FIRST_PRIZE + 3), flag_get(BN6_FLAG_VB_FIRST_PRIZE + 4));
}

static void fight(void) {
	emu_battle_force_record(battler_record(B.k, B.round));
	D.challenge = D.battler = true;
	if (emu_debug_on()) fprintf(stderr, "battler %d: round %d, record %08x\n", B.k, B.round + 1, battler_record(B.k, B.round));
}

void battler_begin(void) {
	if (!B.k) return;
	B.round = 0;
	D.battler_series = true;
	fight();
}

void battler_round_over(int result) {
	D.battler = false;
	B.result = result ? result : BN6_RESULT_LOST;
	if (emu_debug_on()) fprintf(stderr, "battler %d: round %d result %d\n", B.k, B.round + 1, result);
}

void battler_card_note(void) { card_from_game(); }

/* His word on the round just fought: the pot as it stands (the stake and
 * a stake a win), the next pair, the first prize on a run's first series
 * won against him (BN6's own flag for it, clear in every run's game) */
static bool say_result(void) {
	static TextArchive t;
	int stake = battler_stake(B.k), rounds = battler_rounds(B.k), won = B.result == BN6_RESULT_WON;
	char next[48] = "";
	if (won && B.round + 1 < rounds) battler_pair(B.k, B.round + 1, next, sizeof next);
	BattlerPrize prize;
	battler_prize(B.k, &prize);
	int prize_flag = BN6_FLAG_VB_FIRST_PRIZE + B.k - 1;
	BattlerAfter a = { B.result, B.round + 1, rounds, stake * (B.round + 1 + won), next, flag_get(prize_flag) ? NULL : &prize, prize_flag,
		battler_sprite(B.k) };
	ta_begin(&t);
	int script = ta_battler_after(&t, &a);
	return talk_archive(&t, script);
}

void battler_update(void) {
	if (!B.known || B.seed != run.layer_seed) battler_layer();
	if (!card_held() && flag_get(BATTLER_CARD_FLAG)) card_from_game();
	if (!D.battler_series || D.battler || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	if (B.result) {
		if (say_result()) B.result = 0;
		return;
	}
	if (flag_get(BATTLER_ON_FLAG)) {
		flag_clear(BATTLER_ON_FLAG);
		++B.round;
		fight();
	} else if (flag_get(BATTLER_DONE_FLAG)) D.battler_series = false;
}

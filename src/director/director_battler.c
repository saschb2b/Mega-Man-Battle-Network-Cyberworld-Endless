/* A Virus Battler's series on a layer (docs/world/level-design/virus-battler.md, The Virus
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
	bool won;       /* the last round won (a series ended on it: the pot taken) */
	int last;       /* the last round's result, kept for a state's words (0: none fought on this layer) */
	uint32_t seed;
} B;

void battler_forget(void) { B.known = false; }

/* The layer's Battler, once MegaMan is on its map (a new one, or the
 * same one continued): none waiting yet; and the profile's card in the
 * game, whose state a CONTINUE has just put back */
static void battler_layer(void) {
	B.k = B.round = B.result = B.last = 0;
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
	runlog_choice(CHOICE_BATTLER);   /* (the run's choices: a series begun) */
	fight();
}

void battler_round_over(int result) {
	D.battler = false;
	B.result = result ? result : BN6_RESULT_LOST;
	B.won = B.result == BN6_RESULT_WON;
	B.last = B.result;
	if (emu_debug_on()) fprintf(stderr, "battler %d: round %d result %d\n", B.k, B.round + 1, result);
}

void battler_card_note(void) { card_from_game(); }

/* (a round's battle: BN6's VIRUS LIST, the picks placed, its question, the fight) */
static void round_describe(FILE *f) {
	static const struct { int phase; const char *word; } lists[] = { { 4, "list" }, { 8, "ok" }, { 0x0C, "placing" }, { 0x14, "confirm" } };
	int phase = emu_read8(BN6_BATTLE_PHASE), list = emu_read8(BN6_VB_LIST_PHASE);
	const char *screen = phase == 0x0C ? "fight" : "other";
	for (unsigned i = 0; i < sizeof lists / sizeof *lists && phase == 8; ++i)
		if (lists[i].phase == list) screen = lists[i].word;
	fprintf(f, "vbscreen %s\nvbpicked %d\nvbplaced %d\n", screen,
		(emu_read8(BN6_VB_LIST_PICK1) != 0xFF) + (emu_read8(BN6_VB_LIST_PICK2) != 0xFF), emu_read8(BN6_VB_LIST_PLACED));
}

/* (a result in a word) */
static const char *result_word(int result) {
	return !result ? "none" : result == BN6_RESULT_WON ? "won" : result == BN6_RESULT_LOST ? "lost" : "draw";
}

void battler_describe(FILE *f) {
	fprintf(f, "btlrcard %s\nvirusdata %d\n", card_held() ? "held" : "none", card_families());
	if (!B.known || !B.k) { fprintf(f, "battler none\n"); return; }
	bool on = D.battler_series, round = D.battler && !on_map();
	int stake = battler_stake(B.k), wins = on ? B.round + (!D.battler && B.won) : 0;
	const char *series = on ? "on" : flag_get(BATTLER_DONE_FLAG) ? "over" : "none";
	fprintf(f, "battler %d\nstake %d\nvbrounds %d\nseries %s\nvbround %d\nvbwins %d\npot %d\nvblast %s\nvbprize %s\n", B.k, stake,
		battler_rounds(B.k), series, on ? B.round + 1 : 0, wins, on ? stake * (wins + 1) : 0, result_word(B.last),
		flag_get(BN6_FLAG_VB_FIRST_PRIZE + B.k - 1) ? "taken" : "open");
	/* (his word after a win: TAKE THE POT or FIGHT ON showing) */
	if (on && !round) fprintf(f, "potchoice %s\n", emu_read8(BN6_CHATBOX) && emu_read8(BN6_CHATBOX_OPTIONS) >= 2 ? "up" : "none");
	if (round) round_describe(f);
}

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
	if (!card_held() && flag_get(BATTLER_TOLD_FLAG)) card_from_game();
	if (!D.battler_series || D.battler || talk_busy() || emu_read8(BN6_CHATBOX)) return;
	if (B.result) {
		if (say_result()) B.result = 0;
		return;
	}
	if (flag_get(BATTLER_ON_FLAG)) {
		flag_clear(BATTLER_ON_FLAG);
		++B.round;
		runlog_choice(CHOICE_BATTLER_ON);   /* (the run's choices: on after a win) */
		fight();
	} else if (flag_get(BATTLER_DONE_FLAG)) {
		D.battler_series = false;
		if (B.won) runlog_choice(CHOICE_BATTLER_CASH);   /* (... or the pot taken) */
	}
}

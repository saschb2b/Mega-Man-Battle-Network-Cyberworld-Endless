/* A battle in the state's words (director_describe's), which
 * tests/scenarios reads (docs/devtools/scenarios.md, Battles and the
 * run's spine in the state): what kind it is, the phase it stands in, how
 * it ended and what its RESULT paid, the form MegaMan fights in, and the
 * statuses its enemies are held in. On the map after one, how the last
 * ended. Reading changes nothing. */
#include "director_state.h"

#include <stdio.h>

#include "bn6.h"
#include "bn6_battler.h"
#include "boss.h"
#include "chip_entry.h"
#include "emu.h"
#include "glitch.h"

/* a guardian's, the rival's, a Navi gate's SP, the Glitch layer's copy, a
 * Server's, the one a layer owed at its exit, or the layer's own */
static const char *kind(void) {
	return boss_fighting() ? "guardian" : D.duel ? "duel" : D.gate_fight ? "gate" : glitch_copy_fighting() ? "copy"
		: D.challenge ? "server" : D.floor_fight ? "floor" : "random";
}

static const char *result(void) {
	int r = emu_read8(BN6_BATTLE_RESULT);
	return r == BN6_RESULT_WON ? "won" : r == BN6_RESULT_LOST ? "lost" : r == BN6_RESULT_RAN ? "ran" : r == BN6_RESULT_DRAW ? "draw" : "none";
}

/* none, a Cross (Gregar's five), Beast Out, alone or in a Cross, BeastOver */
static const char *form(void) {
	static const char *const cross[6] = { "none", "HeatCross", "ElecCross", "SlashCross", "EraseCross", "ChargeCross" };
	int v = emu_read8(BN6_BATTLE_FORM);
	return v <= 5 ? cross[v] : v == BN6_FORM_BEAST ? "BeastOut" : v >= BN6_FORM_BEAST_OVER ? "BeastOver"
		: v >= BN6_FORM_CROSS_BEAST ? "CrossBeast" : "other";
}

/* The statuses the enemies in play are held in, as BN6 acts on a hit's
 * (docs/rom/battle.md, Statuses as a hit leaves them): CurAction 4
 * paralysed, 5 dragged, 6 frozen, 7 bubbled; each once */
static void statuses(FILE *f) {
	static const char *const names[4] = { "paralysed", "dragged", "frozen", "bubbled" };
	unsigned seen = 0;
	for (uint32_t k = 0; k < BN6_T1_COUNT; ++k) {
		uint32_t o = BN6_T1_OBJECTS + k * BN6_T1_SIZE;
		int action = emu_read8(o + BN6_T1_ACTION);
		if ((emu_read8(o + BN6_T1_IN_PLAY) & 1) && emu_read8(o + BN6_T1_ALLIANCE) && action >= 4 && action <= 7) seen |= 1u << (action - 4);
	}
	fprintf(f, "foestatus");
	for (int i = 0; i < 4; ++i)
		if (seen >> i & 1) fprintf(f, " %s", names[i]);
	fprintf(f, "%s\n", seen ? "" : " none");
}

/* what the RESULT gives: a chip by its id and code, zenny, HP or BugFrags
 * by their amount (chip_entry.h's reward) */
static void reward(FILE *f) {
	static const char *const kinds[4] = { "chip", "zenny", "hp", "bugfrags" };
	uint16_t r = emu_read16(BN6_BATTLE_REWARD);
	if (r == REWARD_NONE) fprintf(f, "reward none\n");
	else if (reward_is_chip(r)) fprintf(f, "reward chip %u %c\n", reward_chip_id(r), reward_chip_code(r) >= 26 ? '*' : (int)('A' + reward_chip_code(r)));
	else fprintf(f, "reward %s %u\n", kinds[reward_kind(r)], reward_amount(r));
}

void battle_state_describe(FILE *f) {
	if (D.battler || D.lab) return;
	if (main_mode() != BN6_MODE_GAME || emu_read8(BN6_GAMESTATE) != BN6_SUB_BATTLE) {
		/* (on the map: the battle before, once one was fought) */
		if (on_map() && emu_read8(BN6_BATTLE_RESULT)) fprintf(f, "lastbattle %s\n", result());
		return;
	}
	int phase = emu_read8(BN6_BATTLE_PHASE);
	fprintf(f, "battle %s\nphase %s\nform %s\n", kind(),
		phase == BN6_PHASE_CUSTOM ? "custom" : phase == BN6_PHASE_FIGHT ? "fight" : phase == BN6_PHASE_RESULTS ? "results" : "other", form());
	statuses(f);
	if (phase != BN6_PHASE_RESULTS) return;
	fprintf(f, "result %s\n", result());
	if (emu_read8(BN6_BATTLE_RESULT) == BN6_RESULT_WON) reward(f);
}

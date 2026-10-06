/* jobs.h. The visit's three requests and a taken one's course. Each asker
 * posts kinds that fit them (the board's NetBattler a chip of an element
 * or a vow, the club a busting style, the lab its readings: every Mystery
 * Data of a layer, or clean wins), and each kind pays by what it costs
 * (docs/HOME.md, piece 5): the effort of a busting style or a layer
 * searched through, zenny or BugFrags about half a layer's worth; a chip
 * given up, a chip a tier up in the folder's codes; a vow's risk, two
 * HPMemory. */
#include "jobs.h"

#include <string.h>

#include "data.h"
#include "game.h"
#include "loot.h"
#include "pacing.h"

int jobs_act(int depth) { return pacing_act(depth) + 7 * pacing_loop(depth); }

/* the two kinds each asker posts */
static const uint8_t kinds_of[JOB_ASKERS][2] = {
	[JOB_BOARD] = { JOB_BRING, JOB_VOW },
	[JOB_CLUB] = { JOB_QUICK, JOB_CLEAN },
	[JOB_LAB] = { JOB_EXPLORE, JOB_CLEAN },
};

/* what `j` asks and pays, for act `a` beginning on layer `depth` */
static void terms(Job *j, int a, int depth) {
	switch (j->kind) {
	case JOB_CLEAN:
		j->need = (uint8_t)(a >= 2 ? 3 : 2);
		j->pay_kind = PAY_ZENNY;
		j->pay = (uint16_t)(500 + 250 * a);
		break;
	case JOB_QUICK:
		j->need = 3;
		j->pay_kind = PAY_BUGFRAGS;
		j->pay = (uint16_t)(5 + 2 * a);
		break;
	case JOB_EXPLORE:
		j->pay_kind = PAY_ZENNY;
		j->pay = (uint16_t)(500 + 250 * a);
		break;
	case JOB_BRING: {
		static const uint8_t elements[4] = { ELEM_FIRE, ELEM_AQUA, ELEM_ELEC, ELEM_WOOD };
		j->need = elements[rng_range(0, 3)];
		char code;
		int id = roll_chip(depth, 1, &code);
		code = loot_fit_code(id, code, true);
		j->pay_kind = PAY_CHIP;
		j->pay = (uint16_t)id;
		j->code = (uint8_t)(code == '*' ? 26 : code - 'A');
		break;
	}
	case JOB_VOW:
		/* (two: a kept vow forgoes the act's patches, two or three halves
		 * of MegaMan's HP, and one HPMemory, below a dealer's 800 zenny for
		 * one, read as no reward for the risk, session 69) */
		j->pay_kind = PAY_HPMEMORY;
		j->pay = 2;
		break;
	default:
		break;
	}
}

void jobs_offers(uint32_t seed, int depth, Job out[JOB_ASKERS]) {
	uint32_t was = rng_state();
	rng_seed(seed ^ 0x4A4F4253u ^ (uint32_t)depth * 0x9E3779B9u);
	int a = jobs_act(depth);
	memset(out, 0, sizeof *out * JOB_ASKERS);
	for (int k = 0; k < JOB_ASKERS; ++k) {
		Job *j = &out[k];
		j->asker = (uint8_t)k;
		j->kind = kinds_of[k][rng_range(0, 1)];
		/* (a vow from the second visit on: at a run's first, Lan's HP and
		 * the act's ways are still unseen, and a playtester bet act 1's
		 * patches on a net whose battles took his Cross, session 69) */
		if (j->kind == JOB_VOW && depth <= 1) j->kind = JOB_BRING;
		/* (none twice: the club's and the lab's clean wins) */
		for (int i = 0; i < k; ++i)
			if (out[i].kind == j->kind) j->kind = kinds_of[k][kinds_of[k][0] == j->kind];
		j->act = (uint8_t)a;
		j->depth = (uint16_t)depth;
		terms(j, a, depth);
	}
	rng_restore(was);
}

bool jobs_due(const Job *j, int depth) { return j->kind != JOB_NONE && j->state && jobs_act(depth) > j->act; }

bool jobs_vow_holds(const Job *j, int depth) { return j->kind == JOB_VOW && j->state == JOB_TAKEN && !jobs_due(j, depth); }

bool jobs_battle(Job *j, bool won, int hp_lost, int frames) {
	if (j->state != JOB_TAKEN || !won) return false;
	bool counts = (j->kind == JOB_CLEAN && hp_lost <= 0) || (j->kind == JOB_QUICK && frames <= JOB_QUICK_FRAMES);
	if (!counts) return false;
	if (++j->got < j->need) return false;
	j->state = JOB_DONE;
	return true;
}

bool jobs_layer_left(Job *j, int nmd, int opened) {
	if (j->state != JOB_TAKEN || j->kind != JOB_EXPLORE || nmd < JOB_EXPLORE_LEAST || opened < nmd) return false;
	j->state = JOB_DONE;
	return true;
}

bool jobs_heal(Job *j) {
	if (j->state != JOB_TAKEN || j->kind != JOB_VOW) return false;
	j->state = JOB_FAILED;
	return true;
}

bool jobs_guardian(Job *j) {
	if (j->state != JOB_TAKEN || j->kind != JOB_VOW) return false;
	j->state = JOB_DONE;
	return true;
}

void jobs_dev(Job *j, uint32_t seed, int depth, bool home, int kind, int state) {
	if (kind <= JOB_NONE || kind >= JOB_KINDS) return;
	int at = home && depth > 3 ? depth - 3 : depth, asker = 0;
	for (int k = 0; k < JOB_ASKERS; ++k)
		if (kinds_of[k][0] == kind || kinds_of[k][1] == kind) { asker = k; break; }
	Job offers[JOB_ASKERS];
	jobs_offers(seed, at, offers);
	*j = offers[asker];
	j->kind = (uint8_t)kind;
	uint32_t was = rng_state();
	terms(j, j->act, at);
	rng_restore(was);
	j->state = (uint8_t)(state > 0 ? state : JOB_TAKEN);
	j->got = j->state == JOB_DONE ? j->need : 0;
}

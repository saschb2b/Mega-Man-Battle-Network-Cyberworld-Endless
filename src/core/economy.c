/* The run's economy, measured and planned (economy.h, docs/game/economy/). */
#include "economy.h"

#include <math.h>

#include "game.h"
#include "pacing.h"
#include "run.h"

/* The flat plan, every lever at 1: the rolls as they were before issue
 * #154 (the economy report's "before"; --economy-plan flat) */
const EconPlan econ_flat = { "flat", {
	[ECON_LEVER_MD] = { 1, 2 }, [ECON_LEVER_CHIPS] = { 1, 2 }, [ECON_LEVER_GOOD] = { 1, 2 },
	[ECON_LEVER_ZENNY] = { 1, 2 }, [ECON_LEVER_LEAN] = { 1, 2 },
} };
/* The opening plan, in play (the owner's yes of 21 October 2026,
 * docs/game/economy/): the first act's green Mystery Data at half, their
 * chips kept at 60%, the good chips' weight in its plain finds at a
 * quarter, its zenny at half and none of its finds leaned to the folder's
 * codes, each opening on the same bend to the flat plan's by the net's
 * last act */
const EconPlan econ_opening = { "opening", {
	[ECON_LEVER_MD] = { 0.5, 2 }, [ECON_LEVER_CHIPS] = { 0.6, 2 }, [ECON_LEVER_GOOD] = { 0.25, 2 },
	[ECON_LEVER_ZENNY] = { 0.5, 2 }, [ECON_LEVER_LEAN] = { 0, 2 },
} };
const EconPlan *econ_plan = &econ_opening;

double econ_curve(EconCurve c, double x) {
	if (c.floor >= 1) return 1;
	if (x <= 0) return c.floor;
	if (x >= 1) return 1;
	double open = c.bend > 0.001 ? log(1 + c.bend * x) / log(1 + c.bend) : x;
	return c.floor + (1 - c.floor) * open;
}

double econ_x(int depth) {
	if (pacing_loop(depth) > 0) return 1;
	int last = run.mode == RUN_SHORT ? 2 : 5, act = pacing_act(depth);
	return act >= last ? 1 : (double)act / last;
}

double econ_factor(int lever, int depth) {
	if (lever < 0 || lever >= ECON_LEVERS) return 1;
	return econ_curve(econ_plan->lever[lever], econ_x(depth));
}

int econ_scale(int n, int lever, int depth) {
	double f = econ_factor(lever, depth);
	return f >= 1 ? n : (int)(n * f + 0.5);
}

bool econ_keep(int lever, int depth) {
	double f = econ_factor(lever, depth);
	return f >= 1 || rng_range(0, 999) < (int)(f * 1000);
}

bool econ_keep_by(int lever, int depth, uint32_t hash) {
	double f = econ_factor(lever, depth);
	hash = (hash ^ hash >> 16) * 0x45D9F3Bu;
	hash ^= hash >> 16;
	return f >= 1 || (int)(hash % 1000u) < (int)(f * 1000);
}

/* Each plan's numbers as the economy report measures them (300 runs of
 * the endless net: the tables in docs/game/economy/), each band a fifth either
 * side of its centre: a change to the drops, the Mystery Data, the
 * dealers or the report's assumptions that moves an act past it is
 * flagged, as the pacing report flags a battle past its band. Per act
 * (the sixth's and the Nest's last): chips gained, good chips' share in
 * percent, Mystery Data laid, zenny earned, codes held at its end (set
 * again when the lean kept the chip and leaned only its code,
 * docs/game/economy/opening-plan.md, The code leans). */
#define BAND_SLACK 0.2
/* (acts 3 to 5's chips and zenny with the tournament's rounds, a profile
 * past the Nest's: its two results screens and two Navi chips an act a
 * bracket stands in, the short net's act 3 in half the runs and one of
 * the endless net's acts 3 to 5 each cycle; docs/game/tournament.md, As
 * built) */
static const double centre[2][ECON_METRICS][ECON_ACTS] = {
	{   /* the flat plan's */
		[ECON_CHIPS] = { 12.5, 10.9, 15.2, 14.2, 17.8, 15.6, 5.5 },
		[ECON_GOOD] = { 24.6, 31.5, 39.3, 44.3, 59.9, 69.3, 68.4 },
		[ECON_MD] = { 15.1, 15.0, 20.2, 19.2, 24.3, 22.4, 7.6 },
		[ECON_ZENNY] = { 3690, 4620, 8250, 9080, 12430, 10510, 5330 },
		[ECON_CODES] = { 5, 7, 11, 15, 18, 22, 23 },
	},
	{   /* the opening plan's */
		[ECON_CHIPS] = { 9.4, 9.0, 13.0, 13.3, 17.2, 15.7, 5.6 },
		[ECON_GOOD] = { 22.9, 30.3, 35.6, 41.7, 58.3, 69.9, 68.7 },
		[ECON_MD] = { 11.7, 12.6, 18.0, 17.9, 24.3, 22.4, 7.6 },
		[ECON_ZENNY] = { 2870, 3840, 7140, 8140, 12240, 10460, 5290 },
		[ECON_CODES] = { 3, 5, 8, 11, 15, 18, 19 },
	},
};

/* ... and the short net's (a new run's): its three acts and its Nest */
static const double centre_short[2][ECON_METRICS][4] = {
	{   /* the flat plan's */
		[ECON_CHIPS] = { 12.6, 10.9, 15.4, 4.8 },
		[ECON_GOOD] = { 24.5, 31.5, 39.9, 43.9 },
		[ECON_MD] = { 15.1, 15.0, 20.2, 6.6 },
		[ECON_ZENNY] = { 3650, 4620, 8530, 3970 },
		[ECON_CODES] = { 5, 7, 11, 12 },
	},
	{   /* the opening plan's */
		[ECON_CHIPS] = { 9.6, 9.7, 15.4, 4.7 },
		[ECON_GOOD] = { 23.0, 30.8, 39.5, 43.1 },
		[ECON_MD] = { 11.7, 13.4, 20.2, 6.6 },
		[ECON_ZENNY] = { 2720, 4070, 8500, 4020 },
		[ECON_CODES] = { 3, 5, 9, 11 },
	},
};

EconBand econ_band(const EconPlan *plan, bool short_net, int metric, int act) {
	if (metric < 0 || metric >= ECON_METRICS || act < 0 || act >= ECON_ACTS) return (EconBand){ 0, 0 };
	int p = plan == &econ_opening;
	if (short_net && act > 2 && act < ECON_ACTS - 1) return (EconBand){ 0, 0 };
	double v = short_net ? centre_short[p][metric][act > 2 ? 3 : act] : centre[p][metric][act];
	return (EconBand){ v * (1 - BAND_SLACK), v * (1 + BAND_SLACK) };
}

int econ_bust_weight(int bust) {
	/* (a fair player's: mostly 7 to S, some slower or hit; docs/game/economy/
	 * sets it beside the autopilot's and BN6's own table) */
	static const int weight[ECON_BUST_S + 1] = { 0, 0, 0, 0, 5, 5, 10, 15, 15, 15, 15, 20 };
	return bust >= 1 && bust <= ECON_BUST_S ? weight[bust] : 0;
}

int econ_battles_roll(void) { return rng_range(ECON_BATTLES_LEAST, ECON_BATTLES_MOST); }

bool econ_chip_good(int tier) { return tier >= ECON_GOOD_TIER; }

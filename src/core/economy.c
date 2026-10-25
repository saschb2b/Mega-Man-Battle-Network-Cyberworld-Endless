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
 * percent, Mystery Data laid, zenny earned, codes held at its end. */
#define BAND_SLACK 0.2
static const double centre[2][ECON_METRICS][ECON_ACTS] = {
	{   /* the flat plan's */
		[ECON_CHIPS] = { 12.2, 11.4, 14.7, 13.9, 17.8, 15.8, 5.4 },
		[ECON_GOOD] = { 26.3, 31.3, 38.3, 45.8, 57.0, 70.1, 69.0 },
		[ECON_MD] = { 14.8, 15.1, 20.3, 19.5, 24.7, 22.6, 7.6 },
		[ECON_ZENNY] = { 3550, 4350, 6880, 7900, 10890, 10760, 5360 },
		[ECON_CODES] = { 5, 8, 11, 15, 20, 24, 26 },
	},
	{   /* the opening plan's */
		[ECON_CHIPS] = { 9.5, 9.4, 12.5, 12.9, 17.1, 15.8, 5.4 },
		[ECON_GOOD] = { 23.4, 29.0, 34.3, 43.8, 56.9, 70.1, 69.5 },
		[ECON_MD] = { 11.6, 12.6, 17.9, 18.2, 24.7, 22.6, 7.6 },
		[ECON_ZENNY] = { 2630, 3610, 5880, 7100, 10740, 10700, 5340 },
		[ECON_CODES] = { 3, 5, 8, 11, 15, 19, 21 },
	},
};

/* ... and the short net's (a new run's): its three acts and its Nest */
static const double centre_short[2][ECON_METRICS][4] = {
	{   /* the flat plan's */
		[ECON_CHIPS] = { 12.2, 11.4, 14.7, 4.6 },
		[ECON_GOOD] = { 26.4, 31.2, 38.3, 41.7 },
		[ECON_MD] = { 14.8, 15.1, 20.3, 6.6 },
		[ECON_ZENNY] = { 3550, 4360, 6880, 4130 },
		[ECON_CODES] = { 5, 8, 12, 13 },
	},
	{   /* the opening plan's */
		[ECON_CHIPS] = { 9.6, 10.0, 14.7, 4.6 },
		[ECON_GOOD] = { 23.3, 30.1, 38.1, 41.9 },
		[ECON_MD] = { 11.6, 13.5, 20.3, 6.6 },
		[ECON_ZENNY] = { 2630, 4000, 6860, 4150 },
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

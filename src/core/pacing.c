/* The run's difficulty curve (docs/PROGRESSION.md). The numbers follow BN6's
 * own: the HP of one random battle in each area in story order (Central
 * about 150, Seaside and Green 200-300, Sky 350, the Undernet 400, the
 * Graveyard 550), measured against the max HP MegaMan is meant to have in
 * each act (100 at the start, about 100 more per guardian). */
#include "pacing.h"

#include "game.h"
#include "run.h"

int pacing_act(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	return p >= 18 ? 6 : p / 3;
}

int pacing_loop(int depth) { return (depth - 1) / CYCLE_LAYERS; }

/* per act: HP aimed for, HP at most, the strongest virus's damage at most
 * (about 40% of MegaMan's HP then; in the first act, with the starting
 * folder, a third less: a playtest lost its first battle to two 20s) */
static const PacingBand bands[PACING_ACTS] = {
	{ 90, 160, 30 }, { 150, 280, 80 }, { 200, 360, 120 }, { 280, 420, 160 },
	{ 350, 500, 200 }, { 400, 580, 240 }, { 450, 650, 280 },
};

static PacingBand scaled(PacingBand b, int loop) {
	/* a quarter more on each later cycle */
	b.lo = b.lo * (4 + loop) / 4;
	b.hi = b.hi * (4 + loop) / 4;
	b.cap = b.cap * (4 + loop) / 4;
	return b;
}

PacingBand pacing_band(int depth, bool challenge, bool easy) {
	int act = pacing_act(depth), loop = pacing_loop(depth);
	PacingBand b = bands[act];
	if (challenge) {
		/* the next act's band, but a hit at most half again this act's cap
		 * (act 1's Server hit for 80 of MegaMan's 100 HP) */
		int soft = b.cap + b.cap / 2;
		b = act + 1 < PACING_ACTS ? bands[act + 1] : pacing_band_wider(b);
		if (b.cap > soft) b.cap = soft;
	}
	b = scaled(b, loop);
	/* an opening battle: the band's lower half, and softer hits */
	if (easy) { b.hi = b.lo + (b.hi - b.lo) / 2; b.cap = b.cap * 2 / 3; }
	return b;
}

PacingBand pacing_band_wider(PacingBand b) {
	b.hi += (b.hi - b.lo) / 2 + 40;
	b.cap += b.cap / 2;
	return b;
}

/* V1 through the first two acts, V2 by the fourth, V3 by the sixth */
static const int8_t version_low[PACING_ACTS] = { 0, 0, 0, 1, 1, 2, 2 }, version_high[PACING_ACTS] = { 0, 0, 1, 1, 2, 2, 2 };

/* What a version rolled at `depth` adds to the act's: the loop, the
 * challenge, and threat 1 (docs/META.md: a version stronger from act 2) */
static int version_more(int depth, bool challenge) {
	return pacing_loop(depth) + (challenge ? 1 : 0) + (run.threat >= 1 && pacing_act(depth) >= 1 ? 1 : 0);
}

int pacing_virus_version(int depth, bool challenge) {
	int act = pacing_act(depth);
	int v = rng_range(version_low[act], version_high[act]) + version_more(depth, challenge);
	return v > 3 ? 3 : v;
}

void pacing_virus_versions(int depth, bool challenge, int *lo, int *hi) {
	int act = pacing_act(depth), more = version_more(depth, challenge);
	*lo = version_low[act] + more > 3 ? 3 : version_low[act] + more;
	*hi = version_high[act] + more > 3 ? 3 : version_high[act] + more;
}

bool pacing_rare(int depth, int roll) {
	return pacing_act(depth) >= 2 && roll < 3 + 2 * pacing_loop(depth);
}

/* The areas by the HP of their own battles (docs/PROGRESSION.md). (Sky
 * HP's battles all come in threes of 200 HP, and the Aquarium Comp's pools
 * and water mazes are a hard first map: not a first act's; the Robot
 * Control Comp's that fit act 1 are all OldStove and Mettaur, which a
 * playtester met in every battle of the act; the Seaside Area's hold five
 * kinds of virus in act 1's band, a third opening where two had brought a
 * playtester the RoboDog Comp four runs running) */
static const uint8_t opening[] = { BIOME_CENTRAL, BIOME_COMP, BIOME_SEASIDE };
static const uint8_t middle[] = { BIOME_JUDGE_COMP, BIOME_GREEN, BIOME_GREEN_HP, BIOME_HOMEPAGE, BIOME_COMP_B, BIOME_SKY_HP,
	BIOME_AQUARIUM_COMP, BIOME_ROBOT_COMP };
static const uint8_t late[] = { BIOME_SKY, BIOME_WEATHER_COMP, BIOME_ACDC_HP, BIOME_COPYBOT_COMP };
enum { NO = sizeof opening, NM = sizeof middle, NL = sizeof late };

int pacing_area_pool(int act, uint8_t out[NO + NM + NL]) {
	int n = 0;
	if (act <= 1) for (int i = 0; i < NO; ++i) out[n++] = opening[i];
	if (act == 1 || act == 2) for (int i = 0; i < NM; ++i) out[n++] = middle[i];
	if (act >= 2 && act <= 3) for (int i = 0; i < NL; ++i) out[n++] = late[i];
	return n;
}

void pacing_area_order(uint8_t out[4]) {
	uint8_t pool[NO + NM + NL];
	int n;
	/* act 1 */
	out[0] = opening[rng_range(0, NO - 1)];
	/* act 2: the other opening areas and the middle ones */
	n = 0;
	for (int i = 0; i < NO; ++i) if (opening[i] != out[0]) pool[n++] = opening[i];
	for (int i = 0; i < NM; ++i) pool[n++] = middle[i];
	out[1] = pool[rng_range(0, n - 1)];
	/* act 3: the middle and late ones */
	n = 0;
	for (int i = 0; i < NM; ++i) if (middle[i] != out[1]) pool[n++] = middle[i];
	for (int i = 0; i < NL; ++i) pool[n++] = late[i];
	out[2] = pool[rng_range(0, n - 1)];
	/* act 4: the late ones */
	n = 0;
	for (int i = 0; i < NL; ++i) if (late[i] != out[2]) pool[n++] = late[i];
	out[3] = pool[rng_range(0, n - 1)];
}

void pacing_guardian_band(int act, int *lo, int *hi) {
	static const int band[PACING_ACTS][2] = {
		{ 400, 600 }, { 600, 700 }, { 800, 1000 }, { 1000, 1300 }, { 1100, 1500 }, { 1200, 2000 }, { 0, 100000 },
	};
	if (act < 0) act = 0;
	if (act >= PACING_ACTS) act = PACING_ACTS - 1;
	*lo = band[act][0];
	*hi = band[act][1];
}

static int miss_at(int navi, int act, int version, int (*hp)(int, int)) {
	int h = hp(navi, version), lo, hi;
	if (h < 0) return 100000;
	pacing_guardian_band(act, &lo, &hi);
	return h < lo ? lo - h : h > hi ? h - hi : 0;
}

int pacing_guardian_version(int navi, int act, int loop, bool always_sp, int (*hp)(int navi, int version)) {
	if (always_sp || loop > 0 || act >= 6) return 2;
	/* EX first from the fourth act, which V1 alone would leave too light */
	static const int8_t early[] = { 0 }, ex_first[] = { 1, 0 };
	const int8_t *order = act < 3 ? early : ex_first;
	int n = act < 3 ? 1 : 2, best = order[0], best_miss = 1 << 30;
	for (int i = 0; i < n; ++i) {
		int m = miss_at(navi, act, order[i], hp);
		if (m == 0) return order[i];
		if (m < best_miss) { best_miss = m; best = order[i]; }
	}
	return best;
}

int pacing_guardian_miss(int navi, int act, int loop, bool always_sp, int (*hp)(int navi, int version)) {
	if (always_sp || loop > 0 || act >= 6) return 0;
	return miss_at(navi, act, pacing_guardian_version(navi, act, loop, always_sp, hp), hp);
}

int pacing_guardian_pick(const uint8_t pool[4], const uint8_t *others, int nothers, int act, int loop,
                         bool always_sp, int (*hp)(int navi, int version)) {
	/* the area's own that suit the act, twice as likely as the other navis
	 * that do (the three opening areas' own gave act 1 BlastMan every run);
	 * in act 1, whose band holds three, all alike (BlastMan, the opening
	 * areas' own, was a playtester's first guardian six runs running, and
	 * seven of eleven with DiveMan the only other) */
	int fit[8 + 32], nfit = 0;
	for (int i = 0; i < 4; ++i)
		if (pacing_guardian_miss(pool[i], act, loop, always_sp, hp) == 0) {
			bool again = false;
			for (int j = 0; j < i; ++j) again |= pool[j] == pool[i];
			fit[nfit++] = pool[i];
			if (act > 0 || loop > 0) fit[nfit++] = pool[i];
			else if (again) --nfit;
		}
	for (int i = 0; i < nothers && nfit < (int)(sizeof fit / sizeof *fit); ++i) {
		bool own = false;
		for (int j = 0; j < 4; ++j) own |= others[i] == pool[j];
		if (!own && pacing_guardian_miss(others[i], act, loop, always_sp, hp) == 0) fit[nfit++] = others[i];
	}
	if (nfit) return fit[rng_range(0, nfit - 1)];
	int best = pool[0], best_miss = 1 << 30;
	for (int i = 0; i < 4; ++i) {
		int m = pacing_guardian_miss(pool[i], act, loop, always_sp, hp);
		if (m < best_miss) { best_miss = m; best = pool[i]; }
	}
	return best;
}

/* An older net's guardian's band: three quarters of BN6's guardians'. He
 * is fought in his own game's engine, where the run's Cross never comes
 * and the buster is the NaviCust's alone (10 a charged shot in act 1):
 * KnightMan at BN6's 600 outlasted a playtester's whole act-1 kit by half,
 * where act 1's BN6 guardian, fought with HeatCross, fell in 28 seconds
 * (sessions 67 and 68). */
void pacing_xguardian_band(int act, int *lo, int *hi) {
	pacing_guardian_band(act, lo, hi);
	*lo = *lo * 3 / 4;
	*hi = *hi * 3 / 4;
}

int pacing_xguardian_version(const int hp[4], int act, int loop, bool ex_early) {
	if (loop > 0 || act >= 6) return 3;
	/* (BN5's versions are finer than BN6's V1, EX and SP: its six V1s lie at
	 * 400-700 HP, its V2s at 700-1200, its V3s at 1200-1800. The lowest whose
	 * HP reaches the band's floor, which the cap then holds under its top:
	 * else the strongest) */
	int lo, hi, first = act >= 3 || (ex_early && act >= 1) ? 1 : 0, best = -1;
	pacing_xguardian_band(act, &lo, &hi);
	for (int v = first; v <= 2; ++v) {
		if (hp[v] < 0) continue;
		best = v;
		if (hp[v] >= lo) break;
	}
	return best < 0 ? first : best;
}

int pacing_xguardian_hp(int hp, int act, int loop) {
	if (loop > 0 || act >= 6) return hp;
	int lo, hi;
	pacing_xguardian_band(act, &lo, &hi);
	return hp > hi ? hi : hp;
}

int pacing_older_acts(int depth, int out[PACING_OLDER]) {
	int p = (depth - 1) % CYCLE_LAYERS, cycle = (depth - 1) / CYCLE_LAYERS, act = p >= 18 ? 6 : p / 3, n = 0;
	/* (back from the act before, into the cycle before; never the Nest's,
	 * act 6, one guardian's layer) */
	for (int a = act - 1; n < PACING_OLDER && cycle >= 0; --a) {
		if (a < 0) { a = 6; --cycle; continue; }
		if (a < 6) out[n++] = cycle * CYCLE_LAYERS + a * 3 + 2;
	}
	return n;
}

int pacing_clock_hp(int hp, int clock) {
	if (hp <= 0 || clock <= 0) return hp;
	long more = (long)hp * (100 + RUN_CLOCK_PERCENT * clock) / 100;
	return more > PACING_HP_MOST ? (hp > PACING_HP_MOST ? hp : PACING_HP_MOST) : (int)more;
}

bool pacing_heal_certain(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	/* the middle layer of each act, until the third cycle takes it away;
	 * and the run's first layer, where the starting folder meets its first
	 * battles (a playtester reached its exit at a third of their HP) */
	return (p < 18 && p % 3 == 1 && pacing_loop(depth) < 2) || depth == 1;
}

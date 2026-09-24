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
 * (about 40% of MegaMan's HP then) */
static const PacingBand bands[PACING_ACTS] = {
	{ 100, 200, 50 }, { 150, 280, 80 }, { 200, 360, 120 }, { 280, 420, 160 },
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
	if (challenge) b = act + 1 < PACING_ACTS ? bands[act + 1] : pacing_band_wider(b);
	b = scaled(b, loop);
	if (easy) b.hi = b.lo + (b.hi - b.lo) / 2;
	return b;
}

PacingBand pacing_band_wider(PacingBand b) {
	b.hi += (b.hi - b.lo) / 2 + 40;
	b.cap += b.cap / 2;
	return b;
}

int pacing_virus_version(int depth, bool challenge) {
	/* V1 through the first two acts, V2 by the fourth, V3 by the sixth */
	static const int8_t low[PACING_ACTS] = { 0, 0, 0, 1, 1, 2, 2 }, high[PACING_ACTS] = { 0, 0, 1, 1, 2, 2, 2 };
	int act = pacing_act(depth);
	int v = rng_range(low[act], high[act]) + pacing_loop(depth) + (challenge ? 1 : 0);
	return v > 3 ? 3 : v;
}

bool pacing_rare(int depth, int roll) {
	return pacing_act(depth) >= 2 && roll < 3 + 2 * pacing_loop(depth);
}

void pacing_area_order(uint8_t out[4]) {
	/* by the HP of the areas' own battles (docs/PROGRESSION.md) */
	uint8_t opening[] = { BIOME_CENTRAL, BIOME_ROBOT_COMP, BIOME_AQUARIUM_COMP, BIOME_SKY_HP, BIOME_COMP };
	uint8_t middle[] = { BIOME_SEASIDE, BIOME_JUDGE_COMP, BIOME_GREEN, BIOME_GREEN_HP, BIOME_HOMEPAGE, BIOME_COMP_B };
	uint8_t late[] = { BIOME_SKY, BIOME_WEATHER_COMP, BIOME_ACDC_HP, BIOME_COPYBOT_COMP };
	enum { NO = sizeof opening, NM = sizeof middle, NL = sizeof late };
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
		{ 400, 600 }, { 600, 800 }, { 800, 1000 }, { 1000, 1300 }, { 1100, 1500 }, { 1200, 2000 }, { 0, 100000 },
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
	static const int8_t early[] = { 0 }, late[] = { 1, 0 };
	const int8_t *order = act < 3 ? early : late;
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
	int fit[4], nfit = 0;
	for (int i = 0; i < 4; ++i)
		if (pacing_guardian_miss(pool[i], act, loop, always_sp, hp) == 0) fit[nfit++] = pool[i];
	if (nfit) return fit[rng_range(0, nfit - 1)];
	/* none of the area's own suits the act: another navi that does */
	int ofit[32], nofit = 0;
	for (int i = 0; i < nothers && nofit < 32; ++i)
		if (pacing_guardian_miss(others[i], act, loop, always_sp, hp) == 0) ofit[nofit++] = others[i];
	if (nofit) return ofit[rng_range(0, nofit - 1)];
	int best = pool[0], best_miss = 1 << 30;
	for (int i = 0; i < 4; ++i) {
		int m = pacing_guardian_miss(pool[i], act, loop, always_sp, hp);
		if (m < best_miss) { best_miss = m; best = pool[i]; }
	}
	return best;
}

bool pacing_heal_certain(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	/* the middle layer of each act, until the third cycle takes it away */
	return p < 18 && p % 3 == 1 && pacing_loop(depth) < 2;
}

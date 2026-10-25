#include "run.h"

#include <string.h>

#include "data.h"
#include "rom.h"
#include "game.h"
#include "pacing.h"
#include "rivals.h"
#include "guardians.h"
#include "net.h"
#include "save.h"
#include "super_boss.h"
#include "xguardian.h"

Run run;

/* Each area's guardian, drawn from navis that suit it; in the areas the
 * run visits, from those whose HP suits the act. */
static const uint8_t pools[BIOME_COUNT][4] = {
	{ 12, 2, 5, 12 },   /* Central: BlastMan, ElecMan, ChargeMan */
	{ 13, 6, 2, 1 },    /* Seaside: DiveMan, SpoutMan, ElecMan, HeatMan */
	{ 16, 15, 8, 2 },   /* Sky: ElementMan, JudgeMan, TenguMan, ElecMan */
	{ 7, 9, 5, 7 },     /* Green: TomahawkMan, GroundMan, ChargeMan */
	{ 4, 10, 3, 4 },    /* Graveyard: EraseMan, DustMan, SlashMan */
	{ 11, 3, 1, 11 },   /* Undernet: ProtoMan, SlashMan, HeatMan */
	{ 11, 11, 11, 11 }, /* Secret Area: ProtoMan SP */
	{ 3, 4, 11, 3 },    /* Cybeast Nest */
	{ 14, 12, 18, 14 }, /* Comp: CircusMan, BlastMan, Colonel */
	{ 14, 16, 13, 16 }, /* Homepage: CircusMan, ElementMan, DiveMan */
	{ 18, 15, 12, 18 }, /* Comp (second): Colonel, JudgeMan, BlastMan */
	{ 1, 12, 5, 1 },    /* Robot Control Comp: HeatMan, BlastMan, ChargeMan */
	{ 2, 6, 13, 2 },    /* Aquarium Comp: ElecMan, SpoutMan, DiveMan */
	{ 3, 7, 15, 3 },    /* Judge Tree Comp: SlashMan, TomahawkMan, JudgeMan */
	{ 4, 8, 16, 4 },    /* Mr. Weather Comp: EraseMan, TenguMan, ElementMan */
	{ 18, 14, 11, 18 }, /* CopyBot's comp: Colonel, CircusMan, ProtoMan */
	{ 1, 5, 12, 1 },    /* ACDC HP: HeatMan, ChargeMan, BlastMan */
	{ 3, 9, 7, 3 },     /* Green HP: SlashMan, GroundMan, TomahawkMan */
	{ 4, 10, 16, 4 },   /* Sky HP: EraseMan, DustMan, ElementMan */
};
static const uint8_t navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };

int run_debug_xguardian, run_debug_area = -1;

int run_dress(int biome) {
	/* (--net-biome xN's area for its BN6 area, whatever the seed says: its
	 * names and guardian with its look) */
	const NetAreaDef *d = run_debug_area >= NET_AREAS ? net_area_def(run_debug_area) : NULL;
	if (d && d->like == biome) return run_debug_area;
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *x = net_area_def(NET_AREAS + k);
		if (x && !x->held && x->like == biome && (run.seed * 2654435761u) >> (16 + k) & 1) return NET_AREAS + k;
	}
	return biome;
}

/* A surface act's: where an area of BN5's dresses it and names a Navi of
 * its roaming there (NetAreaDef.xguard), in half the runs (a coin of its
 * own per area, beside the dress's) */
static int dressed_xguardian(int act, int biome) {
	int a = run_dress(biome);
	const NetAreaDef *x = a >= NET_AREAS ? net_area_def(a) : NULL;
	if (!x || x->xrom <= 0 || !x->xguard[0] || act < 0 || act > 5) return 0;
	uint32_t h = (run.seed ^ 0x47524431u) * 2654435761u;   /* "GRD1" */
	int k = a - NET_AREAS;
	if (!(h >> (8 + k) & 1)) return 0;
	int navi = guardian_of_older(x->xguard[1] && (h >> (20 + k) & 1) ? x->xguard[1] : x->xguard[0]);
	return xguardian_fightable(navi) ? navi : 0;
}

/* (BN5's Navi `navi` fits act `act`'s band at the version he is fought at
 * there: his HP as fought reaching its floor) */
static bool xguardian_fits(int navi, int act) {
	int hp[4], lo, hi;
	for (int v = 0; v < 4; ++v) hp[v] = xguardian_hp(navi, v);
	int v = pacing_xguardian_version(hp, act, 0, false);
	pacing_guardian_band(act, &lo, &hi);
	return hp[v] >= 0 && pacing_xguardian_hp(hp[v], act, 0) >= lo;
}

/* The deep acts' (the Undernet's and the Graveyard's areas, whatever
 * dresses them; docs/MULTIROM.md, One net: where they guard): both nets'
 * Navis that fit the act's band in one pool, each as likely, by a coin of
 * the seed and the act: one of BN5's, or 0, BN6's own pick standing (its
 * area's own favoured, as ever: run.boss_order). None that guards a
 * surface act of the run already. */
static int deep_xguardian(int act, int biome) {
	int n6 = 0, n5 = 0, cand[GUARDIAN_OLDER_LAST - GUARDIAN_OLDER_FIRST + 1];
	for (unsigned i = 0; i < sizeof navis; ++i) n6 += pacing_guardian_miss(navis[i], act, 0, false, navi_hp) == 0;
	for (int navi = GUARDIAN_OLDER_FIRST; navi <= GUARDIAN_OLDER_LAST; ++navi) {
		bool taken = false;
		for (int a = 0; a < 4; ++a) taken |= run.biome_order[a] != biome && dressed_xguardian(a, run.biome_order[a]) == navi;
		if (!taken && xguardian_fightable(navi) && xguardian_fits(navi, act)) cand[n5++] = navi;
	}
	if (!n5) return 0;
	uint32_t h = ((run.seed ^ 0x44454550u) + (uint32_t)act * 0x9E3779B9u) * 2654435761u;   /* "DEEP" */
	uint32_t k = (h >> 8) % (uint32_t)(n5 + n6);
	return k < (uint32_t)n5 ? cand[k] : 0;
}

int run_xguardian_at(int act, int biome) {
	bool deep = biome == BIOME_UNDERNET || biome == BIOME_GRAVEYARD;
	/* (a test's Navi of BN5's in every act BN5's areas dress and the deep's: --guardian 24-29) */
	if (guardian_older(run_debug_xguardian) && xguardian_fightable(run_debug_xguardian) && (deep || run_dress(biome) >= NET_AREAS))
		return act >= 0 && act <= 5 ? run_debug_xguardian : 0;
	if (act < 0 || act > 5) return 0;
	return deep ? deep_xguardian(act, biome) : dressed_xguardian(act, biome);
}

int run_guardian(int biome) {
	int acts = run.mode == RUN_SHORT ? 3 : 6, act = -1;
	for (int a = 0; a < acts && act < 0; ++a)
		if (run.biome_order[a] == biome) act = a;
	/* (a test's area of another game, laid out as the BN6 area it is like,
	 * which the run's order need not hold: the layer's act) */
	int p = (run.depth - 1) % CYCLE_LAYERS;
	if (act < 0 && biome == run.biome && p < 18) act = p / 3;
	int x = act >= 0 && biome >= 0 && biome < BIOME_COUNT ? run_xguardian_at(act, biome) : 0;
	return x ? x : biome >= 0 && biome < MAX_BIOMES ? run.boss_order[biome] : 0;
}

int run_layer_guardian(void) {
	if (run.side_kind == LAYER_BACK) return 0;
	return run.side_kind == LAYER_NORMAL ? run_guardian(run.biome) : run.boss_order[run.biome];
}

void run_debug_guardian(int navi) {
	if (guardian_older(navi)) run_debug_xguardian = navi;
	else for (int b = 0; b < MAX_BIOMES; ++b) run.boss_order[b] = (uint8_t)navi;
}

void run_new(uint32_t seed) {
	memset(&run, 0, sizeof run);
	run.active = true;
	run.seed = seed;
	run.depth = 1;
	rng_seed(seed);
	/* Acts 1-4 visit four of the surface areas, easier ones first; then the
	 * Undernet and the Graveyard, which BN6 keeps for after its story. */
	pacing_area_order(run.biome_order);
	run.biome_order[4] = BIOME_UNDERNET;
	run.biome_order[5] = BIOME_GRAVEYARD;
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = pools[b][rng_range(0, 3)];
	for (int act = 0; act < 6; ++act) {
		int b = run.biome_order[act], g = 0;
		/* (not an earlier act's guardian again, where another fits: act 1's
		 * band shares SpoutMan with act 2's; and one never met, in any run,
		 * where the area has one: a first meeting is a discovery, and a
		 * playtester 34 sessions in had met eight of the seventeen) */
		for (int tries = 0; tries < 14; ++tries) {
			g = pacing_guardian_pick(pools[b], navis, (int)sizeof navis, act, 0, false, navi_hp);
			bool again = false;
			for (int e = 0; e < act; ++e) again |= run.boss_order[run.biome_order[e]] == g;
			const Rival *r = rival(g);
			if (!again && tries < 8 && (r->met || r->megaman_won || r->navi_won)) again = true;
			if (!again) break;
		}
		run.boss_order[b] = (uint8_t)g;
	}
	/* the super bosses' places (docs/BOSSES.md, Super bosses): the endless
	 * net's Nest is the Cybeast's den (a short net's setup picks its own
	 * Nest's guardian, run_setup), and the Secret Area is Bass's once it
	 * has been cleared in any run (the title's S) */
	run.boss_order[BIOME_NEST] = (uint8_t)super_nest_master();
	run.boss_order[BIOME_SECRET] = (uint8_t)super_secret_master(profile.marks, run.boss_order[BIOME_SECRET]);
}

/* Whether guardian `g` is another act's (the four of the surface), or the
 * short net's Nest's. */
static bool guardian_taken(int g) {
	bool taken = run.mode == RUN_SHORT && run.boss_order[BIOME_NEST] == g;
	for (int e = 0; e < 4; ++e) taken |= run.boss_order[run.biome_order[e]] == g;
	return taken;
}

/* The guardian nearest act `act`'s band that no act has, the area's own
 * (`pool`) first where two are as near; -1 none. */
static int nearest_free_guardian(int act, const uint8_t pool[4]) {
	int best = -1, best_miss = 1 << 30;
	for (unsigned i = 0; i < sizeof navis; ++i) {
		if (guardian_taken(navis[i])) continue;
		int miss = pacing_guardian_miss(navis[i], act, 0, false, navi_hp);
		bool own = pool[0] == navis[i] || pool[1] == navis[i] || pool[2] == navis[i] || pool[3] == navis[i];
		if (miss < best_miss || (miss == best_miss && own)) { best_miss = miss; best = navis[i]; }
	}
	return best;
}

int run_route_alt(int act, int *navi) {
	/* (acts 2-4 of the surface; the short net's fourth is its Nest) */
	if (act < 1 || act > 3 || (run.mode == RUN_SHORT && act > 2)) return -1;
	uint8_t pool[PACING_AREA_POOL], free_areas[PACING_AREA_POOL];
	int n = pacing_area_pool(act, pool), m = 0;
	for (int i = 0; i < n; ++i) {
		bool taken = false;
		for (int a = 0; a < 6; ++a) taken |= run.biome_order[a] == pool[i];
		if (!taken) free_areas[m++] = pool[i];
	}
	if (!m) return -1;
	/* (its own random numbers, the game's kept: the same way offered on
	 * the layer and taken at its exit, and after a CONTINUE) */
	uint32_t keep = rng_state();
	rng_seed(run.seed ^ 0x524F5554u ^ (uint32_t)act * 2654435761u);
	int b = free_areas[rng_range(0, m - 1)], g = 0;
	/* (no guardian another act has: the ones before, the way it stands
	 * beside, those after, the short net's Nest) */
	bool again = true;
	for (int tries = 0; tries < 8 && again; ++tries) {
		g = pacing_guardian_pick(pools[b], navis, (int)sizeof navis, act, 0, false, navi_hp);
		again = guardian_taken(g);
	}
	/* (where every try met one, as a band that few fit gives: the nearest
	 * no act has; a playtester's split offered SpoutMan both ways, Green
	 * Area's and Aquarium Comp's, session 59) */
	int free = again ? nearest_free_guardian(act, pools[b]) : -1;
	if (free > 0) g = free;
	rng_restore(keep);
	*navi = g;
	return b;
}

/* The area run_new planned for act `act` (pacing_area_order from the
 * seed), whatever way the act took since */
static int planned_area(int act) {
	if (act >= 4) return act == 4 ? BIOME_UNDERNET : BIOME_GRAVEYARD;
	uint32_t keep = rng_state();
	rng_seed(run.seed);
	uint8_t order[4];
	pacing_area_order(order);
	rng_restore(keep);
	return order[act];
}

int run_dark_area(int act) {
	if (act < 1) return -1;
	uint32_t h = ((run.seed ^ 0x4C494245u) + (uint32_t)act * 0x9E3779B9u) * 2654435761u;   /* "LIBE" */
	if ((h >> 8) % 3 == 0) return -1;
	/* (the fork as offered: the act's own area, and the other way beside it) */
	int own = planned_area(act), was = run.biome_order[act], navi = 0;
	run.biome_order[act] = (uint8_t)own;
	int alt = run_route_alt(act, &navi);
	run.biome_order[act] = (uint8_t)was;
	if (alt < 0) return -1;
	return h >> 20 & 1 ? alt : own;
}

int run_darkloid(int act, int biome) {
	uint32_t keep = rng_state();
	rng_seed(run.seed ^ 0x444C4F44u ^ (uint32_t)act * 2654435761u);   /* "DLOD" */
	int g = 0;
	for (int tries = 0; tries < 8; ++tries) {
		g = pacing_guardian_pick(pools[biome >= 0 && biome < BIOME_COUNT ? biome : 0], navis, (int)sizeof navis, act, 0, false, navi_hp);
		if (!guardian_taken(g)) break;
	}
	rng_restore(keep);
	return g;
}

static const uint8_t nest_pool[4] = { 3, 4, 11, 3 };   /* SlashMan, EraseMan, ProtoMan */

int run_nest_second(void) {
	uint32_t keep = rng_state();
	rng_seed(run.seed ^ 0x4E455332u);
	/* (the endless net's too, a special route's last boss: none of its six
	 * acts' guardians) */
	int first = run.boss_order[BIOME_NEST], g = first, acts = run.mode == RUN_SHORT ? 3 : 6;
	for (int tries = 0; tries < 16; ++tries) {
		g = pacing_guardian_pick(nest_pool, navis, (int)sizeof navis, 3, 0, false, navi_hp);
		bool again = g == first;
		for (int e = 0; e < acts; ++e) again |= run.boss_order[run.biome_order[e]] == g;
		if (!again) break;
	}
	rng_restore(keep);
	return g;
}

/* The Nest's act, after the net's last: its one way, the Nest */
static int nest_act(void) { return ((run.mode == RUN_SHORT ? SHORT_LAYERS : CYCLE_LAYERS) - 1) / 3; }

/* Whether act `act` has the short net's dark way (docs/META.md, branches) */
static bool dark_way_at(int act) { return run.mode == RUN_SHORT && act == 2; }

int run_route_turns(bool dark_open) {
	if (run.route_found < 1) return -1;
	for (int t = run.route_found; t < nest_act(); ++t) {
		int navi;
		/* (a fork with room for a third way: its dark way sealed, or none) */
		if (run_route_alt(t, &navi) >= 0 && !(dark_way_at(t) && dark_open)) return t;
	}
	return nest_act();
}

void run_route_keep(int depth) {
	int act = (depth - 1) % CYCLE_LAYERS / 3;
	if (act < nest_act()) run.route_found = (uint8_t)(act + 1);
}

void run_nest_rebuilt(void) { run.boss_order[BIOME_NEST] = (uint8_t)super_nest_master(); }

int run_nest_hp(int depth, int navi, int hp) {
	if (run_short_nest(depth) || super_boss(navi)) return hp;
	int beast = navi_hp(SUPER_CYBEAST, super_form(SUPER_CYBEAST, depth));
	return beast > hp ? beast : hp;
}

/* The special route's way at act `act`'s fork where the fork has no dark
 * way: another area of the act's tier the run has not taken, not the
 * other way's (`alt`), and its guardian fit for the act, no other act's
 * nor the other way's (`alt_navi`); -1 none. From the seed, as the other
 * way is: the same offered after a CONTINUE. */
static int route_third(int act, int alt, int alt_navi, int *navi) {
	uint8_t pool[PACING_AREA_POOL], free_areas[PACING_AREA_POOL];
	int n = pacing_area_pool(act, pool), m = 0;
	for (int i = 0; i < n; ++i) {
		bool taken = pool[i] == alt;
		for (int a = 0; a < 6; ++a) taken |= run.biome_order[a] == pool[i];
		if (!taken) free_areas[m++] = pool[i];
	}
	if (!m) return -1;
	uint32_t keep = rng_state();
	rng_seed(run.seed ^ 0x52545233u ^ (uint32_t)act * 2654435761u);   /* "RTR3" */
	int b = free_areas[rng_range(0, m - 1)], g = 0;
	bool again = true;
	/* (none of the deep acts' either, as one of the run's own, the way
	 * found is a guardian not met again: the fork's guardians are the
	 * surface's) */
	for (int tries = 0; tries < 12 && again; ++tries) {
		g = pacing_guardian_pick(pools[b], navis, (int)sizeof navis, act, 0, false, navi_hp);
		again = guardian_taken(g) || g == alt_navi;
		for (int a = 4; a < 6 && run.mode != RUN_SHORT; ++a) again |= run.boss_order[run.biome_order[a]] == g;
	}
	int free = again ? nearest_free_guardian(act, pools[b]) : -1;
	if (free > 0 && free != alt_navi) g = free;
	rng_restore(keep);
	*navi = g;
	return b;
}

int run_ways(int act, bool dark_open, RunWay way[3], bool *sealed) {
	bool route = run_route_turns(dark_open) == act;
	*sealed = false;
	/* (the Nest's act: its one way, the Nest; a special route's second
	 * way to it, its second guardian: docs/LIBERATION.md, Special routes) */
	if (act >= nest_act()) {
		int g = run.boss_order[BIOME_NEST];
		way[0] = (RunWay){ BIOME_NEST, g, g, false, false };
		if (!route) return 1;
		g = run_nest_second();
		way[1] = (RunWay){ BIOME_NEST, g, g, false, true };
		return 2;
	}
	int alt_navi = 0, dark_navi = 0, alt = run_route_alt(act, &alt_navi);
	int dark = alt >= 0 ? run_route_dark(act, alt_navi, &dark_navi) : -1;
	int b[3] = { run.biome_order[act], alt, dark }, navi[3] = { run.boss_order[run.biome_order[act]], alt_navi, dark_navi };
	/* (the route opens a sealed dark way, else brings a third area) */
	bool open = dark_open || route;
	int n = alt < 0 ? 1 : dark < 0 || !open ? 2 : 3;
	*sealed = dark >= 0 && !open;
	if (route && alt >= 0 && dark < 0 && (b[2] = route_third(act, alt, alt_navi, &navi[2])) >= 0) n = 3;
	for (int k = 0; k < n; ++k) {
		int x = run_xguardian_at(act, b[k]);
		way[k] = (RunWay){ b[k], navi[k], x ? x : navi[k], k < 2 && run_dark_area(act) == b[k], route && k == 2 && !(dark >= 0 && dark_open) };
	}
	return n;
}

int run_route_dark(int act, int avoid, int *navi) {
	if (!dark_way_at(act)) return -1;
	uint32_t keep = rng_state();
	rng_seed(run.seed ^ 0x4441524Bu);   /* "DARK" */
	int g = 0;
	for (int tries = 0; tries < 8; ++tries) {
		g = pacing_guardian_pick(pools[BIOME_UNDERNET], navis, (int)sizeof navis, act, 0, false, navi_hp);
		bool again = g == avoid || run.boss_order[BIOME_NEST] == g;
		for (int e = 0; e < 3; ++e) again |= run.boss_order[run.biome_order[e]] == g;
		if (!again) break;
	}
	rng_restore(keep);
	*navi = g;
	return BIOME_UNDERNET;
}

void run_setup(int mode, int folder, int threat, int helpers, int cross) {
	run.mode = (uint8_t)mode;
	run.folder = (uint8_t)folder;
	run.threat = (uint8_t)(threat < 0 ? 0 : threat > THREAT_MAX ? THREAT_MAX : threat);
	run.helpers = (uint8_t)helpers;
	run.cross = (uint8_t)(cross >= 1 && cross <= 5 ? cross : 0);
	if (mode != RUN_SHORT) return;
	/* the short net's Nest comes as the fourth act (its guardian no SP),
	 * none of the three acts' guardians again */
	rng_seed(run.seed ^ 0x4E455354u);
	int g = 0;
	for (int tries = 0; tries < 8; ++tries) {
		g = pacing_guardian_pick(nest_pool, navis, (int)sizeof navis, 3, 0, false, navi_hp);
		bool again = false;
		for (int e = 0; e < 3; ++e) again |= run.boss_order[run.biome_order[e]] == g;
		if (!again) break;
	}
	run.boss_order[BIOME_NEST] = (uint8_t)g;
}

int navi_hp(int navi, int version) {
	int hp, damage, id = enemy_id(1, navi, version);
	return id >= 0 && enemy_stats(id, &hp, &damage) ? hp : -1;
}

/* the BattleSettings background (0x00-0x15) of each area's battles, two
 * where the game has a second that suits it */
static const uint8_t biome_bgs[BIOME_COUNT][2] = {
	{ 0x07, 0x09 }, { 0x0B, 0x0A }, { 0x04, 0x10 }, { 0x0D, 0x0C }, { 0x14, 0x12 }, { 0x0F, 0x11 },
	{ 0x13, 0x13 }, { 0x15, 0x15 }, { 0x06, 0x06 }, { 0x03, 0x01 }, { 0x08, 0x00 },
	{ 0x0E, 0x06 }, { 0x0A, 0x0B }, { 0x05, 0x0C }, { 0x10, 0x04 }, { 0x06, 0x0E },
	{ 0x00, 0x00 }, { 0x01, 0x0D }, { 0x04, 0x10 },
};

int biome_bg(int b) { return b >= 0 && b < BIOME_COUNT ? biome_bgs[b][rng_range(0, 1)] : 0; }

int biome_backdrop(int b) { return b >= 0 && b < BIOME_COUNT ? biome_bgs[b][0] : 0; }

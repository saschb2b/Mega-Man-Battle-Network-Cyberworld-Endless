#include "run.h"

#include <string.h>

#include "data.h"
#include "rom.h"
#include "game.h"
#include "pacing.h"
#include "rivals.h"
#include "guardians.h"
#include "guest.h"
#include "net.h"

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

int run_xguardian_at(int act, int biome) {
	int a = run_dress(biome);
	const NetAreaDef *x = a >= NET_AREAS ? net_area_def(a) : NULL;
	if (!x || x->xrom <= 0 || !x->xguard[0] || !guest_possible(x->xrom - 1)) return 0;
	if (guardian_older(run_debug_xguardian)) return run_debug_xguardian;
	/* (not the short net's last act, whose Nest is BN6's: his Soul would
	 * serve no battle of his game's after him) */
	if (act < 0 || act > 5 || (run.mode == RUN_SHORT && act >= 2)) return 0;
	/* (a coin of its own per area, beside the dress's: half the runs) */
	uint32_t h = (run.seed ^ 0x47524431u) * 2654435761u;   /* "GRD1" */
	int k = a - NET_AREAS;
	if (!(h >> (8 + k) & 1)) return 0;
	return guardian_of_older(x->xguard[1] && (h >> (20 + k) & 1) ? x->xguard[1] : x->xguard[0]);
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

int run_layer_guardian(void) { return run.side_kind == LAYER_NORMAL ? run_guardian(run.biome) : run.boss_order[run.biome]; }

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

static const uint8_t nest_pool[4] = { 3, 4, 11, 3 };   /* SlashMan, EraseMan, ProtoMan */

int run_nest_second(void) {
	uint32_t keep = rng_state();
	rng_seed(run.seed ^ 0x4E455332u);
	int first = run.boss_order[BIOME_NEST], g = first;
	for (int tries = 0; tries < 16; ++tries) {
		g = pacing_guardian_pick(nest_pool, navis, (int)sizeof navis, 3, 0, false, navi_hp);
		bool again = g == first;
		for (int e = 0; e < 3; ++e) again |= run.boss_order[run.biome_order[e]] == g;
		if (!again) break;
	}
	rng_restore(keep);
	return g;
}

int run_route_dark(int act, int avoid, int *navi) {
	if (run.mode != RUN_SHORT || act != 2) return -1;
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

/* The pacing report (docs/DEVTOOLS.md): for every act of the first cycles
 * and every area it can visit, the random battles the engine rolls (virus
 * HP together, the strongest hit, versions) against the act's band, and the
 * guardians runs draw, from the ROM without the game. */
#include "pacing_report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "emu.h"
#include "game.h"
#include "guardians.h"
#include "loot.h"
#include "net.h"
#include "navicust.h"
#include "jobs.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "shop.h"
#include "xguardian.h"
#include "xrecords.h"
#include "visitor_families.h"
#include "visitors.h"
#include "xbattles.h"
#include "xinstall.h"

#define ROLLS 300

static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

static void battle_stats(const Encounter *e, int *hp, int *dmg, int *top) {
	*hp = *dmg = 0;
	*top = 0;
	for (int i = 0; i < e->nfoes; ++i) {
		const Foe *f = &e->foes[i];
		int h, d;
		if (f->kind == FOE_ROCK || !enemy_stats(f->id, &h, &d)) continue;
		*hp += h;
		if (d > *dmg) *dmg = d;
		if (f->version > *top) *top = f->version;
	}
}

/* Returns how many battles lay past the band's limits. */
static int battles(FILE *out, int depth, int biome, int kind, const char *label) {
	static int hps[ROLLS];
	/* an opening battle aims low but may take the act's whole band */
	PacingBand band = pacing_band(depth, kind == ENC_CHALLENGE, false);
	int over = 0, navis = 0, maxdmg = 0, versions[6] = { 0 }, families[64] = { 0 }, gems = 0, blocks = 0, older = 0;
	for (int r = 0; r < ROLLS; ++r) {
		Encounter e = make_encounter(depth, biome, kind);
		loot_battle_fought(&e);   /* each roll as a battle fought */
		older += e.xrecord != 0;   /* (one of the older net's records, in BN6's engine) */
		/* (the virus families a battle brings, each counted once: BN6's AI
		 * indexes, the families moved into its engine from 32) */
		uint64_t in = 0;
		for (int i = 0; i < e.nfoes; ++i)
			if (e.foes[i].kind == FOE_VIRUS && e.foes[i].family > 0 && e.foes[i].family < 64) in |= (uint64_t)1 << e.foes[i].family;
		for (int f = 1; f < 64; ++f) families[f] += (int)(in >> f & 1);
		/* (the field's objects: a Mystery Data, and rocks or cubes) */
		bool gem = false, block = false;
		for (int i = 0; i < e.nobj; ++i) {
			gem |= e.obj[i].kind >> 4 == FIELD_GEM;
			block |= e.obj[i].kind >> 4 != FIELD_GEM;
		}
		gems += gem;
		blocks += block;
		int hp, dmg, top;
		battle_stats(&e, &hp, &dmg, &top);
		bool navi = false;
		for (int i = 0; i < e.nfoes; ++i) navi |= e.foes[i].kind == FOE_NAVI;
		if (navi) { ++navis; hps[r] = 0; continue; }   /* a challenge's SP navi keeps his own HP */
		hps[r] = hp;
		if (dmg > maxdmg) maxdmg = dmg;
		if (top >= 0 && top < 6) versions[top]++;
		if (hp > band.hi || dmg > band.cap) ++over;
	}
	qsort(hps, ROLLS, sizeof *hps, cmp_int);
	int n = ROLLS - navis, *h = hps + navis;
	fprintf(out, "  depth %2d %-9s band %3d-%3d cap %3d | hp %3d / %3d / %3d  hit %3d  V1-SP,rare %d/%d/%d/%d/%d",
		depth, label, band.lo, band.hi, band.cap, n ? h[0] : 0, n ? h[n / 2] : 0, n ? h[n - 1] : 0, maxdmg,
		versions[0], versions[1], versions[2], versions[3], versions[4] + versions[5]);
	if (gems || blocks) fprintf(out, "  gem %d rocks %d", gems, blocks);
	if (older) fprintf(out, "  older net's %d", older);
	if (navis) fprintf(out, "  SP navi %d", navis);
	fprintf(out, "  families");
	for (int f = 1; f < 64; ++f) if (families[f]) fprintf(out, " %d:%d", f, families[f]);
	fprintf(out, "%s\n", over ? "  OVER" : "");
	return over;
}

/* ... for every act's areas another game's area dresses, where its ROM is
 * beside BN6's (run_dress: each in about half the runs that come there);
 * each layer's first battle from the band's lower half, as the director
 * picks it */
static int territories(FILE *out, int act, int loop, int biome) {
	int flagged = 0;
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *x = net_area_def(NET_AREAS + k);
		if (!x || x->held || x->like != biome || x->xrom <= 0) continue;
		int first = loop * CYCLE_LAYERS + act * 3 + 1, last = act == 6 ? first : first + 2;
		/* (its own records in BN6's engine, as the director rolls them,
		 * where every one of them can be: issue #82's native territories;
		 * the deep's acts mix both nets, above, whatever dresses them) */
		if (loot_layer_nets(biome, NET_AREAS + k) && loot_territory >= 0) {
			fprintf(out, "  %s's battles, its own records in BN6's engine where it dresses it (native):\n", x->name);
			for (int d = first; d <= last; ++d) flagged += battles(out, d, biome, ENC_NORMAL, "native");
			flagged += battles(out, first, biome, ENC_EASY, "opening");
			if (act < 6) flagged += battles(out, first + 1, biome, ENC_CHALLENGE, "challenge");
			loot_layer_nets(-1, -1);
			continue;
		}
		loot_layer_nets(-1, -1);
		/* (an install that failed: its layers fight the BN6 area's own, above) */
		if (biome != BIOME_UNDERNET && biome != BIOME_GRAVEYARD) fprintf(out, "  %s: not native here, the BN6 area's battles\n", x->name);
	}
	return flagged;
}

/* One area in one act of a cycle: its random battles rolled (from `seed`),
 * then its territory's, where another game's area dresses it; how many
 * past their band */
static int act_area(FILE *out, int act, int loop, int b, uint32_t seed) {
	int flagged = 0, first = loop * CYCLE_LAYERS + act * 3 + 1, last = act == 6 ? first : first + 2;
	/* (the deep's acts draw on both nets, a coin a battle, where BN5's
	 * ROM is read: docs/MULTIROM.md, One net) */
	bool deep = loot_layer_nets(b, b) && loot_deep[0] >= 0;
	fprintf(out, "\nact %d cycle %d  %s%s\n", act + 1, loop + 1, guardian_area_name(b), deep ? " (the deep: both nets' battles, the older net's counted)" : "");
	rng_seed(seed);
	for (int d = first; d <= last; ++d) flagged += battles(out, d, b, ENC_NORMAL, "battle");
	flagged += battles(out, first, b, ENC_EASY, "opening");
	if (first == 1) flagged += battles(out, first, b, ENC_FIRST, "first");
	if (act < 6) flagged += battles(out, first + 1, b, ENC_CHALLENGE, "challenge");
	loot_layer_nets(-1, -1);
	return flagged + territories(out, act, loop, b);
}

/* ---- visitors (visitors.h; docs/MULTIROM.md, One net): each net's
 * families as their own records set them, then every act's random battles
 * from the second on, each given a visitor where one fits, against the
 * act's band ---- */

static void kin_report(FILE *out) {
	static const char *const roles[] = { "either", "back", "front" };
	for (int net = 0; net < 2; ++net) {
		const VisitorKin *k;
		int n = visitor_kin(net, &k);
		fprintf(out, "%s's families (role, stage, placements; * may visit, ! special panels alone):", net == VISIT_BN5 ? "BN5" : "BN6");
		for (int i = 0; i < n; ++i) {
			char name[24] = "?";
			int id = visitor_id(net, k[i].family, 0);
			if (id < 0) id = enemy_id(0, k[i].family, 0);
			if (id >= 0) enemy_name(id, name, sizeof name);
			fprintf(out, " %s%s%s %s s%d n%d", k[i].visits ? "*" : "", k[i].bound ? "!" : "", name, roles[k[i].role], k[i].stage, k[i].placed);
		}
		fprintf(out, "\n");
	}
}

/* One area's battles at `depth` (its own, or a territory's where `tiles`
 * names one), each with a visitor where one fits: how many took one, how
 * many fit no visitor, the pairs (replaced -> visitor), HP against the
 * band; returns those past it */
static int visitor_rolls(FILE *out, int depth, int biome, int tiles, const char *label) {
	loot_layer_nets(biome, tiles);
	PacingBand b = pacing_band(depth, false, false);
	int took = 0, over = 0, hps[ROLLS], pairs[64][64];
	memset(pairs, 0, sizeof pairs);
	for (int r = 0; r < ROLLS; ++r) {
		Encounter e = make_encounter(depth, biome, ENC_NORMAL);
		loot_battle_fought(&e);
		int hp = 0, dmg = 0, top;
		Encounter v = e;
		bool ok = visitor_apply(&v, depth);
		battle_stats(&v, &hp, &dmg, &top);
		hps[r] = hp;
		if (!ok) continue;
		++took;
		int from = e.foes[v.visitor - 1].family, to = v.foes[v.visitor - 1].family;
		if (from > 0 && from < 64 && to > 0 && to < 64) ++pairs[from][to];
		over += hp > b.hi || dmg > b.cap;
	}
	loot_layer_nets(-1, -1);
	qsort(hps, ROLLS, sizeof *hps, cmp_int);
	fprintf(out, "  depth %2d %-24s band %3d-%3d cap %3d | a visitor in %3d of %d, hp %3d / %3d / %3d%s\n    pairs:", depth, label, b.lo, b.hi, b.cap,
		took, ROLLS, hps[0], hps[ROLLS / 2], hps[ROLLS - 1], over ? "  OVER" : "");
	for (int f = 1; f < 64; ++f)
		for (int t = 1; t < 64; ++t)
			if (pairs[f][t]) fprintf(out, " %d>%d:%d", f, t, pairs[f][t]);
	fprintf(out, "\n");
	return over;
}

static int visitors_report(FILE *out) {
	int flagged = 0;
	fprintf(out, "\nVisitors (where BN5's ROM is read): each net's families as their own records set them, then each act's battles\n"
		"(%d rolls a layer) each with a visitor where one fits, as one battle in four from act 2 takes one: how many fit,\n"
		"the battles' HP with it, the pairs (replaced family > visitor family, BN6's AI indexes), OVER past the band.\n", ROLLS);
	kin_report(out);
	for (int act = 1; act < 4; ++act) {
		uint8_t pool[PACING_AREA_POOL];
		int n = pacing_area_pool(act, pool), depth = act * 3 + 2;
		fprintf(out, "act %d:\n", act + 1);
		rng_seed(0x56495349u + (uint32_t)act);
		for (int i = 0; i < n; ++i) {
			flagged += visitor_rolls(out, depth, pool[i], pool[i], guardian_area_name(pool[i]));
			for (int k = 0; k < XAREAS_MAX; ++k) {
				const NetAreaDef *x = net_area_def(NET_AREAS + k);
				if (x && !x->held && x->xrom > 0 && x->like == pool[i] && xterritory_native(NET_AREAS + k))
					flagged += visitor_rolls(out, depth, pool[i], NET_AREAS + k, x->name);
			}
		}
	}
	return flagged;
}

/* A variant of `program` in colour `color` (program * 4 + v), 0 for none. */
static int variant_of(int program, int color) {
	NaviShape s;
	for (int v = 0; v < 4; ++v)
		if (navicust_shape(program * 4 + v, &s) && s.color == color) return program * 4 + v;
	return 0;
}

/* The NaviCust's drafts by the Spins held (docs/META.md, the Spins found in
 * the net): runs of a gift program, then each guardian's draft over the
 * first cycle, one of its options taken at random; how many options each
 * draft had. The draft offers only programs that fit, turned only where
 * their colour's Spin is held. */
#define DRAFT_RUNS 200
static void drafts_report(FILE *out) {
	static const struct { const char *name; unsigned mask; } sets[] = {
		{ "no Spin", 0 }, { "one (white)", 0x01 }, { "three (white, yellow, pink)", 0x07 }, { "all six", 0x3F },
	};
	static const uint8_t gifts[4] = { 1, 2, 35, 37 };   /* (shop.c's gift programs) */
	static const int depths[6] = { 3, 6, 9, 12, 15, 18 };
	fprintf(out, "\nNaviCust drafts by the Spins held (%d runs: a gift program, then a pick at random from each guardian's draft):\n"
		"the share of drafts with three options, and with fewer\n", DRAFT_RUNS);
	for (unsigned s = 0; s < sizeof sets / sizeof *sets; ++s) {
		navicust_set_spins(sets[s].mask);
		int count[6][NAVICUST_DRAFT + 1] = { { 0 } };
		for (int r = 0; r < DRAFT_RUNS; ++r) {
			rng_seed(0x5B1Du + (uint32_t)r * 7919u);
			uint8_t have[8] = { 0 };
			int nhave = 0, g = gifts[rng_range(0, 3)], v = variant_of(g, navicust_color(g));
			if (v) have[nhave++] = (uint8_t)v;
			for (int d = 0; d < 6; ++d) {
				int w, h;
				navicust_board((depths[d] >= 6) + (depths[d] >= 12), &w, &h);
				NaviProgram pick[NAVICUST_DRAFT];
				int n = navicust_draft_fitting(depths[d], pick, have, nhave, w, h);
				++count[d][n < 0 ? 0 : n > NAVICUST_DRAFT ? NAVICUST_DRAFT : n];
				if (n > 0 && nhave < 8) {
					int k = rng_range(0, n - 1), vv = variant_of(pick[k].program, pick[k].color);
					if (vv) have[nhave++] = (uint8_t)vv;
				}
			}
		}
		fprintf(out, "%s:\n", sets[s].name);
		for (int d = 0; d < 6; ++d) {
			fprintf(out, "  layer %2d:", depths[d]);
			for (int n = NAVICUST_DRAFT; n >= 0; --n) fprintf(out, " %d: %3d%%", n, count[d][n] * 100 / DRAFT_RUNS);
			fprintf(out, "\n");
		}
	}
	navicust_set_spins(0x3F);
}

/* What a Net Dealer says the area's viruses can't stand, where the
 * guardian is weak to none (counter_element: 0, no word), by act; then
 * each guardian's element and the one that hits him twice as hard, which
 * the dealers' counter chip answers (- none). */
static void elements_report(FILE *out) {
	fprintf(out, "\nThe viruses' answer by act and area (- none):");
	for (int act = 0; act < 4; ++act) {
		uint8_t pool[PACING_AREA_POOL];
		int n = pacing_area_pool(act, pool);
		fprintf(out, "\nact %d:", act + 1);
		for (int i = 0; i < n; ++i) {
			int c = counter_element(act * 3 + 1, pool[i], 0);
			fprintf(out, " %s %s", guardian_area_name(pool[i]), c > 0 ? elem_name(c) : "-");
		}
	}
	fprintf(out, "\n\nGuardians' elements and weaknesses:");
	/* (BN5's by their own stats: guardian_element, its first wheel) */
	for (int n = 1; n < 32; ++n) {
		if ((!guardian_older(n) && enemy_id(1, n, 0) < 0) || (guardian_older(n) && xguardian_hp(n, 0) < 0) || guardian(n)->name[0] == '?') continue;
		int e = guardian_element(n), w = guardian_weakness(n);
		fprintf(out, " %s%s %s/%s", guardian(n)->name, guardian_older(n) ? " (BN5)" : "", e > 0 ? elem_name(e) : "-", w > 0 ? elem_name(w) : "-");
	}
	fprintf(out, "\n");
}

/* The version the guardian of a first cycle's act `act` (6 the Nest) is
 * fought at; a super boss's his own form (super_boss.h) */
static int cycle_version(int navi, int act, bool nest) {
	return super_boss(navi) ? super_form(navi, CYCLE_LAYERS) : pacing_guardian_version(navi, act, 0, nest, navi_hp);
}

/* The guardians a first cycle draws, over many runs (BN6's: run_new's
 * picks, the endless net's); how many outside their act's band */
static int first_cycle_guardians(FILE *out) {
	int flagged = 0;
	fprintf(out, "\nGuardians of the first cycle over 500 runs: navi, version and HP, how often.\n");
	for (int act = 0; act < 7; ++act) {
		int lo, hi, seen[32][6] = { { 0 } }, outside = 0;
		pacing_guardian_band(act, &lo, &hi);
		for (uint32_t seed = 1; seed <= 500; ++seed) {
			run_new(seed * 2654435761u);
			int b = act < 6 ? run.biome_order[act] : BIOME_NEST, navi = run.boss_order[b];
			int v = cycle_version(navi, act, b == BIOME_NEST), hp = navi_hp(navi, v);
			if (navi >= 0 && navi < 32 && v >= 0 && v < 6) seen[navi][v]++;
			if (act < 6 && (hp < lo || hp > hi)) ++outside;
		}
		fprintf(out, "act %d (band %d-%d):", act + 1, lo, act < 6 ? hi : 0);
		for (int n = 0; n < 32; ++n)
			for (int v = 0; v < 6; ++v)
				if (seen[n][v]) fprintf(out, " %s%s %d (%d)", guardian(n)->name, super_boss(n) ? super_suffix(n, v) : v == 1 ? "EX" : v == 2 ? "SP" : "",
					navi_hp(n, v), seen[n][v]);
		fprintf(out, "%s\n", outside ? "  OUTSIDE" : "");
		flagged += outside;
	}
	return flagged;
}

/* Whether the special route's way `r` at act `act`'s fork (`w[0..n)` the
 * fork's ways, the route's last) shares a guardian with another way or
 * one of the run's acts, or a fork's third way an area with them; or the
 * fork offers otherwise a second time, as after a CONTINUE */
static bool route_clash(const RunWay *w, int n, int act) {
	const RunWay *r = &w[n - 1];
	bool clash = false, nest = r->biome == BIOME_NEST, dark = r->biome == BIOME_UNDERNET;
	for (int k = 0; k < n - 1; ++k) clash |= w[k].navi == r->navi || (!nest && w[k].biome == r->biome);
	for (int a = 0; a < (run.mode == RUN_SHORT ? 3 : 6); ++a)
		clash |= run.boss_order[run.biome_order[a]] == r->navi || (!nest && !dark && run.biome_order[a] == r->biome);
	RunWay again[3];
	bool sealed;
	run.route_found = (uint8_t)act;
	clash |= run_ways(act, false, again, &sealed) != n;
	for (int k = 0; k < n; ++k)
		clash |= again[k].biome != w[k].biome || again[k].navi != w[k].navi || again[k].meets != w[k].meets || again[k].route != w[k].route;
	run.route_found = 0;
	return clash;
}

/* A fork a special route turns: its net, its act (0-based), its name */
typedef struct { int mode, act; const char *what; } RouteFork;

/* One fork of routes_report's over 500 runs: the route's way's guardians
 * counted (navi, version), how many past the act's band (not the endless
 * Nest's), clashing, or not offered; its line written, the three's sum
 * returned */
static int route_fork(FILE *out, const RouteFork *f) {
	int lo, hi, seen[32][6] = { { 0 } }, outside = 0, clash = 0, none = 0, act = f->act;
	pacing_guardian_band(act, &lo, &hi);
	bool banded = !(f->mode == RUN_ENDLESS && act == 6);
	for (uint32_t seed = 1; seed <= 500; ++seed) {
		run_new(seed * 2654435761u);
		run_setup(f->mode, 0, 0, 0, 0);
		RunWay w[3];
		bool sealed;
		run.route_found = (uint8_t)act;
		int n = run_ways(act, false, w, &sealed);
		run.route_found = 0;
		if (n < 2 || !w[n - 1].route) { ++none; continue; }
		/* (the endless Nest's at the Cybeast's HP: its fought HP the one counted) */
		int navi = w[n - 1].navi, v = cycle_version(navi, act, !banded), hp = navi_hp(navi, v);
		if (!banded) hp = run_nest_hp(CYCLE_LAYERS, navi, hp);
		if (navi >= 0 && navi < 32 && v >= 0 && v < 6) seen[navi][v]++;
		outside += banded && (hp < lo || hp > hi);
		clash += route_clash(w, n, act);
	}
	if (banded) fprintf(out, "%s (band %d-%d):", f->what, lo, hi);
	else fprintf(out, "%s (its strongest version at the Cybeast's HP, %d, at least):", f->what, run_nest_hp(CYCLE_LAYERS, 1, 0));
	for (int i = 0; i < 32 * 6; ++i) {
		int n = i / 6, v = i % 6, hp = navi_hp(n, v);
		if (seen[n][v]) fprintf(out, " %s%s %d (%d)", guardian(n)->name, v == 1 ? "EX" : v == 2 ? "SP" : "", banded ? hp : run_nest_hp(CYCLE_LAYERS, n, hp), seen[n][v]);
	}
	if (none) fprintf(out, "  none in %d runs", none);
	if (outside) fprintf(out, "  OUTSIDE %d", outside);
	if (clash) fprintf(out, "  CLASH %d", clash);
	fprintf(out, "\n");
	return outside + clash + none;
}

/* A special route's ways (docs/LIBERATION.md, Special routes) over 500
 * runs: the third way it opens at the endless net's act 4 fork (from act
 * 3's board), the Nest's second way after either net's last fork; each
 * guardian, version and HP, how often, past the act's band (not the
 * endless Nest's, where every version stands) or clashing */
static int routes_report(FILE *out) {
	static const RouteFork forks[] = {
		{ RUN_ENDLESS, 3, "the endless net's act 4 fork, a third way" },
		{ RUN_ENDLESS, 6, "the endless net's Nest, a second way" },
		{ RUN_SHORT, 3, "the short net's Nest, a second way" },
	};
	int flagged = 0;
	fprintf(out, "\nSpecial routes over 500 runs: the way a route opens, its guardian, version and HP, how often (OUTSIDE its act's band, CLASH\n"
		"another way's or act's guardian or area, or not the same twice).\n");
	for (unsigned f = 0; f < sizeof forks / sizeof *forks; ++f) flagged += route_fork(out, &forks[f]);
	return flagged;
}

/* The super bosses (docs/BOSSES.md, Super bosses): BN6's own battles,
 * above the bands by design, each form's HP where it stands, the last
 * act's band beside */
static void super_report(FILE *out) {
	int lo, hi;
	pacing_guardian_band(5, &lo, &hi);
	fprintf(out, "\nSuper bosses, BN6's own battles above the bands (act 6's %d-%d): where, each form's HP.\n", lo, hi);
	fprintf(out, "  the endless net's Nest: Gregar %d on the first Net, Gregar SP %d after\n", navi_hp(SUPER_CYBEAST, super_cybeast_form(0)),
		navi_hp(SUPER_CYBEAST, super_cybeast_form(1)));
	fprintf(out, "  the Secret Area once cleared: Bass %d, Bass SP %d once beaten, Bass BX %d twice beaten with the Cybeast\n",
		navi_hp(SUPER_BASS, super_bass_form(0, false)), navi_hp(SUPER_BASS, super_bass_form(1, false)), navi_hp(SUPER_BASS, super_bass_form(2, true)));
}

/* The Net's clock (docs/HOME.md, going back): each act's guardians as the
 * first cycle draws them, their median HP, at 0 to 6 notches (a trip back
 * each), beside the next act's band: how many trips back make an act's
 * guardian a next act's */
static void clock_report(FILE *out) {
	fprintf(out, "\nThe Net's clock: the median guardian's HP by act (500 runs) at 0-6 notches, %d%% more a notch; the next act's band.\n",
		RUN_CLOCK_PERCENT);
	for (int act = 0; act < 7; ++act) {
		static int hps[500];
		for (uint32_t seed = 1; seed <= 500; ++seed) {
			run_new(seed * 2654435761u);
			int b = act < 6 ? run.biome_order[act] : BIOME_NEST, navi = run.boss_order[b];
			hps[seed - 1] = navi_hp(navi, cycle_version(navi, act, b == BIOME_NEST));
		}
		qsort(hps, 500, sizeof *hps, cmp_int);
		fprintf(out, "act %d:", act + 1);
		for (int n = 0; n <= 6; ++n) fprintf(out, " %d", pacing_clock_hp(hps[250], n));
		int lo, hi;
		pacing_guardian_band(act + 1 < 7 ? act + 1 : 6, &lo, &hi);
		if (act < 5) fprintf(out, "  (act %d's band %d-%d)", act + 2, lo, hi);
		fprintf(out, "\n");
	}
}

/* Requests at home (docs/HOME.md, piece 5): each kind's terms and pay by
 * act, beside the act's guardian's zenny and the Net Dealer's HPMemory
 * price (shop.c), the prices they are measured by */
static void jobs_report(FILE *out) {
	static const char *const kinds[JOB_KINDS] = { "", "clean wins", "quick wins", "every Mystery Data", "a chip handed over", "a vow" };
	fprintf(out, "\nRequests at home: what each kind asks and pays by act; the act's guardian's zenny and HPMemory price beside.\n");
	for (int act = 0; act < 7; ++act) {
		fprintf(out, "act %d:", act + 1);
		for (int kind = JOB_CLEAN; kind < JOB_KINDS; ++kind) {
			Job j;
			memset(&j, 0, sizeof j);
			jobs_dev(&j, 1, act * 3 + 1, false, kind, 0);
			if (kind <= JOB_QUICK) fprintf(out, " %s x%d", kinds[kind], j.need);
			else fprintf(out, " %s%s", kinds[kind], kind == JOB_VOW && act == 0 ? " (not at a run's first visit)" : "");
			if (j.pay_kind == PAY_ZENNY) fprintf(out, " -> %u zenny;", j.pay);
			else if (j.pay_kind == PAY_BUGFRAGS) fprintf(out, " -> %u BugFrags;", j.pay);
			else if (j.pay_kind == PAY_CHIP) fprintf(out, " -> a chip a tier up;");
			else fprintf(out, " -> %u HPMemory;", j.pay);
		}
		fprintf(out, " guardian %d, HPMemory %d\n", 100 * (6 + 3 * act), 100 * (8 + 4 * act));
	}
}

/* Home's shops (docs/HOME.md, piece 6): the SubChip seller's stock by act
 * (the act ahead's keys first), and the Order Service's price beside a
 * Net Dealer's, by the chips' tiers */
static void home_shop_report(FILE *out) {
	static const char *const tiers[] = { "common", "uncommon", "rare", "mega", "legendary" };
	fprintf(out, "\nHome's shops: the SubChip seller's keys for the act ahead (id x stock at price; beside them BN6's MiniEnrg, FullEnrg, "
		"SneakRun and Untrap at its prices) by act; an order at twice a Net Dealer's price, one a visit:");
	for (int t = 0; t < 5; ++t) fprintf(out, " %s %d", tiers[t], 2 * 100 * (5 << t));
	fprintf(out, "\n");
	for (int act = 0; act < 7; ++act) {
		ShopItem items[SHOP_MAX_ITEMS];
		int n = shop_home_subs(act * 3 + 1, items);
		fprintf(out, "act %d:", act + 1);
		for (int i = 0; i < n; ++i) fprintf(out, " %02x x%d at %d", items[i].id, items[i].stock, items[i].price * 100);
		fprintf(out, "\n");
	}
}

/* The acts (a bit each, 0-based) BN6 area `biome` can come in on the first
 * cycle: the four of the surface by their pools (pacing_area_pool), the
 * Undernet's and the Graveyard's */
static unsigned area_acts(int biome) {
	unsigned acts = 0;
	for (int act = 0; act < 4; ++act) {
		uint8_t pool[PACING_AREA_POOL];
		int n = pacing_area_pool(act, pool);
		for (int i = 0; i < n; ++i) acts |= (unsigned)(pool[i] == biome) << act;
	}
	return acts | (unsigned)(biome == BIOME_UNDERNET) << 4 | (unsigned)(biome == BIOME_GRAVEYARD) << 5;
}

/* One of BN5's guardians in act `act` (`where` the area or the deep): his
 * version, his own HP there and the HP he is fought at, against BN6's
 * guardians' band (threat 4's version beside it); 1 where fought outside */
static int older_guardian_act(FILE *out, int navi, int act, const char *where) {
	int hp[4], lo, hi;
	for (int v = 0; v < 4; ++v) hp[v] = xguardian_hp(navi, v);
	int v = pacing_xguardian_version(hp, act, 0, false), v4 = pacing_xguardian_version(hp, act, 0, true);
	pacing_guardian_band(act, &lo, &hi);
	int fought = pacing_xguardian_hp(hp[v], act, 0), fought4 = pacing_xguardian_hp(hp[v4], act, 0);
	bool outside = fought < lo || fought > hi || fought4 < lo || fought4 > hi;
	fprintf(out, "  %-12s %-11s act %d (%4d-%4d): V%d %4d, fought at %4d; threat 4 V%d %4d, at %4d%s%s\n", where, guardian(navi)->name, act + 1, lo,
		hi, v + 1, hp[v], fought, v4 + 1, hp[v4], fought4, xguardian_fightable(navi) ? "" : "  (not moved in: not fought)", outside ? "  OUTSIDE" : "");
	return outside;
}

/* BN5's guardians (docs/BOSSES.md, BN5's Navis), fought in BN6's engine:
 * each in every act his area's BN6 area can come in, and every one in the
 * deep's acts, where both nets' Navis share a pool by band (those whose
 * HP falls short of an act's band are not in its pool: none past it, by
 * the rule); how many fought outside their band */
static int older_guardians(FILE *out) {
	int flagged = 0;
	fprintf(out, "\nGuardians of the older net in BN6's engine, where its ROM is beside BN6's (half the runs its areas dress; the deep's\n"
		"pool by band): navi, act, the band, version, his own HP and as fought (capped at the band's top), on threat 4 too;\n"
		"OUTSIDE when fought outside the band.\n");
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *x = net_area_def(NET_AREAS + k);
		if (!x || x->held || x->xrom <= 0) continue;
		for (int j = 0; j < 2 && x->xguard[j]; ++j) {
			int navi = guardian_of_older(x->xguard[j]);
			unsigned acts = area_acts(x->like);
			for (int act = 0; act < 6; ++act)
				if (acts >> act & 1) flagged += older_guardian_act(out, navi, act, x->name);
			fprintf(out, "  %-12s %-11s later cycles: SP %d\n", x->name, guardian(navi)->name, xguardian_hp(navi, 3));
		}
	}
	for (int navi = GUARDIAN_OLDER_FIRST; navi <= GUARDIAN_OLDER_LAST; ++navi)
		for (int act = 4; act <= 5; ++act) {
			int hp[4], lo, hi;
			for (int v = 0; v < 4; ++v) hp[v] = xguardian_hp(navi, v);
			pacing_guardian_band(act, &lo, &hi);
			if (pacing_xguardian_hp(hp[pacing_xguardian_version(hp, act, 0, false)], act, 0) < lo) {
				fprintf(out, "  %-12s %-11s act %d: short of its band, not in its pool\n", "the deep", guardian(navi)->name, act + 1);
				continue;
			}
			flagged += older_guardian_act(out, navi, act, "the deep");
		}
	return flagged;
}

int pacing_report_run(const char *path) {
	/* (the other games' families, fields and Navis a run's core has, the
	 * engine's objects they run before them, installed as it starts, where
	 * their ROM is read: the territories' records fought in BN6's engine,
	 * issue #82) */
	if (XR[XROM_BN5_COLONEL_US].data) {
		if (!emu_init(R.data, ROM_SIZE)) return 1;
		xinstall_core(XVIRUS_FAMILIES, XNAVI_BN5);
	}
	FILE *out = fopen(path, "w");
	if (!out) return 1;
	static const struct { int act; int biomes[8]; } acts[] = {
		{ 0, { BIOME_CENTRAL, BIOME_COMP, BIOME_SEASIDE, -1 } },
		{ 1, { BIOME_CENTRAL, BIOME_COMP, BIOME_SEASIDE, BIOME_JUDGE_COMP, BIOME_GREEN, BIOME_GREEN_HP, BIOME_HOMEPAGE, BIOME_ROBOT_COMP } },
		{ 1, { BIOME_COMP_B, BIOME_SKY_HP, BIOME_AQUARIUM_COMP, -1 } },
		{ 2, { BIOME_JUDGE_COMP, BIOME_GREEN, BIOME_GREEN_HP, BIOME_HOMEPAGE, BIOME_COMP_B, BIOME_SKY_HP, BIOME_AQUARIUM_COMP, BIOME_ROBOT_COMP } },
		{ 2, { BIOME_SKY, BIOME_WEATHER_COMP, BIOME_ACDC_HP, BIOME_COPYBOT_COMP, -1 } },
		{ 2, { BIOME_UNDERNET, -1 } },   /* (the short net's dark way, docs/META.md) */
		{ 3, { BIOME_SKY, BIOME_WEATHER_COMP, BIOME_ACDC_HP, BIOME_COPYBOT_COMP, -1 } },
		{ 4, { BIOME_UNDERNET, -1 } },
		{ 5, { BIOME_GRAVEYARD, -1 } },
		{ 6, { BIOME_NEST, -1 } },
	};
	int flagged = 0;
	fprintf(out, "Random battles: HP of the viruses together (lowest / median / highest of %d rolls), the strongest hit,\n"
		"the highest version in each battle; OVER when a battle lies past the band or the cap. Where another game's\n"
		"area dresses one (its ROM beside BN6's), its own records in BN6's engine after it (native).\n", ROLLS);
	for (int loop = 0; loop < 2; ++loop)
		for (unsigned a = 0; a < sizeof acts / sizeof *acts; ++a)
			for (int k = 0; k < 8 && acts[a].biomes[k] >= 0; ++k)
				flagged += act_area(out, acts[a].act, loop, acts[a].biomes[k], 0xC0FFEEu + (uint32_t)(a * 97 + k));

	flagged += first_cycle_guardians(out) + older_guardians(out) + routes_report(out);
	if (XR[XROM_BN5_COLONEL_US].data) flagged += visitors_report(out);
	super_report(out);
	clock_report(out);
	jobs_report(out);
	home_shop_report(out);
	elements_report(out);
	/* what the Net Dealers answer each act with, per element (every one a
	 * straight hit; a "+" is over the act's cap, the lightest found) */
	fprintf(out, "\nNet Dealers' answers (%d layers each: chip power share):\n", ROLLS);
	for (int act = 0; act < 6; ++act) {
		int depth = act * 3 + 2, lo, hi;
		pacing_guardian_band(act, &lo, &hi);
		for (int e = 0; e < ELEM_COUNT; ++e) {
			int counter = e ? e : -1, most = counter > 0 ? lo / 6 : lo / 3;
			static int ids[ROLLS];
			int none = 0;
			for (int k = 0; k < ROLLS; ++k) {
				rng_seed(0xDEA1u + (uint32_t)(act * 977 + e * 131 + k));
				char code;
				ids[k] = shop_dealer_answer(depth, counter, &code);
				none += ids[k] < 0;
			}
			qsort(ids, ROLLS, sizeof *ids, cmp_int);
			fprintf(out, "act %d %s (cap %d):", act + 1, e ? elem_name(e) : "any", most);
			for (int k = 0; k < ROLLS;) {
				int j = k;
				while (j < ROLLS && ids[j] == ids[k]) ++j;
				if (ids[k] >= 0 && (j - k) * 100 >= ROLLS * 5) {
					ChipInfo ci;
					chip_info(ids[k], &ci);
					fprintf(out, " %s %d%s %d%%", ci.name, ci.power, ci.power > most ? "+" : "", (j - k) * 100 / ROLLS);
				}
				k = j;
			}
			if (none) fprintf(out, " (none %d%%)", none * 100 / ROLLS);
			fprintf(out, "\n");
		}
	}
	drafts_report(out);
	fclose(out);
	printf("pacing report in %s: %d battles or guardians past their band\n", path, flagged);
	return 0;
}

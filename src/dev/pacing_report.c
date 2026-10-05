/* The pacing report (docs/DEVTOOLS.md): for every act of the first cycles
 * and every area it can visit, the random battles the engine rolls (virus
 * HP together, the strongest hit, versions) against the act's band, and the
 * guardians runs draw, from the ROM without the game. */
#include "pacing_report.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "game.h"
#include "guardians.h"
#include "guest.h"
#include "loot.h"
#include "net.h"
#include "navicust.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "shop.h"
#include "xguardian.h"

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
	int over = 0, navis = 0, maxdmg = 0, versions[6] = { 0 }, families[32] = { 0 }, gems = 0, blocks = 0;
	for (int r = 0; r < ROLLS; ++r) {
		Encounter e = make_encounter(depth, biome, kind);
		loot_battle_fought(&e);   /* each roll as a battle fought */
		/* (the virus families a battle brings, each counted once) */
		unsigned in = 0;
		for (int i = 0; i < e.nfoes; ++i)
			if (e.foes[i].kind == FOE_VIRUS && e.foes[i].family > 0 && e.foes[i].family < 32) in |= 1u << e.foes[i].family;
		for (int f = 1; f < 32; ++f) families[f] += (in >> f) & 1;
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
	if (navis) fprintf(out, "  SP navi %d", navis);
	fprintf(out, "  families");
	for (int f = 1; f < 32; ++f) if (families[f]) fprintf(out, " %d:%d", f, families[f]);
	fprintf(out, "%s\n", over ? "  OVER" : "");
	return over;
}

/* The territories (docs/MULTIROM.md, Guest battles): where another game's
 * area dresses the BN6 area an act visits, its random battles are its own
 * game's records, from the pool the director picks from (guest_pool, each
 * record as likely, its viruses taken up to the act's version): their HP
 * together (lowest / median / highest), the strongest hit, how many under
 * the band's floor, the versions they went up, the pool's size and where
 * it came from; returns its records past the cap (only where none fits and
 * the map's weakest stands in). */
static int guest_battles(FILE *out, int depth, const NetAreaDef *x, int xrom, bool opening) {
	PacingBand b = pacing_band(depth, false, opening);
	int vlo, vhi;
	pacing_virus_versions(depth, false, &vlo, &vhi);
	GuestBand band = { b.lo, b.hi, b.cap, vhi };
	const uint8_t *xb = x->xbattles[layer_in_act(depth)];
	if (!xb[0]) return 0;
	static uint32_t pool[192];
	static uint8_t ups[192];
	static int hps[192];
	bool fits = false;
	int n = guest_pool(xrom, x, xb[0], xb[1], &band, pool, ups, 192, &fits), over = 0, under = 0, maxdmg = 0, own = 0, up = 0;
	uint32_t first = guest_record(xrom, xb[0], xb[1], 0), end = first + 16u * (uint32_t)guest_records(xrom, xb[0], xb[1]);
	for (int i = 0; i < n; ++i) {
		int hp, dmg;
		guest_record_scaled(xrom, pool[i], (GuestScale){ ups[i], band.vcap }, &hp, &dmg);
		hps[i] = hp;
		if (dmg > maxdmg) maxdmg = dmg;
		if (hp > band.hi || dmg > band.cap) ++over;
		under += hp < band.lo;
		up += ups[i];
		own += pool[i] >= first && pool[i] < end;
	}
	qsort(hps, (size_t)n, sizeof *hps, cmp_int);
	fprintf(out, "  depth %2d %-9s band %3d-%3d cap %3d | hp %3d / %3d / %3d  hit %3d  %d records of %s (map %02x:%d), %d under the floor, "
		"%.1f versions up%s\n", depth, opening ? "opening" : "guest", band.lo, band.hi, band.cap, n ? hps[0] : 0, n ? hps[n / 2] : 0,
		n ? hps[n - 1] : 0, maxdmg, n, !fits ? "its weakest, none fitting" : own == n ? "its map" : "other maps", xb[0], xb[1], under,
		n ? (double)up / n : 0.0, over ? "  OVER" : "");
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
		fprintf(out, "  %s's battles in its own engine where it dresses it:\n", x->name);
		int first = loop * CYCLE_LAYERS + act * 3 + 1, last = act == 6 ? first : first + 2;
		for (int d = first; d <= last; ++d) {
			flagged += guest_battles(out, d, x, x->xrom - 1, true);
			flagged += guest_battles(out, d, x, x->xrom - 1, false);
		}
	}
	return flagged;
}

/* One area in one act of a cycle: its random battles rolled (from `seed`),
 * then its territory's, where another game's area dresses it; how many
 * past their band */
static int act_area(FILE *out, int act, int loop, int b, uint32_t seed) {
	int flagged = 0, first = loop * CYCLE_LAYERS + act * 3 + 1, last = act == 6 ? first : first + 2;
	fprintf(out, "\nact %d cycle %d  %s\n", act + 1, loop + 1, guardian_area_name(b));
	rng_seed(seed);
	for (int d = first; d <= last; ++d) flagged += battles(out, d, b, ENC_NORMAL, "battle");
	flagged += battles(out, first, b, ENC_EASY, "opening");
	if (first == 1) flagged += battles(out, first, b, ENC_FIRST, "first");
	if (act < 6) flagged += battles(out, first + 1, b, ENC_CHALLENGE, "challenge");
	return flagged + territories(out, act, loop, b);
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

/* The guardians a first cycle draws, over many runs (BN6's: run_new's
 * picks); how many outside their act's band */
static int first_cycle_guardians(FILE *out) {
	int flagged = 0;
	fprintf(out, "\nGuardians of the first cycle over 500 runs: navi, version and HP, how often.\n");
	for (int act = 0; act < 7; ++act) {
		int lo, hi, seen[32][3] = { { 0 } }, outside = 0;
		pacing_guardian_band(act, &lo, &hi);
		for (uint32_t seed = 1; seed <= 500; ++seed) {
			run_new(seed * 2654435761u);
			int b = act < 6 ? run.biome_order[act] : BIOME_NEST, navi = run.boss_order[b];
			int v = pacing_guardian_version(navi, act, 0, b == BIOME_NEST, navi_hp), hp = navi_hp(navi, v);
			if (navi >= 0 && navi < 32) seen[navi][v]++;
			if (act < 6 && (hp < lo || hp > hi)) ++outside;
		}
		fprintf(out, "act %d (band %d-%d):", act + 1, lo, act < 6 ? hi : 0);
		for (int n = 0; n < 32; ++n)
			for (int v = 0; v < 3; ++v)
				if (seen[n][v]) fprintf(out, " %s%s %d (%d)", guardian(n)->name, v == 1 ? "EX" : v == 2 ? "SP" : "", navi_hp(n, v), seen[n][v]);
		fprintf(out, "%s\n", outside ? "  OUTSIDE" : "");
		flagged += outside;
	}
	return flagged;
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
			hps[seed - 1] = navi_hp(navi, pacing_guardian_version(navi, act, 0, b == BIOME_NEST, navi_hp));
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

/* BN5's guardians (docs/BOSSES.md, BN5's Navis): each in every act his
 * area's BN6 area can come in, his version, his own HP there and the HP he
 * is fought at, against the act's band (threat 4's version beside it);
 * how many fought outside it */
static int older_guardians(FILE *out) {
	int flagged = 0;
	fprintf(out, "\nGuardians of the older net, where its ROM is beside BN6's (half the runs its areas dress): navi, act, the band, version,\n"
		"his own HP and as fought (capped at the band's top), on threat 4 too; OUTSIDE when fought outside the band.\n");
	for (int k = 0; k < XAREAS_MAX; ++k) {
		const NetAreaDef *x = net_area_def(NET_AREAS + k);
		if (!x || x->held || x->xrom <= 0) continue;
		for (int j = 0; j < 2 && x->xguard[j]; ++j) {
			int navi = guardian_of_older(x->xguard[j]), hp[4];
			for (int v = 0; v < 4; ++v) hp[v] = xguardian_hp(navi, v);
			unsigned acts = area_acts(x->like);
			for (int act = 0; act < 6; ++act) {
				if (!(acts >> act & 1)) continue;
				int lo, hi, v = pacing_xguardian_version(hp, act, 0, false), v4 = pacing_xguardian_version(hp, act, 0, true);
				pacing_xguardian_band(act, &lo, &hi);
				int fought = pacing_xguardian_hp(hp[v], act, 0), fought4 = pacing_xguardian_hp(hp[v4], act, 0);
				bool outside = fought < lo || fought > hi || fought4 < lo || fought4 > hi;
				fprintf(out, "  %-12s %-11s act %d (%4d-%4d): V%d %4d, fought at %4d; threat 4 V%d %4d, at %4d%s\n", x->name, guardian(navi)->name,
					act + 1, lo, hi, v + 1, hp[v], fought, v4 + 1, hp[v4], fought4, outside ? "  OUTSIDE" : "");
				flagged += outside;
			}
			fprintf(out, "  %-12s %-11s later cycles: SP %d\n", x->name, guardian(navi)->name, hp[3]);
		}
	}
	return flagged;
}

int pacing_report_run(const char *path) {
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
		"area dresses one (its ROM beside BN6's), its battles in its own engine after it: the records a layer's\n"
		"first battle (opening) and the others (guest) are picked from, each as likely.\n", ROLLS);
	for (int loop = 0; loop < 2; ++loop)
		for (unsigned a = 0; a < sizeof acts / sizeof *acts; ++a)
			for (int k = 0; k < 8 && acts[a].biomes[k] >= 0; ++k)
				flagged += act_area(out, acts[a].act, loop, acts[a].biomes[k], 0xC0FFEEu + (uint32_t)(a * 97 + k));

	flagged += first_cycle_guardians(out) + older_guardians(out);
	clock_report(out);
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

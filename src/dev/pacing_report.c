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
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"

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
	int over = 0, navis = 0, maxdmg = 0, versions[6] = { 0 };
	for (int r = 0; r < ROLLS; ++r) {
		Encounter e = make_encounter(depth, biome, kind);
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
	if (navis) fprintf(out, "  SP navi %d", navis);
	fprintf(out, "%s\n", over ? "  OVER" : "");
	return over;
}

int pacing_report_run(const char *path) {
	FILE *out = fopen(path, "w");
	if (!out) return 1;
	static const struct { int act; int biomes[8]; } acts[] = {
		{ 0, { BIOME_CENTRAL, BIOME_ROBOT_COMP, BIOME_AQUARIUM_COMP, BIOME_SKY_HP, BIOME_COMP, -1 } },
		{ 1, { BIOME_CENTRAL, BIOME_ROBOT_COMP, BIOME_AQUARIUM_COMP, BIOME_SKY_HP, BIOME_COMP, BIOME_SEASIDE, BIOME_JUDGE_COMP, BIOME_GREEN } },
		{ 1, { BIOME_GREEN_HP, BIOME_HOMEPAGE, BIOME_COMP_B, -1 } },
		{ 2, { BIOME_SEASIDE, BIOME_JUDGE_COMP, BIOME_GREEN, BIOME_GREEN_HP, BIOME_HOMEPAGE, BIOME_COMP_B, BIOME_SKY, BIOME_WEATHER_COMP } },
		{ 2, { BIOME_ACDC_HP, BIOME_COPYBOT_COMP, -1 } },
		{ 3, { BIOME_SKY, BIOME_WEATHER_COMP, BIOME_ACDC_HP, BIOME_COPYBOT_COMP, -1 } },
		{ 4, { BIOME_UNDERNET, -1 } },
		{ 5, { BIOME_GRAVEYARD, -1 } },
		{ 6, { BIOME_NEST, -1 } },
	};
	int flagged = 0;
	fprintf(out, "Random battles: HP of the viruses together (lowest / median / highest of %d rolls), the strongest hit,\n"
		"the highest version in each battle; OVER when a battle lies past the band or the cap.\n", ROLLS);
	for (int loop = 0; loop < 2; ++loop)
		for (unsigned a = 0; a < sizeof acts / sizeof *acts; ++a) {
			for (int k = 0; k < 8 && acts[a].biomes[k] >= 0; ++k) {
				int b = acts[a].biomes[k];
				fprintf(out, "\nact %d cycle %d  %s\n", acts[a].act + 1, loop + 1, guardian_area_name(b));
				int first = loop * CYCLE_LAYERS + acts[a].act * 3 + 1, last = acts[a].act == 6 ? first : first + 2;
				rng_seed(0xC0FFEEu + (uint32_t)(a * 97 + k));
				for (int d = first; d <= last; ++d) flagged += battles(out, d, b, ENC_NORMAL, "battle");
				flagged += battles(out, first, b, ENC_EASY, "opening");
				if (acts[a].act < 6) flagged += battles(out, first + 1, b, ENC_CHALLENGE, "challenge");
			}
		}

	/* the guardians a first cycle draws, over many runs */
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
	fclose(out);
	printf("pacing report in %s: %d battles or guardians past their band\n", path, flagged);
	return 0;
}

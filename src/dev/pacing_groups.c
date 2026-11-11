/* pacing_report.h: the encounter groups against their limits
 * (docs/game/progression/encounter-groups.md; groups.h, group_pool.h). */
#include "pacing_report.h"

#include <math.h>
#include <stdlib.h>

#include "data.h"
#include "group_pool.h"
#include "groups.h"
#include "guardians.h"
#include "loot.h"
#include "pacing.h"
#include "run.h"
#include "visitor_families.h"

#define ROLLS 300   /* group battles rolled an act's area and layer */

static const struct { const char *name; unsigned nets; } SETS[] = {
	{ "BN6", 1 }, { "BN6+BN5", 3 }, { "BN6+BN4", 5 }, { "BN6+BN5+BN4", 7 },
};

/* an act's depth at its middle layer, the Nest's its one */
static int act_depth(int act) { return act < 6 ? act * 3 + 2 : CYCLE_LAYERS; }

/* (a ROM set's pool by act, and how many ways each template fills) */
static void set_report(FILE *out, const GroupFam *pool, int n, const char *name, unsigned nets) {
	fprintf(out, "\n%s: the pool by act", name);
	for (int act = 0; act < PACING_ACTS; ++act) {
		GroupLimits l = group_limits(act_depth(act), loot_stage_allowed(act_depth(act)), nets);
		int in = 0;
		for (int i = 0; i < n; ++i) in += group_in_pool(&pool[i], &l);
		fprintf(out, " %d", in);
	}
	fprintf(out, "\n");
	for (int t = 0; t < GROUP_TEMPLATES; ++t) {
		fprintf(out, "  %-20s", group_template_name(t));
		for (int act = 0; act < PACING_ACTS; ++act) {
			GroupLimits l = group_limits(act_depth(act), loot_stage_allowed(act_depth(act)), nets);
			fprintf(out, " %5d", group_fillings(pool, n, &l, t, NULL, NULL));
		}
		fprintf(out, "\n");
	}
}

/* A group battle's own numbers, read again from the ROM's records: its HP
 * together, its hardest hit and the pressure its families' tags predict
 * (-1 where one has none) */
static void measure(const Encounter *e, int *hp, int *hit, int *pressure) {
	double sum = 0;
	*hp = *hit = 0;
	*pressure = 0;
	for (int i = 0; i < e->nfoes; ++i) {
		int h, d, h1, d1, net = visitor_net(e->foes[i].family), v1 = visitor_id(net, e->foes[i].family, 0);
		const GroupTag *tag = group_tag(net, e->foes[i].family);
		if (!enemy_stats(e->foes[i].id, &h, &d) || !enemy_stats(v1, &h1, &d1) || !tag || tag->pressure < 0) {
			*pressure = -1;
			return;
		}
		*hp += h;
		if (d > *hit) *hit = d;
		sum += (double)(tag->pressure * d) / (double)(d1 > 1 ? d1 : 1);
	}
	*pressure = (int)lrint(sum);
}

/* whether two of its foes share a panel, or one stands on a panel that
 * is neither plain nor cracked */
static bool misplaced(const Encounter *e) {
	for (int i = 0; i < e->nfoes; ++i) {
		int p = visitor_panel(e, e->foes[i].col, e->foes[i].row);
		if (p != 2 && p != 3) return true;
		for (int k = 0; k < i; ++k)
			if (e->foes[k].col == e->foes[i].col && e->foes[k].row == e->foes[i].row) return true;
	}
	return false;
}

static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

/* ROLLS group battles at `depth` in area `b`: each against the band, the
 * cap on one hit and the pressure cap; how many lie past one */
static int rolls(FILE *out, int depth, int b, uint32_t seed) {
	static int hps[ROLLS];
	GroupLimits l = group_limits(depth, loot_stage_allowed(depth), group_nets());
	int n = 0, over = 0, top = 0, most = 0, by[GROUP_TEMPLATES] = { 0 }, fields = 0;
	for (int k = 0; k < ROLLS; ++k) {
		Encounter e = { 0 };
		int hp, hit, pressure;
		e.biome = b;
		if (!group_battle(depth, b, seed + (uint32_t)k / 8, k % 8, -1, &e)) continue;
		measure(&e, &hp, &hit, &pressure);
		bool bad = hp < l.band.lo || hp > l.band.hi || hit > l.band.cap || pressure < 0 || pressure > l.pressure || misplaced(&e);
		over += bad;
		hps[n++] = hp;
		if (hit > top) top = hit;
		if (pressure > most) most = pressure;
		fields += e.field != 0;
		if (e.from_pick >= GROUP_FROM_PICK && e.from_pick < GROUP_FROM_PICK + GROUP_TEMPLATES) ++by[e.from_pick - GROUP_FROM_PICK];
	}
	if (!n) {
		fprintf(out, "  depth %2d %-14s no group fills (band %d-%d, cap %d, pressure %d)\n", depth, guardian_area_name(b), l.band.lo, l.band.hi, l.band.cap, l.pressure);
		return 1;
	}
	qsort(hps, (size_t)n, sizeof *hps, cmp_int);
	fprintf(out, "  depth %2d %-14s HP %4d / %4d / %4d (band %d-%d)  hit %3d (cap %d)  pressure %4d (cap %d)  on an area's field %3d%%  templates",
		depth, guardian_area_name(b), hps[0], hps[n / 2], hps[n - 1], l.band.lo, l.band.hi, top, l.band.cap, most, l.pressure, fields * 100 / n);
	for (int t = 0; t < GROUP_TEMPLATES; ++t) fprintf(out, " %d", by[t]);
	fprintf(out, "%s\n", over ? "  OVER" : "");
	return over;
}

int pacing_groups_report(FILE *out) {
	const GroupFam *pool;
	int n = group_pool(&pool), flagged = 0;
	unsigned have = group_nets();
	fprintf(out, "\nEncounter groups (docs/game/progression/encounter-groups.md; behind --dev groups, no run holds one): %d families\n"
		"in the pool. Each ROM set's pool by act (acts 1 to 6, the Nest) and how many ways each template fills, a group held\n"
		"to the act's band, its cap on one hit and its pressure cap (%d %d %d %d %d %d, the Nest %d).\n", n,
		group_pressure_cap(0), group_pressure_cap(1), group_pressure_cap(2), group_pressure_cap(3), group_pressure_cap(4), group_pressure_cap(5),
		group_pressure_cap(6));
	for (unsigned s = 0; s < sizeof SETS / sizeof *SETS; ++s)
		if ((SETS[s].nets & have) == SETS[s].nets) set_report(out, pool, n, SETS[s].name, SETS[s].nets);
	fprintf(out, "\nGroup battles rolled with the ROMs read (%d a line: the layer's seed and the battle's index): HP lowest / median /\n"
		"highest, the hardest hit, the most pressure predicted, the share on a field of the area's own, and each template's\n"
		"count in the order above; OVER where one lies past a limit, shares a panel or stands on a special one.\n", ROLLS);
	for (int act = 0; act < PACING_ACTS; ++act) {
		uint8_t areas[PACING_AREA_POOL] = { act == 4 ? BIOME_UNDERNET : act == 5 ? BIOME_GRAVEYARD : BIOME_NEST };
		int na = act < 4 ? pacing_area_pool(act, areas) : 1, first = act * 3 + 1, last = act == 6 ? first : first + 2;
		for (int i = 0; i < na; ++i)
			for (int d = first; d <= last; ++d) flagged += rolls(out, d, areas[i], 0x6709u + (uint32_t)(act * 977 + i * 131 + d));
	}
	/* (a later cycle's limits: the cap by the band's own growth, groups.h) */
	for (int act = 0; act < PACING_ACTS; act += 3)
		flagged += rolls(out, CYCLE_LAYERS + act * 3 + 1, act == 6 ? BIOME_NEST : act == 3 ? BIOME_SKY : BIOME_CENTRAL, 0x6710u + (uint32_t)act);
	return flagged;
}

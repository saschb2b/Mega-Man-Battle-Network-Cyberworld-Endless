/* pacing_report.h: the encounter groups against their limits
 * (docs/game/progression/encounter-groups.md; groups.h, group_pool.h). */
#include "pacing_report.h"

#include <math.h>
#include <stdlib.h>

#include "chip_entry.h"
#include "data.h"
#include "group_pool.h"
#include "groups.h"
#include "guardians.h"
#include "loot.h"
#include "pacing.h"
#include "game.h"
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
static void measure(const Encounter *e, int *hp, int *hit, int *pressure, int *cost) {
	double sum = 0, spent = 0, scale[MAX_FOES] = { 0 };
	int net[MAX_FOES] = { 0 };
	*hp = *hit = 0;
	*pressure = *cost = 0;
	for (int i = 0; i < e->nfoes; ++i) {
		int h, d, h1, d1;
		net[i] = visitor_net(e->foes[i].family);
		const GroupTag *tag = group_tag(net[i], e->foes[i].family);
		if (!enemy_stats(e->foes[i].id, &h, &d) || !enemy_stats(visitor_id(net[i], e->foes[i].family, 0), &h1, &d1) || !tag || tag->pressure < 0 ||
			tag->cost < 0) {
			*pressure = *cost = -1;
			return;
		}
		*hp += h;
		if (d > *hit) *hit = d;
		scale[i] = (double)d / (double)(d1 > 1 ? d1 : 1);
		sum += (double)(tag->pressure * d) / (double)(d1 > 1 ? d1 : 1);
		spent += (double)(tag->cost * d) / (double)(d1 > 1 ? d1 : 1);
	}
	*cost = (int)lrint(spent);
	/* (two the pairs' pass found pressing harder together: their excess, by the larger scale) */
	for (int i = 0; i < e->nfoes; ++i)
		for (int k = i + 1; k < e->nfoes; ++k) {
			int extra = e->foes[i].family == e->foes[k].family ? 0 : group_pair_extra(net[i], e->foes[i].family, net[k], e->foes[k].family);
			if (extra) sum += extra * (scale[i] > scale[k] ? scale[i] : scale[k]);
		}
	*pressure = (int)lrint(sum);
}

/* whether its field is not fair to a group, two of its foes share a
 * panel, or one stands on a panel that is neither plain nor cracked */
static bool misplaced(const Encounter *e) {
	if (!group_field_fair(e->field)) return true;
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
		int hp, hit, pressure, cost;
		e.biome = b;
		if (!group_battle(depth, b, seed + (uint32_t)k / 8, k % 8, -1, &e)) continue;
		measure(&e, &hp, &hit, &pressure, &cost);
		bool bad = hp < l.band.lo || hp > l.band.hi || hit > l.band.cap || pressure < 0 || pressure > l.pressure || cost > l.cost || misplaced(&e);
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
		"to the act's band, its cap on one hit, its pressure cap (%d %d %d %d %d %d, the Nest %d) and its cap on cost (%d %d %d %d %d %d, %d).\n", n,
		group_pressure_cap(0), group_pressure_cap(1), group_pressure_cap(2), group_pressure_cap(3), group_pressure_cap(4), group_pressure_cap(5),
		group_pressure_cap(6), group_cost_cap(0), group_cost_cap(1), group_cost_cap(2), group_cost_cap(3), group_cost_cap(4), group_cost_cap(5),
		group_cost_cap(6));
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

/* ---- the acts' arcs (group_arc.h) ---- */

/* What reward rows pay, tallied over the rows' healthy halves (entries 0
 * to 9, MegaMan's HP over three eighths: docs/rom/battle.md, Battle
 * rewards): the rows, their chip entries, their zenny, and the kinds of
 * chip among them */
typedef struct { int rows, chips, zenny; uint8_t kind[2048 / 8]; } Pays;

static void pays_row(Pays *p, int id) {
	++p->rows;
	for (int k = 0; k < ENEMY_DROPS / 2; ++k) {
		uint16_t v = enemy_drop(id, k);
		if (reward_is_chip(v)) {
			unsigned chip = reward_chip_id(v) & 2047;
			++p->chips;
			p->kind[chip >> 3] |= (uint8_t)(1u << (chip & 7));
		} else if (v >> 14 == 1) p->zenny += v & 0x3FFF;
	}
}

static int pays_kinds(const Pays *p) {
	int n = 0;
	for (unsigned i = 0; i < sizeof p->kind * 8; ++i) n += p->kind[i >> 3] >> (i & 7) & 1;
	return n;
}

#define ARCS 200   /* runs' arcs rolled an act's area */

/* One beat of an arc as its battle, against limits `l`: 1 where it lies
 * past one (HP over the top, or under the floor where `floor`; the hit;
 * the pressure; a panel), its pressure into *most where it is more */
typedef struct { uint32_t seed; int area; Pays *pays; FILE *list; } ArcRun;

/* (a beat's battle listed once, as the `foes` step takes it: enemy ids in hex at column and row from 1, for
 * tools/encounter_danger.py battles; act 1's alone, whose beats are few enough to fight each) */
static void beat_listed(FILE *out, const Encounter *e, int beat) {
	static uint32_t seen[512];
	static int nseen;
	uint32_t h = 2166136261u;
	for (int i = 0; i < e->nfoes; ++i) h = (h ^ (uint32_t)(e->foes[i].id << 8 | e->foes[i].col << 4 | e->foes[i].row)) * 16777619u;
	for (int i = 0; i < nseen; ++i)
		if (seen[i] == h) return;
	if (nseen >= 512) return;
	seen[nseen++] = h;
	fprintf(out, "      beat %d:", beat);
	for (int i = 0; i < e->nfoes; ++i) fprintf(out, "%s%X@%d%d", i ? "/" : " ", (unsigned)e->foes[i].id, e->foes[i].col + 1, e->foes[i].row + 1);
	fprintf(out, "\n");
}

static int beat_over(const ArcRun *run_, int depth, bool challenge, const GroupLimits *l, bool floor, int *most) {
	Encounter e = { 0 };
	int hp, hit, pressure, cost;
	e.biome = run_->area;
	if (!group_arc_battle(run_->seed, depth, run_->area, challenge, &e)) return 1;
	if (!challenge && loot_paying_id(&e)) pays_row(run_->pays, loot_paying_id(&e));
	if (run_->list && !challenge) beat_listed(run_->list, &e, e.arc_beat);
	measure(&e, &hp, &hit, &pressure, &cost);
	if (pressure > *most) *most = pressure;
	return (floor && hp < l->band.lo) || hp > l->band.hi || hit > l->band.cap || pressure < 0 || pressure > l->pressure || cost > l->cost ||
		misplaced(&e) || !e.signature;
}

/* ARCS runs' arcs of act `act` (0-5, 6 the Nest; cycle `loop`) in area `b` */
static int arcs(FILE *out, int act, int loop, int b) {
	int first = loop * CYCLE_LAYERS + act * 3 + 1, stage = loot_stage_allowed(first), made = 0, home = 0, over = 0, challenges = 0;
	int most[ARC_BEATS + 1] = { 0 }, seen[GROUP_POOL_MAX] = { 0 }, kinds = 0;
	Pays arc_pays = { 0, 0, 0, { 0 } }, own_pays = { 0, 0, 0, { 0 } };
	/* (what the area's own battles pay there: each roll's viruses' rows, as BN6's pick takes any of them) */
	rng_seed(0xA7C6u + (uint32_t)(first * 131 + b));
	for (int k = 0; k < ARCS; ++k) {
		Encounter own = make_encounter(first + (act == 6 ? 0 : 1), b, ENC_NORMAL);
		for (int i = 0; i < own.nfoes; ++i)
			if (own.foes[i].kind == FOE_VIRUS && own.foes[i].id > 0) pays_row(&own_pays, own.foes[i].id);
	}
	unsigned nets = group_nets();
	GroupLimits eased = group_limits_kind(first, stage, nets, GROUP_EASED), plain = group_limits_kind(first, stage, nets, GROUP_PLAIN),
		challenge = group_limits_kind(first, stage, nets, GROUP_CHALLENGE);
	if (act == 0 && loop == 0) plain = eased;   /* (act 1 holds every beat to the eased limits: group_pool.c rules_at) */
	for (int k = 0; k < ARCS; ++k) {
		uint32_t seed = 0xA7C5u + (uint32_t)k * 2654435761u;
		GroupArc arc;
		if (!group_arc_at(seed, first, b, &arc)) continue;
		++made;
		home += arc.home;
		kinds += !seen[arc.signature]++;
		ArcRun of = { seed, b, &arc_pays, act == 0 && loop == 0 ? out : NULL };
		if (act == 6) {
			over += beat_over(&of, first, false, &plain, true, &most[ARC_CONSTELLATION]);
			continue;
		}
		over += beat_over(&of, first, false, &eased, false, &most[ARC_ALONE]);
		over += beat_over(&of, first + 1, false, &plain, act > 0 || loop > 0, &most[ARC_PAIR]);
		over += beat_over(&of, first + 2, false, &plain, act > 0 || loop > 0, &most[ARC_CONSTELLATION]);
		if (arc.has_challenge) {
			++challenges;
			over += beat_over(&of, first + 1, true, &challenge, true, &most[ARC_BEATS]);
		}
	}
	fprintf(out, "  act %d%s %-14s arcs %3d of %d  of the area's own %3d%%  signatures %2d  pressure alone %4d (cap %d) pair %4d constellation %4d (cap %d)"
		"  challenge %3d%% %4d (cap %d)%s\n", act + 1, loop ? " cycle 2" : "", guardian_area_name(b), made, ARCS, made ? home * 100 / made : 0, kinds,
		most[ARC_ALONE], eased.pressure, most[ARC_PAIR], most[ARC_CONSTELLATION], plain.pressure, made ? challenges * 100 / made : 0, most[ARC_BEATS],
		challenge.pressure, over ? "  OVER" : !made ? "  (no family can carry the act here)" : "");
	if (arc_pays.rows && own_pays.rows)
		fprintf(out, "      rows' healthy halves: the beats' chips %d.%d of 10 entries, zenny %d a row, %d kinds of chip; the area's own %d.%d, %d, %d\n",
			arc_pays.chips / arc_pays.rows, arc_pays.chips * 10 / arc_pays.rows % 10, arc_pays.zenny / arc_pays.rows, pays_kinds(&arc_pays),
			own_pays.chips / own_pays.rows, own_pays.chips * 10 / own_pays.rows % 10, own_pays.zenny / own_pays.rows, pays_kinds(&own_pays));
	return over;
}

int pacing_arcs_report(FILE *out) {
	int flagged = 0;
	fprintf(out, "\nThe acts' arcs (group_arc.h; behind --dev arc, no run holds one): %d runs' arcs an act's area, by their seeds. How\n"
		"many acts have one, the share whose signature is one of the area's own (three quarters of act 1's, half, a third, a\n"
		"quarter; the deep any family), how many families were a signature, the most pressure of each beat against its cap, and\n"
		"the Server's challenge (the share of arcs with one, its most pressure); OVER where a beat lies past a limit.\n", ARCS);
	for (int act = 0; act < PACING_ACTS; ++act) {
		uint8_t areas[PACING_AREA_POOL] = { act == 4 ? BIOME_UNDERNET : act == 5 ? BIOME_GRAVEYARD : BIOME_NEST };
		int na = act < 4 ? pacing_area_pool(act, areas) : 1;
		for (int i = 0; i < na; ++i) flagged += arcs(out, act, 0, areas[i]);
	}
	flagged += arcs(out, 0, 1, BIOME_CENTRAL) + arcs(out, 3, 1, BIOME_SKY);
	return flagged;
}

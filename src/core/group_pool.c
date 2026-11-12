/* group_pool.h: the encounter groups' pool, read from the ROMs. */
#include "group_pool.h"

#include <string.h>

#include "data.h"
#include "formations.h"
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "visitor_families.h"
#include "xbattles.h"
#include "xvirus.h"
#include "xvirus_bn4.h"

static struct {
	const uint8_t *rom6, *rom5, *rom4;
	int kin[GROUP_NETS];
	bool made;
	int made_n;      /* how many times it was made: what was counted from it holds for one */
	int n;
	unsigned nets;
	GroupFam fam[GROUP_POOL_MAX];
} P;

/* (net `net`'s family `family` into the pool, unless its game binds it to
 * special panels) */
static void add(int net, int family) {
	const VisitorKin *kin;
	int nk = visitor_kin(net, &kin);
	if (P.n >= GROUP_POOL_MAX) return;
	GroupFam *f = &P.fam[P.n];
	*f = (GroupFam){ (int16_t)family, (int8_t)net, GROUP_FREE, 0, 0, -1, { { 0 } }, -1, { { 0, 0 }, { 0, 0 } } };
	for (int i = 0; i < nk; ++i) {
		if (kin[i].family != family) continue;
		if (kin[i].bound) return;
		f->lean = (int8_t)group_lean(kin[i].cols[0], kin[i].cols[1], kin[i].cols[2]);
		f->stage = kin[i].stage;
	}
	const GroupTag *tag = group_tag(net, family);
	if (tag) {
		f->pressure = tag->pressure;
		f->tricks = tag->tricks;
		f->cost = tag->cost;
		f->multi[0] = tag->multi[0];
		f->multi[1] = tag->multi[1];
	}
	bool any = false;
	for (int v = 0; v < GROUP_VERSIONS; ++v) {
		int id = visitor_id(net, family, v), hp, hit, elem;
		f->ver[v] = (GroupVer){ -1, 0, 0, 0 };
		if (net == VISIT_BN6 && v > 3) id = enemy_id(0, family, v);   /* (its two rare ones, which no act's version reaches) */
		if (id < 0 || !enemy_stats(id, &hp, &hit)) continue;
		elem = enemy_element(id);
		f->ver[v] = (GroupVer){ (int16_t)id, (int16_t)hp, (int16_t)hit, (int8_t)(elem >= ELEM_FIRE && elem <= ELEM_WOOD ? elem : 0) };
		any = true;
	}
	if (!any) return;
	P.nets |= 1u << net;
	++P.n;
}

static void make(void) {
	const VisitorKin *kin;
	P.rom6 = R.data;
	P.rom5 = XR[XROM_BN5_COLONEL_US].data;
	P.rom4 = XR[XROM_BN4_RED_SUN_US].data;
	for (int net = 0; net < GROUP_NETS; ++net) P.kin[net] = visitor_kin(net, &kin);
	P.made = true;
	++P.made_n;
	P.n = 0;
	P.nets = 0;
	if (!R.data) return;
	for (int ai = 1; ai <= 29; ++ai)
		if (enemy_id(0, ai, 0) >= 0) add(VISIT_BN6, ai);
	for (int ai = 1; P.rom5 && ai < 64; ++ai) {
		const XVirusFamily *f = xvirus_installed(XROM_BN5_COLONEL_US, ai);
		if (f && !f->summoned) add(VISIT_BN5, f->ai);
	}
	for (int i = 0; P.rom4 && i < xvirus_bn4_count; ++i) {
		const XVirusFamily *f = xvirus_installed(XROM_BN4_RED_SUN_US, xvirus_bn4[i]->src_ai);
		if (f && !f->summoned) add(VISIT_BN4, f->ai);
	}
}

/* (made again where the ROMs changed, or the families came in since: the
 * nets' kin, which visitor_families.c makes again then, counts them) */
static void ready(void) {
	const VisitorKin *kin;
	bool same = P.made && P.rom6 == R.data && P.rom5 == XR[XROM_BN5_COLONEL_US].data && P.rom4 == XR[XROM_BN4_RED_SUN_US].data;
	for (int net = 0; net < GROUP_NETS && same; ++net) same = P.kin[net] == visitor_kin(net, &kin);
	if (!same) make();
}

int group_pool(const GroupFam **out) {
	ready();
	*out = P.fam;
	return P.n;
}

unsigned group_nets(void) {
	ready();
	/* (a run without the older nets: BN6's families alone, as its finds are, docs/game/meta/extras.md) */
	return run_extra(EXTRA_OLDER) ? P.nets : P.nets & 1u;
}

static uint32_t mix(uint32_t k) {
	k ^= k >> 16;
	k *= 0x7FEB352Du;
	k ^= k >> 15;
	k *= 0x846CA68Bu;
	return k ^ (k >> 16);
}

/* whether every foe of `e` stands on a plain or a cracked panel of field
 * `field` (BN6's) */
static bool stands_plain(Encounter *e, int field) {
	e->field = field;
	for (int i = 0; i < e->nfoes; ++i) {
		int p = visitor_panel(e, e->foes[i].col, e->foes[i].row);
		if (p != 2 && p != 3) return false;
	}
	return true;
}

/* The group's field: one of area `biome`'s own battles' fields that is
 * fair to a group (groups.h's group_field_fair) and its foes all stand
 * plain or cracked on, each such field as likely as any, by `roll`; else
 * the plain field */
static void field_of(Encounter *e, int biome, uint32_t roll) {
	const Formation *list;
	int n = formations_of(biome, &list), fields[64], m = 0;
	for (int i = 0; i < n && m < 64; ++i) {
		bool again = false;
		for (int k = 0; k < m && !again; ++k) again = fields[k] == list[i].battlefield;
		if (!again && group_field_fair(list[i].battlefield) && stands_plain(e, list[i].battlefield)) fields[m++] = list[i].battlefield;
	}
	e->field = m ? fields[mix(roll ^ 0x4649454Cu) % (uint32_t)m] : 0;   /* ("FIEL") */
}

/* How many ways the pool fills each template under `l`, counted once for
 * the limits a layer's battles share (kept till they or the pool change) */
static const int *counts_under(const GroupFam *pool, int n, const GroupLimits *l) {
	static GroupLimits kept;
	static int counts[GROUP_TEMPLATES], made = -1;
	bool same = made == P.made_n && kept.band.lo == l->band.lo && kept.band.hi == l->band.hi && kept.band.cap == l->band.cap &&
		kept.stage == l->stage && kept.nets == l->nets && kept.pressure == l->pressure;
	for (int k = 0; k < GROUP_NETS && same; ++k) same = kept.top[k] == l->top[k];
	if (same) return counts;
	kept = *l;
	made = P.made_n;
	for (int t = 0; t < GROUP_TEMPLATES; ++t) counts[t] = group_fillings(pool, n, l, t, NULL, NULL);
	return counts;
}

/* Battle `e` made the group `g`'s: its foes on the panels, the area's own
 * battle's objects and record let go, its field (field_of), and what the
 * repeat rule keeps of it once fought (loot_battle_fought: no formation's
 * number, its families and its viruses) */
static void battle_of(const GroupFam *pool, const GroupFill *g, int biome, uint32_t roll, Encounter *e) {
	Encounter out = *e;
	out.xrecord = 0;
	out.nobj = 0;
	out.gem_own = false;
	out.player = 0;
	out.sides = 0;
	out.arc_beat = out.signature = 0;
	for (int i = 0; i < MAX_FOES; ++i) out.foes[i] = (Foe){ FOE_VIRUS, 0, 0, 0, 0, -1 };
	group_place(pool, g, &out);
	uint32_t ids = 2166136261u;
	for (int i = 0; i < out.nfoes; ++i) {
		const uint8_t *row = enemy_triple(out.foes[i].id);
		if (row) out.foes[i].version = row[0];
		ids = (ids ^ (uint32_t)out.foes[i].id) * 16777619u;
	}
	out.group = 1 + g->template;
	out.from_biome = biome;
	out.from_pick = GROUP_FROM_PICK + g->template;
	out.from_viruses = ids;
	field_of(&out, biome < BIOME_COUNT ? biome : -1, roll);
	*e = out;
}

bool group_battle(int depth, int biome, uint32_t layer_seed, int index, int only, Encounter *e) {
	const GroupFam *pool;
	int n = group_pool(&pool);
	GroupLimits l = group_limits(depth, loot_stage_allowed(depth), group_nets());
	GroupFill g;
	uint32_t roll = mix(layer_seed ^ (uint32_t)index * 0x9E3779B9u);
	const int *counts = counts_under(pool, n, &l);
	int t = only < 0 ? group_template_roll(counts, roll) : only;
	if (t < 0 || t >= GROUP_TEMPLATES || !group_pick_counted(pool, n, &l, t, counts[t], roll, &g)) return false;
	battle_of(pool, &g, biome, roll, e);
	return true;
}

/* ---- an act's arc (group_arc.h) ---- */

/* The area's own virus families: a BN6 area's from its random battles'
 * formations; an older net's area's (loot.h's loot_arc_territory: the act's, outside the
 * deep) from its records as BN6's engine fights them at `depth` */
static FamilyMask home_of(int depth, int biome) {
	FamilyMask in = FAMILY_MASK_NONE;
	if (loot_deep[0] < 0 && loot_arc_territory >= 0) return xterritory_families(depth, loot_arc_territory, NULL);
	const Formation *list;
	int n = formations_of(biome, &list);
	for (int i = 0; i < n; ++i)
		for (int k = 0; k < list[i].n; ++k) {
			const uint8_t *row = enemy_triple(list[i].ent[k].id);
			if (row && row[1] == 0 && row[2] >= 1 && row[2] <= 29) family_mask_add(&in, row[2]);
		}
	return in;
}

/* The arc's rules at `depth` in area `biome`: the limits of the act's
 * layers (one band an act), the area's own families, the share by act.
 * Act 1 of the first cycle holds every beat to the eased limits (the
 * band's lower half, two thirds of the caps): its MegaMan has 100 HP, and
 * a pair inside the full band deleted the test autopilot from full HP in
 * three runs of twelve; with the lower half no group battle did
 * (docs/game/progression/encounter-groups.md, Easing act 1's pair). */
static ArcRules rules_at(int depth, int biome) {
	int act = pacing_act(depth), stage = loot_stage_allowed(depth);
	unsigned nets = group_nets();
	ArcRules r = { group_limits_kind(depth, stage, nets, GROUP_EASED), group_limits_kind(depth, stage, nets, GROUP_PLAIN),
		group_limits_kind(depth, stage, nets, GROUP_CHALLENGE), home_of(depth, biome), arc_home_share(act), act >= 2 };
	if (act == 0 && pacing_loop(depth) == 0) r.plain = r.eased;
	return r;
}

bool group_arc_at(uint32_t run_seed, int depth, int biome, GroupArc *out) {
	static struct { uint32_t seed; int act, biome, made_n, territory; bool ok; GroupArc arc; } kept = { 0, -1, 0, 0, 0, false, { 0 } };
	const GroupFam *pool;
	int n = group_pool(&pool), act = pacing_loop(depth) * PACING_ACTS + pacing_act(depth);
	if (kept.act != act || kept.seed != run_seed || kept.biome != biome || kept.made_n != P.made_n || kept.territory != loot_arc_territory) {
		/* (under the rules of the act's first layer, whichever layer asks: group_arc.h's group_arc_depth) */
		ArcRules r = rules_at(group_arc_depth(depth), biome);
		kept.seed = run_seed;
		kept.act = act;
		kept.biome = biome;
		kept.made_n = P.made_n;
		kept.territory = loot_arc_territory;
		kept.ok = group_arc_make(pool, n, &r, mix(run_seed ^ 0x41524353u ^ (uint32_t)act * 0x9E3779B9u), &kept.arc);   /* ("ARCS") */
	}
	*out = kept.arc;
	return kept.ok;
}

/* (the first of `e`'s foes of the arc's signature family, 1 + its index) */
static int signature_foe(const GroupFam *pool, const GroupArc *arc, const Encounter *e) {
	for (int i = 0; i < e->nfoes; ++i)
		if (e->foes[i].family == pool[arc->signature].family) return i + 1;
	return 0;
}

bool group_arc_battle(uint32_t run_seed, int depth, int biome, bool challenge, Encounter *e) {
	const GroupFam *pool;
	GroupArc arc;
	group_pool(&pool);
	if (!group_arc_at(run_seed, depth, biome, &arc) || (challenge && !arc.has_challenge)) return false;
	/* (the Nest's one layer is its constellation's) */
	int beat = pacing_act(depth) == 6 ? ARC_CONSTELLATION : ((depth - 1) % CYCLE_LAYERS) % 3;
	battle_of(pool, challenge ? &arc.challenge : &arc.beat[beat], biome, mix(run_seed ^ (uint32_t)depth * 0x9E3779B9u), e);
	e->arc_beat = 1 + (challenge ? ARC_BEATS : beat);
	e->signature = signature_foe(pool, &arc, e);
	return true;
}

/* groups.h: the encounter groups' templates and their filling. */
#include "groups.h"

#include <math.h>
#include <stddef.h>

int group_lean(int front, int middle, int back) {
	int n = front + middle + back, most = front > middle ? front : middle;
	if (back > most) most = back;
	if (n <= 0) return GROUP_FREE;
	if (back * 5 >= n * 2 && back >= most) return GROUP_BACK;
	return front * 5 >= n * 2 && front >= most ? GROUP_FRONT : GROUP_FREE;
}

const GroupTag *group_tag(int net, int family) {
	for (int i = 0; i < group_tags_count; ++i)
		if (group_tags[i].net == net && group_tags[i].family == family) return &group_tags[i];
	return NULL;
}

bool group_field_fair(int field) { return field >= 0 && field < 256 && (group_fair_fields[field >> 3] >> (field & 7) & 1); }

int group_pair_extra(int net_a, int family_a, int net_b, int family_b) {
	for (int i = 0; i < group_pairs_count; ++i) {
		const GroupPair *p = &group_pairs[i];
		if ((p->net_a == net_a && p->family_a == family_a && p->net_b == net_b && p->family_b == family_b) ||
			(p->net_a == net_b && p->family_a == family_b && p->net_b == net_a && p->family_b == family_a))
			return p->extra;
	}
	return 0;
}

/* ---- the templates: what a family must be to take each slot, and what
 * the group must be at the versions fought ---- */

enum { SLOT_ANY, SLOT_FRONT, SLOT_BACK, SLOT_FREE, SLOT_BREAKS, SLOT_SHAPES, SLOT_SUMMONS };
enum { TEST_NONE, TEST_ADJACENT, TEST_SAME, TEST_GLASS, TEST_BULWARK, TEST_NETS };

static const struct { const char *name; uint8_t n, slot[GROUP_SIZE], test; } TEMPLATES[GROUP_TEMPLATES] = {
	[GROUP_WALL] = { "wall and artillery", 2, { SLOT_FRONT, SLOT_BACK }, TEST_NONE },
	[GROUP_PANEL_WAR] = { "panel war", 2, { SLOT_BREAKS, SLOT_FREE }, TEST_NONE },
	[GROUP_SUMMONER] = { "summoner and guard", 2, { SLOT_SUMMONS, SLOT_FRONT }, TEST_NONE },
	[GROUP_CLASH] = { "clashing answers", 2, { SLOT_ANY, SLOT_ANY }, TEST_ADJACENT },
	[GROUP_ONE_ELEMENT] = { "one element", 2, { SLOT_ANY, SLOT_ANY }, TEST_SAME },
	[GROUP_PINCER] = { "pincer", 3, { SLOT_FRONT, SLOT_BACK, SLOT_FREE }, TEST_NONE },
	[GROUP_SWARM] = { "swarm", 1, { SLOT_ANY }, TEST_NONE },
	[GROUP_HELD_GROUND] = { "held ground", 2, { SLOT_SHAPES, SLOT_BACK }, TEST_NONE },
	[GROUP_GLASS] = { "glass cannons", 2, { SLOT_ANY, SLOT_ANY }, TEST_GLASS },
	[GROUP_BULWARK] = { "bulwark", 2, { SLOT_ANY, SLOT_ANY }, TEST_BULWARK },
	[GROUP_NETS_MEET] = { "nets meet", 2, { SLOT_FREE, SLOT_FREE }, TEST_NETS },
};

const char *group_template_name(int t) { return t >= 0 && t < GROUP_TEMPLATES ? TEMPLATES[t].name : t == GROUP_ALONE ? "alone" : "?"; }

static bool slot_takes(int slot, const GroupFam *f) {
	switch (slot) {
	case SLOT_FRONT: return f->lean == GROUP_FRONT || (f->tricks & GROUP_TOUGH);
	case SLOT_BACK: return f->lean == GROUP_BACK;
	case SLOT_FREE: return f->lean == GROUP_FREE;
	case SLOT_BREAKS: return f->tricks & GROUP_BREAKS;
	case SLOT_SHAPES: return f->tricks & GROUP_SHAPES;
	case SLOT_SUMMONS: return f->tricks & GROUP_SUMMONS;
	default: return true;
	}
}

/* (BN6's first wheel: fire burns wood, wood grounds elec, elec shocks
 * aqua, aqua douses fire; data.h's ELEM_FIRE 1, ELEM_AQUA 2, ELEM_ELEC 3,
 * ELEM_WOOD 4) the element that hits `e` twice as hard, 0 none */
static int beaten_by(int e) {
	static const int8_t by[5] = { 0, 2, 3, 4, 1 };
	return e >= 1 && e <= 4 ? by[e] : 0;
}

/* a group at its versions: the families and each one's version there */
typedef struct { const GroupFam *f[GROUP_SIZE]; int v[GROUP_SIZE], n; } At;

static const GroupVer *ver(const At *a, int i) { return &a->f[i]->ver[a->v[i]]; }

static bool test_passes(int test, const At *a, const GroupLimits *l) {
	int e0 = ver(a, 0)->elem, e1 = a->n > 1 ? ver(a, 1)->elem : 0, hp = 0;
	for (int i = 0; i < a->n; ++i) hp += ver(a, i)->hp;
	switch (test) {
	case TEST_ADJACENT: return e0 && e1 && (beaten_by(e0) == e1 || beaten_by(e1) == e0);
	case TEST_SAME: return e0 && e0 == e1;
	case TEST_GLASS: return ver(a, 0)->hit * 4 >= l->band.cap * 3 && ver(a, 1)->hit * 4 >= l->band.cap * 3 && hp * 2 <= l->band.lo + l->band.hi;
	case TEST_BULWARK: return ver(a, 0)->hp * 5 >= hp * 3 && ver(a, 0)->hit * 2 <= l->band.cap && ver(a, 1)->hit * 4 >= l->band.cap * 3;
	case TEST_NETS: return a->f[0]->net != a->f[1]->net;
	default: return true;
	}
}

/* its pressure as its families' own add up (each one's alone at V1, by
 * its hit at the version fought over its V1's), and with two the pairs'
 * pass found pressing harder together, that excess by the larger of
 * their two scales; -1 where one was not measured */
static int pressure_of(const At *a) {
	double sum = 0, scale[GROUP_SIZE];
	for (int i = 0; i < a->n; ++i) {
		const GroupFam *f = a->f[i];
		if (f->pressure < 0) return -1;
		int v1 = f->ver[0].hit > 1 ? f->ver[0].hit : 1;
		scale[i] = (double)ver(a, i)->hit / (double)v1;
		sum += (double)(f->pressure * ver(a, i)->hit) / (double)v1;
	}
	for (int i = 0; i < a->n; ++i)
		for (int k = i + 1; k < a->n; ++k) {
			int extra = a->f[i] == a->f[k] ? 0 : group_pair_extra(a->f[i]->net, a->f[i]->family, a->f[k]->net, a->f[k]->family);
			if (extra) sum += extra * (scale[i] > scale[k] ? scale[i] : scale[k]);
		}
	return (int)lrint(sum);
}

/* what it costs, as its families' own costs add up (each one's alone at V1, by its hit at the version fought over its
 * V1's); -1 where one was not measured */
static int cost_of(const At *a) {
	double sum = 0;
	for (int i = 0; i < a->n; ++i) {
		const GroupFam *f = a->f[i];
		if (f->cost < 0) return -1;
		sum += (double)(f->cost * ver(a, i)->hit) / (double)(f->ver[0].hit > 1 ? f->ver[0].hit : 1);
	}
	return (int)lrint(sum);
}

/* whether the group at its versions keeps to the cap on what it costs (none set: any) */
static bool cost_fits(const At *a, const GroupLimits *l) {
	int c = l->cost == GROUP_NO_COST ? 0 : cost_of(a);
	return c >= 0 && c <= l->cost;
}

/* whether version `v` of `f` may stand in a battle under `l` */
static bool version_fits(const GroupFam *f, int v, const GroupLimits *l) {
	const GroupVer *x = &f->ver[v];
	return x->id >= 0 && v <= l->top[f->net] && x->hit <= l->band.cap && x->hp <= l->band.hi;
}

bool group_in_pool(const GroupFam *f, const GroupLimits *l) {
	if (f->net < 0 || f->net >= GROUP_NETS || !(l->nets >> f->net & 1) || f->stage > l->stage) return false;
	for (int v = 0; v < GROUP_VERSIONS; ++v)
		if (version_fits(f, v, l)) return true;
	return false;
}

/* The group's versions inside the band: the most HP together from its
 * floor to its top, the template's test passed, the pressure at most the
 * cap; the first such where several tie (the last family's version
 * moving fastest). False where none. */
static bool fit(At *a, int test, const GroupLimits *l, GroupFill *out) {
	int vs[GROUP_SIZE][GROUP_VERSIONS], nv[GROUP_SIZE], at[GROUP_SIZE] = { 0 }, keep[GROUP_SIZE] = { 0 }, best = -1, kept = 0;
	for (int i = 0; i < a->n; ++i) {
		nv[i] = 0;
		for (int v = 0; v < GROUP_VERSIONS; ++v)
			if (version_fits(a->f[i], v, l)) vs[i][nv[i]++] = v;
		if (!nv[i]) return false;
	}
	for (;;) {
		int hp = 0;
		for (int i = 0; i < a->n; ++i) {
			a->v[i] = vs[i][at[i]];
			hp += ver(a, i)->hp;
		}
		if (hp >= l->band.lo && hp <= l->band.hi && hp > best && test_passes(test, a, l)) {
			int p = pressure_of(a);
			if (p >= 0 && p <= l->pressure && cost_fits(a, l)) {
				best = hp;
				kept = p;
				for (int i = 0; i < a->n; ++i) keep[i] = a->v[i];
			}
		}
		int k = a->n - 1;
		while (k >= 0 && ++at[k] == nv[k]) at[k--] = 0;
		if (k < 0) break;
	}
	if (best < 0) return false;
	out->hp = best;
	out->pressure = kept;
	for (int i = 0; i < a->n; ++i) {
		out->ver[i] = (int8_t)keep[i];
		a->v[i] = keep[i];
	}
	out->cost = cost_of(a);
	return true;
}

/* ---- every filling of a template ---- */

typedef struct {
	const GroupFam *pool;
	const GroupLimits *l;
	int t, count, most;   /* (most: the walk ends once it has counted as many) */
	void (*each)(const GroupFill *g, void *ctx);
	void *ctx;
} Walk;

/* (the group of pool families `idx`, tried and counted where it fits) */
static bool try_group(Walk *w, const int *idx, int n) {
	At a = { { NULL }, { 0 }, n };
	GroupFill g = { (int8_t)w->t, (int8_t)n, { 0 }, { 0 }, 0, 0, 0 };
	for (int i = 0; i < n; ++i) {
		a.f[i] = &w->pool[idx[i]];
		g.fam[i] = (int16_t)idx[i];
	}
	if (!fit(&a, TEMPLATES[w->t].test, w->l, &g)) return false;
	++w->count;
	if (w->each) w->each(&g, w->ctx);
	return true;
}

bool group_same(const GroupFam *pool, int i, int count, bool floor, const GroupLimits *l, GroupFill *out) {
	const GroupFam *f = &pool[i];
	if (count < 1 || count > GROUP_SIZE || !group_in_pool(f, l)) return false;
	for (int v = GROUP_VERSIONS - 1; v >= 0; --v) {
		if (!version_fits(f, v, l)) continue;
		At a = { { f, f, f }, { v, v, v }, count };
		int hp = count * f->ver[v].hp, p = pressure_of(&a);
		if ((floor && hp < l->band.lo) || hp > l->band.hi || p < 0 || p > l->pressure || !cost_fits(&a, l)) continue;
		*out = (GroupFill){ (int8_t)(count == 3 ? GROUP_SWARM : GROUP_ALONE), (int8_t)count, { (int16_t)i, (int16_t)i, (int16_t)i },
			{ (int8_t)v, (int8_t)v, (int8_t)v }, hp, p, cost_of(&a) };
		return true;
	}
	return false;
}

/* (three of one family: its highest version whose three keep to the band
 * and the cap) */
static void swarm(Walk *w, int i) {
	GroupFill g;
	if (!group_same(w->pool, i, 3, true, w->l, &g)) return;
	++w->count;
	if (w->each) w->each(&g, w->ctx);
}

/* (the walk: every filling in its fixed order, to `w->most` of them) */
static void walk(Walk *w, int n) {
	const uint8_t *slot = TEMPLATES[w->t].slot;
	const GroupFam *pool = w->pool;
	int in[GROUP_POOL_MAX], m = 0, size = TEMPLATES[w->t].n;
	for (int i = 0; i < n; ++i)
		if (group_in_pool(&pool[i], w->l)) in[m++] = i;
	if (w->t == GROUP_SWARM) {
		for (int a = 0; a < m && w->count < w->most; ++a) swarm(w, in[a]);
		return;
	}
	bool alike = size == 2 && slot[0] == slot[1];
	for (int a = 0; a < m; ++a) {
		if (!slot_takes(slot[0], &pool[in[a]])) continue;
		for (int b = alike ? a + 1 : 0; b < m; ++b) {
			if (b == a || !slot_takes(slot[1], &pool[in[b]])) continue;
			int idx[GROUP_SIZE] = { in[a], in[b], 0 }, back[GROUP_SIZE] = { in[b], in[a], 0 };
			/* (alike slots: a pair once, either way round where the first does not fit) */
			if (size == 2 && !try_group(w, idx, 2) && alike) try_group(w, back, 2);
			for (int c = 0; size == 3 && c < m && w->count < w->most; ++c) {
				if (c == a || c == b || !slot_takes(slot[2], &pool[in[c]])) continue;
				idx[2] = in[c];
				try_group(w, idx, 3);
			}
			if (w->count >= w->most) return;
		}
	}
}

int group_fillings(const GroupFam *pool, int n, const GroupLimits *l, int t, void (*each)(const GroupFill *g, void *ctx), void *ctx) {
	if (t < 0 || t >= GROUP_TEMPLATES || n > GROUP_POOL_MAX) return 0;
	Walk w = { pool, l, t, 0, 0x7FFFFFFF, each, ctx };
	walk(&w, n);
	return w.count;
}

static uint32_t mix(uint32_t k) {
	k ^= k >> 16;
	k *= 0x7FEB352Du;
	k ^= k >> 15;
	k *= 0x846CA68Bu;
	return k ^ (k >> 16);
}

/* (the walk's last filling kept) */
static void last(const GroupFill *g, void *ctx) { *(GroupFill *)ctx = *g; }

bool group_pick_counted(const GroupFam *pool, int n, const GroupLimits *l, int t, int count, uint32_t roll, GroupFill *out) {
	if (t < 0 || t >= GROUP_TEMPLATES || n > GROUP_POOL_MAX || count <= 0) return false;
	Walk w = { pool, l, t, 0, 1 + (int)(mix(roll ^ 0x47525550u) % (uint32_t)count), last, out };   /* ("GRUP") */
	walk(&w, n);
	return w.count == w.most;
}

bool group_pick(const GroupFam *pool, int n, const GroupLimits *l, int t, uint32_t roll, GroupFill *out) {
	return group_pick_counted(pool, n, l, t, group_fillings(pool, n, l, t, NULL, NULL), roll, out);
}

int group_template_roll(const int counts[GROUP_TEMPLATES], uint32_t roll) {
	int fills[GROUP_TEMPLATES], m = 0;
	for (int t = 0; t < GROUP_TEMPLATES; ++t)
		if (counts[t] > 0) fills[m++] = t;
	return m ? fills[mix(roll ^ 0x54454D50u) % (uint32_t)m] : -1;   /* ("TEMP") */
}

bool group_pick_any(const GroupFam *pool, int n, const GroupLimits *l, uint32_t roll, GroupFill *out) {
	int counts[GROUP_TEMPLATES];
	for (int t = 0; t < GROUP_TEMPLATES; ++t) counts[t] = group_fillings(pool, n, l, t, NULL, NULL);
	int t = group_template_roll(counts, roll);
	return t >= 0 && group_pick_counted(pool, n, l, t, counts[t], roll, out);
}

/* ---- the act's limits ---- */

int group_pressure_cap(int act) {
	static const int16_t cap[PACING_ACTS] = { 220, 360, 620, 720, 880, 1040, 945 };
	return cap[act < 0 ? 0 : act >= PACING_ACTS ? PACING_ACTS - 1 : act];
}

int group_cost_cap(int act) {
	static const int16_t cap[PACING_ACTS] = { 0, 60, 145, 140, 180, 678, 447 };
	return cap[act < 0 ? 0 : act >= PACING_ACTS ? PACING_ACTS - 1 : act];
}

GroupLimits group_limits(int depth, int stage, unsigned nets) { return group_limits_kind(depth, stage, nets, GROUP_PLAIN); }

GroupLimits group_limits_kind(int depth, int stage, unsigned nets, int kind) {
	bool challenge = kind == GROUP_CHALLENGE;
	PacingBand plain = pacing_band(depth, false, false);
	GroupLimits l = { pacing_band(depth, challenge, kind == GROUP_EASED), { 0 }, (int8_t)stage, (uint8_t)(nets & GROUP_NETS_ALL), 0, GROUP_NO_COST };
	int vlo, vhi, act = pacing_act(depth);
	pacing_virus_versions(depth, challenge, &vlo, &vhi);
	l.top[0] = (int8_t)vhi;
	l.top[1] = l.top[2] = (int8_t)(2 * vhi + 1 > 5 ? 5 : 2 * vhi + 1);
	l.pressure = group_pressure_cap(act);
	l.cost = group_cost_cap(act);
	if (pacing_loop(depth) > 0) {
		/* (a later cycle: by the band's cap on one hit there over the first cycle's, at the act's middle layer) */
		int first = pacing_band(act < 6 ? act * 3 + 2 : 19, false, false).cap;
		if (first > 0) {
			l.pressure = l.pressure * plain.cap / first;
			l.cost = l.cost * plain.cap / first;
		}
	}
	/* (an eased battle: two thirds of the caps, about the median of the act's own battles as the caps are their
	 * three-quarter marks; a challenge: by the next act's cap on one hit over this one's, as its band is the next act's) */
	if (kind == GROUP_EASED) {
		l.pressure = l.pressure * 2 / 3;
		l.cost = l.cost * 2 / 3;
	}
	if (challenge && plain.cap > 0) {
		l.pressure = l.pressure * l.band.cap / plain.cap;
		l.cost = l.cost * l.band.cap / plain.cap;
	}
	return l;
}

/* ---- on the panels ---- */

void group_place(const GroupFam *pool, const GroupFill *g, Encounter *e) {
	static const int8_t rows[GROUP_SIZE] = { 1, 0, 2 };
	bool one = g->n > 1 && g->fam[0] == g->fam[1];
	e->nfoes = 0;
	e->from_families = FAMILY_MASK_NONE;
	for (int i = 0; i < g->n && i < GROUP_SIZE; ++i) {
		const GroupFam *f = &pool[g->fam[i]];
		int col = one ? 3 + i : f->lean == GROUP_FRONT ? 3 : f->lean == GROUP_BACK ? 5 : 4;
		e->foes[e->nfoes++] = (Foe){ FOE_VIRUS, f->family, g->ver[i], col, rows[i], f->ver[g->ver[i]].id };
		family_mask_add(&e->from_families, f->family);
	}
}

/* visitors.h: where the nets meet. */
#include "visitors.h"

#include "data.h"
#include "game.h"
#include "loot.h"
#include "pacing.h"
#include "run.h"
#include "visitor_families.h"

int visitor_role(int front, int middle, int back) {
	int n = front + middle + back;
	if (n <= 0) return VISITOR_FREE;
	if (4 * back >= 3 * n) return VISITOR_BACK;
	return 2 * front > n ? VISITOR_FRONT : VISITOR_FREE;
}

/* (the seed and the battle's four mixed: which of the four is the
 * visitor's) */
static uint32_t mix(uint32_t k) {
	k ^= k >> 16;
	k *= 0x7FEB352Du;
	k ^= k >> 15;
	k *= 0x846CA68Bu;
	return k ^ (k >> 16);
}

bool visitor_gate(uint32_t layer_seed, int index) {
	if (index < 0) return false;
	return (int)(mix(layer_seed ^ 0x56495349u ^ (uint32_t)(index / 4) * 0x9E3779B9u) & 3) == (index & 3);   /* ("VISI") */
}

bool visitors_here(int depth, int biome) {
	if (depth < 4 || (run.helpers & HELP_GENTLE) || !run_extra(EXTRA_OLDER)) return false;
	if (biome == BIOME_UNDERNET || biome == BIOME_GRAVEYARD || biome == BIOME_NEST) return false;
	const VisitorKin *k;
	return visitor_kin(VISIT_BN5, &k) > 0 || visitor_kin(VISIT_BN4, &k) > 0;
}

bool visitor_due(uint32_t layer_seed, int index, int depth, int biome, int kind) {
	return kind == ENC_NORMAL && visitors_here(depth, biome) && (loot_visitors_always || visitor_gate(layer_seed, index));
}

/* Whether a visitor of role `role` stands at column `col` (from 0): the
 * back's in the last, the front's in the first of the enemy's */
static bool stands(int role, int col) { return role == VISITOR_FREE || (role == VISITOR_BACK && col == 5) || (role == VISITOR_FRONT && col == 3); }

/* The highest version up to `top` of family `family` from net `net` with
 * which the battle (its HP `hp` without the foe replaced) keeps to the
 * band: into *id its enemy id; -1 none */
static int version_fit(int net, int family, int top, int hp, PacingBand b, int *id) {
	for (int v = top; v >= 0; --v) {
		int h, d, x = visitor_id(net, family, v);
		if (x < 0 || !enemy_stats(x, &h, &d) || hp + h > b.hi || d > b.cap) continue;
		*id = x;
		return v;
	}
	return -1;
}

/* The battle's viruses' HP together (into *hp), and its families (a bit
 * each) */
static uint64_t held_of(const Encounter *e, int *hp) {
	uint64_t held = 0;
	*hp = 0;
	for (int i = 0; i < e->nfoes; ++i) {
		int h, d;
		if (e->foes[i].kind == FOE_ROCK || !enemy_stats(e->foes[i].id, &h, &d)) continue;
		*hp += h;
		if (e->foes[i].family > 0 && e->foes[i].family < 64) held |= (uint64_t)1 << e->foes[i].family;
	}
	return held;
}

/* A visitor that fits: its net, its kin's index there, its version */
typedef struct { int net, k, ver; } Fit;
#define FITS_MAX (2 * VISITOR_KIN_MAX)

/* The visitors that fit in place of foe `i` (its family's role, a family
 * the battle does not hold, reached at `depth`, a version in the band),
 * of the other nets, BN5's or BN6's first, then BN4's (each family as
 * likely as any: docs/multirom/bn4.md, Its viruses as visitors), into fit;
 * how many */
static int fits_for(const Encounter *e, int i, int depth, Fit fit[FITS_MAX]) {
	int home = e->xrecord ? VISIT_BN5 : VISIT_BN6, hp, h, d;
	const Foe *f = &e->foes[i];
	int role = visitor_role_in(home, f->family), panel = visitor_panel(e, f->col, f->row);
	if (f->kind != FOE_VIRUS || (panel != 2 && panel != 3) || !stands(role, f->col) || !enemy_stats(f->id, &h, &d)) return 0;
	uint64_t held = held_of(e, &hp);
	PacingBand b = pacing_band(depth, false, false);
	int vlo, vhi;
	pacing_virus_versions(depth, false, &vlo, &vhi);
	int allowed = loot_stage_allowed(depth), m = 0;
	for (int from = 0; from < VISIT_NETS; ++from) {
		if (from == home) continue;
		/* (the act's version, BN5's and BN4's by tier: BN6's V1 their V1EX,
		 * and so on) */
		int top = from == VISIT_BN6 ? vhi : 2 * vhi + 1 > 5 ? 5 : 2 * vhi + 1;
		const VisitorKin *kin;
		int nk = visitor_kin(from, &kin);
		for (int k = 0; k < nk && m < FITS_MAX; ++k) {
			int id;
			if (!kin[k].visits || kin[k].role != role || kin[k].stage > allowed || (kin[k].family < 64 && (held >> kin[k].family & 1))) continue;
			int v = version_fit(from, kin[k].family, top, hp - h, b, &id);
			if (v >= 0) fit[m++] = (Fit){ from, k, v };
		}
	}
	return m;
}

bool visitor_apply(Encounter *e, int depth) {
	int order[MAX_FOES], n = 0;
	if (e->visitor || e->boss) return false;
	for (int i = 0; i < e->nfoes; ++i)
		if (e->foes[i].kind == FOE_VIRUS) order[n++] = i;
	if (!n) return false;
	int start = rng_range(0, n - 1);
	for (int t = 0; t < n; ++t) {
		Fit fit[FITS_MAX];
		int i = order[(start + t) % n], m = fits_for(e, i, depth, fit);
		if (!m) continue;
		const Fit *p = &fit[rng_range(0, m - 1)];
		const VisitorKin *kin;
		visitor_kin(p->net, &kin);
		const VisitorKin *k = &kin[p->k];
		int id = visitor_id(p->net, k->family, p->ver);
		const uint8_t *tr = enemy_triple(id);
		Foe *o = &e->foes[i];
		o->kind = FOE_VIRUS;
		o->family = k->family;
		o->version = tr ? tr[0] : p->ver;
		o->id = id;
		e->visitor = i + 1;
		/* (BN4's families, from 64, have no bit: no battle of a run's own
		 * holds one, so the repeat rule's memory needs none) */
		if (k->family < 64) e->from_families |= (uint64_t)1 << k->family;
		return true;
	}
	return false;
}

int visitor_telegraph(int home, int net) {
	bool older = net == VISIT_BN5 || net == VISIT_BN4;
	if (home == VISIT_BN6) return older ? VISITOR_OLDER_NETS : VISITOR_UNSAID;
	return home == VISIT_BN5 && net == VISIT_BN6 ? VISITOR_OURS : VISITOR_UNSAID;
}

int nets_battle_area(const int deep[2], int territory) {
	if (deep[0] < 0) return territory;
	if (rng_range(0, 1)) return -1;
	return deep[1] >= 0 && rng_range(0, 1) ? deep[1] : deep[0];
}

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
	if (depth < 4 || (run.helpers & HELP_GENTLE)) return false;
	if (biome == BIOME_UNDERNET || biome == BIOME_GRAVEYARD || biome == BIOME_NEST) return false;
	const VisitorKin *k;
	return visitor_kin(VISIT_BN5, &k) > 0;
}

bool visitor_due(uint32_t layer_seed, int index, int depth, int biome, int kind) {
	return kind == ENC_NORMAL && visitors_here(depth, biome) && (loot_visitors_always || visitor_gate(layer_seed, index));
}

/* Whether a visitor of role `role` stands at column `col` (from 0): the
 * back's in the last, the front's in the first of the enemy's */
static bool stands(int role, int col) { return role == VISITOR_FREE || (role == VISITOR_BACK && col == 5) || (role == VISITOR_FRONT && col == 3); }

/* The highest version up to `top` of family `kin` from net `net` with
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

/* The visitors that fit in place of foe `i` (its family's role, a family
 * the battle does not hold, reached at `depth`, a version in the band),
 * their kin's index and version into fit and ver; how many */
static int fits_for(const Encounter *e, int i, int depth, int fit[VISITOR_KIN_MAX], int ver[VISITOR_KIN_MAX]) {
	int home = e->xrecord ? VISIT_BN5 : VISIT_BN6, from = home ^ 1, hp, h, d;
	const Foe *f = &e->foes[i];
	int role = visitor_role_in(home, f->family), panel = visitor_panel(e, f->col, f->row);
	if (f->kind != FOE_VIRUS || (panel != 2 && panel != 3) || !stands(role, f->col) || !enemy_stats(f->id, &h, &d)) return 0;
	uint64_t held = held_of(e, &hp);
	PacingBand b = pacing_band(depth, false, false);
	int vlo, vhi;
	pacing_virus_versions(depth, false, &vlo, &vhi);
	/* (the act's version, BN5's by tier: BN6's V1 its V1EX, and so on) */
	int top = from == VISIT_BN5 ? (2 * vhi + 1 > 5 ? 5 : 2 * vhi + 1) : vhi, allowed = loot_stage_allowed(depth);
	const VisitorKin *kin;
	int nk = visitor_kin(from, &kin), m = 0;
	for (int k = 0; k < nk; ++k) {
		int id;
		if (!kin[k].visits || kin[k].role != role || kin[k].stage > allowed || (held >> kin[k].family & 1)) continue;
		int v = version_fit(from, kin[k].family, top, hp - h, b, &id);
		if (v < 0) continue;
		fit[m] = k;
		ver[m++] = v;
	}
	return m;
}

bool visitor_apply(Encounter *e, int depth) {
	int order[MAX_FOES], n = 0;
	if (e->visitor || e->boss) return false;
	for (int i = 0; i < e->nfoes; ++i)
		if (e->foes[i].kind == FOE_VIRUS) order[n++] = i;
	if (!n) return false;
	int from = e->xrecord ? VISIT_BN6 : VISIT_BN5, start = rng_range(0, n - 1);
	for (int t = 0; t < n; ++t) {
		int i = order[(start + t) % n], fit[VISITOR_KIN_MAX], ver[VISITOR_KIN_MAX], m = fits_for(e, i, depth, fit, ver);
		if (!m) continue;
		int pick = rng_range(0, m - 1), id;
		const VisitorKin *kin;
		visitor_kin(from, &kin);
		const VisitorKin *k = &kin[fit[pick]];
		id = visitor_id(from, k->family, ver[pick]);
		const uint8_t *tr = enemy_triple(id);
		Foe *o = &e->foes[i];
		o->kind = FOE_VIRUS;
		o->family = k->family;
		o->version = tr ? tr[0] : ver[pick];
		o->id = id;
		e->visitor = i + 1;
		e->from_families |= (uint64_t)1 << k->family;
		return true;
	}
	return false;
}

int nets_battle_area(const int deep[2], int territory) {
	if (deep[0] < 0) return territory;
	if (rng_range(0, 1)) return -1;
	return deep[1] >= 0 && rng_range(0, 1) ? deep[1] : deep[0];
}

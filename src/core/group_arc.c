/* group_arc.h: an act's signature family and its three beats. */
#include "group_arc.h"

#include <string.h>

int arc_home_share(int act) {
	static const int8_t share[4] = { 9, 6, 4, 3 };
	return act >= 0 && act < 4 ? share[act] : -1;
}

int group_arc_depth(int depth) {
	while (depth > 1 && pacing_act(depth - 1) == pacing_act(depth) && pacing_loop(depth - 1) == pacing_loop(depth)) --depth;
	return depth;
}

static uint32_t mix(uint32_t k) {
	k ^= k >> 16;
	k *= 0x7FEB352Du;
	k ^= k >> 15;
	k *= 0x846CA68Bu;
	return k ^ (k >> 16);
}

static bool holds(const GroupFill *g, int fam) {
	for (int i = 0; i < g->n; ++i)
		if (g->fam[i] == fam) return true;
	return false;
}

/* A walk over the fillings of every template (but the swarm, which is one
 * family's): those of `size` families that hold `with` (and `also`, where
 * 0 or more; not `without`, where 0 or more). Counted; the `want`-th
 * kept, or where `hardest`, the one of most pressure (the more HP of two
 * alike, then the first). */
typedef struct {
	int size, with, also, without, want;
	bool hardest;
	int count;
	GroupFill got;
} Seek;

static void seek_each(const GroupFill *g, void *ctx) {
	Seek *s = ctx;
	if ((s->size && g->n != s->size) || !holds(g, s->with) || (s->also >= 0 && !holds(g, s->also)) || (s->without >= 0 && holds(g, s->without))) return;
	bool take = s->hardest ? !s->count || g->pressure > s->got.pressure || (g->pressure == s->got.pressure && g->hp > s->got.hp) : s->count == s->want;
	if (take) s->got = *g;
	++s->count;
}

static int seek(const GroupFam *pool, int n, const GroupLimits *l, Seek *s) {
	s->count = 0;
	for (int t = 0; t < GROUP_TEMPLATES; ++t)
		if (t != GROUP_SWARM) group_fillings_with(pool, n, l, t, s->with, seek_each, s);
	return s->count;
}

/* one of the fillings a seek finds, each as likely as any, by `roll` */
static bool seek_one(const GroupFam *pool, int n, const GroupLimits *l, Seek s, uint32_t roll, GroupFill *out) {
	int count = seek(pool, n, l, &s);
	if (count <= 0) return false;
	s.want = (int)(mix(roll) % (uint32_t)count);
	seek(pool, n, l, &s);
	*out = s.got;
	return true;
}

/* (each family's two-family fillings counted) */
static void pairs_each(const GroupFill *g, void *ctx) {
	int *pairs = ctx;
	if (g->n != 2) return;
	++pairs[g->fam[0]];
	++pairs[g->fam[1]];
}

/* The first beat: one of the family, at its highest version inside the
 * eased limits. One alone is the battle the tags measured; two of a kind
 * were its first form, and two panel breakers at a version past their V1
 * deleted the test autopilot where the V1's numbers said nothing would
 * (docs/game/progression/encounter-groups.md, Step 3). */
static bool alone(const GroupFam *pool, int i, const GroupLimits *eased, GroupFill *out) { return group_same(pool, i, 1, false, eased, out); }

/* The signature: of the families that can carry the act (alone inside the
 * eased limits, and in a pair), one of the area's own or one that is not,
 * by the share; the other side's where its own has none. -1 none. */
static int signature(const GroupFam *pool, int n, const ArcRules *r, uint32_t roll, bool *home) {
	int pairs[GROUP_POOL_MAX] = { 0 }, own[GROUP_POOL_MAX], away[GROUP_POOL_MAX], nown = 0, naway = 0;
	GroupFill g;
	/* (the two-family templates alone: the three-family one pairs nothing) */
	for (int t = 0; t < GROUP_TEMPLATES; ++t)
		if (t != GROUP_SWARM && t != GROUP_PINCER) group_fillings(pool, n, &r->plain, t, pairs_each, pairs);
	for (int i = 0; i < n; ++i) {
		if (!pairs[i] || !alone(pool, i, &r->eased, &g)) continue;
		if (family_mask_has(r->home, pool[i].family)) own[nown++] = i;
		else away[naway++] = i;
	}
	if (!nown && !naway) return -1;
	bool wants = r->home_share < 0 ? mix(roll ^ 0x484F4D45u) % (uint32_t)(nown + naway) < (uint32_t)nown   /* ("HOME": any family, each as likely) */
	                               : (int)(mix(roll ^ 0x484F4D45u) % ARC_SHARE_OF) < r->home_share;
	*home = nown && (wants || !naway);
	uint32_t pick = mix(roll ^ 0x5349474Eu);   /* ("SIGN") */
	return *home ? own[pick % (uint32_t)nown] : away[pick % (uint32_t)naway];
}

bool group_arc_make(const GroupFam *pool, int n, const ArcRules *r, uint32_t roll, GroupArc *out) {
	if (n <= 0 || n > GROUP_POOL_MAX) return false;
	memset(out, 0, sizeof *out);
	out->partner = -1;
	out->signature = (int16_t)signature(pool, n, r, roll, &out->home);
	int sig = out->signature;
	if (sig < 0 || !alone(pool, sig, &r->eased, &out->beat[ARC_ALONE])) return false;
	/* (the pair: a two-family filling that holds it; its other family the partner) */
	GroupFill *pair = &out->beat[ARC_PAIR], *all = &out->beat[ARC_CONSTELLATION];
	if (!seek_one(pool, n, &r->plain, (Seek){ 2, sig, -1, -1, 0, false, 0, { 0 } }, roll ^ 0x50414952u, pair)) return false;   /* ("PAIR") */
	out->partner = pair->fam[0] == sig ? pair->fam[1] : pair->fam[0];
	/* (the constellation: three families with the partner among them, three families, three of it, the hardest pair with
	 * another partner, the pair again) */
	uint32_t at = roll ^ 0x434F4E53u;   /* ("CONS") */
	Seek other = { 2, sig, -1, out->partner, 0, true, 0, { 0 } };
	if (!(r->second && seek_one(pool, n, &r->plain, (Seek){ 3, sig, out->partner, -1, 0, false, 0, { 0 } }, at, all)) &&
		!seek_one(pool, n, &r->plain, (Seek){ 3, sig, -1, -1, 0, false, 0, { 0 } }, at, all) && !group_same(pool, sig, 3, true, &r->plain, all)) {
		if (seek(pool, n, &r->plain, &other) > 0) *all = other.got;
		else *all = *pair;
	}
	/* (the Server's: the hardest filling that holds it inside the challenge's limits, three of it among them) */
	Seek hard = { 0, sig, -1, -1, 0, true, 0, { 0 } };
	GroupFill three;
	out->has_challenge = seek(pool, n, &r->challenge, &hard) > 0;
	if (out->has_challenge) out->challenge = hard.got;
	if (group_same(pool, sig, 3, true, &r->challenge, &three) && (!out->has_challenge || three.pressure > out->challenge.pressure)) {
		out->challenge = three;
		out->has_challenge = true;
	}
	return true;
}

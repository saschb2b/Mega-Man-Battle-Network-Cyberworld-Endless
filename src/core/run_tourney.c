/* The tournament's acts (run.h; docs/game/tournament.md, the owner's calls
 * of 5 November 2026): where a bracket of three Navis stands in an act's
 * arena in place of its guardian, from the run's seed, and who stands in
 * it. A run whose extras leave the tournament out
 * (docs/game/meta/extras.md) never meets one. */
#include "run.h"

#include "pacing.h"
#include "save.h"

bool run_debug_bracket;

bool run_bracket_at(int depth) {
	if (depth < 1 || !run_extra(EXTRA_TOURNEY)) return false;
	int loop = (depth - 1) / CYCLE_LAYERS, p = (depth - 1) % CYCLE_LAYERS, act = p / 3;
	if (p >= 18 || (run.mode == RUN_SHORT && (loop || act > 2))) return false;
	/* (dev: every act from the third that can hold one) */
	if (run_debug_bracket) return act >= 2 && act <= 4 && !run_glitch_at(loop * CYCLE_LAYERS + act * 3 + 1);
	if (profile.best_depth < SHORT_LAYERS) return false;
	/* (the short net's act 3 in half the runs; the endless net's act 3, 4
	 * or 5, each as likely, every cycle) */
	uint32_t h = (run.seed ^ 0x54524E59u ^ (uint32_t)loop * 0x9E3779B9u) * 2654435761u;   /* "TRNY" */
	int at = run.mode == RUN_SHORT ? (h >> 8 & 1 ? 2 : -1) : 2 + (int)((h >> 8) % 3);
	/* (never the act of the run's Glitch layer: there the act keeps its
	 * single guardian) */
	return act == at && !run_glitch_at(loop * CYCLE_LAYERS + act * 3 + 1);
}

/* (a draw of the act's: the seed's, mixed) */
static uint32_t mix(uint32_t h) {
	h ^= h >> 16;
	h *= 0x7FEB352Du;
	h ^= h >> 15;
	h *= 0x846CA68Bu;
	return h ^ h >> 16;
}

/* The pool's Navi `i` in round `round` of act `act`: the version and the
 * HP he is fought at, and how far it lies outside the round's band (0
 * inside; on later cycles every one fits, as their guardians do) */
static int round_miss(const BracketPool *p, int i, int act, int round, int loop, int *version, int *hp) {
	int lo, hi;
	pacing_round_band(act, round, &lo, &hi);
	if (p->older[i]) {
		int h[4] = { p->hp[i][0], p->hp[i][1], p->hp[i][2], p->hp[i][3] };
		*version = pacing_xround_version(h, act, round, loop);
		*hp = h[*version] < 0 ? -1 : pacing_xround_hp(h[*version], act, round, loop);
	} else {
		int h[3] = { p->hp[i][0], p->hp[i][1], p->hp[i][2] };
		*version = pacing_round_version(h, act, round, loop);
		*hp = h[*version];
	}
	if (*hp < 0) return 100000;
	return loop > 0 ? 0 : *hp < lo ? lo - *hp : *hp > hi ? *hp - hi : 0;
}

/* One round's Navi: among those not `out`, each that fits as likely as
 * another, by `h`; where none fits and `nearest`, the nearest by HP; -1
 * none */
static int round_pick(const BracketPool *p, const bool *out, int act, int round, int loop, uint32_t h, bool nearest) {
	int fit[BRACKET_POOL], n = 0, best = -1, best_miss = 1 << 30, v, hp;
	for (int i = 0; i < p->n; ++i) {
		if (out[i]) continue;
		int m = round_miss(p, i, act, round, loop, &v, &hp);
		if (m == 0) fit[n++] = i;
		if (m < best_miss) { best_miss = m; best = i; }
	}
	return n ? fit[h % (uint32_t)n] : nearest ? best : -1;
}

/* The pool's place of navi `navi`, -1 none */
static int pool_at(const BracketPool *p, int navi) {
	for (int i = 0; i < p->n; ++i)
		if (p->navi[i] == navi) return i;
	return -1;
}

/* A draw's act: its pool, the act and cycle, the final */
typedef struct { const BracketPool *p; int act, loop, final; } Draw;

/* Round `round`'s pick: first without regard to the final, so the same
 * whichever way the act took at Lan's HP, and again without him where he
 * was drawn; the run's past guardians left out wherever another fits */
static int round_draw(const Draw *d, const bool *rounds, const bool *past, int round, uint32_t h) {
	const BracketPool *p = d->p;
	int act = d->act, loop = d->loop, final = d->final;
	bool out[BRACKET_POOL];
	for (int i = 0; i < p->n; ++i) out[i] = rounds[i] || past[i];
	int k = round_pick(p, out, act, round, loop, h, false);
	if (k >= 0 && p->navi[k] != final) return k;
	int f = pool_at(p, final);
	if (f >= 0) out[f] = true;
	k = round_pick(p, out, act, round, loop, h, false);
	if (k >= 0) return k;
	for (int i = 0; i < p->n; ++i) out[i] = rounds[i] || i == f;
	return round_pick(p, out, act, round, loop, h, true);
}

/* A stand-in's chip for round `round` of a draw whose Navi is a stand_in
 * one: BN6's own Navis (neither another game's nor a stand-in), drawn from
 * the round's band as its Navi is, by `h`; into prize and prize_version */
static void prize_draw(const Draw *d, RunBracket *b, int round, uint32_t h) {
	const BracketPool *p = d->p;
	bool out[BRACKET_POOL];
	for (int i = 0; i < p->n; ++i) out[i] = p->older[i] || p->stand_in[i];
	int k = round_pick(p, out, d->act, round, d->loop, h, true), hp;
	b->prize[round] = k >= 0 ? p->navi[k] : 0;
	b->prize_version[round] = 0;
	if (k >= 0) round_miss(p, k, d->act, round, d->loop, &b->prize_version[round], &hp);
}

void run_bracket_draw(int depth, int final, int final_version, int final_hp, const BracketPool *p, RunBracket *b) {
	int loop = (depth - 1) / CYCLE_LAYERS, act = (depth - 1) % CYCLE_LAYERS / 3;
	bool rounds[BRACKET_POOL] = { false }, past[BRACKET_POOL] = { false };
	/* (the guardians of the acts before, in the run's order) */
	for (int a = 0; a < act && a < 6; ++a) {
		int k = pool_at(p, run_guardian(run.biome_order[a]));
		if (k >= 0) past[k] = true;
	}
	uint32_t seed = run.seed ^ 0x42524B54u ^ (uint32_t)loop * 0x9E3779B9u ^ (uint32_t)act * 0x85EBCA6Bu;   /* "BRKT" */
	for (int r = 0; r < 2; ++r) {
		const Draw d = { p, act, loop, final };
		int k = round_draw(&d, rounds, past, r, mix(seed + (uint32_t)r * 0x27D4EB2Fu));
		b->navi[r] = b->prize[r] = k >= 0 ? p->navi[k] : 0;
		b->version[r] = b->hp[r] = b->prize_version[r] = 0;
		if (k < 0) continue;
		rounds[k] = true;
		round_miss(p, k, act, r, loop, &b->version[r], &b->hp[r]);
		b->prize_version[r] = b->version[r];
		/* (a stand-in's chip: its own draw, "PRZE", the rounds' unchanged) */
		if (p->stand_in[k]) prize_draw(&d, b, r, mix(seed ^ 0x50525A45u ^ (uint32_t)r * 0x27D4EB2Fu));
	}
	b->navi[2] = final;
	b->version[2] = final_version;
	b->hp[2] = final_hp;
	/* (the four who fill BN4's other places and lose their matches: any of
	 * the pool's but the three, by the seed) */
	int f = pool_at(p, final);
	if (f >= 0) rounds[f] = true;
	for (int o = 0; o < BRACKET_OTHERS; ++o) {
		int left[BRACKET_POOL], n = 0;
		for (int i = 0; i < p->n; ++i)
			if (!rounds[i]) left[n++] = i;
		b->others[o] = 0;
		if (!n) continue;
		int k = left[mix(seed + 0x9E37u * (uint32_t)(o + 3)) % (uint32_t)n];
		b->others[o] = p->navi[k];
		rounds[k] = true;
	}
}

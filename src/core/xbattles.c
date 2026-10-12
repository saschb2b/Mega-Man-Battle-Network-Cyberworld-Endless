/* xbattles.h: another game's battle records as BN6's Encounters, and a
 * territory's random battles from them. */
#include "xbattles.h"

#include <string.h>

#include "bn5.h"
#include "data.h"
#include "game.h"
#include "loot.h"
#include "net.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "xchips.h"
#include "xfields.h"
#include "xvirus.h"

#define BUS 0x08000000u
#define POOL_MAX 192   /* a territory's records a battle is picked from, at most */

static const uint8_t *rom_of(int xrom) { return xrom == XROM_BN5_COLONEL_US ? XR[xrom].data : NULL; }

static uint32_t rd32(const uint8_t *d, uint32_t a) {
	if (a < BUS || a - BUS + 4 > ROM_SIZE) return 0;
	const uint8_t *p = d + (a - BUS);
	return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/* ---- the virus families both games have, by name ---- */

#define XB_AIS 64
static int8_t shared[XB_AIS];   /* its game's AI index -> BN6's of the same name (its V1's), 0 none */
static bool shared_known;

/* (once both ROMs are read: each family's V1 name in BN6's text codes,
 * rom.h's xrom_text, against BN6's 29: Mettaur, Catack, Champy, WindBox,
 * Trumpy in BN5) */
static void shared_learn(int xrom) {
	if (shared_known || !R.data || !rom_of(xrom)) return;
	shared_known = true;
	char bn6[30][20];
	for (int f = 1; f < 30; ++f) {
		int id = enemy_id(0, f, 0);
		bn6[f][0] = 0;
		if (id > 0) rom_text(R.layout->enemy_names[0], id, bn6[f], sizeof bn6[f]);
	}
	for (int ai = 1; ai < XB_AIS; ++ai) {
		int id = xrecords_family_id(xrom, ai, 0);
		char name[20];
		if (!id) continue;
		xrom_text(xrom, BN5_ENEMY_NAMES - BUS, id, name, sizeof name);
		for (int f = 1; f < 30 && !shared[ai]; ++f)
			if (name[0] && !strcmp(name, bn6[f])) shared[ai] = (int8_t)f;
	}
}

int xbattle_enemy_bn6(int xrom, int id, int *why, int *what) {
	const uint8_t *t = xrecords_triple(xrom, id);
	*what = t ? t[2] : 0;
	*why = !t ? XB_NONE : t[1] == BN5_NAVI_TYPE ? XB_NAVI : t[1] ? XB_OBJECT : XB_FAMILY;
	if (!t || t[1]) return -1;
	shared_learn(xrom);
	/* (BN6's family at the tier of its version: BN5's V1 and V1EX as V1,
	 * V2 and V2EX as V2, V3 and V3EX as V3) */
	if (t[2] < XB_AIS && shared[t[2]]) {
		int bn6 = enemy_id(0, shared[t[2]], t[0] / 2);
		if (bn6 >= 0) return bn6;
	}
	const XVirusFamily *f = xvirus_installed(xrom, t[2]);
	return f && t[0] < XV_VERSIONS ? f->first_id + t[0] : -1;
}

/* ---- a record ---- */

/* Its viruses whatever their panels, as loot.c's repeat rule reads a
 * formation's: their ids sorted, hashed */
static uint32_t viruses_of(const Encounter *e) {
	int id[MAX_FOES], n = e->nfoes;
	for (int k = 0; k < n; ++k) id[k] = e->foes[k].id;
	for (int i = 1; i < n; ++i)
		for (int j = i; j > 0 && id[j - 1] > id[j]; --j) { int t = id[j]; id[j] = id[j - 1]; id[j - 1] = t; }
	uint32_t h = 2166136261u;
	for (int k = 0; k < n; ++k) h = (h ^ (uint32_t)id[k]) * 16777619u;
	return h;
}

/* An enemy of the record (its id in its game, at its panel) as BN6's foe */
static int foe_add(int xrom, int id, int panel, Encounter *e, int *what) {
	int why, bn6 = xbattle_enemy_bn6(xrom, id, &why, what);
	if (bn6 < 0) return why;
	if (e->nfoes >= MAX_FOES) return XB_FULL;
	const uint8_t *t = enemy_triple(bn6);
	if (!t) return XB_FAMILY;
	e->foes[e->nfoes++] = (Foe){ FOE_VIRUS, t[2], t[0], (panel & 15) - 1, (panel >> 4) - 1, bn6 };
	if (t[2] < XB_AIS) e->from_families |= (uint64_t)1 << t[2];
	return XB_FIGHTABLE;
}

/* Its green Mystery Data's finds, its game's row (byte 2 >> 4) as BN6's,
 * in * with the All * helper; false where it names no row of them */
static bool gem_rows(const uint8_t *d, int arg, Encounter *e) {
	uint32_t row = (uint32_t)(arg & 0xFF) >> 4;
	if (row >= BN5_FIND_ROW_COUNT) return false;
	for (uint32_t k = 0; k < 8; ++k) {
		const uint8_t *p = d + (BN5_FIND_ROWS - BUS) + 2u * (8u * row + k);
		uint16_t v = xchips_reward_bn6((uint16_t)(p[0] | p[1] << 8));
		e->gem_row[k] = run_all_star() && v != 0xFFFF && !(v >> 14) ? chip_entry_star(v) : v;
	}
	e->gem_own = true;
	return true;
}

/* A field object of the record: a green Mystery Data (its byte 2 its row
 * and its chance, rolled later), a rock as BN6 has it */
static int object_add(const uint8_t *d, const uint8_t en[4], Encounter *e) {
	int kind = en[0] >> 4, arg = en[2] | en[3] << 8;
	if (kind != FIELD_GEM && kind != FIELD_ROCK) return XB_OBJECT;
	if (kind == FIELD_GEM && e->gem_own) return XB_FIGHTABLE;   /* (one a battle, as BN6 keeps one) */
	if (e->nobj >= MAX_FIELD_OBJS) return XB_FULL;
	if (kind == FIELD_GEM && !gem_rows(d, arg, e)) return XB_OBJECT;
	e->obj[e->nobj].kind = en[0];
	e->obj[e->nobj].panel = en[1];
	e->obj[e->nobj++].arg = arg;
	return XB_FIGHTABLE;
}

int xbattle_translate(int xrom, uint32_t record, XScale sc, Encounter *e, int *what) {
	const uint8_t *d = rom_of(xrom);
	*what = 0;
	if (!d || record < BUS || record - BUS + 16 > ROM_SIZE || d[record - BUS] == 0xFF) return XB_NONE;
	const uint8_t *r = d + (record - BUS);
	if (r[7]) { *what = r[7]; return XB_STORY; }
	memset(e, 0, sizeof *e);
	for (int i = 0; i < MAX_FOES; ++i) e->foes[i].id = -1;
	e->from_pick = -1;
	e->xrecord = record;
	e->field = xfield_bn6(xrom, r[0]);
	if (e->field < 0) { *what = r[0]; return XB_FIELD; }
	uint32_t a = rd32(d, record + 12);
	for (uint32_t k = 0; k < 16 && a >= BUS && a - BUS + 4u * k + 4 <= ROM_SIZE && d[a - BUS + 4u * k] != 0xF0; ++k) {
		const uint8_t *en = d + (a - BUS) + 4u * k;
		int why = XB_FIGHTABLE;
		if (en[0] == 0x00) e->player = en[1];
		else if (en[0] == 0x11) why = foe_add(xrom, xrecords_version_up(d, en[2] | en[3] << 8, sc), en[1], e, what);
		else why = object_add(d, en, e);
		if (why == XB_OBJECT && !*what) *what = en[0];
		if (why != XB_FIGHTABLE) return why;
	}
	if (!e->nfoes) return XB_NONE;
	e->from_viruses = viruses_of(e);
	return XB_FIGHTABLE;
}

int xbattle_encounter(int xrom, uint32_t record, XScale sc, Encounter *out, int *what) {
	Encounter e;
	int why = xbattle_translate(xrom, record, sc, &e, what);
	if (why != XB_FIGHTABLE) return why;
	/* (its Mystery Data by its game's rule: there where its byte 2 beats a
	 * roll of 15, rows 1 on always; BN6 keeps the one it is given,
	 * encounter.c) */
	for (int i = 0; i < e.nobj; ++i) {
		int chance = e.obj[i].arg & 0xFF;
		if (e.obj[i].kind >> 4 != FIELD_GEM || chance >= 15 || rng_range(0, 14) < chance) continue;
		e.obj[i] = e.obj[--e.nobj];
		e.gem_own = false;
		break;
	}
	e.biome = out->biome;
	*out = e;
	return XB_FIGHTABLE;
}

bool xbattle_fightable(int xrom, uint32_t record) {
	int what;
	return xbattle_why(xrom, record, &what) == XB_FIGHTABLE;
}

int xbattle_why(int xrom, uint32_t record, int *what) {
	Encounter e;
	return xbattle_translate(xrom, record, (XScale){ 0, 0 }, &e, what);
}

/* ---- a territory's random battles ---- */

int xterritory_records(int area, int *fightable) {
	const NetAreaDef *a = net_area_def(area);
	int n = 0;
	*fightable = 0;
	for (int m = 0; a && a->xrom > 0 && m < 3; ++m)
		for (int i = 0, k = xrecords_count(a->xrom - 1, a->xbattles[m][0], a->xbattles[m][1]); i < k; ++i) {
			uint32_t r = xrecords_at(a->xrom - 1, a->xbattles[m][0], a->xbattles[m][1], i);
			if (rom_of(a->xrom - 1)[r - BUS + 7]) continue;   /* (a story's: none of its random battles) */
			++n;
			*fightable += xbattle_fightable(a->xrom - 1, r);
		}
	return n;
}

bool xterritory_native(int area) {
	int fightable, n = xterritory_records(area, &fightable);
	return n > 0 && fightable == n;
}

/* The pool's records as the repeat rule weighs them, each one's viruses
 * counted (into nv) and whether it is the last battle again (into again) */
static void candidates(int xrom, const XPool *p, int n, int area, LootCand *c, int *nv, bool *again) {
	int last_area, last_pick;
	loot_last_fought(&last_area, &last_pick);
	for (int i = 0; i < n; ++i) {
		Encounter t;
		int what;
		bool ok = xbattle_translate(xrom, p->out[i], (XScale){ p->ups[i], p->band->vcap }, &t, &what) == XB_FIGHTABLE;
		c[i] = (LootCand){ ok, ok ? t.from_families : 0, ok ? t.from_viruses : 0 };
		nv[i] = ok ? t.nfoes : MAX_FOES + 1;
		again[i] = last_area == area && last_pick == (int)(p->out[i] - BUS);
	}
}

/* The pick among them: an opening battle two viruses at most where the
 * pool has such (the run's first one), each not the last battle again
 * where another fits, by the repeat rule's weights; -1 none */
static int pick_of(LootCand *c, const int *nv, const bool *again, int n, int area, int kind, bool easy) {
	static const int8_t tries[3][3] = { { 9, 0, 0 }, { 2, 9, 0 }, { 1, 2, 9 } };
	const int8_t *most = tries[kind == ENC_FIRST ? 2 : easy ? 1 : 0];
	static int weight[POOL_MAX];
	static bool fit[POOL_MAX];
	for (int i = 0; i < n; ++i) fit[i] = c[i].weight > 0;
	int total = 0;
	for (int t = 0; t < 3 && most[t] && !total; ++t)
		for (int allow = 0; allow < 2 && !total; ++allow) {
			for (int i = 0; i < n; ++i) c[i].weight = fit[i] && nv[i] <= most[t] && (allow || !again[i]) ? 1 : 0;
			total = loot_repeat_weights(c, n, area, weight);
		}
	return total ? loot_weighted_pick(weight, n, total) : -1;
}

/* A record's strength as BN6's engine fights it (XPool.measure): its foes'
 * HP together and the strongest hit, BN6's stats for a family both have,
 * the moved family's own; how many foes, 0 where it cannot be fought */
static int measure(int xrom, uint32_t record, XScale sc, int *hp, int *damage) {
	Encounter e;
	int what;
	*hp = *damage = 0;
	if (xbattle_translate(xrom, record, sc, &e, &what) != XB_FIGHTABLE) return 0;
	for (int i = 0; i < e.nfoes; ++i) {
		int h, d;
		if (!enemy_stats(e.foes[i].id, &h, &d)) return 0;
		*hp += h;
		if (d > *damage) *damage = d;
	}
	return e.nfoes;
}

bool xterritory_encounter(int depth, int area, int kind, Encounter *out) {
	const NetAreaDef *a = net_area_def(area);
	if (!a || a->xrom <= 0 || !rom_of(a->xrom - 1)) return false;
	int xrom = a->xrom - 1;
	const uint8_t *xb = a->xbattles[layer_in_act(depth)];
	if (!xb[0]) return false;
	bool challenge = kind == ENC_CHALLENGE, easy = kind == ENC_EASY || kind == ENC_FIRST || (kind == ENC_NORMAL && (run.helpers & HELP_GENTLE));
	PacingBand b = pacing_band(depth, challenge, easy);
	int vlo, vhi;
	pacing_virus_versions(depth, challenge, &vlo, &vhi);
	/* (the act's version by tier: BN6's V1 lets BN5's records go to V1EX,
	 * its V2 to V2EX, its V3 to V3EX) */
	XBand band = { b.lo, b.hi, b.cap, 2 * vhi + 1 };
	static uint32_t pool[POOL_MAX];
	static uint8_t ups[POOL_MAX];
	XPool p = { xrom, a, xb[0], xb[1], &band, xbattle_fightable, measure, pool, ups, POOL_MAX, false };
	int n = xrecords_pool(&p);
	static LootCand c[POOL_MAX];
	static int nv[POOL_MAX];
	static bool again[POOL_MAX];
	candidates(xrom, &p, n, area, c, nv, again);
	int k = n ? pick_of(c, nv, again, n, area, kind, easy) : -1;
	Encounter e = { 0 };
	int what;
	e.biome = out->biome;
	if (k < 0 || xbattle_encounter(xrom, pool[k], (XScale){ ups[k], band.vcap }, &e, &what) != XB_FIGHTABLE) return false;
	e.from_biome = area;
	e.from_pick = (int)(pool[k] - BUS);
	*out = e;
	return true;
}

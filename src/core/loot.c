#include "loot.h"

#include "data.h"
#include "chip_pool.h"
#include "formations.h"
#include "game.h"
#include "pacing.h"
#include "run.h"
#include "rom.h"

#define NAVI_CHALLENGE 40   /* % of challenges from the fourth act on against one of the area's SP navis */

/* When a virus family first meets MegaMan on the net: 0 the surface
 * areas, 1 the Undernet, 2 the Graveyard and the Underground (3 never). The
 * comps and homepages hold families of every stage and keep to the ones the
 * run has reached. */
static int family_stage(int family) {
	static int8_t stage[32];
	static bool known;
	if (!known) {
		known = true;
		for (int f = 0; f < 32; ++f) stage[f] = 3;
		static const struct { int biome, stage; } areas[] = {
			{ BIOME_CENTRAL, 0 }, { BIOME_SEASIDE, 0 }, { BIOME_SKY, 0 }, { BIOME_GREEN, 0 },
			{ BIOME_UNDERNET, 1 }, { BIOME_GRAVEYARD, 2 }, { BIOME_NEST, 2 },
		};
		for (unsigned a = 0; a < sizeof areas / sizeof *areas; ++a) {
			const Formation *list;
			int n = formations_of(areas[a].biome, &list);
			for (int i = 0; i < n; ++i)
				for (int k = 0; k < list[i].n; ++k) {
					const uint8_t *row = R.data + R.layout->enemy_ids + list[i].ent[k].id * 3;
					if (row[1] == 0 && row[2] < 32 && stage[row[2]] > areas[a].stage) stage[row[2]] = (int8_t)areas[a].stage;
				}
		}
	}
	return family >= 0 && family < 32 ? stage[family] : 3;
}

static bool reached(const Formation *f, int allowed) {
	for (int k = 0; k < f->n; ++k) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[k].id * 3;
		if (row[1] == 0 && row[2] >= 1 && row[2] <= 29 && family_stage(row[2]) > allowed) return false;
	}
	return true;
}

/* enemy_id for viruses, remembered (families 1-29, versions 0-5) */
static int virus_id(int family, int version) {
	static int16_t ids[30][6];
	static bool known;
	if (!known) {
		known = true;
		for (int f = 0; f < 30; ++f)
			for (int v = 0; v < 6; ++v) ids[f][v] = (int16_t)(f ? enemy_id(0, f, v) : -1);
	}
	return family >= 1 && family <= 29 && version >= 0 && version <= 5 ? ids[family][version] : -1;
}

/* A formation's foes with its viruses at `want` (the originals are paced
 * for the story; the late story's heavy families, the dragons and
 * Nightmare, met in packs, stay at their first version on the first
 * cycle), one of them rare with `rare`; their HP together and the
 * strongest one's damage. */
static void build_foes(const Formation *f, int depth, int want, bool rare, Encounter *e, int *hp, int *dmg) {
	e->nfoes = 0;
	e->field = f->battlefield;
	*hp = *dmg = 0;
	for (int i = 0; i < f->n && e->nfoes < MAX_FOES; ++i) {
		const uint8_t *row = R.data + R.layout->enemy_ids + f->ent[i].id * 3;
		Foe *o = &e->foes[e->nfoes++];
		o->version = row[0];
		o->family = row[2];
		o->col = (f->ent[i].panel & 15) - 1;
		o->row = (f->ent[i].panel >> 4) - 1;
		o->id = f->ent[i].id;
		/* a virus is the first entry of its family and version; the rest of
		 * the table's type 0 are rocks, cubes and other objects */
		if (row[1] == 1) o->kind = FOE_NAVI;
		else if (row[1] == 0 && o->family >= 1 && o->family <= 29 && virus_id(o->family, o->version) == o->id) o->kind = FOE_VIRUS;
		else { o->kind = FOE_ROCK; continue; }
		if (o->kind == FOE_VIRUS && o->version <= 3) {
			int v = depth <= CYCLE_LAYERS && family_stage(o->family) >= 2 ? 0 : want;
			if (rare) {
				/* the first virus with a rare form takes it */
				int r = virus_id(o->family, pacing_act(depth) < 4 ? 4 : 5);
				if (r >= 0) { v = pacing_act(depth) < 4 ? 4 : 5; rare = false; }
			}
			int id = virus_id(o->family, v);
			if (id >= 0) { o->version = v; o->id = id; }
		}
		int h, d;
		if (enemy_stats(o->id, &h, &d)) {
			*hp += h;
			if (d > *dmg) *dmg = d;
		}
	}
}

static int last_biome = -1, last_pick = -1;   /* no formation twice in a row */

#define MAX_FIT 160

/* Each formation's version inside the band (fit[i], -1 for none), and their
 * weight together: the ones that reach the band's aim if any do, else the
 * lighter ones. `skip` is left out. */
static int weigh(const Formation *list, int n, int depth, int target, PacingBand band, int allowed, int skip, int8_t fit[MAX_FIT]) {
	static uint8_t inside[MAX_FIT];
	int in_total = 0, any_total = 0;
	for (int i = 0; i < n && i < MAX_FIT; ++i) {
		fit[i] = -1;
		inside[i] = 0;
		if (i == skip || list[i].navi || !reached(&list[i], allowed)) continue;
		for (int v = target; v >= 0; --v) {
			Encounter t;
			int hp, dmg;
			build_foes(&list[i], depth, v, false, &t, &hp, &dmg);
			if (hp > band.hi || dmg > band.cap) continue;
			fit[i] = (int8_t)v;
			inside[i] = hp >= band.lo;
			break;
		}
		if (fit[i] < 0) continue;
		any_total += list[i].weight;
		if (inside[i]) in_total += list[i].weight;
	}
	if (!in_total) return any_total;
	for (int i = 0; i < n && i < MAX_FIT; ++i) if (!inside[i]) fit[i] = -1;
	return in_total;
}

/* A formation from the area's original random battles, at the highest
 * version up to the act's that keeps it inside the act's band
 * (docs/PROGRESSION.md). False when the ROM has none for the area. */
static bool original_encounter(int depth, int biome, int kind, Encounter *e) {
	const Formation *list;
	int n = formations_of(biome, &list);
	if (!n) return false;
	bool challenge = kind == ENC_CHALLENGE, easy = kind == ENC_EASY;
	int p = (depth - 1) % CYCLE_LAYERS, allowed = depth > CYCLE_LAYERS ? 2 : p < 6 ? 0 : p < 12 ? 1 : 2;
	int target = pacing_virus_version(depth, challenge);
	bool rare = pacing_rare(depth, rng_range(0, 99));
	/* from the fourth act a challenge may meet one of the area's SP navis,
	 * who keeps his own strength */
	if (challenge && pacing_act(depth) >= 3 && rng_range(0, 99) < NAVI_CHALLENGE) {
		int total = 0;
		for (int i = 0; i < n; ++i) total += list[i].navi ? list[i].weight : 0;
		if (total) {
			int roll = rng_range(0, total - 1), pick = 0, hp, dmg;
			for (int i = 0; i < n; ++i) {
				if (!list[i].navi) continue;
				if (roll < list[i].weight) { pick = i; break; }
				roll -= list[i].weight;
			}
			build_foes(&list[pick], depth, target, false, e, &hp, &dmg);
			return e->nfoes > 0;
		}
	}
	static int8_t fit[MAX_FIT];
	PacingBand band = pacing_band(depth, challenge, easy);
	int total = 0;
	for (int widen = 0; widen < 4 && !total; ++widen) {
		if (widen == 3) allowed = 3;   /* an area of late viruses only */
		/* not the last battle again, unless nothing else fits */
		total = weigh(list, n, depth, target, band, allowed, biome == last_biome ? last_pick : -1, fit);
		if (!total) total = weigh(list, n, depth, target, band, allowed, -1, fit);
		band = pacing_band_wider(band);
	}
	if (!total) return false;
	int roll = rng_range(0, total - 1), pick = -1;
	for (int i = 0; i < n && i < MAX_FIT; ++i) {
		if (fit[i] < 0) continue;
		if (roll < list[i].weight) { pick = i; break; }
		roll -= list[i].weight;
	}
	if (pick < 0) return false;
	last_biome = biome;
	last_pick = pick;
	int hp, dmg;
	build_foes(&list[pick], depth, fit[pick], false, e, &hp, &dmg);
	band = pacing_band(depth, challenge, easy);
	if (rare) {
		/* a rare virus, if the battle still fits with it */
		Encounter r = *e;
		int rhp, rdmg;
		build_foes(&list[pick], depth, fit[pick], true, &r, &rhp, &rdmg);
		if (rhp <= band.hi && rdmg <= band.cap) *e = r;
	}
	return e->nfoes > 0;
}

Encounter make_encounter(int depth, int biome, int kind) {
	bool challenge = kind == ENC_CHALLENGE;
	Encounter e = { 0 };
	e.biome = biome;
	for (int i = 0; i < MAX_FOES; ++i) e.foes[i].id = -1;
	if (original_encounter(depth, biome, kind, &e)) return e;
	e.nfoes = 0;
	for (int i = 0; i < MAX_FOES; ++i) e.foes[i].id = -1;
	int pool[16], n = 0;
	int p = (depth - 1) % CYCLE_LAYERS + (depth > CYCLE_LAYERS ? 12 : 0);
	for (int i = 0; i < virus_def_count; ++i)
		if ((virus_defs[i].biome_mask >> biome) & 1 && virus_defs[i].first_depth <= p) pool[n++] = virus_defs[i].family;
	if (!n) pool[n++] = 1;
	int count = depth <= 1 ? rng_range(1, 2) : rng_range(2, 3);
	if (challenge) ++count;
	if (count > 4) count = 4;
	bool used[3][3] = { { false } };
	for (int i = 0; i < count; ++i) {
		Foe *f = &e.foes[e.nfoes];
		f->kind = FOE_VIRUS;
		f->family = pool[rng_range(0, n - 1)];
		f->version = pacing_virus_version(depth, challenge);
		for (int tries = 0; tries < 20; ++tries) {
			int c = rng_range(0, 2), r = rng_range(0, 2);
			if (used[r][c]) continue;
			used[r][c] = true;
			f->col = 3 + c;
			f->row = r;
			e.nfoes++;
			break;
		}
	}
	return e;
}

Encounter make_boss(int depth, int biome, int navi) {
	Encounter e = { 0 };
	e.biome = biome;
	e.boss = true;
	e.nfoes = 1;
	e.foes[0].id = -1;
	e.foes[0].kind = FOE_NAVI;
	e.foes[0].family = navi;
	/* the version whose HP suits the act (docs/PROGRESSION.md) */
	e.foes[0].version = pacing_guardian_version(navi, pacing_act(depth), pacing_loop(depth),
		biome == BIOME_NEST || biome == BIOME_SECRET, navi_hp);
	e.foes[0].col = 4;
	e.foes[0].row = 1;
	return e;
}

int roll_chip(int depth, int bonus_tier, char *code) {
	int p = (depth - 1) % CYCLE_LAYERS + 3 * ((depth - 1) / CYCLE_LAYERS);
	/* tiers (chip_pool.h): common and uncommon standard chips early, rarer
	 * ones and Megas deeper, a Giga now and then past the middle */
	int w[CHIP_TIERS] = { 60 - p * 3, 30, 6 + p * 2, p > 5 ? p : 0, p > 12 ? (p - 12) / 2 : 0 };
	if (w[0] < 8) w[0] = 8;
	for (int b = 0; b < bonus_tier; ++b) { w[0] /= 2; w[3] += 6; w[2] += 6; w[4] += p > 8 ? 2 : 0; }
	int total = w[0] + w[1] + w[2] + w[3] + w[4];
	int roll = rng_range(0, total - 1), tier = 0;
	while (tier < CHIP_TIERS - 1 && roll >= w[tier]) roll -= w[tier++];
	int id = chip_pool_pick(tier);
	if (id <= 0) {
		/* without a ROM: the engine's own list */
		int pick[128], n = 0;
		if (tier > 3) tier = 3;
		for (int i = 0; i < chip_def_count; ++i)
			if (chip_defs[i].tier == tier && chip_defs[i].kind != CK_NAVI) pick[n++] = chip_defs[i].rom_id;
		if (!n) for (int i = 0; i < chip_def_count; ++i) if (chip_defs[i].tier <= 1) pick[n++] = chip_defs[i].rom_id;
		id = pick[rng_range(0, n - 1)];
	}
	ChipInfo ci;
	chip_info(id, &ci);
	*code = ci.ncodes ? ci.codes[rng_range(0, ci.ncodes - 1)] : '*';
	return id;
}

int chip_price(int id) {
	/* zenny by tier, dearer on later cycles */
	static const int by_tier[CHIP_TIERS] = { 5, 10, 20, 40, 80 };
	int t = chip_pool_tier(id), loop = (run.depth - 1) / CYCLE_LAYERS;
	int base = t >= 0 ? by_tier[t] : chip_def(id)->price;
	return base * 100 * (2 + loop) / 2;
}

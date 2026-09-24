#include "loot.h"

#include "data.h"
#include "chip_pool.h"
#include "formations.h"
#include "game.h"
#include "run.h"
#include "rom.h"

int virus_version(int depth, bool hard) {
	int loop = (depth - 1) / CYCLE_LAYERS;
	int p = (depth - 1) % CYCLE_LAYERS;
	int v = p < 4 ? 0 : p < 9 ? rng_range(0, 1) : p < 13 ? 1 + (rng_range(0, 3) == 0) : 2;
	v += loop + (hard ? 1 : 0);
	if (v > 3) v = 3;
	if (rng_range(0, 99) < 3) v = 4 + (p > 9); /* rare variants */
	return v;
}

#define NAVI_CHALLENGE 40   /* % of deep challenges against one of the area's SP navis */

static int weight_of(const Formation *list, int n, bool navi) {
	int total = 0;
	for (int i = 0; i < n; ++i) total += list[i].navi == navi ? list[i].weight : 0;
	return total;
}

/* A formation from the area's original random battles, its viruses at the
 * depth's version. False when the ROM has none for the area. */
static bool original_encounter(int depth, int biome, bool challenge, Encounter *e) {
	const Formation *list;
	int n = formations_of(biome, &list);
	if (!n) return false;
	/* deep challenges may meet a navi, everything else the viruses */
	bool navi = challenge && depth >= 8 && rng_range(0, 99) < NAVI_CHALLENGE;
	int total = weight_of(list, n, navi);
	if (!total && navi) total = weight_of(list, n, navi = false);
	if (!total) return false;
	int roll = rng_range(0, total - 1), pick = 0;
	for (int i = 0; i < n; ++i) {
		if (list[i].navi != navi) continue;
		if (roll < list[i].weight) { pick = i; break; }
		roll -= list[i].weight;
	}
	const Formation *f = &list[pick];
	e->field = f->battlefield;
	int target = virus_version(depth, challenge);
	bool rare_done = false;
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
		else if (row[1] == 0 && o->family >= 1 && o->family <= 29 && enemy_id(0, o->family, o->version) == o->id) o->kind = FOE_VIRUS;
		else { o->kind = FOE_ROCK; continue; }
		if (o->kind != FOE_VIRUS || o->version > 3) continue;
		/* the depth's version, up or down (the originals are paced for the
		 * story), one rare at most */
		int want = target > 3 ? (rare_done ? 3 : target) : target;
		if (want == o->version) continue;
		int id = enemy_id(0, o->family, want);
		if (id < 0) continue;
		o->version = want;
		o->id = id;
		rare_done |= want > 3;
	}
	return e->nfoes > 0;
}

Encounter make_encounter(int depth, int biome, bool challenge) {
	Encounter e = { 0 };
	e.biome = biome;
	for (int i = 0; i < MAX_FOES; ++i) e.foes[i].id = -1;
	if (original_encounter(depth, biome, challenge, &e)) return e;
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
		f->version = virus_version(depth, challenge);
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
	e.no_escape = true;
	e.nfoes = 1;
	e.foes[0].id = -1;
	e.foes[0].kind = FOE_NAVI;
	e.foes[0].family = navi;
	int loop = (depth - 1) / CYCLE_LAYERS, act = ((depth - 1) % CYCLE_LAYERS) / 3;
	e.foes[0].version = loop > 0 || biome == BIOME_NEST || biome == BIOME_SECRET ? 2 : act >= 3 ? 1 : 0;
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

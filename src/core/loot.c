#include "loot.h"

#include "data.h"
#include "game.h"
#include "run.h"

int virus_version(int depth, bool hard) {
	int loop = (depth - 1) / CYCLE_LAYERS;
	int p = (depth - 1) % CYCLE_LAYERS;
	int v = p < 4 ? 0 : p < 9 ? rng_range(0, 1) : p < 13 ? 1 + (rng_range(0, 3) == 0) : 2;
	v += loop + (hard ? 1 : 0);
	if (v > 3) v = 3;
	if (rng_range(0, 99) < 3) v = 4 + (p > 9); /* rare variants */
	return v;
}

static int field_for(int biome) {
	switch (biome) {
	case BIOME_CENTRAL: return rng_range(0, 3) == 0 ? 1 : 0;
	case BIOME_SEASIDE: return rng_range(0, 2) == 0 ? 5 : 0;
	case BIOME_SKY: return rng_range(0, 2) == 0 ? 4 : 0;
	case BIOME_GREEN: return 2;
	case BIOME_GRAVEYARD: return rng_range(0, 1) ? 1 : 5;
	case BIOME_UNDERNET: return 3;
	case BIOME_SECRET: return 4;
	default: return 5;
	}
}

Encounter make_encounter(int depth, int biome, bool challenge) {
	Encounter e = { 0 };
	e.biome = biome;
	e.field = field_for(biome);
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
	e.field = biome == BIOME_NEST ? 1 : 0;
	e.boss = true;
	e.no_escape = true;
	e.nfoes = 1;
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
	int w[5] = { 60 - p * 3, 30, 6 + p * 2, p > 5 ? p : 0, 0 };
	if (w[0] < 8) w[0] = 8;
	for (int b = 0; b < bonus_tier; ++b) { w[0] /= 2; w[3] += 6; w[2] += 6; }
	int total = w[0] + w[1] + w[2] + w[3] + w[4];
	int roll = rng_range(0, total - 1), tier = 0;
	while (tier < 4 && roll >= w[tier]) roll -= w[tier++];
	int pick[128], n = 0;
	for (int i = 0; i < chip_def_count; ++i)
		if (chip_defs[i].tier == tier && chip_defs[i].kind != CK_NAVI) pick[n++] = chip_defs[i].rom_id;
	if (!n) for (int i = 0; i < chip_def_count; ++i) if (chip_defs[i].tier <= 1) pick[n++] = chip_defs[i].rom_id;
	int id = pick[rng_range(0, n - 1)];
	ChipInfo ci;
	chip_info(id, &ci);
	*code = ci.ncodes ? ci.codes[rng_range(0, ci.ncodes - 1)] : '*';
	return id;
}

int chip_price(int id) {
	const ChipDef *d = chip_def(id);
	int loop = (run.depth - 1) / CYCLE_LAYERS;
	return d->price * 100 * (2 + loop) / 2;
}

/* ROM-free checks: hashing, decompression and map generation. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "net.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* Minimal stand-ins for the engine pieces the generator links against. */
char g_data_dir[512] = ".";
Run run;
static uint32_t rng_s = 1;
void rng_seed(uint32_t s) { rng_s = s ? s : 1; }
uint32_t rng_state(void) { return rng_s; }
uint32_t rng_next(void) { uint32_t x = rng_s; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return rng_s = x; }
int rng_range(int lo, int hi) { return hi <= lo ? lo : lo + (int)(rng_next() % (uint32_t)(hi - lo + 1)); }

static void test_sha1(void) {
	char hex[41];
	sha1_hex((const uint8_t *)"abc", 3, hex);
	CHECK(!strcmp(hex, "a9993e364706816aba3e25717850c26c9cd0d89d"), "sha1(abc) = %s", hex);
	sha1_hex((const uint8_t *)"", 0, hex);
	CHECK(!strcmp(hex, "da39a3ee5e6b4b0d3255bfef95601890afd80709"), "sha1('') = %s", hex);
	static uint8_t block[1000];
	memset(block, 'a', sizeof block);
	sha1_hex(block, 1000, hex);
	CHECK(!strcmp(hex, "291e9a6c66994949b57ba5e650361e98fc36b1ba"), "sha1(a*1000) = %s", hex);
}

static void test_lz77(void) {
	/* "ABCABCABCABCX": 3 literals, one back-reference (len 9, dist 3), 1 literal */
	const uint8_t src[] = { 0x10, 13, 0, 0, 0x10, 'A', 'B', 'C', 0x60, 0x02, 'X' };
	size_t n;
	uint8_t *out = lz77_decompress(src, sizeof src, &n);
	CHECK(out && n == 13 && !memcmp(out, "ABCABCABCABCX", 13), "lz77 decode");
	free(out);
	const uint8_t bad[] = { 0x10, 8, 0, 0, 0x80, 0x00, 0x05 };
	CHECK(lz77_decompress(bad, sizeof bad, &n) == NULL, "lz77 rejects a reference before any output");
}

static int reachable_cells(int sx, int sy, uint8_t seen[MAP_H][MAP_W]) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(seen, 0, MAP_W * MAP_H);
	int h = 0, t = 0, n = 0;
	qx[t] = (int16_t)sx; qy[t++] = (int16_t)sy;
	seen[sy][sx] = 1;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		++n;
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || seen[ny][nx] || layer.cell[ny][nx] != C_PATH) continue;
			seen[ny][nx] = 1;
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	return n;
}

/* A guardian's arena: one way in, the guardian in its middle, the exit
 * inside it, and nothing else there. */
static void check_arena(uint32_t seed) {
	const Room *a = &layer.rooms[layer.arena];
	int ways = 0;
	for (int y = a->y - 1; y <= a->y + a->h; ++y)
		for (int x = a->x - 1; x <= a->x + a->w; ++x) {
			bool inside = x >= a->x && x < a->x + a->w && y >= a->y && y < a->y + a->h;
			bool corner = (x < a->x || x >= a->x + a->w) && (y < a->y || y >= a->y + a->h);
			if (!inside && !corner && layer.cell[y][x] == C_PATH) ++ways;
		}
	CHECK(ways == 1, "seed %u: the arena has %d ways in", seed, ways);
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		bool inside = (int)o->x >= a->x && (int)o->x < a->x + a->w && (int)o->y >= a->y && (int)o->y < a->y + a->h;
		if (o->type == OBJ_BOSS) CHECK((int)o->x == a->ax && (int)o->y == a->ay, "seed %u: the guardian is off the arena's middle", seed);
		else if (o->type == OBJ_EXIT || o->type == OBJ_RETURN) CHECK(inside, "seed %u: the exit is outside the arena", seed);
		else CHECK(!inside, "seed %u: object type %d in the arena", seed, o->type);
	}
	CHECK(layer.ante >= 0 && layer.ante != layer.arena, "seed %u: no antechamber", seed);
}

static void test_generation(void) {
	static uint8_t seen[MAP_H][MAP_W];
	int boss_layers = 0, arenas = 0;
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	for (uint32_t seed = 1; seed <= 300; ++seed) {
		int depth = 1 + (int)(seed % 25);
		int kind = seed % 7 == 0 ? LAYER_UNDERNET : seed % 11 == 0 ? LAYER_SECRET : LAYER_NORMAL;
		layer_generate(seed * 7919u, depth, biome_for_depth(depth), kind, 3u, 32);
		CHECK(layer.nrooms >= 3, "seed %u: only %d rooms", seed, layer.nrooms);
		NetObj *start = &layer.obj[0];
		CHECK(start->type == OBJ_WARP_IN, "seed %u: first object is the arrival warp", seed);
		int cells = 0;
		for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) cells += layer.cell[y][x] == C_PATH;
		int reach = reachable_cells((int)start->x, (int)start->y, seen);
		CHECK(reach == cells, "seed %u: %d of %d walkable cells reachable", seed, reach, cells);
		bool has_exit = false;
		for (int i = 0; i < layer.nobj; ++i) {
			NetObj *o = &layer.obj[i];
			CHECK(layer.cell[(int)o->y][(int)o->x] == C_PATH, "seed %u: object %d off the path", seed, i);
			for (int j = i + 1; j < layer.nobj; ++j)
				CHECK((int)o->x != (int)layer.obj[j].x || (int)o->y != (int)layer.obj[j].y, "seed %u: objects %d and %d overlap", seed, i, j);
			if (o->type == OBJ_EXIT || o->type == OBJ_RETURN) has_exit = true;
		}
		CHECK(has_exit, "seed %u: no way out", seed);
		int traders = 0;
		for (int i = 0; i < layer.nobj; ++i) traders += layer.obj[i].type == OBJ_TRADER || layer.obj[i].type == OBJ_BUGTRADER;
		CHECK(traders <= 1, "seed %u: %d traders (the trade screen serves one per map)", seed, traders);
		if (layer.boss_layer) {
			bool boss = false;
			for (int i = 0; i < layer.nobj; ++i) boss |= layer.obj[i].type == OBJ_BOSS;
			CHECK(boss, "seed %u: boss layer without a boss", seed);
			boss_layers++;
			if (layer.arena >= 0) { arenas++; check_arena(seed); }
		}
	}
	CHECK(arenas * 10 >= boss_layers * 9, "only %d of %d guardians have an arena", arenas, boss_layers);
	/* Determinism: the same seed builds the same layer. */
	layer_generate(1234, 5, BIOME_SKY, LAYER_NORMAL, 3u, 32);
	static Layer a;
	a = layer;
	layer_generate(1234, 5, BIOME_SKY, LAYER_NORMAL, 3u, 32);
	CHECK(!memcmp(a.cell, layer.cell, sizeof a.cell) && a.nobj == layer.nobj, "generation is deterministic");
}

static bool on_stair(int x, int y) {
	for (int i = 0; i < layer.nstairs; ++i)
		if (x >= layer.stair[i].x && x < layer.stair[i].x + 2 && y >= layer.stair[i].y && y < layer.stair[i].y + 2) return true;
	return false;
}

/* Raised rooms are reached only by their stair, which climbs from a ground
 * landing to the room's floor. */
static void test_stairs(void) {
	int layers = 0;
	for (uint32_t seed = 1; seed <= 400; ++seed) {
		int depth = 1 + (int)(seed % 25);
		layer_generate(seed * 7919u, depth, biome_for_depth(depth), LAYER_NORMAL, 3u, 32);
		if (!layer.nstairs) continue;
		++layers;
		CHECK(layer.rise == 32, "seed %u: rise %d", seed, layer.rise);
		for (int i = 0; i < layer.nstairs; ++i) {
			const Stair *st = &layer.stair[i];
			bool nx = st->dir == STAIR_UP_NX;
			for (int k = 0; k < 2; ++k) {
				int tx = nx ? st->x - 1 : st->x + k, ty = nx ? st->y + k : st->y - 1;       /* above the top */
				int fx = nx ? st->x + 2 : st->x + k, fy = nx ? st->y + k : st->y + 2;       /* below the foot */
				CHECK(layer.level[ty][tx] && layer.cell[ty][tx] == C_PATH, "seed %u: stair %d tops onto no raised floor", seed, i);
				CHECK(!layer.level[fy][fx] && layer.cell[fy][fx] == C_PATH, "seed %u: stair %d has no landing", seed, i);
			}
		}
		for (int y = 1; y < MAP_H - 1; ++y)
			for (int x = 1; x < MAP_W - 1; ++x) {
				if (!layer.level[y][x]) continue;
				static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
				for (int k = 0; k < 4; ++k) {
					int ax = x + d[k][0], ay = y + d[k][1];
					bool ok = layer.cell[ay][ax] != C_PATH || layer.level[ay][ax] || on_stair(ax, ay);
					CHECK(ok, "seed %u: raised floor at %d,%d touches the ground", seed, x, y);
				}
			}
		for (int i = 0; i < layer.nobj; ++i)
			CHECK(!on_stair((int)layer.obj[i].x, (int)layer.obj[i].y), "seed %u: object %d on a stair", seed, i);
	}
	CHECK(layers > 10, "only %d of 400 layers have a stair", layers);
	/* an area without stairs keeps its layers flat */
	layer_generate(7919u, 2, BIOME_SKY, LAYER_NORMAL, 0u, 0);
	CHECK(!layer.nstairs && !layer.rise, "flat area got a stair");
}

static void test_depth_plan(void) {
	memset(&run, 0, sizeof run);
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	CHECK(!is_boss_depth(1) && !is_boss_depth(2) && is_boss_depth(3), "every third layer has a boss");
	CHECK(biome_for_depth(19) == BIOME_NEST && is_boss_depth(19), "layer 19 is the Cybeast Nest");
	CHECK(biome_for_depth(20) == run.biome_order[0], "the cycle restarts after the Nest");
}


/* HP by navi and version (V1, EX, SP) as the ROM has them, for the checks */
static int fake_navi_hp(int navi, int version) {
	static const int hp[19][3] = {
		[1] = { 700, 1500, 1900 }, [2] = { 900, 1400, 1800 }, [3] = { 800, 1200, 1500 }, [4] = { 800, 1200, 1600 },
		[5] = { 1000, 1500, 2000 }, [6] = { 600, 1300, 1700 }, [7] = { 1000, 1500, 2000 }, [8] = { 800, 1200, 1500 },
		[9] = { 1000, 1500, 2000 }, [10] = { 900, 1300, 1800 }, [11] = { 1800, 2000, 2000 }, [12] = { 400, 800, 1400 },
		[13] = { 500, 1000, 1500 }, [14] = { 700, 1200, 1600 }, [15] = { 800, 1100, 1600 }, [16] = { 900, 1300, 1700 },
		[18] = { 1200, 1600, 2000 },
	};
	return navi > 0 && navi < 19 && version >= 0 && version < 3 && hp[navi][0] ? hp[navi][version] : -1;
}

static void test_pacing(void) {
	/* acts and cycles */
	CHECK(pacing_act(1) == 0 && pacing_act(3) == 0 && pacing_act(4) == 1 && pacing_act(18) == 5 && pacing_act(19) == 6,
		"acts of three layers, then the Nest");
	CHECK(pacing_act(20) == 0 && pacing_loop(20) == 1 && pacing_loop(19) == 0, "the second cycle begins at 20");
	/* bands rise with the acts and the cycles; an easy battle stays below */
	for (int d = 1; d < 19; ++d) {
		PacingBand a = pacing_band(d, false, false), b = pacing_band(d + 1, false, false), e = pacing_band(d, false, true);
		CHECK(b.hi >= a.hi && b.cap >= a.cap, "the band never falls from depth %d to %d", d, d + 1);
		CHECK(e.hi <= a.hi && e.hi >= a.lo, "the easy band at depth %d lies in the lower half", d);
		CHECK(pacing_band(d, true, false).hi >= a.hi, "a challenge at depth %d is at least as hard", d);
		CHECK(pacing_band(d + CYCLE_LAYERS, false, false).hi > a.hi, "the next cycle is harder at depth %d", d);
	}
	CHECK(pacing_band(1, false, false).cap <= 50, "the first act keeps hits to half of 100 HP");
	/* versions: V1 through two acts, never above SP, no rares early */
	for (int i = 0; i < 200; ++i) {
		CHECK(pacing_virus_version(1 + i % 6, false) == 0, "V1 in the first two acts");
		int v = pacing_virus_version(1 + i % 60, i & 1);
		CHECK(v >= 0 && v <= 3, "a version from V1 to SP");
	}
	for (int roll = 0; roll < 100; ++roll) CHECK(!pacing_rare(1 + roll % 6, roll), "no rare virus in the first two acts");
	/* areas: easy first, late last, none twice */
	static const int opening[] = { BIOME_CENTRAL, BIOME_ROBOT_COMP, BIOME_AQUARIUM_COMP, BIOME_SKY_HP, BIOME_COMP };
	static const int late[] = { BIOME_SKY, BIOME_WEATHER_COMP, BIOME_ACDC_HP, BIOME_COPYBOT_COMP };
	for (uint32_t seed = 1; seed <= 300; ++seed) {
		rng_seed(seed);
		uint8_t o[4];
		pacing_area_order(o);
		bool first = false, last = false;
		for (int i = 0; i < 5; ++i) first |= o[0] == opening[i];
		for (int i = 0; i < 4; ++i) last |= o[3] == late[i];
		CHECK(first, "seed %u: act 1 in an opening area (%d)", seed, o[0]);
		CHECK(last, "seed %u: act 4 in a late area (%d)", seed, o[3]);
		for (int i = 0; i < 4; ++i)
			for (int j = i + 1; j < 4; ++j) CHECK(o[i] != o[j], "seed %u: an area twice", seed);
	}
	/* guardians: the first act meets a light navi even from a heavy pool */
	static const uint8_t others[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
	static const uint8_t heavy[4] = { 18, 15, 12, 18 }, sky_hp[4] = { 4, 10, 16, 4 };
	for (uint32_t seed = 1; seed <= 100; ++seed) {
		rng_seed(seed);
		int g = pacing_guardian_pick(heavy, others, (int)sizeof others, 0, 0, false, fake_navi_hp);
		CHECK(g == 12, "the lab comps' first-act guardian is BlastMan, got %d", g);
		g = pacing_guardian_pick(sky_hp, others, (int)sizeof others, 0, 0, false, fake_navi_hp);
		CHECK(fake_navi_hp(g, 0) <= 600, "a first-act guardian of 600 HP at most, got %d", g);
	}
	CHECK(pacing_guardian_version(18, 3, 0, false, fake_navi_hp) == 0, "Colonel V1 in the fourth act");
	CHECK(pacing_guardian_version(3, 3, 0, false, fake_navi_hp) == 1, "SlashMan EX in the fourth act");
	CHECK(pacing_guardian_version(3, 0, 1, false, fake_navi_hp) == 2, "SP on the second cycle");
	/* heals: the middle layer of each act on the first two cycles */
	CHECK(pacing_heal_certain(2) && pacing_heal_certain(17) && !pacing_heal_certain(1) && !pacing_heal_certain(19),
		"a heal on each act's middle layer");
	CHECK(pacing_heal_certain(2 + CYCLE_LAYERS) && !pacing_heal_certain(2 + 2 * CYCLE_LAYERS), "the third cycle drops it");
}

int main(void) {
	test_sha1();
	test_lz77();
	test_generation();
	test_stairs();
	test_depth_plan();
	test_pacing();
	if (failures) { printf("%d check(s) failed\n", failures); return 1; }
	printf("all core checks passed\n");
	return 0;
}

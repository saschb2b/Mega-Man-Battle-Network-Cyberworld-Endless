/* ROM-free checks: hashing, decompression and map generation. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "net.h"
#include "net_route.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "area_src.h"
#include "guardians.h"
#include "npc_lines.h"
#include "powers.h"
#include "rivals.h"
#include "text.h"
#include "townmath.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* Minimal stand-ins for the engine pieces the generator links against. */
char g_data_dir[512] = ".";
Run run;
static uint32_t rng_s = 1;
/* (as the game's: the seed mixed first) */
void rng_seed(uint32_t s) {
	s += 0x9E3779B9u;
	s ^= s >> 16; s *= 0x85EBCA6Bu;
	s ^= s >> 13; s *= 0xC2B2AE35u;
	s ^= s >> 16;
	rng_s = s ? s : 0x9E3779B9u;
}
uint32_t rng_state(void) { return rng_s; }
uint32_t rng_next(void) { uint32_t x = rng_s; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return rng_s = x; }
int rng_range(int lo, int hi) { return hi <= lo ? lo : lo + (int)(rng_next() % (uint32_t)(hi - lo + 1)); }
/* (the talk's pieces need no saves or game flags here) */
void flag_set(int flag) { (void)flag; }
bool save_write_blob(const char *name, uint32_t magic, const void *data, size_t n) { (void)name; (void)magic; (void)data; (void)n; return false; }
bool save_read_blob(const char *name, uint32_t magic, void *data, size_t n) { (void)name; (void)magic; (void)data; (void)n; return false; }
uint32_t mapslot_alloc(const void *bytes, int len) { (void)bytes; (void)len; return 0; }

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

/* Whether (x, y) is beside a panel-wide stretch of floor: a walkway's
 * mouth, where a navi stands in the way on */
static bool beside_narrow(int x, int y) {
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < 4; ++k) {
		int nx = x + d[k][0], ny = y + d[k][1];
		if (nx < 1 || ny < 1 || nx >= MAP_W - 1 || ny >= MAP_H - 1 || layer.cell[ny][nx] != C_PATH) continue;
		int ax = d[k][1], ay = d[k][0];   /* across the step */
		if (layer.cell[ny + ay][nx + ax] != C_PATH && layer.cell[ny - ay][nx - ax] != C_PATH) return true;
	}
	return false;
}

static void test_generation(void) {
	static uint8_t seen[MAP_H][MAP_W];
	int boss_layers = 0, arenas = 0, mouths = 0, standing = 0;
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
		/* ... and with the solid objects (navis, Mystery Data, services)
		 * standing in the way, every other cell still; the guardian keeps
		 * its arena until beaten, so it does not count */
		{
			static uint8_t save[MAP_H][MAP_W];
			memcpy(save, layer.cell, sizeof save);
			int blocked = 0;
			for (int i = 1; i < layer.nobj; ++i) {
				const NetObj *o = &layer.obj[i];
				if (!o->solid || o->type == OBJ_BOSS) continue;
				layer.cell[(int)o->y][(int)o->x] = C_VOID;
				++blocked;
			}
			int open = reachable_cells((int)start->x, (int)start->y, seen);
			CHECK(open == cells - blocked, "seed %u: objects cut the way (%d of %d cells)", seed, open, cells - blocked);
			memcpy(layer.cell, save, sizeof save);
		}
		bool has_exit = false;
		for (int i = 0; i < layer.nobj; ++i) {
			NetObj *o = &layer.obj[i];
			CHECK(layer.cell[(int)o->y][(int)o->x] == C_PATH, "seed %u: object %d off the path", seed, i);
			for (int j = i + 1; j < layer.nobj; ++j) {
				const NetObj *q = &layer.obj[j];
				CHECK((int)o->x != (int)q->x || (int)o->y != (int)q->y, "seed %u: objects %d and %d overlap", seed, i, j);
				/* no two solid objects side by side (the guardian keeps its arena) */
				if (o->solid && q->solid && o->type != OBJ_BOSS && q->type != OBJ_BOSS)
					CHECK(abs((int)o->x - (int)q->x) > 1 || abs((int)o->y - (int)q->y) > 1, "seed %u: objects %d and %d side by side", seed, i, j);
			}
			if (o->type == OBJ_EXIT || o->type == OBJ_RETURN) has_exit = true;
		}
		CHECK(has_exit, "seed %u: no way out", seed);
		/* services and navis off the walkways' mouths */
		for (int i = 1; i < layer.nobj; ++i) {
			const NetObj *o = &layer.obj[i];
			bool stands = o->type == OBJ_SHOP || o->type == OBJ_HEAL || o->type == OBJ_TRADER || o->type == OBJ_BUGTRADER ||
				o->type == OBJ_NPC || o->type == OBJ_CHALLENGE || o->type == OBJ_PROGRAMS || o->type == OBJ_GIFT;
			if (stands && beside_narrow((int)o->x, (int)o->y)) ++mouths;
			if (stands) ++standing;
		}
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
	CHECK(mouths == 0, "%d of %d services and navis stand at a walkway's mouth", mouths, standing);
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
		CHECK(g == 12 || g == 13, "the lab comps' first-act guardian is BlastMan or DiveMan, got %d", g);
		g = pacing_guardian_pick(sky_hp, others, (int)sizeof others, 0, 0, false, fake_navi_hp);
		CHECK(fake_navi_hp(g, 0) <= 500, "a first-act guardian of 500 HP at most, got %d", g);
	}
	CHECK(pacing_guardian_version(18, 3, 0, false, fake_navi_hp) == 0, "Colonel V1 in the fourth act");
	CHECK(pacing_guardian_version(3, 3, 0, false, fake_navi_hp) == 1, "SlashMan EX in the fourth act");
	CHECK(pacing_guardian_version(3, 0, 1, false, fake_navi_hp) == 2, "SP on the second cycle");
	/* heals: the middle layer of each act on the first two cycles, and the
	 * run's first layer */
	CHECK(pacing_heal_certain(2) && pacing_heal_certain(17) && pacing_heal_certain(1) && !pacing_heal_certain(3) &&
		!pacing_heal_certain(19) && !pacing_heal_certain(1 + CYCLE_LAYERS), "a heal on each act's middle layer and the first");
	CHECK(pacing_heal_certain(2 + CYCLE_LAYERS) && !pacing_heal_certain(2 + 2 * CYCLE_LAYERS), "the third cycle drops it");
}

/* The town's moves (src/world/townmath.h): a moved tile shows the same
 * world point the source tile showed, moved. */
static void test_town_moves(void) {
	CHECK(town_move_keeps_tiles(0, 0) && town_move_keeps_tiles(-24, 0) && town_move_keeps_tiles(1, 1) && town_move_keeps_tiles(36, 0), "even moves keep the tile grid");
	CHECK(!town_move_keeps_tiles(1, 0) && !town_move_keeps_tiles(0, -3), "odd moves do not");
	CHECK(town_move_keeps_pattern(8, 0) && town_move_keeps_pattern(4, 4) && town_move_keeps_pattern(4, -4) && town_move_keeps_pattern(-4, -12),
		"moves on the (4, 4), (4, -4) lattice keep the brick");
	CHECK(!town_move_keeps_pattern(0, 4) && !town_move_keeps_pattern(2, 2) && !town_move_keeps_pattern(36, 0), "others do not");
	/* Central Town's 132 x 72 tiles onto a 152 x 84 map, moves of every kind */
	static const int moves[][2] = { { 0, 0 }, { 8, 0 }, { 1, 1 }, { -24, 0 }, { 36, 0 }, { -4, -12 }, { 3, -5 } };
	for (int m = 0; m < (int)(sizeof moves / sizeof *moves); ++m)
		for (int wx = -200; wx <= 200; wx += 38)   /* (even: the half pixel of an odd y - x rounds either way) */
			for (int wy = -160; wy <= 320; wy += 42) {
				int dx = moves[m][0], dy = moves[m][1];
				int sx = area_px(132, wx, wy), sy = area_py(72, wx, wy);
				int tx = area_px(152, wx + 8 * dx, wy + 8 * dy), ty = area_py(84, wx + 8 * dx, wy + 8 * dy);
				int mx, my;
				town_move_tile(sx >> 3, sy >> 3, 132, 72, dx, dy, 152, 84, &mx, &my);
				CHECK(mx == tx >> 3 && my == ty >> 3 && (sx & 7) == (tx & 7) && (sy & 7) == (ty & 7),
					"move (%d, %d): world (%d, %d) in tile (%d, %d), moved to (%d, %d), drawn in (%d, %d)", dx, dy, wx, wy, sx >> 3, sy >> 3, mx, my, tx >> 3, ty >> 3);
			}
}

/* Text for the chat box (ta_talk's boxes): only characters the game's
 * charmap has, known speaker marks, boxes that fit ta_pages' buffer and
 * words that fit a line. */
static void check_pages(const char *what, const char *s);
static void check_talk(const char *what, const char *s) {
	if (!s) return;
	check_pages(what, s);
	static const char marks[] = "LMDPBCYHN";
	static const char punct[] = " *-=:%?+!&,.;'\"~/()>_\n|";
	int box = 0, word = 0;
	for (const char *p = s; *p; ++p) {
		if (*p == '@') {
			CHECK((p == s || p[-1] == '|') && p[1] && strchr(marks, p[1]), "%s: a speaker mark out of place or unknown in \"%s\"", what, s);
			if (p[1]) ++p;
			continue;
		}
		bool ok = (*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || strchr(punct, *p);
		CHECK(ok, "%s: '%c' is not in the charmap: \"%s\"", what, *p, s);
		if (*p == '|') { box = word = 0; continue; }
		CHECK(++box < 197, "%s: a box over ta_pages' buffer with its speaker mark: \"%s\"", what, s);
		word = *p == ' ' || *p == '\n' ? 0 : word + 1;
		CHECK(word <= 20, "%s: a word longer than a line: \"%s\"", what, s);
	}
	CHECK(s[0] && s[strlen(s) - 1] != '|', "%s: an empty box: \"%s\"", what, s);
}

/* The pages ta_talk builds from `s`: at most three lines of at most 20
 * characters, and no page with a line alone in a box of more. */
static void check_pages(const char *what, const char *s) {
	if (!s) return;
	static TextArchive t;
	ta_begin(&t);
	ta_talk(&t, s, FACE_MEGAMAN);
	CHECK(!t.full, "%s: the archive overflowed", what);
	int page_lines[16], pages = 0, lines = 0, chars = 0;
	bool in_text = false;
	for (int i = 0; i <= t.len; ++i) {
		int b = i < t.len ? t.buf[i] : 0xE6;
		bool page_end = b == 0xF2 || b == 0xF5 || b == 0xE6;
		if (b == 0xE9) { ++lines; chars = 0; continue; }
		if (page_end || b == 0xE7 || b == 0xE8) {
			if (in_text) { ++lines; in_text = false; }
			if (b == 0xE7 || b == 0xE8) { ++i; continue; }   /* (their argument) */
			if (lines && pages < 16) page_lines[pages++] = lines;
			lines = chars = 0;
			if (b == 0xF5) {
				/* a new box: its pages checked, then forgotten */
				int total = 0;
				for (int k = 0; k < pages; ++k) total += page_lines[k];
				for (int k = 0; k < pages; ++k) {
					CHECK(page_lines[k] <= 3, "%s: a page of %d lines", what, page_lines[k]);
					CHECK(page_lines[k] > 1 || total == 1, "%s: a line alone on a page", what);
				}
				pages = 0;
				i += t.buf[i + 1] == 0x00 ? 2 : 1;   /* F5 00 face, F5 01 */
			}
			continue;
		}
		in_text = true;
		CHECK(++chars <= 20, "%s: a line over 20 characters", what);
	}
	int total = 0;
	for (int k = 0; k < pages; ++k) total += page_lines[k];
	for (int k = 0; k < pages; ++k) {
		CHECK(page_lines[k] <= 3, "%s: a page of %d lines", what, page_lines[k]);
		CHECK(page_lines[k] > 1 || total == 1, "%s: a line alone on a page", what);
	}
}

static void test_talk(void) {
	char what[96];
	/* a Navi chip's version mark (byte 3, [EX]) is spelt in a chat box */
	uint8_t enc[16];
	int n = ta_encode("BlastMn\3 B", enc, (int)sizeof enc);
	CHECK(n == 11 && enc[7] == 0x0B + ('E' - 'A') && enc[8] == 0x0B + ('X' - 'A') && enc[9] == 0x00, "version mark spelt: %d bytes", n);
	for (int depth = 1; depth <= 60; ++depth)
		for (int i = 0; i < 40; ++i) {
			snprintf(what, sizeof what, "npc_line(%d, %d)", depth, i);
			check_talk(what, npc_line(depth, i));
		}
	/* no repeats among a layer's first bystanders */
	for (int depth = 1; depth <= 40; depth += 3)
		for (int a = 0; a < 4; ++a)
			for (int b = a + 1; b < 4; ++b)
				CHECK(npc_line(depth, 7 + a) != npc_line(depth, 7 + b), "npc_line repeats at depth %d", depth);
	memset(&run, 0, sizeof run);
	for (int navi = 1; navi < 32; ++navi) {
		/* every history: a first meeting, a loss, wins up to respect */
		for (int step = 0; step < 7; ++step) {
			if (step == 1) { rival_met(navi); rival_result(navi, RIVAL_NAVI_WON); }
			else if (step > 1) { rival_met(navi); rival_result(navi, RIVAL_MEGAMAN_WON); }
			for (int version = 0; version < 3; ++version)
				for (int biome = 0; biome < BIOME_COUNT; ++biome) {
					snprintf(what, sizeof what, "guardian_intro(%d, %d, %d) step %d", navi, version, biome, step);
					check_talk(what, guardian_intro(navi, version, biome));
				}
		}
		snprintf(what, sizeof what, "guardian_defeat(%d)", navi);
		check_talk(what, guardian_defeat(navi));
		snprintf(what, sizeof what, "guardian_tip(%d)", navi);
		check_talk(what, guardian_tip(navi));
		/* (MegaMan warns of every guardian a run can meet) */
		CHECK(guardian_tip(navi) || navi == 17 || navi > 18, "guardian %d has no tip", navi);
		for (int depth = 3; depth <= 19; depth += 3) {
			snprintf(what, sizeof what, "powers_reward_text(%d, graveyard, %d)", navi, depth);
			check_talk(what, powers_reward_text(navi, BIOME_GRAVEYARD, depth));
		}
	}
	for (int biome = 0; biome < BIOME_COUNT; ++biome)
		for (int side = LAYER_NORMAL; side <= LAYER_SECRET; ++side)
			CHECK(strlen(guardian_area_in_text(biome, side)) < 28, "area %d's name is long for the cards", biome);
}

/* Following the arrow gets MegaMan there: from the arrival and from each
 * room, walking the way it shows at a run's pace (a look every 5 frames, a
 * new way taken when two looks agree, as the director's arrow_update),
 * sliding along the floor's edges and turned into a walkway's mouth (the
 * director's unwedge), he reaches the guardian or the exit pad. (It led
 * past a turn, into a platform's corner, round in circles.) */
static bool walk_floor(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }

/* whether a solid object (a Mystery Data, a navi) stands within MegaMan's
 * reach of (x, y): the game's are circles smaller than a panel, so he can
 * stand in the corner of one's panel */
static bool walk_blocked(double x, double y, int gx, int gy) {
	for (int i = 1; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		if (!o->solid || ((int)o->x == gx && (int)o->y == gy)) continue;
		if ((x - (int)o->x) * (x - (int)o->x) + (y - (int)o->y) * (y - (int)o->y) < 0.45 * 0.45) return true;
	}
	return false;
}

/* one frame's step from (x, y) by (dx, dy), or along one axis of it, short
 * of the objects but the goal (gx, gy) */
static bool walk_step(double *x, double *y, double dx, double dy, int gx, int gy) {
	int cx = (int)lround(*x), cy = (int)lround(*y);
	const double tries[3][2] = { { dx, dy }, { dx, 0 }, { 0, dy } };
	for (int i = 0; i < 3; ++i) {
		if (fabs(tries[i][0]) + fabs(tries[i][1]) < 1e-9) continue;
		double nx = *x + tries[i][0], ny = *y + tries[i][1];
		int ncx = (int)lround(nx), ncy = (int)lround(ny);
		if (!walk_floor(ncx, ncy) || walk_blocked(nx, ny, gx, gy)) continue;
		if (ncx != cx && ncy != cy && !walk_floor(ncx, cy) && !walk_floor(cx, ncy)) continue;
		*x = nx;
		*y = ny;
		return true;
	}
	return false;
}

/* frames to get beside (tx, ty) from (x, y) following the arrow, -1 never */
static int follow_arrow(double x, double y, int tx, int ty) {
	const double run = 2.0 / 32;   /* panels a frame */
	int len, shown = route_way(x, y, tx, ty, &len), pending = shown, stuck = 0;
	if (shown < 0) return -1;
	int budget = 200 + 48 * len;
	for (int f = 1; f <= budget; ++f) {
		int cx = (int)lround(x), cy = (int)lround(y);
		if (abs(cx - tx) + abs(cy - ty) <= 1) return f;
		if (f % 5 == 0) {
			int w = route_way(x, y, tx, ty, &len);
			if (w >= 0 && w == pending) shown = w;
			pending = w;
		}
		/* the pad's way on the grid (RIGHT +x -y, DOWN +x +y) */
		double a = shown * 3.14159265358979 / 4, right = cos(a), down = sin(a);
		double dx = (right + down) / 2, dy = (down - right) / 2, n = sqrt(dx * dx + dy * dy);
		dx = fabs(dx) < 1e-9 ? 0 : dx / n * run;
		dy = fabs(dy) < 1e-9 ? 0 : dy / n * run;
		if (walk_step(&x, &y, dx, dy, tx, ty)) { stuck = 0; continue; }
		/* along one axis into an edge: towards the side the floor goes on */
		if (++stuck >= 3 && (!dx || !dy)) {
			int ax = dx > 0 ? 1 : dx < 0 ? -1 : 0, ay = dy > 0 ? 1 : dy < 0 ? -1 : 0;
			for (int s = -1; s <= 1; s += 2) {
				int bx = ay ? s : 0, by = ax ? s : 0;
				if (!walk_floor(cx + ax, cy + ay) && walk_floor(cx + bx, cy + by) && walk_floor(cx + ax + bx, cy + ay + by)) {
					walk_step(&x, &y, bx * run, by * run, tx, ty);
					break;
				}
			}
		}
	}
	return -1;
}

static void test_arrow(void) {
	int walks = 0, lost = 0;
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	for (uint32_t seed = 1; seed <= 120; ++seed) {
		int depth = 1 + (int)(seed % 25);
		int kind = seed % 7 == 0 ? LAYER_UNDERNET : seed % 11 == 0 ? LAYER_SECRET : LAYER_NORMAL;
		layer_generate(seed * 7919u, depth, biome_for_depth(depth), kind, 3u, 32);
		int tx = -1, ty = -1;
		for (int i = 0; i < layer.nobj; ++i) {
			const NetObj *o = &layer.obj[i];
			bool goal = layer.boss_layer ? o->type == OBJ_BOSS : o->type == OBJ_EXIT || o->type == OBJ_RETURN;
			if (goal && tx < 0) { tx = (int)o->x; ty = (int)o->y; }
		}
		if (tx < 0) continue;
		for (int r = -1; r < layer.nrooms; ++r) {
			int sx = r < 0 ? (int)layer.obj[0].x : layer.rooms[r].ax, sy = r < 0 ? (int)layer.obj[0].y : layer.rooms[r].ay;
			if (abs(sx - tx) + abs(sy - ty) <= 1) continue;
			++walks;
			if (follow_arrow(sx, sy, tx, ty) >= 0) continue;
			if (++lost <= 5) printf("  seed %u (depth %d): lost from %d,%d on the way to %d,%d\n", seed, depth, sx, sy, tx, ty);
		}
	}
	CHECK(lost == 0, "the arrow lost MegaMan on %d of %d walks", lost, walks);
}

int main(void) {
	test_sha1();
	test_lz77();
	test_generation();
	test_stairs();
	test_arrow();
	test_depth_plan();
	test_pacing();
	test_town_moves();
	test_talk();
	if (failures) { printf("%d check(s) failed\n", failures); return 1; }
	printf("all core checks passed\n");
	return 0;
}

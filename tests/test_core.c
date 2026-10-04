/* ROM-free checks: hashing, decompression and map generation. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "net.h"
#include "net_arena.h"
#include "net_layouts.h"
#include "net_route.h"
#include "net_shapes.h"
#include "layer_make.h"
#include "navicust.h"
#include "pacing.h"
#include "rom.h"
#include "run.h"
#include "area_src.h"
#include "guardians.h"
#include "npc_lines.h"
#include "powers.h"
#include "rivals.h"
#include "save.h"
#include "save_blob.h"
#include "flags.h"
#include "mapslot.h"
#include "text.h"
#include "townmath.h"
#include "touch_layout.h"
#include "buttons.h"
#include "xnavi.h"
#include "xsong.h"
#include "debug.h"
#include "emu.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* Minimal stand-ins for the engine pieces the generator links against. */
char g_data_dir[512] = ".";
Run run;
Profile profile;
/* (no ROM, so no other game's areas: each BN6 area draws itself) */
int run_dress(int biome) { return biome; }
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
/* (another game's songs: no core to copy them into) */
void emu_write(uint32_t addr, const void *data, size_t len) { (void)addr; (void)data; (void)len; }
uint32_t emu_read32(uint32_t addr) { (void)addr; return 0; }
bool emu_debug_on(void) { return false; }
void emu_write32(uint32_t addr, uint32_t v) { (void)addr; (void)v; }

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
	/* (fuzzed, under the tests' sanitizers: 20000 blocks of random bytes
	 * behind its header, and every cut of the good one; a read or write
	 * out of bounds fails the run, issue #19) */
	static uint8_t junk[4096];
	uint32_t x = 0x2545F491u;
	for (int i = 0; i < 20000; ++i) {
		size_t len = 4 + (size_t)(i % 97) * 40;
		for (size_t k = 0; k < len; ++k) { x ^= x << 13; x ^= x >> 17; x ^= x << 5; junk[k] = (uint8_t)x; }
		junk[0] = 0x10;
		junk[3] = 0;   /* (a declared size under 64 KB) */
		free(lz77_decompress(junk, len, &n));
	}
	for (size_t cut = 0; cut < sizeof src; ++cut) free(lz77_decompress(src, cut, &n));
}

static int reachable_cells(int sx, int sy, uint8_t seen[MAP_H][MAP_W]) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(seen, 0, MAP_W * MAP_H);
	int h = 0, t = 0, n = 0;
	qx[t] = (int16_t)sx; qy[t++] = (int16_t)sy;
	seen[sy][sx] = 1;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		n += layer.cell[y][x] == C_PATH;
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			/* (4: a teleport's way, gaps_bridge) */
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || seen[ny][nx] || (layer.cell[ny][nx] != C_PATH && layer.cell[ny][nx] != 4)) continue;
			seen[ny][nx] = 1;
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	return n;
}

/* The arena's way in quiet back to its antechamber (boss.c's door_quiet
 * holds random battles off arena_approach's panels while the guardian
 * waits: a playtester met one on the bridge, session 64): every panel of
 * the bridge from the arena's door (wx, wy) back into the antechamber's
 * box is on it, and the arena's own are not. */
static void check_approach(uint32_t seed, int wx, int wy) {
	const Room *r = &layer.rooms[layer.ante], *a = &layer.rooms[layer.arena];
	int back = (layer.arena_dir + 2) % 4, x = wx, y = wy, steps = 0;
	#define ON_MAP(x, y) ((x) >= 0 && (y) >= 0 && (x) < MAP_W && (y) < MAP_H)
	while (ON_MAP(x, y) && layer.cell[y][x] == C_PATH && !(x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) && steps++ < 16) {
		CHECK(arena_approach(x, y), "seed %u: the bridge's panel %d %d is off the arena's approach", seed, x, y);
		x += dir_dx[back];
		y += dir_dy[back];
	}
	CHECK(ON_MAP(x, y) && layer.cell[y][x] == C_PATH && arena_approach(x, y), "seed %u: the bridge does not lead back into the antechamber", seed);
	#undef ON_MAP
	CHECK(!arena_approach(a->ax, a->ay), "seed %u: the arena's middle is on its approach", seed);
}

/* A guardian's arena: one way in, the guardian in its middle, the exit
 * inside it, and nothing else there. */
static void check_arena(uint32_t seed) {
	const Room *a = &layer.rooms[layer.arena];
	int ways = 0, wx = -1, wy = -1;
	for (int y = a->y - 1; y <= a->y + a->h; ++y)
		for (int x = a->x - 1; x <= a->x + a->w; ++x) {
			bool inside = x >= a->x && x < a->x + a->w && y >= a->y && y < a->y + a->h;
			bool corner = (x < a->x || x >= a->x + a->w) && (y < a->y || y >= a->y + a->h);
			if (!inside && !corner && layer.cell[y][x] == C_PATH) { ++ways; wx = x; wy = y; }
		}
	CHECK(ways == 1, "seed %u: the arena has %d ways in", seed, ways);
	if (ways == 1 && layer.ante >= 0) check_approach(seed, wx, wy);
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

/* A kit as the surface areas' (stairs both ways, a two-panel counter each
 * way) with every sprite prop, and one with none of them. */
static const LayerKit kit = { 3u, 32, { 2, 2 }, 0xFFu, true, true, 0xFu };
static const LayerKit undernet_kit = { 3u, 32, { 2, 2 }, 1u << LOOK_STATUE | 1u << LOOK_BRAZIER, true, true, 0xFu };
static const LayerKit flat = { 0u, 0, { 0, 0 }, 0u, false };

/* A counter's cells: the aisle behind it (d 0), its own (1) and the floor
 * before it (2), t along its run. */
static void counter_cell(const NetProp *p, int t, int d, int *x, int *y) {
	*x = p->faces == FACES_X ? p->x - 1 + d : p->x + t;
	*y = p->faces == FACES_X ? p->y + t : p->y - 1 + d;
}

/* Whether a floor lies a panel's gap in front of (x, y), down-left or
 * down-right on the screen: the cell's own wall hides the gap, and from
 * that floor what stands here looks a step away */
static bool behind_gap(int x, int y) {
	return (x + 2 < MAP_W && layer.cell[y][x + 1] == C_VOID && layer.cell[y][x + 2] == C_PATH) ||
		(y + 2 < MAP_H && layer.cell[y + 1][x] == C_VOID && layer.cell[y + 2][x] == C_PATH);
}

/* Whether (x, y) stands in line with a walkway, straight on from it across
 * the floor between: the way across a platform MegaMan runs, and on the
 * comps' and homepages' maps their stripe of walkway floor, which runs on
 * through as much platform as lies on both its sides (a Net Dealer on one
 * stood where a playtester ran back and forth, two sidesteps round him
 * each time). */
static bool in_way_line(int x, int y) {
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < 4; ++k) {
		int dx = d[k][0], dy = d[k][1], ax = dy, ay = dx;   /* (ax, ay) across the line */
		for (int nx = x + dx, ny = y + dy; nx >= 1 && ny >= 1 && nx < MAP_W - 1 && ny < MAP_H - 1 && layer.cell[ny][nx] == C_PATH;
		     nx += dx, ny += dy)
			if (layer.cell[ny + ay][nx + ax] != C_PATH && layer.cell[ny - ay][nx - ax] != C_PATH) return true;
	}
	return false;
}

/* The walk from room `from`'s anchor to the nearest panel beside an object
 * of `type`, -1 none. */
static int walk_to(int from, int type) {
	static int16_t dist[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(dist, -1, sizeof dist);
	int h = 0, t = 0;
	qx[t] = (int16_t)layer.rooms[from].ax; qy[t++] = (int16_t)layer.rooms[from].ay;
	dist[qy[0]][qx[0]] = 0;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		for (int i = 0; i < layer.nobj; ++i)
			if (layer.obj[i].type == type && abs((int)layer.obj[i].x - x) <= 1 && abs((int)layer.obj[i].y - y) <= 1) return dist[y][x];
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || dist[ny][nx] >= 0 || layer.cell[ny][nx] != C_PATH) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	return -1;
}

/* Rush's gaps as floor (Rush lying there) or void again; and a teleport
 * island's pair joined by a line of floor, as the teleport joins them
 * (cells marked 4, back to void after). */
static void gaps_bridge(bool on) {
	for (int g = 0; g < layer.ngaps; ++g)
		for (int k = 1; k <= layer.gap[g].len; ++k)
			layer.cell[layer.gap[g].y + dir_dy[layer.gap[g].dir] * k][layer.gap[g].x + dir_dx[layer.gap[g].dir] * k] = on ? C_PATH : C_VOID;
	/* (an invisible path's panels are floor to the map, issue #46) */
	for (int g = 0; g < layer.npaths; ++g)
		for (int k = 1; k <= layer.path[g].len; ++k)
			layer.cell[layer.path[g].y + dir_dy[layer.path[g].dir] * k][layer.path[g].x + dir_dx[layer.path[g].dir] * k] = on ? C_PATH : C_VOID;
	if (!layer.teleport_island) return;
	int x = layer.teleport_x[1], y = layer.teleport_y[1], tx = layer.teleport_x[0], ty = layer.teleport_y[0];
	while (x != tx || y != ty) {
		if (x != tx) x += x < tx ? 1 : -1; else y += y < ty ? 1 : -1;
		if (on && layer.cell[y][x] == C_VOID) layer.cell[y][x] = 4;
		else if (!on && layer.cell[y][x] == 4) layer.cell[y][x] = C_VOID;
	}
	for (int j = 0; j < MAP_H; ++j)
		for (int i = 0; i < MAP_W; ++i)
			if (layer.cell[j][i] == 4) layer.cell[j][i] = on ? 4 : C_VOID;
}

/* Whether room r is a Rush gap's island, or a teleport's. */
static bool island_room(int r) {
	if (layer.teleport_island && layer.rooms[r].ax == layer.teleport_x[0] && layer.rooms[r].ay == layer.teleport_y[0]) return true;
	for (int g = 0; g < layer.ngaps + layer.npaths; ++g) {
		const NetGap *p = g < layer.ngaps ? &layer.gap[g] : &layer.path[g - layer.ngaps];
		int mx = p->x + dir_dx[p->dir] * (p->len + 2), my = p->y + dir_dy[p->dir] * (p->len + 2);
		if (layer.rooms[r].ax == mx && layer.rooms[r].ay == my) return true;
	}
	return false;
}

static int gap_layers, gap_panels, teleport_layers, teleport_islands, block_layers, block_kinds[BLOCK_NUMBER + 1], tellers;

/* A Link Navi obstacle (issue #42): in a walkway's first panel off the way;
 * closed, the exit is still reached and the pocket behind it holds its one
 * Mystery Data and nothing else. */
/* Whether a navi stands within `reach` panels of (x, y): near a set
 * piece's A (a cube's or an obstacle's mouth, a Rush stand), the engine's
 * A turned to him (it turns to a navi within 52 units first; a P-Code's
 * teller beside its cube took the cube's A). */
static bool talker_near(int x, int y, int reach) {
	for (int i = 0; i < layer.nobj; ++i) {
		int t = layer.obj[i].type;
		if (t == OBJ_WARP_IN || t == OBJ_EXIT || t == OBJ_MYSTERY || t == OBJ_UNDERNET || t == OBJ_RETURN) continue;
		if (abs((int)layer.obj[i].x - x) <= reach && abs((int)layer.obj[i].y - y) <= reach) return true;
	}
	return false;
}

/* The walk from (sx, sy) to (tx, ty) in panels, -1 none. */
static int walk_between(int sx, int sy, int tx, int ty) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H], dist[MAP_H][MAP_W];
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) dist[y][x] = -1;
	int h = 0, t = 0;
	qx[t] = (int16_t)sx; qy[t++] = (int16_t)sy;
	dist[sy][sx] = 0;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		if (x == tx && y == ty) return dist[y][x];
		for (int k = 0; k < 4; ++k) {
			int nx = x + dir_dx[k], ny = y + dir_dy[k];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || dist[ny][nx] >= 0 || layer.cell[ny][nx] != C_PATH) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	return -1;
}

static int pieces_crowded, teller_walk_min = 1 << 20, tellers_near;

static void blocks_check(uint32_t seed, int depth, const NetObj *start, uint8_t seen[MAP_H][MAP_W]) {
	if (!layer.nblocks) return;
	++block_layers;
	tellers += layer.teller > 0;
	for (int b = 0; b < layer.nblocks; ++b) {
		const NetBlock *k = &layer.block[b];
		block_kinds[k->kind]++;
		/* (the Undernet's doors, issue #47: a number door counts the
		 * layer's braziers; a skull door's WWW-ID is sold before it) */
		CHECK(k->kind != BLOCK_NUMBER || layer.braziers >= 2, "seed %u: a number door with %d braziers", seed, layer.braziers);
		CHECK(k->kind != BLOCK_SKULL || (layer.kind == LAYER_NORMAL && layer_in_act(depth) > 0), "seed %u: a skull door before its act's dealer", seed);
		pieces_crowded += talker_near(k->x, k->y, 3);
		if (k->kind == BLOCK_PCODE && layer.teller > 0) {
			const NetObj *t = &layer.obj[layer.teller - 1];
			int w = walk_between(k->x, k->y, (int)t->x, (int)t->y);
			if (w < teller_walk_min) teller_walk_min = w;
			tellers_near += w < 8;
		}
		CHECK(!layer_on_way(k->x, k->y), "seed %u: an obstacle on the way", seed);
		layer.cell[k->y][k->x] = C_VOID;
		reachable_cells((int)start->x, (int)start->y, seen);
		layer.cell[k->y][k->x] = C_PATH;
		int held = 0;
		for (int i = 0; i < layer.nobj; ++i) {
			const NetObj *o = &layer.obj[i];
			if (o->type == OBJ_EXIT || o->type == OBJ_RETURN || o->type == OBJ_BOSS) CHECK(seen[(int)o->y][(int)o->x], "seed %u: the way needs an obstacle cleared", seed);
			if (!seen[(int)o->y][(int)o->x] && layer.cell[(int)o->y][(int)o->x] == C_PATH && !(o->x == k->x + 0.5f && o->y == k->y + 0.5f))
				held += o->type == OBJ_MYSTERY ? 1 : 100;
		}
		/* (a teleport's island or a Rush island holds its own) */
		if (!layer.ngaps && !layer.npaths && !layer.teleport_island) CHECK(held == 1, "seed %u: an obstacle's pocket holds %d", seed, held);
	}
}

/* A teleport pair (issue #44): one panel where a long detour ends, or a pad
 * of its own past the void, and one two to six off the way, never on it;
 * floor all round each and nothing standing there (the gem's, the
 * trigger's); the island's one data on its bottom corner, two panels below
 * the gem, straight on from where MegaMan lands (anywhere round the gem,
 * the way to it crossed the gem's trigger, which beamed him back). */
static void teleports_check(uint32_t seed) {
	if (!layer.nteleports) return;
	++teleport_layers;
	teleport_islands += layer.teleport_island;
	for (int k = 0; k < 2; ++k) {
		int x = layer.teleport_x[k], y = layer.teleport_y[k], held = 0;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) {
				CHECK(layer.cell[y + dy][x + dx] == C_PATH && !layer_on_way(x + dx, y + dy), "seed %u: a teleport without floor round it, or by the way", seed);
				for (int i = 0; i < layer.nobj; ++i)
					if ((int)layer.obj[i].x == x + dx && (int)layer.obj[i].y == y + dy) ++held;
			}
		CHECK(held == 0, "seed %u: a teleport's pad holds %d", seed, held);
	}
	bool data = false;
	for (int i = 0; i < layer.nobj && layer.teleport_island; ++i)
		data |= layer.obj[i].type == OBJ_MYSTERY && (int)layer.obj[i].x == layer.teleport_x[0] + 2 && (int)layer.obj[i].y == layer.teleport_y[0] + 2;
	CHECK(data == layer.teleport_island, "seed %u: a teleport island without its data below the gem", seed);
	int da = layer_detour(layer.teleport_x[0], layer.teleport_y[0]), db = layer_detour(layer.teleport_x[1], layer.teleport_y[1]);
	CHECK((layer.teleport_island ? da < 0 : da >= 7) && db >= 2 && db <= 6, "seed %u: a teleport pair %d and %d off the way", seed, da, db);
}

/* The way never needs Rush: the exit (or the guardian) is reached with
 * every gap open; each gap's stand stands off the way, and its island,
 * reached only past it, holds its one Mystery Data. */
static void gaps_check(uint32_t seed, const NetObj *start, uint8_t seen[MAP_H][MAP_W]) {
	if (!layer.ngaps) return;
	++gap_layers;
	CHECK(!layer.boss_layer, "seed %u: a Rush gap before a guardian", seed);
	reachable_cells((int)start->x, (int)start->y, seen);
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		if (o->type == OBJ_EXIT || o->type == OBJ_RETURN) CHECK(seen[(int)o->y][(int)o->x], "seed %u: the exit needs Rush", seed);
	}
	for (int g = 0; g < layer.ngaps; ++g) {
		const NetGap *p = &layer.gap[g];
		gap_panels += p->len;
		pieces_crowded += talker_near(p->x, p->y, 3);
		CHECK(p->len >= 1 && p->len <= 3, "seed %u: a gap of %d panels", seed, p->len);
		CHECK(!layer_on_way(p->x, p->y), "seed %u: a gap's stand on the way", seed);
		int mx = p->x + dir_dx[p->dir] * (p->len + 2), my = p->y + dir_dy[p->dir] * (p->len + 2), held = 0;
		CHECK(!seen[my][mx], "seed %u: an island reached without Rush", seed);
		for (int i = 0; i < layer.nobj; ++i)
			if (abs((int)layer.obj[i].x - mx) <= 1 && abs((int)layer.obj[i].y - my) <= 1) held += layer.obj[i].type == OBJ_MYSTERY ? 1 : 100;
		CHECK(held == 1, "seed %u: an island holds %d", seed, held);
	}
}

/* An arrow lane (issue #43): from free ground floor a long detour off the
 * way, its panels floor with the void beside each, to free ground floor by
 * the way, where the ride ends; never on the way, and the way never needs
 * it; one way only (layer_step_ok). */
static int lane_layers, lane_panels, lane_saved, lane_planned;

static void lanes_check(uint32_t seed, const NetObj *start, uint8_t seen[MAP_H][MAP_W]) {
	for (int i = 0; i < layer.nlanes; ++i) {
		const NetLane *l = &layer.lane[i];
		int sx = dir_dx[(l->dir + 1) % 4], sy = dir_dy[(l->dir + 1) % 4];
		int ex = l->x + dir_dx[l->dir] * (l->len + 1), ey = l->y + dir_dy[l->dir] * (l->len + 1);
		++lane_layers;
		lane_panels += l->len;
		CHECK(l->len >= 1 && l->len <= 5, "seed %u: a lane of %d panels", seed, l->len);
		CHECK(layer.cell[l->y][l->x] == C_PATH && layer.cell[ey][ex] == C_PATH && !layer.level[l->y][l->x] && !layer.level[ey][ex],
			"seed %u: a lane's ends off the ground floor", seed);
		CHECK(layer_detour(l->x, l->y) >= 5 && layer_detour(ex, ey) <= 3, "seed %u: a lane from %d off the way to %d", seed,
			layer_detour(l->x, l->y), layer_detour(ex, ey));
		for (int o = 0; o < layer.nobj; ++o) {
			int ox = (int)layer.obj[o].x, oy = (int)layer.obj[o].y;
			CHECK(!(ox == ex && oy == ey) && !(ox == l->x && oy == l->y), "seed %u: something stands at a lane's end", seed);
		}
		for (int k = 1; k <= l->len; ++k) {
			int cx = l->x + dir_dx[l->dir] * k, cy = l->y + dir_dy[l->dir] * k;
			CHECK(layer.cell[cy][cx] == C_PATH && layer.cell[cy + sy][cx + sx] == C_VOID && layer.cell[cy - sy][cx - sx] == C_VOID,
				"seed %u: a lane's panel not one wide", seed);
			CHECK(!layer_on_way(cx, cy) && layer_lane_dir(cx, cy) == l->dir, "seed %u: a lane on the way", seed);
			CHECK(layer_step_ok(cx - dir_dx[l->dir], cy - dir_dy[l->dir], cx, cy) && !layer_step_ok(cx, cy, cx - dir_dx[l->dir], cy - dir_dy[l->dir]),
				"seed %u: a lane walked both ways", seed);
			layer.cell[cy][cx] = C_VOID;
		}
		reachable_cells((int)start->x, (int)start->y, seen);
		for (int o = 0; o < layer.nobj; ++o) {
			const NetObj *ob = &layer.obj[o];
			if (ob->type == OBJ_EXIT || ob->type == OBJ_RETURN || ob->type == OBJ_BOSS) CHECK(seen[(int)ob->y][(int)ob->x], "seed %u: the way needs a lane", seed);
		}
		lane_saved += layer_detour(l->x, l->y) - layer_detour(ex, ey);
		for (int k = 1; k <= l->len; ++k) layer.cell[l->y + dir_dy[l->dir] * k][l->x + dir_dx[l->dir] * k] = C_PATH;
	}
}

/* An invisible path (issue #46): from a walkway's tip, its panels void in
 * cell[] (drawn so), to a pad reached only across it, holding its one
 * thing; a navi near the tip hints at it; the way never needs it. */
static int path_layers;

static void paths_check(uint32_t seed, const NetObj *start, uint8_t seen[MAP_H][MAP_W]) {
	if (!layer.npaths) return;
	++path_layers;
	reachable_cells((int)start->x, (int)start->y, seen);
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		if (o->type == OBJ_EXIT || o->type == OBJ_RETURN || o->type == OBJ_BOSS) CHECK(seen[(int)o->y][(int)o->x], "seed %u: the way needs an invisible path", seed);
	}
	for (int g = 0; g < layer.npaths; ++g) {
		const NetGap *p = &layer.path[g];
		CHECK(layer.cell[p->y][p->x] == C_PATH && !layer.level[p->y][p->x], "seed %u: an invisible path from no floor", seed);
		for (int k = 1; k <= p->len; ++k)
			CHECK(layer.cell[p->y + dir_dy[p->dir] * k][p->x + dir_dx[p->dir] * k] == C_VOID, "seed %u: an invisible path drawn", seed);
		int mx = p->x + dir_dx[p->dir] * (p->len + 2), my = p->y + dir_dy[p->dir] * (p->len + 2), held = 0;
		CHECK(!seen[my][mx], "seed %u: an invisible path's pad reached without it", seed);
		for (int i = 0; i < layer.nobj; ++i)
			if (abs((int)layer.obj[i].x - mx) <= 1 && abs((int)layer.obj[i].y - my) <= 1) held += layer.obj[i].type == OBJ_MYSTERY ? 1 : 100;
		CHECK(held == 1, "seed %u: an invisible path's pad holds %d", seed, held);
		const NetObj *h = layer.hinter ? &layer.obj[layer.hinter - 1] : NULL;
		CHECK(h && h->type == OBJ_NPC && abs((int)h->x - p->x) <= 8 && abs((int)h->y - p->y) <= 8, "seed %u: no navi hints at an invisible path", seed);
	}
}

/* Branches off the way five panels or more, and those that end in
 * nothing: no object, no set piece's stand (session 59: a two-wide band
 * led to an empty end); wide where two by two of its panels are floor. */
static void empty_ends(int *branches, int *empty, int *empty_wide) {
	static int16_t far[MAP_W * MAP_H];
	static uint8_t holds[MAP_W * MAP_H], wide[MAP_W * MAP_H];
	memset(far, 0, sizeof far); memset(holds, 0, sizeof holds); memset(wide, 0, sizeof wide);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int d = layer_detour(x, y), b = layer_branch(x, y);
			if (d <= 0 || b < 0) continue;
			if (d > far[b]) far[b] = (int16_t)d;
			if (layer_branch(x + 1, y) == b && layer_branch(x, y + 1) == b && layer_branch(x + 1, y + 1) == b && layer_detour(x + 1, y + 1) > 0) wide[b] = 1;
		}
	#define HOLDS(x, y) do { int bb = layer_detour((x), (y)) > 0 ? layer_branch((x), (y)) : -1; if (bb >= 0) holds[bb] = 1; } while (0)
	for (int i = 0; i < layer.nobj; ++i) HOLDS((int)layer.obj[i].x, (int)layer.obj[i].y);
	for (int i = 0; i < layer.ngaps; ++i) HOLDS(layer.gap[i].x, layer.gap[i].y);
	for (int i = 0; i < layer.nblocks; ++i) HOLDS(layer.block[i].x, layer.block[i].y);
	for (int i = 0; i < layer.npaths; ++i) HOLDS(layer.path[i].x, layer.path[i].y);
	for (int i = 0; i < layer.nlanes; ++i) HOLDS(layer.lane[i].x, layer.lane[i].y);
	for (int i = 0; i < 2 * (layer.nteleports > 0); ++i) HOLDS(layer.teleport_x[i], layer.teleport_y[i]);
	#undef HOLDS
	for (int b = 0; b < MAP_W * MAP_H; ++b) {
		if (far[b] < 5) continue;
		++*branches;
		if (!holds[b]) { ++*empty; *empty_wide += wide[b]; }
	}
}

static void test_generation(void) {
	static uint8_t seen[MAP_H][MAP_W];
	int boss_layers = 0, arenas = 0, mouths = 0, standing = 0, hidden = 0, approached = 0, dealers = 0, counters = 0, sprites = 0, holes = 0,
		landmarks = 0, statues = 0, undernet_layers = 0, layers = 0, emblems = 0, corners = 0, navi_corners = 0, in_line = 0, beside_line = 0, near_pairs = 0,
		talk_pairs = 0, talk_touch = 0, duel_layers = 0, gate_by_duel = 0, heal_far = 0, blue = 0, blue_far = 0, green = 0, purples = 0,
		branches = 0, empty = 0, empty_wide = 0, navis_by_way = 0;
	long blue_walk = 0, green_walk = 0;
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = (uint8_t)(1 + b % 5);   /* (Cross navis: the obstacles a run's Crosses clear) */
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	for (uint32_t seed = 1; seed <= 300; ++seed) {
		int depth = 1 + (int)(seed % 25);
		int kind = seed % 7 == 0 ? LAYER_UNDERNET : seed % 11 == 0 ? LAYER_SECRET : LAYER_NORMAL;
		/* (a third of them in any area: the comps and homepages too) */
		int biome = seed % 3 == 0 ? (int)(seed / 3 % BIOME_COUNT) : biome_for_depth(depth);
		/* (the Undernet with its own looks, its statue and braziers: a
		 * number door counts them, issue #47) */
		layer_generate(seed * 7919u, depth, biome, kind, biome == BIOME_UNDERNET ? &undernet_kit : &kit);
		++layers;
		empty_ends(&branches, &empty, &empty_wide);
		/* (a heal a short walk from every guardian's arena: a playtester
		 * found one a long way back from SpoutMan's) */
		if (layer.arena >= 0) {
			int w = walk_to(layer.ante, OBJ_HEAL);
			heal_far = w > heal_far ? w : heal_far;
			CHECK(w >= 0 && w <= 12, "seed %u: the heal before the arena a walk of %d", seed, w);
		}
		CHECK(layer.nrooms >= 3, "seed %u: only %d rooms", seed, layer.nrooms);
		/* (the game runs 16 NPCs on a map and leaves the rest out: the last
		 * bystanders, and an official gate or ProtoMan, never showed) */
		CHECK(layer_npcs() <= LAYER_NPC_MAX, "seed %u: %d NPCs, the game runs %d", seed, layer_npcs(), LAYER_NPC_MAX);
		/* (the blue data where the detours end, the green ones loose: what
		 * a walk there is worth, as BN6 colours its data) */
		for (int i = 0; i < layer.nobj; ++i) {
			const NetObj *o = &layer.obj[i];
			if (o->type != OBJ_MYSTERY) continue;
			int d = layer_detour((int)o->x, (int)o->y);
			/* (an island's, past a gap or an invisible path, keeps its own rule) */
			if (o->param == MD_PURPLE || d < 0) continue;
			if (o->param >= 1) { ++blue; blue_walk += d; blue_far += d >= 3; }
			else { ++green; green_walk += d; }
		}
		/* (a purple data, none or one a layer as BN6 sets them, off the way:
		 * at the landmark's foot or where a detour ends) */
		{
			int here = 0;
			for (int i = 0; i < layer.nobj; ++i) {
				const NetObj *o = &layer.obj[i];
				if (o->type != OBJ_MYSTERY || o->param != MD_PURPLE) continue;
				++here;
				CHECK(!layer_on_way((int)o->x, (int)o->y), "seed %u: a purple data on the way", seed);
			}
			CHECK(here <= 1, "seed %u: %d purple data", seed, here);
			CHECK(!here || layer_purple(depth, biome, kind), "seed %u: a purple data where none was planned", seed);
			purples += here;
		}
		NetObj *start = &layer.obj[0];
		CHECK(start->type == OBJ_WARP_IN, "seed %u: first object is the arrival warp", seed);
		/* (Rush's gaps: the way never needs one; past one, its island and
		 * the one thing on it; bridged, as Rush lies there once called,
		 * every panel is reached) */
		gaps_check(seed, start, seen);
		teleports_check(seed);
		blocks_check(seed, depth, start, seen);
		lanes_check(seed, start, seen);
		paths_check(seed, start, seen);
		lane_planned += (layer_pieces(depth, biome, kind) & PIECE_ARROW) != 0;
		gaps_bridge(true);
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
				/* (a navi behind a counter stands in its walled-off aisle) */
				if (!o->solid || o->type == OBJ_BOSS || layer.cell[(int)o->y][(int)o->x] != C_PATH) continue;
				layer.cell[(int)o->y][(int)o->x] = C_VOID;
				++blocked;
			}
			int open = reachable_cells((int)start->x, (int)start->y, seen);
			CHECK(open == cells - blocked, "seed %u: objects cut the way (%d of %d cells)", seed, open, cells - blocked);
			memcpy(layer.cell, save, sizeof save);
		}
		gaps_bridge(false);
		/* every act's second layer holds the rival's duel (docs/RIVAL.md) */
		if (kind == LAYER_NORMAL && layer_in_act(depth) == 1 && biome != BIOME_NEST) {
			bool duel = false;
			int official = 0, gates = 0, px = -99, py = -99, gx = 99, gy = 99;
			for (int i = 0; i < layer.nobj; ++i) {
				duel |= layer.obj[i].type == OBJ_DUEL;
				if (layer.obj[i].type == OBJ_DUEL) { px = (int)layer.obj[i].x; py = (int)layer.obj[i].y; }
				if (layer.obj[i].type == OBJ_OFFICIAL) { official = layer.obj[i].param; gx = (int)layer.obj[i].x; gy = (int)layer.obj[i].y; }
				gates += layer.obj[i].type == OBJ_NAVI_GATE || layer.obj[i].type == OBJ_VAULT;
			}
			/* (the gate by ProtoMan: within 8 panels of him) */
			++duel_layers;
			if (abs(px - gx) + abs(py - gy) <= 8) ++gate_by_duel;
			CHECK(duel, "seed %u: no duel on an act's second layer (depth %d, area %d)", seed, depth, biome);
			/* ... and beside it the official gate it opens, the act's level,
			 * the only gate there */
			CHECK(official == (pacing_act(depth) >= 2 ? 2 : 1), "seed %u: official gate %d on a duel layer (depth %d)", seed, official, depth);
			CHECK(!gates, "seed %u: another gate beside the duel (depth %d)", seed, depth);
		}
		bool has_exit = false;
		for (int i = 0; i < layer.nobj; ++i) {
			NetObj *o = &layer.obj[i];
			CHECK(layer.cell[(int)o->y][(int)o->x] == C_PATH || (o->prop >= 0 && layer.cell[(int)o->y][(int)o->x] == C_SOLID),
				"seed %u: object %d off the path", seed, i);
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
		/* bystanders two panels at least from what else stands there */
		for (int i = 0; i < layer.nobj; ++i) {
			if (layer.obj[i].type != OBJ_NPC) continue;
			for (int j = 0; j < layer.nobj; ++j)
				if (j != i) CHECK(abs((int)layer.obj[i].x - (int)layer.obj[j].x) > 2 || abs((int)layer.obj[i].y - (int)layer.obj[j].y) > 2,
					"seed %u: a bystander within two panels of object %d (type %d)", seed, j, layer.obj[j].type);
		}
		/* services and navis off the walkways' mouths */
		for (int i = 1; i < layer.nobj; ++i) {
			const NetObj *o = &layer.obj[i];
			bool stands = o->type == OBJ_SHOP || o->type == OBJ_HEAL || o->type == OBJ_TRADER || o->type == OBJ_BUGTRADER ||
				o->type == OBJ_NPC || o->type == OBJ_CHALLENGE || o->type == OBJ_PROGRAMS || o->type == OBJ_GIFT;
			if (stands && o->prop < 0 && beside_narrow((int)o->x, (int)o->y)) ++mouths;
			/* (nor corner to corner with a walkway's last panel: a bystander
			 * on a platform's corner there stood in the way in) */
			if (stands && o->prop < 0) {
				bool corner = false;
				for (int dy = -1; dy <= 1; dy += 2)
					for (int dx = -1; dx <= 1; dx += 2) {
						int nx = (int)o->x + dx, ny = (int)o->y + dy;
						bool walkway = layer.cell[ny][nx] == C_PATH && ((layer.cell[ny][nx - 1] == C_VOID && layer.cell[ny][nx + 1] == C_VOID) ||
							(layer.cell[ny - 1][nx] == C_VOID && layer.cell[ny + 1][nx] == C_VOID));
						corner |= walkway;
					}
				corners += corner;
				navi_corners += corner && o->type == OBJ_NPC;
			}
			if (stands) ++standing;
			if (stands && o->prop < 0 && in_way_line((int)o->x, (int)o->y)) ++in_line;
			if (stands && o->prop < 0) {
				bool beside = layer_by_way((int)o->x, (int)o->y), near = false;
				/* (a panel's gap on the way itself, with a solid object two panels off) */
				for (int j = 1; j < layer.nobj; ++j) {
					int ox = (int)layer.obj[j].x, oy = (int)layer.obj[j].y, x = (int)o->x, y = (int)o->y;
					if (j == i || !layer.obj[j].solid || abs(ox - x) > 2 || abs(oy - y) > 2 || (abs(ox - x) < 2 && abs(oy - y) < 2)) continue;
					for (int gy = y - 1; gy <= y + 1; ++gy)
						for (int gx = x - 1; gx <= x + 1; ++gx)
							near |= abs(gx - ox) <= 1 && abs(gy - oy) <= 1 && layer_on_way(gx, gy);
				}
				beside_line += beside;
				near_pairs += near;
				navis_by_way += beside && o->type == OBJ_NPC && i != layer.teller - 1 && i != layer.hinter - 1;
			}
			/* (two to talk to within two panels: an A for the heal Prog
			 * opened the Net Dealer beside him, twice) */
			if (stands)
				for (int j = i + 1; j < layer.nobj; ++j) {
					const NetObj *q = &layer.obj[j];
					bool talks = q->type == OBJ_SHOP || q->type == OBJ_HEAL || q->type == OBJ_TRADER || q->type == OBJ_BUGTRADER ||
						q->type == OBJ_NPC || q->type == OBJ_CHALLENGE || q->type == OBJ_PROGRAMS || q->type == OBJ_GIFT;
					int d = abs((int)q->x - (int)o->x) > abs((int)q->y - (int)o->y) ? abs((int)q->x - (int)o->x) : abs((int)q->y - (int)o->y);
					/* (one behind a counter is talked to from its front) */
					int counter = o->prop >= 0 || q->prop >= 0;
					if (talks && d <= 2 + counter) ++talk_pairs;
					if (talks && d <= 1 + counter) ++talk_touch;
				}
			if (stands || o->type == OBJ_MYSTERY) {
				++approached;
				hidden += behind_gap((int)o->x, (int)o->y);
			}
		}
		/* a counter: its aisle walled off on the platform's rim, its own
		 * panels, the floor before it walkable and reached; its navi in the
		 * aisle behind it */
		for (int i = 0; i < layer.nobj; ++i) {
			const NetObj *o = &layer.obj[i];
			if (o->type == OBJ_SHOP) ++dealers;
			if (o->prop < 0) continue;
			++counters;
			CHECK(o->prop < layer.nprops && o->type == OBJ_SHOP, "seed %u: object %d behind prop %d", seed, i, o->prop);
			const NetProp *p = &layer.props[o->prop];
			for (int t = 0; t < p->len; ++t) {
				int x, y;
				counter_cell(p, t, -1, &x, &y);
				CHECK(x < 0 || y < 0 || layer.cell[y][x] == C_VOID, "seed %u: a counter's aisle not on the rim", seed);
				counter_cell(p, t, 0, &x, &y);
				CHECK(layer.cell[y][x] == C_SOLID, "seed %u: a counter's aisle not walled off", seed);
				counter_cell(p, t, 1, &x, &y);
				CHECK(layer.cell[y][x] == C_PROPPED, "seed %u: a counter's panel not propped", seed);
				counter_cell(p, t, 2, &x, &y);
				CHECK(layer.cell[y][x] == C_PATH && seen[y][x], "seed %u: no way to the front of a counter", seed);
			}
			int ax, ay;
			counter_cell(p, (p->len - 1) / 2, 0, &ax, &ay);
			CHECK((int)o->x == ax && (int)o->y == ay, "seed %u: a counter's navi not behind its middle", seed);
		}
		/* sprite props: past a rim on the void, or in a walled hole in the
		 * floor (its cells walled off, floor all round the hole), none on
		 * the way between the warps */
		for (int i = 0; i < layer.nprops; ++i) {
			const NetProp *p = &layer.props[i];
			if (p->kind != PROP_SPRITE) continue;
			++sprites;
			uint8_t c = layer.cell[p->y][p->x];
			CHECK(c == C_VOID || c == C_SOLID, "seed %u: a %d prop on open floor", seed, p->look);
			bool by_floor = false;
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = p->x + dx, ny = p->y + dy;
					if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H) continue;
					if (layer.cell[ny][nx] != C_VOID) by_floor = true;
					if (c == C_SOLID) CHECK(layer.cell[ny][nx] != C_VOID, "seed %u: a hole prop at the floor's edge", seed);
				}
			CHECK(by_floor, "seed %u: a %d prop far from any floor", seed, p->look);
			if (c == C_SOLID) ++holes;
		}
		/* emblems: in ground floor with floor all round, none under an
		 * object, none beside another */
		for (int i = 0; i < layer.nprops; ++i) {
			const NetProp *p = &layer.props[i];
			if (p->kind != PROP_EMBLEM) continue;
			++emblems;
			for (int dy = -1; dy <= 1; ++dy)
				for (int dx = -1; dx <= 1; ++dx)
					CHECK(layer.cell[p->y + dy][p->x + dx] == C_PATH && !layer.level[p->y + dy][p->x + dx],
						"seed %u: an emblem at (%d, %d) without ground floor all round", seed, p->x, p->y);
			for (int o = 0; o < layer.nobj; ++o)
				CHECK((int)layer.obj[o].x != p->x || (int)layer.obj[o].y != p->y, "seed %u: an object on an emblem", seed);
			for (int j = 0; j < i; ++j)
				if (layer.props[j].kind == PROP_EMBLEM)
					CHECK(abs(layer.props[j].x - p->x) > 1 || abs(layer.props[j].y - p->y) > 1, "seed %u: emblems side by side", seed);
		}
		/* (the Undernet's statue apart: it wants a rim five panels long,
		 * which its rooms often lack, as its own maps hold one) */
		for (int i = 0; i < layer.nprops; ++i)
			if (layer.props[i].kind == PROP_SPRITE && (layer.props[i].look == LOOK_GIANT_TREE || layer.props[i].look == LOOK_STATUE ||
			    layer.props[i].look == LOOK_MONUMENT)) { ++*(biome == BIOME_UNDERNET ? &statues : &landmarks); break; }
		undernet_layers += biome == BIOME_UNDERNET;
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
	/* (a detour five panels long or more ends in something: a two-wide band
	 * that read as the way led a playtester to nothing; 106 of 1589 such
	 * bands were empty, and 321 detours in all, before fill_empty_detours) */
	CHECK(empty_wide * 30 <= branches, "%d of %d long detours run wide to nothing", empty_wide, branches);
	CHECK(empty * 6 <= branches, "%d of %d long detours end in nothing", empty, branches);
	CHECK(mouths == 0, "%d of %d services and navis stand at a walkway's mouth", mouths, standing);
	/* (a service where its room has no other place) */
	CHECK(in_line * 20 <= standing, "%d of %d services and navis stand in line with a walkway", in_line, standing);
	/* (by the way where a room on it has no other place, as the room before
	 * an arena; a gap on the way where the room is small) */
	CHECK(beside_line * 4 <= standing, "%d of %d services and navis stand by the way on", beside_line, standing);
	CHECK(near_pairs * 16 <= standing, "%d of %d services and navis leave a panel's gap on the way", near_pairs, standing);
	/* (a bystander never: one beside a neck before an arena's walkway walled
	 * it off, session 65) */
	CHECK(navis_by_way == 0, "%d bystanders stand by the way on", navis_by_way);
	CHECK(talk_touch == 0, "%d pairs of navis to talk to stand side by side, or beside a counter's front", talk_touch);
	/* (the duel's official gate by ProtoMan: a playtester found it alone,
	 * across the layer from him) */
	CHECK(gate_by_duel * 20 >= duel_layers * 19, "the official gate stands by ProtoMan on %d of %d duel layers", gate_by_duel, duel_layers);
	printf("  the official gate within 8 panels of ProtoMan: %d of %d duel layers\n", gate_by_duel, duel_layers);
	printf("  two to talk to within two panels (a counter's navi three): %d pairs, %d side by side\n", talk_pairs, talk_touch);
	printf("  in line with a walkway: %d of %d services and navis; by the way on %d; a panel's gap on the way %d\n",
		in_line, standing, beside_line, near_pairs);
	/* (a bystander never; a service where its room has no other place, as
	 * the heal on a small pad before an arena, beside the bridge) */
	CHECK(navi_corners == 0, "%d bystanders stand corner to corner with a walkway's last panel", navi_corners);
	CHECK(corners * 40 <= standing, "%d of %d services and navis stand corner to corner with a walkway's last panel", corners, standing);
	/* (the Net Dealer stands behind a counter wherever the area has one and
	 * its room a place for it) */
	CHECK(counters * 2 >= dealers, "only %d of %d Net Dealers behind a counter", counters, dealers);
	/* (a landmark on most layers where the area has one) */
	CHECK(landmarks * 2 >= layers - undernet_layers, "a landmark on only %d of %d layers", landmarks, layers - undernet_layers);
	CHECK(emblems >= layers, "only %d emblems on %d layers", emblems, layers);
	printf("  props: %d sprites (%d in holes) and %d emblems on %d layers, landmarks on %d, the Undernet's statue on %d of its %d\n", sprites, holes,
		emblems, layers, landmarks, statues, undernet_layers);
	printf("  the heal before an arena: a walk of %d at most\n", heal_far);
	printf("  Mystery Data: %d blue, %.1f panels off the way on average, %d of them 3 or more; %d green, %.1f\n", blue,
		(double)blue_walk / (blue ? blue : 1), blue_far, green, (double)green_walk / (green ? green : 1));
	CHECK(blue_far * 10 >= blue * 9, "only %d of %d blue data where a detour ends", blue_far, blue);
	printf("  purple data on %d of %d layers; Rush gaps on %d, %d panels in all; teleport pairs on %d, %d to an island\n", purples, layers, gap_layers,
		gap_panels, teleport_layers, teleport_islands);
	printf("  Link Navi obstacles and cubes on %d layers: water %d, tree %d, flames %d, cyclone %d, cloud %d; P-Code cubes %d (%d told), tolls %d\n",
		block_layers, block_kinds[0], block_kinds[1], block_kinds[2], block_kinds[3], block_kinds[4], block_kinds[BLOCK_PCODE], tellers, block_kinds[BLOCK_TOLL]);
	printf("  the Undernet's doors: %d skull doors, %d number doors\n", block_kinds[BLOCK_SKULL], block_kinds[BLOCK_NUMBER]);
	CHECK(tellers == block_kinds[BLOCK_PCODE], "%d P-Code cubes, %d navis to tell their codes", block_kinds[BLOCK_PCODE], tellers);
	printf("  P-Code tellers a walk of %d at least from their cubes\n", teller_walk_min);
	printf("  invisible paths on %d layers\n", path_layers);
	printf("  arrow lanes on %d layers of %d planned, %d panels in all, from %.1f panels off the way on average\n", lane_layers, lane_planned,
		lane_panels, (double)lane_saved / (lane_layers ? lane_layers : 1));
	CHECK(!pieces_crowded, "%d set pieces with a navi within 3 panels of their A", pieces_crowded);
	CHECK(!tellers_near, "%d P-Code tellers under 8 panels' walk from their cubes", tellers_near);
	CHECK(purples * 10 >= layers && purples * 2 <= layers, "purple data on %d of %d layers", purples, layers);
	/* (a Mystery Data a playtester saw beside his walkway was a walk round) */
	CHECK(hidden * 100 <= approached, "%d of %d objects stand behind a hidden gap", hidden, approached);
	/* Determinism: the same seed builds the same layer. */
	layer_generate(1234, 5, BIOME_SKY, LAYER_NORMAL, &kit);
	static Layer a;
	a = layer;
	layer_generate(1234, 5, BIOME_SKY, LAYER_NORMAL, &kit);
	CHECK(!memcmp(a.cell, layer.cell, sizeof a.cell) && a.nobj == layer.nobj, "generation is deterministic");
}

/* The golden hash (issue #19): layer_generate's output over fixed seeds,
 * in every area and kind, against LAYER_MAKE_HASH beside LAYER_MAKE
 * (layer_make.h). The determinism check above builds a seed twice in one
 * build; this one fails when a build makes a seed's layer otherwise. */
static uint32_t fnv(uint32_t h, const void *p, size_t n) {
	const uint8_t *b = p;
	for (size_t i = 0; i < n; ++i) h = (h ^ b[i]) * 16777619u;
	return h;
}

static uint32_t mix(uint32_t h, int v) { int32_t w = v; return fnv(h, &w, sizeof w); }

/* (the set pieces, epic #49: a gap, an obstacle, a teleport or a lane the
 * sample's seeds hold) */
static uint32_t set_pieces_hash(uint32_t h) {
	h = mix(h, layer.ngaps);
	for (int i = 0; i < layer.ngaps; ++i) h = mix(mix(mix(mix(h, layer.gap[i].x), layer.gap[i].y), layer.gap[i].dir), layer.gap[i].len);
	h = mix(mix(h, layer.nblocks), layer.teller);
	for (int i = 0; i < layer.nblocks; ++i) h = mix(mix(mix(mix(h, layer.block[i].x), layer.block[i].y), layer.block[i].dir), layer.block[i].kind);
	h = mix(mix(h, layer.nteleports), layer.teleport_island);
	for (int i = 0; i < 2 && layer.nteleports; ++i) h = mix(mix(h, layer.teleport_x[i]), layer.teleport_y[i]);
	h = mix(h, layer.nlanes);
	for (int i = 0; i < layer.nlanes; ++i) h = mix(mix(mix(mix(h, layer.lane[i].x), layer.lane[i].y), layer.lane[i].dir), layer.lane[i].len);
	return h;
}

static void test_layer_make(void) {
	Run before = run;   /* (the tests after it see the run as it was) */
	memset(&run, 0, sizeof run);
	run.seed = 0x5EED;
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = (uint8_t)(1 + b % 16);
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	uint32_t h = 2166136261u;
	for (uint32_t s = 1; s <= 20; ++s) {
		int depth = 1 + (int)(s * 3 % 26), kind = s % 5 == 0 ? LAYER_UNDERNET : s % 7 == 0 ? LAYER_SECRET : LAYER_NORMAL;
		layer_generate(s * 104729u, depth, s % 2 ? (int)(s % BIOME_COUNT) : biome_for_depth(depth), kind, &kit);
		h = fnv(h, layer.cell, sizeof layer.cell);
		h = fnv(h, layer.level, sizeof layer.level);
		h = mix(h, layer.nstairs);
		for (int i = 0; i < layer.nstairs; ++i) h = mix(mix(mix(h, layer.stair[i].x), layer.stair[i].y), layer.stair[i].dir);
		h = mix(h, layer.rise);
		h = mix(h, layer.nrooms);
		for (int i = 0; i < layer.nrooms; ++i) {
			const Room *r = &layer.rooms[i];
			h = mix(mix(mix(mix(mix(mix(mix(h, r->x), r->y), r->w), r->h), r->ax), r->ay), r->kind);
		}
		h = mix(h, layer.nobj);
		for (int i = 0; i < layer.nobj; ++i) {
			const NetObj *o = &layer.obj[i];
			h = mix(mix(mix(mix(h, o->type), (int)lroundf(o->x * 16)), (int)lroundf(o->y * 16)), o->param);
			h = mix(mix(mix(h, o->solid), o->npc_line), o->prop);
		}
		h = mix(mix(mix(mix(mix(h, layer.biome), layer.kind), layer.boss_layer), layer.boss_navi), layer.exit_room);
		h = mix(mix(mix(mix(h, layer.arena), layer.ante), layer.arena_dir), layer.layout);
		h = mix(h, layer.nprops);
		for (int i = 0; i < layer.nprops; ++i) {
			const NetProp *p = &layer.props[i];
			h = mix(mix(mix(mix(mix(mix(h, p->kind), p->faces), p->x), p->y), p->len), p->look);
		}
		h = set_pieces_hash(h);
	}
	CHECK(h == LAYER_MAKE_HASH, "layer generation changed (hash 0x%08x, LAYER_MAKE_HASH 0x%08x): bump LAYER_MAKE and set LAYER_MAKE_HASH "
		"in src/net/layer_make.h together", h, LAYER_MAKE_HASH);
	run = before;
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
		layer_generate(seed * 7919u, depth, biome_for_depth(depth), LAYER_NORMAL, &kit);
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
	printf("  stairs: %d of 400 layers\n", layers);
	/* an area without stairs keeps its layers flat */
	layer_generate(7919u, 2, BIOME_SKY, LAYER_NORMAL, &flat);
	CHECK(!layer.nstairs && !layer.rise, "flat area got a stair");
}

static void test_depth_plan(void) {
	memset(&run, 0, sizeof run);
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	CHECK(!is_boss_depth(1) && !is_boss_depth(2) && is_boss_depth(3), "every third layer has a boss");
	CHECK(biome_for_depth(19) == BIOME_NEST && is_boss_depth(19), "layer 19 is the Cybeast Nest");
	CHECK(biome_for_depth(20) == run.biome_order[0], "the cycle restarts after the Nest");
	/* the short net: three acts, then the Nest on layer 10 (docs/META.md) */
	run.mode = RUN_SHORT;
	CHECK(is_boss_depth(3) && is_boss_depth(6) && is_boss_depth(9) && !is_boss_depth(8), "the short net's acts end in guardians");
	CHECK(biome_for_depth(9) == run.biome_order[2], "the short net's third act is the run's third area");
	CHECK(biome_for_depth(SHORT_LAYERS) == BIOME_NEST && is_boss_depth(SHORT_LAYERS), "the short net's Nest is layer 10");
	CHECK(layer_in_act(SHORT_LAYERS) == 0, "the short net's Nest counts as a first layer");
	/* (the RegUps, issue #51: 8 MB after the first act, 20 after the
	 * third, none on a guardian's layer or a side layer, none past
	 * REG_MAX) */
	int reg = REG_START;
	for (int d = 1; d <= SHORT_LAYERS; ++d) reg += layer_regup(d, LAYER_NORMAL);
	CHECK(reg == 20, "the short net's RegUps bring 20 MB, not %d", reg);
	CHECK(!layer_regup(3, LAYER_NORMAL) && !layer_regup(SHORT_LAYERS, LAYER_NORMAL) && !layer_regup(2, LAYER_UNDERNET), "no RegUp on a guardian's or a side layer");
	run.mode = RUN_ENDLESS;
	reg = REG_START;
	int at3 = 0, at18 = 0;
	for (int d = 1; d <= 6 * CYCLE_LAYERS; ++d) {
		reg += layer_regup(d, LAYER_NORMAL);
		if (d == 3) at3 = reg;
		if (d == 18) at18 = reg;
	}
	CHECK(at3 == 8 && at18 == 38 && reg <= REG_MAX && reg > REG_MAX - 3, "RegUps: %d MB after act 1, %d after act 6, %d at most", at3, at18, reg);
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
	static const int opening[] = { BIOME_CENTRAL, BIOME_COMP, BIOME_SEASIDE };
	static const int late[] = { BIOME_SKY, BIOME_WEATHER_COMP, BIOME_ACDC_HP, BIOME_COPYBOT_COMP };
	for (uint32_t seed = 1; seed <= 300; ++seed) {
		rng_seed(seed);
		uint8_t o[4];
		pacing_area_order(o);
		bool first = false, last = false;
		for (int i = 0; i < 3; ++i) first |= o[0] == opening[i];
		for (int i = 0; i < 4; ++i) last |= o[3] == late[i];
		CHECK(first, "seed %u: act 1 in an opening area (%d)", seed, o[0]);
		/* (each act's area among those its way on may offer instead) */
		for (int a = 0; a < 4; ++a) {
			uint8_t pool[PACING_AREA_POOL];
			int n = pacing_area_pool(a, pool);
			bool in = false;
			for (int i = 0; i < n; ++i) in |= pool[i] == o[a];
			CHECK(in, "seed %u: act %d's area in its pool", seed, a + 1);
		}
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
		CHECK(g == 12 || g == 13 || g == 6, "the lab comps' first-act guardian is BlastMan, DiveMan or SpoutMan, got %d", g);
		g = pacing_guardian_pick(sky_hp, others, (int)sizeof others, 0, 0, false, fake_navi_hp);
		CHECK(fake_navi_hp(g, 0) <= 600, "a first-act guardian of 600 HP at most, got %d", g);
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
		/* (the net's word on him, as a bystander passes it on, in every
		 * area) */
		if (guardian_rumor(navi))
			for (int biome = 0; biome < BIOME_COUNT; ++biome) {
				char line[256];
				snprintf(line, sizeof line, "They say a copy of %s guards the end of %s.|Word is, %s", guardian(navi)->name,
					guardian_area_in_text(biome, LAYER_NORMAL), guardian_rumor(navi));
				snprintf(what, sizeof what, "guardian_rumor(%d) in area %d", navi, biome);
				check_talk(what, line);
			}
		CHECK(guardian_rumor(navi) || navi == 17 || navi > 18, "guardian %d has no rumor", navi);
		/* (MegaMan warns of every guardian a run can meet) */
		CHECK(guardian_tip(navi) || navi == 17 || navi > 18, "guardian %d has no tip", navi);
		for (int depth = 3; depth <= 19; depth += 3) {
			snprintf(what, sizeof what, "powers_reward_text(%d, graveyard, %d)", navi, depth);
			check_talk(what, powers_reward_text(navi, BIOME_GRAVEYARD, depth));
		}
		/* (and beside every Cross a run can bring) */
		for (int brought = 1; brought <= 18; ++brought) {
			if (!powers_cross_name(brought)) continue;
			int cross = run.cross;
			run.cross = (uint8_t)brought;
			snprintf(what, sizeof what, "powers_reward_text(%d) beside %d", navi, brought);
			check_talk(what, powers_reward_text(navi, 0, 3));
			run.cross = (uint8_t)cross;
		}
	}
	/* (ProtoMan's tip behind MegaMan's first words, whole: a longer one
	 * lost its last clause to ta_pages' buffer in a capture) */
	char terms[GUARDIAN_TERMS_MAX];
	CHECK(guardian_netbattle_terms(terms, sizeof terms) < (int)sizeof terms, "the netbattle's terms are cut: \"%s\"", terms);
	check_talk("the netbattle's terms", terms);
	for (int biome = 0; biome < BIOME_COUNT; ++biome) {
		for (int side = LAYER_NORMAL; side <= LAYER_SECRET; ++side)
			CHECK(strlen(guardian_area_in_text(biome, side)) < 28, "area %d's name is long for the cards", biome);
		/* (the PET's PLACE: twelve letters with a space and two digits) */
		CHECK(strlen(guardian_area_short(biome)) <= 9, "area %d's short name is long for the PET", biome);
	}
	/* (a guardian never battled, as MegaMan speaks of him: a signal he
	 * does not know, or a name a Navi on the net gave, in every area) */
	for (int biome = 0; biome < BIOME_COUNT; ++biome) {
		const char *area = guardian_area_in_text(biome, LAYER_NORMAL);
		char line[400];
		snprintf(line, sizeof line, "@M We're through to %s, Lan! A strong Navi's signal waits at its end. I don't recognize "
			"it.|@L Then let's find out who. Let's go!", area);
		snprintf(what, sizeof what, "the arrival's words in area %d", biome);
		check_talk(what, line);
		snprintf(what, sizeof what, "the briefing in area %d", biome);
		snprintf(line, sizeof line, "@M Layer 19, Lan: %s. A strong Navi's signal waits at its end. I don't recognize it.|@M We've got "
			"no battle data on it, Lan. Watch the yellow panels: they light where an attack will land!", area);
		check_talk(what, line);
		for (int navi = 1; navi <= 18; ++navi) {
			if (navi == 17) continue;
			snprintf(line, sizeof line, "@M Layer 19, Lan: %s. %s waits at its end, if the word on the net is right.|@M We've got no "
				"battle data on him, Lan. Watch the yellow panels: they light where an attack will land!", area, guardian(navi)->name);
			check_talk(what, line);
			snprintf(line, sizeof line, "@M Layer 19, Lan: %s. %s guards the end of it, word is.", area, guardian(navi)->name);
			check_talk(what, line);
		}
		/* (the way on's question, each way's guardian battled or never,
		 * with no dark way, a sealed one and an open one, to every other
		 * area; one never battled is said once, not once a way: session
		 * 64. The area's name copied first: the next call writes over it) */
		char here[32];
		snprintf(here, sizeof here, "%s", area);
		for (int other = 0; other < BIOME_COUNT; ++other) {
			char there[32], q[400];
			snprintf(there, sizeof there, "%s", guardian_area_in_text(other, LAYER_NORMAL));
			const char *areas[3] = { here, there, "the Undernet" };
			for (int dark = 0; dark <= 2; ++dark)
				for (int known = 0; known < 8; ++known) {
					const char *who[3];
					for (int k = 0; k < 3; ++k) who[k] = known >> k & 1 ? "TomahawkMan (Breaker)" : NULL;
					guardian_way_question(q, sizeof q, who, areas, dark);
					snprintf(what, sizeof what, "the way on from area %d to %d (dark way %d, battled %d)", biome, other, dark, known);
					check_talk(what, q);
					const char *once = strstr(q, "Navi we've never battled");
					CHECK(!once || !strstr(once + 1, "Navi we've never battled"), "%s: one never battled named twice: \"%s\"", what, q);
				}
		}
		/* (the way's option, alone on its line in the choice) */
		CHECK(strlen(guardian_area_name(biome)) <= 18, "area %d's name is long for the way on's choice", biome);
	}
}

/* Following the arrow gets MegaMan there: from the arrival and from each
 * room, walking the way it shows at a run's pace (a look every 5 frames, a
 * new way taken when two looks agree, as the director's arrow_update),
 * sliding along the floor's edges as BN6 slides him, and stopped, turned
 * towards the floor ahead, as a player would, he reaches the guardian or
 * the exit pad. (It led past a turn, into a platform's corner, round in
 * circles.) */
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

/* Whether MegaMan stands on the floor at (x, y) as BN6 keeps him there: a
 * panel's floor reaches 12.5 of its 32 units past its middle towards the
 * void (the walls' cells, coords.c and netmap_floor_cell: a run into a
 * corner stops him 12 past it, and a walkway takes him within 12 of its
 * line and stops him 13 off it), on into a floor panel beside it, and
 * into a corner where the three panels about it are floor. (Taken as a
 * panel's whole width, the floor never stopped him: following the arrow,
 * he never once failed to move, where three playtesters stopped about 35
 * times a session at band mouths and junctions, session 63.) */
static bool walk_on(double x, double y) {
	const double m = 12.5 / 32;
	int px = (int)lround(x), py = (int)lround(y);
	double fx = x - px, fy = y - py;
	int sx = fx > m ? 1 : fx < -m ? -1 : 0, sy = fy > m ? 1 : fy < -m ? -1 : 0;
	return walk_floor(px, py) && walk_floor(px + sx, py) && walk_floor(px, py + sy) && walk_floor(px + sx, py + sy);
}

/* one frame's step from (x, y) by (dx, dy), or along one axis of it (BN6's
 * walls push him out square: a way across the grid slides along an edge),
 * short of the objects but the goal (gx, gy) */
static bool walk_step(double *x, double *y, double dx, double dy, int gx, int gy) {
	const double tries[3][2] = { { dx, dy }, { dx, 0 }, { 0, dy } };
	for (int i = 0; i < 3; ++i) {
		if (fabs(tries[i][0]) + fabs(tries[i][1]) < 1e-9) continue;
		double nx = *x + tries[i][0], ny = *y + tries[i][1];
		if (!walk_on(nx, ny) || walk_blocked(nx, ny, gx, gy)) continue;
		*x = nx;
		*y = ny;
		return true;
	}
	return false;
}

/* The floor's width at panel (x, y) across a step (dx, dy) along one axis:
 * the panels in a row through it, crosswise. */
static int floor_across(int x, int y, int dx, int dy) {
	int sx = dy ? 1 : 0, sy = dx ? 1 : 0, w = 1;
	for (int s = -1; s <= 1; s += 2)
		for (int k = 1; walk_floor(x + s * sx * k, y + s * sy * k); ++k) ++w;
	return w;
}

/* a frame's step (*dx, *dy) on the grid holding the pad's way `way` (RIGHT
 * +x -y, DOWN +x +y) */
static void way_step(int way, double speed, double *dx, double *dy) {
	double a = way * 3.14159265358979 / 4, right = cos(a), down = sin(a);
	double x = (right + down) / 2, y = (down - right) / 2, n = sqrt(x * x + y * y);
	*dx = fabs(x) < 1e-9 ? 0 : x / n * speed;
	*dy = fabs(y) < 1e-9 ? 0 : y / n * speed;
}

/* frames to get beside (tx, ty) from (x, y) as a playtester holds the
 * arrow's way: a look where he stands (the director turns the arrow at
 * once as he stops), then that way held for a batch, `hold` frames on a
 * straight run, half as long after each look that turns it, twice as long
 * (up to `hold`) after each that keeps it; -1 never, and *stuck set where
 * a look's way did not move him at all: he would stand there for good.
 * *looks: the looks, *still: frames he held the way and did not move. */
static int hold_arrow(double x, double y, int tx, int ty, int hold, int *looks, int *still, bool *stuck) {
	int len, last = -1, h = hold;
	*stuck = false;
	if (route_way(x, y, tx, ty, &len) < 0) return -1;
	for (int f = 0, budget = 400 + 96 * len; f < budget;) {
		int w = route_way(x, y, tx, ty, &len), moved = 0;
		double dx, dy;
		h = last < 0 ? hold : w == last ? (h * 2 < hold ? h * 2 : hold) : (h / 2 > 4 ? h / 2 : 4);
		last = w;
		way_step(w, 2.0 / 32, &dx, &dy);
		++*looks;
		for (int k = 0; k < h; ++k, ++f) {
			if (abs((int)lround(x) - tx) + abs((int)lround(y) - ty) <= 1) return f;
			if (walk_step(&x, &y, dx, dy, tx, ty)) ++moved;
			else ++*still;
		}
		if (!moved) { *stuck = true; return -1; }
	}
	return -1;
}

/* frames to get beside (tx, ty) from (x, y) following the arrow, -1 never */
static int arrow_turns, arrow_frames, arrow_swings;
/* (and how it walks: frames MegaMan, holding the way it shows, does not
 * move, and the stops they make, each a new direction to find; of its
 * looks, those where he stands on a walkway or band, floor two panels wide
 * or less across the walk, and of those the ones that show a way across
 * the grid, one of the screen's straight four, against the screen's
 * diagonals the walkways run on) */
static int arrow_stalls, arrow_stops, arrow_band_looks, arrow_band_diagonals;
static int follow_arrow(double x, double y, int tx, int ty) {
	const double speed = 2.0 / 32;   /* panels a frame */
	int len, shown = route_way(x, y, tx, ty, &len), pending = shown, stuck = 0;
	if (shown < 0) return -1;
	int budget = 200 + 48 * len, before = -1, turned_at = -1000;
	for (int f = 1; f <= budget; ++f) {
		int cx = (int)lround(x), cy = (int)lround(y);
		if (abs(cx - tx) + abs(cy - ty) <= 1) return f;
		++arrow_frames;
		if (f % 5 == 0) {
			int w = route_way(x, y, tx, ty, &len);
			if (w >= 0 && w == pending && shown != w && !route_way_holds(shown, 0.8)) {
				++arrow_turns;
				if (w == before && f - turned_at < 60) ++arrow_swings;
				before = shown; turned_at = f; shown = w;
			}
			pending = w;
			int fx = route_walk_len ? route_walk[route_walk_len - 1] % MAP_W - cx : 0, fy = route_walk_len ? route_walk[route_walk_len - 1] / MAP_W - cy : 0;
			if (w >= 0 && abs(fx) + abs(fy) == 1 && floor_across(cx, cy, fx, fy) <= 2) {
				++arrow_band_looks;
				arrow_band_diagonals += shown % 2 == 0;
			}
		}
		double dx, dy;
		way_step(shown, speed, &dx, &dy);
		if (walk_step(&x, &y, dx, dy, tx, ty)) { stuck = 0; continue; }
		++arrow_stalls;
		arrow_stops += !stuck;
		/* along one axis into an edge: off the lane of the floor ahead,
		 * towards its line; at the floor's end, towards the side it goes
		 * on */
		if (++stuck >= 3 && (!dx || !dy)) {
			int ax = dx > 0 ? 1 : dx < 0 ? -1 : 0, ay = dy > 0 ? 1 : dy < 0 ? -1 : 0;
			if (walk_floor(cx + ax, cy + ay)) {
				double s = (ax ? y - cy : x - cx) > 0 ? -speed : speed;
				walk_step(&x, &y, ay ? s : 0, ax ? s : 0, tx, ty);
				continue;
			}
			for (int s = -1; s <= 1; s += 2) {
				int bx = ay ? s : 0, by = ax ? s : 0;
				if (walk_floor(cx + bx, cy + by) && walk_floor(cx + ax + bx, cy + ay + by)) {
					walk_step(&x, &y, bx * speed, by * speed, tx, ty);
					break;
				}
			}
		}
	}
	return -1;
}

/* Where the arrow leads on the layer: the guardian on a boss layer, else
 * the exit (or the way back); false for none. */
static bool arrow_goal(int *tx, int *ty) {
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		bool goal = layer.boss_layer ? o->type == OBJ_BOSS : o->type == OBJ_EXIT || o->type == OBJ_RETURN;
		if (goal) { *tx = (int)o->x; *ty = (int)o->y; return true; }
	}
	return false;
}

/* Where MegaMan sets off for the goal (tx, ty) from room r (-1 the
 * arrival): its middle, or beside what stands there, where he would (a
 * Mystery Data on a pad's); false for a room reached only across Rush or a
 * teleport, or one beside the goal. */
static bool arrow_start(int r, int tx, int ty, int *x, int *y) {
	if (r >= 0 && island_room(r)) return false;
	int sx = r < 0 ? (int)layer.obj[0].x : layer.rooms[r].ax, sy = r < 0 ? (int)layer.obj[0].y : layer.rooms[r].ay;
	for (int i = 1; i < layer.nobj; ++i) {
		if (!layer.obj[i].solid || (int)layer.obj[i].x != sx || (int)layer.obj[i].y != sy) continue;
		static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k)
			if (layer.cell[sy + d4[k][1]][sx + d4[k][0]] == C_PATH) { sx += d4[k][0]; sy += d4[k][1]; break; }
		break;
	}
	*x = sx;
	*y = sy;
	return abs(sx - tx) + abs(sy - ty) > 1;
}

static void test_arrow(void) {
	int walks = 0, lost = 0;
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	for (uint32_t seed = 1; seed <= 120; ++seed) {
		int depth = 1 + (int)(seed % 25);
		int kind = seed % 7 == 0 ? LAYER_UNDERNET : seed % 11 == 0 ? LAYER_SECRET : LAYER_NORMAL;
		layer_generate(seed * 7919u, depth, biome_for_depth(depth), kind, &kit);
		int tx, ty, sx, sy;
		if (!arrow_goal(&tx, &ty)) continue;
		for (int r = -1; r < layer.nrooms; ++r) {
			if (!arrow_start(r, tx, ty, &sx, &sy)) continue;
			++walks;
			if (follow_arrow(sx, sy, tx, ty) >= 0) continue;
			if (++lost <= 5) printf("  seed %u (depth %d): lost from %d,%d on the way to %d,%d\n", seed, depth, sx, sy, tx, ty);
		}
	}
	CHECK(lost == 0, "the arrow lost MegaMan on %d of %d walks", lost, walks);
	/* (how steady it is: a turn back to the way before within a second is a
	 * swing; the director holds a way a little past its edge, 0.8 eighths,
	 * which took a quarter off them) */
	printf("  arrow: %d turns over %d frames of walking, %d swung back within a second\n", arrow_turns, arrow_frames, arrow_swings);
}

/* Following the arrow in every area, from the arrival and each room to the
 * exit or the guardian (sixty layers each, as test_walks'): it never loses
 * MegaMan, never shows a way across the grid on a walkway or band, and
 * held a playtester's batch at a time, its way moves him wherever he
 * stands; per area, how it walks him (CW_WAY_STATS). Three playtesters
 * lost about 35 calls to MegaMan stopping at band mouths and junctions,
 * Green HP's crossing bands and the Judge Tree's catwalks, where the arrow
 * pointed straight across the screen on two-wide bands and flipped at
 * their junctions (session 63). */
static void test_arrow_areas(void) {
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	int all[8] = { 0 };   /* walks, lost, stops, band looks, across the grid, walks held there, their looks, stuck */
	for (int b = 0; b < BIOME_COUNT; ++b) {
		int walks = 0, lost = 0, there = 0, looks = 0, still = 0, stuck = 0;
		int frames = arrow_frames, stalls = arrow_stalls, stops = arrow_stops, turns = arrow_turns, swings = arrow_swings;
		int band = arrow_band_looks, diagonals = arrow_band_diagonals;
		for (uint32_t seed = 1; seed <= 60; ++seed) {
			layer_generate(seed * 104729u + (uint32_t)b, seed % 2 ? 6 : 5, b, LAYER_NORMAL, &kit);
			int tx, ty, sx, sy;
			if (!arrow_goal(&tx, &ty)) continue;
			for (int r = -1; r < layer.nrooms; ++r) {
				if (!arrow_start(r, tx, ty, &sx, &sy)) continue;
				++walks;
				lost += follow_arrow(sx, sy, tx, ty) < 0;
				int n = 0, s = 0;
				bool stood;
				if (hold_arrow(sx, sy, tx, ty, 24, &n, &s, &stood) >= 0) { ++there; looks += n; still += s; }
				if (stood && ++stuck <= 3) printf("  area %d seed %u: held, the arrow's way stood MegaMan still on the way from %d,%d to %d,%d\n", b, seed, sx, sy, tx, ty);
			}
		}
		frames = arrow_frames - frames; stalls = arrow_stalls - stalls; stops = arrow_stops - stops; turns = arrow_turns - turns;
		swings = arrow_swings - swings; band = arrow_band_looks - band; diagonals = arrow_band_diagonals - diagonals;
		if (getenv("CW_WAY_STATS"))
			printf("  arrow area %2d: %3d walks of %3.0f frames, %.2f stops (%d frames still) and %.2f turns a walk (%.2f swung back), %4.1f%% of %5d"
				" looks on a band across the grid; held, %.1f looks a walk (%.1f%% of held frames still), %d stuck, %d out of time\n",
				b, walks, (double)frames / walks, (double)stops / walks, stalls, (double)turns / walks, (double)swings / walks, 100.0 * diagonals / band,
				band, (double)looks / there, 100.0 * still / (24.0 * looks), stuck, walks - there - stuck);
		int now[8] = { walks, lost, stops, band, diagonals, there, looks, stuck };
		for (int k = 0; k < 8; ++k) all[k] += now[k];
	}
	CHECK(all[1] == 0, "the arrow lost MegaMan on %d of %d walks in every area", all[1], all[0]);
	CHECK(all[4] == 0, "the arrow pointed across the grid on a walkway or band at %d of %d looks", all[4], all[3]);
	CHECK(all[7] == 0, "held, the arrow's way stood MegaMan still on %d of %d walks", all[7], all[0]);
	printf("  arrow in every area: %d walks, %.3f stops a walk, %d of %d looks on a band across the grid; held, %.1f looks a walk, %d of the walks"
		" not there in time\n", all[0], (double)all[2] / all[0], all[4], all[3], (double)all[6] / all[5], all[0] - all[5] - all[7]);
}

/* The walk from the arrival to the exit (or the guardian) in every area:
 * its legs, the straight runs of two panels or more, fewer than eleven on
 * average. The Aquarium's and Judge Tree's catwalk mazes took 16 against
 * the rest's 8, and a playtester spent 110 of 249 moves walking and
 * reading the map there (session 29). */
static void test_walks(void) {
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	double most = 0;
	int most_area = -1;
	for (int b = 0; b < BIOME_COUNT; ++b) {
		double legs[2] = { 0, 0 };
		int n[2] = { 0, 0 };
		for (uint32_t seed = 1; seed <= 60; ++seed) {
			int boss = seed % 2;
			layer_generate(seed * 104729u + (uint32_t)b, boss ? 6 : 5, b, LAYER_NORMAL, &kit);
			int tx = -1, ty = -1, len = 0;
			for (int i = 0; i < layer.nobj && tx < 0; ++i) {
				const NetObj *o = &layer.obj[i];
				if (layer.boss_layer ? o->type == OBJ_BOSS : o->type == OBJ_EXIT) { tx = (int)o->x; ty = (int)o->y; }
			}
			if (tx < 0 || route_way(layer.obj[0].x, layer.obj[0].y, tx, ty, &len) < 0) continue;
			int k = layer.boss_layer, runs = 0, last = -1, run_len = 0;
			for (int i = 1; i < route_walk_len; ++i) {
				int d = route_walk[i] - route_walk[i - 1], dir = d == 1 ? 0 : d == -1 ? 1 : d > 0 ? 2 : 3;
				if (dir == last) ++run_len;
				else { runs += run_len >= 2; last = dir; run_len = 1; }
			}
			runs += run_len >= 2;
			legs[k] += runs;
			++n[k];
		}
		for (int k = 0; k < 2; ++k) {
			double avg = n[k] ? legs[k] / n[k] : 0;
			CHECK(avg < 11, "area %d: the walk to the %s takes %.1f legs", b, k ? "guardian" : "exit", avg);
			if (getenv("CW_WAY_STATS")) printf("  legs area %2d %s: %.2f\n", b, k ? "guardian" : "exit", avg);
			if (avg > most) { most = avg; most_area = b; }
		}
	}
	printf("  walks: the most winding area (%d) %.1f legs on average\n", most_area, most);
}

/* The way across (docs/LEVEL_DESIGN.md, Navigation): from the arrival to
 * the guardian's antechamber or the exit, a layer crosses no more one-wide
 * walkways than its area's originals do, each a lining up for MegaMan. */
static void test_way_links(void) {
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	int over_all = 0, all = 0, worst_area = -1;
	double worst = 0;
	for (int b = 0; b < BIOME_COUNT; ++b) {
		int over = 0, n = 0, sum = 0, narrows = 0, across = 0, across_big = 0, across_len = 0, over_narrow = 0;
		for (uint32_t seed = 1; seed <= 60; ++seed) {
			layer_generate(seed * 7919u + (uint32_t)b, seed % 2 ? 6 : 5, b, LAYER_NORMAL, &kit);
			const Room *g = &layer.rooms[layer.arena >= 0 ? layer.ante : layer.exit_room];
			int k = layer_way_runs(layer.rooms[0].ax, layer.rooms[0].ay, g->ax, g->ay);
			int nw = layer_way_narrows(layer.rooms[0].ax, layer.rooms[0].ay, g->ax, g->ay);
			narrows += nw;
			over_narrow += layer_narrow_cap(b) && nw > layer_narrow_cap(b);
			{
				static uint8_t cells[MAP_W * MAP_H];
				for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) cells[y * MAP_W + x] = layer.cell[y][x] == C_PATH;
				int big, len;
				across += grid_way_narrows(cells, MAP_W, MAP_H, &big, &len);
				across_big += big; across_len += len;
			}
			sum += k;
			over += k > layer_way_cap(b);
			++n;
		}
		/* (where the area holds every one-wide walkway on the way to its
		 * originals' count, that count is the way's measure: a walkway it
		 * widened joins two small platforms into a big one, whose links
		 * the cap between big platforms then counts) */
		double share = (double)(layer_narrow_cap(b) ? over_narrow : over) / n;
		if (layer_narrow_cap(b))
			CHECK(share <= 0.2, "area %d: %d of %d ways cross more one-wide walkways than its originals' %d", b, over_narrow, n, layer_narrow_cap(b));
		else CHECK(share <= 0.1, "area %d: %d of %d ways cross more one-wide walkways than its cap %d", b, over, n, layer_way_cap(b));
		if (share > worst) { worst = share; worst_area = b; }
		over_all += layer_narrow_cap(b) ? over_narrow : over;
		all += n;
		if (getenv("CW_WAY_STATS"))
			printf("  area %2d: %.2f big-platform links, %.2f one-wide walkways on the way (%d past %d); across: %.2f, %.2f between big platforms, a way of %d\n", b,
				(double)sum / n, (double)narrows / n, over_narrow, layer_narrow_cap(b), (double)across / n, (double)across_big / n, across_len / n);
	}
	printf("  ways: %d of %d past their area's cap of one-wide walkways (the most in area %d, %.0f%%)\n", over_all, all, worst_area, 100 * worst);
}

/* The NaviCust's draft (docs/NAVICUST.md): three programs of three builds,
 * each of a tier its act has reached, none of the left-out ones; and
 * MegaMan's words for the bugs. */
static void test_navicust(void) {
	static const int left_out[] = { 15, 16, 17, 18, 19, 20, 22, 23, 24, 32, 33, 34 };
	int offered[47] = { 0 };
	for (uint32_t seed = 1; seed <= 400; ++seed) {
		rng_seed(seed * 2654435761u);
		int depth = 3 * (1 + (int)(seed % 7));   /* every guardian of a cycle, and the second cycle's first */
		if (depth > 18) depth = 22;
		NaviProgram p[NAVICUST_DRAFT];
		int n = navicust_draft(depth, p);
		CHECK(n == NAVICUST_DRAFT, "depth %d: a draft of %d", depth, n);
		for (int k = 0; k < n; ++k) {
			CHECK(navicust_in_pool(p[k].program), "depth %d: program %d offered", depth, p[k].program);
			for (unsigned j = 0; j < sizeof left_out / sizeof *left_out; ++j)
				CHECK(p[k].program != left_out[j], "depth %d: left-out program %d offered", depth, p[k].program);
			for (int j = 0; j < k; ++j)
				CHECK(navicust_build(p[j].program) != navicust_build(p[k].program), "depth %d: two of one build", depth);
			/* the first act's guardian: small programs only; HP+300 and up
			 * wait for the second cycle */
			if (depth == 3) CHECK(p[k].program <= 2 || p[k].program == 4 || p[k].program == 7 || p[k].program == 8 ||
				p[k].program == 13 || p[k].program == 25 || (p[k].program >= 35 && p[k].program <= 37) ||
				p[k].program == 41 || p[k].program == 42, "act 1's guardian offered program %d", p[k].program);
			if (depth <= 18) CHECK(p[k].program < 44, "the first cycle offered HP+%d00", p[k].program - 41);
			if (p[k].program <= 46) ++offered[p[k].program];
		}
	}
	int kinds = 0;
	for (int i = 1; i <= 46; ++i) kinds += offered[i] > 0;
	CHECK(kinds >= 25, "only %d programs ever offered", kinds);
	CHECK(navicust_expmemry(6) && navicust_expmemry(12) && !navicust_expmemry(3) && !navicust_expmemry(25), "ExpMemry milestones");
	/* the packing (made-up shapes): a bar on the command line, two squares
	 * that must share it (not of one colour), a plus part off it, too
	 * many cells, and a turn */
	NaviShape bar = { .kind = NAVI_PART, .color = 1 }, sq = { .kind = NAVI_PART, .color = 2 }, sq2 = sq, plus = { .kind = NAVI_PLUS, .color = 3 };
	for (int x = 0; x < 4; ++x) bar.cell[3][x] = 1;
	for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) sq.cell[y][x] = sq2.cell[y][x] = 1;
	plus.cell[0][0] = plus.cell[1][0] = 1;
	int bw, bh;
	navicust_board(0, &bw, &bh);
	CHECK(bw == 4 && bh == 4, "the first board is 4x4");
	navicust_board(2, &bw, &bh);
	CHECK(bw == 5 && bh == 5, "two ExpMemry: 5x5");
	CHECK(navicust_pack(&bar, 1, 4, 4), "a bar of four fits the command line");
	/* (a compression code as BN6's table holds it, 0 L 2 R 4 A 6 B: Custom1's,
	 * and none, issue #50) */
	char code[12];
	static const uint8_t custom1[10] = { 0, 6, 6, 2, 6, 4, 4, 0, 2, 2 }, none[10] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	CHECK(navicust_code_text(custom1, code) && !strcmp(code, "LBBRB AALRR"), "Custom1's code spelt five and five: %s", code);
	CHECK(!navicust_code_text(none, code), "a program with no code has none");
	CHECK(navicust_pack(&bar, 1, 3, 4) && !navicust_pack(&bar, 1, 3, 3), "a bar of four turned upright on a board three wide, not on three by three");
	NaviShape two[2] = { sq, sq2 };
	CHECK(!navicust_pack(two, 2, 4, 4), "two squares of one colour both on the command line touch");
	two[1].color = 5;
	CHECK(navicust_pack(two, 2, 4, 4), "two squares of two colours share the command line");
	NaviShape three[3] = { sq, two[1], plus };
	CHECK(navicust_pack(three, 3, 4, 4), "a plus part above them");
	/* (plus parts of one colour may not touch either, as BN6's compile
	 * counts them: a 2x2 and a T, both blue, off a 5x4 board's command
	 * line, session 64) */
	NaviShape blue_sq = { .kind = NAVI_PLUS, .color = 5 }, blue_t = { .kind = NAVI_PLUS, .color = 5 };
	for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) blue_sq.cell[y][x] = 1;
	blue_t.cell[0][0] = blue_t.cell[0][1] = blue_t.cell[0][2] = blue_t.cell[1][1] = 1;
	NaviShape blues[2] = { blue_sq, blue_t };
	CHECK(!navicust_pack(blues, 2, 5, 4), "two blue plus parts, a square and a T, find no clean place off a 5x4 board's command line");
	blues[1].color = 1;
	CHECK(navicust_pack(blues, 2, 5, 4), "a blue square and a white T do");
	NaviShape tall = { .kind = NAVI_PART, .color = 4 };
	for (int y = 0; y < 5; ++y) tall.cell[y][3] = 1;
	CHECK(!navicust_pack(&tall, 1, 4, 4) && navicust_pack(&tall, 1, 5, 4), "five in a column: turned on a board five wide");
	/* (turned only with its colour's Spin: the bar is white, colour 1; the
	 * column red, colour 4) */
	navicust_set_spins(0x3F & ~1u);
	CHECK(navicust_pack(&bar, 1, 4, 4) && !navicust_pack(&bar, 1, 3, 4), "without the white Spin a white bar of four stays flat");
	navicust_set_spins(1u << 3);
	CHECK(!navicust_pack(&bar, 1, 3, 4) && navicust_pack(&tall, 1, 5, 4), "the red Spin turns the red column, not the white bar");
	navicust_set_spins(0);
	CHECK(!navicust_pack(&tall, 1, 5, 4), "without Spins the column stays upright");
	/* (a board laid out turned before its Spin was lost keeps its turn; the
	 * new program does not) */
	NaviShape old_board[2] = { tall, plus };
	CHECK(navicust_pack(old_board, 2, 5, 4), "a column the board already holds turned may stay turned");
	NaviShape new_last[2] = { plus, tall };
	CHECK(!navicust_pack(new_last, 2, 5, 4), "a new column without its Spin is not turned to fit");
	navicust_set_spins(0x3F);
	NaviShape many[5] = { bar, bar, bar, bar, bar };
	CHECK(!navicust_pack(many, 5, 4, 4), "twenty cells on sixteen");
	/* the free cells as the board stands (BN6's grid, the board from (1,
	 * 1), the command line its row 3): a pink pair in the command line's
	 * middle leaves a cell free at each end, so a white pair fits only once
	 * the pink one moves, or turned upright with the white Spin, and a
	 * second pink pair not at all (a playtester's Custom1 fitted only after
	 * he moved two programs, session 55) */
	uint8_t grid[NAVICUST_GRID * NAVICUST_GRID] = { 0 };
	grid[3 * NAVICUST_GRID + 2] = grid[3 * NAVICUST_GRID + 3] = 1;
	const NaviPart parts[1] = { { "PinkPair", NAVI_PART, 3 } };
	NaviShape pink = { .kind = NAVI_PART, .color = 3 }, white = { .kind = NAVI_PART, .color = 1 };
	pink.cell[0][0] = pink.cell[0][1] = white.cell[0][0] = white.cell[0][1] = 1;
	NaviShape stood[2] = { pink, white };
	navicust_set_spins(0);
	CHECK(!navicust_fits_free(grid, parts, 1, &white, 4, 4) && navicust_pack(stood, 2, 4, 4), "a white pair fits only once the pink one moves");
	navicust_set_spins(1);
	CHECK(navicust_fits_free(grid, parts, 1, &white, 4, 4), "with the white Spin the white pair stands upright at an end");
	navicust_set_spins(0x3F);
	CHECK(!navicust_fits_free(grid, parts, 1, &pink, 4, 4), "a second pink pair would touch the first");
	/* the bug words: none, one light, several */
	uint8_t bugs[NAVICUST_BUGS] = { 0 };
	CHECK(!*navicust_bug_words(bugs, false, NULL), "words for no bug");
	bugs[9] = 1;
	CHECK(strstr(navicust_bug_words(bugs, false, NULL), "A light HP bug") != NULL, "a light HP bug: %s", navicust_bug_words(bugs, false, NULL));
	bugs[9] = 2;
	CHECK(strstr(navicust_bug_words(bugs, false, NULL), "An HP bug") != NULL, "an HP bug: %s", navicust_bug_words(bugs, false, NULL));
	bugs[7] = 5;
	bugs[11] = 1;
	const char *w = navicust_bug_words(bugs, false, NULL);
	CHECK(strstr(w, "has bugs!") && strstr(w, "A bad buster bug") && strstr(w, "Five colors"), "several bugs: %s", w);
	CHECK(strstr(w, "command line") != NULL, "a placement bug says where to look: %s", w);
	/* (the cause where the board shows it, in place of the rules) */
	w = navicust_bug_words(bugs, false, "HP+100 is a plus part on the command line: plus parts go anywhere else.");
	CHECK(strstr(w, "HP+100 is a plus part") && !strstr(w, "Bugs come from"), "a placement bug names its cause: %s", w);
	memset(bugs, 0, sizeof bugs);
	bugs[11] = 1;
	CHECK(!strstr(navicust_bug_words(bugs, false, NULL), "command line"), "a colours' bug alone names itself");
	CHECK(strstr(navicust_bug_words(bugs, true, NULL), "the RUN says OK, but") != NULL, "after a RUN, its OK answered: %s", navicust_bug_words(bugs, true, NULL));
	check_talk("navicust_bug_words", navicust_bug_words(bugs, true, NULL));
	/* which programs turn: those whose colour's Spin is held */
	navicust_set_spins(0);
	CHECK(strstr(navicust_turn_words(0), "none yet") != NULL, "no Spin: %s", navicust_turn_words(0));
	check_talk("navicust_turn_words none", navicust_turn_words(0));
	navicust_set_spins(1u << 2);
	CHECK(strstr(navicust_turn_words(0), "only pink programs") && strstr(navicust_turn_words(0), "that Spin"), "the pink Spin: %s", navicust_turn_words(0));
	navicust_set_spins(0x3F & ~(1u << 5));
	CHECK(strstr(navicust_turn_words(0), "white, yellow, pink, red and blue programs") != NULL, "five Spins: %s", navicust_turn_words(0));
	check_talk("navicust_turn_words five", navicust_turn_words(0));
	navicust_set_spins(0x3F);
	CHECK(!strstr(navicust_turn_words(0), "only"), "all six: %s", navicust_turn_words(0));
}

/* Every layout an area draws builds as planned: one that never fits the
 * window falls back to another every time, and its share of the area is
 * silently lost. */
static void test_layouts_build(void) {
	for (int b = 0; b < BIOME_COUNT; ++b)
		for (int l = 0; l < LAYOUT_COUNT; ++l) {
			if (!layout_weight(b, l)) continue;
			int ok = 0;
			for (uint32_t seed = 1; seed <= 40; ++seed) {
				layout_forced = l;
				layer_generate(seed * 7919u, 2 + (int)(seed % 7), b, LAYER_NORMAL, &kit);
				ok += layer.layout == l;
			}
			layout_forced = -1;
			CHECK(ok >= 32, "area %d builds %s on %d of 40 seeds", b, layout_names[l], ok);
		}
}

/* The touch controls: on the screen, clear of the picture and of each
 * other where there is room, thumb sized, and what a finger holds. */
static bool boxes_meet(const TouchBox *a, const TouchBox *b) {
	return fabsf(a->cx - b->cx) * 2 < a->w + b->w && fabsf(a->cy - b->cy) * 2 < a->h + b->h;
}

static bool round_control(int c) { return c == TOUCH_DPAD || c == TOUCH_A || c == TOUCH_B; }

/* Laid out as platform.c does, the controls shown: sw x sh pixels at dpi. */
static void touch_screen_for(int sw, int sh, float dpi, TouchScreen *s) {
	int sx = sw / 240, sy = sh / 160, scale = sx < sy ? sx : sy;
	float dp = dpi / 160;
	scale = touch_fit_scale(sw, sh, dp, scale < 1 ? 1 : scale);
	int w = sw / scale, h = sh / scale, ox = (sw - w * scale) / 2, oy = (sh - h * scale) / 2;
	int cx = (w - 240) / 2, cy = (h - 160) / 2, top = touch_picture_top(sw, sh, dp, scale);
	if (top >= 0 && (top - oy + scale - 1) / scale + 160 <= h) cy = (top - oy + scale - 1) / scale;
	TouchScreen t = { sw, sh, dp, ox + cx * scale, oy + cy * scale, 240 * scale, 160 * scale };
	*s = t;
}

static void check_touch(const char *what, int sw, int sh, float dpi, int shape) {
	TouchScreen s;
	touch_screen_for(sw, sh, dpi, &s);
	TouchPrefs p;
	touch_prefs_default(&p);
	TouchLayout t;
	touch_layout_for(&s, &p, &t);
	static const char *const shapes[] = { "under", "beside", "over" };
	CHECK(t.shape == shape, "%s: the touch controls stand %s the picture, not %s (a %dx%d picture)", what, shapes[t.shape], shapes[shape], s.pw, s.ph);
	TouchBox pic = { s.px + s.pw / 2.f, s.py + s.ph / 2.f, (float)s.pw, (float)s.ph };
	float mm = s.dp * 6.3f;
	for (int c = 0; c < TOUCH_CONTROLS; ++c) {
		const TouchBox *b = &t.box[c];
		CHECK(b->cx - b->w / 2 >= -0.5f && b->cy - b->h / 2 >= -0.5f && b->cx + b->w / 2 <= sw + 0.5f && b->cy + b->h / 2 <= sh + 0.5f,
			"%s: control %d leaves the screen", what, c);
		CHECK(shape == TOUCH_OVER || !boxes_meet(b, &pic), "%s: control %d covers the picture", what, c);
		for (int d = c + 1; d < TOUCH_CONTROLS; ++d) {
			const TouchBox *e = &t.box[d];
			float dx = b->cx - e->cx, dy = b->cy - e->cy, rr = b->w / 2 + e->w / 2;
			CHECK(round_control(c) && round_control(d) ? dx * dx + dy * dy > rr * rr : !boxes_meet(b, e), "%s: controls %d and %d overlap", what, c, d);
		}
	}
	/* thumb sized, in millimetres: a D-pad of 20 at least, A and B of 9 */
	const TouchBox *dp = &t.box[TOUCH_DPAD];
	float r = dp->w / 2, cx = dp->cx, cy = dp->cy;
	CHECK(dp->w / mm >= 19.8f, "%s: a D-pad of %.1f mm is small for a thumb", what, dp->w / mm);
	CHECK(t.box[TOUCH_A].w / mm >= 9 && t.box[TOUCH_B].w / mm >= 9, "%s: A of %.1f mm is small for a thumb", what, t.box[TOUCH_A].w / mm);
	CHECK(dp->w / mm <= 30.5f && t.box[TOUCH_A].w / mm <= 14.5f, "%s: a D-pad of %.1f mm is past its size", what, dp->w / mm);
	/* each control's middle is its own */
	for (int c = 0; c < TOUCH_CONTROLS; ++c)
		CHECK(touch_control_at(&t, t.box[c].cx, t.box[c].cy, -1) == c, "%s: control %d's middle does not reach it", what, c);
	/* the D-pad: nothing in its middle, eight ways of 45 degrees (BN6's
	 * walkways run along the diagonals), four of 90 off the map */
	CHECK(touch_dpad_steer(&t, cx, cy, 0, false) == 0, "%s: the D-pad's middle holds a direction", what);
	CHECK(touch_dpad_steer(&t, cx + r * 0.75f, cy + 1, 0, false) == BTN_RIGHT, "%s: right of the middle is not RIGHT", what);
	CHECK(touch_dpad_steer(&t, cx - 2, cy - r * 0.75f, 0, false) == BTN_UP, "%s: above the middle is not UP", what);
	CHECK(touch_dpad_steer(&t, cx + r / 2, cy + r / 2, 0, false) == (BTN_RIGHT | BTN_DOWN), "%s: the diagonal is not RIGHT+DOWN", what);
	CHECK(touch_dpad_steer(&t, cx + r * 0.866f, cy - r / 2, 0, false) == (BTN_RIGHT | BTN_UP), "%s: 30 degrees up is not RIGHT+UP", what);
	CHECK(touch_dpad_steer(&t, cx + r * 0.94f, cy - r * 0.342f, 0, false) == BTN_RIGHT, "%s: 20 degrees up is not RIGHT", what);
	CHECK(touch_dpad_steer(&t, cx + r * 0.866f, cy - r / 2, 0, true) == BTN_RIGHT, "%s: four ways, 30 degrees up is not RIGHT", what);
	CHECK(touch_dpad_steer(&t, cx + r / 2, cy - r * 0.866f, 0, true) == BTN_UP, "%s: four ways, 60 degrees up is not UP", what);
	CHECK(touch_dpad_steer(&t, cx + r / 2 + 2, cy + r / 2, 0, true) != (BTN_RIGHT | BTN_DOWN), "%s: four ways gave a diagonal", what);
	/* a direction held stays 8 degrees past its edge, and a thumb rolling
	 * back toward the middle keeps it until the middle */
	CHECK(touch_dpad_steer(&t, cx + r * sinf(0.52f), cy - r * cosf(0.52f), BTN_UP, false) == BTN_UP, "%s: UP lost 30 degrees off it", what);
	CHECK(touch_dpad_steer(&t, cx + r * sinf(0.62f), cy - r * cosf(0.62f), BTN_UP, false) == (BTN_UP | BTN_RIGHT), "%s: UP kept 35 degrees off it", what);
	CHECK(touch_dpad_steer(&t, cx, cy - r * 0.16f, BTN_UP, false) == BTN_UP, "%s: UP lost a sixth out", what);
	CHECK(touch_dpad_steer(&t, cx, cy - r * 0.16f, 0, false) == 0, "%s: a sixth out from the middle steers", what);
	CHECK(touch_dpad_steer(&t, cx, cy + r * 0.1f, BTN_UP, false) == 0, "%s: the middle holds a direction", what);
	CHECK(touch_dpad_steer(&t, cx - 3 * r, cy, BTN_LEFT, false) == BTN_LEFT, "%s: a thumb slid off the D-pad lost it", what);
	/* A and B reach past their art, the one held further; between them, the nearer */
	const TouchBox *a = &t.box[TOUCH_A], *b = &t.box[TOUCH_B];
	CHECK(touch_control_at(&t, a->cx + a->w / 2 * 1.25f, a->cy, -1) == TOUCH_A, "%s: a finger just off A misses it", what);
	CHECK(touch_control_at(&t, a->cx + a->w / 2 * 1.5f, a->cy, TOUCH_A) == TOUCH_A, "%s: A held lets go of a wobbling thumb", what);
	CHECK(touch_control_at(&t, a->cx + (b->cx - a->cx) * 0.4f, a->cy + (b->cy - a->cy) * 0.4f, TOUCH_B) == TOUCH_A,
		"%s: nearer A, a thumb from B does not reach A", what);
}

static void test_touch(void) {
	check_touch("a phone upright (1080x2400)", 1080, 2400, 420, TOUCH_BELOW);
	check_touch("a phone upright (1170x2532)", 1170, 2532, 480, TOUCH_BELOW);
	check_touch("a phone upright (720x1600)", 720, 1600, 280, TOUCH_BELOW);
	check_touch("a 16:9 phone upright (1080x1920)", 1080, 1920, 420, TOUCH_BELOW);
	check_touch("a phone's page upright (390x844)", 390, 844, 160, TOUCH_BELOW);
	check_touch("a phone on its side (2400x1080)", 2400, 1080, 420, TOUCH_SIDE);
	check_touch("a phone on its side (2532x1170)", 2532, 1170, 480, TOUCH_SIDE);
	check_touch("a phone on its side (1600x720)", 1600, 720, 280, TOUCH_SIDE);
	check_touch("a 16:9 phone on its side (1920x1080)", 1920, 1080, 420, TOUCH_SIDE);
	check_touch("a phone's page on its side (844x390)", 844, 390, 160, TOUCH_SIDE);
	check_touch("a 4:3 tablet (2048x1536)", 2048, 1536, 320, TOUCH_SIDE);
	check_touch("a 4:3 tablet upright (1536x2048)", 1536, 2048, 320, TOUCH_BELOW);
	check_touch("a 16:10 tablet (2560x1600)", 2560, 1600, 320, TOUCH_SIDE);
	check_touch("a 16:10 tablet upright (1600x2560)", 1600, 2560, 320, TOUCH_BELOW);
	check_touch("a Steam Deck (1280x800)", 1280, 800, 215, TOUCH_SIDE);
	check_touch("a Retroid Pocket (1334x750)", 1334, 750, 326, TOUCH_SIDE);
	check_touch("a square window (1000x1000)", 1000, 1000, 320, TOUCH_OVER);

	/* the player's arrangement (the editor, touch.ini): a control moved
	 * and sized lands where it was put, whole on the screen, the others as
	 * laid out; the file reads back as written */
	TouchScreen s;
	touch_screen_for(1080, 2400, 420, &s);
	TouchPrefs p;
	touch_prefs_default(&p);
	TouchLayout def, t;
	touch_layout_for(&s, &p, &def);
	p.place[TOUCH_BELOW][TOUCH_A] = (TouchPlace){ 500, 900, 150, 60, true };
	p.place[TOUCH_BELOW][TOUCH_START] = (TouchPlace){ 0, 0, 120, 0, false };
	p.place[TOUCH_BELOW][TOUCH_DPAD] = (TouchPlace){ 1000, 1000, 0, 0, true };
	p.place[TOUCH_SIDE][TOUCH_B] = (TouchPlace){ 10, 20, 60, 0, true };
	touch_layout_for(&s, &p, &t);
	const TouchBox *a = &t.box[TOUCH_A];
	CHECK(fabsf(a->cx - 540) <= 1 && fabsf(a->cy - 2160) <= 1, "A moved to (540, 2160) stands at (%.0f, %.0f)", a->cx, a->cy);
	CHECK(fabsf(a->w - def.box[TOUCH_A].w * 1.5f) <= 1, "A at 150%% is %.0f across, laid out %.0f", a->w, def.box[TOUCH_A].w);
	CHECK(fabsf(t.alpha[TOUCH_A] - 0.6f) < 0.01f && t.alpha[TOUCH_B] == 1, "A at 60%% opacity is at %.2f", t.alpha[TOUCH_A]);
	CHECK(touch_control_at(&t, a->cx, a->cy, -1) == TOUCH_A, "a finger on the moved A does not reach it");
	CHECK(fabsf(t.box[TOUCH_START].w - def.box[TOUCH_START].w * 1.2f) < 0.01f && t.box[TOUCH_START].cx == def.box[TOUCH_START].cx,
		"START sized, not moved, left its place or its size");
	const TouchBox *d = &t.box[TOUCH_DPAD];
	CHECK(fabsf(d->cx + d->w / 2 - 1080) < 0.5f && fabsf(d->cy + d->h / 2 - 2400) < 0.5f, "the D-pad put at the corner leaves the screen");
	CHECK(!memcmp(&t.box[TOUCH_B], &def.box[TOUCH_B], sizeof(TouchBox)), "B moved beside the picture moved under it");
	static char text[4096];
	p.size = 80;
	p.opacity = 70;
	p.haptics = false;
	p.left_handed = true;
	touch_prefs_format(&p, text, sizeof text);
	TouchPrefs back;
	touch_prefs_parse(text, &back);
	CHECK(!memcmp(&back, &p, sizeof p), "touch.ini reads back otherwise than written:\n%s", text);
	touch_prefs_parse("size = 999\nopacity = 5\nbelow a 2000 5 100 100\nside nothing 1 2 3 4\nover l - - 999 100\n", &back);
	CHECK(back.size == 100 && back.opacity == 100 && !back.place[TOUCH_BELOW][TOUCH_A].moved && !back.place[TOUCH_OVER][TOUCH_L].size,
		"touch.ini's bad values were taken");

	/* left-handed: the D-pad under the right thumb, A and B the left, apart */
	TouchPrefs left;
	touch_prefs_default(&left);
	left.left_handed = true;
	touch_layout_for(&s, &left, &t);
	CHECK(t.box[TOUCH_DPAD].cx > 540 && t.box[TOUCH_A].cx < 540 && t.box[TOUCH_B].cx < 540, "left-handed, the D-pad is not on the right");
	for (int c = 0; c < TOUCH_CONTROLS; ++c)
		for (int e = c + 1; e < TOUCH_CONTROLS; ++e)
			CHECK(!boxes_meet(&t.box[c], &t.box[e]) || (round_control(c) && round_control(e) &&
				hypotf(t.box[c].cx - t.box[e].cx, t.box[c].cy - t.box[e].cy) > (t.box[c].w + t.box[e].w) / 2),
				"left-handed, controls %d and %d overlap", c, e);
	/* their size: smaller anywhere; larger only where it fits (a phone's
	 * are as large as fit, a tablet's grow) */
	left.left_handed = false;
	left.size = 180;
	touch_layout_for(&s, &left, &t);
	CHECK(t.box[TOUCH_DPAD].w <= def.box[TOUCH_DPAD].w * 1.05f && t.size_max <= 105, "a phone's D-pad grew past what fits (%d%%)", t.size_max);
	left.size = 60;
	touch_layout_for(&s, &left, &t);
	CHECK(fabsf(t.box[TOUCH_DPAD].w - def.box[TOUCH_DPAD].w * 0.6f) < 1, "a phone's D-pad at 60%% is %.0f, laid out %.0f", t.box[TOUCH_DPAD].w, def.box[TOUCH_DPAD].w);
	touch_screen_for(1600, 2560, 320, &s);
	touch_layout_for(&s, &p, &def);
	left.size = 130;
	touch_layout_for(&s, &left, &t);
	touch_prefs_default(&left);
	touch_layout_for(&s, &left, &def);
	CHECK(t.size_max >= 130 && fabsf(t.box[TOUCH_A].w - def.box[TOUCH_A].w * 1.3f) < 1, "a tablet's A at 130%% did not grow (%d%% fits)", t.size_max);
}

/* Why BN6's board bugs (navicust_bug_cause), from its 7x7 grid: each
 * rule caught and named, and a clean board none. */
static void test_bug_cause(void) {
	uint8_t g[49];
	const char *why;
	NaviPart hp = { "HP+100", 1, 3 }, atk = { "Attack+1", 1, 3 }, cust = { "Custom1", 0, 5 }, armor = { "SuperArmor", 0, 4 };
	/* (a plus part over the command line, grid row 3) */
	memset(g, 0, sizeof g);
	g[2 * 7 + 2] = g[2 * 7 + 3] = g[3 * 7 + 2] = g[3 * 7 + 3] = 1;
	why = navicust_bug_cause(g, &hp, 1, 4, 4);
	CHECK(why && strstr(why, "HP+100 is a plus part on the command line"), "bug cause: plus part on the line (%s)", why ? why : "none");
	/* (a program part off it) */
	memset(g, 0, sizeof g);
	g[1 * 7 + 1] = g[2 * 7 + 1] = 1;
	why = navicust_bug_cause(g, &cust, 1, 4, 4);
	CHECK(why && strstr(why, "Custom1 is off the command line"), "bug cause: program part off the line (%s)", why ? why : "none");
	/* (past a 4x4 board's edge at column 5, which a 5-wide board holds) */
	memset(g, 0, sizeof g);
	g[3 * 7 + 4] = g[3 * 7 + 5] = 1;
	why = navicust_bug_cause(g, &armor, 1, 4, 4);
	CHECK(why && strstr(why, "SuperArmor goes past the board's edge"), "bug cause: past the edge (%s)", why ? why : "none");
	CHECK(!navicust_bug_cause(g, &armor, 1, 5, 4), "bug cause: none on a 5-wide board");
	/* (two pink programs side by side, off the line) */
	NaviPart two[2] = { atk, hp };
	memset(g, 0, sizeof g);
	g[1 * 7 + 1] = 1;
	g[1 * 7 + 2] = g[2 * 7 + 2] = 2;
	why = navicust_bug_cause(g, two, 2, 4, 4);
	CHECK(why && strstr(why, "Attack+1 and HP+100, both pink, touch"), "bug cause: one colour side by side (%s)", why ? why : "none");
	/* (clean: the program part on the line, the plus part off it, apart) */
	NaviPart clean[2] = { cust, hp };
	memset(g, 0, sizeof g);
	g[3 * 7 + 1] = g[3 * 7 + 2] = 1;
	g[1 * 7 + 3] = g[1 * 7 + 4] = 2;
	CHECK(!navicust_bug_cause(g, clean, 2, 4, 4), "bug cause: none on a clean board");
}

/* Another game's Navi (src/layer/xnavi.c): a sprite's length, as far as
 * its frame's tiles, palette, sub-animation and object list reach; none
 * where they reach past what can be read. */
static void test_xnavi(void) {
	uint8_t spr[128] = { 0 };
	uint8_t *b = spr + 4;   /* (after the 4-byte header) */
	b[0] = 4;               /* one animation, its frames at 4 */
	b[4] = 24; b[8] = 60; b[12] = 96; b[16] = 104;   /* tiles, palette, sub-animation, object table */
	b[4 + 18] = 0x80;       /* the last frame */
	b[24] = 32;             /* 32 bytes of tiles, to 60 */
	b[60] = 32;             /* a palette, to 96 */
	b[104] = 4;             /* one object list, at 108: an object, then 0xFF 0xFF, to 115 */
	b[113] = 0xFF; b[114] = 0xFF;
	CHECK(xnavi_sprite_len(spr, sizeof spr) == 119, "xnavi: the sprite's length (%u)", xnavi_sprite_len(spr, sizeof spr));
	CHECK(!xnavi_sprite_len(spr, 100), "xnavi: a sprite reaching past the data refused");
}

/* Another game's song (src/audio/xsong.c): the pointers a walk of its
 * tracks finds to move, and the voices it selects; a pointer out of the
 * sequence, or a command whose pointers it can't move, refuses the song. */
static void test_xsong(void) {
	const uint32_t base = 0x08123400u;
	/* 0: VOICE 5, a note, PATT to 20; 10: VOICE 3 and (running status) 7,
	 * GOTO 0; 20: the pattern, a note, PEND, then FINE */
	uint8_t seq[25] = { 0xBD, 5, 0xD0, 60, 100, 0xB3, 0, 0, 0, 0, 0xBD, 3, 7, 0xB2, 0, 0, 0, 0, 0, 0, 0xD0, 64, 100, 0xB4, 0xB1 };
	seq[6] = 0x14; seq[7] = 0x34; seq[8] = 0x12; seq[9] = 0x08;      /* base + 20 */
	seq[14] = 0x00; seq[15] = 0x34; seq[16] = 0x12; seq[17] = 0x08;   /* base */
	uint32_t starts[1] = { 0 }, ptrs[8];
	uint8_t programs[16] = { 0 };
	int n = 0;
	bool ok = xsong_walk(seq, sizeof seq, base, starts, 1, ptrs, &n, 8, programs);
	CHECK(ok && n == 2 && ptrs[0] == 6 && ptrs[1] == 14, "xsong: the PATT's and GOTO's pointers (%d, n %d)", ok, n);
	CHECK(programs[0] == (1 << 3 | 1 << 5 | 1 << 7), "xsong: voices 3, 5 and 7 (%#x)", programs[0]);
	/* (a GOTO out of the sequence) */
	seq[14] = 0x80;
	n = 0;
	CHECK(!xsong_walk(seq, sizeof seq, base, starts, 1, ptrs, &n, 8, programs), "xsong: a pointer out refused");
	seq[14] = 0x00;
	/* (REPT: its count, then its pointer; MEMACC refused) */
	uint8_t rept[12] = { 0xB5, 2, 0x06, 0x34, 0x12, 0x08, 0xD0, 60, 100, 0xB1, 0, 0 };
	n = 0;
	CHECK(xsong_walk(rept, 10, base, starts, 1, ptrs, &n, 8, programs) && n == 1 && ptrs[0] == 2, "xsong: REPT's pointer after its count");
	uint8_t memacc[6] = { 0xB9, 1, 2, 3, 0xB1, 0 };
	n = 0;
	CHECK(!xsong_walk(memacc, 5, base, starts, 1, ptrs, &n, 8, programs), "xsong: MEMACC refused");
}

int main(void) {
	test_sha1();
	test_lz77();
	test_generation();
	test_layer_make();
	test_layouts_build();
	test_stairs();
	test_arrow();
	test_arrow_areas();
	test_walks();
	test_way_links();
	test_navicust();
	test_depth_plan();
	test_pacing();
	test_town_moves();
	test_talk();
	test_touch();
	test_xsong();
	test_bug_cause();
	test_xnavi();
	if (failures) { printf("%d check(s) failed\n", failures); return 1; }
	printf("all core checks passed\n");
	return 0;
}

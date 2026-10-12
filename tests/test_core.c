/* ROM-free checks: hashing, decompression and map generation. */
#include <math.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "darkchips.h"
#include "data.h"
#include "game.h"
#include "net.h"
#include "net_arena.h"
#include "net_layouts.h"
#include "net_route.h"
#include "net_shapes.h"
#include "net_signature.h"
#include "layer_make.h"
#include "navicust.h"
#include "pacing.h"
#include "qr.h"
#include "rom.h"
#include "run.h"
#include "analytics.h"
#include "area_src.h"
#include "backup.h"
#include "chip_pool.h"
#include "guardians.h"
#include "launcher_roms.h"
#include "launcher_text.h"
#include "home_words.h"
#include "jobs.h"
#include "loot.h"
#include "meta.h"
#include "npc_lines.h"
#include "powers.h"
#include "rivals.h"
#include "save.h"
#include "save_blob.h"
#include "super_lines.h"
#include "flags.h"
#include "mapslot.h"
#include "text.h"
#include "town.h"
#include "town_lines.h"
#include "townmath.h"
#include "touch_layout.h"
#include "padmap.h"
#include "buttons.h"
#include "chips.h"
#include "claims.h"
#include "xchips.h"
#include "xchips_record.h"
#include "xchips_registry.h"
#include "xchips_text.h"
#include "soulkinds.h"
#include "xnavi.h"
#include "xnavi_bn5.h"
#include "xsong.h"
#include "debug.h"
#include "emu.h"
#include "bytes.h"
#include "thumb_reloc.h"
#include "battle_rng.h"
#include "bn5.h"
#include "bn6.h"
#include "xpanels.h"
#include "visitor_families.h"
#include "visitors.h"
#include "xpanels_look.h"
#include "xsea.h"
#include "xvirus_bn5.h"
#include "xvirus_dispatch.h"
#include "xvirus_families.h"
#include "xvirus_shim.h"

static int failures;
#define CHECK(cond, ...) do { if (!(cond)) { ++failures; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* Minimal stand-ins for the engine pieces the generator links against. */
char g_data_dir[512] = ".";
Run run;
Profile profile;
/* (no ROM, so no other game's areas: each BN6 area draws itself, and its
 * guardian is the run's pick) */
int run_dress(int biome) { return biome; }
int run_guardian(int biome) { return run.boss_order[biome]; }
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
void rng_restore(uint32_t s) { rng_s = s ? s : 0x9E3779B9u; }
uint32_t rng_next(void) { uint32_t x = rng_s; x ^= x << 13; x ^= x >> 17; x ^= x << 5; return rng_s = x; }
int rng_range(int lo, int hi) { return hi <= lo ? lo : lo + (int)(rng_next() % (uint32_t)(hi - lo + 1)); }
/* (the talk's pieces need no saves or game flags here; the flags set are
 * noted, for the powers a battle gives: test_beast) */
static int flags_set[16], nflags_set;
void flag_set(int flag) { if (nflags_set < 16) flags_set[nflags_set++] = flag; }
static bool flag_was_set(int flag) {
	for (int i = 0; i < nflags_set; ++i)
		if (flags_set[i] == flag) return true;
	return false;
}
bool flag_get(int flag) { (void)flag; return false; }
bool save_write_blob(const char *name, uint32_t magic, const void *data, size_t n) { (void)name; (void)magic; (void)data; (void)n; return false; }
bool save_read_blob(const char *name, uint32_t magic, void *data, size_t n) { (void)name; (void)magic; (void)data; (void)n; return false; }
bool save_read_blob_upto(const char *name, uint32_t magic, void *data, size_t n) { (void)name; (void)magic; (void)data; (void)n; return false; }
uint32_t mapslot_alloc(const void *bytes, int len) { (void)bytes; (void)len; return 0; }
/* (another game's songs: no core to copy them into) */
void emu_write(uint32_t addr, const void *data, size_t len) { (void)addr; (void)data; (void)len; }
uint32_t emu_read32(uint32_t addr) { (void)addr; return 0; }
bool emu_debug_on(void) { return false; }
/* (a request's chip: no ROM's pools here) */
int roll_chip(int depth, int bonus_tier, char *code) { (void)depth; (void)bonus_tier; *code = 'A'; return 1; }
char loot_fit_code(int id, char code, bool always) { (void)id; (void)always; return code; }
void emu_write32(uint32_t addr, uint32_t v) { (void)addr; (void)v; }
/* (the meta layer and a run's start words: no saves, chip pools or town) */
void profile_save(void) {}
int chip_pool_class(int rom_id) { (void)rom_id; return -1; }
int chip_pool_class_count(int cls) { (void)cls; return 0; }
bool chip_pool_older(int rom_id) { (void)rom_id; return false; }
int chip_pool_tier(int rom_id) { (void)rom_id; return -1; }
bool town_after_abandon;
/* (the nets' meeting, issue #82: no ROMs, so the families the visitors
 * come from are the tests' own, test_visitors; the older net's chips'
 * share, home's requests' rolls; BN6's stages by depth) */
int loot_older_share;
bool loot_visitors_always;
int loot_stage_allowed(int depth) { return depth > 12 ? 2 : depth > 6 ? 1 : 0; }
static VisitorKin test_kin[2][4];
static int test_nkin[2], test_role[2][64], test_id[2][64][6], test_panel = 2;
int visitor_kin(int net, const VisitorKin **out) { *out = test_kin[net & 1]; return test_nkin[net & 1]; }
int visitor_role_in(int net, int family) { return family > 0 && family < 64 ? test_role[net & 1][family] : VISITOR_FREE; }
int visitor_id(int net, int family, int version) {
	return family > 0 && family < 64 && version >= 0 && version < 6 && test_id[net & 1][family][version] ? test_id[net & 1][family][version] : -1;
}
int visitor_panel(const Encounter *e, int col, int row) { (void)e; (void)col; (void)row; return test_panel; }

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

/* Whether a teleport's two pads count as joined, as its warp joins them
 * (gaps_bridge's, where the layer's far pad is an island of its own). */
static bool teleport_joined;

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
		for (int k = 0; k < 5; ++k) {
			int nx = k < 4 ? x + d[k][0] : -1, ny = k < 4 ? y + d[k][1] : -1;
			/* (and from one teleport pad to the other) */
			for (int p = 0; k == 4 && teleport_joined && p < 2; ++p)
				if (x == layer.teleport_x[p] && y == layer.teleport_y[p]) { nx = layer.teleport_x[!p]; ny = layer.teleport_y[!p]; }
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || seen[ny][nx] || layer.cell[ny][nx] != C_PATH) continue;
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

/* MegaMan's place to meet the guardian (guardian_stand; docs/BOSSES.md, the
 * sequence): at his height on the screen and turned to him, on the arena's
 * floor under his feet, on the side the bridge comes in from (nearer the
 * antechamber than the place across the guardian), whichever way the
 * arena is entered. */
static void check_stand(uint32_t seed) {
	const Room *a = &layer.rooms[layer.arena], *r = &layer.rooms[layer.ante];
	int spread = super_boss(layer.boss_navi) ? GUARDIAN_STAND_SUPER : GUARDIAN_STAND, sx, sy;
	int face = guardian_stand(layer.arena_dir, 0, 0, spread, &sx, &sy);
	CHECK(sx == sy && (sx > 0 ? face == GUARDIAN_FACE_LEFT : face == GUARDIAN_FACE_RIGHT) && (sx > 0 ? sx : -sx) == spread,
		"seed %u: the place beside the guardian is off his height, or MegaMan is turned away", seed);
	/* (a panel is 32 world units, grid x world +Y and grid y world -X; the
	 * panels under his feet, a third of a panel round the point) */
	double gx = a->ax + sy / 32.0, gy = a->ay - sx / 32.0;
	for (int py = (int)floor(gy + 0.17); py <= (int)floor(gy + 0.83); ++py)
		for (int px = (int)floor(gx + 0.17); px <= (int)floor(gx + 0.83); ++px)
			CHECK(px >= a->x && px < a->x + a->w && py >= a->y && py < a->y + a->h && layer.cell[py][px] == C_PATH,
				"seed %u: the place beside the guardian is off his arena's floor at %d %d", seed, px, py);
	double nx = gx - r->ax, ny = gy - r->ay, fx = 2 * a->ax - gx - r->ax, fy = 2 * a->ay - gy - r->ay;
	CHECK(nx * nx + ny * ny < fx * fx + fy * fy, "seed %u: the place beside the guardian is across him from the bridge", seed);
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
	if (layer.ante >= 0) check_stand(seed);
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
	/* (a teleport island's pad reached by the warp from the other: a way
	 * laid over the void between them, straight then across, had run
	 * through a Mr.Prog by the near pad, and the island counted as cut) */
	teleport_joined = on && layer.teleport_island;
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

static int pieces_crowded, teller_walk_min = 1 << 20, tellers_near, blocks_backwards;

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
		/* (its pocket ahead of it, the floor before it the arrival's: the
		 * walls BN6 lays for a cube or an obstacle hold MegaMan back from
		 * the floor before them alone, and one facing out let him walk in
		 * and never out, issue #111) */
		blocks_backwards += !seen[k->y - dir_dy[k->dir]][k->x - dir_dx[k->dir]] || seen[k->y + dir_dy[k->dir]][k->x + dir_dx[k->dir]];
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
 * led to an empty end); wide where two by two of its panels are floor. A
 * branch whose far end lies on the signature's open floor, a ring road's
 * far side or a plaza's corner seen from the way, has no end (issue #98). */
static void empty_ends(int *branches, int *empty, int *empty_wide) {
	static int16_t far[MAP_W * MAP_H];
	static uint8_t holds[MAP_W * MAP_H], wide[MAP_W * MAP_H], open[MAP_W * MAP_H];
	memset(far, 0, sizeof far); memset(holds, 0, sizeof holds); memset(wide, 0, sizeof wide); memset(open, 0, sizeof open);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			int d = layer_detour(x, y), b = layer_branch(x, y);
			if (d <= 0 || b < 0) continue;
			if (d > far[b]) { far[b] = (int16_t)d; open[b] = layer_detour_open(x, y); }
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
		if (far[b] < 5 || open[b]) continue;
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
	CHECK(blocks_backwards == 0, "%d obstacles and cubes face out of their pockets", blocks_backwards);
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
		h = mix(mix(h, layer.sig), layer.sig_room);
		h = mix(mix(h, layer.arrive_side), layer.arrive_face);
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
	/* BN5's guardians (docs/BOSSES.md, BN5's Navis): the lowest version
	 * whose HP reaches their band, three quarters of BN6's guardians' (no
	 * Cross comes into BN5's engine: 300-450 in act 1), V2 at least from act
	 * 4, and their HP capped at its top, never raised (BN5's own HP: KnightMan
	 * 600/900/1600/1800, ShadowMan 500/700/1200/1600, ToadMan 700/900/1200/
	 * 1500, TomahawkMan 700/900/1400/1800, Colonel 400/1200/1800/2000) */
	const int knight[4] = { 600, 900, 1600, 1800 }, shadow[4] = { 500, 700, 1200, 1600 }, toad[4] = { 700, 900, 1200, 1500 },
		tomahawk[4] = { 700, 900, 1400, 1800 }, colonel[4] = { 400, 1200, 1800, 2000 };
	CHECK(pacing_xguardian_version(knight, 0, 0, false) == 0 && pacing_xguardian_version(knight, 1, 0, false) == 0, "KnightMan V1 in acts 1-2");
	CHECK(pacing_xguardian_version(knight, 1, 0, true) == 1 && pacing_xguardian_hp(900, 1, 0) == 525, "threat 4's V2 from act 2, capped at 525");
	CHECK(pacing_xguardian_version(toad, 0, 0, false) == 0 && pacing_xguardian_hp(700, 0, 0) == 450, "ToadMan V1 in act 1, held to 450");
	CHECK(pacing_xguardian_hp(600, 0, 0) == 450, "KnightMan's 600 held to 450 in act 1 (session 68)");
	CHECK(pacing_xguardian_version(shadow, 1, 0, false) == 0 && pacing_xguardian_version(shadow, 2, 0, false) == 1 &&
		pacing_xguardian_hp(700, 2, 0) == 700, "ShadowMan V1 in act 2, V2 at 700 in act 3");
	CHECK(pacing_xguardian_hp(500, 2, 0) == 500, "never raised to the band's floor");
	CHECK(pacing_xguardian_version(tomahawk, 2, 0, false) == 0 && pacing_xguardian_version(tomahawk, 3, 0, false) == 1 &&
		pacing_xguardian_hp(900, 3, 0) == 900, "TomahawkMan V1 in act 3, V2 at 900 in act 4");
	CHECK(pacing_xguardian_version(colonel, 4, 0, false) == 1, "Colonel V2 in the Undernet's act");
	CHECK(pacing_xguardian_version(colonel, 4, 1, false) == 3 && pacing_xguardian_hp(2000, 4, 1) == 2000, "SP, his own HP, on a later cycle");
	/* heals: the middle layer of each act on the first two cycles, and the
	 * run's first layer */
	CHECK(pacing_heal_certain(2) && pacing_heal_certain(17) && pacing_heal_certain(1) && !pacing_heal_certain(3) &&
		!pacing_heal_certain(19) && !pacing_heal_certain(1 + CYCLE_LAYERS), "a heal on each act's middle layer and the first");
	CHECK(pacing_heal_certain(2 + CYCLE_LAYERS) && !pacing_heal_certain(2 + 2 * CYCLE_LAYERS), "the third cycle drops it");
	/* the Net's clock (docs/HOME.md, going back): a tenth more a notch on a
	 * guardian's HP, the stats rows' top kept */
	CHECK(pacing_clock_hp(400, 0) == 400 && pacing_clock_hp(400, 3) == 520 && pacing_clock_hp(4000, 2) == PACING_HP_MOST &&
		pacing_clock_hp(-1, 2) == -1, "the clock's notches on a guardian's HP");
	/* ... and the acts a trip back goes to: the last two won, newest first,
	 * as their middle layers, never the Nest's */
	int older[PACING_OLDER];
	CHECK(pacing_older_acts(1, older) == 0 && pacing_older_acts(3, older) == 0, "no trip back before an act is won");
	CHECK(pacing_older_acts(4, older) == 1 && older[0] == 2, "after act 1, its middle layer");
	CHECK(pacing_older_acts(10, older) == 2 && older[0] == 8 && older[1] == 5, "before the short net's Nest, acts 3 and 2");
	CHECK(pacing_older_acts(19, older) == 2 && older[0] == 17 && older[1] == 14, "before the endless Nest, the Graveyard and the Undernet");
	CHECK(pacing_older_acts(20, older) == 2 && older[0] == 17 && older[1] == 14, "a new cycle: the last one's acts, not its Nest");
	CHECK(pacing_older_acts(23, older) == 2 && older[0] == 21 && older[1] == 17, "and on into the cycle before");
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
				snprintf(line, sizeof line, "Did you hear?|A copy of %s guards the end of %s!|Word is,%s", guardian(navi)->name,
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
		snprintf(line, sizeof line, "@M We're through to %s,Lan!|@M A strong Navi's signal waits deeper in.|@M ...I don't recognize it.|"
			"@L Then let's go find out who!", area);
		snprintf(what, sizeof what, "the arrival's words in area %d", biome);
		check_talk(what, line);
		snprintf(what, sizeof what, "the briefing in area %d", biome);
		snprintf(line, sizeof line, "@M Layer 19,Lan. We're in %s!|@M Battles here run the older Net's way.|@M A strong Navi's signal waits at "
			"the end...|@M We've got no battle data on it,Lan.|@M Watch the yellow panels! Attacks land there!", area);
		check_talk(what, line);
		/* (where the nets meet, issue #82: an older net's area's own
		 * viruses, the deep's both nets', briefing_words.c) */
		snprintf(line, sizeof line, "@M Layer 19,Lan. We're in %s!|@M Its viruses come from the older Net!|@M A strong Navi's signal waits at "
			"the end...", area);
		check_talk(what, line);
		snprintf(line, sizeof line, "@M Layer 19,Lan. We're in %s!|@M Both Nets' viruses come down here!|@M A strong Navi's signal waits at "
			"the end...", area);
		check_talk(what, line);
		for (int navi = 1; navi <= 18; ++navi) {
			if (navi == 17) continue;
			snprintf(line, sizeof line, "@M Layer 19,Lan. We're in %s!|@M Word is,%s waits at the end!|@M We've got no battle data on him,"
				"Lan.|@M Watch the yellow panels! Attacks land there!", area, guardian(navi)->name);
			check_talk(what, line);
			snprintf(line, sizeof line, "@M Layer 19,Lan. We're in %s!|@M Word is,%s guards this area!", area, guardian(navi)->name);
			check_talk(what, line);
		}
		/* (MegaMan at a port at home, docs/HOME.md: the way's area, its
		 * guardian battled or never, the dark way's and its seal) */
		for (int navi = 0; navi <= 18; ++navi) {
			if (navi == 17) continue;
			snprintf(what, sizeof what, "the port's words to area %d (guardian %d)", biome, navi);
			check_talk(what, home_port_words(biome, navi, false, false));
			check_talk(what, home_port_words(biome, navi, true, false));
		}
		check_talk("the sealed dark way's words", home_port_words(biome, 0, true, true));
	}
}

/* Coming home (docs/HOME.md): MegaMan and Lan's words in Lan's HP after
 * each act of either net, its portals named (every name, the longest ones
 * together), a dark way sealed or open; and what Lan's HP is, the run's
 * first time there. */
static void test_home_words(void) {
	const char *ports[3] = { home_portal_name(0), home_portal_name(2), home_portal_name(4) };
	for (int k = 0; k < 5; ++k) CHECK(home_portal_name(k)[0], "portal %d has a name", k);
	/* (where the nets meet, issue #82, guest_words.c: the first arrival
	 * at an older net's area whose viruses fight our way, for a profile
	 * new to it and one told the old way; the deep's both nets; the first
	 * visitor of either net, named by the longest of their families') */
	check_talk("an older net's area, first", "@M Wait... This Net's a copy of an older one!|@L The Nest copied that too!?|"
		"@M Its viruses are the old ones,Lan.|@M But we fight them our way. Cross and all!|@M Its own Navis fight the old way,though.|"
		"@L No Crosses against them? Got it!");
	check_talk("an older net's area, told the old way", "@M Lan! The older Net's viruses...|@M We fight them our way now. Cross and all!|"
		"@M Its own Navis still fight the old way.|@L Got it. No Crosses against them!");
	check_talk("the deep's both nets", "@M Lan... The two Nets run together down here.|@L The old viruses and ours? Side by side?|"
		"@M Yeah. Anything could come at us!");
	check_talk("a visitor of the older net's", "@M Lan,that Cactikil back there...|@M It came from the older Net!|@L The two Nets are mixing!?|"
		"@M Where they meet,yeah. Stay sharp!");
	check_talk("a visitor of ours", "@M Huh? That HntdCndl was one of ours,Lan!|@L In the older Net!?|@M The Nets meet here. Stay sharp!");
	check_talk("the older net's guardian's battle", "@M Lan,he fought on the older Net's system!|@M Its own Navis fight the old way.|"
		"@M Our chips worked the way it knew them.");
	check_talk("Lan's HP's first time", home_hp_words(false));
	check_talk("Lan's HP, a profile told", home_hp_words(true));
	check_talk("the errands at home", home_errands_words("@M Home's to the upper right,Lan!|@M The PC's up in your room!", true, true, "to the lower left"));
	check_talk("AsterLand's counter", home_aster_words("@M The way out's to the lower right,Lan!|@M Your PC's back home,remember?"));
	check_talk("the house's front door", home_door_words("@M Your room's to the upper left,Lan!|@M The PC's up there!", "to the lower left"));
	/* (going back, docs/HOME.md: the older portals, the way home, L there) */
	for (int clock = 0; clock <= 6; ++clock) {
		char what[80];
		snprintf(what, sizeof what, "home again at clock %d", clock);
		check_talk(what, home_back_words(clock));
		for (int first = 0; first <= 1; ++first) check_talk(what, home_hp_status("to the upper right", first, clock));
		for (int b = 0; b < BIOME_COUNT; ++b)
			for (int taught = 0; taught <= 1; ++taught) check_talk(what, home_back_portal_words(b, taught, clock));
	}
	for (int mode = RUN_SHORT; mode <= RUN_ENDLESS; ++mode)
		for (int depth = 4; depth <= 20; depth += 3) {
			run.mode = (uint8_t)mode;
			run.depth = depth;
			for (int ways = 1; ways <= 3; ++ways) {
				char what[80];
				snprintf(what, sizeof what, "home's words at layer %d (net %d, %d ways)", depth, mode, ways);
				check_talk(what, home_words("TomahawkMan", ways, ports, ways == 2 ? ports[2] : NULL));
				check_talk(what, home_words(NULL, ways, ports, NULL));
			}
		}
	run.depth = 1;
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

/* Where MegaMan at (x, y) stands at the mouth of the lane the walk enters
 * from his panel or from its next one (floor two panels wide or less
 * crosswise, a walkway or band), stopped: outside its ends (12.5 units
 * past its end panels' middles) and against the edge, where the lane's
 * way does not move him (BN6 stops him there). The mouth panel (*mx, *my),
 * the lane's first (*wx, *wy), its ends crosswise from the mouth's middle
 * (*lo, *hi) and whether it runs along x. As of the last route_way; false
 * for none. */
static bool lane_off(double x, double y, int *mx, int *my, int *wx, int *wy, double *lo, double *hi, bool *along_x) {
	int n = route_walk_len, cx = (int)lround(x), cy = (int)lround(y);
	for (int k = 1; k <= 2 && k <= n; ++k) {
		int ax = k == 1 ? cx : route_walk[n - 1] % MAP_W, ay = k == 1 ? cy : route_walk[n - 1] / MAP_W;
		int bx = route_walk[n - k] % MAP_W, by = route_walk[n - k] / MAP_W, ux = by != ay, uy = bx != ax, a = 0, b = 0;
		while (walk_floor(ax - (a + 1) * ux, ay - (a + 1) * uy) && walk_floor(bx - (a + 1) * ux, by - (a + 1) * uy)) ++a;
		while (walk_floor(ax + (b + 1) * ux, ay + (b + 1) * uy) && walk_floor(bx + (b + 1) * ux, by + (b + 1) * uy)) ++b;
		double off = bx != ax ? y - ay : x - ax, l = -a - 12.5 / 32, h = b + 12.5 / 32;
		if (abs(bx - ax) + abs(by - ay) != 1 || a + b + 1 > 2 || (off > l && off < h)) continue;
		/* (and stopped there: the lane's way held moves him less than a
		 * few units) */
		double sx = x, sy = y;
		for (int f = 0; f < 8; ++f) walk_step(&sx, &sy, (bx - ax) / 32.0, (by - ay) / 32.0, -1, -1);
		if (fabs(sx - x) + fabs(sy - y) > 4.0 / 32) continue;
		*mx = ax; *my = ay; *wx = bx; *wy = by; *lo = l; *hi = h; *along_x = bx != ax;
		return true;
	}
	return false;
}

/* MegaMan standing off a lane's mouth (lane_off): whether the arrow's way
 * `way`, held, walks him into the lane, or leaves him standing on its line
 * (its way then walks him in), not past it or short of it. */
static bool held_onto_lane(double x, double y, int way, int tx, int ty) {
	int mx, my, wx, wy;
	double lo, hi, dx, dy;
	bool along_x;
	if (!lane_off(x, y, &mx, &my, &wx, &wy, &lo, &hi, &along_x)) return true;
	way_step(way, 2.0 / 32, &dx, &dy);
	/* (in: past the mouth, on either panel of a lane two wide; on its line:
	 * inside its ends, which are the floor's edge too, where he stops) */
	double off = along_x ? y - my : x - mx;
	for (int f = 0; f < 48; ++f) {
		bool on = off >= lo - 1e-9 && off <= hi + 1e-9;
		if (on && (along_x ? lround(x) == wx : lround(y) == wy)) return true;
		if (!walk_step(&x, &y, dx, dy, tx, ty)) return on;
		off = along_x ? y - my : x - mx;
	}
	return false;
}

/* frames to get beside (tx, ty) from (x, y) following the arrow, -1 never */
static int arrow_turns, arrow_frames, arrow_swings;
/* (and how it walks: frames MegaMan, holding the way it shows, does not
 * move, and the stops they make, each a new direction to find; of its
 * looks, those where he stands on a walkway or band, floor two panels wide
 * or less across the walk, and of those the ones whose way is across the
 * grid, one of the screen's straight four, against the screen's diagonals
 * the walkways run on: the way it turns to, as the slide off a lane's
 * mouth, straight, shows on a look or two into the lane) */
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
				arrow_band_diagonals += w % 2 == 0;
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

/* a walk's turns as MegaMan holds its ways, from panel (sx, sy) on along
 * route_walk (its target first): each change of way after the first step */
static int walk_turns(int sx, int sy) {
	int turns = 0, last = -1, cx = sx, cy = sy;
	for (int i = route_walk_len - 1; i >= 0; --i) {
		int x = route_walk[i] % MAP_W, y = route_walk[i] / MAP_W, way = (x - cx) * 3 + (y - cy);
		turns += last >= 0 && way != last;
		last = way;
		cx = x; cy = y;
	}
	return turns;
}

/* The map's kept walk (route_walk_kept, the second screen's way on, issue
 * #81) against the arrow's own search, from the arrival and each room to
 * the exit or the guardian: as short, as few turns, each step to a panel
 * beside the last, the target at its end. */
static void test_kept_walk(void) {
	int walks = 0, off = 0;
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	for (uint32_t seed = 1; seed <= 120; ++seed) {
		int depth = 1 + (int)(seed % 25);
		int kind = seed % 7 == 0 ? LAYER_UNDERNET : seed % 11 == 0 ? LAYER_SECRET : LAYER_NORMAL;
		layer_generate(seed * 7919u, depth, biome_for_depth(depth), kind, &kit);
		int tx, ty, sx, sy, len;
		if (!arrow_goal(&tx, &ty)) continue;
		route_gone = 0;
		for (int r = -1; r < layer.nrooms; ++r) {
			if (!arrow_start(r, tx, ty, &sx, &sy)) continue;
			if (route_way(sx, sy, tx, ty, &len) < 0) continue;
			int n = route_walk_len, turns = walk_turns(sx, sy), from = n ? route_walk[n - 1] : -1;
			bool ok = route_walk_kept(sx, sy, tx, ty, seed, &len) == 0 && route_walk_len == n && walk_turns(sx, sy) == turns;
			for (int i = 0; ok && i + 1 < route_walk_len; ++i)
				ok = abs(route_walk[i] % MAP_W - route_walk[i + 1] % MAP_W) + abs(route_walk[i] / MAP_W - route_walk[i + 1] / MAP_W) == 1;
			ok = ok && (!n || (route_walk[0] == ty * MAP_W + tx && abs(route_walk[n - 1] % MAP_W - from % MAP_W) + abs(route_walk[n - 1] / MAP_W - from / MAP_W) <= 2));
			++walks;
			if (!ok && ++off <= 5) printf("  seed %u (depth %d): the kept walk from %d,%d to %d,%d is not the arrow's match\n", seed, depth, sx, sy, tx, ty);
		}
	}
	CHECK(off == 0, "the map's kept walk differed from the arrow's search on %d of %d walks", off, walks);
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

/* (not where something solid stands by the mouth or in his panel: the
 * arrow leads him out of a Mystery Data's panel first, and BN6 takes him
 * round its circle, which walk_step does not) */
static bool mouth_clear(int mx, int my, int ax, int ay, double x, double y) {
	for (int i = 1; i < layer.nobj; ++i) {
		int ox = (int)layer.obj[i].x, oy = (int)layer.obj[i].y;
		if (layer.obj[i].solid && ((abs(ox - mx) <= 1 && abs(oy - my) <= 1) || (abs(ox - mx - ax) <= 1 && abs(oy - my - ay) <= 1) ||
			(ox == (int)lround(x) && oy == (int)lround(y)))) return false;
	}
	return true;
}

/* MegaMan at offset o crosswise from the mouth (mx, my) of a lane the walk
 * enters along (ax, ay), walked along it to the edge and stopped there: 0
 * where he is not on the floor or not off the lane (lane_off), 2 where he
 * stands on a walkway or band, else 1 where the arrow's way (*way), held,
 * walks him onto the lane, -1 where not. */
static int mouth_spot(int mx, int my, int ax, int ay, double o, int tx, int ty, int *way) {
	double x = mx + (ay ? o : 0), y = my + (ax ? o : 0);
	if (!walk_on(x, y) || walk_blocked(x, y, tx, ty)) return 0;
	for (int k = 0; k < 32 && walk_on(x + ax / 32.0, y + ay / 32.0) && !walk_blocked(x + ax / 32.0, y + ay / 32.0, tx, ty); ++k) {
		x += ax / 32.0;
		y += ay / 32.0;
	}
	int len, lx, ly, wx, wy;
	double lo, hi;
	bool along_x;
	*way = route_way(x, y, tx, ty, &len);
	if (*way < 0 || !mouth_clear(mx, my, ax, ay, x, y) || !lane_off(x, y, &lx, &ly, &wx, &wy, &lo, &hi, &along_x)) return 0;
	int cx = (int)lround(x), cy = (int)lround(y), fx = route_walk[route_walk_len - 1] % MAP_W - cx, fy = route_walk[route_walk_len - 1] / MAP_W - cy;
	if (abs(fx) + abs(fy) == 1 && floor_across(cx, cy, fx, fy) <= 2) return 2;
	return held_onto_lane(x, y, *way, tx, ty) ? 1 : -1;
}

/* MegaMan stopped at the mouth of each walkway or band the walk from the
 * arrival and each room enters (two panels wide or less crosswise; sixty
 * layers of every area, as test_arrow_areas'), against the edge beside it
 * and off its lane, every two units out to a panel and a half on either
 * side: off a walkway or band himself, the arrow's way, held, walks him
 * onto the lane (held_onto_lane). Two playtesters stood still at such
 * mouths for about 35 calls (session 65): the arrow's way across, held
 * short, moved him a few units with the arrow unchanged, and held on,
 * carried him over the lane where the floor went on past it. On a walkway
 * or band the arrow shows its way (test_arrow_areas); those spots are
 * counted apart. */
static void test_arrow_mouths(void) {
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	static int16_t walk[MAP_W * MAP_H];
	static uint8_t done[MAP_H][MAP_W][4];
	int spots = 0, misses = 0, bands = 0;
	for (int b = 0; b < BIOME_COUNT; ++b) {
		int area_spots = spots, area_misses = misses;
		for (uint32_t seed = 1; seed <= 60; ++seed) {
			layer_generate(seed * 104729u + (uint32_t)b, seed % 2 ? 6 : 5, b, LAYER_NORMAL, &kit);
			int tx, ty, sx, sy, len;
			if (!arrow_goal(&tx, &ty)) continue;
			memset(done, 0, sizeof done);
			for (int r = -1; r < layer.nrooms; ++r) {
				if (!arrow_start(r, tx, ty, &sx, &sy) || route_way(sx, sy, tx, ty, &len) < 0) continue;
				int n = route_walk_len;
				memcpy(walk, route_walk, sizeof walk[0] * (size_t)n);
				/* (each step from walk[i], the mouth, into walk[i - 1], the lane's first panel) */
				for (int i = n - 1; i >= 1; --i) {
					int mx = walk[i] % MAP_W, my = walk[i] / MAP_W, ax = walk[i - 1] % MAP_W - mx, ay = walk[i - 1] / MAP_W - my;
					int d = ax > 0 ? 0 : ax < 0 ? 1 : ay > 0 ? 2 : 3, ux = ay != 0, uy = ax != 0, lo = 0, hi = 0;
					if (done[my][mx][d]) continue;
					done[my][mx][d] = 1;
					while (walk_floor(mx - (lo + 1) * ux, my - (lo + 1) * uy) && walk_floor(mx + ax - (lo + 1) * ux, my + ay - (lo + 1) * uy)) ++lo;
					while (walk_floor(mx + (hi + 1) * ux, my + (hi + 1) * uy) && walk_floor(mx + ax + (hi + 1) * ux, my + ay + (hi + 1) * uy)) ++hi;
					if (lo + hi + 1 > 2) continue;
					/* (whole units off the end panels' middles, as BN6 places him) */
					for (int s = -1; s <= 1; s += 2)
						for (int k = 13; k <= 60; k += 2) {
							double o = s < 0 ? -lo - k / 32.0 : hi + k / 32.0;
							int way, got = mouth_spot(mx, my, ax, ay, o, tx, ty, &way);
							spots += got == 1 || got == -1;
							bands += got == 2;
							if (got >= 0) continue;
							if (++misses <= 5)
								printf("  area %d seed %u: stopped at %.3f,%.3f off the mouth %d,%d, the arrow's way %d held did not walk MegaMan onto its lane\n", b,
									seed, mx + (ay ? o : 0), my + (ax ? o : 0), mx, my, way);
						}
				}
			}
		}
		if (getenv("CW_WAY_STATS")) printf("  mouths area %2d: %d of %d spots stopped off a lane, the arrow's way held not onto it\n", b, misses - area_misses, spots - area_spots);
	}
	CHECK(misses == 0, "stopped off a lane's mouth, the arrow's way held did not walk MegaMan onto the lane at %d of %d spots", misses, spots);
	printf("  mouths: %d spots stopped off a lane's mouth (and %d on a walkway or band), %d where the arrow's way held did not walk MegaMan onto it\n",
		spots, bands, misses);
}

/* eighths apart of two of the pad's ways (0-4) */
static int ways_apart(int a, int b) {
	int d = abs(a - b) % 8;
	return d > 4 ? 8 - d : d;
}

/* The guardian's Guardian Data taken, the exit pad shows behind where he
 * stood (sixty guardian layers of every area): from beside his panel,
 * where MegaMan stands having taken it, and where a straight line over
 * the floor reaches the pad, the arrow's way keeps within an eighth of
 * the line to it, so L does not say the way winds, once the guardian is
 * gone from the floor (route_gone). The walk went round his empty panel:
 * L said "The exit lies down and to the left, but the way winds", and the
 * arrow pointed up, the pad three panels down on the arena's floor
 * (session 65). */
static void test_guardian_gone(void) {
	memset(&run, 0, sizeof run);
	for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = 12;
	for (int i = 0; i < 6; ++i) run.biome_order[i] = (uint8_t)i;
	static const int d8[8][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }, { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 } };
	int spots = 0, winds = 0, wound = 0;   /* (wound: as if he still stood there) */
	for (int b = 0; b < BIOME_COUNT; ++b)
		for (uint32_t seed = 1; seed <= 60; ++seed) {
			layer_generate(seed * 104729u + (uint32_t)b, 6, b, LAYER_NORMAL, &kit);
			int gi = -1, ex = -1, ey = -1;
			for (int i = 0; i < layer.nobj; ++i) {
				if (layer.obj[i].type == OBJ_BOSS) gi = i;
				if (layer.obj[i].type == OBJ_EXIT || layer.obj[i].type == OBJ_RETURN) { ex = (int)layer.obj[i].x; ey = (int)layer.obj[i].y; }
			}
			if (gi < 0 || ex < 0) continue;
			int gx = (int)layer.obj[gi].x, gy = (int)layer.obj[gi].y;
			for (int k = 0; k < 8; ++k) {
				double x = gx + 0.6 * d8[k][0], y = gy + 0.6 * d8[k][1];
				int len, cx = (int)lround(x), cy = (int)lround(y), lie = route_grid_way(ex - x, ey - y);
				if (!walk_on(x, y) || walk_blocked(x, y, gx, gy) || abs(cx - ex) + abs(cy - ey) <= 1) continue;
				route_gone = 0;
				int stood = route_way(x, y, ex, ey, &len);
				route_gone = 1ull << gi;
				int gone = route_way(x, y, ex, ey, &len);
				bool line = gone >= 0 && route_floor_line(cx, cy, ex, ey);
				route_gone = 0;
				if (stood < 0 || !line) continue;
				++spots;
				wound += ways_apart(stood, lie) >= 2;
				if (ways_apart(gone, lie) >= 2 && ++winds <= 5)
					printf("  area %d seed %u: at %.1f,%.1f beside the guardian's panel, the arrow's way %d against the line %d to the exit pad\n", b, seed, x, y,
						gone, lie);
			}
		}
	CHECK(winds == 0, "the guardian gone, the way to the exit pad winds from beside his panel at %d of %d spots", winds, spots);
	printf("  guardian gone: %d spots beside his panel in a straight line to the exit pad, the way winding at %d (him standing: %d)\n", spots, winds, wound);
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
	CHECK(strstr(w, "has bugs!") && strstr(w, "A bad Buster bug") && strstr(w, "Five colors"), "several bugs: %s", w);
	CHECK(strstr(w, "command line") != NULL, "a placement bug says where to look: %s", w);
	/* (the cause where the board shows it, in place of the rules) */
	w = navicust_bug_words(bugs, false, "HP+100 is a plus part on the command line! Those go anywhere else.");
	CHECK(strstr(w, "HP+100 is a plus part") && !strstr(w, "Bugs come from"), "a placement bug names its cause: %s", w);
	memset(bugs, 0, sizeof bugs);
	bugs[11] = 1;
	CHECK(!strstr(navicust_bug_words(bugs, false, NULL), "command line"), "a colours' bug alone names itself");
	CHECK(strstr(navicust_bug_words(bugs, true, NULL), "the RUN says OK...|@M But our NaviCust") != NULL, "after a RUN, its OK answered: %s", navicust_bug_words(bugs, true, NULL));
	check_talk("navicust_bug_words", navicust_bug_words(bugs, true, NULL));
	/* which programs turn: those whose colour's Spin is held */
	navicust_set_spins(0);
	/* (none held: nothing said, BN6's own rule unexplained to its players) */
	CHECK(!*navicust_turn_words(0), "no Spin, no word: %s", navicust_turn_words(0));
	navicust_set_spins(1u << 2);
	CHECK(strstr(navicust_turn_words(0), "turn pink programs") && strstr(navicust_turn_words(0), "That's the Spin"), "the pink Spin: %s", navicust_turn_words(0));
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

/* An area's looks as its maps give them (src/core/rom.c): a layer's
 * landmark is the set its area's looks name. */
static LayerKit area_kit(int b) {
	LayerKit k = kit;
	static const unsigned looks[BIOME_COUNT] = {
		[BIOME_CENTRAL] = 1u << LOOK_TREE | 1u << LOOK_SIGN, [BIOME_SEASIDE] = 1u << LOOK_BBS | 1u << LOOK_SIGN,
		[BIOME_SKY] = 1u << LOOK_TREE | 1u << LOOK_SIGN, [BIOME_GREEN] = 1u << LOOK_TREE | 1u << LOOK_GIANT_TREE | 1u << LOOK_SIGN,
		[BIOME_GRAVEYARD] = 1u << LOOK_MONUMENT | 1u << LOOK_GRAVE, [BIOME_UNDERNET] = 1u << LOOK_STATUE | 1u << LOOK_BRAZIER,
		[BIOME_SECRET] = 1u << LOOK_STATUE | 1u << LOOK_BRAZIER,
	};
	k.looks = looks[b];
	return k;
}

/* Whether a sprite prop stands in room r's box or two panels round it. */
static bool room_has_prop(int r) {
	const Room *m = &layer.rooms[r];
	for (int i = 0; i < layer.nprops; ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind == PROP_SPRITE && p->x >= m->x - 2 && p->x < m->x + m->w + 2 && p->y >= m->y - 2 && p->y < m->y + m->h + 2) return true;
	}
	return false;
}

/* The floor in room r's box, and whether the way runs through it. */
static int room_floor(int r, bool *way) {
	const Room *m = &layer.rooms[r];
	int n = 0;
	*way = false;
	for (int y = m->y; y < m->y + m->h; ++y)
		for (int x = m->x; x < m->x + m->w; ++x) {
			n += layer.cell[y][x] != C_VOID;
			*way |= layer_on_way(x, y);
		}
	return n;
}

/* Identity (docs/LEVEL_DESIGN.md, issue #98): each of a run's layers is
 * built round its signature, the room with the most floor, the way through
 * it; never two layers in a row in one layout and one signature both, where
 * the area has another of either, nor an act's first and third where it
 * has three such pairs, and an act opening in another layout than the act
 * before closed in, where its area has two; the area's
 * landmark at the signature that holds it (the giant tree at a grove, the
 * statue at a court, the monument, Central's and Sky's avenues). */
static void test_identity(void) {
	Run kept = run;
	int layers = 0, sigs = 0, through = 0, most = 0, twins = 0, bounds = 0, same_open = 0, holders = 0, held = 0, acts = 0, act_twins = 0;
	for (uint32_t r = 1; r <= 40; ++r) {
		memset(&run, 0, sizeof run);
		run.seed = r * 2654435761u + 99u;
		for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = (uint8_t)(1 + b % 5);
		/* (every area in turn, six to a run) */
		for (int a = 0; a < 6; ++a) run.biome_order[a] = (uint8_t)((r * 6 + (uint32_t)a) % BIOME_COUNT);
		int prev_layout = -1, prev_sig = -1, open_layout = -1, open_sig = -1;
		for (int d = 1; d <= 18; ++d) {
			int b = biome_for_depth(d);
			LayerKit k = area_kit(b);
			layer_generate(run.seed ^ (uint32_t)(d * 2654435761u), d, b, LAYER_NORMAL, &k);
			++layers;
			int layouts = 0;
			for (int l = 0; l < LAYOUT_COUNT; ++l) layouts += layout_weight(b, l) > 0;
			if (layer.sig_room >= 0) {
				++sigs;
				bool way, other_way;
				int floor = room_floor(layer.sig_room, &way), top = 0;
				for (int i = 0; i < layer.nrooms; ++i)
					if (i != layer.sig_room) { int f = room_floor(i, &other_way); top = f > top ? f : top; }
				through += way;
				most += floor >= top;
				bool takes = b == BIOME_CENTRAL || b == BIOME_SKY || b == BIOME_GRAVEYARD || (b == BIOME_GREEN && layer.sig == SIG_GROVE) ||
					((b == BIOME_UNDERNET || b == BIOME_SECRET) && layer.sig == SIG_COURT);
				holders += takes;
				held += takes && room_has_prop(layer.sig_room);
			}
			twins += layer.layout == prev_layout && layer.sig == prev_sig && (layouts > 1 || sig_pool_size(b) > 1);
			if (layer_in_act(d) == 0 && d > 1 && layouts > 1) { ++bounds; same_open += layer.layout == prev_layout; }
			if (layer_in_act(d) == 0) { open_layout = layer.layout; open_sig = layer.sig; }
			if (layer_in_act(d) == 2 && layouts * sig_pool_size(b) >= 3) { ++acts; act_twins += layer.layout == open_layout && layer.sig == open_sig; }
			prev_layout = layer.layout;
			prev_sig = layer.sig;
		}
	}
	CHECK(sigs * 100 >= layers * 97, "a signature on %d of %d layers", sigs, layers);
	CHECK(through * 100 >= sigs * 90, "the way through the signature on %d of %d layers", through, sigs);
	CHECK(most * 100 >= sigs * 95, "the signature the most floor of the layer's rooms on %d of %d", most, sigs);
	CHECK(twins * 100 <= layers, "%d of %d layers in the layout and signature of the one before", twins, layers);
	CHECK(act_twins * 20 <= acts, "%d of %d acts close in their first layer's layout and signature", act_twins, acts);
	CHECK(same_open * 20 <= bounds, "%d of %d acts open in the layout the act before closed in", same_open, bounds);
	CHECK(held * 100 >= holders * 85, "the landmark at the signature on %d of %d layers", held, holders);
	printf("  identity: a signature on %d of %d layers, the way through it on %d, the most floor on %d; %d twins in a row, %d of %d acts closing as they opened; "
		"%d of %d acts open as the last closed; the landmark at %d of %d\n", sigs, layers, through, most, twins, act_twins, acts, same_open, bounds, held, holders);
	run = kept;
}

/* Which side of the layer's floor on the screen MegaMan arrives at (SIDE_*,
 * SIDE_COUNT for the middle: none within a quarter of the box), and whether
 * the way he faces points into the layer, towards its middle (a unit of
 * x - y 16 pixels across, of x + y 8 down), as src/dev/navstudy.c classes
 * BN6's own maps' arrivals. */
static int arrival_side(bool *into) {
	static const int fu[8] = { 0, 1, 2, 1, 0, -1, -2, -1 }, fv[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };
	double u0 = 1e9, u1 = -1e9, v0 = 1e9, v1 = -1e9;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (layer.cell[y][x] != C_VOID) {
				u0 = fmin(u0, x - y); u1 = fmax(u1, x - y);
				v0 = fmin(v0, x + y); v1 = fmax(v1, x + y);
			}
	double u = layer.obj[0].x - layer.obj[0].y, v = layer.obj[0].x + layer.obj[0].y;
	double d[4] = { (v - v0) / (v1 - v0), (v1 - v) / (v1 - v0), (u - u0) / (u1 - u0), (u1 - u) / (u1 - u0) };
	int side = 0, f = layer.arrive_face & 7;
	for (int k = 1; k < 4; ++k) if (d[k] < d[side]) side = k;
	*into = 4 * fu[f] * ((u0 + u1) / 2 - u) + fv[f] * ((v0 + v1) / 2 - v) > 0;
	return d[side] > 0.25 ? SIDE_COUNT : side;
}

/* Arrivals (docs/LEVEL_DESIGN.md, Arrivals, issue #106): over the runs of
 * test_identity, MegaMan arrives at every side of the screen (BN6's own maps
 * at each about as often), facing along one of the world's axes into the
 * layer; an act's three layers at three sides, and the act after it not
 * opening at the side it closed at; and so no act repeats a layer, its
 * layout, signature and side, though its area has one layout and one
 * signature (the Aquarium's, Mr. Weather's). */
static void test_arrivals(void) {
	Run kept = run;
	int layers = 0, sides[SIDE_COUNT + 1] = { 0 }, axes = 0, into_n = 0, act_repeats = 0, bound_repeats = 0, acts = 0, act_twins = 0;
	for (uint32_t r = 1; r <= 40; ++r) {
		memset(&run, 0, sizeof run);
		run.seed = r * 2654435761u + 99u;
		for (int b = 0; b < BIOME_COUNT; ++b) run.boss_order[b] = (uint8_t)(1 + b % 5);
		for (int a = 0; a < 6; ++a) run.biome_order[a] = (uint8_t)((r * 6 + (uint32_t)a) % BIOME_COUNT);
		int act[3][3] = { { 0 } }, prev_side = -1;
		for (int d = 1; d <= 18; ++d) {
			int b = biome_for_depth(d), k = layer_in_act(d);
			LayerKit kt = area_kit(b);
			layer_generate(run.seed ^ (uint32_t)(d * 2654435761u), d, b, LAYER_NORMAL, &kt);
			++layers;
			bool into;
			++sides[arrival_side(&into)];
			into_n += into;
			axes += layer.arrive_face % 2 == 1;
			if (k == 0 && d > 1) bound_repeats += layer.arrive_side == prev_side;
			act[k][0] = layer.layout; act[k][1] = layer.sig; act[k][2] = layer.arrive_side;
			for (int j = 0; j < k; ++j) {
				act_repeats += act[j][2] == act[k][2];
				act_twins += act[j][0] == act[k][0] && act[j][1] == act[k][1] && act[j][2] == act[k][2];
			}
			acts += k == 2;
			prev_side = layer.arrive_side;
		}
	}
	for (int s = 0; s < SIDE_COUNT; ++s) CHECK(sides[s] * 100 >= layers * 12, "arrivals at side %d on %d of %d layers", s, sides[s], layers);
	CHECK(sides[SIDE_TOP] * 100 <= layers * 45, "arrivals at the top on %d of %d layers", sides[SIDE_TOP], layers);
	CHECK(axes == layers, "MegaMan facing along a world axis on %d of %d layers", axes, layers);
	CHECK(into_n * 100 >= layers * 90, "MegaMan facing into the layer on %d of %d", into_n, layers);
	CHECK(act_repeats == 0, "%d of an act's layers arrive at a side another of its layers did", act_repeats);
	CHECK(bound_repeats == 0, "%d acts open at the side the act before closed at", bound_repeats);
	CHECK(act_twins == 0, "%d of %d acts repeat a layer: its layout, signature and side", act_twins, acts);
	printf("  arrivals: top %d, bottom %d, left %d, right %d, the middle %d of %d layers; facing into the layer on %d; %d acts repeating a layer\n",
		sides[SIDE_TOP], sides[SIDE_BOTTOM], sides[SIDE_LEFT], sides[SIDE_RIGHT], sides[SIDE_COUNT], layers, into_n, act_twins);
	run = kept;
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
	float fill = touch_upright_fill(sw, sh, dp, scale), k = fill > 0 ? fill : (float)scale;
	int w = fill > 0 ? (int)(sw / fill) : sw / scale, h = fill > 0 ? (int)(sh / fill) : sh / scale;
	if (w < 240) w = 240;
	float ox = (sw - w * k) / 2, oy = (sh - h * k) / 2;
	int cx = (w - 240) / 2, cy = (h - 160) / 2, top = touch_picture_top(sw, sh, dp, k);
	int y = (int)((top - oy) / k + 0.999f);
	if (top >= 0 && y + 160 <= h) cy = y;
	TouchScreen t = { sw, sh, dp, (int)(ox + cx * k), (int)(oy + cy * k), (int)(240 * k), (int)(160 * k) };
	*s = t;
}

/* Upright, the picture fills a phone's width (the gap at its sides, in
 * pixels, `gap` at most), and the controls keep their size under it */
static void check_upright_fill(const char *what, int sw, int sh, float dpi, int gap) {
	TouchScreen s;
	touch_screen_for(sw, sh, dpi, &s);
	CHECK(sw - s.pw <= gap, "%s: the picture is %d wide of %d", what, s.pw, sw);
	int sx = sw / 240, sy = sh / 160, scale = touch_fit_scale(sw, sh, dpi / 160, sx < sy ? sx : sy);
	TouchScreen whole = s;
	whole.py = (int)(6.5f * 6.3f * dpi / 160);
	whole.pw = 240 * scale;
	whole.ph = 160 * scale;
	TouchPrefs p;
	touch_prefs_default(&p);
	TouchLayout a, b;
	touch_layout_for(&s, &p, &a);
	touch_layout_for(&whole, &p, &b);
	CHECK(a.box[TOUCH_DPAD].w >= b.box[TOUCH_DPAD].w - 0.5f, "%s: filling the width shrank the D-pad from %.0f to %.0f", what,
		b.box[TOUCH_DPAD].w, a.box[TOUCH_DPAD].w);
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
	check_upright_fill("a phone upright (1080x2400)", 1080, 2400, 420, 0);
	check_upright_fill("an iPhone upright (1179x2556)", 1179, 2556, 460, 0);
	check_upright_fill("a phone upright (1344x2992)", 1344, 2992, 480, 0);
	check_upright_fill("a phone upright (1440x3200, 6x)", 1440, 3200, 560, 0);
	check_upright_fill("a phone upright (1220x2712, 5x of 5.08)", 1220, 2712, 440, 20);
	check_upright_fill("a 4:3 tablet upright (1536x2048)", 1536, 2048, 320, 0);
	check_upright_fill("a foldable open (1812x2176, 7x)", 1812, 2176, 420, 132);
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

/* An SDL mapping's fields (after its GUID and name) each an SDL name with
 * a joystick input: "a:b1", "lefty:a0~", "+leftx:h0.2" */
static bool mapping_fields_ok(const char *m, char *bad, size_t n) {
	const char *f = strchr(m, ',');
	for (f = f ? f + 1 : NULL; f && *f; ) {
		const char *end = strchr(f, ','), *colon = strchr(f, ':');
		if (!end || !colon || colon > end) { snprintf(bad, n, "%s", f); return false; }
		char key[24], val[16];
		snprintf(key, sizeof key, "%.*s", (int)(colon - f), f);
		snprintf(val, sizeof val, "%.*s", (int)(end - colon - 1), colon + 1);
		static const char *const axes[] = { "leftx", "lefty", "rightx", "righty", "lefttrigger", "righttrigger" };
		bool known = padmap_input(key) != PAD_NONE;
		for (size_t i = 0; i < sizeof axes / sizeof *axes; ++i) known |= !strcmp(key, axes[i]);
		if (!known || (val[0] != 'b' && val[0] != 'a' && val[0] != 'h')) { snprintf(bad, n, "%s:%s", key, val); return false; }
		f = end + 1;
	}
	return true;
}

/* The controllers' map (issue #37): the defaults, SDL's names both ways,
 * a binding's swap, the presets, pad.ini written and read back (and its
 * mistakes said), the labels, Android's Joy-Con mappings. */
static void test_padmap(void) {
	PadMap m, back;
	padmap_default(&m);
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_A) == BTN_A && padmap_bits(&m, PAD_FAMILY_NINTENDO, PAD_B) == BTN_B, "a and b are not A and B");
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_AXIS(1, false)) == BTN_UP && padmap_bits(&m, PAD_FAMILY_XBOX, PAD_DPUP) == BTN_UP, "the stick and the D-pad do not both go up");
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_LEFTTRIGGER) == BTN_L && padmap_bits(&m, PAD_FAMILY_XBOX, PAD_RIGHTSHOULDER) == BTN_R, "a trigger or shoulder is not L or R");
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_BACK) == BTN_SELECT && padmap_bits(&m, PAD_FAMILY_XBOX, PAD_START) == BTN_START, "back and start are not SELECT and START");
	CHECK(!padmap_bits(&m, PAD_FAMILY_XBOX, PAD_X) && !padmap_bits(&m, PAD_FAMILY_XBOX, PAD_GUIDE), "x or guide does something by default");
	/* SDL's names, both ways, either case; a trigger has no other way */
	for (int i = 0; i < PAD_INPUTS; ++i)
		if (padmap_name(i)) CHECK(padmap_input(padmap_name(i)) == i, "%s does not name input %d", padmap_name(i), i);
	CHECK(padmap_input("LeftShoulder") == PAD_LEFTSHOULDER && padmap_input("+lefttrigger") == PAD_LEFTTRIGGER, "a name in capitals or a + on a trigger is refused");
	CHECK(padmap_input("-lefttrigger") == PAD_NONE && padmap_input("start2") == PAD_NONE && padmap_input("") == PAD_NONE, "a name SDL has not is taken");
	CHECK(padmap_gba("select") == 9 && padmap_gba("UP") == 0 && padmap_gba("Z") < 0, "the GBA's buttons are misnamed");
	/* B on x: B gives up b, nobody else changes; then A on x: they swap */
	padmap_bind(&m, PAD_FAMILY_XBOX, 5, PAD_X);
	CHECK(m.in[PAD_FAMILY_XBOX][5][0] == PAD_X && m.in[PAD_FAMILY_XBOX][5][1] == PAD_NONE && !padmap_bits(&m, PAD_FAMILY_XBOX, PAD_B), "B on x kept b");
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_A) == BTN_A && padmap_bits(&m, PAD_FAMILY_NINTENDO, PAD_B) == BTN_B, "binding one family's B moved another's");
	padmap_bind(&m, PAD_FAMILY_XBOX, 4, PAD_X);
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_X) == BTN_A && padmap_bits(&m, PAD_FAMILY_XBOX, PAD_A) == BTN_B, "A taking B's x left B without A's a");
	/* a raw joystick's (issue #112): its names both ways, either case; its
	 * defaults (the hat and the first axes move, 8BitDo's buttons and a
	 * pad's common ones); a direction added keeps the rest */
	PadMap r;
	padmap_default(&r);
	for (int i = 0; i < PAD_RAW_INPUTS; ++i)
		CHECK(padmap_input_of(PAD_FAMILY_RAW, padmap_name_of(PAD_FAMILY_RAW, i)) == i, "%s does not name raw input %d", padmap_name_of(PAD_FAMILY_RAW, i), i);
	CHECK(padmap_input_of(PAD_FAMILY_RAW, "H0.4") == PAD_RAW_HAT + 2 && padmap_input_of(PAD_FAMILY_RAW, "+A7") == PAD_RAW_AXIS(7, true), "a raw name in capitals refused");
	CHECK(padmap_input_of(PAD_FAMILY_RAW, "b32") == PAD_NONE && padmap_input_of(PAD_FAMILY_RAW, "h0.3") == PAD_NONE && padmap_input_of(PAD_FAMILY_RAW, "a1") == PAD_NONE &&
		padmap_input_of(PAD_FAMILY_RAW, "b1x") == PAD_NONE && padmap_input_of(PAD_FAMILY_RAW, "-a8") == PAD_NONE && padmap_input_of(PAD_FAMILY_RAW, "start") == PAD_NONE,
		"a raw name that names nothing taken");
	CHECK(padmap_bits(&r, PAD_FAMILY_RAW, PAD_RAW_HAT + 0) == BTN_UP && padmap_bits(&r, PAD_FAMILY_RAW, PAD_RAW_HAT + 3) == BTN_LEFT &&
		padmap_bits(&r, PAD_FAMILY_RAW, PAD_RAW_AXIS(1, true)) == BTN_DOWN && padmap_bits(&r, PAD_FAMILY_RAW, PAD_RAW_AXIS(0, true)) == BTN_RIGHT,
		"a raw joystick's hat or first axes do not move");
	CHECK(padmap_bits(&r, PAD_FAMILY_RAW, 0) == BTN_A && padmap_bits(&r, PAD_FAMILY_RAW, 1) == BTN_B && padmap_bits(&r, PAD_FAMILY_RAW, 6) == BTN_L &&
		padmap_bits(&r, PAD_FAMILY_RAW, 7) == BTN_R && padmap_bits(&r, PAD_FAMILY_RAW, 11) == BTN_START && padmap_bits(&r, PAD_FAMILY_RAW, 10) == BTN_SELECT,
		"8BitDo's A, B, L, R, START or SELECT is not");
	CHECK(padmap_bits(&r, PAD_FAMILY_RAW, 9) == BTN_START && padmap_bits(&r, PAD_FAMILY_RAW, 8) == BTN_SELECT && padmap_bits(&r, PAD_FAMILY_RAW, 4) == BTN_L,
		"a common pad's START, SELECT or L is not");
	/* the controls screen's slots: MAIN replaced keeps ALSO; ALSO set past
	 * the last; taken from another button, which keeps one (a swap), or
	 * refused where it was that one's only input and nothing was replaced */
	CHECK(padmap_set_slot(&r, PAD_FAMILY_RAW, 0, 0, PAD_RAW_AXIS(6, false)) == PAD_SLOT_ALONE && r.in[PAD_FAMILY_RAW][0][0] == PAD_RAW_AXIS(6, false) &&
		r.in[PAD_FAMILY_RAW][0][1] == PAD_RAW_AXIS(1, false) && !padmap_bits(&r, PAD_FAMILY_RAW, PAD_RAW_HAT + 0), "UP's MAIN set wrong");
	CHECK(padmap_set_slot(&r, PAD_FAMILY_XBOX, 4, 1, PAD_X) == PAD_SLOT_ALONE && padmap_count(&r, PAD_FAMILY_XBOX, 4) == 2 &&
		padmap_bits(&r, PAD_FAMILY_XBOX, PAD_X) == BTN_A && padmap_bits(&r, PAD_FAMILY_XBOX, PAD_A) == BTN_A, "A's ALSO set wrong");
	CHECK(padmap_set_slot(&r, PAD_FAMILY_XBOX, 5, 1, PAD_A) == 4 && padmap_bits(&r, PAD_FAMILY_XBOX, PAD_A) == BTN_B && padmap_bits(&r, PAD_FAMILY_XBOX, PAD_X) == BTN_A,
		"a taken from A, which kept x, went wrong");
	CHECK(padmap_set_slot(&r, PAD_FAMILY_XBOX, 4, 1, PAD_START) == PAD_SLOT_REFUSED && padmap_bits(&r, PAD_FAMILY_XBOX, PAD_START) == BTN_START,
		"START's only button taken for A's empty slot");
	CHECK(padmap_set_slot(&r, PAD_FAMILY_XBOX, 9, 0, PAD_START) == 8 && padmap_bits(&r, PAD_FAMILY_XBOX, PAD_BACK) == BTN_START &&
		padmap_bits(&r, PAD_FAMILY_XBOX, PAD_START) == BTN_SELECT, "START and SELECT did not swap");
	CHECK(padmap_set_slot(&r, PAD_FAMILY_XBOX, 6, 0, PAD_LEFTTRIGGER) == PAD_SLOT_ALONE && r.in[PAD_FAMILY_XBOX][6][0] == PAD_LEFTTRIGGER &&
		r.in[PAD_FAMILY_XBOX][6][1] == PAD_LEFTSHOULDER, "L's own ALSO not moved to MAIN");
	CHECK(padmap_clear_slot(&r, PAD_FAMILY_XBOX, 6, 1) && padmap_count(&r, PAD_FAMILY_XBOX, 6) == 1 && !padmap_clear_slot(&r, PAD_FAMILY_XBOX, 6, 0),
		"a slot cleared wrong, or a button's last cleared");
	CHECK(!strcmp(padmap_label_of(PAD_FAMILY_RAW, PAD_STYLE_GENERIC, 3), "Btn 4") && !strcmp(padmap_label_of(PAD_FAMILY_RAW, 0, PAD_RAW_HAT + 1), "Hat right") &&
		!strcmp(padmap_label_of(PAD_FAMILY_RAW, 0, PAD_RAW_AXIS(1, false)), "Axis 2-") && !strcmp(padmap_preset_name(PAD_FAMILY_RAW, 0, 1), "A and B swapped"),
		"a raw joystick's words are off");
	/* Chrome's hat as one axis */
	CHECK(padmap_pov_hat(-1.0) == 1 && padmap_pov_hat(-0.7143) == 3 && padmap_pov_hat(-0.4286) == 2 && padmap_pov_hat(0.1429) == 4 &&
		padmap_pov_hat(0.7143) == 8 && padmap_pov_hat(1.0) == 9 && padmap_pov_hat(1.2857) == 0, "Chrome's hat axis read wrong");
	/* every GBA button keeps one input whatever is bound */
	for (int k = 0; k < 200; ++k) padmap_bind(&m, k & 1, 4 + k % 6, (k * 7) % PAD_INPUTS == 29 ? PAD_Y : (k * 7) % PAD_INPUTS);
	for (int f = 0; f < PAD_FAMILIES; ++f)
		for (int g = 0; g < PAD_GBA; ++g) CHECK(m.in[f][g][0] != PAD_NONE, "GBA button %s lost all its inputs", padmap_gba_name(g));
	/* the presets for A and B */
	padmap_default(&m);
	CHECK(padmap_preset_of(&m, PAD_FAMILY_XBOX) == 0 && padmap_presets(PAD_FAMILY_XBOX) == 3 && padmap_presets(PAD_FAMILY_NINTENDO) == 2, "the presets are off");
	padmap_preset_apply(&m, PAD_FAMILY_XBOX, 1);
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_X) == BTN_B && padmap_bits(&m, PAD_FAMILY_XBOX, PAD_A) == BTN_A && !padmap_bits(&m, PAD_FAMILY_XBOX, PAD_B), "B on X is not");
	CHECK(padmap_preset_of(&m, PAD_FAMILY_XBOX) == 1 && !strcmp(padmap_preset_name(PAD_FAMILY_XBOX, PAD_STYLE_PLAYSTATION, 1), "B on Square"), "B on X not told as itself");
	padmap_preset_apply(&m, PAD_FAMILY_XBOX, 2);
	CHECK(padmap_bits(&m, PAD_FAMILY_XBOX, PAD_B) == BTN_A && padmap_bits(&m, PAD_FAMILY_XBOX, PAD_A) == BTN_B && !padmap_bits(&m, PAD_FAMILY_XBOX, PAD_X), "swapped is not");
	padmap_preset_apply(&m, PAD_FAMILY_XBOX, 0);
	padmap_default(&back);
	CHECK(padmap_same(&m, &back), "the presets there and back left the map changed");
	padmap_bind(&m, PAD_FAMILY_NINTENDO, 5, PAD_Y);
	CHECK(padmap_preset_of(&m, PAD_FAMILY_NINTENDO) == -1 && !strcmp(padmap_preset_name(PAD_FAMILY_NINTENDO, PAD_STYLE_NINTENDO, -1), "Custom"), "a hand-set B told as a preset");
	/* pad.ini: written, read back the same, nothing said */
	static char text[8192], errors[1024];
	padmap_bind(&m, PAD_FAMILY_XBOX, 6, PAD_AXIS(3, false));
	int n = padmap_format(&m, text, sizeof text);
	CHECK(n > 0 && n < (int)sizeof text - 1 && strstr(text, "[nintendo]") && strstr(text, "L      = -righty"), "pad.ini written wrong:\n%s", text);
	CHECK(padmap_parse(text, &back, errors, sizeof errors) == 0 && padmap_same(&m, &back), "pad.ini read back differs: %s", errors);
	CHECK(strstr(text, "[joystick]") && strstr(text, "UP     = h0.1, -a1"), "pad.ini's joysticks written wrong:\n%s", text);
	CHECK(padmap_parse("[joystick]\nUP = -a6, h0.1\nA = b2\nB = start\n", &back, errors, sizeof errors) == 1 &&
		padmap_bits(&back, PAD_FAMILY_RAW, PAD_RAW_AXIS(6, false)) == BTN_UP && padmap_bits(&back, PAD_FAMILY_RAW, 2) == BTN_A, "pad.ini's [joystick] not read: %s", errors);
	/* a hand-written one: lines without a section are [pad]'s, a button left out keeps its default, mistakes are said */
	CHECK(padmap_parse("A = x, y\n[nintendo]\nb = y\nZ = a\nSTART = hat\n[snes]\nA = b\n", &back, errors, sizeof errors) == 3, "pad.ini's mistakes not counted: %s", errors);
	CHECK(padmap_bits(&back, PAD_FAMILY_XBOX, PAD_X) == BTN_A && padmap_bits(&back, PAD_FAMILY_XBOX, PAD_Y) == BTN_A && !padmap_bits(&back, PAD_FAMILY_XBOX, PAD_A), "A = x, y not read");
	CHECK(padmap_bits(&back, PAD_FAMILY_NINTENDO, PAD_Y) == BTN_B && padmap_bits(&back, PAD_FAMILY_NINTENDO, PAD_A) == BTN_A, "[nintendo]'s line went elsewhere");
	CHECK(padmap_bits(&back, PAD_FAMILY_NINTENDO, PAD_START) == 0 && padmap_bits(&back, PAD_FAMILY_XBOX, PAD_B) == BTN_B, "a bad line or an unknown section's changed the map");
	CHECK(strstr(errors, "line 4") && strstr(errors, "\"Z\"") && strstr(errors, "\"hat\"") && strstr(errors, "\"snes\""), "pad.ini's mistakes not said: %s", errors);
	/* the words on the buttons */
	CHECK(!strcmp(padmap_label(PAD_STYLE_PLAYSTATION, PAD_A), "Cross") && !strcmp(padmap_label(PAD_STYLE_NINTENDO, PAD_LEFTTRIGGER), "ZL") &&
		!strcmp(padmap_label(PAD_STYLE_XBOX, PAD_LEFTSHOULDER), "LB") && !strcmp(padmap_label(PAD_STYLE_GENERIC, PAD_BACK), "Select") &&
		!strcmp(padmap_label(PAD_STYLE_NINTENDO, PAD_AXIS(0, false)), "L stick left") && !strcmp(padmap_label(PAD_STYLE_XBOX, PAD_INPUTS), "?"), "a button's words are wrong");
	/* Android's Joy-Cons: a mapping each, alone or as a pair, of SDL's names */
	char bad[64];
	for (int pair = 0; pair < 2; ++pair)
		for (int product = 0x2006; product <= 0x2007; ++product) {
			const char *jc = padmap_joycon(product, pair);
			CHECK(jc && mapping_fields_ok(jc, bad, sizeof bad), "Joy-Con %x (pair %d): field %s", product, pair, jc ? bad : "none");
		}
	CHECK(strstr(padmap_joycon(0x2006, true), "back:b4") && strstr(padmap_joycon(0x2006, true), "dpup:b11") && strstr(padmap_joycon(0x2006, true), "leftx:a0,lefty:a1"),
		"the left half lacks Minus, the D-pad or its stick as the left");
	CHECK(strstr(padmap_joycon(0x2007, true), "a:b1,b:b0,x:b2,y:b3") && strstr(padmap_joycon(0x2007, true), "rightx:a0"), "the right half's labels or stick are wrong");
	CHECK(strstr(padmap_joycon(0x2006, false), "leftx:a1") && strstr(padmap_joycon(0x2007, false), "lefty:a0") && !padmap_joycon(0x2009, true), "a lone Joy-Con is not held sideways");
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
	CHECK(why && strstr(why, "Attack+1 and HP+100 are both pink,and they touch"), "bug cause: one colour side by side (%s)", why ? why : "none");
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

/* Both games' RNG step and a seed's word (battle_rng.c): the guest's
 * battles' words as docs/ROM_DATA.md has them for seed 1 (BN5's random
 * numbers), a fixed point of the step, and one step of the differential's
 * (BN5's word on the frame its buster draws once more: tests/xdiff) */
static void test_battle_rng(void) {
	CHECK(battle_rng_word(0x85EBCA77u) == 0x9BB31115u && battle_rng_word(0x85EBCA77u ^ 0x5EC0DA27u) == 0xDCB010DDu,
		"battle_rng: seed 1's words %08x %08x", (unsigned)battle_rng_word(0x85EBCA77u), (unsigned)battle_rng_word(0x85EBCA77u ^ 0x5EC0DA27u));
	CHECK(battle_rng_step(0x7D14675Cu) == 0x7D14675Cu && battle_rng_step(0x82EB98A1u) == 0x82EB98A1u, "battle_rng: the step's two fixed points");
	CHECK(battle_rng_step(0xEC0BC510u) == 0x5F2B23C7u, "battle_rng: a step (%08x)", (unsigned)battle_rng_step(0xEC0BC510u));
	int on = 0;
	for (uint32_t s = 1; s < 200000; s += 7) {
		uint32_t w = battle_rng_word(s), x = w;
		for (int k = 0; k < 9; ++k) on += (x = battle_rng_step(x)) == w;
	}
	CHECK(!on, "battle_rng: %d seeds' words on the step's shortest cycles", on);
}

/* Thumb code moved (thumb_reloc.c, issue #82); tests/test_emu.c runs a
 * moved block in mGBA. A BL's halves both ways, to the ends of its reach
 * and refused a step past them; an odd address refused. */
static void test_thumb_bl(void) {
	uint16_t hi = 0, lo = 0;
	uint32_t t = 0;
	/* (test_emu's ROM: 0xD2 to 0xF8) */
	CHECK(thumb_bl_encode(0x080000D2, 0x080000F8, &hi, &lo) && hi == 0xF000 && lo == 0xF811, "thumb: bl 0xD2 to 0xF8 (%04x %04x)", hi, lo);
	CHECK(thumb_bl_decode(0x080000D2, 0xF000, 0xF811, &t) && t == 0x080000F8, "thumb: its target (%08x)", t);
	const uint32_t at = 0x08800000;
	CHECK(thumb_bl_encode(at, at + 4 + 0x3FFFFE, &hi, &lo) && hi == 0xF3FF && lo == 0xFFFF, "thumb: the furthest on (%04x %04x)", hi, lo);
	CHECK(thumb_bl_decode(at, hi, lo, &t) && t == at + 4 + 0x3FFFFE, "thumb: the furthest on back (%08x)", t);
	CHECK(thumb_bl_encode(at, at + 4 - 0x400000, &hi, &lo) && hi == 0xF400 && lo == 0xF800, "thumb: the furthest back (%04x %04x)", hi, lo);
	CHECK(thumb_bl_decode(at, hi, lo, &t) && t == at + 4 - 0x400000, "thumb: the furthest back, back (%08x)", t);
	hi = lo = 0x1234;
	CHECK(!thumb_bl_encode(at, at + 4 + 0x400000, &hi, &lo) && !thumb_bl_encode(at, at + 2 - 0x400000, &hi, &lo), "thumb: a step past either end refused");
	CHECK(!thumb_bl_encode(at, at + 0x101, &hi, &lo) && !thumb_bl_encode(at + 1, at + 0x100, &hi, &lo), "thumb: an odd target or address refused");
	CHECK(hi == 0x1234 && lo == 0x1234, "thumb: a refused BL wrote its halves (%04x %04x)", hi, lo);
	/* every distance, both ways, there and back */
	int bad = 0;
	for (int32_t off = -0x400000; off < 0x400000; off += 0x1F2E) {
		uint32_t to = at + 4 + (uint32_t)off;
		if (!thumb_bl_encode(at, to, &hi, &lo) || !thumb_is_bl(hi, lo) || !thumb_bl_decode(at, hi, lo, &t) || t != to) ++bad;
	}
	CHECK(!bad, "thumb: %d BLs not there and back", bad);
	CHECK(!thumb_bl_decode(at, 0xF811, 0xF000, &t) && !thumb_bl_decode(at, 0x2005, 0xF811, &t) && !thumb_bl_decode(at, 0xF000, 0x2005, &t),
		"thumb: halves out of order, or not a BL's, read as one");
}

/* The BLs of a run of code found, half a BL refused; a block's calls out
 * aimed again from where it moved, those within it left; all or nothing. */
static uint32_t thumb_resolve(uint32_t target, void *user) {
	++*(int *)user;
	return target == 0x08002000u ? 0x08A00400u : target == 0x08003000u ? 0x08A0040Cu : target == 0x08004000u ? 0x08000200u : 0;
}

static void test_thumb_block(void) {
	const uint32_t from = 0x08001000, to = 0x08A00000;
	uint8_t code[0x20], was[0x20];
	memset(code, 0, sizeof code);
	put16(code + 0x00, 0x2007);                     /* movs r0,#7 */
	thumb_bl_write(code, 0x02, from, from + 0x18);  /* within */
	thumb_bl_write(code, 0x06, from, 0x08002000);   /* out */
	put16(code + 0x0A, 0x6EB8);                     /* ldr r0,[r7,#0x68] */
	thumb_bl_write(code, 0x0C, from, 0x08003000);   /* out */
	put16(code + 0x10, 0x4770);                     /* bx lr */
	uint32_t offs[8], one[1] = { 0 }, scratch[8];
	int n = thumb_bl_scan(code, 0x12, offs, 8);
	CHECK(n == 3 && offs[0] == 0x02 && offs[1] == 0x06 && offs[2] == 0x0C, "thumb: the run's BLs (%d)", n);
	CHECK(thumb_bl_scan(code, 0x12, one, 1) == 3 && one[0] == 0x02, "thumb: BLs past max counted, not kept");
	CHECK(thumb_bl_scan(code, 0x08, scratch, 8) == -1, "thumb: a run ending in half a BL refused");
	put16(code + 0x12, 0xF800);
	CHECK(thumb_bl_scan(code, sizeof code, scratch, 8) == -1, "thumb: a lone lo half refused");
	memcpy(was, code, sizeof code);

	int asked = 0;
	ThumbBlock b = { code, sizeof code, from, to };
	uint32_t t = 0;
	CHECK(thumb_block_rebase(&b, offs, 3, thumb_resolve, &asked) && asked == 2, "thumb: the block moved (asked %d)", asked);
	CHECK(!memcmp(code + 0x02, was + 0x02, 4) && thumb_bl_read(code, 0x02, to, &t) && t == to + 0x18, "thumb: the call within, as it was (%08x)", t);
	CHECK(thumb_bl_read(code, 0x06, to, &t) && t == 0x08A00400, "thumb: the first call out (%08x)", t);
	CHECK(thumb_bl_read(code, 0x0C, to, &t) && t == 0x08A0040C, "thumb: the second call out (%08x)", t);
	CHECK(!memcmp(code, was, 2) && !memcmp(code + 0x0A, was + 0x0A, 2), "thumb: the rest of the block untouched");

	/* refused, the block as it was: a call out with no answer; one out of
	 * reach from its new address; a move off the alignment; an offset
	 * that holds no BL, an odd one, one past the end */
	memcpy(code, was, sizeof code);
	thumb_bl_write(code, 0x0C, from, 0x08005000);
	memcpy(was, code, sizeof code);
	CHECK(!thumb_block_rebase(&b, offs, 3, thumb_resolve, &asked) && !memcmp(code, was, sizeof code), "thumb: a call with nowhere to go moved");
	thumb_bl_write(code, 0x0C, from, 0x08004000);
	memcpy(was, code, sizeof code);
	CHECK(!thumb_block_rebase(&b, offs, 3, thumb_resolve, &asked) && !memcmp(code, was, sizeof code), "thumb: a call out of reach moved");
	ThumbBlock off4 = { code, sizeof code, from, to + 2 };
	CHECK(!thumb_block_rebase(&off4, offs, 2, thumb_resolve, &asked), "thumb: a block moved off its alignment");
	static const uint32_t no_bl[] = { 0x00 }, odd[] = { 0x03 }, past[] = { 0x1E };
	CHECK(!thumb_block_rebase(&b, no_bl, 1, thumb_resolve, &asked) && !thumb_block_rebase(&b, odd, 1, thumb_resolve, &asked) &&
		!thumb_block_rebase(&b, past, 1, thumb_resolve, &asked) && !memcmp(code, was, sizeof code), "thumb: an offset with no BL moved");
	CHECK(thumb_block_rebase(&b, offs, 0, thumb_resolve, &asked), "thumb: a block without calls refused");
}

/* Immediates in their instruction: the recipe's four (issue #82's
 * CanGard: movs r0, adds r4, movs r2, ldr r0,[r7]) patched, every
 * mismatch refused and the instruction kept; every form there and back. */
static void test_thumb_imm(void) {
	static const struct { uint16_t insn; ThumbImm was; uint16_t imm, now; } ok[] = {
		{ 0x2007, { THUMB_MOVS, 0, 0, 7 }, 9, 0x2009 },
		{ 0x3457, { THUMB_ADDS, 4, 0, 0x57 }, 0x6C, 0x346C },
		{ 0x2224, { THUMB_MOVS, 2, 0, 0x24 }, 0x5C, 0x225C },
		{ 0x6EB8, { THUMB_LDR, 0, 7, 0x68 }, 0x70, 0x6F38 },
		{ 0x2804, { THUMB_CMP, 0, 0, 4 }, 5, 0x2805 },
		{ 0x3901, { THUMB_SUBS, 1, 0, 1 }, 2, 0x3902 },
		{ 0x6048, { THUMB_STR, 0, 1, 4 }, 0x7C, 0x67C8 },
		{ 0x7030, { THUMB_STRB, 0, 6, 0 }, 31, 0x77F0 },
		{ 0x7868, { THUMB_LDRB, 0, 5, 1 }, 2, 0x78A8 },
		{ 0x8CA8, { THUMB_LDRH, 0, 5, 0x24 }, 0x3E, 0x8FE8 },
		{ 0x8008, { THUMB_STRH, 0, 1, 0 }, 2, 0x8048 },
		{ 0x4907, { THUMB_LDR_PC, 1, 0, 28 }, 1020, 0x49FF },
		/* (the sprite preload's id mask, 9 bits to 12: xvirus_grow.c) */
		{ 0x07D2, { THUMB_LSLS, 2, 2, 0x1F }, 0x1C, 0x0712 },
		{ 0x0DD2, { THUMB_LSRS, 2, 2, 0x17 }, 0x14, 0x0D12 },
		{ 0x1109, { THUMB_ASRS, 1, 1, 4 }, 8, 0x1209 },
	};
	for (size_t i = 0; i < sizeof ok / sizeof *ok; ++i) {
		uint16_t insn = ok[i].insn;
		ThumbImm got;
		CHECK(thumb_imm_decode(insn, ok[i].was.form, &got) && got.rd == ok[i].was.rd && got.rn == ok[i].was.rn && got.imm == ok[i].was.imm,
			"thumb: %04x read as its form", insn);
		CHECK(thumb_imm_patch(&insn, &ok[i].was, ok[i].imm) && insn == ok[i].now, "thumb: %04x patched to %04x, not %04x", ok[i].insn, insn, ok[i].now);
	}
	static const struct { uint16_t insn; ThumbImm was; uint16_t imm; } no[] = {
		{ 0x2007, { THUMB_ADDS, 0, 0, 7 }, 9 },      /* another form */
		{ 0x2007, { THUMB_MOVS, 1, 0, 7 }, 9 },      /* another register */
		{ 0x2007, { THUMB_MOVS, 0, 0, 8 }, 9 },      /* another immediate */
		{ 0x2007, { THUMB_MOVS, 0, 0, 7 }, 256 },    /* too big */
		{ 0x6EB8, { THUMB_LDR, 0, 6, 0x68 }, 0x70 }, /* another base */
		{ 0x6EB8, { THUMB_LDR, 0, 7, 0x68 }, 0x72 }, /* not a word's */
		{ 0x6EB8, { THUMB_LDR, 0, 7, 0x68 }, 0x80 }, /* too far */
		{ 0x6EB8, { THUMB_STR, 0, 7, 0x68 }, 0x70 }, /* a store */
		{ 0x6EB8, { THUMB_LDRB, 0, 7, 0x68 }, 0x70 },
		{ 0x7868, { THUMB_LDRB, 0, 5, 1 }, 32 },
		{ 0x8CA8, { THUMB_LDRH, 0, 5, 0x24 }, 0x25 },
		{ 0x4907, { THUMB_LDR_PC, 1, 0, 28 }, 1024 },
		{ 0x07D2, { THUMB_LSLS, 2, 2, 0x1F }, 32 },  /* a shift past 31 */
		{ 0x07D2, { THUMB_LSRS, 2, 2, 0x1F }, 0x1C }, /* lsls read as lsrs */
		{ 0x0DD2, { THUMB_LSRS, 2, 3, 0x17 }, 0x14 }, /* another register shifted */
		{ 0x2007, { THUMB_FORMS, 0, 0, 7 }, 9 },
	};
	for (size_t i = 0; i < sizeof no / sizeof *no; ++i) {
		uint16_t insn = no[i].insn;
		CHECK(!thumb_imm_patch(&insn, &no[i].was, no[i].imm) && insn == no[i].insn, "thumb: patch %zu of %04x taken (%04x)", i, no[i].insn, insn);
	}
	uint16_t insn = 0;
	static const ThumbImm misfit[] = { { THUMB_MOVS, 8, 0, 1 }, { THUMB_MOVS, 0, 1, 1 }, { THUMB_LDR, 0, 8, 4 } };
	for (size_t i = 0; i < sizeof misfit / sizeof *misfit; ++i)
		CHECK(!thumb_imm_encode(&misfit[i], &insn) && insn == 0, "thumb: register %zu that does not fit taken (%04x)", i, insn);
	int bad = 0;
	for (int f = 0; f < THUMB_FORMS; ++f)
		for (unsigned imm = 0; imm < 1024; ++imm)
			for (uint8_t rd = 0; rd < 8; ++rd)
				for (uint8_t rn = 0; rn < 8; ++rn) {
					ThumbImm in = { (ThumbForm)f, rd, rn, (uint16_t)imm }, back;
					if (!thumb_imm_encode(&in, &insn)) continue;
					if (!thumb_imm_decode(insn, in.form, &back) || back.rd != rd || back.rn != rn || back.imm != imm) ++bad;
					for (int g = 0; g < THUMB_FORMS; ++g)
						if (g != f && thumb_imm_decode(insn, (ThumbForm)g, &back)) ++bad;
				}
	CHECK(!bad, "thumb: %d immediates not there and back, or read as another form", bad);
}

/* Veneers: their bytes, a misaligned one refused, read back; a pool of
 * them, one per target, full at its room. */
static void test_thumb_veneers(void) {
	static const uint8_t want[THUMB_VENEER_SIZE] = { 0x03, 0xB4, 0x01, 0x48, 0x01, 0x90, 0x01, 0xBD, 0x81, 0x63, 0x01, 0x08 };
	uint8_t v[THUMB_VENEER_SIZE];
	uint32_t t = 0;
	CHECK(thumb_veneer(v, 0x08B31000, 0x08016381) && !memcmp(v, want, sizeof v), "thumb: a veneer's bytes");
	CHECK(thumb_veneer(v, 0x08B31000, 0x08016380) && !memcmp(v, want, sizeof v), "thumb: a veneer's target given without its Thumb bit");
	CHECK(thumb_veneer_target(v, &t) && t == 0x08016380, "thumb: a veneer's target read back (%08x)", t);
	CHECK(!thumb_veneer(v, 0x08B31002, 0x08016381), "thumb: a veneer off a word boundary");
	v[5] = 0x91;
	CHECK(!thumb_veneer_target(v, &t), "thumb: other bytes read as a veneer");

	uint8_t room[3 * THUMB_VENEER_SIZE];
	ThumbVeneers pool = { room, 0x08B31000, 3, 0 };
	uint32_t a = thumb_veneer_get(&pool, 0x08016381), b = thumb_veneer_get(&pool, 0x0801A152), a2 = thumb_veneer_get(&pool, 0x08016380);
	uint32_t c = thumb_veneer_get(&pool, 0x0801A15C), d = thumb_veneer_get(&pool, 0x0801A166);
	CHECK(a == 0x08B31000 && b == 0x08B3100C && a2 == a && c == 0x08B31018 && !d && pool.n == 3, "thumb: the pool (%08x %08x %08x %08x %08x, %d made)", a, b, a2,
		c, d, pool.n);
	CHECK(thumb_veneer_target(room + THUMB_VENEER_SIZE, &t) && t == 0x0801A152, "thumb: the pool's second veneer (%08x)", t);
	ThumbVeneers off = { room, 0x08B31002, 3, 0 };
	CHECK(!thumb_veneer_get(&off, 0x08016381) && off.n == 0, "thumb: a pool off a word boundary");
}

/* A virus family's recipe (issue #82, xvirus_recipe.h) holds together
 * before any ROM is read: the control's does, and each way of breaking a
 * copy of it is caught with its reason; the CRC32 the blocks are checked
 * by is zlib's. */
static bool recipe_refused(const XVirusFamily *f, const char *what) {
	char why[160] = "";
	bool ok = xvirus_recipe_ok(f, why, sizeof why);
	CHECK(!ok && why[0], "xvirus: a recipe with %s taken", what);
	return !ok;
}

static void test_xvirus_recipe(void) {
	const uint8_t digits[] = "123456789";
	CHECK(xvirus_crc32(digits, 9) == 0xCBF43926u && xvirus_crc32(digits, 0) == 0, "xvirus: CRC32 %08x", xvirus_crc32(digits, 9));
	char why[160] = "";
	const XVirusFamily *c = &xvirus_control;
	CHECK(xvirus_recipe_ok(c, why, sizeof why), "xvirus: the control's recipe refused: %s", why);
	CHECK(c->ai == 32 && c->first_id == XV_FIRST_ID && c->nstates == 13 && c->ncalls == 32 && c->ice == XV_ICE_OWN,
		"xvirus: the control is AI %d from id %#x", c->ai, c->first_id);
	/* (its code and data: every halfword of its block code or one range of data) */
	const XvBlock *b = &c->blocks[0];
	int code = 0, data = 0;
	for (uint32_t a = b->start; a < b->end; a += 2) xvirus_is_code(c, a) ? ++code : ++data;
	CHECK(code * 2 == 1276 && xvirus_block_of(c, b->end) == NULL && xvirus_data_at(c, 0x0810B024) && !xvirus_is_code(c, 0x0810B026),
		"xvirus: the control's block holds %d bytes of code, %d of data", code * 2, data * 2);

	XVirusFamily f = *c;
	f.ai = 31;
	recipe_refused(&f, "one of BN6's own AI indexes");
	f = *c;
	f.ai = XV_AIS;
	recipe_refused(&f, "an AI index past the grown tables");
	f = *c;
	f.first_id = 0xBB;
	recipe_refused(&f, "enemy ids among BN6's own");
	f.first_id = XV_IDS - XV_VERSIONS + 1;
	recipe_refused(&f, "enemy ids past the grown ones");
	f.first_id = XV_IDS - XV_VERSIONS;
	f.ai = XV_AIS - 1;
	CHECK(xvirus_recipe_ok(&f, why, sizeof why), "xvirus: the last AI index and ids refused: %s", why);
	f = *c;
	f.home = XV_HOMES + 4;
	recipe_refused(&f, "a home off its slot");
	f = *c;
	f.ice = XV_ICE_MAX + 1;
	recipe_refused(&f, "an ice block past BN6's sizes");
	f.ice = -2;
	recipe_refused(&f, "an ice block of no size");
	XvBlock blocks[2] = { c->blocks[0], c->blocks[0] };
	f = *c;
	blocks[0].at = XV_CODE + 2;
	f.blocks = blocks;
	recipe_refused(&f, "a block moved off its word alignment");
	blocks[0] = c->blocks[0];
	blocks[1].start = b->end - 0x10;
	blocks[1].end = b->end + 0x10;
	blocks[1].at = XV_CODE + 0x1000;
	f.nblocks = 2;
	recipe_refused(&f, "two blocks over the same code");
	XvData data_bad[64];
	memcpy(data_bad, c->data, (size_t)c->ndata * sizeof *data_bad);
	f = *c;
	f.data = data_bad;
	data_bad[0].start = 0x0810B022;
	recipe_refused(&f, "a literal pool off a word boundary");
	data_bad[0] = c->data[0];
	data_bad[c->ndata] = (XvData){ 0x0810B5F0, 0x0810B5F4, XV_RAW };
	f.ndata = c->ndata + 1;
	recipe_refused(&f, "data past its block");
	data_bad[c->ndata] = (XvData){ 0x0810B020, 0x0810B02C, XV_RAW };
	recipe_refused(&f, "two ranges of data over one another");
	XvRef states[16];
	memcpy(states, c->states, (size_t)c->nstates * sizeof *states);
	f = *c;
	f.states = states;
	states[9] = (XvRef){ 0x0810B029, true };
	recipe_refused(&f, "a state of its own in its data");
	states[9] = (XvRef){ 0x0810AFEC, true };
	recipe_refused(&f, "a state of its own without the Thumb bit");
	states[9] = (XvRef){ 0x09000001, false };
	recipe_refused(&f, "a state past BN6's ROM");
	f = *c;
	f.nstates = 7;
	recipe_refused(&f, "fewer states than BN6's generic eight");
	XvPair calls[40];
	memcpy(calls, c->calls, (size_t)c->ncalls * sizeof *calls);
	f = *c;
	f.calls = calls;
	calls[3].to |= 1;
	recipe_refused(&f, "a call aimed at a Thumb bit");
	calls[3] = c->calls[3];
	calls[4].from = 0x0810B040;
	recipe_refused(&f, "a call into its own code");
	calls[4] = calls[5];
	recipe_refused(&f, "a call named twice");
	XvImm imm = { 0x0810B026, THUMB_MOVS, 0, 0, 1, 2 };
	f = *c;
	f.imms = &imm;
	f.nimms = 1;
	recipe_refused(&f, "an immediate in a literal pool");
}

/* BN5's CanGard's recipe (issue #82): whole before any ROM is read, its
 * two blocks' code and data as its research found them, its state table
 * BN6's twelve, its 39 calls; and each way of breaking what it draws
 * caught: a sprite out of the art's room or over another, an effect among
 * BN6's own, an attack's type twice, its struct1 laid out with a mode
 * past BN6's table */
static void test_xvirus_cangard(void) {
	char why[160] = "";
	const XVirusFamily *c = &xvirus_cangard;
	CHECK(xvirus_recipe_ok(c, why, sizeof why), "xvirus: CanGard's recipe refused: %s", why);
	CHECK(c->game == XROM_BN5_COLONEL_US && c->src_ai == 14 && c->ai == 32 && c->first_id == XV_FIRST_ID && c->nstates == 12 &&
		c->ncalls == 39 && c->nimms == 5 && c->nsprites == 3 && c->traits.laid && c->traits.shadow && !c->traits.element,
		"xvirus: CanGard is AI %d from id %#x, %d states, %d calls", c->ai, c->first_id, c->nstates, c->ncalls);
	int code[2] = { 0, 0 };
	for (int i = 0; i < c->nblocks; ++i)
		for (uint32_t a = c->blocks[i].start; a < c->blocks[i].end; a += 2) code[i] += xvirus_is_code(c, a);
	/* (CanGard's 812 bytes less 142 of data: its state table's 40, 48 of jump tables, 28 of pools, 26 of numbers and
	 * padding; the reticle's 620 less 72: 24 of jump tables, 16 of pools, 32 of numbers and padding) */
	CHECK(code[0] * 2 == 670 && code[1] * 2 == 548, "xvirus: CanGard's blocks hold %d and %d bytes of code",
		code[0] * 2, code[1] * 2);
	CHECK(c->states[8].own && c->states[8].addr == 0x08109087 && !c->states[6].own && c->states[6].addr == 0x08016B37,
		"xvirus: CanGard's decide at 8, BN6's freeze at 6");
	/* (its BN5 state table is dead in the copy: one range, zeroed) */
	const XvData *st = xvirus_data_at(c, 0x08109000);
	CHECK(st && st->kind == XV_ZERO && st->start == 0x08109000 && st->end == 0x08109028, "xvirus: CanGard's BN5 state table zeroed");

	XVirusFamily f = *c;
	XvSprite sprites[3];
	memcpy(sprites, c->sprites, sizeof sprites);
	f.sprites = sprites;
	sprites[1].at = XV_CODE;
	recipe_refused(&f, "a sprite among the code");
	sprites[1] = c->sprites[1];
	sprites[2].at = sprites[1].at + 0x10;
	recipe_refused(&f, "two sprites in one room");
	sprites[2] = c->sprites[2];
	sprites[2].to_index = sprites[1].to_index;
	recipe_refused(&f, "two sprites at one number");
	sprites[2] = c->sprites[2];
	sprites[0].to_list = 4;
	recipe_refused(&f, "its battle sprite out of the viruses' list");
	sprites[0] = c->sprites[0];
	sprites[2].to_list = 3;
	recipe_refused(&f, "a sprite in a list not grown");
	sprites[2] = c->sprites[2];
	sprites[2].to_index = 0xFF;
	CHECK(xvirus_recipe_ok(&f, why, sizeof why), "xvirus: a sprite at the grown list's last number refused: %s", why);
	XvId fx = { 0x57, 0x61 };
	f = *c;
	f.effects = &fx;
	recipe_refused(&f, "an effect among BN6's own");
	XvType types[2] = { { 3, 0x25, XV_TYPE_MOVED }, { 3, 0x25, 0x4B } };
	f = *c;
	f.types = types;
	f.ntypes = 2;
	recipe_refused(&f, "one type named twice");
	types[1] = (XvType){ 3, 0x26, XV_T3_DISPATCH };
	recipe_refused(&f, "a type made the dispatch's own");
	types[1] = (XvType){ 2, 0x26, XV_TYPE_MOVED };
	recipe_refused(&f, "a type of a kind the dispatch has none of");
	types[1] = (XvType){ 1, 0x25, XV_TYPE_MOVED };
	CHECK(xvirus_recipe_ok(&f, why, sizeof why), "xvirus: one number of two kinds refused: %s", why);
	types[1] = (XvType){ 3, 0x26, 0x4B };
	CHECK(xvirus_recipe_ok(&f, why, sizeof why), "xvirus: a twin of BN6's refused: %s", why);
	/* (its reticle's spawn: the dispatch's spawner, never BN6's own nor another of the dispatch's) */
	XvPair calls[39];
	memcpy(calls, c->calls, sizeof calls);
	f = *c;
	f.calls = calls;
	for (int i = 0; i < c->ncalls; ++i)
		if (calls[i].from == BN5_SPAWN_3) calls[i].to = BN6_SPAWN_3;
	recipe_refused(&f, "its game's spawner aimed at BN6's");
	for (int i = 0; i < c->ncalls; ++i)
		if (calls[i].from == BN5_SPAWN_3) calls[i].to = XV_SPAWN_3_LAST;
	recipe_refused(&f, "its game's spawner aimed at another of the dispatch's");
	calls[0].to = XV_SPAWN_3;
	recipe_refused(&f, "a call of no spawner aimed at the dispatch's");
	/* (a mode past BN6's table is taken, BN6's record given XV_MODE_PAST: BN5's TestVirs and Tuby) */
	f = *c;
	f.traits.mode = 0x22;
	CHECK(xvirus_recipe_ok(&f, why, sizeof why), "xvirus: a mode past BN6's table refused: %s", why);
	CHECK(xvirus_list_slot(BN6_SPRITES_VIRUSES) == 0 && xvirus_list_slot(BN6_SPRITES_LIST5) == XV_LISTS_GROWN - 1 && xvirus_list_slot(3) < 0,
		"xvirus: the grown lists' places");
}

/* BN5's families at fixed numbers, all at once (issue #82, the static
 * model): each recipe whole, and no two sharing an AI index, an enemy id,
 * a home, a byte of code or art past EMU_FREE, a sprite number, an effect
 * or an attack's type */
static bool spans_meet(uint32_t a, uint32_t alen, uint32_t b, uint32_t blen) { return a < b + blen && b < a + alen; }

static int static_clashes(const XVirusFamily *f, const XVirusFamily *g) {
	int bad = f->ai == g->ai || spans_meet((uint32_t)f->first_id, XV_VERSIONS, (uint32_t)g->first_id, XV_VERSIONS) ||
		spans_meet(f->home, XV_HOME_SIZE, g->home, XV_HOME_SIZE);
	for (int i = 0; i < f->nblocks; ++i)
		for (int k = 0; k < g->nblocks; ++k)
			bad += spans_meet(f->blocks[i].at, f->blocks[i].end - f->blocks[i].start, g->blocks[k].at, g->blocks[k].end - g->blocks[k].start);
	for (int i = 0; i < f->nsprites; ++i)
		for (int k = 0; k < g->nsprites; ++k)
			bad += spans_meet(f->sprites[i].at, f->sprites[i].room, g->sprites[k].at, g->sprites[k].room) ||
				(f->sprites[i].to_list == g->sprites[k].to_list && f->sprites[i].to_index == g->sprites[k].to_index);
	for (int i = 0; i < f->neffects; ++i)
		for (int k = 0; k < g->neffects; ++k) bad += f->effects[i].to == g->effects[k].to;
	/* (a type both move: two routines for one entry; or both name, with another twin; one moved by a family before the
	 * other, g, and shared by f, is one routine) */
	for (int i = 0; i < f->ntypes; ++i)
		for (int k = 0; k < g->ntypes; ++k) {
			const XvType *a = &f->types[i], *b = &g->types[k];
			if (a->kind != b->kind || a->from != b->from) continue;
			bool shared = (a->to == XV_TYPE_SHARED && b->to == XV_TYPE_MOVED) || (a->to == XV_TYPE_SHARED && b->to == XV_TYPE_SHARED);
			bad += !shared && (a->to == XV_TYPE_MOVED || a->to != b->to);
		}
	/* (a song slot, or a song's bytes over the other's art) */
	for (int i = 0; i < f->nsongs; ++i) {
		for (int k = 0; k < g->nsongs; ++k)
			bad += f->songs[i].slot == g->songs[k].slot || spans_meet(f->songs[i].at, f->songs[i].room, g->songs[k].at, g->songs[k].room);
		for (int k = 0; k < g->nsprites; ++k) bad += spans_meet(f->songs[i].at, f->songs[i].room, g->sprites[k].at, g->sprites[k].room);
	}
	for (int i = 0; i < g->nsongs; ++i)
		for (int k = 0; k < f->nsprites; ++k) bad += spans_meet(g->songs[i].at, g->songs[i].room, f->sprites[k].at, f->sprites[k].room);
	return bad;
}

/* a type each family shares (XV_TYPE_SHARED) is one a family before it in the install order moved, or BN5's engine
 * objects' moved once before any family (xnavi_objects.h: WuNote's image) */
static bool shared_moved_before(int i) {
	const XVirusFamily *f = xvirus_bn5[i];
	for (int t = 0; t < f->ntypes; ++t) {
		if (f->types[t].to != XV_TYPE_SHARED) continue;
		bool found = false;
		for (int u = 0; u < xnavi_objects_bn5.ntypes; ++u)
			found |= xnavi_objects_bn5.types[u].kind == f->types[t].kind && xnavi_objects_bn5.types[u].from == f->types[t].from &&
				xnavi_objects_bn5.types[u].to == XV_TYPE_MOVED;
		for (int k = 0; k < i && !found; ++k)
			for (int u = 0; u < xvirus_bn5[k]->ntypes; ++u)
				found |= xvirus_bn5[k]->types[u].kind == f->types[t].kind && xvirus_bn5[k]->types[u].from == f->types[t].from &&
					xvirus_bn5[k]->types[u].to == XV_TYPE_MOVED;
		if (!found) return false;
	}
	return true;
}

static void test_xvirus_static(void) {
	char why[160] = "";
	/* (BN5's 25 families not BN6's by name, CanGard and Handi first: the static model's slots, issue #82's A2b) */
	CHECK(xvirus_bn5_count == 25 && xvirus_bn5[0]->src_ai == 14 && xvirus_bn5[1]->src_ai == 4, "xvirus: BN5's families listed (%d)", xvirus_bn5_count);
	for (int i = 0; i < xvirus_bn5_count; ++i) {
		const XVirusFamily *f = xvirus_bn5[i];
		CHECK(xvirus_recipe_ok(f, why, sizeof why) && f->game == XROM_BN5_COLONEL_US, "xvirus: %s refused: %s", f->name, why);
		CHECK(f->ai == BN6_FAMILIES + i && f->first_id == XV_FIRST_ID + XV_VERSIONS * i && f->home == XV_HOMES + XV_HOME_SIZE * (uint32_t)i,
			"xvirus: %s off its slot %d", f->name, i);
		CHECK(shared_moved_before(i), "xvirus: %s shares a type no family before it moved", f->name);
		for (int k = 0; k < i; ++k)
			CHECK(!static_clashes(f, xvirus_bn5[k]), "xvirus: %s and %s share numbers or room", xvirus_bn5[k]->name, f->name);
	}
	/* (the check sees a clash: CanGard against itself moved by a home and its ids) */
	XVirusFamily twin = xvirus_cangard;
	twin.ai = 33;
	twin.first_id = XV_FIRST_ID + XV_VERSIONS;
	twin.home = XV_HOMES + XV_HOME_SIZE;
	CHECK(static_clashes(&twin, &xvirus_cangard) > 0, "xvirus: two families' code and art in one place not seen");
	twin.nblocks = twin.nsprites = twin.neffects = twin.ntypes = 0;
	CHECK(!static_clashes(&twin, &xvirus_cangard), "xvirus: a family of other numbers and nothing to place taken for a clash");
	twin.first_id = XV_FIRST_ID + 5;
	CHECK(static_clashes(&twin, &xvirus_cangard) == 1, "xvirus: two families' ids over one another not seen");
}

/* BN5's Navis' recipes as installed (xnavi_bn5.c; docs/MULTIROM.md,
 * Transplanted Navis), each whole and apart from the others and from the
 * families: its AI index, ids, home and code its own; a sprite two Navis
 * draw at one place and number in both (the shared art), no other sprite's
 * place or number another's; an effect two Navis spawn one number, in the
 * Navis' range, and no number two effects' (a family's copy of the same
 * BN5 effect is the families' own number, issue #82's ranges a track each);
 * no type two recipes move, but one a family runs as the moved objects'
 * (XV_TYPE_SHARED); the moved objects' recipe whole, its type moved by it
 * alone, each type a Navi needs one it moves; the Navis' room laid out in
 * order */
static int navi_clashes(const XVirusFamily *f, const XVirusFamily *g, bool navis) {
	int bad = navis && (f->ai == g->ai || spans_meet(f->home, XV_HOME_SIZE, g->home, XV_HOME_SIZE));
	for (int i = 0; i < f->nblocks; ++i)
		for (int k = 0; k < g->nblocks; ++k)
			bad += spans_meet(f->blocks[i].at, f->blocks[i].end - f->blocks[i].start, g->blocks[k].at, g->blocks[k].end - g->blocks[k].start);
	for (int i = 0; i < f->nsprites; ++i)
		for (int k = 0; k < g->nsprites; ++k) {
			const XvSprite *a = &f->sprites[i], *b = &g->sprites[k];
			bool same = a->list == b->list && a->index == b->index && a->to_list == b->to_list && a->to_index == b->to_index && a->at == b->at &&
				a->room == b->room;
			bad += !same && (spans_meet(a->at, a->room, b->at, b->room) || (a->to_list == b->to_list && a->to_index == b->to_index));
		}
	/* (an effect two Navis spawn: one number for one BN5 effect; any two recipes: no number for two) */
	for (int i = 0; i < f->neffects; ++i)
		for (int k = 0; k < g->neffects; ++k) {
			bool to = f->effects[i].to == g->effects[k].to, from = f->effects[i].from == g->effects[k].from;
			bad += navis ? to != from : to && !from;
		}
	for (int i = 0; i < f->ntypes; ++i)
		for (int k = 0; k < g->ntypes; ++k) {
			const XvType *a = &f->types[i], *b = &g->types[k];
			bool shares = (a->to == XV_TYPE_MOVED && b->to == XV_TYPE_SHARED) || (a->to == XV_TYPE_SHARED && b->to == XV_TYPE_MOVED);
			bad += a->kind == b->kind && a->from == b->from && !shares && (a->to == XV_TYPE_MOVED || a->to != b->to);
		}
	return bad;
}

static void test_xnavi_static(void) {
	char why[160] = "";
	const XVirusFamily *o = &xnavi_objects_bn5;
	CHECK(XN_STUB_DS < XN_STUB_BUSTER && XN_STUB_BUSTER < XN_STUB_SIDES && XN_STUB_SIDES < XN_STUB_CHIPDATA && XN_STUB_CHIPDATA < XN_STUBS_END && XN_STUBS_END == XN_SEA && XN_SEA < XN_SEA_END && XN_SEA_END == XN_OBJECTS && XN_OBJECTS < XN_OBJECTS_END && XN_OBJECTS_END <= XN_CODE && XN_CODE < XN_ART &&
		XN_ART < XN_SHARED_ART && XN_SHARED_ART < XN_END && XN_END == XN_DARK_RECORDS && XN_DARK_RECORDS + XREG_IDS * 0x2Cu <= FREE_CHIP_CODE && XN_OBJECTS_HOME + XV_HOME_SIZE <= FREE_NAVI_CODE,
		"xnavi: the Navis' room out of order (the stubs, the sea's code, the moved objects, ..., the Dark Soul's records)");
	CHECK(xnavi_objects_ok(o, why, sizeof why), "xnavi: the moved objects' recipe refused: %s", why);
	CHECK(xnavi_recipe_ok(&xnavi_control, why, sizeof why), "xnavi: the control's recipe refused: %s", why);
	CHECK(xnavi_bn5_count == 7, "xnavi: BN5's Navis listed (%d: the six and the Dark Soul)", xnavi_bn5_count);
	for (int i = 0; i < xnavi_bn5_count; ++i) {
		const XNavi *n = xnavi_bn5[i];
		const XVirusFamily *f = &n->code;
		CHECK(xnavi_recipe_ok(n, why, sizeof why) && f->game == XROM_BN5_COLONEL_US, "xnavi: %s refused: %s", f->name, why);
		CHECK(f->ai == XN_AI_OF(f->src_ai) && f->first_id == XN_ID_OF(f->src_ai), "xnavi: %s off its fixed numbers", f->name);
		for (int e = 0; e < f->neffects; ++e)
			CHECK(f->effects[e].to >= 0xA0 && f->effects[e].to <= 0xBF, "xnavi: %s's effect %#x past the Navis' 0xA0-0xBF", f->name, f->effects[e].to);
		for (int k = 0; k < i; ++k) {
			const XNavi *m = xnavi_bn5[k];
			CHECK(!navi_clashes(f, &m->code, true) && !spans_meet((uint32_t)f->first_id, (uint32_t)n->versions, (uint32_t)m->code.first_id, (uint32_t)m->versions),
				"xnavi: %s and %s share numbers or room", m->code.name, f->name);
		}
		for (int k = 0; k < xvirus_bn5_count; ++k) CHECK(!navi_clashes(f, xvirus_bn5[k], false), "xnavi: %s and %s share numbers or room", f->name, xvirus_bn5[k]->name);
		CHECK(!navi_clashes(f, o, false), "xnavi: %s and the moved objects share numbers or room", f->name);
		for (int k = 0; k < n->nneeds; ++k) {
			bool moved = false;
			for (int t = 0; t < o->ntypes; ++t) moved = moved || (o->types[t].kind == n->needs[k].kind && o->types[t].from == n->needs[k].from);
			CHECK(moved, "xnavi: %s needs the type %d:%#x, which the moved objects do not move", f->name, n->needs[k].kind, n->needs[k].from);
		}
	}
	for (int k = 0; k < xvirus_bn5_count; ++k) CHECK(!navi_clashes(o, xvirus_bn5[k], false), "xnavi: the moved objects and %s share numbers or room", xvirus_bn5[k]->name);
	/* (the check sees a clash: a Navi against itself, its code, effects and types in one place twice) */
	CHECK(xnavi_bn5_count < 1 || navi_clashes(&xnavi_bn5[0]->code, &xnavi_bn5[0]->code, true) > 0, "xnavi: one Navi's room twice not seen");
}

/* The free space's plan (free_space.h; docs/EMULATION.md, Memory the
 * engine writes and BN6's RAM): every writer's region in order and apart,
 * inside the ROM copy past the game's 8 MB; each of issue #82's installers'
 * own layout inside its region (the families' room and the families, the
 * chip registry, the lava and sea); BN6's RAM the plan gives them, its parts
 * in order and apart, the pack's extension and the dispatch's side tables
 * where the plan puts them */
typedef struct { const char *what; uint32_t at, end; } Region;

/* each region not empty, inside [lo, hi), and past the one before it */
static void regions_apart(const char *plan, const Region *r, int n, uint32_t lo, uint32_t hi) {
	for (int i = 0; i < n; ++i)
		CHECK(r[i].at < r[i].end && r[i].at >= lo && r[i].end <= hi && (!i || r[i].at >= r[i - 1].end),
			"%s: %s %08x-%08x is empty, outside %08x-%08x or over %s", plan, r[i].what, (unsigned)r[i].at, (unsigned)r[i].end, (unsigned)lo,
			(unsigned)hi, i ? r[i - 1].what : "nothing");
}

static void test_free_space(void) {
	static const Region rom[] = {
		{ "a direct warp", FREE_WARP, FREE_CALL_RETURN }, { "a game call's return", FREE_CALL_RETURN, FREE_NO_MYSTERY },
		{ "the empty Mystery Data list", FREE_NO_MYSTERY, FREE_RECORDS }, { "the battle records", FREE_RECORDS, FREE_RECORDS_END },
		{ "Rush's gaps", FREE_RUSH, FREE_RUSH_END }, { "handler 3's records", FREE_BLOCKERS, FREE_BLOCKERS_END },
		{ "the layer's warp list", FREE_WARP_LIST, FREE_LAYER }, { "the layers' data", FREE_LAYER, FREE_TILE_MAP },
		{ "the tile map", FREE_TILE_MAP, FREE_COORDS }, { "the coordinate data", FREE_COORDS, FREE_TOWN_TILE_MAP },
		{ "the town's tile map", FREE_TOWN_TILE_MAP, FREE_TOWN_COORDS }, { "the town's coordinate data", FREE_TOWN_COORDS, FREE_TOWN },
		{ "the town's data", FREE_TOWN, FREE_LAN_HP }, { "Lan's HP's and house's", FREE_LAN_HP, FREE_TALK },
		{ "the conversations", FREE_TALK, FREE_LABEL }, { "the map-name label", FREE_LABEL, FREE_PET_LABEL },
		{ "the PET's PLACE", FREE_PET_LABEL, FREE_PET_TEXT }, { "the key items' names", FREE_PET_TEXT, FREE_PET_DESCS },
		{ "their descriptions", FREE_PET_DESCS, FREE_PET_MAILS }, { "the mails' senders and subjects", FREE_PET_MAILS, FREE_PET_BODIES },
		{ "the mails' bodies", FREE_PET_BODIES, FREE_PORTRAITS }, { "the portraits", FREE_PORTRAITS, FREE_PORTRAITS_END },
		{ "a Chip Trader's prizes of both nets", FREE_TRADER_LIST, FREE_TRADER_LIST_END },
		{ "another game's area", FREE_XAREA, FREE_SONGS }, { "the songs", FREE_SONGS, FREE_XNAVIS },
		{ "the Navis and map objects", FREE_XNAVIS, FREE_BACKDROPS }, { "the battle backgrounds", FREE_BACKDROPS, FREE_FLAME },
		{ "BN6's flame", FREE_FLAME, FREE_FLAME_END }, { "the net maps' backdrops", FREE_MAP_BACKDROPS, FREE_INDOORS },
		{ "home's other places", FREE_INDOORS, FREE_VIRUS_ROOM }, { "the families' room", FREE_VIRUS_ROOM, FREE_FAMILIES },
		{ "the families", FREE_FAMILIES, FREE_CHIPS }, { "the chip registry", FREE_CHIPS, FREE_SOULS },
		{ "the Souls' room", FREE_SOULS, FREE_PANELS }, { "the lava and sea", FREE_PANELS, FREE_PANELS_END },
		{ "the Navis' room", FREE_NAVIS, FREE_CHIP_CODE }, { "the chips' code's room", FREE_CHIP_CODE, FREE_TAIL },
	};
	CHECK(FREE_END == 0x08000000u + EMU_ROM_SIZE, "free space: its end %08x, not the copy's", (unsigned)FREE_END);
	regions_apart("the free space", rom, (int)(sizeof rom / sizeof *rom), EMU_FREE, FREE_END);
	/* (the families' room, xvirus_grow.c: BN6's tables grown and the second-level dispatch) */
	const Region room[] = {
		{ "the per-family tables", XV_GROWN, XV_GROWN + XV_TABLES * XV_STEP }, { "the ice sizes", XV_ICE, XV_ICE + XV_AIS },
		{ "the tables by AI index", XV_PER_AI, XV_PER_AI + 3 * XV_STEP }, { "the sprite lists", XV_LISTS, XV_LISTS + XV_LISTS_GROWN * XV_STEP },
		{ "the effects", XV_EFFECTS, XV_EFFECTS + 4u * XV_EFFECTS_MAX }, { "the kinds' types", XV_TYPES, XV_TYPES + 3 * XV_STEP },
		{ "the enemy ids", XV_IDS_AT, XV_IDS_AT + 3u * XV_IDS }, { "the reward rows", XV_ROWS, XV_ROWS + 2u * ENEMY_DROPS * XV_IDS },
		{ "the ids' names", XV_NAMES, XV_NAMES + XV_NAMES_ROOM },
		{ "MOVED", XV_MOVED_TABLES, XV_MOVED_TABLES + XV_DISPATCH_KINDS * XV_STEP },
		{ "TWIN", XV_TWIN_TABLES, XV_TWIN_TABLES + XV_DISPATCH_KINDS * 0x100u }, { "the dispatch's code", XV_STUBS, XV_STUBS_END },
	};
	CHECK(XV_AT == FREE_VIRUS_ROOM, "free space: the families' room at %08x", (unsigned)XV_AT);
	regions_apart("the families' room", room, (int)(sizeof room / sizeof *room), FREE_VIRUS_ROOM, FREE_FAMILIES);
	const Region fam[] = {
		{ "the homes", XV_HOMES, XV_HOMES + XV_FAMILIES * XV_HOME_SIZE }, { "the code", XV_CODE, XV_ART }, { "the art", XV_ART, XV_ART_END },
		{ "the arm points", XV_ARMS, XV_ARMS + XV_LIST_MAX * BN6_ARM_VIRUS_SIZE },
	};
	regions_apart("the families", fam, (int)(sizeof fam / sizeof *fam), FREE_FAMILIES, FREE_CHIPS);
	const Region chips[] = {
		{ "the records", CHIPS_RECORDS, CHIPS_RECORDS + CHIPS_IDS * BN6_CHIP_RECORD_SIZE },
		{ "the names, 0-255", CHIPS_NAMES0, CHIPS_NAMES0 + CHIPS_NAMES_ROOM }, { "the names, 256-511", CHIPS_NAMES1, CHIPS_NAMES1 + CHIPS_NAMES_ROOM },
		{ "the descriptions, 0-255", CHIPS_DESCS0, CHIPS_DESCS0 + CHIPS_DESCS_ROOM },
		{ "the descriptions, 256-511", CHIPS_DESCS1, CHIPS_DESCS1 + CHIPS_DESCS_ROOM },
		{ "the routines", CHIPS_CODE, CHIPS_CODE + 0x400u }, { "the art", CHIPS_ART, CHIPS_FREE_END },
	};
	CHECK(CHIPS_FREE == FREE_CHIPS, "free space: the chip registry at %08x", (unsigned)CHIPS_FREE);
	regions_apart("the chip registry", chips, (int)(sizeof chips / sizeof *chips), FREE_CHIPS, FREE_SOULS);
	const Region panels[] = {
		{ "the panels' flags", XP_FLAGS, XP_FLAGS + 16 * 4 }, { "the lava's cooling", XP_STUB, XP_TILES },
		{ "the tile table", XP_TILES, XP_TILES + XP_TILES_SIZE },
	};
	CHECK(XP_AT == FREE_PANELS && XPANEL_SEA + 1 == 15, "free space: the lava and sea at %08x, %d types", (unsigned)XP_AT, XPANEL_SEA + 1);
	regions_apart("the lava and sea", panels, (int)(sizeof panels / sizeof *panels), FREE_PANELS, FREE_PANELS_END);
	/* (BN6's RAM: persistent, then per battle) */
	static const Region ram[] = {
		{ "the pack's extension", RAM_PACK_EXT, RAM_PACK_EXT_END }, { "spare, persistent", RAM_PACK_EXT_END, RAM_PERSISTENT_END },
		{ "the sea's submerge", RAM_SEA, RAM_DARK_LISTS }, { "the chip AI's lists", RAM_DARK_LISTS, RAM_NAVIS },
		{ "the Navis' own state", RAM_NAVIS, RAM_DARK_AI }, { "the Dark Soul's AI data", RAM_DARK_AI, RAM_BATTLE },
		{ "the moved state", RAM_BATTLE, RAM_DISPATCH_SIDE }, { "the dispatch's side tables", RAM_DISPATCH_SIDE, RAM_COLONEL },
		{ "Colonel's table", RAM_COLONEL, RAM_SOULS }, { "the Souls' and the mood", RAM_SOULS, RAM_BATTLE_SPARE },
		{ "spare, per battle", RAM_BATTLE_SPARE, RAM_BATTLE_END },
	};
	regions_apart("BN6's RAM", ram, (int)(sizeof ram / sizeof *ram), 0x02010CDCu, 0x02040000u);
	CHECK(BN6_PACK_EXT == RAM_PACK_EXT && BN6_PACK_EXT + 12u * (CHIPS_IDS - BN6_PACK_IDS) == RAM_PACK_EXT_END,
		"BN6's RAM: the pack's extension %08x-%08x", (unsigned)BN6_PACK_EXT, (unsigned)(BN6_PACK_EXT + 12u * (CHIPS_IDS - BN6_PACK_IDS)));
	CHECK(BN6_DISPATCH_SIDE == RAM_DISPATCH_SIDE && BN6_DISPATCH_SIDE + XV_DISPATCH_KINDS * BN6_DISPATCH_SIDE_STEP == RAM_COLONEL,
		"BN6's RAM: the dispatch's side tables at %08x", (unsigned)BN6_DISPATCH_SIDE);
	CHECK(BN6_DARK_LISTS == RAM_DARK_LISTS && RAM_DARK_LISTS + 2u * BN5_DARK_LIST_SIZE == RAM_NAVIS && BN6_COLONEL_SIDES == RAM_NAVIS &&
		BN6_DARK_AI_OWNER > BN6_COLONEL_SIDES && BN6_DARK_AI_POOL + 4u <= RAM_DARK_AI && BN6_DARK_AI == RAM_DARK_AI &&
		RAM_DARK_AI + BN5_AI_DATA_SIZE <= RAM_BATTLE,
		"BN6's RAM: the chip AI's lists at %08x, two sides, then Colonel's sides' bits, the Dark Soul's AI data", (unsigned)RAM_DARK_LISTS);
	CHECK(XSEA_MASK == RAM_SEA && XSEA_MARKERS + 32u * 4u <= XSEA_RAM_END && XSEA_RAM_END <= RAM_DARK_LISTS,
		"BN6's RAM: the sea's submerge at %08x-%08x", (unsigned)RAM_SEA, (unsigned)XSEA_RAM_END);
	CHECK(BN6_SOULS == RAM_SOULS && BN6_SOULS + BN6_SOULS_SIZE <= RAM_BATTLE_SPARE && 2 * BN6_SOULS_SIDE <= BN6_SOULS_SIZE,
		"BN6's RAM: the Souls' state at %08x, %#x bytes", (unsigned)BN6_SOULS, (unsigned)BN6_SOULS_SIZE);
}

/* The installers' claims (claims.h): an installer's own bytes again, its
 * neighbour's and a gap between pass; another's over them, nested or after,
 * and a hook outside any installer on an installer's bytes are told */
static void test_claims(void) {
	claims_reset();
	claims_quiet(true);
	claims_begin("a");
	CHECK(claims_note(0x08900000u, 0x10) && claims_note(0x08900010u, 4) && claims_note(0x08900004u, 2), "claims: an installer's own bytes refused");
	claims_begin("b");
	CHECK(claims_note(0x08900020u, 8), "claims: a neighbour's bytes past the first's refused");
	CHECK(!claims_note(0x08900012u, 4), "claims: a nested installer over the first's bytes not told");
	claims_end();
	CHECK(claims_note(0x08900040u, 4) && claims_who(0x08900040u) && !strcmp(claims_who(0x08900040u), "a"),
		"claims: the first installer's writes after the nested one's not its own");
	claims_end();
	CHECK(claims_note(0x08900000u, 0x100), "claims: a write outside any installer claimed");
	CHECK(!claims_hook(0x08900003u) && claims_hook(0x08900030u), "claims: a hook on an installer's bytes not told, or one beside them refused");
	CHECK(claims_who(0x08900030u) && !strcmp(claims_who(0x08900030u), CLAIMS_HOOKS) && !claims_who(0x08900038u),
		"claims: whose a byte is (%s)", claims_who(0x08900030u) ? claims_who(0x08900030u) : "no one's");
	CHECK(claims_overlaps() == 2, "claims: %d overlaps told, not 2", claims_overlaps());
	claims_reset();
	claims_quiet(false);
	CHECK(!claims_overlaps() && !claims_who(0x08900000u), "claims: not forgotten");
}

/* The second-level dispatch's numbers and code (issue #82,
 * xvirus_dispatch.h): BN5's seven spawners each to the dispatch's of its
 * kind and place, BN6's own and any other routine to none; its tables and
 * side tables by kind; and its code as the project encodes it, each
 * literal load reaching the word it names (run on the CPU in
 * tests/test_emu.c) */
static uint32_t stub_literal(const uint8_t *code, uint32_t at) {
	uint16_t op = get16(code + (at - XV_STUBS));
	uint32_t lit = ((at + 4) & ~3u) + 4u * (op & 0xFFu);
	return (op & 0xF800) == 0x4800 && lit + 4 <= XV_STUBS_END ? get32(code + (lit - XV_STUBS)) : 0;
}

static void test_xvirus_dispatch(void) {
	static const struct { uint32_t bn5, to; int kind; uint32_t bn6; } spawners[7] = {
		{ BN5_SPAWN_1, XV_SPAWN_1, 1, BN6_SPAWN_1 }, { BN5_SPAWN_1_LAST, XV_SPAWN_1_LAST, 1, BN6_SPAWN_1_LAST },
		{ BN5_SPAWN_1_LAST2, XV_SPAWN_1_LAST, 1, BN6_SPAWN_1_LAST }, { BN5_SPAWN_3, XV_SPAWN_3, 3, BN6_SPAWN_3 },
		{ BN5_SPAWN_3_LAST, XV_SPAWN_3_LAST, 3, BN6_SPAWN_3_LAST }, { BN5_SPAWN_3_FIRST, XV_SPAWN_3_FIRST, 3, BN6_SPAWN_3_FIRST },
		{ BN5_SPAWN_4, XV_SPAWN_4, 4, BN6_SPAWN_4 },
	};
	for (int i = 0; i < 7; ++i) {
		int kind = 0;
		uint32_t bn6 = 0, to = xvirus_dispatch_spawner(XROM_BN5_COLONEL_US, spawners[i].bn5);
		CHECK(to == spawners[i].to && xvirus_dispatch_is_spawner(to, &kind, &bn6) && kind == spawners[i].kind && bn6 == spawners[i].bn6,
			"dispatch: BN5's spawner %08x to %08x (kind %d, BN6's %08x)", (unsigned)spawners[i].bn5, (unsigned)to, kind, (unsigned)bn6);
	}
	CHECK(!xvirus_dispatch_spawner(XV_GAME_BN6, BN6_SPAWN_3) && !xvirus_dispatch_spawner(XROM_BN5_COLONEL_US, BN6_SPAWN_3) &&
		!xvirus_dispatch_spawner(XROM_BN5_COLONEL_US, 0x08016C42), "dispatch: a spawner for BN6's own, or a routine of no spawn");
	CHECK(!xvirus_dispatch_is_spawner(XV_SPAWN_1 + 2, NULL, NULL) && !xvirus_dispatch_is_spawner(XV_STUB_NONE, NULL, NULL) &&
		!xvirus_dispatch_is_spawner(XV_SPAWN_1 + XV_SPAWNERS * XV_SPAWN_STEP, NULL, NULL), "dispatch: an address of no spawner taken for one");
	CHECK(xvirus_dispatch_type(1) == XV_T1_DISPATCH && xvirus_dispatch_type(3) == XV_T3_DISPATCH && xvirus_dispatch_type(4) == XV_T4_DISPATCH &&
		xvirus_dispatch_type(2) < 0, "dispatch: its types");
	CHECK(xvirus_dispatch_side(3) == BN6_DISPATCH_SIDE + BN6_DISPATCH_SIDE_STEP && xvirus_dispatch_moved_at(4, 0x10) ==
		XV_MOVED_TABLES + 2 * XV_STEP + 0x40 && xvirus_dispatch_twin_at(1, 0xFF) == XV_TWIN_TABLES + 0xFF && !xvirus_dispatch_twin_at(3, 0x100),
		"dispatch: its tables' places");
	/* (its places: the tables, then the code, short of the families' homes) */
	CHECK(XV_TWIN_TABLES >= XV_MOVED_TABLES + 3 * XV_STEP && XV_STUBS >= XV_TWIN_TABLES + 0x300 && XV_STUBS_END <= XV_HOMES &&
		XV_DISPATCH >= XV_NAMES + XV_NAMES_ROOM, "dispatch: its room");
	static uint8_t code[XV_STUBS_SIZE];
	xvirus_dispatch_code(code);
	CHECK(stub_literal(code, XV_STUB_NONE) == (BN6_OBJECT_FREE | 1) && get16(code + 2) == 0x4700, "dispatch: the stray's routine");
	for (int kind = 1; kind <= 4; ++kind) {
		uint32_t r = xvirus_dispatch_routine(kind);
		if (!r) continue;
		CHECK(get16(code + (r - XV_STUBS)) == 0x78E8 && stub_literal(code, r + 2) == xvirus_dispatch_side(kind) &&
			stub_literal(code, r + 8) == xvirus_dispatch_moved_at(kind, 0) && get16(code + (r + 0xC - XV_STUBS)) == 0x4700,
			"dispatch: kind %d's routine", kind);
	}
	for (int i = 0; i < 7; ++i) {
		uint32_t s = spawners[i].to;
		CHECK(get16(code + (s - XV_STUBS)) == 0xB501 && stub_literal(code, s + 2) == xvirus_dispatch_twin_at(spawners[i].kind, 0) &&
			get16(code + (s + 0xE - XV_STUBS)) == (0x2000 | (uint32_t)xvirus_dispatch_type(spawners[i].kind)) &&
			stub_literal(code, s + 0x10) == (spawners[i].bn6 | 1) && get16(code + (s + 0x12 - XV_STUBS)) == 0x46FE &&
			stub_literal(code, s + 0x1E) == xvirus_dispatch_side(spawners[i].kind) && get16(code + (s + 0x2A - XV_STUBS)) == 0xBD00,
			"dispatch: the spawner at %08x", (unsigned)s);
	}
}

/* The recipes tools/xrecipe.py writes (src/emu/xvirus_bn5.c, from the two
 * ROMs) against the ones the build installs (xvirus_families.c's xvirus_bn5,
 * made by hand and each compared frame by frame with BN5's own battle):
 * every field the same, the arrays whose order the install does not depend on
 * (data, calls, words, immediates, numbers, bytes, effects, types) as sets,
 * the rest (blocks, states, hooks, sprites, clones) in their order */
#define SAME_SET(A, NA, B, NB, EQ) same_set((A), (NA), (B), (NB), sizeof *(A), (EQ))
static bool same_set(const void *a, int na, const void *b, int nb, size_t size, bool (*eq)(const void *, const void *)) {
	if (na != nb) return false;
	for (int i = 0; i < na; ++i) {
		int in_a = 0, in_b = 0;
		for (int k = 0; k < na; ++k) in_a += eq((const char *)a + (size_t)i * size, (const char *)a + (size_t)k * size);
		for (int k = 0; k < nb; ++k) in_b += eq((const char *)a + (size_t)i * size, (const char *)b + (size_t)k * size);
		if (in_a != in_b) return false;
	}
	return true;
}
static bool data_eq(const void *x, const void *y) {
	const XvData *a = x, *b = y;
	return a->start == b->start && a->end == b->end && a->kind == b->kind;
}
static bool pair_eq(const void *x, const void *y) {
	const XvPair *a = x, *b = y;
	return a->from == b->from && a->to == b->to;
}
static bool imm_eq(const void *x, const void *y) {
	const XvImm *a = x, *b = y;
	return a->at == b->at && a->form == b->form && a->rd == b->rd && a->rn == b->rn && a->was == b->was && a->now == b->now;
}
static bool number_eq(const void *x, const void *y) {
	const XvNumber *a = x, *b = y;
	return a->at == b->at && a->was == b->was && a->now == b->now;
}
static bool byte_eq(const void *x, const void *y) {
	const XvByte *a = x, *b = y;
	return a->at == b->at && a->was == b->was && a->now == b->now;
}
static bool id_eq(const void *x, const void *y) {
	const XvId *a = x, *b = y;
	return a->from == b->from && a->to == b->to;
}
static bool type_eq(const void *x, const void *y) {
	const XvType *a = x, *b = y;
	return a->kind == b->kind && a->from == b->from && a->to == b->to;
}
static bool ref_eq(XvRef a, XvRef b) { return a.addr == b.addr && a.own == b.own && a.clone == b.clone; }

/* which field differs (NULL: none) */
static const char *recipe_differs(const XVirusFamily *g, const XVirusFamily *h) {
	if (strcmp(g->name, h->name) || g->game != h->game || g->src_ai != h->src_ai || g->ai != h->ai || g->first_id != h->first_id ||
		g->home != h->home)
		return "its numbers (name, game, AI indexes, ids, home)";
	if (g->nblocks != h->nblocks) return "its blocks";
	for (int i = 0; i < g->nblocks; ++i)
		if (memcmp(&g->blocks[i], &h->blocks[i], sizeof *g->blocks)) return "its blocks";
	if (!SAME_SET(g->data, g->ndata, h->data, h->ndata, data_eq)) return "its data";
	if (!SAME_SET(g->calls, g->ncalls, h->calls, h->ncalls, pair_eq)) return "its calls";
	if (!SAME_SET(g->words, g->nwords, h->words, h->nwords, pair_eq)) return "its words";
	if (!SAME_SET(g->imms, g->nimms, h->imms, h->nimms, imm_eq)) return "its immediates";
	if (!SAME_SET(g->numbers, g->nnumbers, h->numbers, h->nnumbers, number_eq)) return "its numbers";
	if (!SAME_SET(g->bytes, g->nbytes, h->bytes, h->nbytes, byte_eq)) return "its bytes";
	if (g->nstates != h->nstates) return "its states";
	for (int i = 0; i < g->nstates; ++i)
		if (!ref_eq(g->states[i], h->states[i])) return "its states";
	for (int i = 0; i < XV_HOOKS; ++i)
		if (!ref_eq(g->hooks[i], h->hooks[i])) return "its hooks";
	if (g->struct1 != h->struct1 || g->struct2 != h->struct2 || memcmp(g->names, h->names, sizeof g->names) || g->held != h->held ||
		g->ice != h->ice || g->traits.laid != h->traits.laid || g->traits.mode != h->traits.mode || g->traits.element != h->traits.element ||
		g->traits.weakness != h->traits.weakness || g->traits.shadow != h->traits.shadow)
		return "its records";
	if (!g->held_chips != !h->held_chips || (g->held_chips && memcmp(g->held_chips, h->held_chips, XV_VERSIONS * 16 * sizeof *g->held_chips)))
		return "its held chips";
	if (g->nsprites != h->nsprites) return "its sprites";
	for (int i = 0; i < g->nsprites; ++i)
		if (memcmp(&g->sprites[i], &h->sprites[i], sizeof *g->sprites)) return "its sprites";
	if (!SAME_SET(g->effects, g->neffects, h->effects, h->neffects, id_eq)) return "its effects";
	if (!SAME_SET(g->types, g->ntypes, h->types, h->ntypes, type_eq)) return "its types";
	if (g->nclones != h->nclones) return "its clones";
	for (int i = 0; i < g->nclones; ++i)
		if (memcmp(&g->clones[i], &h->clones[i], sizeof *g->clones)) return "its clones";
	if (g->nranges != h->nranges || g->ncmps != h->ncmps || g->nsongs != h->nsongs || g->prestep != h->prestep) return "its shims or songs";
	for (int i = 0; i < g->nranges; ++i)
		if (memcmp(&g->ranges[i], &h->ranges[i], sizeof *g->ranges)) return "its NameID ranges";
	for (int i = 0; i < g->ncmps; ++i)
		if (memcmp(&g->cmps[i], &h->cmps[i], sizeof *g->cmps)) return "its chip compares";
	for (int i = 0; i < g->nsongs; ++i)
		if (memcmp(&g->songs[i], &h->songs[i], sizeof *g->songs)) return "its songs";
	return NULL;
}

static void test_xvirus_generated(void) {
	static const XVirusFamily *const by_hand[] = { &xvirus_cangard, &xvirus_handi };
	int compared = 0;
	for (int i = 0; i < xvirus_bn5_count; ++i) {
		const XVirusFamily *g = xvirus_bn5[i];
		for (int k = 0; k < (int)(sizeof by_hand / sizeof *by_hand); ++k) {
			const XVirusFamily *h = by_hand[k];
			if (h->src_ai != g->src_ai) continue;
			const char *d = recipe_differs(g, h);
			CHECK(!d, "xrecipe: the generated %s is not the one made by hand: %s", g->name, d ? d : "");
			++compared;
		}
	}
	CHECK(compared == 2, "xrecipe: %d of the 2 families made by hand generated", compared);
	/* (the comparison sees a difference: CanGard's recipe with one call's twin otherwise) */
	XVirusFamily other = xvirus_cangard;
	XvPair calls[64];
	memcpy(calls, other.calls, (size_t)other.ncalls * sizeof *calls);
	calls[3].to += 2;
	other.calls = calls;
	CHECK(recipe_differs(&other, &xvirus_cangard) != NULL, "xrecipe: a call aimed otherwise not seen");
	other = xvirus_cangard;
	other.nimms -= 1;
	CHECK(recipe_differs(&other, &xvirus_cangard) != NULL, "xrecipe: an immediate missing not seen");
}

/* The families' shims (issue #82's A2b, xvirus_shim.h): a NameID range's
 * loads of BN6's bounds and its branch on to the routine's veneer; a chip
 * compare's stacked scratch register, its compare and the way on where the
 * branch is taken; the two pre-steps ported from BN5's dispatcher, each
 * ending on to BN6's state step; and each refused where it cannot be */
static uint32_t shim_lit(const uint8_t *code, uint32_t off) {
	uint16_t op = get16(code + off);
	return (op & 0xF800) == 0x4800 ? get32(code + ((off + 4) & ~3u) + 4u * (op & 0xFFu)) : 0xFFFFFFFFu;
}

static void test_xvirus_shims(void) {
	uint8_t r[XV_SHIM_RANGE_SIZE];
	const uint32_t home = XV_HOMES + XV_HOME_SHIMS, veneer = XV_HOMES + XV_HOME_VENEERS + 3 * THUMB_VENEER_SIZE;
	CHECK(xvirus_shim_range(r, home, 0x23C, 0x241, veneer), "shims: a NameID range refused");
	CHECK((get16(r) & 0xFF00) == 0x4900 && shim_lit(r, 0) == 0x23C && (get16(r + 2) & 0xFF00) == 0x4A00 && shim_lit(r, 2) == 0x241,
		"shims: a NameID range's bounds %#x %#x", shim_lit(r, 0), shim_lit(r, 2));
	CHECK((get16(r + 4) & 0xF800) == 0xE000 && home + 8 + 2 * ((get16(r + 4) & 0x7FFu)) == veneer, "shims: a NameID range's branch %04x",
		get16(r + 4));
	CHECK(!xvirus_shim_range(r, home + 2, 0, 1, veneer) && !xvirus_shim_range(r, home, 0, 1, home + 0x1000),
		"shims: a NameID range off its word, or its veneer out of a branch's reach, taken");
	uint8_t k[XV_SHIM_CMP_SIZE];
	const uint32_t to = XV_CODE + 0x1234;
	CHECK(xvirus_shim_cmp(k, home, 4, 0x1C8, true, to), "shims: a chip compare refused");
	CHECK(get16(k) == 0xB401 && shim_lit(k, 2) == 0x1C8 && get16(k + 4) == 0x4284 && get16(k + 6) == 0xBC01 && get16(k + 8) == 0xD103,
		"shims: a chip compare's compare (%04x %04x %04x %04x)", get16(k), get16(k + 4), get16(k + 6), get16(k + 8));
	CHECK(get16(k + 10) == 0xB401 && shim_lit(k, 12) == (to | 1) && get16(k + 14) == 0x4686 && get16(k + 16) == 0xBC01 &&
		get16(k + 18) == 0x4770, "shims: a chip compare's way on to %08x", shim_lit(k, 12));
	/* (a compare of r0 keeps r1 aside; bne's taken where the compare is not equal) */
	CHECK(xvirus_shim_cmp(k, home, 0, 0x100, false, to) && get16(k) == 0xB402 && get16(k + 4) == 0x4288 && get16(k + 8) == 0xD003,
		"shims: a compare of r0, or a bne's, made otherwise");
	CHECK(!xvirus_shim_cmp(k, home, 8, 0x100, true, to) && !xvirus_shim_cmp(k, home, 1, 0x100, true, to | 1),
		"shims: a high register, or a way on with its Thumb bit, taken");
	uint8_t p[XV_SHIM_PRESTEP_MAX];
	const uint32_t whiches[2] = { BN5_PRESTEP_APPLEY, BN5_PRESTEP_DOMINERD };
	for (int i = 0; i < 2; ++i) {
		int n = xvirus_shim_prestep(p, sizeof p, home, whiches[i], veneer, veneer + THUMB_VENEER_SIZE);
		CHECK(n > 8 && get32(p + n - 8) == (BN6_VIRUS_STATE_STEP | 1) && get32(p + n - 4) == 0x1FE, "shims: pre-step %08x (%d bytes)",
			(unsigned)whiches[i], n);
		int bx = n - 10;
		while (bx > 0 && get16(p + bx) != 0x4700) bx -= 2;
		CHECK(bx > 0 && shim_lit(p, (uint32_t)bx - 2) == (BN6_VIRUS_STATE_STEP | 1), "shims: pre-step %08x not on to BN6's state step",
			(unsigned)whiches[i]);
	}
	CHECK(!xvirus_shim_prestep(p, sizeof p, home, 0x08104550u, veneer, veneer) && !xvirus_shim_prestep(p, 16, home, BN5_PRESTEP_DOMINERD, veneer, veneer),
		"shims: a pre-step no port makes, or too little room, taken");
}

/* data.c's readers answer for an enemy id a family took (EnemyMoved), not
 * from the ROM, and from it again once let go */
static void test_enemy_moved(void) {
	EnemyMoved e = { { 2, 0, 32 }, { 0x96, 0x10, 2, 8, 0x3C, 0x00 }, { 4, 0x1E, 1, 0, 0x0E, 0, 0x80, 1 }, "CanRaid" };
	CHECK(enemy_moved_set(0x202, &e), "data: an enemy id moved");
	char name[20];
	enemy_name(0x202, name, sizeof name);
	int hp = 0, dmg = 0;
	CHECK(!strcmp(name, "CanRaid") && enemy_stats(0x202, &hp, &dmg) && hp == 150 && dmg == 60, "data: the moved id reads %s, %d HP, %d damage",
		name, hp, dmg);
	/* (its first wheel's element in the HP word's top: 1 Fire, beaten by Aqua; its second-wheel weakness bit 0x80 Sword unread then) */
	CHECK(enemy_element(0x202) == ELEM_FIRE && enemy_weakness(0x202) == ELEM_AQUA, "data: the moved id's element %d, weakness %d",
		enemy_element(0x202), enemy_weakness(0x202));
	const uint8_t *t = enemy_triple(0x202);
	CHECK(t && t[0] == 2 && t[2] == 32 && enemy_id(0, 32, 2) == 0x202, "data: the moved id's triple and its id");
	e.stats[1] = 0;   /* (no first-wheel element: the second wheel's) */
	enemy_moved_set(0x202, &e);
	CHECK(enemy_element(0x202) == ELEM_NULL && enemy_weakness(0x202) == ELEM_SWORD, "data: the moved id's second wheel");
	/* (with no ROM read, nothing then) */
	CHECK(enemy_moved_set(0x202, NULL) && (R.data || (!enemy_stats(0x202, &hp, &dmg) && enemy_triple(0x202) == NULL)), "data: the moved id let go");
	/* (a moved id's own reward row, as its install wrote it: lean_row reads
	 * it, not the player's ROM's, which gives the id another's; issue #82) */
	EnemyMoved a = e, b = e;
	a.drops_own = true;
	for (int k = 0; k < ENEMY_DROPS; ++k) a.drops[k] = (uint16_t)(0x4000 | (100 + k));
	b.triple[0] = 3;
	CHECK(enemy_moved_set(0x200, &a) && enemy_moved_set(0x203, &b), "data: two ids moved");
	CHECK(enemy_drop(0x200, 0) == 0x4064 && enemy_drop(0x200, 19) == 0x4077 && enemy_drop(0x200, 20) == 0xFFFF, "data: a moved id's own reward row (%04X)",
		enemy_drop(0x200, 0));
	CHECK(R.data || enemy_drop(0x203, 0) == 0xFFFF, "data: a moved id without its own row reads the ROM's (none here)");
	/* (the first let go: the last moved into its place, still found by its id) */
	CHECK(enemy_moved_set(0x200, NULL) && enemy_triple(0x203) && enemy_triple(0x203)[0] == 3 && enemy_drop(0x200, 0) != 0x4064,
		"data: an id let go, the other still read as moved");
	enemy_moved_set(0x203, NULL);
	/* (the ids a family may take: to 0x400; one from 0x200 no family took has no name) */
	CHECK(!enemy_moved_set(ENEMY_IDS_MAX, &a) && enemy_moved_set(ENEMY_IDS_MAX - 1, &a) && enemy_triple(ENEMY_IDS_MAX - 1), "data: the last id a family may take");
	enemy_moved_set(ENEMY_IDS_MAX - 1, NULL);
	name[0] = 'x';
	enemy_name(0x201, name, sizeof name);
	CHECK(!name[0] && !enemy_triple(0x201), "data: an id from 0x200 no family took (%s)", name);
}

/* The All * helper (docs/META.md, issue #18): every chip in *. The engine's
 * view of a record (chip_info, which every reward, stock and prize takes
 * its code from) holds * alone while the helper is on, and the record's own
 * codes otherwise; a folder entry goes to *; a pack entry's four counts go
 * to the first code's (the record's one, *), 99 at most, with an order stamp
 * where the first had none. Records in a made-up ROM. */
static void test_all_star(void) {
	static RomLayout layout;
	layout.chip_data = 0x1000;
	layout.chip_names[0] = layout.chip_names[1] = 0x2000;
	R.data = calloc(ROM_SIZE, 1);
	R.layout = &layout;
	chips_forget();   /* (the chips' image of another made-up ROM, maybe at the same address) */
	if (!R.data) { CHECK(false, "all star: no memory for a made-up ROM"); return; }
	static const uint8_t codes[3][4] = { { 0, 1, 2, 26 }, { 1, 3, 5, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF } };   /* Cannon's A B C *, B D F, none */
	for (int i = 0; i < 3; ++i) memcpy(R.data + 0x1000 + 0x2C * (i + 1), codes[i], 4);
	static const char *const own[3] = { "ABC*", "BDF", "" }, *const star[3] = { "*", "*", "" };
	uint8_t was = run.helpers;
	for (int on = 0; on < 2; ++on) {
		run.helpers = (uint8_t)(on ? HELP_ALL_STAR | HELP_GENTLE : HELP_GENTLE);
		for (int i = 0; i < 3; ++i) {
			ChipInfo ci;
			chip_info(i + 1, &ci);
			const char *want = on ? star[i] : own[i];
			CHECK(!strcmp(ci.codes, want) && ci.ncodes == (int)strlen(want), "all star %s: chip %d in \"%s\", not \"%s\"", on ? "on" : "off", i + 1,
				ci.codes, want);
		}
	}
	run.helpers = was;
	free(R.data);
	R.data = NULL;
	R.layout = NULL;
	CHECK(chip_entry_star(1 | 0 << 9) == (1 | 26 << 9) && chip_entry_star(72 | 18 << 9) == (72 | 26 << 9), "all star: a folder entry in *");
	CHECK(chip_entry_star(0xFFFF) == 0xFFFF && chip_entry_star(0) == 0 && chip_entry_star(1 | 26 << 9) == (1 | 26 << 9), "all star: an empty entry, or one in *, kept");
	uint8_t a[CHIP_PACK_ENTRY] = { 2, 1, 0, 3, 0x10, 0x7F, 0x20, 0x7F, 0, 0, 0x30, 0x7F };
	chip_pack_star(a);
	static const uint8_t a_want[CHIP_PACK_ENTRY] = { 6, 0, 0, 0, 0x10, 0x7F, 0, 0, 0, 0, 0, 0 };
	CHECK(!memcmp(a, a_want, sizeof a), "all star: a pack entry's counts in its first code's, its stamp kept (%d %d %d %d)", a[0], a[1], a[2], a[3]);
	uint8_t b[CHIP_PACK_ENTRY] = { 0, 0, 4, 0, 0, 0, 0, 0, 0x34, 0x12, 0, 0 };
	chip_pack_star(b);
	CHECK(b[0] == 4 && b[2] == 0 && b[4] == 0x34 && b[5] == 0x12 && !b[8] && !b[9], "all star: an empty first code takes the stamp of the code it takes");
	uint8_t c[CHIP_PACK_ENTRY] = { 60, 50, 0, 0 };
	chip_pack_star(c);
	CHECK(c[0] == 99 && !c[1], "all star: 99 at most (%d)", c[0]);
}

/* The BeastOut helper (issue #99, docs/META.md): shut until the endless
 * net's own Nest has fallen, a short net's Nest opening none; announced
 * once, on the summary of the run that opened it and NEW on the setup's
 * Help row, NEW too where a run given up for a NEW GAME opened it; in a
 * run, BeastOut from the Cybeast's fall, Dad's CybeastButton after it,
 * and the Graveyard's call unlocking nothing (issue #109); and the words
 * of a run that brought it: Dad's at its start, all of it until the
 * profile has heard them. */
static void test_beast(void) {
	Profile profile_was = profile;
	Run run_was = run;
	memset(&profile, 0, sizeof profile);
	memset(&run, 0, sizeof run);
	CHECK(!meta_beast_open() && meta_helpers_shown() == HELPER_BEAST, "beast: open on a new profile");
	profile.nest_clears = profile.short_wins = 1;
	CHECK(!meta_beast_open(), "beast: open after a short net's win alone");
	profile.nest_clears = 2;
	CHECK(meta_beast_open() && meta_helpers_shown() == HELPERS, "beast: shut after the endless Nest's fall");
	meta_unlocks_unsaid();
	CHECK((profile.beast & BEAST_OPEN) && (profile.setup_new & SETUP_NEW_HELP), "beast: a run given up opened it, not NEW");
	memset(&profile, 0, sizeof profile);
	profile.marks = MARK_NEST;
	CHECK(meta_beast_open(), "beast: shut with Bass's mark");
	/* (the run that opened it: the endless Nest cleared, then deleted
	 * deeper) */
	memset(&profile, 0, sizeof profile);
	profile.nest_clears = 1;
	run.mode = RUN_ENDLESS;
	run.depth = CYCLE_LAYERS + 4;
	for (int pass = 0; pass < 2; ++pass) {
		profile.setup_new = 0;
		meta_run_over(false);
		const char *said[6];
		int n = meta_unlocked(said, 6);
		bool beast = false;
		for (int i = 0; i < n; ++i) beast |= !strcmp(said[i], "the BeastOut start");
		bool fresh = (profile.setup_new & SETUP_NEW_HELP) != 0;
		CHECK(beast == !pass && fresh == !pass, "beast: summary %d says it %s, NEW %s", pass, beast ? "yes" : "no",
			profile.setup_new & SETUP_NEW_HELP ? "yes" : "no");
	}
	CHECK(profile.marks & MARK_NEST, "beast: no Bass's mark with it");
	/* the Graveyard's call stirs the beast and unlocks nothing, in a run
	 * with the helper or without (issue #109) */
	for (int helped = 0; helped < 2; ++helped) {
		run.helpers = helped ? HELP_BEAST : 0;
		for (int navi = 1; navi < 19; ++navi) {
			const char *words = powers_reward_text(navi, BIOME_GRAVEYARD, 15);
			char what[80];
			snprintf(what, sizeof what, "the Graveyard's call to a run %s BeastOut, navi %d", helped ? "with" : "without", navi);
			check_talk(what, words);
			CHECK(words && strstr(words, "calling to the Cybeast") && !strstr(words, "CybeastButton") && !strstr(words, "can now BeastOut"),
				"%s: \"%s\"", what, words ? words : "(none)");
		}
	}
	/* BeastOut from the Cybeast's fall, not the Graveyard's guardian's;
	 * Dad's word on it after the first Net's, unless the run brought it;
	 * and a headless start past the first Nest holding it */
	run.helpers = 0;
	nflags_set = 0;
	for (int navi = 1; navi < 19; ++navi) powers_after_boss(navi);
	CHECK(!flag_was_set(BN6_FLAG_BEAST_OUT), "beast: a guardian gave BeastOut");
	powers_after_boss(SUPER_CYBEAST);
	CHECK(flag_was_set(BN6_FLAG_BEAST_OUT), "beast: the Cybeast's fall gave no BeastOut");
	const char *den = powers_den_text(CYCLE_LAYERS);
	check_talk("Dad's CybeastButton after the Cybeast", den);
	CHECK(den && strstr(den, "CybeastButton") && strstr(den, "can now BeastOut"), "beast: no CybeastButton after the first Net's Cybeast");
	CHECK(!powers_den_text(2 * CYCLE_LAYERS), "beast: the CybeastButton unlocked again on the second Net");
	run.helpers = HELP_BEAST;
	CHECK(!powers_den_text(CYCLE_LAYERS), "beast: the CybeastButton unlocked in a run that brought it");
	run.helpers = 0;
	for (int depth = 1; depth <= CYCLE_LAYERS + 1; depth += CYCLE_LAYERS) {
		run.depth = depth;
		nflags_set = 0;
		powers_bring();
		CHECK(flag_was_set(BN6_FLAG_BEAST_OUT) == (depth > CYCLE_LAYERS), "beast: a start on layer %d %s it", depth, depth > CYCLE_LAYERS ? "without" : "with");
	}
	/* the words of a run that brought it */
	run.helpers = HELP_BEAST;
	profile.seen_intro = true;
	profile.runs = 5;
	for (int k = 0; k < 4; ++k) {
		profile.beast = k & 1 ? BEAST_TAUGHT : 0;
		profile.nest_clears = k & 2 ? 1 : 0;   /* (Dad's call about the Nest first, on an odd run) */
		const char *words = town_intro(NULL);
		char what[64];
		snprintf(what, sizeof what, "a run's start with BeastOut, %s, %s", k & 1 ? "taught" : "first", k & 2 ? "Dad's call before" : "alone");
		check_talk(what, words);
		CHECK(strstr(words, "CybeastButton") != NULL, "%s: no word of it: \"%s\"", what, words);
		CHECK(!!strstr(words, "@D Lan,it's Dad. I've unlocked") == (k == 0), "%s: Dad's call: \"%s\"", what, words);
	}
	run.helpers = 0;
	CHECK(!strstr(town_intro(NULL), "CybeastButton"), "beast: a run's start without it names the CybeastButton");
	profile = profile_was;
	run = run_was;
}

/* Another game's chips as BN6's (src/core/xchips.c; docs/MULTIROM.md,
 * Guest battles), on made-up tables: paired by name each way (a name the
 * other game gives two chips comes back from either; a chip it lacks, or
 * a nameless one, pairs none); a code re-read as the other game's chip of
 * that name has it, the same letter, else its *, else its first (a
 * playtester's Thunder S went in as BN5's Thunder *, which has B, L, P
 * and *); the folder's chips that sit out, each once, in its order. */
static void test_xchips(void) {
	static const char names6[6][XCHIP_NAME] = { "", "Cannon", "WhiCapsl", "Sword", "", "HiCannon" };
	static const char namesx[6][XCHIP_NAME] = { "", "Sword", "Cannon", "DarkSwrd", "Cannon", "" };
	uint16_t to_x[6], from_x[6];
	int paired = xchips_pair(names6, 6, namesx, 6, to_x, from_x);
	CHECK(paired == 2, "xchips: two of BN6's chips pair by name (%d)", paired);
	CHECK(to_x[1] == 2 && to_x[3] == 1 && !to_x[0] && !to_x[2] && !to_x[4] && !to_x[5], "xchips: Cannon as the other's 2 (its first), Sword as its 1, "
		"WhiCapsl, HiCannon and a nameless chip as none (%d %d %d %d %d)", to_x[1], to_x[3], to_x[2], to_x[4], to_x[5]);
	CHECK(from_x[1] == 3 && from_x[2] == 1 && from_x[4] == 1 && !from_x[0] && !from_x[3] && !from_x[5], "xchips: back, both Cannons as BN6's Cannon, "
		"DarkSwrd and a nameless chip as none (%d %d %d %d %d)", from_x[1], from_x[2], from_x[4], from_x[3], from_x[5]);
	static const uint8_t cannon[4] = { 0, 1, 2, 26 }, thunder[4] = { 1, 11, 15, 26 }, two[4] = { 3, 5, 0xFF, 0xFF },
		star[4] = { 26, 0xFF, 0xFF, 0xFF }, none[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
	CHECK(xchips_code(cannon, 1) == 1 && xchips_code(cannon, 26) == 26 && xchips_code(two, 5) == 5, "xchips: a code the record has kept");
	CHECK(xchips_code(cannon, 18) == 26 && xchips_code(thunder, 18) == 26, "xchips: a code it lacks as its * (Cannon S, Thunder S: %d %d)",
		xchips_code(cannon, 18), xchips_code(thunder, 18));
	CHECK(xchips_code(two, 18) == 3, "xchips: without a *, as its first code (%d)", xchips_code(two, 18));
	CHECK(xchips_code(star, 0) == 26, "xchips: the All * helper's record, every code in * (%d)", xchips_code(star, 0));
	CHECK(xchips_code(none, 7) == 7, "xchips: a record of no codes leaves the code (%d)", xchips_code(none, 7));
	/* (WhiCapsl twice, HiCannon, an empty slot both ways, and an id past
	 * the table: three sit out, the first two named) */
	const uint16_t folder[8] = { 1 | 0 << 9, 2 | 26 << 9, 2 | 26 << 9, 5 | 7 << 9, 0xFFFF, 0, 400 | 1 << 9, 3 | 4 << 9 };
	uint16_t out[2] = { 0, 0 };
	int n = xchips_out(folder, 8, to_x, 6, out, 2);
	CHECK(n == 3 && out[0] == 2 && out[1] == 5, "xchips: three of the folder's chips sit out, WhiCapsl and HiCannon first (%d: %d %d)", n, out[0], out[1]);
	CHECK(!xchips_out(folder, 1, to_x, 6, out, 2), "xchips: a folder of paired chips has none out");
	/* (a reward in the first of the folder's S, A and B both records
	 * have, on odd rows only: Cannon in A, Thunder in B, a chip with none
	 * of them as it was) */
	const uint8_t sab[3] = { 19, 1, 2 }, cannon6[4] = { 0, 1, 2, 26 };
	CHECK(xchips_fit(cannon, cannon6, sab, 2, 1) == 0, "xchips: an odd row's Cannon in the folder's A (%d)", xchips_fit(cannon, cannon6, sab, 2, 1));
	CHECK(xchips_fit(cannon, cannon6, sab, 2, 2) == 2, "xchips: an even row's Cannon as it was (%d)", xchips_fit(cannon, cannon6, sab, 2, 2));
	CHECK(xchips_fit(thunder, cannon6, sab, 11, 3) == 1, "xchips: Thunder in B, which both have (%d)", xchips_fit(thunder, cannon6, sab, 11, 3));
	CHECK(xchips_fit(two, cannon6, sab, 3, 5) == 3, "xchips: no code of the folder in both: as it was (%d)", xchips_fit(two, cannon6, sab, 3, 5));
	const uint8_t nofolder[3] = { 0, 0, 0 };
	CHECK(xchips_fit(cannon, cannon6, nofolder, 2, 1) == 2, "xchips: a folder of no codes leaves it (%d)", xchips_fit(cannon, cannon6, nofolder, 2, 1));
	/* (BN5's names in BN6's text codes first, rom.h's xrom_text_bn6: its
	 * M-Cannon's dash is 0x93, which BN6's table reads as nothing, and its
	 * Roll DS's mark 0x9F, which BN6 has no glyph for; issue #82) */
	static const uint8_t mcannon5[] = { 0x17, 0x93, 0x0D, 0x26, 0x33, 0x33, 0x34, 0x33, 0xE6 }, rollds5[] = { 0x1C, 0x34, 0x31, 0x31, 0x9F, 0xE6 };
	static char names5[3][XCHIP_NAME];
	uint8_t bn6[32];
	rom_text_bytes(bn6, xrom_text_bn6(XROM_BN5_COLONEL_US, mcannon5, sizeof mcannon5, bn6, sizeof bn6), names5[1], XCHIP_NAME);
	rom_text_bytes(bn6, xrom_text_bn6(XROM_BN5_COLONEL_US, rollds5, sizeof rollds5, bn6, sizeof bn6), names5[2], XCHIP_NAME);
	static const char names6b[3][XCHIP_NAME] = { "", "M-Cannon", "Roll" };
	uint16_t to5[3], from5[3];
	CHECK(xchips_pair(names6b, 3, (const char (*)[XCHIP_NAME])names5, 3, to5, from5) == 1 && to5[1] == 1 && !to5[2] && !from5[2],
		"xchips: BN5's M-Cannon pairs with BN6's, its Roll DS (%s) not with Roll", names5[2]);
	/* (a reward entry of BN5's as BN6's: no chip paired without the ROMs,
	 * the stand-in's zenny; zenny, BugFrags and none as they are) */
	CHECK(xchips_reward_bn6(0x0001) == (0x4000 | XCHIPS_STAND_IN) && xchips_reward_bn6(0x43E8) == 0x43E8 && xchips_reward_bn6(0xC003) == 0xC003 &&
		xchips_reward_bn6(0xFFFF) == 0xFFFF, "xchips: BN5's rewards as BN6's without a pairing");
}

/* BN5's text in BN6's codes (rom.h, xrom_text_bn6; docs/ROM_DATA.md,
 * BN5's text): BN6 put its five version marks at 0x40-0x44, so BN5's codes
 * from 0x40 are five on, its marks and punctuation where BN6 has their
 * glyphs, its DS mark as the letters D and S, a glyph BN6 lacks left out;
 * letters and digits the same. Both fonts compared glyph for glyph. */
static void test_xtext(void) {
	static const struct { uint8_t in[8]; int n; const char *ascii; } cases[] = {
		{ { 0x17, 0x93, 0x0D, 0x26, 0x33, 0x33, 0x34, 0x33 }, 8, "M-Cannon" },   /* (its dash) */
		{ { 0x18, 0x26, 0x3B, 0x2E, 0x99, 0x03, 0x01 }, 7, "Navi+20" },          /* (its +, BN6's 0x9E; read as BN6's 0x99 it was an x) */
		{ { 0x1C, 0x34, 0x31, 0x31, 0x9E }, 5, "Roll\x04" },                     /* (its SP mark, BN6's 0x43) */
		{ { 0x1C, 0x34, 0x31, 0x31, 0x9F }, 5, "RollDS" },                        /* (its DS mark, which BN6 lacks) */
		{ { 0x0B, 0xB0, 0x0C }, 3, "AB" },                                        /* (its omega, which BN6 lacks) */
		{ { 0x0B, 0x98, 0x0C, 0xE6, 0x0D }, 5, "A?B" },                           /* (its ?, and the end at a command) */
	};
	for (size_t i = 0; i < sizeof cases / sizeof *cases; ++i) {
		uint8_t bn6[16];
		char ascii[24];
		size_t n = xrom_text_bn6(XROM_BN5_COLONEL_US, cases[i].in, (size_t)cases[i].n, bn6, sizeof bn6);
		rom_text_bytes(bn6, n, ascii, sizeof ascii);
		CHECK(!strcmp(ascii, cases[i].ascii), "xtext: BN5's case %d reads %s, not %s", (int)i, ascii, cases[i].ascii);
	}
	const uint8_t kana[] = { 0x40, 0x92, 0x94, 0xA0, 0xAD };
	uint8_t out[8];
	CHECK(xrom_text_bn6(XROM_BN5_COLONEL_US, kana, sizeof kana, out, sizeof out) == 5 && out[0] == 0x45 && out[1] == 0x97 && out[2] == 0x99 &&
		out[3] == 0xA3 && out[4] == 0xB0, "xtext: BN5's codes past BN6's marks, five and three on (%02X %02X %02X %02X %02X)", out[0], out[1],
		out[2], out[3], out[4]);
	/* (its room kept: the DS mark's two letters at its end, one of them) */
	const uint8_t ds[] = { 0x0B, 0x9F };
	CHECK(xrom_text_bn6(XROM_BN5_COLONEL_US, ds, sizeof ds, out, 2) == 2 && out[1] == 0x0E, "xtext: the room for its text kept");
	/* (BN6's own text, and another game's that spells as BN6's: as it is) */
	CHECK(xrom_text_bn6(XROM_COUNT, kana, sizeof kana, out, sizeof out) == 5 && out[0] == 0x40, "xtext: text of no game BN6's numbers differ from");
}

/* The chip registry (src/core/xchips_registry.c; docs/MULTIROM.md, The
 * chip registry, issue #82): the table as decided (Team Colonel's chips a
 * run can meet, one chip per name, the rest in BN6's free ids), its checks
 * failing on made-up rows that break each rule, and its lookups both ways. */
static void test_chip_registry(void) {
	char why[96] = "";
	int bad = xreg_check(xreg_rows, xreg_count, why, sizeof why);
	CHECK(!bad, "registry: the table holds its rules (%s)", why);
	int own = 0, plays = 0, dark = 0, pas = 0, high = 0;
	for (int i = 0; i < xreg_count; ++i) {
		own += xreg_own(&xreg_rows[i]);
		plays += xreg_rows[i].how == XREG_RECORD;
		dark += xreg_rows[i].how == XREG_DARK;
		pas += xreg_rows[i].how == XREG_PA;
		high += xreg_own(&xreg_rows[i]) && xreg_rows[i].bn6 >= 320;
	}
	CHECK(xreg_count == 257 && own == 138 && plays == 12 && dark == 7 && pas == 15, "registry: 257 chips, 138 of ids of their own, 12 playing now, "
		"7 DarkChips, 15 Program Advances (%d %d %d %d %d)", xreg_count, own, plays, dark, pas);
	CHECK(high == 15 + 94, "registry: the Program Advances and 94 chips past 319, in the pack's extension (%d)", high);
	const XChipReg *ws = xreg_of_bn5(9), *cannon = xreg_of_bn5(1), *atk = xreg_of_bn5(149), *z = xreg_of_bn5(218);
	CHECK(ws && ws->bn6 == 204 && ws->how == XREG_RECORD && ws->twin == 23 && xreg_at(204) == ws, "registry: BN5's WideSht1 is 204, over BN6's WideSht");
	CHECK(cannon && cannon->bn6 == 1 && cannon->how == XREG_SAME && !xreg_at(1), "registry: BN5's Cannon is BN6's own");
	CHECK(atk && atk->bn6 == 192 && atk->how == XREG_ALIAS, "registry: Attck+10 is BN6's Atk+10, which BN6's hand merges by its id");
	CHECK(z && xreg_own(z) && z->twin == 381, "registry: Z Saver an id of its own over BN6's pseudo-chip of its name, which no folder holds");
	CHECK(!xreg_of_bn5(215) && !xreg_of_bn5(310) && !xreg_of_bn5(278) && !xreg_of_bn5(313), "registry: Otenko, BugCharg, Django and Phoenix out");
	CHECK(xreg_free_id(203) && xreg_free_id(511) && !xreg_free_id(221) && !xreg_free_id(368) && !xreg_free_id(410) && !xreg_free_id(512),
		"registry: the free ids, BN6's own and its pseudo-chips' not");
	/* (made-up rows, each breaking one rule) */
	static const XChipReg twice[] = { { 9, 204, XREG_RECORD, 23, 0xFF, { 0 }, 0, 0 }, { 9, 205, XREG_PENDING, 0, 0xFF, { 0 }, 0, 0 } };
	static const XChipReg taken[] = { { 12, 221, XREG_PENDING, 0, 0xFF, { 0 }, 0, 0 } };
	static const XChipReg two_at[] = { { 12, 411, XREG_PENDING, 0, 0xFF, { 0 }, 0, 0 }, { 13, 411, XREG_PENDING, 0, 0xFF, { 0 }, 0, 0 } };
	static const XChipReg no_twin[] = { { 9, 204, XREG_RECORD, 0, 0xFF, { 0 }, 0, 0 } };
	static const XChipReg pseudo[] = { { 218, 381, XREG_SAME, 0, 0xFF, { 0 }, 0, 0 } };
	static const XChipReg pa[] = { { 323, 350, XREG_PA, 0, 0xFF, { 0 }, 0, 5 } };
	CHECK(xreg_check(twice, 2, why, sizeof why) == 2 && xreg_check(taken, 1, why, sizeof why) == 1 && xreg_check(two_at, 2, why, sizeof why) == 2 &&
		xreg_check(no_twin, 1, why, sizeof why) == 1 && xreg_check(pseudo, 1, why, sizeof why) == 1 && xreg_check(pa, 1, why, sizeof why) == 1,
		"registry: a BN5 id twice, an id BN6 uses, an id twice, a playing record without its twin, a pseudo-chip's pair, a PA of no PA type: refused");
}

/* A registered chip's record (src/core/xchips_record.c): BN5's numbers,
 * BN6's icon element, its twin's behaviour, out of BN6's Library; a
 * placeholder's; the folder editor's ABC place; BN5's icons' colours; the
 * battle deck without the chips that sit out. Made-up records. */
static void test_chip_records(void) {
	static const int icon[][2] = { { 0, 0 }, { 3, 3 }, { 4, 10 }, { 5, 4 }, { 6, 5 }, { 7, 10 }, { 8, 6 }, { 9, 7 }, { 10, 8 }, { 11, 9 }, { 12, 10 }, { 13, 11 }, { 40, 10 } };
	for (size_t i = 0; i < sizeof icon / sizeof *icon; ++i)
		CHECK(xrec_icon_element(icon[i][0]) == icon[i][1], "records: BN5's icon element %d as BN6's %d (%d)", icon[i][0], icon[i][1], xrec_icon_element(icon[i][0]));
	uint8_t bn5[XREC_SIZE] = { 0x0B, 0x0C, 0x0D, 0xFF, 2, 0, 1, 0, 10, 0x4A, 0x14, 0x25, 0, 0, 4, 0, 3, 0, 0, 0, 0, 0, 0, 22 }, twin[XREC_SIZE] = { 0 }, out[XREC_SIZE];
	bn5[BN6_CHIP_ATTACK_POWER] = 70;
	twin[BN6_CHIP_FAMILY] = 0x30;
	twin[BN6_CHIP_LOCK_ON] = 1;
	twin[BN6_CHIP_LOCK_ON_TYPE] = 3;
	XChipReg reg = { 9, 204, XREG_RECORD, 23, 0xFF, { 0x0B, 0x0C, 0x0D, 0xFF }, 0, 10 };
	XRecordFrom in = { bn5, twin, &reg, 509, { 0x08C11000u, 0x08C20000u, 0x08C30000u } };
	xrec_from_bn5(&in, out);
	CHECK(!memcmp(out, bn5, 4) && out[BN6_CHIP_FAMILY] == 0x30 && out[BN6_CHIP_SUBFAMILY] == 0 && out[BN6_CHIP_LOCK_ON] == 1 && out[BN6_CHIP_LOCK_ON_TYPE] == 3 &&
		out[BN6_CHIP_EFFECT_FLAGS] == 0x0A && out[BN6_CHIP_PARAMS] == 3 && get16(out + BN6_CHIP_ATTACK_POWER) == 70 && out[BN6_CHIP_LIBRARY_NUMBER] == 0 &&
		get16(out + BN6_CHIP_ALPHA_SORT) == 509 && get16(out + BN6_CHIP_ID_SORT) == 204 && out[BN6_CHIP_DARK_ID] == 0xFF && get32(out + BN6_CHIP_ICON_PTR) == 0x08C11000u &&
		get32(out + BN6_CHIP_PALETTE_PTR) == 0x08C30000u && out[BN6_CHIP_ELEMENT] == 1, "records: WideSht1 over WideSht's family with BN5's numbers, out of the Library");
	reg.sub = 2;
	xrec_from_bn5(&in, out);
	CHECK(out[BN6_CHIP_SUBFAMILY] == 2, "records: a subfamily of the row's own (DarkWide's)");
	in.twin = NULL;
	put16(bn5 + BN6_CHIP_ATTACK_POWER, 1045);
	bn5[BN6_CHIP_EFFECT_FLAGS] = 0xDA;
	xrec_from_bn5(&in, out);
	CHECK(out[BN6_CHIP_FAMILY] == XREC_INERT_FAMILY && out[BN6_CHIP_SUBFAMILY] == XREC_INERT_SUB && !out[BN6_CHIP_LOCK_ON] && !out[BN6_CHIP_PARAMS] &&
		!get16(out + BN6_CHIP_ATTACK_POWER) && out[BN6_CHIP_EFFECT_FLAGS] == 0x18, "records: one whose code BN6 lacks inert, its computed power (BN5's) none");
	const uint32_t blank[3] = { 0x087237D0u, 0x086F3AF0u, 0x086F3AF0u };
	XChipReg dark = { 187, 214, XREG_DARK, 142, 0xFF, { 0x11, 0xFF, 0xFF, 0xFF }, 0, 99 };
	xrec_placeholder(&dark, 33, blank, out);
	CHECK(out[0] == 0x11 && out[1] == 0xFF && out[BN6_CHIP_MB] == 99 && out[BN6_CHIP_EFFECT_FLAGS] == BN6_CHIP_DARK_CLASS && !get16(out + BN6_CHIP_ATTACK_POWER) &&
		out[BN6_CHIP_FAMILY] == XREC_INERT_FAMILY && get32(out + BN6_CHIP_ICON_PTR) == 0x087237D0u && get16(out + BN6_CHIP_ID_SORT) == 214 &&
		out[BN6_CHIP_ELEMENT] == 10, "records: a DarkChip's placeholder keeps its codes, MB and class, BN6's blank art");
	const char *names[] = { "Cannon", "AirShot", "WideSwrd", "", "Zeta" };
	const uint16_t places[] = { 52, 5, 509, 7, 0 };
	CHECK(xrec_abc_place("WideSht1", names, places, 5) == 52 && xrec_abc_place("WideSwrd2", names, places, 5) == 509 &&
		xrec_abc_place("Aaa", names, places, 5) == 1 && xrec_abc_place("Zz", names, places, 5) == 509, "records: the ABC place after the name before");
	uint8_t pal5[32] = { 0 }, pal6[32] = { 0 }, map[16];
	put16(pal6 + 2 * 3, 0x001F);   /* (BN6's 3 red, its others black) */
	put16(pal5 + 2 * 12, 0x001E);  /* (BN5's 12 near red) */
	xrec_icon_colours(pal5, pal6, map);
	CHECK(map[0] == 0 && map[9] == 9 && map[12] == 3, "records: BN5's icon colours 0-9 as they are, 12 as BN6's nearest (%d %d)", map[9], map[12]);
	uint8_t ic[0x80];
	memset(ic, 0xC9, sizeof ic);
	xrec_icon_recolour(ic, map);
	CHECK(ic[0] == 0x39 && ic[0x7F] == 0x39, "records: an icon's two pixels a byte recoloured (%02X)", ic[0]);
	uint8_t sits[64] = { 0 };
	sits[204 >> 3] |= 1u << (204 & 7);
	uint16_t deck[8] = { 1, 204 | 11 << 9, 5, 204 | 11 << 9, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
	unsigned first;
	int taken = xrec_deck_sit_out(deck, 8, sits, 3, &first);
	CHECK(taken == 2 && deck[0] == 1 && deck[1] == 5 && deck[2] == 0xFFFF && first == 2u, "records: two chips sit out of the deck, the second of its first "
		"three among them, the rest in order (%d %04X %04X %u)", taken, deck[1], deck[2], first);
}

/* The chips' image (src/core/chips.c) over a made-up BN6 ROM without BN5's:
 * BN6's records and names as they are, the registry's ids as placeholders
 * (their codes, "BN5-9", sitting out), 512 records, names of ids past
 * BN6's archives; and a BN5 description's script in BN6's codes, its
 * commands kept (rom.c, xrom_script_bn6). */
static void test_chips_image(void) {
	static RomLayout layout;
	layout.chip_data = 0x1000;
	layout.chip_names[0] = 0x8000;
	layout.chip_names[1] = 0x9000;
	layout.chip_descs[0] = 0xA000;
	layout.chip_descs[1] = 0xB000;
	R.data = calloc(ROM_SIZE, 1);
	R.layout = &layout;
	chips_forget();
	if (!R.data) { CHECK(false, "chips: no memory for a made-up ROM"); return; }
	uint8_t *cannon = R.data + 0x1000 + XREC_SIZE;
	cannon[0] = 0; cannon[1] = 1; cannon[2] = 2; cannon[3] = 26;
	put16(cannon + BN6_CHIP_ATTACK_POWER, 40);
	/* (names: two entries, "Cannon" for 1) */
	static const uint8_t names0[] = { 4, 0, 5, 0, 0xE6, 0x0D, 0x26, 0x33, 0x33, 0x34, 0x33, 0xE6 };
	memcpy(R.data + 0x8000, names0, sizeof names0);
	ChipInfo ci;
	chip_info(1, &ci);
	CHECK(!strcmp(ci.name, "Cannon") && !strcmp(ci.codes, "ABC*") && ci.power == 40, "chips: BN6's Cannon as its ROM has it (%s %s %d)", ci.name, ci.codes, ci.power);
	chip_info(204, &ci);
	char desc[64];
	chip_desc(204, desc, sizeof desc);
	CHECK(!strcmp(ci.name, "BN5-9") && !strcmp(ci.codes, "LMN") && ci.mb == 10 && chips_sits_out(204) && !chips_sits_out(1) && chips_bn5(204) && !chips_from_bn5(),
		"chips: BN5's WideSht1 without its ROM a placeholder of its codes, sitting out (%s %s %d)", ci.name, ci.codes, ci.mb);
	CHECK(!strcmp(desc, "Needs BN5 ROM back to battle"), "chips: the placeholder's description (%s)", desc);
	CHECK(chips_record(511) && !chips_record(512) && get16(chips_record(204) + BN6_CHIP_ID_SORT) == 204, "chips: 512 records, the registry's its id's sort place");
	chip_info(509, &ci);
	CHECK(!ci.name[0], "chips: an id past BN6's archives and the registry's has no name (%s)", ci.name);
	CHECK(chips_bytes(0x08001000u + XREC_SIZE, 4) == cannon && !chips_bytes(0x0BFFFFFFu, 4) && chips_bytes(CHIPS_RECORDS, XREC_SIZE) == chips_record(0),
		"chips: bytes by bus address, the player's ROM below 8 MB, the chips' place past EMU_FREE");
	free(R.data);
	R.data = NULL;
	R.layout = NULL;
	chips_forget();
	CHECK(!chips_record(1), "chips: none without a ROM");
	static const uint8_t script[] = { 0xE8, 0x06, 0x01, 0x01, 0xF1, 0x00, 0x00, 0x17, 0x93, 0x0D, 0xE9, 0x0B, 0x9E, 0xE7, 0x01, 0xE6, 0xE8, 0x07 };
	uint8_t out[32];
	size_t n = xrom_script_bn6(XROM_BN5_COLONEL_US, script, sizeof script, out, sizeof out);
	static const uint8_t want[] = { 0xE8, 0x06, 0x01, 0x01, 0xF1, 0x00, 0x00, 0x17, 0x98, 0x0D, 0xE9, 0x0B, 0x43, 0xE7, 0x01, 0xE6 };
	CHECK(n == sizeof want && !memcmp(out, want, sizeof want), "chips: a BN5 description's script in BN6's codes, its commands kept, to its end (%d)", (int)n);
	char text[32];
	rom_desc_bytes(want, sizeof want, text, sizeof text);
	CHECK(!strcmp(text, "M-C A\x04"), "chips: the description read past its box's commands (%s)", text);
}

/* BN6's chips as BN5's UNITE reads them (src/emu/soulkinds.c; the Souls
 * track's S2): BN6's eleven kinds as BN5 numbers the kind of their name
 * (BN6 has neither Recovery nor Invisible; none is BN5's 12), and nothing
 * before both ROMs are read. */
static void test_soulkinds(void) {
	static const int as5[11] = { 0, 1, 2, 3, 5, 6, 8, 9, 10, 11, SOULKIND_NONE };   /* (Fire .. Break, none) */
	for (int k = 0; k < 11; ++k)
		CHECK(soulkinds_bn5(k) == as5[k], "soulkinds: BN6's kind %d as BN5's %d (%d)", k, as5[k], soulkinds_bn5(k));
	CHECK(soulkinds_bn5(11) == SOULKIND_NONE && soulkinds_bn5(12) == SOULKIND_NONE && soulkinds_bn5(-1) == SOULKIND_NONE,
		"soulkinds: BN6's Program Advances and others (11, 12) as none");
	CHECK(!soulkinds_make() && soulkinds_of(140) == SOULKIND_NONE && !soulkinds_dark(290), "soulkinds: nothing made without both ROMs");
}

/* The run's DarkChips (src/director/darkchips.c; docs/META.md, BN6's own
 * DarkChips, issue #70): BN6's five as BN5's kinds of their names and back
 * (DarkPlus out of BN6's battles); a flame's pick, BN5's as it was, BN6's
 * among the four its battles play and never one held; the flame a layer
 * was made with kept for a CONTINUE on it, its kind taken by then; the
 * price on max HP, its floor and the HP held to the new max. */
static void test_darkchips(void) {
	static const int kinds[DARK_BN6_COUNT] = { 1, 6, 7, 2, 3 };   /* (BN5's DrkSword 188, DarkThnd 193, DrkRecov 194, DarkInvs 189, DarkPlus 190) */
	for (int i = 0; i < DARK_BN6_COUNT; ++i)
		CHECK(dark_bn6_kind(DARK_BN6_FIRST + i) == kinds[i], "dark: BN6's chip %d as BN5's kind %d (%d)", DARK_BN6_FIRST + i, kinds[i],
			dark_bn6_kind(DARK_BN6_FIRST + i));
	CHECK(dark_bn6_kind(DARK_BN6_FIRST - 1) < 0 && dark_bn6_kind(DARK_BN6_FIRST + DARK_BN6_COUNT) < 0 && dark_bn6_kind(0x47) < 0,
		"dark: other chips are no DarkChip of BN6's");
	CHECK(dark_bn6_id(1) == 286 && dark_bn6_id(6) == 287 && dark_bn6_id(7) == 288 && dark_bn6_id(2) == 289, "dark: the four kinds as BN6's chips");
	CHECK(!dark_bn6_id(3) && !dark_bn6_id(0) && !dark_bn6_id(11), "dark: DarkPlus and BN5's own seven are none of BN6's battles' (%d %d %d)",
		dark_bn6_id(3), dark_bn6_id(0), dark_bn6_id(11));
	uint8_t held[DARK_KINDS] = { 0 };
	int bn6_seen = 0, bad = 0;
	for (uint32_t seed = 1; seed < 400; ++seed)
		for (int depth = 2; depth < 30; depth += 3) {
			int k5 = dark_flame_pick(seed, depth, false, held), was = (int)(((seed ^ (uint32_t)depth * 2654435761u) >> 8) % DARK_KINDS);
			int k6 = dark_flame_pick(seed, depth, true, held);
			bad += k5 != was || dark_bn6_id(k6) == 0;
			if (k6 >= 0) bn6_seen |= 1 << k6;
		}
	CHECK(!bad, "dark: %d picks off BN5's rule as it was, or off the four BN6's battles play", bad);
	CHECK(bn6_seen == (1 << 1 | 1 << 6 | 1 << 7 | 1 << 2), "dark: a BN6 flame offers each of the four over the seeds (%#x)", bn6_seen);
	/* (seeds apart in their low bits alone, a test's, start apart too) */
	int small = 0, kinds_small = 0;
	for (uint32_t seed = 1; seed <= 12; ++seed) small |= 1 << dark_flame_pick(seed, 2, true, held);
	for (int k = 0; k < DARK_KINDS; ++k) kinds_small += small >> k & 1;
	CHECK(kinds_small >= 3, "dark: seeds 1 to 12 on layer 2 offer %d of BN6's four kinds, three at least", kinds_small);
	held[1] = held[6] = held[7] = 1;
	bad = 0;
	for (uint32_t seed = 1; seed < 200; ++seed) bad += dark_flame_pick(seed, 5, true, held) != 2;
	CHECK(!bad, "dark: with three held, a BN6 flame offers the fourth, DarkInvs (%d seeds off)", bad);
	held[2] = 1;
	CHECK(dark_flame_pick(7, 5, true, held) < 0, "dark: with the four held, no BN6 flame, BN5's kinds lacked or not");
	CHECK(dark_flame_pick(7, 5, false, held) >= 0 && !held[dark_flame_pick(7, 5, false, held)], "dark: ... while BN5's offers a kind the run lacks");
	/* (the layer's flame kept: a CONTINUE on it after it was taken finds the
	 * same, not the next kind lacked) */
	dark_new_run(99);
	int k = dark_flame_of(99, 5, 0, true);
	CHECK(k >= 0 && dark_bn6_id(k), "dark: a new run's BN6 flame holds one of the four (%d)", k);
	dark_give(k);
	CHECK(dark_flame_of(99, 5, 0, true) == k, "dark: the layer's flame the same after its chip was taken (%d)", dark_flame_of(99, 5, 0, true));
	int k8 = dark_flame_of(99, 8, 0, true);
	CHECK(k8 >= 0 && k8 != k, "dark: the next act's flame another kind (%d, %d)", k8, k);
	CHECK(dark_flame_of(99, 8, 0, false) >= 0, "dark: BN5's flame on that layer picked anew, not BN6's kept");
	dark_new_run(99);
	int v[3];
	dark_price_hp(100, 120, 50, v);
	CHECK(v[0] == 80 && v[1] == 100 && v[2] == 50, "dark: 20 max HP off the base and the max (%d %d %d)", v[0], v[1], v[2]);
	dark_price_hp(100, 120, 115, v);
	CHECK(v[2] == 100, "dark: HP held to the new max (%d)", v[2]);
	dark_price_hp(25, 45, 45, v);
	CHECK(v[0] == 10 && v[1] == 30 && v[2] == 30, "dark: the base never under 10 (%d %d %d)", v[0], v[1], v[2]);
}

/* A script of the game's own text run as a chat says it (src/layer/text.c,
 * ta_rom_pages; a BBS post of BN6's said by a bystander, docs/META.md): its
 * BBS positioning left out, its characters, breaks, waits and clears kept to
 * its hold; anything else, or no hold, refused. A made-up archive. */
/* The intro's QR code (scene_intro.c): the project's address in version 4,
 * its finders and timing where the standard puts them, and module for
 * module the code the site's qr.js makes (the hashes are of qr.js's
 * modules: a 3DS release link takes version 6, two blocks); a text too
 * long for version 10 makes none. */
static uint32_t qr_hash(const uint8_t *m, int n) {
	uint32_t h = 2166136261u;
	for (int i = 0; i < n * n; ++i) { h ^= m[i]; h *= 16777619u; }
	return h;
}

static void test_qr(void) {
	static uint8_t m[QR_MAX * QR_MAX];
	int n = qr_make("https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless", m);
	CHECK(n == 33, "the repository's address in %d modules a side, not version 4's 33", n);
	for (int i = 0; i < 7 && n == 33; ++i) {
		bool ring = i == 0 || i == 6;
		CHECK(m[i] == 1 && m[i * n] == 1 && m[i * n + n - 1] == 1, "a finder's outer ring broken at %d", i);
		CHECK(m[n + i] == ring, "the top-left finder's second row wrong at %d", i);
	}
	for (int i = 8; i < n - 8 && n == 33; ++i) CHECK(m[6 * n + i] == (i % 2 == 0), "the timing row wrong at %d", i);
	CHECK(qr_hash(m, n) == 0x14B2F150u, "the repository's code differs from qr.js's: %08X", qr_hash(m, n));
	n = qr_make("https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/releases/latest/download/cyberworld-endless.cia", m);
	CHECK(n == 41 && qr_hash(m, n) == 0xEEADAA46u, "a release link's code differs from qr.js's: %d, %08X", n, qr_hash(m, n));
	char longer[300];
	memset(longer, 'x', sizeof longer - 1);
	longer[sizeof longer - 1] = 0;
	CHECK(qr_make(longer, m) == 0, "299 bytes fit no version up to 10");
}

/* a save blob as save.c writes one: magic, size, FNV-1a checksum, bytes */
static void blob_file(const char *path, uint32_t magic, const void *data, uint32_t n) {
	const uint8_t *b = data;
	uint32_t sum = 2166136261u;
	for (uint32_t i = 0; i < n; ++i) sum = (sum ^ b[i]) * 16777619u;
	FILE *f = fopen(path, "wb");
	if (!f) return;
	uint32_t hdr[3] = { magic, n, sum };
	fwrite(hdr, sizeof hdr, 1, f);
	fwrite(data, n, 1, f);
	fclose(f);
}

/* a folder and all in it gone */
static void remove_tree(const char *dir) {
	DIR *d = opendir(dir);
	if (!d) return;
	for (struct dirent *e; (e = readdir(d));) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
		char path[512];
		struct stat st;
		snprintf(path, sizeof path, "%s/%s", dir, e->d_name);
		if (!stat(path, &st) && S_ISDIR(st.st_mode)) remove_tree(path);
		else remove(path);
	}
	closedir(d);
	rmdir(dir);
}

static bool same_file(const char *a, const char *b) {
	size_t na = 0, nb = 0;
	uint8_t *x = backup_read_file(a, &na), *y = backup_read_file(b, &nb);
	bool same = x && y && na == nb && !memcmp(x, y, na);
	free(x);
	free(y);
	return same;
}

/* The saves in one file (backup.c, issue #97): what a phone keeps beside its
 * ROMs and the browser's page exports */
static void test_backup(void) {
	/* (a folder of this run's own, emptied first: an earlier run's saves
	 * are no part of it, and Docker's pids come again) */
	char base[64], from[96], to[96], path[256], other[256];
	snprintf(base, sizeof base, "build/host/test-backup-%d", (int)getpid());
	remove_tree(base);
	snprintf(from, sizeof from, "%s/from", base);
	snprintf(to, sizeof to, "%s/to", base);
	mkdir(base, 0755);
	mkdir(from, 0755);
	mkdir(to, 0755);
	snprintf(path, sizeof path, "%s/savedata", from);
	mkdir(path, 0755);
	snprintf(path, sizeof path, "%s/savedata", to);
	mkdir(path, 0755);
	size_t n = 0;
	uint8_t *none = backup_pack(from, 1, &n);
	CHECK(!none, "backup: no profile, nothing to keep");
	free(none);
	int prof[16] = { 12, 9 };   /* (runs, best: the profile's first two) */
	snprintf(path, sizeof path, "%s/savedata/profile.sav", from);
	blob_file(path, 0x43575032u, prof, sizeof prof);
	int runbuf[25] = { 1, 7, 4 };   /* (active, seed, layer: a run's first three) */
	snprintf(path, sizeof path, "%s/savedata/run.sav", from);
	blob_file(path, 0x43574538u, runbuf, sizeof runbuf);
	snprintf(path, sizeof path, "%s/keys.ini", from);
	FILE *f = fopen(path, "w");
	if (f) { fputs("A = J\n", f); fclose(f); }
	snprintf(path, sizeof path, "%s/savedata/run.sav.tmp", from);
	f = fopen(path, "w");
	if (f) { fputs("half", f); fclose(f); }
	uint8_t *b = backup_pack(from, 1700000000u, &n), *c = NULL;
	BackupInfo info, again;
	CHECK(b && backup_info(b, n, &info), "backup: packed and read back");
	if (!b) return;
	CHECK(info.runs == 12 && info.best == 9 && info.run_depth == 4, "backup: runs %d, best %d, a run on layer %d", info.runs, info.best, info.run_depth);
	CHECK(info.files == 3 && info.stamp == 1700000000u, "backup: %d files (a .tmp left out), stamp %llu", info.files, (unsigned long long)info.stamp);
	size_t m = 0;
	c = backup_pack(from, 1800000000u, &m);
	CHECK(c && backup_info(c, m, &again) && again.hash == info.hash, "backup: the same saves hash the same, made at another time");
	free(c);
	b[n / 2] ^= 1;
	CHECK(!backup_info(b, n, &again) && !backup_unpack(to, b, n), "backup: a damaged one refused");
	b[n / 2] ^= 1;
	CHECK(!backup_info(b, n - 1, &again), "backup: a cut one refused");
	snprintf(path, sizeof path, "%s/savedata/profile.sav", to);
	f = fopen(path, "w");
	if (f) { fputs("older", f); fclose(f); }
	CHECK(backup_unpack(to, b, n), "backup: unpacked");
	snprintf(path, sizeof path, "%s/savedata/run.sav", from);
	snprintf(other, sizeof other, "%s/savedata/run.sav", to);
	CHECK(same_file(path, other), "backup: run.sav comes back as it was");
	snprintf(path, sizeof path, "%s/keys.ini", from);
	snprintf(other, sizeof other, "%s/keys.ini", to);
	CHECK(same_file(path, other), "backup: keys.ini comes back as it was");
	snprintf(other, sizeof other, "%s/savedata.old/profile.sav", to);
	f = fopen(other, "r");
	CHECK(f != NULL, "backup: the saves it replaced kept aside in savedata.old");
	if (f) fclose(f);
	CHECK(backup_local_info(to, &again) && again.hash == info.hash, "backup: the saves unpacked are the saves packed");
	free(b);
	remove_tree(base);
}

/* The launcher (src/launcher/, issue #97): when it shows at a start, and
 * what it says of the saves */
static void test_launcher(void) {
	RomRecord none = { 0 }, both = { true, true, true }, six = { true, true, false };
	CHECK(roms_launcher_wanted(false, true, false, none), "launcher: shown at a first start");
	CHECK(roms_launcher_wanted(false, false, false, both), "launcher: shown without BN6");
	CHECK(roms_launcher_wanted(false, true, false, both), "launcher: shown where BN5 has gone");
	CHECK(!roms_launcher_wanted(false, true, true, six), "launcher: not shown for BN5 come since (the title says so)");
	CHECK(!roms_launcher_wanted(false, true, false, six) && !roms_launcher_wanted(false, true, true, both), "launcher: not shown where nothing changed");
	CHECK(roms_launcher_wanted(true, true, true, both), "launcher: shown when asked for");
	/* the saves told as the title tells them: the best the deeper of the
	 * profile's and the saved run's, one run "1 run", none not counted */
	char said[400];
	BackupInfo one = { .runs = 1, .best = 6, .run_depth = 2 }, run_only = { .run_depth = 1 }, empty = { 0 };
	ask_saves("ROMs", &one, NULL, said, sizeof said);
	CHECK(!strcmp(said, "In ROMs: 1 run, best Layer 6, a run on Layer 2.\nBring them back?"), "launcher: saves found told (%s)", said);
	ask_saves("ROMs", &run_only, &empty, said, sizeof said);
	CHECK(!strcmp(said, "In ROMs: a run on Layer 1.\nOn this device: no runs yet.\nUse the folder's saves?"), "launcher: a run alone, and none, told (%s)", said);
	note_restored(&run_only, said, sizeof said);
	CHECK(!strcmp(said, "Saves brought back: a run on Layer 1."), "launcher: saves brought back told (%s)", said);
	/* a phone's saves' line: two lines, a long folder's name cut */
	saves_line_phone("Mega Man Battle Network ROMs", false, said, sizeof said);
	CHECK(!strcmp(said, "Saves: on this device\nA copy in Mega Man Bat..., for a reinstall"), "launcher: a long folder cut in the saves' line (%s)", said);
	saves_line_phone("ROMs", true, said, sizeof said);
	CHECK(!strcmp(said, "Saves: on this device only\nNo copy in ROMs: choose it again"), "launcher: a copy refused said (%s)", said);
}

static void test_rom_pages(void) {
	const uint8_t arch[] = {
		4, 0, 6, 0,                                      /* two scripts, at 4 and 6 */
		0xE6, 0xE6,                                      /* 0: an end */
		0xF7, 0x02, 0x18, 0x44, 0x03, 0xF7, 0x04, 0xD2, 0x6C,   /* 1: the BBS's positioning */
		0x0B, 0x00, 0x26, 0xE9, 0x26, 0xE7, 0x00, 0xF2, 0x2A, 0xA2, 0xE7, 0x00, 0xEE, 0xFF, 0x00, 0x00,
	};
	const uint8_t want[] = { 0x0B, 0x00, 0x26, 0xE9, 0x26, 0xE7, 0x00, 0xF2, 0x2A, 0xA2, 0xE7, 0x00 };
	uint8_t out[64];
	int n = ta_rom_pages(arch, (int)sizeof arch, 1, out, (int)sizeof out);
	CHECK(n == (int)sizeof want && !memcmp(out, want, sizeof want), "rom pages: the post's text and pages, its positioning left out (%d bytes)", n);
	CHECK(!ta_rom_pages(arch, (int)sizeof arch, 0, out, (int)sizeof out), "rom pages: a script with no hold refused");
	CHECK(!ta_rom_pages(arch, (int)sizeof arch, 1, out, 8), "rom pages: one that overruns refused");
	CHECK(!ta_rom_pages(arch, (int)sizeof arch, 2, out, (int)sizeof out), "rom pages: a script past the archive refused");
	uint8_t face[sizeof arch];
	memcpy(face, arch, sizeof arch);
	face[15] = 0xF5;   /* (a mugshot command in the text) */
	CHECK(!ta_rom_pages(face, (int)sizeof face, 1, out, (int)sizeof out), "rom pages: a command it does not know refused");
}

/* The super bosses (docs/BOSSES.md, Super bosses): their forms by record
 * and by Net; BN6's records' themes, names and Gigas; never at random,
 * the Servers' SP Navis without them */
static void super_records(void) {
	CHECK(super_bass_form(0, false) == SUPER_BASS_V1 && super_bass_form(0, true) == SUPER_BASS_V1, "super: Bass's first form");
	CHECK(super_bass_form(1, true) == SUPER_BASS_SP && super_bass_form(2, false) == SUPER_BASS_SP && super_bass_form(6, false) == SUPER_BASS_SP,
		"super: Bass SP once beaten");
	CHECK(super_bass_form(2, true) == SUPER_BASS_BX && super_bass_form(7, true) == SUPER_BASS_BX, "super: Bass BX with the beast's data");
	/* (turned across the screen to MegaMan beside them, the nearest way
	 * each sprite shows: Bass's, BlastMan's and ElementMan's only towards
	 * the camera, the beast's and Falzar's Navis' mirrored to the right) */
	CHECK(super_body(SUPER_BASS, GUARDIAN_FACE_LEFT).body.anim == 5 && super_body(SUPER_BASS, GUARDIAN_FACE_RIGHT).body.anim == 2 &&
		super_body(SUPER_CYBEAST, GUARDIAN_FACE_RIGHT).body.mirror && !super_body(SUPER_CYBEAST, GUARDIAN_FACE_LEFT).body.mirror &&
		guardian_body(12, GUARDIAN_FACE_LEFT).anim == 5 && guardian_body(16, GUARDIAN_FACE_RIGHT).anim == 2 &&
		guardian_body(1, GUARDIAN_FACE_LEFT).anim == GUARDIAN_FACE_LEFT && guardian_body(7, GUARDIAN_FACE_RIGHT).mirror &&
		!guardian_body(7, GUARDIAN_FACE_LEFT).mirror, "super: facing across the screen");
	CHECK(super_cybeast_form(0) == SUPER_CYBEAST_V1 && super_cybeast_form(1) == SUPER_CYBEAST_SP && super_cybeast_form(4) == SUPER_CYBEAST_SP,
		"super: Gregar SP after the first Net");
	CHECK(super_song(SUPER_BASS) == 0x16 && super_song(SUPER_CYBEAST) == 0x17, "super: their records' themes");
	CHECK(!*super_suffix(SUPER_BASS, SUPER_BASS_V1) && !strcmp(super_suffix(SUPER_BASS, SUPER_BASS_SP), " SP") &&
		!strcmp(super_suffix(SUPER_BASS, SUPER_BASS_BX), " BX") && !*super_suffix(SUPER_CYBEAST, SUPER_CYBEAST_V1) &&
		!strcmp(super_suffix(SUPER_CYBEAST, SUPER_CYBEAST_SP), " SP"), "super: their forms' names");
	CHECK(super_chip(SUPER_BASS, SUPER_BASS_V1) == 306 && super_chip(SUPER_BASS, SUPER_BASS_SP) == 301 && super_chip(SUPER_BASS, SUPER_BASS_BX) == 304 &&
		super_chip(SUPER_CYBEAST, SUPER_CYBEAST_SP) == 305 && super_chip(3, 0) == 0, "super: their Gigas");
	for (int ai = 0; ai < 32; ++ai)
		CHECK(super_reserved_ai(ai) == (ai == 17 || (ai >= 19 && ai <= 24)), "super: AI %d %s", ai, super_reserved_ai(ai) ? "held back" : "rolled");
}

/* ... their places in a run (run_new's picks): the endless Nest's master,
 * the Secret Area's Bass once its S is held; their bigger arenas, a
 * guardian's otherwise, reached from the start */
static void super_places(void) {
	CHECK(super_nest_master() == SUPER_CYBEAST, "super: the endless Nest's master is %d", super_nest_master());
	CHECK(super_secret_master(0, 11) == 11 && super_secret_master(MARK_NEST | MARK_WIN, 11) == 11, "super: Bass in the Secret Area before its S");
	CHECK(super_secret_master(MARK_SECRET, 11) == SUPER_BASS, "super: no Bass in the Secret Area after its S");
	memset(&run, 0, sizeof run);
	run.boss_order[BIOME_NEST] = SUPER_CYBEAST;
	run.boss_order[BIOME_SECRET] = SUPER_BASS;
	static uint8_t seen[MAP_H][MAP_W];
	int big = 0, n = 0;
	for (uint32_t s = 1; s <= 60; ++s) {
		bool bass = s % 2;
		int depth = bass ? 2 + (int)(s % 17) : CYCLE_LAYERS * (1 + (int)(s % 3)), want = bass ? SUPER_BASS : SUPER_CYBEAST;
		layer_generate(s * 7919u, depth, bass ? BIOME_SECRET : BIOME_NEST, bass ? LAYER_SECRET : LAYER_NORMAL, &kit);
		CHECK(layer.boss_layer && layer.boss_navi == want && layer.arena >= 0, "super: layer %u holds %d, no arena for %d", s, layer.boss_navi, want);
		if (layer.arena < 0) continue;
		check_arena(s);
		const Room *a = &layer.rooms[layer.arena];
		big += a->w == SUPER_ARENA_SIZE && a->h == SUPER_ARENA_SIZE;
		++n;
		for (int i = 0; i < layer.nobj; ++i)
			if (layer.obj[i].type == OBJ_WARP_IN) {
				reachable_cells((int)layer.obj[i].x, (int)layer.obj[i].y, seen);
				CHECK(seen[a->ay][a->ax], "super: layer %u's arena out of reach", s);
			}
	}
	CHECK(big * 4 >= n * 3, "super: %d of %d arenas at their size", big, n);
	printf("  super bosses' arenas: %d of %d layers at %d panels a side, the rest a guardian's\n", big, n, SUPER_ARENA_SIZE);
}

/* ... and every line of theirs and of the run's foreshadowing within BN6's
 * box, none naming them before they have met */
static void super_words(void) {
	char what[96];
	static const Rival records[] = {
		{ 0, 0, 0, RIVAL_NONE }, { 1, 0, 0, RIVAL_NONE }, { 1, 0, 1, RIVAL_NAVI_WON }, { 2, 1, 0, RIVAL_MEGAMAN_WON },
		{ 3, 2, 0, RIVAL_MEGAMAN_WON }, { 4, 3, 0, RIVAL_MEGAMAN_WON }, { 5, 3, 1, RIVAL_NAVI_WON },
	};
	static const int forms[2][3] = { { SUPER_BASS_V1, SUPER_BASS_SP, SUPER_BASS_BX }, { SUPER_CYBEAST_V1, SUPER_CYBEAST_SP, SUPER_CYBEAST_SP } };
	memset(&profile, 0, sizeof profile);
	rivals_load();   /* (no saves here: every record blank, the talk's test's gone) */
	for (int k = 0; k < 2; ++k) {
		int navi = k ? SUPER_CYBEAST : SUPER_BASS;
		for (unsigned r = 0; r < sizeof records / sizeof *records; ++r)
			for (int f = 0; f < 3; ++f) {
				snprintf(what, sizeof what, "super_intro(%d, %d) record %u", navi, forms[k][f], r);
				check_talk(what, super_intro(navi, forms[k][f], &records[r]));
			}
		for (int f = 0; f < 3; ++f) {
			snprintf(what, sizeof what, "super_defeat(%d, %d)", navi, forms[k][f]);
			check_talk(what, super_defeat(navi, forms[k][f]));
			for (int first = 0; first < 2; ++first) {
				snprintf(what, sizeof what, "super_reward_words(%d, %d, %d)", navi, forms[k][f], first);
				check_talk(what, super_reward_words(navi, forms[k][f], first));
			}
			/* (the card's lines, as long as a guardian's at most) */
			const char *top, *sub;
			super_card(navi, forms[k][f], k ? 3 * CYCLE_LAYERS : 8, &top, &sub);
			CHECK(strlen(top) < 28 && strlen(sub) < 28, "super: the card's lines \"%s\" \"%s\" are long", top, sub);
		}
		snprintf(what, sizeof what, "super_tip(%d)", navi);
		check_talk(what, super_tip(navi));
		CHECK(super_tip(navi) && guardian_tip(navi) == super_tip(navi), "super: %d's tip", navi);
		CHECK(strlen(super_reward_head(navi)) < 48, "super: %d's data's head is long", navi);
		/* (the net's word on him, and the Nest's port at home, before they
		 * have met: never his name) */
		const char *rumor = super_rumor(navi), *port = home_port_words(BIOME_NEST, navi, false, false);
		snprintf(what, sizeof what, "super_rumor(%d)", navi);
		check_talk(what, rumor);
		snprintf(what, sizeof what, "the Nest's port with %d unmet", navi);
		check_talk(what, port);
		static const char *const names[] = { "Bass", "Gregar" };
		for (int i = 0; i < 2; ++i)
			CHECK((!rumor || !strstr(rumor, names[i])) && !strstr(port, names[i]), "super: %s named before they met: \"%s\" \"%s\"", names[i],
				rumor ? rumor : "", port);
	}
	rival_met(SUPER_CYBEAST);
	rival_result(SUPER_CYBEAST, RIVAL_MEGAMAN_WON);
	check_talk("the Nest's port with the Cybeast met", home_port_words(BIOME_NEST, SUPER_CYBEAST, false, false));
}

static void test_super_bosses(void) {
	Profile profile_was = profile;
	Run run_was = run;
	super_records();
	super_places();
	super_words();
	profile = profile_was;
	run = run_was;
}

/* Requests at home (jobs.c, docs/HOME.md piece 5): the visit's three, a
 * kind each asker posts and none twice, the same at a CONTINUE; pay by
 * the act; each kind's course, and when one is due. */
static void test_jobs(void) {
	static const int posts[JOB_ASKERS][2] = { { JOB_BRING, JOB_VOW }, { JOB_QUICK, JOB_CLEAN }, { JOB_EXPLORE, JOB_CLEAN } };
	for (uint32_t seed = 1; seed <= 300; ++seed)
		for (int depth = 1; depth <= 19; depth += 3) {
			Job a[JOB_ASKERS], b[JOB_ASKERS];
			uint32_t before = rng_state();
			jobs_offers(seed, depth, a);
			CHECK(rng_state() == before, "jobs_offers keeps the game's random numbers as they were");
			jobs_offers(seed, depth, b);
			CHECK(!memcmp(a, b, sizeof a), "seed %u depth %d: the same offers twice", seed, depth);
			for (int k = 0; k < JOB_ASKERS; ++k) {
				CHECK(a[k].asker == k && (a[k].kind == posts[k][0] || a[k].kind == posts[k][1]), "seed %u: asker %d posts kind %d", seed, k, a[k].kind);
				for (int i = 0; i < k; ++i) CHECK(a[i].kind != a[k].kind, "seed %u depth %d: kind %d twice", seed, depth, a[k].kind);
				CHECK(a[k].state == 0 && a[k].act == jobs_act(depth), "an offer, for its act");
			}
			/* (no vow at a run's first visit: the act's ways unseen yet) */
			CHECK(depth > 1 || a[JOB_BOARD].kind == JOB_BRING, "seed %u: the run's first visit posts no vow", seed);
		}
	/* pay and terms by the act */
	Job j;
	memset(&j, 0, sizeof j);
	jobs_dev(&j, 7, 1, false, JOB_CLEAN, 0);
	CHECK(j.state == JOB_TAKEN && j.need == 2 && j.pay_kind == PAY_ZENNY && j.pay == 500, "act 1's clean wins: 2 for 500 Zenny (%d, %u)", j.need, j.pay);
	jobs_dev(&j, 7, 7, false, JOB_CLEAN, 0);
	CHECK(j.need == 3 && j.pay == 1000, "act 3's: 3 for 1000 Zenny (%d, %u)", j.need, j.pay);
	jobs_dev(&j, 7, 1, false, JOB_QUICK, 0);
	CHECK(j.pay_kind == PAY_BUGFRAGS && j.pay == 5 && j.need == 3, "act 1's quick wins: 3 for 5 BugFrags");
	jobs_dev(&j, 7, 1, false, JOB_VOW, 0);
	CHECK(j.pay_kind == PAY_HPMEMORY && j.pay == 2, "a vow: two HPMemory");
	CHECK(jobs_vow_holds(&j, 1) && jobs_vow_holds(&j, 3) && !jobs_vow_holds(&j, 4), "a vow holds through its act, not past it");
	/* their course */
	jobs_dev(&j, 7, 1, false, JOB_CLEAN, 0);
	CHECK(!jobs_battle(&j, true, 10, 100) && j.got == 0, "a win that cost HP is no clean win");
	CHECK(!jobs_battle(&j, false, 0, 100) && j.got == 0, "a battle left is none");
	CHECK(!jobs_battle(&j, true, 0, 900) && j.got == 1, "a clean win counts, however long");
	CHECK(jobs_battle(&j, true, 0, 900) && j.state == JOB_DONE, "the second: done");
	CHECK(!jobs_battle(&j, true, 0, 900) && j.got == 2, "done, it counts no more");
	jobs_dev(&j, 7, 1, false, JOB_QUICK, 0);
	CHECK(!jobs_battle(&j, true, 50, JOB_QUICK_FRAMES + 1) && j.got == 0, "a win past ten seconds is no quick win");
	jobs_battle(&j, true, 50, JOB_QUICK_FRAMES);
	CHECK(j.got == 1, "ten seconds to the frame counts");
	jobs_dev(&j, 7, 1, false, JOB_EXPLORE, 0);
	CHECK(!jobs_layer_left(&j, 2, 2) && j.state == JOB_TAKEN, "a layer of two Mystery Data is too few");
	CHECK(!jobs_layer_left(&j, 4, 3) && j.state == JOB_TAKEN, "one left unopened");
	CHECK(jobs_layer_left(&j, 4, 4) && j.state == JOB_DONE, "every one of four opened: done");
	jobs_dev(&j, 7, 1, false, JOB_VOW, 0);
	CHECK(jobs_heal(&j) && j.state == JOB_FAILED && !jobs_guardian(&j), "a patch breaks the vow, and the guardian's fall then pays nothing");
	CHECK(!jobs_vow_holds(&j, 1), "a broken vow holds no more");
	jobs_dev(&j, 7, 1, false, JOB_VOW, 0);
	CHECK(jobs_guardian(&j) && j.state == JOB_DONE && !jobs_heal(&j), "the guardian fallen first: done, and a patch after costs nothing");
	/* due at the visit after its act, not before */
	jobs_dev(&j, 7, 1, false, JOB_EXPLORE, 0);
	CHECK(!jobs_due(&j, 1) && !jobs_due(&j, 3) && jobs_due(&j, 4), "taken before layer 1: due before layer 4");
	jobs_dev(&j, 7, 4, true, JOB_EXPLORE, JOB_DONE);
	CHECK(j.act == jobs_act(1) && jobs_due(&j, 4) && j.state == JOB_DONE, "--dev at home: one from the act just played, done");
}

/* ---- the anonymous statistics (issue #104): the answer, the events'
 * payloads, the queue's cap and the tries, none of it sent ---- */

static char stats_sent[16][ANALYTICS_PAYLOAD];
static int stats_posts, stats_drops;
static bool stats_system = true;

static bool stats_post(const char *json) {
	if (stats_posts < 16) snprintf(stats_sent[stats_posts], sizeof stats_sent[0], "%s", json);
	++stats_posts;
	return true;
}
static void stats_drop(void) { ++stats_drops; }
static bool stats_ready(void) { return stats_system; }

/* The settings file's statistics line, "" for none */
static void stats_line(const char *path, char *out, size_t n) {
	char line[256];
	out[0] = 0;
	FILE *f = fopen(path, "r");
	if (!f) return;
	while (fgets(line, sizeof line, f))
		if (!strncmp(line, "statistics", 10)) { snprintf(out, n, "%s", line); break; }
	fclose(f);
}

static void stats_begin(const char *settings, bool plain, const char *version) {
	AnalyticsStart s = { "linux", version, 1280, 960, settings, plain, stats_ready, stats_post, stats_drop, NULL };
	stats_posts = stats_drops = 0;
	analytics_begin(&s);
}

static void test_analytics(void) {
	/* the queue: eight wait at most, the ninth is dropped, first in first out */
	static AnalyticsQueue q;
	char body[ANALYTICS_PAYLOAD], name[16];
	for (int i = 0; i < ANALYTICS_QUEUE; ++i) {
		snprintf(name, sizeof name, "e%d", i);
		CHECK(analytics_queue_put(&q, name), "statistics: payload %d queued", i);
	}
	CHECK(!analytics_queue_put(&q, "e8"), "statistics: a ninth payload is dropped, not queued");
	CHECK(analytics_queue_take(&q, body, sizeof body) && !strcmp(body, "e0"), "statistics: the first queued goes first (%s)", body);
	CHECK(analytics_queue_put(&q, "e8"), "statistics: room again once one is taken");
	analytics_queue_clear(&q);
	CHECK(!analytics_queue_take(&q, body, sizeof body), "statistics: a cleared queue holds nothing");
	char big[ANALYTICS_PAYLOAD + 8];
	memset(big, 'x', sizeof big - 1);
	big[sizeof big - 1] = 0;
	CHECK(!analytics_queue_put(&q, big), "statistics: a payload past its size is never queued");
	/* each tried once; three failures in a row and nothing more */
	AnalyticsTries t = { 0, false };
	CHECK(analytics_tried(&t, false) && analytics_tried(&t, true) && analytics_tried(&t, false) && analytics_tried(&t, false),
		"statistics: a success between failures starts their count again");
	CHECK(!analytics_tried(&t, false) && t.gave_up, "statistics: the third failure in a row gives up");
	CHECK(!analytics_tried(&t, true), "statistics: given up, even a success sends nothing more");

	char base[64], settings[128], line[128];
	snprintf(base, sizeof base, "build/host/test-stats-%d", (int)getpid());
	mkdir(base, 0755);
	snprintf(settings, sizeof settings, "%s/settings.ini", base);
	FILE *f = fopen(settings, "w");
	if (f) { fputs("# settings\nsmooth_motion = off\n", f); fclose(f); }
	/* a player's first start: asked, nothing sent before the answer */
	memset(&run, 0, sizeof run);
	run.seed = 77;
	run.depth = 7;
	run.biome = BIOME_SKY;
	run.mode = RUN_SHORT;
	run.threat = 2;
	run.folder = FOLDER_BLADE;
	run.cross = 1;
	run.helpers = HELP_HEALS | HELP_GENTLE;
	profile.runs = 4;
	profile.played_run = 77;
	profile.played_frames = 3600 * 42 + 100;
	stats_begin(settings, true, "0.10.0");
	CHECK(analytics_here() && analytics_unasked(), "statistics: a player's first start asks");
	analytics_run_start();
	analytics_guardian(1, ANALYTICS_WON, 50, 200, 3000);
	analytics_run_end(false, "HeatMan");
	CHECK(stats_posts == 0, "statistics: nothing is sent before the answer (%d sent)", stats_posts);
	/* yes: kept in settings.ini beside what was there, the start's view sent */
	analytics_answer(true);
	stats_line(settings, line, sizeof line);
	CHECK(!strcmp(line, "statistics = on\n"), "statistics: a yes is settings.ini's \"statistics = on\" (%s)", line);
	f = fopen(settings, "r");
	char all[2048] = "";
	size_t got = f ? fread(all, 1, sizeof all - 1, f) : 0;
	if (f) fclose(f);
	all[got] = 0;
	CHECK(strstr(all, "smooth_motion = off") && strstr(all, "# statistics:"), "statistics: settings.ini keeps its lines, and says what the new one is");
	CHECK(stats_posts == 1 && !strstr(stats_sent[0], "\"name\"") && strstr(stats_sent[0], "\"url\":\"/linux\"") &&
		strstr(stats_sent[0], "\"title\":\"Cyberworld Endless 0.10.0\"") && strstr(stats_sent[0], "\"screen\":\"1280x960\"") &&
		strstr(stats_sent[0], "\"website\":\"" GAME_UMAMI_WEBSITE "\"") && strstr(stats_sent[0], "\"hostname\":\"" GAME_UMAMI_HOSTNAME "\"") &&
		!strncmp(stats_sent[0], "{\"type\":\"event\",\"payload\":{", 27),
		"statistics: a yes sends the start's view, Umami's tracker's own payload (%s)", stats_sent[0]);
	analytics_answer(true);
	CHECK(stats_posts == 1, "statistics: the start's view once a session");
	/* the events, as the run tells them */
	analytics_run_start();
	CHECK(stats_posts == 2 && strstr(stats_sent[1], "\"name\":\"run-start\",\"data\":{\"net\":\"short\",\"folder\":\"Blade\"") &&
		strstr(stats_sent[1], "\"threat\":2") && strstr(stats_sent[1], "\"helpers\":\"Heals, Gentle\"") && strstr(stats_sent[1], "\"bn5\":false") &&
		strstr(stats_sent[1], "\"runs\":4") && strstr(stats_sent[1], "\"cross\":\"") && !strstr(stats_sent[1], "\"cross\":\"none\""),
		"statistics: run-start names the setup (%s)", stats_sent[1]);
	analytics_guardian(1, ANALYTICS_WON, 50, 200, 3000);
	/* (the meetings the rivals' record holds, this one counted as he stepped up) */
	char attempt[32];
	snprintf(attempt, sizeof attempt, "\"attempt\":%d}", rival(1)->met > 0 ? rival(1)->met : 1);
	CHECK(stats_posts == 3 && strstr(stats_sent[2], "\"name\":\"guardian\",\"data\":{\"guardian\":\"HeatMan\",\"result\":\"won\",\"layer\":7") &&
		strstr(stats_sent[2], "\"hp\":25") && strstr(stats_sent[2], "\"seconds\":50") && strstr(stats_sent[2], attempt),
		"statistics: guardian names the battle's end (%s)", stats_sent[2]);
	analytics_guardian(1, ANALYTICS_LOST, 0, 200, 0);
	CHECK(stats_posts == 4 && strstr(stats_sent[3], "\"result\":\"lost\"") && !strstr(stats_sent[3], "\"hp\"") && !strstr(stats_sent[3], "\"seconds\""),
		"statistics: a guardian's win over MegaMan, no HP or time (%s)", stats_sent[3]);
	analytics_run_end(false, "HeatMan");
	CHECK(stats_posts == 5 && strstr(stats_sent[4], "\"name\":\"run-end\",\"data\":{\"won\":false,\"layer\":7,\"area\":\"") &&
		strstr(stats_sent[4], "\"by\":\"HeatMan\"") && strstr(stats_sent[4], "\"minutes\":42") && strstr(stats_sent[4], "\"net\":\"short\""),
		"statistics: run-end names how far, by whom and how long (%s)", stats_sent[4]);
	analytics_run_end(true, NULL);
	CHECK(stats_posts == 6 && strstr(stats_sent[5], "\"won\":true") && !strstr(stats_sent[5], "\"by\""), "statistics: a won run names no one (%s)", stats_sent[5]);
	for (int i = 0; i < 6; ++i) CHECK(strlen(stats_sent[i]) < ANALYTICS_PAYLOAD - 200, "statistics: payload %d well within its size", i);
	/* no: kept, what waits dropped, nothing more sent */
	analytics_answer(false);
	stats_line(settings, line, sizeof line);
	CHECK(!strcmp(line, "statistics = off\n") && stats_drops == 1, "statistics: a no is kept (%s) and drops what waits", line);
	analytics_run_start();
	CHECK(stats_posts == 6, "statistics: after a no, nothing is sent");
	/* the next start reads the answer: off asks nothing; on sends its view */
	stats_begin(settings, true, "0.10.0");
	CHECK(analytics_here() && !analytics_unasked() && analytics_consent() == ANALYTICS_OFF && stats_posts == 0, "statistics: a no is remembered");
	analytics_answer(true);
	stats_begin(settings, true, "1.0\"x\\");
	CHECK(analytics_consent() == ANALYTICS_ON && stats_posts == 1 && strstr(stats_sent[0], "\"title\":\"Cyberworld Endless 1.0\\\"x\\\\\""),
		"statistics: a yes is remembered, its start's view sent, its strings escaped (%s)", stats_sent[0]);
	/* where the system cannot send, or no player started it, nothing asks */
	stats_system = false;
	stats_begin(settings, true, "0.10.0");
	CHECK(!analytics_here() && !analytics_unasked() && stats_posts == 0, "statistics: a system that cannot send never asks");
	stats_system = true;
	stats_begin(settings, false, "0.10.0");
	analytics_run_start();
	CHECK(!analytics_here() && stats_posts == 0, "statistics: a test's or a developer's start neither asks nor sends");
	/* a test's switches: asked as a player is, nothing sent but to its own address */
	CHECK(analytics_dev("statistics=ask") && !analytics_dev("speed=2"), "statistics: --dev statistics= is the statistics'");
	stats_begin(settings, false, "0.10.0");
	CHECK(analytics_unasked(), "statistics: --dev statistics=ask asks");
	analytics_answer(true);
	analytics_run_start();
	CHECK(stats_posts == 0 && !strcmp(analytics_url(), GAME_UMAMI_SEND), "statistics: a test's yes sends nothing to Umami");
	CHECK(analytics_dev("statsurl=http://127.0.0.1:9/api/send") && analytics_dev("statistics=on"), "statistics: --dev statsurl=");
	stats_begin(settings, false, "0.10.0");
	analytics_run_start();
	CHECK(stats_posts == 2 && strstr(stats_sent[1], "\"run-start\"") && !strcmp(analytics_url(), "http://127.0.0.1:9/api/send"),
		"statistics: statsurl's address takes a test's payloads (%d)", stats_posts);
	/* the developer's check: one event of its own, whatever the answer */
	CHECK(analytics_dev("statscheck") && analytics_dev("statistics=off") && analytics_checking(), "statistics: --dev statscheck");
	stats_begin(settings, false, "0.10.0");
	analytics_run_start();
	CHECK(stats_posts == 1 && strstr(stats_sent[0], "\"url\":\"/dev-check\"") && strstr(stats_sent[0], "\"name\":\"game-dev-check\"") &&
		strstr(stats_sent[0], "\"platform\":\"linux\""), "statistics: statscheck sends its one event, and no other (%s)", stats_sent[0]);
	remove(settings);
	rmdir(base);
}

/* The nets' meeting (issue #82, docs/MULTIROM.md, One net): a family's
 * role by its placements; the visitors' gate, one battle in each four, the
 * same at every reroll; the rules that keep visitors out; a visitor's
 * place, family and version against the band; the deep's coin between the
 * nets, reproducible from the run's random numbers */
static EnemyMoved test_enemy(int family, int version, int hp, int dmg) {
	EnemyMoved e;
	memset(&e, 0, sizeof e);
	e.triple[0] = (uint8_t)version;
	e.triple[2] = (uint8_t)family;
	e.stats[0] = (uint8_t)hp; e.stats[1] = (uint8_t)(hp >> 8);
	e.stats[4] = (uint8_t)dmg; e.stats[5] = (uint8_t)(dmg >> 8);
	return e;
}

static Encounter test_battle(int n, int col) {
	Encounter e;
	memset(&e, 0, sizeof e);
	for (int i = 0; i < n; ++i) e.foes[e.nfoes++] = (Foe){ FOE_VIRUS, 2, 0, col, i, 0x300 };
	return e;
}

static void test_visitors(void) {
	CHECK(visitor_role(0, 0, 4) == VISITOR_BACK && visitor_role(0, 1, 3) == VISITOR_BACK && visitor_role(1, 1, 2) == VISITOR_FREE &&
		visitor_role(3, 0, 1) == VISITOR_FRONT && visitor_role(2, 1, 1) == VISITOR_FREE && visitor_role(0, 0, 0) == VISITOR_FREE,
		"visitors: a family's role by its placements");
	/* (one in each four, and the same again) */
	int blocks = 0, odd = 0, at[4] = { 0 };
	for (uint32_t seed = 1; seed <= 64; ++seed)
		for (int b = 0; b < 25; ++b) {
			int n = 0;
			for (int i = 4 * b; i < 4 * b + 4; ++i)
				if (visitor_gate(seed * 2654435761u, i)) { ++n; ++at[i & 3]; odd += visitor_gate(seed * 2654435761u, i) != true; }
			blocks += n == 1;
		}
	CHECK(blocks == 64 * 25 && !odd, "visitors: the gate takes one battle in each four (%d of %d blocks)", blocks, 64 * 25);
	CHECK(at[0] && at[1] && at[2] && at[3], "visitors: the gate's battle not always the same of the four");
	CHECK(!visitor_gate(1, -1), "visitors: no gate before a battle");
	/* (the rules: none without the older net's families, before act 2,
	 * past an opening, with the Gentle helper, in the deep) */
	memset(test_kin, 0, sizeof test_kin);
	test_nkin[VISIT_BN6] = test_nkin[VISIT_BN5] = 0;
	loot_visitors_always = true;
	run.helpers = 0;
	CHECK(!visitor_due(1, 0, 8, BIOME_CENTRAL, ENC_NORMAL), "visitors: none without the older net's families");
	test_nkin[VISIT_BN5] = 1;
	test_kin[VISIT_BN5][0] = (VisitorKin){ 40, VISITOR_FREE, 0, false, true, 10 };
	CHECK(visitor_due(1, 0, 8, BIOME_CENTRAL, ENC_NORMAL) && !visitor_due(1, 0, 3, BIOME_CENTRAL, ENC_NORMAL) &&
		!visitor_due(1, 0, 8, BIOME_CENTRAL, ENC_EASY) && !visitor_due(1, 0, 8, BIOME_CENTRAL, ENC_CHALLENGE) &&
		!visitor_due(1, 0, 14, BIOME_UNDERNET, ENC_NORMAL) && !visitor_due(1, 0, 17, BIOME_GRAVEYARD, ENC_NORMAL) &&
		!visitor_due(1, 0, 19, BIOME_NEST, ENC_NORMAL), "visitors: the rules that keep them out");
	run.helpers = HELP_GENTLE;
	CHECK(!visitor_due(1, 0, 8, BIOME_CENTRAL, ENC_NORMAL), "visitors: none with the Gentle helper");
	run.helpers = 0;
	loot_visitors_always = false;
	/* (a battle of BN6's three viruses, 60 HP each, at act 2: band 150-280,
	 * cap 80, BN6's V1, so BN5's V1EX at most) */
	memset(test_id, 0, sizeof test_id);
	memset(test_role, 0, sizeof test_role);
	EnemyMoved foe = test_enemy(2, 0, 60, 10);
	enemy_moved_set(0x300, &foe);
	static const int hp5[6] = { 40, 60, 80, 100, 120, 140 };
	for (int v = 0; v < 6; ++v) {
		EnemyMoved x = test_enemy(40, v, hp5[v], 10 + 10 * v);
		enemy_moved_set(0x310 + v, &x);
		test_id[VISIT_BN5][40][v] = 0x310 + v;
	}
	run.threat = 0;
	rng_seed(7);
	Encounter e = test_battle(3, 4);
	CHECK(visitor_apply(&e, 5) && e.visitor >= 1 && e.foes[e.visitor - 1].family == 40 && e.foes[e.visitor - 1].id == 0x311 &&
		e.foes[e.visitor - 1].version == 1 && e.foes[e.visitor - 1].col == 4 && (e.from_families >> 40 & 1),
		"visitors: one of the older net's, at its highest version under the band, on the replaced one's panel");
	CHECK(!visitor_apply(&e, 5), "visitors: one a battle");
	/* (the band's top: three of 90 HP, 270, where V1EX's 60 for one of
	 * them is 240 and fits, and five of them nothing fits) */
	EnemyMoved heavy = test_enemy(2, 0, 90, 10);
	enemy_moved_set(0x300, &heavy);
	e = test_battle(3, 4);
	CHECK(visitor_apply(&e, 5) && e.foes[e.visitor - 1].id == 0x311, "visitors: the band kept with the replaced one's HP gone");
	EnemyMoved huge = test_enemy(2, 0, 250, 10);
	enemy_moved_set(0x300, &huge);
	e = test_battle(2, 4);
	Encounter was = e;
	CHECK(!visitor_apply(&e, 5) && !memcmp(&e, &was, sizeof e), "visitors: none where no version keeps the band, the battle as it was");
	enemy_moved_set(0x300, &foe);
	/* (the role: a back family's place is the last column, and only a
	 * back family takes it) */
	test_role[VISIT_BN6][2] = VISITOR_BACK;
	e = test_battle(2, 4);
	CHECK(!visitor_apply(&e, 5), "visitors: none for a back family out of the back");
	e = test_battle(2, 5);
	CHECK(!visitor_apply(&e, 5), "visitors: a family of either role never takes a back family's place");
	test_kin[VISIT_BN5][0].role = VISITOR_BACK;
	CHECK(visitor_apply(&e, 5) && e.foes[e.visitor - 1].col == 5, "visitors: a back family for a back family, at the back");
	test_role[VISIT_BN6][2] = VISITOR_FREE;
	test_kin[VISIT_BN5][0].role = VISITOR_FREE;
	/* (a family the battle holds, a bound one, one not reached yet, a
	 * panel neither plain nor cracked: none) */
	e = test_battle(2, 4);
	e.foes[1].family = 40;
	e.foes[1].id = 0x310;
	CHECK(!visitor_apply(&e, 5), "visitors: never a family the battle holds");
	test_kin[VISIT_BN5][0].visits = false;
	e = test_battle(2, 4);
	CHECK(!visitor_apply(&e, 5), "visitors: none that may not visit");
	test_kin[VISIT_BN5][0].visits = true;
	test_kin[VISIT_BN5][0].stage = 2;
	CHECK(!visitor_apply(&e, 5), "visitors: none its own net meets later");
	test_kin[VISIT_BN5][0].stage = 0;
	test_panel = 7;
	CHECK(!visitor_apply(&e, 5), "visitors: none on a panel neither plain nor cracked");
	test_panel = 2;
	/* (the same seed, the same visitor) */
	test_nkin[VISIT_BN5] = 2;
	test_kin[VISIT_BN5][1] = (VisitorKin){ 41, VISITOR_FREE, 0, false, true, 10 };
	for (int v = 0; v < 6; ++v) test_id[VISIT_BN5][41][v] = 0x310 + v;
	Encounter a = test_battle(3, 4), b = test_battle(3, 4);
	rng_seed(99);
	visitor_apply(&a, 5);
	rng_seed(99);
	visitor_apply(&b, 5);
	CHECK(!memcmp(&a, &b, sizeof a), "visitors: the same seed, the same visitor");
	for (int v = 0; v < 6; ++v) enemy_moved_set(0x310 + v, NULL);
	enemy_moved_set(0x300, NULL);
	test_nkin[VISIT_BN5] = 0;
	/* (the deep: a coin a battle between the nets, the Nest's older half
	 * between its two areas; a territory's always its own; nothing drawn
	 * outside the deep) */
	int none[2] = { -1, -1 }, one[2] = { 24, -1 }, two[2] = { 24, 22 }, seen[3] = { 0 };
	rng_seed(5);
	uint32_t s0 = rng_state();
	CHECK(nets_battle_area(none, 21) == 21 && nets_battle_area(none, -1) == -1 && rng_state() == s0, "the deep: outside it a territory's own, nothing drawn");
	int bn6 = 0, first[64];
	for (int i = 0; i < 1000; ++i) {
		int x = nets_battle_area(one, 21);
		bn6 += x == -1;
		if (i < 64) first[i] = x;
		CHECK(x == -1 || x == 24, "the deep: BN6's or the older net's area");
	}
	CHECK(bn6 > 400 && bn6 < 600, "the deep: a coin between the nets (%d of 1000 BN6's)", bn6);
	rng_seed(5);
	bool same = true;
	for (int i = 0; i < 64; ++i) same &= nets_battle_area(one, 21) == first[i];
	CHECK(same, "the deep: the same seed, the same battles");
	for (int i = 0; i < 1000; ++i) {
		int x = nets_battle_area(two, -1);
		++seen[x == -1 ? 0 : x == 24 ? 1 : 2];
	}
	CHECK(seen[0] > 400 && seen[1] > 150 && seen[2] > 150, "the deep: the Nest's both areas (%d %d %d)", seen[0], seen[1], seen[2]);
}

int main(void) {
	test_sha1();
	test_jobs();
	test_home_words();
	test_lz77();
	test_generation();
	test_layer_make();
	test_layouts_build();
	test_identity();
	test_arrivals();
	test_stairs();
	test_arrow();
	test_kept_walk();
	test_arrow_areas();
	test_arrow_mouths();
	test_guardian_gone();
	test_walks();
	test_way_links();
	test_navicust();
	test_depth_plan();
	test_pacing();
	test_town_moves();
	test_talk();
	test_touch();
	test_padmap();
	test_xsong();
	test_thumb_bl();
	test_thumb_block();
	test_battle_rng();
	test_thumb_imm();
	test_thumb_veneers();
	test_xvirus_recipe();
	test_xvirus_cangard();
	test_xvirus_static();
	test_xnavi_static();
	test_free_space();
	test_claims();
	test_xvirus_dispatch();
	test_xvirus_generated();
	test_xvirus_shims();
	test_enemy_moved();
	test_qr();
	test_backup();
	test_launcher();
	test_bug_cause();
	test_xnavi();
	test_all_star();
	test_beast();
	test_xchips();
	test_xtext();
	test_chip_registry();
	test_chip_records();
	test_chips_image();
	test_soulkinds();
	test_darkchips();
	test_rom_pages();
	test_super_bosses();
	test_analytics();
	test_visitors();
	if (failures) { printf("%d check(s) failed\n", failures); return 1; }
	printf("all core checks passed\n");
	return 0;
}

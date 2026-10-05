/* Where a layer's set pieces stand (epic #49, docs/LEVEL_DESIGN.md, Set
 * pieces; which it holds is net_pieces.c's): Rush's gaps, the invisible
 * paths, the teleport pairs, the obstacles and cubes and the navis who
 * tell their P-Codes, and the arrow lanes. */
#include "net_set_pieces.h"

#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "net_detours.h"
#include "net_gen.h"
#include "net_landmarks.h"

/* Whether panel (x, y) is a Rush gap's or an invisible path's: void in
 * cell[], floor to the map. */
static bool in_gap_of(const NetGap *g, int n, int x, int y) {
	for (int i = 0; i < n; ++i)
		for (int k = 1; k <= g[i].len; ++k)
			if (x == g[i].x + dir_dx[g[i].dir] * k && y == g[i].y + dir_dy[g[i].dir] * k) return true;
	return false;
}

static bool in_gap(int x, int y) { return in_gap_of(layer.gap, layer.ngaps, x, y) || in_gap_of(layer.path, layer.npaths, x, y); }

static bool gap_fits(int x, int y, int d, int len) {
	int sx = dir_dx[(d + 1) % 4], sy = dir_dy[(d + 1) % 4];
	for (int k = 1; k <= len + 4; ++k)
		for (int s = -2; s <= 2; ++s) {
			if (k < len && abs(s) > 1) continue;
			int i = x + dir_dx[d] * k + sx * s, j = y + dir_dy[d] * k + sy * s;
			bool island = k > len && k <= len + 3 && abs(s) <= 1;
			if (i < 0 || j < 0 || i >= MAP_W || j >= MAP_H || layer.cell[j][i] != C_VOID || ng_prop_at_cell(i, j) || in_gap(i, j)) return false;
			if ((island || (k <= len && !s)) && !win_in(i, j)) return false;
		}
	return true;
}

static bool stand_ok(int x, int y, int d) {
	int bx = x - dir_dx[d], by = y - dir_dy[d];
	return ng_floor_cell(x, y) && ng_floor_cell(bx, by) && !layer.level[y][x] && !layer.level[by][bx] && !layer_on_way(x, y) &&
		!ng_in_arena(x, y) && ng_detour[y][x] >= 1 && ng_cell_free(x, y) && ng_cell_free(bx, by) && !ng_navi_near(x, y);
}

/* The best stand for a gap `want` panels long, else shorter; the farthest
 * off the way first, a walkway's tip before a platform's edge. */
GapSite ng_plan_gap(int want) {
	GapSite best = { -1, -1, 0, 0, -1 };
	for (int len = want; len >= 1 && best.x < 0; --len)
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x)
				for (int d = 0; d < 4; ++d) {
					if (!stand_ok(x, y, d) || !gap_fits(x, y, d, len)) continue;
					int sx = dir_dx[(d + 1) % 4], sy = dir_dy[(d + 1) % 4];
					bool tip = !ng_floor_cell(x + sx, y + sy) && !ng_floor_cell(x - sx, y - sy);
					int score = ng_detour[y][x] * 2 + (tip ? 8 : 0);
					if (score > best.score) best = (GapSite){ x, y, d, len, score };
				}
	if (best.x >= 0) {
		ng_reserved[best.y][best.x] = 1;
		ng_reserved[best.y - dir_dy[best.d]][best.x - dir_dx[best.d]] = 1;
		ng_hush(best.x, best.y);
	}
	return best;
}

/* The island past the gap, its data in its middle (a tier up past one
 * panel: two or three bones' worth), and the gap itself; nothing where the
 * island would leave the map too big for the game's tile map. */
void ng_carve_gap(const GapSite *g, int rise) {
	int mx = g->x + dir_dx[g->d] * (g->len + 2), my = g->y + dir_dy[g->d] * (g->len + 2);
	carve_shape(SHAPE_RECT, mx - 1, my - 1, 3, 3);
	if (!ng_fits(rise) || layer.ngaps >= MAX_GAPS) {
		for (int j = my - 1; j <= my + 1; ++j)
			for (int i = mx - 1; i <= mx + 1; ++i) layer.cell[j][i] = C_VOID;
		return;
	}
	add_room(mx - 1, my - 1, 3, 3, ROOM_PAD);
	layer.gap[layer.ngaps++] = (NetGap){ g->x, g->y, g->d, g->len, true };
	NetObj *o = ng_add_obj(OBJ_MYSTERY, mx, my);
	if (o) { o->param = g->len >= 2 ? 2 : 1; o->prize = true; }
}

/* ---- Invisible paths (issue #46; docs/LEVEL_DESIGN.md, Set pieces) ----
 * BN6's floor drawn as void (Seaside Area 1, Sky Area 2, Underground 1,
 * Undernet 2): a walkway's tip aimed across the void at a lonely pad, the
 * floor between drawn as void, its pad holding what a player who walks on
 * finds. Its site is a Rush gap's (plan_gap), planned once the islands
 * stand; a navi near the tip says what he saw. */
#define PATH_LEN 3   /* its panels, at most */

/* Where the navi who hints may stand: two to eight panels from the tip,
 * the nearest, apart from what else stands there, off the way and beside
 * it and out of walkways' mouths; false where none may (no path without
 * its cue). */
static bool hinter_spot(const GapSite *g, int *hx, int *hy) {
	for (int r = 2; r <= 8; ++r)
		for (int dy = -r; dy <= r; ++dy)
			for (int dx = -r; dx <= r; ++dx) {
				int x = g->x + dx, y = g->y + dy;
				if ((abs(dx) != r && abs(dy) != r) || x < 1 || y < 1 || x >= MAP_W - 1 || y >= MAP_H - 1) continue;
				if (!ng_cell_free(x, y) || ng_way_band[y][x] || layer.level[y][x] || ng_hushed[y][x] || ng_beside_narrow(x, y) || ng_by_walkway(x, y) ||
					!ng_apart(x, y) || ng_cuts_way(x, y))
					continue;
				*hx = x;
				*hy = y;
				return true;
			}
	return false;
}

/* The path across the void from the site, its pad and its one thing (the
 * best kind: an HPMemory now and then, else a chip three tiers up), and
 * the navi who hints at it; none where the pad would leave the map too
 * big for the game's tile map, or where no navi may stand to hint. */
static void carve_hidden(const GapSite *g, int rise) {
	int mx = g->x + dir_dx[g->d] * (g->len + 2), my = g->y + dir_dy[g->d] * (g->len + 2), hx, hy;
	if (layer.npaths >= MAX_PATHS || !hinter_spot(g, &hx, &hy)) return;
	carve_shape(SHAPE_RECT, mx - 1, my - 1, 3, 3);
	if (!ng_fits(rise)) {
		for (int j = my - 1; j <= my + 1; ++j)
			for (int i = mx - 1; i <= mx + 1; ++i) layer.cell[j][i] = C_VOID;
		return;
	}
	add_room(mx - 1, my - 1, 3, 3, ROOM_PAD);
	layer.path[layer.npaths++] = (NetGap){ g->x, g->y, g->d, g->len, true };
	NetObj *o = ng_add_obj(OBJ_MYSTERY, mx, my);
	if (o) { o->param = 2; o->prize = true; }
	if ((o = ng_add_obj(OBJ_NPC, hx, hy))) {
		o->param = rng_range(0, 5);
		o->npc_line = rng_range(0, 255);
		layer.hinter = layer.nobj;
	}
}

void ng_place_hidden(int rise) {
	GapSite site = ng_plan_gap(PATH_LEN);
	if (site.x >= 0) carve_hidden(&site, rise);
}

/* ---- Teleports (issue #44) ----
 * BN6's gem marks a teleport pad, always one of a pair, each warping
 * MegaMan to the other within the map (Green Area 1 has four pairs): never
 * decoration. A layer's pair is a quick way back: one pad where a long
 * detour ends, the other by the way, as far from it on foot as can be;
 * the detour's data lies on the far pad's rim, its middle the gem's. */
static int walk_between(int ax, int ay, int bx, int by) {
	static int16_t dist[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) dist[y][x] = -1;
	int h = 0, t = 0;
	dist[ay][ax] = 0;
	qx[t] = (int16_t)ax; qy[t++] = (int16_t)ay;
	while (h < t && dist[by][bx] < 0) {
		int x = qx[h], y = qy[h++];
		for (int k = 0; k < 4; ++k) {
			int nx = x + d4[k][0], ny = y + d4[k][1];
			if (!ng_floor_cell(nx, ny) || dist[ny][nx] >= 0) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	return dist[by][bx];
}

/* Whether panel (x, y) can hold a teleport: ground floor all round it (the
 * gem draws its own diamond over it), nothing standing there or beside it,
 * no stair, not the arena, its detour `lo` to `hi` (its trigger reaches a
 * cell into the panels round it, so the way keeps two panels off: no one
 * is warped walking by). */
static bool teleport_spot(int x, int y, int lo, int hi) {
	if (ng_detour[y][x] < lo || ng_detour[y][x] > hi || ng_in_arena(x, y) || ng_near_stair(x, y)) return false;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) {
			int i = x + dx, j = y + dy;
			if (!ng_floor_cell(i, j) || layer.level[j][i] || layer_on_way(i, j) || ng_reserved[j][i] || ng_object_at(i, j)) return false;
		}
	return ng_cell_free(x, y) && !ng_near_talker(x, y);
}

/* An island for a teleport's far pad, as ACDC's homepage has an isolated
 * square reached by its teleport alone: a 4x4 of void, its gem a panel in
 * from its top corner, with void two panels round it and no prop, inside
 * the camera's window, the nearest such to (bx, by), so it is seen from
 * the pad that leads there. */
static bool teleport_island_site(int bx, int by, int *ox, int *oy) {
	int best = 1 << 30;
	for (int y = 3; y < MAP_H - 4; ++y)
		for (int x = 3; x < MAP_W - 4; ++x) {
			int d = abs(x - bx) + abs(y - by);
			if (d >= best) continue;
			bool ok = true;
			for (int j = -3; j <= 4 && ok; ++j)
				for (int i = -3; i <= 4 && ok; ++i)
					ok = layer.cell[y + j][x + i] == C_VOID && !ng_prop_at_cell(x + i, y + j) &&
						(i < -1 || i > 2 || j < -1 || j > 2 || win_in(x + i, y + j));
			if (ok) { best = d; *ox = x; *oy = y; }
		}
	return best < (1 << 30);
}

/* The pair: a quick way back from where a long detour ends to the way,
 * else an island of its own (its data on a corner, carved last). */
void ng_plan_teleport(void) {
	int best = 0, ax = -1, ay = -1, bx = -1, by = -1;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x) {
			if (!teleport_spot(x, y, 7, 999)) continue;
			for (int j = 1; j < MAP_H - 1; ++j)
				for (int i = 1; i < MAP_W - 1; ++i) {
					if (!teleport_spot(i, j, 2, 6)) continue;
					/* (a walk saved of sixteen panels at least, or it is no way back) */
					int w = walk_between(x, y, i, j);
					if (w > best && w >= 16) { best = w; ax = x; ay = y; bx = i; by = j; }
				}
		}
	if (ax < 0) {
		/* (the island: by the pad near the way nearest the arrival's) */
		for (int y = 1; y < MAP_H - 1 && bx < 0; ++y)
			for (int x = 1; x < MAP_W - 1 && bx < 0; ++x)
				if (teleport_spot(x, y, 2, 6)) { bx = x; by = y; }
		if (bx < 0 || !teleport_island_site(bx, by, &ax, &ay)) return;
		layer.teleport_island = true;
	}
	layer.teleport_x[0] = ax; layer.teleport_y[0] = ay;
	layer.teleport_x[1] = bx; layer.teleport_y[1] = by;
	layer.nteleports = 1;
	for (int k = 0; k < 2; ++k)
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) ng_reserved[layer.teleport_y[k] + dy][layer.teleport_x[k] + dx] = 1;
}

/* The teleport's island, once the rest stands: a 4x4 with its gem a panel
 * in from its top corner, and its data on the bottom corner, straight
 * below where MegaMan lands; nothing where it would leave the map too big
 * for the game's tile map. He lands on the gem's rim below it (the warp's
 * facing 4), still beside its trigger: a step up and to either side
 * beamed him back. On the 3x3 island every corner lay that way, its data
 * reached only round the gem's rim, half over the void. */
void ng_carve_teleport_island(int rise) {
	int mx = layer.teleport_x[0], my = layer.teleport_y[0];
	carve_shape(SHAPE_RECT, mx - 1, my - 1, 4, 4);
	if (!ng_fits(rise)) {
		for (int j = my - 1; j <= my + 2; ++j)
			for (int i = mx - 1; i <= mx + 2; ++i) layer.cell[j][i] = C_VOID;
		layer.nteleports = 0;
		layer.teleport_island = false;
		return;
	}
	add_room(mx - 1, my - 1, 4, 4, ROOM_PAD);
	NetObj *o = ng_add_obj(OBJ_MYSTERY, mx + 2, my + 2);
	if (o) { o->param = 1; o->prize = true; }
}

/* ---- Obstacles (issue #42) ----
 * BN6's Link Navi obstacles stand in the mouth of a pocket that holds one
 * thing (SubMemory, Attack+1, HP+100, a purple data), seen from the way.
 * A pocket's mouth is a walkway's first panel off a wider floor, off the
 * way; closed, it cuts off six to forty panels and nothing the way or a
 * service needs; its one thing a blue data where it ends. */
static uint8_t pocket[MAP_H][MAP_W];

/* The floor that panel (wx, wy) closed off cuts from the arrival, marked in
 * `pocket`; how much, or -1 where it holds what must stay reachable. */
static int pocket_of(int wx, int wy) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	static uint8_t seen[MAP_H][MAP_W];
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	memset(seen, 0, sizeof seen);
	int h = 0, t = 0, sx = (int)layer.obj[0].x, sy = (int)layer.obj[0].y;
	seen[sy][sx] = 1;
	seen[wy][wx] = 1;
	qx[t] = (int16_t)sx; qy[t++] = (int16_t)sy;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		for (int k = 0; k < 4; ++k) {
			int nx = x + d4[k][0], ny = y + d4[k][1];
			if (!ng_floor_cell(nx, ny) || seen[ny][nx]) continue;
			seen[ny][nx] = 1;
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	int n = 0;
	memset(pocket, 0, sizeof pocket);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!ng_floor_cell(x, y) || seen[y][x]) continue;
			if (ng_way_band[y][x] == 2 || layer.level[y][x] || ng_reserved[y][x] || ng_object_at(x, y) || ng_near_stair(x, y) || ng_in_arena(x, y)) return -1;
			pocket[y][x] = 1;
			++n;
		}
	return n;
}

/* Whether (x, y), entered from the floor behind it towards `d`, is a
 * walkway's first panel off a wider floor, off the way. */
static bool pocket_mouth(int x, int y, int d) {
	int sx = dir_dx[(d + 1) % 4], sy = dir_dy[(d + 1) % 4], bx = x - dir_dx[d], by = y - dir_dy[d];
	if (!ng_floor_cell(x, y) || !ng_floor_cell(bx, by) || ng_way_band[y][x] == 2 || layer.level[y][x] || layer.level[by][bx]) return false;
	if (ng_floor_cell(x + sx, y + sy) || ng_floor_cell(x - sx, y - sy)) return false;
	return (ng_floor_cell(bx + sx, by + sy) || ng_floor_cell(bx - sx, by - sy)) && ng_cell_free(bx, by) && !ng_navi_near(x, y);
}

void ng_plan_obstacle(int kind) {
	int best = -1, bx = 0, by = 0, bd = 0;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x)
			for (int d = 0; d < 4; ++d) {
				if (!pocket_mouth(x, y, d)) continue;
				int n = pocket_of(x, y);
				if (n < 6 || n > 40) continue;
				/* (seen from the way: its mouth near it; a pad in it) */
				int score = 20 - (ng_detour[y][x] < 20 ? ng_detour[y][x] : 20) + (n >= 9 && n <= 20 ? 5 : 0);
				if (score > best) { best = score; bx = x; by = y; bd = d; }
			}
	if (best < 0 || layer.nblocks >= MAX_BLOCKS) return;
	pocket_of(bx, by);
	/* its one thing where it ends: the farthest panel from the mouth */
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	static int16_t dist[MAP_H][MAP_W];
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) dist[y][x] = -1;
	int h = 0, t = 0, rx = bx, ry = by;
	dist[by][bx] = 0;
	qx[t] = (int16_t)bx; qy[t++] = (int16_t)by;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		if (dist[y][x] > dist[ry][rx]) { rx = x; ry = y; }
		for (int k = 0; k < 4; ++k) {
			int nx = x + dir_dx[k], ny = y + dir_dy[k];
			if (!ng_floor_cell(nx, ny) || dist[ny][nx] >= 0 || !pocket[ny][nx]) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	ng_pad_middle(&rx, &ry);
	layer.block[layer.nblocks++] = (NetBlock){ bx, by, bd, kind, rx, ry };
	/* (nothing else in the pocket, its mouth or the floor before it) */
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (pocket[y][x]) ng_reserved[y][x] = 1;
	ng_reserved[by][bx] = 1;
	ng_reserved[by - dir_dy[bd]][bx - dir_dx[bd]] = 1;
	ng_hush(bx, by);
}

/* A cube's lock (layer_cube_kind): a number door asks how many braziers
 * burn on the layer, so one with fewer than two asks a P-Code instead (a
 * skull door the Net Dealer did not plan for would want a WWW-ID he never
 * stocked; issue #47). */
int ng_cube_kind(int depth, int biome, int kind) {
	layer.braziers = 0;
	for (int i = 0; i < layer.nprops; ++i) layer.braziers += layer.props[i].kind == PROP_SPRITE && layer.props[i].look == LOOK_BRAZIER;
	int k = layer_cube_kind(depth, biome, kind);
	return k == BLOCK_NUMBER && layer.braziers < 2 ? BLOCK_PCODE : k;
}

/* The layer's security cube asking a P-Code (issue #45), -1 none. */
int ng_pcode_cube(void) {
	for (int i = 0; i < layer.nblocks; ++i)
		if (layer.block[i].kind == BLOCK_PCODE) return i;
	return -1;
}

/* Whether a bystander may stand at (x, y): two panels at least from what
 * else stands there. */
bool ng_apart(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i)
		if (abs((int)layer.obj[i].x - x) <= 2 && abs((int)layer.obj[i].y - y) <= 2) return false;
	return true;
}

static bool add_teller(int x, int y) {
	NetObj *o = ng_add_obj(OBJ_NPC, x, y);
	if (!o) return false;
	o->param = rng_range(0, 5);
	o->npc_line = rng_range(0, 255);
	layer.teller = layer.nobj;
	return true;
}

/* Whether the P-Code's teller may stand at (x, y): apart from what else
 * stands there, off the set pieces' quiet, and a walk from the cube (the
 * key on another way than the lock: one beside it told it at once). */
#define TELLER_WALK 8
static bool teller_ok(int x, int y, const NetBlock *b) {
	return ng_apart(x, y) && !ng_hushed[y][x] && walk_between(b->x, b->y, x, y) >= TELLER_WALK;
}

/* A free floor panel off the way with floor all round it. */
static bool open_floor(int x, int y) {
	return ng_cell_free(x, y) && !ng_way_band[y][x] && !ng_behind_gap(x, y) && !ng_beside_narrow(x, y) && !ng_by_walkway(x, y) && ng_floor_cell(x + 1, y) &&
		ng_floor_cell(x - 1, y) && ng_floor_cell(x, y + 1) && ng_floor_cell(x, y - 1) && !ng_cuts_way(x, y);
}

/* The navi who knows the P-Code: a bystander in a room a walk from the
 * cube; else on any free floor off the way; else the cube takes a toll
 * instead (a lock no one can open would close its pocket for good). */
void ng_place_teller(const int *order, int n) {
	int x, y, c = ng_pcode_cube();
	if (c < 0) return;
	const NetBlock *b = &layer.block[c];
	for (int tries = 0; tries < 16 && n; ++tries) {
		const Room *r = &layer.rooms[order[rng_range(0, n - 1)]];
		if (ng_room_spot_in(r, &x, &y, true) && teller_ok(x, y, b) && add_teller(x, y)) return;
	}
	for (y = 1; y < MAP_H - 1; ++y)
		for (x = 1; x < MAP_W - 1; ++x)
			if (open_floor(x, y) && teller_ok(x, y, b) && add_teller(x, y)) return;
	layer.block[c].kind = BLOCK_TOLL;
}

/* The pocket's one thing, once the rest stands: a blue data of the best
 * quality (a Cross is the key; a lock never pays less than the open data). */
void ng_place_block_rewards(void) {
	for (int i = 0; i < layer.nblocks; ++i) {
		NetObj *o = ng_add_obj(OBJ_MYSTERY, layer.block[i].rx, layer.block[i].ry);
		if (o) { o->param = 2; o->prize = true; }
	}
}

/* ---- Arrow lanes (issue #43; docs/LEVEL_DESIGN.md, Set pieces) ----
 * BN6's arrow panels carry MegaMan one way, input held, until he is past
 * them. A layer's lane is the quick way back: one wide, its panels across
 * the void from where a long detour ends to the floor by the way, so the
 * walk in is long and the ride out short. Walked against its arrows, its
 * first panel carries MegaMan back, so it is never a way in. Planned and
 * carved once the rest stands. */
#define LANE_FAR 5     /* the detour it leaves from, at least */
#define LANE_SAVES 5   /* the walk it saves, at least */
#define LANE_MAX 5     /* its panels, at most */

int layer_lane_dir(int x, int y) {
	for (int i = 0; i < layer.nlanes; ++i) {
		const NetLane *l = &layer.lane[i];
		for (int k = 1; k <= l->len; ++k)
			if (x == l->x + dir_dx[l->dir] * k && y == l->y + dir_dy[l->dir] * k) return l->dir;
	}
	return -1;
}

bool layer_step_ok(int x, int y, int nx, int ny) {
	int from = layer_lane_dir(x, y), to = layer_lane_dir(nx, ny);
	if (from >= 0 && (nx - x != dir_dx[from] || ny - y != dir_dy[from])) return false;
	return to < 0 || (nx - x == dir_dx[to] && ny - y == dir_dy[to]);
}

/* The floor a walk from the arrival reaches (no island past a gap or a
 * teleport's void). */
static void reach_from_arrival(uint8_t reach[MAP_H][MAP_W]) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(reach, 0, MAP_W * MAP_H);
	if (!layer.nobj) return;
	int h = 0, t = 0;
	qx[t] = (int16_t)layer.obj[0].x; qy[t++] = (int16_t)layer.obj[0].y;
	reach[qy[0]][qx[0]] = 1;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		for (int k = 0; k < 4; ++k) {
			int nx = x + dir_dx[k], ny = y + dir_dy[k];
			if (!ng_floor_cell(nx, ny) || reach[ny][nx]) continue;
			reach[ny][nx] = 1;
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
}

/* The void panels from (x, y) towards d with the void beside each, before
 * the floor past them: 1 to LANE_MAX of them, nothing in the void there (a
 * prop's sprite, a Rush gap); 0 where they don't lead to free ground floor. */
static int lane_len(int x, int y, int d) {
	int sx = dir_dx[(d + 1) % 4], sy = dir_dy[(d + 1) % 4];
	for (int k = 1; k <= LANE_MAX + 1; ++k) {
		int cx = x + dir_dx[d] * k, cy = y + dir_dy[d] * k;
		if (ng_floor_cell(cx, cy))
			return k > 1 && !layer.level[cy][cx] && ng_cell_free(cx, cy) && !ng_in_arena(cx, cy) && !ng_near_stair(cx, cy) ? k - 1 : 0;
		if (k > LANE_MAX || !ng_void_cell(cx, cy) || !ng_void_cell(cx + sx, cy + sy) || !ng_void_cell(cx - sx, cy - sy) || ng_prop_at_cell(cx, cy) ||
			in_gap(cx, cy) || in_gap(cx + sx, cy + sy) || in_gap(cx - sx, cy - sy))
			return 0;
	}
	return 0;
}

/* A lane from (x, y) towards d, scored: the walk it saves, from a detour
 * LANE_FAR or more off the way to the floor 2 or less off it; -1 none. */
static int lane_score(const uint8_t reach[MAP_H][MAP_W], int x, int y, int d, int *len) {
	if (!(*len = lane_len(x, y, d))) return -1;
	int ex = x + dir_dx[d] * (*len + 1), ey = y + dir_dy[d] * (*len + 1);
	if (!reach[ey][ex] || ng_detour[ey][ex] < 0 || ng_detour[ey][ex] > 3) return -1;
	int saved = walk_between(x, y, ex, ey) - (*len + 1);
	return saved < LANE_SAVES ? -1 : saved * 4 - *len * 2 - ng_detour[ey][ex];
}

/* The lane: from a free ground panel LANE_FAR or more off the way, in a
 * way the area's maps draw arrows for, the walk it saves the most. */
static bool plan_lane(const LayerKit *kit, NetLane *out) {
	static uint8_t reach[MAP_H][MAP_W];
	if (!kit || !kit->arrows) return false;
	reach_from_arrival(reach);
	int best = -1;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x) {
			if (!reach[y][x] || ng_detour[y][x] < LANE_FAR || layer.level[y][x] || !ng_cell_free(x, y) || ng_in_arena(x, y)) continue;
			for (int d = 0; d < 4; ++d) {
				int len, score = kit->arrows >> d & 1 ? lane_score(reach, x, y, d, &len) : -1;
				if (score > best) { best = score; *out = (NetLane){ x, y, d, len }; }
			}
		}
	return best >= 0;
}

/* Its panels floor, and its ends kept clear: a navi at its far end would
 * stop the ride with MegaMan's input held. */
static void carve_lane(const NetLane *l) {
	for (int k = 0; k <= l->len + 1; ++k) {
		int cx = l->x + dir_dx[l->dir] * k, cy = l->y + dir_dy[l->dir] * k;
		if (k >= 1 && k <= l->len) { layer.cell[cy][cx] = C_PATH; layer.level[cy][cx] = 0; }
		ng_reserved[cy][cx] = 1;
	}
	if (layer.nlanes < MAX_LANES) layer.lane[layer.nlanes++] = *l;
}

void ng_place_lane(const LayerKit *kit) {
	NetLane lane;
	if (plan_lane(kit, &lane)) carve_lane(&lane);
}

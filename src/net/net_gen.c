/* Layer generation: a layout after the area's own maps (net_layouts.c),
 * then points of interest (docs/LEVEL_DESIGN.md). */
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "net.h"
#include "net_arena.h"
#include "net_layouts.h"
#include "net_shapes.h"
#include "run.h"

Layer layer;

int biome_for_depth(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	if (p >= 18) return BIOME_NEST;
	return run.biome_order[p / 3];
}

bool is_boss_depth(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	return p % 3 == 2 || p == 18;
}

static NetObj *add_obj(int type, int x, int y) {
	if (layer.nobj >= MAX_OBJS) return NULL;
	NetObj *o = &layer.obj[layer.nobj++];
	memset(o, 0, sizeof *o);
	o->type = type;
	o->x = (float)x + 0.5f;
	o->y = (float)y + 0.5f;
	o->solid = type != OBJ_WARP_IN && type != OBJ_EXIT && type != OBJ_UNDERNET && type != OBJ_SECRET_GATE && type != OBJ_RETURN;
	return o;
}

static bool cell_free(int x, int y) {
	if (layer.cell[y][x] != C_PATH) return false;
	for (int i = 0; i < layer.nstairs; ++i)
		if (x >= layer.stair[i].x && x < layer.stair[i].x + 2 && y >= layer.stair[i].y && y < layer.stair[i].y + 2) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if ((int)layer.obj[i].x == x && (int)layer.obj[i].y == y) return false;
	return true;
}

/* A free cell inside a room, off its middle so paths stay clear. */
static bool room_spot(const Room *r, int *ox, int *oy) {
	for (int tries = 0; tries < 40; ++tries) {
		int x = r->x + rng_range(0, r->w - 1), y = r->y + rng_range(0, r->h - 1);
		if (tries < 30 && x == r->ax && y == r->ay) continue;
		if (cell_free(x, y)) { *ox = x; *oy = y; return true; }
	}
	return false;
}

static int bfs_far(int from) {
	/* Room graph distance by flood fill over cells; returns farthest room. */
	static int16_t dist[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(dist, -1, sizeof dist);
	int h = 0, t = 0;
	qx[t] = (int16_t)layer.rooms[from].ax; qy[t++] = (int16_t)layer.rooms[from].ay;
	dist[qy[0]][qx[0]] = 0;
	while (h < t) {
		int x = qx[h], y = qy[h++];
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = x + d[k][0], ny = y + d[k][1];
			if (layer.cell[ny][nx] != C_PATH || dist[ny][nx] >= 0) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	int best = from, bd = -1;
	for (int i = 0; i < layer.nrooms; ++i) {
		int d = dist[layer.rooms[i].ay][layer.rooms[i].ax];
		if (d > bd) { bd = d; best = i; }
	}
	return best;
}

#define MIN_FLOOR 120   /* panels a layer has at least */
#define ARENA_SIZE 5    /* the guardian's arena, panels a side */

static int floor_cells(void) {
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) n += layer.cell[y][x] == C_PATH;
	return n;
}

/* Whether the layer's tile map fits the game's 0x14000-byte buffer, as
 * netmap centres and sizes it (panel edges up to 28 units off, `rise` for a
 * raised floor). */
static bool fits(int rise) {
	int u0 = 1 << 30, u1 = -(1 << 30), v0 = 1 << 30, v1 = -(1 << 30);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (layer.cell[y][x] != C_PATH) continue;
			if (x - y < u0) u0 = x - y;
			if (x - y > u1) u1 = x - y;
			if (x + y < v0) v0 = x + y;
			if (x + y > v1) v1 = x + y;
		}
	int uh = (u1 - u0) / 2 + 2, vh = (v1 - v0) / 2 + 2;   /* half extents, rounding and centring slack */
	int tw = 2 * ((32 * uh + 88 + 64 + 7) / 8) + 1, th = 2 * ((16 * vh + 14 + 48 + rise + 7) / 8) + 1;
	return tw <= 255 && th <= 255 && tw * th * 4 <= 0x14000;
}

/* MegaMan arrives at the pad nearest the top of the screen (least x + y),
 * which becomes room 0. */
static void choose_arrival(void) {
	int best = 0, bv = 1 << 30;
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *r = &layer.rooms[i];
		int v = r->ax + r->ay - (r->kind == ROOM_PAD ? 6 : 0);
		if (v < bv) { bv = v; best = i; }
	}
	Room t = layer.rooms[0];
	layer.rooms[0] = layer.rooms[best];
	layer.rooms[best] = t;
}

/* Object `type` in the next room of `order`; once each has one, again in
 * the big ones (fields and platforms of 16 cells or more). */
static NetObj *place(int type, const int *order, int n, int next, int *x, int *y) {
	int r = -1;
	if (next < n) r = order[next];
	else {
		int big[MAX_ROOMS], nb = 0;
		for (int i = 0; i < n; ++i)
			if (layer.rooms[order[i]].w * layer.rooms[order[i]].h >= 16) big[nb++] = order[i];
		if (nb) r = big[rng_range(0, nb - 1)];
	}
	return r >= 0 && room_spot(&layer.rooms[r], x, y) ? add_obj(type, *x, *y) : NULL;
}

void layer_generate(uint32_t seed, int depth, int biome, int kind, unsigned stair_dirs, int rise) {
	memset(&layer, 0, sizeof layer);
	rng_seed(seed);
	layer.biome = biome;
	layer.kind = kind;
	layer.boss_layer = kind == LAYER_NORMAL && is_boss_depth(depth);
	if (kind == LAYER_SECRET) layer.boss_layer = !run.secret_cleared;
	layer.arena = layer.ante = -1;
	ArenaInfo arena = { -1, -1, 0, 0, 0 };

	/* bigger layouts deeper into a cycle */
	int p = (depth - 1) % CYCLE_LAYERS;
	int size = depth > CYCLE_LAYERS || p >= 12 ? 2 : p >= 6 || kind != LAYER_NORMAL ? 1 : 0;
	/* an act's three layers each in another of the area's layouts */
	int planned = kind == LAYER_NORMAL
		? layout_in_act(biome, run.seed ^ (uint32_t)((depth - 1) / 3 + 1) * 0x9E3779B9u, (depth - 1) % 3)
		: layout_pick(biome);
	for (int attempt = 0; attempt < 12; ++attempt) {
		memset(layer.cell, 0, sizeof layer.cell);
		layer.nrooms = 0;
		arena.room = -1;
		/* the planned layout, then any of the area's, last the plainest at its smallest */
		bool last = attempt == 11;
		layer.layout = last ? LAYOUT_ROUTE : attempt < 6 ? planned : layout_pick(biome);
		layout_build(layer.layout, biome, last ? 0 : size);
		if (layer.nrooms < 3) continue;
		choose_arrival();
		connect_all();
		if (floor_cells() < MIN_FLOOR || !fits(rise)) continue;
		/* a guardian waits in an arena of its own at the far end */
		if (!layer.boss_layer || last) break;
		if (arena_attach(ARENA_SIZE, &arena) >= 0 && fits(rise)) break;
	}

	if (layer.arena < 0 && layer.boss_layer && arena.room >= 0 && arena.room < layer.nrooms) {
		layer.arena = arena.room;
		layer.ante = arena.ante;
	}
	layer.exit_room = layer.arena >= 0 ? layer.arena : bfs_far(0);
	layer_raise_rooms(seed, stair_dirs, rise);
	int cx = layer.rooms[0].ax, cy = layer.rooms[0].ay;
	add_obj(OBJ_WARP_IN, cx, cy);
	cx = layer.rooms[layer.exit_room].ax;
	cy = layer.rooms[layer.exit_room].ay;
	/* in an arena the exit waits behind the guardian, who holds the middle */
	int bx = cx, by = cy;
	if (layer.arena >= 0) { cx = arena.exit_x; cy = arena.exit_y; }
	NetObj *exit = add_obj(kind == LAYER_NORMAL ? OBJ_EXIT : OBJ_RETURN, cx, cy);
	if (layer.boss_layer && exit) {
		if (layer.arena < 0) {
			/* no room for an arena: the guardian stands before the exit */
			for (int d = 0; d < 4; ++d) {
				static const int off[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
				if (layer.cell[cy + off[d][1]][cx + off[d][0]] == C_PATH) { bx = cx + off[d][0]; by = cy + off[d][1]; break; }
			}
		}
		NetObj *b = add_obj(OBJ_BOSS, bx, by);
		if (b) {
			layer.boss_navi = run.boss_order[biome];
			b->param = layer.boss_navi;
		}
	}

	/* Points of interest in the other rooms. */
	int biome_layer = (depth - 1) % 3;
	bool shop = kind == LAYER_NORMAL && (biome_layer == 1 || rng_range(0, 99) < 25);
	bool heal = rng_range(0, 99) < (layer.boss_layer ? 70 : 30);
	bool trader = rng_range(0, 99) < 25;
	bool programs = kind == LAYER_NORMAL && biome_layer == 1 && rng_range(0, 99) < 60;
	bool bugtrader = kind == LAYER_UNDERNET || (biome == BIOME_GRAVEYARD && rng_range(0, 99) < 40);
	trader &= !bugtrader;   /* the trade screen serves one trader per map */
	bool challenge = rng_range(0, 99) < 20 + depth;
	bool undernet = kind == LAYER_NORMAL && depth >= 4 && !layer.boss_layer &&
		rng_range(0, 99) < (biome == BIOME_GRAVEYARD ? 40 : 12);
	bool secret = kind == LAYER_UNDERNET && !run.secret_cleared;

	int order[MAX_ROOMS], n = 0;
	for (int i = 0; i < layer.nrooms; ++i) if (i != 0 && i != layer.exit_room) order[n++] = i;
	for (int i = n - 1; i > 0; --i) { int j = rng_range(0, i); int t = order[i]; order[i] = order[j]; order[j] = t; }
	/* services on the bigger platforms, the pads left for the better data */
	for (int i = 0, k = 0; i < n; ++i)
		if (layer.rooms[order[i]].kind != ROOM_PAD) { int t = order[k]; order[k++] = order[i]; order[i] = t; }
	int next = 0;
	int x, y;
#define PLACE(t) place((t), order, n, next, &x, &y)
	if (layer.arena >= 0) {
		/* the last stop before the arena: a heal and the Net Dealer, as the
		 * rooms before Hades' guardians hold a fountain and Charon */
		if (room_spot(&layer.rooms[layer.ante], &x, &y)) add_obj(OBJ_HEAL, x, y);
		if (kind == LAYER_NORMAL && room_spot(&layer.rooms[layer.ante], &x, &y)) add_obj(OBJ_SHOP, x, y);
		shop = heal = false;
	}
	if (shop) { PLACE(OBJ_SHOP); ++next; }
	if (heal) { PLACE(OBJ_HEAL); ++next; }
	if (trader) { PLACE(OBJ_TRADER); ++next; }
	if (programs) { PLACE(OBJ_PROGRAMS); ++next; }
	if (bugtrader) { PLACE(OBJ_BUGTRADER); ++next; }
	if (challenge) { PLACE(OBJ_CHALLENGE); ++next; }
	if (undernet) { PLACE(OBJ_UNDERNET); ++next; }
	if (secret) { PLACE(OBJ_SECRET_GATE); ++next; }
	/* Rooms holding better data, more of them deeper and in the Undernet. */
	int rich = 1 + (depth > 6) + (kind == LAYER_UNDERNET);
	for (int k = 0; k < rich; ++k, ++next) {
		NetObj *o = PLACE(OBJ_MYSTERY);
		if (o) o->param = rng_range(0, 99) < 50 ? 1 : 2;
	}
	/* Mystery data scattered through the rest, most at dead ends: the side
	 * ways BN6 rewards exploring */
	static int dx[256], dy[256];
	int nde = dead_ends(dx, dy, 256), di = 0;
	for (int i = nde - 1; i > 0; --i) {
		int j = rng_range(0, i), tx = dx[i], ty = dy[i];
		dx[i] = dx[j]; dy[i] = dy[j]; dx[j] = tx; dy[j] = ty;
	}
	int md = 3 + rng_range(0, 2) + size;
	for (int k = 0; k < md; ++k) {
		bool got = false;
		if (rng_range(0, 99) < 70)
			while (di < nde && !got) { x = dx[di]; y = dy[di++]; got = cell_free(x, y); }
		if (!got && n) got = room_spot(&layer.rooms[order[rng_range(0, n - 1)]], &x, &y);
		if (!got) continue;
		NetObj *o = add_obj(OBJ_MYSTERY, x, y);
		if (!o) break;
		int roll = rng_range(0, 99);
		o->param = roll < 70 ? 0 : roll < 92 ? 1 : 2;
	}
	/* Bystander navis with a word to share. */
	int npcs = 2 + rng_range(0, 1);
	for (int k = 0; k < npcs && n; ++k) {
		Room *r = &layer.rooms[order[rng_range(0, n - 1)]];
		if (!room_spot(r, &x, &y)) continue;
		NetObj *o = add_obj(OBJ_NPC, x, y);
		if (o) { o->param = rng_range(0, 5); o->npc_line = rng_range(0, 255); }
	}
#undef PLACE
}

/* Layer generation: rooms joined by walkways, then points of interest. */
#include <stdlib.h>
#include <string.h>

#include "game.h"
#include "net.h"
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

/* Rooms keep to the top-left GEN_SIZE cells: a cell is a 64x32 panel. */
#define GEN_SIZE 20

static bool room_fits(const Room *r) {
	if (r->x < 2 || r->y < 2 || r->x + r->w > GEN_SIZE - 2 || r->y + r->h > GEN_SIZE - 2) return false;
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *o = &layer.rooms[i];
		if (r->x < o->x + o->w + 3 && o->x < r->x + r->w + 3 && r->y < o->y + o->h + 3 && o->y < r->y + r->h + 3) return false;
	}
	return true;
}

static void carve(int x, int y) {
	if (x >= 1 && y >= 1 && x < MAP_W - 1 && y < MAP_H - 1) layer.cell[y][x] = C_PATH;
}

static void corridor(int ax, int ay, int bx, int by, int width) {
	/* L-shaped walkway; the bend order alternates for variety. */
	bool xfirst = rng_range(0, 1);
	int x = ax, y = ay;
	while (x != bx || y != by) {
		for (int k = 0; k < width; ++k) carve(xfirst ? x : x + k, xfirst ? y + k : y);
		if (xfirst) { if (x != bx) x += x < bx ? 1 : -1; else y += y < by ? 1 : -1; }
		else { if (y != by) y += y < by ? 1 : -1; else x += x < bx ? 1 : -1; }
	}
	carve(bx, by);
}

static void room_center(const Room *r, int *cx, int *cy) {
	*cx = r->x + r->w / 2;
	*cy = r->y + r->h / 2;
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
	for (int i = 0; i < layer.nobj; ++i)
		if ((int)layer.obj[i].x == x && (int)layer.obj[i].y == y) return false;
	return true;
}

/* A free cell inside a room, preferring its edges so paths stay clear. */
static bool room_spot(const Room *r, int *ox, int *oy) {
	for (int tries = 0; tries < 40; ++tries) {
		int x = r->x + rng_range(0, r->w - 1), y = r->y + rng_range(0, r->h - 1);
		int cx, cy;
		room_center(r, &cx, &cy);
		if (tries < 30 && x == cx && y == cy) continue;
		if (cell_free(x, y)) { *ox = x; *oy = y; return true; }
	}
	return false;
}

static int bfs_far(int from) {
	/* Room graph distance by flood fill over cells; returns farthest room. */
	static int16_t dist[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(dist, -1, sizeof dist);
	int cx, cy;
	room_center(&layer.rooms[from], &cx, &cy);
	int h = 0, t = 0;
	qx[t] = (int16_t)cx; qy[t++] = (int16_t)cy;
	dist[cy][cx] = 0;
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
		room_center(&layer.rooms[i], &cx, &cy);
		if (dist[cy][cx] > bd) { bd = dist[cy][cx]; best = i; }
	}
	return best;
}

void layer_generate(uint32_t seed, int depth, int biome, int kind) {
	memset(&layer, 0, sizeof layer);
	rng_seed(seed);
	layer.biome = biome;
	layer.kind = kind;
	layer.boss_layer = kind == LAYER_NORMAL && is_boss_depth(depth);
	if (kind == LAYER_SECRET) layer.boss_layer = !run.secret_cleared;

	int want = 6 + depth / 4;
	if (kind != LAYER_NORMAL) want = 8;
	if (want > MAX_ROOMS) want = MAX_ROOMS;
	for (int tries = 0; tries < 400 && layer.nrooms < want; ++tries) {
		Room r;
		r.w = rng_range(3, 5);
		r.h = rng_range(3, 5);
		r.x = rng_range(2, GEN_SIZE - r.w - 2);
		r.y = rng_range(2, GEN_SIZE - r.h - 2);
		if (!room_fits(&r)) continue;
		layer.rooms[layer.nrooms++] = r;
		for (int y = r.y; y < r.y + r.h; ++y)
			for (int x = r.x; x < r.x + r.w; ++x) layer.cell[y][x] = C_PATH;
	}
	/* Connect with a minimum spanning tree, plus a couple of loops. */
	bool in_tree[MAX_ROOMS] = { true };
	for (int n = 1; n < layer.nrooms; ++n) {
		int ba = -1, bb = -1, bd = 1 << 30;
		for (int a = 0; a < layer.nrooms; ++a) {
			if (!in_tree[a]) continue;
			for (int b = 0; b < layer.nrooms; ++b) {
				if (in_tree[b]) continue;
				int ax, ay, bx, by;
				room_center(&layer.rooms[a], &ax, &ay);
				room_center(&layer.rooms[b], &bx, &by);
				int d = abs(ax - bx) + abs(ay - by);
				if (d < bd) { bd = d; ba = a; bb = b; }
			}
		}
		if (bb < 0) break;
		in_tree[bb] = true;
		int ax, ay, bx, by;
		room_center(&layer.rooms[ba], &ax, &ay);
		room_center(&layer.rooms[bb], &bx, &by);
		corridor(ax, ay, bx, by, rng_range(0, 2) == 0 ? 1 : 2);
	}
	for (int k = 0; k < 2 && layer.nrooms > 3; ++k) {
		int a = rng_range(0, layer.nrooms - 1), b = rng_range(0, layer.nrooms - 1);
		if (a == b) continue;
		int ax, ay, bx, by;
		room_center(&layer.rooms[a], &ax, &ay);
		room_center(&layer.rooms[b], &bx, &by);
		if (abs(ax - bx) + abs(ay - by) < 18) corridor(ax, ay, bx, by, 1);
	}

	layer.exit_room = bfs_far(0);
	int cx, cy;
	room_center(&layer.rooms[0], &cx, &cy);
	add_obj(OBJ_WARP_IN, cx, cy);
	room_center(&layer.rooms[layer.exit_room], &cx, &cy);
	NetObj *exit = add_obj(kind == LAYER_NORMAL ? OBJ_EXIT : OBJ_RETURN, cx, cy);
	if (layer.boss_layer && exit) {
		/* The boss guards the exit, one cell in front of it. */
		int bx = cx, by = cy;
		for (int d = 0; d < 4; ++d) {
			static const int off[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
			if (layer.cell[cy + off[d][1]][cx + off[d][0]] == C_PATH) { bx = cx + off[d][0]; by = cy + off[d][1]; break; }
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
	bool challenge = rng_range(0, 99) < 20 + depth;
	bool undernet = kind == LAYER_NORMAL && depth >= 4 && !layer.boss_layer &&
		rng_range(0, 99) < (biome == BIOME_GRAVEYARD ? 40 : 12);
	bool secret = kind == LAYER_UNDERNET && !run.secret_cleared;

	int order[MAX_ROOMS], n = 0;
	for (int i = 0; i < layer.nrooms; ++i) if (i != 0 && i != layer.exit_room) order[n++] = i;
	for (int i = n - 1; i > 0; --i) { int j = rng_range(0, i); int t = order[i]; order[i] = order[j]; order[j] = t; }
	int next = 0;
	int x, y;
#define PLACE(t) (next < n && room_spot(&layer.rooms[order[next]], &x, &y) ? add_obj((t), x, y) : NULL)
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
	for (int k = 0; k < rich && next < n; ++k, ++next) {
		NetObj *o = PLACE(OBJ_MYSTERY);
		if (o) o->param = rng_range(0, 99) < 50 ? 1 : 2;
	}
	/* Mystery data scattered through the rest. */
	int md = 3 + rng_range(0, 2);
	for (int k = 0; k < md; ++k) {
		Room *r = &layer.rooms[order[n ? rng_range(0, n - 1) : 0]];
		if (!n || !room_spot(r, &x, &y)) continue;
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

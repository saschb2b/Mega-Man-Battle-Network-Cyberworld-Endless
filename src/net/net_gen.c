/* Layer generation: a layout after the area's own maps (net_layouts.c),
 * then points of interest (docs/LEVEL_DESIGN.md). Here the driver
 * (layer_generate), the way through, the floor's searches and where the
 * services, the data and the people stand; the detours are net_detours.c's,
 * the landmarks and props net_landmarks.c's, the set pieces' places
 * net_set_pieces.c's. */

#include "net_gen.h"

#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "game.h"
#include "net_arena.h"
#include "net_detours.h"
#include "net_landmarks.h"
#include "net_layouts.h"
#include "net_plan.h"
#include "net_set_pieces.h"
#include "net_signature.h"
#include "pacing.h"
#include "run.h"

/* Layer generation: a layout after the area's own maps (net_layouts.c),
 * then points of interest (docs/LEVEL_DESIGN.md). */

Layer layer;

int biome_for_depth(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	if (p >= 18 || run_short_nest(depth)) return BIOME_NEST;
	return run.biome_order[p / 3];
}

int layer_in_act(int depth) { return ((depth - 1) % CYCLE_LAYERS) % 3; }

bool is_boss_depth(int depth) {
	int p = (depth - 1) % CYCLE_LAYERS;
	return p % 3 == 2 || p == 18 || run_short_nest(depth);
}

NetObj *ng_add_obj(int type, int x, int y) {
	if (layer.nobj >= MAX_OBJS) return NULL;
	NetObj *o = &layer.obj[layer.nobj++];
	memset(o, 0, sizeof *o);
	o->type = type;
	o->x = (float)x + 0.5f;
	o->y = (float)y + 0.5f;
	o->solid = type != OBJ_WARP_IN && type != OBJ_EXIT && type != OBJ_UNDERNET && type != OBJ_SECRET_GATE && type != OBJ_RETURN &&
		type != OBJ_NAVI_GATE && type != OBJ_VAULT && type != OBJ_OFFICIAL;
	o->prop = -1;
	return o;
}

/* A set piece's panels, kept clear of what is placed after it is planned
 * (a Rush gap's stand and the walk to it). */
uint8_t ng_reserved[MAP_H][MAP_W];
/* ... and the panels round where it is spoken to (a Rush stand, an
 * obstacle's or a cube's mouth), kept clear of navis: the engine's A turns
 * to a navi within 52 units before a map's check (a P-Code's teller two
 * panels from his cube took its A; issue #45). */
#define PIECE_QUIET 3
uint8_t ng_hushed[MAP_H][MAP_W];

bool ng_cell_free(int x, int y) {
	if (layer.cell[y][x] != C_PATH || ng_reserved[y][x]) return false;
	/* (nor before or beside a counter: its navi is spoken to from there) */
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < 4; ++k) {
		int nx = x + d4[k][0], ny = y + d4[k][1];
		if (nx >= 0 && ny >= 0 && nx < MAP_W && ny < MAP_H && layer.cell[ny][nx] == C_PROPPED) return false;
	}
	for (int i = 0; i < layer.nstairs; ++i)
		if (x >= layer.stair[i].x && x < layer.stair[i].x + 2 && y >= layer.stair[i].y && y < layer.stair[i].y + 2) return false;
	/* (nor beside another solid object: A answers whichever is in reach,
	 * and a crowd answered the wrong one) */
	for (int i = 0; i < layer.nobj; ++i) {
		int dx = abs((int)layer.obj[i].x - x), dy = abs((int)layer.obj[i].y - y);
		if (dx == 0 && dy == 0) return false;
		if (layer.obj[i].solid && dx <= 1 && dy <= 1) return false;
	}
	return true;
}

/* The open floor a walk from the arrival reaches, marked 2; how much. */
static int reach_marked(uint8_t blocked[MAP_H][MAP_W]) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	int sx = (int)layer.obj[0].x, sy = (int)layer.obj[0].y, h = 0, t = 0;
	if (blocked[sy][sx]) return 0;
	blocked[sy][sx] = 2;
	qx[t] = (int16_t)sx; qy[t++] = (int16_t)sy;
	while (h < t) {
		int cx = qx[h], cy = qy[h++];
		static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
		for (int k = 0; k < 4; ++k) {
			int nx = cx + d[k][0], ny = cy + d[k][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || blocked[ny][nx] || layer.cell[ny][nx] != C_PATH) continue;
			blocked[ny][nx] = 2;
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	return t;
}

/* Whether solid objects at the n cells would cut the floor: MegaMan cannot
 * pass a navi or a Mystery Data (their radius keeps him about half a panel
 * off), so with the cells blocked, and the cells of the solid objects
 * already placed, every floor cell reached before must still be reached
 * from the arrival (an island past a gap or a teleport's void is reached
 * neither way: counted as cut, it turned every navi placed after it away,
 * a P-Code's teller and an invisible path's). */
bool ng_cuts(int n, const int *xs, const int *ys) {
	static uint8_t blocked[MAP_H][MAP_W];
	if (!layer.nobj) return false;
	memset(blocked, 0, sizeof blocked);
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].solid) blocked[(int)layer.obj[i].y][(int)layer.obj[i].x] = 1;
	int before = reach_marked(blocked), taken = 0;
	for (int i = 0; i < n; ++i) taken += blocked[ys[i]][xs[i]] == 2;
	for (int cy = 0; cy < MAP_H; ++cy)
		for (int cx = 0; cx < MAP_W; ++cx)
			if (blocked[cy][cx] == 2) blocked[cy][cx] = 0;
	for (int i = 0; i < n; ++i) blocked[ys[i]][xs[i]] = 1;
	return reach_marked(blocked) < before - taken;
}

bool ng_cuts_way(int x, int y) { return ng_cuts(1, &x, &y); }

/* Whether (x, y) in room `r` is at one of its exits: a floor cell beside it
 * outside the room (a walkway's mouth, where a Server or a navi stood in
 * the way on, the gap round it hard to find). */
static bool at_exit(const Room *r, int x, int y) {
	/* (corner to corner too: a navi on a platform's corner beside a
	 * walkway's last panel stood in the way in) */
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) {
			int nx = x + dx, ny = y + dy;
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H) continue;
			bool inside = nx >= r->x && nx < r->x + r->w && ny >= r->y && ny < r->y + r->h;
			if (!inside && layer.cell[ny][nx] == C_PATH) return true;
		}
	return false;
}

/* Whether (x, y) is beside a panel-wide stretch of floor, whatever room box
 * holds it: a walkway's mouth (the Net Dealer stood where a walkway met his
 * platform, a corner of the room's box, and the way on went through him) */
bool ng_beside_narrow(int x, int y) {
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < 4; ++k) {
		int nx = x + d[k][0], ny = y + d[k][1];
		if (nx < 1 || ny < 1 || nx >= MAP_W - 1 || ny >= MAP_H - 1 || layer.cell[ny][nx] != C_PATH) continue;
		int ax = d[k][1], ay = d[k][0];   /* across the step */
		if (layer.cell[ny + ay][nx + ax] != C_PATH && layer.cell[ny - ay][nx - ax] != C_PATH) return true;
	}
	return false;
}

static bool beside_other_level(int x, int y) {
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < 4; ++k) {
		int nx = x + d[k][0], ny = y + d[k][1];
		if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || layer.cell[ny][nx] != C_PATH) continue;
		if (layer.level[ny][nx] != layer.level[y][x]) return true;
	}
	return false;
}

/* Whether a floor lies a panel's gap in front of (x, y), down-left or
 * down-right on the screen, where the camera looks from: the cell's own
 * wall hides the gap, so from that floor what stands here looks a step
 * away on a raised block (a playtester pressed A at a Mystery Data that
 * was a walk round, and left it) */
bool ng_behind_gap(int x, int y) {
	return (x + 2 < MAP_W && layer.cell[y][x + 1] == C_VOID && layer.cell[y][x + 2] == C_PATH) ||
		(y + 2 < MAP_H && layer.cell[y + 1][x] == C_VOID && layer.cell[y + 2][x] == C_PATH);
}

/* A free cell inside a room, off its middle, its exits and any walkway's
 * mouth so paths stay clear, where a solid object cuts no way; with `open`, first one with
 * floor on its four sides (a bystander in a panel-wide gap beside a
 * service pinned MegaMan in the nook, where the way still went round). */
/* Whether a panel-wide stretch of walkway touches (x, y) corner to corner. */
bool ng_by_walkway(int x, int y) {
	for (int dy = -1; dy <= 1; dy += 2)
		for (int dx = -1; dx <= 1; dx += 2) {
			int nx = x + dx, ny = y + dy;
			if (nx < 1 || ny < 1 || nx >= MAP_W - 1 || ny >= MAP_H - 1 || layer.cell[ny][nx] != C_PATH) continue;
			if ((layer.cell[ny][nx - 1] == C_VOID && layer.cell[ny][nx + 1] == C_VOID) ||
				(layer.cell[ny - 1][nx] == C_VOID && layer.cell[ny + 1][nx] == C_VOID)) return true;
		}
	return false;
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

/* Whether something solid at (x, y) would leave a panel's gap on the way
 * with a solid object two panels off: with both radii that gap is shut (a
 * playtester wedged between a Server and a Recovery Mr. Prog, the way on
 * between them). The gap: the panels beside both. */
static bool gap_on_way(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i) {
		if (!layer.obj[i].solid) continue;
		int ox = (int)layer.obj[i].x, oy = (int)layer.obj[i].y;
		if (abs(ox - x) > 2 || abs(oy - y) > 2 || (abs(ox - x) < 2 && abs(oy - y) < 2)) continue;
		for (int gy = y - 1; gy <= y + 1; ++gy)
			for (int gx = x - 1; gx <= x + 1; ++gx)
				if (abs(gx - ox) <= 1 && abs(gy - oy) <= 1 && gx >= 0 && gy >= 0 && gx < MAP_W && gy < MAP_H &&
				    ng_way_band[gy][gx] == 2) return true;
	}
	return false;
}

/* Whether one MegaMan talks to stands within two panels of (x, y), three
 * of one behind a counter, who is talked to from the counter's front: an A
 * meant for a heal Prog opened the Net Dealer beside him, twice. */
bool ng_near_talker(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i) {
		int t = layer.obj[i].type, reach = layer.obj[i].prop >= 0 ? 3 : 2;
		bool talks = t == OBJ_SHOP || t == OBJ_HEAL || t == OBJ_TRADER || t == OBJ_BUGTRADER || t == OBJ_NPC ||
			t == OBJ_CHALLENGE || t == OBJ_PROGRAMS || t == OBJ_GIFT;
		if (talks && abs((int)layer.obj[i].x - x) <= reach && abs((int)layer.obj[i].y - y) <= reach) return true;
	}
	return false;
}

/* Whether a navi of any kind stands within PIECE_QUIET panels of (x, y). */
bool ng_navi_near(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i) {
		int t = layer.obj[i].type;
		bool navi = t != OBJ_WARP_IN && t != OBJ_EXIT && t != OBJ_MYSTERY && t != OBJ_UNDERNET && t != OBJ_RETURN;
		if (navi && abs((int)layer.obj[i].x - x) <= PIECE_QUIET && abs((int)layer.obj[i].y - y) <= PIECE_QUIET) return true;
	}
	return false;
}

void ng_hush(int x, int y) {
	for (int j = y - PIECE_QUIET; j <= y + PIECE_QUIET; ++j)
		for (int i = x - PIECE_QUIET; i <= x + PIECE_QUIET; ++i)
			if (i >= 0 && j >= 0 && i < MAP_W && j < MAP_H) ng_hushed[j][i] = 1;
}

bool ng_room_spot_in(const Room *r, int *ox, int *oy, bool open) {
	for (int tries = 0; tries < 40; ++tries) {
		int x = r->x + rng_range(0, r->w - 1), y = r->y + rng_range(0, r->h - 1);
		if ((tries < 30 || open) && x == r->ax && y == r->ay) continue;
		if ((tries < 34 || open) && in_way_line(x, y)) continue;
		/* (off the way and its band most of the tries: navis by the way
		 * stood in a playtester's, the Chip Trader on layer 2's and a
		 * bystander square on the way to an arena, session 61) */
		if (tries < 24 && ng_way_band[y][x]) continue;
		if (tries < 32 && gap_on_way(x, y)) continue;
		if (tries < 30 && ng_near_talker(x, y)) continue;
		/* (a bystander, who may stay away, never at one, nor corner to
		 * corner with a walkway: one on a platform's corner beside its way
		 * in stood in the way) */
		if ((tries < 36 || open) && (at_exit(r, x, y) || ng_beside_narrow(x, y) || ng_by_walkway(x, y))) continue;
		if (open && tries < 24 && !(layer.cell[y][x + 1] == C_PATH && layer.cell[y][x - 1] == C_PATH &&
			layer.cell[y + 1][x] == C_PATH && layer.cell[y - 1][x] == C_PATH)) continue;
		/* (not beside a floor of another height: a NaviCust vendor below a
		 * raised platform's edge looked a step away from it and was a
		 * stair's walk round) */
		if (tries < 30 && beside_other_level(x, y)) continue;
		/* (a bystander, who may stay away, never there) */
		if ((tries < 38 || open) && ng_behind_gap(x, y)) continue;
		if (ng_cell_free(x, y) && !ng_cuts_way(x, y)) { *ox = x; *oy = y; return true; }
	}
	return false;
}

static bool room_spot(const Room *r, int *ox, int *oy) { return ng_room_spot_in(r, ox, oy, false); }

/* Each floor cell's walking distance from room `from`'s anchor, into
 * `dist` (-1 where it is not reached) */
static void flood_from(int from, int16_t dist[MAP_H][MAP_W]) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(dist, -1, sizeof(int16_t[MAP_H][MAP_W]));
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
}

/* The room farthest from the arrival by walking, of those the walk reaches
 * only past the layer's signature where there are any (docs/LEVEL_DESIGN.md,
 * Identity: the way runs through it). */
static int exit_far(void) {
	static int16_t dist[MAP_H][MAP_W];
	ng_walk_past_signature(dist);
	int best = 0, bd = -1;
	for (int i = 0; i < layer.nrooms; ++i) {
		int d = dist[layer.rooms[i].ay][layer.rooms[i].ax];
		if (d > bd) { bd = d; best = i; }
	}
	return best;
}

/* Up to `max` rooms nearest room `from` by walking, `reach` panels at
 * most, nearest first, but `from` and the arena. */
/* The free floor nearest room `from`'s anchor, within `reach` steps of
 * walkway and floor, the arena kept out: off the way's line, cutting no
 * way, and away from those who talk where it can be. */
static bool floor_near(int from, int reach, int *ox, int *oy) {
	const Room *a = layer.arena >= 0 ? &layer.rooms[layer.arena] : NULL;
	#define IN_ARENA(cx, cy) (a && (cx) >= a->x && (cy) >= a->y && (cx) < a->x + a->w && (cy) < a->y + a->h)
	static int16_t dist[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	for (int loose = 0; loose < 2; ++loose) {
		memset(dist, -1, sizeof dist);
		int h = 0, t = 0;
		qx[t] = (int16_t)layer.rooms[from].ax; qy[t++] = (int16_t)layer.rooms[from].ay;
		dist[qy[0]][qx[0]] = 0;
		while (h < t) {
			int x = qx[h], y = qy[h++];
			if (dist[y][x] > 0 && ng_cell_free(x, y) && !ng_cuts_way(x, y) && !in_way_line(x, y) && (loose || !ng_near_talker(x, y))) {
				*ox = x; *oy = y;
				return true;
			}
			if (dist[y][x] >= reach) continue;
			static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
			for (int k = 0; k < 4; ++k) {
				int nx = x + d[k][0], ny = y + d[k][1];
				if (layer.cell[ny][nx] != C_PATH || dist[ny][nx] >= 0 || IN_ARENA(nx, ny)) continue;
				dist[ny][nx] = (int16_t)(dist[y][x] + 1);
				qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
			}
		}
	}
	#undef IN_ARENA
	return false;
}

static int nearest_rooms(int from, int *out, int max, int reach) {
	static int16_t dist[MAP_H][MAP_W];
	int n = 0;
	flood_from(from, dist);
	for (int k = 0; k < max; ++k) {
		int best = -1, bd = reach + 1;
		for (int i = 0; i < layer.nrooms; ++i) {
			int d = dist[layer.rooms[i].ay][layer.rooms[i].ax];
			bool taken = i == from || i == layer.arena;
			for (int j = 0; j < n; ++j) taken |= out[j] == i;
			if (!taken && d >= 0 && d < bd) { bd = d; best = i; }
		}
		if (best < 0) break;
		out[n++] = best;
	}
	return n;
}

#define MIN_FLOOR 120   /* panels a layer has at least */
#define ARENA_SIZE 5    /* the guardian's arena, panels a side */
#define HEAL_REACH 8    /* the heal before an arena, its walk from the room at its door at most */

static int floor_cells(void) {
	int n = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) n += layer.cell[y][x] == C_PATH;
	return n;
}

/* Whether the layer's tile map fits the game's buffer (BN6_TILEMAP_MAX), as
 * netmap centres and sizes it (panel edges up to 28 units off, `rise` for a
 * raised floor). */
bool ng_fits(int rise) {
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
	return tw <= 255 && th <= 255 && tw * th * 4 <= BN6_TILEMAP_MAX;
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
	if (layer.sig_room == best) layer.sig_room = 0;
	else if (layer.sig_room == 0) layer.sig_room = best;
}

bool ng_floor_cell(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }
bool ng_void_cell(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_VOID; }

/* The next room of `order`; once each has had one, one of the big ones
 * (fields and platforms of 16 cells or more); -1 for none. */
static int pick_room(const int *order, int n, int next) {
	if (next < n) return order[next];
	int big[MAX_ROOMS], nb = 0;
	for (int i = 0; i < n; ++i)
		if (layer.rooms[order[i]].w * layer.rooms[order[i]].h >= 16) big[nb++] = order[i];
	return nb ? big[rng_range(0, nb - 1)] : -1;
}

/* Object `type` in the next room of `order`: apart from anyone to talk
 * to (near_talker), in the rooms after it where that one has no such
 * place (the Net Dealer's counter may stand in another's room). */
static NetObj *place(int type, const int *order, int n, int next, int *x, int *y) {
	int r = pick_room(order, n, next);
	if (r < 0) return NULL;
	for (int k = 0; k < n; ++k) {
		int room = k == 0 ? r : order[(next + k) % n];
		if (room_spot(&layer.rooms[room], x, y) && !ng_near_talker(*x, *y)) return ng_add_obj(type, *x, *y);
	}
	return room_spot(&layer.rooms[r], x, y) ? ng_add_obj(type, *x, *y) : NULL;
}

/* The Net Dealer in room r: behind a counter where it has a place for one,
 * else in the first of `order`'s rooms that has (the originals' dealers
 * all stand behind one), else standing in r. */
static NetObj *dealer(int r, const int *order, int n, const LayerKit *kit, int *x, int *y) {
	if (r < 0) return NULL;
	NetObj *o = ng_counter(r, OBJ_SHOP, kit);
	for (int i = 0; i < n && !o; ++i)
		if (order[i] != r) o = ng_counter(order[i], OBJ_SHOP, kit);
	return o ? o : room_spot(&layer.rooms[r], x, y) ? ng_add_obj(OBJ_SHOP, *x, *y) : NULL;
}

/* The last stop before the arena: the Net Dealer and a heal, as the rooms
 * before Hades' guardians hold Charon and a fountain. */
static void last_stop(int kind, const LayerKit *kit) {
	int x, y;
	/* (behind a counter in one of the three rooms nearest it, 18 panels'
	 * walk at most, where it has no place for one: on the pads before
	 * arenas three dealers in four stood bare) */
	int near[3], nn = nearest_rooms(layer.ante, near, 3, 18);
	if (kind == LAYER_NORMAL) dealer(layer.ante, near, nn, kit, &x, &y);
	/* (the heal apart from the dealer: in a room nearest it where the last
	 * one has no place away from his counter, else on the nearest free
	 * floor; never farther than a short walk: a playtester found it a long
	 * way back from SpoutMan's arena, where the rooms by it were full, and
	 * one layer in a hundred had none) */
	int close[3], nc = nearest_rooms(layer.ante, close, 3, HEAL_REACH);
	static int16_t walk[MAP_H][MAP_W];
	flood_from(layer.ante, walk);
	for (int i = -1; i < nc; ++i) {
		const Room *r = &layer.rooms[i < 0 ? layer.ante : close[i]];
		/* (a short walk from the room's anchor too: the layer's signature
		 * before an arena is a room eleven a side, a ring road's pad in its
		 * middle a walk round) */
		for (int tries = 0; tries < 4; ++tries)
			if (room_spot(r, &x, &y) && !ng_near_talker(x, y) && walk[y][x] >= 0 && walk[y][x] <= HEAL_REACH) { ng_add_obj(OBJ_HEAL, x, y); return; }
	}
	if (floor_near(layer.ante, HEAL_REACH, &x, &y) || room_spot(&layer.rooms[layer.ante], &x, &y)) ng_add_obj(OBJ_HEAL, x, y);
}

/* The rooms' last shaping once the arena stands: the exit, the way to it
 * widened past the area's cap (net_way.c, the guardian's own bridge left
 * as his gate), the raised rooms. */
static void finish_rooms(uint32_t seed, unsigned stair_dirs, int rise) {
	layer.exit_room = layer.arena >= 0 ? layer.arena : exit_far();
	const Room *goal = &layer.rooms[layer.arena >= 0 ? layer.ante : layer.exit_room];
	layer_widen_way(layer.biome, goal->ax, goal->ay);
	layer_raise_rooms(seed, stair_dirs, rise);
}

/* The layout: the planned one, then any of the area's, last the plainest
 * at its smallest; its signature at its heart, which the first tries keep
 * on until it has room for it; on a guardian's layer an arena of its own at
 * the far end. */
static void build_layout(int planned, int sig, int biome, int size, int rise, ArenaInfo *arena) {
	for (int attempt = 0; attempt < 12; ++attempt) {
		memset(layer.cell, 0, sizeof layer.cell);
		layer.nrooms = 0;
		arena->room = -1;
		/* the planned layout, then any of the area's, last the plainest at its smallest */
		bool last = attempt == 11;
		layer.layout = last ? LAYOUT_ROUTE : attempt < 6 ? planned : layout_pick(biome);
		layout_build(layer.layout, biome, last ? 0 : size, sig);
		if (layer.nrooms < 3 || (layer.sig && layer.sig_room < 0 && attempt < 8)) continue;
		choose_arrival();
		connect_all();
		if (floor_cells() < MIN_FLOOR || !ng_fits(rise)) continue;
		/* a guardian waits in an arena of its own at the far end */
		if (!layer.boss_layer || last) break;
		if (arena_attach(ARENA_SIZE, arena) >= 0 && ng_fits(rise)) break;
	}

	if (layer.arena < 0 && layer.boss_layer && arena->room >= 0 && arena->room < layer.nrooms) {
		layer.arena = arena->room;
		layer.ante = arena->ante;
		layer.arena_dir = arena->dir;
	}
	if (layer.sig_room < 0) layer.sig = SIG_NONE;
}

/* The layer's ends: where MegaMan arrives, the exit and on a guardian's
 * layer the guardian; then the way between them. */
static void place_ends(int kind, int biome, const ArenaInfo *arena) {
	int cx = layer.rooms[0].ax, cy = layer.rooms[0].ay;
	ng_add_obj(OBJ_WARP_IN, cx, cy);
	cx = layer.rooms[layer.exit_room].ax;
	cy = layer.rooms[layer.exit_room].ay;
	/* in an arena the exit waits behind the guardian, who holds the middle */
	int bx = cx, by = cy;
	if (layer.arena >= 0) { cx = arena->exit_x; cy = arena->exit_y; }
	NetObj *exit = ng_add_obj(kind == LAYER_NORMAL ? OBJ_EXIT : OBJ_RETURN, cx, cy);
	if (layer.boss_layer && exit) {
		if (layer.arena < 0) {
			/* no room for an arena: the guardian stands before the exit */
			for (int d = 0; d < 4; ++d) {
				static const int off[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
				if (layer.cell[cy + off[d][1]][cx + off[d][0]] == C_PATH) { bx = cx + off[d][0]; by = cy + off[d][1]; break; }
			}
		}
		NetObj *b = ng_add_obj(OBJ_BOSS, bx, by);
		if (b) {
			/* (an act's: BN5's own Navi where his game dresses the area and
			 * guards it, run_guardian; a side layer's the run's pick) */
			layer.boss_navi = kind == LAYER_NORMAL ? run_guardian(biome) : run.boss_order[biome];
			b->param = layer.boss_navi;
		}
		ng_mark_way(layer.rooms[0].ax, layer.rooms[0].ay, bx, by);
	} else {
		ng_mark_way(layer.rooms[0].ax, layer.rooms[0].ay, cx, cy);
	}
}

/* What a layer's rolls put on it: its services, gates and the rival. */
typedef struct {
	bool shop, heal, trader, programs, bugtrader, challenge, duel, undernet, secret, navi_gate, vault, official;
	int gate_navi, official_level;
} Services;

/* Points of interest in the other rooms. */
static void roll_services(Services *s, int depth, int biome, int kind) {
	/* each act's middle layer has the Net Dealer and a heal, and so does its
	 * first after the run's first (the guardian's zenny to spend on what the
	 * new act calls for); docs/PROGRESSION.md */
	int biome_layer = layer_in_act(depth);
	/* (a trip back as its act's middle layer: the dealer, the heal and the
	 * vendor's chance, docs/HOME.md) */
	bool own = kind == LAYER_NORMAL || kind == LAYER_BACK;
	s->shop = own && (biome_layer == 1 || (biome_layer == 0 && depth > 1) || rng_range(0, 99) < (biome_layer == 0 ? 50 : 25));
	/* (threat 2, docs/META.md: only the heals an act is sure of; the heals
	 * helper: one on every layer) */
	s->heal = (rng_range(0, 99) < (layer.boss_layer ? 70 : 30) && run.threat < 2) || (own && pacing_heal_certain(depth)) ||
		(run.helpers & HELP_HEALS);
	s->trader = rng_range(0, 99) < (run.threat >= 7 ? 12 : 25);   /* (threat 7: half as often) */
	s->programs = own && biome_layer == 1 && rng_range(0, 99) < 60;
	s->bugtrader = kind == LAYER_UNDERNET || (biome == BIOME_GRAVEYARD && rng_range(0, 99) < 40);
	s->trader &= !s->bugtrader;   /* the trade screen serves one trader per map */
	s->challenge = depth > 1 && rng_range(0, 99) < 20 + depth;
	/* the rival's duel (docs/RIVAL.md): on each act's second layer, where
	 * no strong virus signal stands (one battle to seek out a layer) */
	s->duel = kind == LAYER_NORMAL && biome_layer == 1 && biome != BIOME_NEST;
	s->challenge &= !s->duel;
	s->undernet = kind == LAYER_NORMAL && depth >= 4 && !layer.boss_layer &&
		rng_range(0, 99) < (biome == BIOME_GRAVEYARD ? 40 : 12);
	s->secret = kind == LAYER_UNDERNET && !run.secret_cleared;
}

/* The gates' rolls, after the services'. */
static void roll_gates(Services *s, uint32_t seed, int depth, int kind) {
	/* a gate sealed with a Navi's code (docs/META.md, gates): from act 3,
	 * before the guardian's layer, where no dark warp stands; its Navi one
	 * of the guardians (but ProtoMan, the Secret Area's) */
	s->navi_gate = kind == LAYER_NORMAL && pacing_act(depth) >= 2 && !layer.boss_layer && !s->undernet && rng_range(0, 99) < 30;
	static const uint8_t gate_navis[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 18 };
	s->gate_navi = s->navi_gate ? gate_navis[rng_range(0, (int)sizeof gate_navis - 1)] : 0;
	/* a collector's vault (docs/META.md, gates): from act 2, before the
	 * guardian's layer, where no other gate or dark warp stands */
	s->vault = kind == LAYER_NORMAL && pacing_act(depth) >= 1 && !layer.boss_layer && !s->undernet && !s->navi_gate &&
		rng_range(0, 99) < 20;
	/* an official gate (docs/RIVAL.md): from act 2, where no other gate or
	 * dark warp stands; sealed until Chaud's clearance reaches its level
	 * (from the seed, not the rolls: two rolls more reshuffled the rooms
	 * of every layer after them). And on every act's duel layer, the prize
	 * where the duel is offered, its level the act's (1 to the third, then
	 * 2), the only gate there (the rolls kept): a playtester's promise of
	 * official gates pointed at none he had met */
	if (s->duel) s->navi_gate = s->vault = false;
	uint32_t oh = (seed ^ 0x0FF1C1A1u) * 2654435761u;
	s->official = kind == LAYER_NORMAL && !layer.boss_layer &&
		(s->duel || (pacing_act(depth) >= 1 && !s->undernet && !s->navi_gate && !s->vault && (oh >> 16) % 100 < 25));
	s->official_level = !s->official ? 0 : s->duel ? (pacing_act(depth) >= 2 ? 2 : 1) : 1 + (int)((oh >> 8) & 1);
}

/* The other rooms in a shuffled order: services on the bigger platforms,
 * the pads left for the better data. */
static int room_order(int *order) {
	int n = 0;
	for (int i = 0; i < layer.nrooms; ++i) if (i != 0 && i != layer.exit_room) order[n++] = i;
	for (int i = n - 1; i > 0; --i) { int j = rng_range(0, i); int t = order[i]; order[i] = order[j]; order[j] = t; }
	/* services on the bigger platforms, the pads left for the better data */
	for (int i = 0, k = 0; i < n; ++i)
		if (layer.rooms[order[i]].kind != ROOM_PAD) { int t = order[k]; order[k++] = order[i]; order[i] = t; }
	return n;
}

#define PLACE(t) place((t), order, n, *next, &x, &y)

/* The rolled services, each in the next room of `order`. */
static void place_services(Services *s, int depth, int kind, const LayerKit *kit, const int *order, int n, int *next) {
	int x, y;
	if (layer.arena >= 0) {
		last_stop(kind, kit);
		s->shop = s->heal = false;
	}
	/* a Mr. Prog with a gift by the run's start */
	if (depth == 1 && kind == LAYER_NORMAL && room_spot(&layer.rooms[0], &x, &y)) ng_add_obj(OBJ_GIFT, x, y);
	if (s->shop) { dealer(pick_room(order, n, *next), order, n, kit, &x, &y); ++*next; }
	if (s->heal) { PLACE(OBJ_HEAL); ++*next; }
	if (s->trader) { PLACE(OBJ_TRADER); ++*next; }
	if (s->programs) { PLACE(OBJ_PROGRAMS); ++*next; }
	if (s->bugtrader) { PLACE(OBJ_BUGTRADER); ++*next; }
	if (s->challenge) { PLACE(OBJ_CHALLENGE); ++*next; }
	if (s->undernet) { PLACE(OBJ_UNDERNET); ++*next; }
	if (s->secret) { PLACE(OBJ_SECRET_GATE); ++*next; }
	if (s->navi_gate) { NetObj *g = PLACE(OBJ_NAVI_GATE); if (g) g->param = s->gate_navi; ++*next; }
	if (s->vault) { PLACE(OBJ_VAULT); ++*next; }
	/* (a duel's gate stands by ProtoMan, placed after him) */
	if (s->official && !s->duel) {
		NetObj *g = PLACE(OBJ_OFFICIAL);
		if (g) g->param = s->official_level;
		++*next;
	}
}

/* ProtoMan on a pad apart where one has room, a ring off the way on; else
 * the next room; after the area's props, whose landmark needs a bare room:
 * before them he took it on a layer in a hundred. */
static void place_duel(const int *order, int n, int *next) {
	int x, y;
	NetObj *placed = NULL;
	for (int i = n - 1; i >= *next && !placed; --i)
		if (layer.rooms[order[i]].kind == ROOM_PAD && room_spot(&layer.rooms[order[i]], &x, &y) && !ng_near_talker(x, y))
			placed = ng_add_obj(OBJ_DUEL, x, y);
	if (!placed) { placed = PLACE(OBJ_DUEL); ++*next; }
	/* (a small layer, its rooms all taken: in any room but the
	 * exit's, apart from the others where it can be; an act 3 layer
	 * of the Judge Tree Comp had no ProtoMan) */
	for (int i = 0; i < n && !placed; ++i)
		if (room_spot(&layer.rooms[order[i]], &x, &y) && !ng_near_talker(x, y)) placed = ng_add_obj(OBJ_DUEL, x, y);
	for (int i = 0; i < layer.nrooms && !placed; ++i)
		if (i != layer.exit_room && ng_room_spot_in(&layer.rooms[i], &x, &y, true)) placed = ng_add_obj(OBJ_DUEL, x, y);
}

/* The room whose box holds cell (x, y), or -1. */
static int room_holding(int x, int y) {
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *r = &layer.rooms[i];
		if (x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h) return i;
	}
	return -1;
}

/* The official gate the duel opens, by ProtoMan: in his room, two panels
 * from him or more, else in a room a short walk from his (a playtester
 * found the gate alone across the layer, and his session ran out looking
 * for the rival), else in the near side of any room; eight panels from him
 * at most (a layer's signature is a room eleven a side: one at its far
 * corner was a walk away, and its middle more than a short walk). */
static NetObj *gate_by_rival(const NetObj *rival) {
	int x, y, home = room_holding(rival->x, rival->y);
	int near[4], nn = home >= 0 ? nearest_rooms(home, near, 4, 12) : 0;
	for (int i = -1; i < nn + layer.nrooms && home >= 0; ++i) {
		int room = i < 0 ? home : i < nn ? near[i] : i - nn;
		if (room == layer.exit_room) continue;
		for (int tries = 0; tries < 40; ++tries)
			if (room_spot(&layer.rooms[room], &x, &y) && !ng_near_talker(x, y) && (abs(x - (int)rival->x) >= 2 || abs(y - (int)rival->y) >= 2) &&
			    abs(x - (int)rival->x) + abs(y - (int)rival->y) <= 8) {
				NetObj *g = ng_add_obj(OBJ_OFFICIAL, x, y);
				if (g) return g;
			}
	}
	return NULL;
}

/* That gate, else anywhere with space (a small endless layer with every
 * room taken had its duel and no gate). */
static void place_duel_gate(int level, const int *order, int n, int *next) {
	int x, y;
	const NetObj *rival = NULL;
	for (int i = 0; i < layer.nobj; ++i) if (layer.obj[i].type == OBJ_DUEL) rival = &layer.obj[i];
	NetObj *g = rival ? gate_by_rival(rival) : NULL;
	if (!g) g = PLACE(OBJ_OFFICIAL);
	for (int i = 0; !g && i < n; ++i)
		if (room_spot(&layer.rooms[order[i]], &x, &y) && !ng_near_talker(x, y)) g = ng_add_obj(OBJ_OFFICIAL, x, y);
	for (int i = 0; !g && i < layer.nrooms; ++i)
		if (i != layer.exit_room && ng_room_spot_in(&layer.rooms[i], &x, &y, true)) g = ng_add_obj(OBJ_OFFICIAL, x, y);
	if (g) g->param = level;
	++*next;
}

int layer_npcs(void) {
	int n = 0;
	for (int i = 0; i < layer.nobj; ++i) n += layer.obj[i].type == OBJ_WARP_IN ? 0 : layer.obj[i].type == OBJ_BOSS ? 2 : 1;
	return n;
}

/* What the NPCs the game runs leave for `want` more, the services and
 * gates placed first: the Mystery Data take what is left, the bystanders
 * the rest (on one layer in sixteen the last bystanders, and on one in
 * two hundred an official gate or ProtoMan, were past the sixteenth and
 * never showed). */
static int npcs_held;   /* kept for a set piece placed last (an island's data) */

int ng_npcs_for(int want) {
	int left = LAYER_NPC_MAX - layer_npcs() - npcs_held;
	return want < left ? want : left > 0 ? left : 0;
}

/* The layer's purple data, first of its data: at the landmark's foot, else
 * where the longest detour ends. */
static void place_purple(uint8_t *taken) {
	int x, y;
	DetourEnd end;
	bool at = ng_landmark_foot(&x, &y);
	if (!at && ng_detour_end(DETOUR_BLUE, taken, &end)) { x = end.x; y = end.y; at = true; }
	NetObj *o = at ? ng_add_obj(OBJ_MYSTERY, x, y) : NULL;
	if (o) o->param = MD_PURPLE;
}

/* Blue data where the detours end, more of them deeper and in the Undernet
 * (a dark warp's, or the short net's dark way's act), the farthest a tier
 * up where it is a long walk; in a room where the layer has too few
 * detours. Then green data loose through the rest, most at dead ends. The
 * colour says what a walk there is worth, as BN6's do: where the old roll
 * gave any data any tier, most of the good ones lay by the way, all green
 * (docs/LEVEL_DESIGN.md, Set pieces). */
static void place_data(int depth, int kind, int biome, int size, const int *order, int n, int *next) {
	int x, y;
	static uint8_t taken[MAP_W * MAP_H];
	memset(taken, 0, sizeof taken);
	ng_measure_detours();
	if (layer_purple(depth, biome, kind) && ng_npcs_for(1)) place_purple(taken);
	int rich = ng_npcs_for(2 + (depth > 6) + (kind == LAYER_UNDERNET || biome == BIOME_UNDERNET));
	for (int k = 0; k < rich; ++k) {
		DetourEnd end;
		bool far = ng_detour_end(DETOUR_BLUE, taken, &end);
		NetObj *o = far ? ng_add_obj(OBJ_MYSTERY, end.x, end.y) : PLACE(OBJ_MYSTERY);
		if (!far) ++*next;
		/* (the best three times in ten besides: what the old roll gave the
		 * rich data, about as much in all) */
		if (o) o->param = k == 0 && far && end.d >= DETOUR_FAR ? 2 : rng_range(0, 99) < 30 ? 2 : 1;
	}
	static int dx[256], dy[256];
	int nde = dead_ends(dx, dy, 256), di = 0;
	for (int i = nde - 1; i > 0; --i) {
		int j = rng_range(0, i), tx = dx[i], ty = dy[i];
		dx[i] = dx[j]; dy[i] = dy[j]; dx[j] = tx; dy[j] = ty;
	}
	int md = ng_npcs_for(2 + rng_range(0, 2) + size);
	for (int k = 0; k < md; ++k) {
		bool got = false;
		if (rng_range(0, 99) < 70)
			while (di < nde && !got) { x = dx[di]; y = dy[di++]; got = ng_cell_free(x, y) && !ng_behind_gap(x, y) && !ng_cuts_way(x, y); }
		if (!got && n) got = room_spot(&layer.rooms[order[rng_range(0, n - 1)]], &x, &y);
		if (!got) continue;
		NetObj *o = ng_add_obj(OBJ_MYSTERY, x, y);
		if (!o) break;
		o->param = 0;
	}
}

#undef PLACE

/* Bystander navis with a word to share, two panels at least from what else
 * stands there (one beside a Mystery Data took MegaMan's A, and each A
 * that closed his words opened them again), and never by the way on: they
 * may stay away (one beside a two-wide neck before the walkway down to an
 * arena walled it off, and a playtester never reached the guardian,
 * session 65). */
static void place_bystanders(const int *order, int n) {
	int x, y;
	int npcs = ng_npcs_for(2 + rng_range(0, 1));
	for (int k = 0; k < npcs && n; ++k)
		for (int tries = 0; tries < 12; ++tries) {
			Room *r = &layer.rooms[order[rng_range(0, n - 1)]];
			if (!ng_room_spot_in(r, &x, &y, true)) continue;
			if (!ng_apart(x, y) || ng_hushed[y][x] || layer_by_way(x, y)) continue;
			NetObj *o = ng_add_obj(OBJ_NPC, x, y);
			if (o) { o->param = rng_range(0, 5); o->npc_line = rng_range(0, 255); }
			break;
		}
}

/* The set pieces placed once the rest stands: the islands past the void
 * (a Rush gap's, a teleport's, an invisible path's), the pockets' data,
 * an arrow lane, a P-Code's teller; and the emblems. */
static void place_last(const GapSite *gap, unsigned pieces, int rise, const LayerKit *kit, const int *order, int n) {
	if (gap->x >= 0) ng_carve_gap(gap, rise);
	if (layer.teleport_island) ng_carve_teleport_island(rise);
	if (pieces & PIECE_HIDDEN) ng_place_hidden(rise);
	ng_place_block_rewards();
	if (pieces & PIECE_ARROW) ng_place_lane(kit);
	ng_place_teller(order, n);
	ng_fill_empty_detours();
	ng_emblems(kit);
}

void layer_generate(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit) {
	unsigned stair_dirs = kit ? kit->stair_dirs : 0;
	int rise = kit ? kit->rise : 0;
	memset(&layer, 0, sizeof layer);
	memset(ng_reserved, 0, sizeof ng_reserved);
	memset(ng_hushed, 0, sizeof ng_hushed);
	npcs_held = 0;
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
	/* an act's three layers each in another of the area's layouts and
	 * signatures (docs/LEVEL_DESIGN.md, Identity) */
	int planned, sig;
	layer_plan(seed, depth, biome, kind, kit, &planned, &sig);
	build_layout(planned, sig, biome, size, rise, &arena);
	finish_rooms(seed, stair_dirs, rise);
	place_ends(kind, biome, &arena);

	/* the landmark first, at the back of the layer's signature: its room
	 * bare of counters still (docs/LEVEL_DESIGN.md, Identity) */
	int land = -1;
	if (kit && kit->looks) {
		ng_route_distances();
		land = ng_landmark(kit);
	}
	Services s;
	roll_services(&s, depth, biome, kind);
	roll_gates(&s, seed, depth, kind);
	int order[MAX_ROOMS], n = room_order(order), next = 0;
	place_services(&s, depth, kind, kit, order, n, &next);
	/* the area's other props, set as the originals set theirs, before the
	 * loose Mystery Data and bystanders fill the rooms: rows and the signs
	 * (docs/LEVEL_DESIGN.md, Props) */
	if (kit && kit->looks) {
		ng_rows(kit, land, order, n);
		ng_signs(kit, land, order, n);
	}
	if (s.duel) place_duel(order, n, &next);
	if (s.official && s.duel) place_duel_gate(s.official_level, order, n, &next);
	/* the act's set pieces (net_pieces.c): a Rush gap's stand kept clear,
	 * its island carved once the rest stands */
	GapSite gap = { -1, -1, 0, 0, -1 };
	unsigned pieces = layer_pieces(depth, biome, kind);
	ng_measure_detours();
	if (pieces & PIECE_RUSH) {
		gap = ng_plan_gap(layer_rush_len(depth, biome));
		npcs_held = gap.x >= 0;
	}
	if ((pieces & PIECE_TELEPORT) && kit && kit->gem) {
		ng_plan_teleport();
		npcs_held += layer.teleport_island;   /* (the island's data, placed last) */
	}
	int block = pieces & PIECE_OBSTACLE ? layer_block_kind(depth, biome) : -1;
	if (block >= 0) ng_plan_obstacle(block);
	if (pieces & PIECE_CUBE) ng_plan_obstacle(ng_cube_kind(depth, biome, kind));
	npcs_held += layer.nblocks + (ng_pcode_cube() >= 0);   /* (the pockets' data, and a P-Code's teller, placed last) */
	npcs_held += pieces & PIECE_HIDDEN ? 2 : 0;          /* (an invisible path's data and the navi who hints at it) */
	place_data(depth, kind, biome, size, order, n, &next);
	place_bystanders(order, n);
	npcs_held = 0;
	place_last(&gap, pieces, rise, kit, order, n);
}

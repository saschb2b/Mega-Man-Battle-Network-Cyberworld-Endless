/* Layer generation: a layout after the area's own maps (net_layouts.c),
 * then points of interest (docs/LEVEL_DESIGN.md). */
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "game.h"
#include "net.h"
#include "net_arena.h"
#include "net_layouts.h"
#include "net_shapes.h"
#include "pacing.h"
#include "run.h"

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

static NetObj *add_obj(int type, int x, int y) {
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
static uint8_t reserved[MAP_H][MAP_W];
/* ... and the panels round where it is spoken to (a Rush stand, an
 * obstacle's or a cube's mouth), kept clear of navis: the engine's A turns
 * to a navi within 52 units before a map's check (a P-Code's teller two
 * panels from his cube took its A; issue #45). */
#define PIECE_QUIET 3
static uint8_t hushed[MAP_H][MAP_W];

static bool cell_free(int x, int y) {
	if (layer.cell[y][x] != C_PATH || reserved[y][x]) return false;
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
static bool cuts(int n, const int *xs, const int *ys) {
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

static bool cuts_way(int x, int y) { return cuts(1, &x, &y); }

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
static bool behind_gap(int x, int y) {
	return (x + 2 < MAP_W && layer.cell[y][x + 1] == C_VOID && layer.cell[y][x + 2] == C_PATH) ||
		(y + 2 < MAP_H && layer.cell[y + 1][x] == C_VOID && layer.cell[y + 2][x] == C_PATH);
}

/* A free cell inside a room, off its middle, its exits and any walkway's
 * mouth so paths stay clear, where a solid object cuts no way; with `open`, first one with
 * floor on its four sides (a bystander in a panel-wide gap beside a
 * service pinned MegaMan in the nook, where the way still went round). */
/* Whether a panel-wide stretch of walkway touches (x, y) corner to corner. */
static bool by_walkway(int x, int y) {
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

/* The way from the arrival to the exit or guardian, the shortest walk over
 * the floor (2), and the panels beside it, corner to corner too (1): a
 * navi's radius reaches half a panel past its own, and a Recovery Mr. Prog
 * beside the way's turn stopped MegaMan walking it, three calls to get
 * round. */
static uint8_t way_band[MAP_H][MAP_W];

bool layer_by_way(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && way_band[y][x]; }
bool layer_on_way(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && way_band[y][x] == 2; }

static void mark_way(int sx, int sy, int gx, int gy) {
	static int16_t from[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(way_band, 0, sizeof way_band);
	for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) from[y][x] = -1;
	int head = 0, tail = 0;
	from[sy][sx] = (int16_t)(sy * MAP_W + sx);
	qx[tail] = (int16_t)sx; qy[tail++] = (int16_t)sy;
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	while (head < tail && from[gy][gx] < 0) {
		int x = qx[head], y = qy[head++];
		for (int k = 0; k < 4; ++k) {
			int nx = x + d4[k][0], ny = y + d4[k][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || layer.cell[ny][nx] != C_PATH || from[ny][nx] >= 0) continue;
			from[ny][nx] = (int16_t)(y * MAP_W + x);
			qx[tail] = (int16_t)nx; qy[tail++] = (int16_t)ny;
		}
	}
	if (from[gy][gx] < 0) return;
	for (int x = gx, y = gy;;) {
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx)
				if (x + dx >= 0 && y + dy >= 0 && x + dx < MAP_W && y + dy < MAP_H && !way_band[y + dy][x + dx]) way_band[y + dy][x + dx] = 1;
		way_band[y][x] = 2;
		if (x == sx && y == sy) break;
		int f = from[y][x];
		x = f % MAP_W; y = f / MAP_W;
	}
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
				    way_band[gy][gx] == 2) return true;
	}
	return false;
}

/* Whether one MegaMan talks to stands within two panels of (x, y), three
 * of one behind a counter, who is talked to from the counter's front: an A
 * meant for a heal Prog opened the Net Dealer beside him, twice. */
static bool near_talker(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i) {
		int t = layer.obj[i].type, reach = layer.obj[i].prop >= 0 ? 3 : 2;
		bool talks = t == OBJ_SHOP || t == OBJ_HEAL || t == OBJ_TRADER || t == OBJ_BUGTRADER || t == OBJ_NPC ||
			t == OBJ_CHALLENGE || t == OBJ_PROGRAMS || t == OBJ_GIFT;
		if (talks && abs((int)layer.obj[i].x - x) <= reach && abs((int)layer.obj[i].y - y) <= reach) return true;
	}
	return false;
}

/* Whether a navi of any kind stands within PIECE_QUIET panels of (x, y). */
static bool navi_near(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i) {
		int t = layer.obj[i].type;
		bool navi = t != OBJ_WARP_IN && t != OBJ_EXIT && t != OBJ_MYSTERY && t != OBJ_UNDERNET && t != OBJ_RETURN;
		if (navi && abs((int)layer.obj[i].x - x) <= PIECE_QUIET && abs((int)layer.obj[i].y - y) <= PIECE_QUIET) return true;
	}
	return false;
}

static void hush(int x, int y) {
	for (int j = y - PIECE_QUIET; j <= y + PIECE_QUIET; ++j)
		for (int i = x - PIECE_QUIET; i <= x + PIECE_QUIET; ++i)
			if (i >= 0 && j >= 0 && i < MAP_W && j < MAP_H) hushed[j][i] = 1;
}

static bool room_spot_in(const Room *r, int *ox, int *oy, bool open) {
	for (int tries = 0; tries < 40; ++tries) {
		int x = r->x + rng_range(0, r->w - 1), y = r->y + rng_range(0, r->h - 1);
		if ((tries < 30 || open) && x == r->ax && y == r->ay) continue;
		if ((tries < 34 || open) && in_way_line(x, y)) continue;
		/* (off the way and its band most of the tries: navis by the way
		 * stood in a playtester's, the Chip Trader on layer 2's and a
		 * bystander square on the way to an arena, session 61) */
		if (tries < 24 && way_band[y][x]) continue;
		if (tries < 32 && gap_on_way(x, y)) continue;
		if (tries < 30 && near_talker(x, y)) continue;
		/* (a bystander, who may stay away, never at one, nor corner to
		 * corner with a walkway: one on a platform's corner beside its way
		 * in stood in the way) */
		if ((tries < 36 || open) && (at_exit(r, x, y) || beside_narrow(x, y) || by_walkway(x, y))) continue;
		if (open && tries < 24 && !(layer.cell[y][x + 1] == C_PATH && layer.cell[y][x - 1] == C_PATH &&
			layer.cell[y + 1][x] == C_PATH && layer.cell[y - 1][x] == C_PATH)) continue;
		/* (not beside a floor of another height: a NaviCust vendor below a
		 * raised platform's edge looked a step away from it and was a
		 * stair's walk round) */
		if (tries < 30 && beside_other_level(x, y)) continue;
		/* (a bystander, who may stay away, never there) */
		if ((tries < 38 || open) && behind_gap(x, y)) continue;
		if (cell_free(x, y) && !cuts_way(x, y)) { *ox = x; *oy = y; return true; }
	}
	return false;
}

static bool room_spot(const Room *r, int *ox, int *oy) { return room_spot_in(r, ox, oy, false); }

/* ---- Detours (docs/LEVEL_DESIGN.md, Set pieces) ----
 * How far each floor cell lies off the way, in panels walked (0 on it, -1
 * where no walk reaches), and the way's cell its walk leaves from: the
 * cells of a spur, a room off the way and what lies past them share it, a
 * branch. The originals' blue data lie at a branch's far end, and what a
 * detour holds follows its length. */
static int16_t detour[MAP_H][MAP_W], branch[MAP_H][MAP_W];
/* (a blue data's detour at least, three panels there and three back; the
 * long walk that holds the best) */
#define DETOUR_BLUE 3
#define DETOUR_FAR 8
#define DETOUR_EMPTY 5   /* (a detour this long is never left empty: fill_empty_detours) */

/* (the guardian's arena is no detour: the way ends at him, in its middle) */
static bool in_arena(int x, int y) {
	if (layer.arena < 0) return false;
	const Room *a = &layer.rooms[layer.arena];
	return x >= a->x && x < a->x + a->w && y >= a->y && y < a->y + a->h;
}

static void measure_detours(void) {
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	int h = 0, t = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			detour[y][x] = branch[y][x] = -1;
			if (way_band[y][x] != 2 || layer.cell[y][x] != C_PATH || in_arena(x, y)) continue;
			detour[y][x] = 0;
			branch[y][x] = (int16_t)(y * MAP_W + x);
			qx[t] = (int16_t)x; qy[t++] = (int16_t)y;
		}
	while (h < t) {
		int x = qx[h], y = qy[h++];
		for (int k = 0; k < 4; ++k) {
			int nx = x + d4[k][0], ny = y + d4[k][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || layer.cell[ny][nx] != C_PATH || detour[ny][nx] >= 0 || in_arena(nx, ny)) continue;
			detour[ny][nx] = (int16_t)(detour[y][x] + 1);
			branch[ny][nx] = branch[y][x];
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
}

int layer_detour(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H ? detour[y][x] : -1; }
int layer_branch(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H ? branch[y][x] : -1; }

/* A pad's middle where (x, y) lies on a pad: the originals set a pad's data
 * there; else (x, y). */
static void pad_middle(int *x, int *y) {
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *r = &layer.rooms[i];
		if (r->kind != ROOM_PAD || *x < r->x || *x >= r->x + r->w || *y < r->y || *y >= r->y + r->h) continue;
		int mx = r->x + r->w / 2, my = r->y + r->h / 2;
		if (cell_free(mx, my) && !cuts_way(mx, my)) { *x = mx; *y = my; }
		return;
	}
}

/* The far end of a detour: the farthest free cell, `min` panels off the
 * way or more, of a branch not `taken` yet (which it then takes); false
 * where none is left. One at a time, as each object placed changes where
 * the next may stand. */
typedef struct { int x, y, d; } DetourEnd;

static bool detour_end(int min, uint8_t *taken, DetourEnd *out) {
	enum { MAX_D = 160 };
	static int16_t cells[MAP_W * MAP_H];
	int count[MAX_D + 1] = { 0 }, start[MAX_D + 1], total = 0;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (detour[y][x] >= min) { ++count[detour[y][x] < MAX_D ? detour[y][x] : MAX_D]; ++total; }
	/* (by distance, farthest first; in a distance, in the grid's order) */
	for (int d = MAX_D, at = 0; d >= 0; --d) { start[d] = at; at += count[d]; }
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (detour[y][x] >= min) cells[start[detour[y][x] < MAX_D ? detour[y][x] : MAX_D]++] = (int16_t)(y * MAP_W + x);
	for (int i = 0; i < total; ++i) {
		int x = cells[i] % MAP_W, y = cells[i] / MAP_W, b = branch[y][x];
		if (taken[b] || !cell_free(x, y) || behind_gap(x, y) || near_talker(x, y) || cuts_way(x, y)) continue;
		taken[b] = 1;
		out->d = detour[y][x];
		pad_middle(&x, &y);
		out->x = x; out->y = y;
		return true;
	}
	return false;
}

/* The branch (x, y) lies on, where it is off the way; -1 on the way or off
 * the floor. */
static int off_way_branch(int x, int y) { return layer_detour(x, y) > 0 ? layer_branch(x, y) : -1; }

/* A green data lying loose: on the way, or on a detour shorter than
 * DETOUR_EMPTY (`far`: each branch's longest), and no set piece's; the
 * nearest the way; -1 none. */
static int loose_green(const int16_t *far) {
	int best = -1, best_d = 1 << 30;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		if (o->type != OBJ_MYSTERY || o->param != 0 || o->prize) continue;
		int x = (int)o->x, y = (int)o->y, d = layer_detour(x, y), b = off_way_branch(x, y);
		if (d < 0 || (b >= 0 && far[b] >= DETOUR_EMPTY) || d >= best_d) continue;
		best = i;
		best_d = d;
	}
	return best;
}

static int npcs_for(int want);

/* A branch's cells and how wide it runs: wide[b] where two by two of its
 * panels are floor (a band that reads as the way, not a spur) */
static void branch_shape(int16_t *far, uint8_t *wide) {
	int b;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if ((b = off_way_branch(x, y)) < 0) continue;
			if (detour[y][x] > far[b]) far[b] = detour[y][x];
			if (off_way_branch(x + 1, y) == b && off_way_branch(x, y + 1) == b && off_way_branch(x + 1, y + 1) == b) wide[b] = 1;
		}
}

/* Once all stands: a detour DETOUR_EMPTY panels or more off the way that
 * holds nothing, no object and no set piece's stand, takes a loose green
 * data at its end, the farthest first (a playtester walked a two-wide band
 * to an empty end, session 59; half the layers held one, 321 of 1589
 * such detours); moved, not added. A wide one still empty takes a new
 * one where the map has room: a band reads as the way, a spur to nothing
 * as BN6's own. */
static void fill_empty_detours(void) {
	static uint8_t holds[MAP_W * MAP_H], wide[MAP_W * MAP_H];
	static int16_t far[MAP_W * MAP_H], keep_detour[MAP_H][MAP_W], keep_branch[MAP_H][MAP_W];
	memset(holds, 0, sizeof holds);
	memset(far, 0, sizeof far);
	memset(wide, 0, sizeof wide);
	/* (the layer's detours as its data were placed stay its own: an arrow
	 * lane, carved since, is measured from them) */
	memcpy(keep_detour, detour, sizeof detour);
	memcpy(keep_branch, branch, sizeof branch);
	measure_detours();
	branch_shape(far, wide);
	int b;
	for (int i = 0; i < layer.nobj; ++i)
		if ((b = off_way_branch((int)layer.obj[i].x, (int)layer.obj[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.ngaps; ++i) if ((b = off_way_branch(layer.gap[i].x, layer.gap[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.nblocks; ++i) if ((b = off_way_branch(layer.block[i].x, layer.block[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.npaths; ++i) if ((b = off_way_branch(layer.path[i].x, layer.path[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < layer.nlanes; ++i) if ((b = off_way_branch(layer.lane[i].x, layer.lane[i].y)) >= 0) holds[b] = 1;
	for (int i = 0; i < 2 * (layer.nteleports > 0); ++i)
		if ((b = off_way_branch(layer.teleport_x[i], layer.teleport_y[i])) >= 0) holds[b] = 1;
	DetourEnd end;
	int g;
	while ((g = loose_green(far)) >= 0 && detour_end(DETOUR_EMPTY, holds, &end)) {
		layer.obj[g].x = (float)end.x + 0.5f;
		layer.obj[g].y = (float)end.y + 0.5f;
	}
	for (b = 0; b < MAP_W * MAP_H; ++b) holds[b] |= !wide[b];
	NetObj *o;
	while (npcs_for(1) && detour_end(DETOUR_EMPTY, holds, &end) && (o = add_obj(OBJ_MYSTERY, end.x, end.y))) o->param = 0;
	memcpy(detour, keep_detour, sizeof detour);
	memcpy(branch, keep_branch, sizeof branch);
}

/* ---- Props (docs/LEVEL_DESIGN.md, Props) ---- */

static bool object_at(int x, int y);
static bool near_stair(int x, int y);

/* An emblem on (x, y): ground floor all round, nothing standing there. */
static void emblem_at(int x, int y) {
	if (layer.nprops >= MAX_PROPS || x < 1 || y < 1 || x >= MAP_W - 1 || y >= MAP_H - 1 || object_at(x, y) || near_stair(x, y)) return;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
			if (layer.cell[y + dy][x + dx] != C_PATH || layer.level[y + dy][x + dx]) return;
	/* (rooms that overlap set theirs apart) */
	for (int i = 0; i < layer.nprops; ++i)
		if (layer.props[i].kind == PROP_EMBLEM && abs(layer.props[i].x - x) <= 1 && abs(layer.props[i].y - y) <= 1) return;
	layer.props[layer.nprops++] = (NetProp){ PROP_EMBLEM, 0, x, y, 1, -1 };
}

/* The area's emblem in the floor, as the Graveyard's maps set their
 * crosses: on a small room's middle panel, and on a bigger one in two rows
 * alongside its middle line (its holes, where it has them), two panels to
 * either side, every second or third panel along, from the middle out. */
static void emblems(const LayerKit *kit) {
	if (!kit || !kit->emblem) return;
	for (int i = 0; i < layer.nrooms; ++i) {
		const Room *r = &layer.rooms[i];
		int cx = r->x + r->w / 2, cy = r->y + r->h / 2;
		if (r->w <= 5 && r->h <= 5) { emblem_at(cx, cy); continue; }
		bool along_x = r->w >= r->h;
		int len = along_x ? r->w : r->h, step = len <= 7 ? 2 : 3, reach = len / 2 / step;
		for (int side = -2; side <= 2; side += 4)
			for (int k = -reach; k <= reach; ++k)
				emblem_at(along_x ? cx + k * step : cx + side, along_x ? cy + side : cy + k * step);
	}
}

#define MAX_COUNTER 4   /* panels */

/* within a cell of a stair's block: its landing stays open */
static bool near_stair(int x, int y) {
	for (int i = 0; i < layer.nstairs; ++i)
		if (x >= layer.stair[i].x - 1 && x < layer.stair[i].x + 3 && y >= layer.stair[i].y - 1 && y < layer.stair[i].y + 3) return true;
	return false;
}

static bool object_at(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i)
		if ((int)layer.obj[i].x == x && (int)layer.obj[i].y == y) return true;
	return false;
}

/* A counter in room r for a service of `type` to stand behind, as the
 * originals set their Net Dealers' capsules and NetCafe desks: one panel in
 * from a back edge (grid -x or -y, the top of the screen), facing the
 * camera, with the aisle behind it on the platform's rim (the navi's place,
 * walled off), centred along that edge, and the floor before it free to
 * talk from; nothing it closes off may be cut from the rest. The object
 * stands in the aisle behind the counter's middle; NULL where the room has
 * no such place. */
static NetObj *counter(int r, int type, const LayerKit *kit) {
	const Room *m = &layer.rooms[r];
	if (!kit || layer.nprops >= MAX_PROPS || (kit->counter_len[0] <= 0 && kit->counter_len[1] <= 0)) return NULL;
	int first = rng_range(0, 1);
	for (int k = 0; k < 2; ++k) {
		int faces = first ^ k, len = kit->counter_len[faces];
		/* u along the run, v in depth from the back edge: FACES_X runs along y */
		int span = faces == FACES_X ? m->h : m->w, deep = faces == FACES_X ? m->w : m->h;
		if (len <= 0 || len > MAX_COUNTER || len > span || deep < 3) continue;
		int u_base = faces == FACES_X ? m->y : m->x, v0 = faces == FACES_X ? m->x : m->y;
		int mid = (span - len) / 2;
		for (int j = 0; j <= span - len; ++j) {
			int off = mid + ((j & 1) ? (j + 1) / 2 : -(j / 2));   /* from the middle outwards */
			if (off < 0 || off > span - len) continue;
			int u0 = u_base + off, xs[2 * MAX_COUNTER], ys[2 * MAX_COUNTER], nb = 0;
			bool ok = true;
			for (int t = 0; t < len && ok; ++t)
				/* d -1 behind the aisle (void: the rim), 0 the aisle, 1 the counter, 2 before it */
				for (int d = -1; d <= 2 && ok; ++d) {
					int x = faces == FACES_X ? v0 + d : u0 + t, y = faces == FACES_X ? u0 + t : v0 + d;
					if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H) { ok = d == -1; continue; }
					if (d == -1) { ok = layer.cell[y][x] == C_VOID; continue; }
					if (layer.cell[y][x] != C_PATH || layer.level[y][x] || near_stair(x, y) || object_at(x, y) ||
					    (x == m->ax && y == m->ay)) { ok = false; continue; }
					if (d == 2) { ok = cell_free(x, y); continue; }
					xs[nb] = x; ys[nb++] = y;
				}
			if (!ok || cuts(nb, xs, ys)) continue;
			/* the aisle walled off, the counter's panels floor under its own walls */
			for (int i = 0; i < nb; ++i) layer.cell[ys[i]][xs[i]] = (uint8_t)(i % 2 == 0 ? C_SOLID : C_PROPPED);
			int tm = (len - 1) / 2;
			NetObj *o = add_obj(type, faces == FACES_X ? v0 : u0 + tm, faces == FACES_X ? u0 + tm : v0);
			if (!o) {
				for (int i = 0; i < nb; ++i) layer.cell[ys[i]][xs[i]] = C_PATH;
				return NULL;
			}
			layer.props[layer.nprops] = (NetProp){ PROP_COUNTER, faces, faces == FACES_X ? v0 + 1 : u0, faces == FACES_X ? u0 : v0 + 1, len, 0 };
			o->prop = layer.nprops++;
			return o;
		}
	}
	return NULL;
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
			if (dist[y][x] > 0 && cell_free(x, y) && !cuts_way(x, y) && !in_way_line(x, y) && (loose || !near_talker(x, y))) {
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
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	memset(dist, -1, sizeof dist);
	int h = 0, t = 0, n = 0;
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
}

/* ---- Sprite props: sets as the originals compose them (docs/LEVEL_DESIGN.md,
 * Props) ---- */

static int16_t rdist[MAP_H][MAP_W];   /* cells from the way between the warps */

static bool floor_cell(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_PATH; }
static bool void_cell(int x, int y) { return x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && layer.cell[y][x] == C_VOID; }

/* The cells on a shortest way from the arrival to the exit (or the
 * guardian), and every cell's distance from them, void too: decoration
 * keeps off the way players walk. */
static void route_distances(void) {
	static int16_t da[MAP_H][MAP_W], db[MAP_H][MAP_W];
	static int16_t qx[MAP_W * MAP_H], qy[MAP_W * MAP_H];
	static const int d4[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	int gx = -1, gy = -1;
	for (int i = 0; i < layer.nobj; ++i)
		if (layer.obj[i].type == OBJ_EXIT || layer.obj[i].type == OBJ_RETURN || layer.obj[i].type == OBJ_BOSS) { gx = (int)layer.obj[i].x; gy = (int)layer.obj[i].y; }
	int16_t (*d[2])[MAP_W] = { da, db };
	int sx[2] = { (int)layer.obj[0].x, gx }, sy[2] = { (int)layer.obj[0].y, gy };
	for (int k = 0; k < 2; ++k) {
		for (int y = 0; y < MAP_H; ++y) for (int x = 0; x < MAP_W; ++x) d[k][y][x] = -1;
		if (sx[k] < 0) continue;
		int h = 0, t = 0;
		d[k][sy[k]][sx[k]] = 0;
		qx[t] = (int16_t)sx[k]; qy[t++] = (int16_t)sy[k];
		while (h < t) {
			int cx = qx[h], cy = qy[h++];
			for (int j = 0; j < 4; ++j) {
				int nx = cx + d4[j][0], ny = cy + d4[j][1];
				if (!floor_cell(nx, ny) || d[k][ny][nx] >= 0) continue;
				d[k][ny][nx] = (int16_t)(d[k][cy][cx] + 1);
				qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
			}
		}
	}
	int h = 0, t = 0, whole = gx >= 0 ? da[gy][gx] : -1;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			rdist[y][x] = -1;
			if (whole >= 0 && da[y][x] >= 0 && db[y][x] >= 0 && da[y][x] + db[y][x] == whole) {
				rdist[y][x] = 0;
				qx[t] = (int16_t)x; qy[t++] = (int16_t)y;
			}
		}
	while (h < t) {
		int cx = qx[h], cy = qy[h++];
		for (int j = 0; j < 4; ++j) {
			int nx = cx + d4[j][0], ny = cy + d4[j][1];
			if (nx < 0 || ny < 0 || nx >= MAP_W || ny >= MAP_H || rdist[ny][nx] >= 0) continue;
			rdist[ny][nx] = (int16_t)(rdist[cy][cx] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
}

static int far_from_way(int x, int y) { return rdist[y][x] < 0 ? 99 : rdist[y][x]; }

static bool prop_at_cell(int x, int y) {
	for (int i = 0; i < layer.nprops; ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind == PROP_SPRITE && p->x == x && p->y == y) return true;
	}
	return false;
}

static bool add_sprite(int look, int x, int y) {
	if (layer.nprops >= MAX_PROPS) return false;
	layer.props[layer.nprops++] = (NetProp){ PROP_SPRITE, 0, x, y, 1, look };
	return true;
}

/* A room's back edge, along its -x side (s 0) or -y side (s 1), where it is
 * the platform's rim: the longest run of its edge cells with floor on them
 * (at ground level, with no object and no counter) and void past them.
 * Cell u of it lies at edge(s, u), the void past it at past(s, u). */
typedef struct { int s, v, u0, len; } Rim;
static void edge_cell(const Room *m, int s, int u, int *x, int *y) { *x = s == 0 ? m->x : u; *y = s == 0 ? u : m->y; }
static void past_cell(const Room *m, int s, int u, int k, int *x, int *y) { *x = s == 0 ? m->x - k : u; *y = s == 0 ? u : m->y - k; }

static Rim back_rim(const Room *m, int s) {
	Rim best = { s, 0, 0, 0 };
	int span = s == 0 ? m->h : m->w, base = s == 0 ? m->y : m->x, streak = 0;
	for (int i = 0; i <= span; ++i) {
		bool ok = false;
		if (i < span) {
			int ex, ey, px, py, qx, qy;
			edge_cell(m, s, base + i, &ex, &ey);
			past_cell(m, s, base + i, 1, &px, &py);
			past_cell(m, s, base + i, 2, &qx, &qy);
			/* (void two deep: not a gap between platforms) */
			ok = floor_cell(ex, ey) && !layer.level[ey][ex] && !object_at(ex, ey) && void_cell(px, py) && void_cell(qx, qy);
		}
		if (ok) { ++streak; continue; }
		if (streak > best.len) { best.len = streak; best.u0 = base + i - streak; }
		streak = 0;
	}
	return best;
}

/* Whether a room may carry a set: no counter in it or beside it. */
static bool room_bare(const Room *m) {
	for (int y = m->y - 1; y <= m->y + m->h; ++y)
		for (int x = m->x - 1; x <= m->x + m->w; ++x)
			if (x >= 0 && y >= 0 && x < MAP_W && y < MAP_H && (layer.cell[y][x] == C_SOLID || layer.cell[y][x] == C_PROPPED)) return false;
	return true;
}

/* A walled hole of n cells in a row along the floor (grid x for dir 0, y
 * for 1) from (x, y): floor all round it (the originals' statues and
 * stones stand in the floor's middle), clear of objects, walkway mouths
 * and the way, and cutting nothing off. */
static bool anchor_at(int x, int y) {
	for (int r = 0; r < layer.nrooms; ++r)
		if (layer.rooms[r].ax == x && layer.rooms[r].ay == y) return true;
	return false;
}

static bool hole_fits(int x, int y, int n, int dir) {
	int xs[4], ys[4];
	for (int i = 0; i < n; ++i) {
		int cx = dir == 0 ? x + i : x, cy = dir == 0 ? y : y + i;
		if (!floor_cell(cx, cy) || layer.level[cy][cx] || near_stair(cx, cy) || !cell_free(cx, cy) || beside_narrow(cx, cy) ||
		    anchor_at(cx, cy) || far_from_way(cx, cy) < 1) return false;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = cx + dx, ny = cy + dy;
				bool in_row = dir == 0 ? (ny == cy && nx >= x && nx < x + n) : (nx == cx && ny >= y && ny < y + n);
				if (!in_row && (!floor_cell(nx, ny) || layer.level[ny][nx])) return false;
			}
		xs[i] = cx; ys[i] = cy;
	}
	return !cuts(n, xs, ys);
}

static void wall_off(int x, int y, int n, int dir, int look) {
	for (int i = 0; i < n; ++i) {
		int cx = dir == 0 ? x + i : x, cy = dir == 0 ? y : y + i;
		layer.cell[cy][cx] = C_SOLID;
		add_sprite(look, cx, cy);
	}
}

/* The layer's landmark (one set piece, at the back of its biggest room off
 * the way): Green's giant cybertree in the floor with an avenue of trees
 * past the rim on both sides of it, mirrored; the Undernet's statue past
 * the rim between two braziers, mirrored; the Graveyard's monument past it.
 * Returns the room it stands in, or -1. */
static int landmark(const LayerKit *kit) {
	int set = kit->looks & (1u << LOOK_GIANT_TREE) ? LOOK_GIANT_TREE : kit->looks & (1u << LOOK_STATUE) ? LOOK_STATUE
		: kit->looks & (1u << LOOK_MONUMENT) ? LOOK_MONUMENT : -1;
	if (set < 0) return -1;
	int need = set == LOOK_STATUE ? 5 : 3;
	/* the candidates, best first: big rooms, long rims, far from the way */
	struct { int r, score; Rim rim; } cand[MAX_ROOMS * 2];
	int nc = 0;
	for (int r = 1; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		if (r == layer.exit_room || r == layer.arena || m->kind == ROOM_PAD || m->w * m->h < 12 || !room_bare(m)) continue;
		for (int s = 0; s < 2; ++s) {
			Rim rim = back_rim(m, s);
			if (rim.len < need) continue;
			int ex, ey;
			edge_cell(m, s, rim.u0 + rim.len / 2, &ex, &ey);
			int away = far_from_way(ex, ey);
			if (away < 1) continue;
			cand[nc].r = r;
			cand[nc].rim = rim;
			cand[nc++].score = m->w * m->h * 4 + rim.len * 8 + (away > 6 ? 6 : away) * 10;
		}
	}
	for (int i = 1; i < nc; ++i)
		for (int j = i; j > 0 && cand[j].score > cand[j - 1].score; --j) {
			__typeof__(cand[0]) t = cand[j]; cand[j] = cand[j - 1]; cand[j - 1] = t;
		}
	for (int c = 0; c < nc; ++c) {
		const Room *m = &layer.rooms[cand[c].r];
		Rim rim = cand[c].rim;
		int s = rim.s, mid0 = rim.u0 + rim.len / 2;
		/* the middle of the rim, else the nearest place along it the set fits */
		for (int j = 0; j < 2 * rim.len; ++j) {
			int mid = mid0 + ((j & 1) ? (j + 1) / 2 : -(j / 2));
			int half = set == LOOK_STATUE ? 2 : 1;
			if (mid - half < rim.u0 || mid + half >= rim.u0 + rim.len) continue;
			int px, py;
			past_cell(m, s, mid, 1, &px, &py);
			if (set == LOOK_GIANT_TREE) {
				/* in the floor one panel in from the rim, the trees past it */
				int hx, hy;
				edge_cell(m, s, mid, &hx, &hy);
				if (s == 0) ++hx; else ++hy;
				if (!hole_fits(hx, hy, 1, 0)) continue;
				wall_off(hx, hy, 1, 0, LOOK_GIANT_TREE);
				for (int k = 1; k <= 4; ++k) {
					int a = mid - k, b = mid + k, ax, ay, bx, by;
					if (a < rim.u0 || b >= rim.u0 + rim.len) break;
					past_cell(m, s, a, 1, &ax, &ay);
					past_cell(m, s, b, 1, &bx, &by);
					if (far_from_way(ax, ay) < 2 || far_from_way(bx, by) < 2) break;
					add_sprite(LOOK_TREE, ax, ay);
					add_sprite(LOOK_TREE, bx, by);
				}
				return cand[c].r;
			}
			add_sprite(set, px, py);
			if (set == LOOK_STATUE && kit->looks & (1u << LOOK_BRAZIER)) {
				int ax, ay, bx, by;
				past_cell(m, s, mid - 2, 1, &ax, &ay);
				past_cell(m, s, mid + 2, 1, &bx, &by);
				add_sprite(LOOK_BRAZIER, ax, ay);
				add_sprite(LOOK_BRAZIER, bx, by);
			}
			return cand[c].r;
		}
	}
	return -1;
}

/* Whether void panel (x, y) is a set piece's: a Rush gap's or an
 * invisible path's, which a prop past a rim must keep out of. */
static bool piece_void(int x, int y) {
	const NetGap *runs[MAX_GAPS + MAX_PATHS];
	int n = 0;
	for (int k = 0; k < layer.ngaps; ++k) runs[n++] = &layer.gap[k];
	for (int k = 0; k < layer.npaths; ++k) runs[n++] = &layer.path[k];
	for (int k = 0; k < n; ++k)
		for (int j = 0; j <= runs[k]->len + 1; ++j)
			if (abs(runs[k]->x + dir_dx[runs[k]->dir] * j - x) <= 1 && abs(runs[k]->y + dir_dy[runs[k]->dir] * j - y) <= 1) return true;
	return false;
}

bool layer_landmark_void(int *x, int *y) {
	/* (the monument's place, read off the layer as built: the best room's
	 * back rim, its middle off the way, the void a panel past it) */
	route_distances();
	int best = -1, best_x = 0, best_y = 0;
	for (int r = 1; r < layer.nrooms; ++r) {
		const Room *m = &layer.rooms[r];
		if (r == layer.exit_room || r == layer.arena || m->kind == ROOM_PAD || m->w * m->h < 12 || !room_bare(m)) continue;
		for (int s = 0; s < 2; ++s) {
			Rim rim = back_rim(m, s);
			int ex, ey, px, py;
			if (rim.len < 3) continue;
			edge_cell(m, s, rim.u0 + rim.len / 2, &ex, &ey);
			past_cell(m, s, rim.u0 + rim.len / 2, 1, &px, &py);
			int away = far_from_way(ex, ey), score = m->w * m->h * 4 + rim.len * 8 + (away > 6 ? 6 : away) * 10;
			if (away < 1 || piece_void(px, py) || score <= best) continue;
			best = score; best_x = px; best_y = py;
		}
	}
	*x = best_x; *y = best_y;
	return best >= 0;
}

/* Rows of three a panel apart, as the originals line up their trees past a
 * rim and their gravestones in one walled hole of three panels. */
/* A row of three trees past a room's back rim, as Green's originals line
 * theirs up. */
static bool tree_row(const Room *m) {
	int s = rng_range(0, 1);
	Rim rim = back_rim(m, s);
	if (rim.len < 3) rim = back_rim(m, s ^ 1);
	if (rim.len < 3) return false;
	int mid = rim.u0 + rim.len / 2, ok = 1, xs[3], ys[3];
	for (int k = -1; k <= 1; ++k) {
		past_cell(m, rim.s, mid + k, 1, &xs[k + 1], &ys[k + 1]);
		ok &= far_from_way(xs[k + 1], ys[k + 1]) >= 2 && !prop_at_cell(xs[k + 1], ys[k + 1]);
	}
	if (!ok) return false;
	for (int k = 0; k < 3; ++k) add_sprite(LOOK_TREE, xs[k], ys[k]);
	return true;
}

/* A row of two or three braziers past a room's back rim, as the Undernet's
 * originals line theirs up: the flames of hatred a number door counts
 * (issue #47). */
static bool brazier_row(const Room *m) {
	int s = rng_range(0, 1), len = 2 + rng_range(0, 1);
	Rim rim = back_rim(m, s);
	if (rim.len < len) rim = back_rim(m, s ^ 1);
	if (rim.len < len) return false;
	int first = rim.u0 + (rim.len - len) / 2, xs[3], ys[3];
	for (int k = 0; k < len; ++k) {
		past_cell(m, rim.s, first + k, 1, &xs[k], &ys[k]);
		if (far_from_way(xs[k], ys[k]) < 2 || prop_at_cell(xs[k], ys[k])) return false;
	}
	for (int k = 0; k < len; ++k) add_sprite(LOOK_BRAZIER, xs[k], ys[k]);
	return true;
}

static void rows(const LayerKit *kit, int skip, const int *order, int n) {
	int trees = 0, graves = 0, fires = 0;
	for (int i = 0; i < n; ++i) {
		int r = order[i];
		const Room *m = &layer.rooms[r];
		if (r == skip || r == 0 || r == layer.arena || !room_bare(m)) continue;
		if (!(kit->looks & (1u << LOOK_TREE)) && (kit->looks & (1u << LOOK_BRAZIER)) && fires < 3 && m->w * m->h >= 9 && brazier_row(m)) {
			++fires;
			continue;
		}
		if ((kit->looks & (1u << LOOK_TREE)) && trees < 2 && m->w * m->h >= 9 && tree_row(m)) {
			++trees;
			continue;
		}
		if ((kit->looks & (1u << LOOK_GRAVE)) && graves < 2 && m->w >= 5 && m->h >= 5) {
			int dir = rng_range(0, 1);
			for (int tries = 0; tries < 12; ++tries) {
				int x = m->x + 1 + rng_range(0, m->w - 3 - (dir == 0 ? 2 : 0)), y = m->y + 1 + rng_range(0, m->h - 3 - (dir == 1 ? 2 : 0));
				if (!hole_fits(x, y, 3, dir)) continue;
				wall_off(x, y, 3, dir, LOOK_GRAVE);
				++graves;
				break;
			}
		}
	}
}

/* The WELCOME sign past the rim beside a counter's aisle, at its end, as
 * the originals' NetCafes set theirs by the way in; the BBS past a big
 * room's rim. */
static void signs(const LayerKit *kit, int skip, const int *order, int n) {
	for (int i = 0; i < layer.nprops && (kit->looks & (1u << LOOK_SIGN)); ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind != PROP_COUNTER) continue;
		int cand[2][2];
		if (p->faces == FACES_X) { cand[0][0] = p->x - 2; cand[0][1] = p->y + p->len; cand[1][0] = p->x - 2; cand[1][1] = p->y - 1; }
		else { cand[0][0] = p->x + p->len; cand[0][1] = p->y - 2; cand[1][0] = p->x - 1; cand[1][1] = p->y - 2; }
		for (int k = 0; k < 2; ++k)
			if (void_cell(cand[k][0], cand[k][1]) && !prop_at_cell(cand[k][0], cand[k][1])) { add_sprite(LOOK_SIGN, cand[k][0], cand[k][1]); break; }
	}
	if (!(kit->looks & (1u << LOOK_BBS))) return;
	for (int i = 0; i < n; ++i) {
		const Room *m = &layer.rooms[order[i]];
		if (order[i] == skip || m->w * m->h < 12 || !room_bare(m)) continue;
		Rim rim = back_rim(m, rng_range(0, 1));
		if (rim.len < 3) continue;
		int x, y;
		past_cell(m, rim.s, rim.u0 + rim.len / 2, 1, &x, &y);
		if (far_from_way(x, y) < 2 || prop_at_cell(x, y)) continue;
		add_sprite(LOOK_BBS, x, y);
		return;
	}
}

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
		if (room_spot(&layer.rooms[room], x, y) && !near_talker(*x, *y)) return add_obj(type, *x, *y);
	}
	return room_spot(&layer.rooms[r], x, y) ? add_obj(type, *x, *y) : NULL;
}

/* The Net Dealer in room r: behind a counter where it has a place for one,
 * else in the first of `order`'s rooms that has (the originals' dealers
 * all stand behind one), else standing in r. */
static NetObj *dealer(int r, const int *order, int n, const LayerKit *kit, int *x, int *y) {
	if (r < 0) return NULL;
	NetObj *o = counter(r, OBJ_SHOP, kit);
	for (int i = 0; i < n && !o; ++i)
		if (order[i] != r) o = counter(order[i], OBJ_SHOP, kit);
	return o ? o : room_spot(&layer.rooms[r], x, y) ? add_obj(OBJ_SHOP, *x, *y) : NULL;
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
	for (int i = -1; i < nc; ++i) {
		const Room *r = &layer.rooms[i < 0 ? layer.ante : close[i]];
		if (room_spot(r, &x, &y) && !near_talker(x, y)) { add_obj(OBJ_HEAL, x, y); return; }
	}
	if (floor_near(layer.ante, HEAL_REACH, &x, &y) || room_spot(&layer.rooms[layer.ante], &x, &y)) add_obj(OBJ_HEAL, x, y);
}

/* The rooms' last shaping once the arena stands: the exit, the way to it
 * widened past the area's cap (net_way.c, the guardian's own bridge left
 * as his gate), the raised rooms. */
static void finish_rooms(uint32_t seed, unsigned stair_dirs, int rise) {
	layer.exit_room = layer.arena >= 0 ? layer.arena : bfs_far(0);
	const Room *goal = &layer.rooms[layer.arena >= 0 ? layer.ante : layer.exit_room];
	layer_widen_way(layer.biome, goal->ax, goal->ay);
	layer_raise_rooms(seed, stair_dirs, rise);
}

/* The layout: the planned one, then any of the area's, last the plainest
 * at its smallest; on a guardian's layer an arena of its own at the far
 * end. */
static void build_layout(int planned, int biome, int size, int rise, ArenaInfo *arena) {
	for (int attempt = 0; attempt < 12; ++attempt) {
		memset(layer.cell, 0, sizeof layer.cell);
		layer.nrooms = 0;
		arena->room = -1;
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
		if (arena_attach(ARENA_SIZE, arena) >= 0 && fits(rise)) break;
	}

	if (layer.arena < 0 && layer.boss_layer && arena->room >= 0 && arena->room < layer.nrooms) {
		layer.arena = arena->room;
		layer.ante = arena->ante;
		layer.arena_dir = arena->dir;
	}
}

/* The layer's ends: where MegaMan arrives, the exit and on a guardian's
 * layer the guardian; then the way between them. */
static void place_ends(int kind, int biome, const ArenaInfo *arena) {
	int cx = layer.rooms[0].ax, cy = layer.rooms[0].ay;
	add_obj(OBJ_WARP_IN, cx, cy);
	cx = layer.rooms[layer.exit_room].ax;
	cy = layer.rooms[layer.exit_room].ay;
	/* in an arena the exit waits behind the guardian, who holds the middle */
	int bx = cx, by = cy;
	if (layer.arena >= 0) { cx = arena->exit_x; cy = arena->exit_y; }
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
		mark_way(layer.rooms[0].ax, layer.rooms[0].ay, bx, by);
	} else {
		mark_way(layer.rooms[0].ax, layer.rooms[0].ay, cx, cy);
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
	s->shop = kind == LAYER_NORMAL && (biome_layer == 1 || (biome_layer == 0 && depth > 1) || rng_range(0, 99) < (biome_layer == 0 ? 50 : 25));
	/* (threat 2, docs/META.md: only the heals an act is sure of; the heals
	 * helper: one on every layer) */
	s->heal = (rng_range(0, 99) < (layer.boss_layer ? 70 : 30) && run.threat < 2) || (kind == LAYER_NORMAL && pacing_heal_certain(depth)) ||
		(run.helpers & HELP_HEALS);
	s->trader = rng_range(0, 99) < (run.threat >= 7 ? 12 : 25);   /* (threat 7: half as often) */
	s->programs = kind == LAYER_NORMAL && biome_layer == 1 && rng_range(0, 99) < 60;
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
	if (depth == 1 && kind == LAYER_NORMAL && room_spot(&layer.rooms[0], &x, &y)) add_obj(OBJ_GIFT, x, y);
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
		if (layer.rooms[order[i]].kind == ROOM_PAD && room_spot(&layer.rooms[order[i]], &x, &y) && !near_talker(x, y))
			placed = add_obj(OBJ_DUEL, x, y);
	if (!placed) { placed = PLACE(OBJ_DUEL); ++*next; }
	/* (a small layer, its rooms all taken: in any room but the
	 * exit's, apart from the others where it can be; an act 3 layer
	 * of the Judge Tree Comp had no ProtoMan) */
	for (int i = 0; i < n && !placed; ++i)
		if (room_spot(&layer.rooms[order[i]], &x, &y) && !near_talker(x, y)) placed = add_obj(OBJ_DUEL, x, y);
	for (int i = 0; i < layer.nrooms && !placed; ++i)
		if (i != layer.exit_room && room_spot_in(&layer.rooms[i], &x, &y, true)) placed = add_obj(OBJ_DUEL, x, y);
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
 * for the rival). */
static NetObj *gate_by_rival(const NetObj *rival) {
	int x, y, home = room_holding(rival->x, rival->y);
	int near[4], nn = home >= 0 ? nearest_rooms(home, near, 4, 12) : 0;
	for (int i = -1; i < nn && home >= 0; ++i) {
		int room = i < 0 ? home : near[i];
		if (room == layer.exit_room) continue;
		for (int tries = 0; tries < 12; ++tries)
			if (room_spot(&layer.rooms[room], &x, &y) && !near_talker(x, y) && (abs(x - (int)rival->x) >= 2 || abs(y - (int)rival->y) >= 2)) {
				NetObj *g = add_obj(OBJ_OFFICIAL, x, y);
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
		if (room_spot(&layer.rooms[order[i]], &x, &y) && !near_talker(x, y)) g = add_obj(OBJ_OFFICIAL, x, y);
	for (int i = 0; !g && i < layer.nrooms; ++i)
		if (i != layer.exit_room && room_spot_in(&layer.rooms[i], &x, &y, true)) g = add_obj(OBJ_OFFICIAL, x, y);
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

static int npcs_for(int want) {
	int left = LAYER_NPC_MAX - layer_npcs() - npcs_held;
	return want < left ? want : left > 0 ? left : 0;
}

/* The panel at the landmark's foot, in front of it where it can be (BN6's
 * Central Area 3 keeps its purple data beneath the statue). */
static bool landmark_foot(int *ox, int *oy) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	for (int i = 0; i < layer.nprops; ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind != PROP_SPRITE || (p->look != LOOK_GIANT_TREE && p->look != LOOK_STATUE && p->look != LOOK_MONUMENT)) continue;
		for (int k = 0; k < 4; ++k) {
			int x = p->x + d4[k][0], y = p->y + d4[k][1];
			if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || layer.cell[y][x] != C_PATH || layer_on_way(x, y)) continue;
			if (cell_free(x, y) && !behind_gap(x, y) && !near_talker(x, y) && !cuts_way(x, y)) { *ox = x; *oy = y; return true; }
		}
	}
	return false;
}

/* The layer's purple data, first of its data: at the landmark's foot, else
 * where the longest detour ends. */
static void place_purple(uint8_t *taken) {
	int x, y;
	DetourEnd end;
	bool at = landmark_foot(&x, &y);
	if (!at && detour_end(DETOUR_BLUE, taken, &end)) { x = end.x; y = end.y; at = true; }
	NetObj *o = at ? add_obj(OBJ_MYSTERY, x, y) : NULL;
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
	measure_detours();
	if (layer_purple(depth, biome, kind) && npcs_for(1)) place_purple(taken);
	int rich = npcs_for(2 + (depth > 6) + (kind == LAYER_UNDERNET || biome == BIOME_UNDERNET));
	for (int k = 0; k < rich; ++k) {
		DetourEnd end;
		bool far = detour_end(DETOUR_BLUE, taken, &end);
		NetObj *o = far ? add_obj(OBJ_MYSTERY, end.x, end.y) : PLACE(OBJ_MYSTERY);
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
	int md = npcs_for(2 + rng_range(0, 2) + size);
	for (int k = 0; k < md; ++k) {
		bool got = false;
		if (rng_range(0, 99) < 70)
			while (di < nde && !got) { x = dx[di]; y = dy[di++]; got = cell_free(x, y) && !behind_gap(x, y) && !cuts_way(x, y); }
		if (!got && n) got = room_spot(&layer.rooms[order[rng_range(0, n - 1)]], &x, &y);
		if (!got) continue;
		NetObj *o = add_obj(OBJ_MYSTERY, x, y);
		if (!o) break;
		o->param = 0;
	}
}

#undef PLACE

/* ---- Rush's gaps (issue #14) ----
 * As BN6 sets them: a walkway aimed across one to three void panels at a
 * pad of its own, the pad holding one thing (Green Area 2's HPMemory, Sky
 * Area 1's ColArmy B), where the act's plan calls for one (net_pieces.c).
 * The stand is a ground floor panel off the way with floor behind it; the
 * gap's panels and the island's 3x3 are void with void a panel round them
 * and no prop, inside the camera's window. */
typedef struct { int x, y, d, len, score; } GapSite;

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
			if (i < 0 || j < 0 || i >= MAP_W || j >= MAP_H || layer.cell[j][i] != C_VOID || prop_at_cell(i, j) || in_gap(i, j)) return false;
			if ((island || (k <= len && !s)) && !win_in(i, j)) return false;
		}
	return true;
}

static bool stand_ok(int x, int y, int d) {
	int bx = x - dir_dx[d], by = y - dir_dy[d];
	return floor_cell(x, y) && floor_cell(bx, by) && !layer.level[y][x] && !layer.level[by][bx] && !layer_on_way(x, y) &&
		!in_arena(x, y) && detour[y][x] >= 1 && cell_free(x, y) && cell_free(bx, by) && !navi_near(x, y);
}

/* The best stand for a gap `want` panels long, else shorter; the farthest
 * off the way first, a walkway's tip before a platform's edge. */
static GapSite plan_gap(int want) {
	GapSite best = { -1, -1, 0, 0, -1 };
	for (int len = want; len >= 1 && best.x < 0; --len)
		for (int y = 0; y < MAP_H; ++y)
			for (int x = 0; x < MAP_W; ++x)
				for (int d = 0; d < 4; ++d) {
					if (!stand_ok(x, y, d) || !gap_fits(x, y, d, len)) continue;
					int sx = dir_dx[(d + 1) % 4], sy = dir_dy[(d + 1) % 4];
					bool tip = !floor_cell(x + sx, y + sy) && !floor_cell(x - sx, y - sy);
					int score = detour[y][x] * 2 + (tip ? 8 : 0);
					if (score > best.score) best = (GapSite){ x, y, d, len, score };
				}
	if (best.x >= 0) {
		reserved[best.y][best.x] = 1;
		reserved[best.y - dir_dy[best.d]][best.x - dir_dx[best.d]] = 1;
		hush(best.x, best.y);
	}
	return best;
}

/* The island past the gap, its data in its middle (a tier up past one
 * panel: two or three bones' worth), and the gap itself; nothing where the
 * island would leave the map too big for the game's tile map. */
static void carve_gap(const GapSite *g, int rise) {
	int mx = g->x + dir_dx[g->d] * (g->len + 2), my = g->y + dir_dy[g->d] * (g->len + 2);
	carve_shape(SHAPE_RECT, mx - 1, my - 1, 3, 3);
	if (!fits(rise) || layer.ngaps >= MAX_GAPS) {
		for (int j = my - 1; j <= my + 1; ++j)
			for (int i = mx - 1; i <= mx + 1; ++i) layer.cell[j][i] = C_VOID;
		return;
	}
	add_room(mx - 1, my - 1, 3, 3, ROOM_PAD);
	layer.gap[layer.ngaps++] = (NetGap){ g->x, g->y, g->d, g->len, true };
	NetObj *o = add_obj(OBJ_MYSTERY, mx, my);
	if (o) { o->param = g->len >= 2 ? 2 : 1; o->prize = true; }
}

/* ---- Invisible paths (issue #46; docs/LEVEL_DESIGN.md, Set pieces) ----
 * BN6's floor drawn as void (Seaside Area 1, Sky Area 2, Underground 1,
 * Undernet 2): a walkway's tip aimed across the void at a lonely pad, the
 * floor between drawn as void, its pad holding what a player who walks on
 * finds. Its site is a Rush gap's (plan_gap), planned once the islands
 * stand; a navi near the tip says what he saw. */
#define PATH_LEN 3   /* its panels, at most */

static bool apart(int x, int y);

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
				if (!cell_free(x, y) || way_band[y][x] || layer.level[y][x] || hushed[y][x] || beside_narrow(x, y) || by_walkway(x, y) ||
					!apart(x, y) || cuts_way(x, y))
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
	if (!fits(rise)) {
		for (int j = my - 1; j <= my + 1; ++j)
			for (int i = mx - 1; i <= mx + 1; ++i) layer.cell[j][i] = C_VOID;
		return;
	}
	add_room(mx - 1, my - 1, 3, 3, ROOM_PAD);
	layer.path[layer.npaths++] = (NetGap){ g->x, g->y, g->d, g->len, true };
	NetObj *o = add_obj(OBJ_MYSTERY, mx, my);
	if (o) { o->param = 2; o->prize = true; }
	if ((o = add_obj(OBJ_NPC, hx, hy))) {
		o->param = rng_range(0, 5);
		o->npc_line = rng_range(0, 255);
		layer.hinter = layer.nobj;
	}
}

static void place_hidden(int rise) {
	GapSite site = plan_gap(PATH_LEN);
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
			if (!floor_cell(nx, ny) || dist[ny][nx] >= 0) continue;
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
	if (detour[y][x] < lo || detour[y][x] > hi || in_arena(x, y) || near_stair(x, y)) return false;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) {
			int i = x + dx, j = y + dy;
			if (!floor_cell(i, j) || layer.level[j][i] || layer_on_way(i, j) || reserved[j][i] || object_at(i, j)) return false;
		}
	return cell_free(x, y) && !near_talker(x, y);
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
					ok = layer.cell[y + j][x + i] == C_VOID && !prop_at_cell(x + i, y + j) &&
						(i < -1 || i > 2 || j < -1 || j > 2 || win_in(x + i, y + j));
			if (ok) { best = d; *ox = x; *oy = y; }
		}
	return best < (1 << 30);
}

/* The pair: a quick way back from where a long detour ends to the way,
 * else an island of its own (its data on a corner, carved last). */
static void plan_teleport(void) {
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
			for (int dx = -1; dx <= 1; ++dx) reserved[layer.teleport_y[k] + dy][layer.teleport_x[k] + dx] = 1;
}

/* The teleport's island, once the rest stands: a 4x4 with its gem a panel
 * in from its top corner, and its data on the bottom corner, straight
 * below where MegaMan lands; nothing where it would leave the map too big
 * for the game's tile map. He lands on the gem's rim below it (the warp's
 * facing 4), still beside its trigger: a step up and to either side
 * beamed him back. On the 3x3 island every corner lay that way, its data
 * reached only round the gem's rim, half over the void. */
static void carve_teleport_island(int rise) {
	int mx = layer.teleport_x[0], my = layer.teleport_y[0];
	carve_shape(SHAPE_RECT, mx - 1, my - 1, 4, 4);
	if (!fits(rise)) {
		for (int j = my - 1; j <= my + 2; ++j)
			for (int i = mx - 1; i <= mx + 2; ++i) layer.cell[j][i] = C_VOID;
		layer.nteleports = 0;
		layer.teleport_island = false;
		return;
	}
	add_room(mx - 1, my - 1, 4, 4, ROOM_PAD);
	NetObj *o = add_obj(OBJ_MYSTERY, mx + 2, my + 2);
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
			if (!floor_cell(nx, ny) || seen[ny][nx]) continue;
			seen[ny][nx] = 1;
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	int n = 0;
	memset(pocket, 0, sizeof pocket);
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!floor_cell(x, y) || seen[y][x]) continue;
			if (way_band[y][x] == 2 || layer.level[y][x] || reserved[y][x] || object_at(x, y) || near_stair(x, y) || in_arena(x, y)) return -1;
			pocket[y][x] = 1;
			++n;
		}
	return n;
}

/* Whether (x, y), entered from the floor behind it towards `d`, is a
 * walkway's first panel off a wider floor, off the way. */
static bool pocket_mouth(int x, int y, int d) {
	int sx = dir_dx[(d + 1) % 4], sy = dir_dy[(d + 1) % 4], bx = x - dir_dx[d], by = y - dir_dy[d];
	if (!floor_cell(x, y) || !floor_cell(bx, by) || way_band[y][x] == 2 || layer.level[y][x] || layer.level[by][bx]) return false;
	if (floor_cell(x + sx, y + sy) || floor_cell(x - sx, y - sy)) return false;
	return (floor_cell(bx + sx, by + sy) || floor_cell(bx - sx, by - sy)) && cell_free(bx, by) && !navi_near(x, y);
}

static void plan_obstacle(int kind) {
	int best = -1, bx = 0, by = 0, bd = 0;
	for (int y = 1; y < MAP_H - 1; ++y)
		for (int x = 1; x < MAP_W - 1; ++x)
			for (int d = 0; d < 4; ++d) {
				if (!pocket_mouth(x, y, d)) continue;
				int n = pocket_of(x, y);
				if (n < 6 || n > 40) continue;
				/* (seen from the way: its mouth near it; a pad in it) */
				int score = 20 - (detour[y][x] < 20 ? detour[y][x] : 20) + (n >= 9 && n <= 20 ? 5 : 0);
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
			if (!floor_cell(nx, ny) || dist[ny][nx] >= 0 || !pocket[ny][nx]) continue;
			dist[ny][nx] = (int16_t)(dist[y][x] + 1);
			qx[t] = (int16_t)nx; qy[t++] = (int16_t)ny;
		}
	}
	pad_middle(&rx, &ry);
	layer.block[layer.nblocks++] = (NetBlock){ bx, by, bd, kind, rx, ry };
	/* (nothing else in the pocket, its mouth or the floor before it) */
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (pocket[y][x]) reserved[y][x] = 1;
	reserved[by][bx] = 1;
	reserved[by - dir_dy[bd]][bx - dir_dx[bd]] = 1;
	hush(bx, by);
}

/* A cube's lock (layer_cube_kind): a number door asks how many braziers
 * burn on the layer, so one with fewer than two asks a P-Code instead (a
 * skull door the Net Dealer did not plan for would want a WWW-ID he never
 * stocked; issue #47). */
static int cube_kind(int depth, int biome, int kind) {
	layer.braziers = 0;
	for (int i = 0; i < layer.nprops; ++i) layer.braziers += layer.props[i].kind == PROP_SPRITE && layer.props[i].look == LOOK_BRAZIER;
	int k = layer_cube_kind(depth, biome, kind);
	return k == BLOCK_NUMBER && layer.braziers < 2 ? BLOCK_PCODE : k;
}

/* The layer's security cube asking a P-Code (issue #45), -1 none. */
static int pcode_cube(void) {
	for (int i = 0; i < layer.nblocks; ++i)
		if (layer.block[i].kind == BLOCK_PCODE) return i;
	return -1;
}

/* Whether a bystander may stand at (x, y): two panels at least from what
 * else stands there. */
static bool apart(int x, int y) {
	for (int i = 0; i < layer.nobj; ++i)
		if (abs((int)layer.obj[i].x - x) <= 2 && abs((int)layer.obj[i].y - y) <= 2) return false;
	return true;
}

static bool add_teller(int x, int y) {
	NetObj *o = add_obj(OBJ_NPC, x, y);
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
	return apart(x, y) && !hushed[y][x] && walk_between(b->x, b->y, x, y) >= TELLER_WALK;
}

/* A free floor panel off the way with floor all round it. */
static bool open_floor(int x, int y) {
	return cell_free(x, y) && !way_band[y][x] && !behind_gap(x, y) && !beside_narrow(x, y) && !by_walkway(x, y) && floor_cell(x + 1, y) &&
		floor_cell(x - 1, y) && floor_cell(x, y + 1) && floor_cell(x, y - 1) && !cuts_way(x, y);
}

/* The navi who knows the P-Code: a bystander in a room a walk from the
 * cube; else on any free floor off the way; else the cube takes a toll
 * instead (a lock no one can open would close its pocket for good). */
static void place_teller(const int *order, int n) {
	int x, y, c = pcode_cube();
	if (c < 0) return;
	const NetBlock *b = &layer.block[c];
	for (int tries = 0; tries < 16 && n; ++tries) {
		const Room *r = &layer.rooms[order[rng_range(0, n - 1)]];
		if (room_spot_in(r, &x, &y, true) && teller_ok(x, y, b) && add_teller(x, y)) return;
	}
	for (y = 1; y < MAP_H - 1; ++y)
		for (x = 1; x < MAP_W - 1; ++x)
			if (open_floor(x, y) && teller_ok(x, y, b) && add_teller(x, y)) return;
	layer.block[c].kind = BLOCK_TOLL;
}

/* The pocket's one thing, once the rest stands: a blue data of the best
 * quality (a Cross is the key; a lock never pays less than the open data). */
static void place_block_rewards(void) {
	for (int i = 0; i < layer.nblocks; ++i) {
		NetObj *o = add_obj(OBJ_MYSTERY, layer.block[i].rx, layer.block[i].ry);
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
			if (!floor_cell(nx, ny) || reach[ny][nx]) continue;
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
		if (floor_cell(cx, cy))
			return k > 1 && !layer.level[cy][cx] && cell_free(cx, cy) && !in_arena(cx, cy) && !near_stair(cx, cy) ? k - 1 : 0;
		if (k > LANE_MAX || !void_cell(cx, cy) || !void_cell(cx + sx, cy + sy) || !void_cell(cx - sx, cy - sy) || prop_at_cell(cx, cy) ||
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
	if (!reach[ey][ex] || detour[ey][ex] < 0 || detour[ey][ex] > 3) return -1;
	int saved = walk_between(x, y, ex, ey) - (*len + 1);
	return saved < LANE_SAVES ? -1 : saved * 4 - *len * 2 - detour[ey][ex];
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
			if (!reach[y][x] || detour[y][x] < LANE_FAR || layer.level[y][x] || !cell_free(x, y) || in_arena(x, y)) continue;
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
		reserved[cy][cx] = 1;
	}
	if (layer.nlanes < MAX_LANES) layer.lane[layer.nlanes++] = *l;
}

static void place_lane(const LayerKit *kit) {
	NetLane lane;
	if (plan_lane(kit, &lane)) carve_lane(&lane);
}

/* Bystander navis with a word to share, two panels at least from what else
 * stands there (one beside a Mystery Data took MegaMan's A, and each A
 * that closed his words opened them again), and never by the way on: they
 * may stay away (one beside a two-wide neck before the walkway down to an
 * arena walled it off, and a playtester never reached the guardian,
 * session 65). */
static void place_bystanders(const int *order, int n) {
	int x, y;
	int npcs = npcs_for(2 + rng_range(0, 1));
	for (int k = 0; k < npcs && n; ++k)
		for (int tries = 0; tries < 12; ++tries) {
			Room *r = &layer.rooms[order[rng_range(0, n - 1)]];
			if (!room_spot_in(r, &x, &y, true)) continue;
			if (!apart(x, y) || hushed[y][x] || layer_by_way(x, y)) continue;
			NetObj *o = add_obj(OBJ_NPC, x, y);
			if (o) { o->param = rng_range(0, 5); o->npc_line = rng_range(0, 255); }
			break;
		}
}

/* The set pieces placed once the rest stands: the islands past the void
 * (a Rush gap's, a teleport's, an invisible path's), the pockets' data,
 * an arrow lane, a P-Code's teller; and the emblems. */
static void place_last(const GapSite *gap, unsigned pieces, int rise, const LayerKit *kit, const int *order, int n) {
	if (gap->x >= 0) carve_gap(gap, rise);
	if (layer.teleport_island) carve_teleport_island(rise);
	if (pieces & PIECE_HIDDEN) place_hidden(rise);
	place_block_rewards();
	if (pieces & PIECE_ARROW) place_lane(kit);
	place_teller(order, n);
	fill_empty_detours();
	emblems(kit);
}

void layer_generate(uint32_t seed, int depth, int biome, int kind, const LayerKit *kit) {
	unsigned stair_dirs = kit ? kit->stair_dirs : 0;
	int rise = kit ? kit->rise : 0;
	memset(&layer, 0, sizeof layer);
	memset(reserved, 0, sizeof reserved);
	memset(hushed, 0, sizeof hushed);
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
	/* an act's three layers each in another of the area's layouts */
	int planned = kind == LAYER_NORMAL
		? layout_in_act(biome, run.seed ^ (uint32_t)((depth - 1) / CYCLE_LAYERS * 7 + p / 3 + 1) * 0x9E3779B9u, layer_in_act(depth))
		: layout_pick(biome);
	build_layout(planned, biome, size, rise, &arena);
	finish_rooms(seed, stair_dirs, rise);
	place_ends(kind, biome, &arena);

	Services s;
	roll_services(&s, depth, biome, kind);
	roll_gates(&s, seed, depth, kind);
	int order[MAX_ROOMS], n = room_order(order), next = 0;
	place_services(&s, depth, kind, kit, order, n, &next);
	/* the area's props, set as the originals set theirs, before the loose
	 * Mystery Data and bystanders fill the rooms: a landmark, rows and the
	 * signs (docs/LEVEL_DESIGN.md, Props) */
	if (kit && kit->looks) {
		route_distances();
		int land = landmark(kit);
		rows(kit, land, order, n);
		signs(kit, land, order, n);
	}
	if (s.duel) place_duel(order, n, &next);
	if (s.official && s.duel) place_duel_gate(s.official_level, order, n, &next);
	/* the act's set pieces (net_pieces.c): a Rush gap's stand kept clear,
	 * its island carved once the rest stands */
	GapSite gap = { -1, -1, 0, 0, -1 };
	unsigned pieces = layer_pieces(depth, biome, kind);
	measure_detours();
	if (pieces & PIECE_RUSH) {
		gap = plan_gap(layer_rush_len(depth, biome));
		npcs_held = gap.x >= 0;
	}
	if ((pieces & PIECE_TELEPORT) && kit && kit->gem) {
		plan_teleport();
		npcs_held += layer.teleport_island;   /* (the island's data, placed last) */
	}
	int block = pieces & PIECE_OBSTACLE ? layer_block_kind(depth, biome) : -1;
	if (block >= 0) plan_obstacle(block);
	if (pieces & PIECE_CUBE) plan_obstacle(cube_kind(depth, biome, kind));
	npcs_held += layer.nblocks + (pcode_cube() >= 0);   /* (the pockets' data, and a P-Code's teller, placed last) */
	npcs_held += pieces & PIECE_HIDDEN ? 2 : 0;          /* (an invisible path's data and the navi who hints at it) */
	place_data(depth, kind, biome, size, order, n, &next);
	place_bystanders(order, n);
	npcs_held = 0;
	place_last(&gap, pieces, rise, kit, order, n);
}

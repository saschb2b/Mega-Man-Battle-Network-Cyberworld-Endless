/* A layer's landmarks and props (docs/LEVEL_DESIGN.md, Props): the
 * area's emblems, a shop's counter, the landmark in its best room, the rows
 * of trees, braziers and graves, the signs, as the originals stand them. */
#include "net_landmarks.h"

#include <stdlib.h>

#include "game.h"
#include "net_gen.h"

/* ---- Props (docs/LEVEL_DESIGN.md, Props) ---- */

/* An emblem on (x, y): ground floor all round, nothing standing there. */
static void emblem_at(int x, int y) {
	if (layer.nprops >= MAX_PROPS || x < 1 || y < 1 || x >= MAP_W - 1 || y >= MAP_H - 1 || ng_object_at(x, y) || ng_near_stair(x, y)) return;
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
void ng_emblems(const LayerKit *kit) {
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
bool ng_near_stair(int x, int y) {
	for (int i = 0; i < layer.nstairs; ++i)
		if (x >= layer.stair[i].x - 1 && x < layer.stair[i].x + 3 && y >= layer.stair[i].y - 1 && y < layer.stair[i].y + 3) return true;
	return false;
}

bool ng_object_at(int x, int y) {
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
NetObj *ng_counter(int r, int type, const LayerKit *kit) {
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
					if (layer.cell[y][x] != C_PATH || layer.level[y][x] || ng_near_stair(x, y) || ng_object_at(x, y) ||
					    (x == m->ax && y == m->ay)) { ok = false; continue; }
					if (d == 2) { ok = ng_cell_free(x, y); continue; }
					xs[nb] = x; ys[nb++] = y;
				}
			if (!ok || ng_cuts(nb, xs, ys)) continue;
			/* the aisle walled off, the counter's panels floor under its own walls */
			for (int i = 0; i < nb; ++i) layer.cell[ys[i]][xs[i]] = (uint8_t)(i % 2 == 0 ? C_SOLID : C_PROPPED);
			int tm = (len - 1) / 2;
			NetObj *o = ng_add_obj(type, faces == FACES_X ? v0 : u0 + tm, faces == FACES_X ? u0 + tm : v0);
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

bool ng_prop_at_cell(int x, int y) {
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
			ok = ng_floor_cell(ex, ey) && !layer.level[ey][ex] && !ng_object_at(ex, ey) && ng_void_cell(px, py) && ng_void_cell(qx, qy);
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
		if (!ng_floor_cell(cx, cy) || layer.level[cy][cx] || ng_near_stair(cx, cy) || !ng_cell_free(cx, cy) || ng_beside_narrow(cx, cy) ||
		    anchor_at(cx, cy) || ng_far_from_way(cx, cy) < 1) return false;
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) {
				int nx = cx + dx, ny = cy + dy;
				bool in_row = dir == 0 ? (ny == cy && nx >= x && nx < x + n) : (nx == cx && ny >= y && ny < y + n);
				if (!in_row && (!ng_floor_cell(nx, ny) || layer.level[ny][nx])) return false;
			}
		xs[i] = cx; ys[i] = cy;
	}
	return !ng_cuts(n, xs, ys);
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
int ng_landmark(const LayerKit *kit) {
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
			int away = ng_far_from_way(ex, ey);
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
					if (ng_far_from_way(ax, ay) < 2 || ng_far_from_way(bx, by) < 2) break;
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
	ng_route_distances();
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
			int away = ng_far_from_way(ex, ey), score = m->w * m->h * 4 + rim.len * 8 + (away > 6 ? 6 : away) * 10;
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
		ok &= ng_far_from_way(xs[k + 1], ys[k + 1]) >= 2 && !ng_prop_at_cell(xs[k + 1], ys[k + 1]);
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
		if (ng_far_from_way(xs[k], ys[k]) < 2 || ng_prop_at_cell(xs[k], ys[k])) return false;
	}
	for (int k = 0; k < len; ++k) add_sprite(LOOK_BRAZIER, xs[k], ys[k]);
	return true;
}

void ng_rows(const LayerKit *kit, int skip, const int *order, int n) {
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
void ng_signs(const LayerKit *kit, int skip, const int *order, int n) {
	for (int i = 0; i < layer.nprops && (kit->looks & (1u << LOOK_SIGN)); ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind != PROP_COUNTER) continue;
		int cand[2][2];
		if (p->faces == FACES_X) { cand[0][0] = p->x - 2; cand[0][1] = p->y + p->len; cand[1][0] = p->x - 2; cand[1][1] = p->y - 1; }
		else { cand[0][0] = p->x + p->len; cand[0][1] = p->y - 2; cand[1][0] = p->x - 1; cand[1][1] = p->y - 2; }
		for (int k = 0; k < 2; ++k)
			if (ng_void_cell(cand[k][0], cand[k][1]) && !ng_prop_at_cell(cand[k][0], cand[k][1])) { add_sprite(LOOK_SIGN, cand[k][0], cand[k][1]); break; }
	}
	if (!(kit->looks & (1u << LOOK_BBS))) return;
	for (int i = 0; i < n; ++i) {
		const Room *m = &layer.rooms[order[i]];
		if (order[i] == skip || m->w * m->h < 12 || !room_bare(m)) continue;
		Rim rim = back_rim(m, rng_range(0, 1));
		if (rim.len < 3) continue;
		int x, y;
		past_cell(m, rim.s, rim.u0 + rim.len / 2, 1, &x, &y);
		if (ng_far_from_way(x, y) < 2 || ng_prop_at_cell(x, y)) continue;
		add_sprite(LOOK_BBS, x, y);
		return;
	}
}

/* The panel at the landmark's foot, in front of it where it can be (BN6's
 * Central Area 3 keeps its purple data beneath the statue). */
bool ng_landmark_foot(int *ox, int *oy) {
	static const int d4[4][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
	for (int i = 0; i < layer.nprops; ++i) {
		const NetProp *p = &layer.props[i];
		if (p->kind != PROP_SPRITE || (p->look != LOOK_GIANT_TREE && p->look != LOOK_STATUE && p->look != LOOK_MONUMENT)) continue;
		for (int k = 0; k < 4; ++k) {
			int x = p->x + d4[k][0], y = p->y + d4[k][1];
			if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || layer.cell[y][x] != C_PATH || layer_on_way(x, y)) continue;
			if (ng_cell_free(x, y) && !ng_behind_gap(x, y) && !ng_near_talker(x, y) && !ng_cuts_way(x, y)) { *ox = x; *oy = y; return true; }
		}
	}
	return false;
}

/* The layer's map: SELECT's over the picture and the second screen's
 * (the 3DS's bottom one, issue #9), as far as MegaMan has seen the layer. */
#include "director_map.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "boss.h"
#include "briefing_words.h"
#include "cinema.h"
#include "director.h"
#include "director_see.h"
#include "director_state.h"
#include "director_way.h"
#include "gfx.h"
#include "net_route.h"
#include "netmap.h"
#include "platform.h"
#include "save.h"

bool map_used;   /* the layer's map has been held (SELECT) since the game started */

/* ProtoMan's mark on the layer's map, and its key's */
#define RIVAL_MARK { 255, 96, 176, 255 }

/* The layer's map (SELECT's over the picture, the second screen's): its
 * frame, and a grid step there, s pixels across and s / 2 down (x - y
 * across, x + y down) */
typedef struct {
	int bx, by, bw, bh;   /* the frame, inside its edge */
	int ox, oy, cu, cv, s;   /* the frame's middle, the grid's point there, the step */
	int mx, my;   /* MegaMan's panel */
} MapView;

static int map_x(const MapView *m, int x, int y) { return m->ox + (x - y - m->cu) * m->s; }
static int map_y(const MapView *m, int x, int y) { return m->oy + (x + y - m->cv) * (m->s / 2); }

static bool map_inside(const MapView *m, int sx, int sy, int pad) {
	return sx - pad >= m->bx && sy - pad >= m->by && sx + pad < m->bx + m->bw && sy + pad < m->by + m->bh;
}

/* where the ray from MegaMan to (sx, sy) leaves the frame, pad pixels in */
static bool map_edge(const MapView *m, int sx, int sy, int pad, int *ex, int *ey) {
	double sx0 = map_x(m, m->mx, m->my), sy0 = map_y(m, m->mx, m->my);
	double dx = sx - sx0, dy = sy - sy0, t = 1e9, hx = m->bw / 2.0 - pad, hy = m->bh / 2.0 - pad;
	if (dx > 0) t = fmin(t, (m->ox + hx - sx0) / dx);
	if (dx < 0) t = fmin(t, (m->ox - hx - sx0) / dx);
	if (dy > 0) t = fmin(t, (m->oy + hy - sy0) / dy);
	if (dy < 0) t = fmin(t, (m->oy - hy - sy0) / dy);
	if (!(t > 0 && t < 1e8)) return false;
	*ex = (int)lround(sx0 + dx * t);
	*ey = (int)lround(sy0 + dy * t);
	return true;
}

/* the key's entries, under the map */
enum { MAP_YOU, MAP_EXIT, MAP_HEAL, MAP_SHOP, MAP_BOSS, MAP_EVENT, MAP_RIVAL, MAP_KEYS };
static const struct { const char *what; SDL_Color c; } map_key[MAP_KEYS] = {
	{ "You", { 255, 255, 255, 255 } }, { "Exit", { 255, 230, 60, 255 } },
	{ "Heal", { 90, 255, 120, 255 } }, { "Shop", { 255, 160, 40, 255 } }, { "Boss", { 255, 70, 70, 255 } },
	{ "Event", { 210, 110, 255, 255 } }, { "ProtoMan", RIVAL_MARK },
};

/* an object's entry in the key, -1 for none: a Server, a dark warp or a
 * gate is "Event"; the rival his own, a white eye in it (he and the
 * official gate both showed violet) */
static int map_kind(int type) {
	switch (type) {
	case OBJ_EXIT: case OBJ_RETURN: return MAP_EXIT;
	case OBJ_BOSS: return MAP_BOSS;
	case OBJ_HEAL: return MAP_HEAL;
	case OBJ_SHOP: case OBJ_PROGRAMS: case OBJ_TRADER: case OBJ_BUGTRADER: return MAP_SHOP;
	case OBJ_UNDERNET: case OBJ_SECRET_GATE: case OBJ_CHALLENGE: case OBJ_NAVI_GATE: case OBJ_VAULT: case OBJ_OFFICIAL:
		return MAP_EVENT;
	case OBJ_DUEL: return MAP_RIVAL;
	default: return -1;
	}
}

/* a mark the map leaves out: a Server's once its battle is taken (a
 * playtester saw it still there after he had won), ProtoMan's once his
 * duel is done or put off (he only talks then) */
static bool map_left_out(int type) {
	if (type != OBJ_CHALLENGE && type != OBJ_DUEL) return false;
	for (int i = 0; i < D.objs.nchoices; ++i)
		if (D.objs.choice[i].type == type && (D.chosen & (1u << i))) return true;
	return type == OBJ_DUEL && layer_objs_duel_later;
}

/* whether layer object i is a Mystery Data the map marks (seen.md_marked) */
static bool map_counted(int i) {
	for (int k = 0; k < D.objs.nmd; ++k)
		if (D.objs.md_obj[k] == i) return seen.md_marked >> k & 1;
	return false;
}

/* panel (x, y) taken into the bounds, as its u = x - y and v = x + y */
static void bounds_take(int x, int y, int *umin, int *umax, int *vmin, int *vmax) {
	int u = x - y, v = x + y;
	if (u < *umin) *umin = u;
	if (u > *umax) *umax = u;
	if (v < *vmin) *vmin = v;
	if (v > *vmax) *vmax = v;
}

/* the panels seen, and what MegaMan senses but has not reached (a ring
 * where it stands: off the frame, a playtester read its pip on the edge
 * as standing there, session 59), a lock still shut among it; the exit and
 * the guardian stay hidden */
static void map_bounds(int *umin, int *umax, int *vmin, int *vmax) {
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x)
			if (D.seen[y][x] && layer.cell[y][x] != C_VOID) bounds_take(x, y, umin, umax, vmin, vmax);   /* (a counter's panels are floor too) */
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int x = (int)o->x, y = (int)o->y;
		if (!map_counted(i) && (map_kind(o->type) < 0 || map_left_out(o->type) || o->type == OBJ_EXIT || o->type == OBJ_RETURN ||
		    o->type == OBJ_BOSS))
			continue;
		if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || D.seen[y][x]) continue;
		bounds_take(x, y, umin, umax, vmin, vmax);
	}
	for (int k = 0; k < layer.nblocks; ++k)
		if (seen.locks >> k & 1 && !D.seen[layer.block[k].y][layer.block[k].x]) bounds_take(layer.block[k].x, layer.block[k].y, umin, umax, vmin, vmax);
}

/* the step as large as lets the floor seen so far fit (8 on the second
 * screen's larger frame, 6 over the picture, 4 at the least), on the
 * seen floor's middle, or on MegaMan when it fits at none */
static void map_view(MapView *m) {
	int umin = m->mx - m->my, umax = umin, vmin = m->mx + m->my, vmax = vmin;
	map_bounds(&umin, &umax, &vmin, &vmax);
	int s = m->bw >= 300 ? 8 : 6;
	while (s > 4 && ((umax - umin) * s + 2 * s + 2 > m->bw || (vmax - vmin) * (s / 2) + s + 2 > m->bh)) s -= 2;
	m->s = s;
	m->ox = m->bx + m->bw / 2;
	m->oy = m->by + m->bh / 2;
	m->cu = (umin + umax) / 2;
	m->cv = (vmin + vmax) / 2;
	if (s == 4 && ((umax - umin) * 4 + 10 > m->bw || (vmax - vmin) * 2 + 6 > m->bh)) {
		m->cu = m->mx - m->my;
		m->cv = m->mx + m->my;
	}
}

/* the panels seen: each a diamond 2s - 1 wide and s - 1 high (7 by 3, 11
 * by 5, 15 by 7), a pixel apart from the next; their rows drawn a batch
 * per colour, raised floor lighter (thousands of rows, a call each had
 * cost the 3DS's processor) */
static void map_panels(const MapView *m) {
	static const SDL_Color colour[2] = { { 60, 140, 230, 240 }, { 150, 210, 255, 240 } };
	static SDL_Rect rows[2][512];
	int n[2] = { 0, 0 }, half = m->s / 2 - 1;
	for (int y = 0; y < MAP_H; ++y)
		for (int x = 0; x < MAP_W; ++x) {
			if (!D.seen[y][x] || layer.cell[y][x] == C_VOID) continue;   /* (a counter's panels are floor too) */
			int sx = map_x(m, x, y), sy = map_y(m, x, y), k = layer.level[y][x] != 0;
			if (!map_inside(m, sx, sy, 3)) continue;
			for (int r = -half; r <= half; ++r) {
				if (n[k] == 512) { fill_rects(rows[k], n[k], colour[k]); n[k] = 0; }
				rows[k][n[k]++] = (SDL_Rect){ sx - (m->s - 1 - 2 * abs(r)), sy + r, 2 * m->s - 1 - 4 * abs(r), 1 };
			}
		}
	for (int k = 0; k < 2; ++k) fill_rects(rows[k], n[k], colour[k]);
}

/* the way on, over the panels he has come near, up to the first he
 * hasn't: marks he earned (a V in a comp's maze read as a dead end, the
 * arm on to the exit nowhere on the map); in straight runs as far as a
 * straight line over the floor goes (the walk's steps zig-zagged across a
 * platform) */
static void map_way(const MapView *m, int px, int py) {
	int tx = D.objs.exit_x, ty = D.objs.exit_y, ex, ey, len;
	SDL_Color tc = rgba(255, 230, 60, 200);
	if (D.objs.guardian.navi && !boss_beaten()) { tx = D.objs.guardian.x; ty = D.objs.guardian.y; tc = rgba(255, 110, 90, 200); }
	double wx, wy;
	netmap_grid(px, py, &wx, &wy);
	route_floor();
	if (!netmap_panel(tx, ty, &ex, &ey) || route_way(wx, wy, ex, ey, &len) < 0) return;
	int cx = m->mx, cy = m->my, k = route_walk_len - 1;
	while (k >= 0) {
		int x = route_walk[k] % MAP_W, y = route_walk[k] / MAP_W, far = k;
		if (!D.seen[y][x]) break;
		for (int j = k - 1; j >= 0 && j >= k - 12; --j) {
			int jx = route_walk[j] % MAP_W, jy = route_walk[j] / MAP_W;
			if (!D.seen[jy][jx]) break;
			if (route_floor_line(cx, cy, jx, jy)) far = j;
		}
		int fx = route_walk[far] % MAP_W, fy = route_walk[far] / MAP_W, steps = abs(fx - cx) + abs(fy - cy);
		int ax = map_x(m, cx, cy), ay = map_y(m, cx, cy), bx = map_x(m, fx, fy), by = map_y(m, fx, fy);
		for (int t = 1; t <= steps; ++t) {
			int lx = ax + (bx - ax) * t / steps, ly = ay + (by - ay) * t / steps;
			if (map_inside(m, lx, ly, 3)) fill_rect(lx - (m->s - 1) / 2, ly, m->s - 1, 1, tc);
		}
		cx = fx; cy = fy;
		k = far - 1;
	}
}

/* An arrowhead at (ex, ey) on the frame's edge, pointing out of it: what
 * stands that way, past the frame (a dot there read as standing at the
 * frame's edge, session 59) */
static void map_arrowhead(const MapView *m, int ex, int ey, int size, SDL_Color c) {
	int dl = ex - m->bx, dr = m->bx + m->bw - 1 - ex, dt = ey - m->by, db = m->by + m->bh - 1 - ey;
	int least = dl < dr ? dl : dr, vert = dt < db ? dt : db, h = size / 2;
	for (int k = 0; k < size; ++k) {
		if (vert <= least) fill_rect(ex - k, dt < db ? ey - h + k : ey + h - k, 2 * k + 1, 1, c);
		else fill_rect(dl < dr ? ex - h + k : ex + h - k, ey - k, 1, 2 * k + 1, c);
	}
}

/* one thing standing there, once seen; before, a ring where it stands if
 * MegaMan senses it, or a pip on the frame's edge its way (L named the
 * Recovery Mr. Prog, and it was nowhere on the map): the exit and the
 * guardian neither */
static void map_mark(const MapView *m, int type, int x, int y, SDL_Color c) {
	int sx = map_x(m, x, y), sy = map_y(m, x, y), ex, ey;
	if (!D.seen[y][x]) {
		if (type == OBJ_EXIT || type == OBJ_RETURN || type == OBJ_BOSS) return;
		if (map_inside(m, sx, sy, 3)) {
			fill_rect(sx - 3, sy - 3, 7, 7, c);
			fill_rect(sx - 2, sy - 2, 5, 5, rgba(0, 8, 28, 255));
		} else if (map_edge(m, sx, sy, 3, &ex, &ey))
			map_arrowhead(m, ex, ey, 3, c);
		return;
	}
	if (!map_inside(m, sx, sy, 3)) return;
	fill_rect(sx - 3, sy - 3, 7, 7, rgba(0, 8, 28, 255));
	fill_rect(sx - 2, sy - 2, 5, 5, c);
	if (type == OBJ_DUEL) fill_rect(sx, sy, 1, 1, rgba(255, 255, 255, 255));
}

/* what stands there, and the goal: the guardian while it stands, else
 * the exit */
static void map_marks(const MapView *m, int *gx, int *gy, SDL_Color *gc) {
	bool goal_boss = false;
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		int x = (int)o->x, y = (int)o->y, kind = map_kind(o->type);
		if (kind < 0 || x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || map_left_out(o->type)) continue;
		if (o->type == OBJ_BOSS && !boss_beaten()) { *gx = x; *gy = y; *gc = map_key[kind].c; goal_boss = true; }
		else if (o->type == OBJ_EXIT && !goal_boss) { *gx = x; *gy = y; }
		map_mark(m, o->type, x, y, map_key[kind].c);
	}
}

/* the layer's locks still shut, in its walkways' mouths, as the gates are
 * marked: an Event, a ring while MegaMan only senses one (L names each
 * among what he senses); gone once open. A playtester heard L sense
 * Mystery Data "behind the security cube" and a navi tell its P-Code, and
 * found the cube nowhere on the map (session 64). */
static void map_locks(const MapView *m) {
	for (int k = 0; k < layer.nblocks; ++k)
		if (seen.locks >> k & 1) map_mark(m, -1 /* (no object's type) */, layer.block[k].x, layer.block[k].y, map_key[MAP_EVENT].c);
	/* (and the flame of darkness while it burns, as L names it) */
	if (flame_here()) map_mark(m, -1, (int)layer.obj[D.objs.dark_flame_obj].x, (int)layer.obj[D.objs.dark_flame_obj].y, map_key[MAP_EVENT].c);
}

/* the goal while it is unseen: where the ray from MegaMan to it leaves
 * the frame */
static void map_goal(const MapView *m, int gx, int gy, SDL_Color gc) {
	int ax, ay;
	if (gx < 0 || D.seen[gy][gx] || !map_edge(m, map_x(m, gx, gy), map_y(m, gx, gy), 5, &ax, &ay)) return;
	map_arrowhead(m, ax, ay, 5, gc);
}

/* the key, from (kx, ky): MegaMan, the exit, and what else this layer
 * holds; the gaps close up until it fits the width, then "You" goes,
 * whose mark pulses (ProtoMan's name ran off the picture) */
static void draw_map_key(int kx, int ky, int width) {
	bool has[MAP_KEYS] = { [MAP_YOU] = true, [MAP_EXIT] = true, [MAP_BOSS] = D.objs.guardian.navi != 0, [MAP_EVENT] = seen.locks != 0 };
	for (int i = 0; i < layer.nobj; ++i) {
		int kind = map_kind(layer.obj[i].type);
		if (kind > MAP_EXIT && kind != MAP_BOSS && !map_left_out(layer.obj[i].type)) has[kind] = true;
	}
	int gap = 10, need;
	for (;;) {
		need = 0;
		for (int i = 0; i < MAP_KEYS; ++i) if (has[i]) need += 8 + text_width(map_key[i].what) + gap;
		need -= gap;
		if (need <= width || (gap <= 0 && !has[MAP_YOU])) break;
		if (gap > 2 || !has[MAP_YOU]) gap -= 2;
		else { has[MAP_YOU] = false; gap = 8; }
	}
	if (gap < 0) gap = 0;
	for (int i = 0; i < MAP_KEYS; ++i) {
		if (!has[i]) continue;
		fill_rect(kx, ky + 3, 5, 5, map_key[i].c);
		if (i == MAP_RIVAL) fill_rect(kx + 2, ky + 5, 1, 1, rgba(255, 255, 255, 255));
		text_draw(kx + 7, ky, map_key[i].what, rgba(200, 225, 255, 255), TEXT_LEFT);
		kx += 8 + text_width(map_key[i].what) + gap;
	}
}

/* A Mystery Data crystal, 7 pixels across, its upper left face lit; dark
 * once all of its colour are taken; at alpha a */
static void crystal(int x, int y, SDL_Color c, bool done, int a) {
	static const int half[7] = { 0, 1, 2, 3, 2, 1, 0 };
	SDL_Color base = done ? rgba(c.r / 3, c.g / 3, c.b / 3, (Uint8)a) : rgba(c.r, c.g, c.b, (Uint8)a);
	SDL_Color lit = done ? base : rgba(c.r + (255 - c.r) / 2, c.g + (255 - c.g) / 2, c.b + (255 - c.b) / 2, (Uint8)a);
	for (int r = 0; r < 7; ++r) {
		fill_rect(x + 3 - half[r], y + r, 2 * half[r] + 1, 1, base);
		if (r > 0 && r < 4) fill_rect(x + 3 - half[r], y + r, half[r], 1, lit);
	}
}

/* The layer's Mystery Data counters from (x, y): a crystal of each colour
 * MegaMan knows of and "taken/known" beside it, dim once all are taken
 * (what is left draws the eye), at alpha a; their width, drawn only where
 * a > 0. */
static int md_counts(int x, int y, int a) {
	static const SDL_Color tone[3] = { { 70, 230, 110, 255 }, { 80, 170, 255, 255 }, { 200, 110, 255, 255 } };
	int w = 0;
	for (int c = 0; c < 3; ++c) {
		if (!seen.md_known[c]) continue;
		char t[12];
		snprintf(t, sizeof t, "%d/%d", seen.md_taken[c], seen.md_known[c]);
		bool done = seen.md_taken[c] >= seen.md_known[c];
		if (w) w += 7;
		if (a > 0) {
			crystal(x + w, y + 3, tone[c], done, a);
			text_draw(x + w + 9, y, t, done ? rgba(100, 120, 150, (Uint8)a) : rgba(200, 225, 255, (Uint8)a), TEXT_LEFT);
		}
		w += 9 + text_width(t);
	}
	return w;
}

/* The Mystery Data the map shows, untaken: a small crystal of its colour
 * where it stands (the counters had counted data a playtester had seen
 * near him but not on the screen, and he had "no idea where", session 61;
 * what the header counts as seen, the map now shows). Data only sensed
 * behind a set piece stays unmarked: L names those, and the map marks the
 * lock before them (map_locks), not what it keeps. */
static void map_mystery(const MapView *m) {
	/* (the header's tones, the blue lighter: on the map's blue floor it faded) */
	static const SDL_Color tone[3] = { { 70, 230, 110, 255 }, { 160, 220, 255, 255 }, { 200, 110, 255, 255 } };
	for (int k = 0; k < D.objs.nmd; ++k) {
		if (!(seen.md_marked >> k & 1)) continue;
		const NetObj *o = &layer.obj[D.objs.md_obj[k]];
		int sx = map_x(m, (int)o->x, (int)o->y), sy = map_y(m, (int)o->x, (int)o->y), ex, ey;
		SDL_Color c = tone[D.objs.md_colour[k] == MYSTERY_GREEN ? 0 : D.objs.md_colour[k] == MYSTERY_BLUE ? 1 : 2];
		if (!map_inside(m, sx, sy, 3)) {
			/* (past the frame: an arrowhead its way, as a sensed service's) */
			if (map_edge(m, sx, sy, 3, &ex, &ey)) map_arrowhead(m, ex, ey, 3, c);
			continue;
		}
		for (int r = -3; r <= 3; ++r) fill_rect(sx - (3 - abs(r)), sy + r, 2 * (3 - abs(r)) + 1, 1, rgba(0, 8, 28, 255));
		for (int r = -2; r <= 2; ++r) fill_rect(sx - (2 - abs(r)), sy + r, 2 * (2 - abs(r)) + 1, 1, c);
	}
}

/* L's overlay of the counters, through its words and a few seconds after
 * (seen.counts_a): in the corner note's box, the HP's opposite, and under
 * the note while one shows ("Run saved" stands there as a layer begins,
 * when L is pressed first) */
void director_draw_counts(void) {
	int a = seen.counts_a, w = a ? md_counts(0, 0, 0) + 8 : 0;
	if (!w) return;
	int x = P.core_x + 240 - w - 3, y = P.core_y + 3 + cinema_note_height();
	fill_rect(x, y, w, TEXT_H + 4, rgba(0, 16, 40, (Uint8)(170 * a / 255)));
	fill_rect(x, y + TEXT_H + 3, w, 1, rgba(120, 248, 255, (Uint8)(200 * a / 255)));
	md_counts(x + 4, y + 2, a);
}

/* The layer's map in w x h from (x0, y0): SELECT's over the picture, and
 * the second screen's (the 3DS's bottom one, issue #9). */
static void draw_map(int x0, int y0, int w, int h) {
	fill_rect(x0, y0, w, h, rgba(0, 8, 28, 255));
	MapView m = { .bx = x0 + 6, .by = y0 + 18, .bw = w - 12, .bh = h - 38 };
	SDL_Color edge = rgba(120, 200, 255, 220);
	fill_rect(m.bx - 2, m.by - 2, m.bw + 4, 1, edge);
	fill_rect(m.bx - 2, m.by + m.bh + 1, m.bw + 4, 1, edge);
	fill_rect(m.bx - 2, m.by - 2, 1, m.bh + 4, edge);
	fill_rect(m.bx + m.bw + 1, m.by - 2, 1, m.bh + 4, edge);
	text_drawf(m.bx, y0 + 3, rgba(170, 220, 255, 255), TEXT_LEFT, "Layer %d", run.depth);
	md_counts(m.bx + m.bw - md_counts(0, 0, 0), y0 + 3, 255);
	int px = seen.px, py = seen.py;
	if (!netmap_panel(px, py, &m.mx, &m.my)) return;
	map_view(&m);
	map_panels(&m);
	map_way(&m, px, py);
	int gx = -1, gy = -1;
	SDL_Color gc = map_key[MAP_EXIT].c;
	map_mystery(&m);
	map_marks(&m, &gx, &gy, &gc);
	map_locks(&m);
	map_goal(&m, gx, gy, gc);
	/* MegaMan, always there, his border pulsing */
	int ms = map_x(&m, m.mx, m.my), mt = map_y(&m, m.mx, m.my);
	if (map_inside(&m, ms, mt, 3)) {
		fill_rect(ms - 3, mt - 3, 7, 7, (D.frame / 10) % 2 ? rgba(120, 200, 255, 255) : rgba(0, 8, 28, 255));
		fill_rect(ms - 2, mt - 2, 5, 5, rgba(255, 255, 255, 255));
	}
	draw_map_key(m.bx + 1, m.by + m.bh + 5, m.bw - 2);
}

/* SELECT held on a layer: the layer as far as MegaMan has seen it, over the
 * dimmed game, the exit pad, the arena, the services and the ways off it
 * marked. Panels stand apart, so a walkway reads as a line and a room as a
 * block; the whole seen floor is fitted in when it fits, else the map
 * follows MegaMan. The goal, until seen, is a mark on the frame the way it
 * lies. */
void director_draw_map(void) {
	if (!D.active || D.town || !D.map_shown || !seen.on_map) return;
	draw_map(P.core_x, P.core_y, 240, 160);
}

bool director_draw_second_screen(int w, int h) {
	/* (on the net, in battle too: the town and the title keep it dark) */
	if (!D.active || D.town) return false;
	draw_map(0, 0, w, h);
	return true;
}

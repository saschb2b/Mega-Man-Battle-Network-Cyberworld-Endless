/* The "by" dev step (docs/devtools/scenarios.md, What stands on a layer):
 * MegaMan put beside something on the layer, facing it, as a player walks
 * up to it, so a test's A is the player's A at the thing itself and no
 * test holds a coordinate; and the way he faces, for the pad's AHEAD. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6_fields.h"
#include "director.h"
#include "director_dev.h"
#include "director_state.h"
#include "emu.h"
#include "net_shapes.h"
#include "netmap.h"
#include "scripts.h"

/* The "by NAME" step's panel: the layer's first object of a --talk's kind
 * (a trader of either kind as --talk tells them), its exit or return pad,
 * a Mystery Data by its colour (green, blue, purple) or by the key item
 * it holds (scrtdata, regup), a number after a kind or a colour for a
 * later one ("npc2", "blue3"), or a set piece's (block: a lock;
 * teller: the Navi who knows its P-Code; hinter: the one who hints at an
 * invisible path; teleport: a pair's first pad; gap, lane, path: the
 * floor a Rush gap, an arrow lane or an invisible path leaves from);
 * false for none */
static int by_along;   /* the way a piece leaves the panel (DIR_*), -1 none */

/* ... an object of the layer's: the `nth` of `name`'s kind or colour */
static bool by_object(const char *name, int nth, int *x, int *y) {
	int type = !strcmp(name, "exit") ? OBJ_EXIT : !strcmp(name, "return") ? OBJ_RETURN : director_dev_kind(name);
	for (int i = 0; i < layer.nobj; ++i) {
		const NetObj *o = &layer.obj[i];
		const char *colour = o->param == 1 || o->param == 2 ? "blue" : o->param ? "purple" : "green";
		if ((o->type == OBJ_MYSTERY ? strcmp(name, colour) != 0 : o->type != type) || --nth > 0) continue;
		*x = (int)o->x; *y = (int)o->y;
		return true;
	}
	return false;
}

/* ... the Mystery Data that holds a ScrtData, or the layer's RegUp: a key item's, kind 4 */
static bool by_find(const char *name, int *x, int *y) {
	for (int k = 0; k < D.objs.nmd; ++k) {
		unsigned item = D.objs.md_find[k] >> 16 == 4 ? D.objs.md_find[k] & 0xFFFFu : 0;
		bool regup = item >= SCRIPTS_REG_UP1 && item <= SCRIPTS_REG_UP1 + 2;
		if (!item || strcmp(name, item == SCRIPTS_SECRET_DATA ? "scrtdata" : regup ? "regup" : "") != 0) continue;
		*x = (int)layer.obj[D.objs.md_obj[k]].x; *y = (int)layer.obj[D.objs.md_obj[k]].y;
		return true;
	}
	return false;
}

/* ... a set piece's: a lock, its teller or a path's hinter, a teleport's
 * pad, the floor a gap, a lane or a path leaves from (MegaMan facing along it) */
static bool by_piece(const char *name, int *x, int *y) {
	int who = !strcmp(name, "teller") ? layer.teller : !strcmp(name, "hinter") ? layer.hinter : 0;
	if (who) { *x = (int)layer.obj[who - 1].x; *y = (int)layer.obj[who - 1].y; return true; }
	if (!strcmp(name, "block") && layer.nblocks) { *x = layer.block[0].x; *y = layer.block[0].y; return true; }
	if (!strcmp(name, "teleport") && layer.nteleports) { *x = layer.teleport_x[0]; *y = layer.teleport_y[0]; return true; }
	const NetGap *g = !strcmp(name, "gap") && layer.ngaps ? &layer.gap[0] : !strcmp(name, "path") && layer.npaths ? &layer.path[0] : NULL;
	if (g) { *x = g->x; *y = g->y; by_along = g->dir; return true; }
	if (!strcmp(name, "lane") && layer.nlanes) { *x = layer.lane[0].x; *y = layer.lane[0].y; by_along = layer.lane[0].dir; return true; }
	return false;
}

static bool by_panel(const char *word, int *x, int *y) {
	/* ("npc2": the layer's second) */
	char name[16];
	snprintf(name, sizeof name, "%s", word);
	size_t len = strcspn(name, "0123456789");
	int nth = name[len] ? atoi(name + len) : 1;
	name[len] = 0;
	by_along = -1;
	return by_object(name, nth, x, y) || by_find(name, x, y) || by_piece(name, x, y);
}

/* whether panel (x, y) is floor no object stands on */
static bool by_free(int x, int y) {
	if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || layer.cell[y][x] != C_PATH) return false;
	for (int i = 0; i < layer.nobj; ++i)
		if ((int)layer.obj[i].x == x && (int)layer.obj[i].y == y) return false;
	return true;
}

/* The floor MegaMan walks to from where he stands, past no lock: a lock's
 * near side, not its pocket's */
static uint8_t by_reach[MAP_H][MAP_W];

static void by_walk(void) {
	static int16_t queue[MAP_W * MAP_H];
	int n = 0, cx, cy;
	memset(by_reach, 0, sizeof by_reach);
	if (!netmap_panel(bn6_player_x(), bn6_player_y(), &cx, &cy)) return;
	for (int k = 0; k < layer.nblocks; ++k) by_reach[layer.block[k].y][layer.block[k].x] = 2;
	by_reach[cy][cx] = 1;
	queue[n++] = (int16_t)(cy * MAP_W + cx);
	for (int i = 0; i < n; ++i)
		for (int k = 0; k < 4; ++k) {
			int x = queue[i] % MAP_W + dir_dx[k], y = queue[i] / MAP_W + dir_dy[k];
			if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H || layer.cell[y][x] != C_PATH || by_reach[y][x]) continue;
			by_reach[y][x] = 1;
			queue[n++] = (int16_t)(y * MAP_W + x);
		}
}

/* whether MegaMan talks to panel (x, y) from the panel d steps off it on
 * side k: free floor of its height, the panels between free floor too (or,
 * d of 2, a keeper's counter); `walked`: floor he walks to alone */
static bool by_side(int x, int y, int k, int d, bool walked) {
	int px = x + dir_dx[k] * d, py = y + dir_dy[k] * d, mx = x + dir_dx[k], my = y + dir_dy[k];
	if (!by_free(px, py) || layer.level[py][px] != layer.level[y][x] || (walked && by_reach[py][px] != 1)) return false;
	for (int j = 1; j < d; ++j)
		if (!by_free(x + dir_dx[k] * j, y + dir_dy[k] * j) && (d != 2 || layer.cell[my][mx] != C_PROPPED)) return false;
	return true;
}

int director_dev_facing(void) { return emu_read8(BN6_PLAYER_FACING) & 7; }

bool director_dev_by(const char *name) {
	int x, y, wx, wy;
	if (!director_on_map()) return false;
	/* (a pad is stepped on: MegaMan on it; the rest are talked to from the
	 * nearest floor beside them he walks to, else any: a prize's island) */
	static const char *const pads[] = { "exit", "return", "teleport", "gap", "lane", "path" };
	bool pad = false;
	for (unsigned i = 0; i < sizeof pads / sizeof *pads; ++i) pad |= !strcmp(name, pads[i]);
	/* ("exitside": two panels off the pad, clear of its zone, not on it) */
	char of[16];
	snprintf(of, sizeof of, "%s", name);
	size_t len = strlen(of);
	bool side = len > 4 && !strcmp(of + len - 4, "side");
	if (side) { of[len - 4] = 0; name = of; }
	if (!by_panel(name, &x, &y)) { printf("by: no %s on this layer\n", name); return false; }
	by_walk();
	for (int pass = pad ? 4 : 0; pass < 5; ++pass)
		for (int k = 0; k < 4; ++k) {
			int d = pass == 4 ? 0 : 1 + pass % 2 + side;
			if (d && !by_side(x, y, k, d, pass < 2)) continue;
			/* (facing it: the grid's +x lies down-right on the screen, +y down-left) */
			static const int face[4] = { 7, 1, 3, 5 };
			netmap_world(x + dir_dx[k] * d, y + dir_dy[k] * d, &wx, &wy);
			director_dev_place(wx, wy, d ? face[k] : by_along >= 0 ? face[(by_along + 2) % 4] : -1);
			/* (on a raised room's floor, at its height) */
			emu_write32(BN6_PLAYER_Z, (uint32_t)(layer.level[y][x] ? netmap_rise() : 0) << 16);
			return true;
		}
	printf("by: no free floor by the layer's %s\n", name);
	return false;
}

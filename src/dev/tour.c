/* The tour (docs/DEVTOOLS.md): the game itself shows each area's layer,
 * room by room, without walking. For each area a layer is built and
 * entered, MegaMan is warped onto every room in turn, and once the map has
 * settled the frame is saved. Random battles are off and MegaMan cannot be
 * deleted meanwhile. */
#include "tour.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "devtools.h"
#include "emu.h"
#include "town.h"
#include "gamecall.h"
#include "director.h"
#include "game.h"
#include "net.h"
#include "platform.h"
#include "run.h"

#define SETTLE_LAYER 150   /* frames after entering a layer */
#define SETTLE_ROOM   50   /* frames after a warp */

static struct {
	bool on;
	bool world;            /* the real world's maps instead of the layers */
	bool town;             /* the run's town */
	int group, number, spot;
	int spots[6][2];
	int nspots;
	char dir[400];
	bool want[BIOME_COUNT];
	int biome, room, t;
	bool entered;
} T;

bool tour_parse(const char *spec) {
	char biomes[256] = "all";
	snprintf(T.dir, sizeof T.dir, ".build/tour");
	sscanf(spec, "%399[^:]:%255s", T.dir, biomes);
	if (!strcmp(biomes, "town")) {
		T.town = true;
		T.on = true;
		T.spot = 0;
		emu_start_in_town = true;   /* the run starts there */
		dev.quiet = dev.god = true;
		return true;
	}
	if (!strcmp(biomes, "world")) {
		T.world = true;
		T.on = true;
		T.group = 0;
		T.number = -1;
		dev.quiet = dev.god = true;
		return true;
	}
	if (!strcmp(biomes, "all")) for (int b = 0; b < BIOME_COUNT; ++b) T.want[b] = true;
	else for (char *t = strtok(biomes, ","); t; t = strtok(NULL, ",")) { int b = atoi(t); if (b >= 0 && b < BIOME_COUNT) T.want[b] = true; }
	T.on = true;
	T.biome = -1;
	dev.quiet = dev.god = true;
	return true;
}

bool tour_on(void) { return T.on; }

/* A floor panel of room r nothing stands on (not the exit pad, which would
 * take MegaMan on), nearest its middle. */
static void room_cell(int r, int *cx, int *cy) {
	const Room *m = &layer.rooms[r];
	int best = 1 << 30;
	*cx = m->ax; *cy = m->ay;
	for (int y = m->y; y < m->y + m->h; ++y)
		for (int x = m->x; x < m->x + m->w; ++x) {
			if (layer.cell[y][x] != C_PATH) continue;
			bool taken = false;
			for (int i = 0; i < layer.nobj && !taken; ++i) {
				int dx = (int)layer.obj[i].x - x, dy = (int)layer.obj[i].y - y;
				taken = dx * dx + dy * dy <= 2;
			}
			int d = abs(x - m->ax) + abs(y - m->ay);
			if (!taken && d < best) { best = d; *cx = x; *cy = y; }
		}
}

static void warp_room(int r) {
	int x, y;
	room_cell(r, &x, &y);
	director_dev_warp_cell(x, y);
}

/* The next area to show, entered; false when all are done. */
static bool next_area(void) {
	do ++T.biome; while (T.biome < BIOME_COUNT && !T.want[T.biome]);
	if (T.biome >= BIOME_COUNT) return false;
	director_debug_biome = T.biome;
	run.depth = 2;
	run.side_kind = LAYER_NORMAL;
	T.room = -1;
	T.t = 0;
	T.entered = director_start_layer();
	return true;
}

/* ---- the real world: each map, Lan warped to a few places on its floor ---- */

static const int world_maps[7] = { 2, 5, 11, 5, 5, 4, 5 };

/* Up to six floor cells spread over the map: nearest its middle and the
 * middles of its quarters and of its far corner. */
static void world_spots(int group, int number) {
	T.nspots = 0;
	AreaSrc a;
	if (!area_src_load(group, number, &a)) return;
	if (!a.rings) { area_src_free(&a); return; }
	int x0 = a.rx0 * 8, y0 = a.ry0 * 8, x1 = (a.rx0 + a.rw) * 8, y1 = (a.ry0 + a.rh) * 8;
	int mx = (x0 + x1) / 2, my = (y0 + y1) / 2, qx = (x1 - x0) / 4, qy = (y1 - y0) / 4;
	int target[6][2] = { { mx, my }, { mx - qx, my - qy }, { mx + qx, my - qy }, { mx - qx, my + qy }, { mx + qx, my + qy }, { mx, my + qy * 2 - 16 } };
	for (int t = 0; t < 6; ++t) {
		int best = 1 << 30, bx = 0, by = 0;
		for (int y = y0 + 4; y < y1; y += 8)
			for (int x = x0 + 4; x < x1; x += 8) {
				if (area_src_walled_floor(&a, x, y) != 1) continue;
				/* not on a wall's cell or next to one */
				bool near_wall = false;
				for (int i = 0; i < a.nsec[0] && !near_wall; ++i)
					near_wall = abs(a.sec[0][i].x + 4 - x) <= 12 && abs(a.sec[0][i].y + 4 - y) <= 12;
				if (near_wall) continue;
				int d = abs(x - target[t][0]) + abs(y - target[t][1]);
				if (d < best) { best = d; bx = x; by = y; }
			}
		if (best < (1 << 30)) { T.spots[T.nspots][0] = bx; T.spots[T.nspots][1] = by; ++T.nspots; }
	}
	area_src_free(&a);
}

static bool world_next_map(void) {
	for (;;) {
		if (++T.number >= world_maps[T.group]) { T.number = 0; if (++T.group >= 7) return false; }
		world_spots(T.group, T.number);
		if (T.nspots) break;
	}
	T.spot = 0;
	T.t = 0;
	emu_warp(T.group, T.number, T.spots[0][0], T.spots[0][1], 4);
	return true;
}

static void world_update(void) {
	if (T.number < 0) {
		director_stop();
		if (!world_next_map()) { T.on = false; P.quit = true; }
		return;
	}
	if (++T.t < 90) return;
	snprintf(devtools_shot, sizeof devtools_shot, "%s/world_%02x_%d_%d.bmp", T.dir, T.group, T.number, T.spot);
	if (++T.spot < T.nspots) {
		emu_warp(T.group, T.number, T.spots[T.spot][0], T.spots[T.spot][1], 4);
		T.t = 0;
		return;
	}
	if (!world_next_map()) { T.on = false; P.quit = true; }
}

/* ---- the town: Lan at its start, its port, and around its square ---- */

static void town_spot(int i, int *x, int *y) {
	const TownInfo *ti = town_info();
	int spots[6][2] = {
		{ ti->start_x, ti->start_y }, { ti->port_x, ti->port_y + 24 },
		{ ti->port_x - 120, ti->port_y }, { ti->port_x - 200, ti->port_y + 40 },
		{ ti->start_x + 80, ti->start_y - 80 }, { ti->port_x - 40, ti->port_y + 120 },
	};
	*x = spots[i][0];
	*y = spots[i][1];
}

static void town_update(void) {
	if (!director_in_town()) { T.t = 0; return; }
	if (++T.t < 120) return;
	snprintf(devtools_shot, sizeof devtools_shot, "%s/town_%d.bmp", T.dir, T.spot);
	if (++T.spot >= 6) { T.on = false; P.quit = true; return; }
	int x, y;
	town_spot(T.spot, &x, &y);
	emu_warp(TOWN_GROUP, TOWN_NUMBER, x, y, 2);
	T.t = 0;
}

void tour_update(void) {
	if (!T.on) return;
	if (T.town) { town_update(); return; }
	if (T.world) { world_update(); return; }
	if (T.biome < 0 && !next_area()) { P.quit = true; return; }
	if (!director_on_map()) { T.t = 0; return; }   /* while a map loads */
	++T.t;
	if (T.room < 0) {
		if (T.t < SETTLE_LAYER) return;
		T.room = 0;
		warp_room(0);
		T.t = 0;
		return;
	}
	if (T.t < SETTLE_ROOM) return;
	snprintf(devtools_shot, sizeof devtools_shot, "%s/tour_b%02d_r%02d.bmp", T.dir, T.biome, T.room);
	if (++T.room < layer.nrooms) {
		warp_room(T.room);
		T.t = 0;
		return;
	}
	if (!next_area()) T.on = false, P.quit = true;
}

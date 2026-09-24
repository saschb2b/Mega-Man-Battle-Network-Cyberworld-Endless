/* The tour (docs/DEVTOOLS.md): the game itself shows each area's layer,
 * room by room, without walking. For each area a layer is built and
 * entered, MegaMan is warped onto every room in turn, and once the map has
 * settled the frame is saved. Random battles are off and MegaMan cannot be
 * deleted meanwhile. */
#include "tour.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devtools.h"
#include "director.h"
#include "game.h"
#include "net.h"
#include "platform.h"
#include "run.h"

#define SETTLE_LAYER 150   /* frames after entering a layer */
#define SETTLE_ROOM   50   /* frames after a warp */

static struct {
	bool on;
	char dir[400];
	bool want[BIOME_COUNT];
	int biome, room, t;
	bool entered;
} T;

bool tour_parse(const char *spec) {
	char biomes[256] = "all";
	snprintf(T.dir, sizeof T.dir, ".build/tour");
	sscanf(spec, "%399[^:]:%255s", T.dir, biomes);
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

void tour_update(void) {
	if (!T.on) return;
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

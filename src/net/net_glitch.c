/* The run's Glitch layer in the generator (net_glitch.h;
 * docs/game/glitch-layer.md): its warp on the layer that holds it, its copy
 * at the Glitch layer's heart, and its data pulled from deeper. */
#include "net_glitch.h"

#include <stdlib.h>
#include <string.h>

#include "net_detours.h"
#include "net_gen.h"

/* The run's Glitch warp (docs/game/glitch-layer.md): in view of the way, a
 * rare thing a player who keeps to the way still passes, never by the
 * arrival: the first panels of a spur first, then a room the way crosses,
 * then farther off */
#define GLITCH_FROM_ARRIVAL 6

void ng_place_glitch(const int *order, int n) {
	int bx = -1, by = -1, best = 1 << 30, x, y;
	const Room *arrival = &layer.rooms[0];
	for (int i = 0; i < n; ++i)
		for (int tries = 0; tries < 6; ++tries) {
			if (!ng_room_spot_in(&layer.rooms[order[i]], &x, &y, false) || ng_near_talker(x, y)) continue;
			int d = ng_detour[y][x];
			if (d < 0 || abs(x - arrival->ax) + abs(y - arrival->ay) < GLITCH_FROM_ARRIVAL) continue;
			int score = d >= 1 && d <= 4 ? d : d == 0 ? 6 : 10 + d;
			if (score < best) { best = score; bx = x; by = y; }
		}
	if (bx >= 0) ng_add_obj(OBJ_GLITCH, bx, by);
	else if (n) ng_place(OBJ_GLITCH, order, n, 0, &x, &y);
}

/* A Glitch layer's copy at its heart: the farthest end of its detours, or
 * the next room where it has none so long */
void ng_place_copy(const int *order, int n, int *next) {
	static uint8_t taken[MAP_W * MAP_H];
	memset(taken, 0, sizeof taken);
	DetourEnd end;
	int x, y;
	if (ng_detour_end(DETOUR_BLUE, taken, &end) && ng_add_obj(OBJ_COPY, end.x, end.y)) return;
	ng_place(OBJ_COPY, order, n, *next, &x, &y);
	++*next;
}

/* A Glitch layer's data pulled from deeper in the Net, by how far off the
 * way each lies: an act deeper on a detour, two acts at its far end or
 * past a set piece's lock, and the farthest of them two acts deeper on
 * any layout */
void ng_glitch_deep(void) {
	int far = -1, far_d = 0;
	for (int i = 0; i < layer.nobj; ++i) {
		NetObj *o = &layer.obj[i];
		if (o->type != OBJ_MYSTERY) continue;
		int d = ng_detour[(int)o->y][(int)o->x];
		o->deep = d == 0 ? 0 : d > 0 && d < DETOUR_FAR ? 3 : 6;
		if (d > far_d) { far_d = d; far = i; }
	}
	if (far >= 0) layer.obj[far].deep = 6;
}

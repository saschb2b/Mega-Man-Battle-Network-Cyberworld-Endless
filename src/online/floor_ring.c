/* floor_ring.h */
#include "floor_ring.h"

#include <math.h>

#include "emu.h"
#include "gfx.h"
#include "map_label.h"
#include "platform.h"

#define POINTS 720     /* the most a ring is drawn with (a crew's radius on screen is some 360 by 180) */
#define BODY 7         /* half a MegaMan's width at his feet: the ring's back left out there */

void floor_ring(int x, int y, int z, int r, const int camera[3], const FloorRing *how) {
	static SDL_Rect dot[POINTS], edge[POINTS];
	int cx, cy, n = 0, steps = r * 6;
	if (steps > POINTS) steps = POINTS;
	if (steps < 16) steps = 16;
	map_label_floor(x, y, z, camera, &cx, &cy);
	int lastx = 1 << 20, lasty = 0;
	for (int i = 0; i < steps; ++i) {
		double t = 6.283185307179586 * i / steps;
		int dx = (int)lround(r * cos(t)), dy = (int)lround(r * sin(t));
		/* (BN6's projection: map_label_at's, a world unit across a pixel across and half a row down) */
		int ox = dx + dy, oy = (dy - dx) / 2;
		if (ox == lastx && oy == lasty) continue;
		lastx = ox, lasty = oy;
		if (how->dotted && (i / 3) & 1) continue;
		if (how->around && oy < 0 && ox > -BODY && ox < BODY) continue;
		int px = cx + ox, py = cy + oy;
		if (px < 0 || px + 1 >= EMU_W || py < 1 || py + 2 >= how->bottom) continue;
		/* (two pixels wide, a dark edge above and under: read on a floor of its own colour too) */
		edge[n] = (SDL_Rect){ P.core_x + px - 1, P.core_y + py - 1, 4, 3 };
		dot[n++] = (SDL_Rect){ P.core_x + px, P.core_y + py, 2, 1 };
	}
	fill_rects(edge, n, rgba(0, 24, 41, 230));
	fill_rects(dot, n, how->colour);
}

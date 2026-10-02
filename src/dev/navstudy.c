/* The way across BN6's own net maps, counted as a layer's is (docs/
 * LEVEL_DESIGN.md, Navigation; docs/DEVTOOLS.md): each area's original
 * maps, the floor their walls give a panel at a time, and the one-wide
 * walkways the walk between their two farthest panels crosses
 * (grid_way_narrows, net_way.c). */
#include "navstudy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "net.h"
#include "rom.h"

#define SPAN 96   /* panels each way from the map's origin */
#define PANEL 32

/* Map (group, number)'s floor, a cell a panel: where its walls put the
 * panel's middle on floor. */
static bool map_floor(int group, int number, uint8_t *cells) {
	AreaSrc a;
	if (!area_src_load(group, number, &a)) return false;
	memset(cells, 0, (size_t)(2 * SPAN) * (2 * SPAN));
	for (int B = -SPAN; B < SPAN; ++B)
		for (int A = -SPAN; A < SPAN; ++A)
			cells[(B + SPAN) * 2 * SPAN + A + SPAN] = area_src_walled_floor(&a, a.ex + PANEL / 2 + PANEL * A, a.ey + PANEL / 2 + PANEL * B) == 1;
	area_src_free(&a);
	return true;
}

int navstudy_run(void) {
	static uint8_t cells[(2 * SPAN) * (2 * SPAN)];
	for (int b = 0; b < NET_AREAS; ++b) {
		const NetAreaDef *d = &R.layout->net_area[b];
		if (d->xrom || !d->nmaps) continue;
		int maps = 0, narrows = 0, links = 0, len = 0;
		printf("area %2d (maps %02X:%d-%d):", b, d->battles, d->first, d->first + d->nmaps - 1);
		for (int m = d->first; m < d->first + d->nmaps; ++m) {
			if (!map_floor(d->battles, m, cells)) continue;
			int big, way, n = grid_way_narrows(cells, 2 * SPAN, 2 * SPAN, &big, &way);
			printf(" %d/%d (%d)", n, big, way);
			narrows += n; links += big; len += way; ++maps;
		}
		if (maps) printf("  ->  %.2f one-wide walkways across, %.2f between big platforms, a way of %d panels\n",
			(double)narrows / maps, (double)links / maps, len / maps);
		else printf("  none read\n");
	}
	return 0;
}

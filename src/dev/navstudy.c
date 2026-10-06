/* BN6's own net maps measured for the layers' navigation (docs/
 * LEVEL_DESIGN.md; docs/DEVTOOLS.md): the way across, counted as a layer's
 * is (each area's original maps, the floor their walls give a panel at a
 * time, and the one-wide walkways the walk between their two farthest
 * panels crosses: grid_way_narrows, net_way.c; Navigation), and where
 * MegaMan arrives on them (every jack-in and every warp from another map,
 * the side of the floor on the screen it lies at and the way it faces him;
 * Arrivals, issue #106). */
#include "navstudy.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "emu.h"
#include "mapslot.h"
#include "net.h"
#include "rom.h"

#define SPAN 96   /* panels each way from the map's origin */
#define PANEL 32

/* Map (group, number)'s floor, a cell a panel: where its walls put the
 * panel's middle on floor; panel (A, B)'s middle at world (ex + 16 + 32A,
 * ey + 16 + 32B), (ex, ey) given where asked. */
static bool map_floor_at(int group, int number, uint8_t *cells, int *ex, int *ey) {
	AreaSrc a;
	if (!area_src_load(group, number, &a)) return false;
	memset(cells, 0, (size_t)(2 * SPAN) * (2 * SPAN));
	for (int B = -SPAN; B < SPAN; ++B)
		for (int A = -SPAN; A < SPAN; ++A)
			cells[(B + SPAN) * 2 * SPAN + A + SPAN] = area_src_walled_floor(&a, a.ex + PANEL / 2 + PANEL * A, a.ey + PANEL / 2 + PANEL * B) == 1;
	if (ex) *ex = a.ex;
	if (ey) *ey = a.ey;
	area_src_free(&a);
	return true;
}

static bool map_floor(int group, int number, uint8_t *cells) { return map_floor_at(group, number, cells, NULL, NULL); }

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

/* ---- Where MegaMan arrives (docs/LEVEL_DESIGN.md, Arrivals; issue #106) ---- */

/* BN6's eight facings, UP first (the player's facing probes' order, docs/
 * ROM_DATA.md, Talking reach), as steps in u = A + B (across the screen,
 * 16 pixels a unit) and v = B - A (down it, 8 pixels): the diagonals are
 * the world's axes. */
static const char *const face_names[8] = { "up", "up-right", "right", "down-right", "down", "down-left", "left", "up-left" };
static const int face_u[8] = { 0, 1, 2, 1, 0, -1, -2, -1 }, face_v[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };
static const char *const side_names[5] = { "top", "bottom", "left", "right", "middle" };

/* An arrival onto a net map: from a jack-in (from < 0: its record) or a
 * warp of map (from_group, from_number); one a place and facing, however
 * many warps lead there (the doghouse comps' nine to one spot). */
typedef struct { int group, number, x, y, facing, from_group, from_number, record, ways; } Arrival;
#define MAX_ARRIVALS 400
static Arrival arrivals[MAX_ARRIVALS];
static int narrivals;

static void add_arrival(Arrival a) {
	for (int i = 0; i < narrivals; ++i) {
		Arrival *b = &arrivals[i];
		if (b->group == a.group && b->number == a.number && b->x == a.x && b->y == a.y && b->facing == a.facing) { ++b->ways; return; }
	}
	if (narrivals < MAX_ARRIVALS) { a.ways = 1; arrivals[narrivals++] = a; }
}

static bool net_group(int g) { return g >= 0x80 && g <= 0x96; }

/* Warp entry `e` (WarpData: group, number, departure, facing, x, y) of map
 * (g, n)'s list, as an arrival onto another net map: not a teleport within
 * one map (departure 0x0C) nor a jack-out to where Lan jacked in (0x10),
 * nor bytes past the list's end. */
static bool warp_arrival(uint32_t e, int g, int n, Arrival *a) {
	int dg = emu_read8(e), dn = emu_read8(e + 1), dep = emu_read8(e + 2), face = emu_read8(e + 3);
	int x = (int32_t)emu_read32(e + 4) >> 16, y = (int32_t)emu_read32(e + 8) >> 16;
	if (!net_group(dg) || dn > 23 || face > 7 || dep == 0x0C || dep == 0x10 || dep > 0x18 || (dep & 3) || abs(x) > 4096 || abs(y) > 4096) return false;
	*a = (Arrival){ dg, dn, x, y, face, g, n, -1, 0 };
	return dg != g || dn != n;
}

/* Every warp of every map (the real world's groups and the internet's)
 * onto a net map, each list read once. */
static void gather_warps(void) {
	static uint32_t seen[512];
	int nseen = 0;
	for (int g = 0; g <= 0x96; ++g) {
		if (g > 6 && g < 0x80) continue;
		for (int n = 0; n < 24; ++n) {
			uint32_t list = mapslot_warps(g, n);
			bool dup = list < 0x08000000u || list >= 0x0A000000u || (list & 3);
			for (int k = 0; k < nseen && !dup; ++k) dup = seen[k] == list;
			if (dup || nseen >= 512) continue;
			seen[nseen++] = list;
			Arrival a;
			for (int i = 0; i < 16; ++i)
				if (warp_arrival(list + 16u * (uint32_t)i, g, n, &a)) add_arrival(a);
		}
	}
	for (int i = 0; i < 43; ++i) {
		int dg, dn, x, y, face;
		if (mapslot_jack_original(i, &dg, &dn, &x, &y, &face) && net_group(dg) && face <= 7) add_arrival((Arrival){ dg, dn, x, y, face, -1, -1, i, 0 });
	}
}

/* Which side of the floor's box on the screen (u across, v down) point
 * (u, v) lies nearest: its edge within a quarter of the box, else the
 * middle; and whether facing f points into the box, towards its middle. */
static int box_side(double u, double v, const double box[4], int f, bool *into) {
	double fu = (u - box[0]) / (box[1] - box[0] + 1e-9), fv = (v - box[2]) / (box[3] - box[2] + 1e-9);
	double d[4] = { fv, 1 - fv, fu, 1 - fu };
	int side = 0;
	for (int k = 1; k < 4; ++k) if (d[k] < d[side]) side = k;
	double cu = (box[0] + box[1]) / 2 - u, cv = (box[2] + box[3]) / 2 - v;
	/* (in pixels: a unit of u 16 across, of v 8 down) */
	*into = 4 * face_u[f] * cu + face_v[f] * cv > 0;
	return d[side] > 0.25 ? 4 : side;
}

/* The box of map's floor on the screen: u = A + B, v = B - A over its panels. */
static void floor_box(const uint8_t *cells, double box[4]) {
	box[0] = box[2] = 1e9;
	box[1] = box[3] = -1e9;
	for (int B = -SPAN; B < SPAN; ++B)
		for (int A = -SPAN; A < SPAN; ++A) {
			if (!cells[(B + SPAN) * 2 * SPAN + A + SPAN]) continue;
			double u = A + B, v = B - A;
			box[0] = fmin(box[0], u); box[1] = fmax(box[1], u);
			box[2] = fmin(box[2], v); box[3] = fmax(box[3], v);
		}
}

/* Map (group, number)'s arrivals printed, its sides and facings counted. */
static void map_arrivals(int group, int number, int sides[5], int faces[8], int *into_n, int *total) {
	static uint8_t cells[(2 * SPAN) * (2 * SPAN)];
	int ex, ey;
	if (!map_floor_at(group, number, cells, &ex, &ey)) return;
	double box[4];
	floor_box(cells, box);
	for (int i = 0; i < narrivals; ++i) {
		const Arrival *a = &arrivals[i];
		if (a->group != group || a->number != number) continue;
		double A = (a->x - ex - PANEL / 2) / (double)PANEL, B = (a->y - ey - PANEL / 2) / (double)PANEL;
		bool into;
		int side = box_side(A + B, B - A, box, a->facing, &into);
		++sides[side]; ++faces[a->facing]; *into_n += into; ++*total;
		if (a->record >= 0) printf("    %02X:%d jack-in %2d at %5d %5d: %-6s facing %-10s %s\n", group, number, a->record, a->x, a->y,
			side_names[side], face_names[a->facing], into ? "into the map" : "out of it");
		else printf("    %02X:%d from %02X:%-2d   at %5d %5d: %-6s facing %-10s %s (%d warps)\n", group, number, a->from_group, a->from_number, a->x, a->y,
			side_names[side], face_names[a->facing], into ? "into the map" : "out of it", a->ways);
	}
}

static void print_counts(const char *what, const int sides[5], const int faces[8], int into_n, int total) {
	printf("  %s: %d arrivals; sides", what, total);
	for (int k = 0; k < 5; ++k) printf(" %s %d", side_names[k], sides[k]);
	printf("; facing");
	for (int f = 0; f < 8; ++f) printf(" %s %d", face_names[f], faces[f]);
	printf("; into the map %d\n", into_n);
}

int navstudy_arrivals(void) {
	if (!emu_init(R.data, ROM_SIZE)) { fprintf(stderr, "arrivals: no core\n"); return 1; }
	gather_warps();
	int all_sides[5] = { 0 }, all_faces[8] = { 0 }, all_into = 0, all = 0;
	for (int b = 0; b < NET_AREAS; ++b) {
		const NetAreaDef *d = &R.layout->net_area[b];
		if (d->xrom || !d->nmaps) continue;
		int sides[5] = { 0 }, faces[8] = { 0 }, into_n = 0, total = 0;
		printf("area %2d (maps %02X:%d-%d):\n", b, d->battles, d->first, d->first + d->nmaps - 1);
		for (int m = d->first; m < d->first + d->nmaps; ++m) map_arrivals(d->battles, m, sides, faces, &into_n, &total);
		print_counts("area", sides, faces, into_n, total);
	}
	/* (the net areas' own maps, groups 0x90-0x96, each once: the Secret
	 * Area shares the Undernet's; the comps and homepages apart) */
	printf("the net areas' maps:\n");
	for (int g = 0x90; g <= 0x96; ++g)
		for (int m = 0; m < 6; ++m) {
			bool any = false;
			for (int i = 0; i < narrivals && !any; ++i) any = arrivals[i].group == g && arrivals[i].number == m;
			if (any) map_arrivals(g, m, all_sides, all_faces, &all_into, &all);
		}
	print_counts("groups 90-96", all_sides, all_faces, all_into, all);
	return 0;
}

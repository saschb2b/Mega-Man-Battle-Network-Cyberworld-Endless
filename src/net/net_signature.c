/* A layer's signature room (net_signature.h; docs/LEVEL_DESIGN.md,
 * Identity): which one a layer holds, and its shape carved. */
#include "net_signature.h"

#include "game.h"
#include "net_shapes.h"
#include "run.h"

const char *const sig_names[SIG_COUNT] = { "none", "crater", "plaza", "ring", "field", "lagoon", "grove", "slab", "court", "cross" };

/* Each area's pool, after what its own maps hold, and how often each comes:
 * Central's crater (Central Area 3) and plaza (Central Area 1's field);
 * Seaside's great fields, one round a pool of the void (Seaside Area 2's
 * round its water); Sky's ring road (Sky Area 3) and its pods grown to a
 * plaza (Sky Area 1); Green's giant cybertree (Green Area 2) and plain grass
 * field (Green Area 1); the Graveyard's great slab with its monument, and a
 * slab round a hole (its loops are holes punched in its slabs); the
 * Undernet's statue court and a great plus; the Nest's plus round the
 * Underground's raised block, and its slabs; the comps and homepages, whose
 * originals are single rooms, a plaza or a plus (Mr. Weather's one plain
 * slab, its field as before: a hole or a plus in it its art draws in
 * pieces). */
#define POOL 3
static const struct { uint8_t sig[POOL], weight[POOL]; } pools[BIOME_COUNT] = {
	[BIOME_CENTRAL] = { { SIG_CRATER, SIG_PLAZA }, { 50, 50 } },
	[BIOME_SEASIDE] = { { SIG_FIELD, SIG_LAGOON }, { 50, 50 } },
	[BIOME_SKY] = { { SIG_RING, SIG_PLAZA }, { 50, 50 } },
	[BIOME_GREEN] = { { SIG_GROVE, SIG_PLAZA }, { 60, 40 } },
	[BIOME_GRAVEYARD] = { { SIG_SLAB, SIG_RING }, { 60, 40 } },
	[BIOME_UNDERNET] = { { SIG_COURT, SIG_CROSS }, { 60, 40 } },
	[BIOME_SECRET] = { { SIG_COURT }, { 100 } },
	[BIOME_NEST] = { { SIG_CROSS, SIG_SLAB }, { 50, 50 } },
	[BIOME_COMP] = { { SIG_PLAZA, SIG_CROSS }, { 50, 50 } },
	[BIOME_HOMEPAGE] = { { SIG_PLAZA, SIG_CROSS }, { 50, 50 } },
	[BIOME_COMP_B] = { { SIG_PLAZA, SIG_CROSS }, { 50, 50 } },
	[BIOME_ROBOT_COMP] = { { SIG_PLAZA, SIG_CROSS }, { 60, 40 } },
	[BIOME_AQUARIUM_COMP] = { { SIG_PLAZA }, { 100 } },
	[BIOME_JUDGE_COMP] = { { SIG_PLAZA, SIG_CROSS }, { 50, 50 } },
	[BIOME_WEATHER_COMP] = { { SIG_PLAZA }, { 100 } },
	[BIOME_COPYBOT_COMP] = { { SIG_PLAZA }, { 100 } },
	[BIOME_ACDC_HP] = { { SIG_PLAZA, SIG_CROSS }, { 50, 50 } },
	[BIOME_GREEN_HP] = { { SIG_PLAZA, SIG_CROSS }, { 50, 50 } },
	[BIOME_SKY_HP] = { { SIG_PLAZA, SIG_CROSS }, { 50, 50 } },
};

static int area(int biome) { return biome >= 0 && biome < BIOME_COUNT ? biome : BIOME_CENTRAL; }

int sig_pool_size(int biome) {
	int n = 0;
	while (n < POOL && pools[area(biome)].sig[n]) ++n;
	return n;
}

/* (as layout_in_act draws an act's layouts: a weighted draw without
 * replacement, on numbers of its own) */
int sig_in_act(int biome, uint32_t act_seed, int index) {
	int b = area(biome), n = sig_pool_size(b), left[POOL], total = 0, order[POOL], k = 0;
	for (int i = 0; i < n; ++i) { left[i] = pools[b].weight[i]; total += left[i]; }
	uint32_t r = (act_seed ^ 0x5161A7u) | 1;
	while (total > 0) {
		r = r * 1103515245u + 12345u;
		int roll = (int)((r >> 16) % (uint32_t)total), sum = 0;
		for (int i = 0; i < n; ++i) {
			sum += left[i];
			if (left[i] && roll < sum) { order[k++] = pools[b].sig[i]; total -= left[i]; left[i] = 0; break; }
		}
	}
	return k ? order[index % k] : SIG_NONE;
}

int sig_for_seed(int biome, uint32_t seed) {
	int b = area(biome), n = sig_pool_size(b), total = 0;
	for (int i = 0; i < n; ++i) total += pools[b].weight[i];
	if (!total) return SIG_NONE;
	uint32_t h = (seed ^ 0x5161A7u) * 2654435761u;
	int roll = (int)((h >> 16) % (uint32_t)total);
	for (int i = 0; i < n; ++i) {
		if (roll < pools[b].weight[i]) return pools[b].sig[i];
		roll -= pools[b].weight[i];
	}
	return SIG_NONE;
}

/* A plaza in the area's own platforms' shape: Sky's pods and the Undernet's
 * plateaus cut at the corners, the comps' rooms and Central's fields
 * square, Green's grass holed as Green Area 1's fields are, the Judge Tree's
 * ragged; the Aquarium's a glass pool the size of its pads, its water never
 * widening into a field. */
static int plaza_shape(int biome) {
	switch (biome) {
	case BIOME_SKY: case BIOME_UNDERNET: case BIOME_SECRET: case BIOME_HOMEPAGE: case BIOME_ACDC_HP: case BIOME_GREEN_HP: case BIOME_SKY_HP:
		return SHAPE_OCTAGON;
	case BIOME_GREEN: return SHAPE_HOLED;
	case BIOME_JUDGE_COMP: return SHAPE_RAGGED;
	default: return SHAPE_RECT;
	}
}

void sig_box(int sig, int biome, int size, int *w, int *h) {
	int s;
	switch (sig) {
	case SIG_CRATER: s = 9 + size; break;
	case SIG_PLAZA: s = biome == BIOME_AQUARIUM_COMP ? 6 : biome == BIOME_WEATHER_COMP ? 9 + size : 8 + size; break;
	case SIG_RING: s = 11; break;
	case SIG_FIELD: case SIG_LAGOON: s = 10 + size; break;
	case SIG_CROSS: s = 11 + (size > 1) * 2; break;
	default: s = 9 + size; break;   /* the grove, the slab, the court */
	}
	*w = *h = s;
}

/* A field round a pit, its rim three panels wide where the box allows, else
 * two (Central Area 3's crater). */
static void carve_crater(int x, int y, int w, int h) {
	int rim = w >= 9 && h >= 9 ? 3 : 2;
	carve_shape(SHAPE_RECT, x, y, w, h);
	carve_void(x + rim, y + rim, w - 2 * rim, h - 2 * rim);
}

/* A ring road two panels wide round a sunken middle, and where the middle
 * has room, a 3x3 pad in it on a bridge from one side (its DIR_* from the
 * pad, returned; -1 for none): the pad a room of its own. */
static int carve_ring(int x, int y, int w, int h) {
	carve_shape(SHAPE_RECT, x, y, w, h);
	carve_void(x + 2, y + 2, w - 4, h - 4);
	if (w < 11 || h < 11) return -1;
	int mx = x + w / 2, my = y + h / 2, d = rng_range(0, 3), reach = d == DIR_E || d == DIR_W ? w / 2 : h / 2;
	carve_shape(SHAPE_RECT, mx - 1, my - 1, 3, 3);
	for (int k = 2; k < reach - 1; ++k) put(mx + dir_dx[d] * k, my + dir_dy[d] * k);
	return d;
}

/* A ragged field with a pool of the void in it, off its middle, three panels
 * in from its edges at least (the field round it two panels wide). */
static void carve_lagoon(int x, int y, int w, int h) {
	carve_shape(SHAPE_RAGGED, x, y, w, h);
	if (w < 9 || h < 9) return;
	bool wide = rng_range(0, 1);
	int pw = wide ? 3 : 2, ph = wide ? 2 : 3;
	carve_void(x + 3 + rng_range(0, w - 6 - pw), y + 3 + rng_range(0, h - 6 - ph), pw, ph);
}

int sig_carve(int sig, int biome, int x, int y, int w, int h) {
	int bridge = -1;
	switch (sig) {
	case SIG_CRATER: carve_crater(x, y, w, h); break;
	case SIG_PLAZA: carve_shape(plaza_shape(biome), x, y, w, h); break;
	case SIG_RING: bridge = carve_ring(x, y, w, h); break;
	case SIG_FIELD: carve_shape(SHAPE_RAGGED, x, y, w, h); break;
	case SIG_LAGOON: carve_lagoon(x, y, w, h); break;
	case SIG_SLAB: carve_shape(SHAPE_HOLED, x, y, w, h); break;
	/* (square, so a side holds the statue and its braziers whole, five
	 * panels: cut at the corners, a court of seven kept three) */
	case SIG_COURT: carve_shape(SHAPE_RECT, x, y, w, h); break;
	case SIG_CROSS: carve_shape(SHAPE_PLUS, x, y, w, h); break;
	case SIG_GROVE: carve_shape(SHAPE_RECT, x, y, w, h); break;   /* (its tree stands at the back: net_landmarks.c) */
	default: return -1;
	}
	int r = add_room(x, y, w, h, ROOM_FIELD);
	if (r < 0) return -1;
	layer.sig_room = r;
	if (sig == SIG_RING) {
		/* (the ring stands on its road, the side across from its pad's
		 * bridge, not on the pad in its middle; the pad a room of its own,
		 * a detour's end) */
		int d = bridge >= 0 ? bridge : DIR_E;
		layer.rooms[r].ax = x + w / 2 - dir_dx[d] * (w / 2);
		layer.rooms[r].ay = y + h / 2 - dir_dy[d] * (h / 2);
		if (bridge >= 0) add_room(x + w / 2 - 1, y + h / 2 - 1, 3, 3, ROOM_PAD);
	}
	return r;
}

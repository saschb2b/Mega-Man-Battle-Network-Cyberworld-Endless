/* An area's original maps learned for its layers (netmap.c): their view,
 * their counters, decorations and arrows, kept per area. */
#include "netmap_learn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "panels.h"

/* ---- helpers ---- */

int nm_floordiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }

/* Where map a's tiles fall in its panels (world units mod 32, of tile 0's
 * centre). */
static void lattice(const AreaSrc *a, int *fx, int *fy) {
	int u = 4 - a->tw * 4, v = 4 - a->th * 4;
	*fx = ((u - 2 * v) / 2 - a->ex) & 31;
	*fy = ((u + 2 * v) / 2 - a->ey) & 31;
}

/* Whether map b's tiles lie where map a's do in their panels: then a tile
 * of b drawn in a layer made on a's grid shows as it did in b. Tiles step
 * by (4, 4) and (-8, 8) world units. */
static bool aligned(const AreaSrc *a, const AreaSrc *b) {
	int ax, ay, bx, by;
	lattice(a, &ax, &ay);
	lattice(b, &bx, &by);
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 4; ++j)
			if (((ax + 4 * i - 8 * j - bx) & 31) == 0 && ((ay + 4 * i + 8 * j - by) & 31) == 0) return true;
	return false;
}

/* A map of the area's own game, BN6's or another's (docs/multirom/). */
/* How deep under a pad's lower edges its stamp is cut: its map's side faces
 * as its first book measured them. */
static int pad_deep(const Learned *L) {
	int deep = L->nbooks ? L->book[0].face : 0;
	return deep > 12 ? deep : 12;
}

static bool load_map(const NetAreaDef *na, int group, int number, AreaSrc *a) {
	return area_src_load_as(na->xrom, group, number, na->recolour, a);
}

/* Learns the tiles of map `src` and its mirror image, where their tiles
 * line up with the layer's grid (`grid_of`, src itself or another's: its
 * fields alone are read). The mirror is src turned in place and back
 * (area_src_mirror_in): a copy of it took the New 3DS's heap past its size
 * (docs/engine/emulation/3ds-memory.md). */
static void learn_view(AreaSrc *src, const AreaSrc *grid_of, int area, bool own, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	/* (its own map's pads alone, TILES_PADS_ALONE: its other maps lend all) */
	uint32_t styles = (own ? na->styles : na->styles & ~TILES_PADS_ALONE) | (netmap_whole_forced || (na->lay | netmap_rules_forced) & (LAY_WHOLE | LAY_PADS_WHOLE) ? TILES_WHOLE : 0);
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, src)) {
		tiles_learn(src, styles, na->walk_styles, na->skip_styles, na->joint_hues, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, src, na->bg_in_map);
		breaks_add(&L->breaks, src);
	}
	AreaSrc m;
	if (!area_src_mirror_in(src, &m)) return;
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, &m)) {
		tiles_learn(&m, styles, na->walk_styles, na->skip_styles, na->joint_hues, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, &m, na->bg_in_map);
		breaks_add(&L->breaks, &m);
	}
	area_src_mirror_end(src, &m);
}

/* ... at each of its floor heights: raised floors are drawn higher, so each
 * is learned in a view that brings it down to the ground (made in place,
 * as the mirror is). */
static void learn_map(AreaSrc *src, const AreaSrc *grid_of, int area, int map, Learned *L) {
	bool own = !map;
	int count[256] = { 0 }, first = L->nbooks;
	for (int i = 0; src->hz && i < src->hw * src->hh; ++i) count[src->hz[i]]++;
	learn_view(src, grid_of, area, own, L);
	for (int z = 8; z < HEIGHT_UNEVEN; z += 8) {
		if (count[z] < NETMAP_LEVEL_MIN_CELLS) continue;
		AreaSrc r;
		if (!area_src_raise_in(src, z, &r)) continue;
		learn_view(&r, grid_of, area, own, L);
		area_src_raise_end(src, &r);
	}
	/* (its books are one map's: the layer's lead, netmap.c) */
	for (int k = first; k < L->nbooks; ++k) L->book_map[k] = (uint8_t)map;
}

/* ---- a map's walkways by their look ---- */

#define HUES 13   /* hue buckets 0-11 and grey (tl_style_at) */

/* Where panel (A, B)'s middle at height z is drawn in map a; false off its
 * picture or where nothing is drawn there. */
static bool panel_px(const AreaSrc *a, int A, int B, int z, int *px, int *py) {
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B;
	*px = area_px(a->tw, X, Y);
	*py = area_py(a->th, X, Y) - z;
	return *px >= 0 && *py >= 0 && *px < a->tw * 8 && *py < a->th * 8 && a->px[(size_t)*py * a->tw * 8 + *px] >> 24;
}

/* Whether panel (A, B) of map a is floor at height z with something drawn
 * at its middle. */
static bool floor_at(const AreaSrc *a, int A, int B, int z) {
	int X = a->ex + 16 + 32 * A, Y = a->ey + 16 + 32 * B, px, py;
	return panel_px(a, A, B, z, &px, &py) && area_src_walled_floor(a, X, Y) == 1 && area_src_height(a, X, Y) == z;
}

/* ... and a walkway's: in no 2 x 2 block of such floor. Its hue bucket, -1
 * where it is none. */
static int walk_hue(const AreaSrc *a, int A, int B, int z) {
	int px, py;
	if (!floor_at(a, A, B, z)) return -1;
	for (int db = -1; db <= 0; ++db)
		for (int da = -1; da <= 0; ++da)
			if (floor_at(a, A + da, B + db, z) && floor_at(a, A + da + 1, B + db, z) && floor_at(a, A + da, B + db + 1, z) && floor_at(a, A + da + 1, B + db + 1, z)) return -1;
	panel_px(a, A, B, z, &px, &py);
	return tl_style_at(a, px, py);
}

/* Map a's walkways at height z by their hue: per bucket, how many panels
 * show it and its longest straight run. */
static void walk_hues(const AreaSrc *a, int z, int count[HUES], int run[HUES]) {
	memset(count, 0, HUES * sizeof *count);
	memset(run, 0, HUES * sizeof *run);
	for (int B = -64; B < 64; ++B)
		for (int A = -64; A < 64; ++A) {
			int h = walk_hue(a, A, B, z);
			if (h < 0) continue;
			count[h]++;
			for (int axis = 0; axis < 2; ++axis) {
				int da = !axis, db = axis, len = 1;
				if (walk_hue(a, A - da, B - db, z) == h) continue;   /* (a run is measured from its first panel) */
				while (walk_hue(a, A + len * da, B + len * db, z) == h) ++len;
				if (len > run[h]) run[h] = len;
			}
		}
}

/* The hues of map a's second look of walkway on the ground, its roads; 0
 * none. Its walkways' own hues are the area's (walk_styles), or where it
 * names none the hue most of its maps' walkways show (`more`: the other
 * maps' counts; BN4's Town Area 3 is mostly road, its walkways Town Area
 * 1's). A road is another hue, none of its platforms' and none next to its
 * walkways' (a shade of the same look: a dark area's blue beside its
 * cyan), that a dozen panels of walkway show, five of them in one straight
 * run: BN4's Park Area's orange (30 panels, a run of 11), its ACDC Area's
 * cyan (15, 8), its Town Area's yellow (20, 5). A joint's or a pattern's
 * hue (BN4's Undernet's steel joints: 42 panels, never 3 in a row; the
 * Undernet's bridges of three hues) is none. */
#define ROAD_PANELS 12
#define ROAD_RUN 5
static bool hues_apart(int h, unsigned own) {
	if (h == 12) return true;
	return !(own >> ((h + 1) % 12) & 1) && !(own >> ((h + 11) % 12) & 1);
}

static uint16_t road_hues(const AreaSrc *a, const NetAreaDef *na, const int more[HUES]) {
	int count[HUES], run[HUES], most = 0;
	walk_hues(a, 0, count, run);
	for (int h = 1; h < HUES; ++h) if (count[h] + more[h] > count[most] + more[most]) most = h;
	unsigned own = na->walk_styles & 0x1FFFu ? na->walk_styles & 0x1FFFu : 1u << most, roads = 0;
	for (int h = 0; h < HUES; ++h)
		if (!((own | na->styles) >> h & 1) && hues_apart(h, own) && count[h] >= ROAD_PANELS && run[h] >= ROAD_RUN) roads |= 1u << h;
	return (uint16_t)roads;
}

/* The walkways of the area's other maps by their hue, added up: each map
 * loaded and let go, before its own is. */
static void more_hues(const NetAreaDef *na, int total[HUES]) {
	memset(total, 0, HUES * sizeof *total);
	for (int k = 0; k < NET_MORE_MAPS && na->more[k][0]; ++k) {
		AreaSrc b;
		int count[HUES], run[HUES];
		if (!load_map(na, na->more[k][0], na->more[k][1], &b)) continue;
		walk_hues(&b, 0, count, run);
		for (int h = 0; h < HUES; ++h) total[h] += count[h];
		area_src_free(&b);
	}
}

/* The area's roads, a second look of walkway, found in its own map
 * (road_hues: BN4's Park Area's orange paths, its ACDC Area's cyan, its
 * Town Area's yellow; no area names them): the map learned again, and its
 * mirror image, with the roads' hues for its walkways' and its walkways'
 * for the floor that lends no tiles; no platform floor of theirs. */
static void learn_roads(AreaSrc *src, int area, const int more[HUES], Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	L->nroads = 0;
	L->road_hues = 0;
	uint16_t roads = road_hues(src, na, more);
	if (emu_debug_on()) fprintf(stderr, "roads area %d: hues %04x\n", area, roads);
	if (!roads) return;
	/* (its panels laid whole, whatever the area's own are: a road's rim is
	 * laid with its panels alone, tilemap.c) */
	uint32_t styles = (na->styles & ~(0x1FFFu | TILES_PADS_ALONE)) | TILES_WHOLE;
	uint16_t skip = na->walk_styles & 0x1FFFu;
	tiles_learn(src, styles, roads, skip, 0, na->bg_in_map, &L->road[L->nroads++]);
	AreaSrc m;
	if (!area_src_mirror_in(src, &m)) return;
	if (aligned(src, &m)) tiles_learn(&m, styles, roads, skip, 0, na->bg_in_map, &L->road[L->nroads++]);
	area_src_mirror_end(src, &m);
	/* (a road is one mostly in the open: where three quarters of its
	 * panels lie beside other floor, as BN5's Oran Area's planks between
	 * its hubs, it is laid in scraps) */
	const TilePanels *p = &L->road[0].panels;
	if (p->n * 4 < p->n + p->npart) {
		for (int k = 0; k < L->nroads; ++k) tiles_free(&L->road[k]);
		L->nroads = 0;
	}
	L->road_hues = L->nroads ? roads : 0;
}

/* The Net Dealer's counter, cut from its map facing world +Y (FACES_X) and
 * from that map's mirror image facing world -X (FACES_Y). Its tiles must
 * fall on the layer's lattice (`own`'s) at some small offset (paste). */
static void learn_counter(const int16_t *c, const AreaSrc *own, Learned *L) {
	if (!c[0]) return;
	AreaSrc src, m;
	if (!area_src_load(c[0], c[1], &src)) return;
	props_learn_counter(&src, c[2], c[3], FACES_X, &L->counter[FACES_X]);
	/* (its mirror made in place, src not read again till it is let go) */
	if (area_src_mirror_in(&src, &m)) {
		props_mirror_walls(&src, &m);
		props_learn_counter(&m, -c[3], -c[2], FACES_Y, &L->counter[FACES_Y]);
	}
	/* anchored on a panel corner of the layer (whose lattice is `own`'s:
	 * the same phase whatever the corner), the least move that sets its
	 * tiles on the lattice, along its run before in its depth (its back
	 * row stays on the panel edge the aisle's walls run along) */
	for (int f = 0; f < 2; ++f) {
		PropStamp *st = &L->counter[f];
		if (!st->ok) continue;
		int best = -1;
		for (int dx = -16; dx <= 16; dx += 4)
			for (int dy = -16; dy <= 16; dy += 4) {
				int X = own->ex + dx, Y = own->ey + dy;
				int px = area_px(own->tw, X, Y) + st->tiles[0].px, py = area_py(own->th, X, Y) + st->tiles[0].py;
				if ((px & 7) || (py & 7)) continue;
				int cost = f == FACES_X ? abs(dx) + 4 * abs(dy) : 4 * abs(dx) + abs(dy);
				if (best < 0 || cost < best) { best = cost; L->counter_dx[f] = dx; L->counter_dy[f] = dy; }
			}
		if (best < 0) props_free(st);
	}
	if (emu_debug_on())
		for (int f = 0; f < 2; ++f)
			fprintf(stderr, "counter faces %d ok %d len %d tiles %d walls %d navi %d,%d\n", f, L->counter[f].ok, L->counter[f].len,
				L->counter[f].ntiles, L->counter[f].nwalls, L->counter[f].navi_x, L->counter[f].navi_y);
	if (emu_debug_on())
		for (int f = 0; f < 2; ++f) fprintf(stderr, "counter faces %d moved %d,%d onto the lattice\n", f, L->counter_dx[f], L->counter_dy[f]);
	area_src_mirror_end(&src, &m);
	area_src_free(&src);
}

/* Marks the first-layer tiles `a` draws in palette bank `to`. */
static void rebank_seen(const AreaSrc *a, int to, Learned *L) {
	if (!to) return;
	for (int i = 0; i < a->tw * a->th; ++i)
		if (a->tile[0][i] >> 12 == to && (a->tile[0][i] & 0x3FF)) L->rebank_to[(a->tile[0][i] & 0x3FF) >> 3] |= (uint8_t)(1u << (a->tile[0][i] & 7));
}

/* The area's other maps in the same tiles and colours, for the places its
 * own never shows, on its own map's grid. */
/* map m's free-standing pieces, unless the area sets none (TILES_NO_SCENERY) */
static void learn_decor(const AreaSrc *m, const NetAreaDef *na, Learned *L) {
	if (!(na->styles & TILES_NO_SCENERY)) decor_learn(m, na->bg_in_map, &L->decor);
}

/* (debug output) what area `area` learned */
static void learn_report(int area, const Learned *L) {
	for (int k = 0; k < L->nroads; ++k)
		fprintf(stderr, "roads area %d book %d: %d pairs, %d panels whole and %d in part, floor %d px down, faces %d px, hanging %d px\n", area, k,
			L->road[k].n, L->road[k].panels.n, L->road[k].panels.npart, L->road[k].dv, L->road[k].face, L->road[k].hang);
	fprintf(stderr, "tiles area %d floor %d px down, faces %d px, hanging %d px, %d books, %d pieces of scenery, its middle's period %dx%d, its looks %d\n",
		area, L->book[0].dv, L->book[0].face, L->book[0].hang, L->nbooks, L->decor.n, L->book[0].pa, L->book[0].pb,
		L->book[0].vary ? L->book[0].vary_first[64] : 0);
	for (int d = 0; d < STAIR_DIRS; ++d)
		fprintf(stderr, "stairs area %d dir %d ok %d rise %d ramp %d walls %d prio %d tiles %d\n", area, d, L->stairs[d].ok,
			L->stairs[d].rise, L->stairs[d].nramp, L->stairs[d].nwalls, L->stairs[d].nprio, L->stairs[d].ntiles);
	for (int d = 0; d < 4; ++d) fprintf(stderr, "arrow panel area %d way %d ok %d tiles %d\n", area, d, L->arrow[d].ok, L->arrow[d].ntiles);
}

/* Map a's arrow panels, the ways not learned yet: BN6's, and BN5's, whose
 * maps mark theirs with the same section-3 values 0x48-0x4F (SciLab 4, End
 * Area 1-3, Oran Area 2, the Undernet 1-3 and Nebula Area 2 and 4:
 * docs/rom/bn5-world.md, Its arrow panels and ramps); the lane on a layer
 * is BN6's own ride either way, its art the area's. */
static void learn_arrows(const AreaSrc *a, Learned *L) {
	for (int d = 0; d < 4; ++d) props_learn_arrow(a, d, &L->arrow[d]);
}

static void learn_more(const AreaSrc *grid, int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	for (int k = 0; k < NET_MORE_MAPS && na->more[k][0]; ++k) {
		AreaSrc b;
		if (!load_map(na, na->more[k][0], na->more[k][1], &b)) continue;
		learn_map(&b, grid, area, k + 1, L);
		learn_decor(&b, na, L);
		rebank_seen(&b, na->rebank[1], L);
		for (int o = 0; o < 3 && !na->xrom; ++o)
			if (!L->ornament[o].ok && aligned(grid, &b)) props_learn_ornament(&b, nm_ornament_tile[o], &L->ornament[o]);
		/* (its pads' tiles too, where its own map's all have a bridge
		 * beside them there) */
		if (na->pad_hues && aligned(grid, &b)) props_learn_pad(&b, na->pad_hues, pad_deep(L), &L->pad);
		/* (its emblem, where its own map draws none: BN4's Undernet's
		 * plain plates, crossed in another of its maps) */
		if (na->emblem && !L->emblem.ok && aligned(grid, &b)) props_learn_floor_emblem(&b, na->emblem, &L->emblem);
		if (aligned(grid, &b)) learn_arrows(&b, L);
		area_src_free(&b);
	}
	/* (and the arrow panels of its maps listed for them alone) */
	for (int k = 0; k < 2 && na->arrow_maps[k][0]; ++k) {
		AreaSrc b;
		if (!load_map(na, na->arrow_maps[k][0], na->arrow_maps[k][1], &b)) continue;
		if (aligned(grid, &b)) learn_arrows(&b, L);
		area_src_free(&b);
	}
}

/* The books settled, once all are learned; whether its maps have pads. */
static void settle(Learned *L) {
	for (int k = 0; k < L->nbooks; ++k) panels_settle(&L->book[k].panels);
	for (int k = 0; k < L->nroads; ++k) panels_settle(&L->road[k].panels);
	L->pads_seen = false;
	for (int k = 0; k < L->nbooks; ++k)
		for (int i = 0; i < L->book[k].n && !L->pads_seen; ++i) L->pads_seen = L->book[k].cand[i].pad;
}

bool nm_learn(int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	AreaSrc a;
	int more[HUES];
	if (!na) return false;
	more_hues(na, more);
	if (!load_map(na, na->group, na->number, &a)) return false;
	L->ex = a.ex; L->ey = a.ey; L->tw = a.tw; L->th = a.th;
	L->desc = L->src_desc = a.desc; L->coord_slot = a.coord_slot;
	/* (another game's area takes over a BN6 map: its slots are BN6's) */
	L->other = na->xrom != 0;
	L->def = na;
	if (L->other ? !area_src_slots(na->over[0], na->over[1], &L->desc, &L->coord_slot)
		: na->host && !area_src_slots(na->group, na->host - 1, &L->desc, &L->coord_slot)) { area_src_free(&a); return false; }
	L->nbooks = 0;
	learn_map(&a, &a, area, 0, L);
	learn_roads(&a, area, more, L);
	learn_decor(&a, na, L);
	stairs_learn(&a, L->stairs);
	learn_counter(na->counter, &a, L);
	/* (the pads' centrepieces are BN6's tiles) */
	for (int k = 0; k < 3 && !na->xrom; ++k) props_learn_ornament(&a, nm_ornament_tile[k], &L->ornament[k]);
	if (na->emblem) props_learn_floor_emblem(&a, na->emblem, &L->emblem);
	if (na->pad_hues) props_learn_pad(&a, na->pad_hues, pad_deep(L), &L->pad);
	learn_arrows(&a, L);
	L->rebank[0] = na->rebank[0];
	L->rebank[1] = na->rebank[1];
	rebank_seen(&a, na->rebank[1], L);
	if (emu_debug_on()) fprintf(stderr, "emblem area %d ok %d tiles %d\n", area, L->emblem.ok, L->emblem.ntiles);
	if (na->looks & (1u << LOOK_TREE) && na->looks & (1u << LOOK_GIANT_TREE))   /* (Green's) */
		for (int k = 0; k < 2; ++k) props_learn_void_art(&a, k ? 0x361 : 0x292, &L->bush[k]);
	/* the colours its floors show (an area's maps share its palette) */
	static uint8_t seen[TILE_COLOURS];
	memset(seen, 0, sizeof seen);
	int own = L->nbooks;
	for (int k = 0; k < own; ++k) tiles_colours(&L->book[k], seen);
	/* pads in the look of the other maps' pads alone, where the own map's
	 * small platforms are something else (Robot Control Comp 2's conveyor) */
	if (na->styles & TILES_MORE_PADS)
		for (int k = 0; k < own; ++k) tiles_drop_pads(&L->book[k]);
	/* (its own map's grid alone from here, freed before the others load: a
	 * map, its mirror and the own map at once took a New 3DS's memory) */
	AreaSrc grid = { .tw = a.tw, .th = a.th, .ex = a.ex, .ey = a.ey };
	area_src_free(&a);
	learn_more(&grid, area, L);
	if (emu_debug_on()) fprintf(stderr, "pad stamp area %d ok %d tiles %d, %d panels beside its lower sides\n", area, L->pad.ok, L->pad.ntiles, L->pad.low);
	/* but not their pieces in colours this one's floors never show: another
	 * surface's (Central Area 2's raised plateau, yellow where Central's
	 * fields are green; an Undernet map's raised grey court), which fit some
	 * joins and dead ends better than the area's own tiles and stand out
	 * there, unless the area takes those colours too */
	for (int k = own; k < L->nbooks; ++k) tiles_other_colours(&L->book[k], seen, !(na->styles & TILES_MORE_COLOURS));
	settle(L);
	if (emu_debug_on()) learn_report(area, L);
	return true;
}

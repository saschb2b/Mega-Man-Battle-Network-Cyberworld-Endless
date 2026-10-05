/* An area's original maps learned for its layers (netmap.c): their view,
 * their counters, decorations and arrows, kept per area. */
#include "netmap_learn.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"

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

/* A map of the area's own game, BN6's or another's (docs/MULTIROM.md). */
static bool load_map(const NetAreaDef *na, int group, int number, AreaSrc *a) {
	return area_src_load_as(na->xrom, group, number, na->recolour, a);
}

/* Learns the tiles of map `src` and its mirror image, where their tiles
 * line up with the layer's grid (`grid_of`). */
static void learn_view(const AreaSrc *src, const AreaSrc *grid_of, int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, src)) {
		tiles_learn(src, na->styles, na->walk_styles, na->skip_styles, na->joint_hues, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, src, na->bg_in_map);
	}
	AreaSrc m;
	if (!area_src_mirror(src, &m)) return;
	if (L->nbooks < MAX_BOOKS && aligned(grid_of, &m)) {
		tiles_learn(&m, na->styles, na->walk_styles, na->skip_styles, na->joint_hues, na->bg_in_map, &L->book[L->nbooks++]);
		seams_add(&L->seams, &m, na->bg_in_map);
	}
	area_src_free(&m);
}

/* ... at each of its floor heights: raised floors are drawn higher, so each
 * is learned in a view that brings it down to the ground. */
static void learn_map(const AreaSrc *src, const AreaSrc *grid_of, int area, Learned *L) {
	int count[256] = { 0 };
	for (int i = 0; src->hz && i < src->hw * src->hh; ++i) count[src->hz[i]]++;
	learn_view(src, grid_of, area, L);
	for (int z = 8; z < HEIGHT_UNEVEN; z += 8) {
		if (count[z] < NETMAP_LEVEL_MIN_CELLS) continue;
		AreaSrc r;
		if (!area_src_raise(src, z, &r)) continue;
		learn_view(&r, grid_of, area, L);
		area_src_free(&r);
	}
}

/* The Net Dealer's counter, cut from its map facing world +Y (FACES_X) and
 * from that map's mirror image facing world -X (FACES_Y). Its tiles must
 * fall on the layer's lattice (`own`'s) at some small offset (paste). */
static void learn_counter(const int16_t *c, const AreaSrc *own, Learned *L) {
	if (!c[0]) return;
	AreaSrc src, m;
	if (!area_src_load(c[0], c[1], &src)) return;
	props_learn_counter(&src, c[2], c[3], FACES_X, &L->counter[FACES_X]);
	if (area_src_mirror(&src, &m)) {
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
	area_src_free(&m);
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
	fprintf(stderr, "tiles area %d floor %d px down, faces %d px, hanging %d px, %d books, %d pieces of scenery, its middle's period %dx%d, its looks %d\n",
		area, L->book[0].dv, L->book[0].face, L->book[0].hang, L->nbooks, L->decor.n, L->book[0].pa, L->book[0].pb,
		L->book[0].vary ? L->book[0].vary_first[64] : 0);
	for (int d = 0; d < STAIR_DIRS; ++d)
		fprintf(stderr, "stairs area %d dir %d ok %d rise %d ramp %d walls %d prio %d tiles %d\n", area, d, L->stairs[d].ok,
			L->stairs[d].rise, L->stairs[d].nramp, L->stairs[d].nwalls, L->stairs[d].nprio, L->stairs[d].ntiles);
	for (int d = 0; d < 4; ++d) fprintf(stderr, "arrow panel area %d way %d ok %d tiles %d\n", area, d, L->arrow[d].ok, L->arrow[d].ntiles);
}

/* Map a's arrow panels, the ways not learned yet (BN6's maps alone: another
 * game's sections are its own). */
static void learn_arrows(const AreaSrc *a, const NetAreaDef *na, Learned *L) {
	for (int d = 0; d < 4 && !na->xrom; ++d) props_learn_arrow(a, d, &L->arrow[d]);
}

static void learn_more(const AreaSrc *grid, int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	for (int k = 0; k < NET_MORE_MAPS && na->more[k][0]; ++k) {
		AreaSrc b;
		if (!load_map(na, na->more[k][0], na->more[k][1], &b)) continue;
		learn_map(&b, grid, area, L);
		learn_decor(&b, na, L);
		rebank_seen(&b, na->rebank[1], L);
		for (int o = 0; o < 3 && !na->xrom; ++o)
			if (!L->ornament[o].ok && aligned(grid, &b)) props_learn_ornament(&b, nm_ornament_tile[o], &L->ornament[o]);
		/* (its pads' tiles too, where its own map's all have a bridge
		 * beside them there) */
		if (na->pad_hues && aligned(grid, &b)) props_learn_pad(&b, na->pad_hues, &L->pad);
		if (aligned(grid, &b)) learn_arrows(&b, na, L);
		area_src_free(&b);
	}
	/* (and the arrow panels of its maps listed for them alone) */
	for (int k = 0; k < 2 && na->arrow_maps[k][0]; ++k) {
		AreaSrc b;
		if (!load_map(na, na->arrow_maps[k][0], na->arrow_maps[k][1], &b)) continue;
		if (aligned(grid, &b)) learn_arrows(&b, na, L);
		area_src_free(&b);
	}
}

bool nm_learn(int area, Learned *L) {
	const NetAreaDef *na = net_area_def(area);
	AreaSrc a;
	if (!na || !load_map(na, na->group, na->number, &a)) return false;
	L->ex = a.ex; L->ey = a.ey; L->tw = a.tw; L->th = a.th;
	L->desc = L->src_desc = a.desc; L->coord_slot = a.coord_slot;
	/* (another game's area takes over a BN6 map: its slots are BN6's) */
	L->other = na->xrom != 0;
	L->def = na;
	if (L->other ? !area_src_slots(na->over[0], na->over[1], &L->desc, &L->coord_slot)
		: na->host && !area_src_slots(na->group, na->host - 1, &L->desc, &L->coord_slot)) { area_src_free(&a); return false; }
	L->nbooks = 0;
	learn_map(&a, &a, area, L);
	learn_decor(&a, na, L);
	stairs_learn(&a, L->stairs);
	learn_counter(na->counter, &a, L);
	/* (the pads' centrepieces are BN6's tiles) */
	for (int k = 0; k < 3 && !na->xrom; ++k) props_learn_ornament(&a, nm_ornament_tile[k], &L->ornament[k]);
	if (na->emblem) props_learn_floor_emblem(&a, na->emblem, &L->emblem);
	if (na->pad_hues) props_learn_pad(&a, na->pad_hues, &L->pad);
	learn_arrows(&a, na, L);
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
	L->pads_seen = false;
	for (int k = 0; k < L->nbooks; ++k)
		for (int i = 0; i < L->book[k].n && !L->pads_seen; ++i) L->pads_seen = L->book[k].cand[i].pad;
	if (emu_debug_on()) learn_report(area, L);
	return true;
}

/* The town (docs/OVERWORLD.md): a square in Central Town's style, planned
 * from the run's seed and drawn with Central Town's own tiles.
 *
 * The plan is a grid of 8-unit world cells of ground materials, built from
 * rectangles: the sidewalk slab, a cobbled court, a brick plaza with the
 * jack-in port, a road. Each side of a rectangle takes one of the edges
 * the source map shows between the two materials (its curbs, the slab's
 * planting strips and faces), moved by up to two cells to a place where
 * the source has it. townsrc picks the tiles; the walls ring the walkable
 * ground; trees stand on the planting strips, townsfolk on the square. */
#include "town.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "area_src.h"
#include "bytes.h"
#include "coords.h"
#include "emu.h"
#include "lz.h"
#include "mapslot.h"
#include "npc.h"
#include "rom.h"
#include "text.h"
#include "townsrc.h"

#define TOWN_TILEMAP_AT (EMU_FREE + 0x100000) /* the town's tile map (LZ77) */
#define TOWN_TILEMAP_MAX 0x30000

#define PLAN_R 60                 /* the plan spans cells -PLAN_R .. PLAN_R - 1 */
#define PLAN_N (2 * PLAN_R)

/* The port's jack-in trigger: 0x40 is the map's jack-in destination 0. */
#define JACK_IN_TRIGGER 0x40

/* Sprites: the port (list 7), townsfolk (list 5), trees (map object) */
#define SPR_PORT      0x24
#define OBJ_TREE      0x7D
/* a spawn record's first byte picks the spawner: 5 places a map object */
#define OBJ_SPAWN_MAP_OBJECT 5
#define SPR_TREE      0x51

static struct {
	TownBook *book;
	uint8_t plan[PLAN_N][PLAN_N];
	TownInfo info;
	uint16_t *tiles;
	uint8_t *miss;
	uint32_t rng;
	/* where things stand: the plaza (port), trees, people */
	int bx0, by0, bx1, by1;
	int sx0, sy0, sx1, sy1;
	int ry0, ry1;
	int gx0, gy0, gx1, gy1;   /* the court's planter */
	int trees[32][2], ntrees;
	int folk[4][2], nfolk;
	int folk_kind[4];
} T;

static uint32_t rnd(void) {
	uint32_t x = T.rng ? T.rng : 0x9E3779B9u;
	x ^= x << 13; x ^= x >> 17; x ^= x << 5;
	return T.rng = x;
}
static int rnd_range(int lo, int hi) { return lo + (int)(rnd() % (uint32_t)(hi - lo + 1)); }

static int fmod_(int a, int b) { int m = a % b; return m < 0 ? m + b : m; }

/* townsfolk: a sprite of list 5 and what they say */
static const struct { int sprite; const char *words; } folk_lines[] = {
	{ 0x2F, "They say the net under this\ntown never ends.|Every time you jack in,\nit's a new one." },
	{ 0x22, "The deeper you go, the\ntougher the viruses get!|Keep your folder sharp!" },
	{ 0x32, "Each area of the net has a\nguardian at its end.|The card you see when you\narrive tells you who." },
	{ 0x46, "Jacking in? The port's\nright there on the plaza." },
	{ 0x25, "Mystery Data out at the\ndead ends of the net...|Worth the detour, I say." },
	{ 0x2C, "If a Navi deletes MegaMan,\nthe run is over.|But the net will be\nwaiting for you again." },
	{ 0x33, "I heard the Net Dealer sets\nup shop on the middle\nlayer of every area." },
	{ 0x40, "Some say three ScrtData\nopen a gate to a secret\narea somewhere down there." },
};
#define FOLK_KINDS ((int)(sizeof folk_lines / sizeof *folk_lines))

/* ---- the plan ---- */

static int plan_at(int cx, int cy) {
	int x = cx + PLAN_R, y = cy + PLAN_R;
	if (x < 0 || y < 0 || x >= PLAN_N || y >= PLAN_N) return TM_VOID;
	return T.plan[y][x];
}

static void fill(int x0, int y0, int x1, int y1, int m) {
	for (int y = y0; y < y1; ++y)
		for (int x = x0; x < x1; ++x)
			if (x + PLAN_R >= 0 && y + PLAN_R >= 0 && x + PLAN_R < PLAN_N && y + PLAN_R < PLAN_N) T.plan[y + PLAN_R][x + PLAN_R] = (uint8_t)m;
}

/* the edges a plan may use: seen 3 times or more, with only curbs and
 * grass between the two materials */
static bool usable(const TownProfile *p) {
	if (p->count < 4) return false;
	for (int i = 0; i < p->nmid; ++i) if (p->mid[i] != TM_MARK && p->mid[i] != TM_GRASS) return false;
	return true;
}

/* The most frequent edge between `first` and `second` along axis o whose
 * boundary (at cell `at` + d, less the curb for a side facing low
 * coordinates) the source shows, d within two cells. */
static const TownProfile *edge(int o, int first, int second, int at, bool low_side, bool mirror, int *d_out) {
	const TownProfile *list;
	int n = townsrc_profiles(T.book, &list);
	static const int ds[5] = { 0, 1, -1, 2, -2 };
	const TownProfile *best = NULL;
	for (int k = 0; k < 5; ++k)
		for (int i = 0; i < n; ++i) {
			const TownProfile *p = &list[i];
			if (p->o != o || p->a != first || p->b != second || !usable(p) || (p->mirrored && !mirror)) continue;
			int boundary = low_side ? at + ds[k] - p->nmid : at + ds[k];
			if (p->ph != fmod_(boundary * 8, 32)) continue;
			/* the most often seen; of equals, the plainer (fewer curb cells) */
			if (!best || p->count > best->count || (p->count == best->count && p->nmid < best->nmid)) { best = p; *d_out = ds[k]; }
		}
	return best;
}

/* A rectangle of m over cells [x0, x1) x [y0, y1) in `outside`; sides
 * (x low x, X high x, y, Y) take the source's edges. Returns its bounds. */
static void rect(int *x0, int *y0, int *x1, int *y1, int m, int outside, const char *sides) {
	const TownProfile *px = NULL, *pX = NULL, *py = NULL, *pY = NULL;
	int d;
	/* (the mirror's edges only for the slab's north-west side: its
	 * planting strip) */
	bool nw_strip = outside == TM_VOID;
	if (strchr(sides, 'x') && (px = edge(0, outside, m, *x0, true, false, &d))) *x0 += d;
	if (strchr(sides, 'X') && (pX = edge(0, m, outside, *x1, false, false, &d))) *x1 += d;
	if (strchr(sides, 'y') && (py = edge(1, outside, m, *y0, true, nw_strip, &d))) *y0 += d;
	if (strchr(sides, 'Y') && (pY = edge(1, m, outside, *y1, false, false, &d))) *y1 += d;
	fill(*x0, *y0, *x1, *y1, m);
	if (getenv("CYBERWORLD_TOWN_DEBUG")) {
		const TownProfile *ps[4] = { px, pX, py, pY };
		for (int k = 0; k < 4; ++k)
			if (ps[k]) fprintf(stderr, "rect m%d side %c: %d>%d mid %d [%d %d] ph %d count %d\n", m, "xXyY"[k], ps[k]->a, ps[k]->b, ps[k]->nmid, ps[k]->mid[0], ps[k]->mid[1], ps[k]->ph, ps[k]->count);
			else if (strchr(sides, "xXyY"[k])) fprintf(stderr, "rect m%d side %c: none\n", m, "xXyY"[k]);
	}
	/* curbs and strips outside the rectangle */
	for (int i = 0; px && i < px->nmid; ++i) fill(*x0 - px->nmid + i, *y0, *x0 - px->nmid + i + 1, *y1, px->mid[i]);
	for (int i = 0; pX && i < pX->nmid; ++i) fill(*x1 + i, *y0, *x1 + i + 1, *y1, pX->mid[i]);
	for (int i = 0; py && i < py->nmid; ++i) fill(*x0, *y0 - py->nmid + i, *x1, *y0 - py->nmid + i + 1, py->mid[i]);
	for (int i = 0; pY && i < pY->nmid; ++i) fill(*x0, *y1 + i, *x1, *y1 + i + 1, pY->mid[i]);
}

static bool walkable(int cx, int cy) {
	int m = plan_at(cx, cy);
	if (m == TM_VOID || m == TM_GRASS || m == TM_EDGE) return false;
	if (m == TM_MARK)   /* a planter's curb */
		for (int dy = -1; dy <= 1; ++dy)
			for (int dx = -1; dx <= 1; ++dx) if (plan_at(cx + dx, cy + dy) == TM_GRASS) return false;
	return true;
}

static int step4(int lo, int hi) { return lo + 4 * rnd_range(0, (hi - lo) / 4); }

static void design(void) {
	memset(T.plan, TM_VOID, sizeof T.plan);
	/* (sizes in steps of four cells, the period of the source's edges, so
	 * that each takes effect) */
	int W = step4(32, 44), H = step4(28, 36);
	int x0 = -W / 2, y0 = -H / 2, x1 = x0 + W, y1 = y0 + H;
	/* the sidewalk slab: planting strips on its upper edges, faces below */
	rect(&x0, &y0, &x1, &y1, TM_SIDE, TM_VOID, "xXyY");
	T.sx0 = x0; T.sy0 = y0; T.sx1 = x1; T.sy1 = y1;
	/* the road across, above or below the plaza's band */
	bool road_first = rnd_range(0, 2) == 0, road = rnd_range(0, 4) != 0;
	int bh = step4(8, 12);
	int b0 = road && road_first ? y0 + 11 : y0 + 4;
	/* a cobbled court against the south-west edge, a planter in its far
	 * corner, as Central Town has them */
	int cx0 = x0, cy0 = b0, cx1 = x0 + step4(8, 12), cy1 = b0 + bh;
	rect(&cx0, &cy0, &cx1, &cy1, TM_COBB, TM_VOID, "x");
	rect(&cx0, &cy0, &cx1, &cy1, TM_COBB, TM_SIDE, "XyY");
	int gx0 = cx1 - rnd_range(4, 6), gy0 = cy0 + rnd_range(2, 3), gx1 = cx1, gy1 = cy1;
	rect(&gx0, &gy0, &gx1, &gy1, TM_GRASS, TM_COBB, "xy");
	rect(&gx0, &gy0, &gx1, &gy1, TM_GRASS, TM_SIDE, "XY");
	T.gx0 = gx0; T.gy0 = gy0; T.gx1 = gx1; T.gy1 = gy1;
	/* the brick plaza against the north-east edge, as Central Town's: its
	 * side there runs into the planting strip */
	int bw = step4(9, 17);
	int bx0 = x1 - bw, by0 = b0, bx1 = x1, by1 = b0 + bh;
	rect(&bx0, &by0, &bx1, &by1, TM_BRICK, TM_VOID, "X");
	rect(&bx0, &by0, &bx1, &by1, TM_BRICK, TM_SIDE, "xyY");
	T.bx0 = bx0; T.by0 = by0; T.bx1 = bx1; T.by1 = by1;
	T.ry0 = T.ry1 = 0;
	if (road) {
		int rx0 = x0 + 2, ry0 = road_first ? y0 + 4 : by1 + 3, rx1 = x1 - 3, ry1 = ry0 + 5;
		rect(&rx0, &ry0, &rx1, &ry1, TM_ROAD, TM_SIDE, "xXyY");
		T.ry0 = ry0; T.ry1 = ry1;
	}
}

static bool paved(int cx, int cy) {
	int m = plan_at(cx, cy);
	return m == TM_SIDE || m == TM_BRICK || m == TM_COBB;
}

/* a tree may stand on paving with paving around it, away from the port,
 * Lan's start and the other trees */
static bool tree_fits(int cx, int cy) {
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx) if (!paved(cx + dx, cy + dy)) return false;
	int x = cx * 8 + 4, y = cy * 8 + 4;
	if (abs(x - T.info.port_x) + abs(y - T.info.port_y) < 48) return false;
	if (abs(x - T.info.start_x) + abs(y - T.info.start_y) < 40) return false;
	for (int i = 0; i < T.ntrees; ++i) if (abs(x - T.trees[i][0]) + abs(y - T.trees[i][1]) < 40) return false;
	for (int i = 0; i < T.nfolk; ++i) if (abs(x - T.folk[i][0]) + abs(y - T.folk[i][1]) < 32) return false;
	return true;
}

static void add_tree(int cx, int cy) {
	if (T.ntrees >= 32 || !tree_fits(cx, cy)) return;
	T.trees[T.ntrees][0] = cx * 8 + 4;
	T.trees[T.ntrees][1] = cy * 8 + 4;
	++T.ntrees;
}

/* Central Town's trees stand on its paving in rows: at the plaza's
 * corners, along the road, beside the planting strips */
static void place_trees(void) {
	T.ntrees = 0;
	add_tree(T.bx0 + 1, T.by0 + 1);
	add_tree(T.bx0 + 1, T.by1 - 2);
	add_tree(T.bx1 - 3, T.by1 - 2);
	if (T.ry1) {
		int gap = rnd_range(7, 9);
		for (int cx = T.sx0 + 4; cx < T.sx1 - 4; cx += gap) {
			add_tree(cx, T.ry0 - 2);
			add_tree(cx + gap / 2, T.ry1 + 1);
		}
	}
	int gap = rnd_range(6, 8);
	for (int cx = T.sx0 + 3; cx < T.bx0 - 1; cx += gap) add_tree(cx, T.sy0 + 1);
	for (int cy = T.by1 + 2; cy < T.sy1 - 2; cy += gap) add_tree(T.sx1 - 2, cy);
}

/* One plan and its tiles; the number of tiles no source tile matched. */
static int plan_once(uint32_t seed) {
	T.rng = seed * 2246822519u + 0x165667B1u;
	design();
	/* the port in the plaza's middle; Lan at the square's south corner */
	T.info.port_x = (T.bx0 + T.bx1) * 4;
	T.info.port_y = (T.by0 + T.by1) * 4;
	int scx = T.sx0 + 4, scy = T.sy1 - 5;
	while (!walkable(scx, scy) && scx < T.sx1) ++scx;
	T.info.start_x = scx * 8 + 4;
	T.info.start_y = scy * 8 + 4;
	/* townsfolk: by the plaza, the court and the road, some of them */
	T.nfolk = 0;
	int spots[5][2] = {
		{ T.bx0 - 2, (T.by0 + T.by1) / 2 }, { (T.bx0 + T.bx1) / 2 - 2, T.by1 + 1 },
		{ T.gx0 - 2, T.gy1 + 1 }, { T.sx0 + 6, T.ry1 ? T.ry0 - 1 : T.sy1 - 8 }, { T.sx1 - 8, T.ry1 ? T.ry1 + 1 : T.sy1 - 4 },
	};
	for (int i = 0; i < 5 && T.nfolk < 4; ++i) {
		if (!walkable(spots[i][0], spots[i][1]) || rnd_range(0, 4) == 0) continue;
		T.folk[T.nfolk][0] = spots[i][0] * 8 + 4;
		T.folk[T.nfolk][1] = spots[i][1] * 8 + 4;
		T.folk_kind[T.nfolk] = rnd_range(0, FOLK_KINDS - 1);
		for (int k = 0; k < T.nfolk; ++k) if (T.folk_kind[k] == T.folk_kind[T.nfolk]) T.folk_kind[T.nfolk] = (T.folk_kind[T.nfolk] + 1) % FOLK_KINDS;
		++T.nfolk;
	}
	place_trees();
	/* the tile map: big enough for every cell, with room for the faces */
	int umax = 0, vmax = 0;
	for (int cy = -PLAN_R; cy < PLAN_R; ++cy)
		for (int cx = -PLAN_R; cx < PLAN_R; ++cx) {
			if (plan_at(cx, cy) == TM_VOID) continue;
			for (int k = 0; k < 4; ++k) {
				int x = cx * 8 + (k & 1) * 8, y = cy * 8 + (k >> 1) * 8;
				int u = abs(x + y), v = abs((y - x) / 2);
				if (u > umax) umax = u;
				if (v > vmax) vmax = v;
			}
		}
	int stw, sth;
	townsrc_size(T.book, &stw, &sth);
	/* the camera shows 120 pixels either side of Lan and 80 above and
	 * below: the map reaches that far past the square's edges */
	int tw = 2 * ((umax + 136 + 7) / 8), th = 2 * ((vmax + 104 + 7) / 8);
	if ((tw & 1) != (stw & 1)) ++tw;
	if ((th & 1) != (sth & 1)) ++th;
	if (tw > 255 || th > 255 || (size_t)tw * th * 4 > TOWN_TILEMAP_MAX) return -1;
	free(T.tiles);
	free(T.miss);
	T.tiles = calloc((size_t)tw * th * 2, 2);
	T.miss = calloc((size_t)tw * th, 1);
	TownSynthStats st;
	townsrc_synth(T.book, plan_at, tw, th, seed, T.tiles, T.miss, &st);
	T.info.tw = tw;
	T.info.th = th;
	T.info.picks = st.picks;
	T.info.misses = st.near;
	return st.near;
}

#define PLAN_TRIES 8
#define PLAN_GOOD  4   /* tiles without a match the town accepts */

bool town_plan(uint32_t seed) {
	if (!T.book && !(T.book = townsrc_learn(TOWN_GROUP, TOWN_NUMBER))) return false;
	/* plans for this seed and the next few until one's tiles all match
	 * (nearly): the same run always gets the same town */
	int best = -1, best_misses = 1 << 30;
	for (int k = 0; k < PLAN_TRIES; ++k) {
		int m = plan_once(seed + (uint32_t)k * 7919u);
		if (m >= 0 && m < best_misses) { best_misses = m; best = k; }
		if (m >= 0 && m <= PLAN_GOOD) return true;
	}
	return best >= 0 && plan_once(seed + (uint32_t)best * 7919u) >= 0;
}

const TownInfo *town_info(void) { return &T.info; }
const uint16_t *town_tiles(void) { return T.tiles; }
const uint8_t *town_misses(void) { return T.miss; }

uint32_t *town_render(int *w, int *h) {
	if (!T.tiles) return NULL;
	*w = T.info.tw * 8;
	*h = T.info.th * 8;
	return area_src_render(TOWN_GROUP, TOWN_NUMBER, T.tiles, T.info.tw, T.info.th);
}

/* ---- in the game ---- */

/* Compressed sprites only draw once the map loads them. */
static void need_sprite(NpcList *npcs, int category, int index) {
	uint32_t list = emu_read32(0x08000000u + R.layout->sprite_lists + (uint32_t)category * 4);
	if (!(emu_read32(list + (uint32_t)index * 4) & 0x80000000u)) return;
	for (int i = 0; i < npcs->nsprites; ++i)
		if (npcs->sprite_idx[i] == index && npcs->sprite_cat[i] == category * 4) return;
	if (npcs->nsprites >= 8) return;
	npcs->sprite_cat[npcs->nsprites] = (uint8_t)(category * 4);
	npcs->sprite_idx[npcs->nsprites++] = (uint8_t)index;
}

bool town_install(int to_group, int to_number, int x, int y) {
	if (!T.tiles) return false;
	uint32_t desc, coord_slot;
	townsrc_slots(T.book, &desc, &coord_slot);
	int tw = T.info.tw, th = T.info.th;
	/* the tile map */
	size_t cells = (size_t)tw * th, raw = cells * 4;
	uint8_t *out = malloc(16 + raw + raw / 8 + 16);
	size_t lz = lz_literal((const uint8_t *)T.tiles, raw, out + 12);
	out[0] = (uint8_t)tw; out[1] = (uint8_t)th; out[2] = out[3] = 0;
	put32(out + 4, 12);
	put32(out + 8, (uint32_t)(12 + cells * 2));
	emu_write(TOWN_TILEMAP_AT, out, 12 + lz);
	free(out);
	emu_write32(0x08000000u + desc + 8, TOWN_TILEMAP_AT);
	/* walls, and the port's jack-in trigger around it */
	static CoordCell trig[64];
	int nt = 0, pcx = T.info.port_x >> 3, pcy = T.info.port_y >> 3;
	for (int dy = -3; dy <= 3; ++dy)
		for (int dx = -3; dx <= 3; ++dx) {
			if (abs(dx) <= 1 && abs(dy) <= 1) continue;   /* the port itself */
			int type = dx == -3 && dy == -3 ? 0x09 : dx == 3 && dy == -3 ? 0x0A : dx == -3 && dy == 3 ? 0x0B : dx == 3 && dy == 3 ? 0x0C : 0x11;
			trig[nt++] = (CoordCell){ (int16_t)((pcx + dx) * 8), (int16_t)((pcy + dy) * 8), 0, JACK_IN_TRIGGER, 8, (uint8_t)type };
		}
	CoordExtra extra = { { NULL, NULL, NULL, trig }, { 0, 0, 0, nt } };
	if (!coords_write_town(coord_slot, walkable, &extra)) return false;
	/* people, the port and the trees, in the town's own space */
	mapslot_town(true);
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	TextArchive text;
	ta_begin(&text);
	static const int none[4] = { -1, -1, -1, -1 };
	int port_script = ta_talk(&text, "A public jack-in port.|Stand close to it and\npress R to jack in.", none);
	int folk_script[4];
	for (int i = 0; i < T.nfolk; ++i) folk_script[i] = ta_talk(&text, folk_lines[T.folk_kind[i]].words, none);
	uint32_t archive = ta_commit(&text);
	need_sprite(&npcs, 7, SPR_PORT);
	npcs.script[npcs.n++] = npc_talker(7, SPR_PORT, T.info.port_x, T.info.port_y, 0, 0, archive, port_script, -1, false);
	for (int i = 0; i < T.nfolk; ++i) {
		int spr = folk_lines[T.folk_kind[i]].sprite;
		need_sprite(&npcs, 5, spr);
		npcs.script[npcs.n++] = npc_talker(5, spr, T.folk[i][0], T.folk[i][1], 0, 4, archive, folk_script[i], -1, false);
	}
	/* trees: the game's own tree objects (20-byte spawn records) */
	need_sprite(&npcs, 7, SPR_TREE);
	uint8_t objs[33 * 20];
	int no = 0;
	for (int i = 0; i < T.ntrees && no < 32; ++i, ++no) {
		uint8_t *r = objs + no * 20;
		memset(r, 0, 20);
		r[0] = OBJ_SPAWN_MAP_OBJECT;
		put32(r + 4, (uint32_t)(T.trees[i][0] << 16));
		put32(r + 8, (uint32_t)(T.trees[i][1] << 16));
		put32(r + 16, OBJ_TREE);
	}
	objs[no * 20] = 0xFF;
	npcs.objects = mapslot_alloc(objs, no * 20 + 4);
	if (getenv("CYBERWORLD_TOWN_DEBUG")) fprintf(stderr, "town: %d trees, objects at %08x, %d people\n", no, npcs.objects, T.nfolk);
	bool ok = archive && mapslot_install(TOWN_GROUP, TOWN_NUMBER, &npcs, NULL, 0) &&
		mapslot_jack_in(TOWN_GROUP, TOWN_NUMBER, to_group, to_number, x, y, 4) &&
		mapslot_music(TOWN_GROUP, TOWN_NUMBER, TOWN_SONG);
	mapslot_town(false);
	return ok;
}

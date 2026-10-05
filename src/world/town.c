/* The town (docs/OVERWORLD.md): Central Town or ACDC Town as Capcom built
 * them, cut into pieces and set out again for each run.
 *
 * A plan is a list of pieces over 8-unit world cells. A copied piece is a
 * rectangle of the original moved as a whole: its tiles (the ground, and
 * the front layer's art that stands in it: houses, shop fronts, gates,
 * roofs), its walls, the cells that draw people behind that art, its trees
 * and statues, its jack-in cells and checks. A tiled piece repeats a
 * strip of the original's ground to stretch a road, a plaza or a court. The
 * cuts run through ground that is the same all along them, and the moves
 * keep the original's tile grid and the phase of its brick and
 * cobblestone, so what is copied lines up with what is stretched. townsrc
 * picks the tiles of the stretched ground, continuing the copied tiles
 * beside them. */
#include "town.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bn6.h"
#include "bytes.h"
#include "coords.h"
#include "debug.h"
#include "emu.h"
#include "lz.h"
#include "mapslot.h"
#include "npc.h"
#include "rom.h"
#include "text.h"
#include "town_lines.h"
#include "town_ports.h"
#include "townmath.h"
#include "townsrc.h"

#define TOWN_TILEMAP_AT (EMU_FREE + 0x100000) /* the town's tile map (LZ77) */

#define PLAN_R 64                 /* the plan spans cells -PLAN_R .. PLAN_R - 1 */
#define PLAN_N (2 * PLAN_R)

#define MAX_PIECES 48
#define MAX_OBJS 40
#define MAX_SEC2 2048
#define MAX_TRIG 256
#define MAX_WALLS 2048   /* a town copied whole: its original's walls and heights */

/* The jack-in trigger: 0x40 is the map's jack-in destination 0. */
#define JACK_IN_TRIGGER 0x40
/* check triggers: 0xF0 + n shows the map's check n when A is pressed */
#define CHECK_TRIGGER 0xF0
/* a spawn record's first byte picks the spawner: 5 places a map object */
#define OBJ_SPAWN_MAP_OBJECT 5
#define OBJ_TREE 0x7D
/* OverworldMapObjects: 16-byte entries (category, sprite, animation, ...) per object */
#define OW_MAP_OBJECTS 0x0A4F24u

enum { P_COPY, P_TILE };
enum { F_JACK_IN = 1 };

typedef struct {
	uint8_t kind, flags;
	int x0, y0, x1, y1;   /* target cells, inclusive */
	int sx, sy;           /* the source cell at (x0, y0) */
	int sw, sh;           /* P_TILE: the source strip, repeated */
} Piece;

typedef struct TownStyle Style;

static struct {
	const Style *style;
	const TownLines *lines;   /* its people and words */
	TownBook *books[4], *book;   /* each style's source, learned once; the current one */
	int stw, sth;
	uint8_t src_obj[32][20];
	int nsrc_obj;
	Piece piece[MAX_PIECES];
	int npieces;
	/* per plan cell: ground, walkable, and the piece that set it (-1 none) */
	uint8_t mat[PLAN_N][PLAN_N], walk[PLAN_N][PLAN_N];
	int8_t owner[PLAN_N][PLAN_N];
	TownPin *pins;
	CoordCell sec2[MAX_SEC2], trig[MAX_TRIG], walls[MAX_WALLS], heights[MAX_WALLS];
	int nsec2, ntrig, nwalls, nheights;
	uint8_t obj[MAX_OBJS][20];
	int nobj;
	/* trees the plan adds, off the walkable ground (world units) */
	int more_trees[8][2], nmore;
	int folk_at[MAX_FOLK][2];
	int gx, gy;           /* the whole plan's move to the map's middle */
	TownInfo info;
	uint16_t *tiles;
	uint8_t *miss;
	uint32_t rng;
} T;

static uint32_t rnd(void) {
	uint32_t x = T.rng ? T.rng : 0x9E3779B9u;
	x ^= x << 13; x ^= x >> 17; x ^= x << 5;
	return T.rng = x;
}
/* (from the high bits: the low bits of the first draws follow the seed's) */
static int rnd_range(int lo, int hi) { return lo + (int)(((uint64_t)rnd() * (uint32_t)(hi - lo + 1)) >> 32); }

static int fdiv(int a, int b) { return a >= 0 ? a / b : -((-a + b - 1) / b); }
static int fmod_(int a, int b) { int m = a % b; return m < 0 ? m + b : m; }

/* ---- pieces ---- */

static Piece *add(int kind, int x0, int y0, int x1, int y1) {
	if (T.npieces >= MAX_PIECES || x1 < x0 || y1 < y0) return NULL;
	Piece *p = &T.piece[T.npieces++];
	memset(p, 0, sizeof *p);
	p->kind = (uint8_t)kind;
	p->x0 = x0; p->y0 = y0; p->x1 = x1; p->y1 = y1;
	return p;
}

/* The original's cells [sx0, sx1] x [sy0, sy1], moved by (dx, dy). */
static void copy(int sx0, int sy0, int sx1, int sy1, int dx, int dy, int flags) {
	Piece *p = add(P_COPY, sx0 + dx, sy0 + dy, sx1 + dx, sy1 + dy);
	if (!p) return;
	p->sx = sx0; p->sy = sy0;
	p->flags = (uint8_t)flags;
}

/* Cells [x0, x1] x [y0, y1] of the original's strip of sw x sh cells at
 * (sx, sy), repeated. */
static void tile(int x0, int y0, int x1, int y1, int sx, int sy, int sw, int sh) {
	Piece *p = add(P_TILE, x0, y0, x1, y1);
	if (!p) return;
	p->sx = sx; p->sy = sy; p->sw = sw; p->sh = sh;
}

/* The source cell a piece shows at plan cell (cx, cy). */
static void source_of(const Piece *p, int cx, int cy, int *sx, int *sy) {
	if (p->kind == P_TILE) {
		*sx = p->sx + fmod_(cx - p->x0, p->sw);
		*sy = p->sy + fmod_(cy - p->y0, p->sh);
	} else {
		*sx = p->sx + cx - p->x0;
		*sy = p->sy + cy - p->y0;
	}
}

static bool in_source(const Piece *p, int sx, int sy) {
	return sx >= p->sx && sy >= p->sy && sx <= p->sx + p->x1 - p->x0 && sy <= p->sy + p->y1 - p->y0;
}

static void move_all(int dx, int dy) {
	for (int i = 0; i < T.npieces; ++i) {
		T.piece[i].x0 += dx; T.piece[i].x1 += dx;
		T.piece[i].y0 += dy; T.piece[i].y1 += dy;
	}
	for (int i = 0; i < T.nmore; ++i) { T.more_trees[i][0] += dx * 8; T.more_trees[i][1] += dy * 8; }
}

static void more_tree(int x, int y) {
	if (T.nmore >= 8) return;
	T.more_trees[T.nmore][0] = x;
	T.more_trees[T.nmore++][1] = y;
}

/* ---- the plan ---- */

static int plan_at(int cx, int cy) {
	int x = cx + PLAN_R, y = cy + PLAN_R;
	if (x < 0 || y < 0 || x >= PLAN_N || y >= PLAN_N) return TM_VOID;
	return T.mat[y][x];
}

static bool walkable(int cx, int cy) {
	int x = cx + PLAN_R, y = cy + PLAN_R;
	return x >= 0 && y >= 0 && x < PLAN_N && y < PLAN_N && T.walk[y][x];
}

/* Lays the pieces out on the plan, later ones over earlier ones. */
static void raster(void) {
	memset(T.mat, TM_VOID, sizeof T.mat);
	memset(T.walk, 0, sizeof T.walk);
	memset(T.owner, -1, sizeof T.owner);
	for (int i = 0; i < T.npieces; ++i) {
		const Piece *p = &T.piece[i];
		for (int cy = p->y0; cy <= p->y1; ++cy)
			for (int cx = p->x0; cx <= p->x1; ++cx) {
				int x = cx + PLAN_R, y = cy + PLAN_R;
				if (x < 0 || y < 0 || x >= PLAN_N || y >= PLAN_N) continue;
				int sx, sy;
				source_of(p, cx, cy, &sx, &sy);
				T.mat[y][x] = (uint8_t)townsrc_mat(T.book, sx, sy);
				T.walk[y][x] = townsrc_walk(T.book, sx, sy);
				T.owner[y][x] = (int8_t)i;
			}
	}
}

/* ---- the styles ---- */

/* A town's style: the original it is cut from (and whose map it takes
 * over), its plan, and what is said there. Points are in the original's
 * world units, moved with the piece they lie in. */
struct TownStyle {
	int group, number, song;
	unsigned jack_ins;                  /* the original's jack-in points kept: bit n for 0x40 + n */
	int front[4];                       /* the landmark's front (the original's cells x0, y0, x1, y1):
	                                     * R jacks in anywhere Lan stands there */
	int statue[4];                      /* the statue's own cells: R jacks in all round it too */
	TownMatOf mat_of;
	void (*design)(void);
	int start_x, start_y, start_face;   /* where Lan arrives */
	int look[TOWN_SPOTS][2];            /* places worth a look */
	bool whole;                         /* copied whole, with its own walls and heights (not the
	                                     * plan's walls round flat walkable cells) */
	const char *name, *landmark, *landmark_at;
	int port_src[2];                    /* the act's second and third ways' ports: the original's
	                                     * jack-in point (0x40 + n) or check (0xF0 + n) made one */
};

static int env_or(const char *name, int value) {
	const char *v = getenv(name);
	return v ? atoi(v) : value;
}

/* -- Central Town (0x01:0) -- */

static int central_mat(int i) {
	if (i == 0) return TM_VOID;
	if (i == 10) return TM_ROAD;
	if (i >= 1 && i <= 3) return TM_MARK;
	if (i == 4) return TM_SIDE;
	if (i >= 5 && i <= 8) return TM_COBB;
	if (i >= 80 && i <= 84) return TM_BRICK;
	if ((i >= 32 && i <= 39) || i == 15 || i == 17 || i == 18 || i == 20 || i == 97 || i == 98)
		return TM_GRASS;   /* (the pinks 106-109 are the houses' walls) */
	return TM_EDGE;
}

/* The original's cells, generously: the slab spans x -25..20, y -25..40;
 * past it hang its faces and the backs of the tall pieces. */
#define SX0 (-31)
#define SX1 24
#define SY0 (-33)
#define SY1 45

/* The plan: the original, widened by w cells (0 or 8): the Expo gates
 * stand further from the Academy, the plaza, the main road and the houses'
 * cobbled court grow wider. The cuts run through ground that is the same
 * all along them: the north-west road's middle (row -18|-17) splits the
 * gates' band, stretched in the gap between the Expo gates and the Academy
 * (x -5|-4), from the middle, stretched through the plaza's brick and the
 * main road (x 5|6); the main road's near curb (row 10|11) splits that from
 * the houses', stretched through their court (x -22|-21). */
static void design_central(void) {
	int w = env_or("CYBERWORLD_TOWN_VARIANT", rnd_range(0, 1)) ? 8 : 0;
	/* the gates' band */
	copy(SX0, SY0, -5, -18, 0, 0, 0);
	tile(-4, SY0, -5 + w, -18, -5, SY0, 1, -18 - SY0 + 1);
	copy(-4, SY0, SX1, -18, w, 0, 0);
	/* Aster Land, the plaza, the main road */
	copy(SX0, -17, 5, 10, 0, 0, F_JACK_IN);
	tile(6, -17, 5 + w, 10, 5, -17, 1, 28);
	copy(6, -17, SX1, 10, w, 0, F_JACK_IN);
	/* the houses and the bus stop */
	copy(SX0, 11, -22, SY1, 0, 0, 0);
	tile(-21, 11, -22 + w, SY1, -22, 11, 1, SY1 - 11 + 1);
	copy(-21, 11, SX1, SY1, w, 0, 0);
	/* the trees on the verge between the gates, a row 4 cells apart (where
	 * in their cells the original's stand) */
	for (int x = -3; x < -3 + w; x += 4) more_tree(x * 8 + 4, -24 * 8 + 4);
}

/* -- ACDC Town (0x00:0) -- */

static int acdc_mat(int i) {
	if (i == 0) return TM_VOID;
	if (i == 6 || (i >= 182 && i <= 186)) return TM_ROAD;   /* (with its joints) */
	if (i == 10) return TM_MARK;                             /* the crossings */
	if (i >= 1 && i <= 3) return TM_SIDE;
	if (i >= 11 && i <= 15) return TM_GRASS;                 /* lawn, walkable in its rims */
	if (i >= 89 && i <= 92) return TM_GRASS;                 /* bushes */
	if (i >= 121 && i <= 123) return TM_COBB;                /* the park's dirt */
	return TM_EDGE;
}

/* The original's cells: the slab spans x -39..39, y -23..22. */
#define AX0 (-45)
#define AX1 46
#define AY0 (-31)
#define AY1 29

/* The plan: ACDC Town's blocks in one row between its roads: the park and
 * Higsby's (x -33..-14), the L of houses round the Metroline with Dex's
 * block in its notch (x -9..22), the mansion at the end. The first two
 * trade places half the time, and the road between them (4 cells) goes
 * with them, stretched from the original's. The cuts run down the roads
 * from the back road to the avenue's near curb (row 10|11); below it they
 * run down the avenue and the promenade clear of each block's crossing, so
 * a crossing and its landing go with the block at whose corner they lie. */
static void design_acdc(void) {
	if (!env_or("CYBERWORLD_TOWN_VARIANT", rnd_range(0, 1))) {
		copy(AX0, AY0, AX1, AY1, 0, 0, F_JACK_IN);
		return;
	}
	/* the blocks and the road between them */
	copy(AX0, AY0, -34, 10, 0, 0, 0);
	copy(-9, AY0, 22, 10, -24, 0, F_JACK_IN);
	tile(-1, AY0, 2, 10, -12, AY0, 1, 10 - AY0 + 1);
	copy(-33, AY0, -14, 10, 36, 0, F_JACK_IN);
	copy(23, AY0, AX1, 10, 0, 0, 0);
	/* the avenue, the promenade and the slab's face below them */
	copy(AX0, 11, -36, AY1, 0, 0, 0);
	copy(-11, 11, 22, AY1, -24, 0, 0);
	tile(-1, 11, 0, AY1, -12, 11, 1, AY1 - 11 + 1);
	copy(-35, 11, -14, AY1, 36, 0, 0);
	copy(23, 11, AX1, AY1, 0, 0, 0);
}

/* -- Seaside Town (0x03:0) -- */

/* (its ground is copied whole: the materials only tell sky from the rest) */
static int seaside_mat(int i) { return i == 0 ? TM_VOID : TM_EDGE; }

/* The plan: the original as it stands, with its own walls and heights (the
 * plaza at 0, the walkway over the whale and the station at 64, the pier
 * at -32), which the other towns' flat plans do not model; the whole of
 * it, since the roofs' and the whale's art stands on cells far up the
 * picture. */
static void design_seaside(void) { copy(-62, -62, 62, 62, 0, 0, F_JACK_IN); }

/* -- Green Town (0x04:0) -- */

/* (copied whole, as Seaside Town: its planks, ponds and stumps stand at 16,
 * its plaza, paths and the pond by the Judge Tree at 0) */
static int green_mat(int i) { return i == 0 ? TM_VOID : TM_EDGE; }
static void design_green(void) { copy(-62, -62, 62, 62, 0, 0, F_JACK_IN); }

static const Style styles[] = {
	{ 0x01, 0x00, 0x03, 1 << 0, { 12, -9, 16, -2 }, { 14, -9, 16, -6 }, central_mat, design_central, -40, 266, FACE_SW,
	  { { -40, 266 }, { 100, -30 }, { -150, -30 }, { 90, -150 }, { 110, 110 }, { -150, -160 } }, false,
	  "Central Town", "bird statue", "bird statue on the plaza", { 0x41, 0xFA } },
	{ 0x00, 0x00, 0x24, 1 << 0, { -23, -10, -17, -3 }, { -20, -9, -16, -3 }, acdc_mat, design_acdc, -60, -108, FACE_SW,
	  { { -60, -108 }, { -190, -30 }, { -190, -120 }, { 110, -120 }, { 260, -60 }, { 60, 120 } }, false,
	  "ACDC Town", "squirrel statue", "squirrel statue in the park", { 0x41, 0xF7 } },
	/* (its own two ports the act's other ways: the fountain is the landmark) */
	{ 0x03, 0x00, 0x06, 0, { -8, -15, -6, -8 }, { -18, -16, -9, -7 }, seaside_mat, design_seaside, 4, -100, FACE_SW,
	  { { 4, -100 }, { -100, -100 }, { -120, -170 }, { 40, -70 }, { 150, 40 }, { 300, 0 } }, true,
	  "Seaside Town", "mermaid fountain", "mermaid fountain on the plaza", { 0x40, 0x41 } },
	/* (its port the original's own, round the knight statue) */
	{ 0x04, 0x00, 0x08, 1 << 0, { -7, -22, -1, -16 }, { -5, -22, -1, -18 }, green_mat, design_green, 40, -100, FACE_SW,
	  { { 40, -100 }, { -60, -204 }, { -20, -268 }, { -196, -172 }, { -32, 24 }, { -64, 312 } }, true,
	  "Green Town", "knight statue", "knight statue on the flower plaza", { 0xF1, 0xF7 } },
};
#define STYLES ((int)(sizeof styles / sizeof *styles))
_Static_assert(STYLES == TOWN_LINES, "each style its people and words (town_lines.c)");

/* ---- what the copied pieces bring ---- */

/* The source tile's centre, as a world cell of the original. */
static void tile_cell(int stx, int sty, int *cx, int *cy) {
	int a = stx * 8 + 4 - T.stw * 4, b = (sty * 8 + 4 - T.sth * 4) * 2;
	*cx = fdiv(fdiv(a - b, 2), 8);
	*cy = fdiv(fdiv(a + b, 2), 8);
}

/* The copied piece holding source cell (sx, sy), or NULL. */
static const Piece *copied(int sx, int sy) {
	for (int i = T.npieces - 1; i >= 0; --i) {
		const Piece *p = &T.piece[i];
		if (p->kind == P_COPY && in_source(p, sx, sy)) return p;
	}
	return NULL;
}

/* A point of the original (world units) where the plan has moved it. */
static bool moved(int x, int y, int *tx, int *ty) {
	const Piece *p = copied(fdiv(x, 8), fdiv(y, 8));
	if (!p) return false;
	*tx = x + (p->x0 - p->sx) * 8;
	*ty = y + (p->y0 - p->sy) * 8;
	return true;
}

/* Calls fn for each source tile a copied piece takes, with the target's
 * tile offset: layer 0 where the tile's middle lies in the piece, layer 1
 * where the art it belongs to stands in it. */
typedef void (*TileFn)(const Piece *p, int stx, int sty, int layers, void *ctx);
static void copied_tiles(TileFn fn, void *ctx) {
	const int *part;
	const TownFoot *foot;
	townsrc_parts(T.book, &part, &foot);
	for (int i = 0; i < T.npieces; ++i) {
		const Piece *p = &T.piece[i];
		if (p->kind != P_COPY) continue;
		for (int sty = 0; sty < T.sth; ++sty)
			for (int stx = 0; stx < T.stw; ++stx) {
				int cx, cy, layers = 0;
				tile_cell(stx, sty, &cx, &cy);
				if (in_source(p, cx, cy)) layers |= 1;
				int k = part[sty * T.stw + stx];
				if (k >= 0 && in_source(p, foot[k].cx, foot[k].cy)) layers |= 2;
				if (layers) fn(p, stx, sty, layers, ctx);
			}
	}
}

/* A copied tile's place on a tw x th map. */
static void tile_target(const Piece *p, int stx, int sty, int tw, int th, int *tx, int *ty) {
	town_move_tile(stx, sty, T.stw, T.sth, p->x0 - p->sx, p->y0 - p->sy, tw, th, tx, ty);
}

typedef struct { int x0, x1, y0, y1; } Box;
static void box_add(Box *b, int x, int y) {
	if (x < b->x0) b->x0 = x;
	if (x > b->x1) b->x1 = x;
	if (y < b->y0) b->y0 = y;
	if (y > b->y1) b->y1 = y;
}

/* the copied tiles' extent, in pixels from the map's middle */
static void tiles_box(const Piece *p, int stx, int sty, int layers, void *ctx) {
	(void)layers;
	int dx = p->x0 - p->sx, dy = p->y0 - p->sy;
	int X = stx * 8 - T.stw * 4 + 8 * (dx + dy), Y = sty * 8 - T.sth * 4 + 4 * (dy - dx);
	box_add(ctx, X, Y);
	box_add(ctx, X + 8, Y + 8);
}

typedef struct { int tw, th, conflicts; } PinCtx;
static void pin_tile(const Piece *p, int stx, int sty, int layers, void *ctx) {
	PinCtx *c = ctx;
	int tx, ty;
	tile_target(p, stx, sty, c->tw, c->th, &tx, &ty);
	if (tx < 0 || ty < 0 || tx >= c->tw || ty >= c->th) return;
	TownPin *pin = &T.pins[ty * c->tw + tx];
	if (layers & 1) {
		if (pin->x0 < 0) { pin->x0 = (int16_t)stx; pin->y0 = (int16_t)sty; }
		else if (pin->x0 != stx || pin->y0 != sty) c->conflicts++;
	}
	if (layers & 2) {
		if (pin->x1 < 0) { pin->x1 = (int16_t)stx; pin->y1 = (int16_t)sty; }
		else if (pin->x1 != stx || pin->y1 != sty) c->conflicts++;
	}
}

static bool sec2_seen(int cx, int cy) {
	for (int i = 0; i < T.nsec2; ++i) if (T.sec2[i].x == cx * 8 && T.sec2[i].y == cy * 8) return true;
	return false;
}

/* Section 2 (people drawn behind the front layer), the jack-in and the
 * trees and statue of every copied piece. */
/* The act's other way (1, 2) whose port the original's trigger `value`
 * is; 0 none */
static int port_of(int value) {
	for (int k = 0; k < 2; ++k)
		if (value == T.style->port_src[k]) return k + 1;
	return 0;
}

static bool on_trigger(int cx, int cy);

/* A check made port `port`: the ground Lan stands on to look at it
 * (source cell (cx, cy), the piece's move dx, dy), as a check's own cells
 * lie on what it shows, where no one stands */
static void port_before(int port, int cx, int cy, int dx, int dy) {
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	for (int k = 0; k < 4 && T.ntrig < MAX_TRIG; ++k) {
		int sx = cx + d[k][0], sy = cy + d[k][1], x = (sx + dx) * 8, y = (sy + dy) * 8;
		if (!townsrc_walk(T.book, sx, sy) || on_trigger(sx + dx, sy + dy) || !ports_cell(port, x, y, 0)) continue;
		T.trig[T.ntrig++] = (CoordCell){ (int16_t)x, (int16_t)y, 0, JACK_IN_TRIGGER | 0x80, 8, 0x11 };
	}
}

/* The landmark's front, and two cells all round its statue: R jacks in
 * there (a player walks up to it from any side) */
static bool front_cell(int cx, int cy) {
	const int *f = T.style->front, *st = T.style->statue;
	return (cx >= f[0] && cy >= f[1] && cx <= f[2] && cy <= f[3]) ||
		(cx >= st[0] - 2 && cy >= st[1] - 2 && cx <= st[2] + 2 && cy <= st[3] + 2);
}

/* One of a copied piece's triggers (source cell c, the piece's move dx,
 * dy): a jack-in point kept, a check in front of a thing, or the act's
 * other way's port (town_ports.c); every point is the town's one, n = 0 */
static void carry_trigger(const Piece *p, const CoordCell *c, int dx, int dy) {
	int cx = fdiv(c->x, 8), cy = fdiv(c->y, 8);
	int port = port_of(c->value);
	bool jack = (port && c->value < CHECK_TRIGGER) || (c->value >= JACK_IN_TRIGGER && c->value < JACK_IN_TRIGGER + 0x10 &&
		(p->flags & F_JACK_IN) && (T.style->jack_ins >> (c->value - JACK_IN_TRIGGER) & 1));
	bool check = c->value >= CHECK_TRIGGER && T.lines->checks[c->value - CHECK_TRIGGER];
	bool front = front_cell(cx, cy) && townsrc_walk(T.book, cx, cy);
	if (!(jack || check) || front || !in_source(p, cx, cy) || T.ntrig >= MAX_TRIG) return;
	CoordCell o = *c;
	o.x = (int16_t)(c->x + dx * 8);
	o.y = (int16_t)(c->y + dy * 8);
	/* (marked apart from the front, whose middle the autopilot heads for;
	 * the whole cell, as the front's: AsterLand's own point was part
	 * shape 0x0C, where BN6 never jacked Lan in) */
	if (jack) { o.value = JACK_IN_TRIGGER | 0x80; o.height = 8; o.type = 0x11; }
	/* (a port's cell only where the port keeps it: else R there would
	 * take the landmark's way) */
	if (jack && port && !ports_cell(port, o.x, o.y, c->value)) return;
	T.trig[T.ntrig++] = o;
	if (port && !jack && ports_in_region(port, cx, cy)) port_before(port, cx, cy, dx, dy);
}

/* The landmark's whole front jacks in (where a player stands to, the
 * original has the statue's check), and its ring */
static void carry_ring(const Piece *p, int dx, int dy) {
	const int *f = T.style->front, *st = T.style->statue;
	int x0 = f[0] < st[0] - 2 ? f[0] : st[0] - 2, y0 = f[1] < st[1] - 2 ? f[1] : st[1] - 2;
	int x1 = f[2] > st[2] + 2 ? f[2] : st[2] + 2, y1 = f[3] > st[3] + 2 ? f[3] : st[3] + 2;
	for (int cy = y0; cy <= y1; ++cy)
		for (int cx = x0; cx <= x1; ++cx)
			if (front_cell(cx, cy) && in_source(p, cx, cy) && townsrc_walk(T.book, cx, cy) && T.ntrig < MAX_TRIG) {
				/* (the ring's own cells marked apart too: the front's middle
				 * stays where the autopilot heads) */
				bool in_front = cx >= f[0] && cy >= f[1] && cx <= f[2] && cy <= f[3];
				T.trig[T.ntrig++] = (CoordCell){ (int16_t)((cx + dx) * 8), (int16_t)((cy + dy) * 8), 0,
					in_front ? JACK_IN_TRIGGER : JACK_IN_TRIGGER | 0x80, 8, 0x11 };
			}
}

/* A copied piece's triggers: its own (carry_trigger), then the landmark's
 * front and ring */
static void carry_triggers(const Piece *p, const AreaSrc *a, int dx, int dy) {
	for (int k = 0; k < a->nsec[3]; ++k) carry_trigger(p, &a->sec[3][k], dx, dy);
	if (p->flags & F_JACK_IN) carry_ring(p, dx, dy);
}

/* The ports' names, and which cells of a check made a port are its: those
 * round its part nearest the landmark */
static void plan_ports(const AreaSrc *a) {
	ports_begin();
	const int *st = T.style->statue;
	for (int k = 0; k < 2; ++k) ports_region(a->sec[3], a->nsec[3], k + 1, T.style->port_src[k], (st[0] + st[2]) / 2, (st[1] + st[3]) / 2);
}

static void carry(void) {
	const AreaSrc *a = townsrc_area(T.book);
	const int *part;
	const TownFoot *foot;
	townsrc_parts(T.book, &part, &foot);
	T.nsec2 = T.ntrig = T.nobj = T.nwalls = T.nheights = 0;
	plan_ports(a);
	for (int i = 0; i < T.npieces; ++i) {
		const Piece *p = &T.piece[i];
		if (p->kind != P_COPY) continue;
		int dx = p->x0 - p->sx, dy = p->y0 - p->sy;
		/* (a town copied whole: its own walls and heights, as they were) */
		for (int s = 0; s < 2 && T.style->whole; ++s)
			for (int k = 0; k < a->nsec[s]; ++k) {
				const CoordCell *c = &a->sec[s][k];
				int *n = s ? &T.nheights : &T.nwalls;
				if (!in_source(p, fdiv(c->x, 8), fdiv(c->y, 8)) || *n >= MAX_WALLS) continue;
				CoordCell o = *c;
				o.x = (int16_t)(c->x + dx * 8);
				o.y = (int16_t)(c->y + dy * 8);
				(s ? T.heights : T.walls)[(*n)++] = o;
			}
		/* behind the art: the piece's own cells, and those its art covers */
		for (int k = 0; k < a->nsec[2]; ++k) {
			const CoordCell *c = &a->sec[2][k];
			int cx = fdiv(c->x, 8), cy = fdiv(c->y, 8);
			bool in = in_source(p, cx, cy);
			if (!in) {
				int X = area_px(a->tw, c->x + 4, c->y + 4), Y = area_py(a->th, c->x + 4, c->y + 4);
				for (int up = 0; up <= 8 && !in; up += 8) {
					int tx = X / 8, ty = (Y - up) / 8;
					if (tx < 0 || ty < 0 || tx >= a->tw || ty >= a->th) continue;
					int q = part[ty * a->tw + tx];
					in = q >= 0 && in_source(p, foot[q].cx, foot[q].cy);
				}
			}
			if (!in || T.nsec2 >= MAX_SEC2 || sec2_seen(cx + dx, cy + dy)) continue;
			CoordCell o = *c;
			o.x = (int16_t)((cx + dx) * 8);
			o.y = (int16_t)((cy + dy) * 8);
			T.sec2[T.nsec2++] = o;
		}
		carry_triggers(p, a, dx, dy);
		/* trees and the statue: the game's own map objects */
		for (int k = 0; k < T.nsrc_obj && T.nobj < MAX_OBJS; ++k) {
			const uint8_t *r = T.src_obj[k];
			uint32_t id = get32(r + 16);
			/* (trees, the bird statue, the Chip Trader, Green Town's
			 * knight: none an event flag hides or shows) */
			bool green = T.style->group == 0x04 && id == 0x72;   /* (Green Town's knight statue) */
			if (r[0] != OBJ_SPAWN_MAP_OBJECT || r[1] || (id != 0x7D && id != 0x7E && id != 0x16 && id != 0xBC && !green)) continue;
			int x = (int32_t)get32(r + 4) >> 16, y = (int32_t)get32(r + 8) >> 16;
			if (!in_source(p, fdiv(x, 8), fdiv(y, 8))) continue;
			uint8_t *o = T.obj[T.nobj++];
			memcpy(o, r, 20);
			put32(o + 4, (uint32_t)(x + dx * 8) << 16);
			put32(o + 8, (uint32_t)(y + dy * 8) << 16);
		}
	}
	/* (off the walkable ground, as the original's on that verge are) */
	for (int i = 0; i < T.nmore && T.nobj < MAX_OBJS; ++i) {
		uint8_t *o = T.obj[T.nobj++];
		memset(o, 0, 20);
		o[0] = OBJ_SPAWN_MAP_OBJECT;
		put32(o + 4, (uint32_t)T.more_trees[i][0] << 16);
		put32(o + 8, (uint32_t)T.more_trees[i][1] << 16);
		put32(o + 16, OBJ_TREE);
	}
}

/* (people keep off the jack-in and the checks alike) */
static bool on_trigger(int cx, int cy) {
	for (int i = 0; i < T.ntrig; ++i) if (T.trig[i].x == cx * 8 && T.trig[i].y == cy * 8) return true;
	return false;
}

/* Stretched ground repeats the original's tiles where it can: each tile of
 * a tiled piece is hinted the source tile a few whole steps back along the
 * strip (steps that keep the tile grid, and in brick and cobblestone their
 * pattern), taken when its key is the same. */
static void hint_tiles(int tw, int th) {
	for (int ty = 0; ty < th; ++ty)
		for (int tx = 0; tx < tw; ++tx) {
			TownPin *pin = &T.pins[ty * tw + tx];
			if (pin->x0 >= 0) continue;
			int a = tx * 8 + 4 - tw * 4, b = (ty * 8 + 4 - th * 4) * 2;
			int cx = fdiv(fdiv(a - b, 2), 8), cy = fdiv(fdiv(a + b, 2), 8);
			int x = cx + PLAN_R, y = cy + PLAN_R;
			if (x < 0 || y < 0 || x >= PLAN_N || y >= PLAN_N || T.owner[y][x] < 0) continue;
			const Piece *p = &T.piece[T.owner[y][x]];
			if (p->kind != P_TILE) continue;
			int sx, sy;
			source_of(p, cx, cy, &sx, &sy);
			/* the move from the strip, before the plan's own; rounded up to
			 * whole steps: 8 cells in brick and cobblestone (their
			 * pattern's), else 2 (the tile grid's), staying by the strip */
			int m = T.mat[y][x], step = m == TM_BRICK || m == TM_COBB ? 8 : 2;
			int dx = cx - sx - T.gx, dy = cy - sy - T.gy;
			dx = step * fdiv(dx + step - 1, step) + T.gx;
			dy = step * fdiv(dy + step - 1, step) + T.gy;
			pin->hx = (int16_t)(tx - (dx + dy) - (tw - T.stw) / 2);
			pin->hy = (int16_t)(ty - (dy - dx) / 2 - (th - T.sth) / 2);
		}
}

/* ---- planning ---- */

int town_style_for(uint32_t seed) {
	uint32_t keep = T.rng;
	T.rng = seed * 2246822519u + 0x165667B1u;
	int si = env_or("CYBERWORLD_TOWN_STYLE", rnd_range(0, STYLES - 1));
	T.rng = keep;
	return si < 0 || si >= STYLES ? 0 : si;
}

/* One plan and its tiles; the number of tiles no source tile matched. */
/* CYBERWORLD_TOWN_DEBUG: where the folk and the jack-in cells came to be */
static void debug_where(void) {
	for (int i = 0; i < T.lines->nfolk; ++i)
		if (T.folk_at[i][0] != 1 << 20) fprintf(stderr, "town: %d:%02x at %d,%d\n", T.lines->folk[i].cat, T.lines->folk[i].sprite, T.folk_at[i][0], T.folk_at[i][1]);
	for (int i = 0; i < T.ntrig; ++i)
		fprintf(stderr, "town: %s cell at %d,%d (%02x, z %d, %d %02x)\n", (T.trig[i].value & 0x7F) == JACK_IN_TRIGGER ? "jack-in" : "trigger",
			T.trig[i].x, T.trig[i].y, T.trig[i].value, T.trig[i].z, T.trig[i].height, T.trig[i].type);
}

static int plan_once(uint32_t seed) {
	T.rng = seed * 2246822519u + 0x165667B1u;
	/* the style, its source learned once (town_style_for the same) */
	int si = env_or("CYBERWORLD_TOWN_STYLE", rnd_range(0, STYLES - 1));
	if (si < 0 || si >= STYLES) si = 0;
	T.style = &styles[si];
	T.lines = &town_lines[si];
	if (!T.books[si] && !(T.books[si] = townsrc_learn(T.style->group, T.style->number, T.style->mat_of))) return -1;
	T.book = T.books[si];
	townsrc_size(T.book, &T.stw, &T.sth);
	T.nsrc_obj = townsrc_objects(T.book, T.src_obj, 32);
	T.npieces = 0;
	T.nmore = 0;
	T.style->design();
	/* the map's size: the floor with room for the camera, and every copied
	 * tile (roofs rise past the floor); then the whole plan moved to sit
	 * in the map's middle, by steps that keep the tile grid and the brick */
	raster();
	Box fl = { 1 << 30, -(1 << 30), 1 << 30, -(1 << 30) }, tl = fl;
	for (int cy = -PLAN_R; cy < PLAN_R; ++cy)
		for (int cx = -PLAN_R; cx < PLAN_R; ++cx) {
			if (!walkable(cx, cy)) continue;
			for (int k = 0; k < 4; ++k) {
				int x = cx * 8 + (k & 1) * 8, y = cy * 8 + (k >> 1) * 8;
				box_add(&fl, x + y, (y - x) / 2);
			}
		}
	copied_tiles(tiles_box, &tl);
	int best = 1 << 30, bgx = 0, bgy = 0, btw = 0, bth = 0;
	for (int gy = -48; gy <= 48; gy += 4)
		for (int gx = -48; gx <= 48; gx += 4) {
			if (!town_move_keeps_pattern(gx, gy)) continue;
			int du = 8 * (gx + gy), dv = 4 * (gy - gx);
			int hu = abs(fl.x0 + du) + 136, hv = abs(fl.y0 + dv) + 104;
			if (abs(fl.x1 + du) + 136 > hu) hu = abs(fl.x1 + du) + 136;
			if (abs(fl.y1 + dv) + 104 > hv) hv = abs(fl.y1 + dv) + 104;
			if (abs(tl.x0 + du) > hu) hu = abs(tl.x0 + du);
			if (abs(tl.x1 + du) > hu) hu = abs(tl.x1 + du);
			if (abs(tl.y0 + dv) > hv) hv = abs(tl.y0 + dv);
			if (abs(tl.y1 + dv) > hv) hv = abs(tl.y1 + dv);
			int tw = 2 * ((hu + 7) / 8), th = 2 * ((hv + 7) / 8);
			if ((tw & 1) != (T.stw & 1)) ++tw;
			if ((th & 1) != (T.sth & 1)) ++th;
			if (tw * th < best) { best = tw * th; bgx = gx; bgy = gy; btw = tw; bth = th; }
		}
	int tw = btw, th = bth;
	if (tw > 255 || th > 255 || (size_t)tw * th * 4 > BN6_TILEMAP_MAX) {
		fprintf(stderr, "town: %dx%d tiles is too big\n", tw, th);
		return -1;
	}
	move_all(bgx, bgy);
	T.gx = bgx;
	T.gy = bgy;
	raster();
	/* the copied tiles */
	free(T.pins);
	T.pins = malloc(sizeof(TownPin) * (size_t)tw * th);
	for (int i = 0; i < tw * th; ++i) T.pins[i] = (TownPin){ -1, -1, -1, -1, -1, -1 };
	PinCtx pc = { tw, th, 0 };
	copied_tiles(pin_tile, &pc);
	/* the front layer is the copied art only: no posts or cones picked for
	 * the stretched ground */
	for (int i = 0; i < tw * th; ++i) if (T.pins[i].x1 < 0) T.pins[i].x1 = -2;
	hint_tiles(tw, th);
	carry();
	/* Lan at his door, the jack-in's middle, the townsfolk */
	memset(&T.info, 0, sizeof T.info);
	T.info.group = T.style->group;
	T.info.number = T.style->number;
	T.info.name = T.style->name;
	T.info.landmark = T.style->landmark;
	T.info.landmark_at = T.style->landmark_at;
	int sx = T.style->start_x, sy = T.style->start_y;
	if (getenv("CYBERWORLD_TOWN_START")) sscanf(getenv("CYBERWORLD_TOWN_START"), "%d,%d", &sx, &sy);   /* (the original's world units) */
	moved(sx, sy, &T.info.start_x, &T.info.start_y);
	T.info.start_face = T.style->start_face;
	int jx = 0, jy = 0, nj = 0;
	for (int i = 0; i < T.ntrig; ++i)
		if (T.trig[i].value == JACK_IN_TRIGGER) { jx += T.trig[i].x + 4; jy += T.trig[i].y + 4; ++nj; }
	if (nj) { T.info.port_x = jx / nj; T.info.port_y = jy / nj; }
	for (int i = 0; i < T.ntrig; ++i) if (T.trig[i].value == (JACK_IN_TRIGGER | 0x80)) T.trig[i].value = JACK_IN_TRIGGER;
	for (int i = 0; i < T.lines->nfolk; ++i) {
		int fx, fy;
		T.folk_at[i][0] = T.folk_at[i][1] = 1 << 20;
		/* some people are out today (the Mr. Prog who explains the jack-in
		 * and the robot dog never) */
		bool out = T.lines->folk[i].cat == 5 && rnd_range(0, 3) == 0;
		if (out || !moved(T.lines->folk[i].x, T.lines->folk[i].y, &fx, &fy)) continue;
		/* on walkable ground, the nearest cell to where they belong */
		for (int r = 0; r <= 3 && T.folk_at[i][0] == 1 << 20; ++r)
			for (int dy = -r; dy <= r && T.folk_at[i][0] == 1 << 20; ++dy)
				for (int dx = -r; dx <= r; ++dx) {
					int cx = fdiv(fx, 8) + dx, cy = fdiv(fy, 8) + dy;
					/* (off the triggers, clear of where Lan arrives, and off the
					 * statue's approach: a kid on the squirrel's front corner
					 * stood where the arrow pointed) */
					int px = fx + dx * 8 - T.info.port_x, py = fy + dy * 8 - T.info.port_y;
					if ((abs(dx) != r && abs(dy) != r) || !walkable(cx, cy) || on_trigger(cx, cy) ||
						abs(fx + dx * 8 - T.info.start_x) + abs(fy + dy * 8 - T.info.start_y) < 24 ||
						(nj && px * px + py * py < 36 * 36)) continue;
					T.folk_at[i][0] = fx + dx * 8;
					T.folk_at[i][1] = fy + dy * 8;
					break;
				}
	}
	for (int i = 0; i < TOWN_SPOTS; ++i) moved(T.style->look[i][0], T.style->look[i][1], &T.info.spots[i][0], &T.info.spots[i][1]);
	free(T.tiles);
	free(T.miss);
	T.tiles = calloc((size_t)tw * th * 2, 2);
	T.miss = calloc((size_t)tw * th, 1);
	TownSynthStats st;
	townsrc_synth(T.book, plan_at, tw, th, seed, T.pins, T.tiles, T.miss, &st);
	T.info.tw = tw;
	T.info.th = th;
	T.info.picks = st.picks;
	T.info.misses = st.near;
	if (getenv("CYBERWORLD_TOWN_DEBUG")) {
		debug_where();
		fprintf(stderr, "town: style %d:%d, %dx%d tiles, moved (%d, %d), %d picked (%d hinted, %d coherent, %d near), %d conflicts, %d behind-art cells, %d trigger cells (%d jack-in), %d objects\n",
			T.style->group, T.style->number, tw, th, bgx, bgy, st.picks, st.hinted, st.coherent, st.near, pc.conflicts, T.nsec2, T.ntrig, nj, T.nobj);
	}
	return st.near;
}

bool town_plan(uint32_t seed) {
	return plan_once(seed) >= 0;
}

const TownInfo *town_info(void) { return &T.info; }
const uint8_t *town_misses(void) { return T.miss; }

bool town_route(int x, int y, int *wx, int *wy) { return town_walk(x, y, 3, wx, wy, NULL); }

bool town_walk(int x, int y, int steps, int *wx, int *wy, int *cells) {
	/* breadth first from the jack-in's middle over walkable cells; then
	 * `steps` cells along the way from Lan's */
	static int16_t dist[PLAN_N][PLAN_N];
	static int16_t q[PLAN_N * PLAN_N];
	int gx = fdiv(T.info.port_x, 8) + PLAN_R, gy = fdiv(T.info.port_y, 8) + PLAN_R;
	int sx = fdiv(x, 8) + PLAN_R, sy = fdiv(y, 8) + PLAN_R;
	if (gx < 0 || gy < 0 || gx >= PLAN_N || gy >= PLAN_N || sx < 0 || sy < 0 || sx >= PLAN_N || sy >= PLAN_N) return false;
	for (int i = 0; i < PLAN_N; ++i) for (int j = 0; j < PLAN_N; ++j) dist[i][j] = -1;
	int h = 0, t = 0;
	dist[gy][gx] = 0;
	q[t++] = (int16_t)(gy * PLAN_N + gx);
	static const int d[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	while (h < t) {
		int c = q[h++], cx = c % PLAN_N, cy = c / PLAN_N;
		for (int k = 0; k < 4; ++k) {
			int nx = cx + d[k][0], ny = cy + d[k][1];
			if (nx < 0 || ny < 0 || nx >= PLAN_N || ny >= PLAN_N || dist[ny][nx] >= 0 || !T.walk[ny][nx]) continue;
			dist[ny][nx] = (int16_t)(dist[cy][cx] + 1);
			q[t++] = (int16_t)(ny * PLAN_N + nx);
		}
	}
	if (dist[sy][sx] < 0) return false;
	if (cells) *cells = dist[sy][sx];
	/* (at the goal's cell: the point itself, which may be a cell's corner) */
	if (!dist[sy][sx]) { *wx = T.info.port_x; *wy = T.info.port_y; return true; }
	int cx = sx, cy = sy;
	for (int step = 0; step < steps && dist[cy][cx] > 0; ++step)
		for (int k = 0; k < 4; ++k) {
			int nx = cx + d[k][0], ny = cy + d[k][1];
			if (nx >= 0 && ny >= 0 && nx < PLAN_N && ny < PLAN_N && dist[ny][nx] >= 0 && dist[ny][nx] < dist[cy][cx]) { cx = nx; cy = ny; break; }
		}
	*wx = (cx - PLAN_R) * 8 + 4;
	*wy = (cy - PLAN_R) * 8 + 4;
	return true;
}

bool town_home_spot(int *x, int *y, int *face) {
	int cells = 0;
	/* (where no walk on one floor joins them, as Green Town's heights
	 * part its start from its knight: where Lan starts) */
	if (!town_walk(T.info.start_x, T.info.start_y, 0, x, y, &cells) ||
		!town_walk(T.info.start_x, T.info.start_y, cells > 2 ? cells - 2 : 0, x, y, NULL)) {
		*x = T.info.start_x;
		*y = T.info.start_y;
		*face = T.info.start_face;
		return true;
	}
	/* (the game's facings: 1 +x, 3 +y, 5 -x, 7 -y) */
	int dx = T.info.port_x - *x, dy = T.info.port_y - *y;
	*face = abs(dx) > abs(dy) ? (dx > 0 ? 1 : 5) : (dy > 0 ? 3 : 7);
	return true;
}

uint32_t *town_render(int *w, int *h) {
	if (!T.tiles) return NULL;
	*w = T.info.tw * 8;
	*h = T.info.th * 8;
	return area_src_render(0, T.style->group, T.style->number, T.tiles, T.info.tw, T.info.th);
}

int town_triggers(const CoordCell **cells) {
	*cells = T.trig;
	return T.ntrig;
}

bool town_after_abandon;

bool town_on_port(int x, int y) {
	for (int i = 0; i < T.ntrig; ++i) {
		const CoordCell *c = &T.trig[i];
		if ((c->value & 0x7F) == JACK_IN_TRIGGER && ports_value(c->x, c->y, JACK_IN_TRIGGER) == JACK_IN_TRIGGER &&
			x >= c->x && y >= c->y && x < c->x + 8 && y < c->y + 8) return true;
	}
	return false;
}

int town_port_near(int x, int y, int *px, int *py) {
	int best = -1;
	for (int i = 0; i < T.ntrig; ++i) {
		const CoordCell *c = &T.trig[i];
		if ((c->value & 0x7F) != JACK_IN_TRIGGER) continue;
		int dx = c->x + 4 - x, dy = c->y + 4 - y, d = dx * dx + dy * dy;
		if (best < 0 || d < best) { best = d; *px = c->x + 4; *py = c->y + 4; }
	}
	return best;
}

void town_objects(void (*fn)(int id, int x, int y, void *ctx), void *ctx) {
	for (int i = 0; i < T.nobj; ++i) fn((int)get32(T.obj[i] + 16), (int32_t)get32(T.obj[i] + 4) >> 16, (int32_t)get32(T.obj[i] + 8) >> 16, ctx);
	for (int i = 0; i < T.lines->nfolk; ++i) if (T.folk_at[i][0] != 1 << 20) fn(-1, T.folk_at[i][0], T.folk_at[i][1], ctx);
}

/* ---- in the game ---- */

/* The face a townsperson talks with: a person of sprite list 5 has the
 * face of its sprite less 0x20, the port guide is a Mr. Prog, the robot
 * dog has none. */
static int folk_face(const Folk *f) {
	if (f->cat == 5) return f->sprite - 0x20;
	if (f->cat == 7 && f->sprite == 0x0F) return FACE_PROG;
	return FACE_NONE;
}

/* Compressed sprites only draw once the map loads them. */
static void need_sprite(NpcList *npcs, int category, int index) {
	uint32_t list = emu_read32(0x08000000u + R.layout->sprite_lists + (uint32_t)category * 4);
	if (!(emu_read32(list + (uint32_t)index * 4) & 0x80000000u)) return;
	for (int i = 0; i < npcs->nsprites; ++i)
		if (npcs->sprite_idx[i] == index && npcs->sprite_cat[i] == category * 4) return;
	if (npcs->nsprites >= MAPSLOT_SPRITES) return;
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
	/* walls around the walkable cells, the cells behind the art, the jack-in */
	/* (a closed port's ground before a check no trigger) */
	static CoordCell trig[MAX_TRIG];
	int ntrig = 0;
	for (int i = 0; i < T.ntrig; ++i) {
		trig[ntrig] = T.trig[i];
		trig[ntrig].value = (uint8_t)ports_value(T.trig[i].x, T.trig[i].y, T.trig[i].value);
		if (trig[ntrig].value) ++ntrig;
	}
	CoordExtra extra = { { T.walls, T.heights, T.sec2, trig }, { T.nwalls, T.nheights, T.nsec2, ntrig } };
	if (!(T.style->whole ? coords_write_raw(coord_slot, &extra) : coords_write_town(coord_slot, walkable, &extra))) return false;
	/* people and the trees, in the town's own space */
	mapslot_town(true);
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	static TextArchive text;
	ta_begin(&text);
	int script[MAX_FOLK];
	for (int i = 0; i < T.lines->nfolk; ++i) script[i] = ta_talk(&text, T.lines->folk[i].words, folk_face(&T.lines->folk[i]));
	/* what Lan and MegaMan say when they step out (the director runs it) */
	T.info.intro = ta_talk(&text, town_intro(T.style->landmark_at, T.lines->arrival), FACE_MEGAMAN);
	uint32_t archive = ta_commit(&text);
	T.info.talk_archive = archive;
	if (emu_debug_on()) fprintf(stderr, "town text hash %08x\n", ta_hash(&text));
	for (int i = 0; i < T.lines->nfolk && npcs.n < 16; ++i) {
		if (T.folk_at[i][0] == 1 << 20) continue;
		const Folk *f = &T.lines->folk[i];
		need_sprite(&npcs, f->cat, f->sprite);
		npcs.script[npcs.n++] = f->walk ? npc_walker(f->cat, f->sprite, T.folk_at[i][0], T.folk_at[i][1], f->face, f->walk, archive, script[i]) :
			npc_talker(f->cat, f->sprite, T.folk_at[i][0], T.folk_at[i][1], 0, f->face, archive, script[i], -1, false);
	}
	/* the checks' words: the map's own text archive */
	TextArchive words;
	ta_begin(&words);
	uint8_t check_script[16];
	memset(check_script, 0xFF, sizeof check_script);
	for (int n = 0; n < 16; ++n)
		if (T.lines->checks[n]) check_script[n] = (uint8_t)ta_talk(&words, T.lines->checks[n], FACE_NONE);
	static uint8_t archive_bytes[TEXT_ARCHIVE_MAX];
	int archive_len = ta_build(&words, archive_bytes);
	if (emu_debug_on()) fprintf(stderr, "town checks hash %08x\n", ta_hash(&words));
	/* trees and the statue: the game's own objects (20-byte spawn records) */
	uint8_t objs[(MAX_OBJS + 1) * 20];
	for (int i = 0; i < T.nobj; ++i) {
		memcpy(objs + i * 20, T.obj[i], 20);
		uint32_t id = get32(T.obj[i] + 16) & 0xFFFF;
		const uint8_t *e = R.data + OW_MAP_OBJECTS + id * 16;
		need_sprite(&npcs, e[0] / 4, e[1]);
	}
	memset(objs + T.nobj * 20, 0, 4);
	objs[T.nobj * 20] = 0xFF;
	npcs.objects = mapslot_alloc(objs, T.nobj * 20 + 4);
	if (getenv("CYBERWORLD_TOWN_DEBUG")) fprintf(stderr, "town: %d objects at %08x, %d people, %d sprites loaded\n", T.nobj, npcs.objects, npcs.n, npcs.nsprites);
	int g = T.style->group, n = T.style->number;
	bool ok = archive && mapslot_install(g, n, &npcs, NULL, 0) &&
		mapslot_town_warps(g, n, T.info.start_x, T.info.start_y, T.info.start_face) &&
		mapslot_checks(g, n, check_script, archive_bytes, archive_len) &&
		mapslot_jack_in(g, n, to_group, to_number, x, y, 4) &&
		mapslot_music(g, n, T.style->song);
	mapslot_town(false);
	return ok;
}

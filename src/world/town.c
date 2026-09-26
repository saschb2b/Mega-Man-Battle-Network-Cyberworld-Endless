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

#include "area_src.h"
#include "bn6.h"
#include "bytes.h"
#include "coords.h"
#include "emu.h"
#include "lz.h"
#include "mapslot.h"
#include "npc.h"
#include "rom.h"
#include "save.h"
#include "text.h"
#include "townmath.h"
#include "townsrc.h"

#define TOWN_TILEMAP_AT (EMU_FREE + 0x100000) /* the town's tile map (LZ77) */

#define PLAN_R 64                 /* the plan spans cells -PLAN_R .. PLAN_R - 1 */
#define PLAN_N (2 * PLAN_R)

#define MAX_PIECES 48
#define MAX_OBJS 40
#define MAX_SEC2 2048
#define MAX_TRIG 160
#define MAX_FOLK 14   /* of the game's 16 NPCs */

/* The jack-in trigger: 0x40 is the map's jack-in destination 0. */
#define JACK_IN_TRIGGER 0x40
/* check triggers: 0xF0 + n shows the map's check n when A is pressed */
#define CHECK_TRIGGER 0xF0
/* a spawn record's first byte picks the spawner: 5 places a map object */
#define OBJ_SPAWN_MAP_OBJECT 5
#define OBJ_TREE 0x7D
/* OverworldMapObjects: 16-byte entries (category, sprite, animation, ...) per object */
#define OW_MAP_OBJECTS 0x0A4F24u

/* Central Town's facings (the game's: 1 +x, 3 +y, 5 -x, 7 -y) */
enum { FACE_NE = 1, FACE_SE = 3, FACE_SW = 5, FACE_NW = 7 };

enum { P_COPY, P_TILE };
enum { F_JACK_IN = 1 };

typedef struct {
	uint8_t kind, flags;
	int x0, y0, x1, y1;   /* target cells, inclusive */
	int sx, sy;           /* the source cell at (x0, y0) */
	int sw, sh;           /* P_TILE: the source strip, repeated */
} Piece;

/* Someone in the town: where (source world units, moved with the piece
 * there), which way, what they say, and how far they pace that way and
 * back (0: they stand). */
typedef struct {
	int x, y, face, cat, sprite;
	const char *words;
	int walk;
} Folk;

typedef struct TownStyle Style;

static struct {
	const Style *style;
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
	CoordCell sec2[MAX_SEC2], trig[MAX_TRIG];
	int nsec2, ntrig;
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
	TownMatOf mat_of;
	void (*design)(void);
	int start_x, start_y, start_face;   /* where Lan arrives */
	const Folk *folk;
	int nfolk;
	const char *const *checks;          /* what check 0xF0 + n says */
	int look[TOWN_SPOTS][2];            /* places worth a look */
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

/* Its people: at the statue, the shop, the school gate, the bus stop, the
 * houses, the closed road to the Expo. */
static const Folk central_folk[] = {
	{ 108, -68, FACE_SW, 5, 0x36, "Did you hear? The port by the bird statue opens into a brand new net!" },
	{ 100, -52, 0, 7, 0x11, "Woof! Woof!" },   /* (the robot dog: one animation) */
	{ 60, -96, FACE_SE, 7, 0x0F, "I'M THE PLAZA'S PORT GUIDE PROGRAM!|STAND BEFORE THE STATUE AND PRESS R TO JACK IN!" },
	{ 44, -20, FACE_NE, 5, 0x2E, "I jacked in there yesterday. Today the paths were all different!|It really does go on forever down there." },
	{ -172, -4, FACE_NW, 5, 0x2C, "Aster Land just got new chips in! I could look at them all day." },
	{ -108, -36, FACE_NE, 5, 0x2B, "My dad parks here every Sunday. Aster Land is the best!" },
	{ 130, 150, FACE_NE, 5, 0x2F, "The bus should be here any minute now..." },
	{ 84, -180, FACE_SE, 5, 0x34, "Hey, Lan! No class today...|Are you diving into the Endless Net too?" },
	{ -164, 196, FACE_NE, 5, 0x38, "Heading out, Lan? Be careful on the net!" },
	{ 18, 290, FACE_SW, 5, 0x39, "The flowers on this street are something, aren't they?" },
	{ -146, -184, FACE_SE, 5, 0x3A, "The road to the Expo site is closed today.|Such a shame. I wanted to see the pavilions." },
	{ 132, 180, FACE_SE, 5, 0x2D, "Nothing beats a walk after a long day at the lab.", 10 },
	{ -12, -164, FACE_NE, 5, 0x28, "Do you go to the Cyber Academy too? I'm late for NetBattle club!", 10 },
};

/* What its checks (triggers 0xF0 + n, in front of each thing) say: the
 * houses (Lan's, the pink one, the orange one, the gray pair), the flower
 * bed, the bus stop, Aster Land's door, the Expo gates' signs and road, the
 * Academy's gate, the statue, Aster Land's window. */
static const char *const central_checks[16] = {
	"@L Home sweet home. Mom's making curry tonight!|@M Then let's be back in time for dinner, Lan!",
	"A pink house. The curtains are drawn.",
	"Someone is watering the plants on the roof terrace.",
	"Two gray houses, side by side. It's quiet in there.",
	"The flowers are in full bloom.",
	"The bus stop.|\"Next bus: ACDC Town\"",
	"@M Aster Land! Let's go shopping later, Lan. The net's waiting!",
	"EXPO\nThe sign lists the pavilions on show.",
	"Cyber Academy. The gate is closed for the day.",
	"A statue of a blue bird. Its port leads into the Endless Net.",
	"The road to the Expo site. It's closed off today.",
	"EXPO\nA map of the site. It's huge!",
	"Chips and PETs line the shelves in the window.",
	NULL, NULL, NULL,
};

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

/* Its people: kids in the park, the chip shop, the Metroline, the
 * mansion, the houses, the promenade. */
static const Folk acdc_folk[] = {
	{ -196, -20, FACE_SE, 5, 0x2B, "Meet you at the squirrel! Last one there's a Mettaur!" },
	{ -180, -4, FACE_NW, 5, 0x34, "Lan! You came all the way from Central Town?|The squirrel's port goes into the Endless Net too!" },
	{ -204, -4, FACE_NE, 5, 0x37, "Squirrel! Squirrel!" },
	{ -180, -44, FACE_SE, 7, 0x0F, "I'M THE PARK'S PORT GUIDE PROGRAM!|STAND BESIDE THE SQUIRREL STATUE AND PRESS R TO JACK IN!" },
	{ -188, -92, FACE_NE, 5, 0x2D, "Higsby's got rare chips in... but have you seen the prices?|Guess I'll keep saving my Zenny." },
	{ 4, -132, FACE_SW, 5, 0x30, "The Metroline runs straight to Central Town. So handy!" },
	{ 252, -28, FACE_SW, 5, 0x2E, "That's the Ayano mansion. They say it has its own garden inside!" },
	{ 100, 36, FACE_SE, 5, 0x36, "Nobody lives in that house anymore...|But somebody still waters the flowers." },
	{ 60, 164, FACE_SW, 5, 0x39, "A fine breeze today. Just right for a stroll down the avenue." },
	{ 124, -84, FACE_SW, 5, 0x38, "The boy who lives here practices NetBattles day and night. So noisy!" },
	{ -60, 164, FACE_NE, 5, 0x2C, "Every morning I walk the promenade, then dive a few layers!", 12 },
};

static const char *const acdc_checks[16] = {
	"@L Our old house... It feels like only yesterday we lived here.|@M We had so many adventures in ACDC Town, Lan.",
	"The hedge is neatly trimmed.",
	"Mayl's house. Piano music drifts out of the window.|@M Mayl's practicing again, Lan!",
	"The squirrel statue! Its port leads into the Endless Net.",
	"Higsby's chip shop.|\"Rare chips in stock!\"",
	"A blue house. The mailbox says \"Oyama.\"|@L Dex is probably NetBattling again...",
	"A tall wall runs around the Ayano mansion.",
	"The Ayano mansion. The gate is shut tight.",
	"A Chip Trader. It's out of order today.",
	NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

#define FOLK(list) list, (int)(sizeof list / sizeof *list)

static const Style styles[] = {
	{ 0x01, 0x00, 0x03, 1 << 0, { 12, -9, 16, -2 }, central_mat, design_central, -40, 266, FACE_SW, FOLK(central_folk), central_checks,
	  { { -40, 266 }, { 100, -30 }, { -150, -30 }, { 90, -150 }, { 110, 110 }, { -150, -160 } } },
	{ 0x00, 0x00, 0x24, 1 << 0 | 1 << 1, { -23, -10, -17, -3 }, acdc_mat, design_acdc, -60, -108, FACE_SW, FOLK(acdc_folk), acdc_checks,
	  { { -60, -108 }, { -190, -30 }, { -190, -120 }, { 110, -120 }, { 260, -60 }, { 60, 120 } } },
};
#define STYLES ((int)(sizeof styles / sizeof *styles))
_Static_assert(sizeof central_folk / sizeof *central_folk <= MAX_FOLK && sizeof acdc_folk / sizeof *acdc_folk <= MAX_FOLK, "at most MAX_FOLK townsfolk");

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
static void carry(void) {
	const AreaSrc *a = townsrc_area(T.book);
	const int *part;
	const TownFoot *foot;
	townsrc_parts(T.book, &part, &foot);
	T.nsec2 = T.ntrig = T.nobj = 0;
	for (int i = 0; i < T.npieces; ++i) {
		const Piece *p = &T.piece[i];
		if (p->kind != P_COPY) continue;
		int dx = p->x0 - p->sx, dy = p->y0 - p->sy;
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
		/* the jack-in points and the checks in front of things; the
		 * landmark's whole front jacks in (where a player stands to, the
		 * original has the statue's check), and every point is the town's
		 * one: n = 0 */
		const int *f = T.style->front;
		for (int k = 0; k < a->nsec[3]; ++k) {
			const CoordCell *c = &a->sec[3][k];
			int cx = fdiv(c->x, 8), cy = fdiv(c->y, 8);
			bool jack = c->value >= JACK_IN_TRIGGER && c->value < JACK_IN_TRIGGER + 0x10 && (p->flags & F_JACK_IN) &&
				(T.style->jack_ins >> (c->value - JACK_IN_TRIGGER) & 1);
			bool check = c->value >= CHECK_TRIGGER && T.style->checks[c->value - CHECK_TRIGGER];
			bool front = cx >= f[0] && cy >= f[1] && cx <= f[2] && cy <= f[3];
			if (!(jack || check) || front || !in_source(p, cx, cy) || T.ntrig >= MAX_TRIG) continue;
			CoordCell o = *c;
			o.x = (int16_t)(c->x + dx * 8);
			o.y = (int16_t)(c->y + dy * 8);
			/* (marked apart from the front, whose middle the autopilot heads for) */
			if (jack) o.value = JACK_IN_TRIGGER | 0x80;
			T.trig[T.ntrig++] = o;
		}
		for (int cy = f[1]; cy <= f[3] && (p->flags & F_JACK_IN); ++cy)
			for (int cx = f[0]; cx <= f[2]; ++cx)
				if (in_source(p, cx, cy) && townsrc_walk(T.book, cx, cy) && T.ntrig < MAX_TRIG)
					T.trig[T.ntrig++] = (CoordCell){ (int16_t)((cx + dx) * 8), (int16_t)((cy + dy) * 8), 0, JACK_IN_TRIGGER, 8, 0x11 };
		/* trees and the statue: the game's own map objects */
		for (int k = 0; k < T.nsrc_obj && T.nobj < MAX_OBJS; ++k) {
			const uint8_t *r = T.src_obj[k];
			uint32_t id = get32(r + 16);
			/* (trees, the bird statue, the Chip Trader: none an event
			 * flag hides or shows) */
			if (r[0] != OBJ_SPAWN_MAP_OBJECT || r[1] || (id != 0x7D && id != 0x7E && id != 0x16 && id != 0xBC)) continue;
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

/* One plan and its tiles; the number of tiles no source tile matched. */
static int plan_once(uint32_t seed) {
	T.rng = seed * 2246822519u + 0x165667B1u;
	/* the style, its source learned once */
	int si = env_or("CYBERWORLD_TOWN_STYLE", rnd_range(0, STYLES - 1));
	if (si < 0 || si >= STYLES) si = 0;
	T.style = &styles[si];
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
	int sx = T.style->start_x, sy = T.style->start_y;
	if (getenv("CYBERWORLD_TOWN_START")) sscanf(getenv("CYBERWORLD_TOWN_START"), "%d,%d", &sx, &sy);   /* (the original's world units) */
	moved(sx, sy, &T.info.start_x, &T.info.start_y);
	T.info.start_face = T.style->start_face;
	int jx = 0, jy = 0, nj = 0;
	for (int i = 0; i < T.ntrig; ++i)
		if (T.trig[i].value == JACK_IN_TRIGGER) { jx += T.trig[i].x + 4; jy += T.trig[i].y + 4; ++nj; }
	if (nj) { T.info.port_x = jx / nj; T.info.port_y = jy / nj; }
	for (int i = 0; i < T.ntrig; ++i) if (T.trig[i].value == (JACK_IN_TRIGGER | 0x80)) T.trig[i].value = JACK_IN_TRIGGER;
	for (int i = 0; i < T.style->nfolk; ++i) {
		int fx, fy;
		T.folk_at[i][0] = T.folk_at[i][1] = 1 << 20;
		/* some people are out today (the Mr. Prog who explains the jack-in
		 * and the robot dog never) */
		bool out = T.style->folk[i].cat == 5 && rnd_range(0, 3) == 0;
		if (out || !moved(T.style->folk[i].x, T.style->folk[i].y, &fx, &fy)) continue;
		/* on walkable ground, the nearest cell to where they belong */
		for (int r = 0; r <= 3 && T.folk_at[i][0] == 1 << 20; ++r)
			for (int dy = -r; dy <= r && T.folk_at[i][0] == 1 << 20; ++dy)
				for (int dx = -r; dx <= r; ++dx) {
					int cx = fdiv(fx, 8) + dx, cy = fdiv(fy, 8) + dy;
					/* (off the triggers, and clear of where Lan arrives) */
					if ((abs(dx) != r && abs(dy) != r) || !walkable(cx, cy) || on_trigger(cx, cy) ||
						abs(fx + dx * 8 - T.info.start_x) + abs(fy + dy * 8 - T.info.start_y) < 24) continue;
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
	if (getenv("CYBERWORLD_TOWN_DEBUG"))
		for (int i = 0; i < T.style->nfolk; ++i)
			if (T.folk_at[i][0] != 1 << 20) fprintf(stderr, "town: %d:%02x at %d,%d\n", T.style->folk[i].cat, T.style->folk[i].sprite, T.folk_at[i][0], T.folk_at[i][1]);
	if (getenv("CYBERWORLD_TOWN_DEBUG"))
		fprintf(stderr, "town: style %d:%d, %dx%d tiles, moved (%d, %d), %d picked (%d hinted, %d coherent, %d near), %d conflicts, %d behind-art cells, %d trigger cells (%d jack-in), %d objects\n",
			T.style->group, T.style->number, tw, th, bgx, bgy, st.picks, st.hinted, st.coherent, st.near, pc.conflicts, T.nsec2, T.ntrig, nj, T.nobj);
	return st.near;
}

bool town_plan(uint32_t seed) {
	return plan_once(seed) >= 0;
}

const TownInfo *town_info(void) { return &T.info; }
const uint16_t *town_tiles(void) { return T.tiles; }
const uint8_t *town_misses(void) { return T.miss; }
bool town_walkable(int cx, int cy) { return walkable(cx, cy); }

bool town_route(int x, int y, int *wx, int *wy) {
	/* breadth first from the jack-in's middle over walkable cells; then a
	 * few cells along the way from Lan's */
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
	/* (at the goal's cell: the point itself, which may be a cell's corner) */
	if (!dist[sy][sx]) { *wx = T.info.port_x; *wy = T.info.port_y; return true; }
	int cx = sx, cy = sy;
	for (int step = 0; step < 3 && dist[cy][cx] > 0; ++step)
		for (int k = 0; k < 4; ++k) {
			int nx = cx + d[k][0], ny = cy + d[k][1];
			if (nx >= 0 && ny >= 0 && nx < PLAN_N && ny < PLAN_N && dist[ny][nx] >= 0 && dist[ny][nx] < dist[cy][cx]) { cx = nx; cy = ny; break; }
		}
	*wx = (cx - PLAN_R) * 8 + 4;
	*wy = (cy - PLAN_R) * 8 + 4;
	return true;
}

uint32_t *town_render(int *w, int *h) {
	if (!T.tiles) return NULL;
	*w = T.info.tw * 8;
	*h = T.info.th * 8;
	return area_src_render(T.style->group, T.style->number, T.tiles, T.info.tw, T.info.th);
}

int town_triggers(const CoordCell **cells) {
	*cells = T.trig;
	return T.ntrig;
}

void town_objects(void (*fn)(int id, int x, int y, void *ctx), void *ctx) {
	for (int i = 0; i < T.nobj; ++i) fn((int)get32(T.obj[i] + 16), (int32_t)get32(T.obj[i] + 4) >> 16, (int32_t)get32(T.obj[i] + 8) >> 16, ctx);
	for (int i = 0; i < T.style->nfolk; ++i) if (T.folk_at[i][0] != 1 << 20) fn(-1, T.folk_at[i][0], T.folk_at[i][1], ctx);
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

/* What Lan and MegaMan say as a run begins: on the first dive ever, Dad's
 * call about the Endless Net; after that, a word about the last one. */
static const char *intro(void) {
	static char buf[900];
	bool acdc = T.style->group == 0x00;
	const char *port = acdc ? "squirrel statue in the park" : "bird statue on the plaza";
	int k = 0;
	#define ADD(...) (k += snprintf(buf + k, k < (int)sizeof buf ? sizeof buf - (size_t)k : 0, __VA_ARGS__))
	if (acdc) ADD("@M The Metroline got us to ACDC Town in no time, Lan!|");
	if (!profile.seen_intro) {
		ADD("@D Lan, it's Dad. Have you got a minute?|"
			"@D A new stretch of net just opened up under town. Its paths change every time someone jacks in.|"
			"@D And it only goes down. The Navis are calling it the Endless Net.|"
			"@M The Endless Net... Lan, that sounds like an adventure!|"
			"@D Everything down there is copied data. Nothing you find will come back out with you.|"
			"@D Something at the very bottom is doing the copying. We're calling it the Nest.|"
			"@D And if MegaMan gets deleted, my backup program will bring him home safe.|"
			"@D So dive as deep as you can, and send me your readings!|"
			"@L Leave it to us, Dad!|"
			"@M The port's by the %s. Let's go, Lan!", port);
	} else if (profile.nest_clears > 0 && profile.runs % 2) {
		ADD("@D Lan, MegaMan has reached the Nest before. The net's changed all over again since then.|"
			"@D Be careful down there!|@L Got it, Dad!");
	} else if (profile.runs == 0) {
		/* (the call heard, but no run over yet: no best to speak of) */
		ADD("@L The Endless Net again... I wonder what's changed down there.|@M Let's find out, Lan! The port's by the %s.", port);
	} else {
		switch (profile.runs % 3) {
		case 0: ADD("@M Ready for another dive, Lan? Our best is layer %d!|@L This time we'll go even deeper!", profile.best_depth); break;
		case 1: ADD("@M Dad's backup got me home safe last time.|@L Good! Let's beat layer %d today!", profile.best_depth); break;
		default: ADD("@L The Endless Net again... I wonder what's changed down there.|@M Let's find out, Lan! The port's by the %s.", port); break;
		}
	}
	#undef ADD
	return buf;
}

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
	/* walls around the walkable cells, the cells behind the art, the jack-in */
	CoordExtra extra = { { NULL, NULL, T.sec2, T.trig }, { 0, 0, T.nsec2, T.ntrig } };
	if (!coords_write_town(coord_slot, walkable, &extra)) return false;
	/* people and the trees, in the town's own space */
	mapslot_town(true);
	NpcList npcs;
	memset(&npcs, 0, sizeof npcs);
	static TextArchive text;
	ta_begin(&text);
	int script[MAX_FOLK];
	for (int i = 0; i < T.style->nfolk; ++i) script[i] = ta_talk(&text, T.style->folk[i].words, folk_face(&T.style->folk[i]));
	/* what Lan and MegaMan say when they step out (the director runs it) */
	T.info.intro = ta_talk(&text, intro(), FACE_MEGAMAN);
	uint32_t archive = ta_commit(&text);
	T.info.talk_archive = archive;
	for (int i = 0; i < T.style->nfolk && npcs.n < 16; ++i) {
		if (T.folk_at[i][0] == 1 << 20) continue;
		const Folk *f = &T.style->folk[i];
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
		if (T.style->checks[n]) check_script[n] = (uint8_t)ta_talk(&words, T.style->checks[n], FACE_NONE);
	static uint8_t archive_bytes[TEXT_ARCHIVE_MAX];
	int archive_len = ta_build(&words, archive_bytes);
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

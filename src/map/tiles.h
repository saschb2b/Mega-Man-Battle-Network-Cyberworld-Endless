/* Floor tiles learned from an original map, by class: where a tile's centre
 * falls inside a panel and which of the 3x3 panels around it are floor
 * (see tiles.c). */
#ifndef CW_TILES_H
#define CW_TILES_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "area_src.h"
#include "seams.h"

#define TILE_PLAIN 4   /* plain floor looks kept per phase */

/* Floor materials: the area's platform floor and its walkway floor. */
enum { TILE_VOID, TILE_A, TILE_B };
/* With a floor: this panel is a pad (a small platform on a spur), drawn in
 * the pads' own look where the original has one. */
#define TILE_PAD 4
/* ... it belongs to a piece drawn apart from the rest (tilemap.c). */
#define TILE_APART 8
#define TILE_MATERIAL(m) ((m) & 3)

/* One pair of layer entries seen for a class (phase, then the platform and
 * walkway panels around), how
 * often, which of its 64 pixels are drawn (bit y * 8 + x) and their
 * colours (BGR555). */
typedef struct {
	uint32_t key;
	uint16_t e0, e1;
	uint32_t count;
	uint8_t pad;          /* seen on a pad of the original */
	uint8_t other;        /* draws colours the area's own map never shows on its floors */
	uint64_t mask;
	uint16_t px[64];
} TileCand;

/* A class's pairs: from its first to the next class's. */
typedef struct { uint32_t key; int first; } TileClass;

/* Sorted by key and then most common first. */
typedef struct {
	TileCand *cand;
	int n;
	TileClass *cls;                     /* its classes in order (a pick passes a class whole) */
	int ncls;
	int dv, face, hang;                 /* how the floor is drawn, see TileGrid */
	int joins;                          /* pairs where the two floors meet */
	int plain[2][64][TILE_PLAIN], nplain[2][64];   /* per material and phase: the floor's usual looks */
	uint32_t *shapes;                   /* the panels' neighbourhoods seen (TILE_SHAPE), sorted */
	int nshapes;
	/* the platform floor's middle as the original lays it, period `pa` x
	 * `pb` panels: per panel of the period and phase, its pair (an index
	 * into cand, -1 none). A field of Mr. Weather Comp's solar panels ran
	 * as bands its whole length where each tile took its phase's most
	 * common pair. */
	int16_t *patch;
	int pa, pb;
	/* ... where it has no period: per phase, its pairs whose look keeps to
	 * the plain one's along the tile's edges (a light or a crack inside the
	 * tile), with how often the original draws each, to scatter as it
	 * does (Mr. Weather Comp's solar panels' lights) */
	TileCand *vary;
	uint32_t *vary_weight;
	int vary_first[65];
} TileBook;

/* A tile map's size, where its panel edges fall (world units mod 32), how
 * many pixels below them the floor is drawn, how tall the side faces under
 * its bottom edges are and how far below those edges things may hang, such
 * as the pedestals under a homepage's panels (from TileBook). */
typedef struct {
	int tw, th, ex, ey, dv, face, hang;
	bool rimmed;       /* floor edges are rims, not plain floor (areas told by shape, TILES_RIMMED) */
	bool apart;        /* the TILE_APART pieces are drawn as if no other floor touched them (tilemap.c) */
	bool single;       /* pairs that draw on one layer only */
	int side;          /* how far down beside a side face a tile may draw (0: the face's height) */
} TileGrid;

/* World panel (A, B)'s floor: TILE_VOID, TILE_A or TILE_B, maybe | TILE_PAD
 * and | TILE_APART. */
typedef int (*TileFloor)(int A, int B, const void *ctx);

/* Learns the tiles of the map's panels whose middle has a hue in `styles`
 * (platforms) or `walk_styles` (walkways; 0 for none), or with
 * TILES_BY_SHAPE, platform floor what lies in 2 x 2 blocks of floor with
 * floor all around, and walkway the rest (walkways and platforms' rims),
 * as a generated layer tells them; but none beside a panel with a hue in
 * `skip_styles`, floor still in the neighbourhoods seen (with
 * SKIP_ANY_PIXEL, a panel whose top shows those hues anywhere, not only in
 * its middle: Seaside's arrow panels, orange chevrons in a pink frame,
 * whose pieces its boardwalks' joins took). */
#define SKIP_ANY_PIXEL 0x8000
/* ... or with SKIP_PALE, a platform panel whose top is pale in places
 * (bright, weakly coloured): Mr. Weather Comp's fan belts and the clouds
 * lying on its fields, laid as pieces at its field's edges and middles,
 * half domes at the ends of its solar panels' bands. */
#define SKIP_PALE 0x4000
#define TILES_BY_SHAPE 0x8000
/* ... and with TILES_NO_PAD_LOOK, no look of its pads (small platforms on
 * spurs): the Judge Tree's are round stumps, whose rings a square pad would
 * show in its middle alone. */
#define TILES_NO_PAD_LOOK 0x4000
/* ... and with TILES_MORE_COLOURS, its other maps' pairs are kept where they
 * draw colours its own map never shows on its floors (netmap.c): Seaside's
 * second floor forms fields only in Seaside 2 and 3, as their yellow panels,
 * which draw its guardian's arena. */
#define TILES_MORE_COLOURS 0x2000
/* ... and with TILES_RIMMED, its platforms' edges are rims, not held to the
 * plain look (netmap.c): Robot Control's white platforms are framed by two
 * light bands half a panel deep, reaching past where a tile must already
 * look like plain floor. */
#define TILES_RIMMED 0x10000
/* ... and with TILES_INNER_WALLS, the rings of walls inside its floors keep
 * MegaMan off what stands on them and are no holes: floor where the map
 * draws a panel there (the grey cubes on Robot Control's white platforms).
 * The neighbourhoods seen, which legal.c asks after, keep them as holes. */
#define TILES_INNER_WALLS 0x20000
/* ... and with TILES_MORE_PADS, its pads take the look of its other maps'
 * pads alone (netmap.c): Robot Control Comp 2's one small platform is the
 * striped conveyor before the robot's door. */
#define TILES_MORE_PADS 0x40000
/* ... and with TILES_CROSSING, a walkway meeting a platform square on runs
 * on straight across it in its own floor (netmap.c), as the comps' and
 * homepages' maps cross their fields with stripes of it: a walkway stopped
 * at a field's edge is a join of the two floors the originals never draw. */
#define TILES_CROSSING 0x80000
/* ... and with TILES_ARENA_FLOOR, its guardian's arena in its platforms'
 * floor, not its walkways': theirs is a pattern a panel wide (Mr. Weather
 * Comp's lamps and arcs, Robot Control's circuits), which laid over a 5x5
 * field came out tiled over itself. */
#define TILES_ARENA_FLOOR 0x100000
/* ... and with TILES_NO_SCENERY, none of its maps' free-standing pieces set
 * beside its floor (decor.c): the Cybeast Nest's only one is a grey cube,
 * debris under the original's altar, which stood alone in the void. */
#define TILES_NO_SCENERY 0x200000
/* ... and with TILES_WALK_NARROW, a panel of its walkways' hues is
 * walkway where it lies in no 2 x 2 block of floor, and platform floor
 * where it does: Sky Area's fields of framed squares share its cyan glass
 * catwalks' hue, and their thick lavender edges with orange lights stood
 * in for the catwalks' thin ones with their clips. */
#define TILES_WALK_NARROW 0x400000
void tiles_learn(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, bool bg_in_map, TileBook *out);
void tiles_free(TileBook *b);
/* (dev) The panels of map `a` as tiles_learn sees them, as text in a
 * generated layer's rows and columns: 'a' platform floor, 'b' walkway
 * floor, upper case ('P', 'Q') on its pads; floor of another style by its
 * hue bucket (0-9, 'X' 10, 'Y' 11, 'G' grey). */
void tiles_src_text(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, bool bg_in_map, FILE *f);
/* The colours the pairs of `b` draw, marked in `seen` (one byte per BGR555
 * colour); then the pairs of another book that draw others marked (`other`),
 * and with `drop` left out. */
#define TILE_COLOURS 32768
void tiles_colours(const TileBook *b, uint8_t seen[TILE_COLOURS]);
void tiles_other_colours(TileBook *b, const uint8_t seen[TILE_COLOURS], bool drop);
/* The pairs of `b` seen on the original's pads left out. */
void tiles_drop_pads(TileBook *b);

/* The class of tile (tx, ty): its phase and the panel it lies in. */
void tile_class(const TileGrid *g, int tx, int ty, int *phase, int *A, int *B);

/* How the picks went since the last reset (the dev atlas reads them): each
 * tile matched exactly, near (the nearest neighbourhood seen, or meeting a
 * neighbour badly) or by falling back on the least bad tile; and netmap
 * counts over the map as drawn (not the tiles set whole over the picks):
 * the pairs of neighbouring tiles left meeting badly, the pairs in colours
 * the area's own map never shows on its floors and the tiles drawn off. */
typedef struct { int picks, near, fallbacks, seams, other, off_near, off_edge; } TileStats;
extern TileStats tiles_stats;

/* With tiles_measure (the dev atlas), how far the pair the last tiles_pick
 * took was seen from the tile's own neighbourhood: exactly; on panels the
 * tile does not show; or where it shows them (their floors, side faces and
 * what hangs under those, in their materials) at a few pixels, or more: the
 * tile draws a floor, a material or an edge that is not there. netmap counts
 * the last two over a map in tiles_stats. */
enum { TILE_EXACT, TILE_OFF_FAR, TILE_OFF_EDGE, TILE_OFF_NEAR };
extern bool tiles_measure;
extern int tiles_pick_off;
/* ... and for one drawn off, why: no pair of its phase shows what it
 * shows; those that do cover the floor wrongly here (the pixel test); they
 * do not look like plain floor well inside it; or one would do and lost to
 * a nearer neighbourhood or a seam. */
enum { TILE_WHY_NONE, TILE_WHY_UNSEEN, TILE_WHY_PIXELS, TILE_WHY_PLAIN, TILE_WHY_RANKED };
extern int tiles_pick_why;
/* ... and whether it draws colours the area's own map never shows on its
 * floors (1), counted in tiles_stats by netmap over the map as drawn. */
extern int tiles_pick_other;

/* The tiles beside one (left, above, right, below) as far as they are
 * picked: how they look to the seams (SEAM_ANY: not yet) and what they draw. */
typedef struct { uint32_t look[4]; uint64_t mask[4]; } TileNeighbours;

/* How badly a tile that looks like `look` and draws `mask` meets them:
 * SEAM_COST for each neighbour no original map sets beside it (where
 * either is not drawn whole: at the floor's edges), and a point
 * for each pixel along its top and bottom where it draws and the tile over
 * or under it does not (or the other way round), past the few a diagonal
 * edge crossing there leaves: a floor that stops on a tile's edge instead
 * of its own shows as steps. */
#define SEAM_COST 3
#define CUT_EDGE 3
int tiles_trouble(const TileSeams *s, uint32_t look, uint64_t mask, const TileNeighbours *n);

/* The layer entries for tile (tx, ty) of a map whose floor is `floor`: of
 * the pairs whose pixels cover the floor there, do not reach beyond it and
 * look like plain floor well inside it, the one seen with the nearest
 * neighbours. Where the two floors meet and the original never joins them,
 * the walkway's tile over the platform's. False off the floor.
 * With `seams`, each pair also costs its trouble with neighbours `n`
 * (tilemap.c); *look and *mask: the tile picked, as tiles_trouble takes it. */
bool tiles_pick(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty,
	TileFloor floor, const void *ctx, const TileSeams *seams, const TileNeighbours *n,
	uint16_t *e0, uint16_t *e1, uint32_t *look, uint64_t *mask);
/* Where floors `fa` and `fb`, drawn apart on the two tile layers, overlap
 * in tile (tx, ty): whether fa shows in front. A floor's top in front of the
 * other's side faces; where only faces meet, the floor lower on screen. */
bool tiles_in_front(const TileGrid *g, int tx, int ty, TileFloor fa, const void *ca, TileFloor fb, const void *cb);
/* From here to tiles_keep_end, what `floor` answers for grid g's pixels is
 * kept per pixel: the floor must not change meanwhile (a map's pick). */
void tiles_keep_begin(const TileGrid *g, TileFloor floor, const void *ctx);
void tiles_keep_end(void);
/* A panel's neighbourhood: its 3 x 3 panels' floors (bit k: panel
 * (A + k % 3 - 1, B + k / 3 - 1)) and whether it lies on a pad. */
#define TILE_SHAPE(oa, ob, pad) ((uint32_t)(oa) << 10 | (uint32_t)(ob) << 1 | ((pad) ? 1u : 0u))
/* Whether one of the original maps has a panel with this neighbourhood:
 * its tiles are drawn for it. */
bool tiles_shape_seen(const TileBook *books, int nbooks, uint32_t shape);

#endif

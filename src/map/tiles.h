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
/* ... it lies beside the floor of a piece drawn apart, which its own view
 * takes for void: a walkway's end at a room. No map of such an area joins
 * the two, and the end is no dead end: its panel and what lies round it are
 * picked tile by tile, none of a map's laid whole there (panels.h), where a
 * dead end's took the map's dead-end piece (BN4's Park Area's round pad,
 * its arrow head), which the room then covered half of. The room's panel
 * just below such an end is marked so too, as the walkway's floor: the
 * walkway runs on under the room's upper edge (tilemap.c, pieces_floor). */
#define TILE_JOINS 16
/* ... a walkway's panel drawn in the area's second look of walkway (BN4's
 * Park Area's orange roads, found in its map: netmap_learn.c), from books of its own
 * (TileGrid.roads): a piece drawn apart (tilemap.c). */
#define TILE_ROAD 32
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
	uint32_t side[4];     /* its pixels along its left, top, right and bottom sides, as a strip takes them (seams_side) */
} TileCand;

/* The original's panels whole (panels.h): each with its sixteen tiles, by
 * the slot of their phase, as indices into `tile` (a pair, the pixels it
 * draws and its sides, as TileCand's); sorted by shape, and `at` by place.
 * A shape's panels mostly look alike; where a few differ (an edge with
 * something standing on it, a raised floor's face before it), those are
 * not laid: the Graveyard's slabs lost their upper rims to them. After
 * them the panels in part: those with floor of another kind beside them,
 * or a tile that is none of the floor's, which lend the tiles they have
 * where a tile shows nothing of what differs (panels.h). */
#define PANEL_TILES 16
#define PANEL_NO_TILE 0xFFFFu   /* a slot a panel in part has no tile for */
typedef struct { uint16_t e0, e1; uint64_t mask; uint32_t side[4]; } PanelTile;
typedef struct {
	uint32_t shape;
	int16_t A, B;
	uint16_t tile[PANEL_TILES];
	uint16_t rare;   /* its look is one its shape seldom has in its map (a third as often as the usual one, or less): not laid */
	uint16_t has;    /* the slots it has a tile for */
	uint16_t no;     /* the panels round it (bit k, as its shape's) that are floor of another kind: like no layer's */
	uint16_t edge;   /* the slots whose tile holds a border's art past where a tile may draw, the floor's own by its colour and place in its map: laid on trust, and followed from the tile beside it (panels_lay.c) */
} TilePanel;
typedef struct {
	TilePanel *panel;
	int n, npart;        /* whole ones, then those in part */
	uint16_t *at;        /* the panels' indices by (B, A), all of them */
	PanelTile *tile;
	int ntile;
	uint16_t *px;        /* the tiles' colours while an area's books are learned (panels_settle) */
	int8_t slot[64];     /* a phase's slot, -1 none */
} TilePanels;

/* A class's pairs: from its first to the next class's. */
typedef struct { uint32_t key; int first; } TileClass;

/* Sorted by key and then most common first. */
typedef struct TileBook {
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
	TilePanels panels;                  /* its panels whole */
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
	bool plain_ends;   /* a walkway's end at a piece drawn apart is marked TILE_JOINS: the area has no joint to stand there */
	int wall;          /* how far over a floor's upper edge a wall of it may stand, which lies in front of what is behind it (0: none known; 8 where the area's panels are laid whole) */
	const struct TileBook *roads;   /* the books of the area's roads, its TILE_ROAD panels' (NULL: it has none) */
	int nroads;
	unsigned rules;    /* the thin books' rules of laying that hold here though the books are not thin (LAY_*, panels_lay.c) */
	bool thin;         /* its books are thin (a road's): a panel taken up for how it meets its neighbours is laid again from another that meets them (panels_lay.c) */
} TileGrid;

/* TileGrid.rules: an end's panels lend a panel that is no end nothing; a
 * straight run is laid from its shape's one plain panel; beside a panel
 * laid whole its map's own tiles; a panel taken up is laid again. A thin
 * grid has them all. */
enum { LAY_NO_END_ART = 1, LAY_PLAIN_RUNS = 2, LAY_KEEP_TO_WHOLE = 4, LAY_AGAIN = 8, LAY_RULES = 15 };
/* NetAreaDef.lay beside them: its maps' panels laid whole, as TILES_WHOLE. */
#define LAY_WHOLE 16
/* ... or its pads' panels alone laid whole, the rest of its floor tile by
 * tile as before: a pad's frame from its map's pads, each panel in its
 * place, where tile by tile a pad of another size than its map's came out
 * as a jumble of frame and walkway. */
#define LAY_PADS_WHOLE 64
/* ... its rooms stand over the walkways beside them (BN4's squares 17
 * pixels tall, its walkways 10): where both draw a tile, on the room's
 * panel, beside it or under its faces, the room's goes in front (tilemap.c). */
#define LAY_ROOMS_OVER 128

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
/* ... and with SKIP_QUARTER beside it, only a panel showing them over a
 * quarter of its top (64 pixels of some 270), not any 8: BN4's ACDC Area
 * frames its squares in teal lines, 8 to 60 pixels a panel in its cyan
 * roads' hue, and the squares' upper rims lent no tiles: a room's upper
 * edges came out as steps of walkway floor cut into its field. (The bit is
 * the grey bucket's where a panel is told by its middle's hue.) */
#define SKIP_QUARTER 0x1000
/* ... or with SKIP_PALE, a platform panel whose top is pale in places
 * (bright, weakly coloured): Mr. Weather Comp's fan belts and the clouds
 * lying on its fields, laid as pieces at its field's edges and middles,
 * half domes at the ends of its solar panels' bands. */
#define SKIP_PALE 0x4000
/* ... and with SKIP_OFF_PADS, none on the map's pads (small platforms on
 * spurs), whatever their hue: BN5's Oran Area runs one look of walkway to
 * its octagon hubs, brown planks in the hubs' own colours beside its olive
 * circuits, and one look is learned, the hubs whole. */
#define SKIP_OFF_PADS 0x2000
/* (told by shape, a hue named in `styles` or `walk_styles` keeps it to
 * those hues: a panel of another is neither floor, as BN5's Nebula Area's
 * pale arrows on its paths) */
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
/* ... and with TILES_ARENA_DRAWN, its guardian's arena drawn in its
 * platforms' floor though laid out as before (the legalizer reads it as
 * walkway floor): the Undernet's walkway floor is bridges, stripes and
 * gems, which laid over a 5x5 field came out a jumble of joints. */
#define TILES_ARENA_DRAWN 0x800000
/* ... and with TILES_ONE_WIDE, its maps draw no floor two panels wide but
 * their pads: every room is drawn as one (netmap.c) and the way's walkways
 * stay one wide (net_gen.c, LayerKit.one_wide). BN5's End Area is one-wide
 * roads, rings round posts and pads; a field or a widened way laid out as
 * Seaside's was worn by the legalizer into rings and stubs, a junction at
 * every other panel (issue #164). */
#define TILES_ONE_WIDE 0x1000000
/* ... and with TILES_SQUARE_PADS, its pads are its platforms of 9 to 16
 * panels, not the smaller: BN4's ACDC Area frames squares of four by four,
 * which a room drawn as a pad takes (pad_rooms), where its round studs at
 * its paths' ends, pads of four panels, ringed a room's corners. */
#define TILES_SQUARE_PADS 0x2000000
/* NetAreaDef.joint_hues: JOINT_GREY, its joints are its walkways' grey
 * panels with no platform floor round them, not those showing its hues:
 * BN4's Undernet's steel joints between its orange cushions, a cushion's
 * blue stud in each */
#define JOINT_GREY 0x1000
/* ... and with TILES_WALK_HUE, told by shape, a panel of its walkways'
 * hue (walk_styles) is walkway floor however wide its floor lies there:
 * BN4's countries lay their green walkways two and three wide, whose
 * middles stood in their cream platforms' middles as green blotches. */
#define TILES_WALK_HUE 0x8000000
/* ... and with TILES_UNDER, platform floor that passes under what its map
 * sets over it on the second layer is floor, whatever the walls and heights
 * there say, learned from the first layer: BN4's Park Area's conveyor
 * crosses over the middle of its one platform, which the walls give as two
 * halves, each ending under it in an edge that is none; rooms' ends came
 * out as plain floor cut off. */
#define TILES_UNDER 0x10000000
/* ... and with TILES_WHOLE, its maps' panels are laid whole where a
 * layer's floor has them, and piece by piece where it has not (panels.h):
 * BN4's Park Area's platform with its frame, its corners' marks and the
 * wall on its upper edges, which tile by tile came out in pieces; BN5's
 * Oran Area's circuits; BN4's countries' cream platforms, its Undernet's
 * cushions, BN5's Undernet's plates; BN4's Town Area's squares on their
 * slabs. Not everywhere, each looked at (docs/devtools/tile-test.md): Sky's
 * glass catwalks came out as its fields' squares, BN5's ACDC Area's
 * circuits lost their rims, BN4's ACDC Area's walkways took ends and joints
 * at every panel, the Graveyard's slabs gained breaks where they join. */
#define TILES_WHOLE 0x20000000
/* ... and with TILES_OUTER_FLOOR, only what lies inside one ring of its
 * walls is its floor: a floor ringed again inside a hole (three rings and
 * more) is another level's. BN4's Town Area 3 runs pale blue paths under
 * its roads, inside their walls' rings; read as its walkways, their tiles
 * drew a layer's, and their edges set its side faces at 17 pixels where
 * Town Area 1's walkways have 10. */
#define TILES_OUTER_FLOOR 0x40000000
/* ... and with TILES_PADS_ALONE, its own map lends its pads alone, the
 * tiles of its other floor none (its other maps lend all of theirs): BN4's
 * Town Area 3 has the area's squares, but its other floor is roads, hubs
 * and paths of another level, whose pieces a layer's walkways took at
 * their ends and turns; the walkways are Town Area 1's. A tile of such a
 * map draws its pads' own art and no more (tiles_art.c): a hub
 * set against a square's corner, a road joined to its side are none of the
 * square's, though they lie where its faces and legs may. */
#define TILES_PADS_ALONE 0x80000000u
void tiles_learn(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, uint16_t joint_hues, bool bg_in_map, TileBook *out);
void tiles_free(TileBook *b);
/* (dev) The panels of map `a` as tiles_learn sees them, as text in a
 * generated layer's rows and columns: 'a' platform floor, 'b' walkway
 * floor, upper case ('P', 'Q') on its pads; floor of another style by its
 * hue bucket (0-9, 'X' 10, 'Y' 11, 'G' grey). */
void tiles_src_text(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, uint16_t joint_hues, bool bg_in_map, FILE *f);
/* (dev) ... and as numbers: TILES_SRC_SPAN panels a side round the world's
 * origin, panel (A, B) at [(B + SPAN / 2) * SPAN + A + SPAN / 2]: 0 no
 * floor, TILE_A, TILE_B, 3 floor of another kind, and | TILE_PAD on its
 * pads (small platforms on spurs). NULL without memory (free it). */
#define TILES_SRC_SPAN 128
uint8_t *tiles_src_panels(const AreaSrc *a, uint32_t styles, uint16_t walk_styles, uint16_t skip_styles, uint16_t joint_hues, bool bg_in_map);
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
 * the area's own map never shows on its floors and the tiles drawn off;
 * and, where the breaks are measured, those in its picture: the tile edges
 * across which it shows what no original map shows there, half an edge or
 * more a break (a block of another floor or shade), less a nick (a panel's
 * line cut short; breaks.h). */
typedef struct { int picks, near, fallbacks, seams, other, off_near, off_edge, breaks, nicks, whole; } TileStats;
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
 * picked: how they look to the seams (SEAM_ANY: not yet), what they draw
 * and their side towards the tile (a pair's `side`, or nothing's; `known`
 * bit k where a pair draws neighbour k or nothing does: not where two are
 * laid over each other, or none is picked yet). */
typedef struct { uint32_t look[4]; uint64_t mask[4]; uint32_t side[4]; unsigned known; } TileNeighbours;
/* The sides of a tile drawing `px` where `mask` says (seams_side of each:
 * left, top, right, bottom). */
void tiles_sides(const uint16_t *px, uint64_t mask, uint32_t side[4]);

/* How badly a tile that looks like `look` and draws `mask` with sides
 * `side` (NULL: not known) meets them: SEAM_COST for each neighbour no original map sets beside it
 * (where either is not drawn whole: at the floor's edges; inside the
 * floor, where both are, the classes keep the look, and pairs the
 * originals happen not to show are no seam), BREAK_COST more where the
 * picture breaks across their edge, inside the floor too (the strip of
 * pixels there is one no original map shows at a tile's edge, seams.h: a
 * panel's line cut off, a block of another shade), and a point
 * for each pixel along its top and bottom where it draws and the tile over
 * or under it does not (or the other way round), past the few a diagonal
 * edge crossing there leaves: a floor that stops on a tile's edge instead
 * of its own shows as steps. */
#define SEAM_COST 3
#define BREAK_COST 4
#define CUT_EDGE 3
int tiles_trouble(const TileSeams *s, uint32_t look, uint64_t mask, const uint32_t *side, const TileNeighbours *n);

/* The layer entries for tile (tx, ty) of a map whose floor is `floor`: of
 * the pairs whose pixels cover the floor there, do not reach beyond it and
 * look like plain floor well inside it, the one seen with the nearest
 * neighbours. Where the two floors meet and the original never joins them,
 * the walkway's tile over the platform's. False off the floor.
 * With `seams`, each pair also costs its trouble with neighbours `n`
 * (tilemap.c); *out: the tile picked, its entries and how tiles_trouble
 * takes it (side NULL where two pairs' tiles lie over each other). */
typedef struct { uint16_t e0, e1; uint32_t look; uint64_t mask; const uint32_t *side; } TilePick;
bool tiles_pick(const TileBook *books, int nbooks, const TileGrid *g, int tx, int ty,
	TileFloor floor, const void *ctx, const TileSeams *seams, const TileNeighbours *n, TilePick *out);
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

/* The original maps' panels whole. A panel is drawn by the sixteen tiles
 * whose centres lie in it; where a generated floor's panel has the very
 * floors around it that a panel of an original map has (its 3 x 3, pads
 * told apart), it takes that panel's sixteen as the map lays them, and the
 * panel beside it the one beside that where it fits too: a room's rim, its
 * corners and what hangs under them come out as stretches of the original,
 * not tile by tile (tiles_pick.c, which picks what is left).
 *
 * Where no map has the floor's panel, it is laid piece by piece: a tile
 * shows only the panels it lies on or beside (and off the floor those whose
 * faces or walls reach it), so it is taken from any panel that is like the
 * floor's in those. A room three panels wide takes its field's left half
 * from the left of a map's wider field and its right half from the right.
 * So a map's panel with floor of another kind beside it, or with a tile
 * that is none of the floor's, is kept in part and lends the tiles that
 * show nothing of it. What lies round the floor (its faces, legs and walls)
 * is laid after it, as a map lays it beside what is laid there: a void's
 * own neighbourhood does not say what kind of floor it lies under.
 *
 * A border is kept whole. Its art runs past the line a tile may draw up to
 * (a slab a tile wider than its floor, a wall over an upper edge), and a
 * tile judged alone is turned down for it: the border came out cut at that
 * tile's edge, a tooth at every other tile. So the learner follows the
 * floor's own art from tile to tile in its map (tiles_learn.c, own_art) and
 * keeps the tiles that hold it, each marked at its place (TilePanel.edge);
 * and a laid tile that draws up to its edge takes the marked tile its map
 * sets beside it along (panels_lay.c, follow_edges).
 *
 * Learning is panels.c's, laying panels_lay.c's. */
#ifndef CW_PANELS_H
#define CW_PANELS_H

#include <stdbool.h>
#include <stdint.h>

#include "tiles.h"
#include "tiles_parts.h"

/* A panel's neighbourhood: its 3 x 3 panels' platform and walkway floors
 * and which of them have a pad's look (bit k: panel (A + k % 3 - 1,
 * B + k / 3 - 1)). */
#define PANEL_SHAPE(oa, ob, pads) ((uint32_t)(oa) << 18 | (uint32_t)(ob) << 9 | (uint32_t)(pads))

/* ---- learning (tiles_learn.c): a map's tiles told one by one ---- */

typedef struct PanelsLearn PanelsLearn;
/* A tile's place: its panel and its phase in it. */
typedef struct {
	int A, B, phase;
	bool edge;   /* its tile holds a border's art past where a tile may draw (TilePanel.edge) */
} PanelSpot;

PanelsLearn *panels_begin(void);   /* (NULL: no memory, and the calls below do nothing) */
/* Panel (A, B) is not to be laid: it is floor of another kind itself. */
void panels_bad(PanelsLearn *l, int A, int B);
/* Panel (A, B) lacks a tile (one that is none of the floor's): laid in
 * part at most. */
void panels_lacks(PanelsLearn *l, int A, int B);
/* A panel's tile: the pair, the pixels it draws and their colours
 * (mask 0: it draws nothing there). */
void panels_tile(PanelsLearn *l, PanelSpot at, uint16_t e0, uint16_t e1, uint64_t mask, const uint16_t *px);
/* ... one the pixel test turned down, at map tile (tx, ty), which must
 * draw `must` and not `never` (the caller leaves out of it where a wall may
 * stand, over a platform's upper edge): kept after all (panels_end) where
 * it draws so, in the floor's own colours. */
void panels_loose(PanelsLearn *l, PanelSpot at, int tx, int ty, uint64_t must, uint64_t never);
/* ... and of the tile told last so, its pixels that are the floor's own
 * art (tiles_learn.c, own_art), which it may draw wherever they lie; with
 * `only`, it may draw nothing else at all. */
void panels_own(PanelsLearn *l, uint64_t own, bool only);
/* The panels with all their tiles, then those in part (lacking a tile, or
 * with floor of another kind beside them), into `out` (its pairs learned
 * already: their colours are the floor's own); `l` freed. */
void panels_end(PanelsLearn *l, const Src *src, const uint8_t *pads, TileBook *out);
/* As a book's pairs are left out (tiles.h): the panels with a tile in
 * colours not `seen`, and those on or beside a pad. */
void panels_other_colours(TilePanels *p, const uint8_t seen[TILE_COLOURS]);
void panels_drop_pads(TilePanels *p);
/* Once an area's books are settled: their tiles' colours freed. */
void panels_settle(TilePanels *p);

/* ---- laying (tilemap.c) ---- */

/* The floor's panels that an original's panel draws whole, laid, and the
 * rest piece by piece as far as the maps' panels have their tiles: each
 * tile of theirs in `pick` (the others' left as they were); how many tiles.
 * With g->single, only panels drawn on one layer. */
int panels_lay(const TileBook *books, int nbooks, const TileSeams *seams, const TileGrid *g, TileFloor floor, const void *ctx, TilePick *pick);
/* Marks the tiles of the panel tile (tx, ty) lies in. */
void panels_mark(const TileGrid *g, int tx, int ty, uint8_t *mark);

#endif

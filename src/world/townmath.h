/* Moving pieces of a real-world map (docs/OVERWORLD.md): which moves keep
 * the tile grid and the ground's patterns, and where a moved tile lands. */
#ifndef CW_TOWNMATH_H
#define CW_TOWNMATH_H

#include <stdbool.h>

/* A move of (dx, dy) 8-unit world cells moves the picture by 8 (dx + dy)
 * pixels across and 4 (dy - dx) down: whole 8 x 8 tiles when dx + dy is
 * even. */
static inline bool town_move_keeps_tiles(int dx, int dy) { return ((dx + dy) & 1) == 0; }

/* ... and the brick's and cobblestone's pattern, 8 x 4 tiles, when both
 * are multiples of 4 and differ by a multiple of 8. */
static inline bool town_move_keeps_pattern(int dx, int dy) {
	return (dx & 3) == 0 && (dy & 3) == 0 && ((dx - dy) & 7) == 0;
}

/* Where tile (stx, sty) of a stw x sth map lands on a tw x th one when
 * moved by (dx, dy) cells; both maps are centred on world (0, 0), as the
 * game draws them, and of the same parity. */
static inline void town_move_tile(int stx, int sty, int stw, int sth, int dx, int dy, int tw, int th, int *tx, int *ty) {
	*tx = stx + dx + dy + (tw - stw) / 2;
	*ty = sty + (dy - dx) / 2 + (th - sth) / 2;
}

#endif

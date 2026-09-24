/* The pieces BN6's net areas are built from (docs/LEVEL_DESIGN.md):
 * platforms of a few shapes, 1-wide bridges, 3x3 pads on spurs and the
 * stubs of comb boardwalks, carved into the layer being generated. */
#ifndef NET_SHAPES_H
#define NET_SHAPES_H

#include <stdbool.h>

#include "net.h"

/* The window a layer is built in: a diamond on the grid, a rectangle on
 * screen of 2 * WIN_U + 1 by 2 * WIN_V + 1 panels along its diagonals,
 * which fits the game's tile map buffer. */
#define WIN_C 32
#define WIN_U 14   /* |x - y| from the centre: across the screen */
#define WIN_V 26   /* |x + y| from the centre: up and down the screen */

enum { DIR_E, DIR_S, DIR_W, DIR_N };   /* +x, +y, -x, -y */
extern const int dir_dx[4], dir_dy[4];

bool win_in(int x, int y);
bool floor_at(int x, int y);
void put(int x, int y);

/* Whether a w x h box at (x, y), and `margin` cells around it, lies in the
 * window with no floor. */
bool box_free(int x, int y, int w, int h, int margin);

/* Platform shapes, carved into their box. */
enum { SHAPE_RECT, SHAPE_OCTAGON, SHAPE_PLUS, SHAPE_RAGGED, SHAPE_HOLED, SHAPE_CRATER };
void carve_shape(int shape, int x, int y, int w, int h);

/* A room for points of interest: its box, a floor cell to stand on, and
 * its kind. Returns its index or -1. */
int add_room(int x, int y, int w, int h, int kind);

/* A straight 1-wide bridge of `len` cells from (x, y) in direction d;
 * returns false (carving nothing) when it would run out of the window. */
bool bridge_line(int x, int y, int d, int len);
/* A 1-wide bridge from (ax, ay) to (bx, by) with one bend. */
void bridge_l(int ax, int ay, int bx, int by, bool x_first);
/* A bridge of `len` from (x, y) in direction d, ending in a 3x3 pad (a
 * room of kind ROOM_PAD); the pad's index or -1 when there is no room. */
int pad_spur(int x, int y, int d, int len);
/* Teeth: 1-cell stubs off a straight run, every other cell. */
void teeth(int x, int y, int d, int len, int side);

/* A floor cell of room r's edge facing d, and the cell past it. */
bool room_edge(int r, int d, int *x, int *y);

/* Joins every piece of floor to the one holding room 0 with the shortest
 * bridges. */
void connect_all(void);

/* Floor cells with exactly one floor neighbour. */
int dead_ends(int *xs, int *ys, int max);

#endif

/* An original internet map, decoded from the ROM as the source of a
 * generated layer's tiles (see netmap.c). */
#ifndef CW_AREA_SRC_H
#define CW_AREA_SRC_H

#include <stdbool.h>
#include <stdint.h>

/* One cell of a coordinate-data section: its 8x8 world cell (x, y) and
 * shape (lowest z, value, height, type). */
typedef struct {
	int16_t x, y;
	int8_t z;
	uint8_t value, height, type;
} CoordCell;

typedef struct {
	int group, number;
	int tw, th;            /* size in tiles */
	int layers;
	uint16_t *tile[2];     /* map entries, row-major, per layer */
	uint32_t *px;          /* the map drawn (ARGB, alpha 0 where empty) */
	uint8_t *front;        /* 1 where the front layer (0) is drawn */
	int ex, ey;            /* panel edges in world units: X = ex, Y = ey (mod 32) */
	uint32_t desc;         /* ROM offset of its MapBGDescriptor */
	uint32_t coord_slot;   /* ROM offset of its coordinate-data pointer */
	CoordCell *sec[4];     /* walls, floor heights, layer priorities, triggers */
	int nsec[4];
	/* floor heights by wall cell (section 1), and the one this view shows */
	uint8_t *hz;           /* HEIGHT_UNEVEN on ramps */
	int hx0, hy0, hw, hh;  /* the cells hz covers */
	int level;
	/* inside how many rings of walls each wall cell lies, odd on floor */
	uint8_t *rings;
	int rx0, ry0, rw, rh;
} AreaSrc;

#define HEIGHT_UNEVEN 255

bool area_src_load(int group, int number, AreaSrc *a);
void area_src_free(AreaSrc *a);
/* The map flipped left-right: world (X, Y) becomes (-Y, -X), tiles flip. */
void area_src_mirror(const AreaSrc *a, AreaSrc *m);
/* The map drawn `z` pixels lower (a multiple of 8), so that its floor at
 * height z lies where ground floor would: a view of that level. */
void area_src_raise(const AreaSrc *a, int z, AreaSrc *r);
/* Floor height at world (X, Y): 0 where section 1 says nothing. */
int area_src_height(const AreaSrc *a, int X, int Y);
/* Whether the walls put world (X, Y) on floor (1), off it (0) or cannot
 * tell (-1): floor lies inside an odd number of rings of walls, since the
 * walls ring each floor and again each hole in it. */
int area_src_walled_floor(const AreaSrc *a, int X, int Y);

/* World <-> map pixel, as the game's camera routine maps them. */
static inline int area_px(int tw, int x, int y) { return x + y + tw * 4; }
static inline int area_py(int th, int x, int y) { return (y - x) / 2 + th * 4; }

#endif

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
} AreaSrc;

bool area_src_load(int group, int number, AreaSrc *a);
void area_src_free(AreaSrc *a);
/* The map flipped left-right: world (X, Y) becomes (-Y, -X), tiles flip. */
void area_src_mirror(const AreaSrc *a, AreaSrc *m);

/* World <-> map pixel, as the game's camera routine maps them. */
static inline int area_px(int tw, int x, int y) { return x + y + tw * 4; }
static inline int area_py(int th, int x, int y) { return (y - x) / 2 + th * 4; }

#endif

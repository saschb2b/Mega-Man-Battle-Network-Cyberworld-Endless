/* An original internet map, decoded from the ROM as the source of a
 * generated layer's tiles (see netmap.c). */
#ifndef CW_AREA_SRC_H
#define CW_AREA_SRC_H

#include <stdbool.h>
#include <stdint.h>

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
} AreaSrc;

bool area_src_load(int group, int number, AreaSrc *a);
void area_src_free(AreaSrc *a);

/* World <-> map pixel, as the game's camera routine maps them. */
static inline int area_px(int tw, int x, int y) { return x + y + tw * 4; }
static inline int area_py(int th, int x, int y) { return (y - x) / 2 + th * 4; }

#endif

/* An original internet map, decoded from the ROM as the source of a
 * generated layer's tiles (see netmap.c). */
#ifndef CW_AREA_SRC_H
#define CW_AREA_SRC_H

#include <stdbool.h>
#include <stddef.h>
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
	int8_t rom;            /* where it was read: 0 BN6, 1 + XRomId an extra game's (docs/MULTIROM.md) */
	int tw, th;            /* size in tiles */
	int layers;
	uint16_t *tile[2];     /* map entries, row-major, per layer */
	uint32_t *px;          /* the map as the game shows it, the second layer in
	                        * front of the first (ARGB, alpha 0 where empty) */
	uint32_t *px0;         /* the first layer alone */
	uint8_t *front;        /* 1 where the last layer (the second) is drawn */
	uint8_t *idx;          /* the colour index shown (0 where empty): a
	                        * 256-colour map's materials are index ranges */
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
/* The same from an extra game's ROM (XRomId, docs/MULTIROM.md), where it
 * has been read. */
bool area_src_load_x(int xrom, int group, int number, AreaSrc *a);
/* Map (group, number)'s descriptor (tile set, palette, tile map: ROM
 * offsets of its 12 bytes) and its coordinate data's pointer. */
bool area_src_slots(int group, int number, uint32_t *desc, uint32_t *coord_slot);
/* A map of BN6 (rom 0) or another game's (1 + XRomId) as an area reads it
 * (NetAreaDef.recolour): loaded, then its tiles of both layers in palette
 * bank `from` read as bank `to` and the map drawn again, `recolour`
 * holding pairs of nibbles (from, then to), the lowest first (0x21: bank 2
 * as bank 1; 0: as it is). A game whose colour banks run alike draws one
 * surface in another's shades: BN5's Undernet its raised courts' pale
 * stone and purple slime, its plates' mauve stone and lava. */
bool area_src_load_as(int rom, int group, int number, uint32_t recolour, AreaSrc *a);
void area_src_free(AreaSrc *a);
/* The map flipped left-right: world (X, Y) becomes (-Y, -X), tiles flip.
 * False, and `m` empty, where the memory for it could not be had. */
bool area_src_mirror(const AreaSrc *a, AreaSrc *m);
/* The map drawn `z` pixels lower (a multiple of 8), so that its floor at
 * height z lies where ground floor would: a view of that level. False,
 * and `r` empty, where the memory for it could not be had. */
bool area_src_raise(const AreaSrc *a, int z, AreaSrc *r);
/* Tile maps (tw x th entries of layer 0, then layer 1) drawn with map
 * (group, number)'s tiles and colours as the game shows them, layer 1 in
 * front: ARGB pixels, alpha 0 where empty (free them). The map is BN6's
 * (rom 0) or another game's (1 + XRomId, where read). */
uint32_t *area_src_render(int rom, int group, int number, const uint16_t *tiles, int tw, int th);
/* Map (group, number)'s tile set and colours in game `rom` (as for
 * area_src_render), to stand anywhere: the tile set's header with its
 * blocks encoded again after it (*ts_out, *nts bytes), the palette as it
 * is (*pal_out, *npal); free both. */
bool area_src_gfx(int rom, int group, int number, uint8_t **ts_out, size_t *nts, uint8_t **pal_out, size_t *npal);
/* Floor height at world (X, Y): 0 where section 1 says nothing. */
int area_src_height(const AreaSrc *a, int X, int Y);
/* Whether the walls put world (X, Y) on floor (1), off it (0) or cannot
 * tell (-1): floor lies inside an odd number of rings of walls, since the
 * walls ring each floor and again each hole in it. */
int area_src_walled_floor(const AreaSrc *a, int X, int Y);
/* ... and inside how many rings (-1: cannot tell): 2 and more, even, inside
 * a ring within the floor, a hole or what the walls keep MegaMan off. */
int area_src_rings(const AreaSrc *a, int X, int Y);

/* Pixel i of the map's floors: the first layer where it draws, else the
 * second, as if nothing stood in front of them (the originals set their
 * bridges, spikes and ornaments on the second layer, and some floors). */
static inline uint32_t area_src_floor_px(const AreaSrc *a, size_t i) { return a->px0[i] >> 24 ? a->px0[i] : a->px[i]; }

/* World <-> map pixel, as the game's camera routine maps them. */
static inline int area_px(int tw, int x, int y) { return x + y + tw * 4; }
static inline int area_py(int th, int x, int y) { return (y - x) / 2 + th * 4; }

#endif

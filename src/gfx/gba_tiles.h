/* A mode-0 background's pixels read a row at a time from video memory,
 * a screen entry and a row of its tile for every eight pixels: what
 * gba_layers.c and gba_world.c read their backgrounds by. Read a pixel at
 * a time, depth lighting's two took 2.3 ms of a frame (docs/engine/voxel.md,
 * Its cost). */
#ifndef CW_GBA_TILES_H
#define CW_GBA_TILES_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* A background as its control and scroll registers set it */
typedef struct { bool on, wide; unsigned prio, chars, screen, size, hofs, vofs; } GbaBg;

static inline unsigned gba_rd16(const uint8_t *p, unsigned at) { return (unsigned)p[at] | (unsigned)p[at + 1] << 8; }

/* Background `b` (0-3) from the IO registers at 0x04000000 */
static inline GbaBg gba_bg(const uint8_t *io, unsigned b) {
	unsigned cnt = gba_rd16(io, 0x08u + 2u * b);
	return (GbaBg){ (gba_rd16(io, 0) >> (8u + b) & 1u) != 0, (cnt & 0x80u) != 0, cnt & 3u, (cnt >> 2 & 3u) * 0x4000u, (cnt >> 8 & 31u) * 0x800u,
		cnt >> 14, gba_rd16(io, 0x10u + 4u * b) & 0x1FFu, gba_rd16(io, 0x12u + 4u * b) & 0x1FFu };
}

/* Screen entry `e`'s tile at its row `ty`, flipped as the entry says:
 * the eight pixels' palette indices into `out` (0 clear) */
static inline void gba_tile_row(const uint8_t *vram, const GbaBg *b, unsigned e, unsigned ty, uint8_t out[8]) {
	if (e & 0x800u) ty = 7u - ty;
	bool flip = (e & 0x400u) != 0;
	if (b->wide) {
		const uint8_t *p = &vram[(b->chars + (e & 0x3FFu) * 64u + ty * 8u) & 0xFFFFu];
		for (int i = 0; i < 8; ++i) out[flip ? 7 - i : i] = p[i];
		return;
	}
	const uint8_t *p = &vram[(b->chars + (e & 0x3FFu) * 32u + ty * 4u) & 0xFFFFu];
	unsigned bank = (e >> 12) * 16u;
	for (int i = 0; i < 4; ++i) {
		unsigned lo = p[i] & 15u, hi = (unsigned)p[i] >> 4;
		out[flip ? 7 - 2 * i : 2 * i] = (uint8_t)(lo ? bank + lo : 0u);
		out[flip ? 6 - 2 * i : 2 * i + 1] = (uint8_t)(hi ? bank + hi : 0u);
	}
}

/* The `w` pixels from the frame's (x0, y) of a background's own screen
 * map, wrapped as the GBA wraps it: their palette indices (0 clear) */
static inline void gba_screen_row(const uint8_t *vram, const GbaBg *b, int x0, int y, int w, uint8_t *idx) {
	if (!b->on) { memset(idx, 0, (size_t)w); return; }
	unsigned wm = (b->size & 1u) ? 511u : 255u, py = ((unsigned)y + b->vofs) & ((b->size & 2u) ? 511u : 255u);
	unsigned rowat = b->screen + (py >> 8) * ((b->size & 1u) ? 2u : 1u) * 0x800u + (py >> 3 & 31u) * 64u;
	uint8_t row[8];
	for (int i = 0; i < w;) {
		unsigned px = ((unsigned)(x0 + i) + b->hofs) & wm;
		gba_tile_row(vram, b, gba_rd16(vram, (rowat + (px >> 8) * 0x800u + (px >> 3 & 31u) * 2u) & 0xFFFEu), py & 7u, row);
		for (unsigned tx = px & 7u; tx < 8u && i < w; ++tx, ++i) idx[i] = row[tx];
	}
}

/* One pixel of a background's own screen map at the frame's (x, y): its palette index (0 clear) */
static inline unsigned gba_bg_pixel(const uint8_t *vram, const GbaBg *b, int x, int y) {
	uint8_t one;
	gba_screen_row(vram, b, x, y, 1, &one);
	return one;
}

#endif

/* The game's battle backgrounds (the BattleSettings background, 0x00-0x15)
 * drawn from the ROM: 4bpp tiles, a 32x32 map and a palette, with the
 * game's tile and palette animations and the scroll each one has in battle
 * (docs/ROM_DATA.md). */
#ifndef BACKDROP_H
#define BACKDROP_H

#include <stdbool.h>
#include <stdint.h>

#define BACKDROP_COUNT 0x16

typedef struct {
	uint32_t p0, p1;          /* palette: destination, size; tiles: ROM source, VRAM destination */
	uint8_t cmd, count;       /* 0 palette copy, 4 tile copy (count tiles) */
	int nsteps, total, key;
	uint32_t step[16];
	int delay[16];
	bool loop;
} BackdropAnim;

typedef struct {
	int id;                   /* -1: nothing loaded */
	uint8_t vram[0x8000];     /* the BG character block as the game fills it */
	uint16_t map[32 * 32];
	uint8_t pal[512];
	BackdropAnim anim[4];
	int nanim;
	int dx, dy;               /* scroll per frame in 1/16 pixel (the BG offset registers) */
} Backdrop;

bool backdrop_load(Backdrop *b, int id);
/* The 240x160 view at `frame` of the animations and scroll, moved a further
 * (ox, oy) pixels, into px (ARGB, 240 wide). Each row is darkened by dim[y]
 * steps of 31 per channel; mosaic > 1 draws blocks of that size. */
void backdrop_draw(Backdrop *b, int frame, int ox, int oy, const uint8_t *dim, int mosaic, uint32_t *px);

#endif

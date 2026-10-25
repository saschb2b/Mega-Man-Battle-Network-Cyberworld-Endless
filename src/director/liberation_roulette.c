/* A BonusPanel's flashes and roulette drawn as BN5 draws them
 * (liberation_roulette.h), traced in BN5 (docs/rom/liberation.md, BN5's
 * Liberation Missions: the roulette's objects 0x23 and 0x24). Positions
 * are the GBA picture's. Its sprites are list 9's, each drawn as BN5's
 * objects set them: the first frame's sub-animation the object names (the
 * words' one frame holds the prompt and the five names as lists of its
 * own), its OBJs read from the sprite's bytes. */
#include "liberation_roulette.h"

#include <stdint.h>
#include <stdlib.h>

#include "bn5_liberation.h"
#include "gfx.h"
#include "liberation_bonus.h"
#include "platform.h"
#include "rom.h"
#include "xtiles.h"

/* BN5's bytes at bus address `a`, `n` of them; NULL where its ROM is not there */
static const uint8_t *bn5(uint32_t a, uint32_t n) {
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	uint32_t o = a - 0x08000000u;
	return x->data && a >= 0x08000000u && o + n <= ROM_SIZE ? x->data + o : NULL;
}

static uint32_t le32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

/* List 9's sprite `index`: its bytes past its header (decompressed once
 * where it is compressed: the words), NULL none */
static const uint8_t *sprite_bytes(int index) {
	static uint8_t *unpacked;
	static uint32_t unpacked_from;
	const XRom *x = &XR[XROM_BN5_COLONEL_US];
	const uint8_t *list = x->data ? bn5(x->layout->sprite_lists + 0x08000000u + 4u * BN5_LIB_BONUS_ICON_LIST, 4) : NULL;
	const uint8_t *at = list ? bn5(le32(list) + 4u * (uint32_t)index, 4) : NULL;
	if (!at) return NULL;
	uint32_t p = le32(at);
	if (!(p & 0x80000000u)) return bn5(p + 4u, 64);
	if (unpacked_from != p) {
		const uint8_t *lz = bn5(p & 0x7FFFFFFFu, 4);
		size_t n = 0;
		free(unpacked);
		unpacked = lz ? lz77_decompress(lz, ROM_SIZE - ((p & 0x7FFFFFFFu) - 0x08000000u), &n) : NULL;
		if (unpacked && n < 64) { free(unpacked); unpacked = NULL; }
		unpacked_from = p;
	}
	return unpacked ? unpacked + 8 : NULL;
}

/* Its first frame's sub-animation `sub` at (x, y), as BN5's OBJs show it */
static void draw_sub(const uint8_t *b, int sub, int x, int y) {
	static const uint8_t dims[3][4][2] = {
		{ { 1, 1 }, { 2, 2 }, { 4, 4 }, { 8, 8 } }, { { 2, 1 }, { 4, 1 }, { 4, 2 }, { 8, 4 } }, { { 1, 2 }, { 1, 4 }, { 2, 4 }, { 4, 8 } },
	};
	if (!b) return;
	const uint8_t *f = b + le32(b);
	const uint8_t *tiles = b + le32(f) + 4, *pals = b + le32(f + 4) + 4, *mini = b + le32(f + 8), *objtab = b + le32(f + 12);
	if (sub < 0 || (uint32_t)sub >= le32(mini) / 4 || (uint32_t)sub >= 32) return;
	int list = (mini + le32(mini + 4 * sub))[0];
	for (const uint8_t *o = objtab + le32(objtab + 4 * list); !(o[0] == 0xFF && o[1] == 0xFF); o += 5) {
		int shape = o[4] & 3, size = o[3] & 3;
		if (shape > 2) continue;
		xtiles_draw(tiles + 32u * o[0], pals + 32 * (o[4] >> 4), x + (int8_t)o[1], y + (int8_t)o[2], dims[shape][size][0], dims[shape][size][1],
			(o[3] & 0x40 ? 1 : 0) | (o[3] & 0x80 ? 2 : 0));
	}
}

void lib_roulette_draw(void) {
	LibBonusView v;
	lib_bonus_view(&v);
	const uint8_t *pal = bn5(BN5_LIB_PHASE_WORDS_PAL, 32);
	if (!pal) return;
	if (v.white) xtiles_blend_rect(P.core_x, P.core_y, 240, 160, 0x7FFF, v.white, 16 - v.white);
	if (!v.band_rows) return;
	xtiles_blend_rect(P.core_x, P.core_y + v.band_top, 240, v.band_rows,
		(uint16_t)(pal[2 * BN5_LIB_BAND_COLOUR] | pal[2 * BN5_LIB_BAND_COLOUR + 1] << 8), 16, 8);
	SDL_Rect core = { P.core_x, P.core_y, 240, 160 };
	gfx_clip(&core);
	/* (the slot's icon: its sprite and sub-animation, BN5_LIB_BONUS_ICONS) */
	const uint8_t *e = v.icon && v.slot >= 0 && v.slot < LIB_BONUS_SLOTS ? bn5(BN5_LIB_BONUS_ICONS + 4u * (uint32_t)v.slot, 4) : NULL;
	if (e) draw_sub(sprite_bytes(e[0]), e[1], P.core_x + v.icon_x, P.core_y + BN5_LIB_BONUS_ICON_Y);
	if (v.words >= 0) draw_sub(sprite_bytes(BN5_LIB_BONUS_WORDS), v.words, P.core_x + BN5_LIB_BONUS_WORDS_X, P.core_y + BN5_LIB_BONUS_WORDS_Y);
	gfx_clip(NULL);
}

/* lcd_grid.h. The shader's numbers and steps, in its float arithmetic: its
 * fragment's angle is vTexCoord * TextureSize / InputSize * omega, the
 * screen pixel's centre across the picture (0 to 1) times 2 pi the
 * picture's pixels (OrigInputSize); its colour the picture's, sampled
 * nearest (filter_linear0 = false), times the factors. */
#include "lcd_grid.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#define PI                 3.141592654f   /* (the shader's) */
#define BRIGHTEN_LCD       4.0f           /* (its parameters' defaults) */
#define BRIGHTEN_SCANLINES 16.0f

static bool on;

bool lcd_grid(void) { return on; }

void lcd_set_grid(bool v) {
#ifdef __3DS__
	(void)v;
#else
	on = v;
#endif
}

static uint8_t to255(float f) { return (uint8_t)(f * 255.0f + 0.5f); }

/* A column's factors and a row's, in 255ths: screen column `x` of
 * `across` showing a picture `w` wide, row `y` of `down` showing it `h` high */
static void column(int x, int across, int w, uint8_t rgb[3]) {
	static const float offsets[3] = { PI * (1.0f / 2.0f), PI * (1.0f / 2.0f - 2.0f / 3.0f), PI * (1.0f / 2.0f - 4.0f / 3.0f) };
	float angle = ((float)x + 0.5f) / (float)across * (PI * 2.0f * (float)w);
	for (int c = 0; c < 3; ++c) rgb[c] = to255((BRIGHTEN_LCD + sinf(angle + offsets[c])) / (BRIGHTEN_LCD + 1.0f));
}

static uint8_t row(int y, int down, int h) {
	float angle = ((float)y + 0.5f) / (float)down * (PI * 2.0f * (float)h);
	return to255((BRIGHTEN_SCANLINES + sinf(angle)) / (BRIGHTEN_SCANLINES + 1.0f));
}

/* ---- the strips: a row of the columns' factors, a column of the rows',
 * each stretched over the picture and multiplied into it ---- */

typedef struct {
	SDL_Texture *tex;
	SDL_Renderer *on;
	int len, size;   /* the screen pixels it spans, the picture's it shows */
	int lost;        /* (P.textures_lost when it was made) */
} Strip;
static Strip columns, rows;

static void strip_make(Strip *s, SDL_Renderer *r, int len, int size, bool across) {
	if (s->tex && s->on == r && s->len == len && s->size == size && s->lost == P.textures_lost) return;
	if (s->tex) SDL_DestroyTexture(s->tex);
	*s = (Strip){ SDL_CreateTexture(r, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, across ? len : 1, across ? 1 : len), r, len, size, P.textures_lost };
	if (!s->tex) return;
	uint32_t *px = malloc((size_t)len * sizeof *px);
	if (!px) { SDL_DestroyTexture(s->tex); s->tex = NULL; return; }
	for (int i = 0; i < len; ++i) {
		uint8_t f[3];
		if (across) column(i, len, size, f);
		else f[0] = f[1] = f[2] = row(i, len, size);
		px[i] = 0xFF000000u | (uint32_t)f[0] << 16 | (uint32_t)f[1] << 8 | f[2];
	}
	SDL_UpdateTexture(s->tex, NULL, px, across ? len * 4 : 4);
	free(px);
	SDL_SetTextureBlendMode(s->tex, SDL_BLENDMODE_MOD);
	SDL_SetTextureScaleMode(s->tex, SDL_ScaleModeNearest);
}

void lcd_grid_draw(SDL_Renderer *r, const SDL_Rect *dst, int w, int h) {
	if (!on || dst->w < 1 || dst->h < 1) return;
	strip_make(&columns, r, dst->w, w, true);
	strip_make(&rows, r, dst->h, h, false);
	if (columns.tex) SDL_RenderCopy(r, columns.tex, NULL, dst);
	if (rows.tex) SDL_RenderCopy(r, rows.tex, NULL, dst);
}

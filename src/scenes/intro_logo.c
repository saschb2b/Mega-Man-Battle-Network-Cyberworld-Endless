/* intro_logo.h. Saschb2b's letters are pixels of our own, a bold pixel
 * font in the manner of a handheld's (strokes two pixels thick, the
 * capital and the ascenders 11 high, the x-height 8), drawn at twice their
 * size with a light shadow a pixel down and right. Near-black on white,
 * timed to the chime's two tones (intro_chime.h), in frames:
 *
 *   1-12     in from black
 *   20       the chime
 *   21       its first tone: "Sasch" comes up in the middle out of a
 *            mosaic, as the GBA's hardware mosaic resolves
 *   35       its second: "b2b" comes up beside it the same way, and the
 *            two glide left together until the name stands in the middle
 *   92-104   out to white */
#include "intro_logo.h"

#include <string.h>

#include "audio.h"
#include "intro_chime.h"
#include "platform.h"

#define NAME "Saschb2b"
#define FIRST_PIECE 5       /* "Sasch", then "b2b" */
#define GLYPH_H 11
#define K 2                 /* screen pixels to one of the font's */

#define CHIME_AT 20
/* the frame a tone of the chime begins in (a piece shows from the next,
 * by when the sound has come through the device's buffer) */
#define TONE_AT(sample) (CHIME_AT + (sample) / (INTRO_CHIME_RATE / 60))
#define RESOLVE 7           /* frames a piece's mosaic takes to resolve */
#define GLIDE 14            /* ... and the two pieces to glide */

#define PAPER 0xFFFFFFFFu
#define INK_RGB 28, 28, 30
#define SHADOW_RGB 208, 208, 214

static const struct { char ch; int w; const char *rows[GLYPH_H]; } glyphs[] = {
	{ 'S', 7, { ".######", "#######", "##.....", "##.....", "######.", ".######", ".....##", ".....##", ".....##", "#######", "######." } },
	{ 'a', 7, { ".......", ".......", ".......", ".#####.", ".######", ".....##", ".######", "#######", "##...##", "#######", ".######" } },
	{ 's', 6, { "......", "......", "......", ".#####", "######", "##....", "#####.", ".#####", "....##", "######", "#####." } },
	{ 'c', 6, { "......", "......", "......", ".#####", "######", "##....", "##....", "##....", "##....", "######", ".#####" } },
	{ 'h', 7, { "##.....", "##.....", "##.....", "##.###.", "#######", "###..##", "##...##", "##...##", "##...##", "##...##", "##...##" } },
	{ 'b', 7, { "##.....", "##.....", "##.....", "##.###.", "#######", "###..##", "##...##", "##...##", "###..##", "#######", "##.###." } },
	{ '2', 7, { ".#####.", "#######", "##...##", ".....##", "....###", "..####.", ".####..", "###....", "##.....", "#######", "#######" } },
};

/* a piece of the name drawn into it: 2 ink, 1 shadow */
static uint8_t mask[CORE_H][CORE_W];

static int glyph_of(char ch) {
	for (int i = 0; i < (int)(sizeof glyphs / sizeof *glyphs); ++i)
		if (glyphs[i].ch == ch) return i;
	return 0;
}

static int piece_first(int piece) { return piece ? FIRST_PIECE : 0; }
static int piece_end(int piece) { return piece ? (int)strlen(NAME) : FIRST_PIECE; }

/* a piece's width in the font's pixels, a pixel between letters */
static int piece_w(int piece) {
	int w = -1;
	for (int i = piece_first(piece); i < piece_end(piece); ++i) w += glyphs[glyph_of(NAME[i])].w + 1;
	return w;
}

static void block(int x, int y, uint8_t v) {
	for (int dy = 0; dy < K; ++dy)
		for (int dx = 0; dx < K; ++dx) {
			int sx = x + dx, sy = y + dy;
			if (sx >= 0 && sy >= 0 && sx < CORE_W && sy < CORE_H && mask[sy][sx] < v) mask[sy][sx] = v;
		}
}

/* the piece's letters into the mask at (x0, y0), their shadows under them */
static void piece_mask(int piece, int x0, int y0) {
	for (int i = piece_first(piece), x = x0; i < piece_end(piece); ++i) {
		int g = glyph_of(NAME[i]);
		for (int r = 0; r < GLYPH_H; ++r)
			for (int c = 0; c < glyphs[g].w; ++c)
				if (glyphs[g].rows[r][c] == '#') {
					block(x + c * K, y0 + r * K, 2);
					block(x + (c + 1) * K, y0 + (r + 1) * K, 1);
				}
		x += (glyphs[g].w + 1) * K;
	}
}

/* a piece at (x0, y0) `since` frames after its tone: out of a mosaic of
 * blocks shrinking to single pixels, fading in, as on the GBA */
static void piece_draw(uint32_t *px, int piece, int x0, int y0, int since) {
	if (since <= 0) return;
	int w = (piece_w(piece) + 1) * K, h = (GLYPH_H + 1) * K;
	int m = since >= RESOLVE ? 1 : RESOLVE + 1 - since;
	float alpha = since >= 5 ? 1.0f : (float)since / 5.0f;
	for (int y = y0; y < y0 + h; ++y)
		for (int x = x0; x < x0 + w; ++x)
			if (x >= 0 && y >= 0 && x < CORE_W && y < CORE_H) mask[y][x] = 0;
	piece_mask(piece, x0, y0);
	static const int colour[2][3] = { { SHADOW_RGB }, { INK_RGB } };
	for (int y = y0 < 0 ? 0 : y0; y < y0 + h && y < CORE_H; ++y)
		for (int x = x0 < 0 ? 0 : x0; x < x0 + w && x < CORE_W; ++x) {
			int sx = x0 + (x - x0) / m * m, sy = y0 + (y - y0) / m * m;
			if (sx < 0 || sy < 0 || !mask[sy][sx]) continue;
			const int *c = colour[mask[sy][sx] - 1];
			uint32_t o = px[y * CORE_W + x];
			int r = (int)(o >> 16 & 255), gr = (int)(o >> 8 & 255), b = (int)(o & 255);
			r += (int)((float)(c[0] - r) * alpha);
			gr += (int)((float)(c[1] - gr) * alpha);
			b += (int)((float)(c[2] - b) * alpha);
			px[y * CORE_W + x] = 0xFF000000u | (uint32_t)r << 16 | (uint32_t)gr << 8 | (uint32_t)b;
		}
}

void intro_logo_render(uint32_t *px, int t) {
	for (int i = 0; i < CORE_W * CORE_H; ++i) px[i] = PAPER;
	/* "Sasch" in the middle of the screen alone, until "b2b" comes up
	 * beside it and the two glide together (eased out) until the name is
	 * in the middle */
	int name_w = (piece_w(0) + 1 + piece_w(1)) * K, left = (CORE_W - name_w) / 2, top = (CORE_H - GLYPH_H * K) / 2;
	int second = TONE_AT(INTRO_CHIME_TONE2), shift = (CORE_W - piece_w(0) * K) / 2 - left;
	float v = (float)(t - second) / GLIDE;
	v = v < 0 ? 0 : v > 1 ? 1 : v;
	int x = left + (int)((float)shift * (1 - v) * (1 - v) * (1 - v) + 0.5f);
	piece_draw(px, 0, x, top, t - TONE_AT(INTRO_CHIME_TONE1));
	piece_draw(px, 1, x + (piece_w(0) + 1) * K, top, t - second);
}

void intro_logo_sound(int t) {
	if (t == CHIME_AT) audio_chime();
}

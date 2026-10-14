/* The title logo assembled from the player's ROM and the engine's own subtitle. */
#include "title_art.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "rom.h"
#include "text.h"

#define T (R.layout->title)
#define LOGO_UP 8
#define LOGO_BOTTOM 95
#define LOGO_END 0x50

static uint8_t logo[160][256];
static uint32_t logo_col[256];
static bool ready;

/* Where the 6 starts on each row of the picture: all of it above MEGAMAN,
 * then along the italic N it leans against, then after BATTLE NETWORK's K. */
static int six_left(int y) {
	if (y < 44) return 160;
	if (y < 54) return 177 - (y - 44) / 2;
	if (y < 82) return 171 - (y - 54) / 3;
	return 175;
}

/* the dark rim the 6 has around its outline */
static bool rim(uint8_t v) { return v == 0x01 || v == 0x02 || (v >= 0x16 && v <= 0x1C); }

#define OUTLINE 0x13   /* black */

/* Puts a mask (1 fill, 2 outline) into the logo at (x0, y0): its fill in
 * graded rows (or a light top edge with `bevel`), a black outline and rings. */
typedef struct { const uint8_t *fill; int nfill; uint8_t bevel; const uint8_t *ring; int nring; } StampStyle;

static void stamp(const uint8_t *m, int w, int h, int x0, int y0, const StampStyle *style) {
	const uint8_t *fill = style->fill, *ring = style->ring;
	int nfill = style->nfill, nring = style->nring;
	uint8_t bevel = style->bevel;
#define AT(r, c) ((r) >= 0 && (r) < h && (c) >= 0 && (c) < w ? m[(r) * w + (c)] : 0)
	int reach = nring + 1;
	for (int r = -reach; r < h + reach; ++r)
		for (int c = -reach; c < w + reach; ++c) {
			int X = x0 + c, Y = y0 + r;
			if (X < 0 || X >= 256 || Y < 0 || Y >= 160) continue;
			uint8_t v = AT(r, c);
			if (v == 2) { logo[Y][X] = OUTLINE; continue; }
			if (v == 1) {
				logo[Y][X] = bevel && !AT(r - 1, c) ? bevel : fill[r * nfill / h];
				continue;
			}
			int d = reach + 1;
			for (int dr = -reach; dr <= reach; ++dr)
				for (int dc = -reach; dc <= reach; ++dc) {
					int k = abs(dr) > abs(dc) ? abs(dr) : abs(dc);
					if (k < d && AT(r + dr, c + dc)) d = k;
				}
			if (d == 1) logo[Y][X] = OUTLINE;
			else if (d - 2 < nring) logo[Y][X] = ring[d - 2];
		}
#undef AT
}

/* CYBERWORLD ENDLESS in bold capitals, leaning like the logo. */
static const struct { char ch; const char *rows[9]; } glyphs[] = {
	{ 'C', { ".#####.", "##...##", "##.....", "##.....", "##.....", "##.....", "##.....", "##...##", ".#####." } },
	{ 'Y', { "##...##", "##...##", "##...##", ".##.##.", "..###..", "..###..", "..###..", "..###..", "..###.." } },
	{ 'B', { "######.", "##...##", "##...##", "##...##", "######.", "##...##", "##...##", "##...##", "######." } },
	{ 'E', { "#######", "##.....", "##.....", "##.....", "######.", "##.....", "##.....", "##.....", "#######" } },
	{ 'R', { "######.", "##...##", "##...##", "##...##", "######.", "##.##..", "##..##.", "##...##", "##...##" } },
	{ 'W', { "##...##", "##...##", "##...##", "##.#.##", "##.#.##", "##.#.##", "##.#.##", "#######", ".##.##." } },
	{ 'O', { ".#####.", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", ".#####." } },
	{ 'L', { "##.....", "##.....", "##.....", "##.....", "##.....", "##.....", "##.....", "##.....", "#######" } },
	{ 'D', { "######.", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", "##...##", "######." } },
	{ 'N', { "##...##", "###..##", "####.##", "##.####", "##..###", "##...##", "##...##", "##...##", "##...##" } },
	{ 'S', { ".######", "##.....", "##.....", "##.....", ".#####.", ".....##", ".....##", ".....##", "######." } },
};
#define GLYPH_H 9
#define GLYPH_ADV 8
#define SPACE_ADV 5
#define LEAN 3          /* rows per pixel of slant */

static void build_subtitle(void) {
	const char *s = "CYBERWORLD ENDLESS";
	int w = (GLYPH_H - 1) / LEAN + 2;
	for (const char *p = s; *p; ++p) w += *p == ' ' ? SPACE_ADV : GLYPH_ADV;
	uint8_t *m = calloc((size_t)w * GLYPH_H, 1);
	if (!m) return;
	int x = 0;
	for (const char *p = s; *p; ++p) {
		if (*p == ' ') { x += SPACE_ADV; continue; }
		for (size_t g = 0; g < sizeof glyphs / sizeof *glyphs; ++g) {
			if (glyphs[g].ch != *p) continue;
			for (int r = 0; r < GLYPH_H; ++r)
				for (int k = 0; glyphs[g].rows[r][k]; ++k)
					if (glyphs[g].rows[r][k] == '#') m[r * w + x + k + (GLYPH_H - 1 - r) / LEAN] = 1;
		}
		x += GLYPH_ADV;
	}
	/* the plate's colours: green, white in the middle, green (its rows'
	 * palette indices), a black outline, a dark and a grey rim */
	static const uint8_t fill[] = { 0x05, 0x04, 0x35, 0x2A, 0x2D, 0x2D, 0x2C, 0x24, 0x07 };
	static const uint8_t ring[] = { 0x18, 0x27 };
	StampStyle style = { fill, (int)sizeof fill, 0, ring, 2 };
	stamp(m, w, GLYPH_H, 200 - w, 100, &style);
	free(m);
}

/* The infinity mark: a lemniscate of Bernoulli drawn as a thick stroke,
 * slanted like the logo, the stroke from the upper right to the lower left
 * crossing over the other one; the size of MEGAMAN's letters beside it (at
 * the 6's size it stood over them and outweighed the name). */
static void build_infinity(void) {
	enum { W = 96, H = 52, N = 360 };
	const double A = 23.8, B = 40.8, RAD = 3.6, SLANT = 0.42;
	static double seg[2][N + 1][2];
	for (int s = 0; s < 2; ++s)
		for (int i = 0; i <= N; ++i) {
			double t = M_PI * (s + (double)i / N), k = 1 + sin(t) * sin(t);
			seg[s][i][0] = A * cos(t) / k;
			seg[s][i][1] = B * sin(t) * cos(t) / k;
		}
	static uint8_t m[H][W];
	memset(m, 0, sizeof m);
	for (int r = 0; r < H; ++r)
		for (int c = 0; c < W; ++c) {
			double py = r - H / 2, px = c - W / 2 + py * SLANT, d[2] = { 1e9, 1e9 };
			for (int s = 0; s < 2; ++s)
				for (int i = 0; i <= N; ++i) {
					double dx = px - seg[s][i][0], dy = py - seg[s][i][1], q = dx * dx + dy * dy;
					if (q < d[s]) d[s] = q;
				}
			if (d[0] <= RAD * RAD) m[r][c] = 1;
			else if (d[1] <= RAD * RAD) m[r][c] = d[0] <= (RAD + 1.2) * (RAD + 1.2) && fabs(px) < 2 * RAD + 2 ? 2 : 1;
		}
	/* rows of the 6's colours from top to bottom (palette indices) */
	static const uint8_t fill[] = { 0x3E, 0x3E, 0x3D, 0x3D, 0x3D, 0x3C, 0x41, 0x42, 0x45, 0x45,
	                                0x44, 0x44, 0x39, 0x33, 0x31, 0x30, 0x30, 0x2F, 0x2F, 0x2E };
	static const uint8_t ring[] = { 0x18, 0x02 };
	int top = H, bottom = 0;
	for (int r = 0; r < H; ++r)
		for (int c = 0; c < W; ++c)
			if (m[r][c]) { if (r < top) top = r; bottom = r; }
	if (top > bottom) return;
	StampStyle style = { fill, (int)sizeof fill, 0x46, ring, 2 };
	stamp(&m[top][0], W, bottom - top + 1, 201 - W / 2, 62 - (bottom - top + 1) / 2, &style);
}

static bool build_logo(void) {
	size_t n = 0;
	uint8_t *tiles = lz77_decompress(R.data + T.bg_tiles, ROM_SIZE - T.bg_tiles, &n);
	if (!tiles) return false;
	memset(logo, 0, sizeof logo);
	for (int ty = 0; ty < 20; ++ty)
		for (int tx = 0; tx < 32; ++tx) {
			uint16_t e = rom_u16(T.bg_map + (uint32_t)(ty * 32 + tx) * 2);
			size_t at = 4 + (size_t)(e & 0x3FF) * 64;
			if (at + 64 > n) continue;
			for (int y = 0; y < 8; ++y)
				for (int x = 0; x < 8; ++x) {
					int X = tx * 8 + ((e & 0x400) ? 7 - x : x), Y = ty * 8 + ((e & 0x800) ? 7 - y : y);
					uint8_t v = tiles[at + y * 8 + x];
					if (v < LOGO_END && Y <= LOGO_BOTTOM && X < six_left(Y)) logo[Y][X] = v;
				}
		}
	free(tiles);
	/* the 6's rim where it leant against the N */
	for (int y = 44; y < 82; ++y)
		for (int x = six_left(y) - 1; x > 0 && rim(logo[y][x]); --x) logo[y][x] = 0;
	build_subtitle();
	build_infinity();
	for (int i = 0; i < 256; ++i) logo_col[i] = bgr555(i < 0xE0 ? rom_u16(T.bg_pal + (uint32_t)i * 2) : 0);
	return true;
}

/* ------------------------------------------------------------------ */

void title_art_build(void) { if (!ready) ready = build_logo(); }

static uint32_t lighten(uint32_t c) {
	uint32_t r = (c >> 16) & 255, g = (c >> 8) & 255, b = c & 255;
	return 0xFF000000u | (r + (255 - r) / 2) << 16 | (g + (255 - g) / 2) << 8 | (b + (255 - b) / 2);
}

void title_art_draw(uint32_t *px, int tick) {
		/* a shine runs over the logo now and then */
		int shine = tick % 300 < 40 ? (tick % 300) * 8 - 40 : -1000;
		for (int y = 0; y < 160; ++y) {
			int ly = y + LOGO_UP;
			if (ly >= 160) break;
			for (int x = 0; x < 240; ++x) {
				uint8_t v = logo[ly][x];
				if (!v) continue;
				int s = x + ly / 2 - shine;
				px[y * 240 + x] = s >= 0 && s < 7 ? lighten(logo_col[v]) : logo_col[v];
			}
		}
	}

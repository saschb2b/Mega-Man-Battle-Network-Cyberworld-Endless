/* pet_look.h. The measures are BN6's: KeyItem's band is 14 rows over a
 * row of the rails' shadow, its title 7 rows from x 16, y 4; its list
 * window a pixel of light edge, three of cyan band and a sunk blue edge
 * round the navy field; its information box a light green field in a
 * dark outline. The letters of the lists are the chat font's glyphs as
 * the ROM holds them; the capitals are the engine's own (minifont's and
 * pixfont's letters drawn heavier, as BN6's labels and titles are), as
 * BN6 draws its labels from pictures of each word, not from a font. */
#include "pet_look.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "minifont.h"
#include "pixfont.h"
#include "rom.h"

/* a row of pixels from x0 to x1 (both in) */
static void span(int x0, int x1, int y, SDL_Color c) {
	if (x1 >= x0) fill_rect(x0, y, x1 - x0 + 1, 1, c);
}

/* ---- the screen ---- */

/* a rail's column: its light, three of metal and its shadow (the right
 * one mirrored), from row y0 down h */
static void rail(int x, int y0, int h, bool right) {
	fill_rect(x + (right ? 4 : 0), y0, 1, h, right ? LOOK_METAL_LO : LOOK_METAL_HI);
	fill_rect(x + 1, y0, 3, h, LOOK_METAL);
	fill_rect(x + (right ? 0 : 4), y0, 1, h, right ? LOOK_METAL_HI : LOOK_METAL_LO);
}

/* the band's ends: the rails seven wide, their tops rounded */
static void band_ends(int x, int y, int w) {
	int rx = x + w - 1;
	for (int r = 0; r < LOOK_BAND_H; ++r) {
		int in = r < 4 ? 3 - r : 0;
		span(x + in, x + 5, y + r, LOOK_METAL);
		fill_rect(x + 6, y + r, 1, 1, LOOK_METAL_LO);
		span(x + w - 7, rx - in, y + r, LOOK_METAL);
		fill_rect(x + w - 7, y + r, 1, 1, LOOK_METAL_HI);
		if (r == 0) {
			span(x + 3, x + 5, y, LOOK_METAL_HI);
			span(x + w - 7, rx - 3, y, LOOK_METAL_HI);
		} else if (r < 4) {
			span(x + in, x + in + 1, y + r, LOOK_METAL_HI);
			span(rx - in - 1, rx - in, y + r, LOOK_METAL_HI);
		} else {
			fill_rect(x, y + r, 1, 1, LOOK_METAL_HI);
			fill_rect(rx, y + r, 1, 1, r == 4 ? LOOK_METAL_HI : LOOK_METAL_LO);
		}
	}
	span(x, x + 6, y + LOOK_BAND_H, LOOK_METAL_LO);
	span(x + w - 7, rx, y + LOOK_BAND_H, LOOK_METAL_LO);
}

void look_stripes(int sx, int y, int right, int h) {
	span(sx, right, y + 6, LOOK_GREEN);
	span(sx - 1, right, y + 7, LOOK_GREEN);
	for (int r = 8; r < h; ++r) span(sx + 6 - r, sx + 8 - r, y + r, LOOK_GREEN);
	span(sx + 2, right, y + 10, LOOK_GREEN);
	span(sx + 1, right, y + 11, LOOK_GREEN);
	for (int r = 12; r < h; ++r) span(sx + 12 - r, sx + 13 - r, y + r, LOOK_GREEN);
}

/* the foot: a rail across, a groove along its middle */
static void foot(int x, int y, int w) {
	span(x, x + w - 1, y, LOOK_METAL_HI);
	fill_rect(x, y + 1, w, 3, LOOK_METAL);
	fill_rect(x, y + 1, 1, 3, LOOK_METAL_LO);
	span(x + 24, x + w - 25, y + 2, LOOK_METAL_LO);
	span(x + 24, x + w - 25, y + 3, LOOK_METAL_HI);
	span(x, x + w - 1, y + 4, LOOK_METAL_LO);
}

void look_band(int x, int y, int w, const char *title) {
	fill_rect(x + 7, y, w - 14, LOOK_BAND_H, LOOK_BAND);
	band_ends(x, y, w);
	look_title(x + 16, y + 4, title, LOOK_PAGE);
	int sx = x + w - 97, after = x + 16 + look_title_width(title) + 12;
	look_stripes(sx > after ? sx : after, y, x + w - 8, LOOK_BAND_H);
}

SDL_Rect look_screen(int x, int y, int w, int h, const char *title) {
	look_band(x, y, w, title);
	fill_rect(x + 7, y + LOOK_BAND_H, w - 14, 1, LOOK_GREEN);
	int top = y + LOOK_BAND_H + 1, foot_y = y + h - LOOK_RAIL;
	rail(x, top, foot_y - top, false);
	rail(x + w - LOOK_RAIL, top, foot_y - top, true);
	SDL_Rect body = { x + LOOK_RAIL, top, w - 2 * LOOK_RAIL, foot_y - top };
	fill_rect(body.x, body.y, body.w, body.h, LOOK_GREEN);
	foot(x, foot_y, w);
	return body;
}

void look_band_note(int x, int y, int w, const char *s, SDL_Color c) {
	look_caps(x + w - 10 - look_caps_width(s), y + 12, s, c, LOOK_GREEN_LO);
}

SDL_Rect look_band_hint(int x, int y, int w, const char *key, const char *what) {
	int ww = look_hint_width(key, what), hx = x + w - 10 - ww;
	look_hint(hx, y + 12, key, what);
	return (SDL_Rect){ hx - 2, y + 9, ww + 4, 11 };
}

int look_hint_width(const char *key, const char *what) { return look_caps_width(key) + 6 + look_caps_width(what) + 1; }

void look_hint(int x, int y, const char *key, const char *what) {
	int kw = look_caps_width(key) + 1;
	look_caps(x, y, key, LOOK_PAGE, LOOK_GREEN_LO);
	look_caps(x + kw, y, ":", LOOK_PAGE, LOOK_GREEN_LO);
	look_caps(x + kw + 6, y, what, LOOK_ORANGE, LOOK_GREEN_LO);
}

/* ---- windows ---- */

/* a raised edge round x, y, w, h: light along the top and left, blue
 * along the bottom and right, the corners left out */
static void raised(int x, int y, int w, int h) {
	span(x + 1, x + w - 2, y, LOOK_EDGE_HI);
	fill_rect(x, y + 1, 1, h - 2, LOOK_EDGE_HI);
	fill_rect(x + 1, y + 1, 1, 1, LOOK_EDGE_HI);
	span(x + 1, x + w - 2, y + h - 1, LOOK_EDGE_LO);
	fill_rect(x + w - 1, y + 1, 1, h - 2, LOOK_EDGE_LO);
	fill_rect(x + w - 2, y + h - 2, 1, 1, LOOK_EDGE_LO);
}

/* a sunk edge round the field: blue along its top and left, light along
 * its bottom and right, a corner pixel more of each */
static void sunk(SDL_Rect f) {
	span(f.x - 1, f.x + f.w - 1, f.y - 1, LOOK_EDGE_LO);
	fill_rect(f.x - 1, f.y, 1, f.h, LOOK_EDGE_LO);
	fill_rect(f.x, f.y, 1, 1, LOOK_EDGE_LO);
	span(f.x - 1, f.x + f.w, f.y + f.h, LOOK_EDGE_HI);
	fill_rect(f.x + f.w, f.y - 1, 1, f.h + 1, LOOK_EDGE_HI);
	fill_rect(f.x + f.w - 1, f.y + f.h - 1, 1, 1, LOOK_EDGE_HI);
}

static void shadow(int x, int y, int w, int h) {
	fill_rect(x + w, y + 3, 1, h - 3, LOOK_GREEN_LO);
	span(x + 2, x + w, y + h, LOOK_GREEN_LO);
}

SDL_Rect look_window(int x, int y, int w, int h, int top, bool bar) {
	int right = bar ? LOOK_FRAME_BAR : LOOK_FRAME;
	SDL_Rect f = { x + LOOK_FRAME, y + LOOK_FRAME + top, w - LOOK_FRAME - right, h - 2 * LOOK_FRAME - top };
	fill_rect(x + 1, y + 1, w - 2, h - 2, LOOK_EDGE);
	raised(x, y, w, h);
	shadow(x, y, w, h);
	fill_rect(f.x, f.y, f.w, f.h, LOOK_NAVY);
	sunk(f);
	if (bar) {
		/* (the track: two pixels sunk in the band, light on its right and foot) */
		int tx = f.x + f.w + 3;
		fill_rect(tx, f.y, 2, f.h, LOOK_NAVY);
		span(tx, tx + 1, f.y, LOOK_EDGE_LO);
		fill_rect(tx + 2, f.y, 1, f.h + 1, LOOK_EDGE_HI);
		span(tx, tx + 1, f.y + f.h, LOOK_EDGE_HI);
	}
	return f;
}

SDL_Rect look_box(int x, int y, int w, int h, SDL_Color fill) {
	if (h < 16) {
		/* (too small for the band: the raised edge alone) */
		fill_rect(x + 1, y + 1, w - 2, h - 2, fill);
		raised(x, y, w, h);
		return (SDL_Rect){ x + 1, y + 1, w - 2, h - 2 };
	}
	SDL_Rect f = { x + 3, y + 3, w - 6, h - 6 };
	fill_rect(x + 1, y + 1, w - 2, h - 2, LOOK_EDGE);
	raised(x, y, w, h);
	fill_rect(f.x, f.y, f.w, f.h, fill);
	sunk(f);
	return f;
}

void look_scroll(SDL_Rect win, int top, int first, int shown, int n) {
	if (n <= shown || shown <= 0) return;
	int tx = win.x + win.w - LOOK_FRAME_BAR + 3, ty = win.y + LOOK_FRAME + top + 1, th = win.h - 2 * LOOK_FRAME - top - 2;
	int bh = th * shown / n, by = ty + (th - (bh < 5 ? 5 : bh)) * first / (n - shown);
	if (bh < 5) bh = 5;
	fill_rect(tx, by, 2, bh, LOOK_TEXT);
	fill_rect(tx + 1, by + 1, 1, bh - 1, LOOK_GREEN_HI);
}

SDL_Rect look_info(int x, int y, int w, int h, const char *label) {
	span(x + 1, x + w - 2, y, LOOK_GREEN_LO);
	span(x + 1, x + w - 2, y + h - 1, LOOK_GREEN_LO);
	fill_rect(x, y + 1, 1, h - 2, LOOK_GREEN_LO);
	fill_rect(x + w - 1, y + 1, 1, h - 2, LOOK_GREEN_LO);
	fill_rect(x + 1, y + 1, w - 2, h - 2, LOOK_GREEN_HI);
	fill_rect(x + 1, y + 1, 1, 1, LOOK_GREEN_LO);
	int top = 1;
	if (label && *label) {
		look_caps(x + 4, y + 2, label, LOOK_PAGE, (SDL_Color){ 0, 0, 0, 0 });
		top = 2 + LOOK_CAPS_H + 1;
	}
	return (SDL_Rect){ x + 2, y + top, w - 4, h - top - 2 };
}

void look_tag(int x, int y, int w, int h, const char *s, SDL_Color fill) {
	fill_rect(x + 1, y, w - 2, h, fill);
	fill_rect(x, y + 1, w, h - 2, fill);
	look_caps(x + (w - look_caps_width(s)) / 2, y + (h - LOOK_CAPS_H) / 2, s, LOOK_PAGE, (SDL_Color){ 0, 0, 0, 0 });
}

/* ---- the cursor ---- */

void look_cursor(int x, int y) {
	/* (rows of 4 to 8 pixels, the point at the fifth: lit along its top
	 * and left, darker along its lower edges, a shade under them) */
	static const SDL_Color hi = { 255, 247, 115, 255 }, body = { 255, 173, 16, 255 }, lo = { 214, 107, 0, 255 }, under = { 16, 82, 82, 255 };
	for (int r = 0; r < 9; ++r) {
		int len = 4 + (r <= 4 ? r : 8 - r);
		if (r >= 4) fill_rect(x + 1, y + r + 1, len, 1, under);
		fill_rect(x, y + r, len, 1, r == 0 ? hi : r == 8 ? lo : body);
		fill_rect(x, y + r, 1, 1, r == 8 ? lo : hi);
		fill_rect(x + len - 1, y + r, 1, 1, r < 4 ? LOOK_GOLD : lo);
	}
}

/* ---- the capitals ---- */

int look_caps_width(const char *s) {
	int n = (int)strlen(s);
	return n ? n * 6 - 1 : 0;
}

/* `s` in minifont's letters, their outer columns two pixels wide */
static void caps_pass(int x, int y, const char *s, SDL_Color c) {
	static const int at[3] = { 0, 2, 3 }, wide[3] = { 2, 1, 2 };
	SDL_Rect px[256];
	int n = 0;
	for (; *s; ++s, x += 6) {
		unsigned short g = minifont_glyph(*s);
		for (int row = 0; row < 5; ++row)
			for (int col = 0; col < 3 && n < 256; ++col)
				if (g >> ((4 - row) * 3 + (2 - col)) & 1) px[n++] = (SDL_Rect){ x + at[col], y + row, wide[col], 1 };
	}
	fill_rects(px, n, c);
}

void look_caps(int x, int y, const char *s, SDL_Color c, SDL_Color shade) {
	if (shade.a) caps_pass(x + 1, y + 1, s, shade);
	caps_pass(x, y, s, c);
}

void look_note(int x, int y, const char *s, SDL_Color c) {
	minifont_draw(x + 1, y + 1, s, LOOK_GREEN_LO, 1);
	minifont_draw(x, y, s, c, 1);
}

/* a heavy letter: pixfont's, each pixel two wide (struck twice, a pixel
 * apart), as BN6's titles have their strokes */
static int heavy_width(char ch) {
	const char *const *rows;
	int w = pixfont_glyph((char)(ch >= 'a' && ch <= 'z' ? ch - 'a' + 'A' : ch), &rows);
	return rows ? w + 1 : 4;
}

int look_title_width(const char *s) {
	int w = 0, n = 0;
	for (; *s; ++s, ++n) w += heavy_width(*s) + 1;
	return n ? w - 1 : 0;
}

void look_title(int x, int y, const char *s, SDL_Color c) {
	SDL_Rect px[512];
	int n = 0;
	for (; *s; ++s) {
		char ch = (char)(*s >= 'a' && *s <= 'z' ? *s - 'a' + 'A' : *s);
		const char *const *rows;
		int w = pixfont_glyph(ch, &rows);
		for (int r = 0; rows && r < PIXFONT_CAP && rows[r]; ++r)
			for (int col = 0; col < w && rows[r][col]; ++col)
				if (rows[r][col] == '#' && n < 512) px[n++] = (SDL_Rect){ x + col, y + r, 2, 1 };
		x += heavy_width(ch) + 1;
	}
	fill_rects(px, n, c);
}

/* ---- the lists' letters ---- */

/* each character's pixels as runs along its rows (the chat font's index 1
 * the letter, 3 its shade), built at the first draw */
typedef struct { uint8_t x, y, len, shade; } Run;
#define RUNS 6144
static Run runs[RUNS];
static uint16_t run_first[129];
static uint8_t run_w[128];
static bool runs_built;

static void build_runs(void) {
	runs_built = true;
	uint32_t glyphs = rom_off(BN6_CHAT_FONT), wide = rom_off(BN6_CHAT_FONT_WIDTHS);
	int n = 0;
	for (int ch = 0; ch < 128; ++ch) {
		run_first[ch] = (uint16_t)n;
		int code = ch ? text_code((unsigned char)ch) : -1;
		if (code < 0) continue;
		int w = R.data[wide + (uint32_t)code];
		run_w[ch] = (uint8_t)(w > 16 ? 16 : w);
		const uint8_t *g = R.data + glyphs + (uint32_t)code * 0x60;
		for (int y = 0; y < 12; ++y)
			for (int x = 0; x < run_w[ch];) {
				int v = x & 1 ? g[y * 8 + x / 2] >> 4 : g[y * 8 + x / 2] & 15;
				int k = x + 1;
				if (v == 1 || v == 3) {
					while (k < run_w[ch] && ((k & 1 ? g[y * 8 + k / 2] >> 4 : g[y * 8 + k / 2] & 15) == v)) ++k;
					if (n < RUNS) runs[n++] = (Run){ (uint8_t)x, (uint8_t)y, (uint8_t)(k - x), v == 3 };
				}
				x = k;
			}
	}
	run_first[128] = (uint16_t)n;
}

static bool have_font(void) { return R.data != NULL; }

int look_text_width(const char *s) {
	if (!have_font()) return minifont_width(s, 1);
	if (!runs_built) build_runs();
	int w = 0;
	for (; *s; ++s)
		if ((unsigned char)*s < 128) w += run_w[(unsigned char)*s];
	return w;
}

void look_text(int x, int y, const char *s, SDL_Color c, SDL_Color shade, int align) {
	if (align == TEXT_CENTER) x -= look_text_width(s) / 2;
	else if (align == TEXT_RIGHT) x -= look_text_width(s);
	if (!have_font()) { minifont_draw(x, y + 4, s, c, 1); return; }
	if (!runs_built) build_runs();
	SDL_Rect r[2][384];
	int n[2] = { 0, 0 };
	for (; *s; ++s) {
		unsigned char ch = (unsigned char)*s;
		if (ch >= 128) continue;
		for (int k = run_first[ch]; k < run_first[ch + 1]; ++k) {
			const Run *u = &runs[k];
			if (n[u->shade] < 384) r[u->shade][n[u->shade]++] = (SDL_Rect){ x + u->x, y + u->y, u->len, 1 };
		}
		x += run_w[ch];
	}
	if (shade.a) fill_rects(r[1], n[1], shade);
	fill_rects(r[0], n[0], c);
}

int look_wrapped(const char *s, int x, int y, int w, int most, SDL_Color c, SDL_Color shade) {
	const int line = LOOK_LINE;
	char cur[96] = "";
	int n = 0;
	most = most < 6 ? most : 6;
	for (const char *p = s; n < most;) {
		while (*p == ' ') ++p;
		size_t k = strcspn(p, " |");
		char next[192];
		snprintf(next, sizeof next, "%s%s%.*s", cur, *cur && k ? " " : "", (int)(k < 95 ? k : 95), p);
		if (*cur && k && look_text_width(next) > w) {
			look_text(x, y + n++ * line, cur, c, shade, TEXT_LEFT);
			snprintf(cur, sizeof cur, "%.*s", (int)(k < 95 ? k : 95), p);
		} else snprintf(cur, sizeof cur, "%.95s", next);
		p += k;
		if (*p == '|' || !*p) {
			if (*cur || *p == '|') look_text(x, y + n++ * line, cur, c, shade, TEXT_LEFT);
			*cur = 0;
			if (!*p) break;
			++p;
		}
	}
	return y + n * line;
}

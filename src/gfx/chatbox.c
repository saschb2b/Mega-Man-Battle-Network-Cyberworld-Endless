/* BN6's chat box (chatbox.h). The frame is drawn here to the measure of the
 * game's own (captured in a chat); the glyphs and their widths are read from
 * the ROM (BN6_CHAT_FONT). */
#include "chatbox.h"

#include <string.h>

#include "bn6.h"
#include "gfx.h"
#include "platform.h"
#include "rom.h"

#define LINE rgba(0x08, 0x6B, 0xC6, 255)
#define BAND rgba(0x4A, 0xBD, 0xF7, 255)
#define PAGE rgba(0xF7, 0xFF, 0xF7, 255)

static void span(int x0, int x1, int y, SDL_Color c) {
	if (x1 >= x0) fill_rect(x0, y, x1 - x0 + 1, 1, c);
}

void chatbox_frame(int x, int y, int w, int h) {
	int r = x + w - 1;
	/* by rows from the nearer edge: the line's, the band's and the page's
	 * insets from the sides (-1 none), which round the corners */
	for (int i = 0; i < h; ++i) {
		int e = i < h - 1 - i ? i : h - 1 - i;
		int line = e == 0 ? 2 : e == 1 ? 1 : 0;
		int band = e == 0 ? -1 : e == 1 ? 3 : e == 2 ? 2 : 1;
		int page = e < 3 ? -1 : e == 3 ? 4 : 3;
		span(x + line, r - line, y + i, LINE);
		if (band >= 0) span(x + band, r - band, y + i, BAND);
		if (page >= 0) span(x + page, r - page, y + i, PAGE);
	}
	/* the tab: its left side slopes, its band runs into the box's, whose
	 * top-right corner it squares */
	span(r - 48, r - 2, y - 4, LINE);
	span(r - 49, r - 1, y - 3, LINE);
	span(r - 47, r - 3, y - 3, BAND);
	span(r - 50, r, y - 2, LINE);
	span(r - 48, r - 2, y - 2, BAND);
	span(r - 51, r, y - 1, LINE);
	span(r - 49, r - 46, y - 1, BAND);
	span(r - 3, r - 1, y - 1, BAND);
	span(r - 50, r - 47, y, BAND);
	for (int i = 0; i < 3; ++i) {
		span(r - 3, r - 1, y + i, BAND);
		span(r, r, y + i, LINE);
	}
}

/* ---- The chat font: 16x12 cells of 4 bits a pixel, 0x60 bytes a code,
 * rows of 8 bytes, the left pixel in the low nibble; its index 1 is the
 * text and 3 a light shade, the rest the page. The atlas is built at first
 * use, a cell for each byte of text as the battle font's. */

static SDL_Texture *font;
static uint8_t widths[128];

static void build(void) {
	static uint32_t px[16 * 128 * 12];
	memset(px, 0, sizeof px);
	uint32_t glyphs = rom_off(BN6_CHAT_FONT), wide = rom_off(BN6_CHAT_FONT_WIDTHS);
	for (int ch = 1; ch < 128; ++ch) {
		int code = text_code((unsigned char)ch);
		if (code < 0) continue;
		int w = R.data[wide + (uint32_t)code];
		widths[ch] = (uint8_t)(w > 16 ? 16 : w);
		const uint8_t *g = R.data + glyphs + (uint32_t)code * 0x60;
		for (int y = 0; y < 12; ++y)
			for (int x = 0; x < widths[ch]; ++x) {
				int v = x & 1 ? g[y * 8 + x / 2] >> 4 : g[y * 8 + x / 2] & 15;
				if (v == 1 || v == 3) px[y * 16 * 128 + ch * 16 + x] = v == 1 ? 0xFF423939u : 0xFFDEE7DEu;
			}
	}
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(px, 16 * 128, 12, 32, 16 * 128 * 4, SDL_PIXELFORMAT_ARGB8888);
	font = SDL_CreateTextureFromSurface(P.renderer, s);
	SDL_SetTextureBlendMode(font, SDL_BLENDMODE_BLEND);
	SDL_FreeSurface(s);
}

int chatbox_text(int x, int y, const char *s, int n) {
	if (!font) build();
	int x0 = x;
	for (; *s && n != 0; ++s, --n) {
		unsigned char ch = (unsigned char)*s;
		if (ch >= 128) continue;
		SDL_Rect src = { ch * 16, 0, 16, 12 }, dst = { x, y, 16, 12 };
		SDL_RenderCopy(P.renderer, font, &src, &dst);
		x += widths[ch];
	}
	return x - x0;
}

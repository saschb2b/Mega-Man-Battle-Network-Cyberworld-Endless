/* The engine's own 3x5 font (minifont.h). */
#include "minifont.h"

#include "gfx.h"

/* Each glyph is five rows of three pixels, the top row in the highest
 * bits, the left pixel the row's highest bit; a row is written as its
 * pixels in decimal digits (101 is #.#, 10 is .#., 1 is ..#). */
#define R(r) (((r) / 100 % 10) << 2 | ((r) / 10 % 10) << 1 | (r) % 10)
#define G(a, b, c, d, e) (unsigned short)((R(a) << 12) | (R(b) << 9) | (R(c) << 6) | (R(d) << 3) | R(e))

/* (a table a character: one per character, 32 to 127, 0 for none) */
static const unsigned short glyphs[96] = {
	['A' - 32] = G(10, 101, 111, 101, 101),
	['B' - 32] = G(110, 101, 110, 101, 110),
	['C' - 32] = G(11, 100, 100, 100, 11),
	['D' - 32] = G(110, 101, 101, 101, 110),
	['E' - 32] = G(111, 100, 110, 100, 111),
	['F' - 32] = G(111, 100, 110, 100, 100),
	['G' - 32] = G(11, 100, 101, 101, 11),
	['H' - 32] = G(101, 101, 111, 101, 101),
	['I' - 32] = G(111, 10, 10, 10, 111),
	['J' - 32] = G(1, 1, 1, 101, 10),
	['K' - 32] = G(101, 101, 110, 101, 101),
	['L' - 32] = G(100, 100, 100, 100, 111),
	['M' - 32] = G(101, 111, 111, 101, 101),
	['N' - 32] = G(110, 101, 101, 101, 101),
	['O' - 32] = G(10, 101, 101, 101, 10),
	['P' - 32] = G(110, 101, 110, 100, 100),
	['Q' - 32] = G(10, 101, 101, 110, 11),
	['R' - 32] = G(110, 101, 110, 101, 101),
	['S' - 32] = G(11, 100, 10, 1, 110),
	['T' - 32] = G(111, 10, 10, 10, 10),
	['U' - 32] = G(101, 101, 101, 101, 111),
	['V' - 32] = G(101, 101, 101, 101, 10),
	['W' - 32] = G(101, 101, 111, 111, 101),
	['X' - 32] = G(101, 101, 10, 101, 101),
	['Y' - 32] = G(101, 101, 10, 10, 10),
	['Z' - 32] = G(111, 1, 10, 100, 111),
	['0' - 32] = G(111, 101, 101, 101, 111),
	['1' - 32] = G(10, 110, 10, 10, 111),
	['2' - 32] = G(110, 1, 10, 100, 111),
	['3' - 32] = G(110, 1, 10, 1, 110),
	['4' - 32] = G(101, 101, 111, 1, 1),
	['5' - 32] = G(111, 100, 110, 1, 110),
	['6' - 32] = G(11, 100, 111, 101, 111),
	['7' - 32] = G(111, 1, 10, 10, 10),
	['8' - 32] = G(111, 101, 111, 101, 111),
	['9' - 32] = G(111, 101, 111, 1, 110),
	['.' - 32] = G(0, 0, 0, 0, 10),
	[',' - 32] = G(0, 0, 0, 10, 100),
	[':' - 32] = G(0, 10, 0, 10, 0),
	[';' - 32] = G(0, 10, 0, 10, 100),
	['!' - 32] = G(10, 10, 10, 0, 10),
	['?' - 32] = G(110, 1, 10, 0, 10),
	['\'' - 32] = G(10, 10, 0, 0, 0),
	['"' - 32] = G(101, 101, 0, 0, 0),
	['(' - 32] = G(1, 10, 10, 10, 1),
	[')' - 32] = G(100, 10, 10, 10, 100),
	['-' - 32] = G(0, 0, 111, 0, 0),
	['+' - 32] = G(0, 10, 111, 10, 0),
	['*' - 32] = G(0, 101, 10, 101, 0),
	['=' - 32] = G(0, 111, 0, 111, 0),
	['/' - 32] = G(1, 1, 10, 100, 100),
	['_' - 32] = G(0, 0, 0, 0, 111),
	['~' - 32] = G(0, 11, 110, 0, 0),
	['&' - 32] = G(10, 101, 10, 101, 11),
	['%' - 32] = G(101, 1, 10, 100, 101),
};

unsigned short minifont_glyph(char ch) {
	if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
	unsigned char u = (unsigned char)ch;
	return u >= 32 && u < 128 ? glyphs[u - 32] : 0;
}

int minifont_width(const char *s, int scale) {
	int n = 0;
	for (; *s; ++s) ++n;
	return n ? (n * 4 - 1) * scale : 0;
}

void minifont_draw(int x, int y, const char *s, SDL_Color c, int scale) {
	for (; *s; ++s, x += 4 * scale) {
		unsigned short g = minifont_glyph(*s);
		for (int row = 0; row < 5; ++row)
			for (int col = 0; col < 3; ++col)
				if (g >> ((4 - row) * 3 + (2 - col)) & 1) fill_rect(x + col * scale, y + row * scale, scale, scale, c);
	}
}

void minifont_draw_centered(int x, int y, const char *s, SDL_Color c, int scale) {
	minifont_draw(x - minifont_width(s, scale) / 2, y, s, c, scale);
}

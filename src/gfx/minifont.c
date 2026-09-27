/* The engine's own 3x5 font (minifont.h). */
#include "minifont.h"

#include "gfx.h"

/* Each glyph is five rows of three pixels, the top row in the highest
 * bits, the left pixel the row's highest bit; a row is written as its
 * pixels in decimal digits (101 is #.#, 10 is .#., 1 is ..#). */
#define R(r) (((r) / 100 % 10) << 2 | ((r) / 10 % 10) << 1 | (r) % 10)
#define G(a, b, c, d, e) (unsigned short)((R(a) << 12) | (R(b) << 9) | (R(c) << 6) | (R(d) << 3) | R(e))

static unsigned short glyph(char ch) {
	if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
	switch (ch) {
	case 'A': return G(10, 101, 111, 101, 101);
	case 'B': return G(110, 101, 110, 101, 110);
	case 'C': return G(11, 100, 100, 100, 11);
	case 'D': return G(110, 101, 101, 101, 110);
	case 'E': return G(111, 100, 110, 100, 111);
	case 'F': return G(111, 100, 110, 100, 100);
	case 'G': return G(11, 100, 101, 101, 11);
	case 'H': return G(101, 101, 111, 101, 101);
	case 'I': return G(111, 10, 10, 10, 111);
	case 'J': return G(1, 1, 1, 101, 10);
	case 'K': return G(101, 101, 110, 101, 101);
	case 'L': return G(100, 100, 100, 100, 111);
	case 'M': return G(101, 111, 111, 101, 101);
	case 'N': return G(110, 101, 101, 101, 101);
	case 'O': return G(10, 101, 101, 101, 10);
	case 'P': return G(110, 101, 110, 100, 100);
	case 'Q': return G(10, 101, 101, 110, 11);
	case 'R': return G(110, 101, 110, 101, 101);
	case 'S': return G(11, 100, 10, 1, 110);
	case 'T': return G(111, 10, 10, 10, 10);
	case 'U': return G(101, 101, 101, 101, 111);
	case 'V': return G(101, 101, 101, 101, 10);
	case 'W': return G(101, 101, 111, 111, 101);
	case 'X': return G(101, 101, 10, 101, 101);
	case 'Y': return G(101, 101, 10, 10, 10);
	case 'Z': return G(111, 1, 10, 100, 111);
	case '0': return G(111, 101, 101, 101, 111);
	case '1': return G(10, 110, 10, 10, 111);
	case '2': return G(110, 1, 10, 100, 111);
	case '3': return G(110, 1, 10, 1, 110);
	case '4': return G(101, 101, 111, 1, 1);
	case '5': return G(111, 100, 110, 1, 110);
	case '6': return G(11, 100, 111, 101, 111);
	case '7': return G(111, 1, 10, 10, 10);
	case '8': return G(111, 101, 111, 101, 111);
	case '9': return G(111, 101, 111, 1, 110);
	case '.': return G(0, 0, 0, 0, 10);
	case ',': return G(0, 0, 0, 10, 100);
	case ':': return G(0, 10, 0, 10, 0);
	case ';': return G(0, 10, 0, 10, 100);
	case '!': return G(10, 10, 10, 0, 10);
	case '?': return G(110, 1, 10, 0, 10);
	case '\'': return G(10, 10, 0, 0, 0);
	case '"': return G(101, 101, 0, 0, 0);
	case '(': return G(1, 10, 10, 10, 1);
	case ')': return G(100, 10, 10, 10, 100);
	case '-': return G(0, 0, 111, 0, 0);
	case '+': return G(0, 10, 111, 10, 0);
	case '=': return G(0, 111, 0, 111, 0);
	case '/': return G(1, 1, 10, 100, 100);
	case '_': return G(0, 0, 0, 0, 111);
	case '~': return G(0, 11, 110, 0, 0);
	case '&': return G(10, 101, 10, 101, 11);
	default: return 0;
	}
}

int minifont_width(const char *s, int scale) {
	int n = 0;
	for (; *s; ++s) ++n;
	return n ? (n * 4 - 1) * scale : 0;
}

void minifont_draw(int x, int y, const char *s, SDL_Color c, int scale) {
	for (; *s; ++s, x += 4 * scale) {
		unsigned short g = glyph(*s);
		for (int row = 0; row < 5; ++row)
			for (int col = 0; col < 3; ++col)
				if (g >> ((4 - row) * 3 + (2 - col)) & 1) fill_rect(x + col * scale, y + row * scale, scale, scale, c);
	}
}

void minifont_draw_centered(int x, int y, const char *s, SDL_Color c, int scale) {
	minifont_draw(x - minifont_width(s, scale) / 2, y, s, c, scale);
}

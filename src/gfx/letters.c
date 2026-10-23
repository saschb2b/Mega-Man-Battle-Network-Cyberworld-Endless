/* letters.h */
#include "letters.h"

#include <stdbool.h>

static bool filled(const LetterStrip *s, int x, int y) {
	return x >= 0 && x < s->w && y >= 0 && y < s->h && s->px[y * s->w + x] == s->fill;
}

static bool column_filled(const LetterStrip *s, int x) {
	for (int y = 0; y < s->h; ++y)
		if (filled(s, x, y)) return true;
	return false;
}

int letters_cut(const LetterStrip *s, int at[][2], int max) {
	int n = 0;
	for (int x = 0; x < s->w && n < max; ++x) {
		if (!column_filled(s, x)) continue;
		at[n][0] = x;
		while (x + 1 < s->w && column_filled(s, x + 1)) ++x;
		at[n++][1] = x;
	}
	return n;
}

int letters_top(const LetterStrip *s) {
	for (int y = 0; y < s->h; ++y)
		for (int x = 0; x < s->w; ++x)
			if (filled(s, x, y)) return y;
	return -1;
}

/* letter n's columns, false where it has none */
static bool letter_at(const LetterStrip *s, int n, int *first, int *last) {
	int at[64][2];
	if (n < 0 || n >= letters_cut(s, at, 64)) return false;
	*first = at[n][0];
	*last = at[n][1];
	return true;
}

int letters_put(const LetterStrip *s, int n, LetterWord *w, int x, int dy, const uint8_t map[16]) {
	int first, last;
	if (!letter_at(s, n, &first, &last)) return -1;
	/* (its fill's columns and the shade's after them) */
	int cols = last - first + 2;
	if (x < 0 || x + cols > w->w) return -1;
	for (int y = 0; y < s->h; ++y) {
		int to = y + dy;
		if (to < 0 || to >= w->h) continue;
		for (int c = 0; c < cols && first + c < s->w; ++c) {
			uint8_t v = map[s->px[y * s->w + first + c] & 15];
			if (v) w->px[to * w->w + x + c] = v;
		}
	}
	return x + cols;
}

int letters_put_closed(const LetterStrip *s, int n, LetterWord *w, int x, uint8_t fill, uint8_t shade) {
	int first, last;
	if (!letter_at(s, n, &first, &last)) return -1;
	/* the O: the C's columns 0..wide-1 and their mirror, 1..wide */
	int wide = last - first + 1, cols = wide + 2;
	if (x < 0 || x + cols > w->w || s->h > w->h) return -1;
#define O_AT(c, y) ((c) >= 0 && (c) <= wide && (filled(s, first + (c), (y)) || filled(s, first + wide - (c), (y))))
	for (int y = 0; y < s->h; ++y)
		for (int c = 0; c < cols; ++c) {
			uint8_t *p = &w->px[y * w->w + x + c];
			if (O_AT(c, y)) *p = fill;
			else if (O_AT(c - 1, y) || O_AT(c, y - 1) || O_AT(c - 1, y - 1)) *p = shade;
		}
#undef O_AT
	return x + cols;
}

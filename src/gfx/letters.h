/* A word in the game's own lettering, set from other words' letters: a
 * strip of the ROM's pixels (the title's PRESS START, a PET menu label)
 * is cut where a column holds none of its letters' fill, and the letters
 * picked are set side by side a column apart, each with the shade it
 * casts, as the game sets them (the title's OPTIONS, the PET's Options).
 * The pixels are the player's ROM's, read as the game runs: none are in
 * the source. */
#ifndef CW_LETTERS_H
#define CW_LETTERS_H

#include <stdint.h>

typedef struct {
	const uint8_t *px;   /* colour indices, row-major, w x h */
	int w, h;
	uint8_t fill;        /* the letters' own colour */
} LetterStrip;

typedef struct {
	uint8_t *px;         /* the word's colour indices, row-major, w x h */
	int w, h;
} LetterWord;

/* The strip's letters, left to right: the first and last column of each
 * one's fill (at[n][0], at[n][1]); how many, at most `max`. */
int letters_cut(const LetterStrip *s, int at[][2], int max);
/* The first row holding the strip's fill, -1 where none does. */
int letters_top(const LetterStrip *s);
/* Letter `n` of `s` set into `w` at column x, `dy` rows lower, with the
 * column its shade falls in after it: the pixels `map` takes (a strip's
 * colour as the word takes it, 0 not taken: the background), as they
 * are. The next letter's column, or -1 where `n` is no letter or it does
 * not fit. */
int letters_put(const LetterStrip *s, int n, LetterWord *w, int x, int dy, const uint8_t map[16]);
/* A C closed into an O: letter n's fill and its mirror image, a column
 * wider, in `fill`, shaded in `shade` right of, below and below-right of
 * the fill, as BN6's menu font shades its letters. The next letter's
 * column, or -1. */
int letters_put_closed(const LetterStrip *s, int n, LetterWord *w, int x, uint8_t fill, uint8_t shade);

#endif

/* What Central Town's people say of the run and the runs (town_words.c,
 * docs/HOME.md, a town that remembers), for town.c. */
#ifndef CW_TOWN_WORDS_H
#define CW_TOWN_WORDS_H

/* The words of the person of sprite list `cat`'s `sprite` at this visit
 * home: the run's news, its progress, the runs before it, where they have
 * a line of their own; else `place_line`, what is said where they stand. */
const char *town_folk_words(int cat, int sprite, const char *place_line);

#endif

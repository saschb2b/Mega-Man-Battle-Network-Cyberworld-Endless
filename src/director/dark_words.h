/* MegaMan on DarkChips (dark_words.c), for the director's other parts. */
#ifndef CW_DARK_WORDS_H
#define CW_DARK_WORDS_H

#include <stdbool.h>
#include <stddef.h>

/* which of MegaMan's words on a DarkChip's price (dark_price_words) */
enum { DARK_PRICE_BN6, DARK_PRICE_FIRST, DARK_PRICE_MORE };
const char *dark_price_words(int kind, int lost);
void dark_base_words(char *out, size_t n, const char *dark, const char *base);
/* What a DarkChip played by BN5's rule does, for a flame's words (pages
 * apart by '|'), "" where its words say none: DarkPlus's `plus` more for
 * the next attack chip, DarkInvs's (`invs`) ten seconds of the darkness
 * fighting for him, unseen (BN5's 600 frames; the owner's calls of 13
 * October 2026) */
const char *dark_bn5_does(int plus, bool invs);

#endif

/* MegaMan on DarkChips (dark_words.c), for the director's other parts. */
#ifndef CW_DARK_WORDS_H
#define CW_DARK_WORDS_H

#include <stddef.h>

void dark_rose_said(void);
/* which of MegaMan's words on a DarkChip's price (dark_price_words) */
enum { DARK_PRICE_BN6, DARK_PRICE_FIRST, DARK_PRICE_MORE };
const char *dark_price_words(int kind, int lost);
void dark_base_words(char *out, size_t n, const char *dark, const char *base);

#endif

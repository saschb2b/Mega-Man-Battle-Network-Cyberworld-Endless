/* What the layer's navis say of what stands on it, and MegaMan at what he
 * finds there (layer_words.c). */
#ifndef CW_LAYER_WORDS_H
#define CW_LAYER_WORDS_H

#include <stdbool.h>
#include <stddef.h>

#include "shop.h"
#include "text.h"

/* A shop's words: its greeting, its line when visited again, sold out */
typedef struct { const char *hello, *again, *sold_out; } ShopWords;

/* What the Net Dealer speaks of */
typedef struct {
	int navi;                /* the act's guardian, 0 none */
	bool tells, again;       /* he names him (the net's word has come back); met in this act already */
	int counter;             /* the element that answers the act */
	const ShopItem *stock;   /* his list, his pick first */
	int nstock;
	bool purple, skull;      /* purple data, skull doors on this layer */
} DealerTalk;

ShopWords dealer_words(const DealerTalk *d);
ShopWords vendor_words(char names[][16], int n);
/* ProtoMan's word where his netbattle waits for a later act */
const char *netbattle_later_words(void);
void duel_terms(char *terms, size_t n, int met, int foes, int frames, int rung);
const char *rumor_words(int navi);
const char *hinter_words(void);
const char *fragment_words(int held);
/* What a Spin of `colour` does, and that it stays (`first`: the profile's
 * first, how they are found too): MegaMan's script, in `text` */
int spin_words(TextArchive *text, int colour, bool first);

#endif

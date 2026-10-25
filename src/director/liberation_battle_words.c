/* What a liberation's battle stakes, in words: the menu's lines, in BN6's
 * small capitals, as BN5's own banners count it (TURN, LIBERATE). */
#include "liberation_battle_words.h"

#include <stdio.h>

void lib_stakes_words(const LibStakes *s, char *a, size_t na, char *b, size_t nb) {
	b[0] = 0;
	if (s->darkloid) {
		snprintf(a, na, "%d TURNS: DELETES HIM", s->turns);
		return;
	}
	if (s->hole) {
		snprintf(a, na, "%d TURNS: CLOSES IT", s->turns);
		snprintf(b, nb, "FREES %d", s->freed);
		return;
	}
	if (s->unit) snprintf(a, na, "%d TURNS: DELETES IT", s->turns);
	else snprintf(a, na, "%d TURNS: FREES %d", s->turns, s->freed);
	if (s->one_turn > 0) snprintf(b, nb, "1 TURN: %d MORE", s->one_turn);
}

void lib_surrounded_words(const LibStakes *s, char *c, size_t nc) {
	c[0] = 0;
	if (s->surrounded) snprintf(c, nc, "SURROUNDED: L/R TURNS");
}

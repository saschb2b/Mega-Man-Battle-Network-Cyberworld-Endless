/* square_looks_pick.h. */
#include "square_looks_pick.h"

#include <stdlib.h>

#define MOST 16   /* peers a pick weighs at most (a channel shows seven) */

bool square_looks_same(const Look *a, const Look *b) {
	for (int c = 0; c < LOOK_CHANNELS; ++c)
		if (a->palette[c] != b->palette[c] || a->colour[c] != b->colour[c]) return false;
	return true;
}

static bool plain(const Look *l) {
	for (int c = 0; c < LOOK_CHANNELS; ++c)
		if (l->palette[c] != LOOK_NONE || l->colour[c] != LOOK_NONE) return false;
	return true;
}

static const LookPeer *sorting;
static int hold_by;

/* nearest first, one drawn in his look last time `hold_by` nearer than he stands */
static int nearer(const void *a, const void *b) {
	const LookPeer *p = &sorting[*(const int *)a], *q = &sorting[*(const int *)b];
	int dp = p->distance - (p->held ? hold_by : 0), dq = q->distance - (q->held ? hold_by : 0);
	if (dp != dq) return dp < dq ? -1 : 1;
	return *(const int *)a - *(const int *)b;
}

/* The looks drawn, `room` palettes in all with MegaMan's: true where every
 * peer has his own (none falls back to BN6's own). */
static bool fill(const LookPeer *peers, const int *order, int n, const Look *own, int room, bool *shown) {
	Look drawn[MOST + 1];
	int count = 0;
	drawn[count++] = *own;
	bool all = true;
	for (int k = 0; k < n; ++k) {
		int i = order[k];
		bool known = false;
		for (int j = 0; j < count && !known; ++j) known = square_looks_same(&drawn[j], &peers[i].look);
		if (!known && count < room) { drawn[count++] = peers[i].look; known = true; }
		shown[i] = known;
		all &= known;
	}
	return all;
}

void square_looks_pick(const LookPeer *peers, int n, const Look *own, int cap, int hold, bool *shown) {
	if (n > MOST) n = MOST;
	int order[MOST];
	for (int i = 0; i < n; ++i) order[i] = i;
	sorting = peers; hold_by = hold;
	qsort(order, (size_t)n, sizeof order[0], nearer);
	int room = cap < 0 ? 1 : cap + 1;
	if (fill(peers, order, n, own, room, shown) || plain(own)) return;
	/* (a peer falls back to BN6's own colours, which MegaMan does not wear:
	 * they take a palette of the room too, unless a peer drawn wears them) */
	bool plain_drawn = false;
	for (int i = 0; i < n; ++i) plain_drawn |= shown[i] && plain(&peers[i].look);
	if (!plain_drawn) fill(peers, order, n, own, room - 1, shown);
}

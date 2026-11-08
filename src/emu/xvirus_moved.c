/* xvirus_moved.h: the ranges the installs moved, kept for a trace's reader. */
#include "xvirus_moved.h"

#include <stdio.h>
#include <stdlib.h>

#define FIRST 512   /* ranges room is made for at first, doubled as it fills (BN5's, BN4's and the chips' together past 1000) */

static XvMoved *moved;
static int nmoved, room, lost;
static int keep = -1;   /* (-1: not yet asked; the environment asked at the first range) */

void xvirus_moved_keep(void) { keep = 1; }

void xv_moved_add(uint32_t from, uint32_t to, uint32_t len) {
	if (keep < 0) keep = getenv("CYBERWORLD_BTRACE") || getenv("CYBERWORLD_XDRAWS");   /* (battle_trace.h's readers) */
	if (!keep) return;
	for (int i = 0; i < nmoved; ++i)
		if (moved[i].from == from && moved[i].to == to) return;
	if (nmoved == room) {
		int more = room ? 2 * room : FIRST;
		XvMoved *m = realloc(moved, (size_t)more * sizeof *m);
		if (!m) {
			if (!lost++) fprintf(stderr, "xvirus: no memory for the moved ranges past %d: a trace is refused\n", nmoved);
			return;
		}
		moved = m;
		room = more;
	}
	moved[nmoved++] = (XvMoved){ from, to, len };
}

const XvMoved *xvirus_moved(int *n) {
	*n = 0;
	if (keep != 1) {
		fprintf(stderr, "xvirus: the moved ranges were not kept (xvirus_moved_keep before the installs): a trace is refused\n");
		return NULL;
	}
	if (lost) {
		fprintf(stderr, "xvirus: %d moved ranges lost to the heap: a trace is refused\n", lost);
		return NULL;
	}
	static const XvMoved none[1];   /* (none moved yet: an empty list, not a refusal) */
	*n = nmoved;
	return moved ? moved : none;
}

void xvirus_moved_reset(void) {
	free(moved);
	moved = NULL;
	nmoved = room = lost = 0;
	keep = -1;
}

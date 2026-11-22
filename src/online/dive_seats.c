/* dive_seats.h. */
#include "dive_seats.h"

#define MOST 16   /* a room's divers this looks at, at most */

/* whether diver a comes before b: a crewmate before a stranger, then the
 * nearer, then the lower session (the same order every frame) */
static bool before(const SeatDiver *a, const SeatDiver *b, int mx, int my) {
	if (a->crew != b->crew) return a->crew;
	long ax = a->x - mx, ay = a->y - my, bx = b->x - mx, by = b->y - my;
	long da = ax * ax + ay * ay, db = bx * bx + by * by;
	if (da != db) return da < db;
	return a->id < b->id;
}

int dive_seats(uint32_t *seat, int seats, const SeatDiver *d, int n, int mx, int my) {
	int order[MOST];
	if (n > MOST) n = MOST;
	/* (the room in order, by insertion: eight at most) */
	for (int i = 0; i < n; ++i) {
		int j = i;
		while (j > 0 && before(&d[i], &d[order[j - 1]], mx, my)) { order[j] = order[j - 1]; --j; }
		order[j] = i;
	}
	int chosen = n < seats ? n : seats;
	bool seated[MOST] = { false };
	/* (a puppet whose diver is still chosen keeps him; the others are freed) */
	for (int k = 0; k < seats; ++k) {
		bool keep = false;
		for (int c = 0; c < chosen && !keep; ++c)
			if (seat[k] && seat[k] == d[order[c]].id && !seated[c]) seated[c] = keep = true;
		if (!keep) seat[k] = 0;
	}
	/* (the chosen not yet seated, into the freed puppets in order) */
	for (int c = 0, k = 0; c < chosen; ++c) {
		if (seated[c]) continue;
		while (k < seats && seat[k]) ++k;
		if (k < seats) seat[k] = d[order[c]].id;
	}
	return n - chosen;
}

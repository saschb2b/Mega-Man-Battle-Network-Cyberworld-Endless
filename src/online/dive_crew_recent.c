/* dive_crew_recent.h */
#include "dive_crew_recent.h"

#include <string.h>

#include "dive_crew.h"

#define KEPT 8   /* crewmates remembered: a crew's three, twice over, and room */

static struct {
	uint32_t session[KEPT], at[KEPT];
	int n;
} R;

void dive_crew_recent_reset(void) { memset(&R, 0, sizeof R); }

static void keep(uint32_t session, uint32_t now) {
	int k = 0;
	while (k < R.n && R.session[k] != session) ++k;
	if (k == R.n) {
		if (R.n < KEPT) ++R.n;
		else {
			/* (full: the one longest out of the crew makes room) */
			k = 0;
			for (int j = 1; j < KEPT; ++j)
				if (R.at[j] < R.at[k]) k = j;
		}
	}
	R.session[k] = session;
	R.at[k] = now;
}

void dive_crew_recent_note(const CrewState *before, const CrewState *after, uint32_t me, uint32_t now) {
	for (int k = 0; k < before->count; ++k)
		if (before->m[k].session != me) keep(before->m[k].session, now);
	for (int k = 0; k < after->count; ++k)
		if (after->m[k].session != me) keep(after->m[k].session, now);
}

/* whether `session` was in MegaMan's crew within CREW_RECENT_MS (in it now counts) */
static int was(uint32_t session, uint32_t now) {
	if (dive_crew_has(session)) return 1;
	for (int k = 0; k < R.n; ++k)
		if (R.session[k] == session) return now - R.at[k] <= CREW_RECENT_MS;
	return 0;
}

int dive_crew_recent_here(const OnlineSnapshot *s, uint32_t now, uint32_t *out, int most) {
	int n = 0;
	for (size_t i = 0; s->connected && i < s->peer_count && n < most; ++i) {
		const OnlinePeer *p = &s->peers[i];
		if (p->id != s->player_id && p->busy == 0 && was(p->id, now)) out[n++] = p->id;
	}
	return n;
}

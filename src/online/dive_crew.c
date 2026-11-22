/* dive_crew.h */
#include "dive_crew.h"

#include <string.h>

#include <SDL.h>

#include "dive_comings.h"
#include "dive_crew_view.h"
#include "dive_join.h"
#include "online_transport.h"

static CrewState C;           /* the crew as the server last said (count 0 alone) */

void dive_crew_reset(void) { memset(&C, 0, sizeof C); }

int dive_crew_count(void) { return C.count; }

const CrewMember *dive_crew_member(int k) { return k >= 0 && k < C.count ? &C.m[k] : NULL; }

void dive_crew_describe(FILE *f) {
	fprintf(f, "crew %d", dive_crew_count());
	const CrewMember *m;
	for (int k = 0; (m = dive_crew_member(k)) != NULL; ++k) fprintf(f, "%s%s", k ? " " : ": ", m->name);
	fputc('\n', f);
	dive_join_describe(f);
	dive_crew_view_describe(f);
	dive_comings_describe(f);
}

bool dive_crew_has(uint32_t session) {
	for (int k = 0; k < C.count; ++k)
		if (C.m[k].session == session) return true;
	return false;
}

/* (the log's line, for the scenarios and a session's eye; nothing said in the game: the ring shows it, #716) */
static void log_crew(const char *what, const CrewState *c) {
	fprintf(stderr, "dive: crew %s:", what);
	for (int k = 0; k < c->count; ++k) fprintf(stderr, " %s", c->m[k].name);
	fputc('\n', stderr);
}

/* this game's session */
static uint32_t me(void) {
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	return s.player_id;
}

void dive_crew_mail(const uint8_t *m, size_t n) {
	CrewState c;
	if (!dive_wire_crew_state(m, n, &c)) return;
	/* (a crewmate's battle offered, or MegaMan's own fought alone: the join-in's, dive_join.h) */
	if (c.stage == ONLINE_CREW_OFFER || c.stage == ONLINE_CREW_ALONE) dive_join_crew_state(&c);
	else if (c.stage == ONLINE_CREW_FULL) dive_comings_full(&c);
	else if (c.stage == ONLINE_CREW_FORMED || c.stage == ONLINE_CREW_VITALS) {
		bool changed = c.count != C.count;
		for (int k = 0; !changed && k < c.count; ++k) changed = c.m[k].session != C.m[k].session;
		dive_comings_crew(&C, &c, me(), SDL_GetTicks());
		C = c;
		if (changed) log_crew("now", &C);
	} else if (c.stage == ONLINE_CREW_SPLIT_UP && C.count) {
		CrewState none;
		memset(&none, 0, sizeof none);
		dive_comings_crew(&C, &none, me(), SDL_GetTicks());
		C = none;
		log_crew("none", &C);
	}
}

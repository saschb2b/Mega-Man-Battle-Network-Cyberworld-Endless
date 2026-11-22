/* dive_mail.h; dive_link_mail (dive_link.h) */
#include "dive_mail.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "analytics_online.h"
#include "dive_crew.h"
#include "dive_link.h"
#include "dive_pack.h"
#include "dive_party.h"
#include "dive_splits.h"
#include "dive_traces.h"
#include "dive_wire.h"
#include "dive_words.h"
#include "online_mail.h"
#include "online_transport.h"
#include "online_week.h"
#include "weekly.h"

static struct {
	int layers, shared, most, crew, parties, party;   /* the dive's company (analytics_dive_together) */
} M;

void dive_mail_reset(void) { memset(&M, 0, sizeof M); }

/* the dive's end's wait for the Board's answer (the server posts the end and sends WEEK_BOARD at once, week.py) */
#define END_WAIT_MS 6000
static struct {
	bool on;
	uint32_t since, serial;
} A;

void dive_mail_end_stop(void) {
	if (!A.on) return;
	A.on = false;
	weekly_end_pump = NULL;
	online_transport_end();
}

/* a frame of the wait (the end screen's, weekly_end_pump): the Board come in with this diver's place, or the time up */
static void end_pump(void) {
	if (!A.on) return;
	online_mail_frame();
	const WeekBoard *b = online_week_board();
	if (b->serial != A.serial) {
		weekly_end_placed(b->place > 0 ? b->place : -1);
		fprintf(stderr, "dive: the Board's answer: place %d\n", b->place);
		dive_mail_end_stop();
	} else if (SDL_GetTicks() - A.since > END_WAIT_MS) {
		weekly_end_placed(-1);
		fprintf(stderr, "dive: no answer from the Board\n");
		dive_mail_end_stop();
	}
}

bool dive_mail_told(bool linked) {
	analytics_dive_together(M.layers, M.shared, M.most, M.crew, M.parties, M.party);
	dive_mail_end_stop();
	if (!linked) { weekly_end_placed(-1); return false; }
	A.on = true;
	A.since = SDL_GetTicks();
	A.serial = online_week_board()->serial;
	weekly_end_placed(0);
	weekly_end_pump = end_pump;
	return true;
}

/* the parties begun and the crews formed, counted for the statistics */
static void together(const uint8_t *m, size_t n) {
	QueueState q;
	CrewState c;
	if (m[0] == ONLINE_QUEUE_STATE && dive_wire_queue_state(m, n, &q) && q.stage == ONLINE_QUEUE_STARTED) {
		++M.parties;
		if (q.count > M.party) M.party = q.count;
	}
	if (m[0] == ONLINE_CREW_STATE && dive_wire_crew_state(m, n, &c) && c.stage == ONLINE_CREW_FORMED && c.count > M.crew) M.crew = c.count;
}

/* the traces, a split's place and the Board's records (dive_traces.h, dive_splits.h, dive_pack.h) */
static void traces_mail(const uint8_t *m, size_t n) {
	static DiveTraces t;
	static WeekSplits w;
	DivePlace p;
	switch (m[0]) {
	case ONLINE_DIVE_TRACES: if (dive_wire_traces(m, n, &t)) dive_traces_mail(&t); break;
	case ONLINE_DIVE_PLACE: if (dive_wire_place(m, n, &p)) dive_splits_place(&p); break;
	case ONLINE_WEEK_SPLITS: if (dive_wire_splits(m, n, &w)) dive_pack_splits(&w); break;
	default: break;
	}
}

static unsigned depth_here;   /* MegaMan's layer, as the server last put him in a room */

static void room(const DiveRoom *r) {
	depth_here = r->way.depth;
	fprintf(stderr, "dive: in layer %u's room %u, %u divers\n", r->way.depth, r->instance, r->divers);
	++M.layers;
	M.shared += r->divers > 1;
	if (r->divers > M.most) M.most = r->divers;
}

void dive_link_mail(const uint8_t *m, size_t n) {
	DiveRoom r;
	DiveFeed f;
	DiveDivers d;
	switch (m[0]) {
	case ONLINE_DIVE_ROOM:
		if (dive_wire_room(m, n, &r)) room(&r);
		break;
	case ONLINE_DIVE_FEED:
		if (dive_wire_feed(m, n, &f)) dive_link_line(dive_feed_words(f.kind, f.name, f.way.depth, f.party, depth_here));
		break;
	case ONLINE_DIVERS:
		if (dive_wire_divers(m, n, &d)) dive_pack_divers(&d);
		break;
	case ONLINE_QUEUE_STATE: case ONLINE_ARENA_STATE: case ONLINE_ARENA_SEAT: case ONLINE_PARTY_HP:
		together(m, n);
		dive_party_mail(m, n);
		break;
	case ONLINE_CREW_STATE:
		together(m, n);
		dive_crew_mail(m, n);
		break;
	default: traces_mail(m, n); break;
	}
}

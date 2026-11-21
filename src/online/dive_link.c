/* dive_link.h */
#include "dive_link.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "director.h"
#include "dive_band.h"
#include "dive_crew.h"
#include "dive_join.h"
#include "dive_party.h"
#include "dive_finds.h"
#include "dive_mail.h"
#include "dive_people.h"
#include "dive_splits.h"
#include "dive_traces.h"
#include "dive_wire.h"
#include "emu.h"
#include "online_mail.h"
#include "online_transport.h"
#include "run.h"
#include "square_week.h"
#include "weekly.h"

#define LOST_MS 20000        /* a link down so long leaves the dive solo */

static struct {
	bool on;
	bool starting;           /* begun, the scene's restart for it still to come */
	uint16_t week;
	uint32_t seed;
	DiveWay room;            /* the layer the server has MegaMan in (depth 0 none) */
	int x, y, z;             /* his place as last sent */
	uint32_t lost_at;
	uint32_t resumes;        /* the transport's resumes seen (issue #180: a resume puts him in no room) */
} L;

void dive_link_adopt(void) {
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	memset(&L, 0, sizeof L);
	if (!s.connected || !s.week || !s.week_seed) {
		fprintf(stderr, "dive: no link to keep; the dive goes on alone\n");
		online_transport_end();
		dive_people_on(false);
		return;
	}
	L.on = true;
	dive_party_on(true);
	dive_crew_reset();
	dive_join_reset();
	dive_mail_reset();
	dive_band_reset();
	L.week = s.week;
	L.seed = s.week_seed;
	L.resumes = s.resumes;
	dive_people_on(true);
	fprintf(stderr, "dive: the link kept for week %u's dive\n", s.week);
}

bool dive_link_on(void) { return L.on; }

void dive_link_describe(FILE *f) {
	if (!L.on) return;
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	fprintf(f, "divers %u", (unsigned)s.peer_count);
	for (size_t i = 0; i < s.peer_count; ++i) fprintf(f, "%s%s", i ? " " : ": ", s.peers[i].name);
	fputc('\n', f);
	for (size_t i = 0; i < s.peer_count; ++i) fprintf(f, "diver %s at %d %d\n", s.peers[i].name, (int)s.peers[i].x, (int)s.peers[i].y);
	dive_people_describe(f);
	dive_crew_describe(f);
	fprintf(f, "link %s\n", s.reconnecting ? "reconnecting" : s.quiet ? "quiet" : s.connected ? "up" : "down");
}

uint32_t dive_link_keys(uint32_t keys) {
	return L.on ? dive_traces_keys(dive_party_keys(dive_join_keys(keys))) : dive_traces_keys(keys);
}

bool dive_link_begin(void) {
	/* (the puppets' room asked for before the dive's first layer is built) */
	dive_people_on(true);
	if (!square_week_dive()) { dive_people_on(false); return false; }
	dive_link_adopt();
	L.starting = L.on;
	return true;
}


bool dive_link_diving(void) { return weekly_on(); }

void dive_link_close(void) {
	if (!L.on) return;
	L.on = false;
	dive_people_on(false);
	dive_party_on(false);
	dive_join_reset();
	dive_crew_reset();
	online_transport_end();
}

static void send(const uint8_t *m, size_t n) {
	if (n && !online_transport_send(m, n)) fprintf(stderr, "dive: a message of kind %u not sent\n", m[0]);
}

/* the dive over (deleted or won): told where, then the link closed */
static void ended(void) {
	const WeeklyRecord *r = weekly_record();
	uint8_t m[ONLINE_DIVE_END_BYTES];
	send(m, dive_wire_end(m, r->pending && r->r_won ? ONLINE_DIVE_WON : ONLINE_DIVE_DELETED, L.x, L.y, L.z, dive_splits_cause()));
	dive_mail_told();
	fprintf(stderr, "dive: the dive's end told; the link closed\n");
	dive_link_close();
}

void dive_link_scene_left(void) {
	if (L.starting) return;
	/* (a deletion or a win ends the run and leaves the scene in one frame,
	 * before dive_link_frame sees it: its end told here, or no mark is left) */
	if (L.on && !weekly_on() && weekly_record()->pending) ended();
	else dive_link_close();
}

/* the layer MegaMan stands on, its room entered as it changes */
static void room(void) {
	DiveWay w = { (uint8_t)run.depth, (uint8_t)run.side_kind, (uint8_t)run.biome };
	if (w.depth == L.room.depth && w.side == L.room.side && w.way == L.room.way) return;
	uint8_t m[ONLINE_DIVE_ENTER_BYTES];
	size_t n = dive_wire_enter(m, L.week, L.seed, w);
	send(m, n);
	if (n) { L.room = w; dive_traces_room(w); }
}


void dive_link_frame(void) {
	if (!L.on) return;
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	uint32_t now = SDL_GetTicks();
	if (s.error[0] || (!s.connected && L.lost_at && now - L.lost_at > LOST_MS)) {
		fprintf(stderr, "dive: the link is gone (%s); the dive goes on alone\n", s.error[0] ? s.error : "lost");
		dive_link_close();
		return;
	}
	L.starting = false;
	L.lost_at = s.connected ? 0 : L.lost_at ? L.lost_at : now;
	/* (the link back after a drop: the server took him out of his room and
	 * his crew as it dropped, holds.py's dive.dropped, so the room is entered
	 * again below and the crew shown is the server's next word; a battle
	 * under way goes on, its seat held, party.py) */
	if (s.connected && !s.reconnecting && s.resumes != L.resumes) {
		L.resumes = s.resumes;
		memset(&L.room, 0, sizeof L.room);
		dive_crew_reset();
		fprintf(stderr, "dive: the link back; the layer's room entered again\n");
	}
	online_mail_frame();
	/* (the dive over: its end told where it ended, else only the link let go) */
	if (!weekly_on()) {
		if (weekly_record()->pending) ended();
		else dive_link_close();
		return;
	}
	dive_band_frame(now);
	/* (the party at a guardian's door, and its fight; the crew's battles) */
	dive_party_frame(&s, L.room);
	dive_join_frame();
	/* (at home, between acts: nothing sent, his last place stands) */
	if (!director_on_layer() || !s.connected) return;
	room();
	L.x = bn6_player_x(); L.y = bn6_player_y(); L.z = bn6_player_z();
	online_transport_poll(L.x, L.y, L.z, emu_read8(BN6_PLAYER_FACING) & 7u);
	dive_people_frame(&s);
	dive_traces_frame();
	dive_finds_frame();
	dive_splits_frame();
}







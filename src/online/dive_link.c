/* dive_link.h */
#include "dive_link.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "director.h"
#include "dive_finds.h"
#include "dive_pack.h"
#include "dive_people.h"
#include "dive_splits.h"
#include "dive_traces.h"
#include "dive_wire.h"
#include "dive_words.h"
#include "emu.h"
#include "gfx.h"
#include "online_mail.h"
#include "online_transport.h"
#include "pet_look.h"
#include "platform.h"
#include "run.h"
#include "square_week.h"
#include "weekly.h"

#define FEED_MOST 4          /* lines waiting their turn */
#define FEED_MS 3500         /* each shown so long */
#define LOST_MS 20000        /* a link down so long leaves the dive solo */

static struct {
	bool on;
	bool starting;           /* begun, the scene's restart for it still to come */
	uint16_t week;
	uint32_t seed;
	DiveWay room;            /* the layer the server has MegaMan in (depth 0 none) */
	int x, y, z;             /* his place as last sent */
	uint32_t lost_at;
	char feed[FEED_MOST][64];
	int nfeed;
	uint32_t shown_at;       /* the first line's start, 0 not yet */
	uint16_t divers;         /* how many dive now, as DIVERS last said */
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
	L.week = s.week;
	L.seed = s.week_seed;
	dive_people_on(true);
	fprintf(stderr, "dive: the link kept for week %u's dive\n", s.week);
}

bool dive_link_begin(void) {
	/* (the puppets' room asked for before the dive's first layer is built) */
	dive_people_on(true);
	if (!square_week_dive()) { dive_people_on(false); return false; }
	dive_link_adopt();
	L.starting = L.on;
	return true;
}

void dive_link_scene_left(void) {
	if (L.starting) return;
	dive_link_close();
}

bool dive_link_diving(void) { return weekly_on(); }

void dive_link_close(void) {
	if (!L.on) return;
	L.on = false;
	dive_people_on(false);
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
	fprintf(stderr, "dive: the dive's end told; the link closed\n");
	dive_link_close();
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
	online_mail_frame();
	/* (the dive over: its end told where it ended, else only the link let go) */
	if (!weekly_on()) {
		if (weekly_record()->pending) ended();
		else dive_link_close();
		return;
	}
	if (L.nfeed && L.shown_at && now - L.shown_at > FEED_MS) {
		memmove(L.feed, L.feed + 1, sizeof L.feed[0] * (size_t)(L.nfeed - 1));
		L.shown_at = --L.nfeed ? now : 0;
	}
	if (L.nfeed && !L.shown_at) L.shown_at = now;
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

void dive_link_line(const char *line) {
	if (!*line) return;
	if (L.nfeed == FEED_MOST) { memmove(L.feed, L.feed + 1, sizeof L.feed[0] * (FEED_MOST - 1)); --L.nfeed; L.shown_at = 0; }
	snprintf(L.feed[L.nfeed++], sizeof L.feed[0], "%s", line);
	fprintf(stderr, "dive: feed: %s\n", line);
}

static void feed(const DiveFeed *f) { dive_link_line(dive_feed_words(f->kind, f->name, f->way.depth, f->party)); }

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

void dive_link_mail(const uint8_t *m, size_t n) {
	DiveRoom r;
	DiveFeed f;
	DiveDivers d;
	switch (m[0]) {
	case ONLINE_DIVE_ROOM:
		if (dive_wire_room(m, n, &r)) fprintf(stderr, "dive: in layer %u's room %u, %u divers\n", r.way.depth, r.instance, r.divers);
		break;
	case ONLINE_DIVE_FEED:
		if (dive_wire_feed(m, n, &f)) feed(&f);
		break;
	case ONLINE_DIVERS:
		if (dive_wire_divers(m, n, &d)) { L.divers = d.total; dive_pack_divers(&d); }
		break;
	default: traces_mail(m, n); break;
	}
}

void dive_link_draw(void) {
	if (!L.on || !L.nfeed || !director_on_map()) return;
	look_band(P.core_x, P.core_y + 160 - LOOK_BAND_H - 1, 240, L.feed[0]);
}

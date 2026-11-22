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

/* A dropped link on a dive (issue #751, docs/online/arena/dropped-link.md,
 * A dropped link on a dive layer): the transport tries again for its 30 seconds,
 * and past them the dive takes its link up again as long as it lasts, a
 * new start of the link after these waits; the band says it is down, that
 * he dives alone meanwhile, and that it is back. Only the server's refusal
 * (another version, a ban, maintenance) ends the link for the dive, said
 * plainly first. */
#define AGAIN_FIRST_MS 3000
#define AGAIN_MOST_MS  15000
#define DOWN_SHOWN_MS  1000   /* a link down past a second says so (a moment's quiet does not) */
#define ALONE_SHOWN_MS 8000   /* the refusal's word shown so long before the link ends */
#define URL_BYTES 256         /* a server address, network_prefs.h's NETWORK_URL */

static struct {
	bool on;
	bool starting;           /* begun, the scene's restart for it still to come */
	uint16_t week;
	uint32_t seed;
	DiveWay room;            /* the layer the server has MegaMan in (depth 0 none) */
	int x, y, z;             /* his place as last sent */
	uint32_t resumes;        /* the transport's resumes seen (issue #180: a resume puts him in no room) */
	uint32_t down_since;     /* the link down since (SDL ticks), 0 up */
	bool away;               /* the transport gave up: started again from again_at, `tries` so far */
	uint32_t again_at;
	unsigned tries;
	uint32_t alone_at;       /* the server's refusal: the dive alone from then, its word shown, 0 none */
} L;

/* CONTINUE into a dive (issue #759): the dive taken up with no link, the
 * Square called as the gate calls it, the link down until it answers and
 * tried again as a dropped one is */
static unsigned seen;        /* the dive's resumes handled (weekly_resumes) */

/* the dive's link on, for week `week`'s net on `seed` */
static void link_up(uint16_t week, uint32_t seed) {
	L.on = true;
	dive_party_on(true);
	dive_crew_reset();
	dive_join_reset();
	dive_mail_reset();
	dive_band_reset();
	L.week = week;
	L.seed = seed;
	dive_people_on(true);
}

void dive_link_adopt(void) {
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	memset(&L, 0, sizeof L);
	/* (a link down as the portal is taken keeps the dive's: it is taken up again as any drop on a layer, issue #751;
	 * only a game that never heard the week goes on alone) */
	if (!s.week || !s.week_seed || (!s.connected && !s.reconnecting && !s.error[0] && !s.player_id)) {
		fprintf(stderr, "dive: no link to keep; the dive goes on alone\n");
		online_transport_end();
		dive_people_on(false);
		return;
	}
	link_up(s.week, s.week_seed);
	L.resumes = s.resumes;
	fprintf(stderr, "dive: the link kept for week %u's dive\n", s.week);
	if (!s.connected) {
		L.down_since = SDL_GetTicks() | 1u;
		fprintf(stderr, "dive: the link down as the dive begins; kept, taken up again once back\n");
	}
}

bool dive_link_on(void) { return L.on; }

/* (the link's state in the state's words: up, down, away (the transport
 * given up, started again now and then) or alone (refused)) */
static const char *state_word(void) { return L.alone_at ? "alone" : L.away ? "away" : L.down_since ? "down" : "up"; }

void dive_link_describe(FILE *f) {
	if (!L.on) return;
	fprintf(f, "divestate %s\n", state_word());
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	fprintf(f, "divers %u", (unsigned)s.peer_count);
	for (size_t i = 0; i < s.peer_count; ++i) fprintf(f, "%s%s", i ? " " : ": ", s.peers[i].name);
	fputc('\n', f);
	for (size_t i = 0; i < s.peer_count; ++i) fprintf(f, "diver %s at %d %d\n", s.peers[i].name, (int)s.peers[i].x, (int)s.peers[i].y);
	dive_people_describe(f);
	dive_party_describe(f);
	dive_crew_describe(f);
	dive_band_describe(f);
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

/* the link's parts let go; the transport too where `ending` is false (a dive's end keeps it for the Board's answer) */
static void let_go(bool ending) {
	L.on = false;
	dive_people_on(false);
	dive_party_on(false);
	dive_join_reset();
	dive_crew_reset();
	if (!ending) online_transport_end();
}

void dive_link_close(void) {
	dive_mail_end_stop();
	if (L.on) let_go(false);
}

static void send(const uint8_t *m, size_t n) {
	if (n && !online_transport_send(m, n)) fprintf(stderr, "dive: a message of kind %u not sent\n", m[0]);
}

/* the dive over (deleted or won): told where, then the link closed */
static void ended(void) {
	/* (this dive's own end, which the server posts to the Board: the week's best the record keeps may be another's) */
	const WeeklyEnd *w = weekly_last_end();
	DiveEnd e = { w->won ? ONLINE_DIVE_WON : ONLINE_DIVE_DELETED, L.x, L.y, L.z, dive_splits_cause(), w->minutes,
		w->pulled ? ONLINE_WEEK_PULLED : 0u, 0 };
	uint8_t m[ONLINE_DIVE_END_BYTES];
	send(m, dive_wire_end(m, &e));
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	bool waits = dive_mail_told(s.connected && !s.reconnecting);
	fprintf(stderr, "dive: the dive's end told; %s\n", waits ? "the Board's answer waited for" : "the link closed");
	let_go(waits);
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


/* the wait before the link's next start, away: 3 seconds, then twice as long, 15 at most */
static uint32_t again_wait(unsigned tries) {
	uint32_t wait = AGAIN_FIRST_MS << (tries < 3 ? tries : 3);
	return wait < AGAIN_MOST_MS ? wait : AGAIN_MOST_MS;
}

/* the transport gave up (an error): a refusal of the server's ends the
 * link once its word has been shown; else the link is started again now
 * and then for as long as the dive lasts. True while away or alone: the
 * rest of the frame waits */
static bool held_away(const OnlineSnapshot *s, uint32_t now) {
	if (L.alone_at) {
		if ((int32_t)(now - L.alone_at) >= ALONE_SHOWN_MS) dive_link_close();
		return true;
	}
	if (!s->error[0]) return false;
	if (s->error_code) {
		fprintf(stderr, "dive: the server turned the link down (%s); the dive goes on alone\n", s->error);
		L.alone_at = now | 1u;
		dive_band_alone();
		return true;
	}
	if (!L.away) {
		fprintf(stderr, "dive: the link is gone (%s); diving alone, the link started again from time to time\n", s->error);
		L.away = true;
		L.tries = 0;
		L.again_at = now + again_wait(0);
		if (!L.down_since) L.down_since = now | 1u;
	}
	if ((int32_t)(now - L.again_at) >= 0) {
		online_transport_again();
		L.again_at = now + again_wait(++L.tries);
	}
	return true;
}

/* the link down (dropped, tried again) or back: back after a drop, the
 * server took him out of his room and his crew as it dropped (holds.py's
 * dive.dropped, or a new session altogether after the transport gave up),
 * so the room is entered again below and the crew shown is the server's
 * next word; a battle under way goes on, its seat held, party.py */
static void back_or_down(const OnlineSnapshot *s, uint32_t now) {
	bool down = !s->connected || s->reconnecting;
	if (down) {
		if (!L.down_since) L.down_since = now | 1u;
		return;
	}
	bool again = L.away || s->resumes != L.resumes;
	if (L.down_since && (int32_t)(now - L.down_since) >= DOWN_SHOWN_MS) dive_band_back();
	L.down_since = 0;
	L.away = false;
	L.resumes = s->resumes;
	if (!again) return;
	memset(&L.room, 0, sizeof L.room);
	dive_crew_reset();
	fprintf(stderr, "dive: the link back; the layer's room entered again\n");
}

bool dive_link_down(uint32_t now, bool *away) {
	if (!L.on || L.alone_at || !L.down_since || (int32_t)(now - L.down_since) < DOWN_SHOWN_MS) return false;
	*away = L.away;
	return true;
}

/* a dive taken up again with no link (the title's CONTINUE; the portal's comes with its link): the Square called,
 * the link down until it answers and tried again meanwhile as a dropped link is; no address kept, the dive alone */
static void call_again(void) {
	if (L.on || !weekly_on()) { seen = weekly_resumes(); return; }
	if (seen == weekly_resumes()) return;
	seen = weekly_resumes();
	char url[URL_BYTES], name[ONLINE_NAME_BYTES + 1];
	uint8_t id[ONLINE_IDENTITY_BYTES];
	bool called = director_gate_address(url, sizeof url, name, sizeof name, id) && online_transport_start(url, name, id);
	const WeeklyRecord *r = weekly_record();
	memset(&L, 0, sizeof L);
	link_up(r->week, r->seed);
	uint32_t now = SDL_GetTicks();
	L.down_since = now | 1u;
	if (!called) { L.alone_at = now | 1u; dive_band_alone(); }
	fprintf(stderr, "dive: CONTINUE into week %u's dive; %s\n", r->week, called ? "the Square called" : "no Square to call");
}

/* the Square on a week past the dive's (its own or the last kept: dive.py's enter): the dive alone for good */
static bool past_week(const OnlineSnapshot *s, uint32_t now) {
	if (!s->connected || !s->week || (s->week <= L.week + 1u && s->week >= L.week)) return false;
	fprintf(stderr, "dive: the Square is on week %u, the dive week %u's; the dive goes on alone\n", s->week, L.week);
	L.alone_at = now | 1u;
	dive_band_week();
	return true;
}

void dive_link_frame(void) {
	call_again();
	if (!L.on) {
		if (weekly_on()) dive_band_tick(SDL_GetTicks());
		return;
	}
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	uint32_t now = SDL_GetTicks();
	if (held_away(&s, now) || past_week(&s, now)) return;
	L.starting = false;
	back_or_down(&s, now);
	online_mail_frame();
	/* (the dive over: its end told where it ended, else only the link let go) */
	if (!weekly_on()) {
		if (weekly_record()->pending) ended();
		else dive_link_close();
		return;
	}
	dive_band_frame(now, s.connected, s.player_id);
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







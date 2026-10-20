#include "square_life.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "chips.h"
#include "debug.h"
#include "emu.h"
#include "netmap.h"
#include "netpass.h"
#include "npc.h"
#include "online_board.h"
#include "online_square.h"
#include "square_arena.h"
#include "square_watch.h"
#include "square_life_words.h"
#include "square_week.h"
#include "talk.h"
#include "text.h"
#include "trade_central.h"

#define WALKER_PHASE 977   /* frames the second walker's round runs behind the first's */
#define PARADE_EVERY (180 * 60)   /* a parade every three minutes of the wall clock, */
#define PARADE_LONG (30 * 60)     /* thirty seconds long, */
#define PARADE_GAP 64             /* the walkers two panels apart */
#define BEAM_FRAMES 48     /* BN6's log-in beam shown so long */
#define BEAM_SOUND 0x76    /* ... and its sound (stage_npc.c's) */
#define CHEER_SOUND 0x94   /* BN6's sparkle as something shows (stage_npc.c's SOUND_APPEAR) */
#define MARK_Z 46          /* the "!" over a Navi's head (lanhp.c's courier's) */
#define MARK_FRAMES 120
#define BARK_FRAMES 180    /* a word over a head shown so long */
#define TOGETHER 240       /* emotes this close count as together */
#define CHEER_REST 600     /* the Square answers once in so long */
#define SETTLE 90          /* frames before log-ins are greeted (who was here already is not) */
#define SPOT 3             /* panels round the landmark, or the stage's middle, that count */
#define FACE_GREEN_NAVI 0x3E   /* GreenNavi's mugshot (text.h's FACE_NAVI: list 6's sprite 62) */

typedef struct { int x[SQUARE_ROUTE], y[SQUARE_ROUTE], len[SQUARE_ROUTE], pause[SQUARE_ROUTE], n; uint32_t period; } Route;

static struct {
	bool ready;
	OnlineSnapshot s;
	Route route[SQUARE_WALKERS], parade;
	bool parading;
	int hide_t;   /* frames the walkers stay hidden, logging in to or out of the parade */
	int wx[SQUARE_WALKERS], wy[SQUARE_WALKERS], talking;
	time_t second;
	unsigned sub, frames, turn;
	struct { uint32_t id; int x, y, z; char name[ONLINE_NAME_BYTES + 1]; } seen[SQUARE_PEERS];
	int nseen;
	char arrived[ONLINE_NAME_BYTES + 1];
	int beam_want, beam_anim, beam_t, beam[3];
	int mark_t, mark_on;
	bool mark_sound;
	const char *bark;
	int bark_t, bark_at[3], bark_on;   /* (bark_on: the walker it follows, -1 none) */
	uint32_t event_serial;
	struct { uint32_t id; unsigned at; } cheer[ONLINE_MAX_PLAYERS];
	unsigned cheered_at;
	uint32_t greeter_said, referee_said;
	unsigned bouts;   /* the battles fought from the Square the referee has called */
} L;

void square_life_reset(void) {
	memset(&L, 0, sizeof L);
	L.talking = L.beam_want = L.bark_on = -1;
}

/* the wall clock in frames: every client whose clock agrees walks the
 * walkers alike (the server sends no clock of its own) */
static uint64_t wall_frames(void) {
	time_t now = time(NULL);
	if (now != L.second) { L.second = now; L.sub = 0; }
	else if (L.sub < 59) ++L.sub;
	return (uint64_t)now * 60u + L.sub;
}

/* BN6's facing (0-7) nearest world direction (dx, dy), by its own probes */
static int face_to(int dx, int dy) {
	int best = 3;
	double best_c = -2;
	double d = sqrt((double)dx * dx + (double)dy * dy);
	if (d < 1) return best;
	for (int k = 0; k < 8; ++k) {
		int px, py;
		npc_probe(k, &px, &py);
		double p = sqrt((double)px * px + (double)py * py);
		double c = p > 0 ? (px * dx + py * dy) / (p * d) : -2;
		if (c > best_c) { best_c = c; best = k; }
	}
	return best;
}

/* a loop of `n` panels' corners as a route in world units, its pauses in
 * frames (seconds given, or none) */
static void route_of(Route *r, const int (*at)[3], const int (*corners)[2], int n) {
	r->period = 0;
	r->n = n;
	for (int i = 0; i < n; ++i) {
		netmap_world(at ? at[i][0] : corners[i][0], at ? at[i][1] : corners[i][1], &r->x[i], &r->y[i]);
		r->pause[i] = at ? at[i][2] * 60 : 0;
	}
	for (int i = 0; i < n; ++i) {
		int j = (i + 1) % n;
		r->len[i] = abs(r->x[j] - r->x[i]) + abs(r->y[j] - r->y[i]);
		r->period += (uint32_t)(r->pause[i] + r->len[i]);
	}
}

/* the plan's routes and its parade in world units */
static void setup(void) {
	const SquarePlan *p = online_square_plan();
	for (int k = 0; k < SQUARE_WALKERS; ++k) route_of(&L.route[k], (const int (*)[3])p->route[k], NULL, p->route_len[k]);
	route_of(&L.parade, NULL, (const int (*)[2])p->parade, 4);
	L.event_serial = L.s.event_serial;
	L.bouts = square_arena_last()->bouts;
	L.ready = true;
}

static bool parade_on(uint64_t t) { return t % PARADE_EVERY < PARADE_LONG; }

/* where on route `r` a walker is `ph` frames into its round: standing at a
 * waypoint facing the next, then walking to it a world unit a frame */
static void route_at(const Route *r, uint32_t ph, int *x, int *y, int *anim) {
	*x = r->x[0]; *y = r->y[0]; *anim = 3;
	for (int i = 0; i < r->n; ++i) {
		int j = (i + 1) % r->n, dx = r->x[j] - r->x[i], dy = r->y[j] - r->y[i], face = face_to(dx, dy);
		if (ph < (uint32_t)r->pause[i]) { *x = r->x[i]; *y = r->y[i]; *anim = face; return; }
		ph -= (uint32_t)r->pause[i];
		if (ph < (uint32_t)r->len[i]) {
			*x = r->x[i] + dx * (int)ph / r->len[i];
			*y = r->y[i] + dy * (int)ph / r->len[i];
			*anim = face + 8;
			return;
		}
		ph -= (uint32_t)r->len[i];
	}
}

/* walker k at frame t of the wall clock: on his route, or in the parade,
 * the second two panels behind the first */
static void walker_at(int k, uint64_t t, int *x, int *y, int *anim) {
	if (parade_on(t) && L.parade.period) {
		uint32_t into = (uint32_t)(t % PARADE_EVERY), back = (uint32_t)k * PARADE_GAP % L.parade.period;
		route_at(&L.parade, (into + L.parade.period - back) % L.parade.period, x, y, anim);
		return;
	}
	const Route *r = &L.route[k];
	route_at(r, r->period ? (uint32_t)((t + (uint64_t)k * WALKER_PHASE) % r->period) : 0, x, y, anim);
}

static void bark(const char *text, int x, int y, int z, int on);

/* the parade begun or ended: the walkers log out where they were and in
 * where they go (BN6's beam where the first does), the first's word as
 * it begins */
static void parade_turn(uint64_t t) {
	bool on = parade_on(t);
	if (on == L.parading) return;
	L.parading = on;
	if (L.frames <= 1) return;   /* (as MegaMan arrives: they are where they are) */
	L.hide_t = BEAM_FRAMES;
	int x, y, anim;
	walker_at(0, t, &x, &y, &anim);
	L.beam_want = 0;
	L.beam[0] = x; L.beam[1] = y; L.beam[2] = 0;
	if (on) bark(square_parade_bark(), x, y, 0, 0);
}

static void walkers(uint64_t t) {
	parade_turn(t);
	if (L.hide_t) --L.hide_t;
	for (int k = 0; k < SQUARE_WALKERS; ++k) {
		int x, y, anim;
		if (L.talking == k && talk_busy()) {
			/* (spoken to: he stands facing MegaMan till the talk ends) */
			x = L.wx[k]; y = L.wy[k];
			anim = face_to(bn6_player_x() - x, bn6_player_y() - y);
		} else {
			if (L.talking == k) L.talking = -1;
			walker_at(k, t, &x, &y, &anim);
		}
		L.wx[k] = x; L.wy[k] = y;
		online_square_actor(SQUARE_ACTOR_WALKER0 + k, !L.hide_t, anim, x, y, 0);
	}
}

/* a word over a head: at world (x, y, z), or over walker `on` as he goes */
static void bark(const char *text, int x, int y, int z, int on) {
	L.bark = text;
	L.bark_t = BARK_FRAMES;
	L.bark_at[0] = x; L.bark_at[1] = y; L.bark_at[2] = z;
	L.bark_on = on;
}

/* a Navi logged in (`in`) or out at (x, y, z): BN6's beam there, and the
 * greeter's word on it */
static void logged(const char *name, bool in, int x, int y, int z) {
	L.beam_want = in ? 0 : 1;
	L.beam[0] = x; L.beam[1] = y; L.beam[2] = z;
	const SquarePlan *p = online_square_plan();
	int gx, gy;
	netmap_world(p->at[SQ_GREETER][0], p->at[SQ_GREETER][1], &gx, &gy);
	bark(square_greeter_bark(name, in), gx, gy, 0, -1);
	if (in) snprintf(L.arrived, sizeof L.arrived, "%s", name);
}

/* a battle of this game's over, MegaMan back: the referee's call on it */
static void bout_called(void) {
	const SquareBout *b = square_arena_last();
	if (b->bouts == L.bouts) return;
	L.bouts = b->bouts;
	const SquarePlan *p = online_square_plan();
	int rx, ry;
	netmap_world(p->at[SQ_REFEREE][0], p->at[SQ_REFEREE][1], &rx, &ry);
	bark(square_referee_bark((int)b->result, b->name), rx, ry, 0, -1);
	printf("square: the referee calls bout %u: %s (%u-%u)\n", b->bouts, L.bark, b->won, b->lost);
}

static bool still_here(const OnlineSnapshot *s, uint32_t id) {
	for (size_t j = 0; j < s->peer_count; ++j) if (s->peers[j].id == id) return true;
	return false;
}

/* who logged in and out since the last frame (those here as MegaMan came
 * are not greeted) */
static void arrivals(const OnlineSnapshot *s) {
	bool settled = L.frames > SETTLE;
	for (int i = 0; i < L.nseen && settled; ++i)
		if (!still_here(s, L.seen[i].id)) logged(L.seen[i].name, false, L.seen[i].x, L.seen[i].y, L.seen[i].z);
	for (size_t j = 0; j < s->peer_count && settled; ++j) {
		bool seen = false;
		for (int i = 0; i < L.nseen; ++i) seen |= L.seen[i].id == s->peers[j].id;
		if (!seen) logged(s->peers[j].name, true, s->peers[j].x, s->peers[j].y, s->peers[j].z);
	}
	L.nseen = 0;
	for (size_t j = 0; j < s->peer_count && L.nseen < SQUARE_PEERS; ++j) {
		const OnlinePeer *p = &s->peers[j];
		L.seen[L.nseen].id = p->id;
		L.seen[L.nseen].x = p->x; L.seen[L.nseen].y = p->y; L.seen[L.nseen].z = p->z;
		snprintf(L.seen[L.nseen++].name, sizeof L.seen[0].name, "%s", p->name);
	}
}

/* the beam as asked: an animation shown again is first changed while
 * hidden (BN6 starts one only as it changes) */
static void beam_frame(void) {
	if (L.beam_want >= 0 && L.beam_want == L.beam_anim) {
		L.beam_anim ^= 1;
		online_square_actor(SQUARE_ACTOR_BEAM, false, L.beam_anim, L.beam[0], L.beam[1], L.beam[2]);
		return;
	}
	if (L.beam_want >= 0) {
		L.beam_anim = L.beam_want;
		L.beam_want = -1;
		L.beam_t = BEAM_FRAMES;
		online_square_actor(SQUARE_ACTOR_BEAM, true, L.beam_anim, L.beam[0], L.beam[1], L.beam[2]);
		online_square_actor_sound(SQUARE_ACTOR_BEAM, BEAM_SOUND);
		return;
	}
	if (L.beam_t) --L.beam_t;
	online_square_actor(SQUARE_ACTOR_BEAM, L.beam_t > 0, L.beam_anim, L.beam[0], L.beam[1], L.beam[2]);
}

/* world (x, y) round the landmark or on the stage */
static bool cheer_spot(int x, int y) {
	const SquarePlan *p = online_square_plan();
	int gx, gy;
	if (!netmap_panel(x, y, &gx, &gy) || gx < 0 || gy < 0 || gx >= SQUARE_W || gy >= SQUARE_H) return false;
	bool stage = p->level[gy][gx] && abs(gx - p->at[SQ_STAGE][0]) <= SPOT && abs(gy - p->at[SQ_STAGE][1]) <= SPOT;
	return stage || (abs(gx - p->at[SQ_CENTER][0]) <= SPOT && abs(gy - p->at[SQ_CENTER][1]) <= SPOT);
}

static bool where(const OnlineSnapshot *s, uint32_t id, int *x, int *y) {
	if (id == s->player_id) { *x = bn6_player_x(); *y = bn6_player_y(); return true; }
	for (size_t j = 0; j < s->peer_count; ++j)
		if (s->peers[j].id == id) { *x = s->peers[j].x; *y = s->peers[j].y; return true; }
	return false;
}

/* the walkers answer: a "!" over the one nearer the landmark, a word over
 * the other, and BN6's sparkle */
static void cheer(void) {
	const SquarePlan *p = online_square_plan();
	int cx, cy;
	netmap_world(p->at[SQ_CENTER][0], p->at[SQ_CENTER][1], &cx, &cy);
	int d0 = abs(L.wx[0] - cx) + abs(L.wy[0] - cy), d1 = abs(L.wx[1] - cx) + abs(L.wy[1] - cy);
	L.mark_on = d1 < d0;
	L.mark_t = MARK_FRAMES;
	int other = !L.mark_on;
	bark(square_cheer_bark(), L.wx[other], L.wy[other], 0, other);
	L.mark_sound = true;
	L.cheered_at = L.frames;
	if (emu_debug_on()) fprintf(stderr, "online: the Square cheers (walker %d marked)\n", L.mark_on);
}

/* a phrase or an emote by player `id` at a cheering spot noted; two
 * players' within TOGETHER of each other cheer */
static void noted(uint32_t id) {
	int slot = 0;
	for (int i = 0; i < ONLINE_MAX_PLAYERS; ++i) {
		if (L.cheer[i].id == id) { slot = i; break; }
		if (L.cheer[i].at < L.cheer[slot].at) slot = i;
	}
	L.cheer[slot].id = id;
	L.cheer[slot].at = L.frames;
	if (emu_debug_on()) fprintf(stderr, "online: player %u spoke at a cheering spot\n", id);
	int together = 0;
	for (int i = 0; i < ONLINE_MAX_PLAYERS; ++i) together += L.cheer[i].id && L.frames - L.cheer[i].at <= TOGETHER;
	if (together >= 2 && (!L.cheered_at || L.frames - L.cheered_at > CHEER_REST)) cheer();
}

static void emotes(const OnlineSnapshot *s) {
	uint32_t newest = L.event_serial;
	for (unsigned i = 0; i < ONLINE_EVENT_HISTORY; ++i) {
		const OnlineEvent *e = &s->events[i];
		if (!e->serial || (int32_t)(e->serial - L.event_serial) <= 0) continue;
		if ((int32_t)(e->serial - newest) > 0) newest = e->serial;
		int x, y;
		if (where(s, e->player_id, &x, &y) && cheer_spot(x, y)) noted(e->player_id);
	}
	L.event_serial = newest;
}

static void mark_frame(void) {
	if (L.mark_t) --L.mark_t;
	int k = L.mark_on;
	/* (a step before him in depth, drawn over him where they meet) */
	online_square_actor(SQUARE_ACTOR_MARK, L.mark_t > 0, 0, L.wx[k] - 1, L.wy[k] + 1, MARK_Z);
	/* (after the frame's place: the sound plays as the script runs on) */
	if (L.mark_sound) online_square_actor_sound(SQUARE_ACTOR_MARK, CHEER_SOUND);
	L.mark_sound = false;
}

/* the trade board's offers, the Square's names on them: MegaMan's own first */
static void board_posts(SquareFacts *f, const OnlineSnapshot *s) {
	const BoardOffer *all;
	int n = online_board_offers(&all);
	for (int pass = 0; pass < 2; ++pass)
		for (int i = 0; i < n && f->nposts < 4; ++i) {
			bool mine = all[i].player == s->player_id;
			if (mine != !pass) continue;
			const char *name = NULL;
			for (size_t k = 0; !mine && k < s->peer_count; ++k) if (s->peers[k].id == all[i].player) name = s->peers[k].name;
			if (!mine && !name) continue;
			char chip[24];
			chips_name(all[i].have.id, chip, sizeof chip);
			snprintf(f->posts[f->nposts].chip, sizeof f->posts[0].chip, "%s %c", chip, all[i].have.code == 26 ? '*' : 'A' + all[i].have.code);
			f->posts[f->nposts].name = name;
			f->posts[f->nposts++].want = all[i].want;
		}
}

static void facts(SquareFacts *f) {
	memset(f, 0, sizeof *f);
	const OnlineSnapshot *s = &L.s;
	for (size_t i = 0; i < s->peer_count && f->nnames < 3; ++i) f->names[f->nnames++] = s->peers[i].name;
	f->navis = (unsigned)s->peer_count + 1;
	f->channel = s->channel;
	f->channels = s->channels;
	time_t now = time(NULL);
	const struct tm *lt = localtime(&now);
	f->hour = lt ? lt->tm_hour : 12;
	const SquarePlan *p = online_square_plan();
	f->area = p->area;
	f->landmark = p->landmark;
	f->arrived = L.arrived[0] ? L.arrived : NULL;
	f->traded = trade_central_last();
	const SquareBout *b = square_arena_last();
	f->result = (int)b->result;
	f->fought = b->name;
	f->won = b->won;
	f->lost = b->lost;
	f->arena = square_arena_offered();
	f->stamp = netpass_has(STAMP_ARENA);
	f->parade = L.parading;
	f->battle_on = square_watch_live();
	f->turn = L.turn;
	board_posts(f, s);
}

/* (what the words were made of, to write them again as it changes) */
static uint32_t name_hash(const char *name, uint32_t h) {
	for (const char *c = name ? name : ""; *c; ++c) h = (h ^ (uint8_t)*c) * 16777619u;
	return h;
}

/* the greeter's and the referee's words as the channel stands, rewritten
 * as it changes (never under an open chat box) */
static void progs(void) {
	if (emu_read8(BN6_CHATBOX)) return;
	SquareFacts f;
	facts(&f);
	uint32_t g = name_hash(f.arrived, f.navis | f.channel << 8) ^ square_week_said() * 2654435761u;
	uint32_t r = name_hash(f.fought, f.navis | (uint32_t)f.result << 8 | (f.won + f.lost) << 10) ^ square_watch_said();
	if (g + 1 != L.greeter_said && square_week_greeter(square_greeter_words(&f))) L.greeter_said = g + 1;
	/* (the channel's NetBattle and the question to watch it after his own words: square_watch.c) */
	if (r + 1 != L.referee_said && square_watch_referee(square_referee_words(&f))) L.referee_said = r + 1;
}

void square_life_frame(const OnlineSnapshot *s) {
	L.s = *s;
	if (!L.ready) setup();
	++L.frames;
	uint64_t t = wall_frames();
	walkers(t);
	if (emu_debug_on() && t % 600 == 0)
		fprintf(stderr, "online: walkers at clock %llu: %d,%d and %d,%d\n", (unsigned long long)(t / 60), L.wx[0], L.wy[0], L.wx[1], L.wy[1]);
	arrivals(s);
	bout_called();
	beam_frame();
	emotes(s);
	mark_frame();
	progs();
	if (L.bark_t) --L.bark_t;
}

/* the prop MegaMan faces: on the panel ahead of him, along his probe
 * (BN6's own, 8 units long) 20 and 40 units on; the landmark on any
 * panel of its base */
static int prop_ahead(int x, int y, int face) {
	const SquarePlan *p = online_square_plan();
	int dx, dy;
	npc_probe(face, &dx, &dy);
	double d = sqrt((double)dx * dx + (double)dy * dy);
	if (d < 1) return -1;
	static const int reach[2] = { 20, 40 };
	for (int step = 0; step < 2; ++step) {
		int gx, gy;
		if (!netmap_panel(x + (int)(dx * reach[step] / d), y + (int)(dy * reach[step] / d), &gx, &gy)) continue;
		for (int i = 0; i < p->nprops; ++i)
			if (p->props[i].x == gx && p->props[i].y == gy) return p->props[i].look;
		/* (the landmark's walled base round it) */
		if (abs(gx - p->at[SQ_CENTER][0]) <= 1 && abs(gy - p->at[SQ_CENTER][1]) <= 1) return p->landmark;
	}
	return -1;
}

bool square_life_talk(int x, int y, int face) {
	if (!L.ready) return false;
	SquareFacts f;
	facts(&f);
	int k = online_square_actor_ahead(x, y, face);
	if (k >= 0) {
		L.talking = k;
		++L.turn;
		return talk_start(square_walker_words(k, &f), k ? FACE_TECH : FACE_GREEN_NAVI);
	}
	int look = prop_ahead(x, y, face);
	if (look == LOOK_BBS) {
		/* (the Board's posts, the week's net's after them: square_week.c) */
		static char posts[1800];
		const char *stands = square_watch_board();
		snprintf(posts, sizeof posts, "%s|%s%s%s", square_check_words(look, &f), stands, *stands ? "|" : "", square_week_board_words());
		return talk_start(posts, FACE_NONE);
	}
	return look >= 0 && talk_start(square_check_words(look, &f), FACE_NONE);
}

bool square_life_bark(const char **text, int *x, int *y, int *z) {
	if (!L.bark_t || !L.bark) return false;
	*text = L.bark;
	bool walker = L.bark_on >= 0 && L.bark_on < SQUARE_WALKERS;
	*x = walker ? L.wx[L.bark_on] : L.bark_at[0];
	*y = walker ? L.wy[L.bark_on] : L.bark_at[1];
	*z = L.bark_at[2];
	return true;
}

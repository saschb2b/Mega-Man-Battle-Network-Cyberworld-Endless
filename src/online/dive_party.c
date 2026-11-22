/* dive_party.h */
#include "dive_party.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "boss.h"
#include "bn6.h"
#include "bytes.h"
#include "dive_crew.h"
#include "dive_join.h"
#include "dive_words.h"
#include "emu.h"
#include "online_transport.h"
#include "party_play.h"
#include "pet_look.h"
#include "talk.h"

#define SEND_EVERY 6          /* frames between the fight's messages, each sent only on a change */
#define ARENA_START 3         /* ARENA_STATE's stage START (shared/online/README.md) */
#define ASK_AGAIN_MS 3000     /* a full door's queue asked again for a place (issue #753) */
#define CREW_WAIT_MS 10000    /* the door's wait for crewmates on the layer (issue #752, the owner's call) */
#define HOLD_FRAMES 45        /* A held this long goes in early: never one press (issue #752) */
#define CALM_FRAMES 20        /* the door's A only after a chat box has been shut this long: not the paging's */
#define DOWN_ALONE_MS 10000   /* a queued diver's link down this long: in alone (issue #751) */
#define REJOIN_MS 500         /* ... back sooner: the queue joined again this long on, his room entered first */

enum { DOOR_NONE, DOOR_ALONE, DOOR_QUEUED, DOOR_FULL, DOOR_STARTED };

static struct {
	int door;                 /* DOOR_* */
	DiveWay at;               /* the door's layer */
	QueueState q;
	uint32_t keys;            /* the pad's last frame, for a press */
	uint32_t notices;         /* the NOTICEs counted as the join was sent (a refusal is the next) */
	uint32_t asked_at;        /* the join's last ask (ms) */
	uint32_t me;              /* this game's session */
	uint32_t waited_from;     /* the door's wait's start (ms): the join */
	int pending;              /* crewmates on the layer not yet in the queue */
	int calm;                 /* frames since a chat box or a talk was last open */
	bool holding;             /* A pressed with no chat box, held since */
	int held;                 /* ... its frames */
	/* the fight */
	PartyState party;         /* its START: battle, seat, seats, format */
	bool fighting;
	int most, kept;           /* the party's HP at the fight's start; as seat 0 keeps it, or as it last told */
	int written;              /* what this game last left in its guardian */
	unsigned mine, theirs[ONLINE_SEATS];   /* the damage each seat has dealt (mine this game's) */
	unsigned sent;            /* ... as last sent or told */
	unsigned frame;
	uint32_t down_at;         /* the link down while queued (ms), 0 up; the queue to join again from rejoin_at */
	uint32_t rejoin_at;
} Q;

static void send(const uint8_t *m, size_t n) {
	if (n && !online_transport_send(m, n)) fprintf(stderr, "dive: a party message of kind %u not sent\n", m[0]);
}

static void queue(unsigned action) {
	uint8_t m[ONLINE_QUEUE_BYTES];
	send(m, dive_wire_queue(m, action, Q.at));
}

static void reset(void) {
	memset(&Q, 0, sizeof Q);
	boss_party_set(1);
}

/* (a full door holds MegaMan too: he waits there for a place, or goes in alone with A) */
static bool hold(void) { return Q.door == DOOR_QUEUED || Q.door == DOOR_FULL; }

void dive_party_on(int on) {
	boss_door_hold = on ? hold : NULL;
	reset();
}

static void join(const OnlineSnapshot *s) {
	queue(ONLINE_QUEUE_JOIN);
	Q.notices = s->notice_serial;
	Q.asked_at = Q.waited_from = SDL_GetTicks();
}

static void go_in(const char *why) {
	queue(ONLINE_QUEUE_START);
	fprintf(stderr, "dive: in at the door: %s\n", why);
}

static bool queued(uint32_t session) {
	for (unsigned k = 0; k < Q.q.count; ++k)
		if (Q.q.e[k].session == session) return true;
	return false;
}

/* The leader's door (issue #752, the owner's call of 22 November 2026): it
 * waits up to CREW_WAIT_MS for his crewmates on the layer, each told he
 * waits (the server's flare); meanwhile only A held takes the party in */
static void lead(void) {
	const CrewMember *m;
	int pending = 0;
	for (int k = 0; (m = dive_crew_member(k)) != NULL; ++k)
		if (m->session != Q.me && m->where == ONLINE_WHERE_LAYER && m->depth == Q.at.depth && !queued(m->session)) ++pending;
	Q.pending = pending;
}

/* the seconds the door still waits for the crew (0: it waits for none) */
static unsigned wait_left(void) {
	uint32_t waited = SDL_GetTicks() - Q.waited_from;
	return Q.pending && waited < CREW_WAIT_MS ? (CREW_WAIT_MS - waited + 999) / 1000 : 0;
}

/* a join the server refused: the door's queue holds its four already (its
 * NOTICE before any QUEUE_STATE; issue #753) */
static bool refused(const OnlineSnapshot *s) {
	return Q.door == DOOR_QUEUED && Q.q.count == 0 && s->notice_serial != Q.notices && s->notice_code == ONLINE_NOTICE_QUEUE;
}

/* the door: others on the layer and MegaMan saved at it joins its queue;
 * alone, he goes in as ever; out of the arena's mouth, he leaves it; a
 * full queue told so, and asked again for a place now and then */
static void door(const OnlineSnapshot *s, DiveWay room) {
	bool at = boss_at_door();
	if (!at) {
		if (Q.door == DOOR_QUEUED) queue(ONLINE_QUEUE_LEAVE);
		if (Q.door != DOOR_STARTED) Q.door = DOOR_NONE;
		return;
	}
	if (refused(s)) {
		Q.door = DOOR_FULL;
		fprintf(stderr, "dive: the door's party is full\n");
	}
	if (Q.door == DOOR_FULL && SDL_GetTicks() - Q.asked_at > ASK_AGAIN_MS) join(s);
	if (Q.door != DOOR_NONE) return;
	Q.at = room;
	if (!s->connected || s->peer_count == 0) { Q.door = DOOR_ALONE; boss_party_set(1); return; }
	join(s);
	Q.door = DOOR_QUEUED;
	Q.q.count = 0;
}

/* the guardian on side 1, the fight's first actor there (0 none) */
static uint32_t guardian(void) { return emu_read32(BN6_BATTLE_PLAYERS + BN6_BATTLE_PLAYER_STEP); }

static void fight_begin(uint32_t g) {
	Q.fighting = true;
	Q.most = Q.kept = Q.written = emu_read16(g + BN6_T1_HP);
	fprintf(stderr, "dive: the party's fight, seat %u of %u, the guardian's HP %d\n", Q.party.seat, (unsigned)Q.q.count, Q.most);
}

/* this game's hits counted, and the guardian left at the party's HP: seat
 * 0's own count, the others' as seat 0 last told; never raised over what
 * this game's battle has left him */
static void fight(void) {
	uint32_t g = guardian();
	if (!g) return;
	if (!Q.fighting) fight_begin(g);
	int hp = emu_read16(g + BN6_T1_HP);
	if (hp < Q.written) Q.mine += (unsigned)(Q.written - hp);
	if (Q.party.seat == 0) {
		unsigned all = Q.mine;
		for (unsigned k = 1; k < ONLINE_SEATS; ++k) all += Q.theirs[k];
		Q.kept = all >= (unsigned)Q.most ? 0 : Q.most - (int)all;
	}
	int want = Q.kept < hp ? Q.kept : hp;
	if (want != hp) {
		uint8_t b[2];
		put16(b, (uint16_t)want);
		emu_write(g + BN6_T1_HP, b, sizeof b);
	}
	Q.written = want;
	if (++Q.frame % SEND_EVERY) return;
	uint8_t m[ONLINE_PARTY_BYTES];
	unsigned now = Q.party.seat == 0 ? (unsigned)Q.kept : Q.mine;
	if (now == Q.sent) return;
	Q.sent = now;
	if (Q.party.seat == 0) send(m, dive_wire_party(m, ONLINE_PARTY_HP, Q.party.battle, (unsigned)Q.kept, (unsigned)Q.most));
	else send(m, dive_wire_party(m, ONLINE_PARTY_DAMAGE, Q.party.battle, Q.mine, 0));
}

/* a queued diver's own link down (issue #751): the server took him out of
 * the door's queue as it dropped; back within DOWN_ALONE_MS, he joins it
 * again (once his room is entered again, dive_link.c), else he goes in
 * alone; never held in a queue that no longer knows him. True while it waits */
static bool queued_through_a_drop(const OnlineSnapshot *s) {
	uint32_t now = SDL_GetTicks();
	bool down = !s->connected || s->reconnecting || s->error[0];
	if (Q.door != DOOR_QUEUED && Q.door != DOOR_FULL) { Q.down_at = Q.rejoin_at = 0; return false; }
	if (down) {
		if (!Q.down_at) { Q.down_at = now | 1u; fprintf(stderr, "dive: the link dropped at the door; waiting on it\n"); }
		if ((int32_t)(now - Q.down_at) < DOWN_ALONE_MS) return true;
		fprintf(stderr, "dive: the link stayed down at the door: in alone\n");
		Q.door = DOOR_ALONE;
		Q.down_at = 0;
		boss_party_set(1);
		return true;
	}
	if (Q.down_at) { Q.down_at = 0; Q.rejoin_at = (now + REJOIN_MS) | 1u; }
	if (!Q.rejoin_at) return false;
	if ((int32_t)(now - Q.rejoin_at) < 0) return true;
	Q.rejoin_at = 0;
	fprintf(stderr, "dive: the link back at the door: its queue joined again\n");
	join(s);
	Q.door = DOOR_QUEUED;
	Q.q.count = 0;
	return true;
}

void dive_party_frame(const OnlineSnapshot *s, DiveWay room) {
	if (Q.door == DOOR_STARTED && Q.party.battle) {
		/* (one field: the party's core fights him, party_play.c; linked: this game's battle, his HP the party's) */
		if (boss_fighting()) {
			if (Q.party.form == ONLINE_ARENA_PARTY) Q.fighting = true;
			else fight();
			return;
		}
		/* (the fight over, won or lost: the next door begins afresh) */
		if (Q.fighting) { fprintf(stderr, "dive: the party's fight over\n"); reset(); }
		return;
	}
	Q.me = s->player_id;
	if (queued_through_a_drop(s)) return;
	door(s, room);
	if (Q.door == DOOR_QUEUED && Q.q.place == 1) lead();
}

/* the door's own A: pressed with no chat box open for CALM_FRAMES, never
 * the press that paged MegaMan's words on the way in; `hold`: held then
 * for HOLD_FRAMES too */
static bool door_a(uint32_t keys, uint32_t pressed, bool hold) {
	bool calm = Q.calm >= CALM_FRAMES;
	if ((pressed & KEY_A) && calm) { Q.holding = true; Q.held = 0; }
	if (!(keys & KEY_A) || !calm) Q.holding = false;
	if (!Q.holding || ++Q.held < (hold ? HOLD_FRAMES : 1)) return false;
	Q.holding = false;
	return true;
}

uint32_t dive_party_keys(uint32_t keys) {
	uint32_t pressed = keys & ~Q.keys;
	Q.keys = keys;
	Q.calm = emu_read8(BN6_CHATBOX) || talk_busy() ? 0 : Q.calm + 1;
	/* (a full door: A goes in alone, every other key MegaMan's own) */
	if (Q.door == DOOR_FULL) {
		if (door_a(keys, pressed, false)) {
			Q.door = DOOR_ALONE;
			boss_party_set(1);
			fprintf(stderr, "dive: in alone, the party full\n");
		}
		return keys & ~(uint32_t)KEY_A;
	}
	if (Q.door != DOOR_QUEUED) return keys;
	/* (the leader's A: held while the door waits for his crew, a press after it) */
	if (Q.q.place == 1 && door_a(keys, pressed, wait_left() > 0)) go_in(wait_left() ? "A held, the crew not waited for" : "A");
	/* (B alone: not the leader's while his crew is waited for, A held is his early way) */
	if ((pressed & KEY_B) && !(Q.q.place == 1 && wait_left())) {
		queue(ONLINE_QUEUE_LEAVE);
		Q.door = DOOR_ALONE;
		boss_party_set(1);
	}
	return keys & ~(uint32_t)(KEY_A | KEY_B | KEY_START);
}

static void queue_state(const QueueState *q) {
	Q.q = *q;
	switch (q->stage) {
	case ONLINE_QUEUE_STARTED:
		Q.door = DOOR_STARTED;
		boss_party_set(q->count);
		fprintf(stderr, "dive: the party of %u goes in\n", (unsigned)q->count);
		break;
	case ONLINE_QUEUE_ALONE:
		Q.door = DOOR_ALONE;
		boss_party_set(1);
		break;
	case ONLINE_QUEUE_OUT:
		if (Q.door == DOOR_QUEUED) Q.door = DOOR_ALONE;
		break;
	case ONLINE_QUEUE_WAITING:
		/* (a place opened at a full door: asked again, he is in) */
		if (Q.door == DOOR_FULL) Q.door = DOOR_QUEUED;
		break;
	default: break;
	}
}

/* a party battle's START (one field: the party's core takes the guardian's
 * fight, party_play.h; linked: this game's own) and a seat gone */
static void party_state(const PartyState *p) {
	if (p->stage == ONLINE_ARENA_SEAT_GONE && p->battle && (p->battle == Q.party.battle || p->battle == dive_join_battle())) {
		party_play_gone(p->seat);
		return;
	}
	if (p->stage != ARENA_START) return;
	/* (a crew's random battle MegaMan offered or joined: dive_join.h) */
	if (Q.door != DOOR_STARTED && dive_join_start(p)) return;
	Q.party = *p;
	if (p->form != ONLINE_ARENA_PARTY) return;
	unsigned n = 0;
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) n += p->seats[k] != 0;
	party_play_on(true);
	party_play_start(p->battle, p->seat, n ? n : Q.q.count, p->delay, p->seats);
}

void dive_party_mail(const uint8_t *m, size_t n) {
	QueueState q;
	PartyState p;
	unsigned seat, damage, hp, most;
	uint32_t battle;
	switch (m[0]) {
	case ONLINE_QUEUE_STATE:
		if (dive_wire_queue_state(m, n, &q)) queue_state(&q);
		break;
	case ONLINE_ARENA_STATE:
		if (dive_wire_party_state(m, n, &p)) party_state(&p);
		break;
	case ONLINE_ARENA_SEAT:
		if (party_play_mail(m, n)) break;
		if (dive_wire_damage(m, n, &seat, &battle, &damage, &hp) && battle == Q.party.battle && seat < ONLINE_SEATS) Q.theirs[seat] = damage;
		break;
	case ONLINE_PARTY_HP:
		if (dive_wire_party_hp(m, n, &battle, &hp, &most) && battle == Q.party.battle && Q.party.seat) Q.kept = (int)hp;
		break;
	default: break;
	}
}

void dive_party_draw(int x, int y) {
	if (Q.door == DOOR_FULL) { look_band(x, y + 160 - LOOK_BAND_H - 1, 240, dive_door_full_words()); return; }
	if (Q.door != DOOR_QUEUED) return;
	const char *leader = Q.q.count ? Q.q.e[0].name : "";
	look_band(x, y + 160 - LOOK_BAND_H - 1, 240, dive_door_words(Q.q.count, Q.q.place == 1, leader, wait_left()));
}

int dive_party_members(const QueueEntry **out) {
	*out = Q.q.e;
	return Q.door == DOOR_QUEUED && Q.q.count > 1 ? Q.q.count : 0;
}

void dive_party_describe(FILE *f) {
	static const char *const doors[] = { "none", "alone", "queued", "full", "started" };
	fprintf(f, "door %s", doors[Q.door]);
	if (Q.door == DOOR_QUEUED)
		for (unsigned k = 0; k < Q.q.count; ++k) fprintf(f, "%s%s", k ? " " : ": ", Q.q.e[k].name);
	fputc('\n', f);
	if (Q.door == DOOR_QUEUED && Q.q.place == 1) fprintf(f, "doorwait %u for %d\n", wait_left(), Q.pending);
}

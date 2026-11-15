/* dive_party.h */
#include "dive_party.h"

#include <stdio.h>
#include <string.h>

#include "boss.h"
#include "bn6.h"
#include "bytes.h"
#include "dive_words.h"
#include "emu.h"
#include "online_transport.h"
#include "pet_look.h"

#define SEND_EVERY 6          /* frames between the fight's messages, each sent only on a change */
#define ARENA_START 3         /* ARENA_STATE's stage START (shared/online/README.md) */

enum { DOOR_NONE, DOOR_ALONE, DOOR_QUEUED, DOOR_STARTED };

static struct {
	int door;                 /* DOOR_* */
	DiveWay at;               /* the door's layer */
	QueueState q;
	uint32_t keys;            /* the pad's last frame, for a press */
	/* the fight */
	PartyState party;         /* its START: battle, seat, seats, format */
	bool fighting;
	int most, kept;           /* the party's HP at the fight's start; as seat 0 keeps it, or as it last told */
	int written;              /* what this game last left in its guardian */
	unsigned mine, theirs[ONLINE_SEATS];   /* the damage each seat has dealt (mine this game's) */
	unsigned sent;            /* ... as last sent or told */
	unsigned frame;
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

static bool hold(void) { return Q.door == DOOR_QUEUED; }

void dive_party_on(int on) {
	boss_door_hold = on ? hold : NULL;
	reset();
}

/* the door: others on the layer and MegaMan saved at it joins its queue;
 * alone, he goes in as ever; out of the arena's mouth, he leaves it */
static void door(const OnlineSnapshot *s, DiveWay room) {
	bool at = boss_at_door();
	if (!at) {
		if (Q.door == DOOR_QUEUED) queue(ONLINE_QUEUE_LEAVE);
		if (Q.door != DOOR_STARTED) Q.door = DOOR_NONE;
		return;
	}
	if (Q.door != DOOR_NONE) return;
	Q.at = room;
	if (!s->connected || s->peer_count == 0) { Q.door = DOOR_ALONE; boss_party_set(1); return; }
	queue(ONLINE_QUEUE_JOIN);
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

void dive_party_frame(const OnlineSnapshot *s, DiveWay room) {
	if (Q.door == DOOR_STARTED && Q.party.battle) {
		if (boss_fighting()) { fight(); return; }
		/* (the fight over, won or lost: the next door begins afresh) */
		if (Q.fighting) { fprintf(stderr, "dive: the party's fight over\n"); reset(); }
		return;
	}
	door(s, room);
}

uint32_t dive_party_keys(uint32_t keys) {
	uint32_t pressed = keys & ~Q.keys;
	Q.keys = keys;
	if (Q.door != DOOR_QUEUED) return keys;
	if ((pressed & KEY_A) && Q.q.place == 1) queue(ONLINE_QUEUE_START);
	if (pressed & KEY_B) {
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
	default: break;
	}
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
		if (dive_wire_party_state(m, n, &p) && p.stage == ARENA_START) Q.party = p;
		break;
	case ONLINE_ARENA_SEAT:
		if (dive_wire_damage(m, n, &seat, &battle, &damage, &hp) && battle == Q.party.battle && seat < ONLINE_SEATS) Q.theirs[seat] = damage;
		break;
	case ONLINE_PARTY_HP:
		if (dive_wire_party_hp(m, n, &battle, &hp, &most) && battle == Q.party.battle && Q.party.seat) Q.kept = (int)hp;
		break;
	default: break;
	}
}

void dive_party_draw(int x, int y) {
	if (Q.door != DOOR_QUEUED) return;
	const char *leader = Q.q.count ? Q.q.e[0].name : "";
	look_band(x, y + 160 - LOOK_BAND_H - 1, 240, dive_door_words(Q.q.count, Q.q.place == 1, leader));
}

/* dive_join.h */
#include "dive_join.h"

#include <SDL.h>
#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "bn6_fields.h"
#include "boss.h"
#include "director.h"
#include "director_crew.h"
#include "dive_crew.h"
#include "dive_splits.h"
#include "dive_words.h"
#include "emu.h"
#include "encounter.h"
#include "online_transport.h"
#include "party_play.h"
#include "protocol.h"
#include "talk.h"

#define LATE_MS 3000          /* past the window, an answer given up on */

/* (the one field runs a core beside the game's: not on the New 3DS, nor on the PSP's gpSP, which fight linked) */
#if defined(__3DS__) || defined(__PSP__)
#define ONE_FIELD false
#else
#define ONE_FIELD true
#endif

enum { JOIN_NONE, JOIN_CALLING, JOIN_JOINED, JOIN_FIGHTING };

static struct {
	int stage;                /* JOIN_* */
	uint32_t owner;           /* the battle's owner (an offer's; 0 MegaMan's own) */
	int index;                /* its index on the layer */
	char name[ONLINE_NAME_BYTES + 1];
	uint32_t since;           /* the stage's start (ms) */
	uint32_t battle;          /* its ARENA_STATE battle once taken */
	Encounter e;              /* ... and its battle */
} J;

void dive_join_reset(void) {
	memset(&J, 0, sizeof J);
	emu_encounters_offer(false);
	emu_encounter_go();
}

static void to(int stage) {
	J.stage = stage;
	J.since = SDL_GetTicks();
}

static void send(unsigned action, uint32_t ref, unsigned value) {
	uint8_t m[ONLINE_CREW_BYTES];
	size_t n = dive_wire_crew(m, action, ref, value);
	if (n && !online_transport_send(m, n)) fprintf(stderr, "dive: a join's message not sent\n");
}

/* MegaMan free on the layer's map: no chat, no door, no fight of a party's */
static bool free_here(void) {
	return ONE_FIELD && director_on_layer() && director_on_map() && !emu_read8(BN6_CHATBOX) && !talk_busy() && !boss_at_door() &&
		!party_play_held();
}

/* a crewmate's name ("" none) */
static const char *crewmate(uint32_t session) {
	const CrewMember *m;
	for (int k = 0; (m = dive_crew_member(k)) != NULL; ++k)
		if (m->session == session) return m->name;
	return "";
}

/* the owner's: a roll held while MegaMan was in a crew (the crew is who stands near, dive_crew.h), offered */
static void call(void) {
	J.owner = 0;
	J.index = director_crew_index();
	send(ONLINE_CREW_BATTLE, 0, (unsigned)J.index);
	fprintf(stderr, "dive: battle %d held, the crew called\n", J.index);
	to(JOIN_CALLING);
}

static void alone(const char *why) {
	fprintf(stderr, "dive: battle %d fought alone: %s\n", J.index, why);
	emu_encounter_go();
	to(JOIN_NONE);
}

void dive_join_frame(void) {
	uint32_t now = SDL_GetTicks(), waited = now - J.since;
	switch (J.stage) {
	case JOIN_NONE: {
		bool on = free_here() && dive_crew_count() > 1;
		if (on != emu_encounters_offering()) fprintf(stderr, "dive: %s\n", on ? "in a crew: a battle rolled is offered" : "no crew here");
		emu_encounters_offer(on);
		if (emu_encounter_held()) call();
		break;
	}
	case JOIN_CALLING:
		emu_encounters_offer(false);
		if (waited > ONLINE_CREW_JOIN_MS + LATE_MS) alone("no answer");
		break;
	case JOIN_JOINED:
		if (waited > ONLINE_CREW_JOIN_MS + LATE_MS) { fprintf(stderr, "dive: the joined battle never started\n"); to(JOIN_NONE); }
		break;
	case JOIN_FIGHTING: {
		if (party_play_crew_fighting()) break;
		/* (fought to its end on the field: a battle of this run's own; else, the field not made, the owner fights it here
		 * and a joiner is back as he was) */
		bool won;
		int hp;
		uint16_t reward;
		if (party_play_crew_end(&won, &hp, &reward)) {
			director_crew_over(&J.e, won, hp, reward);
			dive_splits_fought();
		}
		fprintf(stderr, "dive: the crew's battle over, back on the layer\n");
		J.battle = 0;
		to(JOIN_NONE);
		break;
	}
	default: break;
	}
}

uint32_t dive_join_keys(uint32_t keys) {
	/* (MegaMan stands still while his battle waits on the answer, or his join on its start) */
	return J.stage == JOIN_CALLING || J.stage == JOIN_JOINED ? 0 : keys;
}

void dive_join_crew_state(const CrewState *c) {
	if (c->stage == ONLINE_CREW_ALONE) {
		if (J.stage == JOIN_CALLING) alone("nobody jumped in");
		return;
	}
	/* (an offer while one was joined: the server's last, the earlier one over) */
	if (c->stage != ONLINE_CREW_OFFER || (J.stage != JOIN_NONE && J.stage != JOIN_JOINED)) return;
	if (J.stage != JOIN_NONE) to(JOIN_NONE);
	if (!free_here() || !dive_crew_has(c->other)) {
		fprintf(stderr, "dive: an offer of battle %u not taken (%s)\n", c->value, free_here() ? "not in the crew" : "busy");
		return;
	}
	/* (no question: a crewmate walking this close fights it too, the owner's call of 21 November 2026, issue #715;
	 * walking apart, or splitting up, is the way to stay out. The join's wait is counted from here: one the server
	 * turned down as late holds MegaMan only so long) */
	J.owner = c->other;
	J.index = c->value;
	snprintf(J.name, sizeof J.name, "%s", crewmate(c->other));
	send(ONLINE_CREW_JOIN, J.owner, 0);
	fprintf(stderr, "dive: jumping into %s's battle %d, unasked\n", J.name, J.index);
	to(JOIN_JOINED);
}

/* the crew's battle into the party's core, every game of it making the same */
static void take(const PartyState *p) {
	unsigned n = 0;
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) n += p->seats[k] != 0;
	J.e = director_crew_battle(J.index);
	int walked = director_crew_steps_reset();
	fprintf(stderr, "dive: the crew's battle %d, seat %u of %u; the encounter count reset (walked %d)\n", J.index, p->seat, n, walked);
	/* (the run's next battle too, so that the owner fights it alone where the field cannot be made) */
	director_crew_record(&J.e);
	party_play_on(true);
	party_play_start(p->battle, p->seat, n, p->delay, p->seats);
	if (!party_play_crew(&J.e)) {
		if (p->seat == 0) alone("the field could not take it");
		else to(JOIN_NONE);
		return;
	}
	J.battle = p->battle;
	to(JOIN_FIGHTING);
}

bool dive_join_start(const PartyState *p) {
	bool mine = J.stage == JOIN_CALLING && p->seat == 0, joined = J.stage == JOIN_JOINED && p->seat > 0;
	if (!mine && !joined) return false;
	if (p->form != ONLINE_ARENA_PARTY) {
		/* (a linked fighter's start: a crew's battle is never one, the server offering none to a game without the one field) */
		if (mine) alone("a linked start");
		else to(JOIN_NONE);
		return true;
	}
	take(p);
	return true;
}

uint32_t dive_join_battle(void) { return J.stage == JOIN_FIGHTING ? J.battle : 0; }

void dive_join_describe(FILE *f) {
	static const char *const stages[] = { "none", "calling", "joined", "fighting" };
	fprintf(f, "join %s\n", stages[J.stage]);
}

const char *dive_join_band(void) {
	switch (J.stage) {
	case JOIN_CALLING: return dive_join_calling_words();
	case JOIN_JOINED: return dive_join_joined_words();
	default: return NULL;
	}
}

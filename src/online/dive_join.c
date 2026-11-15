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

#define NEAR 128              /* world units: four panels of the layer's (netmap.c's 32) */
#define LATE_MS 3000          /* past the window, an answer given up on */

/* (the one field runs a core beside the game's: not on the New 3DS, nor on the PSP's gpSP, which fight linked) */
#if defined(__3DS__) || defined(__PSP__)
#define ONE_FIELD false
#else
#define ONE_FIELD true
#endif

enum { JOIN_NONE, JOIN_CALLING, JOIN_OFFERED, JOIN_JOINED, JOIN_FIGHTING };

static struct {
	int stage;                /* JOIN_* */
	uint32_t owner;           /* the battle's owner (an offer's; 0 MegaMan's own) */
	int index;                /* its index on the layer */
	char name[ONLINE_NAME_BYTES + 1];
	uint32_t since;           /* the stage's start (ms) */
	uint32_t battle;          /* its ARENA_STATE battle once taken */
	int fought_from;          /* the layer's battles as it was taken (director_crew_index): one more once the field fought it */
	uint32_t keys;
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
	DiveWay none = { 0, 0, 0 };
	size_t n = dive_wire_crew(m, action, ref, value, none);
	if (n && !online_transport_send(m, n)) fprintf(stderr, "dive: a join's message not sent\n");
}

/* MegaMan free on the layer's map: no chat, no door, no fight of a party's */
static bool free_here(void) {
	return ONE_FIELD && director_on_layer() && director_on_map() && !emu_read8(BN6_CHATBOX) && !talk_busy() && !boss_at_door() &&
		!party_play_held();
}

/* the room's diver `id` (a crewmate: 0 any) within NEAR of MegaMan; his name */
static bool near(const OnlineSnapshot *s, uint32_t id, const char **name) {
	int x = bn6_player_x(), y = bn6_player_y();
	for (size_t i = 0; s->connected && i < s->peer_count; ++i) {
		const OnlinePeer *p = &s->peers[i];
		if ((id && p->id != id) || !dive_crew_has(p->id)) continue;
		long dx = p->x - x, dy = p->y - y;
		if (dx * dx + dy * dy > (long)NEAR * NEAR) continue;
		if (name) *name = p->name;
		return true;
	}
	return false;
}

/* the owner's: a roll held while a crewmate stood near, offered */
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

void dive_join_frame(const OnlineSnapshot *s) {
	uint32_t now = SDL_GetTicks(), waited = now - J.since;
	switch (J.stage) {
	case JOIN_NONE: {
		bool on = free_here() && near(s, 0, NULL);
		if (on != emu_encounters_offering()) fprintf(stderr, "dive: %s\n", on ? "a crewmate near: a battle rolled is offered" : "no crewmate near");
		emu_encounters_offer(on);
		if (emu_encounter_held()) call();
		break;
	}
	case JOIN_CALLING:
		emu_encounters_offer(false);
		if (waited > ONLINE_CREW_JOIN_MS + LATE_MS) alone("no answer");
		break;
	case JOIN_OFFERED:
		/* (the window over, or MegaMan taken up by something else meanwhile) */
		if (waited > ONLINE_CREW_JOIN_MS || !director_on_layer() || !director_on_map()) to(JOIN_NONE);
		break;
	case JOIN_JOINED:
		if (waited > ONLINE_CREW_JOIN_MS + LATE_MS) { fprintf(stderr, "dive: the joined battle never started\n"); to(JOIN_NONE); }
		break;
	case JOIN_FIGHTING:
		if (!party_play_crew_fighting()) {
			fprintf(stderr, "dive: the crew's battle over, back on the layer\n");
			if (director_crew_index() > J.fought_from) dive_splits_fought();
			J.battle = 0;
			to(JOIN_NONE);
		}
		break;
	default: break;
	}
}

uint32_t dive_join_keys(uint32_t keys) {
	uint32_t pressed = keys & ~J.keys;
	J.keys = keys;
	/* (MegaMan stands still while his battle waits on the answer, or his join on its start) */
	if (J.stage == JOIN_CALLING || J.stage == JOIN_JOINED) return 0;
	if (J.stage != JOIN_OFFERED || emu_read8(BN6_CHATBOX) || talk_busy()) return keys;
	/* (the window over before this frame's A: the server's offer is gone, and a join now would be for the owner's next) */
	if (SDL_GetTicks() - J.since > ONLINE_CREW_JOIN_MS) {
		to(JOIN_NONE);
		return keys;
	}
	if (pressed & KEY_A) {
		send(ONLINE_CREW_JOIN, J.owner, 0);
		fprintf(stderr, "dive: jumping into %s's battle %d\n", J.name, J.index);
		/* (its wait counted from the offer: a join the server turned down as late holds MegaMan only so long) */
		J.stage = JOIN_JOINED;
		return 0;
	}
	return keys;
}

void dive_join_crew_state(const CrewState *c) {
	if (c->stage == ONLINE_CREW_ALONE) {
		if (J.stage == JOIN_CALLING) alone("nobody jumped in");
		return;
	}
	/* (an offer while one stood or was joined: the server's last, the earlier one over) */
	if (c->stage != ONLINE_CREW_OFFER || (J.stage != JOIN_NONE && J.stage != JOIN_OFFERED && J.stage != JOIN_JOINED)) return;
	if (J.stage != JOIN_NONE) to(JOIN_NONE);
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	const char *name = "";
	if (!free_here() || !near(&s, c->other, &name)) {
		fprintf(stderr, "dive: an offer of battle %u not shown (%s)\n", c->value, free_here() ? "too far" : "busy");
		return;
	}
	J.owner = c->other;
	J.index = c->value;
	snprintf(J.name, sizeof J.name, "%s", name);
	fprintf(stderr, "dive: %s's battle %d offered\n", J.name, J.index);
	to(JOIN_OFFERED);
}

/* the crew's battle into the party's core, every game of it making the same */
static void take(const PartyState *p) {
	unsigned n = 0;
	for (unsigned k = 0; k < ONLINE_SEATS; ++k) n += p->seats[k] != 0;
	Encounter e = director_crew_battle(J.index);
	int walked = director_crew_steps_reset();
	fprintf(stderr, "dive: the crew's battle %d, seat %u of %u; the encounter count reset (walked %d)\n", J.index, p->seat, n, walked);
	party_play_on(true);
	party_play_start(p->battle, p->seat, n, p->delay);
	if (!party_play_crew(&e)) {
		if (p->seat == 0) alone("the field could not take it");
		else to(JOIN_NONE);
		return;
	}
	J.battle = p->battle;
	J.fought_from = director_crew_index();
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

const char *dive_join_band(void) {
	switch (J.stage) {
	case JOIN_CALLING: return dive_join_calling_words();
	case JOIN_OFFERED: return dive_join_offer_words(J.name);
	case JOIN_JOINED: return dive_join_joined_words();
	default: return NULL;
	}
}

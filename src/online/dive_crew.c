/* dive_crew.h. The chats are BN6's own, written into the layer's room for
 * them (dive_people_talk_room) as the Square writes its own; their answers
 * in the Square's team flags, which a layer otherwise never sets, cleared
 * as read. */
#include "dive_crew.h"

#include <stdio.h>
#include <string.h>

#include "arena_words.h"
#include "bn6.h"
#include "director.h"
#include "dive_join.h"
#include "dive_people.h"
#include "dive_wire.h"
#include "dive_words.h"
#include "emu.h"
#include "flags.h"
#include "online_transport.h"
#include "run.h"
#include "script_kit.h"
#include "talk.h"
#include "text.h"

#define CHAT_WAIT 90          /* frames a chat asked for may take to show */
#define YES_FLAG 0x14C7       /* the Square's team Yes (docs/engine/emulation/layer.md), on a layer the crew's */

enum { CHAT_NONE, CHAT_ASK, CHAT_SPLIT, CHAT_ASKED };

static struct {
	CrewState c;              /* the crew as the server last said (count 0 alone) */
	uint32_t asked_by;        /* an ask waiting for its chat (0 none) */
	uint32_t about;           /* the diver the open chat is about */
	char name[ONLINE_NAME_BYTES + 1];
	int chat;                 /* CHAT_*: the chat open, read as it closes */
	bool shown;               /* ... and seen open */
	int waited;               /* frames it has been asked for and not shown */
	uint32_t keys;
	char news[96];
	bool news_new;
	int chosen;               /* the home portal MegaMan waits on, -1 none */
} C;

void dive_crew_reset(void) { memset(&C, 0, sizeof C); C.chosen = -1; }

bool dive_crew_has(uint32_t session) {
	for (int k = 0; k < C.c.count; ++k)
		if (C.c.m[k].session == session) return true;
	return false;
}

const char *dive_crew_news(void) {
	if (!C.news_new) return NULL;
	C.news_new = false;
	return C.news;
}

static void news(const char *s) {
	snprintf(C.news, sizeof C.news, "%s", s);
	for (char *p = C.news; *p; ++p) if (*p == '\n' || *p == '|') *p = ' ';
	C.news_new = true;
}

static void send_way(unsigned action, uint32_t ref, unsigned value, DiveWay w) {
	uint8_t m[ONLINE_CREW_BYTES];
	size_t n = dive_wire_crew(m, action, ref, value, w);
	if (n && !online_transport_send(m, n)) fprintf(stderr, "dive: a crew message not sent\n");
}

static void send(unsigned action, uint32_t ref, unsigned value) {
	DiveWay none = { 0, 0, 0 };
	send_way(action, ref, value, none);
}

bool dive_crew_waiting(void) { return C.chosen >= 0; }

bool dive_crew_way_hold(int portal) {
	DiveWay none = { 0, 0, 0 };
	if (portal < 0 || C.c.count < 2) {
		if (C.chosen >= 0) send_way(ONLINE_CREW_WAY, 0, 0, none);
		C.chosen = -1;
		return false;
	}
	/* (the way a portal takes: its index past 0, the same on every
	 * crewmate's game, whose run is the same week's) */
	DiveWay w = { (uint8_t)run.depth, 0, (uint8_t)(portal + 1) };
	if (C.chosen != portal) { C.chosen = portal; send_way(ONLINE_CREW_WAY, 0, 0, w); }
	for (int k = 0; k < C.c.count; ++k)
		if (C.c.m[k].way.way != w.way) return true;
	fprintf(stderr, "dive: the crew all on portal %d\n", portal);
	C.chosen = -1;
	return false;
}

static const char *name_of(const OnlineSnapshot *s, uint32_t id) {
	for (size_t i = 0; s->connected && i < s->peer_count; ++i)
		if (s->peers[i].id == id) return s->peers[i].name;
	return "";
}

/* MegaMan's words and BN6's Yes/No (No first), its Yes in YES_FLAG */
static bool ask(const char *words, const char *question) {
	uint32_t room = dive_people_talk_room();
	if (!room) return false;
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	ta_pages(&t, words, FACE_MEGAMAN, &first);
	ta_ask_in(&t, FACE_MEGAMAN, question, 1, !first, false);
	ta_flag_set(&t, YES_FLAG);
	ta_end(&t);
	ta_script(&t);
	ta_end(&t);
	ta_closing(&t);
	int n = ta_build(&t, out);
	if (n > 1024) return false;
	flag_clear(YES_FLAG);
	emu_write(room, out, (size_t)n);
	return talk_script(room, 0);
}

static void open_chat(int chat, uint32_t about, const char *name, const char *words, const char *question) {
	if (!ask(words, question)) { fprintf(stderr, "dive: the crew's chat could not open\n"); return; }
	C.chat = chat;
	C.shown = false;
	C.waited = 0;
	C.about = about;
	snprintf(C.name, sizeof C.name, "%s", name);
}

/* the chat closed: its Yes sent as the server's action */
static void read_chat(void) {
	bool yes = flag_get(YES_FLAG);
	flag_clear(YES_FLAG);
	fprintf(stderr, "dive: the crew's chat %d about %s: %s\n", C.chat, C.name, yes ? "yes" : "no");
	if (C.chat == CHAT_ASKED) send(ONLINE_CREW_ANSWER, C.about, yes);
	else if (yes && C.chat == CHAT_ASK) { send(ONLINE_CREW_ASK, C.about, 0); news(arena_team_sent_words(C.name)); }
	else if (yes && C.chat == CHAT_SPLIT) send(ONLINE_CREW_SPLIT, 0, 0);
	C.chat = CHAT_NONE;
}

void dive_crew_frame(const OnlineSnapshot *s) {
	bool open = talk_busy() || emu_read8(BN6_CHATBOX);
	if (C.chat != CHAT_NONE) {
		if (emu_read8(BN6_CHATBOX)) C.shown = true;
		else if (C.shown && !open) read_chat();
		/* (a chat that never showed, its map gone under it: given up) */
		else if (!C.shown && ++C.waited > CHAT_WAIT) { fprintf(stderr, "dive: the crew's chat never showed\n"); talk_reset(); C.chat = CHAT_NONE; }
		return;
	}
	if (!C.asked_by || open || !director_on_map()) return;
	uint32_t who = C.asked_by;
	C.asked_by = 0;
	const char *name = name_of(s, who);
	open_chat(CHAT_ASKED, who, name, arena_team_asked_words(name), arena_team_asked_question());
}

uint32_t dive_crew_keys(uint32_t keys) {
	uint32_t pressed = keys & ~C.keys;
	C.keys = keys;
	if (!(pressed & KEY_A) || C.chat != CHAT_NONE || talk_busy() || emu_read8(BN6_CHATBOX) || !director_on_map()) return keys;
	uint32_t who = dive_people_ahead();
	if (!who) return keys;
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	const char *name = name_of(&s, who);
	bool mate = dive_crew_has(who);
	open_chat(mate ? CHAT_SPLIT : CHAT_ASK, who, name, dive_crew_meet_words(name, mate), mate ? dive_crew_split_question() : arena_team_asked_question());
	return keys & ~(uint32_t)KEY_A;
}

static void crew_state(const CrewState *c) {
	switch (c->stage) {
	case ONLINE_CREW_ASKED: C.asked_by = c->other; break;
	case ONLINE_CREW_FORMED: {
		/* (a crewmate's way: the crew as it stands, no news) */
		bool again = C.c.count == c->count;
		C.c = *c;
		if (again) break;
		OnlineSnapshot s;
		online_transport_snapshot(&s);
		uint32_t me = s.player_id;
		const char *other = "";
		for (int k = 0; k < c->count; ++k) if (c->m[k].session != me) { other = c->m[k].name; break; }
		news(arena_team_formed_words(other));
		break;
	}
	case ONLINE_CREW_SPLIT_UP:
		news(arena_team_split_words(C.c.count ? C.c.m[0].name : ""));
		memset(&C.c, 0, sizeof C.c);
		break;
	default: break;
	}
}

void dive_crew_mail(const uint8_t *m, size_t n) {
	CrewState c;
	if (!dive_wire_crew_state(m, n, &c)) return;
	/* (a crewmate's battle offered, or MegaMan's own fought alone: the join-in's, dive_join.h) */
	if (c.stage == ONLINE_CREW_OFFER || c.stage == ONLINE_CREW_ALONE) dive_join_crew_state(&c);
	else crew_state(&c);
}

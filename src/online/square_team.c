/* Team NetBattles in the Net Square (square_team.h).
 *
 * The server keeps the teams and the match (server/cyberworld_server/
 * teams.py) and tells each player as it changes (TEAM_STATE); this game
 * keeps the last it was told and acts on it: the team question asked once
 * no chat is open, the captain's order question, a bout called for this
 * game's MegaMan gone into once nothing is on the picture (its card
 * closed, its watch over), a bout of the others watched, unasked. The
 * HP a winner carries is his own battle's tally, checked against the
 * server's report in the log. */
#include "square_team.h"

#include <stdio.h>
#include <string.h>

#include "arena_wire.h"
#include "arena_words.h"
#include "bn6.h"
#include "emu.h"
#include "flags.h"
#include "online_square.h"
#include "online_transport.h"
#include "online_visit.h"
#include "script_kit.h"
#include "square_arena.h"
#include "square_watch.h"
#include "talk.h"

#define TEAM_YES_FLAG 0x14C6   /* the Square's own (docs/EMULATION.md): Yes to a team asked of us */
#define ORDER_FLAG    0x14C7   /* + 0 we lead, + 1 the partner does (0x14C7, 0x14C8); the format's too, + 0 one by one, + 1 side by
                                  side (asked as a challenge goes, never with the order open) */

typedef enum { Q_NONE, Q_ASK, Q_OPEN } Question;

static struct {
	OnlineSnapshot s;
	uint32_t partner, asked_by, proposed;
	uint32_t proposed_at;          /* the notices' serial as my ask went, to tell a refusal of it */
	char proposed_name[ONLINE_NAME_BYTES + 1];
	char partner_name[ONLINE_NAME_BYTES + 1], asked_name[ONLINE_NAME_BYTES + 1], names[4][ONLINE_NAME_BYTES + 1];
	Question team_q, order_q;
	TeamWireState st;              /* the match as last told (stage 0 none) */
	uint32_t taken, watched;       /* the bout gone into, the bout watched */
	uint16_t carry;                /* my HP left, where I stay on */
	const char *said;
	char said_buf[160];
} T;

static void send(unsigned action, uint32_t ref, bool value) {
	uint8_t m[ONLINE_TEAM_BYTES];
	if (arena_wire_team(m, action, ref, value)) online_transport_send(m, sizeof m);
}

static const char *name_of(uint32_t id, const char *kept) {
	const char *n = square_arena_name_of(id);
	return n ? n : kept;
}

static void say(const char *words) {
	snprintf(T.said_buf, sizeof T.said_buf, "%s", words);
	T.said = T.said_buf;
}

void square_team_reset(void) {
	memset(&T, 0, sizeof T);
	flag_clear(TEAM_YES_FLAG);
	flag_clear(ORDER_FLAG);
	flag_clear(ORDER_FLAG + 1);
}

void square_team_end(void) {
	if (T.partner || T.proposed) send(TEAM_ACT_SPLIT, 0, false);
	square_team_reset();
}

uint32_t square_team_partner(void) { return T.partner; }

bool square_team_on(void) { return T.st.stage >= TEAM_MATCH && T.st.stage < TEAM_OVER; }

const char *square_team_option(const OnlinePeer *peer) { return arena_team_option(peer->id == T.partner); }

void square_team_chosen(const OnlinePeer *peer) {
	if (peer->id == T.partner) {
		printf("team: we split up with %s\n", T.partner_name);
		send(TEAM_ACT_SPLIT, 0, false);
		return;
	}
	if (T.partner) { talk_start(arena_team_have_words(T.partner_name), FACE_MEGAMAN); return; }
	T.proposed = peer->id;
	T.proposed_at = T.s.notice_serial;
	snprintf(T.proposed_name, sizeof T.proposed_name, "%s", peer->name);
	send(TEAM_ACT_PROPOSE, peer->id, false);
	talk_start(arena_team_sent_words(peer->name), FACE_MEGAMAN);
	printf("team: asked %s (session %u) to team up\n", peer->name, peer->id);
}

/* the four names kept as the match was told (a player gone keeps his) */
static void keep_names(void) {
	for (int k = 0; k < 4; ++k) {
		const char *n = T.st.players[k] == T.s.player_id ? online_visit_name() : square_arena_name_of(T.st.players[k]);
		if (n) snprintf(T.names[k], sizeof T.names[k], "%s", n);
	}
}

static void formed(const TeamWireState *s) {
	T.partner = s->partner;
	T.proposed = 0;
	snprintf(T.partner_name, sizeof T.partner_name, "%s", name_of(s->partner, "?"));
	say(arena_team_formed_words(T.partner_name));
	printf("team: a team with %s\n", T.partner_name);
}

static void split(const TeamWireState *s) {
	const char *who = name_of(s->partner, s->partner == T.partner ? T.partner_name : T.asked_name);
	if (s->partner == T.partner && T.partner) say(arena_team_split_words(who));
	else if (s->partner == T.proposed) say(arena_team_declined_words(who));
	if (s->partner == T.asked_by) T.asked_by = 0, T.team_q = Q_NONE;
	if (s->partner == T.partner) T.partner = 0;
	if (s->partner == T.proposed) T.proposed = 0;
	printf("team: no team with %s\n", who);
}

/* a match's step: begun (the team challenge's own state dropped), the order, a bout, its end */
static void match_in(const TeamWireState *s) {
	bool fresh = s->stage == TEAM_MATCH;
	T.st = *s;
	keep_names();
	if (fresh) {
		square_arena_team_begun();
		T.taken = T.watched = 0;
		T.carry = 0;
		/* (the captain picks who goes first: the one who asked to team up; side by side, the captains lead, nothing asked) */
		T.order_q = s->players[0] == T.s.player_id && s->form == ONLINE_TEAM_RELAY ? Q_ASK : Q_NONE;
		printf("team: match %u begins%s, %s and %s against %s and %s\n", s->match, s->form ? " side by side" : "", T.names[0], T.names[1],
			T.names[2], T.names[3]);
		return;
	}
	if (s->stage == TEAM_ORDERED && s->form == ONLINE_TEAM_RELAY) {
		T.order_q = Q_NONE;
		say(arena_team_first_words(s->players[0] == T.s.player_id, T.names[0], T.names[2]));
		printf("team: the order: %s then %s, against %s then %s\n", T.names[0], T.names[1], T.names[2], T.names[3]);
	}
	if (s->stage == TEAM_BOUT)
		printf("team: bout %u called, %s (%u standing) vs %s (%u standing), the stayer's HP %u\n", s->number, T.names[0], s->left[0],
			T.names[2], s->left[1], s->hp);
	if (s->stage == TEAM_OVER) {
		say(arena_team_over_words(s->result));
		printf("team: match %u over, result %u\n", s->match, s->result);
	}
}

void square_team_mail(const uint8_t *data, size_t size) {
	TeamWireState s;
	if (!arena_wire_team_state(data, size, &s)) { fprintf(stderr, "online: a malformed TEAM_STATE set aside\n"); return; }
	switch (s.stage) {
	case TEAM_ASKED:
		T.asked_by = s.partner;
		snprintf(T.asked_name, sizeof T.asked_name, "%s", name_of(s.partner, "?"));
		T.team_q = Q_ASK;
		printf("team: %s asks us to team up\n", T.asked_name);
		break;
	case TEAM_WAITING: break;
	case TEAM_FORMED: formed(&s); break;
	case TEAM_SPLIT_UP: split(&s); break;
	default: match_in(&s); break;
	}
}

/* the question built in BN6's chat box: MegaMan's `words`, then two
 * choices, each setting ORDER_FLAG + its index; B the first, or none
 * where `b_none` */
static bool two_ways(const char *words, const char *labels[2], bool b_none) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	ta_pages(&t, words, FACE_MEGAMAN, &first);
	ta_mugshot(&t, FACE_MEGAMAN);
	ta_clear(&t);
	static const int to[2] = { 1, 2 };
	ta_options(&t, labels, 2);
	ta_choose(&t, to, 2, b_none ? 3 : 1);
	for (int k = 0; k < 2 + b_none; ++k) {
		ta_script(&t);
		if (k < 2) ta_flag_set(&t, ORDER_FLAG + k);
		ta_end(&t);
	}
	ta_closing(&t);
	int size = ta_build(&t, out);
	flag_clear(ORDER_FLAG);
	flag_clear(ORDER_FLAG + 1);
	if (size > SQUARE_TALK_ROOM) return false;
	emu_write(online_square_talk_room(), out, (size_t)size);
	return talk_script(online_square_talk_room(), 0);
}

bool square_team_format_ask(void) {
	const char *labels[2] = { arena_team_format_relay(), arena_team_format_side() };
	return two_ways(arena_team_format_words(), labels, true);
}

int square_team_format_picked(void) {
	int form = flag_get(ORDER_FLAG + 1) ? ONLINE_TEAM_SIDE_BY_SIDE : flag_get(ORDER_FLAG) ? ONLINE_TEAM_RELAY : -1;
	flag_clear(ORDER_FLAG);
	flag_clear(ORDER_FLAG + 1);
	return form;
}

const char *square_team_name(uint32_t id) {
	for (int k = 0; k < 4; ++k)
		if (T.st.players[k] == id && T.names[k][0]) return T.names[k];
	return NULL;
}

/* the captain's question: who goes first (B: we do) */
static bool order_ask(void) {
	const char *labels[2] = { arena_team_order_us(), T.partner_name };
	return two_ways(arena_team_order_words(), labels, false);
}

static void questions(bool open) {
	if (T.team_q == Q_ASK && !open && T.asked_by) {
		flag_clear(TEAM_YES_FLAG);
		if (talk_ask(arena_team_asked_words(T.asked_name), FACE_MEGAMAN, arena_team_asked_question(), TEAM_YES_FLAG)) T.team_q = Q_OPEN;
	} else if (T.team_q == Q_OPEN && !open) {
		bool yes = flag_get(TEAM_YES_FLAG);
		flag_clear(TEAM_YES_FLAG);
		send(TEAM_ACT_ANSWER, T.asked_by, yes);
		printf("team: %s to %s's team\n", yes ? "yes" : "no", T.asked_name);
		T.asked_by = 0;
		T.team_q = Q_NONE;
	} else if (T.order_q == Q_ASK && !open && square_team_on() && order_ask()) T.order_q = Q_OPEN;
	else if (T.order_q == Q_OPEN && !open) {
		bool partner = flag_get(ORDER_FLAG + 1);
		flag_clear(ORDER_FLAG);
		flag_clear(ORDER_FLAG + 1);
		send(TEAM_ACT_ORDER, partner ? T.partner : T.s.player_id, false);
		printf("team: %s leads, our pick\n", partner ? T.partner_name : "we");
		T.order_q = Q_NONE;
	} else if (T.said && !open && talk_start(T.said, FACE_MEGAMAN)) T.said = NULL;
}

/* a bout called for my MegaMan, gone into once nothing is on the picture:
 * the HP carried my own tally's where I won the last bout (the server's
 * report beside it in the log) */
static void bout_mine(void) {
	if (T.taken == T.st.bout || square_watch_on() || !square_arena_idle()) return;
	uint16_t left = square_arena_last_hp();
	T.carry = T.st.number > 1 && T.st.hp && left ? left : 0;
	if (T.carry) printf("team: we stay on with %u HP (the server's report: %u)\n", T.carry, T.st.hp);
	if (square_arena_bout(T.st.bout, T.st.players[2], T.names[2], T.carry)) T.taken = T.st.bout;
}

void square_team_update(const OnlineSnapshot *s, bool held) {
	T.s = *s;
	bool open = !held && (talk_busy() || emu_read8(BN6_CHATBOX));
	if (!held) questions(open);
	/* (a question or a word just begun holds the bout back too) */
	open = !held && (talk_busy() || emu_read8(BN6_CHATBOX));
	/* (MegaMan's word on the order heard before a bout is gone into or watched) */
	if (!square_team_on() || !T.st.bout || open || (!held && T.said)) return;
	/* (side by side every one of the four fights) */
	bool mine = T.st.players[0] == T.s.player_id || (T.st.form == ONLINE_TEAM_SIDE_BY_SIDE && T.st.players[1] == T.s.player_id);
	if (T.st.stage == TEAM_BOUT && mine) bout_mine();
	/* (a bout of the others: watched from the stands, unasked) */
	if (T.st.stage == TEAM_FIGHT && !mine && T.watched != T.st.bout && !square_watch_on() && square_arena_idle() &&
		square_watch_team(T.st.bout))
		T.watched = T.st.bout;
	/* (a FIGHT come before my own bout was gone into: the same bout) */
	if (T.st.stage == TEAM_FIGHT && mine) bout_mine();
}

/* my ask refused by the server: busy (16), gone (17), the Arena closed (18), on a team already (29) */
bool square_team_notice(uint32_t serial, uint16_t code) {
	if (!T.proposed || T.partner || serial == T.proposed_at) return false;
	const char *n = T.proposed_name;
	const char *words = code == 16 ? arena_busy_words(n) : code == 17 ? arena_not_here_words(n) : code == 18 ? arena_closed_words() :
		code == ONLINE_NOTICE_TEAM ? arena_team_taken_words(n) : NULL;
	if (!words) return false;
	printf("team: our ask to %s refused (notice %u)\n", n, (unsigned)code);
	T.proposed = 0;
	say(words);
	return true;
}

void square_team_card(SquareTeamCard *out) {
	memset(out, 0, sizeof *out);
	if (!T.st.stage || T.st.stage < TEAM_MATCH) return;
	out->on = true;
	out->standing[0] = T.st.left[0];
	out->standing[1] = T.st.left[1];
	out->side_by_side = T.st.form == ONLINE_TEAM_SIDE_BY_SIDE;
	if (T.st.stage == TEAM_OVER) out->result = T.st.result;
	else if (!out->side_by_side) { out->next[0] = T.names[0]; out->next[1] = T.names[2]; }
}

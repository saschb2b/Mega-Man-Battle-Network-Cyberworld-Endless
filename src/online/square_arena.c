/* The Arena from the Net Square (square_arena.h).
 *
 * The challenger's Battle sends ARENA's challenge; the server answers him
 * WAITING and the other player CHALLENGED, whose MegaMan asks Lan once no
 * chat is open. Yes accepts, No (or B) declines; the server then STARTs
 * both, each with its side of the cable and the input delay. Each game
 * sends its MegaMan (arena_loadout_pack) and builds the pair from both,
 * its own as the other reads it, so that the two pairs are the same; then
 * the way in, the ready with the pair's hash, the battle and its result.
 * Side 0 tells the server the battle is over; a failure or a leave tells
 * it too, unless the server has already said it is. */
#include "square_arena.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "arena.h"
#include "arena_link_net.h"
#include "arena_loadout.h"
#include "arena_pair.h"
#include "arena_view.h"
#include "arena_wire.h"
#include "arena_words.h"
#include "bn6.h"
#include "devtools.h"
#include "emu.h"
#include "flags.h"
#include "online_mail.h"
#include "online_people.h"
#include "online_transport.h"
#include "online_visit.h"
#include "platform.h"
#include "rom.h"
#include "talk.h"
#include "text.h"

#define ACCEPT_FLAG     0x14A1   /* the Square's (online_chat.c's): Yes to a challenge */
#define LOADOUT_WAIT_MS 15000    /* the other's MegaMan, after START */
#define READY_WAIT_MS   60000    /* the other's way in (a slow machine's) */
#define SENT_WAIT_MS    5000     /* the server's word on a challenge */
#define OVER_TICKS      300      /* the result shown before the Square */
#define OVER_HELD       30       /* ... at least, before A goes back */
#define STALL_SHOWN     20

typedef enum {
	SA_IDLE,
	SA_SENT,        /* my challenge sent: the server's word awaited */
	SA_WAITING,     /* ... the other's answer */
	SA_ASKING,      /* challenged: the question to ask once no chat is open */
	SA_ASKED,       /* ... open */
	SA_ACCEPTED,    /* Yes said: START awaited */
	SA_LOADOUT,     /* START: the other's MegaMan awaited (from here the Square holds) */
	SA_CONNECTING,  /* the way in */
	SA_READY,       /* the other's ready awaited */
	SA_BATTLE,
	SA_OVER,        /* the result, or why it stopped, on the picture */
} Stage;

static struct {
	Stage stage;
	uint32_t match, other, since, latency;
	char name[ONLINE_NAME_BYTES + 1];
	bool ended;               /* the server has said this battle is over */
	OnlineSnapshot s;
	uint32_t notice_serial;
	ArenaNetLink link;
	Arena *arena;
	OnlineLoadout mine, theirs;
	bool mine_carried, theirs_carried, theirs_in;
	int over_t, stalled;
	const char *said;         /* MegaMan's word, once back in the Square */
	char why[160];
} A;

/* the last battle this game fought from the Square, for its referee */
static struct { unsigned result; char name[ONLINE_NAME_BYTES + 1]; } last;

bool square_arena_offered(void) { return CW_ARENA; }

unsigned square_arena_last(const char **name) {
	*name = last.result ? last.name : NULL;
	return last.result;
}

static bool out(void *context, const uint8_t *data, size_t size) {
	(void)context;
	return online_transport_send(data, size);
}

static void act(unsigned action, uint32_t ref) {
	uint8_t m[ONLINE_ARENA_BYTES];
	if (arena_wire_action(m, action, ref, A.latency)) online_transport_send(m, sizeof m);
}

static void say(const char *words) { A.said = words; }

static void drop_arena(void) {
	arena_free(A.arena);
	A.arena = NULL;
}

void square_arena_reset(void) {
	drop_arena();
	memset(&A, 0, sizeof A);
	flag_clear(ACCEPT_FLAG);
}

void square_arena_end(void) {
	if (A.match && !A.ended && A.stage != SA_IDLE) act(ARENA_ACT_LEAVE, A.match);
	square_arena_reset();
}

/* the battle stopped: the server told unless it said so first, the
 * reason on the picture */
static void fail_as(const char *why, bool both) {
	snprintf(A.why, sizeof A.why, "%s", why);
	printf("arena: match %u stopped: %s\n", A.match, A.why);
	/* (what both games see, two hashes apart, is the cable's first's to
	 * tell, as the battle's end is: the server would answer the second
	 * with a notice that it is over) */
	if (!A.ended && (!both || A.link.link.side == 0)) act(ARENA_ACT_LEAVE, A.match);
	A.ended = true;
	say(arena_stopped_words());
	A.stage = SA_OVER;
	A.over_t = 0;
}

static void fail(const char *why) { fail_as(why, false); }

/* my MegaMan can go into a plain BN6 NetBattle (the game's start's where
 * none was carried); else MegaMan says why */
static bool mine_goes(void) {
	const OnlineLoadout *l = online_visit_loadout();
	char why[120];
	uint8_t bytes[ONLINE_ARENA_LOADOUT_MAX];
	if (!l || (arena_loadout_check(l, why, sizeof why) && arena_loadout_pack(l, bytes, sizeof bytes))) return true;
	printf("arena: our MegaMan can't go: %s\n", why);
	talk_start(arena_cant_words(l->souls != 0), FACE_MEGAMAN);
	return false;
}

void square_arena_challenge(const OnlinePeer *peer, uint32_t latency_ms) {
	if (!CW_ARENA || A.stage != SA_IDLE || !mine_goes()) return;
	A.latency = latency_ms;
	A.other = peer->id;
	A.match = 0;
	A.ended = false;
	snprintf(A.name, sizeof A.name, "%s", peer->name);
	act(ARENA_ACT_CHALLENGE, peer->id);
	A.stage = SA_SENT;
	A.since = SDL_GetTicks();
	A.notice_serial = A.s.notice_serial;
	talk_start(arena_sent_words(peer->name), FACE_MEGAMAN);
	printf("arena: challenged %s (session %u)\n", peer->name, peer->id);
}

/* START: my MegaMan sent, and kept as the other reads him */
static void start(unsigned side, unsigned delay) {
	arena_link_net(&A.link, A.match, side, delay, out, NULL);
	const OnlineLoadout *carried = online_visit_loadout();
	static OnlineLoadout tried;
	/* (arenahp=N: the carried MegaMan's max HP made N, a test's short battle) */
	if (carried && dev.arena_hp > 0) {
		tried = *carried;
		tried.max_hp = tried.base_hp = (uint16_t)dev.arena_hp;
		carried = &tried;
	}
	uint8_t bytes[ONLINE_ARENA_LOADOUT_MAX], m[ARENA_LOADOUT_HEAD + ONLINE_ARENA_LOADOUT_MAX];
	size_t n = arena_loadout_pack(carried, bytes, sizeof bytes);
	A.why[0] = 0;
	size_t size = n ? arena_wire_loadout(m, sizeof m, A.match, bytes, n) : 0;
	A.stage = SA_LOADOUT;
	A.since = SDL_GetTicks();
	A.theirs_in = false;
	printf("arena: match %u starts: side %u, delay %u frames\n", A.match, A.link.link.side, A.link.link.delay);
	if (!size || !online_transport_send(m, size) || !arena_loadout_unpack(bytes, n, &A.mine, &A.mine_carried, A.why, sizeof A.why))
		fail("our MegaMan could not be sent");
}

/* the server said the battle is over for us (the other left, declined,
 * or finished) */
static void ended(void) {
	A.ended = true;
	switch (A.stage) {
	case SA_SENT: case SA_WAITING: say(arena_declined_words()); A.stage = SA_IDLE; break;
	case SA_ASKING: A.stage = SA_IDLE; break;
	case SA_ACCEPTED: say(arena_gone_words()); A.stage = SA_IDLE; break;
	case SA_LOADOUT: case SA_CONNECTING: case SA_READY: fail("the other player left"); say(arena_gone_words()); break;
	/* (its keys already come are spent first: arena_link_net.h) */
	case SA_BATTLE: arena_link_net_ended(&A.link); break;
	default: break;
	}
}

static const char *name_of(uint32_t id) {
	for (size_t i = 0; i < A.s.peer_count; ++i) if (A.s.peers[i].id == id) return A.s.peers[i].name;
	return NULL;
}

static void state_in(const ArenaWireState *s) {
	if (s->stage == ARENA_WIRE_CHALLENGED) {
		const char *name = name_of(s->other);
		bool blocked = online_people_flags(s->identity) & ONLINE_BLOCKED;
		if (!CW_ARENA || A.stage != SA_IDLE || blocked || !name) { act(ARENA_ACT_LEAVE, s->match); return; }
		A.match = s->match;
		A.other = s->other;
		A.ended = false;
		snprintf(A.name, sizeof A.name, "%s", name);
		A.stage = SA_ASKING;
		printf("arena: %s challenged us (match %u)\n", name, s->match);
		return;
	}
	if (s->stage == ARENA_WIRE_WAITING && A.stage == SA_SENT && s->other == A.other) { A.match = s->match; A.stage = SA_WAITING; return; }
	if (s->match != A.match) return;
	if (s->stage == ARENA_WIRE_START && (A.stage == SA_WAITING || A.stage == SA_ACCEPTED)) start(s->side, s->delay);
	else if (s->stage == ARENA_WIRE_ENDED) ended();
}

static void loadout_in(const uint8_t *data, size_t size) {
	uint32_t match;
	const uint8_t *bytes;
	size_t n;
	if (A.stage != SA_LOADOUT || !arena_wire_read_loadout(data, size, &match, &bytes, &n) || match != A.match) return;
	char why[120];
	if (!arena_loadout_unpack(bytes, n, &A.theirs, &A.theirs_carried, why, sizeof why) ||
	    (A.theirs_carried && !arena_loadout_check(&A.theirs, why, sizeof why))) {
		char said[160];
		snprintf(said, sizeof said, "the other MegaMan can't go: %s", why);
		fail(said);
		return;
	}
	A.theirs_in = true;
}

/* (arenadesync=N: the other's keys from frame N for a second turned to
 * other directions and buttons here, a link gone wrong: the hashes' stop
 * tested) */
#define DESYNC_FRAMES 60
#define DESYNC_KEYS   0x33u   /* A, B, RIGHT, LEFT */
static size_t desynced(const uint8_t *data, size_t size, uint8_t *out, size_t cap) {
	uint32_t match, frame, at = (uint32_t)dev.arena_desync;
	uint16_t keys[ONLINE_ARENA_INPUT_MAX];
	unsigned count;
	if (dev.arena_desync <= 0 || !arena_wire_read_input(data, size, &match, &frame, keys, &count)) return 0;
	bool changed = false;
	for (unsigned i = 0; i < count; ++i)
		if (frame + i >= at && frame + i < at + DESYNC_FRAMES) { keys[i] ^= DESYNC_KEYS; changed = true; }
	if (changed && frame <= at) printf("arena: (arenadesync) the other's keys from frame %u changed here\n", at);
	return changed ? arena_wire_input(out, cap, match, frame, keys, count) : 0;
}

void square_arena_mail(const uint8_t *data, size_t size) {
	ArenaWireState s;
	if (data[0] == ONLINE_ARENA_STATE) { if (arena_wire_state(data, size, &s)) state_in(&s); return; }
	if (data[0] == ONLINE_ARENA_LOADOUT) { loadout_in(data, size); return; }
	if (A.stage < SA_LOADOUT || A.stage >= SA_OVER) return;
	uint8_t changed[ONLINE_MAX_MESSAGE];
	size_t n = desynced(data, size, changed, sizeof changed);
	arena_link_net_take(&A.link, n ? changed : data, n ? n : size);
}

/* the challenged player's answer, as its chat closes */
static void answered(void) {
	bool yes = flag_get(ACCEPT_FLAG);
	flag_clear(ACCEPT_FLAG);
	if (A.ended) { A.stage = SA_IDLE; if (yes) say(arena_gone_words()); return; }
	if (!yes || !mine_goes()) { act(ARENA_ACT_LEAVE, A.match); A.ended = true; A.stage = SA_IDLE; return; }
	act(ARENA_ACT_ACCEPT, A.match);
	A.stage = SA_ACCEPTED;
	printf("arena: we took %s's challenge\n", A.name);
}

void square_arena_update(const OnlineSnapshot *s) {
	A.s = *s;
	A.latency = s->latency_ms;
	bool open = talk_busy() || emu_read8(BN6_CHATBOX);
	/* (a challenge the server refused with a notice: the target busy or gone) */
	if (A.stage == SA_SENT && (s->notice_serial != A.notice_serial || SDL_GetTicks() - A.since > SENT_WAIT_MS)) A.stage = SA_IDLE;
	if (A.stage == SA_ASKED && !open) answered();
	else if (A.stage == SA_ASKING && !open) {
		flag_clear(ACCEPT_FLAG);
		if (talk_ask(arena_challenged_words(A.name), FACE_MEGAMAN, arena_challenged_question(), ACCEPT_FLAG)) A.stage = SA_ASKED;
	} else if (A.said && !open && A.stage == SA_IDLE && talk_start(A.said, FACE_MEGAMAN)) A.said = NULL;
}

bool square_arena_on(void) { return A.stage >= SA_LOADOUT; }

static void build(void) {
	const OnlineLoadout *loadout[2];
	unsigned side = A.link.link.side;
	loadout[side] = A.mine_carried ? &A.mine : NULL;
	loadout[side ^ 1] = A.theirs_carried ? &A.theirs : NULL;
	A.arena = arena_new(R.data, ROM_SIZE, loadout, &A.link.link);
	if (!A.arena) { fail("too little memory for the two PETs"); return; }
	A.stage = SA_CONNECTING;
}

static void connecting(void) {
	arena_view_connect(A.arena);
	ArenaStage stage = arena_stage(A.arena);
	if (stage == ARENA_FAILED) { fail(arena_error(A.arena)); return; }
	if (stage != ARENA_BATTLE) return;
	uint64_t hash = arena_hash(A.arena);
	printf("arena: ready, the pair's hash %016llx\n", (unsigned long long)hash);
	if (!arena_link_net_ready(&A.link, hash)) { fail("our PET's ready could not be sent"); return; }
	A.stage = SA_READY;
	A.since = SDL_GetTicks();
}

static void ready(void) {
	uint64_t theirs;
	if (!arena_link_net_peer_ready(&A.link, &theirs)) {
		if (SDL_GetTicks() - A.since > READY_WAIT_MS) fail("the other PET never got ready");
		return;
	}
	if (theirs != arena_hash(A.arena)) { fail_as("the two games went apart before the battle", true); return; }
	printf("arena: both ready, the battle begins\n");
	A.stage = SA_BATTLE;
}

static void battle(uint32_t keys) {
	uint32_t stalls = arena_stalls(A.arena);
	ArenaStage stage = arena_tick(A.arena, (uint16_t)(keys & ARENA_KEYS_MASK));
	arena_sound(A.arena);
	A.stalled = arena_stalls(A.arena) != stalls ? A.stalled + 1 : 0;
	if (stage == ARENA_FAILED) {
		/* (the other gone, his keys spent: the link broke; else the games went apart, or the cable stuck) */
		bool left = A.ended && A.link.broken[0];
		fail_as(arena_error(A.arena), arena_apart(A.arena));
		if (left) say(arena_gone_words());
		return;
	}
	if (stage != ARENA_OVER) return;
	unsigned result = arena_result(A.arena);
	if (A.link.link.side == 0 && !A.ended) {
		arena_link_net_flush(&A.link);
		act(ARENA_ACT_OVER, A.match);
	}
	A.ended = true;
	last.result = result == 1 || result == 2 ? result : 3;
	snprintf(last.name, sizeof last.name, "%s", A.name);
	say(result == 1 ? arena_won_words() : result == 2 ? arena_lost_words() : arena_over_words());
	A.stage = SA_OVER;
	A.over_t = 0;
}

/* back on the Square's map, where MegaMan stood */
static void back(void) {
	drop_arena();
	arena_view_free();
	A.stage = SA_IDLE;
	A.match = 0;
	printf("arena: back in the Square\n");
}

void square_arena_frame(uint32_t keys) {
	online_mail_frame();
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	bool lost = s.error[0] || !s.connected;
	if (lost && A.stage == SA_BATTLE) arena_link_net_lost(&A.link, "the connection to the Square was lost");
	else if (lost && A.stage < SA_OVER) fail("the connection to the Square was lost");
	switch (A.stage) {
	case SA_LOADOUT:
		if (A.theirs_in) build();
		else if (SDL_GetTicks() - A.since > LOADOUT_WAIT_MS) fail("the other MegaMan never came");
		break;
	case SA_CONNECTING: connecting(); break;
	case SA_READY: ready(); break;
	case SA_BATTLE: battle(keys); break;
	case SA_OVER:
		if (++A.over_t >= OVER_TICKS || (A.over_t >= OVER_HELD && (btn_pressed(BTN_A) || btn_pressed(BTN_B)))) back();
		break;
	default: break;
	}
}

void square_arena_draw(void) {
	arena_view_clear();
	const uint32_t *v = A.arena ? arena_video(A.arena) : NULL;
	if (A.stage == SA_OVER) {
		if (v) arena_view_core(v);
		if (A.why[0] || !v) arena_view_stopped(A.why);
		else arena_view_result(arena_result(A.arena), true);
		return;
	}
	if (A.stage == SA_BATTLE && v) {
		arena_view_core(v);
		if (A.stalled >= STALL_SHOWN) arena_view_waiting();
		return;
	}
	arena_view_connecting(A.arena ? arena_progress(A.arena) : 0);
}

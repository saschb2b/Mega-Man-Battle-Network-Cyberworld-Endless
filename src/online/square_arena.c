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
 * it too, unless the server has already said it is.
 *
 * Every battle is fought by full rules (issue #131): both games hold the
 * NetPass's Arena stamp (netpass.h), Battle offered only where both do
 * (online_chat.c), the server refusing the rest (NOTICE 28). Where more than one of MegaMan's three
 * folders holds chips, Lan picks the one carried in BN6's chat box; where
 * none does, MegaMan says so. The carried MegaMan is then checked, the
 * same check on both games.
 *
 * A battle with a result is offered again (docs/ONLINE_ARENA.md,
 * Rematches): back in the Square, MegaMan says how it went and asks Lan
 * whether to go again with the same player, the cursor on No. A Yes sends
 * the same challenge again, quietly; the other
 * game takes a challenge from that player at once where its player said
 * Yes too (his question still open: it waits for the answer), turns it
 * down where he said No, and asks as ever once the rematch's time is out.
 * The folder carried is the last battle's, asked only where it no longer
 * fits. Both Yeses at once cross on the server, which turns the second
 * challenge down as busy: the first is taken. */
#include "square_arena.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "analytics_online.h"
#include "arena_folder.h"
#include "arena_link_net.h"
#include "arena_loadout.h"
#include "arena_report.h"
#include "arena_view.h"
#include "arena_wire.h"
#include "arena_words.h"
#include "bn6.h"
#include "devtools.h"
#include "emu.h"
#include "online_mail.h"
#include "online_people.h"
#include "online_transport.h"
#include "online_visit.h"
#include "platform.h"
#include "rom.h"
#include "talk.h"
#include "text.h"

#define LOADOUT_WAIT_MS 15000    /* the other's MegaMan, after START */
#define READY_WAIT_MS   60000    /* the other's way in (a slow machine's) */
#define SENT_WAIT_MS    5000     /* the server's word on a challenge */
#define OVER_TICKS      300      /* the result shown before the Square */
#define OVER_HELD       30       /* ... at least, before A goes back */
#define STALL_SHOWN     20
#define AGAIN_MS        45000    /* a rematch's answer kept, from the battle's end */

typedef enum {
	SA_IDLE,
	SA_FOLDER,      /* the folder to carry asked, before my challenge or my Yes */
	SA_SENT,        /* my challenge sent: the server's word awaited */
	SA_WAITING,     /* ... the other's answer */
	SA_ASKING,      /* challenged: the question to ask once no chat is open */
	SA_ASKED,       /* ... open */
	SA_ACCEPTED,    /* Yes said: START awaited */
	SA_AGAIN,       /* back after a result: the rematch to ask once no chat is open */
	SA_AGAIN_ASKED, /* ... open */
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
	bool challenger;          /* this game challenged (the folder asked before the challenge) */
	int folder;               /* the folder carried, 0-2 */
	OnlineSnapshot s;
	ArenaPings pings;         /* the round trips measured, for the delay asked for (arena_sync.h) */
	uint32_t notice_serial;
	ArenaNetLink link;
	Arena *arena;
	OnlineLoadout mine, theirs;
	bool mine_carried, theirs_carried, theirs_in;
	int over_t, stalled;
	bool counted;             /* the battle's end told to the statistics */
	const char *said;         /* MegaMan's word, once back in the Square */
	char why[160];
} A;

/* the last battle this game fought from the Square, for its referee */
static struct { SquareBout bout; uint32_t other; char name[ONLINE_NAME_BYTES + 1]; } last;

/* the rematch: the last battle's other player and folder, Lan's answer,
 * and a challenge of his come before it */
enum { AGAIN_UNSAID, AGAIN_YES, AGAIN_NO };
static struct {
	uint32_t other, since, held;
	char name[ONLINE_NAME_BYTES + 1];
	int folder, answer;
} again;

bool square_arena_offered(void) { return CW_ARENA; }

const SquareBout *square_arena_last(void) {
	last.bout.name = last.bout.result ? last.name : NULL;
	return &last.bout;
}

/* a rematch's answer still kept */
static bool again_on(void) { return again.other && SDL_GetTicks() - again.since < AGAIN_MS; }

static bool out(void *context, const uint8_t *data, size_t size) {
	(void)context;
	return online_transport_send(data, size);
}

/* the latency a challenge or a Yes says (protocol v8): the round trip with
 * its jitter, less what this system's rollback window covers (the frame's
 * cost kept here, arena_window.h); arenawindow=N's window, or arenadelay=N
 * asking for that delay (the server's: two frames and the latency's) */
static uint32_t asked(void) {
	if (dev.arena_delay > 0) return dev.arena_delay > 2 ? (uint32_t)((dev.arena_delay - 2) * 1000 / 60) : 0;
	unsigned window = dev.arena_window >= 0 ? (unsigned)dev.arena_window : arena_window_kept();
	uint32_t ms = arena_ask_ms(A.latency, window);
	printf("arena: asking with %u ms: the round trips' %u ms less a window of %u\n", (unsigned)ms, (unsigned)A.latency, window);
	return ms;
}

static void act(unsigned action, uint32_t ref) {
	uint8_t m[ONLINE_ARENA_BYTES];
	if (arena_wire_action(m, action, ref, asked())) online_transport_send(m, sizeof m);
}

static void say(const char *words) { A.said = words; }

/* The statistics' NetBattle (analytics_online.h), once a battle whose pair
 * was built: how it ended and the network's numbers the Arena kept */
static void counted(int how) {
	if (!A.arena || A.counted) return;
	A.counted = true;
	ArenaNumbers n;
	arena_numbers(A.arena, &n);
	AnalyticsBattle b = { how, (int)A.link.link.side, (int)n.delay, (int)n.window, (int)n.rollbacks, (int)n.deepest,
		(int)n.stalls, n.worst_tick_ms, (int)n.frames };
	analytics_netbattle(&b);
}

static void drop_arena(void) {
	arena_free(A.arena);
	A.arena = NULL;
}

void square_arena_reset(void) {
	drop_arena();
	memset(&A, 0, sizeof A);
	memset(&again, 0, sizeof again);
	arena_folder_reset();
}

void square_arena_end(void) {
	if (A.stage == SA_BATTLE) counted(ANALYTICS_BATTLE_LEFT);
	if (A.match && !A.ended && A.stage != SA_IDLE) act(ARENA_ACT_LEAVE, A.match);
	if (again.held) act(ARENA_ACT_LEAVE, again.held);
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

/* my MegaMan can go with some folder (the game's start's where none was
 * carried), `fits` the folders that can; else MegaMan says why. The
 * carried MegaMan checked as the other game will. */
static bool mine_goes(unsigned *fits) {
	const OnlineLoadout *l = online_visit_loadout();
	*fits = l ? arena_folders_fitting(l) : 1u;
	if (!l) return true;
	printf("arena: the folders that hold chips %#x\n", *fits);
	if (!*fits) {
		talk_start(arena_no_folder_words(), FACE_MEGAMAN);
		return false;
	}
	static OnlineLoadout carried;
	char why[120];
	uint8_t bytes[ONLINE_ARENA_LOADOUT_MAX];
	arena_format_carry(l, arena_folder_first(*fits), &carried);
	if (arena_loadout_check(&carried, why, sizeof why) && arena_loadout_pack(&carried, bytes, sizeof bytes)) return true;
	printf("arena: our MegaMan can't go: %s\n", why);
	talk_start(arena_cant_words(), FACE_MEGAMAN);
	return false;
}

/* the folder to carry asked, where Lan has a choice */
static bool folder_question(unsigned fits) { return arena_folder_asked(fits) && arena_folder_ask(fits); }

/* (a rematch's quietly: nothing changed since the last) */
static void challenge_sent(bool quiet) {
	act(ARENA_ACT_CHALLENGE, A.other);
	A.stage = SA_SENT;
	A.since = SDL_GetTicks();
	A.notice_serial = A.s.notice_serial;
	if (!quiet) talk_start(arena_sent_words(A.name), FACE_MEGAMAN);
	printf("arena: challenged %s (session %u), folder %d\n", A.name, A.other, A.folder + 1);
}

static void send_challenge(void) { challenge_sent(false); }

void square_arena_challenge(const OnlinePeer *peer, uint32_t latency_ms) {
	if (!CW_ARENA || A.stage != SA_IDLE) return;
	unsigned fits;
	if (!mine_goes(&fits)) return;
	A.latency = A.pings.n ? arena_pings_latency(&A.pings) : latency_ms;
	A.other = peer->id;
	A.match = 0;
	A.ended = false;
	A.challenger = true;
	snprintf(A.name, sizeof A.name, "%s", peer->name);
	A.folder = arena_folder_first(fits);
	if (folder_question(fits)) { A.stage = SA_FOLDER; return; }
	send_challenge();
}

/* START: my MegaMan sent, and kept as the other reads him */
static void start(unsigned side, unsigned delay) {
	/* (a rematch's answer spent: the next is asked after this battle's result) */
	memset(&again, 0, sizeof again);
	arena_link_net(&A.link, A.match, side, delay, out, NULL);
	const OnlineLoadout *carried = online_visit_loadout();
	static OnlineLoadout tried;
	/* (the folder picked as the one he fights with) */
	if (carried) {
		arena_format_carry(carried, A.folder, &tried);
		carried = &tried;
	}
	/* (arenahp=N: the carried MegaMan's max HP made N, a test's short battle) */
	if (carried && dev.arena_hp > 0) tried.max_hp = tried.base_hp = (uint16_t)dev.arena_hp;
	uint8_t bytes[ONLINE_ARENA_LOADOUT_MAX], m[ARENA_LOADOUT_HEAD + ONLINE_ARENA_LOADOUT_MAX];
	size_t n = arena_loadout_pack(carried, bytes, sizeof bytes);
	A.why[0] = 0;
	size_t size = n ? arena_wire_loadout(m, sizeof m, A.match, bytes, n) : 0;
	A.stage = SA_LOADOUT;
	A.since = SDL_GetTicks();
	A.theirs_in = false;
	/* (the stands' report is the cable's first's to send: arena_report.h) */
	if (side == 0) arena_report_begin(A.match);
	printf("arena: match %u starts: side %u, delay %u frames, folder %d\n", A.match, A.link.link.side, A.link.link.delay, A.folder + 1);
	if (!size || !online_transport_send(m, size) || !arena_loadout_unpack(bytes, n, &A.mine, &A.mine_carried, A.why, sizeof A.why))
		fail("our MegaMan could not be sent");
}

/* the server said the battle is over for us (the other left, declined,
 * or finished) */
static bool apart(void);

static void ended(void) {
	A.ended = true;
	switch (A.stage) {
	case SA_SENT: case SA_WAITING:
		printf("arena: %s turned the challenge down (match %u)\n", A.name, A.match);
		say(arena_declined_words());
		A.stage = SA_IDLE;
		break;
	case SA_ASKING: A.stage = SA_IDLE; break;
	case SA_ACCEPTED: say(arena_gone_words()); A.stage = SA_IDLE; break;
	/* (gone once his ready came apart from ours: the stop he saw, his to tell the server as the cable's first) */
	case SA_READY: if (apart()) break;
	/* fall through */
	case SA_LOADOUT: case SA_CONNECTING: fail("the other player left"); say(arena_gone_words()); break;
	/* (its keys already come are spent first: arena_link_net.h) */
	case SA_BATTLE: arena_link_net_ended(&A.link); break;
	default: break;
	}
}

static const char *name_of(uint32_t id) {
	for (size_t i = 0; i < A.s.peer_count; ++i) if (A.s.peers[i].id == id) return A.s.peers[i].name;
	return NULL;
}

static void accept(void);

/* the folder carried again: the last battle's where it still fits; false
 * where MegaMan can't go (he says why) or Lan is asked */
static bool again_folder(void) {
	unsigned fits;
	if (!mine_goes(&fits)) return false;
	bool same = fits >> again.folder & 1;
	A.folder = same ? again.folder : arena_folder_first(fits);
	if (!same && folder_question(fits)) { A.stage = SA_FOLDER; return false; }
	return true;
}

/* the rematch's battle, as the last one's other player */
static void again_as(bool challenger) {
	A.other = again.other;
	A.challenger = challenger;
	A.ended = false;
	snprintf(A.name, sizeof A.name, "%s", again.name);
}

/* his challenge taken at once, both having said Yes */
static void again_take(uint32_t match) {
	A.stage = SA_IDLE;
	again_as(false);
	A.match = match;
	printf("arena: %s's rematch taken (match %u)\n", A.name, match);
	if (again_folder()) accept();
	else if (A.stage != SA_FOLDER) { act(ARENA_ACT_LEAVE, match); A.ended = true; }
}

/* Lan's Yes, no challenge of his come: ours sent */
static void again_send(void) {
	A.stage = SA_IDLE;
	again_as(true);
	A.match = 0;
	if (again_folder()) challenge_sent(true);
}

/* a challenge from the last battle's other player while the rematch's
 * answer is kept: taken, turned down, or held for Lan's; false where it
 * is a challenge as ever */
static bool again_in(const ArenaWireState *s) {
	if (!again_on() || s->other != again.other) return false;
	if (again.answer == AGAIN_NO) {
		act(ARENA_ACT_LEAVE, s->match);
		printf("arena: %s's rematch turned down (match %u)\n", again.name, s->match);
		return true;
	}
	/* (both Yeses crossed: his came first, ours the server turns down as busy) */
	if (again.answer == AGAIN_YES && (A.stage == SA_IDLE || A.stage == SA_SENT)) { again_take(s->match); return true; }
	if (A.stage != SA_OVER && A.stage != SA_AGAIN && A.stage != SA_AGAIN_ASKED) return false;
	again.held = s->match;
	printf("arena: %s asks for a rematch (match %u)\n", again.name, s->match);
	return true;
}

static void state_in(const ArenaWireState *s) {
	if (s->stage == ARENA_WIRE_CHALLENGED && again_in(s)) return;
	/* (a held rematch ended: the other left, or its time on the server ran out) */
	if (s->stage == ARENA_WIRE_ENDED && again.held && s->match == again.held) { again.held = 0; return; }
	if (s->stage == ARENA_WIRE_CHALLENGED) {
		const char *name = name_of(s->other);
		bool blocked = online_people_flags(s->identity) & ONLINE_BLOCKED;
		/* (a game without the NetPass's Arena stamp is never challenged: the server's NOTICE 28) */
		if (!CW_ARENA || A.stage != SA_IDLE || blocked || !name) { act(ARENA_ACT_LEAVE, s->match); return; }
		A.match = s->match;
		A.other = s->other;
		A.ended = false;
		A.challenger = false;
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

static void accept(void) {
	act(ARENA_ACT_ACCEPT, A.match);
	A.stage = SA_ACCEPTED;
	printf("arena: we took %s's challenge, folder %d\n", A.name, A.folder + 1);
}

/* the challenged player's answer, as its chat closes */
static void answered(void) {
	bool yes = arena_folder_challenge_yes();
	unsigned fits = 0;
	if (A.ended) { A.stage = SA_IDLE; if (yes) say(arena_gone_words()); return; }
	if (!yes || !mine_goes(&fits)) { act(ARENA_ACT_LEAVE, A.match); A.ended = true; A.stage = SA_IDLE; return; }
	A.folder = arena_folder_first(fits);
	if (folder_question(fits)) { A.stage = SA_FOLDER; return; }
	accept();
}

/* --dev arenaboth: Lan's Yes at once, the first folder that fits (a
 * NetBattle that plays itself on a device or in a browser, arena.h) */
static void bot_yes(void) {
	unsigned fits = 0;
	if (A.ended || !mine_goes(&fits)) { act(ARENA_ACT_LEAVE, A.match); A.ended = true; A.stage = SA_IDLE; return; }
	A.folder = arena_folder_first(fits);
	accept();
}

/* Lan's answer to the rematch, as its chat closes */
static void again_answered(void) {
	bool yes = arena_folder_challenge_yes();
	uint32_t held = again.held;
	again.held = 0;
	again.answer = yes ? AGAIN_YES : AGAIN_NO;
	A.stage = SA_IDLE;
	printf("arena: a rematch with %s: %s\n", again.name, yes ? "yes" : "no");
	if (!yes) { if (held) act(ARENA_ACT_LEAVE, held); return; }
	if (held) again_take(held);
	else again_send();
}

/* back after a result: the rematch asked, MegaMan's word on the battle
 * first, where the other is still here */
static void again_ask(void) {
	if (!name_of(again.other)) { A.stage = SA_IDLE; return; }
	if (!arena_folder_again(A.said ? A.said : "", again.name)) return;
	A.said = NULL;
	A.stage = SA_AGAIN_ASKED;
}

/* the folder picked, as its chat closes: the challenge sent, or the Yes */
static void folder_picked(void) {
	int k = arena_folder_picked();
	if (k >= 0) A.folder = k;
	if (A.challenger) {
		if (k < 0) A.stage = SA_IDLE;
		else send_challenge();
		return;
	}
	if (A.ended) { A.stage = SA_IDLE; if (k >= 0) say(arena_gone_words()); return; }
	if (k < 0) { act(ARENA_ACT_LEAVE, A.match); A.ended = true; A.stage = SA_IDLE; return; }
	accept();
}

void square_arena_update(const OnlineSnapshot *s) {
	A.s = *s;
	/* (the latency said in a challenge or a Yes: the round trip with its jitter) */
	arena_pings_note(&A.pings, s->latency_ms, SDL_GetTicks());
	A.latency = A.pings.n ? arena_pings_latency(&A.pings) : s->latency_ms;
	bool open = talk_busy() || emu_read8(BN6_CHATBOX);
	/* (a challenge the server refused with a notice: the target busy or gone) */
	if (A.stage == SA_SENT && (s->notice_serial != A.notice_serial || SDL_GetTicks() - A.since > SENT_WAIT_MS)) A.stage = SA_IDLE;
	if (A.stage == SA_ASKED && !open) answered();
	else if (A.stage == SA_AGAIN_ASKED && !open) again_answered();
	else if (A.stage == SA_AGAIN && !open) again_ask();
	else if (A.stage == SA_FOLDER && !open) folder_picked();
	else if (A.stage == SA_ASKING && !open) {
		if (dev.arena_both) bot_yes();
		else if (arena_folder_challenge(A.name)) A.stage = SA_ASKED;
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
	A.counted = false;
	arena_window(A.arena, dev.arena_window);
	A.stage = SA_CONNECTING;
}

/* the other's ready come and apart from ours: both stopped before the first frame, told by its cause where the
 * installs' records say it (BN5's installs made otherwise: a game that took BN5 in after its core started, or another
 * build); false where it has not come or agrees */
static bool apart(void) {
	uint64_t theirs, record;
	if (!arena_link_net_peer_ready(&A.link, &theirs) || theirs == arena_hash(A.arena)) return false;
	bool installs = arena_link_net_peer_record(&A.link, &record) && record != arena_record(A.arena);
	fail_as(installs ? "the two games installed BN5 apart (one took BN5 in after it started, or another build)" :
		"the two games went apart before the battle", true);
	counted(ANALYTICS_BATTLE_APART);
	if (installs) say(arena_installs_apart_words());
	return true;
}

static void connecting(void) {
	arena_view_connect(A.arena);
	ArenaStage stage = arena_stage(A.arena);
	if (stage == ARENA_FAILED) { fail(arena_error(A.arena)); return; }
	if (stage != ARENA_BATTLE) return;
	uint64_t hash = arena_hash(A.arena);
	printf("arena: ready, the pair's hash %016llx\n", (unsigned long long)hash);
	/* (the installs' record first: a ready apart is then told by its cause where the records are) */
	if (!arena_link_net_record(&A.link, arena_record(A.arena)) || !arena_link_net_ready(&A.link, hash)) {
		fail("our PET's ready could not be sent");
		return;
	}
	A.stage = SA_READY;
	A.since = SDL_GetTicks();
}

static void ready(void) {
	uint64_t theirs;
	if (!arena_link_net_peer_ready(&A.link, &theirs)) {
		if (SDL_GetTicks() - A.since > READY_WAIT_MS) fail("the other PET never got ready");
		return;
	}
	if (apart()) return;
	printf("arena: both ready, the battle begins\n");
	A.stage = SA_BATTLE;
}

/* a result: the referee's tally, the bouts with one player in a row, and
 * the rematch's answer to come */
static void bouts(unsigned result) {
	SquareBout *b = &last.bout;
	if (last.other != A.other || strcmp(last.name, A.name)) b->won = b->lost = 0;
	b->result = result == 1 || result == 2 ? result : 3;
	b->won += result == 1;
	b->lost += result == 2;
	++b->bouts;
	last.other = A.other;
	snprintf(last.name, sizeof last.name, "%s", A.name);
	memset(&again, 0, sizeof again);
	again.other = A.other;
	again.since = SDL_GetTicks();
	again.folder = A.folder;
	snprintf(again.name, sizeof again.name, "%s", A.name);
}

static void battle(uint32_t keys, uint32_t rtt_ms) {
	static uint32_t tick;
	if (dev.arena_both) keys = arena_bot_keys(A.arena, A.link.link.side, tick++);
	uint32_t stalls = arena_stalls(A.arena);
	arena_rtt(A.arena, rtt_ms);
	ArenaStage stage = arena_tick(A.arena, (uint16_t)(keys & ARENA_KEYS_MASK));
	arena_sound(A.arena);
	arena_report_frame(A.arena);
	A.stalled = arena_stalls(A.arena) != stalls ? A.stalled + 1 : 0;
	if (stage == ARENA_FAILED) {
		/* (the other gone, his keys spent: the link broke; else the games went apart, or the cable stuck) */
		bool left = A.ended && A.link.broken[0];
		counted(arena_apart(A.arena) ? ANALYTICS_BATTLE_APART : A.link.broken[0] ? ANALYTICS_BATTLE_DISCONNECTED : ANALYTICS_BATTLE_STOPPED);
		fail_as(arena_error(A.arena), arena_apart(A.arena));
		if (left) say(arena_gone_words());
		return;
	}
	if (stage != ARENA_OVER) return;
	unsigned result = arena_result(A.arena);
	if (A.link.link.side == 0 && !A.ended) {
		arena_link_net_flush(&A.link);
		arena_report_end(result);
		act(ARENA_ACT_OVER, A.match);
	}
	A.ended = true;
	counted(result == 1 ? ANALYTICS_BATTLE_WON : result == 2 ? ANALYTICS_BATTLE_LOST : ANALYTICS_BATTLE_OVER);
	bouts(result);
	say(result == 1 ? arena_won_words() : result == 2 ? arena_lost_words() : arena_over_words());
	A.stage = SA_OVER;
	A.over_t = 0;
}

/* back on the Square's map, where MegaMan stood: the rematch asked after
 * a result */
static void back(void) {
	drop_arena();
	arena_view_free();
	A.stage = !A.why[0] && again.other == A.other && again.answer == AGAIN_UNSAID ? SA_AGAIN : SA_IDLE;
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
	case SA_BATTLE: battle(keys, s.latency_ms); break;
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

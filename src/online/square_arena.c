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
 * The way in shows a PET Network panel (arena_card.h): both players,
 * each MegaMan as carried, the set's score, whose PET is still on its
 * way. The battle shows the other's name on the Custom screen, the link's
 * badge, and a stall's note that names whose PET it waits for, offers a
 * way out and stops at its end (arena_view.h). It ends on a result card:
 * vs whom, the time, the HP left, the chips used, the set's score; a
 * stop's on the card too, in the PET's words, and a bout broken off is
 * counted neither won nor lost (docs/ONLINE_ARENA.md, The way in and the
 * result). A challenge the server refuses MegaMan says no to in the chat
 * box, the corner's notice left unshown.
 *
 * A battle with a result is offered again on its card (docs/ONLINE_ARENA.md,
 * Rematches), the cursor on No. A Yes sends the same challenge again,
 * quietly, the card kept up ("Waiting for NAME..."); the other game, its
 * card still up, holds a challenge from that player and shows it ("NAME
 * wants another!"), takes it at once where its player says Yes (or said
 * it first), and turns it down where he said No. Closing the card is No.
 * The folder carried is the last battle's, asked in the Square only where
 * it no longer fits. Both Yeses at once cross on the server, which turns
 * the second challenge down as busy: the first is taken. MegaMan says how
 * it went back in the Square.
 *
 * A Team NetBattle's bout (square_team.h) is this battle too, gone into
 * without a question (square_arena_bout), its MegaMan carrying the HP his
 * last bout left him; its card asks no rematch, and MegaMan's word waits
 * for the match's end. */
#include "square_arena.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "analytics_online.h"
#include "arena_folder.h"
#include "arena_loadout.h"
#include "arena_pair.h"
#include "arena_report.h"
#include "arena_view.h"
#include "arena_wire.h"
#include "arena_words.h"
#include "devtools.h"
#include "emu.h"
#include "online_mail.h"
#include "online_people.h"
#include "online_transport.h"
#include "online_visit.h"
#include "rom.h"
#include "square_arena_state.h"
#include "square_team.h"
#include "talk.h"
#include "text.h"

#define LOADOUT_WAIT_MS 15000    /* the other's MegaMan, after START */
#define READY_WAIT_MS   60000    /* the other's way in (a slow machine's) */
#define SENT_WAIT_MS    5000     /* the server's word on a challenge */

SquareArenaState SA;
SquareArenaLast SA_LAST;
SquareArenaAgain SA_AGAIN;

bool square_arena_offered(void) { return CW_ARENA; }

const SquareBout *square_arena_last(void) {
	SA_LAST.bout.name = SA_LAST.bout.result ? SA_LAST.name : NULL;
	return &SA_LAST.bout;
}

/* a rematch's answer still kept */
static bool again_on(void) { return SA_AGAIN.other && SDL_GetTicks() - SA_AGAIN.since < AGAIN_MS; }

static bool out(void *context, const uint8_t *data, size_t size) {
	(void)context;
	return online_transport_send(data, size);
}

/* the latency a challenge or a Yes says (protocol v8): the round trip with
 * its jitter, less what this system's rollback window covers but a frame
 * (the frame's cost kept here, arena_window.h); arenawindow=N's window, or
 * arenadelay=N asking for that delay (the server's: a frame and the
 * latency's, issue #167) */
static uint32_t asked(void) {
	if (dev.arena_delay > 0) return dev.arena_delay > 1 ? (uint32_t)((dev.arena_delay - 1) * 1000 / 60) : 0;
	unsigned window = dev.arena_window >= 0 ? (unsigned)dev.arena_window : arena_window_kept();
	uint32_t ms = arena_ask_ms(SA.latency, window);
	printf("arena: asking with %u ms: the round trips' %u ms less a window of %u\n", (unsigned)ms, (unsigned)SA.latency, window);
	return ms;
}

static void act(unsigned action, uint32_t ref) {
	uint8_t m[ONLINE_ARENA_BYTES];
	if (arena_wire_action(m, action, ref, asked())) online_transport_send(m, sizeof m);
}

/* (a team bout's word is the team's, said at the match's end: square_team.c) */
static void say(const char *words) { if (!SA.team) SA.said = words; }

/* The statistics' NetBattle (analytics_online.h), once a battle whose pair
 * was built: how it ended and the network's numbers the Arena kept */
static void counted(int how) {
	if (!SA.arena || SA.counted) return;
	SA.counted = true;
	ArenaNumbers n;
	arena_numbers(SA.arena, &n);
	AnalyticsBattle b = { how, (int)SA.link.link.side, (int)n.delay, (int)n.window, (int)n.rollbacks, (int)n.deepest,
		(int)n.stalls, n.worst_tick_ms, (int)n.frames };
	analytics_netbattle(&b);
}

static void drop_arena(void) {
	arena_free(SA.arena);
	SA.arena = NULL;
}

static void drop_kept(void) {
	arena_kept_free(SA.kept);
	SA.kept = NULL;
}

void square_arena_reset(void) {
	drop_arena();
	drop_kept();
	memset(&SA, 0, sizeof SA);
	memset(&SA_AGAIN, 0, sizeof SA_AGAIN);
	arena_folder_reset();
	square_team_reset();
}

void square_arena_end(void) {
	if (SA.stage == SA_BATTLE) counted(ANALYTICS_BATTLE_LEFT);
	if (SA.match && !SA.ended && SA.stage != SA_IDLE) act(ARENA_ACT_LEAVE, SA.match);
	if (SA_AGAIN.held) act(ARENA_ACT_LEAVE, SA_AGAIN.held);
	square_team_end();
	square_arena_reset();
}

static void bouts(unsigned result);

/* the battle stopped: the server told unless it said so first, the cause
 * in the log and the PET's line on the card; a bout begun counted broken
 * off, neither won nor lost */
static void fail_as(const char *why, ArenaStop stop, bool both) {
	snprintf(SA.why, sizeof SA.why, "%s", why);
	SA.stop = stop;
	printf("arena: match %u stopped: %s\n", SA.match, SA.why);
	/* (what both games see, two hashes apart, is the cable's first's to
	 * tell, as the battle's end is: the server would answer the second
	 * with a notice that it is over) */
	if (!SA.ended && (!both || SA.link.link.side == 0)) act(ARENA_ACT_LEAVE, SA.match);
	SA.ended = true;
	drop_kept();
	say(arena_stopped_words());
	bool begun = SA.stage == SA_BATTLE;
	if (begun) bouts(4);
	SA.stage = SA_OVER;
	square_card_up(begun ? 4 : 5, ARENA_ASK_NONE);
}

static void fail(const char *why, ArenaStop stop) { fail_as(why, stop, false); }

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
	act(ARENA_ACT_CHALLENGE, SA.other);
	SA.stage = SA_SENT;
	SA.since = SDL_GetTicks();
	SA.seen_at = 0;
	SA.notice_serial = SA.s.notice_serial;
	if (!quiet) talk_start(arena_sent_words(SA.name), FACE_MEGAMAN);
	printf("arena: challenged %s (session %u), folder %d\n", SA.name, SA.other, SA.folder + 1);
}

static void send_challenge(void) { challenge_sent(false); }

/* our open challenge called off from his card (protocol v13): the server
 * told, MegaMan's word; a WAITING still to come is left as it comes, and
 * the battle's ENDED is no longer this game's (state_in) */
static void call_off(void) {
	if (SA.match) act(ARENA_ACT_LEAVE, SA.match);
	printf("arena: the challenge to %s called off (match %u)\n", SA.name, SA.match);
	SA.stage = SA_IDLE;
	SA.match = 0;
	SA.ended = true;
	SA.seen_at = 0;
	talk_start(arena_called_off_words(SA.name), FACE_MEGAMAN);
}

bool square_arena_open_to(uint32_t id) { return id && (SA.stage == SA_SENT || SA.stage == SA_WAITING) && !SA.card.up && SA.other == id; }

bool square_arena_open(uint32_t *other, const char **name, uint32_t *seen_ms) {
	if ((SA.stage != SA_SENT && SA.stage != SA_WAITING) || SA.card.up) return false;
	*other = SA.other;
	*name = SA.name;
	*seen_ms = SA.seen_at ? SDL_GetTicks() - SA.seen_at + 1 : 0;
	return true;
}

uint32_t square_arena_asking(void) { return (SA.stage == SA_ASKING || SA.stage == SA_ASKED) && !SA.ended ? SA.other : 0; }

void square_arena_challenge(const OnlinePeer *peer, uint32_t latency_ms) {
	/* (a challenge open: Call off on his card, MegaMan's word on another's) */
	if (square_arena_open_to(peer->id)) { call_off(); return; }
	if (SA.stage == SA_SENT || SA.stage == SA_WAITING) { talk_start(arena_asked_already_words(SA.name), FACE_MEGAMAN); return; }
	if (!CW_ARENA || SA.stage != SA_IDLE) return;
	/* (our own partner: said at once, as the server's NOTICE 29 would) */
	if (peer->id && peer->id == square_team_partner()) { talk_start(arena_team_partner_words(peer->name), FACE_MEGAMAN); return; }
	unsigned fits;
	if (!mine_goes(&fits)) return;
	SA.latency = SA.pings.n ? arena_pings_latency(&SA.pings) : latency_ms;
	SA.other = peer->id;
	SA.match = 0;
	SA.ended = false;
	SA.challenger = true;
	SA.team = false;
	snprintf(SA.name, sizeof SA.name, "%s", peer->name);
	SA.folder = arena_folder_first(fits);
	if (folder_question(fits)) { SA.stage = SA_FOLDER; return; }
	send_challenge();
}

/* START: my MegaMan sent, and kept as the other reads him */
static void start(unsigned side, unsigned delay) {
	/* (a rematch's answer spent: the next is asked after this battle's result) */
	memset(&SA_AGAIN, 0, sizeof SA_AGAIN);
	/* (a rematch from the card: the last battle's pair kept as it began,
	 * gone into again where both MegaMen and sides are the same, the rest
	 * given back first; its tally kept on the card) */
	drop_kept();
	SA.kept = arena_keep(SA.arena);
	drop_arena();
	arena_view_free();
	arena_link_net(&SA.link, SA.match, side, delay, out, NULL);
	const OnlineLoadout *carried = online_visit_loadout();
	static OnlineLoadout tried;
	/* (the folder picked as the one he fights with) */
	if (carried) {
		arena_format_carry(carried, SA.folder, &tried);
		carried = &tried;
	}
	/* (arenahp=N: the carried MegaMan's max HP made N, a test's short battle) */
	if (carried && dev.arena_hp > 0) tried.max_hp = tried.base_hp = (uint16_t)dev.arena_hp;
	/* (a NetBattle begins whole; a Team NetBattle's winner goes on with what he has left) */
	if (carried) tried.hp = SA.team && SA.carry && SA.carry < tried.max_hp ? SA.carry : tried.max_hp;
	uint8_t bytes[ONLINE_ARENA_LOADOUT_MAX], m[ARENA_LOADOUT_HEAD + ONLINE_ARENA_LOADOUT_MAX];
	size_t n = arena_loadout_pack(carried, bytes, sizeof bytes);
	SA.why[0] = 0;
	size_t size = n ? arena_wire_loadout(m, sizeof m, SA.match, bytes, n) : 0;
	SA.stage = SA_LOADOUT;
	SA.since = SDL_GetTicks();
	SA.theirs_in = false;
	/* (the stands' report is the cable's first's to send: arena_report.h) */
	if (side == 0) arena_report_begin(SA.match);
	printf("arena: match %u starts: side %u, delay %u frames, folder %d%s\n", SA.match, SA.link.link.side, SA.link.link.delay, SA.folder + 1,
		SA.team ? ", a Team NetBattle's bout" : "");
	/* (into the way in's panel: from a rematch's card, or from the Square) */
	SA.card.up = false;
	SA.t = 0;
	arena_card_cue(ARENA_CUE_OPEN);
	if (!size || !online_transport_send(m, size) || !arena_loadout_unpack(bytes, n, &SA.mine, &SA.mine_carried, SA.why, sizeof SA.why))
		fail("our MegaMan could not be sent", ARENA_STOP_ERROR);
}

/* the server said the battle is over for us (the other left, declined,
 * or finished) */
static bool apart(void);

static void ended(void) {
	SA.ended = true;
	switch (SA.stage) {
	case SA_SENT: case SA_WAITING:
		printf("arena: %s turned the challenge down (match %u)\n", SA.name, SA.match);
		/* (a rematch asked on the card: his No shown there) */
		if (SA.card.up) square_card_gone();
		else say(arena_declined_words());
		SA.stage = SA.card.up ? SA_OVER : SA_IDLE;
		break;
	case SA_ASKING: SA.stage = SA_IDLE; break;
	case SA_ACCEPTED:
		if (SA.card.up) { square_card_gone(); SA.stage = SA_OVER; break; }
		say(arena_gone_words());
		SA.stage = SA_IDLE;
		break;
	/* (gone once his ready came apart from ours: the stop he saw, his to tell the server as the cable's first) */
	case SA_READY: if (apart()) break;
	/* fall through */
	case SA_LOADOUT: case SA_CONNECTING: fail("the other player left", ARENA_STOP_LEFT); say(arena_gone_words()); break;
	/* (its keys already come are spent first: arena_link_net.h) */
	case SA_BATTLE: arena_link_net_ended(&SA.link); break;
	default: break;
	}
}

const char *square_arena_name_of(uint32_t id) {
	for (size_t i = 0; i < SA.s.peer_count; ++i) if (SA.s.peers[i].id == id) return SA.s.peers[i].name;
	return NULL;
}

static void accept(void);

/* the folder carried again: the last battle's where it still fits; false
 * where MegaMan can't go (he says why) or Lan is asked */
static bool again_folder(void) {
	unsigned fits;
	if (!mine_goes(&fits)) return false;
	bool same = fits >> SA_AGAIN.folder & 1;
	SA.folder = same ? SA_AGAIN.folder : arena_folder_first(fits);
	/* (the folder no longer fits: the card closed, Lan asked in the Square) */
	if (!same && arena_folder_asked(fits)) { SA.card.up = false; drop_arena(); arena_view_free(); }
	if (!same && folder_question(fits)) { SA.stage = SA_FOLDER; return false; }
	return true;
}

/* the rematch's battle, as the last one's other player */
static void again_as(bool challenger) {
	SA.other = SA_AGAIN.other;
	SA.challenger = challenger;
	SA.ended = false;
	snprintf(SA.name, sizeof SA.name, "%s", SA_AGAIN.name);
}

/* his challenge taken at once, both having said Yes */
void square_arena_again_take(uint32_t match) {
	SA.stage = SA_IDLE;
	again_as(false);
	SA.match = match;
	printf("arena: %s's rematch taken (match %u)\n", SA.name, match);
	if (again_folder()) accept();
	else if (SA.stage != SA_FOLDER) { act(ARENA_ACT_LEAVE, match); SA.ended = true; }
	if (SA.card.up && SA.stage == SA_IDLE) SA.stage = SA_OVER;
}

/* Lan's Yes, no challenge of his come: ours sent */
void square_arena_again_send(void) {
	SA.stage = SA_IDLE;
	again_as(true);
	SA.match = 0;
	if (again_folder()) challenge_sent(true);
	if (SA.card.up && SA.stage == SA_IDLE) SA.stage = SA_OVER;
}

/* a challenge from the last battle's other player while the rematch's
 * answer is kept: taken, turned down, or held for Lan's; false where it
 * is a challenge as ever */
static bool again_in(const ArenaWireState *s) {
	if (!again_on() || s->other != SA_AGAIN.other) return false;
	if (SA_AGAIN.answer == AGAIN_NO) {
		act(ARENA_ACT_LEAVE, s->match);
		printf("arena: %s's rematch turned down (match %u)\n", SA_AGAIN.name, s->match);
		return true;
	}
	/* (both Yeses crossed: his came first, ours the server turns down as busy) */
	if (SA_AGAIN.answer == AGAIN_YES && (SA.stage == SA_IDLE || SA.stage == SA_SENT || SA.stage == SA_OVER)) { square_arena_again_take(s->match); return true; }
	if (!SA.card.up) return false;
	SA_AGAIN.held = s->match;
	SA.card.theirs = ARENA_THEIRS_YES;
	arena_card_cue(ARENA_CUE_ANSWER);
	/* (his Yes shown on the card: seen, its 30 seconds from now, protocol v13) */
	act(ARENA_ACT_SEEN, s->match);
	printf("arena: %s asks for a rematch (match %u)\n", SA_AGAIN.name, s->match);
	return true;
}

static void state_in(const ArenaWireState *s) {
	if (s->stage == ARENA_WIRE_CHALLENGED && again_in(s)) return;
	/* (a held rematch ended: the other left, or its time on the server ran out) */
	if (s->stage == ARENA_WIRE_ENDED && SA_AGAIN.held && s->match == SA_AGAIN.held) {
		SA_AGAIN.held = 0;
		if (SA.card.up) square_card_gone();
		return;
	}
	if (s->stage == ARENA_WIRE_CHALLENGED) {
		const char *name = square_arena_name_of(s->other);
		bool blocked = online_people_flags(s->identity) & ONLINE_BLOCKED;
		/* (a game without the NetPass's Arena stamp is never challenged: the server's NOTICE 28) */
		if (!CW_ARENA || SA.stage != SA_IDLE || blocked || !name) { act(ARENA_ACT_LEAVE, s->match); return; }
		SA.match = s->match;
		SA.other = s->other;
		SA.ended = false;
		SA.challenger = false;
		SA.team = false;
		snprintf(SA.name, sizeof SA.name, "%s", name);
		SA.stage = SA_ASKING;
		/* (the PET rings: a challenge has come, its question once no chat is open) */
		arena_card_cue(ARENA_CUE_RING);
		printf("arena: %s challenged us (match %u)\n", name, s->match);
		return;
	}
	if (s->stage == ARENA_WIRE_WAITING && SA.stage == SA_SENT && s->other == SA.other) { SA.match = s->match; SA.stage = SA_WAITING; return; }
	/* (his question up on his screen: his 30 seconds run from now, protocol v13) */
	if (s->stage == ARENA_WIRE_SEEN) {
		if (s->match == SA.match && SA.stage == SA_WAITING && !SA.seen_at) SA.seen_at = SDL_GetTicks() | 1u;
		printf("arena: %s reads our challenge (match %u)\n", SA.name, s->match);
		return;
	}
	/* (a challenge of ours no longer wanted: a rematch's Yes taken back by closing the card before the server's word) */
	if (s->stage == ARENA_WIRE_WAITING) { act(ARENA_ACT_LEAVE, s->match); return; }
	if (s->match != SA.match) return;
	if (s->stage == ARENA_WIRE_START && (SA.stage == SA_WAITING || SA.stage == SA_ACCEPTED)) start(s->side, s->delay);
	else if (s->stage == ARENA_WIRE_ENDED) ended();
}

static void loadout_in(const uint8_t *data, size_t size) {
	uint32_t match;
	const uint8_t *bytes;
	size_t n;
	if (SA.stage != SA_LOADOUT || !arena_wire_read_loadout(data, size, &match, &bytes, &n) || match != SA.match) return;
	char why[120];
	if (!arena_loadout_unpack(bytes, n, &SA.theirs, &SA.theirs_carried, why, sizeof why) ||
	    (SA.theirs_carried && !arena_loadout_check(&SA.theirs, why, sizeof why))) {
		char said[160];
		snprintf(said, sizeof said, "the other MegaMan can't go: %s", why);
		fail(said, ARENA_STOP_CANT);
		return;
	}
	SA.theirs_in = true;
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
	if (SA.stage < SA_LOADOUT || SA.stage >= SA_OVER) return;
	uint8_t changed[ONLINE_MAX_MESSAGE];
	size_t n = desynced(data, size, changed, sizeof changed);
	arena_link_net_take(&SA.link, n ? changed : data, n ? n : size);
}

static void accept(void) {
	act(ARENA_ACT_ACCEPT, SA.match);
	SA.stage = SA_ACCEPTED;
	printf("arena: we took %s's challenge, folder %d\n", SA.name, SA.folder + 1);
}

/* the challenged player's answer, as its chat closes */
static void answered(void) {
	bool yes = arena_folder_challenge_yes();
	unsigned fits = 0;
	if (SA.ended) { SA.stage = SA_IDLE; if (yes) say(arena_gone_words()); return; }
	if (!yes || !mine_goes(&fits)) { act(ARENA_ACT_LEAVE, SA.match); SA.ended = true; SA.stage = SA_IDLE; return; }
	SA.folder = arena_folder_first(fits);
	if (folder_question(fits)) { SA.stage = SA_FOLDER; return; }
	accept();
}

/* --dev arenaboth: Lan's Yes at once, the first folder that fits (a
 * NetBattle that plays itself on a device or in a browser, arena.h) */
static void bot_yes(void) {
	unsigned fits = 0;
	if (SA.ended || !mine_goes(&fits)) { act(ARENA_ACT_LEAVE, SA.match); SA.ended = true; SA.stage = SA_IDLE; return; }
	SA.folder = arena_folder_first(fits);
	accept();
}

/* the folder picked, as its chat closes: the challenge sent, or the Yes */
static void folder_picked(void) {
	int k = arena_folder_picked();
	if (k >= 0) SA.folder = k;
	if (SA.challenger) {
		if (k < 0) SA.stage = SA_IDLE;
		else send_challenge();
		return;
	}
	if (SA.ended) { SA.stage = SA_IDLE; if (k >= 0) say(arena_gone_words()); return; }
	if (k < 0) { act(ARENA_ACT_LEAVE, SA.match); SA.ended = true; SA.stage = SA_IDLE; return; }
	accept();
}

/* a challenge the server refused with a notice (protocol.json's codes:
 * busy, no longer here, the Arena closed, the NetPass's stamp): MegaMan
 * says so in the chat box, the corner's notice left unshown
 * (square_arena_notice) */
static bool refusal(uint16_t code) { return code == 16 || code == 17 || code == 18 || code == ONLINE_NOTICE_PASS || code == ONLINE_NOTICE_TEAM; }

/* NOTICE 29: our partner himself, or a team and a lone Navi */
static const char *team_refusal(void) {
	uint32_t partner = square_team_partner();
	return partner == SA.other ? arena_team_partner_words(SA.name) : arena_team_lone_words(SA.name, partner != 0);
}

static void refused(uint16_t code) {
	SA.stage = SA_IDLE;
	printf("arena: the challenge to %s refused (notice %u)\n", SA.name, (unsigned)code);
	say(code == 16 ? arena_busy_words(SA.name) : code == 17 ? arena_not_here_words(SA.name) : code == 18 ? arena_closed_words() :
		code == ONLINE_NOTICE_PASS ? arena_no_stamp_words() :
		code == ONLINE_NOTICE_TEAM ? team_refusal() : arena_declined_words());
}

bool square_arena_notice(uint32_t serial, uint16_t code) {
	/* (a team asked of a player, refused: square_team.h) */
	return (SA.stage == SA_SENT && refusal(code)) || serial == SA.quiet_notice || square_team_notice(serial, code);
}

/* a notice come while the Square is held (the card, a rematch's challenge
 * sent from it): a refusal of that challenge shown on the card, and none
 * of the Arena's shown in the corner as the Square comes back */
static void notice_held(const OnlineSnapshot *s) {
	if (s->notice_serial == SA.s.notice_serial) return;
	SA.s.notice_serial = s->notice_serial;
	SA.s.notice_code = s->notice_code;
	if (!refusal(s->notice_code)) return;
	SA.quiet_notice = s->notice_serial;
	if (SA.card.up && SA.stage == SA_SENT) {
		printf("arena: the rematch's challenge to %s refused (notice %u)\n", SA.name, (unsigned)s->notice_code);
		SA.stage = SA_OVER;
		square_card_gone();
	}
}

void square_arena_update(const OnlineSnapshot *s) {
	SA.s = *s;
	/* (the latency said in a challenge or a Yes: the round trip with its jitter) */
	arena_pings_note(&SA.pings, s->latency_ms, SDL_GetTicks());
	SA.latency = SA.pings.n ? arena_pings_latency(&SA.pings) : s->latency_ms;
	bool open = talk_busy() || emu_read8(BN6_CHATBOX);
	/* (a challenge the server refused with a notice: the target busy or gone) */
	if (SA.stage == SA_SENT && s->notice_serial != SA.notice_serial) refused(s->notice_code);
	else if (SA.stage == SA_SENT && SDL_GetTicks() - SA.since > SENT_WAIT_MS) SA.stage = SA_IDLE;
	if (SA.stage == SA_ASKED && !open) answered();
	else if (SA.stage == SA_FOLDER && !open) folder_picked();
	else if (SA.stage == SA_ASKING && !open) {
		if (dev.arena_both) bot_yes();
		else if (arena_folder_challenge(SA.name, square_team_partner() != 0, SA.other)) {
			/* (the question up: the server's clock for the answer starts now, and the challenger sees it) */
			SA.stage = SA_ASKED;
			act(ARENA_ACT_SEEN, SA.match);
		}
	} else if (SA.said && !open && SA.stage == SA_IDLE && talk_start(SA.said, FACE_MEGAMAN)) SA.said = NULL;
	square_team_update(s, false);
}

void square_arena_held_frame(void) {
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	square_team_update(&s, true);
}

bool square_arena_on(void) { return SA.stage >= SA_LOADOUT || SA.card.up; }

static void build(void) {
	const OnlineLoadout *loadout[2];
	unsigned side = SA.link.link.side;
	loadout[side] = SA.mine_carried ? &SA.mine : NULL;
	loadout[side ^ 1] = SA.theirs_carried ? &SA.theirs : NULL;
	/* (the bout's seed its match, the same on both games and every
	 * spectator; arenaseedoff=N moves it on this side alone, a mutation) */
	uint32_t seed = SA.match + (uint32_t)dev.arena_seed_off;
	SA.arena = arena_new_bout(R.data, ROM_SIZE, loadout, &SA.link.link, seed, SA.kept);
	SA.kept = NULL;
	if (!SA.arena) { fail("too little memory for the two PETs", ARENA_STOP_MEMORY); return; }
	SA.counted = false;
	arena_window(SA.arena, dev.arena_window);
	SA.stage = SA_CONNECTING;
}

/* the other's ready come and apart from ours: both stopped before the first frame, told by its cause where the
 * installs' records say it (BN5's installs made otherwise: a game that took BN5 in after its core started, or another
 * build); false where it has not come or agrees */
static bool apart(void) {
	uint64_t theirs, record;
	if (!arena_link_net_peer_ready(&SA.link, &theirs) || theirs == arena_hash(SA.arena)) return false;
	bool installs = arena_link_net_peer_record(&SA.link, &record) && record != arena_record(SA.arena);
	fail_as(installs ? "the two games installed BN5 apart (one took BN5 in after it started, or another build)" :
		"the two games went apart before the battle", installs ? ARENA_STOP_INSTALLS : ARENA_STOP_APART, true);
	counted(ANALYTICS_BATTLE_APART);
	if (installs) say(arena_installs_apart_words());
	return true;
}

static void connecting(void) {
	arena_view_connect(SA.arena);
	ArenaStage stage = arena_stage(SA.arena);
	if (stage == ARENA_FAILED) { fail(arena_error(SA.arena), ARENA_STOP_ERROR); return; }
	if (stage != ARENA_BATTLE) return;
	uint64_t hash = arena_hash(SA.arena);
	printf("arena: ready, the pair's hash %016llx\n", (unsigned long long)hash);
	/* (the installs' record first: a ready apart is then told by its cause where the records are) */
	if (!arena_link_net_record(&SA.link, arena_record(SA.arena)) || !arena_link_net_ready(&SA.link, hash)) {
		fail("our PET's ready could not be sent", ARENA_STOP_ERROR);
		return;
	}
	SA.stage = SA_READY;
	SA.since = SDL_GetTicks();
}

static void ready(void) {
	uint64_t theirs;
	if (!arena_link_net_peer_ready(&SA.link, &theirs)) {
		if (SDL_GetTicks() - SA.since > READY_WAIT_MS) fail("the other PET never got ready", ARENA_STOP_NOT_READY);
		return;
	}
	if (apart()) return;
	/* (the frames from START: a rematch from the kept pair's about a second, docs/ONLINE_ARENA.md) */
	printf("arena: both ready, the battle begins %u frames after START%s\n", (unsigned)SA.t,
		arena_again(SA.arena) ? ", the way in again from the kept pair" : "");
	SA.stage = SA_BATTLE;
	SA.ticks = 0;
	/* (from the panel into the Custom screen, through white) */
	SA.t = 0;
	arena_card_cue(ARENA_CUE_BATTLE);
}

/* a result, or a bout broken off (4): the referee's tally, the bouts with
 * one player in a row, and the rematch's answer to come */
static void bouts(unsigned result) {
	SquareBout *b = &SA_LAST.bout;
	if (SA_LAST.other != SA.other || strcmp(SA_LAST.name, SA.name)) b->won = b->lost = 0;
	b->result = result == 1 || result == 2 || result == 4 ? result : 3;
	b->won += result == 1;
	b->lost += result == 2;
	++b->bouts;
	SA_LAST.other = SA.other;
	snprintf(SA_LAST.name, sizeof SA_LAST.name, "%s", SA.name);
	memset(&SA_AGAIN, 0, sizeof SA_AGAIN);
	/* (a bout broken off offers no rematch: his next challenge is asked as ever) */
	if (result == 4) return;
	SA_AGAIN.other = SA.other;
	SA_AGAIN.since = SDL_GetTicks();
	SA_AGAIN.folder = SA.folder;
	snprintf(SA_AGAIN.name, sizeof SA_AGAIN.name, "%s", SA.name);
}

/* a stall past its end, or B once it offers the way out: the battle
 * left, broken off; false where it waits on */
static bool stalled_out(void) {
	uint32_t waited = arena_waited(SA.arena);
	bool leave = square_card_leave();
	if (!leave && waited < STALL_END) return false;
	printf("arena: %u ticks waiting on %s's keys: %s\n", (unsigned)waited, SA.name, leave ? "we leave" : "given up");
	counted(ANALYTICS_BATTLE_DISCONNECTED);
	fail(leave ? "we left, waiting on the other's keys" : "the other's keys stopped coming", leave ? ARENA_STOP_WE_LEFT : ARENA_STOP_SILENT);
	return true;
}

/* arenafreeze=N: the battle held still on this side from its tick N, its link kept */
#define FREEZE_TICKS 2400
static bool frozen(uint32_t tick) { return dev.arena_freeze > 0 && tick >= (uint32_t)dev.arena_freeze && tick < (uint32_t)dev.arena_freeze + FREEZE_TICKS; }

static void battle(uint32_t keys, uint32_t rtt_ms) {
	static uint32_t tick;
	if (frozen(++SA.ticks)) return;
	if (dev.arena_both) keys = arena_bot_keys(SA.arena, SA.link.link.side, tick++);
	arena_rtt(SA.arena, rtt_ms);
	ArenaStage stage = arena_tick(SA.arena, (uint16_t)(keys & ARENA_KEYS_MASK));
	arena_sound(SA.arena);
	arena_report_frame(SA.arena);
	if (stage == ARENA_FAILED) {
		/* (the other gone, his keys spent: the link broke; else the games went apart, or the cable stuck) */
		bool left = SA.ended && SA.link.broken[0], went_apart = arena_apart(SA.arena);
		counted(went_apart ? ANALYTICS_BATTLE_APART : SA.link.broken[0] ? ANALYTICS_BATTLE_DISCONNECTED : ANALYTICS_BATTLE_STOPPED);
		fail_as(arena_error(SA.arena), went_apart ? ARENA_STOP_APART : left ? ARENA_STOP_LEFT : ARENA_STOP_ERROR, went_apart);
		if (left) say(arena_gone_words());
		return;
	}
	if (stage == ARENA_BATTLE && stalled_out()) return;
	if (stage != ARENA_OVER) return;
	unsigned result = arena_result(SA.arena);
	if (SA.link.link.side == 0 && !SA.ended) {
		arena_link_net_flush(&SA.link);
		arena_report_end(result);
		act(ARENA_ACT_OVER, SA.match);
	}
	SA.ended = true;
	counted(result == 1 ? ANALYTICS_BATTLE_WON : result == 2 ? ANALYTICS_BATTLE_LOST : ANALYTICS_BATTLE_OVER);
	bouts(result);
	say(result == 1 ? arena_won_words() : result == 2 ? arena_lost_words() : arena_over_words());
	SA.stage = SA_OVER;
	square_card_up(result == 1 || result == 2 ? result : 3, square_arena_name_of(SA.other) && !SA.team ? ARENA_ASK_OPEN : ARENA_ASK_NONE);
	SA.hp_left = SA.card.tally.hp[SA.link.link.side & 1u];
	printf("arena: our MegaMan has %u HP left\n", SA.hp_left);
}

bool square_arena_idle(void) { return SA.stage == SA_IDLE && !SA.card.up; }

uint16_t square_arena_last_hp(void) { return SA.hp_left; }

void square_arena_team_begun(void) {
	if (SA.stage == SA_SENT || SA.stage == SA_WAITING || SA.stage == SA_ACCEPTED) SA.stage = SA_IDLE;
	SA.match = 0;
	SA.ended = true;
}

bool square_arena_bout(uint32_t bout, uint32_t other, const char *name, uint16_t carry) {
	if (!CW_ARENA || !square_arena_idle()) return false;
	unsigned fits;
	if (!mine_goes(&fits)) { act(ARENA_ACT_LEAVE, bout); return true; }
	SA.match = bout;
	SA.other = other;
	SA.ended = false;
	SA.challenger = false;
	SA.team = true;
	SA.carry = carry;
	snprintf(SA.name, sizeof SA.name, "%s", name);
	/* (the folder the last bout carried where it still fits, else the first: no question mid-match) */
	SA.folder = fits >> SA_AGAIN.folder & 1 ? SA_AGAIN.folder : arena_folder_first(fits);
	printf("arena: into %s's team bout (match %u), %u HP carried\n", name, bout, (unsigned)carry);
	accept();
	return true;
}

/* back on the Square's map, where MegaMan stood: the card closed, a
 * rematch not said Yes to turned down (his challenge held, or one to come
 * while the answer is kept), my Yes's challenge called off */
void square_arena_back(void) {
	drop_arena();
	drop_kept();
	arena_view_free();
	SA.card.up = false;
	if (SA_AGAIN.answer == AGAIN_UNSAID && SA_AGAIN.other) SA_AGAIN.answer = AGAIN_NO;
	if (SA_AGAIN.held) { act(ARENA_ACT_LEAVE, SA_AGAIN.held); SA_AGAIN.held = 0; }
	if ((SA.stage == SA_SENT || SA.stage == SA_WAITING || SA.stage == SA_ACCEPTED) && SA.match) act(ARENA_ACT_LEAVE, SA.match);
	SA.stage = SA_IDLE;
	SA.match = 0;
	printf("arena: back in the Square\n");
}

void square_arena_frame(uint32_t keys) {
	online_mail_frame();
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	/* (who is here, for the card: the Square's own update waits while it is up) */
	SA.s.peer_count = s.peer_count;
	memcpy(SA.s.peers, s.peers, sizeof s.peers);
	notice_held(&s);
	++SA.t;
	bool lost = s.error[0] || !s.connected;
	if (lost && SA.stage == SA_BATTLE) arena_link_net_lost(&SA.link, "the connection to the Square was lost");
	else if (lost && SA.stage >= SA_LOADOUT && SA.stage < SA_OVER) fail("the connection to the Square was lost", ARENA_STOP_LOST);
	else if (lost && SA.card.up && !SA.card.closing) SA.card.closing = 1;
	switch (SA.stage) {
	case SA_LOADOUT:
		if (SA.theirs_in) build();
		else if (SDL_GetTicks() - SA.since > LOADOUT_WAIT_MS) fail("the other MegaMan never came", ARENA_STOP_NEVER_CAME);
		break;
	case SA_CONNECTING: connecting(); break;
	case SA_READY: ready(); break;
	case SA_BATTLE: battle(keys, s.latency_ms); break;
	default: break;
	}
	if (SA.card.up && (SA.stage < SA_LOADOUT || SA.stage == SA_OVER)) square_card_frame();
}


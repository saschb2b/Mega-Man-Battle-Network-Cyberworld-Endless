/* trade_central.h. Trade Central in the Net Square (docs/online/): BN6's
 * reception desk in the Square's Trade wing, its clerk's words in BN6's
 * chat boxes. Offered from his desk: the Navi to trade with, a page of two
 * names at a time, then the chip on the spares list (trade_screen.h, issue
 * #145), MegaMan's spare chips beside a card of the one under the cursor.
 * An offer that comes in is MegaMan's news wherever he stands, answered at
 * the desk the same way. Trades need the NetPass's Trade stamp on both games
 * (netpass.h, issue #131): without it the clerk says so and offers
 * nothing; with it he lists the Navis whose pass has it too (each peer's
 * ROMs the server's word, which refuses the rest with NOTICE 28), and
 * every spare chip, BN5's too. Once both chips are known the chip trade's
 * screen shows them, You and Them, the cursor on No deal; the server
 * commits when both said Deal!, the cards swap, and the swap is kept on
 * disk at once until the run has it at home (online_trade.c).
 *
 * The trade board (online_board.c): the clerk reads the channel's offers
 * one a box, each answered with a spare that meets its want (the spares
 * list listing only those), and puts MegaMan's own up (a spare, then
 * the want from a list) or takes it down. A taken offer comes back
 * answered, both chips named: the poster's game checks the want before
 * its Deal. One trade a visit: the run keeps one swap until it is home.
 * None on the week's dive (weekly.h): every diver keeps the standard
 * setup's chips, the clerk says so and an offer that comes in is called
 * off unheard of. */
#include "trade_central.h"

#include <stdio.h>
#include <string.h>

#include "analytics_online.h"
#include "bn6.h"
#include "bytes.h"
#include "chips.h"
#include "emu.h"
#include "flags.h"
#include "netpass.h"
#include "online_board.h"
#include "online_square.h"
#include "online_trade.h"
#include "online_transport.h"
#include "script_kit.h"
#include "talk.h"
#include "trade_list.h"
#include "trade_screen.h"
#include "trade_words.h"
#include "weekly.h"

/* Trade Central's event flags (the Square's own map: its RAM is thrown away) */
#define OFFER_FLAG 0x1494   /* the clerk's "Offer a chip" */
#define BACK_FLAG  0x1495   /* "Offer one back" */
#define NO_FLAG    0x1496   /* "No thanks" to an offer */
#define OFF_FLAG   0x1497   /* "Call it off" */
/* (0x1498 the review's Yes, before the chip trade's screen: free) */
#define TO_FLAG    0x1499   /* + k: the Navi picked, 0 to SQUARE_PEERS - 1 */
#define LAST_FLAG  (TO_FLAG + SQUARE_PEERS - 1)
#define BOARD_FLAG 0x14A5   /* the clerk's "Trade board" (0x14A0-0x14A4 are the Arena's) */
#define POST_FLAG  0x14A6   /* "Post an offer" */
#define DOWN_FLAG  0x14A7   /* "Take mine down" */
#define TAKE_FLAG  0x14A8   /* + k: the board's offer answered, 0 to SQUARE_PEERS - 1 */
#define TAKE_LAST  (TAKE_FLAG + SQUARE_PEERS - 1)
#define CHIP_FLAG  0x14B0   /* + k: the want picked in the chat box's list, 0 to LISTED - 1 */
#define LISTED 20           /* the chat box's list: at most these choices (its pages, choices and end in BN6's 32 scripts) */

_Static_assert(LAST_FLAG < BOARD_FLAG - 5 && TAKE_LAST < CHIP_FLAG && WANT_COUNT <= LISTED, "Trade Central's flags apart");

static struct {
	OnlineSnapshot s;
	uint32_t said;              /* the view's serial MegaMan has spoken of */
	uint32_t clerk;             /* the clerk's words as of (stage, peers, traded), 0 none yet */
	bool choosing, boarding;
	bool wanting;               /* the wants' list open, a post's second step */
	bool traded;                /* a trade done this visit: one a visit */
	bool closed;                /* on the week's dive: MegaMan's board offer taken down */
	bool sent;                  /* this game's answer, or take, sent: the chip trade shown at once when it comes back */
	int mode;                   /* TRADE_FOR_*: what the chip picked is for */
	uint32_t partner;
	char partner_name[ONLINE_NAME_BYTES + 1];
	uint32_t kept;              /* the done trade kept on disk */
	uint32_t deal;              /* the trade whose chip trade shows once MegaMan's word on it is said, 0 none */
	uint32_t who[SQUARE_PEERS];  /* the Navis listed, their sessions */
	int nwho;
	BoardOffer shown[SQUARE_PEERS];  /* the board's offers as the clerk read them */
	int nshown;
	BoardOffer offer;           /* the one answered */
	uint32_t took;              /* its poster, once the take is sent: this game the taker */
	uint32_t hushed;            /* a trade called off unheard of: an answer to MegaMan's offer not its want */
	TradeChip post;             /* the spare picked to put up */
} C;

/* the last player this game traded with, for the Square's talk */
static char last_partner[ONLINE_NAME_BYTES + 1];

const char *trade_central_last(void) { return last_partner[0] ? last_partner : NULL; }

static void clear_flags(void) {
	for (int f = OFFER_FLAG; f <= LAST_FLAG; ++f) flag_clear(f);
	for (int f = BOARD_FLAG; f <= TAKE_LAST; ++f) flag_clear(f);
}

void trade_central_reset(void) {
	memset(&C, 0, sizeof C);
	online_trade_reset();
	online_board_reset();
	TradeChip give, get;
	/* (a swap still kept from before: the run takes it first) */
	C.traded = online_trade_pending(&give, &get);
	clear_flags();
	for (int k = 0; k < LISTED; ++k) flag_clear(CHIP_FLAG + k);
}

static void chip_words(TradeChip c, char *out, size_t n) {
	char name[24];
	chips_name(c.id, name, sizeof name);
	snprintf(out, n, "%s %c", name, c.code == 26 ? '*' : 'A' + c.code);
}

/* the other player's name, as the snapshot has it */
static const char *name_of(uint32_t id) {
	for (size_t i = 0; i < C.s.peer_count; ++i) if (C.s.peers[i].id == id) return C.s.peers[i].name;
	return id == C.partner && C.partner_name[0] ? C.partner_name : "A Navi";
}

/* an archive into the Square's talk room, then its script 0 run */
static bool run(TextArchive *t) {
	static uint8_t out[TEXT_ARCHIVE_MAX];
	int n = ta_build(t, out);
	if (n > SQUARE_TALK_ROOM) return false;
	emu_write(online_square_talk_room(), out, (size_t)n);
	return talk_script(online_square_talk_room(), 0);
}

/* the clerk's words as the trade stands: hello and his options, an offer
 * to answer, the other player to wait on, or this visit's trade done */
static void clerk(void) {
	const TradeView *v = online_trade_view();
	/* (the top bit set: never 0, so the first words are written alone in
	 * the channel too, not the desk's empty archive) */
	bool diving = weekly_on(), stamp = netpass_has(STAMP_TRADE);
	uint32_t now = 1u << 31 | (uint32_t)v->stage << 24 | (v->serial & 0xFFFF) << 8 | (uint32_t)C.s.peer_count << 1 | (C.traded || diving || !stamp);
	if (now == C.clerk || emu_read8(BN6_CHATBOX)) return;
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	char chip[32];
	const char *const *options = trade_options;
	int n = 3, flags[3] = { OFFER_FLAG, BOARD_FLAG, -1 };
	ta_begin(&t);
	bool first = true;
	if (diving || !stamp) {
		ta_say(&t, FACE_PROG, stamp ? trade_diving() : trade_no_stamp());
		if (online_square_trade_say(out, ta_build(&t, out))) C.clerk = now;
		return;
	}
	if (v->stage == TRADE_OFFERED) {
		ta_script(&t);
		chip_words(v->theirs, chip, sizeof chip);
		ta_pages(&t, trade_offered(name_of(v->other), chip), FACE_PROG, &first);
		options = trade_offer_options;
		n = 2; flags[0] = BACK_FLAG; flags[1] = NO_FLAG;
	} else if (v->stage == TRADE_WAITING || v->stage == TRADE_ACCEPTED || v->stage == TRADE_ANSWERED) {
		ta_script(&t);
		ta_pages(&t, trade_waiting(name_of(v->other)), FACE_PROG, &first);
		options = trade_wait_options;
		n = 2; flags[0] = -1; flags[1] = OFF_FLAG;
	} else if (C.traded) {
		ta_say(&t, FACE_PROG, trade_traded());
		if (online_square_trade_say(out, ta_build(&t, out))) C.clerk = now;
		return;
	} else { ta_script(&t); ta_pages(&t, trade_hello(), FACE_PROG, &first); }
	ta_mugshot(&t, FACE_PROG);
	ta_clear(&t);
	ta_options(&t, options, n);
	int to[3] = { 1, 2, 3 };
	ta_choose(&t, to, n, n + 1);
	for (int k = 0; k < n; ++k) {
		ta_script(&t);
		if (flags[k] >= 0) ta_flag_set(&t, flags[k]);
		/* ("How it works": his boxes) */
		else if (options == trade_options) { bool f = true; ta_pages(&t, trade_how(), FACE_PROG, &f); }
		ta_end(&t);
	}
	ta_say(&t, FACE_PROG, trade_bye());
	int size = ta_build(&t, out);
	if (online_square_trade_say(out, size)) C.clerk = now;
	else if (C.clerk != (now & ~(1u << 31))) {
		C.clerk = now & ~(1u << 31);
		fprintf(stderr, "online: the clerk's words did not fit (%d bytes)\n", size);
	}
}


/* a page of two choices at a time, More going on, the first page with
 * `words` (labels[i] jumps to script `pick` + i) */
static void pages(TextArchive *t, const char *words, const char *const *labels, int n, int pick, int end) {
	int count = (n + 1) / 2;
	for (int p = 0; p < count; ++p) {
		ta_script(t);
		if (!p) ta_page(t, FACE_PROG, words, true);
		ta_mugshot(t, FACE_PROG);
		ta_clear(t);
		const char *shown[3];
		int to[3], k = 0;
		for (int i = 2 * p; i < n && i < 2 * p + 2; ++i) { shown[k] = labels[i]; to[k++] = pick + i; }
		if (count > 1) { shown[k] = p + 1 < count ? "More..." : "Back"; to[k++] = p + 1 < count ? p + 1 : 0; }
		ta_options(t, shown, k);
		ta_choose(t, to, k, end);
	}
}

/* whom to trade with: the channel's Navis whose NetPass has the Trade
 * stamp, where MegaMan has a spare, two a page, More going on */
static void who(void) {
	if (!C.s.peer_count) { talk_start(trade_none_here(), FACE_PROG); return; }
	static TradeChip spares[TRADE_LIST_MOST];
	static uint8_t count[TRADE_LIST_MOST];
	if (!online_trade_spares(spares, count, TRADE_LIST_MOST)) { talk_start(trade_no_spares(), FACE_PROG); return; }
	C.nwho = 0;
	for (size_t i = 0; i < C.s.peer_count; ++i)
		if (netpass_peer_full(C.s.peers[i].roms)) C.who[C.nwho++] = C.s.peers[i].id;
	if (!C.nwho) { talk_start(trade_none_stamped(), FACE_PROG); return; }
	static TextArchive t;
	const char *labels[SQUARE_PEERS] = { 0 };
	int n = C.nwho, pick = (n + 1) / 2;
	for (int i = 0; i < n; ++i) labels[i] = name_of(C.who[i]);
	ta_begin(&t);
	pages(&t, trade_who(), labels, n, pick, pick + n);
	for (int i = 0; i < n; ++i) { ta_script(&t); ta_flag_set(&t, TO_FLAG + i); ta_end(&t); }
	ta_closing(&t);
	run(&t);
}

/* a list in BN6's chat box, two a page, More going on, its pick in
 * CHIP_FLAG + k: a post's wants */
static bool list(const char *words, const char *const *labels, int n) {
	static TextArchive t;
	n = n < LISTED ? n : LISTED;
	int pick = (n + 1) / 2;
	ta_begin(&t);
	pages(&t, words, labels, n, pick, pick + n);
	for (int i = 0; i < n; ++i) { ta_script(&t); ta_flag_set(&t, CHIP_FLAG + i); ta_end(&t); }
	ta_closing(&t);
	for (int k = 0; k < LISTED; ++k) flag_clear(CHIP_FLAG + k);
	return run(&t);
}

int trade_central_spares(TradeRow *rows) {
	static TradeChip chips[TRADE_LIST_MOST];
	static uint8_t count[TRADE_LIST_MOST];
	int n = online_trade_spares(chips, count, TRADE_LIST_MOST);
	for (int i = 0; i < n; ++i) {
		const uint8_t *rec = chips_record(chips[i].id);
		rows[i] = (TradeRow){ chips[i], count[i], rec ? get16(rec + BN6_CHIP_ID_SORT) : chips[i].id, rec ? get16(rec + BN6_CHIP_ALPHA_SORT) : 0 };
	}
	return n;
}

/* the chip for `partner` (0: one to put up on the board), of the kind
 * `want` (-1 any): the spares list, those that meet it */
static void pick(uint32_t partner, int want) {
	static TradeRow all[TRADE_LIST_MOST], fit[TRADE_LIST_MOST];
	int n = trade_central_spares(all), m = trade_list_filter(all, n, want, online_board_meets, fit);
	if (!n) { talk_start(trade_no_spares(), FACE_PROG); return; }
	if (!m) { talk_start(trade_take_none(name_of(partner), want), FACE_PROG); return; }
	TradeChip theirs = C.mode == TRADE_FOR_ANSWER ? online_trade_view()->theirs : (TradeChip){ 0, 0 };
	trade_screen_list(fit, m, C.mode, partner ? name_of(partner) : NULL, want, theirs);
}

/* the board as the clerk reads it: one offer a box (whose, what he has,
 * what he wants), Answer it and Next offer under it, and MegaMan's own
 * to put up or take down */
static void board(void) {
	const BoardOffer *all;
	int n = online_board_offers(&all);
	C.nshown = 0;
	for (int i = 0; i < n && C.nshown < SQUARE_PEERS; ++i)
		if (all[i].player != C.s.player_id) C.shown[C.nshown++] = all[i];
	bool posted = online_board_of(C.s.player_id) != NULL;
	static TextArchive t;
	int m = C.nshown, boxes = m ? m : 1, own = boxes + m, end = own + 1;
	ta_begin(&t);
	for (int p = 0; p < boxes; ++p) {
		ta_script(&t);
		if (!p) ta_page(&t, FACE_PROG, trade_board_intro(m), true);
		const char *labels[3];
		int to[3], k = 0;
		if (m) {
			char chip[32];
			chip_words(C.shown[p].have, chip, sizeof chip);
			ta_page(&t, FACE_PROG, trade_board_offer(name_of(C.shown[p].player), chip, C.shown[p].want), false);
			labels[k] = trade_board_options[0]; to[k++] = boxes + p;
			if (m > 1) { labels[k] = trade_board_options[1]; to[k++] = (p + 1) % m; }
		}
		labels[k] = trade_board_mine(posted); to[k++] = own;
		if (!m) { labels[k] = trade_board_leave(); to[k++] = end; }
		ta_mugshot(&t, FACE_PROG);
		ta_clear(&t);
		ta_options(&t, labels, k);
		ta_choose(&t, to, k, end);
	}
	for (int p = 0; p < m; ++p) { ta_script(&t); ta_flag_set(&t, TAKE_FLAG + p); ta_end(&t); }
	ta_script(&t);
	ta_flag_set(&t, posted ? DOWN_FLAG : POST_FLAG);
	ta_end(&t);
	ta_closing(&t);
	run(&t);
}

/* an offer on the board answered: still up as read, a spare of its want */
static void take(BoardOffer o) {
	const BoardOffer *now = online_board_of(o.player);
	if (!now || now->have.id != o.have.id || now->have.code != o.have.code) return;
	C.offer = o;
	C.partner = o.player;
	snprintf(C.partner_name, sizeof C.partner_name, "%s", name_of(o.player));
	C.mode = TRADE_FOR_TAKE;
	pick(o.player, o.want);
}

/* the first of the chat box's list picked, -1 none; its flags cleared */
static int listed_pick(int n) {
	int got = -1;
	for (int k = 0; k < LISTED; ++k) {
		if (got < 0 && flag_get(CHIP_FLAG + k) && k < n) got = k;
		flag_clear(CHIP_FLAG + k);
	}
	return got;
}

/* a chip picked on the spares list: sent (or, for a post, the wants
 * listed next) */
static void picked(TradeChip c) {
	if (C.mode == TRADE_FOR_ANSWER) C.sent = online_trade_answer(c);
	else if (C.mode == TRADE_FOR_TAKE) {
		if (online_trade_take(C.offer.player, c, C.offer.have)) { C.took = C.offer.player; C.sent = true; }
	} else if (C.mode == TRADE_FOR_POST) { C.post = c; C.wanting = list(trade_post_want(), trade_want_options, WANT_COUNT); }
	else if (online_trade_propose(C.partner, c)) talk_start(trade_sent(C.partner_name), FACE_PROG);
}

/* the wants' list closed: the post sent */
static void wanted(void) {
	C.wanting = false;
	int want = listed_pick(WANT_COUNT);
	if (want < 0 || !online_board_post(C.post, want)) return;
	char chip[32];
	chip_words(C.post, chip, sizeof chip);
	talk_start(trade_posted(chip, want), FACE_PROG);
}

/* a trade begun by MegaMan's own offer on the board taken: true where
 * the chip it brings is not his want (or the offer not his): called off */
static bool unwanted(const TradeView *v) {
	if (!v->taken || v->other == C.took) return false;
	const BoardOffer *mine = online_board_of(C.s.player_id);
	return !mine || mine->have.id != v->mine.id || mine->have.code != v->mine.code || !online_board_meets(mine->want, v->theirs);
}

/* the chip trade's screen for the trade answered: both chips, You and Them */
static void deal(const TradeView *v) {
	C.deal = 0;
	if (v->stage == TRADE_ANSWERED) trade_screen_deal(v->mine, v->theirs, name_of(v->other));
}

/* what MegaMan says as the server's word on the trade comes */
static void news(void) {
	const TradeView *v = online_trade_view();
	/* (his word on an answer said: its chip trade) */
	if (C.deal && C.deal == v->id) { deal(v); return; }
	C.deal = 0;
	if (v->serial == C.said) return;
	char theirs[32];
	chip_words(v->theirs, theirs, sizeof theirs);
	bool said = true;
	/* (one called off unheard of: only its end, should it have been done) */
	if (v->id == C.hushed && v->stage != TRADE_DONE) said = true;
	else if (v->stage == TRADE_OFFERED) said = talk_start(trade_heard(name_of(v->other), theirs), FACE_MEGAMAN);
	else if (v->stage == TRADE_ANSWERED && unwanted(v)) {
		fprintf(stderr, "online: an answer to our board offer with %u %u, not its want: called off\n", v->theirs.id, v->theirs.code);
		C.hushed = v->id;
		online_trade_cancel();
	} else if (v->stage == TRADE_ANSWERED && C.sent) {
		/* (this game's own answer, or take: the pair shown at once) */
		C.sent = false;
		deal(v);
	} else if (v->stage == TRADE_ANSWERED) {
		/* (an answer to MegaMan's offer, or his board offer taken: his word, then the pair) */
		bool board = v->taken && v->other != C.took;
		said = talk_start(board ? trade_board_taken(name_of(v->other)) : trade_answered(name_of(v->other)), FACE_MEGAMAN);
		if (said) C.deal = v->id;
	} else if (v->stage == TRADE_DONE) said = talk_start(trade_done(theirs), FACE_MEGAMAN);
	else if (v->stage == TRADE_CANCELLED) {
		C.took = 0;
		C.sent = false;
		if (v->id != C.hushed) said = talk_start(trade_off(), FACE_MEGAMAN);
	}
	if (said) C.said = v->serial;
}

/* the server's word on the trade, wherever MegaMan stands: one done kept
 * on disk at once (the run takes it at home, or at CONTINUE) and its
 * cards swapped; the chip trade closed for one called off */
static void settled(void) {
	const TradeView *v = online_trade_view();
	if (v->stage == TRADE_CANCELLED && trade_screen_dealing()) trade_screen_off();
	if (v->stage != TRADE_DONE || v->id == C.kept) return;
	C.kept = v->id;
	snprintf(last_partner, sizeof last_partner, "%s", name_of(v->other));
	if (!online_trade_keep(v)) fprintf(stderr, "online: the trade could not be kept; it is lost\n");
	analytics_trade(v->taken);
	C.traded = true;
	C.took = 0;
	trade_screen_swap();
}

/* what was chosen on Trade Central's screens */
static void screens(void) {
	TradeChip c;
	int e = trade_screen_event(&c);
	if (e == TRADE_EVENT_PICKED) picked(c);
	else if (e == TRADE_EVENT_DEAL && !online_trade_accept()) trade_screen_off();
	else if (e == TRADE_EVENT_DECLINE) online_trade_cancel();
}

/* the flags of the chat just closed, acted on */
static void chosen(void) {
	if (flag_get(NO_FLAG) || flag_get(OFF_FLAG)) online_trade_cancel();
	if (flag_get(OFFER_FLAG)) C.choosing = true;
	if (flag_get(BOARD_FLAG)) C.boarding = true;
	for (int k = 0; k < SQUARE_PEERS; ++k)
		if (flag_get(TO_FLAG + k) && k < C.nwho) {
			C.partner = C.who[k];
			snprintf(C.partner_name, sizeof C.partner_name, "%s", name_of(C.who[k]));
			C.mode = TRADE_FOR_OFFER;
			pick(C.partner, -1);
		}
	for (int k = 0; k < SQUARE_PEERS; ++k) if (flag_get(TAKE_FLAG + k) && k < C.nshown) take(C.shown[k]);
	if (flag_get(POST_FLAG)) { C.mode = TRADE_FOR_POST; pick(0, -1); }
	if (flag_get(DOWN_FLAG) && online_board_clear()) talk_start(trade_post_down(), FACE_PROG);
	if (flag_get(BACK_FLAG)) {
		C.mode = TRADE_FOR_ANSWER;
		C.partner = online_trade_view()->other;
		snprintf(C.partner_name, sizeof C.partner_name, "%s", name_of(C.partner));
		pick(C.partner, -1);
	}
	clear_flags();
}

void trade_central_frame(const OnlineSnapshot *s) {
	C.s = *s;
	/* (on the week's dive, from the moment it starts: nothing trades) */
	if (weekly_on()) {
		if (!C.closed) C.closed = online_board_clear();
		uint32_t id = online_trade_view()->id;
		if (id != C.hushed && online_trade_cancel()) C.hushed = id;
	}
	clerk();
	settled();
	screens();
	if (talk_busy() || emu_read8(BN6_CHATBOX) || trade_screen_shown()) return;
	if (C.wanting) { wanted(); return; }
	chosen();
	if (C.choosing) { C.choosing = false; who(); return; }
	if (C.boarding) { C.boarding = false; board(); return; }
	news();
}

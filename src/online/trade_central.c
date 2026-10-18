/* trade_central.h. Trade Central in the Net Square (docs/ONLINE.md): BN6's
 * reception desk in the Square's Trade wing, its clerk's words in BN6's
 * chat boxes. Offered from his desk: the Navi to trade with, a page of two
 * names at a time, then the chip on BN6's own Order Service screen, which
 * lists MegaMan's spare chips at no price (the Square's RAM is thrown away
 * on the way home; the chip ordered is the one whose stock fell). An offer
 * that comes in is MegaMan's news wherever he stands, answered at the
 * desk the same way; once both chips are known MegaMan asks "Deal,Lan?",
 * the cursor on No; the server commits when both said yes, and the swap
 * is kept on disk until the run has it at home (online_trade.c). */
#include "trade_central.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "chips.h"
#include "emu.h"
#include "flags.h"
#include "online_square.h"
#include "online_trade.h"
#include "script_kit.h"
#include "shop.h"
#include "talk.h"
#include "trade_words.h"

/* Trade Central's event flags (the Square's own map: its RAM is thrown away) */
#define OFFER_FLAG 0x1494   /* the clerk's "Offer a chip" */
#define BACK_FLAG  0x1495   /* "Offer one back" */
#define NO_FLAG    0x1496   /* "No thanks" to an offer */
#define OFF_FLAG   0x1497   /* "Call it off" */
#define DEAL_FLAG  0x1498   /* the review's Yes */
#define TO_FLAG    0x1499   /* + k: the Navi picked, 0 to SQUARE_PEERS - 1 */
#define LAST_FLAG  (TO_FLAG + SQUARE_PEERS - 1)
#define SPARES 320

static struct {
	OnlineSnapshot s;
	uint32_t said;              /* the view's serial MegaMan has spoken of */
	uint32_t clerk;             /* the clerk's words as of (stage, peers) */
	bool picking, answering, reviewing, choosing;
	int chosen;                 /* the chip picked on the order screen, -1 none yet */
	uint32_t partner;
	char partner_name[ONLINE_NAME_BYTES + 1];
	TradeChip listed[SPARES];
	uint8_t stock[SPARES];
	int nlisted;
} C;

void trade_central_reset(void) {
	memset(&C, 0, sizeof C);
	online_trade_reset();
	for (int f = OFFER_FLAG; f <= LAST_FLAG; ++f) flag_clear(f);
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
 * to answer, or the other player to wait on */
static void clerk(void) {
	const TradeView *v = online_trade_view();
	uint32_t now = (uint32_t)v->stage << 24 | v->serial << 8 | (uint32_t)C.s.peer_count;
	if (now == C.clerk || emu_read8(BN6_CHATBOX)) return;
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	char chip[32];
	const char *const *options = trade_options;
	int flags[2] = { OFFER_FLAG, -1 };
	ta_begin(&t);
	ta_script(&t);
	bool first = true;
	if (v->stage == TRADE_OFFERED) {
		chip_words(v->theirs, chip, sizeof chip);
		ta_pages(&t, trade_offered(name_of(v->other), chip), FACE_PROG, &first);
		options = trade_offer_options;
		flags[0] = BACK_FLAG; flags[1] = NO_FLAG;
	} else if (v->stage == TRADE_WAITING || v->stage == TRADE_ACCEPTED || v->stage == TRADE_ANSWERED) {
		ta_pages(&t, trade_waiting(name_of(v->other)), FACE_PROG, &first);
		options = trade_wait_options;
		flags[0] = -1; flags[1] = OFF_FLAG;
	} else ta_pages(&t, trade_hello(), FACE_PROG, &first);
	ta_mugshot(&t, FACE_PROG);
	ta_clear(&t);
	ta_options(&t, options, 2);
	int to[2] = { 1, 2 };
	ta_choose(&t, to, 2, 3);
	for (int k = 0; k < 2; ++k) {
		ta_script(&t);
		if (flags[k] >= 0) ta_flag_set(&t, flags[k]);
		/* ("How it works": his three boxes) */
		else if (options == trade_options) { bool f = true; ta_pages(&t, trade_how(), FACE_PROG, &f); }
		ta_end(&t);
	}
	ta_say(&t, FACE_PROG, trade_bye());
	if (online_square_trade_say(out, ta_build(&t, out))) C.clerk = now;
}

/* whom to trade with: the channel's Navis, two a page, More going on */
static void who(void) {
	if (!C.s.peer_count) { talk_start(trade_none_here(), FACE_PROG); return; }
	static TextArchive t;
	int n = (int)C.s.peer_count, pages = (n + 1) / 2, pick = pages, end = pick + n;
	ta_begin(&t);
	for (int p = 0; p < pages; ++p) {
		ta_script(&t);
		if (!p) ta_page(&t, FACE_PROG, trade_who(), true);
		ta_mugshot(&t, FACE_PROG);
		ta_clear(&t);
		const char *labels[3];
		int to[3], k = 0;
		for (int i = 2 * p; i < n && i < 2 * p + 2; ++i) { labels[k] = C.s.peers[i].name; to[k++] = pick + i; }
		if (pages > 1) { labels[k] = p + 1 < pages ? "More..." : "Back"; to[k++] = p + 1 < pages ? p + 1 : 0; }
		ta_options(&t, labels, k);
		ta_choose(&t, to, k, end);
	}
	for (int i = 0; i < n; ++i) { ta_script(&t); ta_flag_set(&t, TO_FLAG + i); ta_end(&t); }
	ta_closing(&t);
	run(&t);
}

/* the chip: BN6's order screen, MegaMan's spares listed at no price */
static void pick(void) {
	uint8_t count[SPARES];
	C.nlisted = online_trade_spares(C.listed, count, SPARES);
	ShopItem items[SPARES];
	/* (one each: a chip picked shows sold out after) */
	for (int i = 0; i < C.nlisted; ++i) items[i] = (ShopItem){ 2, 1, C.listed[i].id, C.listed[i].code, 0 };
	if (!C.nlisted || !shop_order_list(items, C.nlisted)) { talk_start(trade_no_spares(), FACE_PROG); return; }
	for (int i = 0; i < C.nlisted; ++i) C.stock[i] = 1;
	C.chosen = -1;
	static TextArchive t;
	ta_begin(&t);
	ta_counter(&t, SHOP_ORDER, FACE_PROG, trade_pick(), -1, NULL);
	C.picking = run(&t);
}

/* while the order screen is open: the first chip whose stock fell (more
 * picked after it are not offered) */
static void watch(void) {
	for (int i = 0; C.chosen < 0 && i < C.nlisted; ++i)
		if (shop_order_stock(i) >= 0 && shop_order_stock(i) < C.stock[i]) C.chosen = i;
}

/* the order screen closed: that chip, sent */
static void picked(void) {
	C.picking = false;
	if (C.chosen < 0) return;
	TradeChip c = C.listed[C.chosen];
	if (C.answering) online_trade_answer(c);
	else if (online_trade_propose(C.partner, c)) talk_start(trade_sent(C.partner_name), FACE_PROG);
}

/* what MegaMan says as the server's word on the trade comes */
static void news(void) {
	const TradeView *v = online_trade_view();
	if (v->serial == C.said) return;
	char mine[32], theirs[32];
	chip_words(v->mine, mine, sizeof mine);
	chip_words(v->theirs, theirs, sizeof theirs);
	bool said = true;
	if (v->stage == TRADE_OFFERED) said = talk_start(trade_heard(name_of(v->other), theirs), FACE_MEGAMAN);
	else if (v->stage == TRADE_ANSWERED) {
		flag_clear(DEAL_FLAG);
		said = C.reviewing = talk_ask(trade_review(name_of(v->other), mine, theirs), FACE_MEGAMAN, trade_deal_question(), DEAL_FLAG);
	} else if (v->stage == TRADE_DONE) {
		/* (kept on disk first: the run takes it at home, or at CONTINUE) */
		if (!online_trade_keep(v)) fprintf(stderr, "online: the trade could not be kept; it is lost\n");
		said = talk_start(trade_done(theirs), FACE_MEGAMAN);
	} else if (v->stage == TRADE_CANCELLED) said = talk_start(trade_off(), FACE_MEGAMAN);
	if (said) C.said = v->serial;
}

/* the flags of the chat just closed, acted on */
static void chosen(void) {
	if (C.reviewing) {
		C.reviewing = false;
		if (flag_get(DEAL_FLAG)) online_trade_accept();
		else online_trade_cancel();
	}
	if (flag_get(NO_FLAG) || flag_get(OFF_FLAG)) online_trade_cancel();
	if (flag_get(OFFER_FLAG)) C.choosing = true;
	for (int k = 0; k < SQUARE_PEERS; ++k)
		if (flag_get(TO_FLAG + k) && (size_t)k < C.s.peer_count) {
			C.partner = C.s.peers[k].id;
			snprintf(C.partner_name, sizeof C.partner_name, "%s", C.s.peers[k].name);
			C.answering = false;
			pick();
		}
	if (flag_get(BACK_FLAG)) { C.answering = true; pick(); }
	for (int f = OFFER_FLAG; f <= LAST_FLAG; ++f) flag_clear(f);
}

void trade_central_frame(const OnlineSnapshot *s) {
	C.s = *s;
	clerk();
	if (C.picking) watch();
	bool shop = emu_read8(emu_read32(BN6_TOOLKIT)) == BN6_MODE_SHOP;
	if (talk_busy() || emu_read8(BN6_CHATBOX) || shop) return;
	if (C.picking) { picked(); return; }
	chosen();
	if (C.choosing) { C.choosing = false; who(); return; }
	news();
}

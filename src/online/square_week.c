#include "square_week.h"

#include <stdio.h>
#include <string.h>

#include "analytics_online.h"
#include "netpass.h"
#include "online_square.h"
#include "online_text.h"
#include "online_trade.h"
#include "online_week.h"
#include "script_kit.h"
#include "text.h"
#include "weekly.h"

/* NOTICE codes the server answers an implausible post with (week.py) */
#define NOTICE_FIRST 23
#define NOTICE_LAST 26

static struct {
	OnlineSnapshot s;
	bool sent;              /* the kept end posted this visit */
	uint32_t sent_board, sent_notice;
	int posted;             /* its layer, once the server took it (0 none) */
	int choice;             /* the greeter's: 0 none, else 1 + square_week_choose's */
	uint16_t week;          /* ... and the week's net it chose */
	uint32_t seed;
} Q;

void square_week_reset(void) {
	memset(&Q, 0, sizeof Q);
	online_week_reset();
}

/* the kept end posted once the server has said its week; forgotten when
 * the server answers it, or at once where its week is past taking */
static void post(void) {
	const WeeklyRecord *r = weekly_record();
	/* (kept till a visit with the DeepNet stamp: the server refuses the rest, NOTICE 28) */
	if (!r->pending || !Q.s.week || !netpass_has(STAMP_DEEPNET)) return;
	if (!Q.sent) {
		if ((unsigned)r->r_week + 1u < Q.s.week || r->r_week > Q.s.week) { weekly_posted(); return; }
		const WeekBoard *b = online_week_board();
		Q.sent_board = b->serial;
		Q.sent_notice = Q.s.notice_serial;
		Q.sent = online_week_post(r->r_week, r->r_seed, r->r_depth, r->r_act, r->r_won, r->r_minutes, r->r_hash);
		return;
	}
	const WeekBoard *b = online_week_board();
	bool refused = Q.s.notice_serial != Q.sent_notice && Q.s.notice_code >= NOTICE_FIRST && Q.s.notice_code <= NOTICE_LAST;
	if (b->serial != Q.sent_board || refused) {
		/* (the greeter names it where it is the Board's now: a shallower
		 * dive than the week's best leaves the best up) */
		if (!refused && b->mine.depth == r->r_depth && b->week == r->r_week) Q.posted = r->r_depth;
		if (!refused) analytics_dive_post(r->r_depth, r->r_act, r->r_won);
		fprintf(stderr, "online: the week's dive to layer %u %s\n", r->r_depth, refused ? "refused" : "posted");
		weekly_posted();
	}
}

void square_week_frame(const OnlineSnapshot *s) {
	Q.s = *s;
	post();
}

WeekOffer square_week_offer(void) {
	uint16_t week = Q.s.week;
	if (!week || !Q.s.week_seed) return WEEK_OFFER_NONE;
	if (weekly_on()) return WEEK_OFFER_ON;
	if (!netpass_has(STAMP_DEEPNET)) return WEEK_OFFER_STAMP;
	TradeChip give, get;
	if (online_trade_pending(&give, &get)) return WEEK_OFFER_TRADE;
	const WeeklyRecord *r = weekly_record();
	/* (an unfinished dive of this week; one of an older week's net gives
	 * way to this week's, whose dive begins over it) */
	if (r->running && r->week == week) return WEEK_OFFER_AGAIN;
	return WEEK_OFFER_NEW;
}

const char *square_week_board_words(void) { return Q.s.week ? square_week_board(online_week_board(), Q.s.week) : ""; }

/* the greeter's words on the week's net (pages; his question or options
 * after them) */
static const char *square_week_words(void) { return square_week_post(square_week_offer(), Q.s.week, Q.posted); }

unsigned square_week_said(void) { return (unsigned)square_week_offer() | (unsigned)Q.posted << 4 | (unsigned)Q.s.week << 12; }

void square_week_choose(bool over) {
	/* (1 a new dive, 2 the unfinished one kept on) */
	Q.choice = !over && square_week_offer() == WEEK_OFFER_AGAIN ? 2 : 1;
	Q.week = Q.s.week;
	Q.seed = Q.s.week_seed;
}

bool square_week_leaving(void) { return Q.choice != 0; }

bool square_week_dive(void) {
	int choice = Q.choice;
	Q.choice = 0;
	if (choice == 2) return weekly_continue();
	return choice && weekly_dive(Q.week, Q.seed);
}

/* a new dive: Yes or No, the cursor on No (script 1 the No's) */
static void ask(TextArchive *t, const char *words, WeekOffer offer) {
	ta_script(t);
	bool first = true;
	ta_pages(t, words, FACE_PROG, &first);
	ta_ask_in(t, FACE_PROG, square_week_question(offer), 1, !first, true);
	ta_flag_set(t, SQUARE_DIVE_FLAG);
	bool more = false;
	ta_pages(t, square_week_off(), FACE_PROG, &more);
	ta_end(t);
	ta_closing(t);
}

/* the unfinished dive: kept on, begun again, or not now (B too) */
static void options(TextArchive *t, const char *words) {
	ta_script(t);
	bool first = true;
	ta_pages(t, words, FACE_PROG, &first);
	ta_mugshot(t, FACE_PROG);
	ta_clear(t);
	ta_options(t, square_week_again_options, WEEK_AGAIN_OPTIONS);
	int to[WEEK_AGAIN_OPTIONS] = { 1, 2, 3 };
	ta_choose(t, to, WEEK_AGAIN_OPTIONS, 3);
	for (int k = 0; k < 2; ++k) {
		ta_script(t);
		ta_flag_set(t, SQUARE_DIVE_FLAG);
		if (k) ta_flag_set(t, SQUARE_DIVE_OVER_FLAG);
		bool f = true;
		ta_pages(t, square_week_off(), FACE_PROG, &f);
		ta_end(t);
	}
	ta_closing(t);
}

bool square_week_greeter(const char *welcome) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	static char words[640];
	WeekOffer offer = square_week_offer();
	const char *week = square_week_words();
	snprintf(words, sizeof words, "%s%s%s", welcome, *week ? "|" : "", week);
	ta_begin(&t);
	if (offer == WEEK_OFFER_NEW) ask(&t, words, offer);
	else if (offer == WEEK_OFFER_AGAIN) options(&t, words);
	else ta_say(&t, FACE_PROG, words);
	return online_square_prog_say(SQUARE_PROG_GREETER, out, ta_build(&t, out));
}

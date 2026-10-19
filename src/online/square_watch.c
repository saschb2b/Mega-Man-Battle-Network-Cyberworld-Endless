/* The stands in the Net Square (square_watch.h).
 *
 * The referee's archive is written again as the server's report changes
 * (never under an open chat box: square_life.c). His question sets the
 * Square's flag WATCH_FLAG; as the chat closes the game asks the server
 * to watch the battle he named. The server answers WATCH_STATE with the
 * battle's sides, rules and delay, then hands on both MegaMen, the
 * players' ready and hashes, and every key from the first frame; the
 * pair is built and run here (arena_watch.c). A battle this machine was
 * too slow to catch up with is not offered again: the next one is. */
#include "square_watch.h"

#include <stdio.h>
#include <string.h>

#include <SDL.h>

#include "arena_format.h"
#include "arena_pair.h"
#include "arena_view.h"
#include "arena_watch.h"
#include "bn6.h"
#include "chips.h"
#include "emu.h"
#include "flags.h"
#include "gfx.h"
#include "online_mail.h"
#include "online_square.h"
#include "online_stands.h"
#include "online_transport.h"
#include "platform.h"
#include "rom.h"
#include "script_kit.h"
#include "stands_text.h"
#include "stands_words.h"
#include "talk.h"

#define WATCH_FLAG    0x14C4   /* the referee's Yes to watching: the Square's own (docs/EMULATION.md) */
#define ASK_WAIT_MS   5000     /* the server's answer to WATCH */
#define OVER_TICKS    300      /* the end shown before the Square */
#define OVER_HELD     30       /* ... at least, before A or B goes back */
#define KEYS_SHOWN    240      /* the watching screen's keys shown so long */
#define EMOTE_CHEER   1        /* the Square's Cheer emote (online_text.c's online_emotes) */

typedef enum { W_IDLE, W_ASKED, W_ON, W_DONE } Stage;

static struct {
	Stage stage;
	uint32_t match, since, notice_serial;
	uint32_t refused;          /* a battle this machine could not keep up with: not offered again */
	char name[2][ONLINE_NAME_BYTES + 1];
	unsigned rules, view;
	ArenaWatch *w;
	OnlineSnapshot s;
	int over_t, shown_t;
	bool over_said;            /* WATCH_STATE's over come: no more keys */
	char why[160];
} W;

static void drop(void) {
	arena_watch_free(W.w);
	W.w = NULL;
}

void square_watch_reset(void) {
	drop();
	memset(&W, 0, sizeof W);
	online_stands_reset();
}

static void send_watch(unsigned action, uint32_t match) {
	uint8_t m[ONLINE_WATCH_BYTES];
	if (stands_wire_watch(m, action, match)) online_transport_send(m, sizeof m);
}

void square_watch_end(void) {
	if (W.stage == W_ASKED || W.stage == W_ON) send_watch(WATCH_ACT_STOP, W.match);
	square_watch_reset();
}

/* the battle on in the channel, and the last one over: NULL none */
static const StandsEntry *entry(bool on) {
	const StandsEntry *e;
	int n = online_stands_entries(&e);
	for (int k = n - 1; k >= 0; --k) {
		bool live = e[k].stage == STANDS_WAY_IN || e[k].stage == STANDS_FIGHTING;
		if (live == on) return &e[k];
	}
	return NULL;
}

static bool mine(const StandsEntry *e) { return e->side[0] == W.s.player_id || e->side[1] == W.s.player_id; }

/* this game can watch it: the Arena built here, the rules its ROMs fight by, not its own battle */
static bool can_watch(const StandsEntry *e) {
	return CW_ARENA && e && !mine(e) && e->match != W.refused && W.stage == W_IDLE && arena_format_ok(e->rules, online_transport_roms());
}

/* a chip's name where this game can name it (a BN5 chip only with BN5's ROM), else NULL */
static const char *chip_name(unsigned id, char *out, size_t n) {
	bool bn5 = id > ONLINE_BN6_CHIP_LAST || BN6_CHIP_BLANK(id);
	if (!id || (bn5 && !(online_transport_roms() & ONLINE_ROMS_BN5))) return NULL;
	chips_name((int)id, out, n);
	return out[0] ? out : NULL;
}

static void bout_of(const StandsEntry *e, StandsBout *b, char chips[2][24]) {
	memset(b, 0, sizeof *b);
	b->on = e->stage == STANDS_WAY_IN || e->stage == STANDS_FIGHTING;
	b->report = e->report.max_hp[0] && e->report.max_hp[1];
	for (int side = 0; side < 2; ++side) {
		b->name[side] = e->name[side];
		b->hp[side] = e->report.hp[side];
		b->chip[side] = chip_name(e->report.chip[side], chips[side], sizeof chips[side]);
	}
	b->watchers = e->watchers;
	b->cheers = e->cheers;
	b->winner = e->stage == STANDS_OVER ? e->report.winner : 0;
	b->mine = mine(e);
}

/* the stands' words for the referee or the Board */
static const char *words_of(const char *(*say)(const StandsBout *, const StandsBout *)) {
	const StandsEntry *live = entry(true), *last = entry(false);
	StandsBout a, z;
	static char chips[2][2][24];
	if (live) bout_of(live, &a, chips[0]);
	if (last) bout_of(last, &z, chips[1]);
	return say(live ? &a : NULL, last ? &z : NULL);
}

uint32_t square_watch_said(void) {
	return online_stands_serial() * 2654435761u ^ (uint32_t)can_watch(entry(true)) ^ W.refused << 1;
}

bool square_watch_referee(const char *words) {
	static TextArchive t;
	static uint8_t out[TEXT_ARCHIVE_MAX];
	static char all[720];
	snprintf(all, sizeof all, "%s%s", words, words_of(stands_referee_words));
	ta_begin(&t);
	if (can_watch(entry(true))) {
		/* (Yes or No, the cursor on Yes: script 1 the No's) */
		ta_script(&t);
		bool first = true;
		ta_pages(&t, all, FACE_PROG, &first);
		ta_ask_in(&t, FACE_PROG, stands_watch_question(), 1, !first, false);
		ta_flag_set(&t, WATCH_FLAG);
		bool more = false;
		ta_pages(&t, stands_watch_off(), FACE_PROG, &more);
		ta_end(&t);
		ta_closing(&t);
	} else ta_say(&t, FACE_PROG, all);
	return online_square_prog_say(SQUARE_PROG_REFEREE, out, ta_build(&t, out));
}

const char *square_watch_board(void) { return words_of(stands_board_words); }

/* the referee's Yes: the server asked for the battle he named */
static void ask(void) {
	const StandsEntry *e = entry(true);
	if (!can_watch(e)) return;
	W.match = e->match;
	W.rules = e->rules;
	for (int side = 0; side < 2; ++side) snprintf(W.name[side], sizeof W.name[side], "%s", e->name[side]);
	W.stage = W_ASKED;
	W.since = SDL_GetTicks();
	W.notice_serial = W.s.notice_serial;
	send_watch(WATCH_ACT_WATCH, W.match);
	printf("watch: asked to watch %s vs %s (match %u, rules %u)\n", W.name[0], W.name[1], W.match, W.rules);
}

void square_watch_update(const OnlineSnapshot *s) {
	W.s = *s;
	bool open = talk_busy() || emu_read8(BN6_CHATBOX);
	if (W.stage == W_IDLE && !open && flag_get(WATCH_FLAG)) {
		flag_clear(WATCH_FLAG);
		ask();
	}
	/* (refused with a notice: the battle over, the stands full) */
	if (W.stage == W_ASKED && (s->notice_serial != W.notice_serial || SDL_GetTicks() - W.since > ASK_WAIT_MS)) W.stage = W_IDLE;
}

static void started(const WatchState *st) {
	if (W.stage != W_ASKED || st->match != W.match) return;
	W.w = arena_watch_new(R.data, ROM_SIZE, st->rules, st->match);
	if (!W.w) { send_watch(WATCH_ACT_STOP, W.match); W.stage = W_IDLE; return; }
	W.stage = W_ON;
	W.view = 0;
	W.shown_t = 0;
	W.over_said = false;
	W.why[0] = 0;
	printf("watch: watching match %u, sides %u and %u, rules %u, delay %u\n", st->match, st->side[0], st->side[1], st->rules, st->delay);
}

static void relayed(const uint8_t *data, size_t size) {
	unsigned side;
	const uint8_t *inner;
	size_t n;
	if (W.w && stands_wire_relay(data, size, &side, &inner, &n)) arena_watch_relayed(W.w, side, inner, n);
}

void square_watch_mail(const uint8_t *data, size_t size) {
	WatchState st;
	uint32_t match, first;
	unsigned side, count;
	const uint8_t *keys;
	switch (data[0]) {
	case ONLINE_STANDS:
		if (!online_stands_mail(data, size)) fprintf(stderr, "online: a malformed STANDS set aside\n");
		break;
	case ONLINE_WATCH_STATE:
		if (!stands_wire_state(data, size, &st)) break;
		if (st.stage == WATCH_ON) started(&st);
		else if (W.w && st.match == W.match) { arena_watch_ended(W.w); W.over_said = true; }
		break;
	case ONLINE_WATCH_RELAY: relayed(data, size); break;
	case ONLINE_WATCH_KEYS:
		if (W.w && stands_wire_keys(data, size, &match, &side, &first, &keys, &count) && match == W.match)
			arena_watch_keys(W.w, side, first, keys, count);
		break;
	default: break;
	}
}

bool square_watch_on(void) { return W.stage == W_ON || W.stage == W_DONE; }

static void done(const char *why) {
	snprintf(W.why, sizeof W.why, "%s", why ? why : "");
	if (!W.over_said) send_watch(WATCH_ACT_STOP, W.match);
	W.stage = W_DONE;
	W.over_t = 0;
}

static void back(void) {
	drop();
	arena_view_free();
	W.stage = W_IDLE;
	printf("watch: back in the Square\n");
}

static void watching(void) {
	if (btn_pressed(BTN_B)) { done(NULL); back(); return; }
	if (btn_pressed(BTN_R)) W.view ^= 1u;
	if (btn_pressed(BTN_L)) online_transport_social(1, EMOTE_CHEER);
	WatchingStage stage = arena_watch_tick(W.w, P.headless);
	arena_watch_sound(W.w, W.view);
	if (arena_watch_video(W.w, W.view)) ++W.shown_t;
	if (stage == WATCHING_OVER) done(NULL);
	else if (stage == WATCHING_FAILED) {
		const char *why = arena_watch_error(W.w);
		if (strstr(why, "too slow")) W.refused = W.match;
		done(why);
	}
}

void square_watch_frame(uint32_t keys) {
	(void)keys;
	online_mail_frame();
	OnlineSnapshot s;
	online_transport_snapshot(&s);
	if ((s.error[0] || !s.connected) && W.stage == W_ON) done("the connection to the Square was lost");
	if (W.stage == W_ON) watching();
	else if (W.stage == W_DONE && (++W.over_t >= OVER_TICKS || (W.over_t >= OVER_HELD && (btn_pressed(BTN_A) || btn_pressed(BTN_B)))))
		back();
}

/* the way in's picture: the heading, the words and a bar */
static void setting_up(int progress) {
	int cx = P.w / 2, y = P.core_y + 52, w = 160;
	text_draw(cx, y, stands_heading(W.name[0], W.name[1]), rgba(170, 200, 255, 255), TEXT_CENTER);
	text_draw(cx, y + 16, stands_setting_up, WHITE, TEXT_CENTER);
	fill_rect(cx - w / 2, y + 36, w, 6, rgba(40, 60, 90, 255));
	fill_rect(cx - w / 2, y + 36, w * progress / 100, 6, rgba(120, 200, 255, 255));
}

void square_watch_draw(void) {
	arena_view_clear();
	const uint32_t *v = W.w ? arena_watch_video(W.w, W.view) : NULL;
	if (v) arena_view_core(v);
	if (W.stage == W_DONE) {
		unsigned result = W.w ? arena_watch_result(W.w) : 0;
		if (W.why[0]) arena_view_note(stands_stopped, W.why);
		else if (result == 1 || result == 2) arena_view_note(stands_won(W.name[result - 1]), stands_back);
		else arena_view_note(stands_over, stands_back);
		return;
	}
	if (!v) { setting_up(W.w ? arena_watch_progress(W.w) : 0); return; }
	if (arena_watch_catching_up(W.w)) arena_view_note(stands_heading(W.name[0], W.name[1]), stands_catching_up);
	else if (W.shown_t < KEYS_SHOWN) arena_view_note(stands_heading(W.name[0], W.name[1]), stands_keys);
}

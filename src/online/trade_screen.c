/* trade_screen.h. */
#include "trade_screen.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "buttons.h"
#include "online_types.h"
#include "platform.h"
#include "trade_screen_draw.h"

enum { SHUT, LIST, DEAL };

static struct {
	int kind;
	bool letting_go;     /* closed, its A or B still held: kept from the game */
	int event;
	TradeChip picked;
	char partner[ONLINE_NAME_BYTES + 1];
	TradeRow rows[TRADE_LIST_MOST];
	TradeScreenView v;
} S;

/* the session's sort: the list opens in it again */
static int sort_key = TRADE_SORT_ID;

static void show(int kind, const char *partner) {
	memset(&S.v, 0, sizeof S.v);
	S.kind = kind;
	S.event = TRADE_EVENT_NONE;
	snprintf(S.partner, sizeof S.partner, "%s", partner ? partner : "");
	S.v.partner = S.partner;
	S.v.rows = S.rows;
	S.v.want = -1;
	audio_sfx(SFX_DESC_OPEN);
}

static void close_with(int event, int sfx) {
	S.kind = SHUT;
	S.event = event;
	S.letting_go = true;
	audio_sfx(sfx);
}

void trade_screen_list(const TradeRow *rows, int n, int purpose, const char *partner, int want, TradeChip theirs) {
	show(LIST, partner);
	n = n < 0 ? 0 : n > TRADE_LIST_MOST ? TRADE_LIST_MOST : n;
	memcpy(S.rows, rows, (size_t)n * sizeof *rows);
	trade_list_sort(S.rows, n, sort_key);
	S.v.n = n;
	S.v.sort = sort_key;
	S.v.purpose = purpose;
	S.v.want = want;
	S.v.theirs = theirs;
}

void trade_screen_deal(TradeChip mine, TradeChip theirs, const char *partner) {
	show(DEAL, partner);
	S.v.mine = mine;
	S.v.get = theirs;
	S.v.stage = TRADE_ASK;
}

void trade_screen_swap(void) {
	if (S.kind != DEAL || S.v.stage >= TRADE_SWAP) return;
	S.v.stage = TRADE_SWAP;
	S.v.swap_t = 0;
	audio_sfx(SFX_CHIP_SELECT);
}

void trade_screen_off(void) {
	if (S.kind == SHUT) return;
	S.kind = SHUT;
	audio_sfx(SFX_DESC_CLOSE);
}

int trade_screen_event(TradeChip *chip) {
	int e = S.event;
	S.event = TRADE_EVENT_NONE;
	if (e == TRADE_EVENT_PICKED && chip) *chip = S.picked;
	return e;
}

bool trade_screen_shown(void) { return S.kind != SHUT; }

bool trade_screen_dealing(void) { return S.kind == DEAL; }

/* the list's cursor `by` rows on, round at its ends, the rows shown following it */
static void step(int by) {
	int n = S.v.n;
	if (n <= 0) return;
	int at = S.v.at + by;
	if (by == 1 || by == -1) at = (at + n) % n;
	else at = at < 0 ? 0 : at >= n ? n - 1 : at;
	if (at == S.v.at) return;
	S.v.at = at;
	if (at < S.v.top) S.v.top = at;
	if (at >= S.v.top + TRADE_ROWS) S.v.top = at - TRADE_ROWS + 1;
	audio_sfx(SFX_CHIP_CURSOR);
}

/* the next sort, the cursor kept on its row's chip */
static void sort(void) {
	TradeRow was = S.v.n ? S.rows[S.v.at] : (TradeRow){ 0 };
	sort_key = S.v.sort = trade_list_next_key(S.v.sort);
	trade_list_sort(S.rows, S.v.n, sort_key);
	for (int i = 0; i < S.v.n; ++i)
		if (S.rows[i].chip.id == was.chip.id && S.rows[i].chip.code == was.chip.code) S.v.at = i;
	S.v.top = S.v.at - TRADE_ROWS / 2;
	if (S.v.top > S.v.n - TRADE_ROWS) S.v.top = S.v.n - TRADE_ROWS;
	if (S.v.top < 0) S.v.top = 0;
	audio_sfx(SFX_SELECT);
}

static void list_frame(void) {
	if (btn_repeat(BTN_UP)) step(-1);
	else if (btn_repeat(BTN_DOWN)) step(1);
	else if (btn_repeat(BTN_LEFT)) step(-TRADE_ROWS);
	else if (btn_repeat(BTN_RIGHT)) step(TRADE_ROWS);
	if (btn_pressed(BTN_START) && S.v.n > 1) sort();
	if (btn_pressed(BTN_A) && S.v.n) {
		S.picked = S.rows[S.v.at].chip;
		close_with(TRADE_EVENT_PICKED, SFX_CHIP_OK);
	} else if (btn_pressed(BTN_B)) close_with(TRADE_EVENT_BACK, SFX_CANCEL);
}

static void deal_frame(void) {
	TradeScreenView *v = &S.v;
	if (v->stage == TRADE_ASK) {
		if (btn_pressed(BTN_LEFT | BTN_RIGHT)) { v->cursor ^= 1; audio_sfx(SFX_CURSOR); }
		if (btn_pressed(BTN_B) || (btn_pressed(BTN_A) && !v->cursor)) close_with(TRADE_EVENT_DECLINE, SFX_CANCEL);
		else if (btn_pressed(BTN_A)) {
			v->stage = TRADE_WAIT;
			S.event = TRADE_EVENT_DEAL;
			audio_sfx(SFX_CHIP_OK);
		}
	} else if (v->stage == TRADE_WAIT) {
		if (btn_pressed(BTN_B)) close_with(TRADE_EVENT_DECLINE, SFX_CANCEL);
	} else if (v->stage == TRADE_SWAP) {
		/* (the cards cross, then land with BN6's jingle, as a stamp of the NetPass's lands) */
		if (++v->swap_t >= TRADE_SWAP_FRAMES) {
			v->stage = TRADE_SWAPPED;
			audio_sfx(SFX_GOT);
		}
	} else if (btn_pressed(BTN_A | BTN_B)) {
		S.kind = SHUT;
		S.letting_go = true;
		audio_sfx(SFX_DESC_CLOSE);
	}
}

bool trade_screen_update(void) {
	if (S.kind == SHUT) {
		if (S.letting_go && btn_held(BTN_A | BTN_B)) return true;
		S.letting_go = false;
		return false;
	}
	++S.v.t;
	/* (its first frame's keys are the ones that opened it) */
	if (S.v.t > 1) {
		if (S.kind == LIST) list_frame();
		else deal_frame();
	}
	return true;
}

void trade_screen_draw(void) {
	if (S.kind == LIST) trade_screen_draw_list(P.core_x, P.core_y, &S.v);
	else if (S.kind == DEAL) trade_screen_draw_deal(P.core_x, P.core_y, &S.v);
}

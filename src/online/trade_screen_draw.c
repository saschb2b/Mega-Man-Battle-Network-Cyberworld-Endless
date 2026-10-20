/* trade_screen_draw.h. */
#include "trade_screen_draw.h"

#include <stdio.h>

#include "chips.h"
#include "data.h"
#include "minifont.h"
#include "netpass_draw.h"
#include "second_art.h"
#include "second_frame.h"
#include "trade_screen_text.h"

#define W 240
#define HEAD_H 18
#define INK      rgba(0, 49, 74, 255)
#define TRAY     rgba(8, 41, 62, 255)       /* the NetPass's tray */
#define ROW_LINE rgba(10, 58, 80, 255)
#define ROW_LIT  rgba(24, 132, 160, 255)
#define GOLD_EDGE rgba(198, 90, 0, 255)

/* the spares list: the card on the left, the list on the right, the bar under them */
#define CARD_X 4
#define BODY_Y 21
#define CARD_W 106
#define BODY_H 113
#define LIST_X 114
#define LIST_W 122
#define ROW_H 16
#define ROWS_Y 14     /* the rows from the list's top */
#define BAR_Y 137
#define BAR_H 20

/* the chip trade: two cards, the arrows between */
#define DEAL_W 98
#define DEAL_H 84
#define DEAL_Y 31
#define DEAL_LEFT 8
#define DEAL_RIGHT (W - 8 - DEAL_W)
#define BUTTON_Y 121
#define BUTTON_W 78
#define BUTTON_H 17

static char code_char(uint8_t code) { return code >= 26 ? '*' : (char)('A' + code); }

/* the header: BN6's green band, the title and its stripes, `right` on a slot */
static void band(int x, int y, const char *title, const char *right) {
	fill_rect(x, y, W, HEAD_H, PET_GREEN);
	fill_rect(x, y, W, 1, PET_LINE);
	fill_rect(x, y + HEAD_H - 2, W, 2, PET_DARK);
	text_draw(x + 7, y + 3, title, PET_WHITE, TEXT_LEFT);
	second_stripes(x + 11 + text_width(title), y + 3);
	int sw = text_width(right) + 12, sx = x + W - 4 - sw;
	second_slot(sx, y + 2, sw, HEAD_H - 5);
	text_draw(sx + 6, y + 2, right, PET_CYAN_HI, TEXT_LEFT);
}

/* a key's cap with `cap` in it, then `what`: the x after them */
static int key(int x, int y, const char *cap, const char *what) {
	int cw = minifont_width(cap, 1) + 5;
	netpass_panel(x, y, cw, 9, PET_CYAN_HI, PET_CYAN);
	minifont_draw(x + 3, y + 2, cap, INK, 1);
	minifont_draw(x + cw + 3, y + 2, what, PET_WHITE, 1);
	return x + cw + 3 + minifont_width(what, 1) + 9;
}

/* the chip's picture, its card's 56 x 48, in a frame from (x, y) */
static void picture(int id, int x, int y, SDL_Color edge) {
	fill_rect(x, y, 58, 50, edge);
	fill_rect(x + 1, y + 1, 56, 48, INK);
	second_chip_art(id, x + 1, y + 1, 1);
}

/* the chip's power as its card shows it, "" none */
static void power_words(const ChipInfo *ci, char *out, size_t n) {
	if (ci->power > 0) snprintf(out, n, "%d", ci->power);
	else if (n) out[0] = 0;
}

/* ---- the spares list ---- */

/* the cursor's chip as a card: its name, picture, code, element, power and description */
static void list_card(int x, int y, TradeChip c) {
	netpass_panel(x, y, CARD_W, BODY_H, PET_SLOT, PET_CYAN);
	fill_rect(x + 2, y + 1, CARD_W - 4, 1, rgba(33, 123, 132, 255));
	if (!c.id) return;
	ChipInfo ci;
	chip_info(c.id, &ci);
	text_draw(x + 5, y + 2, ci.name, PET_WHITE, TEXT_LEFT);
	picture(c.id, x + 4, y + 16, PET_CYAN);
	int cx = x + 62 + (CARD_W - 62) / 2;
	char code[2] = { code_char(c.code), 0 }, power[12];
	text_draw_scaled(cx, y + 12, code, PET_GOLD, TEXT_CENTER, 2);
	second_element_icon(second_chip_element(c.id), cx - 8, y + 38);
	power_words(&ci, power, sizeof power);
	text_draw(cx, y + 54, power, PET_WHITE, TEXT_CENTER);
	fill_rect(x + 4, y + 69, CARD_W - 8, 1, PET_SLOT_EDGE);
	char desc[160];
	chip_desc(c.id, desc, sizeof desc);
	second_wrapped(desc, x + 5, y + 71, CARD_W - 10, 3, PET_WHITE);
}

/* a row: the chip's icon, its name, its code and the spares */
static void row(int x, int y, int w, const TradeRow *r, bool lit) {
	if (lit) {
		fill_rect(x, y, w, ROW_H, PET_CYAN_HI);
		fill_rect(x + 1, y + 1, w - 2, ROW_H - 2, ROW_LIT);
	} else fill_rect(x + 2, y + ROW_H - 1, w - 4, 1, ROW_LINE);
	second_chip_icon(r->chip.id, x + 1, y);
	char name[24], code[2] = { code_char(r->chip.code), 0 }, count[8];
	chips_name(r->chip.id, name, sizeof name);
	text_draw(x + 18, y + 2, name, PET_WHITE, TEXT_LEFT);
	text_draw(x + w - 24, y + 2, code, PET_GOLD, TEXT_CENTER);
	snprintf(count, sizeof count, "%d", r->count);
	text_draw(x + w - 3, y + 2, count, lit ? PET_WHITE : PET_CYAN_HI, TEXT_RIGHT);
}

static void list_panel(int x, int y, const TradeScreenView *v) {
	netpass_panel(x, y, LIST_W, BODY_H, TRAY, PET_SLOT_EDGE);
	minifont_draw(x + 5, y + 4, trade_text_spares, PET_DIM, 1);
	char n[8];
	snprintf(n, sizeof n, "%d", v->n);
	minifont_draw(x + 9 + minifont_width(trade_text_spares, 1), y + 4, n, PET_WHITE, 1);
	/* (the sort on its slot, a small triangle for its way: up) */
	const char *sort = trade_text_sorts[v->sort];
	int sw = minifont_width(sort, 1) + 13, sx = x + LIST_W - 4 - sw;
	second_slot(sx, y + 2, sw, 9);
	minifont_draw(sx + 3, y + 4, sort, PET_CYAN_HI, 1);
	for (int k = 0; k < 3; ++k) fill_rect(sx + sw - 7 + k, y + 5 + k, 5 - 2 * k, 1, PET_GOLD);
	bool bar = v->n > TRADE_ROWS;
	int rw = LIST_W - 6 - (bar ? 4 : 0);
	for (int i = 0; i < TRADE_ROWS && v->top + i < v->n; ++i)
		row(x + 3, y + ROWS_Y + i * ROW_H, rw, &v->rows[v->top + i], v->top + i == v->at);
	if (!bar) return;
	/* (the scroll bar: where the rows shown lie in the list) */
	int th = BODY_H - ROWS_Y - 4, tx = x + LIST_W - 6, ty = y + ROWS_Y;
	fill_rect(tx, ty, 2, th, ROW_LINE);
	int bh = th * TRADE_ROWS / v->n, by = ty + (th - bh) * v->top / (v->n - TRADE_ROWS);
	fill_rect(tx, by, 2, bh < 4 ? 4 : bh, PET_CYAN);
}

static void list_bar(int x, int y, const TradeScreenView *v) {
	netpass_panel(x, y, W - 8, BAR_H, TRAY, PET_SLOT_EDGE);
	char theirs[32] = "";
	if (v->theirs.id) {
		char name[24];
		chips_name(v->theirs.id, name, sizeof name);
		snprintf(theirs, sizeof theirs, "%s %c", name, code_char(v->theirs.code));
	}
	minifont_draw(x + 5, y + 3, trade_text_list_note(v->purpose, v->want, v->partner, theirs), PET_CYAN_HI, 1);
	int kx = key(x + 4, y + 10, "A", trade_text_key_pick);
	kx = key(kx, y + 10, "B", trade_text_key_back);
	if (v->n > 1) key(kx, y + 10, "START", trade_text_key_sort);
}

void trade_screen_draw_list(int x, int y, const TradeScreenView *v) {
	netpass_ground(x, y, W, 160);
	band(x, y, trade_text_list_title(v->purpose), trade_text_list_for(v->purpose, v->partner));
	list_card(x + CARD_X, y + BODY_Y, v->n ? v->rows[v->at].chip : (TradeChip){ 0, 0 });
	list_panel(x + LIST_X, y + BODY_Y, v);
	list_bar(x + 4, y + BAR_Y, v);
}

/* ---- the chip trade ---- */

/* a chip's card in the trade: its picture, name, code, element and power */
static void deal_card(int x, int y, TradeChip c, SDL_Color edge) {
	netpass_panel(x, y, DEAL_W, DEAL_H, PET_SLOT, edge);
	fill_rect(x + 2, y + 1, DEAL_W - 4, 1, rgba(33, 123, 132, 255));
	int cx = x + DEAL_W / 2;
	picture(c.id, cx - 29, y + 4, edge);
	ChipInfo ci;
	chip_info(c.id, &ci);
	text_draw(cx, y + 55, ci.name, PET_WHITE, TEXT_CENTER);
	char code[2] = { code_char(c.code), 0 }, power[12];
	power_words(&ci, power, sizeof power);
	/* (the code, the element's icon and the power in a row, centred) */
	int el = second_chip_element(c.id), cw = text_width(code), pw = text_width(power);
	int rw = cw + 4 + (el >= 0 ? 16 + 4 : 0) + pw, rx = cx - rw / 2;
	text_draw(rx, y + 68, code, PET_GOLD, TEXT_LEFT);
	rx += cw + 4;
	if (el >= 0) { second_element_icon(el, rx, y + 66); rx += 20; }
	text_draw(rx, y + 68, power, PET_WHITE, TEXT_LEFT);
}

/* a card's label on a tab over it */
static void tab(int x, int y, const char *label, SDL_Color c) {
	int w = minifont_width(label, 1) + 10;
	netpass_panel(x + 6, y, w, 10, c, c);
	fill_rect(x + 7, y + 8, w - 2, 2, c);
	minifont_draw(x + 11, y + 3, label, INK, 1);
}

/* an arrow 22 wide from (x, y), its middle row y + 3, pointing right (or left) */
static void arrow(int x, int y, bool right, SDL_Color c) {
	SDL_Rect r[8];
	int n = 0;
	r[n++] = (SDL_Rect){ right ? x : x + 7, y + 2, 15, 3 };
	for (int k = 0; k < 4; ++k) r[n++] = (SDL_Rect){ right ? x + 15 + k : x + 6 - k, y + k, 1, 7 - 2 * k };
	fill_rects(r, n, c);
}

/* a button of the two, lit where the cursor is */
static void button(int x, int y, const char *label, bool lit) {
	netpass_panel(x, y, BUTTON_W, BUTTON_H, lit ? ROW_LIT : TRAY, lit ? PET_GOLD : PET_SLOT_EDGE);
	if (lit) fill_rect(x + 2, y + 1, BUTTON_W - 4, 1, PET_CYAN_HI);
	text_draw(x + BUTTON_W / 2, y + 2, label, lit ? PET_WHITE : PET_DIM, TEXT_CENTER);
	if (lit) for (int k = 0; k < 4; ++k) fill_rect(x + 5 + k, y + 5 + k, 1, 7 - 2 * k, PET_GOLD);
}

/* the cards where they stand, crossing in the swap: MegaMan's going right
 * over the top, the other's left under it */
static void cards(int x, int y, const TradeScreenView *v) {
	int lx = x + DEAL_LEFT, rx = x + DEAL_RIGHT, cy = y + DEAL_Y;
	if (v->stage < TRADE_SWAP) {
		deal_card(lx, cy, v->mine, PET_CYAN);
		deal_card(rx, cy, v->get, PET_CYAN);
		return;
	}
	if (v->stage == TRADE_SWAPPED) {
		deal_card(lx, cy, v->get, PET_GOLD);
		deal_card(rx, cy, v->mine, PET_SLOT_EDGE);
		return;
	}
	/* (eased, 0 to 1024, and an arc of 10 pixels at its middle) */
	int p = v->swap_t * 1024 / TRADE_SWAP_FRAMES;
	int e = p * p / 1024 * (3 * 1024 - 2 * p) / 1024;
	int lift = 40 * e / 1024 * (1024 - e) / 1024;
	deal_card(rx + (lx - rx) * e / 1024, cy + lift, v->get, PET_CYAN);
	deal_card(lx + (rx - lx) * e / 1024, cy - lift, v->mine, PET_CYAN);
}

/* under the cards: the buttons, the wait, or the trade done; its keys */
static void foot(int x, int y, const TradeScreenView *v) {
	int ky = y + 148;
	if (v->stage == TRADE_ASK) {
		button(x + W / 2 - 4 - BUTTON_W, y + BUTTON_Y, trade_text_no_deal, !v->cursor);
		button(x + W / 2 + 4, y + BUTTON_Y, trade_text_deal, v->cursor);
		int kx = key(x + 8, ky, "A", trade_text_key_choose);
		key(kx, ky, "B", trade_text_no_deal);
	} else if (v->stage == TRADE_WAIT) {
		text_draw(x + W / 2, y + BUTTON_Y + 2, trade_text_waiting(v->partner), PET_WHITE, TEXT_CENTER);
		key(x + 8, ky, "B", trade_text_key_call_off);
	} else if (v->stage == TRADE_SWAPPED) {
		text_draw(x + W / 2, y + BUTTON_Y - 2, trade_text_traded, PET_GOLD, TEXT_CENTER);
		minifont_draw_centered(x + W / 2, y + BUTTON_Y + 12, trade_text_home, PET_CYAN_HI, 1);
		key(x + 8, ky, "A", trade_text_key_ok);
	}
}

void trade_screen_draw_deal(int x, int y, const TradeScreenView *v) {
	netpass_ground(x, y, W, 160);
	band(x, y, trade_text_deal_title, trade_text_deal_with(v->partner));
	tab(x + DEAL_LEFT, y + DEAL_Y - 10, trade_text_you, PET_CYAN_HI);
	tab(x + DEAL_RIGHT, y + DEAL_Y - 10, trade_text_them, PET_CYAN_HI);
	cards(x, y, v);
	if (v->stage < TRADE_SWAP) {
		/* (the arrows, the one MegaMan's chip goes by gold; pulsing while the other decides) */
		bool dim = v->stage == TRADE_WAIT && (v->t / 20) % 2;
		int ax = x + DEAL_LEFT + DEAL_W + (DEAL_RIGHT - DEAL_LEFT - DEAL_W - 22) / 2;
		arrow(ax, y + DEAL_Y + 26, true, dim ? GOLD_EDGE : PET_GOLD);
		arrow(ax, y + DEAL_Y + 46, false, dim ? PET_SLOT_EDGE : PET_CYAN);
	}
	foot(x, y, v);
}

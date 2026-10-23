/* trade_screen_draw.h. */
#include "trade_screen_draw.h"

#include <stdio.h>

#include "chips.h"
#include "data.h"
#include "minifont.h"
#include "pet_look.h"
#include "second_art.h"
#include "second_frame.h"
#include "trade_screen_text.h"

#define W 240
#define GOLD_EDGE rgba(198, 90, 0, 255)

/* the spares list: the card on the left, the list on the right, the note and keys under them */
#define CARD_X 7
#define BODY_Y 18
#define CARD_W 104
#define BODY_H 118
#define LIST_X 114
#define LIST_W 119
#define LIST_TOP 11   /* the count and the sort over the rows */
#define ROW_H 16
#define NOTE_Y 139

/* the chip trade: two cards, the arrows between */
#define DEAL_W 98
#define DEAL_H 84
#define DEAL_Y 31
#define DEAL_LEFT 10
#define DEAL_RIGHT (W - 10 - DEAL_W)
#define BUTTON_Y 121
#define BUTTON_W 78
#define BUTTON_H 17
#define KEYS_Y 146

static char code_char(uint8_t code) { return code >= 26 ? '*' : (char)('A' + code); }

/* the screen: BN6's frame, its band with the title and `right` as its note */
static void band(int x, int y, const char *title, const char *right) {
	look_screen(x, y, W, 160, title);
	look_band_note(x, y, W, right, LOOK_PAGE);
}

/* a key's hint (BN6's "START:SORT"), then the x after it */
static int key(int x, int y, const char *cap, const char *what) {
	look_hint(x, y, cap, what);
	return x + look_hint_width(cap, what) + 10;
}

/* a line of small letters on the body, its shade under it */
static void body_note(int x, int y, const char *s, bool centred) {
	if (centred) x -= minifont_width(s, 1) / 2;
	minifont_draw(x + 1, y + 1, s, LOOK_GREEN_LO, 1);
	minifont_draw(x, y, s, LOOK_PAGE, 1);
}

/* the chip's picture, its card's 56 x 48, in a frame from (x, y) */
static void picture(int id, int x, int y, SDL_Color edge) {
	fill_rect(x, y, 58, 50, edge);
	fill_rect(x + 1, y + 1, 56, 48, LOOK_NAVY_LO);
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
	SDL_Rect f = look_window(x, y, CARD_W, BODY_H, 0, false);
	if (!c.id) return;
	ChipInfo ci;
	chip_info(c.id, &ci);
	text_draw(f.x + 2, f.y, ci.name, LOOK_TEXT, TEXT_LEFT);
	picture(c.id, f.x + 1, f.y + 14, LOOK_EDGE_LO);
	int cx = f.x + 59 + (f.w - 59) / 2;
	char code[2] = { code_char(c.code), 0 }, power[12];
	text_draw_scaled(cx, f.y + 10, code, LOOK_GOLD, TEXT_CENTER, 2);
	second_element_icon(second_chip_element(c.id), cx - 8, f.y + 36);
	power_words(&ci, power, sizeof power);
	text_draw(cx, f.y + 52, power, LOOK_TEXT, TEXT_CENTER);
	fill_rect(f.x + 1, f.y + 67, f.w - 2, 1, LOOK_SHADE);
	char desc[160];
	chip_desc(c.id, desc, sizeof desc);
	look_wrapped(desc, f.x + 2, f.y + 69, f.w - 4, 3, LOOK_TEXT, LOOK_SHADE);
}

/* a row: the cursor's arrow over the frame, the chip's icon, its name, its code and the spares */
static void row(int x, int y, int w, const TradeRow *r, bool lit) {
	if (lit) look_cursor(x - 7, y + 4);
	second_chip_icon(r->chip.id, x + 2, y);
	char name[24], code[2] = { code_char(r->chip.code), 0 }, count[8];
	chips_name(r->chip.id, name, sizeof name);
	text_draw(x + 19, y + 2, name, LOOK_TEXT, TEXT_LEFT);
	text_draw(x + w - 20, y + 2, code, LOOK_GOLD, TEXT_CENTER);
	snprintf(count, sizeof count, "%d", r->count);
	text_draw(x + w - 2, y + 2, count, LOOK_EDGE_HI, TEXT_RIGHT);
}

static void list_panel(int x, int y, const TradeScreenView *v) {
	SDL_Rect f = look_window(x, y, LIST_W, BODY_H, LIST_TOP, true);
	/* (over the rows in the band: the count, and the sort with a small triangle for its way: up) */
	char n[24];
	snprintf(n, sizeof n, "%s %d", trade_text_spares, v->n);
	look_caps(x + 6, y + 7, n, LOOK_PAGE, LOOK_EDGE_LO);
	const char *sort = trade_text_sorts[v->sort];
	int sx = x + LIST_W - 8 - 7 - look_caps_width(sort);
	look_caps(sx, y + 7, sort, LOOK_PAGE, LOOK_EDGE_LO);
	for (int k = 0; k < 3; ++k) fill_rect(x + LIST_W - 13 + k, y + 8 + k, 5 - 2 * k, 1, LOOK_GOLD);
	for (int i = 0; i < TRADE_ROWS && v->top + i < v->n; ++i)
		row(f.x, f.y + i * ROW_H, f.w, &v->rows[v->top + i], v->top + i == v->at);
	look_scroll((SDL_Rect){ x, y, LIST_W, BODY_H }, LIST_TOP, v->top, TRADE_ROWS, v->n);
}

static void list_bar(int x, int y, const TradeScreenView *v) {
	char theirs[32] = "";
	if (v->theirs.id) {
		char name[24];
		chips_name(v->theirs.id, name, sizeof name);
		snprintf(theirs, sizeof theirs, "%s %c", name, code_char(v->theirs.code));
	}
	body_note(x + 9, y + NOTE_Y, trade_text_list_note(v->purpose, v->want, v->partner, theirs), false);
	int kx = key(x + 9, y + KEYS_Y + 1, "A", trade_text_key_pick);
	kx = key(kx, y + KEYS_Y + 1, "B", trade_text_key_back);
	if (v->n > 1) key(kx, y + KEYS_Y + 1, "START", trade_text_key_sort);
}

void trade_screen_draw_list(int x, int y, const TradeScreenView *v) {
	band(x, y, trade_text_list_title(v->purpose), trade_text_list_for(v->purpose, v->partner));
	list_card(x + CARD_X, y + BODY_Y, v->n ? v->rows[v->at].chip : (TradeChip){ 0, 0 });
	list_panel(x + LIST_X, y + BODY_Y, v);
	list_bar(x, y, v);
}

/* ---- the chip trade ---- */

/* a chip's card in the trade: its picture, name, code, element and power;
 * `edge` gold for the chip come, dim for the one gone */
static void deal_card(int x, int y, TradeChip c, SDL_Color edge) {
	bool gone = edge.r == PET_SLOT_EDGE.r && edge.g == PET_SLOT_EDGE.g, come = edge.r == PET_GOLD.r && edge.g == PET_GOLD.g;
	SDL_Rect f = look_window(x, y, DEAL_W, DEAL_H, 0, false);
	if (gone) fill_rect(f.x, f.y, f.w, f.h, LOOK_NAVY_LO);
	if (come) {
		SDL_Rect r[4] = { { x + 1, y, DEAL_W - 2, 1 }, { x + 1, y + DEAL_H - 1, DEAL_W - 2, 1 }, { x, y + 1, 1, DEAL_H - 2 }, { x + DEAL_W - 1, y + 1, 1, DEAL_H - 2 } };
		fill_rects(r, 4, LOOK_GOLD);
	}
	int cx = x + DEAL_W / 2;
	picture(c.id, cx - 29, f.y, LOOK_EDGE_LO);
	ChipInfo ci;
	chip_info(c.id, &ci);
	text_draw(cx, f.y + 50, ci.name, LOOK_TEXT, TEXT_CENTER);
	char code[2] = { code_char(c.code), 0 }, power[12];
	power_words(&ci, power, sizeof power);
	/* (the code, the element's icon and the power in a row, centred) */
	int el = second_chip_element(c.id), cw = text_width(code), pw = text_width(power);
	int rw = cw + 4 + (el >= 0 ? 16 + 4 : 0) + pw, rx = cx - rw / 2;
	text_draw(rx, f.y + 62, code, LOOK_GOLD, TEXT_LEFT);
	rx += cw + 4;
	if (el >= 0) { second_element_icon(el, rx, f.y + 60); rx += 20; }
	text_draw(rx, f.y + 62, power, LOOK_TEXT, TEXT_LEFT);
}

/* a card's label on a tag over it */
static void tab(int x, int y, const char *label) {
	look_tag(x + 6, y, look_caps_width(label) + 10, 9, label, LOOK_BAND);
}

/* an arrow 22 wide from (x, y), its middle row y + 3, pointing right (or left) */
static void arrow(int x, int y, bool right, SDL_Color c) {
	SDL_Rect r[8];
	int n = 0;
	r[n++] = (SDL_Rect){ right ? x : x + 7, y + 2, 15, 3 };
	for (int k = 0; k < 4; ++k) r[n++] = (SDL_Rect){ right ? x + 15 + k : x + 6 - k, y + k, 1, 7 - 2 * k };
	fill_rects(r, n, c);
}

/* a button of the two: a slim window, the cursor's arrow at its left where it is */
static void button(int x, int y, const char *label, bool lit) {
	look_box(x, y, BUTTON_W, BUTTON_H, lit ? LOOK_NAVY : LOOK_NAVY_LO);
	text_draw(x + BUTTON_W / 2 + 4, y + 2, label, lit ? LOOK_TEXT : LOOK_DIM, TEXT_CENTER);
	if (lit) look_cursor(x - 4, y + 4);
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
	int ky = y + KEYS_Y + 1;
	if (v->stage == TRADE_ASK) {
		button(x + W / 2 - 4 - BUTTON_W, y + BUTTON_Y, trade_text_no_deal, !v->cursor);
		button(x + W / 2 + 4, y + BUTTON_Y, trade_text_deal, v->cursor);
		int kx = key(x + 10, ky, "A", trade_text_key_choose);
		key(kx, ky, "B", trade_text_no_deal);
	} else if (v->stage == TRADE_WAIT) {
		look_text(x + W / 2, y + BUTTON_Y + 3, trade_text_waiting(v->partner), LOOK_PAGE, LOOK_GREEN_LO, TEXT_CENTER);
		key(x + 10, ky, "B", trade_text_key_call_off);
	} else if (v->stage == TRADE_SWAPPED) {
		text_draw(x + W / 2, y + BUTTON_Y - 2, trade_text_traded, LOOK_GOLD, TEXT_CENTER);
		body_note(x + W / 2, y + BUTTON_Y + 12, trade_text_home, true);
		key(x + 10, ky, "A", trade_text_key_ok);
	}
}

void trade_screen_draw_deal(int x, int y, const TradeScreenView *v) {
	band(x, y, trade_text_deal_title, trade_text_deal_with(v->partner));
	tab(x + DEAL_LEFT, y + DEAL_Y - 11, trade_text_you);
	tab(x + DEAL_RIGHT, y + DEAL_Y - 11, trade_text_them);
	cards(x, y, v);
	if (v->stage < TRADE_SWAP) {
		/* (the arrows, the one MegaMan's chip goes by gold; pulsing while the other decides) */
		bool dim = v->stage == TRADE_WAIT && (v->t / 20) % 2;
		int ax = x + DEAL_LEFT + DEAL_W + (DEAL_RIGHT - DEAL_LEFT - DEAL_W - 22) / 2;
		arrow(ax, y + DEAL_Y + 26, true, dim ? GOLD_EDGE : LOOK_GOLD);
		arrow(ax, y + DEAL_Y + 46, false, dim ? LOOK_GREEN_LO : LOOK_EDGE_HI);
	}
	foot(x, y, v);
}

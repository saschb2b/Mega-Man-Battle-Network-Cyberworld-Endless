/* trade_screen_draw.h. */
#include "trade_screen_draw.h"

#include <stdio.h>

#include "chips.h"
#include "data.h"
#include "minifont.h"
#include "pet_look.h"
#include "second_art.h"
#include "text.h"
#include "trade_screen_text.h"

#define W 240
#define GOLD_EDGE rgba(198, 90, 0, 255)

/* the spares list: the chip's card on the left (the Library's), the list
 * on the right, a note under them where there is one */
#define CARD_X 7
#define BODY_Y 18
#define CARD_W 84
#define BODY_H 128
#define LIST_X 94
#define LIST_W 139
#define LIST_TOP 11   /* the count and the sort over the rows */
#define ROW_H 16
#define NOTE_Y 149

/* the chip trade: two cards, the arrows between, the two pills under them */
/* the chip trade: two cards, the arrows between, MegaMan in the NAVIGATOR
 * box under them (E-Mail's place) asking, waiting and telling */
#define DEAL_W 98
#define DEAL_H 78
#define DEAL_Y 18
#define DEAL_LEFT 10
#define DEAL_RIGHT (W - 10 - DEAL_W)
#define NAV_Y 96
#define YES_X 91   /* Yes and No where BN6's chat box has them */
#define NO_X 138

static char code_char(uint8_t code) { return code >= 26 ? '*' : (char)('A' + code); }

/* the screen: BN6's frame, its band with the title and `right` as its note */
static void band(int x, int y, const char *title, const char *right) {
	look_screen(x, y, W, 160, title);
	look_band_note(x, y, W, right, LOOK_PAGE);
}

/* a line of small letters on the body, its shade under it */
static void body_note(int x, int y, const char *s, bool centred) {
	if (centred) x -= minifont_width(s, 1) / 2;
	minifont_draw(x + 1, y + 1, s, LOOK_GREEN_LO, 1);
	minifont_draw(x, y, s, LOOK_PAGE, 1);
}

/* the chip's picture, its card's 56 x 48, in a frame from (x, y), 58 x 50 */
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

/* the strip under a card's picture, as the Library's: the code, the
 * element's icon and the power on the dark, 58 x 16 from (x, y) */
static void card_strip(int id, int code, int x, int y) {
	ChipInfo ci;
	chip_info(id, &ci);
	fill_rect(x, y, 58, 16, LOOK_CARD_DARK);
	char c[2] = { code_char((uint8_t)code), 0 }, power[12];
	text_draw(x + 3, y + 4, c, LOOK_GOLD, TEXT_LEFT);
	int el = second_chip_element(id);
	if (el >= 0) second_element_icon(el, x + 14, y + 1);
	power_words(&ci, power, sizeof power);
	text_draw(x + 55, y + 4, power, LOOK_TEXT, TEXT_RIGHT);
}

/* ---- the spares list ---- */

/* the cursor's chip as the Library's card: its picture, the code's strip, its description */
static void list_card(int x, int y, TradeChip c) {
	SDL_Rect f = look_card(x, y, CARD_W, BODY_H);
	if (!c.id) return;
	int px = f.x + (f.w - 58) / 2;
	picture(c.id, px, f.y + 1, LOOK_CARD_DARK);
	card_strip(c.id, c.code, px, f.y + 51);
	char desc[160];
	chip_desc(c.id, desc, sizeof desc);
	/* (the card a little wider than BN6's: the chat font's letters are wider than its card's) */
	look_wrapped(desc, f.x + 2, f.y + 69, f.w - 3, 3, LOOK_CARD_INK, LOOK_PAGE);
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
	/* (no keys: BN6's folder editor sorts on START with no word of it, its sort shown over the list) */
	body_note(x + 9, y + NOTE_Y, trade_text_list_note(v->purpose, v->want, v->partner, theirs), false);
}

/* START's SORT pop-up over the list, as the folder editor's: a small
 * window, SORT on its band, the keys in the chat font, the arrow on the
 * one the cursor is on */
static void sort_popup(int x, int y, const TradeScreenView *v) {
	const int w = 62, top = 9, line = 13;
	int px = x + LIST_X + LIST_W - w - 6, py = y + BODY_Y + 10;
	SDL_Rect f = look_window(px, py, w, 2 * LOOK_FRAME + top + TRADE_SORT_KEYS * line + 1, top, false);
	look_caps(px + 6, py + 5, trade_text_sort, LOOK_PAGE, (SDL_Color){ 0, 0, 0, 0 });
	for (int k = 0; k < TRADE_SORT_KEYS; ++k) look_text(f.x + 8, f.y + k * line, trade_text_sorts[k], LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
	look_cursor(f.x - 3, f.y + v->sort_at * line + 2);
}

void trade_screen_draw_list(int x, int y, const TradeScreenView *v) {
	band(x, y, trade_text_list_title(v->purpose), trade_text_list_for(v->purpose, v->partner));
	list_card(x + CARD_X, y + BODY_Y, v->n ? v->rows[v->at].chip : (TradeChip){ 0, 0 });
	list_panel(x + LIST_X, y + BODY_Y, v);
	list_bar(x, y, v);
	if (v->sorting) sort_popup(x, y, v);
}

/* ---- the chip trade ---- */

/* a chip's card in the trade, as the Library's: its picture and the
 * code's strip (MegaMan names the chips); the chip come marked by the
 * ChipFolder's EQUIP corners */
static void deal_card(int x, int y, TradeChip c, bool come) {
	SDL_Rect f = look_card(x, y, DEAL_W, DEAL_H);
	int px = x + (DEAL_W - 58) / 2;
	picture(c.id, px, f.y, LOOK_CARD_DARK);
	card_strip(c.id, c.code, px, f.y + 50);
	if (come) look_corners(x - 3, y - 2, DEAL_W + 6, DEAL_H + 5);
}

/* a chip's name and code as MegaMan says them ("MrkCan1 M") */
static void chip_words(TradeChip c, char *out, size_t n) {
	char name[24];
	chips_name(c.id, name, sizeof name);
	snprintf(out, n, "%s %c", name, code_char(c.code));
}

/* an arrow 22 wide from (x, y), its middle row y + 3, pointing right (or left) */
static void arrow(int x, int y, bool right, SDL_Color c) {
	SDL_Rect r[8];
	int n = 0;
	r[n++] = (SDL_Rect){ right ? x : x + 7, y + 2, 15, 3 };
	for (int k = 0; k < 4; ++k) r[n++] = (SDL_Rect){ right ? x + 15 + k : x + 6 - k, y + k, 1, 7 - 2 * k };
	fill_rects(r, n, c);
}


/* the cards where they stand, crossing in the swap: MegaMan's going right
 * over the top, the other's left under it */
static void cards(int x, int y, const TradeScreenView *v) {
	int lx = x + DEAL_LEFT, rx = x + DEAL_RIGHT, cy = y + DEAL_Y;
	if (v->stage < TRADE_SWAP) {
		deal_card(lx, cy, v->mine, false);
		deal_card(rx, cy, v->get, false);
		return;
	}
	if (v->stage == TRADE_SWAPPED) {
		deal_card(lx, cy, v->get, true);
		deal_card(rx, cy, v->mine, false);
		return;
	}
	/* (eased, 0 to 1024, and an arc of 10 pixels at its middle) */
	int p = v->swap_t * 1024 / TRADE_SWAP_FRAMES;
	int e = p * p / 1024 * (3 * 1024 - 2 * p) / 1024;
	int lift = 40 * e / 1024 * (1024 - e) / 1024;
	deal_card(rx + (lx - rx) * e / 1024, cy + lift, v->get, false);
	deal_card(lx + (rx - lx) * e / 1024, cy - lift, v->mine, false);
}

/* under the cards, MegaMan in E-Mail's NAVIGATOR box: the trade asked
 * as BN6 asks (Yes and No, the green triangle), the wait, the swap, the
 * trade done; A and B as everywhere in BN6, no keys shown */
static void navigator(int x, int y, const TradeScreenView *v) {
	SDL_Rect pg = look_navigator(x + 2, y + NAV_Y, W - 4, trade_text_navigator, FACE_MEGAMAN);
	int tx = pg.x + 4, ty = pg.y + 4;
	char mine[32], get[32];
	chip_words(v->mine, mine, sizeof mine);
	chip_words(v->get, get, sizeof get);
	const char *say = v->stage == TRADE_ASK ? trade_text_ask(mine, v->partner, get)
		: v->stage == TRADE_WAIT ? trade_text_waiting(v->partner)
		: v->stage == TRADE_SWAP ? trade_text_swapping : trade_text_traded(get);
	look_wrapped(say, tx, ty, pg.w - 8, v->stage == TRADE_ASK ? 2 : 3, LOOK_INK, LOOK_INK_SHADE);
	if (v->stage != TRADE_ASK) return;
	int oy = ty + 2 * LOOK_NAV_LINE;
	look_text(x + YES_X, oy, trade_text_yes, LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
	look_text(x + NO_X, oy, trade_text_no, LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
	look_choice(x + (v->cursor ? YES_X : NO_X) - LOOK_CHOICE_W - 3, oy - 1);
}

void trade_screen_draw_deal(int x, int y, const TradeScreenView *v) {
	band(x, y, trade_text_deal_title, trade_text_deal_with(v->partner));
	cards(x, y, v);
	if (v->stage < TRADE_SWAP) {
		/* (the arrows, the one MegaMan's chip goes by gold; pulsing while the other decides) */
		bool dim = v->stage == TRADE_WAIT && (v->t / 20) % 2;
		int ax = x + DEAL_LEFT + DEAL_W + (DEAL_RIGHT - DEAL_LEFT - DEAL_W - 22) / 2;
		arrow(ax, y + DEAL_Y + 24, true, dim ? GOLD_EDGE : LOOK_GOLD);
		arrow(ax, y + DEAL_Y + 44, false, dim ? LOOK_GREEN_LO : LOOK_EDGE_HI);
	}
	navigator(x, y, v);
}

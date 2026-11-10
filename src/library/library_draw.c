/* library_draw.h. */
#include "library_draw.h"

#include <stdio.h>

#include "bn6_chips.h"
#include "chips.h"
#include "data.h"
#include "gfx.h"
#include "library_text.h"
#include "pet_look.h"
#include "second_art.h"

#define W 240
#define H 160
#define COUNT_EDGE rgba(173, 255, 189, 255)   /* a count's pill: its outline and digits (FOLDER EDIT's 30/30) */

/* BN6's Library's frame under the entries' tabs: the card on the left,
 * the list's window on the right, both to the screen's foot */
#define TABS_X 7
#define TABS_Y 19
#define TABS_W (W - 14)
#define CARD_X 7
#define BODY_Y 36
#define CARD_W 84
#define BODY_H 117
#define LIST_X 94
#define LIST_W 139
#define LIST_TOP 11   /* the class and its count over the slots */
#define ROW_H 16

/* ---- the card: the chip under the cursor, as BN6's Library shows it ---- */

static void card(int x, int y, const LibraryView *v) {
	SDL_Rect f = look_card(x, y, CARD_W, BODY_H);
	int px = f.x + (f.w - 58) / 2;
	/* (the picture's dark frame, empty where the slot is: BN6's card of a number never seen) */
	fill_rect(px, f.y + 1, 58, 50, LOOK_CARD_DARK);
	if (!v->n || !v->seen[v->at]) return;
	int id = v->rows[v->at].chip;
	second_chip_art(id, px + 1, f.y + 2, 1);
	/* (under it on the dark: the element's icon and the power, no code, as the Library's) */
	ChipInfo ci;
	chip_info(id, &ci);
	fill_rect(px, f.y + 51, 58, 16, LOOK_CARD_DARK);
	int el = second_chip_element(id);
	if (el >= 0) second_element_icon(el, px + 14, f.y + 51);
	/* (a power the game computes, 1001 and up, is none to show) */
	if (ci.power > 0 && ci.power <= 1000) text_drawf(px + 55, f.y + 55, LOOK_TEXT, TEXT_RIGHT, "%d", ci.power);
	char desc[160];
	chip_desc(id, desc, sizeof desc);
	look_wrapped(desc, f.x + 2, f.y + 67, f.w - 3, 3, LOOK_CARD_INK, LOOK_PAGE);
}

/* ---- the list: the page's slots ---- */

static SDL_Rect list_field(void) {
	return (SDL_Rect){ LIST_X + LOOK_FRAME, BODY_Y + LOOK_FRAME + LIST_TOP, LIST_W - LOOK_FRAME - LOOK_FRAME_BAR, BODY_H - 2 * LOOK_FRAME - LIST_TOP };
}

SDL_Rect library_row_rect(int i) {
	SDL_Rect f = list_field();
	return (SDL_Rect){ f.x, f.y + i * ROW_H, f.w, ROW_H };
}

SDL_Rect library_turn_rect(int right) { return (SDL_Rect){ right ? LIST_X + LIST_W - 22 : LIST_X, BODY_Y, 22, LOOK_FRAME + LIST_TOP }; }

SDL_Rect library_tab_rect(int entry) {
	int l = TABS_X + LOOK_TABS_IN, r = TABS_X + TABS_W - LOOK_TABS_IN, tw = (r - l - (LIB_ENTRIES - 1) * LOOK_TAB_GAP) / LIB_ENTRIES;
	return (SDL_Rect){ l + entry * (tw + LOOK_TAB_GAP), TABS_Y, tw, LOOK_PILL_H };
}

/* a chip's rarity as BN6's list shows it: its stars in gold, the rest of the five as dots, 21 x 5 from (x, y) */
static void stars(int x, int y, int rarity) {
	static const char *const star[5] = { "..#..", "#####", ".###.", "##.##", "#...#" };
	SDL_Rect px[5 * 16];
	int n = 0;
	for (int s = 0; s < 5; ++s) {
		if (s > rarity) { fill_rect(x + s * 4 + 2, y + 2, 1, 1, LOOK_EDGE_HI); continue; }
		for (int r = 0; r < 5; ++r)
			for (int c = 0; c < 5; ++c)
				if (star[r][c] == '#') px[n++] = (SDL_Rect){ x + s * 4 + c, y + r, 1, 1 };
	}
	fill_rects(px, n, LOOK_GOLD);
}

static void row(int x, int y, int w, const LibSlot *s, bool seen, bool lit) {
	if (lit) look_cursor(x - 7, y + 4);
	text_drawf(x + 21, y + 2, LOOK_TEXT, TEXT_RIGHT, "%d", s->number);
	if (!seen) {
		text_draw(x + 60, y + 2, library_unseen, LOOK_TEXT, TEXT_CENTER);
		return;
	}
	second_chip_icon(s->chip, x + 23, y);
	char name[24];
	chips_name(s->chip, name, sizeof name);
	text_draw(x + 39, y + 2, name, LOOK_TEXT, TEXT_LEFT);
	const uint8_t *rec = chips_record(s->chip);
	/* (a name past BN6's eight letters and a mark leaves the stars no room) */
	if (rec && text_width(name) <= 64) stars(x + w - 21, y + 5, rec[BN6_CHIP_RARITY] > 4 ? 4 : rec[BN6_CHIP_RARITY]);
}

/* the page's count in its pill, right-ended at x: light green on navy, gold where the class is whole */
static void count_pill(int right, int y, const LibraryView *v) {
	char s[16];
	library_count_words(s, sizeof s, v->held, v->of);
	SDL_Color c = v->of > 0 && v->held >= v->of ? LOOK_GOLD : COUNT_EDGE;
	int w = look_caps_width(s) + 10, x = right - w;
	fill_rect(x + 1, y, w - 2, 9, c);
	fill_rect(x, y + 1, w, 7, c);
	fill_rect(x + 1, y + 1, w - 2, 7, LOOK_NAVY);
	look_caps(x + 5, y + 2, s, c, (SDL_Color){ 0, 0, 0, 0 });
}

static void list_panel(int x, int y, const LibraryView *v) {
	look_window(x + LIST_X, y + BODY_Y, LIST_W, BODY_H, LIST_TOP, true);
	/* (the class in the band over its slots, between the page arrows where LEFT and RIGHT turn, its count before the right one) */
	look_arrow(x + LIST_X + 4, y + BODY_Y + 5, false);
	look_arrow(x + LIST_X + LIST_W - 12, y + BODY_Y + 5, true);
	text_draw(x + LIST_X + 15, y + BODY_Y + 4, library_class_names[v->cls], LOOK_TEXT, TEXT_LEFT);
	count_pill(x + LIST_X + LIST_W - 15, y + BODY_Y + 4, v);
	for (int i = 0; i < LIBRARY_ROWS && v->top + i < v->n; ++i) {
		SDL_Rect r = library_row_rect(i);
		row(x + r.x, y + r.y, r.w, &v->rows[v->top + i], v->seen[v->top + i], v->top + i == v->at);
	}
	if (v->n > LIBRARY_ROWS) look_scroll((SDL_Rect){ x + LIST_X, y + BODY_Y, LIST_W, BODY_H }, LIST_TOP, v->top, LIBRARY_ROWS, v->n);
}

void library_draw_view(int x, int y, const LibraryView *v) {
	look_screen(x, y, W, H, library_title);
	/* (the entry's whole count at the band's right end, gold where its three classes are whole) */
	char all[24];
	library_entry_words(all, sizeof all, v->entry, v->entry_held, v->entry_of);
	look_band_note(x, y, W, all, v->entry_of > 0 && v->entry_held >= v->entry_of ? LOOK_GOLD : LOOK_PAGE);
	look_tabs(x + TABS_X, y + TABS_Y, TABS_W, LIB_ENTRIES, v->entry, library_entry_names);
	card(x + CARD_X, y + BODY_Y, v);
	list_panel(x, y, v);
}

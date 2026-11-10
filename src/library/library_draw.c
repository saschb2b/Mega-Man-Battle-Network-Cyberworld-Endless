/* library_draw.h. The frame is BN6's own Library's, measured from Gregar's
 * (docs/engine/ui.md): the card at its place and size, the window, the
 * rows; the entries stand on the band, where BN6's has nothing. */
#include "library_draw.h"

#include <stdio.h>
#include <string.h>

#include "bn6_chips.h"
#include "chips.h"
#include "data.h"
#include "gfx.h"
#include "library_text.h"
#include "pet_look.h"
#include "second_art.h"

#define W 240
#define H 160
#define NONE (SDL_Color){ 0, 0, 0, 0 }
#define COUNT_EDGE rgba(181, 255, 189, 255)   /* the count's pill: its outline (the Library's 12/200) */

#define CARD_X 7
#define CARD_Y 17
#define CARD_W 80
#define CARD_H 134
#define FIELD_X 95    /* the list's field, 130 x 114: seven rows */
#define FIELD_W 130
#define ROW_H 16
#define ROW_Y 33
#define TAG_H 9       /* an entry's tag on the band */
#define TAG_Y 3
#define TAG_GAP 3
#define TAG_RIGHT (W - 6)

/* ---- the card: the chip under the cursor, BN6's Library's ---- */

static const SDL_Color card_line = { 82, 107, 132, 255 }, card_lit = { 247, 247, 247, 255 }, card_shade = { 173, 181, 198, 255 };

/* its face's colour by class: a Mega's cyan, a Giga's pink */
static SDL_Color card_face(int cls) {
	static const SDL_Color face[LIB_CLASSES] = { { 222, 222, 239, 255 }, { 173, 239, 239, 255 }, { 247, 206, 231, 255 } };
	return face[cls];
}

/* the card's body from (x, y): its outline, the lit left edge and shaded right, the cut corner, the waist over its foot */
static void card_body(int x, int y, SDL_Color face) {
	/* (the face and its edges, then what is cut out of them) */
	fill_rect(x + 1, y + 1, CARD_W - 2, CARD_H - 2, face);
	fill_rect(x + 1, y + 5, 2, 116, card_lit);
	fill_rect(x + 77, y + 1, 2, 132, card_shade);
	fill_rect(x, y + 5, 1, 116, card_line);
	fill_rect(x + 79, y + 1, 1, 120, card_line);
	/* (the top left corner cut, lit along the cut; the tabs at the top's ends) */
	for (int r = 0; r < 4; ++r) {
		fill_rect(x, y + 1 + r, 4 - r, 1, LOOK_GREEN);
		fill_rect(x + 4 - r - 1, y + 1 + r, 2, 1, card_lit);
	}
	fill_rect(x, y + 4, 1, 1, card_shade);
	fill_rect(x + 4, y, 3, 1, card_lit);
	fill_rect(x + 72, y, 4, 1, card_lit);
	fill_rect(x + 76, y, 1, 1, face);
	fill_rect(x + 77, y, 1, 1, card_line);
	fill_rect(x + 78, y, 2, 1, LOOK_EDGE);
	/* (the waist: two rows in at each side over the foot) */
	fill_rect(x, y + 120, 3, 1, card_line);
	fill_rect(x + 77, y + 120, 3, 1, card_line);
	fill_rect(x, y + 121, 2, 4, LOOK_GREEN);
	fill_rect(x + 78, y + 121, 2, 4, LOOK_EDGE);
	fill_rect(x + 2, y + 121, 1, 4, card_line);
	fill_rect(x + 77, y + 121, 1, 4, card_line);
	fill_rect(x + 3, y + 120, 1, 6, card_lit);
	fill_rect(x + 76, y + 120, 1, 5, card_shade);
	fill_rect(x, y + 125, 3, 1, card_line);
	fill_rect(x + 77, y + 125, 3, 1, card_line);
	/* (the foot: lit left, shaded right and along its last rows, its corners cut) */
	fill_rect(x, y + 126, 1, 6, card_line);
	fill_rect(x + 79, y + 126, 1, 6, card_line);
	fill_rect(x + 1, y + 126, 3, 1, card_lit);
	fill_rect(x + 1, y + 127, 2, 4, card_lit);
	fill_rect(x + 76, y + 126, 3, 1, card_shade);
	fill_rect(x + 77, y + 127, 2, 4, card_shade);
	fill_rect(x + 1, y + 131, 2, 1, face);
	fill_rect(x + 3, y + 131, 3, 2, card_shade);
	fill_rect(x + 74, y + 131, 5, 2, card_shade);
	fill_rect(x, y + 132, 1, 1, LOOK_GREEN);
	fill_rect(x + 1, y + 132, 1, 1, card_line);
	fill_rect(x + 2, y + 132, 1, 1, face);
	fill_rect(x + 79, y + 132, 1, 1, LOOK_EDGE);
	fill_rect(x + 78, y + 132, 1, 1, card_line);
	fill_rect(x, y + 133, 2, 1, LOOK_GREEN);
	fill_rect(x + 2, y + 133, 76, 1, card_line);
	fill_rect(x + 78, y + 133, 2, 1, LOOK_EDGE);
}

/* the hole at its top left and the grip down its left edge */
static void card_marks(int x, int y, SDL_Color face) {
	fill_rect(x + 3, y + 8, 2, 1, card_shade);
	fill_rect(x + 2, y + 9, 4, 4, LOOK_CARD_DARK);
	const SDL_Rect corners[4] = { { x + 2, y + 9, 1, 1 }, { x + 5, y + 9, 1, 1 }, { x + 2, y + 12, 1, 1 }, { x + 5, y + 12, 1, 1 } };
	fill_rects(corners, 4, card_shade);
	fill_rect(x + 6, y + 10, 1, 3, card_lit);
	fill_rect(x + 1, y + 13, 5, 1, card_lit);
	fill_rect(x + 1, y + 23, 4, 1, card_shade);
	fill_rect(x + 2, y + 24, 2, 22, face);
	fill_rect(x + 4, y + 24, 1, 22, card_shade);
	fill_rect(x + 5, y + 23, 1, 24, card_lit);
	fill_rect(x + 1, y + 46, 4, 1, card_shade);
	fill_rect(x + 1, y + 47, 4, 1, card_lit);
}

/* the contacts along its foot: gold in columns of two, lit between them, white at their feet */
static void card_contacts(int x, int y) {
	static const SDL_Color first = { 222, 206, 123, 255 }, dark = { 189, 148, 66, 255 }, gold = { 255, 247, 132, 255 };
	fill_rect(x + 7, y + 123, 66, 1, card_line);
	fill_rect(x + 6, y + 124, 1, 9, card_line);
	fill_rect(x + 73, y + 124, 1, 8, card_lit);
	for (int r = 0; r < 9; ++r)
		for (int c = 0; c < 66; ++c) {
			bool light = c % 4 < 2;
			SDL_Color px = !light ? dark : r == 0 || c == 0 ? first : r >= 6 || (r == 5 && c % 4 == 0) ? card_lit : gold;
			if (c == 1 && r >= 1 && r <= 5) px = gold;
			fill_rect(x + 7 + c, y + 124 + r, 1, 1, px);
		}
}

/* the dark block at its head: the picture's frame, and under the picture the element's icon and the power (chip 0: the frame alone) */
static void card_head(int x, int y, int id) {
	static const SDL_Color top = { 206, 206, 222, 255 }, glint = { 231, 247, 247, 255 };
	fill_rect(x + 7, y + 1, 66, 1, card_shade);
	fill_rect(x + 16, y + 1, 8, 1, top);
	fill_rect(x + 18, y + 1, 4, 1, glint);
	fill_rect(x + 7, y + 2, 66, 66, LOOK_CARD_DARK);
	if (!id) return;
	second_chip_art(id, x + 12, y + 4, 1);
	int el = second_chip_element(id);
	if (el >= 0) second_element_icon(el, x + 24, y + 52);
	ChipInfo ci;
	chip_info(id, &ci);
	/* (a power the game computes, 1001 and up, is none to show) */
	char power[8];
	int digits = snprintf(power, sizeof power, "%d", ci.power);
	if (ci.power > 0 && ci.power <= 1000) text_draw_cells(x + 72 - 8 * digits, y + 53, power, rgba(231, 231, 231, 255), LOOK_CARD_DARK);
}

void library_card_lines(int x, int y, const char *lines) {
	static const SDL_Color ink = { 66, 57, 57, 255 }, under = { 189, 189, 189, 255 };
	char desc[160];
	snprintf(desc, sizeof desc, "%s", lines);
	int line = 0;
	for (char *s = desc; s && *s && line < 3; ++line) {
		char *end = strchr(s, '\n');
		if (end) *end = 0;
		/* (its shade a pixel right and below every stroke, as the card sets it) */
		look_text_cells(x + 5, y + 72 + line * 16, s, under, NONE, 8);
		look_text_cells(x + 4, y + 71 + line * 16, s, ink, NONE, 8);
		s = end ? end + 1 : NULL;
	}
}

void library_card_blank(int x, int y, int cls) {
	SDL_Color face = card_face(cls);
	card_body(x, y, face);
	card_marks(x, y, face);
	card_head(x, y, 0);
	card_contacts(x, y);
}

void library_card(int x, int y, int chip, int cls) {
	library_card_blank(x, y, cls);
	if (!chip) return;
	card_head(x, y, chip);
	/* its description in the game's own three lines, each letter in a cell of 8, 16 apart */
	char desc[160];
	chips_desc_lines(chip, desc, sizeof desc);
	library_card_lines(x, y, desc);
}

/* ---- the band: the entries ---- */

/* entry `e`'s words on its tag: its name, and its count where it is the one shown */
static void tag_words(const LibraryView *v, int e, char *out, size_t n) {
	if (e == v->entry) library_entry_words(out, n, e, v->entry_held, v->entry_of);
	else snprintf(out, n, "%s", library_entry_names[e]);
}

SDL_Rect library_tab_rect(const LibraryView *v, int entry) {
	int x = TAG_RIGHT;
	SDL_Rect r = { 0, TAG_Y, 0, TAG_H };
	for (int e = LIB_ENTRIES - 1; e >= entry; --e) {
		char s[24];
		tag_words(v, e, s, sizeof s);
		r.w = look_caps_width(s) + 8;
		x -= r.w;
		r.x = x;
		x -= TAG_GAP;
	}
	return r;
}

/* the entries on the band's right: a tag each, the one shown lit as NaviCust's RUN! with its count, gold where its three classes are whole */
static void tags(int x, int y, const LibraryView *v) {
	static const SDL_Color lit = { 16, 231, 41, 255 }, rest = { 16, 82, 107, 255 };
	for (int e = 0; e < LIB_ENTRIES; ++e) {
		SDL_Rect t = library_tab_rect(v, e);
		char s[24];
		tag_words(v, e, s, sizeof s);
		bool on = e == v->entry, whole = on && v->entry_of > 0 && v->entry_held >= v->entry_of;
		look_tag(x + t.x, y + t.y, t.w, t.h, "", on ? lit : rest);
		look_caps(x + t.x + 4, y + t.y + 2, s, whole ? LOOK_GOLD : on ? LOOK_PAGE : LOOK_TEXT, on ? LOOK_GREEN_LO : NONE);
	}
}

/* ---- the list: the page's slots ---- */

SDL_Rect library_row_rect(int i) { return (SDL_Rect){ FIELD_X, ROW_Y - 1 + i * ROW_H, FIELD_W, ROW_H }; }

SDL_Rect library_turn_rect(int right) { return (SDL_Rect){ right ? W - 20 : CARD_X + CARD_W, 16, 20, 16 }; }

/* a picture of `n` rows from (x, y): each letter of a row a colour of `pal` ('a' its first), '.' none */
static void marks(int x, int y, const char *const *rows, int n, const SDL_Color *pal) {
	for (int r = 0; r < n; ++r)
		for (int c = 0; rows[r][c]; ++c)
			if (rows[r][c] != '.') fill_rect(x + c, y + r, 1, 1, pal[rows[r][c] - 'a']);
}

/* a chip's rarity as BN6's list shows it, from (x, y): its stars, four pixels apart with their shadows under them, the
 * rest of the five as dots */
static void stars(int x, int y, int rarity) {
	static const char *const star[5] = { "..a..", "aaabb", ".caca", ".bcc.", "cb.cc" };
	static const SDL_Color pal[3] = { { 247, 255, 247, 255 }, { 255, 239, 115, 255 }, { 247, 198, 41, 255 } };
	static const SDL_Color shadow = { 24, 66, 82, 255 }, dot = { 148, 231, 222, 255 };
	static const SDL_Rect dark[5] = { { 4, 2, 2, 1 }, { 4, 3, 1, 1 }, { 2, 4, 1, 1 }, { 1, 5, 2, 1 }, { 4, 5, 2, 1 } };
	for (int s = 0; s < 5; ++s)
		if (s > rarity) fill_rect(x + s * 4 + 2, y + 2, 2, 2, dot);
		else for (int k = 0; k < 5; ++k) fill_rect(x + s * 4 + dark[k].x, y + dark[k].y, dark[k].w, dark[k].h, shadow);
	/* (the stars over the shadows, the left one last, as BN6 lays them) */
	for (int s = rarity < 4 ? rarity : 4; s >= 0; --s) marks(x + s * 4, y, star, 5, pal);
}

/* BN6's Library's cursor from (x, y): the PET's orange arrow with its tail, stepping as KeyItem's (one pixel right,
 * back, two right, six frames each) */
static void cursor(int x, int y, int t) {
	static const char *const rows[11] = { "aaaab", "accccb", "acccccb", "accccccb", "cdddccccb", "cdddccccde", "abbbcccdee", "acccccdee", "accccdee", "dddddee", "..eeee" };
	static const SDL_Color pal[5] = { { 255, 247, 115, 255 }, { 255, 206, 24, 255 }, { 255, 173, 16, 255 }, { 214, 107, 0, 255 }, { 16, 82, 82, 255 } };
	static const int step[3] = { 0, 1, 2 };
	marks(x + step[t / 6 % 3], y, rows, 11, pal);
}

/* a row from the picture's left at y: the cursor's arrow on the window's edge, the number, the chip's icon and name, its stars */
static void row(int x, int y, const LibSlot *s, bool seen, int lit_t) {
	if (lit_t >= 0) cursor(x + 91, y + 2, lit_t);
	char number[8];
	int digits = snprintf(number, sizeof number, "%d", s->number);
	text_draw_cells(x + 120 - 8 * digits, y + 1, number, LOOK_TEXT, LOOK_SHADE);
	if (!seen) {
		text_draw_cells(x + 152, y + 1, library_unseen, LOOK_TEXT, LOOK_SHADE);
		return;
	}
	second_chip_icon(s->chip, x + 120, y - 1);
	char name[24];
	chips_name(s->chip, name, sizeof name);
	text_draw_cells(x + 136, y + 1, name, LOOK_TEXT, LOOK_SHADE);
	const uint8_t *rec = chips_record(s->chip);
	/* (a name past BN6's eight letters leaves the stars no room) */
	if (rec && strlen(name) <= 8) stars(x + 201, y + 7, rec[BN6_CHIP_RARITY] > 4 ? 4 : rec[BN6_CHIP_RARITY]);
}

/* the page's count in its pill, as the Library's 12/200: right-ended at x, 12 tall, gold where the class is whole */
static void count_pill(int right, int y, const LibraryView *v) {
	char s[16];
	library_count_words(s, sizeof s, v->held, v->of);
	SDL_Color c = v->of > 0 && v->held >= v->of ? LOOK_GOLD : COUNT_EDGE;
	/* (48 wide as BN6's for its six digits, wider for seven; the digits the engine's own heavy ones, BN6's being a font of its ROM) */
	int w = look_title_width(s) + 8 < 48 ? 48 : look_title_width(s) + 8, x = right - w;
	fill_rect(x + 1, y, w - 2, 12, c);
	fill_rect(x, y + 1, w, 10, c);
	fill_rect(x + 2, y + 1, w - 4, 10, LOOK_NAVY);
	fill_rect(x + 1, y + 2, w - 2, 8, LOOK_NAVY);
	look_title(x + w - 4 - look_title_width(s), y + 3, s, v->of > 0 && v->held >= v->of ? LOOK_GOLD : LOOK_PAGE);
}

/* BN6's cyan panel from the card's edge to the screen's, the list's navy field sunk in it (blue along its top and
 * left, light along its foot and right), the next page's field a sliver at the screen's edge */
static void panel(int x, int y) {
	fill_rect(x + 87, y + 17, 152, 133, LOOK_EDGE);
	fill_rect(x + 83, y + 16, 157, 1, LOOK_EDGE_HI);
	fill_rect(x + 86, y + 150, 154, 1, LOOK_EDGE_LO);
	fill_rect(x + 83, y + 151, 157, 1, LOOK_GREEN_LO);
	fill_rect(x + 95, y + 31, 130, 1, LOOK_EDGE_LO);
	fill_rect(x + 94, y + 32, 1, 113, LOOK_EDGE_LO);
	fill_rect(x + 95, y + 32, 130, 113, LOOK_NAVY);
	fill_rect(x + 96, y + 145, 129, 1, LOOK_NAVY);
	fill_rect(x + 95, y + 145, 1, 1, LOOK_EDGE_HI);
	fill_rect(x + 96, y + 146, 129, 1, LOOK_EDGE_HI);
	fill_rect(x + 225, y + 32, 1, 114, LOOK_EDGE_HI);
	fill_rect(x + 239, y + 31, 1, 1, LOOK_EDGE_LO);
	fill_rect(x + 238, y + 32, 1, 113, LOOK_EDGE_LO);
	fill_rect(x + 239, y + 32, 1, 113, LOOK_NAVY);
	fill_rect(x + 239, y + 145, 1, 1, LOOK_EDGE_HI);
}

/* BN6's scroll bar right of the field: a navy track, light along its right, and a capsule on it at `first` of `n` lines' place */
static void scroll_bar(int x, int y, int first, int shown, int n) {
	static const SDL_Color rim = { 16, 82, 82, 255 }, lit = { 247, 247, 247, 255 }, glass = { 173, 231, 255, 255 }, shade = { 99, 156, 189, 255 },
		lamp = { 8, 181, 0, 255 }, lamp_lo = { 8, 123, 0, 255 };
	const int top = 29, travel = 100;
	int ty = y + top + (n > shown ? first * travel / (n - shown) : 0);
	fill_rect(x + 229, y + 42, 2, 100, LOOK_NAVY);
	fill_rect(x + 231, y + 42, 1, 100, LOOK_EDGE_HI);
	fill_rect(x + 229, y + 142, 3, 1, LOOK_EDGE_HI);
	/* (the capsule, 8 x 13: its rim, a lit left, the glass, a shaded right and foot, its lamp) */
	fill_rect(x + 227, ty, 6, 13, rim);
	fill_rect(x + 226, ty + 2, 8, 9, rim);
	fill_rect(x + 228, ty + 1, 4, 2, lit);
	fill_rect(x + 227, ty + 3, 1, 7, lit);
	fill_rect(x + 228, ty + 3, 4, 7, glass);
	fill_rect(x + 232, ty + 3, 1, 7, shade);
	fill_rect(x + 228, ty + 10, 4, 2, shade);
	fill_rect(x + 229, ty + 4, 2, 5, lamp_lo);
	fill_rect(x + 229, ty + 5, 1, 4, lamp);
}

/* the Library's page arrow, 8 x 8 from (x, y), in the colours its own screen gives it */
static void page_arrow(int x, int y, bool right) {
	static const char *const rows[8] = { "abbbb", "accccb", "acccccb", "accccccd", "acccccde", "accccde", "cdddde", ".eeee" };
	static const SDL_Color pal[5] = { { 247, 247, 247, 255 }, { 214, 255, 148, 255 }, { 156, 255, 57, 255 }, { 82, 222, 16, 255 }, { 16, 82, 82, 255 } };
	for (int r = 0; r < 8; ++r)
		for (int c = 0; rows[r][c]; ++c)
			if (rows[r][c] != '.') fill_rect(right ? x + c : x + 7 - c, y + r, 1, 1, pal[rows[r][c] - 'a']);
}

static void list_panel(int x, int y, const LibraryView *v) {
	panel(x, y);
	/* (the class in the band over its slots, between the page arrows where Left and Right turn to the page before and after, its count before the right one) */
	page_arrow(x + 87, y + 20, false);
	page_arrow(x + 226, y + 20, true);
	text_draw_cells(x + 96, y + 18, library_class_names[v->cls], LOOK_TEXT, LOOK_SHADE);
	count_pill(x + 224, y + 18, v);
	for (int i = 0; i < LIBRARY_ROWS && v->top + i < v->n; ++i)
		row(x, y + ROW_Y + i * ROW_H, &v->rows[v->top + i], v->seen[v->top + i], v->top + i == v->at ? v->t : -1);
	scroll_bar(x, y, v->top, LIBRARY_ROWS, v->n);
}

/* BN6's Library's screen: the PET's band and body to the screen's edges, no rail but the foot's */
static void frame(int x, int y) {
	look_screen(x, y, W, H, library_title);
	fill_rect(x, y, 7, LOOK_BAND_H, LOOK_BAND);
	fill_rect(x + W - 8, y, 8, LOOK_BAND_H, LOOK_BAND);
	fill_rect(x + W - 8, y + 6, 8, 2, LOOK_GREEN);
	fill_rect(x + W - 8, y + 10, 8, 2, LOOK_GREEN);
	fill_rect(x, y + LOOK_BAND_H, W, 1, LOOK_GREEN);
	fill_rect(x, y + LOOK_BAND_H + 1, LOOK_RAIL, H - LOOK_BAND_H - 1 - LOOK_RAIL, LOOK_GREEN);
	fill_rect(x + W - LOOK_RAIL, y + LOOK_BAND_H + 1, LOOK_RAIL, H - LOOK_BAND_H - 1 - LOOK_RAIL, LOOK_GREEN);
	fill_rect(x, y + H - 4, 1, 3, LOOK_METAL);
	fill_rect(x + W - 1, y + H - 4, 1, 3, LOOK_METAL);
}

void library_draw_view(int x, int y, const LibraryView *v) {
	frame(x, y);
	tags(x, y, v);
	list_panel(x, y, v);
	library_card(x + CARD_X, y + CARD_Y, v->n && v->seen[v->at] ? v->rows[v->at].chip : 0, v->cls);
}

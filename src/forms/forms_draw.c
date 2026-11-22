/* forms_draw.h. Nothing here is new art: the rows are CROSSSELECT's own,
 * read from the player's ROM as they are drawn (Falzar's made from it as
 * the core starts, cross_rows.c), and the rest is the PET's pieces
 * (pet_look.h). */
#include "forms_draw.h"

#include <stdio.h>

#include "forms_text.h"
#include "gfx.h"
#include "pet_look.h"
#include "xtiles.h"

#define W 240
#define H 160
#define NONE (SDL_Color){ 0, 0, 0, 0 }
#define PILL_EDGE rgba(181, 255, 189, 255)   /* the count's pill's outline, as the Library's 12/200 */

#define WIN_Y 17
#define WIN_TOP 7          /* the window's band over its field, its label in it */
#define ROW_W 72           /* CROSSSELECT's row: 9 x 2 tiles */
#define ROW_H 16
#define ROW_IN 11          /* a row in from its field's left, the orange arrow before it */
#define FIELD_W (ROW_IN + ROW_W + 1)
#define FIELD_H (FORMS_ACTIVE * ROW_H + 2)
#define WIN_W (FIELD_W + 2 * LOOK_FRAME)
#define WIN_H (FIELD_H + 2 * LOOK_FRAME + WIN_TOP)
#define FIVE_X 14
#define SPARE_X (W - 14 - WIN_W)
#define SOULS_X 14
#define SOULS_W (W - 28)
#define INFO_Y (WIN_Y + WIN_H + 3)
#define INFO_H (H - 3 - INFO_Y)
#define TAG_Y 3
#define TAG_H 9
#define PILL_W 30

static int win_x(int side) { return side == FORMS_FIVE ? FIVE_X : SPARE_X; }

SDL_Rect forms_cross_rect(int side, int row) {
	return (SDL_Rect){ win_x(side) + LOOK_FRAME, WIN_Y + LOOK_FRAME + WIN_TOP + 1 + row * ROW_H, FIELD_W, ROW_H };
}

SDL_Rect forms_soul_rect(int i) { return (SDL_Rect){ SOULS_X + LOOK_FRAME, WIN_Y + LOOK_FRAME + WIN_TOP + 1 + i * ROW_H, SOULS_W - 14, ROW_H }; }

SDL_Rect forms_tag_rect(int page) {
	/* (left of the count's pill, the Souls' tag the nearer) */
	int right = W - 6 - PILL_W - 4;
	SDL_Rect r = { 0, TAG_Y, 0, TAG_H };
	for (int p = FORMS_PAGES - 1; p >= page; --p) {
		r.w = look_caps_width(forms_word(p == FORMS_PAGE_SOULS ? FT_SOULS : FT_CROSSES)) + 8;
		r.x = right - r.w;
		right = r.x - 3;
	}
	return r;
}

/* the pages on the band, the one shown lit as NaviCust's RUN!, as the Library's entries */
static void tags(int x, int y, const FormsView *v) {
	static const SDL_Color lit = { 16, 231, 41, 255 }, rest = { 16, 82, 107, 255 };
	for (int p = 0; v->pages && p < FORMS_PAGES; ++p) {
		SDL_Rect t = forms_tag_rect(p);
		bool on = p == v->page;
		look_tag(x + t.x, y + t.y, t.w, t.h, "", on ? lit : rest);
		look_caps(x + t.x + 4, y + t.y + 2, forms_word(p == FORMS_PAGE_SOULS ? FT_SOULS : FT_CROSSES), on ? LOOK_PAGE : LOOK_TEXT,
			on ? LOOK_GREEN_LO : NONE);
	}
}

/* CROSSSELECT's rows filled, in a navy pill outlined light green at the band's right end, as FOLDER EDIT's 30/30 */
static void count_pill(int x, int y, int five) {
	char s[8];
	forms_count_words(s, sizeof s, five);
	int px = x + W - 6 - PILL_W, py = y + 1;
	fill_rect(px + 1, py, PILL_W - 2, 12, PILL_EDGE);
	fill_rect(px, py + 1, PILL_W, 10, PILL_EDGE);
	fill_rect(px + 1, py + 1, PILL_W - 2, 10, LOOK_NAVY);
	look_title(px + (PILL_W - look_title_width(s)) / 2, py + 3, s, five == FORMS_ACTIVE ? LOOK_PAGE : PILL_EDGE);
}

/* a Cross's row at (x, y): CROSSSELECT's art, in colour among the five and grey among the spares, its row's "chosen" palette once taken up */
static void cross_row(int x, int y, const FormsCross *c, int side, bool taken) {
	const uint8_t *art = side == FORMS_FIVE || taken ? c->art : c->grey, *pal = taken ? c->chosen : c->pal;
	if (art && pal) xtiles_draw(art, pal, x, y, 9, 2, 0);
	else look_text(x + 4, y + 2, c->name, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
}

static void cross_window(int x, int y, const FormsView *v, int side) {
	int wx = x + win_x(side);
	look_window(wx, y + WIN_Y, WIN_W, WIN_H, WIN_TOP, false);
	look_caps(wx + LOOK_FRAME + 1, y + WIN_Y + 3, forms_word(side == FORMS_FIVE ? FT_FIVE : FT_SPARE), LOOK_PAGE, LOOK_EDGE_LO);
	for (int r = 0; r < v->n[side]; ++r) {
		SDL_Rect at = forms_cross_rect(side, r);
		bool taken = v->taken_side == side && v->taken_row == r;
		cross_row(x + at.x + ROW_IN, y + at.y, &v->run.crosses[v->col[side][r]], side, taken);
		if (v->page == FORMS_PAGE_CROSSES && side == v->side && r == v->row) look_cursor(x + at.x + 1, y + at.y + 3);
	}
}

static void souls_window(int x, int y, const FormsView *v) {
	bool bar = v->run.n_souls > FORMS_SOUL_ROWS;
	look_window(x + SOULS_X, y + WIN_Y, SOULS_W, WIN_H, WIN_TOP, bar);
	look_caps(x + SOULS_X + LOOK_FRAME + 1, y + WIN_Y + 3, forms_word(FT_SOULS), LOOK_PAGE, LOOK_EDGE_LO);
	for (int i = 0; i < FORMS_SOUL_ROWS && v->soul_top + i < v->run.n_souls; ++i) {
		SDL_Rect at = forms_soul_rect(i);
		const FormsSoul *s = &v->run.souls[v->soul_top + i];
		text_draw_cells(x + at.x + ROW_IN, y + at.y + 2, s->name, LOOK_TEXT, LOOK_SHADE);
		look_text(x + at.x + at.w - 4, y + at.y + 2, s->kind, LOOK_TEXT, LOOK_SHADE, TEXT_RIGHT);
		if (v->soul_top + i == v->soul_at) look_cursor(x + at.x + 1, y + at.y + 3);
	}
	look_scroll((SDL_Rect){ x + SOULS_X, y + WIN_Y, SOULS_W, WIN_H }, WIN_TOP, v->soul_top, FORMS_SOUL_ROWS, v->run.n_souls);
}

/* KeyItem's INFORMATION box: what the cursor's row is, two lines on its white page */
static void info(int x, int y, const FormsView *v) {
	SDL_Rect r = look_info(x + FIVE_X, y + INFO_Y, W - 2 * FIVE_X, INFO_H, forms_word(FT_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	char s[160];
	snprintf(s, sizeof s, "%s", forms_word(FT_NONE));
	if (v->page == FORMS_PAGE_SOULS && v->run.n_souls) {
		const FormsSoul *so = &v->run.souls[v->soul_at];
		forms_soul_words(s, sizeof s, so->name, so->kind);
	} else if (v->page == FORMS_PAGE_CROSSES && v->n[v->side]) {
		const FormsCross *c = &v->run.crosses[v->col[v->side][v->row]];
		if (v->taken_side >= 0 && v->taken_side != v->side)
			forms_taken_words(s, sizeof s, v->run.crosses[v->col[v->taken_side][v->taken_row]].name, c->name);
		else forms_cross_words(s, sizeof s, c->name, c->weak, c->strong);
	}
	LookLines at = { r.x + 5, r.y + 1, r.w - 10, 0, 2, LOOK_INK, LOOK_INK_SHADE };
	look_wrapped_from(s, &at);
}

void forms_draw_view(int x, int y, const FormsView *v) {
	look_screen(x, y, W, H, forms_word(FT_TITLE));
	tags(x, y, v);
	if (v->page == FORMS_PAGE_SOULS) souls_window(x, y, v);
	else {
		count_pill(x, y, v->n[FORMS_FIVE]);
		cross_window(x, y, v, FORMS_FIVE);
		cross_window(x, y, v, FORMS_SPARE);
	}
	info(x, y, v);
}

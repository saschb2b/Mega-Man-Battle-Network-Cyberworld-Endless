/* forms_screen.h. The Crosses' page is a deck of five over what the run holds:
 * the five are always CROSSSELECT's rows filled (every Cross held where
 * five or fewer are), so a change is a swap, one in for one out, and the
 * deck is never left short. */
#include "forms_screen.h"

#include <string.h>

#include "audio.h"
#include "buttons.h"
#include "forms_draw.h"
#include "game.h"
#include "pet_look.h"
#include "platform.h"

static struct {
	bool shown, letting_go;
	FormsView v;
} F;

static int bits(uint16_t m) {
	int n = 0;
	for (; m; m &= (uint16_t)(m - 1)) ++n;
	return n;
}

uint16_t forms_five(uint16_t held, uint16_t marked) {
	uint16_t five = marked & held;
	while (bits(five) > FORMS_ACTIVE) five &= (uint16_t)(five - 1);
	for (int i = 0; i < FORMS_CROSSES && bits(five) < FORMS_ACTIVE; ++i) five |= (uint16_t)(held & 1u << i);
	return five;
}

static uint16_t held_of(const FormsRun *r) {
	uint16_t held = 0;
	for (int i = 0; i < r->n_crosses; ++i) held |= (uint16_t)(1u << r->crosses[i].cross);
	return held;
}

/* the two windows' rows from the five */
static void columns(void) {
	FormsView *v = &F.v;
	v->n[FORMS_FIVE] = v->n[FORMS_SPARE] = 0;
	for (int i = 0; i < v->run.n_crosses; ++i) {
		int side = v->run.active >> v->run.crosses[i].cross & 1 ? FORMS_FIVE : FORMS_SPARE;
		v->col[side][v->n[side]++] = i;
	}
}

/* the cursor kept on a row of its side, the spares' empty side never taken */
static void cursor_to(int side, int row) {
	FormsView *v = &F.v;
	if (!v->n[side]) side = FORMS_FIVE;
	v->side = side;
	v->row = row < 0 ? 0 : row >= v->n[side] ? (v->n[side] ? v->n[side] - 1 : 0) : row;
}

/* the run's index of the Cross at a side's row */
static int at(int side, int row) { return F.v.col[side][row]; }

/* the Cross taken up and the one under the cursor, on the other side, change places */
static void swap(void) {
	FormsView *v = &F.v;
	int taken = at(v->taken_side, v->taken_row), there = at(v->side, v->row);
	v->run.active ^= (uint16_t)(1u << v->run.crosses[taken].cross | 1u << v->run.crosses[there].cross);
	v->taken_side = -1;
	columns();
	/* (the cursor stays on its side, on the Cross come into it) */
	for (int r = 0; r < v->n[v->side]; ++r)
		if (at(v->side, r) == taken) v->row = r;
	if (v->run.choose) v->run.choose(v->run.active);
	audio_sfx(SFX_CONFIRM);
	bool into_five = v->side == FORMS_FIVE;
	printf("forms: %s in, %s out\n", v->run.crosses[into_five ? taken : there].name, v->run.crosses[into_five ? there : taken].name);
}

/* A on the cursor's Cross: taken up (and the cursor to the other side), or
 * swapped with the one taken up on the other side */
static void take(void) {
	FormsView *v = &F.v;
	if (!v->n[FORMS_SPARE]) { audio_sfx(SFX_ERROR); return; }
	if (v->taken_side >= 0 && v->taken_side != v->side) { swap(); return; }
	v->taken_side = v->side;
	v->taken_row = v->row;
	audio_sfx(SFX_SELECT);
	cursor_to(!v->side, v->row);
}

static void page_to(int page) {
	FormsView *v = &F.v;
	if (!v->pages || page == v->page) return;
	v->page = page;
	v->taken_side = -1;
	audio_sfx(SFX_SELECT);
}

static void close_it(void) {
	F.shown = false;
	F.letting_go = true;
	platform_picture_taps(false);
	audio_sfx(SFX_DESC_CLOSE);
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }

/* a tap on the picture: a page's tag; a row takes the cursor, and a tap on the cursor's own row is A */
static void taps(void) {
	FormsView *v = &F.v;
	int x, y;
	if (!platform_tap(&x, &y)) return;
	x -= P.core_x;
	y -= P.core_y;
	for (int p = 0; v->pages && p < FORMS_PAGES; ++p)
		if (inside(forms_tag_rect(p), x, y)) { page_to(p); return; }
	if (v->page == FORMS_PAGE_SOULS) {
		for (int i = 0; i < FORMS_SOUL_ROWS && v->soul_top + i < v->run.n_souls; ++i)
			if (inside(forms_soul_rect(i), x, y)) v->soul_at = v->soul_top + i;
		return;
	}
	for (int s = 0; s < FORMS_SIDES; ++s)
		for (int r = 0; r < v->n[s]; ++r)
			if (inside(forms_cross_rect(s, r), x, y)) {
				if (s == v->side && r == v->row) take();
				else { cursor_to(s, r); audio_sfx(SFX_CURSOR); }
				return;
			}
}

static void soul_keys(void) {
	FormsView *v = &F.v;
	int to = v->soul_at + (btn_repeat(BTN_DOWN) ? 1 : 0) - (btn_repeat(BTN_UP) ? 1 : 0);
	if (to < 0 || to >= v->run.n_souls || to == v->soul_at) return;
	v->soul_at = to;
	if (to < v->soul_top) v->soul_top = to;
	if (to >= v->soul_top + FORMS_SOUL_ROWS) v->soul_top = to - FORMS_SOUL_ROWS + 1;
	audio_sfx(SFX_CURSOR);
}

static void cross_keys(void) {
	FormsView *v = &F.v;
	if (btn_repeat(BTN_UP) && v->row > 0) { --v->row; audio_sfx(SFX_CURSOR); }
	else if (btn_repeat(BTN_DOWN) && v->row < v->n[v->side] - 1) { ++v->row; audio_sfx(SFX_CURSOR); }
	else if (btn_pressed(BTN_LEFT | BTN_RIGHT)) {
		int to = btn_pressed(BTN_LEFT) ? FORMS_FIVE : FORMS_SPARE;
		if (to != v->side && v->n[to]) { cursor_to(to, v->row); audio_sfx(SFX_CURSOR); }
	}
	if (btn_pressed(BTN_A) && v->n[v->side]) take();
}

static void keys(void) {
	FormsView *v = &F.v;
	if (btn_pressed(BTN_L)) page_to(FORMS_PAGE_CROSSES);
	else if (btn_pressed(BTN_R)) page_to(FORMS_PAGE_SOULS);
	if (v->page == FORMS_PAGE_SOULS) soul_keys();
	else cross_keys();
	/* (B puts back a Cross taken up, the cursor on it again; else it leaves, and START as it leaves BN6's Library) */
	if (btn_pressed(BTN_B) && v->taken_side >= 0) {
		cursor_to(v->taken_side, v->taken_row);
		v->taken_side = -1;
		audio_sfx(SFX_CANCEL);
	} else if (btn_pressed(BTN_B | BTN_START)) close_it();
}

void forms_open(const FormsRun *run) {
	memset(&F.v, 0, sizeof F.v);
	FormsView *v = &F.v;
	v->run = *run;
	if (v->run.n_crosses > FORMS_CROSSES) v->run.n_crosses = FORMS_CROSSES;
	if (v->run.n_souls > FORMS_SOULS) v->run.n_souls = FORMS_SOULS;
	v->run.active = forms_five(held_of(&v->run), run->active);
	v->pages = v->run.n_crosses && v->run.n_souls;
	v->page = !v->run.n_crosses && v->run.n_souls ? FORMS_PAGE_SOULS : FORMS_PAGE_CROSSES;
	v->taken_side = -1;
	columns();
	cursor_to(FORMS_FIVE, 0);
	F.shown = true;
	F.letting_go = false;
	platform_picture_taps(true);
	audio_sfx(SFX_SELECT);
	printf("forms: open, %d Crosses (%d in CROSSSELECT), %d Souls\n", v->run.n_crosses, v->n[FORMS_FIVE], v->run.n_souls);
}

bool forms_shown(void) { return F.shown; }

const FormsRun *forms_run(void) { return F.shown ? &F.v.run : NULL; }

/* (a side's Crosses by name, apart by commas, "none" for none) */
static void side_words(FILE *f, int side) {
	const FormsView *v = &F.v;
	for (int r = 0; r < v->n[side]; ++r) fprintf(f, "%s%s", r ? "," : "", v->run.crosses[at(side, r)].name);
	if (!v->n[side]) fprintf(f, "none");
}

void forms_describe(FILE *f) {
	const FormsView *v = &F.v;
	if (!F.shown) return;
	fprintf(f, "forms page %s five ", v->page == FORMS_PAGE_SOULS ? "souls" : "crosses");
	side_words(f, FORMS_FIVE);
	fprintf(f, " spare ");
	side_words(f, FORMS_SPARE);
	if (v->page == FORMS_PAGE_SOULS) fprintf(f, " at %s", v->run.n_souls ? v->run.souls[v->soul_at].name : "none");
	else fprintf(f, " at %s", v->n[v->side] ? v->run.crosses[at(v->side, v->row)].name : "none");
	fprintf(f, " taken %s souls %d\n", v->taken_side >= 0 ? v->run.crosses[at(v->taken_side, v->taken_row)].name : "none", v->run.n_souls);
}

void forms_shut(void) {
	if (!F.shown) return;
	F.shown = F.letting_go = false;
	platform_picture_taps(false);
}

bool forms_update(void) {
	if (!F.shown) {
		if (F.letting_go && btn_held(BTN_A | BTN_B | BTN_START | BTN_R)) return true;
		F.letting_go = false;
		return false;
	}
	/* (its first frame's keys are the ones that opened it) */
	if (F.v.t > 0) {
		keys();
		if (F.shown) taps();
	}
	++F.v.t;
	return true;
}

void forms_draw(void) {
	if (!F.shown) return;
	forms_draw_view(P.core_x, P.core_y, &F.v);
	look_fade_in(P.core_x, P.core_y, CORE_W, CORE_H, F.v.t);
}

bool forms_cursor(int *cross, int *soul) {
	const FormsView *v = &F.v;
	*cross = *soul = -1;
	if (!F.shown) return false;
	if (v->page == FORMS_PAGE_SOULS) *soul = v->run.n_souls ? v->soul_at : -1;
	else *cross = v->n[v->side] ? at(v->side, v->row) : -1;
	return true;
}

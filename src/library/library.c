/* library.h. */
#include "library.h"

#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "buttons.h"
#include "devtools.h"
#include "game.h"
#include "library_draw.h"
#include "library_text.h"
#include "meta.h"
#include "pet_look.h"
#include "platform.h"

static struct {
	bool shown, letting_go;
	LibraryView v;
} L;

/* the page made: its slots listed (a StdChip page's every one, the others' the filled alone, as BN6's tabs) and counted */
static void page_make(void) {
	LibraryView *v = &L.v;
	static LibSlot all[LIB_SLOTS_MOST];
	int n = library_page(v->entry, v->cls, all, LIB_SLOTS_MOST);
	v->n = 0;
	for (int i = 0; i < n; ++i) {
		bool seen = meta_library_has(all[i].chip);
		if (!seen && v->cls != LIB_STD) continue;
		v->rows[v->n] = all[i];
		v->seen[v->n++] = seen;
	}
	v->at = v->top = 0;
	library_count(v->entry, v->cls, &v->held, &v->of);
	library_count(v->entry, -1, &v->entry_held, &v->entry_of);
}

static void cursor_to(int at) {
	LibraryView *v = &L.v;
	v->at = at < 0 ? 0 : at >= v->n ? (v->n ? v->n - 1 : 0) : at;
	if (v->at < v->top) v->top = v->at;
	if (v->at >= v->top + LIBRARY_ROWS) v->top = v->at - LIBRARY_ROWS + 1;
}

static void entry_to(int entry) {
	L.v.entry = (entry + LIB_ENTRIES) % LIB_ENTRIES;
	L.v.cls = LIB_STD;
	page_make();
	audio_sfx(SFX_SELECT);
}

/* the page before or after, as BN6's Left and Right turn its tabs: an entry's three classes, then the next entry's */
static void page_turn(int by) {
	int pages = LIB_ENTRIES * LIB_CLASSES, at = (L.v.entry * LIB_CLASSES + L.v.cls + by + pages) % pages;
	L.v.entry = at / LIB_CLASSES;
	L.v.cls = at % LIB_CLASSES;
	page_make();
	audio_sfx(SFX_SELECT);
}

/* a screenful up or down, as BN6's L and R: the list moves, the cursor keeps its row on the screen */
static void list_page(int by) {
	LibraryView *v = &L.v;
	int most = v->n > LIBRARY_ROWS ? v->n - LIBRARY_ROWS : 0, top = v->top + by * LIBRARY_ROWS;
	top = top < 0 ? 0 : top > most ? most : top;
	if (top == v->top) return;
	v->at += top - v->top;
	v->top = top;
	audio_sfx(SFX_CURSOR);
}

static void close_it(void) {
	L.shown = false;
	L.letting_go = true;
	platform_picture_taps(false);
	audio_sfx(SFX_DESC_CLOSE);
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }

/* a tap on the picture: a row takes the cursor, a tab its entry, the arrows turn the page */
static void taps(void) {
	int x, y;
	if (!platform_tap(&x, &y)) return;
	x -= P.core_x;
	y -= P.core_y;
	for (int i = 0; i < LIBRARY_ROWS && L.v.top + i < L.v.n; ++i)
		if (inside(library_row_rect(i), x, y)) { cursor_to(L.v.top + i); return; }
	for (int e = 0; e < LIB_ENTRIES; ++e)
		if (inside(library_tab_rect(e), x, y)) { if (e != L.v.entry) entry_to(e); return; }
	for (int r = 0; r < 2; ++r)
		if (inside(library_turn_rect(r), x, y)) { page_turn(r ? 1 : -1); return; }
}

static void keys(void) {
	LibraryView *v = &L.v;
	if (btn_repeat(BTN_UP) && v->at > 0) { cursor_to(v->at - 1); audio_sfx(SFX_CURSOR); }
	else if (btn_repeat(BTN_DOWN) && v->at < v->n - 1) { cursor_to(v->at + 1); audio_sfx(SFX_CURSOR); }
	if (btn_repeat(BTN_L)) list_page(-1);
	else if (btn_repeat(BTN_R)) list_page(1);
	else if (btn_pressed(BTN_LEFT)) page_turn(-1);
	else if (btn_pressed(BTN_RIGHT)) page_turn(1);
	/* (B leaves, and START as it leaves BN6's own) */
	if (btn_pressed(BTN_B | BTN_START)) close_it();
}

void library_open(void) {
	memset(&L.v, 0, sizeof L.v);
	/* (--dev library=N: every Nth chip id held, for the pages' pictures) */
	for (int id = dev.library; dev.library > 0 && id < META_LIBRARY_IDS; id += dev.library) meta_library_add(id);
	L.shown = true;
	L.letting_go = false;
	page_make();
	platform_picture_taps(true);
	audio_sfx(SFX_SELECT);
	printf("library: open, BN6 %d/%d\n", L.v.entry_held, L.v.entry_of);
}

bool library_shown(void) { return L.shown; }

void library_shut(void) {
	if (!L.shown) return;
	L.shown = L.letting_go = false;
	platform_picture_taps(false);
}

bool library_update(void) {
	if (!L.shown) {
		if (L.letting_go && btn_held(BTN_A | BTN_B | BTN_START)) return true;
		L.letting_go = false;
		return false;
	}
	/* (its first frame's keys are the ones that opened it) */
	if (L.v.t > 0) {
		keys();
		if (L.shown) taps();
	}
	++L.v.t;
	return true;
}

void library_draw(void) {
	if (!L.shown) return;
	library_draw_view(P.core_x, P.core_y, &L.v);
	look_fade_in(P.core_x, P.core_y, CORE_W, CORE_H, L.v.t);
}

bool library_cursor(int *chip, bool *unseen) {
	*chip = 0;
	*unseen = false;
	if (!L.shown) return false;
	if (L.v.n) {
		*unseen = !L.v.seen[L.v.at];
		*chip = *unseen ? 0 : L.v.rows[L.v.at].chip;
	}
	return true;
}

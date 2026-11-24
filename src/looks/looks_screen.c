/* looks_screen.h. The look is the profile's (looks.h): the screen sets the
 * slot worn and its colours as the cursor moves and picks, and applies
 * each change to the core's ROM copy in its update (look_apply), never in
 * its draw. */
#include "looks_screen.h"

#include <string.h>

#include "audio.h"
#include "buttons.h"
#include "game.h"
#include "look_apply.h"
#include "looks.h"
#include "looks_draw.h"
#include "pet_look.h"
#include "platform.h"

static struct {
	bool shown, letting_go;
	LooksView v;
} S;

/* the look as it stands into the game, and the preview's colours from it */
static void apply(void) {
	look_apply(&S.v.look);
	looks_preview_colours(&S.v.preview, &S.v.look);
}

/* the channel under the list's cursor, -1 on the looks' row */
static int channel(void) { return S.v.row > 0 ? S.v.chan[S.v.row - 1] : -1; }

/* the palette a channel's row shows: its own, else Classic */
static int page_of(int c) {
	if (c < 0) return S.v.page;
	uint8_t p = S.v.look.palette[c];
	return p == LOOK_NONE ? LOOK_CLASSIC : p;
}

/* the grid's cell a channel has on page p: its colour there, the plain cell where it has none */
static int cell_of(int c, int p) {
	const Look *l = &S.v.look;
	if (l->palette[c] == LOOK_NONE) return LOOKS_PLAIN;
	return l->palette[c] == p ? l->colour[c] : 0;
}

/* the grid's cursor on him: the cell's colour on the channel, his own where the page is locked */
static void try_cell(void) {
	LooksView *v = &S.v;
	int c = channel();
	v->look = v->before;
	if (v->cell == LOOKS_PLAIN) v->look.palette[c] = v->look.colour[c] = LOOK_NONE;
	else if (looks_owned(v->page)) {
		v->look.palette[c] = (uint8_t)v->page;
		v->look.colour[c] = (uint8_t)v->cell;
	}
	apply();
}

static void slot_to(int slot) {
	LooksView *v = &S.v;
	slot = (slot + LOOK_SLOTS) % LOOK_SLOTS;
	if (slot == v->slot) return;
	v->slot = slot;
	looks_select(slot);
	v->look = looks_get(slot);
	apply();
	audio_sfx(SFX_CURSOR);
	printf("looks: look %d worn\n", slot);
}

static void row_to(int row) {
	LooksView *v = &S.v;
	if (row < 0 || row > v->n_chan || row == v->row) return;
	v->row = row;
	if (row > 0) v->page = page_of(channel());
	audio_sfx(SFX_CURSOR);
}

static void page_by(int d) {
	LooksView *v = &S.v;
	v->page = (v->page + d + LOOK_PALETTES) % LOOK_PALETTES;
	audio_sfx(SFX_SELECT);
	if (v->grid) try_cell();
}

/* A on a channel: into the grid, its cursor on the channel's colour */
static void enter(void) {
	LooksView *v = &S.v;
	if (v->row == 0) return;
	v->grid = true;
	v->before = v->look;
	v->cell = cell_of(channel(), v->page);
	audio_sfx(SFX_SELECT);
}

/* A in the grid: the colour kept, back to the list; a locked palette's buzzes */
static void pick(void) {
	LooksView *v = &S.v;
	if (v->cell != LOOKS_PLAIN && !looks_owned(v->page)) {
		audio_sfx(SFX_ERROR);
		return;
	}
	try_cell();
	looks_set(v->slot, &v->look);
	v->grid = false;
	audio_sfx(SFX_CONFIRM);
	printf("looks: %s set\n", channel() == 0 ? "armour" : channel() == 1 ? "suit" : "accent");
}

/* B in the grid: the colour as it was */
static void back(void) {
	LooksView *v = &S.v;
	v->look = v->before;
	v->grid = false;
	apply();
	audio_sfx(SFX_CANCEL);
}

static void close_it(void) {
	S.shown = false;
	S.letting_go = true;
	platform_picture_taps(false);
	audio_sfx(SFX_DESC_CLOSE);
}

/* the grid's cursor: four across, four down, the plain cell under them all */
static void grid_keys(void) {
	LooksView *v = &S.v;
	int to = v->cell;
	if (btn_repeat(BTN_LEFT) && v->cell != LOOKS_PLAIN && v->cell % 4) to = v->cell - 1;
	else if (btn_repeat(BTN_RIGHT) && v->cell != LOOKS_PLAIN && v->cell % 4 < 3) to = v->cell + 1;
	else if (btn_repeat(BTN_UP)) to = v->cell == LOOKS_PLAIN ? 12 : v->cell >= 4 ? v->cell - 4 : v->cell;
	else if (btn_repeat(BTN_DOWN)) to = v->cell == LOOKS_PLAIN ? v->cell : v->cell >= 12 ? LOOKS_PLAIN : v->cell + 4;
	if (to != v->cell) {
		v->cell = to;
		audio_sfx(SFX_CURSOR);
		try_cell();
	}
	if (btn_pressed(BTN_A)) pick();
	else if (btn_pressed(BTN_B)) back();
}

static void list_keys(void) {
	LooksView *v = &S.v;
	if (btn_repeat(BTN_UP)) row_to(v->row - 1);
	else if (btn_repeat(BTN_DOWN)) row_to(v->row + 1);
	else if (v->row == 0 && btn_pressed(BTN_LEFT)) slot_to(v->slot - 1);
	else if (v->row == 0 && btn_pressed(BTN_RIGHT)) slot_to(v->slot + 1);
	if (btn_pressed(BTN_A)) enter();
	else if (btn_pressed(BTN_B | BTN_START)) close_it();
}

static bool inside(SDL_Rect r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }

/* a tap: a row takes the cursor (the looks' row turns to the next look), a
 * cell takes the grid's cursor (a tap on it again is A), an arrow pages */
static void taps(void) {
	LooksView *v = &S.v;
	int x, y;
	if (!platform_tap(&x, &y)) return;
	x -= P.core_x;
	y -= P.core_y;
	for (int d = 0; d < 2; ++d)
		if (inside(looks_page_rect(d), x, y)) { page_by(d ? 1 : -1); return; }
	if (v->row > 0)
		for (int c = 0; c <= LOOKS_PLAIN; ++c)
			if (inside(looks_cell_rect(c), x, y)) {
				if (!v->grid) enter();
				if (c == v->cell) pick();
				else { v->cell = c; audio_sfx(SFX_CURSOR); try_cell(); }
				return;
			}
	for (int r = 0; r <= v->n_chan; ++r)
		if (inside(looks_row_rect(r), x, y)) {
			if (v->grid) back();
			if (r == 0 && v->row == 0) slot_to(v->slot + 1);
			else row_to(r);
			return;
		}
}

void looks_screen_open(void) {
	memset(&S.v, 0, sizeof S.v);
	LooksView *v = &S.v;
	for (int c = 0; c < LOOK_CHANNELS; ++c) v->chan[v->n_chan++] = c;
	v->slot = looks_slot();
	v->look = looks_get(v->slot);
	v->page = LOOK_CLASSIC;
	looks_preview_start(&v->preview);
	apply();
	S.shown = true;
	S.letting_go = false;
	platform_picture_taps(true);
	audio_sfx(SFX_SELECT);
	printf("looks: open, look %d, %d channels\n", v->slot, v->n_chan);
}

bool looks_screen_shown(void) { return S.shown; }

void looks_screen_shut(void) {
	if (!S.shown) return;
	if (S.v.grid) {
		S.v.look = S.v.before;
		apply();
	}
	S.shown = S.letting_go = false;
	platform_picture_taps(false);
}

bool looks_screen_update(void) {
	if (!S.shown) {
		if (S.letting_go && btn_held(BTN_A | BTN_B | BTN_START)) return true;
		S.letting_go = false;
		return false;
	}
	/* (its first frame's keys are the ones that opened it) */
	if (S.v.t > 0) {
		if (btn_pressed(BTN_L)) page_by(-1);
		else if (btn_pressed(BTN_R)) page_by(1);
		if (S.v.grid) grid_keys();
		else list_keys();
		if (S.shown) taps();
	}
	looks_preview_step(&S.v.preview);
	++S.v.t;
	return true;
}

void looks_screen_draw(void) {
	if (!S.shown) return;
	looks_draw_view(P.core_x, P.core_y, &S.v);
	look_fade_in(P.core_x, P.core_y, CORE_W, CORE_H, S.v.t);
}

/* (a channel's colour: PALETTE/N, or plain) */
static void colour_words(FILE *f, const Look *l, int c) {
	if (l->palette[c] == LOOK_NONE) fprintf(f, " %s plain", c == 0 ? "armour" : c == 1 ? "suit" : "accent");
	else fprintf(f, " %s %s/%d", c == 0 ? "armour" : c == 1 ? "suit" : "accent", looks_palette_name(l->palette[c]), l->colour[c]);
}

void looks_screen_describe(FILE *f) {
	Look worn = looks_worn();
	fprintf(f, "look %c", 'A' + looks_slot());
	for (int c = 0; c < LOOK_CHANNELS; ++c) colour_words(f, &worn, c);
	fprintf(f, "\n");
	if (!S.shown) return;
	const LooksView *v = &S.v;
	int c = v->row > 0 ? v->chan[v->row - 1] : -1;
	fprintf(f, "looks at %s %s page %s%s", c < 0 ? "look" : c == 0 ? "armour" : c == 1 ? "suit" : "accent", v->grid ? "grid" : "list",
		looks_palette_name(v->page), looks_owned(v->page) ? "" : " locked");
	if (v->grid) {
		if (v->cell == LOOKS_PLAIN) fprintf(f, " cell plain");
		else fprintf(f, " cell %d", v->cell);
	}
	fprintf(f, "\n");
}

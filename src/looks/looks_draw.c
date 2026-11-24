/* looks_draw.h. Nothing here is new art: MegaMan is BN6's own battle
 * sprite, read from the player's ROM as it is drawn, in his palette as the
 * look makes it; the colours are the palettes' own numbers (looks.h); the
 * rest is the PET's pieces (pet_look.h). */
#include "looks_draw.h"

#include <stdio.h>

#include "gfx.h"
#include "looks_screen_text.h"
#include "pet_look.h"

#define W 240
#define H 160
#define NONE (SDL_Color){ 0, 0, 0, 0 }
#define ORANGE rgba(255, 170, 40, 255)   /* Options' pointers beside a value the d-pad turns */

#define TOP 19
#define MID_H 92            /* the slot's and the windows' height */
#define SLOT_X 8
#define SLOT_W 44
#define LIST_X 55
#define LIST_W 88
#define PAL_X 146
#define PAL_W 86
#define PAL_TOP 7           /* the palette window's band over its field, its name in it */
#define ROW_H 18
#define ROW_IN 11           /* a row's words from its field's left, the orange arrow before them */
#define SWATCH 10
#define CELL 13             /* a grid's cell, CELL_STEP apart */
#define CELL_STEP 14
#define GRID_W (4 * CELL_STEP - 1)
#define PLAIN_H 11
#define INFO_Y (TOP + MID_H + 3)
#define INFO_H (H - 3 - INFO_Y)

/* a GBA colour (R in the low bits) as the canvas's */
static SDL_Color colour(uint16_t c) {
	int r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
	return (SDL_Color){ (Uint8)(r << 3 | r >> 2), (Uint8)(g << 3 | g >> 2), (Uint8)(b << 3 | b >> 2), 255 };
}

static SDL_Color half(SDL_Color c, SDL_Color to) {
	return (SDL_Color){ (Uint8)((c.r + to.r) / 2), (Uint8)((c.g + to.g) / 2), (Uint8)((c.b + to.b) / 2), 255 };
}

static SDL_Rect list_field(void) { return (SDL_Rect){ LIST_X + LOOK_FRAME, TOP + LOOK_FRAME, LIST_W - 2 * LOOK_FRAME, MID_H - 2 * LOOK_FRAME }; }

static SDL_Rect pal_field(void) {
	return (SDL_Rect){ PAL_X + LOOK_FRAME, TOP + LOOK_FRAME + PAL_TOP, PAL_W - 2 * LOOK_FRAME, MID_H - 2 * LOOK_FRAME - PAL_TOP };
}

SDL_Rect looks_row_rect(int row) {
	SDL_Rect f = list_field();
	return (SDL_Rect){ f.x, f.y + 3 + row * ROW_H, f.w, ROW_H };
}

SDL_Rect looks_cell_rect(int cell) {
	SDL_Rect f = pal_field();
	int gx = f.x + (f.w - GRID_W) / 2, gy = f.y + 3;
	if (cell >= LOOKS_PLAIN) return (SDL_Rect){ gx, gy + 4 * CELL_STEP + 1, GRID_W, PLAIN_H };
	return (SDL_Rect){ gx + cell % 4 * CELL_STEP, gy + cell / 4 * CELL_STEP, CELL, CELL };
}

SDL_Rect looks_page_rect(bool right) {
	return (SDL_Rect){ right ? PAL_X + PAL_W - LOOK_FRAME - LOOK_ARROW_W - 1 : PAL_X + LOOK_FRAME + 1, TOP + 2, LOOK_ARROW_W + 2, PAL_TOP + 3 };
}

/* the channel's colour as he shows it: the colour picked, else BN6's own mid shade of its ramp */
static SDL_Color channel_colour(const LooksView *v, int c) {
	static const int mid[LOOK_CHANNELS] = { 3, 7, 11 };   /* (battle palette 0: the armour's ramp 2-5, the suit's 6-9, the gold 11) */
	if (v->look.palette[c] != LOOK_NONE) return colour(looks_colour(v->look.palette[c], v->look.colour[c]));
	const uint8_t *pal = v->preview.pal[1];
	return v->preview.have[1] ? colour((uint16_t)(pal[2 * mid[c]] | pal[2 * mid[c] + 1] << 8)) : LOOK_PAGE;
}

/* STATUS's slot with BN6's MegaMan standing in it, in the look's colours */
static void slot(int x, int y, const LooksView *v) {
	SDL_Rect f = look_slot(x + SLOT_X, y + TOP, SLOT_W, MID_H);
	looks_preview_draw(&v->preview, f.x + f.w / 2 - 2, f.y + f.h - 16);
	char s[8];
	snprintf(s, sizeof s, "%s", looks_slot_name(v->slot));
	look_title(f.x + (f.w - look_title_width(s)) / 2, f.y + f.h - 10, s, LOOK_PAGE);
}

static void list(int x, int y, const LooksView *v) {
	look_window(x + LIST_X, y + TOP, LIST_W, MID_H, 0, false);
	SDL_Rect f = list_field();
	for (int r = 0; r <= v->n_chan; ++r) {
		SDL_Rect at = looks_row_rect(r);
		int ry = y + at.y + 3, right = x + f.x + f.w - 6;
		look_text(x + at.x + ROW_IN, ry, r ? looks_channel_name(v->chan[r - 1]) : looks_word(LW_LOOK), LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		if (r == v->row) look_cursor(x + at.x + 1, ry + 2);
		if (!r) {
			/* (the look worn between Options' pointers, as a value Left and Right turn) */
			const char *s = looks_slot_name(v->slot);
			int l = right - 4 - look_text_width(s);
			look_text(l, ry, s, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
			look_pointer(l - 5, ry, 4, true, ORANGE);
			look_pointer(right, ry, 4, false, ORANGE);
			continue;
		}
		int sx = right - SWATCH + 4, sy = ry + 1;
		fill_rect(sx - 1, sy - 1, SWATCH + 2, SWATCH + 2, LOOK_PAGE);
		fill_rect(sx, sy, SWATCH, SWATCH, channel_colour(v, v->chan[r - 1]));
	}
}

static void cell(int x, int y, const LooksView *v, int i, bool owned) {
	SDL_Rect r = looks_cell_rect(i);
	r.x += x;
	r.y += y;
	if (i == LOOKS_PLAIN) {
		fill_rect(r.x, r.y, r.w, r.h, LOOK_NAVY_LO);
		look_caps(r.x + (r.w - look_caps_width(looks_word(LW_PLAIN))) / 2, r.y + 3, looks_word(LW_PLAIN), LOOK_TEXT, LOOK_SHADE);
		return;
	}
	SDL_Color c = colour(looks_colour(v->page, i));
	fill_rect(r.x, r.y, r.w, r.h, owned ? c : half(half(c, LOOK_NAVY), LOOK_NAVY));
}

/* the channel's colour on this page, framed white, as STATUS frames its slot */
static void worn_mark(int x, int y, const LooksView *v, int c) {
	const Look *l = v->grid ? &v->before : &v->look;
	int i = l->palette[c] == LOOK_NONE ? LOOKS_PLAIN : l->palette[c] == v->page ? l->colour[c] : -1;
	if (i < 0) return;
	SDL_Rect r = looks_cell_rect(i);
	r.x += x - 1;
	r.y += y - 1;
	fill_rect(r.x, r.y, r.w + 2, 1, LOOK_PAGE);
	fill_rect(r.x, r.y + r.h + 1, r.w + 2, 1, LOOK_PAGE);
	fill_rect(r.x, r.y, 1, r.h + 2, LOOK_PAGE);
	fill_rect(r.x + r.w + 1, r.y, 1, r.h + 2, LOOK_PAGE);
}

/* the palette's window: its name between the green page arrows, its grid, the plain cell */
static void palette(int x, int y, const LooksView *v) {
	bool owned = looks_owned(v->page);
	look_window(x + PAL_X, y + TOP, PAL_W, MID_H, PAL_TOP, false);
	const char *name = looks_palette_name(v->page);
	char caps[24];
	int n = 0;
	for (; name[n] && n < (int)sizeof caps - 1; ++n) caps[n] = (char)(name[n] >= 'a' && name[n] <= 'z' ? name[n] - 32 : name[n]);
	caps[n] = 0;
	look_caps(x + PAL_X + (PAL_W - look_caps_width(caps)) / 2, y + TOP + 4, caps, owned ? LOOK_PAGE : LOOK_DIM, LOOK_EDGE_LO);
	SDL_Rect l = looks_page_rect(false), r = looks_page_rect(true);
	look_arrow(x + l.x + 1, y + l.y + 1, false);
	look_arrow(x + r.x + 1, y + r.y + 1, true);
	for (int i = 0; i <= LOOKS_PLAIN; ++i) cell(x, y, v, i, owned);
	if (!owned) {
		/* (locked: its mark across the grid's middle) */
		SDL_Rect a = looks_cell_rect(4), b = looks_cell_rect(11);
		int my = y + (a.y + b.y + b.h) / 2 - 5;
		fill_rect(x + a.x, my, GRID_W, 10, LOOK_NAVY_LO);
		look_caps(x + a.x + (GRID_W - look_caps_width(looks_word(LW_LOCKED))) / 2, my + 3, looks_word(LW_LOCKED), LOOK_DIM, NONE);
	}
	if (v->row > 0) worn_mark(x, y, v, v->chan[v->row - 1]);
	if (v->grid) {
		SDL_Rect c = looks_cell_rect(v->cell);
		look_corners(x + c.x - 2, y + c.y - 2, c.w + 4, c.h + 4);
	}
}

/* KeyItem's INFORMATION box: what the cursor is on */
static void info(int x, int y, const LooksView *v) {
	SDL_Rect r = look_info(x + SLOT_X, y + INFO_Y, W - 2 * SLOT_X, INFO_H, looks_word(LW_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	char s[160];
	int c = v->row > 0 ? v->chan[v->row - 1] : -1;
	if (c < 0) looks_slot_words(s, sizeof s, v->slot);
	else if (!looks_owned(v->page) && (!v->grid || v->cell != LOOKS_PLAIN)) looks_locked_words(s, sizeof s, v->page, looks_palette_name(v->page));
	else if (v->grid) looks_colour_words(s, sizeof s, v->page, looks_palette_name(v->page), v->cell == LOOKS_PLAIN ? -1 : v->cell);
	else {
		const Look *l = &v->look;
		looks_channel_words(s, sizeof s, c, l->palette[c] == LOOK_NONE ? NULL : looks_palette_name(l->palette[c]), l->colour[c]);
	}
	LookLines at = { r.x + 5, r.y + 1, r.w - 10, 0, 2, LOOK_INK, LOOK_INK_SHADE };
	look_wrapped_from(s, &at);
}

void looks_draw_view(int x, int y, const LooksView *v) {
	look_screen(x, y, W, H, looks_word(LW_TITLE));
	slot(x, y, v);
	list(x, y, v);
	palette(x, y, v);
	info(x, y, v);
}

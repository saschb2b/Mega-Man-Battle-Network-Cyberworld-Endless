/* looks_draw.h. Nothing here is new art: MegaMan is BN6's own, read from the
 * player's ROM as he is drawn, in his palettes as the look makes them; the
 * colours are the palettes' own numbers (looks.h); the rest is the PET's
 * pieces (pet_look.h). The three panels stand on one top line, their fields
 * under bands of one height, the list's rows at BN6's 16-pixel pitch. */
#include "looks_draw.h"

#include <stdio.h>

#include "gfx.h"
#include "looks_screen_text.h"
#include "pet_look.h"

#define W 240
#define H 160
#define NONE (SDL_Color){ 0, 0, 0, 0 }
#define ORANGE rgba(255, 170, 40, 255)   /* Options' pointers beside a value the d-pad turns */

#define TOP 18
#define MID_H 98            /* the slot's and the windows' height */
#define SLOT_X 8
#define SLOT_W 50
#define LIST_X 61
#define LIST_W 80
#define PAL_X 144
#define PAL_W 88
#define WIN_TOP 7           /* a window's band over its field, its label in it */
#define ROW_H 16            /* BN6's lists' pitch */
#define ROW_IN 13           /* a row's words from its field's left, the orange arrow before them */
#define ROW_RIGHT 8         /* ... its value's right end from the field's right */
#define SWATCH 10
#define CELL 11             /* a grid's cell, CELL_STEP apart: EQUIP's corners stand in the gutter */
#define CELL_STEP 15
#define GRID_W (4 * CELL_STEP - (CELL_STEP - CELL))
#define PLAIN_H 11
#define PIP 4               /* the palettes' pips, PIP_STEP apart */
#define PIP_STEP 6
#define INFO_Y (TOP + MID_H + 3)
#define INFO_H (H - 3 - INFO_Y)
#define POP_X (LIST_X + 6)  /* ChipFolder's SELECT pop-up, over the list */
#define POP_Y (TOP + 20)
#define POP_W 70
#define POP_ROW 14

/* a GBA colour (R in the low bits) as the canvas's */
static SDL_Color colour(uint16_t c) {
	int r = c & 31, g = c >> 5 & 31, b = c >> 10 & 31;
	return (SDL_Color){ (Uint8)(r << 3 | r >> 2), (Uint8)(g << 3 | g >> 2), (Uint8)(b << 3 | b >> 2), 255 };
}

static SDL_Color half(SDL_Color c, SDL_Color to) {
	return (SDL_Color){ (Uint8)((c.r + to.r) / 2), (Uint8)((c.g + to.g) / 2), (Uint8)((c.b + to.b) / 2), 255 };
}

static void frame(int x, int y, int w, int h, SDL_Color c) {
	fill_rect(x, y, w, 1, c);
	fill_rect(x, y + h - 1, w, 1, c);
	fill_rect(x, y, 1, h, c);
	fill_rect(x + w - 1, y, 1, h, c);
}

static SDL_Rect field_of(int x, int w) { return (SDL_Rect){ x + LOOK_FRAME, TOP + LOOK_FRAME + WIN_TOP, w - 2 * LOOK_FRAME, MID_H - 2 * LOOK_FRAME - WIN_TOP }; }

SDL_Rect looks_row_rect(int row) {
	SDL_Rect f = field_of(LIST_X, LIST_W);
	return (SDL_Rect){ f.x, f.y + 4 + row * ROW_H, f.w, ROW_H };
}

SDL_Rect looks_cell_rect(int cell) {
	SDL_Rect f = field_of(PAL_X, PAL_W);
	int gx = f.x + (f.w - GRID_W) / 2, gy = f.y + 4;
	if (cell >= LOOKS_PLAIN) return (SDL_Rect){ gx, gy + 4 * CELL_STEP, GRID_W, PLAIN_H };
	return (SDL_Rect){ gx + cell % 4 * CELL_STEP, gy + cell / 4 * CELL_STEP, CELL, CELL };
}

SDL_Rect looks_page_rect(bool right) {
	return (SDL_Rect){ right ? PAL_X + PAL_W - LOOK_FRAME - LOOK_ARROW_W - 1 : PAL_X + LOOK_FRAME + 1, TOP + 2, LOOK_ARROW_W + 2, WIN_TOP + 3 };
}

SDL_Rect looks_popup_rect(int row) { return (SDL_Rect){ POP_X + LOOK_FRAME, POP_Y + LOOK_FRAME + WIN_TOP + 2 + row * POP_ROW, POP_W - 2 * LOOK_FRAME, POP_ROW }; }

/* STATUS's slot with BN6's MegaMan in it, his face over his body */
static void slot(int x, int y, const LooksView *v) {
	SDL_Rect f = look_slot(x + SLOT_X, y + TOP, SLOT_W, MID_H);
	looks_preview_draw(&v->preview, f.x, f.y, f.w, f.h);
}

/* a channel's swatch: its colour in white, or hollow where it is plain (BN6 draws it); white while a pick flashes */
static void swatch(int x, int y, const LooksView *v, int c, bool flash) {
	fill_rect(x - 1, y - 1, SWATCH + 2, SWATCH + 2, LOOK_PAGE);
	if (flash) return;
	if (v->look.palette[c] == LOOK_NONE) {
		fill_rect(x, y, SWATCH, SWATCH, LOOK_SLOT);
		fill_rect(x + 2, y + SWATCH / 2 - 1, SWATCH - 4, 2, LOOK_PAGE);   /* (a dash: nothing set) */
	} else fill_rect(x, y, SWATCH, SWATCH, colour(looks_colour(v->look.palette[c], v->look.colour[c])));
}

static void list(int x, int y, const LooksView *v) {
	look_window(x + LIST_X, y + TOP, LIST_W, MID_H, WIN_TOP, false);
	SDL_Rect f = field_of(LIST_X, LIST_W);
	bool flashing = v->flash > 0 && v->flash / 3 % 2;
	for (int r = 0; r <= v->n_chan; ++r) {
		SDL_Rect at = looks_row_rect(r);
		int ry = y + at.y + 2, right = x + f.x + f.w - ROW_RIGHT;
		look_text(x + at.x + ROW_IN, ry, r ? looks_channel_name(v->chan[r - 1]) : looks_word(LW_LOOK), LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		/* (in the grid the list's arrow holds still and dim: the grid's corners are the cursor) */
		if (r == v->row && v->grid) look_cursor_scaled(x + at.x + 2, ry + 2, 1, 120);
		else if (r == v->row && v->popup < 0) look_cursor(x + at.x + 2, ry + 2);
		if (!r) {
			/* (the look worn between Options' pointers, as a value Left and Right turn) */
			const char *s = looks_slot_name(v->slot);
			int l = right - 3 - look_text_width(s);
			if (!(flashing && v->flash_row == 0)) look_text(l, ry, s, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
			look_pointer(l - 5, ry, 4, true, ORANGE);
			look_pointer(right + 1, ry, 4, false, ORANGE);
			continue;
		}
		swatch(right - SWATCH + 1, ry + 1, v, v->chan[r - 1], flashing && v->flash_row == r);
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
	fill_rect(r.x, r.y, r.w, r.h, owned ? c : half(c, LOOK_NAVY));
}

/* the colour the channel wears on this page, framed white as STATUS frames its slot */
static void worn_mark(int x, int y, const LooksView *v, int c) {
	const Look *l = v->grid ? &v->before : &v->look;
	int i = l->palette[c] == LOOK_NONE ? LOOKS_PLAIN : l->palette[c] == v->page ? l->colour[c] : -1;
	if (i < 0) return;
	SDL_Rect r = looks_cell_rect(i);
	frame(x + r.x - 1, y + r.y - 1, r.w + 2, r.h + 2, LOOK_PAGE);
}

/* the twelve palettes as pips under the grid: owned lit, the one shown gold */
static void pips(int x, int y, const LooksView *v) {
	SDL_Rect f = field_of(PAL_X, PAL_W), p = looks_cell_rect(LOOKS_PLAIN);
	int px = x + f.x + (f.w - (LOOK_PALETTES * PIP_STEP - (PIP_STEP - PIP))) / 2, py = y + p.y + p.h + 3;
	for (int i = 0; i < LOOK_PALETTES; ++i) {
		SDL_Color c = i == v->page ? LOOK_GOLD : looks_owned(i) ? LOOK_PAGE : LOOK_NAVY_LO;
		fill_rect(px + i * PIP_STEP, py, PIP, PIP, c);
		if (i != v->page && !looks_owned(i)) frame(px + i * PIP_STEP, py, PIP, PIP, LOOK_SHADE);
	}
}

/* the palette's window: its name between the green page arrows, its grid, PLAIN, the pips */
static void palette(int x, int y, const LooksView *v) {
	bool owned = looks_owned(v->page);
	look_window(x + PAL_X, y + TOP, PAL_W, MID_H, WIN_TOP, false);
	const char *name = looks_band_name(v->page);
	look_caps(x + PAL_X + (PAL_W - look_caps_width(name)) / 2, y + TOP + 4, name, LOOK_PAGE, LOOK_EDGE_LO);
	SDL_Rect l = looks_page_rect(false), r = looks_page_rect(true);
	look_arrow(x + l.x + 1, y + l.y + 1, false);
	look_arrow(x + r.x + 1, y + r.y + 1, true);
	for (int i = 0; i <= LOOKS_PLAIN; ++i) cell(x, y, v, i, owned);
	if (!owned) {
		/* (locked: its mark across the grid's middle, between its second and third rows) */
		SDL_Rect a = looks_cell_rect(4);
		int my = y + a.y + CELL + 1;
		fill_rect(x + a.x, my, GRID_W, 7, LOOK_SHADE);
		look_caps(x + a.x + (GRID_W - look_caps_width(looks_word(LW_LOCKED))) / 2, my + 1, looks_word(LW_LOCKED), LOOK_TEXT, NONE);
	}
	if (v->row > 0) worn_mark(x, y, v, v->chan[v->row - 1]);
	if (v->grid) {
		SDL_Rect c = looks_cell_rect(v->cell);
		look_corners(x + c.x - 2, y + c.y - 2, c.w + 4, c.h + 4);
	}
	pips(x, y, v);
}

/* ChipFolder's SELECT pop-up over the list: the look's name on its band, a copy to each other look, all plain */
static void popup(int x, int y, const LooksView *v) {
	if (v->popup < 0) return;
	look_window(x + POP_X, y + POP_Y, POP_W, LOOKS_POPUP_ROWS * POP_ROW + 2 * LOOK_FRAME + WIN_TOP + 4, WIN_TOP, false);
	char label[16], row[24];
	snprintf(label, sizeof label, "LOOK %s", looks_slot_name(v->slot));
	look_caps(x + POP_X + LOOK_FRAME + 1, y + POP_Y + 4, label, LOOK_PAGE, LOOK_EDGE_LO);
	for (int r = 0; r < LOOKS_POPUP_ROWS; ++r) {
		SDL_Rect at = looks_popup_rect(r);
		if (r < LOOKS_POPUP_ROWS - 1) looks_copy_row(row, sizeof row, (v->slot + 1 + r) % LOOK_SLOTS);
		else snprintf(row, sizeof row, "%s", looks_word(LW_ALL_PLAIN));
		look_text(x + at.x + ROW_IN, y + at.y + 1, row, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		if (r == v->popup) look_cursor(x + at.x + 2, y + at.y + 3);
	}
}

/* KeyItem's INFORMATION box: what the cursor is on */
static void info(int x, int y, const LooksView *v) {
	SDL_Rect r = look_info(x + SLOT_X, y + INFO_Y, W - 2 * SLOT_X, INFO_H, looks_word(LW_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	char s[200];
	int c = v->row > 0 ? v->chan[v->row - 1] : -1;
	bool owned = looks_owned(v->page);
	if (v->popup >= 0) looks_popup_words(s, sizeof s, v->slot, v->popup);
	else if (c < 0) looks_slot_words(s, sizeof s, v->slot, &v->look);
	else if (v->grid) looks_colour_words(s, sizeof s, v->page, v->cell == LOOKS_PLAIN ? -1 : v->cell, owned);
	else if (!owned) looks_locked_words(s, sizeof s, v->page);
	else looks_channel_words(s, sizeof s, c, &v->look);
	LookLines at = { r.x + 5, r.y + 1, r.w - 10, 0, 2, LOOK_INK, LOOK_INK_SHADE };
	look_wrapped_from(s, &at);
}

void looks_draw_view(int x, int y, const LooksView *v) {
	look_screen(x, y, W, H, looks_word(LW_TITLE));
	look_band_hint(x, y, W, looks_word(LW_KEYS), looks_word(LW_PAGES));
	slot(x, y, v);
	list(x, y, v);
	palette(x, y, v);
	popup(x, y, v);
	info(x, y, v);
}

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
#define LOOKS_ROWS_SHOWN (1 + LOOK_CHANNELS)
#define POP_Y_OF (looks_row_rect(0).y + ROW_H)

#define TOP 18
#define MID_H 98            /* the slot's and the windows' height */
#define SLOT_X 8
#define SLOT_W 50
#define LIST_X 61
#define LIST_W 86
#define PAL_X 150
#define PAL_W 82
#define WIN_TOP 7           /* a window's band over its field, its label in it */
#define ROW_H 16            /* BN6's lists' pitch */
#define ROW_IN 13           /* a row's words from its field's left, the orange arrow before them */
#define ROW_RIGHT 5         /* ... its value's right end from the field's right */
#define SWATCH 10
#define CELL 11             /* a grid's cell, CELL_STEP apart: EQUIP's corners stand in the gutter */
#define CELL_STEP 15
#define GRID_W (4 * CELL_STEP - (CELL_STEP - CELL))
#define PLAIN_H 11
#define PILL_W 30           /* the palettes owned, in FOLDER EDIT's count pill */
#define PILL_H 12
#define PILL_EDGE rgba(181, 255, 189, 255)
#define INFO_Y (TOP + MID_H + 3)
#define INFO_H (H - 3 - INFO_Y)
#define POP_X (LIST_X + 4)  /* ChipFolder's SELECT pop-up, under the looks' row, inside the list */
#define POP_W 78
#define POP_ROW 13

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
	return (SDL_Rect){ f.x, f.y + (f.h - LOOKS_ROWS_SHOWN * ROW_H) / 2 + row * ROW_H, f.w, ROW_H };
}

SDL_Rect looks_slot_value_rect(void) {
	SDL_Rect r = looks_row_rect(0);
	return (SDL_Rect){ r.x + r.w / 2, r.y, r.w / 2, r.h };
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

SDL_Rect looks_popup_box(void) { return (SDL_Rect){ POP_X, POP_Y_OF, POP_W, LOOKS_POPUP_ROWS * POP_ROW + 2 * LOOK_FRAME + WIN_TOP + 4 }; }

SDL_Rect looks_popup_rect(int row) { return (SDL_Rect){ POP_X + LOOK_FRAME, POP_Y_OF + LOOK_FRAME + WIN_TOP + 2 + row * POP_ROW, POP_W - 2 * LOOK_FRAME, POP_ROW }; }

/* STATUS's slot with BN6's MegaMan in it, his face over his body */
static void slot(int x, int y, const LooksView *v) {
	SDL_Rect f = look_slot(x + SLOT_X, y + TOP, SLOT_W, MID_H);
	looks_preview_draw(&v->preview, f.x, f.y, f.w, f.h);
}

/* the channel's colour as he wears it: the colour picked, else BN6's own mid shade of its ramp */
static SDL_Color channel_colour(const LooksView *v, int c) {
	static const int mid[LOOK_CHANNELS] = { 3, 7, 11 };   /* (battle palette 0: the armour's ramp 2-5, the suit's 6-9, the gold 11) */
	if (v->look.palette[c] != LOOK_NONE) return colour(looks_colour(v->look.palette[c], v->look.colour[c]));
	const uint8_t *pal = v->preview.pal[LOOKS_ART_BATTLE];
	return colour((uint16_t)(pal[2 * mid[c]] | pal[2 * mid[c] + 1] << 8));
}

/* a channel's swatch: its colour in white, its frame orange while a pick flashes */
static void swatch(int x, int y, const LooksView *v, int c, bool flash) {
	fill_rect(x - 1, y - 1, SWATCH + 2, SWATCH + 2, flash ? ORANGE : LOOK_PAGE);
	fill_rect(x, y, SWATCH, SWATCH, channel_colour(v, c));
}

static void list(int x, int y, const LooksView *v) {
	look_window(x + LIST_X, y + TOP, LIST_W, MID_H, WIN_TOP, false);
	SDL_Rect f = field_of(LIST_X, LIST_W);
	char label[16];
	snprintf(label, sizeof label, "%s %s", looks_word(LW_LOOK_CAPS), looks_slot_name(v->slot));
	look_caps(x + LIST_X + (LIST_W - look_caps_width(label)) / 2, y + TOP + 4, label, LOOK_PAGE, LOOK_EDGE_LO);
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
			look_text(l, ry, s, flashing && v->flash_row == 0 ? ORANGE : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
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
		/* (a locked page's LOCKED stands where PLAIN does: nothing on it can be had) */
		const char *w = looks_word(owned ? LW_PLAIN : LW_LOCKED);
		fill_rect(r.x, r.y, r.w, r.h, owned ? LOOK_NAVY_LO : LOOK_SHADE);
		look_caps(r.x + (r.w - look_caps_width(w)) / 2, r.y + 3, w, LOOK_TEXT, owned ? LOOK_SHADE : NONE);
		return;
	}
	SDL_Color c = colour(looks_colour(v->page, i));
	fill_rect(r.x, r.y, r.w, r.h, owned ? c : half(c, LOOK_NAVY));
}

/* the colour the channel wears on this page, framed white as STATUS frames its slot */
static void worn_mark(int x, int y, const LooksView *v, int c) {
	const Look *l = v->grid ? &v->before : &v->look;
	int i = l->palette[c] == LOOK_NONE ? LOOKS_PLAIN : l->palette[c] == v->page ? l->colour[c] : -1;
	if (i < 0 || (i == LOOKS_PLAIN && !looks_owned(v->page))) return;
	SDL_Rect r = looks_cell_rect(i);
	frame(x + r.x - 1, y + r.y - 1, r.w + 2, r.h + 2, LOOK_PAGE);
}

/* the palettes owned of the twelve, in FOLDER EDIT's count pill under PLAIN, gold where all are */
static void count_pill(int x, int y) {
	SDL_Rect f = field_of(PAL_X, PAL_W), p = looks_cell_rect(LOOKS_PLAIN);
	int owned = 0;
	for (int i = 0; i < LOOK_PALETTES; ++i) owned += looks_owned(i);
	char s[8];
	looks_count_words(s, sizeof s, owned, LOOK_PALETTES);
	int px = x + f.x + (f.w - PILL_W) / 2, py = y + p.y + p.h + 2;
	fill_rect(px + 1, py, PILL_W - 2, PILL_H, PILL_EDGE);
	fill_rect(px, py + 1, PILL_W, PILL_H - 2, PILL_EDGE);
	fill_rect(px + 1, py + 1, PILL_W - 2, PILL_H - 2, LOOK_NAVY);
	look_title(px + (PILL_W - look_title_width(s)) / 2, py + 3, s, owned == LOOK_PALETTES ? LOOK_GOLD : PILL_EDGE);
}

/* the palette's window: its name between the green page arrows, its grid, PLAIN, the palettes owned */
static void palette(int x, int y, const LooksView *v) {
	bool owned = looks_owned(v->page);
	look_window(x + PAL_X, y + TOP, PAL_W, MID_H, WIN_TOP, false);
	const char *name = looks_band_name(v->page);
	look_caps(x + PAL_X + (PAL_W - look_caps_width(name)) / 2, y + TOP + 4, name, LOOK_PAGE, LOOK_EDGE_LO);
	SDL_Rect l = looks_page_rect(false), r = looks_page_rect(true);
	look_arrow(x + l.x + 1, y + l.y + 1, false);
	look_arrow(x + r.x + 1, y + r.y + 1, true);
	for (int i = 0; i <= LOOKS_PLAIN; ++i) cell(x, y, v, i, owned);
	if (v->row > 0) worn_mark(x, y, v, v->chan[v->row - 1]);
	if (v->grid) {
		SDL_Rect c = looks_cell_rect(v->cell);
		look_corners(x + c.x - 2, y + c.y - 2, c.w + 4, c.h + 4);
	}
	count_pill(x, y);
}

/* ChipFolder's SELECT pop-up over the list: the look's name on its band, a copy to each other look, all plain */
static void popup(int x, int y, const LooksView *v) {
	if (v->popup < 0) return;
	SDL_Rect box = looks_popup_box();
	look_window(x + box.x, y + box.y, box.w, box.h, WIN_TOP, false);
	char label[16], row[24];
	snprintf(label, sizeof label, "%s %s", looks_word(LW_LOOK_CAPS), looks_slot_name(v->slot));
	look_caps(x + box.x + LOOK_FRAME + 1, y + box.y + 4, label, LOOK_PAGE, LOOK_EDGE_LO);
	for (int r = 0; r < LOOKS_POPUP_ROWS; ++r) {
		SDL_Rect at = looks_popup_rect(r);
		if (r < LOOKS_POPUP_ROWS - 1) looks_copy_row(row, sizeof row, (v->slot + 1 + r) % LOOK_SLOTS);
		else snprintf(row, sizeof row, "%s", looks_word(LW_ALL_PLAIN));
		look_text(x + at.x + ROW_IN, y + at.y, row, LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
		if (r == v->popup) look_cursor(x + at.x + 2, y + at.y + 2);
	}
}

/* KeyItem's INFORMATION box: what the cursor is on */
static void info(int x, int y, const LooksView *v) {
	SDL_Rect r = look_info(x + SLOT_X, y + INFO_Y, W - 2 * SLOT_X, INFO_H, looks_word(LW_INFORMATION));
	fill_rect(r.x + 1, r.y + 1, r.w - 2, r.h - 1, LOOK_PAGE);
	char s[200];
	int c = v->row > 0 ? v->chan[v->row - 1] : -1;
	bool owned = looks_owned(v->page);
	if (v->popup >= 0 && v->asking) {
		/* (the question, Yes and No on its second line behind BN6's green triangle) */
		looks_ask_words(s, sizeof s, v->slot, v->popup);
		look_text(r.x + 5, r.y + 1, s, LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
		for (int k = 0; k < 2; ++k) {
			int ax = r.x + 5 + LOOK_CHOICE_W + 2 + k * 48;
			look_text(ax, r.y + 1 + LOOK_LINE, looks_yes_no(k == 0), LOOK_INK, LOOK_INK_SHADE, TEXT_LEFT);
			if ((k == 0) == v->yes) look_choice(ax - LOOK_CHOICE_W - 1, r.y + LOOK_LINE);
		}
		return;
	}
	if (v->popup >= 0) {
		int to = v->popup < LOOKS_POPUP_ROWS - 1 ? (v->slot + 1 + v->popup) % LOOK_SLOTS : v->slot;
		Look there = looks_get(to);
		looks_popup_words(s, sizeof s, v->slot, v->popup, &there);
	} else if (v->told) looks_told_words(s, sizeof s, v->told == LOOKS_TOLD_COPY, v->slot, v->told_from);
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

/* netpass_draw.h. */
#include "netpass_draw.h"

#include <stdio.h>
#include <string.h>

#include "gfx.h"
#include "minifont.h"
#include "netpass_text.h"
#include "pixfont.h"
#include "second_frame.h"
#include "square_arena.h"
#include "text.h"

#define INK        rgba(0, 49, 74, 255)      /* the PET's darkest: a shade under pixel letters */
#define BACK_DOT   rgba(10, 66, 92, 255)     /* the screen's dotted grid */
#define FACE_BACK  rgba(16, 54, 74, 255)     /* behind a face, as the cartridges' labels have it */
#define GOLD_EDGE  rgba(198, 90, 0, 255)
#define GOLD_HI    rgba(255, 247, 165, 255)
#define GOLD_LO    rgba(247, 165, 0, 255)
#define CELL_OFF   rgba(10, 58, 80, 255)
#define OUTLINE    rgba(24, 40, 56, 255)     /* the cartridges' (launcher_art.c) */
#define METAL_HI   rgba(198, 222, 239, 255)
#define METAL      rgba(165, 189, 214, 255)
#define METAL_LO   rgba(99, 132, 148, 255)
#define GRIP       rgba(140, 165, 189, 255)

#define HEAD_H 18
#define CARD_X 4
#define CARD_Y 21
#define CARD_W 110
#define CARD_H 84
#define GRID_X 118
#define CELL_W 57
#define CELL_H 41
#define CELL_GAP 4
#define TRAY_Y 110
#define TRAY_H 46
#define CART_W 40
#define CART_H 26

static const SDL_Color none = { 0, 0, 0, 0 };

/* a label's colours by the game's slot: its top and foot and its light stripe (launcher_art.c's) */
static SDL_Color label_color(int game, int part) {
	static const SDL_Color c[2][3] = {
		{ { 8, 189, 115, 255 }, { 0, 140, 82, 255 }, { 74, 231, 115, 255 } },
		{ { 49, 156, 222, 255 }, { 16, 99, 165, 255 }, { 107, 222, 255, 255 } },
	};
	static const SDL_Color other[3] = { { 165, 140, 222, 255 }, { 107, 82, 165, 255 }, { 206, 189, 255, 255 } };
	return game >= 0 && game < 2 ? c[game][part] : other[part];
}

/* a panel: `fill` inside a 1-pixel `edge`, its corners cut */
static void panel(int x, int y, int w, int h, SDL_Color fill, SDL_Color edge) {
	fill_rect(x + 1, y, w - 2, h, edge);
	fill_rect(x, y + 1, w, h - 2, edge);
	fill_rect(x + 1, y + 1, w - 2, h - 2, fill);
}

/* a disc of radius r round (cx, cy) */
static void disc(int cx, int cy, int r, SDL_Color c) {
	SDL_Rect rows[32];
	int n = 0;
	for (int dy = -r; dy <= r && n < 32; ++dy) {
		int dx = 0;
		while ((dx + 1) * (dx + 1) + dy * dy <= r * r + r) ++dx;
		rows[n++] = (SDL_Rect){ cx - dx, cy + dy, 2 * dx + 1, 1 };
	}
	fill_rects(rows, n, c);
}

/* a ring of radius r, in dashes where `dashed` */
static void ring(int cx, int cy, int r, SDL_Color c, bool dashed) {
	SDL_Rect dots[256];
	int n = 0, k = 0;
	for (int dy = -r; dy <= r; ++dy)
		for (int dx = -r; dx <= r && n < 256; ++dx) {
			int d = dx * dx + dy * dy;
			if (d > r * r + r || d < (r - 1) * (r - 1) + (r - 1)) continue;
			/* (dashes by the angle's octant step: every other pair of pixels) */
			if (dashed && ((dx + 2 * r) / 2 + (dy + 2 * r) / 2) % 2) { ++k; continue; }
			dots[n++] = (SDL_Rect){ cx + dx, cy + dy, 1, 1 };
		}
	(void)k;
	fill_rects(dots, n, c);
}

/* a check mark, 9 x 7, its middle at (cx, cy) */
static void check(int cx, int cy, SDL_Color c) {
	static const char *const rows[7] = { "........##", ".......###", "......###.", "##...###..", "###.###...", ".#####....", "..###....." };
	SDL_Rect px[40];
	int n = 0;
	for (int r = 0; r < 7; ++r)
		for (int i = 0; rows[r][i] && n < 40; ++i)
			if (rows[r][i] == '#') px[n++] = (SDL_Rect){ cx - 5 + i, cy - 3 + r, 1, 1 };
	fill_rects(px, n, c);
}

/* a star, 9 x 9 from (x, y) */
static void star(int x, int y, SDL_Color c, SDL_Color edge) {
	static const char *const rows[9] = { "....#....", "...###...", "...###...", "#########", ".#######.", "..#####..", "..#####..", ".###.###.",
		".##...##." };
	SDL_Rect px[60];
	int n = 0;
	for (int r = 0; r < 9; ++r)
		for (int i = 0; i < 9 && n < 60; ++i)
			if (rows[r][i] == '#') px[n++] = (SDL_Rect){ x + i, y + r, 1, 1 };
	if (edge.a) {
		SDL_Rect sh[60];
		for (int i = 0; i < n; ++i) sh[i] = (SDL_Rect){ px[i].x + 1, px[i].y + 1, 1, 1 };
		fill_rects(sh, n, edge);
	}
	fill_rects(px, n, c);
}

/* the screen's ground: the PET's dark blue under a dotted grid */
static void ground(int x, int y, int w, int h) {
	fill_rect(x, y, w, h, PET_EXIT);
	SDL_Rect dots[1024];
	int n = 0;
	for (int j = 4; j < h && n < 1024; j += 8)
		for (int i = 4; i < w && n < 1024; i += 8) dots[n++] = (SDL_Rect){ x + i, y + j, 1, 1 };
	fill_rects(dots, n, BACK_DOT);
}

/* the header: BN6's green band, the title and its stripes, the level on a slot */
static void header(int x, int y, const NetPassView *v) {
	fill_rect(x, y, NETPASS_W, HEAD_H, PET_GREEN);
	fill_rect(x, y, NETPASS_W, 1, PET_LINE);
	fill_rect(x, y + HEAD_H - 2, NETPASS_W, 2, PET_DARK);
	text_draw(x + 7, y + 3, netpass_title, PET_WHITE, TEXT_LEFT);
	second_stripes(x + 11 + text_width(netpass_title), y + 3);
	const char *level = v->full ? netpass_level_full : netpass_level_square;
	int lw = text_width(level) + (v->full ? 12 : 0), sw = lw + 12, sx = x + NETPASS_W - 4 - sw;
	second_slot(sx, y + 2, sw, HEAD_H - 5);
	int tx = sx + 6;
	if (v->full) { star(tx, y + 4, PET_GOLD, GOLD_EDGE); tx += 12; }
	text_draw(tx, y + 2, level, v->full ? PET_GOLD : PET_CYAN_HI, TEXT_LEFT);
}

/* MegaMan's face from the player's ROM in a 40 x 48 window from (x, y) */
static void face(int x, int y, bool into) {
	fill_rect(x - 1, y - 1, 42, 50, PET_CYAN_HI);
	fill_rect(x, y, 40, 48, FACE_BACK);
	Sprite *s = sprite_get(SPR_MUGSHOT, FACE_MEGAMAN);
	if (!s) return;
	/* (a mugshot's origin is its middle) */
	if (into) sprite_draw_into(s, 0, 0, x + 20, y + 24, 0, 1);
	else sprite_draw_frame(s, 0, 0, x + 20, y + 24, false, 0, 0);
}

static int held_count(const NetPassView *v) {
	int n = 0;
	for (int s = 0; s < STAMP_COUNT; ++s) n += v->held[s];
	return n;
}

/* the operator card: the face, Lan and MegaMan, the stamps' bar */
static void card(int x, int y, const NetPassView *v) {
	panel(x, y, CARD_W, CARD_H, PET_SLOT, PET_CYAN);
	fill_rect(x + 2, y + 1, CARD_W - 4, 1, rgba(33, 123, 132, 255));
	face(x + 5, y + 5, v->into);
	int cx = x + 51;
	minifont_draw(cx, y + 7, netpass_operator_label, PET_DIM, 1);
	text_draw(cx, y + 13, netpass_operator, PET_WHITE, TEXT_LEFT);
	minifont_draw(cx, y + 30, netpass_navi_label, PET_DIM, 1);
	text_draw(cx, y + 36, netpass_navi, PET_WHITE, TEXT_LEFT);
	fill_rect(x + 4, y + 57, CARD_W - 8, 1, PET_SLOT_EDGE);
	minifont_draw(x + 5, y + 62, netpass_stamps_label, PET_DIM, 1);
	char count[16];
	int held = held_count(v);
	netpass_count_words(count, sizeof count, held, STAMP_COUNT);
	text_draw(x + CARD_W - 5, y + 58, count, held == STAMP_COUNT ? PET_GOLD : PET_WHITE, TEXT_RIGHT);
	/* (a segment a stamp, gold where held) */
	int sw = (CARD_W - 10 - 3 * 2) / STAMP_COUNT;
	for (int s = 0; s < STAMP_COUNT; ++s) {
		int sx = x + 5 + s * (sw + 2), sy = y + 73;
		if (v->held[s]) {
			fill_rect(sx, sy, sw, 6, PET_GOLD);
			fill_rect(sx, sy, sw, 1, GOLD_HI);
			fill_rect(sx, sy + 5, sw, 1, GOLD_LO);
		} else {
			fill_rect(sx, sy, sw, 6, PET_SLOT_EDGE);
			fill_rect(sx + 1, sy + 1, sw - 2, 4, PET_EXIT);
		}
	}
}

/* a stamp's seal round (cx, cy), radius r: gold with a check where held
 * (a white flash as it lands), a dashed ring with a "?" where not */
static void seal(int cx, int cy, int r, bool held, int landed) {
	if (!held) {
		ring(cx, cy, r, PET_DIM, true);
		pixfont_draw(cx + 1, cy - 3, "?", PET_DIM, none, PIXFONT_CENTER, 1);
		return;
	}
	/* (landing: larger for a few frames, as a stamp pressed down) */
	int big = landed >= 0 && landed < 6 ? 2 - landed / 3 : 0;
	disc(cx + 1, cy + 1, r + big, INK);
	disc(cx, cy, r + big, GOLD_EDGE);
	disc(cx, cy, r + big - 1, PET_GOLD);
	ring(cx, cy, r + big - 3, GOLD_LO, false);
	fill_rect(cx - r / 2, cy - r + 2, r, 1, GOLD_HI);
	check(cx, cy, INK);
	if (landed >= 0 && landed < 8) disc(cx, cy, r + big, rgba(255, 255, 255, 220 - landed * 25));
}

static void stamp_cell(int x, int y, int s, const NetPassView *v) {
	bool held = v->held[s];
	panel(x, y, CELL_W, CELL_H, held ? PET_NAVY : CELL_OFF, held ? PET_CYAN : PET_SLOT_EDGE);
	int cx = x + CELL_W / 2;
	seal(cx, y + 13, 10, held, v->landing == s ? v->landing_t : -1);
	pixfont_draw(cx, y + 25, netpass_stamp_names[s], held ? PET_WHITE : PET_DIM, INK, PIXFONT_CENTER, 1);
	minifont_draw_centered(cx, y + 34, netpass_stamp_captions[s], held ? PET_CYAN_HI : PET_DIM, 1);
}

/* a cartridge of the set's game `g`, small, from (x, y): the ROMs
 * screen's shell (launcher_art.c), its label in the game's colours */
static void cartridge(int x, int y, int g) {
	const NetPassGame *game = netpass_game(g);
	fill_rect(x + 1, y, CART_W - 2, CART_H, OUTLINE);
	fill_rect(x, y + 1, CART_W, CART_H - 2, OUTLINE);
	fill_rect(x + 1, y + 1, CART_W - 2, CART_H - 2, METAL);
	fill_rect(x + 1, y + 1, CART_W - 2, 1, METAL_HI);
	fill_rect(x + 1, y + 1, 1, CART_H - 3, METAL_HI);
	/* (the grip along its top) */
	fill_rect(x + 3, y + 2, CART_W - 6, 2, GRIP);
	for (int i = x + 5; i < x + CART_W - 5; i += 3) fill_rect(i, y + 2, 1, 2, METAL_LO);
	/* (its label in a recess) */
	fill_rect(x + 4, y + 5, CART_W - 8, 15, rgba(60, 80, 100, 255));
	fill_rect(x + 5, y + 6, CART_W - 10, 7, label_color(g, 0));
	fill_rect(x + 5, y + 13, CART_W - 10, 6, label_color(g, 1));
	fill_rect(x + 5, y + 8, CART_W - 10, 1, label_color(g, 2));
	pixfont_draw(x + CART_W / 2, y + 9, game ? game->label : "", PET_WHITE, INK, PIXFONT_CENTER, 1);
	/* (the connector's slot under it) */
	fill_rect(x + 6, y + CART_H - 4, CART_W - 12, 2, OUTLINE);
}

/* an open slot of a cartridge's size: dashes round a recess, marching */
static void open_slot(int x, int y, int g, int t) {
	const NetPassGame *game = netpass_game(g);
	fill_rect(x + 1, y + 1, CART_W - 2, CART_H - 2, rgba(10, 58, 80, 255));
	int per = 2 * (CART_W + CART_H) - 4, phase = t / 6;
	SDL_Rect dash[160];
	int n = 0;
	for (int i = 0; i < per && n < 160; ++i) {
		int px, py;
		if (i < CART_W) { px = x + i; py = y; }
		else if (i < CART_W + CART_H - 1) { px = x + CART_W - 1; py = y + i - CART_W + 1; }
		else if (i < 2 * CART_W + CART_H - 2) { px = x + CART_W - 2 - (i - CART_W - CART_H + 1); py = y + CART_H - 1; }
		else { px = x; py = y + CART_H - 2 - (i - 2 * CART_W - CART_H + 2); }
		if ((i + phase) % 5 < 3) dash[n++] = (SDL_Rect){ px, py, 1, 1 };
	}
	fill_rects(dash, n, PET_CYAN);
	pixfont_draw(x + CART_W / 2, y + 9, game ? game->label : "", PET_DIM, none, PIXFONT_CENTER, 1);
}

/* the tray: each game's slot, in or open, and the PET's line beside them */
static void tray(int x, int y, const NetPassView *v) {
	panel(x, y, NETPASS_W - 8, TRAY_H, rgba(8, 41, 62, 255), PET_SLOT_EDGE);
	int games = netpass_games(), cx = x + 5;
	const NetPassGame *missing = NULL;
	for (int g = 0; g < games; ++g, cx += CART_W + 4) {
		bool in = netpass_game_in(g);
		if (in) cartridge(cx, y + 5, g);
		else open_slot(cx, y + 5, g, v->t);
		if (!in && !missing) missing = netpass_game(g);
		const char *word = in ? netpass_cart_in : netpass_cart_open;
		SDL_Color c = in ? PET_LINE : PET_DIM;
		int w = minifont_width(word, 1) + (in ? 8 : 0), wx = cx + (CART_W - w) / 2;
		if (in) check(wx + 3, y + 37, c);
		minifont_draw(wx + (in ? 8 : 0), y + 35, word, c, 1);
	}
	const char *line = missing ? netpass_missing_words(missing->name) : square_arena_built() ? netpass_complete : netpass_complete_no_ring;
	int tx = cx + 2, tw = x + NETPASS_W - 8 - 5 - tx;
	second_wrapped(line, tx, y + 4, tw, 3, missing ? PET_WHITE : PET_GOLD);
}

void netpass_view_now(NetPassView *v) {
	memset(v, 0, sizeof *v);
	for (int s = 0; s < STAMP_COUNT; ++s) v->held[s] = netpass_has((NetStamp)s);
	v->full = netpass_full();
	v->landing = -1;
}

void netpass_draw(int x, int y, const NetPassView *v) {
	ground(x, y, NETPASS_W, NETPASS_H);
	header(x, y, v);
	card(x + CARD_X, y + CARD_Y, v);
	for (int s = 0; s < STAMP_COUNT; ++s)
		stamp_cell(x + GRID_X + s % 2 * (CELL_W + CELL_GAP), y + CARD_Y + s / 2 * (CELL_H + CELL_GAP - 1), s, v);
	tray(x + 4, y + TRAY_Y, v);
}

void netpass_draw_strip(int x, int y, const NetPassView *v) {
	ground(x, y, NETPASS_W, NETPASS_STRIP_H);
	header(x, y, v);
	int w = (NETPASS_W - 8 - 3 * 3) / STAMP_COUNT;
	for (int s = 0; s < STAMP_COUNT; ++s) {
		int cx = x + 4 + s * (w + 3), cy = y + HEAD_H + 2;
		bool held = v->held[s];
		panel(cx, cy, w, NETPASS_STRIP_H - HEAD_H - 4, held ? PET_NAVY : CELL_OFF, held ? PET_CYAN : PET_SLOT_EDGE);
		seal(cx + w / 2, cy + 11, 8, held, v->landing == s ? v->landing_t : -1);
		pixfont_draw(cx + w / 2, cy + 22, netpass_stamp_names[s], held ? PET_WHITE : PET_DIM, INK, PIXFONT_CENTER, 1);
	}
}

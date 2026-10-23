/* emote_strip_draw.h. The Custom screen's manner (the chips in rows, the
 * one under the cursor large in a window with its name), drawn as the
 * NetPass and Trade Central's screens are, in BN6's PET screens' look
 * (pet_look.h): the band, slim windows on the body's green. MegaMan's poses are his overworld
 * sprite's frames, read from the player's ROM as they are drawn. */
#include "emote_strip_draw.h"

#include <stdio.h>
#include <string.h>

#include "bn6.h"
#include "chatbox.h"
#include "emote_strip_text.h"
#include "minifont.h"
#include "netpass_draw.h"
#include "online_text.h"
#include "pet_look.h"
#include "pixfont.h"
#include "second_frame.h"
#include "square_emote.h"

#define INK       rgba(0, 49, 74, 255)
#define FACE_BACK rgba(16, 54, 74, 255)
#define TRAY_BACK rgba(8, 41, 62, 255)

#define PV_X 4
#define PV_Y 4
#define PV_W 92
#define NAME_H 14
#define WIN_Y (PV_Y + NAME_H + 1)
#define WIN_H 54
#define AIM_Y (WIN_Y + WIN_H + 1)
#define AIM_H 12
#define TRAY_Y 88
#define BAND_H 12
#define CELLS_Y (TRAY_Y + BAND_H + 2)
#define CELL_W 22
#define CELL_H 28
#define PLATE_W 68
#define PLATE_H 13
#define FAV_X 146
#define FAV_W 90
#define FAV_PLATE_W 44
#define FAV_PLATE_H 11

#define FACING 3   /* (emote_strip.c's: toward the screen's lower left) */
#define GOT_IT 6   /* (online_emotes' "Got it!", Wave's pose) */

SDL_Rect strip_cell(int page, int i) {
	int cols = STRIP_COLS(page), col = i % cols, row = i / cols;
	if (page) return (SDL_Rect){ 4 + col * (PLATE_W + 2), CELLS_Y + row * (PLATE_H + 1), PLATE_W, PLATE_H };
	return (SDL_Rect){ 4 + col * (CELL_W + 1), CELLS_Y + row * (CELL_H + 1), CELL_W, CELL_H };
}

SDL_Rect strip_preview(void) { return (SDL_Rect){ PV_X, PV_Y, PV_W, AIM_Y + AIM_H - PV_Y }; }

SDL_Rect strip_tab(int page) { return (SDL_Rect){ page ? 66 : 16, TRAY_Y + 1, page ? 54 : 48, BAND_H - 2 }; }

SDL_Rect strip_favourite(int way) {
	static const int at[4][2] = { { FAV_X + (FAV_W - FAV_PLATE_W) / 2, 9 }, { FAV_X + FAV_W - FAV_PLATE_W, 22 },
		{ FAV_X + (FAV_W - FAV_PLATE_W) / 2, 35 }, { FAV_X, 22 } };
	return (SDL_Rect){ at[way & 3][0], CELLS_Y + at[way & 3][1], FAV_PLATE_W, FAV_PLATE_H };
}

void strip_arrow(int x, int y, int way, SDL_Color c) {
	static const char *const up[5] = { "..#..", ".###.", "#####", "..#..", "..#.." };
	SDL_Rect px[25];
	int n = 0;
	for (int r = 0; r < 5; ++r)
		for (int i = 0; i < 5; ++i) {
			if (up[r][i] != '#') continue;
			int ax = way == 0 ? i : way == 1 ? 4 - r : way == 2 ? i : r, ay = way == 0 ? r : way == 1 ? i : way == 2 ? 4 - r : i;
			px[n++] = (SDL_Rect){ x + ax, y + ay, 1, 1 };
		}
	fill_rects(px, n, c);
}

/* a small sparkle, 5 x 5, from (x, y): Got it!'s mark beside the pose Wave shares */
static void sparkle(int x, int y, SDL_Color c) {
	SDL_Rect r[3] = { { x + 2, y, 1, 5 }, { x, y + 2, 5, 1 }, { x + 1, y + 1, 3, 3 } };
	fill_rects(r, 2, c);
	fill_rect(r[2].x + 1, r[2].y + 1, 1, 1, PET_WHITE);
}

/* which way of L holds item `item`, -1 none */
static int way_of(const StripView *v, int item) {
	for (int k = 0; k < 4; ++k) if (v->favourites[k] == item) return k;
	return -1;
}

static const char *item_name(int item) { return item < SQUARE_PHRASE ? online_emotes[item] : online_phrases[item - SQUARE_PHRASE]; }

/* MegaMan in pose `anim`, frame `frame`, his feet at (x, y), kept inside `clip` */
static void megaman(int anim, int frame, int x, int y, SDL_Rect clip) {
	Sprite *s = sprite_get(SPR_NPC, BN6_OW_MEGAMAN);
	if (!s) return;
	gfx_clip(&clip);
	sprite_draw_frame(s, anim, frame, x, y, false, 0, 0);
	gfx_clip(NULL);
}

/* the window at the top left: the name on its plate, MegaMan playing the
 * pose (a phrase over his head), whom it goes to */
static void preview(int x, int y, const StripView *v) {
	const char *name = item_name(v->cursor);
	netpass_panel(x + PV_X, y + PV_Y, PV_W, NAME_H, PET_SLOT, PET_CYAN);
	text_draw(x + PV_X + PV_W / 2, y + PV_Y + 1, name, PET_WHITE, TEXT_CENTER);
	fill_rect(x + PV_X - 1, y + WIN_Y - 1, PV_W + 2, WIN_H + 2, PET_CYAN_HI);
	fill_rect(x + PV_X, y + WIN_Y, PV_W, WIN_H, FACE_BACK);
	SDL_Rect win = { x + PV_X, y + WIN_Y, PV_W, WIN_H };
	megaman(v->anim, v->frame, x + PV_X + PV_W / 2, y + WIN_Y + WIN_H - 6, win);
	if (v->cursor >= SQUARE_PHRASE) {
		int w = chatbox_text_width(name) + 10;
		chatbox_bubble(x + PV_X + (PV_W - w) / 2, y + WIN_Y + 2, w, 18);
		chatbox_text(x + PV_X + (PV_W - w) / 2 + 5, y + WIN_Y + 5, name, -1);
	}
	netpass_panel(x + PV_X, y + AIM_Y, PV_W, AIM_H, TRAY_BACK, PET_SLOT_EDGE);
	const char *aim = strip_text_aim(v->aims ? v->aim : "");
	minifont_draw_centered(x + PV_X + PV_W / 2, y + AIM_Y + 3, aim, v->aims && v->aim[0] ? PET_GOLD : PET_DIM, 1);
}

/* the band over the tray: L, the two pages' tabs, R, the keys */
static void band(int x, int y, const StripView *v) {
	int by = y + TRAY_Y;
	/* (BN6's band: pet_look.h) */
	fill_rect(x, by, NETPASS_W, BAND_H, LOOK_BAND);
	fill_rect(x, by + BAND_H - 1, NETPASS_W, 1, LOOK_GREEN_LO);
	static const char *const keys[2] = { "L", "R" };
	for (int k = 0; k < 2; ++k) look_tag(x + (k ? 122 : 4), by + 2, 9, 8, keys[k], LOOK_GREEN);
	for (int p = 0; p < 2; ++p) {
		SDL_Rect r = strip_tab(p);
		bool on = p == v->page;
		if (on) netpass_panel(x + r.x, y + r.y, r.w, r.h, PET_SLOT, PET_CYAN_HI);
		pixfont_draw(x + r.x + r.w / 2, y + r.y + 1, strip_text_pages[p], on ? PET_WHITE : LOOK_EDGE_HI, on ? INK : (SDL_Color){ 0, 0, 0, 0 }, PIXFONT_CENTER, 1);
	}
	minifont_draw(x + NETPASS_W - 4 - minifont_width(strip_text_keys, 1), by + 4, strip_text_keys, LOOK_PAGE, 1);
}

/* an emote's icon: MegaMan's pose in a cell, lit under the cursor, its way of L in its corner */
static void icon(int x, int y, int e, const StripView *v) {
	SDL_Rect r = strip_cell(0, e);
	bool under = v->cursor == e;
	r.x += x;
	r.y += y;
	netpass_panel(r.x, r.y, r.w, r.h, under ? PET_NAVY : FACE_BACK, under ? PET_GOLD : PET_SLOT_EDGE);
	if (under) fill_rect(r.x + 1, r.y + 1, r.w - 2, 1, PET_GOLD);
	/* (I'm beat's icon is him down, the pose's last frame; Got it! shares Wave's pose, a sparkle beside it) */
	int anim = square_emote_anim(e, FACING, 0), frame = anim == BN6_OW_ANIM_COLLAPSE ? 2 : 0;
	SDL_Rect in = { r.x + 1, r.y + 1, r.w - 2, r.h - 2 };
	megaman(anim, frame, r.x + r.w / 2, r.y + r.h - (frame ? 6 : 2), in);
	if (e == GOT_IT) sparkle(r.x + r.w - 7, r.y + 3, PET_GOLD);
	/* (its way of L in the corner under his feet, clear of his head) */
	int way = way_of(v, e);
	if (way >= 0) {
		fill_rect(r.x + 1, r.y + r.h - 8, 7, 7, v->kept == way && v->t / 4 % 2 ? PET_WHITE : PET_GOLD);
		strip_arrow(r.x + 2, r.y + r.h - 7, way, INK);
	}
}

/* a phrase's plate, lit under the cursor, its way of L at its left */
static void plate(int x, int y, int i, const StripView *v) {
	SDL_Rect r = strip_cell(1, i);
	int item = SQUARE_PHRASE + i, way = way_of(v, item);
	bool under = v->cursor == item;
	r.x += x;
	r.y += y;
	netpass_panel(r.x, r.y, r.w, r.h, under ? PET_NAVY : FACE_BACK, under ? PET_GOLD : PET_SLOT_EDGE);
	if (way >= 0) {
		fill_rect(r.x + 2, r.y + 3, 7, 7, v->kept == way && v->t / 4 % 2 ? PET_WHITE : PET_GOLD);
		strip_arrow(r.x + 3, r.y + 4, way, INK);
	}
	pixfont_draw(r.x + r.w / 2 + (way >= 0 ? 4 : 0), r.y + 3, item_name(item), under ? PET_WHITE : PET_CYAN_HI, INK, PIXFONT_CENTER, 1);
}

/* the favourites' box: L's four, as the D-pad holds them */
static void favourites(int x, int y, const StripView *v) {
	netpass_panel(x + FAV_X, y + CELLS_Y, FAV_W, 56, TRAY_BACK, PET_SLOT_EDGE);
	minifont_draw(x + FAV_X + 4, y + CELLS_Y + 4, strip_text_favourites, LOOK_EDGE_HI, 1);
	for (int k = 0; k < 4; ++k) {
		SDL_Rect r = strip_favourite(k);
		bool lit = v->kept == k;
		netpass_panel(x + r.x, y + r.y, r.w, r.h, lit ? PET_GOLD : PET_SLOT, lit ? PET_WHITE : PET_SLOT_EDGE);
		strip_arrow(x + r.x + 2, y + r.y + 3, k, lit ? INK : PET_GOLD);
		char name[16];
		snprintf(name, sizeof name, "%s", item_name(v->favourites[k]));
		/* (cut to the plate) */
		while (name[0] && minifont_width(name, 1) > r.w - 9) name[strlen(name) - 1] = 0;
		minifont_draw(x + r.x + 8, y + r.y + 3, name, lit ? INK : PET_WHITE, 1);
	}
	const char *hint = v->kept >= 0 ? strip_text_kept(v->kept) : strip_text_keep;
	minifont_draw_centered(x + FAV_X + FAV_W / 2, y + CELLS_Y + 47, hint, v->kept >= 0 ? PET_GOLD : LOOK_DIM, 1);
}

void strip_draw(int x, int y, const StripView *v) {
	preview(x, y, v);
	netpass_ground(x, y + TRAY_Y, NETPASS_W, NETPASS_H - TRAY_Y);
	band(x, y, v);
	if (v->page) for (int i = 0; i < ONLINE_PHRASE_COUNT; ++i) plate(x, y, i, v);
	else for (int e = 0; e < ONLINE_EMOTE_COUNT; ++e) icon(x, y, e, v);
	favourites(x, y, v);
}

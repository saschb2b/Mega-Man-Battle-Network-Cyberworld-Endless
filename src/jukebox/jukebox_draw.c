/* jukebox_draw.h. */
#include "jukebox_draw.h"

#include <math.h>
#include <stdio.h>

#include "gfx.h"
#include "jukebox_text.h"
#include "minifont.h"
#include "netpass_draw.h"
#include "second_frame.h"

#define W 240
#define HEAD_H 18
#define INK      rgba(0, 49, 74, 255)
#define TRAY     rgba(8, 41, 62, 255)       /* the NetPass's tray */
#define ROW_LINE rgba(10, 58, 80, 255)
#define ROW_LIT  rgba(24, 132, 160, 255)
#define LIGHT_OFF rgba(10, 58, 80, 255)

/* Trade Central's spares list's frame, the list wider for songs' names: the card on the left, the list on the right, the keys under them */
#define CARD_X 4
#define BODY_Y 21
#define CARD_W 88
#define BODY_H 113
#define LIST_X 96
#define LIST_W 140
#define ROW_H 16
#define ROWS_Y 17
#define BAR_Y 137
#define BAR_H 20
#define LIGHTS_Y 64   /* the tracks' lights from the card's top */
#define LIGHT_SEGS 8
#define LIGHT_SEG_H 3

/* a game's label colours, the ROMs screen's cartridges' (launcher_art.c): BN6 green, BN5 blue */
static SDL_Color game_color(int game, bool light) {
	static const SDL_Color c[2][2] = { { { 8, 189, 115, 255 }, { 74, 231, 115, 255 } }, { { 49, 156, 222, 255 }, { 107, 222, 255, 255 } } };
	return c[game ? 1 : 0][light];
}

/* a note, 6 x 8 from (x, y) */
static void note(int x, int y, SDL_Color c) {
	static const char *const rows[8] = { "..####", "..#..#", "..#..#", "..#..#", "..#..#", "###.##", "###.##", ".#...." };
	SDL_Rect px[30];
	int n = 0;
	for (int r = 0; r < 8; ++r)
		for (int i = 0; rows[r][i] && n < 30; ++i)
			if (rows[r][i] == '#') px[n++] = (SDL_Rect){ x + i, y + r, 1, 1 };
	fill_rects(px, n, c);
}

/* a triangle pointing left or right, 4 x 7 from (x, y) */
static void arrow(int x, int y, bool right, SDL_Color c) {
	for (int k = 0; k < 4; ++k) fill_rect(right ? x + k : x + 3 - k, y + k, 1, 7 - 2 * k, c);
}

/* ---- the header ---- */

SDL_Rect jukebox_mode_rect(const JukeView *v) {
	const char *mode = jukebox_mode_names[v->mode];
	int sw = text_width(mode) + 12;
	return (SDL_Rect){ W - 4 - sw, 2, sw, HEAD_H - 5 };
}

static void band(int x, int y, const JukeView *v) {
	fill_rect(x, y, W, HEAD_H, PET_GREEN);
	fill_rect(x, y, W, 1, PET_LINE);
	fill_rect(x, y + HEAD_H - 2, W, 2, PET_DARK);
	text_draw(x + 7, y + 3, jukebox_title, PET_WHITE, TEXT_LEFT);
	second_stripes(x + 11 + text_width(jukebox_title), y + 3);
	SDL_Rect m = jukebox_mode_rect(v);
	second_slot(x + m.x, y + m.y, m.w, m.h);
	text_draw(x + m.x + 6, y + 2, jukebox_mode_names[v->mode], PET_CYAN_HI, TEXT_LEFT);
}

/* ---- the card: the song playing ---- */

/* the tracks' lights: a column of segments a track, lit to its loudness */
static void lights(int x, int y, const JukeView *v) {
	int n = v->tracks ? v->tracks : JUKE_TRACKS, cw = (CARD_W - 10 - (n - 1) * 2) / n;
	for (int i = 0; i < n; ++i) {
		/* (by the loudness's root, as a level meter's decibels lean: a quiet track still shows) */
		int lit = v->playing >= 0 ? (int)lroundf(sqrtf((float)v->levels[i] / 255.0f) * LIGHT_SEGS) : 0;
		for (int s = 0; s < LIGHT_SEGS; ++s) {
			int sy = y + (LIGHT_SEGS - 1 - s) * (LIGHT_SEG_H + 1);
			SDL_Color c = s >= lit ? LIGHT_OFF : s >= LIGHT_SEGS - 2 ? PET_GOLD : s >= LIGHT_SEGS - 4 ? PET_CYAN_HI : PET_CYAN;
			fill_rect(x + i * (cw + 2), sy, cw, LIGHT_SEG_H, c);
		}
	}
}

static void card(int x, int y, const JukeView *v) {
	netpass_panel(x, y, CARD_W, BODY_H, PET_SLOT, PET_CYAN);
	fill_rect(x + 2, y + 1, CARD_W - 4, 1, rgba(33, 123, 132, 255));
	bool on = v->playing >= 0;
	const JukeSong *s = on ? &jukebox_songs[v->playing] : NULL;
	/* (the game's label, its cartridge's colours) */
	int game = on ? s->game : v->game;
	const char *tag = jukebox_game_tags[game ? 1 : 0];
	int tw = minifont_width(tag, 1) + 8;
	netpass_panel(x + 4, y + 4, tw, 11, game_color(game, false), INK);
	fill_rect(x + 5, y + 5, tw - 2, 1, game_color(game, true));
	minifont_draw(x + 8, y + 7, tag, PET_WHITE, 1);
	minifont_draw(x + tw + 8, y + 7, on ? jukebox_now_label : jukebox_idle_label, on ? PET_CYAN_HI : PET_DIM, 1);
	if (!on) {
		second_wrapped(jukebox_pick_hint, x + 5, y + 20, CARD_W - 10, 3, PET_WHITE);
		lights(x + 5, y + LIGHTS_Y, v);
		return;
	}
	second_wrapped(s->title, x + 5, y + 18, CARD_W - 10, 2, PET_WHITE);
	char num[16], time[16];
	jukebox_number_words(num, sizeof num, s->song);
	snprintf(time, sizeof time, "%d:%02d", v->seconds / 60, v->seconds % 60);
	minifont_draw(x + 5, y + 50, num, PET_GOLD, 1);
	minifont_draw(x + CARD_W - 5 - minifont_width(time, 1), y + 50, time, PET_WHITE, 1);
	lights(x + 5, y + LIGHTS_Y, v);
	char loops[32];
	jukebox_loops_words(loops, sizeof loops, v->loops);
	minifont_draw(x + 5, y + LIGHTS_Y + LIGHT_SEGS * (LIGHT_SEG_H + 1) + 6, loops, PET_DIM, 1);
}

/* ---- the list: the section's songs ---- */

SDL_Rect jukebox_row_rect(int i) { return (SDL_Rect){ LIST_X + 3, BODY_Y + ROWS_Y + i * ROW_H, LIST_W - 10, ROW_H }; }

SDL_Rect jukebox_turn_rect(int right) { return (SDL_Rect){ right ? LIST_X + LIST_W - 18 : LIST_X, BODY_Y, 18, 15 }; }

static void row(int x, int y, int w, int song, bool lit, bool playing) {
	if (lit) {
		fill_rect(x, y, w, ROW_H, PET_CYAN_HI);
		fill_rect(x + 1, y + 1, w - 2, ROW_H - 2, ROW_LIT);
	} else fill_rect(x + 2, y + ROW_H - 1, w - 4, 1, ROW_LINE);
	const JukeSong *s = &jukebox_songs[song];
	if (playing) note(x + 4, y + 4, PET_GOLD);
	text_draw(x + 13, y + 2, s->title, playing ? PET_GOLD : PET_WHITE, TEXT_LEFT);
}

static void list_panel(int x, int y, const JukeView *v) {
	netpass_panel(x, y, LIST_W, BODY_H, TRAY, PET_SLOT_EDGE);
	/* (the section on a slot between its arrows: the game's tag, its name, its place) */
	second_slot(x + 14, y + 2, LIST_W - 28, 12);
	char name[40];
	snprintf(name, sizeof name, "%s %s", jukebox_game_tags[v->game ? 1 : 0], v->section ? v->section : "");
	minifont_draw(x + LIST_W / 2 - minifont_width(name, 1) / 2, y + 5, name, PET_CYAN_HI, 1);
	bool turns = v->sections > 1;
	arrow(x + 6, y + 5, false, turns ? PET_GOLD : PET_DIM);
	arrow(x + LIST_W - 10, y + 5, true, turns ? PET_GOLD : PET_DIM);
	bool bar = v->n > JUKE_ROWS;
	int rw = LIST_W - 6 - (bar ? 4 : 0);
	for (int i = 0; i < JUKE_ROWS && v->top + i < v->n; ++i) {
		int song = v->rows[v->top + i];
		row(x + 3, y + ROWS_Y + i * ROW_H, rw, song, v->top + i == v->at, song == v->playing);
	}
	if (!bar) return;
	/* (the scroll bar: where the rows shown lie in the section) */
	int th = BODY_H - ROWS_Y - 4, tx = x + LIST_W - 6, ty = y + ROWS_Y;
	fill_rect(tx, ty, 2, th, ROW_LINE);
	int bh = th * JUKE_ROWS / v->n, by = ty + (th - bh) * v->top / (v->n - JUKE_ROWS);
	fill_rect(tx, by, 2, bh < 4 ? 4 : bh, PET_CYAN);
}

/* ---- the keys ---- */

static const char *const caps[4] = { "A", "B", "L R", "START" };

static const char *key_word(int k) {
	return k == 0 ? jukebox_key_play : k == 1 ? jukebox_key_back : k == 2 ? jukebox_key_section : jukebox_key_mode;
}

/* the keys laid out along the bar, from the picture's left */
SDL_Rect jukebox_key_rect(int k) {
	int x = 4 + 5;
	for (int i = 0; i < 4; ++i) {
		int w = minifont_width(caps[i], 1) + 5 + 3 + minifont_width(key_word(i), 1);
		if (i == k) return (SDL_Rect){ x - 2, BAR_Y + 2, w + 4, BAR_H - 4 };
		x += w + 12;
	}
	return (SDL_Rect){ 0, 0, 0, 0 };
}

static void keys_bar(int x, int y) {
	netpass_panel(x + 4, y + BAR_Y, W - 8, BAR_H, TRAY, PET_SLOT_EDGE);
	for (int k = 0; k < 4; ++k) {
		SDL_Rect r = jukebox_key_rect(k);
		int cx = x + r.x + 2, cy = y + BAR_Y + 6, cw = minifont_width(caps[k], 1) + 5;
		netpass_panel(cx, cy, cw, 9, PET_CYAN_HI, PET_CYAN);
		minifont_draw(cx + 3, cy + 2, caps[k], INK, 1);
		minifont_draw(cx + cw + 3, cy + 2, key_word(k), PET_WHITE, 1);
	}
}

void jukebox_draw_view(int x, int y, const JukeView *v) {
	netpass_ground(x, y, W, 160);
	band(x, y, v);
	card(x + CARD_X, y + BODY_Y, v);
	list_panel(x + LIST_X, y + BODY_Y, v);
	keys_bar(x, y);
}

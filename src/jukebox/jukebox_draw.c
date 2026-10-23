/* jukebox_draw.h. */
#include "jukebox_draw.h"

#include <math.h>
#include <stdio.h>

#include "gfx.h"
#include "jukebox_text.h"
#include "pet_look.h"

#define W 240
#define H 160
#define LIGHT_OFF rgba(0, 90, 52, 255)
#define NONE (SDL_Color){ 0, 0, 0, 0 }

/* BN6's Library's frame: the song playing on the left in SubChip's flat
 * panel, the section's songs in the screen's one window on the right,
 * its name in the window's band between the page arrows, both to the
 * screen's foot */
#define CARD_X 7
#define BODY_Y 18
#define CARD_W 88
#define BODY_H 134
#define LIST_X 98
#define LIST_W 135
#define LIST_TOP 12   /* the section's name over the songs */
#define ROW_H 16
#define LIGHT_SEGS 8
#define LIGHT_SEG_H 3

/* a game's tag colours, the ROMs screen's cartridges' (launcher_art.c): BN6 green, BN5 blue */
static SDL_Color game_color(int game) {
	static const SDL_Color c[2] = { { 0, 140, 82, 255 }, { 16, 99, 165, 255 } };
	return c[game ? 1 : 0];
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

/* ---- the band ---- */

static void band(int x, int y, const JukeView *v) {
	look_screen(x, y, W, H, jukebox_title);
	look_band_hint(x, y, W, "START", jukebox_mode_names[v->mode]);
}

SDL_Rect jukebox_mode_rect(const JukeView *v) {
	/* (where the hint lies, as look_band_hint lays it: nothing is drawn here) */
	int ww = look_hint_width("START", jukebox_mode_names[v->mode]);
	return (SDL_Rect){ W - 10 - ww - 2, 9, ww + 4, 11 };
}

/* ---- the card: the song playing ---- */

/* the tracks' lights: a column of segments a track, lit to its loudness */
static void lights(int x, int y, int w, const JukeView *v) {
	int n = v->tracks ? v->tracks : JUKE_TRACKS, cw = (w - (n - 1) * 2) / n;
	for (int i = 0; i < n; ++i) {
		/* (by the loudness's root, as a level meter's decibels lean: a quiet track still shows) */
		int lit = v->playing >= 0 ? (int)lroundf(sqrtf((float)v->levels[i] / 255.0f) * LIGHT_SEGS) : 0;
		for (int s = 0; s < LIGHT_SEGS; ++s) {
			int sy = y + (LIGHT_SEGS - 1 - s) * (LIGHT_SEG_H + 1);
			SDL_Color c = s >= lit ? LIGHT_OFF : s >= LIGHT_SEGS - 2 ? LOOK_GOLD : s >= LIGHT_SEGS - 4 ? LOOK_EDGE_HI : LOOK_EDGE;
			fill_rect(x + i * (cw + 2), sy, cw, LIGHT_SEG_H, c);
		}
	}
}

static void card(int x, int y, const JukeView *v) {
	SDL_Rect f = look_panel(x, y, CARD_W, BODY_H);
	int fx = f.x + 2, fw = f.w - 4;
	bool on = v->playing >= 0;
	const JukeSong *s = on ? &jukebox_songs[v->playing] : NULL;
	look_caps(fx, f.y + 3, on ? jukebox_now_label : jukebox_idle_label, on ? LOOK_PAGE : LOOK_DIM, NONE);
	int game = on ? s->game : v->game;
	const char *tag = jukebox_game_tags[game ? 1 : 0];
	int ly = f.y + 42;
	if (!on) {
		lights(fx, ly + 12, fw, v);
		return;
	}
	look_wrapped(s->title, fx, f.y + 12, fw, 2, LOOK_TEXT, LOOK_PANEL_SHADE);
	/* (the game's tag and the song's number; under the lights how long it has played, how often round) */
	int tw = look_caps_width(tag) + 6;
	look_tag(fx, ly - 2, tw, 9, tag, game_color(game));
	char num[16], time[16], loops[32];
	jukebox_number_words(num, sizeof num, s->song);
	look_caps(fx + tw + 4, ly, num, LOOK_GOLD, NONE);
	lights(fx, ly + 12, fw, v);
	int ty = ly + 12 + LIGHT_SEGS * (LIGHT_SEG_H + 1) + 4;
	snprintf(time, sizeof time, "%d:%02d", v->seconds / 60, v->seconds % 60);
	look_caps(fx, ty, time, LOOK_TEXT, NONE);
	jukebox_loops_words(loops, sizeof loops, v->loops);
	look_caps(fx, ty + 8, loops, LOOK_DIM, NONE);
}

/* ---- the list: the section's songs ---- */

static SDL_Rect list_field(void) {
	return (SDL_Rect){ LIST_X + LOOK_FRAME, BODY_Y + LOOK_FRAME + LIST_TOP, LIST_W - LOOK_FRAME - LOOK_FRAME_BAR, BODY_H - 2 * LOOK_FRAME - LIST_TOP };
}

SDL_Rect jukebox_row_rect(int i) {
	SDL_Rect f = list_field();
	return (SDL_Rect){ f.x, f.y + i * ROW_H, f.w, ROW_H };
}

SDL_Rect jukebox_turn_rect(int right) { return (SDL_Rect){ right ? LIST_X + LIST_W - 22 : LIST_X, BODY_Y, 22, LOOK_FRAME + LIST_TOP }; }

static void row(int x, int y, int w, int song, bool lit, bool playing) {
	const JukeSong *s = &jukebox_songs[song];
	if (lit) look_cursor(x + 2, y + 4);
	if (playing) note(x + w - 10, y + 4, LOOK_GOLD);
	look_text(x + 13, y + 1, s->title, playing ? LOOK_GOLD : LOOK_TEXT, LOOK_SHADE, TEXT_LEFT);
}

static void list_panel(int x, int y, const JukeView *v) {
	bool bar = v->n > JUKE_ROWS;
	look_window(x + LIST_X, y + BODY_Y, LIST_W, BODY_H, LIST_TOP, true);
	/* (the section in the band over its songs, between its arrows: its game's tag and its name) */
	char name[40];
	snprintf(name, sizeof name, "%s %s", jukebox_game_tags[v->game ? 1 : 0], v->section ? v->section : "");
	look_text(x + LIST_X + LIST_W / 2, y + BODY_Y + 3, name, LOOK_TEXT, LOOK_EDGE_LO, TEXT_CENTER);
	/* (the Library's page arrows, where L and R turn to another section) */
	if (v->sections > 1) {
		look_arrow(x + LIST_X + 4, y + BODY_Y + 5, false);
		look_arrow(x + LIST_X + LIST_W - 12, y + BODY_Y + 5, true);
	}
	for (int i = 0; i < JUKE_ROWS && v->top + i < v->n; ++i) {
		SDL_Rect r = jukebox_row_rect(i);
		int song = v->rows[v->top + i];
		row(x + r.x, y + r.y, r.w, song, v->top + i == v->at, song == v->playing);
	}
	if (bar) look_scroll((SDL_Rect){ x + LIST_X, y + BODY_Y, LIST_W, BODY_H }, LIST_TOP, v->top, JUKE_ROWS, v->n);
}

void jukebox_draw_view(int x, int y, const JukeView *v) {
	band(x, y, v);
	card(x + CARD_X, y + BODY_Y, v);
	list_panel(x, y, v);
}
